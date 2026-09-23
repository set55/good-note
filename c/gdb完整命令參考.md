# gdb 怎麼用？—— 啟動選項、命令文法與全部 173 個命令

> 日期：2026-09-22
> 情境：第 ① 站第 1 課的延伸（[gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md) 用到 `gdb`，當時只列了 7 個命令，
> 使用者要求完整列出）。第 3 課「用 `gdb` 找 segfault」會以本篇為參考實際操作
> 適用範圍：本機 GNU gdb 15.1（Ubuntu 24.04）。命令清單取自本機 `gdb -batch -ex 'help all'`，
> 啟動選項取自本機 `gdb --help`，英文原文說明都是 gdb 自己印的，中文為對照翻譯
> 相關：[gcc最佳化等級-O0到O3差在哪.md](./gcc最佳化等級-O0到O3差在哪.md)（`-g` 與 `<optimized out>`）、
> [GNU是什麼-Linux核心與GNU工具的分工.md](../linux/GNU是什麼-Linux核心與GNU工具的分工.md)

> **這是「參考型」筆記，不是拿來背的。** 用法是：需要時打開來查，或在 `gdb` 裡打 `help` 問它自己。
> 要記住的只有一件事：**遇到什麼問題，該找哪一類命令**——這也是文末自我測驗唯一會考的東西
> （測驗不考選項字面，可以翻著這份筆記作答）。

**一句話結論：`gdb` 的命令多但結構簡單——`help` 把它們分成 13 類，本機共 173 個頂層命令；
其中 `info`（65 個子命令）、`set`（456 個）、`show`（463 個）、`maintenance`（197 個）是「前綴命令」，
底下還有子命令。日常除錯真正會用的是 running（怎麼跑）、breakpoints（在哪停）、data（看什麼）、
stack（在哪一層）四類；其他類別是遠端除錯、追蹤點、腳本化、gdb 自身維護用的。
任何時候都可以用 `help <命令>`、`apropos <關鍵字>` 問 gdb 自己。**

---

## 目錄

- [一、gdb 在做什麼，為什麼要 `-g`](#一gdb-在做什麼為什麼要--g)
- [二、啟動 gdb：命令列文法與全部選項](#二啟動-gdb命令列文法與全部選項)
- [三、gdb 內部命令的共通文法](#三gdb-內部命令的共通文法)
- [四、全部 173 個頂層命令（依 gdb 的 13 類）](#四全部-173-個頂層命令依-gdb-的-13-類)
- [五、`info` 的全部 65 個子命令](#五info-的全部-65-個子命令)
- [六、`set`／`show`／`maintenance`：數量、為什麼不逐項列、怎麼查](#六setshowmaintenance數量為什麼不逐項列怎麼查)
- [七、容易混淆的命令：差在哪、什麼時候用哪個](#七容易混淆的命令差在哪什麼時候用哪個)
- [八、實際走一次，與常見錯誤訊息](#八實際走一次與常見錯誤訊息)
- [自我測驗](#自我測驗)

---

## 一、gdb 在做什麼，為什麼要 `-g`

`gdb`（GNU Debugger，GNU 除錯器）讓你：**把程式停在指定的地方、一步一步執行、隨時查看與修改記憶體和變數、
看是誰呼叫了誰**。在 Linux 上它是透過核心的 `ptrace` 系統呼叫控制被除錯的行程（第 ② 站會講行程）。

gdb 看到的只是機器碼和記憶體位址。要把位址對回「原始碼第幾行、哪個變數」，需要編譯時加 **`-g`**，
把**除錯資訊（debug information）**寫進執行檔。`-g` 和最佳化等級 `-O` 是兩回事，
兩者搭配的效果見 [gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md) 第三節。學習與除錯時固定用 `gcc -g -O0`。

gdb 的術語：被除錯的程式叫 **inferior**（下級程式），這個詞在命令說明裡到處出現。

### 常見誤會：`gcc -S -O0 -g` 之後「沒看到 gdb 的功能」（2026-09-22 補充）

`gcc` 和 `gdb` 是**兩個不同的程式**：`gcc` 負責把程式編出來，`-g` 只是讓它**順便把對照表寫進去**；
要除錯，得另外**啟動 `gdb`、把一個能執行的程式交給它**。所以 `gcc -S -O0 -g sumto.c -o sumto.s` 不會出現任何除錯畫面，原因有三層：

1. **`-S` 停在組合語言，產出的 `.s` 不是能執行的程式。** gdb 要的是可執行檔（第 1 課第一節的最後一站）。
2. **`-g` 在 `.s` 裡其實有作用，只是長得不像「功能」。** 本機實測同一個 `sumto.c`：不加 `-g` 的 `.s` 是 69 行，
   加了 `-g` 變成 334 行，多出來的是除錯資訊，例如：
   ```asm
   	.loc 1 3 9              ← 「接下來的指令屬於第 1 個檔案的第 3 行第 9 欄」
   	movl	$0, -8(%rbp)       ← 也就是 int total = 0;
   	.loc 1 4 14
   	movl	$1, -4(%rbp)       ← for 迴圈的 i = 1
   ```
   以及檔案後面的 `.debug_info`、`.debug_line` 等區段。組譯、連結後，這些就變成 gdb 用來「把位址對回行號與變數」的對照表。
3. **只有函式、沒有 `main` 的檔案連結不出執行檔。** 本機實測 `sumto.c`（只有 `sum_to` 和 `answer`）：
   ```
   $ gcc -g -O0 sumto.c -o sumto
   /usr/bin/ld: .../Scrt1.o: in function `_start':
   (.text+0x1b): undefined reference to `main'
   collect2: error: ld returned 1 exit status
   ```
   啟動碼 `_start` 要呼叫 `main`（第 1 課第五節「程式不是從 `main` 開始的」），找不到就連結失敗。

正確的流程：

```
① 寫一個有 main 的程式（或另一個檔案提供 main）
② gcc -g -O0 檔案.c -o 程式          ← 編出「可執行檔」，不要加 -S 或 -c
③ gdb ./程式                          ← 啟動 gdb，把程式交給它
④ (gdb) break sum_to                  ← 在 gdb 的提示符號後面下命令
   (gdb) run
   (gdb) next / print / info locals …
```

本機用一個另外提供 `main`（呼叫 `answer()` 並印出結果）的檔案實測，`gdb` 裡看到的：

```
Breakpoint 1, sum_to (n=100) at sumto.c:3
3	    int total = 0;
(gdb) bt
#0  sum_to (n=100) at sumto.c:3
#1  0x000055555555518d in answer () at sumto.c:11
#2  0x000055555555519c in main () at demo_main.c:3
(gdb) next  →  next  →  next
(gdb) info locals
i = 1
total = 1
(gdb) finish
Value returned is $1 = 5050
```

`finish` 讓 `sum_to` 跑完並印出回傳值 5050——這是 `-O0` 下實際一圈一圈跑出來的，
跟 [gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md) 裡 `-O2` 在編譯時就算好的 5050 是同一個答案、完全不同的過程。

---

## 二、啟動 gdb：命令列文法與全部選項

```
gdb [選項] [執行檔 [core 檔 或 行程 ID]]
gdb [選項] --args 執行檔 [傳給程式的參數 ...]
```

| 位置 | 意思 | 省略時 |
|---|---|---|
| `執行檔` | 要除錯的程式 | 進入 gdb 後再用 `file` 指定 |
| `core 檔` | 程式崩潰時留下的記憶體傾印，用來事後檢查 | 不載入 |
| `行程 ID` | 附加到一個**正在執行**的行程 | 不附加 |
| `--args` | 之後的**所有**字都當成「執行檔 ＋ 它的參數」，而不是 gdb 的選項 | 程式參數要進 gdb 後用 `run 參數` 或 `set args` 給 |

選項依本機 `gdb --help` 的分組，**全部列出**：

**選擇要除錯的對象與檔案**

| 選項 | 作用 |
|---|---|
| `--args` | 執行檔後面的參數都傳給被除錯的程式 |
| `--core=COREFILE` | 分析 core 傾印檔 |
| `--exec=EXECFILE` | 指定執行檔 |
| `--pid=PID` | 附加到執行中的行程 PID |
| `--directory=DIR` | 在 DIR 找原始碼 |
| `--se=FILE` | FILE 同時當符號檔與執行檔 |
| `--symbols=SYMFILE` | 從 SYMFILE 讀符號（除錯資訊放在另一個檔案時） |
| `--readnow` | 一開始就把符號檔全部讀完（啟動慢、之後快） |
| `--readnever` | 完全不讀符號檔 |
| `--write` | 允許寫入執行檔與 core 檔（可以直接修改檔案裡的機器碼） |

**啟動時執行的命令與命令檔**

| 選項 | 作用 |
|---|---|
| `--command=FILE`、`-x FILE` | 執行 FILE 裡的 gdb 命令 |
| `--init-command=FILE`、`-ix FILE` | 同 `-x`，但在載入程式**之前**執行 |
| `--eval-command=COMMAND`、`-ex COMMAND` | 執行一個 gdb 命令；可重複多次，也可和 `-x` 並用 |
| `--init-eval-command=COMMAND`、`-iex COMMAND` | 同 `-ex`，但在載入程式之前執行 |
| `--nh` | 不讀 `~/.gdbinit` |
| `--nx` | 不讀任何目錄的 `.gdbinit` |

**輸出與介面**

| 選項 | 作用 |
|---|---|
| `--fullname` | 輸出給 Emacs 整合用的資訊 |
| `--interpreter=INTERP` | 指定直譯器／介面（例如 `mi`：給 IDE 用的機器介面） |
| `--tty=TTY` | 被除錯的程式用 TTY 做輸入輸出（程式輸出很多時，跟 gdb 的畫面分開） |
| `-w` | 使用 GUI 介面 |
| `--nw` | 不使用 GUI 介面 |
| `--tui` | 使用終端機文字介面（TUI，Text User Interface）：畫面上方顯示原始碼 |
| `-q`、`--quiet`、`--silent` | 啟動時不印版本資訊 |

**執行模式**

| 選項 | 作用 |
|---|---|
| `--batch` | 處理完選項（`-ex`、`-x`）就結束，不進互動模式。本筆記的實驗都是這樣跑的 |
| `--batch-silent` | 同 `--batch`，但不印 gdb 自己的輸出 |
| `--return-child-result` | gdb 的結束狀態碼改用被除錯程式的結束狀態碼 |
| `--configuration` | 印出 gdb 的編譯組態後結束 |
| `--help` | 印出說明後結束 |
| `--version` | 印出版本後結束 |

**遠端除錯**

| 選項 | 作用 |
|---|---|
| `-b BAUDRATE` | 遠端除錯用的序列埠鮑率 |
| `-l TIMEOUT` | 遠端除錯的逾時秒數 |

**其他**

| 選項 | 作用 |
|---|---|
| `--cd=DIR` | 先切到 DIR 目錄 |
| `--data-directory=DIR`、`-D DIR` | 指定 gdb 的資料目錄 |

本機 gdb 啟動時會讀系統層級的 `/etc/gdb/gdbinit`（`--help` 最後幾行列出）。

---

## 三、gdb 內部命令的共通文法

```
(gdb) <命令> [子命令] [參數...]
```

### 縮寫與別名

- **任何命令都可以縮寫成「不會跟別的命令混淆」的開頭**（`help` 的原文：*Command name abbreviations are allowed if unambiguous*）。
- 常用命令另外有**正式定義的別名**，即使有歧義也能用：`b` = `break`、`c` = `continue`、`n` = `next`、`s` = `step`、
  `p` = `print`、`bt` = `backtrace`、`i` = `info`。第四節表格的「命令」欄把每個命令的全部別名都列出來了。
- **直接按 Enter 會重複上一個命令**（例如連續 `next`）。少數命令用 `dont-repeat` 關掉了這個行為。

### 位置（LOCATION）：`break`、`list`、`until`、`advance` 等共用

依 `help break`，位置有三種寫法：

| 寫法 | 例子 | 意思 |
|---|---|---|
| 行號規格（linespec） | `42`、`avg.c:42`、`average`、`avg.c:average` | 行號、檔名:行號、函式名、檔名:函式名（檔名用來區分同名的 `static` 函式） |
| 位址 | `*0x401136`、`*main + 4` | 以 `*` 開頭，精確的機器碼位址 |
| 明確位置（explicit） | `-source avg.c -function average -line 8` | 用選項形式寫出每個部分 |

`break` 不給位置時，用「目前選取的堆疊框正在執行的位址」。

### 表達式、值歷史、便利變數

- `print`、`display`、`watch`、`if` 條件裡寫的是**被除錯程式所用語言的表達式**——C 程式就用 C 語法：
  `print a[1] + n`、`print *ptr`、`print &sum`、`print sizeof(int)`。
- 每次 `print` 的結果會存成 `$1`、`$2`……（值歷史），之後可以直接拿來用：`print $1 * 2`；`$` 是上一個、`$$` 是上上個。
- `$` 開頭的自訂名字是**便利變數（convenience variable）**：`set $i = 0`。暫存器也用 `$` 表示：`$rip`、`$rsp`、`$rax`。

### 輸出格式 `/FMT`：`print` 與 `x` 共用

`print/FMT 表達式`、`x/FMT 位址`。依 `help x`，FMT 是「重複次數＋格式字母＋大小字母」：

| 格式字母 | 意思 | 大小字母（只有 `x` 用） | 意思 |
|---|---|---|---|
| `o` | 八進位 | `b` | 1 byte |
| `x` | 十六進位 | `h` | 2 bytes（halfword） |
| `d` | 有號十進位 | `w` | 4 bytes（word） |
| `u` | 無號十進位 | `g` | 8 bytes（giant） |
| `t` | 二進位 | | |
| `f` | 浮點數 | | |
| `a` | 位址（並顯示屬於哪個符號） | | |
| `i` | 機器指令（反組譯） | | |
| `c` | 字元 | | |
| `s` | 字串 | | |
| `z` | 十六進位，左邊補 0 | | |

例：`x/4dw data` = 從 `data` 開始，以有號十進位印 4 個 4-byte 的值；`x/3i $rip` = 印出接下來 3 條指令；
`print/x n` = 用十六進位印 `n`。省略時沿用上一次的格式與大小，次數預設 1。

### 問 gdb 自己

| 命令 | 作用 |
|---|---|
| `help` | 列出 13 個類別 |
| `help <類別>` | 列出該類別的所有命令，例如 `help running` |
| `help <命令>` | 單一命令的完整說明與文法，例如 `help break` |
| `help all` | 列出**全部**命令與子命令（本機 1617 行） |
| `apropos <正規表示式>` | 搜尋說明裡提到關鍵字的命令，例如 `apropos watchpoint` |
| `apropos -v <正規表示式>` | 同上，印出完整說明 |
| `complete <開頭>` | 列出能補完的命令，例如 `complete info b` |
| Tab 鍵 | 互動模式下補完命令、檔名、符號名 |

---

## 四、全部 173 個頂層命令（依 gdb 的 13 類）

以下依 `help all` 的分類與順序**全部列出**。「命令」欄是 gdb 定義的名稱與所有別名；說明是 gdb 原文的翻譯。
每一類的標題註明頂層命令數。類別裡**第一次學就該會的**用 ★ 標出。

### 4.1 running：控制程式怎麼跑（32 個）

| 命令 | 作用 |
|---|---|
| `advance` | 讓程式繼續跑到指定位置（位置寫法同 `break`） |
| `attach` | 附加到 gdb 之外的一個行程或檔案 |
| ★ `continue`、`fg`、`c` | 在訊號或中斷點停下後繼續執行 |
| `detach` | 脫離先前 `attach` 的行程（行程繼續自己跑） |
| `disconnect` | 中斷與目標（target）的連線 |
| ★ `finish`、`fin` | 執行到目前這個函式回傳為止，並印出回傳值 |
| `handle` | 指定收到各種訊號（signal）時 gdb 要怎麼處理（停不停、印不印、傳不傳給程式） |
| `inferior` | 在多個被除錯程式之間切換 |
| `interrupt` | 中斷正在執行的被除錯程式 |
| `jump`、`j` | 跳到指定的行或位址繼續執行（跳過中間的程式碼，危險） |
| `kill` | 結束被除錯的程式 |
| ★ `next`、`n` | 執行一行原始碼；遇到函式呼叫**不進去**，整個呼叫當一步 |
| `nexti`、`ni` | 執行一條機器指令；遇到 `call` 不進去 |
| `queue-signal` | 排一個訊號，在目前執行緒恢復執行時送給它 |
| `reverse-continue`、`rc` | **倒著**繼續執行（需先 `record`） |
| `reverse-finish` | 倒著執行到目前函式被呼叫之前 |
| `reverse-next`、`rn` | 倒著走一行，不進入函式 |
| `reverse-nexti`、`rni` | 倒著走一條指令，不進入函式 |
| `reverse-step`、`rs` | 倒著走到另一行原始碼的開頭 |
| `reverse-stepi`、`rsi` | 倒著走剛好一條指令 |
| ★ `run`、`r` | 開始執行被除錯的程式；後面可以接程式的參數，也能用 `<`、`>` 重新導向 |
| `signal` | 帶著指定的訊號繼續執行程式 |
| ★ `start` | 開始執行，並在 `main` 的開頭停下來（等於在 `main` 設暫時中斷點再 `run`） |
| `starti` | 開始執行，停在**第一條指令**（比 `main` 更早，在啟動碼裡——第 1 課說的「程式不是從 `main` 開始」） |
| ★ `step`、`s` | 執行到下一行原始碼；遇到函式呼叫**會進去** |
| `stepi`、`si` | 剛好執行一條機器指令（會進入 `call`） |
| `taas` | 對所有執行緒執行一個命令（忽略錯誤與空輸出） |
| `target` | 連接到目標機器或行程（遠端除錯、core 檔等） |
| `task` | 在 Ada 語言的 task 之間切換 |
| `tfaas` | 對所有執行緒的所有堆疊框執行一個命令 |
| `thread`、`t` | 在執行緒之間切換 |
| ★ `until`、`u` | 執行到超過目前這一行（常用來跑完一個迴圈），或跑到指定位置 |

### 4.2 breakpoints：讓程式在哪裡停（24 個）

| 命令 | 作用 |
|---|---|
| `awatch` | 設「存取」監看點：運算式被讀或被寫時停下 |
| ★ `break`、`brea`、`bre`、`br`、`b` | 在指定位置設中斷點；完整文法：`break [PROBE_MODIFIER] [LOCATION] [thread N] [-force-condition] [if 條件]` |
| `break-range` | 對一段位址範圍設中斷點 |
| `catch` | 設 catchpoint：捕捉事件（例如 `catch syscall write`、`catch fork`、`catch throw`） |
| `clear`、`cl` | 刪除**指定位置**上的中斷點 |
| `commands` | 設定某個中斷點被觸發時要自動執行的 gdb 命令 |
| `condition` | 讓第 N 號中斷點只在條件成立時才停 |
| ★ `delete`、`del`、`d` | 刪除**指定編號**的中斷點（不給編號就全刪） |
| `disable`、`disa`、`dis` | 停用中斷點（保留，但不生效） |
| `dprintf` | 動態 printf：程式經過這裡時印出訊息但**不停下來**，不用改程式碼 |
| `enable`、`en` | 啟用被停用的中斷點 |
| `ftrace` | 設快速追蹤點（fast tracepoint，遠端目標用） |
| `hbreak` | 設硬體輔助中斷點（用 CPU 的除錯暫存器，數量有限；在唯讀記憶體等處也能用） |
| `ignore` | 讓第 N 號中斷點忽略接下來 COUNT 次觸發 |
| `rbreak` | 對名字符合正規表示式的**所有函式**設中斷點 |
| `rwatch` | 設「讀取」監看點：運算式被讀時停下 |
| `save` | 把中斷點定義存成腳本，之後用 `source` 載回 |
| `skip` | 單步時略過某個函式（不要 `step` 進去） |
| `strace` | 設靜態追蹤點（static tracepoint）。**跟 Linux 的 `strace` 工具無關** |
| `tbreak` | 設暫時中斷點：停一次後自動刪除 |
| `tcatch` | 設暫時 catchpoint |
| `thbreak` | 設暫時的硬體輔助中斷點 |
| `trace`、`trac`、`tra`、`tr`、`tp` | 設追蹤點（tracepoint）：記錄資料但不停下，遠端目標用 |
| ★ `watch` | 設監看點（watchpoint）：運算式的值**改變**時停下 |

### 4.3 data：查看與修改資料（22 個）

| 命令 | 作用 |
|---|---|
| `agent-printf` | 只給目標端代理（agent）用的格式化輸出 |
| `append` | 把目標程式的程式碼／資料附加到本機檔案 |
| `call` | 呼叫被除錯程式裡的一個函式並印出回傳值 |
| ★ `disassemble` | 反組譯一段記憶體（不給參數就是目前的函式）；`/s` 同時顯示原始碼，`/r` 顯示機器碼位元組 |
| ★ `display` | 每次程式停下來都自動印出某個運算式 |
| `dump` | 把目標程式的程式碼／資料傾印到本機檔案 |
| `explore` | 互動式地逐層探索一個值或型別 |
| `find` | 在記憶體裡搜尋一串位元組 |
| `init-if-undefined` | 便利變數還沒定義時才初始化它 |
| `mem` | 定義記憶體區域的屬性 |
| `memory-tag` | 查看與操作記憶體標籤（memory tagging，特定 CPU 功能） |
| `output` | 像 `print`，但不存進值歷史、不換行（寫腳本用） |
| ★ `print`、`inspect`、`p` | 印出運算式的值；`print/FMT` 指定格式 |
| `print-object`、`po` | 請 Objective-C 物件印出自己 |
| `printf` | 像 C 的 `printf` 一樣格式化輸出 |
| ★ `ptype` | 印出型別的**完整定義**（例如 struct 的所有欄位） |
| `restore` | 把檔案內容寫回目標程式的記憶體 |
| `set` | 計算運算式並指定給變數；**也是**設定 gdb 選項的前綴命令（見第六節） |
| `undisplay` | 取消 `display` |
| `whatis` | 印出運算式的**型別名稱** |
| `with`、`w` | 暫時把某個設定改成某值、執行一個命令、再改回來 |
| ★ `x` | 直接檢查記憶體：`x/FMT 位址`（格式見第三節） |

### 4.4 stack：呼叫堆疊（7 個）

| 命令 | 作用 |
|---|---|
| ★ `backtrace`、`where`、`bt` | 印出所有堆疊框（誰呼叫了誰），或最內層的 COUNT 個 |
| `down`、`dow`、`do` | 選取並印出「被目前這層呼叫」的那一層（往內） |
| `faas` | 對所有堆疊框執行一個命令 |
| ★ `frame`、`f` | 選取並印出一個堆疊框（`frame 1` 切到呼叫者那層，才能看它的區域變數） |
| `return` | 讓目前選取的堆疊框**立刻**回傳給呼叫者（跳過函式剩下的程式碼） |
| `select-frame` | 選取堆疊框但不印出任何東西 |
| `up` | 選取並印出「呼叫了目前這層」的那一層（往外） |

### 4.5 files：指定與查看檔案（21 個）

| 命令 | 作用 |
|---|---|
| `add-symbol-file` | 從檔案載入符號（假設該檔已被動態載入） |
| `add-symbol-file-from-memory` | 從記憶體裡一個動態載入的目的檔讀取符號 |
| `cd` | 改變 gdb 的工作目錄 |
| `core-file` | 載入 core 傾印檔來檢查記憶體與暫存器 |
| `directory` | 把目錄加到原始碼搜尋路徑的最前面 |
| `edit` | 用編輯器開啟指定的檔案或函式 |
| `exec-file` | 指定執行檔（只用來讀取它的內容） |
| `file` | 指定要除錯的程式（同時讀符號） |
| `forward-search`、`fo`、`search` | 從上次 `list` 的位置往後用正規表示式搜尋原始碼 |
| `generate-core-file`、`gcore` | 把目前行程的狀態存成 core 檔 |
| ★ `list`、`l` | 列出原始碼：`list 函式`、`list 行號`、`list 起,迄`；`list +`／`list -` 往後／往前，`list .` 目前執行點附近（預設 10 行） |
| `load` | 把檔案動態載入到執行中的程式（遠端目標用） |
| `nosharedlibrary` | 卸載所有共享函式庫的符號 |
| `path` | 把目錄加到目的檔搜尋路徑的最前面 |
| `pwd` | 印出 gdb 的工作目錄 |
| `remote` | 操作遠端系統上的檔案 |
| `remove-symbol-file` | 移除用 `add-symbol-file` 加入的符號檔 |
| `reverse-search`、`rev` | 從上次 `list` 的位置往前搜尋原始碼 |
| `section` | 改變執行檔某個區段的基底位址 |
| `sharedlibrary` | 載入名字符合正規表示式的共享函式庫的符號 |
| `symbol-file` | 從執行檔載入符號表 |

### 4.6 status：狀態查詢（3 個）

| 命令 | 作用 |
|---|---|
| ★ `info`、`inf`、`i` | 顯示**被除錯程式**的各種資訊；本身是前綴，65 個子命令見第五節 |
| `macro` | 處理 C 前處理器巨集的前綴命令（需要 `-g3` 編譯才有巨集資訊） |
| `show`、`info set` | 顯示 **gdb 自己**的設定；本身是前綴，見第六節 |

### 4.7 support：輔助功能（23 個）

| 命令 | 作用 |
|---|---|
| `add-auto-load-safe-path` | 把目錄加入「可以安全自動載入檔案」的清單 |
| `add-auto-load-scripts-directory` | 把目錄加入自動載入腳本的目錄清單 |
| `alias` | 替既有命令定義新別名 |
| `apropos` | 搜尋符合正規表示式的命令 |
| `define` | 定義新的使用者命令（一串 gdb 命令組成） |
| `define-prefix` | 定義或標記一個使用者前綴命令 |
| `demangle` | 還原 C++ 等語言被編碼過的符號名稱 |
| `document` | 替使用者命令或別名寫說明 |
| `dont-repeat` | 讓命令在按 Enter 時不重複（用在使用者命令裡） |
| `down-silently` | 同 `down`，但不印任何東西 |
| `echo` | 印出一段固定字串 |
| ★ `help`、`h` | 列出命令與說明 |
| `if` | 條件成立時執行內層命令（腳本用） |
| `interpreter-exec` | 用指定的直譯器執行命令 |
| `make` | 在 gdb 裡執行 `make`，後面的字都當成 `make` 的參數 |
| `new-ui` | 建立新的使用者介面（例如在另一個終端機再開一個 gdb 介面） |
| `overlay`、`ov`、`ovly` | 除錯 overlay（嵌入式系統的程式碼覆蓋技術） |
| `pipe`、`\|` | 把 gdb 命令的輸出交給 shell 命令處理，例如 `\| info functions \| grep avg` |
| ★ `quit`、`exit`、`q` | 離開 gdb |
| `shell`、`!` | 執行一行 shell 命令 |
| `source` | 從檔案讀入並執行 gdb 命令 |
| `up-silently` | 同 `up`，但不印任何東西 |
| `while` | 條件成立時重複執行內層命令（腳本用） |

### 4.8 text-user-interface：文字介面（6 個）

| 命令 | 作用 |
|---|---|
| `+` | 視窗往下捲 |
| `-` | 視窗往上捲 |
| `<` | 視窗內容往左捲 |
| `>` | 視窗內容往右捲 |
| `tui` | TUI 相關命令的前綴（例如 `tui enable`、`tui layout src`、`tui layout asm`） |
| `update` | 把原始碼視窗更新到目前執行點 |

### 4.9 tracepoints：不停下程式的追蹤（13 個）

追蹤點主要用在**遠端目標**（例如透過 `gdbserver` 除錯的嵌入式裝置），在本機一般行程上多半不支援。

| 命令 | 作用 |
|---|---|
| `actions` | 指定追蹤點觸發時要做的動作 |
| `collect` | 指定追蹤點要收集的資料 |
| `end` | 結束一串命令或動作（`commands`、`define`、`if`、`while` 等也用它收尾） |
| `passcount` | 設定追蹤點觸發幾次後停止收集 |
| `tdump` | 印出目前追蹤框收集到的所有資料 |
| `teval` | 指定追蹤點要計算的運算式 |
| `tfind` | 選取一個追蹤框來檢查 |
| `tsave` | 把追蹤資料存檔 |
| `tstart` | 開始收集追蹤資料 |
| `tstatus` | 顯示追蹤收集的狀態 |
| `tstop` | 停止收集追蹤資料 |
| `tvariable` | 定義追蹤狀態變數 |
| `while-stepping`、`stepping`、`ws` | 指定追蹤點觸發後單步多少次並收集 |

### 4.10 obscure：少用的功能（12 個）

| 命令 | 作用 |
|---|---|
| `checkpoint` | 複製一份目前的行程當檢查點（實驗性） |
| `compare-sections` | 比較目標上的區段資料與執行檔是否一致 |
| `compile`、`expression` | 編譯一段原始碼並注入被除錯程式執行 |
| `complete` | 列出一行命令可能的補完 |
| `guile`、`gu` | 執行 Guile（Scheme）運算式 |
| `guile-repl`、`gr` | 開啟 Guile 互動提示 |
| `monitor` | 送命令給遠端監控程式（只限遠端目標） |
| `python`、`py` | 執行 Python 命令（gdb 內建 Python，可寫擴充） |
| `python-interactive`、`pi` | 開啟 Python 互動提示 |
| `record`、`rec` | 開始記錄執行過程——`reverse-*` 倒著執行的前提 |
| `restart` | 從檢查點恢復程式狀態 |
| `stop` | 沒有 `stop` 這個命令；它是一個可以掛 hook 的名字（程式停下時觸發） |

### 4.11 user-defined：使用者定義與 Python 擴充提供的命令（9 個）

| 命令 | 作用 |
|---|---|
| `add-inferior` | 新增一個被除錯程式 |
| `clone-inferior` | 複製一個被除錯程式 |
| `eval` | 組出一個 gdb 命令字串再執行它 |
| `flash-erase` | 抹除所有 flash 記憶體區域（嵌入式） |
| `function` | 用來查看便利函式說明的佔位命令（`help function`） |
| `jit-reader-load` | 載入 JIT 編譯程式碼的除錯資訊讀取器 |
| `jit-reader-unload` | 卸載 JIT 讀取器 |
| `remove-inferiors` | 移除被除錯程式 |
| `unset` | 某些 `set` 命令的反向（例如 `unset environment`） |

### 4.12 internals：gdb 自身維護（1 個）

| 命令 | 作用 |
|---|---|
| `maintenance`、`mt` | 給 gdb 維護者用的命令前綴（197 個子命令，見第六節） |

### 4.13 aliases：使用者自訂別名（0 個）

`alias` 命令定義出來的別名會列在這一類；本機沒有定義任何別名，所以是空的。

合計：32 + 24 + 22 + 7 + 21 + 3 + 23 + 6 + 13 + 12 + 9 + 1 + 0 = **173**。

---

## 五、`info` 的全部 65 個子命令

`info` 顯示**被除錯程式**的狀態（對照：`show` 顯示 gdb 自己的設定）。依本機 `help all` **全部列出**，★ 為常用：

| 子命令 | 作用 |
|---|---|
| `info address` | 描述某個符號存放在哪裡 |
| `info all-registers` | 所有暫存器（含浮點、向量）與內容 |
| ★ `info args` | 目前堆疊框的所有參數 |
| `info auto-load` | 自動載入檔案的狀態 |
| `info auxv` | 被除錯程式的輔助向量（auxiliary vector，核心在 `execve` 時交給程式的資訊） |
| `info bookmarks` | 書籤狀態（搭配 `record`） |
| ★ `info breakpoints`、`info b` | 所有中斷點／監看點／catchpoint 的狀態與編號 |
| `info checkpoints` | 已知的檢查點 |
| `info classes` | Objective-C 類別 |
| `info common` | Fortran COMMON 區塊的內容 |
| `info connections` | 使用中的目標連線 |
| `info copying` | gdb 的再散布條款（GPL） |
| `info dcache` | 資料快取的效能資訊 |
| `info display` | 所有 `display` 的運算式與編號 |
| `info exceptions` | Ada 例外名稱 |
| `info extensions` | 副檔名與語言的對應 |
| `info files` | 目標與正在除錯的檔案、各區段的位址範圍 |
| `info float` | 浮點運算單元的狀態 |
| ★ `info frame`、`info f` | 目前堆疊框的詳細資訊（回傳位址、呼叫者、暫存器存放位置） |
| `info frame-filter` | 已註冊的 Python frame filter |
| ★ `info functions` | 所有函式名稱（可加正規表示式過濾） |
| `info guile`、`info gu` | Guile 相關資訊的前綴 |
| `info inferiors` | 目前管理的所有被除錯程式 |
| ★ `info line` | 某一行原始碼對應的機器碼位址 |
| ★ `info locals` | 目前堆疊框的所有區域變數 |
| `info macro` | 某個巨集的定義與位置（需 `-g3`） |
| `info macros` | 某處所有巨集的定義（需 `-g3`） |
| `info main` | 程式的進入點符號 |
| `info mem` | 記憶體區域屬性 |
| `info missing-debug-handlers` | 缺少除錯資訊時的處理器 |
| `info module` | 模組資訊（Fortran） |
| `info modules` | 所有模組名稱（Fortran） |
| `info os` | 作業系統資料（例如 `info os processes`） |
| `info pretty-printer` | 已註冊的 pretty-printer |
| `info probes` | 可用的靜態探針 |
| ★ `info proc` | 行程的額外資訊（`info proc mappings` 看記憶體配置——第 ③ 站會用） |
| `info program` | 程式的執行狀態（為什麼停下） |
| `info record`、`info rec` | 記錄功能的狀態 |
| ★ `info registers`、`info r` | 整數暫存器與內容（`info registers rip` 只看一個） |
| `info scope` | 某個範圍內的區域變數 |
| `info selectors` | Objective-C selector |
| ★ `info sharedlibrary`、`info dll` | 已載入的共享函式庫（第 1 課的 `libc.so.6`、`ld-linux-x86-64.so.2` 會出現在這裡） |
| `info signals`、`info handle` | 每種訊號 gdb 會怎麼處理 |
| `info skip` | `skip` 設定的狀態 |
| `info source` | 目前原始碼檔的資訊（包含有沒有除錯資訊、編譯選項） |
| `info sources` | 程式所有原始碼檔 |
| `info stack`、`info s` | 堆疊回溯（同 `backtrace`） |
| `info static-tracepoint-markers` | 靜態追蹤點標記 |
| `info symbol` | 某個位址上是什麼符號 |
| `info target` | 目標與檔案（同 `info files`） |
| `info tasks` | Ada task |
| `info terminal` | 被除錯程式保存的終端機狀態 |
| ★ `info threads` | 目前所有執行緒（第 ④ 站會用） |
| `info tracepoints`、`info tp` | 追蹤點狀態 |
| `info tvariables` | 追蹤狀態變數 |
| `info type-printers` | 已註冊的 type printer |
| `info types` | 所有型別名稱 |
| `info unwinder` | 堆疊展開器（unwinder） |
| `info variables` | 所有全域與 `static` 變數 |
| `info vector` | 向量運算單元的狀態 |
| `info vtbl` | C++ 物件的虛擬函式表 |
| `info warranty` | gdb 的免責聲明 |
| `info watchpoints` | 監看點狀態 |
| `info win` | TUI 顯示中的視窗 |
| `info xmethod` | 已註冊的 xmethod |

其中幾個還有更下一層的子命令（`info frame` 4 個、`info proc` 8 個、`info auto-load` 4 個、`info probes` 3 個、
`info module` 2 個），用 `help info proc` 這樣查。

---

## 六、`set`／`show`／`maintenance`：數量、為什麼不逐項列、怎麼查

| 前綴 | 本機子命令數 | 是什麼 |
|---|---|---|
| `set` | 456 | 改 gdb 的設定（也用來改程式變數：`set var x = 3`） |
| `show` | 463 | 看 gdb 的設定（跟 `set` 大致一一對應） |
| `maintenance` | 197 | gdb 開發者用的內部命令 |

**這三組沒有逐項列出**：它們是 gdb 的「設定項目清單」而不是操作命令（`set` 與 `show` 幾乎兩兩成對，
實際約 450 個設定），大部分是特定語言、遠端協定、gdb 內部的選項；`maintenance` 是給修改 gdb 本身的人用的。
完整清單的查法：

```
(gdb) help set            ← 列出所有 set 子命令與一行說明
(gdb) help show
(gdb) show                ← 不給參數：印出「目前所有設定的值」
(gdb) apropos listsize    ← 用關鍵字找設定
(gdb) help maintenance
```

本系列課程會用到的設定（**節錄，不是全部**）：

| 設定 | 作用 |
|---|---|
| `set var 變數 = 值` | 修改被除錯程式的變數（寫成 `set var` 可避免跟 gdb 設定名稱撞名） |
| `set args 參數...`／`show args` | 設定 `run` 時傳給程式的參數 |
| `set listsize N` | `list` 一次列幾行 |
| `set disassembly-flavor intel` | 反組譯改用 Intel 語法（預設 AT&T 語法，也就是第 1 課 `.s` 檔裡的寫法） |
| `set print pretty on` | struct 分行縮排印出 |
| `set pagination off` | 輸出很長時不要停下來問「要不要繼續」 |
| `set logging enabled on` | 把 gdb 輸出記錄到檔案（預設 `gdb.txt`） |
| `set debuginfod enabled off` | 不要詢問是否從網路下載系統函式庫的除錯資訊（本機每次啟動都會問） |

---

## 七、容易混淆的命令：差在哪、什麼時候用哪個

**單步執行**

| 命令 | 單位 | 遇到函式呼叫 | 什麼時候用 |
|---|---|---|---|
| `next` | 一行原始碼 | 不進去 | 相信被呼叫的函式沒問題，只看這一層 |
| `step` | 一行原始碼 | 進去 | 懷疑問題在被呼叫的函式裡 |
| `nexti`／`stepi` | 一條機器指令 | 不進去／進去 | 看組合語言層級發生什麼事（最佳化過的程式常需要） |
| `finish` | 到目前函式回傳 | — | 不小心 `step` 進了不想看的函式，快速跳出 |
| `until` | 跑到超過目前這一行 | — | 在迴圈結尾處用，一次跑完整個迴圈 |
| `advance 位置` | 跑到指定位置 | — | 等於「設暫時中斷點再 `continue`」 |
| `continue` | 跑到下一個中斷點 | — | 中間沒有要看的東西 |

#### 實驗：用 `avg.c` 看 `next` 與 `step` 的差別（2026-09-23 補充）

> 起因：使用者在 `average()` 裡面交替使用 `next` 和 `step`，結果完全一樣，看不出差別。
> `avg.c` 的原始碼在 [gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md) 第三節；以下都用 `gcc -g -O0` 編譯。

**差別只會出現在「這一行有呼叫函式」的時候。** `average()` 的每一行（`int sum = 0;`、`for (...)`、`sum += a[i];`）
都沒有呼叫任何函式，所以 `next` 和 `step` 走出來的結果一模一樣：

```
Breakpoint 1, average (a=0x7fffffffd2fc, n=3) at avg.c:5
5	    int sum = 0;
(gdb) next
6	    for (int i = 0; i < n; i++)
(gdb) step                               ← 換成 step，結果還是一樣
7	        sum += a[i];
```

要看出差別，**中斷點要設在有函式呼叫的那一行**，也就是 `main` 的第 15 行 `printf("%d\n", average(a, 3));`：

```
(gdb) break 15                           ← 同一個起點，分兩次試
(gdb) run
15	    printf("%d\n", average(a, 3));

(gdb) next                               ← 第一次：next
6                                        ← average 和 printf 都執行完了（印出 6）
16	    return 0;                        ← 直接到下一行，整行當成一步

(gdb) step                               ← 第二次：從第 15 行重來，改用 step
average (a=0x7fffffffd2fc, n=3) at avg.c:5
5	    int sum = 0;                     ← 進到 average 裡面了
```

這一行其實有**兩個**函式呼叫。C 要先算出 `average(a, 3)` 的結果，才能把它交給 `printf`，
所以 `step` 先進的是 `average`。接下來：

```
(gdb) finish                             ← 跑完目前這個函式（average），回到呼叫它的地方
Run till exit from #0  average (a=0x7fffffffd2fc, n=3) at avg.c:5
0x00005555555551fd in main () at avg.c:15
15	    printf("%d\n", average(a, 3));    ← 回到第 15 行，但這一行還沒做完（printf 還沒呼叫）
Value returned is $1 = 6                 ← finish 會印出回傳值

(gdb) step                               ← 再 step 一次：這次進的是 printf
__printf (format=0x555555556004 "%d\n") at ./stdio-common/printf.c:32
32	./stdio-common/printf.c: No such file or directory
```

最後一步出乎意料：`step` 連 glibc 的 `printf` 都進去了。原因是本機裝了 **`libc6-dbg`**
（glibc 的除錯資訊套件，`dpkg -l libc6-dbg` 顯示 `ii`），所以 gdb 知道 `printf` 對應 `printf.c` 的第幾行。
但原始碼檔案本身不在本機，所以出現 `No such file or directory`。這是正常現象，不是錯誤。
這正是自我測驗第 1 題說的「不小心 `step` 進了不想看的函式」，這時用 `finish` 回到上一層。

**`step` 什麼時候會進去、什麼時候會跳過**，由 gdb 的設定 `step-mode` 決定（`help set step-mode` 實測）：

> When set, doing a step over a function without debug line information will stop at the first instruction of that function.
> Otherwise, the function is skipped and the step command stops at a different source line.

- 預設是 `off`（`show step-mode` → `Mode of the step operation is off.`）：函式**沒有行號資訊**時，`step` 會直接跳過，效果等於 `next`。
- 所以在沒有裝 `libc6-dbg` 的機器上，`step` 不會進 `printf`（本機有裝，這個情況**未實測**）。
- 函式有行號資訊的條件：它是用 `-g` 編譯的（例如自己寫的 `average`），或者系統裝了它的除錯資訊套件（例如 `libc6-dbg`）。

一句話：**`next` 與 `step` 只在「目前這一行有呼叫函式，而且那個函式有除錯資訊」時才會不一樣。**


**開始執行**

| 命令 | 停在哪 |
|---|---|
| `run` | 不停，直到碰到中斷點、訊號或程式結束 |
| `start` | `main` 的第一行 |
| `starti` | 程式的第一條指令（`main` 之前的啟動碼） |

**中斷點家族**

| 命令 | 差別 |
|---|---|
| `break` | 一般中斷點，一直有效 |
| `tbreak` | 暫時的，停一次就自動刪除 |
| `hbreak`／`thbreak` | 用 CPU 硬體除錯暫存器實作（數量有限，通常只有 4 個） |
| `rbreak 正規表示式` | 一次對很多函式設中斷點 |
| `dprintf` | 不停下、只印訊息——不改程式碼的 `printf` 除錯 |
| `watch`／`rwatch`／`awatch` | 看**資料**不看位置：值被改／被讀／被讀或寫時停下 |
| `catch` | 看**事件**：系統呼叫、`fork`、`exec`、C++ 例外 |

#### 實驗：`watch` 在 `-O0` 與 `-O2` 下抓全域變數（2026-09-23 補充）

> 起因：自我測驗第 5 題問「變數在 `-O2` 下只放在暫存器時，`watch` 還可靠嗎」，但本篇原本沒有講 `watch` 怎麼運作，這是出題缺陷。
> 這裡用實驗補上。檔案 `g.c` 只在實驗用的暫存目錄，沒有放進 repo，原始碼全文如下。

```c
#include <stdio.h>

int counter;                               /* 全域變數 */

__attribute__((noinline)) void bump(int n) /* 把 counter 加 1，加 n 次 */
{
    for (int i = 0; i < n; i++)
        counter += 1;
}

__attribute__((noinline)) void oops(void)  /* 「兇手」：偷偷把 counter 改掉 */
{
    counter = 42;
}

int main(void)
{
    bump(5);
    oops();
    printf("%d\n", counter);
    return 0;
}
```

操作：`break main` → `run` → `watch counter` → 一直 `continue`。兩種編法的結果：

```
===== gcc -g -O0                              ===== gcc -g -O2
Hardware watchpoint 2: counter                Hardware watchpoint 2: counter
Old value = 0 / New value = 1                 Old value = 0 / New value = 5    ← 只停一次
7	    for (int i = 0; ...                    bump (n=n@entry=5) at g.c:9
Old value = 1 / New value = 2                 9	}
（……共 5 次，每加一次停一次）                  Old value = 5 / New value = 42
Old value = 5 / New value = 42                oops () at g.c:14
14	}                                          14	}
```

讀法：

1. **`Hardware watchpoint`：`watch` 盯的是「記憶體位址」。** gdb 請 CPU 的除錯暫存器監看 `counter` 所在的那幾個 byte，
   有指令**寫入**那個位址時，CPU 就讓程式停下來。所以它不管是哪一行 C 寫的，只要記憶體真的被寫入就抓得到。
2. **全域變數一定有記憶體位址，`-O2` 也一樣。** 別的函式、別的檔案隨時可能讀它，所以編譯器最後一定要把值寫回記憶體。
   `-O2` 下兇手 `oops` 照樣被抓到（`Old value = 5 / New value = 42 ... oops ()`）。**用 `watch` 找「誰改了全域變數」，在 `-O2` 下仍然可靠。**
3. **但中間的變化可能被合併。** `-O0` 停了 5 次（每加一次停一次）；`-O2` 只停 1 次（0 直接變成 5）。
   `-O2` 版的 `bump` 被改寫成下面這樣（`objdump -d` 實測，每行右邊是中文說明；組合語言排在第 4、5 課，這裡只需要看說明）：

   ```
   test   %edi,%edi                       ← 檢查 n 是不是 ≤ 0
   jle    119e <bump+0xe>                 ← 是的話直接跳到 ret，什麼都不做
   add    %edi,0x2e76(%rip)  # <counter>  ← 把 n 一次加到 counter 的記憶體上（整個迴圈變成這一條）
   ret                                    ← 返回
   ```

   迴圈被最佳化成「一次加 n」，記憶體只被寫入一次，所以 `watch` 也只停一次。
4. **停下時顯示的行號是「寫完之後」的位置。** CPU 是在寫入的指令**執行完**才讓程式停下，
   所以 gdb 顯示的是**下一個**位置：`-O0` 停在 `for` 那一行（`counter += 1` 的下一步）、`oops` 停在 `}`。
   找兇手時，要看**顯示那一行的前一行**，或用 `bt` 確認在哪個函式。`-O2` 下行號更不精確，但**函式名稱仍然正確**。
5. **什麼時候 `watch` 真的不可靠：只活在暫存器裡的區域變數。** 硬體監看點只能監看記憶體位址；
   一個在 `-O2` 下只放在暫存器的區域變數，沒有位址可以監看。這個情況**未實測**，排在第 3 課用 `gdb` 找 bug 時再做。


**刪除與停用**

| 命令 | 用什麼指定 | 效果 |
|---|---|---|
| `delete N` | 中斷點**編號**（看 `info breakpoints`） | 刪除；不給編號就全刪 |
| `clear 位置` | **位置** | 刪除那個位置上的中斷點 |
| `disable N`／`enable N` | 編號 | 暫時停用／恢復，保留設定 |

**看資料**

| 命令 | 差別 |
|---|---|
| `print` | 印一次，結果存成 `$N` |
| `display` | 每次停下都自動印 |
| `output` | 印一次，不存、不換行（腳本用） |
| `x` | 直接看**記憶體位元組**，不管變數型別 |
| `ptype` | 印出型別的完整定義 |
| `whatis` | 只印型別名稱 |

**`info` 與 `show`**：`info` 看被除錯**程式**的狀態，`show` 看 **gdb 自己**的設定。

**`up`／`down`／`frame`**：只換「現在在看哪一層」，程式本身完全沒動。

---

## 八、實際走一次，與常見錯誤訊息

用 [gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md) 第三節的 `avg.c`（`gcc -g -O0 avg.c -o avg_O0`），本機實測：

```
$ gdb -q ./avg_O0
(gdb) break average
Breakpoint 1 at 0x1178: file avg.c, line 5.
(gdb) run
Breakpoint 1, average (a=0x7fffffffd1cc, n=3) at avg.c:5
5	    int sum = 0;
(gdb) bt
#0  average (a=0x7fffffffd1cc, n=3) at avg.c:5
#1  0x00005555555551fd in main () at avg.c:15
(gdb) next
6	    for (int i = 0; i < n; i++)
(gdb) next
7	        sum += a[i];
(gdb) next
6	    for (int i = 0; i < n; i++)
(gdb) info locals
i = 0
sum = 3
avg = 0
(gdb) continue
6                                  ← 程式印出的結果
```

用 `-batch` 可以把同樣的步驟寫在命令列一次跑完（本筆記的實驗都這樣做）：

```sh
gdb -q -batch -ex 'break average' -ex run -ex bt -ex next -ex 'info locals' ./avg_O0
```

**錯誤訊息對照**（都是本機實際出現過的）：

| 訊息 | 意思 | 怎麼辦 |
|---|---|---|
| `No symbol "sum" in current context.` | 目前這層沒有這個名字——可能沒加 `-g`、停在別的函式、或變數被最佳化掉 | 確認 `-g`；用 `bt`、`frame` 確認在哪一層 |
| `The program is not being run.` | 程式還沒 `run` 或已經結束，卻下了 `next`、`continue` 之類的命令 | 先 `run` 或 `start` |
| `No stack.` | 程式沒在跑，沒有堆疊可看（`bt`、`frame`、`info locals` 會出現） | 同上 |
| `<optimized out>` | 變數存在於原始碼，但此刻沒有可以讀的位置 | 見 [gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md) 第三節 |
| 中斷點設了卻從來沒停 | 函式被內嵌、或整段在編譯時被算掉了 | 用 `-O0` 重編；或看 `disassemble` 確認函式還在不在 |
| `Breakpoint 1, 0x... in average ()`（沒有行號與參數） | 沒有除錯資訊（沒加 `-g`） | 加 `-g` 重編 |

---

## 自我測驗

（**可以翻筆記作答**：本篇是參考型筆記。題目考的是「差別」與「該用哪個」，不考選項字面。）

1. `next` 和 `step` 差在哪？如果不小心 `step` 進了一個不想看的函式（例如 `printf`），用哪個命令最快回到原本那一層？為什麼不是 `continue`？
2. `info` 和 `show` 都是「顯示」，差別是什麼？`set` 為什麼同時出現在 data 類和第六節？
3. `break`、`watch`、`catch` 三者分別讓程式在「什麼條件」停下？各舉一個它比另外兩個更適合的情境。
4. 為什麼 gdb 裡的 `print` 可以寫 `a[1] + n`、`*ptr` 這種 C 語法？如果程式沒有用 `-g` 編譯，`print sum` 會發生什麼事？
5. 情境題：你懷疑某個全域變數在程式某處被意外改掉了，但不知道是哪一行改的。你會用本篇的哪個命令找出兇手？如果那個變數在 `-O2` 下被最佳化成只放在暫存器，這個方法還可靠嗎？
