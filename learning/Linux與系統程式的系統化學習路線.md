# 知識點太零散，怎麼從頭系統化學 Linux 與 CS 基礎？—— 一條「系統程式」主線

> 日期：2026-09-22
> 情境：累積了幾十篇 Linux／網路筆記（fd、socket、TPROXY、nftables、shell…），但都是遇到問題才學，
> 覺得太零散；想從頭有系統地學 Linux，並把 process、thread、TCP 等 CS 基礎，搭配 C／C++／Rust 一起學
> 適用範圍：自學規劃；書與課程都是公開、長期被當作標準教材的版本
> 相關：[學了就忘-如何把知識轉成長期記憶.md](./學了就忘-如何把知識轉成長期記憶.md)（每一站怎麼讀、怎麼複習）

**一句話結論：不要「從頭重學 Linux」，而是換一條主線——用「一支程式從寫出來到跑起來、跟別人通訊」
這條路，依序學編譯與記憶體 → 行程 → 執行緒與同步 → 檔案與 I/O → 網路。這條線同時就是 CS 基礎、
Linux 系統呼叫、C 語言的交集。每一站用 C 寫一個小專案、用 `strace`／`gdb` 看核心在做什麼，
最後再用 Rust 重寫一次。零散的舊筆記不用丟，它們會變成這張地圖上已經點亮的點。
目標若是成為 kernel 維護者，這六站就是地基；上面再接第二段「kernel 開發」——而維護者不是考出來的職位，
是在**一個子系統**長期送 patch、審別人的 patch、被信任之後才被寫進 `MAINTAINERS` 的，以年為單位。**

---

## 目錄

- [一、為什麼會覺得零散：缺的是主線，不是知識量](#一為什麼會覺得零散缺的是主線不是知識量)
- [二、主線：六站](#二主線六站)
- [三、教材：只選這幾本](#三教材只選這幾本)
- [四、語言怎麼搭：C 先、Rust 後、C++ 延後](#四語言怎麼搭c-先rust-後c-延後)
- [五、每一站的固定節奏](#五每一站的固定節奏)
- [六、舊筆記在地圖上的位置與缺口](#六舊筆記在地圖上的位置與缺口)
- [七、目標是 kernel 維護者：第二段主線](#七目標是-kernel-維護者第二段主線)
- [八、為什麼建議從 netfilter 切入](#八為什麼建議從-netfilter-切入)
- [自我測驗](#自我測驗)

---

## 一、為什麼會覺得零散：缺的是主線，不是知識量

問題驅動的學習（遇到 TPROXY 不通 → 學 ip rule → 學 socket）學得很深，但每個點是**從問題長出來的**，
彼此之間沒有一條「先有 A 才有 B」的骨架。結果是：

- 知道 `fd` 是什麼，但不知道它跟 `fork()`、`exec()`、管線（pipe）是同一套機制。
- 知道 socket 是端點，但沒寫過一支 `accept()` 迴圈，所以 backlog、thread、`epoll` 各自獨立。
- 知道 root 執行腳本要 x 位元，但 `execve()` 在行程生命週期裡的位置是空白的。

**缺的是一條依賴順序清楚的主線。** 有了主線，新問題學到的東西就有地方掛（這正是
[學了就忘](./學了就忘-如何把知識轉成長期記憶.md) 第五節說的「精緻化」：新知識要掛在舊知識上才留得住）。

---

## 二、主線：六站

順序是依「後面的站需要前面的站」排的：

```
① 程式怎麼變成能跑的東西      C、編譯、連結、資料在記憶體裡長怎樣、讀得懂組合語言
        ↓
② 行程（process）            fork / exec / wait、signal、fd、管線 —— shell 就是這一站的總和
        ↓
③ 虛擬記憶體                 位址空間、分頁、mmap、malloc 怎麼來的
        ↓
④ 執行緒與同步（thread）      thread vs process、競態條件、mutex、條件變數、死結
        ↓
⑤ 檔案與 I/O                 inode、權限、掛載、緩衝、阻塞 vs 非阻塞、epoll
        ↓
⑥ 網路                       socket API → TCP 交握與狀態 → 並行伺服器 → 路由與 netfilter
```

每一站一個**動手專案**，專案是這一站的「畢業考」：

| 站 | 專案 | 做完你會真正懂的東西 |
|---|---|---|
| ① | 用 C 寫幾個小工具（`wc`、`cat` 的簡化版），故意寫錯看 segfault，用 `gdb` 找 | 指標、陣列、字串、編譯與連結、`gdb` |
| ② | **寫一個迷你 shell**：執行命令、`$?`、`>` 重新導向、`\|` 管線、Ctrl-C | `fork`／`execve`／`waitpid`、`dup2`、`pipe`、signal——你的 shell 筆記全部串起來 |
| ③ | 讀 `/proc/<pid>/maps`、寫一個最簡單的 `malloc` | 虛擬記憶體、堆積（heap）、`brk`／`mmap` |
| ④ | 執行緒池（thread pool）＋一個會出競態的計數器，先重現 bug 再修 | race condition、mutex、condvar、死結 |
| ⑤ | 自己的 `ls -l`（讀 inode、權限位元）；用 `strace` 比較有無緩衝的寫檔 | inode、權限、`stat`、系統呼叫成本 |
| ⑥ | echo server → 多執行緒版 → `epoll` 版 → 最小 HTTP 伺服器；同時用 `tcpdump`／`ss` 看交握 | socket API、TCP 狀態、backlog、並行模型的取捨 |

⑥ 之後，你既有的 ip rule／nftables／TPROXY 筆記就是「⑦ 核心怎麼處理封包」這一站，已經走了一大段。

---

## 三、教材：只選這幾本

教材越多越散。核心只有三本，其餘按需要查：

| 角色 | 教材 | 用在哪幾站 | 為什麼是它 |
|---|---|---|---|
| **主幹** | *Computer Systems: A Programmer's Perspective*（CS:APP，Bryant & O'Hallaron，第 3 版） | ①②③⑤⑥ | 從程式設計師的角度串起資料表示、連結、行程、虛擬記憶體、I/O、網路、並行，全用 C；CMU 公開了配套的實驗（lab），其中 shell lab、malloc lab 正好是上面的專案 |
| **作業系統觀念** | *Operating Systems: Three Easy Pieces*（OSTEP，Arpaci-Dusseau，免費線上） | ②③④⑤ | 行程排程、虛擬記憶體、並行、持久化講得最清楚；並行（concurrency）那一部分是 ④ 的主教材 |
| **Linux 系統呼叫字典** | *The Linux Programming Interface*（TLPI，Michael Kerrisk） | 全部，當字典查 | Linux 專屬的細節（`epoll`、`/proc`、signal 的邊角）最完整；作者也是 man-pages 的維護者 |
| 網路程式 | *Beej's Guide to Network Programming*（免費） | ⑥ | 最短路徑寫出第一支 socket 程式 |
| Rust | *The Rust Programming Language*（官方書，免費）→ *Rust Atomics and Locks*（Mara Bos，免費線上） | 第二輪 | 後者專講執行緒、原子操作、記憶體順序，是 ④ 的 Rust 版 |

（書名與作者依公開出版資訊；版本以你拿到的為準。）

**進階、可以之後再決定的**：MIT 6.1810（前身 6.S081）用教學作業系統 xv6 讓你**改核心**——做完主線
六站、還想知道「系統呼叫在核心那一側長怎樣」時再上。網路協定本身想更深，再補 Kurose & Ross 的
*Computer Networking: A Top-Down Approach*。

---

## 四、語言怎麼搭：C 先、Rust 後、C++ 延後

| 語言 | 什麼時候 | 理由 |
|---|---|---|
| **C** | 主線全程 | Linux 的系統呼叫介面就是 C 函式；`man 2 fork` 的範例是 C；CS:APP 與 TLPI 都是 C。用 C 才看得到「核心交給你的是什麼」——沒有執行期（runtime）幫你藏起來 |
| **Rust** | 每一站做完 C 版之後，用 Rust **重寫同一個專案** | 對照最有效：C 版的 use-after-free、資料競爭，在 Rust 會變成**編譯錯誤**。你會具體理解所有權（ownership）與 `Send`／`Sync` 在防什麼，而不是背規則 |
| **C++** | 延後，除非工作需要 | C++ 跟 C、Rust 的重疊太大，三個一起學只會三個都淺。主線要的「看得到底層」C 已經給了，「安全的抽象」Rust 已經給了 |

---

## 五、每一站的固定節奏

1. **讀**：主幹教材對應的章節（一次一章，不要一口氣讀完一本）。
2. **觀察**：用工具看真實系統——`strace -f` 看系統呼叫、`gdb` 看記憶體、`/proc/<pid>/` 看核心對行程的記帳、
   `ss`／`tcpdump` 看網路。**書上的每一個概念，都要在自己機器上找到它的影子。**
3. **做**：這一站的專案，C 版。
4. **寫筆記**：照本筆記庫的規範（根因、文法、自我測驗），放進對應領域資料夾。
5. **提取**：隔幾天、隔一週回頭做自我測驗（方法見 [學了就忘](./學了就忘-如何把知識轉成長期記憶.md)）。
6. **重寫**：Rust 版，筆記補一段「C 與 Rust 的差別」。

**問題驅動的學習不用停。** 路由器、K8s 上遇到的問題照樣追到底，只是寫完筆記後多做一步：
在下面的地圖上標記它屬於哪一站。

---

## 六、舊筆記在地圖上的位置與缺口

`linux/` 目前的筆記，依主線重新歸位（只列相關的）：

| 站 | 已經有的 | 缺口（主線學到時補） |
|---|---|---|
| ① 程式與編譯 | [bin與sbin的差別](../linux/bin與sbin的差別-以及怎麼正確找到執行檔.md)（`PATH`） | C 語言本身、編譯與連結、ELF、動態函式庫 |
| ② 行程 | [fd是什麼](../linux/fd是什麼-行程的檔案描述符表.md)、[輸出重新導向與dev-null](../linux/輸出重新導向與dev-null.md)、[PTY是什麼](../linux/PTY是什麼-終端機模擬器與虛擬終端.md)、[從終端機啟動GUI程式並脫離終端機](../linux/從終端機啟動GUI程式並脫離終端機.md)、[root執行腳本也Permission-denied](../linux/root執行腳本也Permission-denied-缺執行位元.md)（`execve`）、shell 系列（[條件判斷](../linux/shell條件判斷測試.md)、[for](../linux/shell的for迴圈語法.md)、[case](../linux/shell的case語法-樣式比對與分支.md)、[錢字號展開](../linux/錢字號展開的四種形式-bad-substitution的根因.md)）、[journalctl看服務日誌](../linux/journalctl看服務日誌.md) | **`fork`／`exec`／`wait` 本身**、signal、行程狀態與排程、systemd 與開機流程 |
| ③ 虛擬記憶體 | （無） | 整站都是缺口 |
| ④ 執行緒與同步 | （無） | 整站都是缺口 |
| ⑤ 檔案與 I/O | 同上 root 那篇的 `chmod` 部分、[procfs與conntrack遺失問題](../linux/procfs與conntrack遺失問題.md)（`/proc`） | inode、掛載、緩衝、`epoll` |
| ⑥ 網路（socket 與 TCP） | [socket是什麼](../linux/socket是什麼-從核心看連線的端點.md)、[什麼是backlog](../linux/什麼是backlog-TCP交握背後的兩個佇列.md)、[netstat與ss](../linux/netstat怎麼用-以及該改用ss的理由.md) | 自己寫過 socket 程式、TCP 狀態機、並行模型 |
| ⑦ 核心怎麼處理封包 | [iptables與nftables](../linux/iptables與nftables怎麼用.md)、[nft完整用法](../linux/nft完整用法-從命令到規則語法.md)、[多張nft表的執行順序](../linux/多張nft表的執行順序與ip-rule的位置.md)、[ip-rule與ip-route](../linux/ip-rule與ip-route完整指南.md)、[TPROXY與REDIRECT](../linux/TPROXY與REDIRECT的差別與原理.md)、[sysctl](../linux/sysctl是什麼-核心參數的讀寫與持久化.md)、[DHCP](../linux/DHCP完整流程與封包逐位元組解剖.md) | 已經走得很深——這一站是你目前最強的 |
| 核心周邊 | [核心模組檢查](../linux/核心模組檢查.md)、[dpkg與dkms的關係](../linux/dpkg與dkms的關係.md)、[NVIDIA-DKMS](../linux/NVIDIA-DKMS核心升級失敗排查.md) | 核心與使用者空間的邊界（系統呼叫怎麼進核心） |

**這張表說明了「零散感」的來源**：⑦ 很深，但它依賴的 ②③④⑥ 有大塊空白。
從 ① 開始往下走，會一站一站把 ⑦ 底下的地基補起來。

---

## 七、目標是 kernel 維護者：第二段主線

### 先把「維護者」是什麼講清楚

kernel 原始碼根目錄的 `MAINTAINERS` 檔，每個子系統一段，裡面的欄位：

| 欄位 | 意思 |
|---|---|
| `M:` | 維護者（maintainer）：負責收 patch、決定合不合、送往上游 |
| `R:` | 指定審查者（designated reviewer）：patch 會自動抄送給他審 |
| `L:` | 這個子系統的郵件列表（mailing list），patch 與討論都在這裡 |
| `S:` | 狀態：`Maintained`、`Supported`、`Odd Fixes`、`Orphan`… |
| `F:` | 這個子系統管哪些檔案 |

沒有考試或申請流程。一般的路是：**在同一個子系統持續送出被接受的 patch → 持續審別人的 patch
（回 `Reviewed-by:`／`Tested-by:`）→ 被加成 `R:` → 某個區塊需要人時被加成 `M:`**。
這是以年為單位的事，核心是**信任**：維護者相信你的判斷，才會把一塊程式交給你。

所以目標要拆成兩件事：**技術上能改 kernel**，以及**在社群裡建立紀錄**。後者沒有捷徑，
只能靠長期、穩定、在同一塊地方出現。

### 第二段主線：K0～K4

**前提**：第一段的 ②③④（行程、虛擬記憶體、並行）是必要的地基——kernel 程式碼充滿
自旋鎖（spinlock）、RCU（Read-Copy-Update，讀取-複製-更新）、記憶體配置旗標，沒有 ④ 讀不懂。
但 **K0 不必等**，第一段學到 ② 就可以平行開始。

| 站 | 做什麼 | 完成的標準 |
|---|---|---|
| **K0 自己編 kernel** | 從原始碼設定、編譯 kernel，用 QEMU 開機；寫一個 hello world 核心模組，`insmod`／`rmmod`、看 `dmesg` | 能改一行 `printk`，重編、開機、看到它 |
| **K1 學會工作流程** | kernel 用**郵件**收 patch，不用 GitHub PR：`git format-patch` → `scripts/checkpatch.pl` 檢查風格 → `scripts/get_maintainer.pl` 查該寄給誰 → `git send-email` 寄出；用 `b4` 下載與套用列表上的 patch；在 lore.kernel.org 讀歷史討論 | 讀完 `Documentation/process/` 裡的 `howto.rst`、`submitting-patches.rst`、`coding-style.rst`；對自己的測試信箱完整走一次寄送 |
| **K2 第一批 patch** | 從風險低的開始：文件錯誤、編譯警告、`drivers/staging/` 的清理、syzbot（Google 的自動 fuzzing 機器人）回報的 bug；同時學會跑測試：kselftest、KUnit | 至少一個 patch 被合進主線（出現在 `git log --author` 裡） |
| **K3 選定一個子系統深耕** | 訂閱它的郵件列表、每週讀新 patch 與討論、在自己的環境跑它的測試、開始回覆審查意見 | 維護者開始認得你的名字；你審過的 patch 帶著你的 `Reviewed-by:` 被合進去 |
| **K4 成為審查者／維護者** | 持續 K3，主動接下沒人管的小區塊（例如某個 `S: Orphan` 的驅動或測試） | `MAINTAINERS` 裡出現你的名字 |

### kernel 開發的教材（與第一段分開）

| 教材 | 用途 |
|---|---|
| kernel 原始碼裡的 `Documentation/process/` | **流程的權威來源**，其他教學都在轉述它 |
| Linux Foundation 免費課程 *A Beginner's Guide to Linux Kernel Development*（LFD103） | K0～K2 的導覽：編譯、第一個 patch、寄送 |
| Bootlin 的 *Linux kernel and driver development* 訓練教材（免費公開投影片與實驗） | 驅動、裝置模型、同步機制，內容跟著新版 kernel 更新 |
| LWN.net | 每週的 kernel 開發新聞與深度文章；讀它是理解「社群現在在吵什麼」最快的方法 |
| KernelNewbies（kernelnewbies.org） | 新手社群、每個版本的變更摘要 |
| 導師計畫：Linux Foundation 的 LFX Mentorship（Linux Kernel Bug Fixing）、Outreachy | 有人帶著做 K2，也是認識維護者的正式管道 |

較舊但仍常被推薦的書：Robert Love 的 *Linux Kernel Development*（2010，kernel 2.6，觀念仍有用、細節已過時）、
*Linux Device Drivers* 第 3 版（更舊，只適合讀觀念）。**以原始碼與 `Documentation/` 為準，書只當導讀。**

### Rust 在 kernel 裡的位置

Rust 支援從 6.1 版起進入主線，由 Rust for Linux 專案推動，目前主要用在新的驅動與抽象層。
**kernel 的主體仍然是 C**，要當任何子系統的維護者都必須能讀寫 C。Rust 是加分與未來的方向：
第一段「C 寫一次、Rust 重寫一次」的習慣，正好對應 kernel 裡「C 的子系統、Rust 的抽象層」這個現況。

---

## 八、為什麼建議從 netfilter 切入

K3 要選一個子系統。選的原則是：**你已經在用、已經有問題想問、而且能自己測試的**。
你目前最深的正是 netfilter／nftables 與路由（地圖上的 ⑦），這是天然的候選：

- 你已經在讀它的使用者介面（`nft`）與語意（hook、priority、verdict）。
- 你有真實的使用場景（OpenWrt 的 fw4、TPROXY），遇到的問題能變成測試案例。
- 它有獨立的郵件列表（netfilter-devel）與測試（kselftest 裡的 netfilter 測試）。
- 它屬於網路子系統，流程規矩寫在 `Documentation/process/maintainer-netdev.rst`（例如 `net` 與
  `net-next` 兩棵樹的分工、合併窗口期間不收新功能），是必讀的「在地規則」。

**已經可以做的第一個 kernel 閱讀練習**：`net/netfilter/nft_tproxy.c` 的 `nft_tproxy_eval_v4()`，
不到 50 行。[TPROXY與REDIRECT的差別與原理.md](../linux/TPROXY與REDIRECT的差別與原理.md)
裡「找不到 socket 時規則會中斷」「先找已建立的 socket 再找監聽 socket」「只能用在 prerouting」
這幾件事，就是從這個函式讀出來的。**從你已經會用的功能讀進它的原始碼**，是進入 kernel 最自然的方式。

（這不是唯一選擇。若之後對檔案系統、排程或驅動更有興趣，換子系統也完全正常——重點是
選定之後要**長期待在同一個地方**。）

---

## 自我測驗

1. 為什麼說零散的原因是「缺主線」而不是「知識量不夠」？既有的哪些筆記其實屬於同一站，卻被當成不相關的東西學？
2. 主線為什麼是「行程 → 虛擬記憶體 → 執行緒 → 檔案 → 網路」這個順序？舉一個「後面的站需要前面的站」的具體例子。
3. 為什麼主線用 C，而不是直接用 Rust？「用 Rust 重寫同一個專案」比「直接學 Rust」多得到什麼？
4. 情境題：你讀完 CS:APP 的行程那一章，覺得都懂了。依第五節的節奏，接下來還缺哪幾步，這一站才算真的完成？
5. 「成為 kernel 維護者」為什麼不能當成一個可以「學完」的目標？`MAINTAINERS` 裡的 `R:` 和 `M:` 分別代表什麼，一般人是怎麼走到那裡的？為什麼建議你從 netfilter 而不是隨便一個子系統開始？
