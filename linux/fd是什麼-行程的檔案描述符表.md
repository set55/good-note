# fd 是什麼？—— 行程手上的那張對照表

> 日期：2026-09-20
> 情境：談 `CLOSE_WAIT` 堆積會撞上 `too many open files` 時追問的：fd 到底是什麼
> 適用範圍：實測 Ubuntu 24.04（kernel 7.0）
> 相關：[輸出重新導向與dev-null.md](./輸出重新導向與dev-null.md)（fd 0／1／2 的用法在那篇）、
> [netstat怎麼用-以及該改用ss的理由.md](./netstat怎麼用-以及該改用ss的理由.md)、
> [排查方法論.md](./排查方法論.md)

**一句話結論：fd（file descriptor，檔案描述符）是**一個整數**，是行程手上那張「編號 → 已開啟資源」
對照表的索引。關鍵有三點：這張表**每個行程各一份**（所以編號只在該行程內有意義）；表裡指的
**不只是檔案**（socket、pipe、計時器都是 fd，這就是「一切皆檔案」的實作）；而且**編號背後還有
一層**（open file description），`dup` 與 `fork` 共享的是那一層，這解釋了很多看似奇怪的行為。**

---

## 一、先看實物

```bash
$ ls -l /proc/self/fd/
lr-x------ 0 -> /dev/null
l-wx------ 1 -> pipe:[202486]
l-wx------ 2 -> /dev/null
lr-x------ 3 -> /proc/44101/fd
```

左邊那個 `0`、`1`、`2`、`3` 就是 fd——**它就只是個整數**。右邊是它目前指向的東西。

`/proc/<PID>/fd/` 是核心直接把這張表以符號連結的形式呈現出來，所以「fd 是什麼」不用想像，
直接 `ls` 就看得到。

**這張表是每個行程各一份**：

```bash
$ echo "$(readlink /proc/self/fd/1)"          # pipe:[176011]
$ sh -c 'readlink /proc/self/fd/1'            # pipe:[222420]   ← 不同的東西
```

同樣是編號 1，在不同行程裡指向完全不同的資源。**fd 的編號離開它的行程就沒有意義**——
不能把 fd 3 傳給另一個程式叫它用（真要傳得透過 Unix socket 的 `SCM_RIGHTS` 機制，
核心會在對方的表裡新建一個項目）。

新開的 fd 一律拿**當前最小的可用編號**，所以關掉 fd 1 之後再開檔案，新檔案就會變成 fd 1——
這正是 shell 做重新導向的原理。

---

## 二、三層結構：這是最關鍵的部分

```
行程 A 的 fd 表          系統層的 open file description        inode（檔案本體）
┌────┬──────┐           ┌──────────────────────┐           ┌──────────────┐
│ 0  │ ───► │──────────►│ offset: 1024         │──────────►│ /var/log/x   │
│ 1  │ ───► │──────┐    │ flags: O_WRONLY      │           │ 權限、大小…  │
│ 2  │ ───► │──────┘    └──────────────────────┘           └──────────────┘
└────┴──────┘                （1 與 2 指向同一個）
```

| 層 | 屬於誰 | 存了什麼 |
|---|---|---|
| **fd** | 行程 | 就是一個整數索引 |
| **open file description** | 系統（核心） | **讀寫位置（offset）**、開啟模式、旗標 |
| **inode** | 檔案系統 | 檔案本體、權限、大小 |

**分成三層的後果，全都在「誰跟誰共享 offset」上：**

| 情況 | 共享 description？ | 後果 |
|---|---|---|
| `dup` / `dup2`（`2>&1` 用的） | ✅ 共享 | **共用同一個 offset**，一邊寫另一邊的位置也跟著動 |
| `fork` 出的子行程 | ✅ 共享 | 父子寫同一個檔案不會互相覆蓋 |
| 對同一個檔案 `open` 兩次 | ❌ 各自獨立 | 兩個 offset 互不相干，會互相覆蓋 |

**這就是 `>/dev/null 2>&1` 和 `2>&1 >/dev/null` 結果不同的根本原因**：`2>&1` 複製的是
「fd 1 **當下**指向的那個 description」，而不是「fd 1 這個編號」。（完整推導見
[輸出重新導向與dev-null.md](./輸出重新導向與dev-null.md)。）

同理，`cmd >>file` 的 append 模式是記在 description 上的旗標，所以父子行程同時 append
不會打架；而兩次獨立 `open` 的非 append 寫入就會。

---

## 三、fd 指的不只是檔案

看一個真實的大型程式（本機 chrome，479 個 fd）：

```
  117  （一般檔案）
  110  (deleted)              ← 已被刪除但還開著的檔案
   99  socket:[…]             ← 網路連線
    9  anon_inode:[eventfd]   ← 行程內／執行緒間的事件通知
    8  pipe:[…]               ← 管線
    4  anon_inode:[eventpoll] ← epoll 實例本身也是一個 fd
```

| 型別 | 是什麼 |
|---|---|
| 一般檔案 | 真的檔案 |
| `socket:[N]` | TCP／UDP／Unix socket；`N` 是 inode 編號，可以拿去跟 `ss -e` 對上 |
| `pipe:[N]` | 管線的一端 |
| `anon_inode:[eventfd]` | 純粹用來喚醒對方的計數器 |
| `anon_inode:[eventpoll]` | **epoll 實例**——用來監看其他 fd 的 fd |
| `anon_inode:[timerfd]` | 計時器 |
| `anon_inode:[inotify]` | 檔案變動監看 |

**「一切皆檔案」的實際意思就是這個**：不管底層是網路連線、計時器還是事件通知，對程式來說
都是「一個 fd」，都能用 `read`／`write`／`close`、都能丟進 `epoll` 一起等。
這讓一個事件迴圈能同時處理網路、計時與訊號，不必為每種資源寫一套邏輯。

---

## 四、`(deleted)` 的意義：空間沒被釋放的元兇

chrome 那 110 個 `(deleted)` 是一個非常實用的排查線索。

**在 Linux 上刪除檔案，刪的是「目錄項」（unlink），不是資料本身。** 只要還有任何行程開著它，
inode 與磁碟空間就不會釋放：

```
df -h   → 磁碟滿了
du -sh  → 加起來卻遠小於那個數字
```

**兩者對不上，就去找「已刪除但還開著」的檔案**：

```bash
lsof +L1                              # 列出連結數為 0（已刪除）卻還開著的檔案
ls -l /proc/<PID>/fd/ | grep deleted
```

最經典的場景是有人用 `rm` 刪了一個正在被寫入的大 log 檔——空間完全沒有回來，
要等到重啟那個行程（或它自己 `close`）才會釋放。

---

## 五、上限有三層

```bash
$ ulimit -Sn / -Hn        # 1048576 / 1048576   ← 單一行程（soft/hard）
$ sysctl -n fs.nr_open    # 1048576             ← 單一行程的硬天花板
$ sysctl -n fs.file-max   # 9223372036854775807 ← 全系統（本機等於無限）
$ cat /proc/sys/fs/file-nr
16658   0   9223372036854775807                  # 已配置 / 未使用 / 上限
```

| 層 | 誰管 | 超過時 |
|---|---|---|
| `ulimit -n`（soft） | 每個行程，可自行調高到 hard | **`EMFILE`：too many open files** |
| `fs.nr_open` | 核心，`ulimit` 不能超過它 | 調 ulimit 時被拒 |
| `fs.file-max` | 全系統總量 | **`ENFILE`：file table overflow** |

**`EMFILE` 與 `ENFILE` 是兩個不同的問題**：前者是「這個行程開太多」（多半是程式洩漏），
後者是「整台機器開太多」（罕見）。看到 `too many open files` 先確認是哪一個——
調 `ulimit` 對 `ENFILE` 沒有用。

> 服務用 systemd 啟動時，`ulimit` 由 unit 的 `LimitNOFILE=` 決定，**不會**繼承你 shell 裡
> 調好的值。這是「我明明調大了 ulimit，服務還是報 EMFILE」的標準答案。

---

## 六、fd 洩漏：怎麼診斷

**症狀**：服務跑一陣子後開始報 `too many open files`，重啟就好——典型的洩漏曲線。

```bash
# 1. 哪個行程 fd 最多
for p in /proc/[0-9]*; do echo "$(ls $p/fd 2>/dev/null | wc -l) $(basename $p)"; done \
  | sort -rn | head -5

# 2. 持續觀察單一行程（會不會只增不減）
watch -n5 "ls /proc/<PID>/fd | wc -l"

# 3. 洩漏的是什麼型別
ls -l /proc/<PID>/fd/ | awk '{print $NF}' | sed 's/\[[0-9]*\]//' | sort | uniq -c | sort -rn

# 4. 和它的上限比
cat /proc/<PID>/limits | grep 'open files'
```

**第 3 步的型別會直接告訴你洩漏在哪**：

- `socket:[…]` 一直增加 → 沒有 `close()` 連線。**搭配 `ss -tp state close-wait` 一起看**，
  大量 `CLOSE_WAIT` + fd 只增不減，就是同一個 bug 的兩個症狀（見
  [netstat 那篇](./netstat怎麼用-以及該改用ss的理由.md)）。
- 一般檔案一直增加 → 開檔沒關，常見於錯誤處理路徑忘了關。
- `(deleted)` 一直增加 → 開著已刪除的暫存檔。

**權限提醒**：非 root 讀不到別人的 `/proc/<PID>/fd/`。本機實測 `verge-mihomo` 是以 **root**
執行的，所以一般使用者 `ls` 它的 fd 會得到空結果——**不是沒有 fd，是沒權限**。
這種「空結果」最容易誤導，先用 `ps -o user` 確認擁有者。

---

## 七、速查

```bash
ls -l /proc/self/fd/                 # 自己的 fd 表
ls -l /proc/<PID>/fd/                # 別人的（需同使用者或 root）
lsof -p <PID>                        # 同樣的資訊，格式更好讀
lsof -i :8080                        # 誰開著這個埠
lsof +L1                             # 已刪除但還開著的檔案 ← 磁碟空間對不上時用
cat /proc/<PID>/limits               # 這個行程實際的上限
ulimit -n                            # 當前 shell 的 soft limit
ss -e                                # socket 的 inode 編號，可與 fd 的 socket:[N] 對上
```

---

## 自我測驗

1. 為什麼說「fd 的編號離開它的行程就沒有意義」？那要把一個已開啟的資源交給另一個行程，
   核心提供了什麼機制？
2. `dup` 出來的兩個 fd 和「對同一個檔案 open 兩次」得到的兩個 fd，差別在哪一層？
   這個差別會造成什麼實際後果？
3. `df` 說磁碟滿了，`du` 卻加不出那麼多。最可能是什麼原因？用一個指令就能找出兇手，是哪個？
4. `EMFILE` 和 `ENFILE` 分別代表什麼？為什麼看到 `too many open files` 不能反射性地調 `ulimit`？
   如果服務是用 systemd 啟動的，還要多注意什麼？
5. 情境題：某服務的 fd 數量只增不減，你 `ls -l /proc/<PID>/fd/` 發現九成是 `socket:[…]`。
   這指向哪一類 bug？你會再用哪一個網路指令交叉驗證、預期看到什麼？
