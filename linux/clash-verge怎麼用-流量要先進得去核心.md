# clash-verge 怎麼用？—— GUI 只是外殼，流量得先「進得去」核心

> 日期：2026-09-20
> 情境：想用 clash-verge 在本機做 L4（第四層）代理，取代只對部分程式有效的環境變數
> 對應檔案：`~/.local/share/io.github.clash-verge-rev.clash-verge-rev/`（個人資料目錄，不入 git）、
> `/etc/systemd/system/clash-verge-service.service`（安裝套件時產生的系統設定）
> 適用範圍：Clash Verge Rev 2.2.3、核心 verge-mihomo、Ubuntu 24.04
> 延伸自：[讓整個系統走SOCKS5代理.md](./讓整個系統走SOCKS5代理.md)

**一句話結論：clash-verge 是 GUI 外殼，真正幹活的是 `verge-mihomo` 核心；GUI 只做三件事——管 profile、產生核心的 yaml、用 RESTful API 指揮核心。所以「怎麼用」的重點不是按哪個鈕，而是先回答一個問題：你的流量要用哪種方式進到核心？（系統代理／TUN／手動指定埠）沒選任何一種，核心就只是空轉。**

---

## 目錄

- [一、四個角色：誰負責什麼](#一四個角色誰負責什麼)
- [二、流量進入核心的三條路](#二流量進入核心的三條路)
- [三、系統代理與 TUN 模式：差在「攔截點」](#三系統代理與-tun-模式差在攔截點)
- [四、實測：核心在跑，但流量是零](#四實測核心在跑但流量是零)
- [五、開 TUN 模式（L4）](#五開-tun-模式l4)
- [六、TUN 開啟後系統發生了什麼](#六tun-開啟後系統發生了什麼)
- [七、規則模式：流量怎麼被分流](#七規則模式流量怎麼被分流)
- [八、把自架的代理接進來](#八把自架的代理接進來)
- [九、六個會讓你誤判的坑](#九六個會讓你誤判的坑)
- [十、驗證與救援](#十驗證與救援)
- [自我測驗](#自我測驗)

---

## 一、四個角色：誰負責什麼

很多人把 clash-verge 當成一個程式，其實是四層：

```
Clash Verge（GUI，Tauri 寫的桌面程式）
  │  ① 管理 profile（訂閱／本地 yaml）
  │  ② 把 profile + 你的設定合併，產生核心真正讀的 clash-verge.yaml
  │  ③ 透過 RESTful API（127.0.0.1:9097）即時指揮核心
  ▼
verge-mihomo（核心，真正處理封包的那個行程）
  ▲
  │  ④ 需要 root 的事（建 TUN 介面、改路由表）委託給它
clash-verge-service（systemd 服務，以 root 執行的 helper）
```

在本機對應到：

```bash
$ pgrep -a -f mihomo
29633 /usr/bin/verge-mihomo -d ~/.local/share/io.github.clash-verge-rev.clash-verge-rev \
                            -f ~/.local/share/io.github.clash-verge-rev.clash-verge-rev/clash-verge.yaml

$ systemctl is-active clash-verge-service
active
```

**這個分層的意義**：GUI 上每個開關，底層都只是「改 yaml 再叫核心 reload」或「打一個 API」。所以 GUI 壞了、看不懂選項時，可以直接看 `clash-verge.yaml` 或打 API 查現況——這比在介面上點來點去可靠得多。

資料目錄（Clash Verge Rev 的路徑，和舊版 `~/.config/clash-verge` 不同）：

| 檔案 | 用途 |
|---|---|
| `config.yaml` | **Verge 應用層**的設定：各種 port、TUN 開關、external-controller |
| `clash-verge.yaml` | **核心真正讀的**完整設定（profile + 上面那些合併後的產物，本機 354 KB） |
| `profiles/` | 你訂閱或自建的 profile 原始檔 |
| `logs/` | 核心日誌 |

---

## 二、流量進入核心的三條路

核心開了好幾個入口埠（本機 `config.yaml` 實況）：

| 埠 | 型別 | 說明 |
|---|---|---|
| `mixed-port: 7897` | HTTP + SOCKS5 **混合** | 最常用；同一個埠兩種協定都收 |
| `socks-port: 7898` | 純 SOCKS5 | |
| `port: 7899` | 純 HTTP | |
| `redir-port: 7895` | REDIRECT 透明代理 | 給 iptables `REDIRECT` 用，只支援 TCP |
| `tproxy-port: 7896` | TPROXY 透明代理 | 支援 TCP + UDP |

但**開著埠不等於有流量**。要讓程式的連線走進來，只有三種方式：

| 方式 | 做法 | 涵蓋範圍 | 對應前一篇的層級 |
|---|---|---|---|
| **系統代理** | GUI 開關 → 幫你設 GNOME 的 `org.gnome.system.proxy` 指向 `127.0.0.1:7897` | 讀系統／環境代理設定的程式 | L1 + L2 |
| **TUN 模式** | 建虛擬網卡 + 改路由，從 IP 層接管 | **所有程式** | **L4** |
| **手動指定** | 自己把程式指向 `127.0.0.1:7897` | 你手動設的那些 | L1 |

**這就是「怎麼用 clash-verge」的核心問題**：不是按哪個鈕，而是先決定用哪條路把流量送進核心。

---

## 三、系統代理與 TUN 模式：差在「攔截點」

GUI 上那兩個開關常被當成「兩種強度」，其實它們是**兩種完全不同的機制**。共同點只有一個：
兩者都只是把流量送進 mihomo 核心，**核心之後的行為（規則分流、選節點）完全一樣**。
差別全在攔截發生在哪一層。

### 系統代理：不攔任何東西，靠程式自願

它**不碰任何封包**，只是去改一組系統設定值，讓程式「自己願意」把連線交給 `127.0.0.1:7897`。
在 GNOME 上改的就是這組 gsettings key（本機實測，Verge 已經預填好，只差 `mode` 沒打開）：

```bash
$ gsettings get org.gnome.system.proxy mode
'none'                          # ← 開關本身，目前關著
$ gsettings get org.gnome.system.proxy.http host / port
'127.0.0.1' / 7897              # ← 指向 clash 的 mixed-port
$ gsettings get org.gnome.system.proxy ignore-hosts
['localhost', '127.0.0.1', '192.168.0.0/16', '10.0.0.0/8', ...]   # ← no_proxy 的 GUI 版
```

程式拿到這個設定後，是用**應用層協定**把請求託付給 clash——HTTP 的 `CONNECT example.com:443`，
或 SOCKS5 的交握。**程式知道自己在用代理。**

### TUN 模式：從 IP 層真的攔下來

它建一張虛擬網卡 `utun`（`198.18.0.1/30`），用 `auto-route` 把預設路由改指向它。之後核心從網卡
讀到的是**原始 IP 封包**，再用 gVisor 的使用者空間 TCP/IP 堆疊還原成 TCP／UDP 流交給規則引擎。

**程式毫不知情**——它以為自己在正常地連 `example.com`，不需要支援任何代理協定。這就是為什麼
`ssh`、遊戲、靜態連結的 Go 程式這些「不合作」的對象也能被涵蓋。

### 逐項對照

| | 系統代理 | TUN 模式 |
|---|---|---|
| 攔截點 | 不攔，靠程式自願 | IP 層，真的攔 |
| 改了什麼 | gsettings／環境變數 | 虛擬網卡 + 路由表 + DNS 劫持 |
| 程式知情嗎 | 知道（主動連 proxy） | 不知道 |
| 傳什麼給 clash | `CONNECT host:port` / SOCKS5 交握 | 原始 IP 封包 |
| `ssh` / 遊戲 / 靜態連結程式 | ❌ | ✅ |
| Docker 容器內 | ❌ | ❌（容器有自己的 netns） |
| UDP | 看程式支不支援 | ✅ |
| 需要 root | ❌ | ✅（`CAP_NET_ADMIN`，故需 service helper） |
| DNS 誰解析 | **clash／節點**（送的是域名） | 程式會自己查，**要靠 `dns-hijack` + fake-ip 攔** |
| 設錯的後果 | 程式連不上，關掉就好 | **可能整台機器斷網** |
| 對已在執行的程式 | gsettings 式多半即時；環境變數式無效 | 立即生效 |

### 最關鍵的差異：DNS

這一點決定了兩者的設定複雜度天差地遠：

- **系統代理**：`CONNECT example.com:443` 送出去的是**域名**，DNS 實際由 clash 或節點解析——
  天生就不會洩漏，不用額外設定。
- **TUN 模式**：程式會先自己發一個 UDP 53 的查詢。如果不攔，域名就由原本的 DNS 解析
  （洩漏），而且規則引擎只看得到 IP、比對不到域名規則。所以才需要 `dns-hijack: any:53`
  把查詢攔進核心，再用 fake-ip 回假位址。

**一句話：系統代理天生帶著域名走，TUN 模式得自己把域名搶回來。**

### 能不能用 `all_proxy` 指向 clash？可以，但要分清楚兩件事

**第一件：GUI 的「系統代理」開關和環境變數是兩條獨立管道。**
Verge 在 Linux 上按下那個開關，改的是 gsettings，它**不會也不可能**把環境變數塞進已經在跑的
shell 或 GUI 程式——環境變數只在行程啟動時繼承，事後無法注入。所以「開了系統代理」不等於
「`all_proxy` 被設好了」，反過來也一樣。

兩者涵蓋的對象幾乎互補：

| 管道 | 誰會讀 | 誰讀不到 |
|---|---|---|
| gsettings（系統代理開關） | Chrome／Electron／GNOME 應用程式 | 終端機裡的 `curl`、`git`、`pip` |
| 環境變數（`all_proxy` 等） | `curl`、`git`、`pip`、多數 CLI | 從選單啟動的 GUI 程式（實測 `gnome-shell` 的 environ 裡一個 proxy 變數都沒有） |

**要兩邊都覆蓋，就兩個都設，不是二選一。**

**第二件：手動 export 完全可行，但協定要選對。**
`mixed-port` 顧名思義同一個埠同時收 HTTP 與 SOCKS5，實測三種寫法都通：

```
curl -x http://127.0.0.1:7897     → 200
curl -x socks5://127.0.0.1:7897   → 200
curl -x socks5h://127.0.0.1:7897  → 200
```

但**三者送給 clash 的東西不一樣**，核心日誌是鐵證：

```
--> example.com:443       ← http://    送的是域名
--> 104.20.23.154:443     ← socks5://  送的是 IP，域名在本機就被解析掉了
--> example.com:443       ← socks5h:// 送的是域名
```

**`socks5://` 會讓 clash 的域名規則全部失效。** 因為 curl 先在本機把域名解析成 IP 才送出去，
核心收到的只有 `104.20.23.154`，那 7433 條 `DOMAIN-SUFFIX` 規則一條都比對不到，只剩 IP 類
規則（`IP-CIDR`、`GEOIP`）還有作用——分流結果會和你預期的完全不同。順帶一提，DNS 也是在本機
查的，等於繞過了 clash 的 DNS 設定。

所以指向 clash 時：

```bash
export all_proxy=socks5h://127.0.0.1:7897     # ← h 不能少
export http_proxy=http://127.0.0.1:7897
export https_proxy=http://127.0.0.1:7897
export no_proxy=localhost,127.0.0.1,::1,192.168.0.0/16,10.0.0.0/8,*.local
```

`no_proxy` 要含 `localhost`／`127.0.0.1`，否則連本機服務也會繞一圈 clash。

**這條路屬於第二節的第三種——「手動指定」，不是「系統代理」。** 它是 L1，只對讀環境變數的
程式有效；真正要涵蓋所有程式，仍然只有 TUN 模式。

### 為什麼不建議兩個一起開

流量會繞兩圈：程式 → 7897 → 核心 → 出站封包又撞上 TUN。mihomo 會標記自己的流量避免無窮
迴圈，所以通常不會壞，但你多了一層多餘處理，而且排查時根本分不清某條連線走的是哪條路。
**開 TUN 就把系統代理關掉。**

### 實測：出口 IP 沒變，不代表沒走 clash

```
對照組 A：curl --noproxy '*'        → 14.153.77.0
對照組 B：curl -x 127.0.0.1:7897    → 14.153.77.0     ← 同一個 IP！
核心的連線數：0 → 1，累計下行 0 → 5647 bytes          ← 但流量確實經過了核心
```

兩次出口 IP 一模一樣，很容易誤判成「代理沒生效」。實際上流量**有**進核心，核心日誌講得很清楚：

```
[TCP] 127.0.0.1:42494 --> example.com:443 doesn't match any rule using DIRECT
```

**沒有任何一條規則命中，於是落到預設的 DIRECT。** 本機那 7433 條規則多半是針對特定域名的清單，
最後沒有一條 `MATCH,<某個 group>` 的兜底規則，所以清單外的流量一律直連——這也解釋了為什麼
出口 IP 完全沒變。

**教訓：驗證代理有沒有在工作，要看核心的連線數與流量，不是看出口 IP。**
IP 沒變有兩種完全不同的原因——流量沒進來，或進來了但規則判直連——只有 `/connections` 分得出來。

### 該用哪一個

- 只要瀏覽器、`curl`、`git` 這些會讀設定的工具走代理 → **系統代理**，零風險、不需 root。
- 要涵蓋 `ssh`、遊戲、不合作的商用軟體，或你要的是第四層（L4）全系統代理 → **TUN 模式**。
- 兩者都達不到的：Docker 容器（要另外設）、其他 network namespace。

---

## 四、實測：核心在跑，但流量是零

本機當下的狀態，三條路一條都沒走：

```bash
$ gsettings get org.gnome.system.proxy
'none'                                   # ← 系統代理沒開

$ curl -s 127.0.0.1:9097/configs | jq '.tun.enable'
false                                    # ← TUN 沒開

$ ip -br link show type tun
(空)                                     # ← 沒有 tun 介面

$ env | grep -i proxy
all_proxy=socks5://192.168.1.1:1080      # ← 指向路由器，不是 clash 的 7897

$ curl -s 127.0.0.1:9097/connections | jq '{n: (.connections|length), up: .uploadTotal, down: .downloadTotal}'
{ "n": 0, "up": 0, "down": 0 }           # ← 核心一個位元組都沒處理過
```

**核心跑著、節點也測得到延遲、GUI 看起來一切正常——但它完全空轉。** 這是新手最常見的誤解：以為「程式開著就會生效」。

**心法：判斷 clash 有沒有在工作，不要看 GUI 有沒有開，要看 `/connections` 有沒有連線數。**

---

## 五、開 TUN 模式（L4）

### 前置條件

TUN 需要建立虛擬網卡、改路由表，這些要 `CAP_NET_ADMIN`（Capability for Network Administration，網路管理權限）。GUI 是一般使用者身分，所以必須有 **Service Mode**——也就是那個以 root 執行的 helper：

```bash
$ systemctl is-active clash-verge-service
active            # ✅ 本機已具備，可以直接開 TUN
```

若顯示 `inactive`，GUI 的「服務模式」頁有安裝鈕（背後執行
`/usr/lib/Clash Verge/resources/install-service-x86_64-unknown-linux-gnu`）。

### 步驟

1. GUI → **設定（Settings）** → 確認 **服務模式（Service Mode）** 已啟用。
2. 同頁打開 **TUN 模式（TUN Mode）**。
3. 確認 **系統代理（System Proxy）關掉**——TUN 已經從 IP 層接管，再套一層系統代理只會讓封包繞兩圈。
4. **把 shell 裡的 `http_proxy`／`all_proxy` 環境變數拿掉**（見第八節第 1 點，這是最容易誤判的坑）。

---

## 六、TUN 開啟後系統發生了什麼

不要把它當黑盒子。開啟後可以逐項驗證：

```bash
ip -br addr show                # 多一張 utun，位址 198.18.0.1/30
ip route show                   # default 被導向 utun（auto-route 幹的）
ip rule show                    # 多出 mihomo 用來避免迴圈的策略路由規則
```

本機 TUN 的設定（`config.yaml` 與 API 查到的一致）：

| 欄位 | 值 | 意義 |
|---|---|---|
| `stack` | `gvisor` | 用 gVisor 的使用者空間 TCP/IP 堆疊解析封包（另有 `system`／`mixed`） |
| `auto-route` | `true` | **自動改路由表**把預設路由指向 TUN——沒有這個，TUN 只是一張沒人用的網卡 |
| `auto-detect-interface` | `true` | 自動偵測真正的出口網卡，核心自己的連線走它而不是繞回 TUN |
| `strict-route` | `false` | 寬鬆模式，不強制攔截所有流量（見第八節第 4 點） |
| `dns-hijack` | `any:53` | **把所有送到 53 埠的 DNS 查詢劫持進核心** |
| `inet4-address` | `198.18.0.1/30` | TUN 介面自己的位址 |

搭配的 DNS 設定：

```yaml
dns:
  enable: true
  enhanced-mode: fake-ip          # 關鍵
  fake-ip-range: 198.18.0.1/16
  fake-ip-filter: ['*.lan', '*.local', ...]
```

**fake-ip 是 TUN 模式能運作的關鍵機制**：程式查 `example.com` 時，核心不真的去解析，而是回一個 `198.18.x.x` 的假位址；程式拿著假位址去連，封包進 TUN 後，核心用「假 IP ↔ 域名」的對照表還原出真正的域名，再交給規則引擎判斷該走哪個節點。

這樣做的理由：**規則是寫域名的**（`DOMAIN-SUFFIX,google.com,🚀 节点选择`），但 IP 層的封包裡只有 IP 沒有域名。先發假 IP，就能在封包回來時把域名找回來。代價是 `fake-ip-filter` 裡列的域名（`*.lan`、NTP 等）必須排除，否則本地服務會拿到假位址而連不上。

---

## 七、規則模式：流量怎麼被分流

`mode` 有三種，GUI 上就是那三顆按鈕：

| mode | 行為 |
|---|---|
| `rule` | **照規則分流**（預設，也是本機的設定）——每條連線比對 `rules` 決定走哪個 proxy-group |
| `global` | 全部走同一個節點，規則完全略過 |
| `direct` | 全部直連，等於只當個擺設（但 TUN 還在攔截） |

本機的規模：**7 個 Trojan 節點、3 個 proxy-group、7433 條規則**（規則多半來自訂閱附帶的規則集）。

proxy-group 是「一組節點 + 一個選擇策略」，規則指向 group 而不是直接指向節點，所以你換節點不用改規則：

```bash
$ curl -s 127.0.0.1:9097/proxies | jq -r '...'
  [Selector] 🚀 节点选择   → now=...      （5 個候選）   # Selector = 手動選
  [Selector] 🎯 全球直连   → now=DIRECT   （2 個候選）
  [Selector] 🐟 漏网之鱼   → now=🚀 节点选择（2 個候選）  # 沒被任何規則命中的流量
```

常見的 group 型別：`Selector`（手動）、`URLTest`（自動選延遲最低）、`Fallback`（主節點掛了才換）、`LoadBalance`（分散）。

規則的比對是**由上而下、第一個命中就決定**，最後一條通常是 `MATCH,🐟 漏网之鱼`。所以要讓某個域名走特定節點，規則要加在前面。

---

## 八、把自架的代理接進來

**重要前提**：訂閱來的節點和你自架的代理是兩回事。本機實測 `grep '192.168.1.1' clash-verge.yaml` **完全沒有命中**——目前這 7 個 Trojan 節點是訂閱提供的，clash 直接連它們，**沒有經過你自己的 OpenWrt v2ray**。

若要讓 clash 把流量交給自架的 SOCKS5（例如路由器上的 v2ray `192.168.1.1:1080`），在 profile 裡加一個節點：

```yaml
proxies:
  - name: "我的路由器-v2ray"
    type: socks5
    server: 192.168.1.1
    port: 1080
    udp: true          # 不加這行，UDP 不會走它

proxy-groups:
  - name: "🚀 节点选择"
    type: select
    proxies:
      - "我的路由器-v2ray"     # 加進候選，GUI 上就選得到
      - ...
```

Clash Verge 的正確做法**不是直接改 `clash-verge.yaml`**（它是產生物，下次更新訂閱就被覆蓋），而是用 **Profiles 頁的「全域擴展設定（Merge）」或「擴展腳本（Script）」**——這些會在每次產生設定時自動合併進去。本機 `profiles/Merge.yaml` 就是這個用途的檔案。

---

## 九、六個會讓你誤判的坑

### 1. 開了 TUN，卻還留著 proxy 環境變數（最常見）

你的 `~/.zshrc` 有 `all_proxy=socks5://192.168.1.1:1080`。開了 TUN 之後：

```
curl → 讀到 all_proxy → 連 192.168.1.1:1080（LAN 位址，規則判定 DIRECT，不進 TUN）
     → 流量走了路由器的 v2ray，clash 完全沒參與
     → 你以為「TUN 沒作用」，其實是流量根本沒進來
```

**開 TUN 就該把環境變數拿掉**，否則你測的一直是另一條路。

### 2. `/connections` 才是唯一可信的證據

GUI 顯示「已連線」、節點延遲測得到，都只代表**核心自己**連得上節點，不代表**你的程式**走了核心。看連線數。

### 3. TUN 不涵蓋的東西

- **Docker 容器**：容器有自己的 network namespace，主機的 TUN 路由管不到（容器要另外設 proxy，見 [Docker拉取映像檔失敗-daemon缺proxy排查.md](../docker/Docker拉取映像檔失敗-daemon缺proxy排查.md)）。
- **其他 network namespace**、某些以 root 身分且被規則排除的流量。

### 4. `strict-route: false` 的意義

寬鬆模式下，已經綁定特定網卡的流量、或核心判定該繞過的流量不會被強制拉進 TUN。好處是不容易把自己鎖死，壞處是**可能有漏網流量沒走代理**。要求「一個都不漏」時才開 `strict-route`，但它更容易造成斷網。

### 5. DNS 沒一起處理 = 白做

`dns-hijack: any:53` 只攔 53 埠的明文 DNS。若你的系統用 **DoH／DoT（DNS over HTTPS／TLS）**（走 443／853），那些查詢不會被攔到，域名還是由原本的 DNS 解析。本機是 systemd-resolved（見 [查詢目前使用的DNS伺服器.md](./查詢目前使用的DNS伺服器.md)），要確認它沒開 DoT。

### 6. 代理伺服器自己的位址必須排除

如果你把 `192.168.1.1:1080` 設成 clash 的節點，同時又開 TUN：核心連向 `192.168.1.1` 的封包若被自己的 TUN 攔回去，就會形成無窮迴圈、整台機器斷網。防止機制有兩層——`auto-detect-interface` 讓核心的出站連線走實體網卡，以及規則裡對私有網段的 DIRECT 判定。**自架代理在 LAN 上時，務必確認規則裡有 `GEOIP,private,DIRECT` 或等價的 `IP-CIDR` 規則。**

---

## 十、驗證與救援

```bash
# --- 核心狀態（API，唯讀）---
curl -s 127.0.0.1:9097/configs   | jq '{tun: .tun.enable, mode: .mode, port: ."mixed-port"}'
curl -s 127.0.0.1:9097/connections | jq '.connections | length'    # ← 有沒有真的在處理流量
curl -s 127.0.0.1:9097/proxies   | jq -r '.proxies.GLOBAL.now'     # 目前選中的節點

# --- 系統層 ---
ip -br addr show                  # utun 在不在、位址對不對
ip route show | head              # default 是不是指向 utun
resolvectl query example.com      # 回 198.18.x.x → fake-ip 生效中

# --- 端到端 ---
curl -s https://ifconfig.me; echo            # 出口 IP 變了沒
curl -s --interface utun https://ifconfig.me # 指定走 TUN 對照

# --- 救援（照順序）---
# 1. GUI 關掉 TUN 開關（最正常的做法）
# 2. GUI 沒反應 → 停掉核心，路由會自動還原
pkill verge-mihomo
# 3. 還是不通 → 停掉 root helper
sudo systemctl stop clash-verge-service
# 4. 確認路由已還原
ip route show | head
```

**心法：動 TUN 之前先想好怎麼退。** 它會改預設路由，設錯就是整台機器斷網——而你很可能正在遠端連線上做這件事。

---

## 自我測驗

1. 系統代理與 TUN 模式的攔截點各在哪一層？為什麼 `ssh` 在前者底下一定不會走代理、在後者底下卻會？並說明為什麼只有 TUN 模式需要那個以 root 執行的 service helper。
2. 核心跑著、節點延遲也測得出來、GUI 一切正常，但 `curl` 出去的 IP 沒變。要看哪一個數字才能判斷流量到底有沒有經過核心？為什麼「GUI 顯示已連線」不能當證據？
3. fake-ip 為什麼是 TUN 模式的關鍵？如果不用 fake-ip、讓系統正常解析 DNS，規則引擎會遇到什麼困難？
4. 你在 `~/.zshrc` 留著 `all_proxy=socks5://192.168.1.1:1080`，然後開了 TUN 模式，結果發現 clash 的連線數一直是 0。解釋整條路徑為什麼繞過了 clash。接著：若改成指向 clash 自己的 `socks5://127.0.0.1:7897`，流量確實進核心了，但分流結果完全不如預期——為什麼？該怎麼寫才對？
5. 情境題：你把自家路由器的 SOCKS5（`192.168.1.1:1080`）加成 clash 的一個節點並選用它，開 TUN 後整台機器立刻斷網。最可能的原因是什麼？要檢查哪一類規則？哪兩個設定本來應該防止這件事？
