# 學 C、系統程式、kernel，對寫 Go 後端有沒有用？能幫到系統設計嗎？

> 日期：2026-09-27
> 情境：正在上 [系統化學習路線](./Linux與系統程式的系統化學習路線.md)（C → Rust → kernel）；本職是用 Go 寫後台服務，
> 擔心自己「學得太上層」，想知道這條底層路線能不能跟工作融會貫通、對系統設計（system design）有沒有幫助
> 對應檔案：本篇為學習方法論筆記，不對應任何設定檔；第二節兩個 Go 實驗的完整原始碼就寫在筆記裡，沒有另存到 repo
> 適用範圍：本機 Ubuntu 24.04、x86-64、Go 1.24.2 實測；提到 Go 1.25 行為的地方未實測（本機沒有 1.25）

**一句話結論：有用，而且 Go 後端正是最需要它的地方——Go 的 runtime 把行程、執行緒、記憶體、網路都藏起來了，出事時看起來很玄，底層知識就是把「玄」變成「可以解釋」的東西。但它只覆蓋系統設計的「單機那一層」；分散式那一層（複製、一致性、分割）這條路線沒教，要另外補。而且融會貫通不會自動發生，要每一課刻意「接回 Go」。**

---

## 目錄

- [一、先回答：有用，但有兩個前提](#一先回答有用但有兩個前提)
- [二、Go 替你藏了什麼：兩個實驗](#二go-替你藏了什麼兩個實驗)
- [三、每一站對應到後端的哪些真實問題](#三每一站對應到後端的哪些真實問題)
- [四、對系統設計：兩層，這條路線只管下面那層](#四對系統設計兩層這條路線只管下面那層)
- [五、kernel 段的價值要分開看](#五kernel-段的價值要分開看)
- [六、怎麼讓它真的融會貫通](#六怎麼讓它真的融會貫通)

---

## 一、先回答：有用，但有兩個前提

「學得太上層」的感覺，通常來自這種經驗：服務出事了——延遲尖峰、被 OOM（Out Of Memory，記憶體不足）殺掉、
`too many open files`、連線打不出去——你能照網路上的做法調參數把它壓下去，但說不出**為什麼**。
會調參數的人很多；能從現象一路推到根因、而且預先知道設計會在哪裡出事的人很少。後者就是一般說的「大神」。

底層知識給的正是這個「為什麼」。但有兩個前提：

1. **它不會自動接上。** 學完虛擬記憶體，不會自己想到「所以容器被 OOMKilled 時，Go 的 heap 明明才 200 MB」該怎麼查。
   每學一個機制，要刻意問一次「這在我的 Go 服務裡長什麼樣子」（第六節）。
2. **它只覆蓋一部分。** 後端工程師的深度有兩條軸：往下（作業系統、硬體）和往外（資料庫、分散式系統）。
   這條路線只走往下那條。系統設計面試與實務裡的大半內容在往外那條（第四節）。

---

## 二、Go 替你藏了什麼：兩個實驗

Go 跟 C 最大的不同，是你的程式和核心之間多了一層 **Go runtime（執行期系統）**：goroutine 排程器（scheduler）、
垃圾回收（GC，Garbage Collection）、網路輪詢器（netpoller）。你寫的是 goroutine，核心看到的是執行緒（thread）；
你寫的是 `conn.Read`，核心看到的是 `epoll`。平常這層很好用，出事時它就是你看不懂的那一層。

### 實驗 1：Go 執行檔也有 interpreter——第 ① 站的東西直接出現在 Docker 映像檔裡

`main.go`：

```go
package main

import (
	"fmt"
	"net"
)

func main() {
	addrs, err := net.LookupHost("localhost")
	fmt.Println(addrs, err)
}
```

編譯兩次，一次用預設值，一次關掉 cgo（讓 Go 呼叫 C 函式庫的機制）：

```bash
go mod init gonet
go build -o net_cgo .
CGO_ENABLED=0 go build -o net_static .
file net_cgo net_static
readelf -l net_cgo | grep -A1 INTERP
readelf -d net_cgo | grep NEEDED
readelf -l net_static | grep INTERP || echo "net_static: no INTERP"
```

實際輸出（`file` 的 BuildID 已刪掉）：

```
net_cgo:    ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, with debug_info, not stripped
net_static: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, with debug_info, not stripped
  INTERP         0x0000000000000fe4 0x0000000000400fe4 0x0000000000400fe4
                 0x000000000000001c 0x000000000000001c  R      0x1
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
---
net_static: no INTERP
```

很多人以為「Go 編出來一定是靜態連結」。其實只要用到 `net`（或 `os/user`）且本機有 C 編譯器，預設會走 cgo，
產出的執行檔就有 `INTERP` 段，需要 `/lib64/ld-linux-x86-64.so.2` 和 glibc 的 `libc.so.6`。

後端日常會撞到的情境：把 `net_cgo` 放進 `FROM scratch`、`distroless/static` 或 Alpine（用 musl，載入器路徑是
`/lib/ld-musl-x86_64.so.1`，沒有 glibc 那個）的映像檔，啟動時得到 `no such file or directory`——檔案明明在。
這正是 [ELF的interpreter是什麼](../c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md) 第五節
〈直譯器不存在時：明明有檔案，卻說找不到〉那個現象；網路上的「解法」是加 `CGO_ENABLED=0`，
但懂第 ① 站的人知道**為什麼**加了就好：`INTERP` 沒了，核心不必去找載入器。
（未實測：在容器裡實際跑一次。機制與筆記第五節的本機實驗相同。）

### 實驗 2：goroutine 不等於執行緒——但卡在系統呼叫裡的 goroutine 會各佔一條

`main.go`：

```go
package main

import (
	"fmt"
	"os"
	"runtime"
	"strings"
	"syscall"
	"time"
)

// 讀 /proc/self/status 的 Threads: 欄位 = 這個行程目前有幾條 OS 執行緒
func osThreads() string {
	b, _ := os.ReadFile("/proc/self/status")
	for _, line := range strings.Split(string(b), "\n") {
		if strings.HasPrefix(line, "Threads:") {
			return strings.TrimSpace(strings.TrimPrefix(line, "Threads:"))
		}
	}
	return "?"
}

func main() {
	fmt.Println("GOMAXPROCS =", runtime.GOMAXPROCS(0))
	fmt.Println("開始：goroutine", runtime.NumGoroutine(), "OS 執行緒", osThreads())

	// 情況一：10000 個 goroutine 都在 time.Sleep（由 Go runtime 管，不佔 OS 執行緒）
	for i := 0; i < 10000; i++ {
		go func() { time.Sleep(time.Hour) }()
	}
	time.Sleep(200 * time.Millisecond)
	fmt.Println("10000 個 Sleep 後：goroutine", runtime.NumGoroutine(), "OS 執行緒", osThreads())

	// 情況二：50 個 goroutine 卡在「阻塞的系統呼叫」裡（nanosleep 直接進核心睡）
	for i := 0; i < 50; i++ {
		go func() {
			ts := syscall.Timespec{Sec: 3600}
			syscall.Nanosleep(&ts, nil)
		}()
	}
	time.Sleep(500 * time.Millisecond)
	fmt.Println("再加 50 個阻塞 syscall 後：goroutine", runtime.NumGoroutine(), "OS 執行緒", osThreads())
}
```

```bash
go mod init gothr
go run .
```

實際輸出：

```
GOMAXPROCS = 32
開始：goroutine 1 OS 執行緒 5
10000 個 Sleep 後：goroutine 10001 OS 執行緒 23
再加 50 個阻塞 syscall 後：goroutine 10051 OS 執行緒 53
```

- 一萬個 `time.Sleep` 的 goroutine 只用了 23 條執行緒：睡著的 goroutine 由 runtime 自己記在計時器裡，
  **不佔**執行緒。23 條是建立它們時 runtime 為了平行執行而開的（上限跟 `GOMAXPROCS` 有關）。
- 50 個卡在 `nanosleep` 系統呼叫裡的 goroutine，讓執行緒跳到 53：一個 goroutine 進了核心睡覺，
  **它所在的那條執行緒也一起被核心停住**，runtime 只能再拿（或新開）一條執行緒繼續跑其他 goroutine。
  所以大約是「每個卡在系統呼叫裡的 goroutine 佔一條」，加上 runtime 自己的幾條。

這解釋了幾個後端常見現象：

- **網路 I/O 不會這樣爆**：`net.Conn` 的讀寫不是直接阻塞在 `read` 上，而是交給 netpoller，底下是 `epoll`
  （第 ⑥ 站的內容）。所以一萬條閒置的 HTTP 連線不會變成一萬條執行緒。
- **磁碟 I/O、cgo 呼叫會**：大量 goroutine 同時做慢的檔案操作或 cgo 呼叫，執行緒數會衝上去，
  超過 runtime 的預設上限 10000 條時程式直接崩潰（`debug.SetMaxThreads` 可調）。
- **`GOMAXPROCS = 32` 是本機的 CPU 數**。放進 CPU 限制為 2 的容器裡，Go 1.24 以前仍會設成主機的 CPU 數，
  32 條執行緒搶 2 顆 CPU 的配額，被 cgroup（control group，核心的資源限制機制）的 CFS（Completely Fair Scheduler，
  完全公平排程器）配額節流（throttling），表現是 p99 延遲（最慢那 1% 請求的延遲）莫名尖峰。
  Go 1.25 起 runtime 會讀 cgroup 的 CPU 限制來設預設值（未實測：本機是 1.24.2）。
  這一條要第 ② 站（行程）＋第 ④ 站（執行緒）＋ kernel 的 cgroup 才能完整講清楚。

---

## 三、每一站對應到後端的哪些真實問題

| 站 | 學的是什麼 | 在 Go 後端裡的樣子 |
|---|---|---|
| ① 程式怎麼變成能跑的東西 | 編譯、連結、ELF、載入器、資料表示 | 實驗 1 的映像檔問題；`encoding/binary` 為什麼要指定 `BigEndian`／`LittleEndian`（網路協定的位元組順序）；struct 欄位順序影響記憶體對齊與大小（`unsafe.Sizeof`）；Go 整數溢位是定義好的回繞、不是 UB（Undefined Behavior，未定義行為）——跟第 2 課的 C 對照 |
| ① 陣列、指標、組合語言 | 指標運算、堆疊框 | slice 其實是（指標, len, cap）三欄的小 struct，`append` 後兩個 slice 共用底層陣列的 bug；逃逸分析（escape analysis，`go build -gcflags=-m`）決定變數放堆疊還是 heap；讀 `pprof` 與 `go tool objdump` 找熱點 |
| ② 行程 | `fork`／`exec`、訊號、行程狀態 | 優雅關機（graceful shutdown）：K8s 送 `SIGTERM`，沒處理就等到 `SIGKILL`；容器裡 PID 1 的特殊性與殭屍行程（zombie）；`os/exec` 底下發生什麼 |
| ③ 虛擬記憶體 | 分頁、`mmap`、heap、page cache | 被 OOMKilled 但 Go heap 不大：RSS（Resident Set Size，常駐記憶體）、GC 還沒還給核心的記憶體、page cache 怎麼算進 cgroup；`GOMEMLIMIT` 在限制什麼 |
| ④ 執行緒 | 執行緒、鎖、原子操作、記憶體模型 | 實驗 2；`sync.Mutex` 在競爭激烈時會去睡（`futex`）；data race 為什麼不是「偶爾算錯」而是未定義；false sharing（兩個 CPU 搶同一條快取線） |
| ⑤ 檔案與 I/O | 檔案描述符、緩衝、`fsync` | `too many open files`（fd 上限）；寫檔後當機資料還在不在（`fsync`）——資料庫的 WAL（Write-Ahead Log，預寫日誌）為什麼要這樣設計 |
| ⑥ 網路 | socket、TCP 狀態、`epoll` | Go netpoller 就是 `epoll`；`TIME_WAIT` 太多、連線池（`http.Transport` 的 `MaxIdleConnsPerHost`）設錯導致每個請求都重新握手；listen backlog 滿了；conntrack 表滿了丟封包（你 `linux/` 裡的 netfilter 筆記直接接得上） |
| Rust | 所有權、借用、`Send`／`Sync` | Go 不強制，但 Rust 會逼你想清楚「這份資料誰擁有、誰能同時改」——回頭寫 Go 的並行程式會少很多 race |

---

## 四、對系統設計：兩層，這條路線只管下面那層

系統設計可以粗分成兩層：

**下層：單機的物理限制。** 一台機器一秒能處理多少請求、延遲從哪裡來、瓶頸是 CPU、記憶體、磁碟還是網路。
估算（back-of-the-envelope estimation）時說「一台機器撐一萬條連線沒問題」「這個查詢每次多一次 `fsync` 就會慢一個數量級」，
靠的就是這一層。p99 延遲尖峰的來源——GC 暫停、page fault、CPU 節流、context switch（上下文切換）——全都在這一層。
**這條路線幾乎完整覆蓋這層**，而且這層是很多只會畫方塊圖的人最弱的地方：他們畫得出「加一層快取」，
說不出快取放同機還是跨網路差多少、為什麼。

**上層：分散式系統。** 資料怎麼複製（replication）、怎麼分割（partitioning／sharding）、一致性（consistency）
要到哪個程度、網路斷掉時選什麼、共識（consensus，例如 Raft）、冪等（idempotency）、訊息佇列的「至少一次」語意、
交易（transaction）與隔離等級（isolation level）。**這條路線完全沒教**，而系統設計的討論大半在這層。

所以直接的答案是：**對系統設計有幫助，但不夠。** 要補上層，最有效率的教材是
DDIA（*Designing Data-Intensive Applications*，Martin Kleppmann 著）——它剛好是從單機儲存講到分散式，
而你走完第 ③、⑤ 站後讀它的前半（儲存引擎、B-tree 與 LSM-tree、`fsync`）會有「原來是這樣」的感覺，
這就是融會貫通發生的地方。資料庫內部可以再加 CMU 15-445（公開課程）。

---

## 五、kernel 段的價值要分開看

第二段的 K0～K4 要誠實分開看：

- **K0～K1（自己編 kernel、懂 cgroup／namespace、會讀核心原始碼）**：對後端很有用。容器就是 cgroup＋namespace，
  eBPF（extended Berkeley Packet Filter，在核心裡安全執行小程式的機制）與 `perf` 是現代線上排查的主力工具。
- **K2～K4（送 patch、深耕子系統、成為維護者）**：對「寫後台服務」這份工作的邊際效益會明顯下降。
  它的價值在別處：開源履歷、跟頂尖工程師共事的機會、以及一種很難從其他地方得到的嚴謹（每行程式都會被逐字審查）。
  這是**職涯選擇**，不是成為好的後端工程師的必要條件。走到 K2 時值得重新問自己一次要不要繼續。

---

## 六、怎麼讓它真的融會貫通

「融會貫通」的意思是：看到一個現象，腦中能自動跑出底層機制。這要靠**刻意建立連結**，不是等它自己發生：

1. **每學完一站，寫下一個「這在我的 Go 服務裡長什麼樣子」的例子**，最好能寫成可以跑的 Go 程式（像第二節的兩個實驗）。
   這是 [學了就忘](./學了就忘-如何把知識轉成長期記憶.md) 裡的精緻化提問（elaborative interrogation）——把新知識掛到已有的知識上。
2. **把工作上遇到的線上問題帶進課堂問。** 真實事故是最好的教材：它逼你把兩三站的知識串起來（實驗 2 的 CPU 節流就橫跨三站）。
3. **每站專案做完 C 版與 Rust 版後，可以再用 Go 寫一次比較**（例如第 ⑥ 站的 echo 伺服器：C 用 `epoll` 手寫事件迴圈，
   Go 一條連線一個 goroutine——然後用 `strace` 看 Go 版底下其實也是 `epoll_wait`）。
4. **在系統設計的題目裡強迫自己估數字**：每一個方塊（快取、佇列、資料庫）旁邊寫出延遲與容量的數量級，並說出是哪一站的機制決定的。

---

## 自我測驗

1. 同事說「Go 編出來的都是靜態執行檔，丟進 `FROM scratch` 一定能跑」。這句話在什麼情況下是錯的？錯的時候會看到什麼錯誤訊息，為什麼那個訊息看起來跟事實矛盾？
2. 一個 Go 服務同時有一萬個 goroutine 在等 HTTP 請求，另一個服務同時有一千個 goroutine 在讀一個很慢的 NFS 磁碟。哪一個比較可能把 OS 執行緒數衝高？為什麼兩者不一樣？
3. 情境題：一個 Go 服務部署在 CPU 限制為 1 的 K8s Pod 裡（Go 1.23，主機有 64 顆 CPU），平均延遲正常，但 p99 每隔一段時間就尖峰。依這篇筆記，你會先懷疑什麼？它牽涉到哪幾站的知識？
4. 為什麼說這條學習路線「對系統設計有幫助，但不夠」？分別舉一個它能幫你回答、和它完全幫不上的系統設計問題。
