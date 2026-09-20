# 什麼是 backlog？—— TCP 交握背後的兩個佇列

> 日期：2026-09-20
> 情境：看 `ss -ltn` 時不懂為什麼 LISTEN 狀態下的 `Recv-Q`／`Send-Q` 意義會變成別的東西
> 適用範圍：Linux（實測 Ubuntu 24.04、kernel 7.0）
> 延伸自：[netstat怎麼用-以及該改用ss的理由.md](./netstat怎麼用-以及該改用ss的理由.md)

**一句話結論：TCP 連線是**核心**建立的，不是你的程式建立的——程式呼叫 `accept()` 只是「從核心
已經建好的連線堆裡領一個走」。既然兩邊速度不一樣，中間就需要佇列暫存，`backlog` 就是這個佇列
的上限。所以在 LISTEN 狀態下，`Recv-Q` = 現在有幾條連線在排隊等你領，`Send-Q` = 這個佇列最多
能排幾條。**

---

## 一、先看清楚誰在做事

一個 TCP 伺服器的程式碼骨架：

```c
fd = socket(...);
bind(fd, ...);          // 綁定位址與埠
listen(fd, 511);        // ← 這個 511 就是 backlog
while (1) {
    conn = accept(fd);  // 領一條已經建好的連線
    handle(conn);       // 處理它
}
```

**關鍵：三向交握（three-way handshake）從頭到尾是核心自己完成的。** 你的程式就算正在忙、
還沒呼叫 `accept()`，核心照樣會回應 SYN、完成交握、把連線準備好。

於是產生一個落差：**核心「生產」連線的速度，和程式「消費」連線的速度不一樣。**
中間必須有個緩衝區——那就是 backlog。

**這正是 `Recv-Q`／`Send-Q` 在 LISTEN 狀態下意義會變的原因**：LISTEN 的 socket 上根本不會
傳輸資料（資料走的是 `accept()` 之後產生的新 socket），那兩個欄位就被拿去表示連線佇列了。

---

## 二、其實是兩個佇列，不是一個

三向交握有兩個階段，對應兩個不同的佇列：

```
客戶端                            伺服器核心

  SYN ────────────────────────►  收到 → 建立半連線
                                 放進 【SYN queue】（半連線佇列）
  ◄──────────────── SYN+ACK      回應

  ACK ────────────────────────►  收到 → 交握完成！
                                 從 SYN queue 移到 【Accept queue】（全連線佇列）
                                        │
                                        │  程式呼叫 accept()
                                        ▼
                                   領走一條，佇列少一個
```

| | SYN queue（半連線） | Accept queue（全連線） |
|---|---|---|
| 裡面裝什麼 | 收到 SYN、還在等對方最後那個 ACK 的連線 | **交握已完成**、等程式 `accept()` 的連線 |
| 對應的 TCP 狀態 | `SYN_RECV` | `ESTABLISHED`（但程式還沒拿到） |
| 上限由誰決定 | `net.ipv4.tcp_max_syn_backlog` | **`min(listen() 的 backlog, net.core.somaxconn)`** |
| 滿了的典型成因 | SYN flood 攻擊、大量半開連線 | **程式 `accept()` 太慢** |

**`ss -ltn` 顯示的是第二個佇列**：

```
State  Recv-Q Send-Q Local Address:Port
LISTEN   0      511     0.0.0.0:80
         │      └── Accept queue 的上限（backlog）
         └───────── Accept queue 目前排了幾條
```

---

## 三、本機實測：那些數字從哪來

```bash
$ sysctl -n net.core.somaxconn            # 4096   ← 系統層的天花板
$ sysctl -n net.ipv4.tcp_max_syn_backlog  # 1024   ← SYN queue 上限
$ sysctl -n net.ipv4.tcp_syncookies       # 1      ← SYN queue 滿了的備案
$ sysctl -n net.ipv4.tcp_abort_on_overflow# 0      ← Accept queue 滿了的行為
```

各服務的 backlog（`ss -ltn` 的 Send-Q）：

| 埠 | backlog | 為什麼是這個數字 |
|---|---|---|
| `:80`、`:2080` | **511** | nginx 系的預設值（`listen 80 backlog=511`） |
| `:5432` | **200** | PostgreSQL 的預設 |
| `:2049` | **64** | NFS 的保守預設 |
| `:6060` | 1024 | 程式自己指定 |
| 其餘多數 | **4096** | 程式傳了很大的值或 `SOMAXCONN`，被 `somaxconn` 砍到 4096 |

**最後一列是重點：`listen()` 傳進去的 backlog 會被 `net.core.somaxconn` 封頂。**
程式寫 `listen(fd, 65535)` 也沒用，實際上限還是 4096。這是「我明明調大了 backlog，怎麼沒變」
的標準答案——**兩個地方都要調**。

---

## 四、佇列滿了會發生什麼

### Accept queue 滿（程式 accept 不贏）

預設 `tcp_abort_on_overflow = 0` 的行為是：**默默丟棄對方那個 ACK**。

後果很違反直覺：

```
客戶端：送出 ACK → 我方認定「連線建立成功」→ 開始送資料
伺服器：ACK 被丟掉 → 這條連線不存在 → 對送來的資料不理會
客戶端：沒收到回應 → 重傳 → 等待 → 重傳（指數退避）
```

**所以使用者的體感不是「連線被拒絕」，而是「偶爾卡個幾秒才通，或者直接逾時」**——
這就是為什麼這種問題特別難查：沒有錯誤訊息、CPU 和記憶體都正常。

把 `tcp_abort_on_overflow` 設成 1 會改成直接回 RST（連線被重置），客戶端立刻收到明確錯誤。
**除錯時很有用**（問題立刻現形），但正式環境通常不開——重傳至少還有機會成功，RST 是直接失敗。

### SYN queue 滿

丟棄新的 SYN。但因為本機 `tcp_syncookies = 1`，核心會改用 **SYN cookie**：不佔佇列空間，
把必要資訊編碼進序號裡，等對方 ACK 回來時再還原。這是對抗 SYN flood 的標準防禦。

### 怎麼確認到底有沒有發生

**這些是累計計數器，要看的是「有沒有在增加」**：

```bash
$ nstat -az | grep -iE 'ListenOverflows|ListenDrops|SyncookiesSent'
TcpExtSyncookiesSent     0        # SYN queue 溢位次數
TcpExtListenOverflows    0        # ← Accept queue 溢位次數（最關鍵）
TcpExtListenDrops        0        # 各種原因丟棄的連線
```

本機三個都是 0，代表從開機到現在沒發生過。

```bash
# 觀察是否持續增加（排查時這樣跑）
watch -d -n1 "nstat -az | grep -E 'ListenOverflows|ListenDrops'"

# 同時看即時佇列長度
watch -d -n1 "ss -ltn | grep ':80 '"
```

**判斷法：`ListenOverflows` 持續增加 + `ss -ltn` 的 Recv-Q 經常逼近 Send-Q → 確定是
accept 不贏，不是機器不夠力。**

---

## 五、要調的話，調哪裡

**先搞清楚瓶頸在哪**——backlog 只是緩衝，**調大它並不會讓程式處理得更快**，只是讓尖峰時
少丟幾條連線。如果程式持續消費不完，佇列調到多大都會滿。

真要調，三個地方缺一不可：

```bash
# 1. 系統天花板
sysctl -w net.core.somaxconn=4096
# 永久生效：寫進 /etc/sysctl.d/99-xxx.conf

# 2. SYN queue（防 SYN flood 才需要動）
sysctl -w net.ipv4.tcp_max_syn_backlog=4096

# 3. 應用程式自己的 listen() 參數——這個最常被忘記
#    nginx:      listen 80 backlog=1024;
#    程式碼:      listen(fd, 1024)
#    systemd socket: Backlog=1024
```

**順序：先確認 `somaxconn` 夠大，再改應用程式的設定，然後用 `ss -ltn` 驗證 Send-Q 真的變了。**
沒驗證就等於沒改——這正是 [排查方法論.md](./排查方法論.md) 說的「不要相信自己以為的狀態」。

---

## 六、常見誤解

| 誤解 | 實情 |
|---|---|
| backlog 是「最大同時連線數」 | 不是。它只管「等著被 accept 的連線」。已經 accept 的連線不佔這個額度 |
| 調大 backlog 能提升效能 | 不能。它只是緩衝，處理速度取決於程式本身 |
| 佇列滿了客戶端會收到錯誤 | 預設不會。是默默丟棄 ACK，客戶端只感覺到「慢」 |
| `listen(fd, 65535)` 就有 65535 | 會被 `somaxconn` 砍到 4096 |
| LISTEN 的 Recv-Q 是收到的資料量 | 不是。LISTEN socket 不傳資料，那欄是排隊的連線數 |

---

## 自我測驗

1. 為什麼 LISTEN 狀態的 socket 上，`Recv-Q`／`Send-Q` 會被拿去表示連線佇列，而不是資料佇列？
2. SYN queue 與 Accept queue 分別裝什麼？一條連線是在哪一個動作之後從前者移到後者的？
3. 你把程式的 `listen()` backlog 改成 65535，但 `ss -ltn` 顯示的 Send-Q 還是 4096。為什麼？
4. Accept queue 滿了的預設行為是「丟棄對方的 ACK」。為什麼這會讓使用者的體感是「偶爾很慢」
   而不是「連線被拒絕」？把 `tcp_abort_on_overflow` 設成 1 之後現象會怎麼變、為什麼正式環境通常不開？
5. 情境題：某服務尖峰時偶爾連不上，CPU、記憶體、頻寬都不高。請說出你要看的兩個數字
   （一個是當下狀態、一個是累計計數器），以及看到什麼樣的組合可以確定是 accept queue 溢位。
   確定之後，把 backlog 調大是不是正確的解法？
