# Learning 筆記索引

這個目錄收錄「怎麼學」本身的方法論——學習、記憶、複習策略，以及這些原則該如何套用到
本筆記庫的寫作與複習流程上。與技術領域無關，但決定了其他資料夾裡的筆記能不能真的留在腦子裡。

## 筆記列表

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [學了就忘-如何把知識轉成長期記憶.md](./學了就忘-如何把知識轉成長期記憶.md) | 「學了就忘」的根因是只做輸入沒做提取，而非記憶力衰退：儲存強度與提取強度的區分、流暢性錯覺為何讓重讀無效、提取練習／間隔重複／交錯練習三大機制、40 歲後流體智力與晶體智力的消長該如何調整策略，以及把自我測驗區塊與 `> 下次複習：` 排程欄位加進本筆記庫的具體作法 | `retrieval practice` `testing effect` `spacing effect` `interleaving` `desirable difficulties` `fluency illusion` `forgetting curve` `storage strength` `retrieval strength` `SRS` `Anki` `elaborative interrogation` `generation effect` `consolidation` `crystallized intelligence` |
| [Linux與系統程式的系統化學習路線.md](./Linux與系統程式的系統化學習路線.md) | 零散感來自缺主線而非知識量：改走「程式 → 行程 → 虛擬記憶體 → 執行緒 → 檔案與 I/O → 網路」六站，每站一個 C 專案（迷你 shell、malloc、thread pool、echo→epoll 伺服器）再用 Rust 重寫；教材只選 CS:APP（主幹）、OSTEP（觀念）、TLPI（字典），C++ 延後；並把 `linux/` 既有筆記歸位到地圖上，看出網路／netfilter 很深、但虛擬記憶體與執行緒整站空白；第二段為 kernel 維護者路線（K0 編 kernel → K1 郵件 patch 流程 → K2 第一批 patch → K3 深耕子系統 → K4 `MAINTAINERS`），建議從 netfilter 切入 | `學習路線` `CS:APP` `OSTEP` `TLPI` `Beej` `Rust Atomics and Locks` `xv6` `process` `thread` `virtual memory` `socket` `strace` `gdb` `C` `Rust` `MAINTAINERS` `checkpatch.pl` `get_maintainer.pl` `git send-email` `b4` `lore.kernel.org` `netfilter-devel` |
| [學習進度.md](./學習進度.md) | 逐課教學的進度檔：目前在哪一站、哪一課、哪些作業與自我測驗還沒交、下一課教什麼，以及第 ① 站的課程規劃與間隔複習排程；每課結束都要更新 | `學習進度` `目前位置` `課程紀錄` `待複習` `第 ① 站` |
| [學C與kernel對Go後端有沒有用.md](./學C與kernel對Go後端有沒有用-底層知識怎麼接到日常工作與系統設計.md) | 底層路線對 Go 後端有用，因為 Go runtime 把行程／執行緒／記憶體／網路藏起來、出事時才看起來玄；兩個實測：用了 `net` 的 Go 執行檔預設有 `INTERP`（進 scratch／Alpine 會 `no such file or directory`）、卡在系統呼叫的 goroutine 各佔一條 OS 執行緒；六站對應到後端真實問題的對照表；系統設計分單機與分散式兩層，本路線只覆蓋單機層，分散式要另補 DDIA；kernel 段 K2 以後是職涯選擇而非必要 | `Go runtime` `goroutine` `GOMAXPROCS` `CGO_ENABLED` `INTERP` `netpoller` `epoll` `cgroup` `CFS throttling` `OOMKilled` `GOMEMLIMIT` `system design` `DDIA` `p99` |

---
> 新增筆記時，記得把它加進上面的「筆記列表」表格。
