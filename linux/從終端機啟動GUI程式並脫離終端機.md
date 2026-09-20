# 從終端機開 GUI 程式，關掉終端機程式也跟著死 —— 行程還掛在終端機的 session 上

> 日期：2026-09-19
> 情境：想在終端機打程式名稱（例如 `code`）開 GUI 程式，但終端機會一直印出程式的輸出；而且一關終端機，程式也跟著被關掉
> 適用範圍：Linux 桌面環境，bash / zsh（zsh 是本機預設 shell）

**一句話結論：程式會「跟著終端機死」，是因為關終端機時 shell 會把 SIGHUP 轉發給它的工作（job）；會「一直印東西」，是因為程式的 stdout/stderr 還接在終端機上。解法就是把這兩條線都剪斷——`setsid -f prog >/dev/null 2>&1`，或 zsh 下的 `prog &>/dev/null &!`。**

---

## 一、先看最常用的解法

```bash
# 1. 最推薦：開一個全新 session，完全脫離這個終端機
setsid -f firefox >/dev/null 2>&1

# 2. zsh 專屬簡寫：&! （或 &|）= 背景執行 + 立刻 disown
firefox &>/dev/null &!

# 3. bash / zsh 通用：背景執行後，把它從 job 表移除
firefox &>/dev/null & disown

# 4. 老牌寫法：讓程式忽略 SIGHUP
nohup firefox >/dev/null 2>&1 &

# 5. 用檔案類型 / .desktop 開啟，跟從選單點開幾乎一樣
xdg-open ~/some.pdf            # 用預設程式開某個檔案
gtk-launch code                # 依 .desktop 檔（/usr/share/applications/code.desktop）啟動
```

`>/dev/null 2>&1` 負責「不要再印東西」（語法細節見 [輸出重新導向與dev-null.md](./輸出重新導向與dev-null.md)）；`setsid` / `&!` / `disown` / `nohup` 負責「關終端機不會死」。
兩件事是**分開的**，下面分別解釋。

---

## 二、為什麼關終端機程式會死：SIGHUP 的傳遞鏈

從終端機打 `firefox &`，行程的關係長這樣：

```
終端機模擬器（gnome-terminal 等）
  └─ 虛擬終端（pty, pseudo-terminal；詳見 PTY是什麼-終端機模擬器與虛擬終端.md）
       └─ zsh           ← 這個 session 的 session leader
            └─ firefox  ← zsh 的 job，同一個 session，控制終端機（controlling terminal）是這個 pty
```

關掉終端機視窗時（pty 的 master／slave 結構見 [PTY是什麼-終端機模擬器與虛擬終端.md](./PTY是什麼-終端機模擬器與虛擬終端.md)）：

1. 終端機模擬器關閉 pty 的 master 端。
2. 核心對 session leader（zsh）送出 **SIGHUP（Signal Hang Up，掛斷信號）**——這名稱來自早期撥接數據機「電話掛斷了」。
3. **zsh 收到 SIGHUP 後，會把 SIGHUP 轉送給自己 job 表裡的每一個工作**（zsh 預設開著 `HUP` 選項；bash 則是 `huponexit` 等相關行為加上收到 SIGHUP 時轉發給 job）。
4. 大部分程式對 SIGHUP 的預設處理是「結束行程」→ firefox 死掉。

所以各解法對應剪斷的是鏈上的不同環節：

| 解法 | 剪斷哪一環 | 原理 |
|------|-----------|------|
| `disown` / `&!` | 第 3 步 | 把程式從 shell 的 job 表拿掉，shell 就不會轉發 SIGHUP 給它 |
| `nohup` | 第 4 步 | 把 SIGHUP 設成忽略（ignore），信號送到也沒事 |
| `setsid` | 整條鏈 | 程式變成**新 session 的 leader**，沒有控制終端機，根本不在這條鏈上 |

`setsid -f` 最乾淨：不只躲過 SIGHUP，也不會被終端機的 Ctrl+C（SIGINT 只送給前景行程群組）或其他作業控制（job control）影響。`-f`（fork）確保即使從前景行程群組呼叫，也一定會 fork 出新行程再 `setsid()`。

---

## 三、為什麼終端機會一直印輸出：檔案描述符是繼承來的

子行程會**繼承**父行程的檔案描述符（file descriptor）。從 zsh 啟動的程式，fd 1（stdout）與 fd 2（stderr）都指向同一個 pty，所以程式的 log、警告全部印在你的終端機上。

- `&` 只是讓 shell 不等它結束（不佔用提示字元），**不會**改變 fd——所以 `firefox &` 還是會把輸出噴在畫面上。
- `>/dev/null 2>&1`（zsh / bash 可簡寫 `&>/dev/null`）才是把 fd 1、2 重新導向到黑洞。
- `nohup` 若發現 stdout 是終端機，會自動改寫到 `nohup.out`——這就是為什麼用 `nohup` 常會在目錄裡多一個 `nohup.out`；自己導向 `/dev/null` 就不會產生。

**`setsid` 也不會幫你處理輸出。** 常見的誤解是「開了新 session 就跟終端機完全無關了」。其實 `setsid` 只改變 session 與控制終端機（controlling terminal），**不會動任何已開啟的 fd**。這兩件事互相獨立：

- 控制終端機是 session 層級的屬性，決定誰會收到 SIGHUP、SIGINT 這類終端機信號。
- fd 是行程手上握著的檔案參照，只要沒被關掉或重新導向，就一直指向原本那個 pty。

實測 `setsid -f sleep 30`：`ps` 的 `TT` 欄顯示 `?`（沒有控制終端機），但 `ls -l /proc/<pid>/fd` 裡的 1、2 仍然指向 `/dev/pts/N`。所以只用 `setsid`，程式照樣會在你的終端機上印東西。

**只做 `disown` 或 `setsid`、不導向輸出**的話：終端機關掉之後，程式再寫 stdout 會拿到錯誤（寫入已關閉的 pty 可能收到 SIGPIPE 或 `EIO`），有些程式會因此崩潰。所以兩件事要一起做。

---

## 四、`code` 其實是特例

VS Code 的 `code` 指令（`/usr/bin/code`）是個 shell script，最後呼叫 VS Code 內建的 CLI，而那個 CLI **本來就會把 Electron 主程式以 detached 方式啟動、自己立刻退出**。所以正常情況下 `code .` 會馬上回到提示字元，關終端機也不會關到 VS Code。

如果你看到 `code` 卡住並持續印輸出，通常是以下之一：

- 加了 `--verbose` 或 `--wait`（`-w`）——這兩個參數會讓 CLI 刻意留在前景。
- 你打的其實不是 `/usr/bin/code`（用 `which -a code` / `type code` 確認；例如 alias、snap/flatpak 版本、直接執行 Electron 二進位檔）。
- 你遇到的其實是別的程式（firefox、nautilus、自己寫的 GUI 程式……），它們沒有自己 detach 的機制，就需要上面的解法。

判斷方法：啟動後看提示字元有沒有立刻回來；或用 `ps -o pid,sid,tty,cmd -C code` 看它的 `SID`（session ID）與 `TTY`——`TTY` 是 `?` 就代表它沒有控制終端機，關終端機不會影響它。

### 找出 VS Code 的主行程 PID（Process ID，行程識別碼）

VS Code 是 Electron（Chromium）應用程式（Electron 是什麼見 [Electron是什麼-為什麼VSCode有十幾個行程.md](../electron/Electron是什麼-為什麼VSCode有十幾個行程.md)），採多行程架構（multi-process architecture）：一個主行程（main process），底下有 zygote、GPU、renderer、extension host 等子行程。所以 `pgrep code` 會列出十幾個 PID，要挑的是**主行程**——也就是「父行程不是 `code`」的那一個。

```bash
pgrep -a -x code                  # 列出所有叫 code 的行程（含命令列）
pstree -p $(pgrep -o -x code)     # -o = 最舊的那個，通常就是主行程；pstree 看整棵樹
ps -o pid,ppid,sid,tty,etime,cmd -p $(pgrep -o -x code)
```

實測結果（2026-09-19，本機）：

```
    PID    PPID     SID TT   ELAPSED CMD
  13767    1997   13767 ?      12:54 /usr/share/code/code
```

這一行剛好驗證了本篇前面的說法：

- `PPID 1997` 是 `systemd --user`，不是 zsh——啟動它的 CLI 已經退出，主行程被收養（reparent）了。
- `SID == PID`——它自己就是一個新 session 的 leader，等於 VS Code 自己做了 `setsid`。
- `TT ?`——沒有控制終端機，所以關終端機收不到 SIGHUP。

注意：`pgrep -o` 挑最舊的，只在你沒有同時開別的 `code` 實例（例如不同 `--user-data-dir`）時才準；有疑慮就用 `pstree` 看誰是樹根。

---

## 五、想常用的話：寫成函式

放進 `~/.zshrc`：

```zsh
# 用法：run firefox ~/file.html
run() { setsid -f "$@" >/dev/null 2>&1 }
```

---

## 自我測驗

1. 關掉終端機視窗時，程式被關掉的整條因果鏈是什麼？SIGHUP 是誰送給誰的？
2. `disown`、`nohup`、`setsid` 都能讓程式在關終端機後存活，它們分別切斷了鏈上的哪一環？為什麼說 `setsid` 最徹底？
3. 同事說「我已經用 `firefox &` 放背景了，為什麼終端機還一直跳 log？」——他搞混了哪兩件事？
4. 情境題：某人用 `myapp & disown` 啟動程式，關掉終端機後程式沒有立刻死，但過一陣子在某個時間點突然崩潰。你會先懷疑什麼原因？怎麼改？
5. 有人說「`setsid` 已經讓程式脫離終端機了，所以不用再加 `>/dev/null 2>&1`」。這句話錯在哪？脫離控制終端機跟 fd 指向哪裡為什麼是兩回事？
6. 情境題：你在終端機打 `code .` 後提示字元立刻回來，關掉終端機 VS Code 也沒被關；但同樣打 `firefox` 就會卡住終端機。為什麼兩者行為不同？
