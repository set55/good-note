# socket 是什麼？—— 從 Linux 核心看：它是「端點」，不是「連線」

> 日期：2026-09-22
> 情境：讀 TPROXY 時一直看到「交給 12345 那個 socket」「找已建立的連線 socket」，
> 原本對 socket 的理解只有「好像就是連線」，想從核心的角度搞清楚
> 適用範圍：Linux 通用；實測本機 kernel 7.0、Python 3、iproute2 的 `ss`
> 相關：[fd是什麼-行程的檔案描述符表.md](./fd是什麼-行程的檔案描述符表.md)、
> [什麼是backlog-TCP交握背後的兩個佇列.md](./什麼是backlog-TCP交握背後的兩個佇列.md)、
> [netstat怎麼用-以及該改用ss的理由.md](./netstat怎麼用-以及該改用ss的理由.md)、
> [TPROXY與REDIRECT的差別與原理.md](./TPROXY與REDIRECT的差別與原理.md)

**一句話結論：socket 是核心裡代表「一個通訊端點（endpoint）」的物件，裡面放著這一端的位址與埠、
狀態、收發緩衝區；行程手上拿到的只是指向它的 fd。一條 TCP 連線是**兩個** socket（兩端各一個）
加上把它們配對起來的四元組；而伺服器那個「監聽中」的 socket 根本不屬於任何連線——每接受一條連線，
核心就另外生一個新的 socket。封包進來時，核心的工作就是「照封包標頭找出該交給哪個 socket」，
TPROXY 做的正是**跳過這一步、事先指定好 socket**。**

---

## 目錄

- [一、socket 不是連線：先看實物](#一socket-不是連線先看實物)
- [二、核心裡的四層結構](#二核心裡的四層結構)
- [三、TCP：監聽 socket 與連線 socket 是兩種東西](#三tcp監聽-socket-與連線-socket-是兩種東西)
- [四、UDP：一個 socket 對所有人](#四udp一個-socket-對所有人)
- [五、封包進來時，核心怎麼找到 socket](#五封包進來時核心怎麼找到-socket)
- [六、回到 TPROXY：每一句話現在都說得通](#六回到-tproxy每一句話現在都說得通)
- [七、自己觀察的方法](#七自己觀察的方法)
- [自我測驗](#自我測驗)

---

## 一、socket 不是連線：先看實物

用一支 Python 小程式在本機做實驗：一個 TCP 伺服器聽在 `127.0.0.1:18080`，同一個行程裡再開兩個
客戶端連過去、伺服器 `accept()` 兩次；另外開一個 UDP socket 聽在 `18081`。

```
listen fd 3   accepted fds [6, 7]   client fds [4, 5]   udp fd 8
```

`ss -tanp` 看到的 TCP 部分：

```
State  Local Address:Port  Peer Address:Port  Process
LISTEN     127.0.0.1:18080      0.0.0.0:*     fd=3   ← 監聽 socket：沒有對端
ESTAB      127.0.0.1:18080    127.0.0.1:49722 fd=6   ← 伺服器端，第 1 條連線
ESTAB      127.0.0.1:49722    127.0.0.1:18080 fd=4   ← 客戶端，第 1 條連線
ESTAB      127.0.0.1:49734    127.0.0.1:18080 fd=5   ← 客戶端，第 2 條連線
ESTAB      127.0.0.1:18080    127.0.0.1:49734 fd=7   ← 伺服器端，第 2 條連線
```

**2 條連線，卻有 5 個 TCP socket。** 數字是這樣來的：

```
第 1 條連線 = fd 4（客戶端那頭）＋ fd 6（伺服器那頭），用四元組 127.0.0.1:49722 ↔ 127.0.0.1:18080 配對
第 2 條連線 = fd 5 ＋ fd 7，四元組 127.0.0.1:49734 ↔ 127.0.0.1:18080
監聽 socket = fd 3，不屬於任何一條連線
```

所以「socket 就是連線」不對。比較準確的說法是：

| 詞 | 是什麼 |
|---|---|
| **socket** | 核心裡的一個物件，代表**一個端點**：「我這一頭」的位址、埠、狀態、緩衝區 |
| **連線（connection）** | 兩個端點之間的一段關係，由**四元組**（本地位址、本地埠、對端位址、對端埠）識別；加上協定就是**五元組** |

真實網路中兩端通常在不同機器上，所以每台機器上只看得到一個 socket；在同一台機器上實驗，才會兩個都看到。

---

## 二、核心裡的四層結構

`socket()` 系統呼叫回傳的只是一個整數。從那個整數往下，核心裡是這樣串起來的：

```
行程的 fd 表                     （fd 是什麼見 fd是什麼-行程的檔案描述符表.md）
  fd 6 ──►  struct file          ← 「一切皆檔案」的那一層：所以能 read()/write()/close()
              │
              ▼
           struct socket         ← 通用的 socket 層（BSD socket 層）：型別（STREAM/DGRAM）、
              │                     一張「這種 socket 的操作函式表」（bind/listen/accept/sendmsg…）
              ▼
           struct sock           ← 協定層的本體，真正的狀態都在這裡：
              │                     本地位址／埠、對端位址／埠、狀態（LISTEN、ESTABLISHED…）、
              │                     接收佇列、傳送佇列、各種選項（IP_TRANSPARENT、SO_MARK…）
              ▼
           struct tcp_sock       ← TCP 專屬的延伸：序號、視窗、重傳計時器、壅塞控制…
          （UDP 則是 udp_sock）
```

`/proc/<pid>/fd` 裡看到的 `socket:[159253]`，那個數字是這個 socket 的 inode 編號；
`ss -e` 印出的 `ino:159253` 就是同一個，可以用來把「哪個 fd」和「哪一條 socket」對起來
（本機實測兩邊完全對得上）。

**心智模型：fd 是你手上的號碼牌，`struct sock` 才是櫃檯後面那個真正存放狀態的抽屜。**
關掉 fd（`close()`）只是還號碼牌；核心會在沒有人再引用時才收掉抽屜——TCP 甚至會為了把連線
收尾（FIN、TIME_WAIT）讓抽屜在程式結束後多留一陣子。

---

## 三、TCP：監聽 socket 與連線 socket 是兩種東西

伺服器端從無到有的過程，每個系統呼叫各做一件事：

| 系統呼叫 | 對 socket 做了什麼 | 之後的狀態 |
|---|---|---|
| `socket(AF_INET, SOCK_STREAM, 0)` | 在核心建立一個空的 socket 物件，回傳 fd | 什麼都沒有：沒位址、沒埠 |
| `bind(fd, 0.0.0.0:12345)` | 填入「我這一頭」的本地位址與埠 | 有位址了，但還不收連線 |
| `listen(fd, backlog)` | 把它變成**監聽 socket**，並配上兩個佇列（SYN queue、accept queue，見 [backlog 那篇](./什麼是backlog-TCP交握背後的兩個佇列.md)） | `LISTEN` |
| （客戶端送來 SYN，核心完成三向交握） | **核心另外建立一個新的 socket**，本地端與對端都填好，放進 accept queue | 新 socket 是 `ESTABLISHED` |
| `accept(fd)` | 從 accept queue 取出那個新 socket，給它一個**新的 fd** 回傳 | 程式拿到連線 socket |

客戶端那邊短得多：`socket()` → `connect(對方位址)`。`connect()` 會自動挑一個本地埠（就是上面的
`49722`），送出 SYN，交握完成後這個 socket 就是 `ESTABLISHED`。

**兩個關鍵事實：**

1. **監聽 socket 永遠不傳資料**，它只負責「接新客人」。所有資料都走 `accept()` 生出來的連線 socket。
   所以伺服器有 N 個客戶端時，會有 **N+1 個** socket。
2. **連線 socket 的本地位址，就是對方當初連過來的目的位址。** 實測 `accept()` 回來的 socket
   `getsockname()` 是 `('127.0.0.1', 18080)`、`getpeername()` 是 `('127.0.0.1', 49722)`。
   這件事在第六節很重要。

沒有任何 socket 在聽的埠，連過去會怎樣？核心找不到 socket，直接回 RST，客戶端看到的就是：

```
connect 沒人聽的埠: [Errno 111] Connection refused
```

---

## 四、UDP：一個 socket 對所有人

UDP 沒有交握，也沒有「連線」，所以**沒有監聽 socket 與連線 socket 之分**：

```
UNCONN  127.0.0.1:18081  0.0.0.0:*   fd=8     ← ss 顯示 UNCONN（unconnected）
```

同一個 UDP socket 會收到所有人寄來的 datagram，每次 `recvfrom()` 由核心告訴你「這一包是誰寄的」：

```
$ （兩個不同的客戶端各寄一包到 18082）
(b'from a', ('127.0.0.1', 59931))
(b'from b', ('127.0.0.1', 47389))     ← 同一個 socket，不同來源
```

| | TCP | UDP |
|---|---|---|
| 伺服器有幾個 socket | 1 個監聽 ＋ 每條連線 1 個 | 通常就 1 個 |
| 對端資訊存在哪 | 連線 socket 本身（`getpeername()`） | 每一包的附帶資訊（`recvfrom()` 的回傳值） |
| 「這一包原本要寄去哪」 | 就是連線 socket 的本地位址（`getsockname()`） | 要另外請核心附上（`IP_RECVORIGDSTADDR`） |

UDP socket 也可以 `connect()`，但那只是「記住預設的對端、過濾掉其他來源」，不會送出任何封包，
不是 TCP 意義的連線。

---

## 五、封包進來時，核心怎麼找到 socket

這一步叫**解多工（demultiplexing）**：網卡收到一個封包，核心要決定它屬於哪一個 socket。
它看的是**封包標頭**上的位址和埠。

**TCP 的查找順序：**

```
封包：src=192.168.1.50:54321  dst=192.168.1.1:22

① 已建立連線的雜湊表：用完整四元組精確比對
      有沒有 socket 的 本地=192.168.1.1:22、對端=192.168.1.50:54321？
      有 → 交給它（同一條連線的第 2 個封包之後都在這裡命中）
② 監聽 socket 的雜湊表：只用「目的位址:目的埠」比對
      有沒有 socket 聽在 192.168.1.1:22？沒有的話，有沒有聽在 0.0.0.0:22？
      （綁定特定位址的優先於 0.0.0.0）
      有 → 交給它，由它處理交握、生出新的連線 socket
③ 都沒有 → 回 RST（就是上面的 Connection refused）
```

**UDP** 比較單純：用「目的位址:目的埠」找（有 `connect()` 過的 UDP socket 會用四元組優先比對），
找不到就丟掉，並回一個 ICMP port unreachable。

**這個查找只在「路由判定說這封包是給本機的」之後才會發生。** 被判定為轉發的封包根本不會走到
傳輸層，也就不會找 socket。

---

## 六、回到 TPROXY：每一句話現在都說得通

有了上面的模型，[TPROXY與REDIRECT的差別與原理.md](./TPROXY與REDIRECT的差別與原理.md)
裡的句子可以逐一對上：

**「`tproxy to :12345` 把封包交給 12345 的 socket」**
LAN 電腦的封包是 `dst=1.1.1.1:443`。照第五節的查找，核心會去找「本地=1.1.1.1:443」的 socket——
本機根本沒有，結果就是 RST。`tproxy` 在 prerouting 先把**監聽在 12345 的那個 socket**綁在封包身上，
傳輸層看到「已經指定好了」，就**跳過第五節的查找**，直接交給它。

**「socket 要設 `IP_TRANSPARENT`」**
這是 `struct sock` 上的一個選項旗標。它允許這個 socket 接收、以及使用**不屬於本機的位址**。
沒有它，`tproxy` 找到了 socket 也不會用（核心會檢查這個旗標）。

**「TCP 的原始目的地怎麼拿」**
監聽 socket 收到 SYN 後，核心照第三節的流程生出新的連線 socket，而連線 socket 的本地位址
**就是封包的目的位址**——在這裡是 `1.1.1.1:443`，不是 `192.168.1.1:12345`。所以代理程式
`accept()` 之後呼叫 `getsockname()`，拿到的就是原始目的地。

**「tproxy 先找已建立的連線 socket」**
同一條連線的第 2 個封包起，四元組是 `192.168.1.50:54321 ↔ 1.1.1.1:443`，正好對上那個連線 socket
的本地與對端——`tproxy` 先照這個四元組找，找到就交給它；只有新連線的第一個封包才會落到監聽 socket。

**「UDP 要用 `IP_RECVORIGDSTADDR`」**
UDP 只有一個 socket 收所有人（第四節），沒有「每條連線一個、本地位址就是原始目的地」的 socket
可以問，只能請核心在每一包上附帶原始目的地。

**「回程要偽裝成 1.1.1.1」**
TCP 的連線 socket 本地位址本來就是 `1.1.1.1:443`，回覆時自然用它當來源。UDP 則要由代理程式
另外開一個 socket、`bind()` 到 `1.1.1.1:443`（同樣靠 `IP_TRANSPARENT` 才綁得了非本機位址）
再送回去。

---

## 七、自己觀察的方法

```sh
ss -tanp                 # 所有 TCP socket：-t TCP、-a 含 LISTEN、-n 不解析名字、-p 顯示行程與 fd
ss -ltnp                 # 只看監聽中的 TCP
ss -uanp                 # UDP
ss -tanpe                # 多印 ino:（inode）與 uid，用來對照 /proc/<pid>/fd
ls -l /proc/<pid>/fd     # 看行程手上哪些 fd 是 socket:[inode]
```

在路由器上驗證 dokodemo-door 時：`ss -ltnp | grep 12345` 應該看到一個 `LISTEN` 的 v2ray；
有 LAN 裝置正在被代理時，`ss -tnp` 會看到本地位址是**外部網站位址**（例如 `1.1.1.1:443`）、
行程卻是 v2ray 的 `ESTAB` socket——這就是第六節說的「連線 socket 的本地位址是原始目的地」，
也是 TPROXY 正在運作最直接的證據（這一項沒有在路由器上實測，依原理推得）。

`ss` 的完整選項與每一欄的意義見 [netstat怎麼用-以及該改用ss的理由.md](./netstat怎麼用-以及該改用ss的理由.md)。
這裡列的系統呼叫只有本篇用到的幾個；完整清單見 `man 7 socket`、`man 7 tcp`、`man 7 udp`、`man 7 ip`。

---

## 自我測驗

1. 一台 web 伺服器同時有 100 個客戶端連著，它有多少個 TCP socket？為什麼不是 100？
2. 「socket 就是連線」錯在哪？請用「端點」與「四元組」說明兩者的關係。
3. 監聽 socket 和 `accept()` 回來的 socket，各自的本地位址、對端位址是什麼？為什麼這個差別讓 TPROXY 的 TCP 代理可以用 `getsockname()` 拿到原始目的地？
4. 封包 `dst=1.1.1.1:443` 被 fwmark 路由判定為「給本機」，但規則裡沒有 `tproxy`。照第五節的查找順序，會發生什麼事？
5. 情境題：你在路由器上用 `ss -tnp` 看到一個 `ESTAB` 的 socket，本地位址是 `142.250.196.110:443`、行程是 v2ray。這台路由器並沒有這個 IP。為什麼會這樣？這代表什麼正在正常運作？
