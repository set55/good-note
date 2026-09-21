# `scp` 怎麼用？—— 以及 OpenSSH 9 之後那個會讓你連不上的變動

> 日期：2026-09-21
> 情境：想把檔案傳到路由器／伺服器，順便搞懂 scp 的完整用法
> 適用範圍：實測本機 OpenSSH_9.6p1（Ubuntu 24.04）與 OpenWrt 24.10.2（dropbear 2024.86）
> 相關：[SSH走代理設定.md](./SSH走代理設定.md)、
> [OpenWrt的服務管理-service與procd.md](../openwrt/OpenWrt的服務管理-service與procd.md)

**一句話結論：`scp 來源 目的地`，其中任一邊可以寫成 `使用者@主機:路徑`。但從 OpenSSH 9 開始，
`scp` **底層改用 SFTP 協定**了——對方沒有 sftp-server 就會失敗（OpenWrt 的 dropbear 正是如此），
這時要加 **`-O`** 退回舊版 SCP 協定。這是現在最常見的「scp 突然不能用了」。**

---

## 一、文法骨架

```
scp [選項] 來源... 目的地
```

**來源與目的地各自可以是本機路徑或遠端路徑**，遠端路徑的形式是：

```
[使用者@]主機:[路徑]
 └─┬──┘ └┬─┘ └┬──┘
   │     │    └ 省略＝遠端的家目錄
   │     └ 主機名或 IP（也可以是 ~/.ssh/config 裡的別名）
   └ 省略＝用本機的使用者名
```

**冒號是關鍵**——有冒號就是遠端，沒有就是本機路徑：

```bash
scp file.txt root@192.168.1.1:/tmp/      # 本機 → 遠端
scp root@192.168.1.1:/tmp/file.txt .     # 遠端 → 本機
scp a.txt b.txt root@host:/tmp/          # 多個來源 → 一個目的地（目的地必須是目錄）
scp root@h1:/f root@h2:/tmp/             # 遠端 → 遠端
```

**忘記冒號是最常見的低級錯誤**：`scp file.txt root@192.168.1.1/tmp/`
會被當成「把 `file.txt` 複製成一個叫 `root@192.168.1.1/tmp/` 的本機檔案」。

---

## 二、⚠️ OpenSSH 9 的協定變動（最重要的一節）

**OpenSSH 8.8 起 `scp` 的底層改用 SFTP 協定，9.0 起成為預設。** 原因是舊的 SCP 協定
把檔名交給遠端 shell 展開，存在安全問題（CVE-2020-15778 等）。

**後果：對方必須提供 `sftp-server`。** 而很多嵌入式系統沒有——

```bash
# 本機是 OpenSSH_9.6p1，路由器是 dropbear
$ ssh -V
OpenSSH_9.6p1 Ubuntu-3ubuntu13.19

$ opkg list-installed | grep -E 'dropbear|sftp'      # 在路由器上
dropbear - 2024.86-r1          # ← 只有 dropbear，沒有 openssh-sftp-server
```

**實測（預設 SFTP 模式）：**

```
$ scp root@192.168.1.1:/etc/openwrt_release /tmp/
ash: /usr/libexec/sftp-server: not found
scp: Connection closed                              ❌ 失敗
```

**加上 `-O` 退回舊版 SCP 協定：**

```
$ scp -O root@192.168.1.1:/etc/openwrt_release /tmp/
$ ls -l /tmp/openwrt_release
-rw-r--r-- 1 set set 226 ...                        ✅ 成功
```

**所以對 OpenWrt（dropbear）傳檔，一律加 `-O`。** 三種解法：

| 做法 | 說明 |
|---|---|
| **`scp -O ...`** | 每次加，最直接 |
| 在路由器裝 `openssh-sftp-server` | `opkg install openssh-sftp-server`，之後就不用 `-O`（要花 flash 空間） |
| **改用 `ssh` + 管線**（見第六節） | 不依賴任何額外套件 |

---

## 三、完整選項（依本機 `man scp`，OpenSSH 9.6）

**旗標型（不帶參數）**

| 選項 | 作用 |
|---|---|
| `-3` | 兩個遠端之間複製時，**資料經由本機中轉**（預設行為） |
| `-4` / `-6` | 只用 IPv4／IPv6 |
| `-A` | 允許轉發 ssh-agent 到遠端（**預設不轉發**） |
| `-B` | **批次模式**：不詢問密碼或通行碼（腳本用，失敗就直接失敗） |
| `-C` | **啟用壓縮**（傳大量文字檔有感，傳已壓縮檔反而變慢） |
| **`-O`** | **改用舊版 SCP 協定** ← 見第二節 |
| `-p` | **保留修改時間、存取時間與權限位元**（小寫 p） |
| `-q` | 安靜模式：關掉進度條與警告訊息 |
| `-R` | 兩個遠端之間複製時，**直接在來源主機上執行 scp**（需要來源能免密碼連到目的地） |
| `-r` | **遞迴複製整個目錄**（會跟隨符號連結） |
| `-s` | 使用 SFTP 協定（9.x 的預設，寫出來只是為了明確） |
| `-T` | **關閉嚴格檔名檢查**（預設會檢查收到的檔名是否符合你要求的，防止遠端塞額外檔案） |
| `-v` | 囉嗦模式（**連不上時第一個該加的**，`-vvv` 更詳細） |

**帶參數型**

| 選項 | 作用 |
|---|---|
| `-c <cipher>` | 指定加密演算法（直接傳給 ssh） |
| `-D <sftp_server路徑>` | 直接連到**本機**的 SFTP server 程式（除錯用） |
| `-F <ssh_config>` | 指定替代的 ssh 設定檔 |
| `-i <金鑰檔>` | **指定私鑰** |
| `-J <目的地>` | **經由跳板機**（等同 `ProxyJump`，可用逗號串多層） |
| `-l <限速>` | **限制頻寬，單位 Kbit/s** |
| `-o <ssh 選項>` | **傳任意 ssh_config 格式的選項**（沒有專屬旗標的都靠它） |
| `-P <埠>` | **指定埠**（大寫 P，因為小寫 p 已被「保留屬性」佔用） |
| `-S <程式>` | 指定用哪個程式建立加密連線 |
| `-X <sftp選項>` | 調整 SFTP 行為：`nrequests=<值>`（並行請求數，預設 64）、`buffer=<值>`（緩衝區大小） |

> **`-P` 與 `-p` 大小寫意義完全不同**，而且和 `ssh` 的 `-p`（小寫是埠）**相反**——
> 這是 scp 最容易打錯的地方。

---

## 四、常用寫法

```bash
# 基本
scp file.txt root@192.168.1.1:/tmp/              # 上傳
scp root@192.168.1.1:/tmp/file.txt ./            # 下載
scp -O root@192.168.1.1:/etc/config/network ./   # 對 OpenWrt 記得加 -O

# 目錄
scp -r ./mydir root@host:/opt/                   # 遞迴上傳整個目錄
scp -rp ./mydir root@host:/opt/                  # 連時間戳與權限一起保留

# 非標準埠 / 指定金鑰 / 跳板
scp -P 2222 file.txt root@host:/tmp/
scp -i ~/.ssh/id_ed25519 file.txt root@host:/tmp/
scp -J jump@gateway file.txt root@internal:/tmp/

# 限速與壓縮（傳大檔時不要塞爆線路）
scp -l 8000 -C bigfile.tar root@host:/tmp/       # 限 8000 Kbit/s ≈ 1 MB/s

# 腳本裡用
scp -B -q -O file.txt root@host:/tmp/            # 不問密碼、不印進度

# 連不上時
scp -v file.txt root@host:/tmp/                  # 看它卡在哪一步
```

**路徑含空白時要跳脫兩次**——一次給本機 shell，一次給遠端 shell：

```bash
scp 'root@host:/tmp/my\ file.txt' ./             # 遠端路徑的空白要用 \ 跳脫
```

---

## 五、`~/.ssh/config` 讓指令變短

scp 會讀 ssh 的設定檔，所以定義好別名之後：

```
# ~/.ssh/config
Host router
    HostName 192.168.1.1
    User root
```

```bash
scp -O file.txt router:/tmp/        # 不用再打 IP 與使用者
```

（代理與跳板的設定見 [SSH走代理設定.md](./SSH走代理設定.md)。）

---

## 六、什麼時候不該用 scp

OpenSSH 官方在 man page 裡直接寫了：scp 的協定「已過時、不夠彈性、不易修正」，
建議改用 `sftp` 或 `rsync`。

| 需求 | 建議工具 |
|---|---|
| 單一檔案、一次性 | **scp** 就好，最短 |
| 大量檔案、要續傳／增量同步 | **`rsync -avz --partial`** |
| 互動式瀏覽遠端目錄 | **`sftp`** |
| **遠端沒有 sftp-server 也沒有 rsync**（嵌入式） | **`ssh` + 管線**（見下） |

**`ssh` + 管線的萬用解法**——只要對方有 shell 就能用，不依賴任何額外套件：

```bash
# 上傳
cat local.txt | ssh root@192.168.1.1 'cat > /tmp/remote.txt'

# 下載
ssh root@192.168.1.1 'cat /etc/config/network' > ./network

# 整個目錄（用 tar 打包串流）
tar czf - ./mydir | ssh root@host 'tar xzf - -C /opt/'
ssh root@host 'tar czf - -C /etc config' | tar xzf -
```

**這招對 OpenWrt 特別實用**——dropbear 沒有 sftp-server，但一定有 shell 與 tar。

---

## 七、常見錯誤對照

| 症狀 | 原因 |
|---|---|
| `sftp-server: not found` / `Connection closed` | **遠端沒有 SFTP server** → 加 `-O` |
| 檔案被複製成本機的怪檔名 | **忘記打冒號** |
| `Permission denied` | 遠端路徑沒有寫入權限，或金鑰／使用者不對 |
| 用了 `-p` 卻連不上非標準埠 | **埠要用大寫 `-P`** |
| 目錄沒複製過去 | 忘記 `-r` |
| 腳本卡住不動 | 它在等密碼輸入 → 加 `-B`，並確認金鑰認證可用 |
| 傳完時間戳全變成現在 | 沒加 `-p` |

---

## 自我測驗

1. `scp file.txt root@192.168.1.1/tmp/`（少了冒號）會發生什麼事？為什麼不會報錯？
2. OpenSSH 9 之後 `scp` 的底層協定改成什麼？為什麼要改？對 OpenWrt 這類系統造成什麼後果、
   怎麼解決（說出三種）？
3. `-p` 和 `-P` 分別是什麼？為什麼 scp 的埠號旗標和 `ssh` 不一樣？
4. 你要從遠端下載一個目錄，但遠端既沒有 sftp-server 也沒有 rsync，只有 shell 和 tar。
   要怎麼做？
5. 情境題：你寫了一個備份腳本，每晚用 scp 把檔案傳到伺服器。它偶爾會卡住不動，
   而且傳完的檔案時間戳都變成當下。請說出該加哪三個選項、各自解決什麼問題。
