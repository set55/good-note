# 怎麼讓「整個系統」都走 `all_proxy=socks5://...`？—— 沒有這種東西

> 日期：2026-09-20
> 情境：想讓系統上所有程式都走 `all_proxy=socks5://192.168.1.1:1080`
> 對應檔案：`~/.zshrc`（個人用，不入 git）、`/etc/apt/apt.conf.d/`、
> `/etc/systemd/system/docker.service.d/http-proxy.conf`（系統設定，本機已存在）
> 適用範圍：Ubuntu 24.04／任何 systemd + GNOME 的桌面 Linux
> 相關：[SSH走代理設定.md](./SSH走代理設定.md)、[Docker拉取映像檔失敗-daemon缺proxy排查.md](../docker/Docker拉取映像檔失敗-daemon缺proxy排查.md)

**一句話結論：Linux 核心沒有「全系統代理」這個開關——`all_proxy` 只是一個環境變數慣例，只有「自己願意讀它」的程式才會理你。所以「讓所有程式走 proxy」不是一個設定，而是四層選擇：環境變數（對願意讀的程式）→ 各程式自己的設定檔（對不讀環境變數的）→ `LD_PRELOAD` 攔截（對不合作的）→ TUN 虛擬網卡（對全部，這才是真的「全系統」）。**

---

## 目錄

- [一、先推翻前提：代理不是系統功能](#一先推翻前提代理不是系統功能)
- [二、四層方案與怎麼選](#二四層方案與怎麼選)
- [三、第一層：環境變數要寫在哪裡](#三第一層環境變數要寫在哪裡)
- [四、環境變數的五個坑](#四環境變數的五個坑)
- [五、第二層：不讀環境變數的程式要各自設定](#五第二層不讀環境變數的程式要各自設定)
- [六、第三層：`LD_PRELOAD` 強制攔截（proxychains）](#六第三層ld_preload-強制攔截proxychains)
- [七、第四層：TUN 模式——唯一真正的「全系統」](#七第四層tun-模式唯一真正的全系統)
- [八、驗證方法](#八驗證方法)
- [自我測驗](#自我測驗)

---

## 一、先推翻前提：代理不是系統功能

Windows 有「系統 Proxy 設定」（WinINET），macOS 有網路偏好設定的代理欄位，兩者作業系統層有統一入口。**Linux 沒有。**

原因在於代理發生的位置：

```
應用程式  ← proxy 在這一層（程式自己決定要不要把連線交給 proxy）
--------
socket API / TCP / IP   ← 核心只認 IP 與 port，不知道「proxy」是什麼概念
--------
網路卡
```

核心的 `connect()` 只會照你給的 IP 去連。**要走 SOCKS5，得由程式自己先連到 proxy、再用 SOCKS5 協定說「幫我轉接到 example.com:443」**——這是應用層的行為。所以：

- `http_proxy` / `all_proxy` 這些變數**不是核心讀的**，是 libcurl、Python requests、Go 的 `net/http` 這些函式庫各自去讀的。
- 沒去讀的程式（`ssh`、`ping`、`nc`、絕大多數自己寫 socket 的程式）**設了也完全沒用**。
- `ping` 永遠不可能走 SOCKS5——SOCKS5 只代理 TCP 與 UDP associate，ICMP 根本不在協定裡。

**這就是為什麼「讓所有程式走 proxy」必須分層處理，而不是找一個設定檔。**

---

## 二、四層方案與怎麼選

| 層 | 做法 | 涵蓋範圍 | 代價 |
|---|---|---|---|
| L1 | 環境變數（`all_proxy` 等） | 只有讀環境變數的程式（curl、git、pip、Go/Python 的 HTTP client） | 最簡單，但涵蓋率最低 |
| L2 | 各程式自己的設定檔 | apt、docker daemon、snap、GNOME GUI、systemd 服務 | 要一個一個設，但最穩、最可控 |
| L3 | `proxychains4`（`LD_PRELOAD` 攔 `connect()`） | 動態連結且你願意加前綴跑的程式 | 對靜態連結／Go 程式無效，只能逐次呼叫 |
| L4 | TUN 虛擬網卡 + 路由（clash-verge／sing-box 的 TUN mode） | **真正全部**，含 ssh、ping、遊戲、靜態連結程式 | 要 root／`CAP_NET_ADMIN`，設錯會斷網 |

**選法：**

- 只想讓終端機的下載、`git clone`、`pip install` 走 → **L1 + L2 就夠**，不要碰 L4。
- 有特定程式不合作、只是偶爾要繞 → **L3**（`proxychains4 <cmd>`）。
- 真的要「整個系統」，包含 ssh、ping、不合作的商用軟體 → **只有 L4**。你機器上已經裝了 `clash-verge`，打開它的 TUN 模式就是這一層。

---

## 三、第一層：環境變數要寫在哪裡

**關鍵在「哪個檔案會被哪種程式讀到」。** 本機實測（用 `env -i` 清空環境後啟動各種 shell）：

| 設定位置 | 互動終端機 | `zsh -c` 腳本／cron | login shell | systemd user service | GUI（從選單點開） |
|---|:---:|:---:|:---:|:---:|:---:|
| `~/.zshrc`（現況） | ✅ | ❌ | ❌ | ❌ | ❌ |
| `~/.zshenv` | ✅ | ✅ | ✅ | ❌ | ❌ |
| `/etc/environment` | ✅ | ✅ | ✅ | ❌ | ✅ |
| `~/.config/environment.d/*.conf` | ✅ | ✅ | ✅ | ✅ | ✅ |
| systemd unit 的 `Environment=` | — | — | — | ✅ | — |

實測輸出（`all_proxy` 只寫在 `~/.zshrc` 時）：

```
1. 互動 zsh（你平常的終端機）  : socks5://192.168.1.1:1080
2. 非互動 zsh -c（腳本、cron） : （空）
3. login shell zsh -l          : （空）
4. sh -c（腳本 #!/bin/sh）     : （空）
5. systemd user service        : （空）
6. gnome-shell 的 /proc/PID/environ : （沒有任何 proxy 變數）
```

**為什麼？** zsh 的啟動檔各有適用情境：`.zshenv` 每次都讀、`.zshrc` **只有互動式** shell 讀、`.zprofile` 只有 login shell 讀。把 export 放在 `.zshrc`，等於宣告「只有我手打指令時才走 proxy」。

### 建議的寫法

**桌面與 GUI 也要涵蓋** → `/etc/environment`（由 PAM 的 `pam_env` 在登入時套用，圖形登入也算）：

```bash
sudo tee -a /etc/environment >/dev/null <<'EOF'
all_proxy=socks5h://192.168.1.1:1080
http_proxy=http://192.168.1.1:8880
https_proxy=http://192.168.1.1:8880
no_proxy=localhost,127.0.0.1,::1,192.168.0.0/16,10.0.0.0/8,172.16.0.0/12,*.local
EOF
```

> **`/etc/environment` 不是 shell script**：不能寫 `export`、不能用 `$VAR` 展開、不能有 `if`。寫錯不會報錯，只會安靜地不生效。

**只要自己用、且要涵蓋 systemd user service** → `~/.config/environment.d/proxy.conf`（同樣是 `KEY=value` 格式，不是 script）。

**要重開機才生效**：這兩種都是登入時載入，改完要重新登入（或 `systemctl --user import-environment` 手動補）。

---

## 四、環境變數的五個坑

### 1. `all_proxy` 不是通用標準——`wget` 根本不支援

實測：`wget --help` 裡「socks」出現 **0 次**；man page 只列 `http_proxy`、`https_proxy`、`ftp_proxy`。**wget 完全不支援 SOCKS**，你設 `all_proxy` 它視而不見。要讓 wget 走 SOCKS，只能靠 L3/L4，或改用 `curl`。

`all_proxy` 主要是 **curl／libcurl 系**的慣例（`git`、`pip`、`aria2` 等跟進）。

### 2. `socks5://` 與 `socks5h://`：DNS 在誰那邊解析

這是最容易出事、也最少人知道的一點。本機實測：

```
$ curl -v -x socks5://192.168.1.1:1080 https://example.com
* Host example.com:443 was resolved.
* SOCKS5 connect to 104.20.23.154:443 (locally resolved)      ← 本機先查 DNS

$ curl -v -x socks5h://192.168.1.1:1080 https://example.com
* SOCKS5 connect to example.com:443 (remotely resolved)       ← 把域名交給 proxy 查
```

| | 誰解析 DNS | 後果 |
|---|---|---|
| `socks5://` | **本機** | DNS 查詢不走 proxy（會洩漏你在查什麼）；本機 DNS 被污染或查不到該域名時，連線直接失敗 |
| `socks5h://` | **proxy 端** | DNS 也走代理；翻越 DNS 污染要用這個 |

**`h` 就是 hostname 的意思。** 如果你設代理的目的包含「繞過 DNS 污染／存取內網域名」，`socks5://` 會讓你白忙一場——這時要用 `socks5h://`。

### 3. `no_proxy` 的比對規則很不一致

`no_proxy=192.168.0.0/16` 這種 CIDR 寫法，**curl 7.86 以後才支援**，更舊的版本與很多語言的 HTTP client（Python requests、Go）只做字尾字串比對，CIDR 會被當成無意義字串。跨工具最保險的是列舉：

```
no_proxy=localhost,127.0.0.1,::1,.local,.internal,192.168.1.1
```

另外別漏掉 **proxy 自己的位址**（`192.168.1.1`），否則某些程式會試圖「透過 proxy 連 proxy」。

### 4. 大小寫：`HTTP_PROXY` 有安全考量

curl 對 `http_proxy` **只讀小寫**，這是為了擋 httpoxy 漏洞（CVE-2016-5385）——CGI 環境會把 HTTP 標頭 `Proxy:` 變成環境變數 `HTTP_PROXY`，攻擊者就能劫持伺服器的對外連線。其餘變數（`https_proxy`、`all_proxy`、`no_proxy`）大小寫都讀。**兩種都設是最保險的做法**（Java、Go 等偏好大寫）。

### 5. `sudo` 預設會把環境變數清掉

sudoers 預設 `env_reset`，所以 `sudo curl ...` 拿不到你的 proxy 變數——這是「我明明設了，為什麼 sudo 執行就不通」的頭號原因。兩種解法：

```bash
sudo -E curl https://example.com        # 臨時：-E 保留當前環境
# 永久：sudo visudo，加上
Defaults env_keep += "http_proxy https_proxy all_proxy no_proxy HTTP_PROXY HTTPS_PROXY ALL_PROXY NO_PROXY"
```

---

## 五、第二層：不讀環境變數的程式要各自設定

| 程式 | 設定位置 | 備註 |
|---|---|---|
| **apt** | `/etc/apt/apt.conf.d/80proxy`（本機實際檔名；`apt.conf.d/` 下檔名自訂，依字母序讀入） | `Acquire::http::Proxy "http://...";` 與 `Acquire::https::Proxy`（本機兩者都設 8880）。apt **會讀** `http_proxy` 環境變數（`man apt-transport-http`），但實務上一律 `sudo apt`，而 `sudo` 預設 `env_reset` 會清掉變數，所以要寫進設定檔才穩。用 `apt-config dump \| grep -i proxy` 確認實際生效值 |
| **apt 走 socks5** | `Acquire::http::Proxy "socks5h://192.168.1.1:1080";` | apt 支援的 scheme 只有 `socks5h`、`http`、`https`，**不支援 `socks5`**（只有 h 版） |
| **docker daemon**（pull 映像） | `/etc/systemd/system/docker.service.d/http-proxy.conf` | 改完要 `systemctl daemon-reload && systemctl restart docker`。本機已設 |
| **docker 容器內**（build／run） | `~/.docker/config.json` 的 `proxies` 欄位 | daemon 的設定**不會**傳進容器，這是兩回事 |
| **git** | `git config --global http.proxy socks5h://192.168.1.1:1080` | 只對 `http(s)://` 的 remote 有效；`git@github.com` 走 SSH，要改 `~/.ssh/config` 的 `ProxyCommand` |
| **npm / pip** | `~/.npmrc`、`~/.config/pip/pip.conf` | 會讀環境變數，但設定檔優先 |
| **snap** | `sudo snap set system proxy.http=...` | snapd 是系統服務，不讀你的環境變數 |
| **GNOME / Chrome / Electron 程式** | `gsettings set org.gnome.system.proxy mode 'manual'` 等 | 本機目前是 `'none'`。Chrome 系會讀 GNOME 設定 |
| **任何 systemd 服務** | `/etc/systemd/system/<svc>.service.d/proxy.conf` 的 `Environment=` | systemd 服務**完全不繼承**你的 shell 環境 |

「所有 systemd 服務都走 proxy」可以用 `/etc/systemd/system.conf` 的 `DefaultEnvironment=`，但**不建議**：開機早期的服務（網路還沒起來）也會套用，出問題時很難排查。

---

## 六、第三層：`LD_PRELOAD` 強制攔截（proxychains）

你機器上已裝好 `proxychains4`。原理是用 `LD_PRELOAD` 把自己的 `connect()` 插到程式前面，攔下每一個對外連線再改送 SOCKS5：

```bash
# /etc/proxychains4.conf 的 [ProxyList] 改成
socks5 192.168.1.1 1080

proxychains4 wget https://example.com     # 連不支援 SOCKS 的 wget 都能走
proxychains4 -q nc example.com 443
```

設定檔裡兩個關鍵選項（本機現況是 `strict_chain` + `proxy_dns`）：

- `strict_chain` / `dynamic_chain`：前者要求鏈上每個 proxy 都通，後者自動跳過掛掉的。
- `proxy_dns`：把 DNS 查詢也導向 proxy——效果等同 `socks5h`。

**失效情況（重要）：**

- **靜態連結的程式攔不到**——`LD_PRELOAD` 只對動態連結有效。
- **Go 程式通常攔不到**：Go 預設靜態連結，而且直接下 syscall，不經過 libc 的 `connect()`。
- setuid 程式（如 `ping`）會忽略 `LD_PRELOAD`。
- 要逐個指令加前綴，不是「全系統」。

---

## 七、第四層：TUN 模式——唯一真正的「全系統」

這層不再問程式願不願意合作，而是**從路由下手**：建一張虛擬網卡（TUN），把預設路由指過去，核心就把所有 IP 封包交給它；使用者空間的程式（clash-verge / sing-box / tun2socks）收到封包後，解析出目的地，再用 SOCKS5 送出去。

```
程式（毫不知情）→ connect() → 核心路由表 → tun0 → clash/sing-box → SOCKS5 → 192.168.1.1:1080
```

因為攔截點在 IP 層，**ssh、ping（ICMP 由工具自行處理）、遊戲、靜態連結程式全都涵蓋**，程式完全不需要支援 proxy。

你機器上已經有 `clash-verge`，所以最省事的做法就是在它的設定裡開 **TUN Mode（服務模式）**，把 `192.168.1.1:1080` 設為 outbound。

**代價與風險：**

- 需要 root 或 `CAP_NET_ADMIN`（clash-verge 會裝一個 service helper）。
- 會改動預設路由——設錯就斷網。**務必先確認 proxy 本身的位址被排除**（`192.168.1.1` 要走原本的路由，否則封包會繞回 tun0 形成迴圈，整台機器斷線）。
- DNS 要一起處理（fake-ip 或 DNS 劫持），否則域名解析仍走原本的路。
- 排查時多一層：`ip route` 看到的 `default dev tun0` 就是它幹的（見 [查詢轉發表與路由表.md](./查詢轉發表與路由表.md)）。

> 另一個等價作法是 `redsocks` + iptables `REDIRECT`（把 TCP 導進本機的 redsocks 再轉 SOCKS5），概念相同但只能處理 TCP，且 iptables 規則寫錯同樣會斷網。

---

## 八、驗證方法

```bash
# 1. 變數到底有沒有進到「那個」程式（不要只看你自己的 shell）
env | grep -i proxy                       # 當前 shell
sudo env | grep -i proxy                  # sudo 之後還在嗎（多半不在）
tr '\0' '\n' < /proc/<PID>/environ | grep -i proxy    # 某個已在跑的程式真正拿到什麼
systemctl show <svc> -p Environment       # systemd 服務拿到什麼

# 2. 連線實際走了哪條路
curl -v -x socks5h://192.168.1.1:1080 https://example.com 2>&1 | grep SOCKS5
curl -s https://ifconfig.me; echo        # 出口 IP 是不是 proxy 的
ss -tnp | grep 1080                      # 有沒有真的連到 proxy 的 1080

# 3. 分辨「沒走 proxy」與「proxy 壞了」
curl --noproxy '*' -sS -o /dev/null -w '%{http_code}\n' https://example.com   # 強制不走 proxy 對照
nc -zv 192.168.1.1 1080                  # proxy 本身通不通
```

**心法：永遠用「同一個請求走／不走 proxy」做對照組**，才能分清楚是沒設到、還是 proxy 本身有問題——這就是 [排查方法論.md](./排查方法論.md) 的對照組原則。

---

## 九、本機現況與建議

| 現況 | 問題 |
|---|---|
| `~/.zshrc` 有 export（4 個變數） | 只有互動終端機有效；腳本、cron、systemd、GUI 全都拿不到 |
| `/etc/apt/apt.conf.d/` 已設 http 8880 | ✅ apt 正常 |
| docker drop-in 已設 8880 | ✅ daemon pull 正常；但容器內沒有（需 `~/.docker/config.json`） |
| GNOME proxy mode = `none` | Chrome／Electron 系 GUI 不走 proxy |
| `http_proxy` 用 8880、`all_proxy` 用 1080 | 兩條不同通道，行為不一致——讀 `all_proxy` 的走 socks5，讀 `http_proxy` 的走 http |
| `all_proxy` 用的是 `socks5://` | DNS 在本機解析，繞不過 DNS 污染。建議改 `socks5h://` |

**建議順序：**

1. 把 `~/.zshrc` 那四行搬到 `/etc/environment`（涵蓋 GUI 與腳本），`socks5` 改成 `socks5h`。
2. `sudo visudo` 加 `env_keep`，讓 `sudo` 也帶得過去。
3. 需要 GUI 瀏覽器走 proxy → 設 `gsettings org.gnome.system.proxy`。
4. 真的要「所有程式」（含 ssh、ping、不合作的軟體）→ 開 clash-verge 的 TUN 模式，前三步就可以全部省略。

---

## proxychains4 的完整選項

```
proxychains4 [-q] [-f 設定檔] <程式> [參數...]
```

**它只有兩個選項：**

| 選項 | 作用 |
|---|---|
| `-q` | **安靜模式**（蓋過設定檔裡的 `quiet_mode`）——不印那些 `[proxychains] Strict chain ...` |
| `-f <設定檔>` | **指定設定檔**，不用預設的搜尋順序 |

**真正的設定全在設定檔裡，不在命令列。** 設定檔的搜尋順序是：
`-f` 指定的 → `./proxychains.conf` → `$HOME/.proxychains/proxychains.conf` →
`/etc/proxychains4.conf`。

設定檔的關鍵項目（本機 `/etc/proxychains4.conf`）：

| 項目 | 作用 |
|---|---|
| `strict_chain` | 依序經過清單裡**每一個** proxy，任一掛掉就失敗 |
| `dynamic_chain` | 同上但**自動跳過掛掉的** |
| `random_chain` | 隨機挑一個 |
| `round_robin_chain` | 輪流 |
| `proxy_dns` | **把 DNS 查詢也導向 proxy**（效果等同 `socks5h`） |
| `remote_dns_subnet` | 假 IP 的網段（預設 224） |
| `tcp_read_time_out` / `tcp_connect_time_out` | 逾時（毫秒） |
| `localnet <網段>` | **排除不走 proxy 的網段** |
| `[ProxyList]` | 每行一個：`<類型> <主機> <埠> [帳號] [密碼]`，類型為 `http`／`socks4`／`socks5` |

## 自我測驗

1. 為什麼 Linux 沒有像 Windows 那樣的「系統代理」開關？從 `connect()` 發生在哪一層說明，並解釋為什麼 `ping` 不管怎麼設都不會走 SOCKS5。
2. `socks5://` 和 `socks5h://` 差在哪？如果你設代理的目的是繞過 DNS 污染，用錯哪一個會完全失效、為什麼？
3. 你把 `export all_proxy=...` 寫在 `~/.zshrc`，終端機裡 `curl` 正常，但 cron 裡的備份腳本與從選單點開的 GUI 程式都不走 proxy。用「哪個檔案被哪種 shell 讀」解釋，並說出兩個正確的存放位置。
4. `sudo apt update` 走了 proxy，但 `sudo curl https://example.com` 卻直連。這兩個為什麼行為不同？各自的設定來源是什麼？
5. 情境題：你用 `proxychains4` 跑一個 Go 寫的 CLI 工具，發現它完全沒走 proxy，但同樣指令跑 `wget` 就正常。請說明原因，並指出要讓這支 Go 工具走 proxy 有哪兩條路可走。

（以下 6～8 題是 2026-09-23 追問「apt 走代理的設定在哪」時新增的，答案在第五節的 apt 那兩列）

6. 你在 `~/.zshrc` 設了 `export http_proxy=...`，然後 `sudo apt update` 沒走代理——可是 apt 明明會讀 `http_proxy`。問題出在哪？
7. 同事只設了 `Acquire::http::Proxy "http://proxy:8880/";`，`apt update` 時某個 `https://download.docker.com` 的來源仍然直連。為什麼？
8. 有人寫 `Acquire::http::Proxy "socks5://192.168.1.1:1080";` 想讓 apt 走 SOCKS。會成功嗎？該怎麼改？那個 `h` 代表什麼差別？
