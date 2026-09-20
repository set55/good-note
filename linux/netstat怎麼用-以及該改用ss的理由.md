# netstat 怎麼用？—— 讀懂每一欄，以及該改用 ss 的理由

> 日期：2026-09-20
> 情境：想完整搞懂 netstat 的用法，而不只是背 `netstat -tulnp`
> 適用範圍：實測 Ubuntu 24.04（net-tools 2.10、iproute2 的 ss）與 OpenWrt 24.10（BusyBox 1.36 的 netstat）
> 相關：[查詢轉發表與路由表.md](./查詢轉發表與路由表.md)、
> [clash-verge怎麼用-流量要先進得去核心.md](./clash-verge怎麼用-流量要先進得去核心.md)、
> [排查方法論.md](./排查方法論.md)

**一句話結論：netstat 回答的永遠是同一個問題——「這台機器上有哪些 socket、各自是什麼狀態」。
把 `Local Address`（綁在哪，決定誰連得到）、`State`（連線走到哪一步）、`Recv-Q/Send-Q`
（有沒有積壓）這三欄讀懂，八成的網路排查就有方向了。但它屬於已停止維護的 net-tools，
新系統該用 `ss`——只是 netstat 在 BusyBox、舊系統、容器裡到處都有，所以仍要看得懂。**

---

## 目錄

- [一、先講定位：netstat 已經是舊工具](#一先講定位netstat-已經是舊工具)
- [二、四個最常用的任務](#二四個最常用的任務)
- [三、讀懂輸出的每一欄](#三讀懂輸出的每一欄)
- [四、`Local Address` 決定誰連得到](#四local-address-決定誰連得到)
- [五、TCP 狀態：哪些正常、哪些是警訊](#五tcp-狀態哪些正常哪些是警訊)
- [六、netstat → ss 對照](#六netstat--ss-對照)
- [七、實戰配方](#七實戰配方)
- [八、五個陷阱](#八五個陷阱)
- [自我測驗](#自我測驗)

---

## 一、先講定位：netstat 已經是舊工具

netstat 屬於 **net-tools** 這套工具集（還包括 `ifconfig`、`route`、`arp`），**已經停止維護**。
取代它的是 iproute2 的 `ss`（socket statistics）與 `ip`。

```bash
# 本機實測
$ command -v netstat && dpkg -l net-tools | tail -1
/usr/bin/netstat
ii  net-tools  2.10-0.1ubuntu4.4          # ← 有裝，但 Ubuntu 24.04 預設不裝

$ command -v ss
/usr/bin/ss                               # ← 預設就有

# 路由器上
$ ls -l $(which netstat)
/bin/netstat -> busybox                   # ← BusyBox 的精簡版，旗標比較少
```

> 順帶一個小觀察：netstat 在 `/usr/bin` 而不是 `/usr/sbin`——因為它只是**讀取**資訊，
> 一般使用者執行有意義（見 [bin與sbin的差別](./bin與sbin的差別-以及怎麼正確找到執行檔.md)）。

**為什麼 ss 更快**：netstat 是逐行讀 `/proc/net/tcp` 這類文字檔再解析，socket 一多就慢；
ss 走 netlink 的 `sock_diag` 介面，直接跟核心要結構化資料，而且能做核心端過濾。

**但還是要會 netstat**：BusyBox、Alpine 容器、老舊伺服器、別人寫好的腳本裡到處都是它。

---

## 二、四個最常用的任務

```bash
netstat -tulnp        # 1. 有哪些服務在監聽（最常用）
netstat -tnp          # 2. 目前有哪些連線
netstat -rn           # 3. 路由表（等同 route -n、ip route）
netstat -s            # 4. 協定層的累計統計
```

旗標逐一拆解：

| 旗標 | 意思 | 備註 |
|---|---|---|
| `-t` | TCP | |
| `-u` | UDP | |
| `-x` | Unix domain socket | 本機行程間通訊，不走網路 |
| `-l` | 只看 **LISTEN** 狀態 | 不加就只顯示非監聽的連線 |
| `-a` | **全部**（監聽 + 已連線） | |
| `-n` | **數字形式**，不做名稱解析 | **一定要加**，見下 |
| `-p` | 顯示 PID 與程式名 | **要 root 才看得到別人的** |
| `-c` | 持續每秒更新 | |
| `-r` | 路由表 | |
| `-i` | 網卡統計 | |
| `-s` | 協定統計 | |

**`-n` 為什麼一定要加**：不加 `-n` 時 netstat 會做**兩種完全不同的名稱查詢**，而代價天差地遠：

| 查什麼 | 查哪裡 | 慢不慢 |
|---|---|---|
| 埠號 → 服務名（`:80` → `http`） | `/etc/services`，**純本機檔案** | 很快 |
| IP → 主機名（`192.168.1.1` → `OpenWrt.lan`） | NSS → `/etc/hosts` → **DNS 反查** | **可能卡到逾時** |

本機實測，同一批連線：

```
netstat -tan （有 -n）：     15 ms
netstat -ta  （沒 -n）：  30082 ms      ← 兩千倍
ss -tan      （對照）：       7 ms
```

那 30 秒全花在一個外部 IP 的反查上——`resolvectl query 208.103.161.2` 也一樣等到逾時，
因為它根本沒有 PTR 記錄。**「netstat 卡住不動」就是這樣來的。**

### 埠號沒被換成名字？因為 `/etc/services` 查不到它

把 `-n` 拿掉卻覺得「沒變」，多半是因為你看的埠不在那份清單裡。本機 `/etc/services`
（由 `netbase` 套件提供）只有 **318 個服務名**，而 IANA 正式清單有數千筆：

```
53   → domain        2049 → nfs          6379 → redis
80   → http          111  → sunrpc       5432 → postgresql
8080 → http-alt

2080 / 7897 / 39685 / 52221 / 60339  → 查無，原樣保留數字
3000 / 6443 / 9090 / 27017           → 也查無
```

所以像 clash 的 `7897` 這種埠，不管加不加 `-n` 都是數字；但同一行的 IP 仍然會被反查成
`localhost`——**「沒變」往往只是剛好看了不會變的那一欄。**

> 另一個容易誤會的例子：`127.0.0.54` 會顯示成 `_localdnsproxy`。它不在 `/etc/hosts` 裡，
> 是 systemd-resolved 替 `127.0.0.53`／`127.0.0.54` **合成的 PTR 記錄**
> （實測 `resolvectl query 127.0.0.54` 1.3 ms 就回答了）。

記法：**`tulnp` = TCP、UDP、Listening、Numeric、Process**。

---

## 三、讀懂輸出的每一欄

本機實測：

```
Proto Recv-Q Send-Q Local Address       Foreign Address   State    PID/Program name
tcp        0      0 127.0.0.54:53       0.0.0.0:*         LISTEN   -
tcp        0      0 0.0.0.0:80          0.0.0.0:*         LISTEN   -
tcp        0      0 127.0.0.1:7897      0.0.0.0:*         LISTEN   -
tcp        0      0 127.0.0.1:39685     0.0.0.0:*         LISTEN   10058/Code --standa
```

| 欄位 | 意義 |
|---|---|
| `Proto` | tcp／tcp6／udp／unix |
| `Recv-Q` | **接收佇列**：已經到了本機但程式還沒讀走的資料量 |
| `Send-Q` | **傳送佇列**：已送出但對方還沒確認（ACK）的資料量 |
| `Local Address` | 本機綁定的「位址:埠」 |
| `Foreign Address` | 對端；LISTEN 時是 `0.0.0.0:*`（還沒有對象） |
| `State` | TCP 狀態；UDP 沒有狀態所以是空的 |
| `PID/Program name` | 哪個行程持有這個 socket |

### `Recv-Q`／`Send-Q` 在兩種狀態下意義完全不同

這是最有價值、也最少人知道的一點：

**已連線（ESTABLISHED）時**——就是字面意思的佇列：

- `Recv-Q` 持續不為 0 → **程式讀不贏**（卡住、處理太慢、忘了 read）。
- `Send-Q` 持續不為 0 → **對方收不贏或網路塞住**（封包送出去沒被 ACK）。

**監聽（LISTEN）時——意義換成了連線佇列**：

```
$ ss -ltn
State  Recv-Q Send-Q Local Address:Port
LISTEN 0      4096      127.0.0.54:53      # Send-Q = backlog 上限
LISTEN 0      511          0.0.0.0:2080
LISTEN 0      64           0.0.0.0:2049     # NFS 的 backlog 只有 64
```

- `Recv-Q` = **已完成三向交握、等著程式 `accept()` 的連線數**。
- `Send-Q` = **backlog 上限**（`listen()` 的第二個參數，受 `net.core.somaxconn` 限制）。

**所以「LISTEN 的 Recv-Q 一直逼近 Send-Q」就是明確的警訊**：連線進得來，但程式 accept 不贏——
新連線會被丟棄或重傳。這是排查「服務偶爾連不上」最直接的證據，比看 CPU 有用得多。

---

## 四、`Local Address` 決定誰連得到

同樣在監聽 80 埠，綁定位址不同，結果天差地遠：

| Local Address | 誰連得到 |
|---|---|
| `0.0.0.0:80` | **所有網卡**，包含區網與外網 |
| `127.0.0.1:80` | **只有本機**，別台機器完全連不到 |
| `192.168.1.171:80` | 只有從那張網卡進來的 |
| `:::80`（IPv6 的 `::`） | 所有 IPv6 位址；Linux 預設也會一併收 IPv4 |

本機實測的兩行正好是對照組：

```
0.0.0.0:80        ← 對外服務，區網任何裝置都連得到
127.0.0.1:7897    ← clash 的 mixed-port，只綁 localhost
```

**這正是 clash 設定裡 `allow-lan: false` 的實際效果**：開關的作用就是決定它綁 `127.0.0.1`
還是 `0.0.0.0`。所以「同事說連不到你的 clash」不是防火牆問題，是**它根本沒有綁在對外的位址上**
——看 `Local Address` 一眼就能排除一半的可能。

**心法：服務連不上時，先看它綁在哪、再查防火牆。** 綁在 `127.0.0.1` 的服務，防火牆開再多都沒用。

---

## 五、TCP 狀態：哪些正常、哪些是警訊

本機當下的分布：

```bash
$ netstat -tn | awk 'NR>2 {print $6}' | sort | uniq -c | sort -rn
     38 ESTABLISHED
      4 TIME_WAIT
      4 SYN_SENT
```

| 狀態 | 意義 | 大量出現代表什麼 |
|---|---|---|
| `LISTEN` | 等著被連 | 正常 |
| `ESTABLISHED` | 連線中 | 正常 |
| `SYN_SENT` | 送出 SYN 在等回應 | **對方沒回**——防火牆擋掉、埠沒開、或路由不通 |
| `SYN_RECV` | 收到 SYN 在等最後一步 | 大量堆積可能是 SYN flood |
| `TIME_WAIT` | **主動關閉方**關完後的等待期（2×MSL，約 60 秒） | **正常現象**，高併發短連線必然一堆 |
| `CLOSE_WAIT` | **對方已關閉，等我方 `close()`** | **你的程式有 bug**——沒關 socket |
| `FIN_WAIT_2` | 我方關了，等對方關 | 大量堆積代表對方不關 |

**`TIME_WAIT` 與 `CLOSE_WAIT` 是最常被搞混的一對，但責任歸屬完全相反：**

- `TIME_WAIT` 出現在**主動關閉的那一方**，是 TCP 協定要求的（確保最後的 ACK 送達、
  避免舊封包污染新連線）。多不是病，真的困擾時該調的是連線複用（keep-alive），
  不是去調核心參數硬清。
- `CLOSE_WAIT` 出現在**被動關閉的那一方**，代表對方已經說「我說完了」，而**你的程式沒有呼叫
  `close()`**。數量只增不減 = 檔案描述符洩漏，最後會撞上 `too many open files`。
  **這是程式的錯，不是網路的錯。**

**排查口訣：`TIME_WAIT` 多 → 看架構（連線沒複用）；`CLOSE_WAIT` 多 → 看程式碼（socket 沒關）。**

---

## 六、netstat → ss 對照

| netstat | ss | 說明 |
|---|---|---|
| `netstat -tulnp` | `ss -tulnp` | 旗標幾乎一樣 |
| `netstat -tnp` | `ss -tnp` | |
| `netstat -an` | `ss -an` | |
| `netstat -rn` | `ip route` | ss 不管路由 |
| `netstat -i` | `ip -s link` | |
| `netstat -s` | `ss -s`（摘要）／`nstat` | |
| `netstat -tn \| grep :443` | `ss -tn 'dport = :443'` | **ss 能在核心端過濾，快很多** |

ss 獨有、netstat 做不到的：

```bash
ss -tn state established '( dport = :443 or sport = :443 )'   # 條件式過濾
ss -tni                       # 顯示 RTT、cwnd、重傳次數等 TCP 內部狀態
ss -tp state close-wait       # 直接抓出有問題的連線與它的程式
ss -K dst 1.2.3.4             # 強制關閉符合條件的連線（需 root）
```

`ss -tni` 那個特別值得知道——它能看到每條連線的 RTT（Round-Trip Time，往返時間）與重傳次數，
是判斷「慢是網路還是程式」的直接證據。

---

## 七、實戰配方

```bash
# 80 埠被誰佔了（服務起不來時的第一個問題）
sudo netstat -tulnp | grep ':80 '
sudo ss -tulnp 'sport = :80'
sudo lsof -i :80                       # 第三種查法，資訊最全

# 某個程式開了哪些連線
sudo netstat -tnp | grep nginx
sudo ss -tnp | grep nginx

# 連線狀態統計（排查的起手式）
netstat -tn | awk 'NR>2 {print $6}' | sort | uniq -c | sort -rn

# 誰連我最多（找異常來源）
netstat -tn | awk 'NR>2 {split($5,a,":"); print a[1]}' | sort | uniq -c | sort -rn | head

# 找出 CLOSE_WAIT 堆積的兇手
sudo ss -tp state close-wait | head

# 確認服務綁在哪（連不上時先看這個）
sudo ss -ltn | grep 8080
```

---

## 八、五個陷阱

1. **不加 `-n` 會卡住**：DNS 反查逾時讓你以為程式壞了。永遠加 `-n`。
2. **`-p` 沒 root 只看得到自己的**：實測非 root 執行時，別人的行程全部顯示 `-`。
   看到一整排 `-` 不是沒程式，是你權限不夠。
3. **BusyBox 版的旗標不一樣**：路由器上 `netstat --help` 只列出 `[-ral] [-tuwx] [-enWp]`，
   沒有 GNU 版的長選項。腳本要跨平台就只用最基本的旗標。
4. **UDP 沒有「連線」**：UDP 那幾行的 State 是空的，看到 UDP 不要找 ESTABLISHED。
5. **容器裡看到的是容器的 netns**：在容器內跑 netstat，看到的是容器自己的網路命名空間
   （network namespace），不是主機的。要看主機的得在主機上跑，或 `nsenter` 進去。

---

## 自我測驗

1. `Recv-Q` 與 `Send-Q` 這兩欄，在 `ESTABLISHED` 與 `LISTEN` 兩種狀態下的意義完全不同。
   分別是什麼？「LISTEN 的 Recv-Q 一直很大」代表系統出了什麼問題？
2. 大量 `TIME_WAIT` 和大量 `CLOSE_WAIT`，哪一個是程式的 bug？為什麼？兩者分別出現在連線的哪一方？
3. 有人回報「連不到你機器上的服務」。你用 netstat 看到它在 LISTEN。接下來要看哪一欄、為什麼
   它比查防火牆更該先看？
4. 為什麼 `ss` 比 `netstat` 快？從兩者取得資料的方式說明，並說出一個 ss 做得到而 netstat 做不到的事。
5. 情境題：某服務平常正常，但流量一大就偶爾連不上，CPU 與記憶體都不高。你要看哪個數字來驗證
   「不是機器不夠力，而是程式 accept 不贏」？看到什麼樣的數值可以確認？
