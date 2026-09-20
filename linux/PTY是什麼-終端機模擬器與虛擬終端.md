# PTY 是什麼？終端機視窗、shell、tmux 之間到底怎麼接起來的

> 日期：2026-09-19
> 情境：讀 [從終端機啟動GUI程式並脫離終端機.md](./從終端機啟動GUI程式並脫離終端機.md) 時遇到「pty」一詞而追問
> 適用範圍：Linux（macOS 等 Unix 系統概念相同，裝置路徑不同）；本機環境為 alacritty + tmux + zsh

**一句話結論：PTY（pseudo-terminal，虛擬終端）是核心提供的一對裝置——master 端給「扮演終端機的程式」（alacritty、tmux、sshd）握著，slave 端（`/dev/pts/N`）給 shell 當成 stdin／stdout／stderr。中間的核心負責把 Ctrl+C 變成 SIGINT、處理回顯與行編輯；master 一關，核心就對 slave 那頭送 SIGHUP。**

---

## 一、名字從哪來：TTY → PTY

- **TTY（TeleTYpewriter，電傳打字機）**：早期 Unix 的「終端機」是一台實體的打字機／顯示器，用序列線接到主機。核心裡負責跟它溝通的那層叫 TTY 子系統（tty subsystem）。
- 後來終端機變成桌面上的**程式**（終端機模擬器（terminal emulator））或網路連線（ssh），沒有實體線路了——但 shell 等程式仍然預期自己在跟「一台終端機」說話。
- **PTY（pseudo-terminal，虛擬／偽終端）** 就是核心用軟體模擬出來的那條線路：一頭假裝成終端機硬體，另一頭假裝成接在主機上的序列埠。

所以 PTY 的存在目的是：**讓「用程式扮演的終端機」能沿用整套為實體終端機設計的機制**，shell 完全不需要知道對面是 alacritty 還是真的打字機。

---

## 二、一對裝置：master 與 slave

```
 你按鍵盤
    │
 alacritty（終端機模擬器）
    │  讀寫 master 端（打開 /dev/ptmx 取得）
 ┌──┴───────────────────────────┐
 │ 核心：line discipline（線路規則）  │ ← Ctrl+C→SIGINT、回顯、行編輯、Ctrl+D→EOF
 └──┬───────────────────────────┘
    │  slave 端 = /dev/pts/0
 zsh 的 fd 0／1／2 都指向 /dev/pts/0
```

| 端 | 誰握著 | 在哪 |
|----|-------|------|
| **master** | 終端機模擬器、tmux server、sshd、`script`、`expect` | 打開 `/dev/ptmx` 取得（每打開一次就配出一對新的 PTY） |
| **slave** | shell 與它啟動的程式 | `/dev/pts/N`，N 是編號 |

資料流：
- 你按 `l` `s` Enter → alacritty 寫進 master → 核心處理 → shell 從 slave（fd 0）讀到 `ls\n`。
- `ls` 寫結果到 fd 1（slave）→ 核心 → alacritty 從 master 讀到 → 畫在視窗上。

`tty` 指令會告訴你目前 shell 的 slave 是哪一個（例如 `/dev/pts/0`）；`ps` 的 `TT` 欄位顯示的也是它。

---

## 三、中間那層核心做了什麼：line discipline

PTY 不只是一條管子。核心在中間放了一層 **line discipline（線路規則）**，這就是終端機「有行為」的原因：

| 你按 | 核心做的事 |
|------|-----------|
| Ctrl+C | 不送字元給程式，而是對**前景行程群組**送 SIGINT |
| Ctrl+Z | 送 SIGTSTP（暫停，作業控制用） |
| Ctrl+D | 讓讀取的程式收到 EOF |
| 一般字元 | 回顯（echo）到畫面、在按 Enter 前允許 Backspace 編輯 |

這些都能用 `stty -a` 查看與修改（`intr = ^C`、`susp = ^Z`、`eof = ^D`）。
輸入密碼時字不會顯示，就是程式暫時關掉了回顯（`stty -echo`）；vim、less 這類全螢幕程式會切成 raw mode，自己處理每一個按鍵。

**這也是 PTY 和 pipe 的差別**：pipe 只是單純的位元組管道，沒有信號、沒有回顯。所以很多程式會檢查 stdout 是不是終端機（`isatty()`），是的話才輸出顏色——`ls | cat` 沒有顏色就是這個原因。

---

## 四、跟 SIGHUP 的關係

「控制終端機（controlling terminal）」指的就是一個 session 綁定的那個 PTY slave。

關掉終端機視窗時：alacritty 結束 → master 端被關閉 → 核心對「以這個 PTY 為控制終端機的 session leader」送 SIGHUP → shell 再轉送給自己的 job。這就是 [從終端機啟動GUI程式並脫離終端機.md](./從終端機啟動GUI程式並脫離終端機.md) 那條 SIGHUP 傳遞鏈的起點。

`setsid` 後 `ps` 的 `TT` 顯示 `?`，意思就是「這個 session 沒有綁任何 PTY」，所以 master 關掉時跟它無關。

---

## 五、實例：本機 alacritty + tmux 的 PTY 接線

實測（2026-09-19，`ps -eo pid,tty,comm`、`tmux list-panes -a -F '#{pane_tty}'`、檢查各行程的 `/proc/<pid>/fd` 是否握著 `ptmx`）：

```
alacritty (9032)                 ← 握 1 個 master
  └─ /dev/pts/0
       └─ zsh (9043)
            └─ tmux attach (16899)   ← tmux client，只負責把畫面轉給 alacritty

tmux server (9603, TT = ?)       ← 握 2 個 master，自己沒有控制終端機
  ├─ /dev/pts/1  → zsh (9607) → claude    （session good-note）
  └─ /dev/pts/2  → zsh (16523)             （session workspace）
```

這解釋了**為什麼關掉 alacritty，tmux 裡的東西還活著**：

- alacritty 關閉只會關掉 `pts/0` 的 master → SIGHUP 只送到 `pts/0` 那個 session（zsh 與 tmux client）。
- 你的工作跑在 `pts/1`、`pts/2`，它們的 master 由 tmux server 握著；tmux server 本身 `TT = ?`（已脫離終端機，跟 `setsid` 同一招），所以不受影響。
- 之後 `tmux attach` 只是開一個新的 client，重新把畫面接到新的終端機視窗。

**tmux 其實就是一個「不會因為視窗關掉而消失的終端機模擬器」。** ssh 也是同樣結構：sshd 握 master，遠端 shell 用 slave，網路斷線 = master 關閉 = SIGHUP。

---

## 自我測驗

1. 既然現在已經沒有實體終端機了，為什麼核心還需要 PTY？直接用 pipe 接 shell 不行嗎？缺了什麼？
2. PTY 的 master 和 slave 分別由誰握著？你在 alacritty 裡按 Ctrl+C，信號是誰產生、送給誰的？
3. `ps` 的 `TT` 欄位顯示 `pts/3` 和 `?` 各代表什麼？這跟關閉終端機時會不會收到 SIGHUP 有什麼關係？
4. 情境題：你在 tmux 裡跑一個要跑三小時的編譯，然後把 alacritty 視窗關掉。編譯會不會中斷？用 PTY 的 master 由誰握著來解釋。
5. 情境題：你 ssh 到伺服器、直接（不用 tmux）執行一個長時間的指令，結果網路斷線，指令也跟著死了。為什麼？跟本機關掉終端機視窗是同一件事嗎？
