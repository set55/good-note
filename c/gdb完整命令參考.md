# gdb 怎麼用？—— 啟動選項、命令文法與全部 173 個命令

> 課程：[Linux 與系統程式系統化學習](../learning/課程目錄.md#linux-與系統程式系統化學習)｜第 ① 站第 1 課・延伸（參考型）
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

**怎麼查**：知道要做什麼（停在某處、一步步走、看變數、看堆疊、看組合語言）→ 先看[第零節〈依任務查〉](#零依任務查常用命令各一個例子)，每個常用命令都有實際輸出；要找某個命令的全名、縮寫、還有哪些相關命令 → 第四節（依 gdb 的 13 類）與第五節（`info` 的子命令）的表。

---

## 目錄

- [零、依任務查：常用命令各一個例子](#零依任務查常用命令各一個例子)
- [一、gdb 在做什麼，為什麼要 `-g`](#一gdb-在做什麼為什麼要--g)
- [二、啟動 gdb：命令列文法與全部選項](#二啟動-gdb命令列文法與全部選項)
- [三、gdb 內部命令的共通文法](#三gdb-內部命令的共通文法)
- [四、全部 173 個頂層命令（依 gdb 的 13 類）](#四全部-173-個頂層命令依-gdb-的-13-類)
- [五、`info` 的全部 65 個子命令](#五info-的全部-65-個子命令)
- [六、`set`／`show`／`maintenance`：數量、為什麼不逐項列、怎麼查](#六setshowmaintenance數量為什麼不逐項列怎麼查)
- [七、容易混淆的命令：差在哪、什麼時候用哪個](#七容易混淆的命令差在哪什麼時候用哪個)
- [八、實際走一次，與常見錯誤訊息](#八實際走一次與常見錯誤訊息)
- [九、課後討論](#九課後討論)
- [自我測驗](#自我測驗)

---

## 零、依任務查：常用命令各一個例子

> 2026-11-04 新增：使用者反映「寫的內容讓人不知道怎麼查與用」。本節照「想做什麼」分組，每個常用命令（第四、五節標 ★ 的）各一個實際跑過的例子：粗體寫結論、一個命令與它的輸出、下面緊接說明（`CLAUDE.md` 筆記規範 6-4）。涵蓋第四節標 ★ 的 21 個頂層命令（全部 173 個裡的 21 個）加上常用的 `set var`，以及第五節標 ★ 的 10 個 `info` 子命令（全部 65 個裡的 10 個）；其他命令看第四、五節的表。

### 範例程式與怎麼開始

本節所有例子都用這支 `tour.c`（檔案：[`tour.c`](../small-project/s1-01-hello-compile/examples/gdbtour/tour.c)）。行號在下面的例子裡會用到，所以把每一行的編號一起列出：

```c
/* tour.c：gdb 筆記〈依任務查〉用的範例程式 */          /* 第 1 行 */
#include <stdio.h>

struct point {
    int x;
    int y;
};

int counter = 0;                          /* 全域變數：watch 的例子用 */   /* 第 9 行 */

int square(int n)                                                      /* 第 11 行 */
{
    int r = n * n;                                                     /* 第 13 行 */
    return r;                                                          /* 第 14 行 */
}

int sum_squares(int *a, int len)                                       /* 第 17 行 */
{
    int total = 0;
    for (int i = 0; i < len; i++) {                                    /* 第 20 行 */
        total += square(a[i]);                                         /* 第 21 行 */
        counter++;                                                     /* 第 22 行 */
    }
    return total;                                                      /* 第 24 行 */
}

int main(void)                                                         /* 第 27 行 */
{
    int nums[4] = {1, 2, 3, 4};
    struct point p = {3, 4};
    int s = sum_squares(nums, 4);                                      /* 第 31 行 */
    printf("sum=%d p=(%d,%d) counter=%d\n", s, p.x, p.y, counter);
    return 0;
}
```

（檔案裡的 `tour.c` 沒有右邊這些「第 N 行」註解，行號跟這裡一致。）

```text
$ gcc -g -O0 tour.c -o tour      # -g 才有行號與變數名；-O0 讓每一行都照原樣存在
$ gdb -q ./tour                  # -q 不印版權說明；進去之後提示符號是 (gdb)
```

- 下面每個例子都是在 `(gdb)` 提示符號後面打的命令。例子的開頭會寫「先做什麼」把程式停到那個狀態，你照著打就能重現。
- 輸出裡的位址（`0x5555…`、`0x7fff…`）每台機器、每次編譯可能不同，比對時看結構就好。
- `run`、`start` 時印的 `[Thread debugging using libthread_db enabled]` 兩行是 gdb 載入執行緒除錯支援的訊息，每次都有，可以忽略。

### 1. 開始與結束

#### `run`（`r`）：從頭執行

```text
(gdb) run
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
sum=30 p=(3,4) counter=4
[Inferior 1 (process 18269) exited normally]
```

- 沒有設中斷點，程式就一路跑完：`sum=30 …` 是程式自己印的，最後一行是 gdb 報告「行程正常結束」。
- 要給程式參數或重新導向，直接接在後面：`run a.txt b.txt`、`run < input.txt`。

#### `start`：執行並停在 `main` 開頭

```text
(gdb) start
Temporary breakpoint 1 at 0x11f1: file tour.c, line 28.
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".

Temporary breakpoint 1, main () at tour.c:28
28	{
```

- 等於「在 `main` 設一個用一次就消失的暫時中斷點（`Temporary breakpoint`），再 `run`」。不知道要停在哪時，先 `start` 再一步步走。
- 停在第 28 行（`main` 的 `{`）而不是第 29 行：`{` 這一行對應建立堆疊框的指令。

#### `help`（`h`）：問 gdb 某個命令怎麼用

```text
(gdb) help next
next, n
Step program, proceeding through subroutine calls.
Usage: next [N]
Unlike "step", if the current source line calls a subroutine,
this command does not enter the subroutine, but instead steps over
the call, in effect treating it as a single source line.
```

- 第一行列出全名與縮寫（`next, n`），`Usage:` 是文法。想找「跟某件事有關的命令」用 `apropos 關鍵字`。

#### `quit`（`q`）：離開

- 打 `quit` 就離開；程式還在跑時，gdb 會問 `A debugging session is active … Quit anyway? (y or n)`，答 `y` 會連程式一起結束。

### 2. 讓程式停在想看的地方

#### `break`（`b`）：在函式設中斷點

```text
(gdb) break square
Breakpoint 1 at 0x1174: file tour.c, line 13.
```

- 編號 1、位址 `0x1174`、對應 `tour.c` 第 13 行：gdb 會自動跳過函式開頭建立堆疊框的幾條指令，停在第一行真正的程式碼。
- 還沒 `run`，位址是檔案裡的位址（`0x1174`）；程式跑起來後會加上載入基底（見下面的 `info registers`）。

#### `break 檔案:行號`：在某一行設中斷點

```text
(gdb) break tour.c:22
Breakpoint 1 at 0x11c5: file tour.c, line 22.
```

- 停在第 22 行 `counter++;` **執行之前**。只有一個原始碼檔時可以省略檔名寫 `break 22`。

#### `break … if 條件`：條件中斷點

先 `break square if n == 3`，再：

```text
(gdb) run
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".

Breakpoint 1, square (n=3) at tour.c:13
13	    int r = n * n;
```

- 前兩次呼叫 `square`（`n` 是 1、2）沒停，第三次 `n=3` 才停。迴圈跑幾萬次、只想看某一次時用它。

#### `info breakpoints`（`info b`）：列出所有中斷點

先 `break square`、`break tour.c:22 if i == 2`，再：

```text
(gdb) info breakpoints
Num     Type           Disp Enb Address            What
1       breakpoint     keep y   0x0000000000001174 in square at tour.c:13
2       breakpoint     keep y   0x00000000000011c5 in sum_squares at tour.c:22
	stop only if i == 2
```

- `Num` 是編號（`delete`、`disable` 用它）；`Disp` 的 `keep` 表示停過還留著（`tbreak` 是 `del`）；`Enb` 的 `y` 表示啟用中。
- 有條件的中斷點，下面多一行 `stop only if i == 2`。

#### `delete`（`d`）：刪掉中斷點

先 `break square`、`break tour.c:22`，再：

```text
(gdb) delete 1
```

```text
(gdb) info breakpoints
Num     Type           Disp Enb Address            What
2       breakpoint     keep y   0x00000000000011c5 in sum_squares at tour.c:22
```

- `delete 1` 刪掉編號 1（`square` 那個），剩下編號 2。不給編號的 `delete` 會**全部**刪掉（gdb 會先問）。

#### `watch`：變數的值改變時停下

先 `start` 停在 `main`，再：

```text
(gdb) watch counter
Hardware watchpoint 2: counter
```

```text
(gdb) continue
Hardware watchpoint 2: counter

Old value = 0
New value = 1
sum_squares (a=0x7fffffffd490, len=4) at tour.c:20
20	    for (int i = 0; i < len; i++) {
```

- `watch counter` 設監看點（watchpoint）；`Hardware` 表示用 CPU 的除錯暫存器做，不會拖慢程式。
- `continue` 之後，`counter` 從 0 變成 1 的那一刻停下，印出舊值、新值。停的位置是**改完之後**的下一行（第 20 行），不是 `counter++` 那行。
- 用途：「這個變數到底是誰改壞的」，不必猜要在哪裡下中斷點。

#### `continue`（`c`）：繼續跑到下一個中斷點

先 `break square`、`run`（停在 `n=1`），再：

```text
(gdb) continue
Breakpoint 1, square (n=2) at tour.c:13
13	    int r = n * n;
```

- 程式繼續跑，下一次進 `square` 又停，這次 `n=2`。

### 3. 一步一步走

#### `next`（`n`）：走一行，遇到函式呼叫不進去

先 `break tour.c:21`、`run`（停在 `total += square(a[i]);`），再：

```text
(gdb) next
22	        counter++;
```

- 第 21 行呼叫了 `square`，`next` 把整個呼叫當一步，直接到第 22 行。

#### `step`（`s`）：走一行，遇到函式呼叫會進去

跟上一個例子同樣停在第 21 行，改打：

```text
(gdb) step
square (n=1) at tour.c:13
13	    int r = n * n;
```

- 這次進到 `square` 裡面，停在它的第 13 行，還印出參數 `n=1`。
- 同一行、`next` 與 `step` 只差在要不要進函式。

#### `finish`：跑完目前這個函式，印出回傳值

先 `break square`、`run`（停在 `square` 裡），再：

```text
(gdb) finish
0x00005555555551c2 in sum_squares (a=0x7fffffffd490, len=4) at tour.c:21
21	        total += square(a[i]);
Value returned is $1 = 1
```

- `square` 跑完，回到呼叫它的第 21 行（還沒把結果加進 `total`），`Value returned is $1 = 1` 是回傳值。不小心 `step` 進了不想看的函式，用 `finish` 出來。

#### `until`（`u`）：跑完這個迴圈

先 `break tour.c:22`、`run`、`delete`（刪掉中斷點，不然會一直被它攔下），再：

```text
(gdb) next
20	    for (int i = 0; i < len; i++) {
```

```text
(gdb) until
24	    return total;
```

- `next` 從第 22 行走回迴圈開頭的第 20 行。
- 在第 20 行打 `until`：一直跑到「比目前這行更後面的行」才停，所以剩下的三圈（包括裡面的 `square`）一次跑完，停在迴圈後面的第 24 行。用 `next` 的話要按十幾次。

### 4. 看資料、改資料

#### `print`（`p`）：印出變數或運算式

先 `break tour.c:21`、`run`、`continue`（停在第二圈，`i` 是 1），再：

```text
(gdb) print i
$1 = 1
```

```text
(gdb) print total
$2 = 1
```

```text
(gdb) print a[i]
$3 = 2
```

- `print` 可以印任何 C 運算式：變數、`a[i]` 都行。每個結果編號成 `$1`、`$2`……之後可以用 `$1` 引用。

```text
(gdb) print *a@len
$4 = {1, 2, 3, 4}
```

- `*a@len`：`a` 只是一個指標，gdb 不知道它指向幾個元素；`@len` 告訴它「從 `*a` 開始連續印 `len` 個」，就印出整個陣列。

```text
(gdb) print/x 255
$5 = 0xff
```

- `print/x` 用十六進位印；`/FMT` 的全部格式字母見第三節〈輸出格式 `/FMT`〉。

再換一個停點：先 `break tour.c:31`、`run`（停在 `main` 裡），再：

```text
(gdb) print p
$1 = {x = 3, y = 4}
```

```text
(gdb) print p.x + p.y
$2 = 7
```

```text
(gdb) print nums
$3 = {1, 2, 3, 4}
```

```text
(gdb) print &nums[1]
$4 = (int *) 0x7fffffffd494
```

- 結構（`struct`）印出每個欄位；真正的陣列（`nums`）直接印出全部元素，不用 `@`。
- `&nums[1]` 印出位址與型別 `(int *)`：`nums` 從 `0x…440` 開始，第 1 格在 `0x…444`，差 4 byte。

#### `set var`：改變數的值

先 `break square`、`run`（停在 `n=1`），再：

```text
(gdb) set var n = 10
```

```text
(gdb) finish
0x00005555555551c2 in sum_squares (a=0x7fffffffd490, len=4) at tour.c:21
21	        total += square(a[i]);
Value returned is $1 = 100
```

- `set var n = 10` 把參數改成 10，再 `finish`，回傳值變成 100。用來測「如果這個值是別的，程式會怎樣」，不必改程式重編。（`set var` 不在 ★ 裡，但很常用；`print n = 10` 也能改。）

#### `display`：每次停下都自動印

先 `break square`、`run`，再：

```text
(gdb) display n
```

```text
(gdb) continue
Breakpoint 1, square (n=2) at tour.c:13
13	    int r = n * n;
1: n = 2
```

- `display n` 之後，每次程式停下都會自動多印一行 `1: n = …`（`1:` 是 display 的編號，`undisplay 1` 取消）。一邊 `continue`、一邊盯著某個變數時用。

#### `ptype`：看型別的完整定義

先 `break tour.c:31`、`run`，再：

```text
(gdb) ptype p
type = struct point {
    int x;
    int y;
}
```

```text
(gdb) ptype sum_squares
type = int (int *, int)
```

- 結構印出所有欄位；函式印出原型（傳回 `int`、參數是 `int *` 與 `int`）。看到一個不熟的變數，先 `ptype` 它。

#### `x`：直接看記憶體

先 `break tour.c:31`、`run`，再：

```text
(gdb) x/4dw nums
0x7fffffffd490:	1	2	3	4
```

- `x/4dw nums`：從 `nums` 的位址開始，印 4 個（`4`）十進位（`d`）的 4 byte 單位（`w`，word）。左邊是起始位址。

```text
(gdb) x/8xb &p
0x7fffffffd488:	0x03	0x00	0x00	0x00	0x04	0x00	0x00	0x00
```

- `x/8xb &p`：印 8 個十六進位（`x`）的 byte（`b`）。`p.x = 3` 存成 `03 00 00 00`、`p.y = 4` 存成 `04 00 00 00`：小端序，低位 byte 在前（[第 2 課第三節〈位元組順序（endianness）〉](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#三位元組順序endianness)）。`x` 的大小字母（`b`／`h`／`w`／`g`）見第三節〈輸出格式 `/FMT`〉。

#### `info locals`：目前這層的所有區域變數

先 `break tour.c:21`、`run`，再：

```text
(gdb) info locals
i = 0
total = 0
```

- 一次印出 `sum_squares` 的所有區域變數，不必一個個 `print`。

#### `info args`：目前這層的所有參數

同樣停在第 21 行：

```text
(gdb) info args
a = 0x7fffffffd490
len = 4
```

- `a` 是指標，印出的是位址；要看內容用 `print *a@len`。

### 5. 看堆疊：現在在哪一層、誰呼叫誰

#### `backtrace`（`bt`、`where`）：印出呼叫鏈

先 `break square`、`run`，再：

```text
(gdb) backtrace
#0  square (n=1) at tour.c:13
#1  0x00005555555551c2 in sum_squares (a=0x7fffffffd490, len=4) at tour.c:21
#2  0x000055555555523b in main () at tour.c:31
```

- 從最內層往外讀：`#0` 現在停在 `square`（`n=1`），`#1` 是 `sum_squares` 在第 21 行呼叫了它，`#2` 是 `main` 在第 31 行呼叫了 `sum_squares`。程式 segfault 時第一件事就是 `bt`（[第 3 課第六節〈用 gdb 找 segfault：實際走一次〉](./陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#六用-gdb-找-segfault實際走一次)）。

#### `frame N`（`f`）：切到某一層

同樣停在 `square` 裡：

```text
(gdb) frame 1
#1  0x00005555555551c2 in sum_squares (a=0x7fffffffd490, len=4) at tour.c:21
21	        total += square(a[i]);
```

```text
(gdb) info locals
i = 0
total = 0
```

- `frame 1` 切到呼叫者 `sum_squares` 那一層；之後的 `info locals`、`print` 都是看**這一層**的變數，所以看得到 `i`、`total`（在 `square` 那層看不到）。`up`／`down` 是往外／往內切一層。

#### `info frame`：這一層的詳細資訊

同樣停在 `square` 裡：

```text
(gdb) info frame
Stack level 0, frame at 0x7fffffffd450:
 rip = 0x555555555174 in square (tour.c:13); saved rip = 0x5555555551c2
 called by frame at 0x7fffffffd480
 source language c.
 Arglist at 0x7fffffffd440, args: n=1
 Locals at 0x7fffffffd440, Previous frame's sp is 0x7fffffffd450
 Saved registers:
  rbp at 0x7fffffffd440, rip at 0x7fffffffd448
```

- `saved rip` 是返回位址（`square` 結束後要回到 `sum_squares` 的哪裡，跟 `bt` 的 `#1` 那個位址一樣）；`Saved registers` 列出舊 `%rbp`、返回位址存在堆疊的哪裡（[第 5 課第四節〈堆疊框〉](./x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#四堆疊框pushq-rbp-movq-rsp-rbp-與-leave)）。

### 6. 看原始碼、組合語言、暫存器

#### `list`（`l`）：印出原始碼

（不必先 `run`）

```text
(gdb) list square
7	};
8	
9	int counter = 0;                          /* 全域變數：watch 的例子用 */
10	
11	int square(int n)
12	{
13	    int r = n * n;
14	    return r;
15	}
16
```

- `list 函式名` 印出那個函式附近的 10 行。

```text
(gdb) list 20,24
20	    for (int i = 0; i < len; i++) {
21	        total += square(a[i]);
22	        counter++;
23	    }
24	    return total;
```

- `list 起,迄` 印指定範圍。設中斷點前先 `list` 查行號。

#### `disassemble`：反組譯目前的函式

先 `break square`、`run`，再：

```text
(gdb) disassemble
Dump of assembler code for function square:
   0x0000555555555169 <+0>:	endbr64
   0x000055555555516d <+4>:	push   %rbp
   0x000055555555516e <+5>:	mov    %rsp,%rbp
   0x0000555555555171 <+8>:	mov    %edi,-0x14(%rbp)
=> 0x0000555555555174 <+11>:	mov    -0x14(%rbp),%eax
   0x0000555555555177 <+14>:	imul   %eax,%eax
   0x000055555555517a <+17>:	mov    %eax,-0x4(%rbp)
   0x000055555555517d <+20>:	mov    -0x4(%rbp),%eax
   0x0000555555555180 <+23>:	pop    %rbp
   0x0000555555555181 <+24>:	ret
End of assembler dump.
```

- 印出 `square` 的組合語言，`=>` 指著**下一條要執行的指令**。`<+11>` 是離函式開頭幾個 byte。
- 對照[第 4 課第六節〈逐行讀 `sum_to` 的 `-O0` 版〉](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#六逐行讀-sum_to-的--o0-版)：`mov %edi,-0x14(%rbp)` 把參數 `n` 存到堆疊，`imul %eax,%eax` 就是 `n * n`。加 `/s` 會把原始碼穿插在中間。

#### `info line`：某一行原始碼對應哪些機器碼

（不必先 `run`）

```text
(gdb) info line 21
Line 21 of "tour.c" starts at address 0x11a5 <sum_squares+35> and ends at 0x11b9 <sum_squares+55>.
```

- 第 21 行的機器碼從 `sum_squares+35` 到 `+55`。想用 `break *位址` 在某一行中間停、或對照 `disassemble` 時用。

#### `info registers`（`info r`）：看暫存器

先 `break square`、`run`，再：

```text
(gdb) info registers rip rdi
rip            0x555555555174      0x555555555174 <square+11>
rdi            0x1                 1
```

- `rip` 是下一條要執行的指令位址（`square+11`，跟 `disassemble` 的 `=>` 同一條）；`rdi` 是 1：呼叫慣例的第一個參數 `n`（[第 5 課第五節〈參數與回傳值放哪裡〉](./x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#五參數與回傳值放哪裡)）。
- 這時位址是 `0x555555555174`：程式載入後，檔案位址 `0x1174` 加上了基底 `0x555555554000`。不給暫存器名稱就印出全部。

### 7. 看程式與行程的資訊

#### `info functions`：找函式

（不必先 `run`）

```text
(gdb) info functions square
All functions matching regular expression "square":

File tour.c:
11:	int square(int);
17:	int sum_squares(int *, int);
```

- 參數是正規表示式：含 `square` 的函式有兩個，連行號和原型一起列出。不給參數會列出全部（包括 libc 的，很長）。

#### `info sharedlibrary`：載入了哪些 `.so`

先 `start`，再：

```text
(gdb) info sharedlibrary
From                To                  Syms Read   Shared Object Library
0x00007ffff7fc6000  0x00007ffff7ff0195  Yes         /lib64/ld-linux-x86-64.so.2
0x00007ffff7c28800  0x00007ffff7db0039  Yes         /lib/x86_64-linux-gnu/libc.so.6
```

- 動態載入器 `ld-linux-x86-64.so.2` 與 `libc.so.6`，以及它們在這個行程裡的位址範圍（[第 1 課第十節〈三個名字很像的檔案〉](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#十三個名字很像的檔案ldld-linux-x86-64so2libcso6)、[第 7 課第五節〈執行時怎麼找到 `.so`〉](./靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.md#五執行時怎麼找到-so)）。

#### `info proc`：行程的基本資訊

先 `start`，再：

```text
(gdb) info proc
process 18651
cmdline = '/home/set/work/Projects/github/good-note/small-project/s1-01-hello-compile/examples/gdbtour/tour'
cwd = '/home/set/work/Projects/github/good-note/small-project/s1-01-hello-compile/examples/gdbtour'
exe = '/home/set/work/Projects/github/good-note/small-project/s1-01-hello-compile/examples/gdbtour/tour'
```

- 行程編號（PID）、命令列、目前工作目錄、執行檔路徑。`info proc mappings` 印出整個記憶體配置（[第 ③ 站](../learning/Linux與系統程式的系統化學習路線.md#二主線六站)會用）。

#### `info threads`：所有執行緒

先 `start`，再：

```text
(gdb) info threads
  Id   Target Id                                Frame 
* 1    Thread 0x7ffff7f9e740 (LWP 18664) "tour" main () at tour.c:28
```

- 這支程式只有一個執行緒；`*` 標的是目前選中的那個，`LWP` 後面是它在核心裡的編號。多執行緒程式（[第 ④ 站](../learning/Linux與系統程式的系統化學習路線.md#二主線六站)）用 `thread N` 切換。

#### `info`：前綴命令

- `info` 本身只是一個前綴，後面接子命令才有作用；上面的 `info breakpoints`、`info locals`、`info frame` 都是它的子命令。全部 65 個見第五節。

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

1. **`-S` 停在組合語言，產出的 `.s` 不是能執行的程式。** gdb 要的是可執行檔（[第 1 課第一節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#一四個階段總覽)的最後一站）。
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
   啟動碼 `_start` 要呼叫 `main`（[第 1 課第五節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#五連結把洞接上但-puts-還是沒接完)「程式不是從 `main` 開始執行的」），找不到就連結失敗。

正確的流程：

```
① 寫一個有 main 的程式（或另一個檔案提供 main）
② gcc -g -O0 檔案.c -o 程式          ← 編出「可執行檔」，不要加 -S 或 -c
③ gdb ./程式                          ← 啟動 gdb，把程式交給它
④ (gdb) break sum_to                  ← 在 gdb 的提示符號後面下命令
   (gdb) run
   (gdb) next / print / info locals …
```

本機用一個另外提供 `main`（呼叫 `answer()` 並印出結果）的檔案實測。`sumto.c` 的原始碼就是
[gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md) 第二節那一段（`sum_to` 在第 1～7 行、`answer` 在第 9～12 行）；
`demo_main.c` 完整原始碼如下（當初沒有保存，這是重建版，重新實測的輸出與下面逐字相同）：

```c
#include <stdio.h>
int answer(void);
int main(void) { printf("%d\n", answer()); return 0; }
```

第 2 行是**宣告**：告訴編譯器 `answer` 在別的檔案裡，回傳 `int`（[第 1 課第九節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#第-2-題刪掉-include-為什麼只是警告程式還能跑部分教過關鍵一句沒明講)〈第 2 題：刪掉 `#include` 為什麼只是警告〉：沒有宣告，編譯器就只能用猜的）。

```
$ gcc -g -O0 sumto.c demo_main.c -o sumto      ← 兩個 .c 一起編，連結成一個執行檔
$ gdb -q ./sumto
(gdb) break sum_to
(gdb) run
```

`gdb` 裡看到的：

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

### 一步一步編譯時，`-g` 要加在哪個階段？（2026-10-05 使用者提問）

> 情境：使用者照第 1 課的四個階段分開編 `sumto0`：`gcc -S -O0 sumto.c -o sumto0.s` → `gcc -c sumto0.s -o sumto0.o` → `gcc sumto0.o -o sumto0`，
> 問 `-g` 應該加在哪一步。

**答案：加在「C 變成組合語言」那一步（`-S`），也就是唯一還看得到 `.c` 的那一步。** 除錯資訊的核心是「哪個位址對應**原始檔**第幾行、哪個變數」，
只有讀 `.c` 的編譯器 `cc1` 知道這件事；它把對照表寫成 `.loc` 指示詞和 `.debug_*` 區段放進 `.s`，後面的 `as`、`ld` 只是照著組譯、合併。

用的程式是第 4 課的 `sumto.c`（有 `main`，第 18 行是 `printf`、第 19 行是 `return 0;`）：

```c
#include <stdio.h>

int sum_to(int n)
{
    int total = 0;
    for (int i = 1; i <= n; i++)
        total += i;
    return total;
}

int answer(void)
{
    return sum_to(100);
}

int main(void)
{
	printf("answer:%d\n", answer());
	return 0;
}
```

本機把 `-g` 分別只放在三個階段之一，各編一次：

```sh
# A：-g 放在 -S（C → 組合語言）
gcc -S -O0 -g sumto.c -o A.s && gcc -c A.s -o A.o && gcc A.o -o A
# B：-g 只放在 -c（組合語言 → .o）
gcc -S -O0 sumto.c -o B.s && gcc -c -g B.s -o B.o && gcc B.o -o B
# C：-g 只放在連結
gcc -c B.s -o C.o && gcc -g C.o -o C

for x in A B C; do readelf -S -W $x | grep -c debug; gdb -q -batch -ex 'break 19' -ex run -ex 'info source' ./$x; done
```

| | `-g` 放在 | `.s` 行數 | 執行檔的 `.debug_*` 區段 | `break 19` 的結果 |
|---|---|---|---|---|
| A | `-S` | 506（B.s 只有 97） | 6 個 | `Breakpoint 1 at 0x11b2: file sumto.c, line 19.` → 停在 `main () at sumto.c:19`（`return 0;`） ✅ |
| B | `-c` | 97 | 6 個 | `Breakpoint 1 at 0x1164: file B.s, line 19.` → 停在 `sum_to () at B.s:19` ⚠️ |
| C | 連結 | 97 | 0 個 | 沒有除錯資訊，`break 19` 下不了，`run` 直接跑完 ❌ |

**A 是正解。** `A.s` 裡多了這些東西（`.file 0` 那行原本是完整的目錄路徑）：

```asm
	.file 0 "<編譯時所在的目錄>" "sumto.c"
	.file 1 "sumto.c"              ← 1 號檔案 = sumto.c
	.loc 1 4 1                     ← 下面的指令屬於 1 號檔案第 4 行第 1 欄
	...
	.loc 1 19 9 is_stmt 1
	movl	$0, %eax               ← 第 19 行 return 0;
	.loc 1 20 1
	popq	%rbp
	...
	.section	.debug_info,"",@progbits     ← 還有 .debug_abbrev、.debug_aranges、.debug_line、.debug_line_str、.debug_str
```

**B 很有意思：它也有除錯資訊，但對照的是 `B.s` 本身的行號。** `B.s` 裡沒有 `.loc`，`as` 不知道 `sumto.c` 的存在；
加了 `-g` 之後，`gcc` 叫 `as` 時多傳一個 `--gdwarf-5`（`gcc -v -c -g B.s` 看得到：`as -v --gdwarf-5 --64 -o ... B.s`），
意思是「替**這個組合語言檔**產生行號資訊」。所以 `break 19` 停的是 `B.s` 的第 19 行——`sum_to` 迴圈裡的 `movl -4(%rbp), %eax`，
`gdb` 的 `list` 顯示的也是組合語言。這在「自己手寫組合語言」時才有用；對 C 程式來說，變數名稱、型別、C 的行號全都沒有。

**C 什麼都沒有。** 連結時的 `-g` 不會憑空產生除錯資訊——`ld` 只會把各個 `.o` 已經有的 `.debug_*` 區段合併起來，`C.o` 裡沒有，就什麼都沒有。

**實務上**：每一步都加 `-g` 也無妨（A 的做法再加上 `-c -g`、連結 `-g`，結果跟 A 一樣：`A.s` 裡已經有 `.loc`，`as` 會照它的，不會改用 `.s` 的行號——本機實測三步都加 `-g`，`break 19` 仍是 `file sumto.c, line 19`、停在 `main () at sumto.c:19`）。
真正必要的只有 `-S` 那一步；一次編完時（`gcc -g -O0 sumto.c -o sumto0`）就不用想這件事，`gcc` 會把 `-g` 交給 `cc1`。

#### 陷阱：只重跑加了 `-g` 的第一步，後面兩個檔案還是舊的（2026-10-05 使用者實際遇到）

> 情境：使用者在舊的 `sumto0.s`、`sumto0.o`、`sumto0` 都還在的情況下，加 `-g` 重跑第一步，`break 19` 還是不行；
> 把三個檔案刪掉、三步重跑一次就可以了。他懷疑「同名檔案存在時 gcc 不會重新建立」。

**不是 gcc 不覆寫。** `-o` 指定的輸出檔存在時，gcc 一律**覆寫**。本機實測同一個 `sumto0.s` 重編前後：

```
sumto0.s inode=18483378 mtime=2026-10-05 19:36:00.023660069 +0800
sumto0.s inode=18483378 mtime=2026-10-05 19:36:03.238718871 +0800   ← 同一個檔案，內容與修改時間更新了
```

**真正的原因：三個階段是三個獨立的命令，每個只產生自己 `-o` 的那個檔案。** 只重跑第一步，就只有 `sumto0.s` 是新的；
`sumto0.o` 和 `sumto0` 還是舊的、沒有除錯資訊的版本，gdb 讀的是 `sumto0`。本機重現（先不加 `-g` 三步編好，再只重跑第一步）：

```sh
gcc -S -O0 sumto.c -o sumto0.s && gcc -c sumto0.s -o sumto0.o && gcc sumto0.o -o sumto0
gcc -S -O0 -g sumto.c -o sumto0.s          # 只重跑第一步
ls -l --time-style=+%T sumto0.s sumto0.o sumto0
file sumto0
```

```
-rwxrwxr-x 1 set set 16024 19:35:58 sumto0       ← 舊的
-rw-rw-r-- 1 set set  1808 19:35:57 sumto0.o     ← 舊的
-rw-rw-r-- 1 set set  7518 19:36:00 sumto0.s     ← 只有它是新的（裡面已經有 17 個 .loc）
sumto0: ELF 64-bit LSB pie executable, ..., not stripped        ← 沒有 with debug_info
```

接著補跑第二、三步，`sumto0.o` 從 1808 變 4576 byte、`sumto0` 從 16024 變 17560 byte，`file` 變成 `with debug_info, not stripped`，`break 19` 就成功了：

```
Breakpoint 1 at 0x11b2: file sumto.c, line 19.
```

**怎麼檢查自己拿到的執行檔是不是新的、有沒有除錯資訊：**

| 檢查 | 命令 | 看什麼 |
|---|---|---|
| 檔案新舊 | `ls -l --time-style=+%T sumto0.s sumto0.o sumto0` | 後一步的產物，時間應該比前一步晚 |
| 有沒有除錯資訊 | `file sumto0` | 有 `with debug_info` 才有 |
| 同上，更直接 | `readelf -S sumto0 \| grep debug` | 有 `.debug_info`、`.debug_line` 等區段 |
| gdb 載入時 | `gdb ./sumto0` 開頭 | 出現 `(No debugging symbols found in ./sumto0)` 就是沒有 |

另一個可能的干擾：如果一直沿用**同一個 gdb**，前面設錯的中斷點（例如被綁到 glibc 的 `errname.c` 那兩個，見[第八節](#沒加--g-就用行號下中斷點中斷點跑到-glibc-裡永遠不會停2026-10-05-使用者實際遇到)）還留著，用 `info breakpoints` 看、`delete` 清掉，或乾脆重開 gdb。

「改了來源檔，要記得把**後面所有依賴它的檔案**都重做」正是 `make` 要解決的問題：它比較檔案的修改時間，自動判斷哪幾步要重跑（第 ① 站第 6 課）。在那之前，分階段編譯時就三步一起重跑。

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

上表「位址」那一列的 `*` 是位置的語法記號（「後面是一個算出位址的式子」），不是 C 的取值 `*`；沒加 `*` 時 gdb 會把 `main + 4` 當成函式名去找而報 `Function "main + 4" not defined.`（本機實測）。另外 `break 函式名` 會跳過函式開頭的準備指令、`break *函式名` 停在第一條指令，兩者在 `-O0` 下位址不同。實測與說明見 [x86-64組合語言入門第三節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#三指令的文法助憶符大小後綴運算元)〈上面 `break *widen_char+4` 的 `*`：為什麼要加？〉（2026-10-06 補充）。

#### 怎麼知道要用哪個行號（2026-09-25 補充）

> 起因：使用者問「要怎麼知道一個 `.c` 檔在 gdb 裡正確的行數」。以下用 `small-project/s1-01-hello-compile/avg.c`
> 複製一份到實驗目錄，以 `gcc -g -O0 avg.c -o avg_O0` 編譯後實測。

**gdb 的行號就是 `.c` 原始檔的行號**，從 1 開始數，空行、註解、`#include` 都算一行。它來自 `-g` 寫進執行檔的除錯資訊，
不是 gdb 自己去數的。`avg.c` 的完整原始碼與行號（`cat -n avg.c`）：

```
     1	#include <stdio.h>
     2
     3	int average(int *a, int n)
     4	{
     5	    int sum = 0;                          /* 第 5 行 */
     6	    for (int i = 0; i < n; i++)
     7	        sum += a[i];
     8	    int avg = sum / n;
     9	    return avg;
    10	}
    11
    12	int main(void)
    13	{
    14	    int a[] = {3, 6, 9};
    15	    printf("%d\n", average(a, 3));        /* 第 15 行 */
    16	    return 0;
    17	}
```

**查行號的方法**

| 在哪裡 | 方法 | 說明 |
|---|---|---|
| gdb 外 | `cat -n avg.c` | 每行前面加行號（`nl -ba avg.c` 也可以；`nl` 不加 `-ba` 會跳過空行不編號，跟 gdb 對不上） |
| gdb 外 | `grep -n 'average(a' avg.c` | 直接找某段文字在第幾行：`15:    printf(...)` |
| gdb 外 | 編輯器的行號 | vim 用 `:set number` |
| gdb 內 | `list 函式名` | 印出那個函式附近的原始碼，**左邊就是 gdb 用的行號**，最可靠 |
| gdb 內 | `list 12,17` | 印出第 12～17 行 |
| gdb 內 | `info line 15` | 第 15 行對應哪一段機器碼：`Line 15 of "avg.c" starts at address 0x11ec <main+48> and ends at 0x1213 <main+87>.` |
| gdb 內 | `info source` | 程式停下來後，看 gdb 讀的是**哪個路徑的**原始檔、共幾行、用什麼選項編譯（`-g -O0`） |

`list` 的全部寫法（`help list` 實測）：

| 寫法 | 印出什麼 |
|---|---|
| `list`（不給參數） | 接著上一次，再印 10 行 |
| `list +` | 上一次那 10 行的**後面** 10 行 |
| `list -` | 上一次那 10 行的**前面** 10 行 |
| `list .` | 目前執行到的那一行附近 10 行 |
| `list 位置` | 那個位置附近 10 行；位置可以是 `行號`、`檔名:行號`、`函式名`、`檔名:函式名`、`*位址` |
| `list 起,迄` | 第「起」到第「迄」行；省略一邊（`list ,20`、`list 10,`）表示離另一邊 10 行 |

一次印幾行由 `set listsize N` 調整（預設 10，`show listsize` 查看）。

**行號對不上的三種情況**（本機實測）：

1. **中斷點設在沒有程式碼的行**（空行、只有 `{` 的行）：gdb 自動移到**下一個有程式碼的行**，並在訊息裡告訴你實際位置：
   ```
   (gdb) break 2
   Breakpoint 1 at 0x1178: file avg.c, line 5.      ← 要求第 2 行（空行），實際設在第 5 行
   (gdb) break 11
   Breakpoint 2 at 0x11c8: file avg.c, line 13.     ← 第 11 行（空行）→ 第 13 行
   ```
   **設完中斷點一定要看它回報的 `line N`**，那才是真正的位置。

2. **改了 `.c` 卻沒有重新編譯**：這是最危險的情況。執行檔裡的除錯資訊還是舊的行號，gdb 卻去讀**新的**原始檔顯示文字，
   兩者就錯開了。實測在 `avg.c` 最上面插入一行註解、不重編：
   ```
   (gdb) break 15
   (gdb) run
   Breakpoint 1, main () at avg.c:15
   warning: Source file is more recent than executable.   ← 唯一的警告
   15	    int a[] = {3, 6, 9};                             ← 顯示的文字是新檔的第 15 行
   ```
   程式其實停在 `printf(...)`（舊檔的第 15 行），畫面上卻顯示 `int a[] = ...`。**看到這個 warning 就先重新編譯。**
   重編之後，`printf` 那一行變成第 16 行，`break 16` 才對。

3. **最佳化過的程式（`-O2`）**：程式碼被重排、合併，行號只是大概。實測：
   ```
   (gdb) break 15
   Breakpoint 2 at 0x1080: file avg.c, line 16.     ← 要第 15 行，被放到第 16 行
   (gdb) info line 8
   Line 8 of "avg.c" is at address 0x11b2 <average+50> but contains no code.   ← 第 8 行的計算被併進別的地方
   ```
   要照行號單步除錯，就用 `-g -O0` 編（見 [gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md) 第三節）。

**為什麼 `#include` 沒有把行號弄亂？** 前處理後的 `.i` 有 830 行（`stdio.h` 整份貼進來了），`printf` 那行早就不在第 15 行。
但前處理器會在 `.i` 裡留下**行號標記**（`gcc -E avg.c | grep '"avg.c"'`）：

```
# 0 "avg.c"
# 1 "avg.c"
# 2 "avg.c" 2      ← 從 stdio.h 回到 avg.c 的第 2 行
# 3 "avg.c"        ← 接下來這行是 avg.c 的第 3 行
```

編譯器照這些標記記錄「這行原本是哪個檔案的第幾行」，所以除錯資訊裡的行號永遠是**原始 `.c` 檔**的行號
（前處理見 [hello.c怎麼變成能跑的程式](./hello.c怎麼變成能跑的程式-編譯的四個階段.md) 第二節）。


### 表達式、值歷史、便利變數

- `print`、`display`、`watch`、`if` 條件裡寫的是**被除錯程式所用語言的表達式**——C 程式就用 C 語法：
  `print a[1] + n`、`print *ptr`、`print &sum`、`print sizeof(int)`。
- 每次 `print` 的結果會存成 `$1`、`$2`……（值歷史），之後可以直接拿來用：`print $1 * 2`；`$` 是上一個、`$$` 是上上個。
- `$` 開頭的自訂名字是**便利變數（convenience variable）**：`set $i = 0`。暫存器也用 `$` 表示：`$rip`、`$rsp`、`$rax`。
  另外有 4 個**每種架構都通用的標準暫存器名字**（gdb 手冊〈Registers〉）：`$pc`（program counter，下一個要執行的指令位址；x86-64 上＝`$rip`）、`$sp`（stack pointer；＝`$rsp`）、`$fp`（frame pointer，目前這層函式的框架位址）、`$ps`（processor status；＝`eflags`）。
  打錯暫存器名字**不會報錯**：`p $eip`（x86-64 沒有這個暫存器）印出 `void`，因為 gdb 把它當成還沒設定的便利變數。實測見 [x86-64組合語言入門第五節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面 gdb 命令裡的 `$pc` 是什麼？〉（2026-10-07 補充）。
- 除了 C 的語法，gdb 的表達式還多了兩個**gdb 自己的**運算子（`help print` 裡的說明，2026-09-27 第 3 課補充）：
  - **`FOO@NUM`**：把「從 `FOO` 開始、連續 `NUM` 個同型別的東西」當成陣列。函式參數是 `int *p` 時，`print p` 只印位址，
    `print *p@10` 才印出它指向的 10 個 `int`（gdb 跟 C 一樣不知道指標指向幾個元素，要你告訴它）。`FOO` 必須是在記憶體裡的東西。
  - **`{TYPE} 位址`**：把某個位址上的資料當成 `TYPE` 讀，例如 `print {int} 0x7fffffffd2f0`。效果同 C 的 `*(int *)0x7fffffffd2f0`。
  - 實例見 [陣列、字串與指標運算](./陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md) 第六節。

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
每一類的標題註明頂層命令數。類別裡**第一次學就該會的**用 ★ 標出；★ 是連結，點下去跳到[第零節](#零依任務查常用命令各一個例子)那個命令的例子。

### 4.1 running：控制程式怎麼跑（32 個）

| 命令 | 作用 |
|---|---|
| `advance` | 讓程式繼續跑到指定位置（位置寫法同 `break`） |
| `attach` | 附加到 gdb 之外的一個行程或檔案 |
| [★](#continuec繼續跑到下一個中斷點) `continue`、`fg`、`c` | 在訊號或中斷點停下後繼續執行 |
| `detach` | 脫離先前 `attach` 的行程（行程繼續自己跑） |
| `disconnect` | 中斷與目標（target）的連線 |
| [★](#finish跑完目前這個函式印出回傳值) `finish`、`fin` | 執行到目前這個函式回傳為止，並印出回傳值 |
| `handle` | 指定收到各種訊號（signal）時 gdb 要怎麼處理（停不停、印不印、傳不傳給程式） |
| `inferior` | 在多個被除錯程式之間切換 |
| `interrupt` | 中斷正在執行的被除錯程式 |
| `jump`、`j` | 跳到指定的行或位址繼續執行（跳過中間的程式碼，危險） |
| `kill` | 結束被除錯的程式 |
| [★](#nextn走一行遇到函式呼叫不進去) `next`、`n` | 執行一行原始碼；遇到函式呼叫**不進去**，整個呼叫當一步 |
| `nexti`、`ni` | 執行一條機器指令；遇到 `call` 不進去 |
| `queue-signal` | 排一個訊號，在目前執行緒恢復執行時送給它 |
| `reverse-continue`、`rc` | **倒著**繼續執行（需先 `record`） |
| `reverse-finish` | 倒著執行到目前函式被呼叫之前 |
| `reverse-next`、`rn` | 倒著走一行，不進入函式 |
| `reverse-nexti`、`rni` | 倒著走一條指令，不進入函式 |
| `reverse-step`、`rs` | 倒著走到另一行原始碼的開頭 |
| `reverse-stepi`、`rsi` | 倒著走剛好一條指令 |
| [★](#runr從頭執行) `run`、`r` | 開始執行被除錯的程式；後面可以接程式的參數，也能用 `<`、`>` 重新導向 |
| `signal` | 帶著指定的訊號繼續執行程式 |
| [★](#start執行並停在-main-開頭) `start` | 開始執行，並在 `main` 的開頭停下來（等於在 `main` 設暫時中斷點再 `run`） |
| `starti` | 開始執行，停在**第一條指令**（比 `main` 更早，在啟動碼裡——第 1 課說的「程式不是從 `main` 開始」） |
| [★](#steps走一行遇到函式呼叫會進去) `step`、`s` | 執行到下一行原始碼；遇到函式呼叫**會進去** |
| `stepi`、`si` | 剛好執行一條機器指令（會進入 `call`） |
| `taas` | 對所有執行緒執行一個命令（忽略錯誤與空輸出） |
| `target` | 連接到目標機器或行程（遠端除錯、core 檔等） |
| `task` | 在 Ada 語言的 task 之間切換 |
| `tfaas` | 對所有執行緒的所有堆疊框執行一個命令 |
| `thread`、`t` | 在執行緒之間切換 |
| [★](#untilu跑完這個迴圈) `until`、`u` | 執行到超過目前這一行（常用來跑完一個迴圈），或跑到指定位置 |

### 4.2 breakpoints：讓程式在哪裡停（24 個）

| 命令 | 作用 |
|---|---|
| `awatch` | 設「存取」監看點：運算式被讀或被寫時停下 |
| [★](#breakb在函式設中斷點) `break`、`brea`、`bre`、`br`、`b` | 在指定位置設中斷點；完整文法：`break [PROBE_MODIFIER] [LOCATION] [thread N] [-force-condition] [if 條件]` |
| `break-range` | 對一段位址範圍設中斷點 |
| `catch` | 設 catchpoint：捕捉事件（例如 `catch syscall write`、`catch fork`、`catch throw`） |
| `clear`、`cl` | 刪除**指定位置**上的中斷點 |
| `commands` | 設定某個中斷點被觸發時要自動執行的 gdb 命令 |
| `condition` | 讓第 N 號中斷點只在條件成立時才停 |
| [★](#deleted刪掉中斷點) `delete`、`del`、`d` | 刪除**指定編號**的中斷點（不給編號就全刪） |
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
| [★](#watch變數的值改變時停下) `watch` | 設監看點（watchpoint）：運算式的值**改變**時停下 |

### 4.3 data：查看與修改資料（22 個）

| 命令 | 作用 |
|---|---|
| `agent-printf` | 只給目標端代理（agent）用的格式化輸出 |
| `append` | 把目標程式的程式碼／資料附加到本機檔案 |
| `call` | 呼叫被除錯程式裡的一個函式並印出回傳值 |
| [★](#disassemble反組譯目前的函式) `disassemble` | 反組譯一段記憶體（不給參數就是目前的函式）；`/s` 同時顯示原始碼，`/r` 顯示機器碼位元組 |
| [★](#display每次停下都自動印) `display` | 每次程式停下來都自動印出某個運算式 |
| `dump` | 把目標程式的程式碼／資料傾印到本機檔案 |
| `explore` | 互動式地逐層探索一個值或型別 |
| `find` | 在記憶體裡搜尋一串位元組 |
| `init-if-undefined` | 便利變數還沒定義時才初始化它 |
| `mem` | 定義記憶體區域的屬性 |
| `memory-tag` | 查看與操作記憶體標籤（memory tagging，特定 CPU 功能） |
| `output` | 像 `print`，但不存進值歷史、不換行（寫腳本用） |
| [★](#printp印出變數或運算式) `print`、`inspect`、`p` | 印出運算式的值；`print/FMT` 指定格式 |
| `print-object`、`po` | 請 Objective-C 物件印出自己 |
| `printf` | 像 C 的 `printf` 一樣格式化輸出 |
| [★](#ptype看型別的完整定義) `ptype` | 印出型別的**完整定義**（例如 struct 的所有欄位） |
| `restore` | 把檔案內容寫回目標程式的記憶體 |
| `set` | 計算運算式並指定給變數；**也是**設定 gdb 選項的前綴命令（見第六節） |
| `undisplay` | 取消 `display` |
| `whatis` | 印出運算式的**型別名稱** |
| `with`、`w` | 暫時把某個設定改成某值、執行一個命令、再改回來 |
| [★](#x直接看記憶體) `x` | 直接檢查記憶體：`x/FMT 位址`（格式見第三節） |

### 4.4 stack：呼叫堆疊（7 個）

| 命令 | 作用 |
|---|---|
| [★](#backtracebtwhere印出呼叫鏈) `backtrace`、`where`、`bt` | 印出所有堆疊框（誰呼叫了誰），或最內層的 COUNT 個 |
| `down`、`dow`、`do` | 選取並印出「被目前這層呼叫」的那一層（往內） |
| `faas` | 對所有堆疊框執行一個命令 |
| [★](#frame-nf切到某一層) `frame`、`f` | 選取並印出一個堆疊框（`frame 1` 切到呼叫者那層，才能看它的區域變數） |
| `return` | 讓目前選取的堆疊框**立刻**回傳給呼叫者（跳過函式剩下的程式碼） |
| `select-frame` | 選取堆疊框但不印出任何東西 |
| `up` | 選取並印出「呼叫了目前這層」的那一層（往外） |

> 課後討論：[第九節第 1 章〈印不出 `buf`、現在在哪層、怎麼跳層：用 `frame`、`up`／`down`、`bt` 切換與查看堆疊框〉](#1-印不出-buf現在在哪層怎麼跳層用-frameupdownbt-切換與查看堆疊框)

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
| [★](#listl印出原始碼) `list`、`l` | 列出原始碼：`list 函式`、`list 行號`、`list 起,迄`；`list +`／`list -` 往後／往前，`list .` 目前執行點附近（預設 10 行） |
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
| [★](#info前綴命令) `info`、`inf`、`i` | 顯示**被除錯程式**的各種資訊；本身是前綴，65 個子命令見第五節 |
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
| [★](#helph問-gdb-某個命令怎麼用) `help`、`h` | 列出命令與說明 |
| `if` | 條件成立時執行內層命令（腳本用） |
| `interpreter-exec` | 用指定的直譯器執行命令 |
| `make` | 在 gdb 裡執行 `make`，後面的字都當成 `make` 的參數 |
| `new-ui` | 建立新的使用者介面（例如在另一個終端機再開一個 gdb 介面） |
| `overlay`、`ov`、`ovly` | 除錯 overlay（嵌入式系統的程式碼覆蓋技術） |
| `pipe`、`\|` | 把 gdb 命令的輸出交給 shell 命令處理，例如 `\| info functions \| grep avg` |
| [★](#quitq離開) `quit`、`exit`、`q` | 離開 gdb |
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
| [★](#info-args目前這層的所有參數) `info args` | 目前堆疊框的所有參數 |
| `info auto-load` | 自動載入檔案的狀態 |
| `info auxv` | 被除錯程式的輔助向量（auxiliary vector，核心在 `execve` 時交給程式的資訊） |
| `info bookmarks` | 書籤狀態（搭配 `record`） |
| [★](#info-breakpointsinfo-b列出所有中斷點) `info breakpoints`、`info b` | 所有中斷點／監看點／catchpoint 的狀態與編號 |
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
| [★](#info-frame這一層的詳細資訊) `info frame`、`info f` | 目前堆疊框的詳細資訊（回傳位址、呼叫者、暫存器存放位置） |
| `info frame-filter` | 已註冊的 Python frame filter |
| [★](#info-functions找函式) `info functions` | 所有函式名稱（可加正規表示式過濾） |
| `info guile`、`info gu` | Guile 相關資訊的前綴 |
| `info inferiors` | 目前管理的所有被除錯程式 |
| [★](#info-line某一行原始碼對應哪些機器碼) `info line` | 某一行原始碼對應的機器碼位址 |
| [★](#info-locals目前這層的所有區域變數) `info locals` | 目前堆疊框的所有區域變數 |
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
| [★](#info-proc行程的基本資訊) `info proc` | 行程的額外資訊（`info proc mappings` 看記憶體配置——第 ③ 站會用） |
| `info program` | 程式的執行狀態（為什麼停下） |
| `info record`、`info rec` | 記錄功能的狀態 |
| [★](#info-registersinfo-r看暫存器) `info registers`、`info r` | 整數暫存器與內容（`info registers rip` 只看一個） |
| `info scope` | 某個範圍內的區域變數 |
| `info selectors` | Objective-C selector |
| [★](#info-sharedlibrary載入了哪些-so) `info sharedlibrary`、`info dll` | 已載入的共享函式庫（第 1 課的 `libc.so.6`、`ld-linux-x86-64.so.2` 會出現在這裡） |
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
| [★](#info-threads所有執行緒) `info threads` | 目前所有執行緒（第 ④ 站會用） |
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

> 課後討論：[第九節第 2 章〈在 gdb 裡查區段：位址在哪（`info files`）、內容是什麼（`x`、`disassemble`、`dump`）〉](#2-在-gdb-裡查區段位址在哪info-files內容是什麼xdisassembledump)

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
| `No current process: you must name one.` | `info proc`（包括 `info proc mappings`）要看的是**正在跑的行程**；程式還沒 `run`，或已經跑完（`exited normally`） | 先讓程式**停**在某處再下，例如 `break sum_to` → `run`，或 `start`、`starti` |
| `Warning: Cannot insert breakpoint 1. Cannot access memory at address 0x11f4`（`run` 時） | 用 `break *數字` 寫了 `objdump -d` 查到的**檔案裡的位址**；PIE 程式執行時被搬到基底 `0x555555554000` 之後，這個位址沒有對映 | 改用 `break *函式名+位移`（gdb 會重新算），或寫執行時的位址（`0x555555554000` ＋ 檔案位址）。見 [x86-64組合語言入門第三節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#三指令的文法助憶符大小後綴運算元)〈上面中斷點 4 的 `break *0x11f4`：為什麼 `run` 之後失敗？〉 |
| `Function "deref+4" not defined.` 接著問 `Make breakpoint pending …?` | 想用「函式＋位移」下中斷點卻忘了 `*`，gdb 把 `deref+4` 整串當成函式名 | 回答 `n`，改成 `break *deref+4` |
| `No line 19 in the current file.` 接著問 `Make breakpoint pending on future shared library load? (y or [n])` | 「目前的檔案」裡沒有第 19 行可以停——最常見是程式**沒加 `-g`**，gdb 根本不知道 `.c` 的行號 | 回答 `n`；改用函式名稱（`break main`），或加 `-g` 重編。見下面〈沒加 `-g` 就用行號下中斷點〉 |

### 沒加 `-g` 就用行號下中斷點：中斷點跑到 glibc 裡，永遠不會停（2026-10-05 使用者實際遇到）

> 情境：第 4 課想照筆記用 `info proc mappings` 看 `sumto0` 的記憶體對映。`sumto0` 是照筆記 `gcc -O0 sumto.c -o sumto0` 編的（**沒有 `-g`**）。
> 使用者 `run` 完下 `info proc mapping` → `No current process`；`break 19` → 回答 `y` 設成 pending；再 `break 18` → 停到 glibc 的 `errname.c`；每次 `run` 都直接印出 `answer:5050` 跑完。

本機重現（`set breakpoint pending on` 等於互動時對那個問題回答 `y`）：

```sh
gcc -O0 sumto.c -o sumto0          # sumto.c 見第 4 課筆記第一節，第 18 行是 printf、第 19 行是 return 0
gdb -q -batch -ex 'set breakpoint pending on' -ex run -ex 'break 19' -ex run \
    -ex 'info source' -ex 'break 18' -ex 'info breakpoints' ./sumto0
```

```
answer:5050
[Inferior 1 (process 17294) exited normally]       ← 1. 沒有中斷點，run 直接跑完；這時下 info proc mappings 就是 No current process
No line 19 in the current file.                    ← 2. 沒有 -g，gdb 不知道 sumto.c 的行號
Breakpoint 1 (19) pending.                         ←    回答 y：先記著「某個檔案的第 19 行」，等之後載入的函式庫有這一行再設
answer:5050
[Inferior 1 (process 17297) exited normally]
Current source file is ./stdio-common/errname.c    ← 3. 「目前的檔案」變成 glibc 的原始檔！
...
Breakpoint 2 at 0x7ffff7c83400: ./stdio-common/errname.c:18. (2 locations)
Num     Type           Disp Enb Address            What
1       breakpoint     keep y   <MULTIPLE>
1.1                         y   0x00007ffff7c83400 in __get_errname at ./stdio-common/errname.c:55
1.2                         y   0x00007ffff7fef960 in __get_errname at ./stdio-common/errname.c:55
2       breakpoint     keep y   <MULTIPLE>
...
```

三件事疊在一起：

1. **`info proc mappings` 要有正在跑的行程。** `run` 之後沒有任何中斷點擋著，程式就一路跑完、行程消失，什麼都看不到。
2. **行號要靠 `-g` 的除錯資訊。** `sumto0` 沒有 `-g`，gdb 開頭就說了 `(No debugging symbols found in ./sumto0)`，它只有符號表（函式名稱），不知道哪一行對應哪個位址。
3. **行號中斷點是「目前的檔案」的第幾行，而目前的檔案被 glibc 佔走了。** 本機裝了 `libc6-dbg`（glibc 的除錯資訊，[第七節〈實驗：用 `avg.c` 看 `next` 與 `step` 的差別〉](#實驗用-avgc-看-next-與-step-的差別2026-09-23-補充)提過），
   程式跑過一次、libc 和 `ld.so` 被載入後，gdb 能找到行號的「目前檔案」就變成 glibc 的 `errname.c`。
   於是 pending 的「第 19 行」和新下的「第 18 行」都被綁到 `errname.c`（那幾行沒有程式碼，往下移到有程式碼的第 55 行，函式 `__get_errname`）；
   `2 locations` 是因為 libc（`0x7ffff7c8...`）和 `ld.so`（`0x7ffff7fe...`）各有一份這個函式。`sumto0` 從來不會呼叫它，所以永遠不停。

**怎麼做才對**（都實測過）：

| 做法 | 命令 | 說明 |
|---|---|---|
| 用函式名稱下中斷點 | `break sum_to` → `run` → `info proc mappings` | 函式名稱在符號表裡，沒有 `-g` 也能用。第 4 課筆記就是這樣做的 |
| 停在 `main` | `start` → `info proc mappings` | `start` ＝ 在 `main` 設暫時中斷點再 `run`；沒有 `-g` 也行（實測停在 `0x0000555555555197 in main ()`） |
| 停在第一個指令 | `starti` → `info proc mappings` | 停在 `ld.so` 的 `_start`，`main` 都還沒開始；`sumto0` 的分頁已經對映好了 |
| 要用行號 | `gcc -g -O0 sumto.c -o sumto0` 重編，再 `break 19` | 有除錯資訊後，`break 19` 才會綁到 `sumto.c:19`；不放心就先 `info source` 或 `list` 確認目前的檔案（[第三節〈怎麼知道要用哪個行號〉](#怎麼知道要用哪個行號2026-09-25-補充)） |

**教訓：gdb 問 `Make breakpoint pending ...?` 時，除非你確定那個位置在「之後才載入的共享函式庫」裡，否則回答 `n`——它通常代表你下錯了，而不是要等。**
已經設錯的中斷點用 `info breakpoints` 看、`delete 1` 刪掉。

---

## 九、課後討論

這一節收**課後提問與討論**，一個主題一章。正文裡被問到的地方有一行「課後討論」連結跳到這裡，每章開頭的「正文出處」再連回正文。

- [第 1 章](#1-印不出-buf現在在哪層怎麼跳層用-frameupdownbt-切換與查看堆疊框)：印不出 `buf`、現在在哪層、怎麼跳層：用 `frame`、`up`／`down`、`bt` 切換與查看堆疊框
- [第 2 章](#2-在-gdb-裡查區段位址在哪info-files內容是什麼xdisassembledump)：在 gdb 裡查區段：位址在哪（`info files`）、內容是什麼（`x`、`disassemble`、`dump`）

### 1. 印不出 `buf`、現在在哪層、怎麼跳層：用 `frame`、`up`／`down`、`bt` 切換與查看堆疊框

> 2026-11-04 課後提問（第 7 課的 `app_dyn`，同一串連問四則）：
> - 「我現在在嘗試在app_dyn用gdb想打印變數buf都打印不出來, 我是先break *trim 在run」
> - 「有指令可以知道當前frame的位置嗎?」
> - 「可以指定跳第幾層嗎?」
> - 「有沒有辦法一次看所有的frame有哪些?」
>
> 正文出處：[第一節〈gdb 在做什麼，為什麼要 `-g`〉](#一gdb-在做什麼為什麼要--g)、[第四節 4.4 stack](#44-stack呼叫堆疊7-個)的命令表（那裡只寫了 `frame 1` 一個例子）、[第八節](#八實際走一次與常見錯誤訊息)錯誤訊息表的 `No symbol "sum" in current context.` 那一列。

**一句話：區域變數只存在它自己那一層堆疊框裡——停在 `trim` 時看不到 `main` 的 `buf`，要先用 `up`／`frame 1` 切過去（或用 `bt full` 一次印出每層的區域變數）；而且這些都要 `-g` 才有。**

#### 實驗用的程式

第 7 課 `--as-needed` 實驗的同一組檔案（[`examples/asneeded/`](../small-project/s1-07-libraries/examples/asneeded/)，`none.c` 這裡沒用到）：主程式 `main` 呼叫 `libtxt.so` 裡的 `trim`，`trim` 再呼叫 `is_space`，所以停在 `is_space` 時堆疊上有三層。

`main.c`（檔案：[`main.c`](../small-project/s1-07-libraries/examples/asneeded/main.c)；第 9 行是 `printf`）：

```c
/* main.c */
#include <stdio.h>

char *trim(char *s);

int main(void)
{
    char buf[] = "   hello";
    printf("[%s]\n", trim(buf));
    return 0;
}
```

`trim.c`（檔案：[`trim.c`](../small-project/s1-07-libraries/examples/asneeded/trim.c)；第 5 行是 `{`、第 6 行是 `while`）：

```c
/* trim.c */
int is_space(char c);

char *trim(char *s)
{
    while (is_space(*s))
        s++;
    return s;
}
```

`space.c`（檔案：[`space.c`](../small-project/s1-07-libraries/examples/asneeded/space.c)；第 2 行是整個函式）：

```c
/* space.c */
int is_space(char c) { return c == ' ' || c == '\t' || c == '\n'; }
```

`case.c`（檔案：[`case.c`](../small-project/s1-07-libraries/examples/asneeded/case.c)；只是一起包進 `libtxt.so`，本章用不到）：

```c
/* case.c */
void to_upper_str(char *s) { for (; *s; s++) if (*s >= 'a' && *s <= 'z') *s -= 32; }
void to_lower_str(char *s) { for (; *s; s++) if (*s >= 'A' && *s <= 'Z') *s += 32; }
```

```sh
# 版本一：照第 7 課的做法，沒有 -g
gcc -fPIC -shared -o libtxt.so trim.c space.c case.c
gcc -o app_dyn main.c -L. -ltxt -Wl,-rpath,'$ORIGIN'
# 版本二：兩邊都加 -g（.so 和執行檔各有各的除錯資訊，要看 trim 的參數，.so 也要加）
gcc -g -fPIC -shared -o libtxt.so trim.c space.c case.c
gcc -g -o app_dyn main.c -L. -ltxt -Wl,-rpath,'$ORIGIN'
```

#### (1) `break *trim` → `run` → `print buf` 為什麼印不出來：兩個原因疊在一起

**原因一：沒有 `-g`。** 版本一（`readelf -S app_dyn | grep -c debug` 與 `libtxt.so` 都是 `0`）：

```text
$ gdb -q -batch -ex 'break *trim' -ex run -ex 'print buf' -ex up -ex 'print buf' -ex 'print $rdi' -ex 'x/s $rdi' ./app_dyn
Breakpoint 1 at 0x1090
Breakpoint 1, 0x00007ffff7fb7119 in trim () from ./libtxt.so     ← 沒有參數、沒有「at trim.c:5」：沒有除錯資訊
No symbol "buf" in current context.
#1  0x00005555555551c2 in main ()
No symbol "buf" in current context.                              ← 切到 main 那層也一樣：根本不知道有 buf
$1 = 140737488344127
0x7fffffffd43f:	"   hello"                                       ← 沒有 -g 時的替代做法：第一個參數還在 %rdi
```

函式名稱在一般的符號表裡（`nm` 看得到），所以 `break *trim` 下得了；變數名稱、型別、它在哪一格，只有 `-g` 的 `.debug_*` 區段才有（第一節）。
停在 `trim` 的**第一條指令**時，第一個參數還在 `%rdi`（[第 5 課第五節〈參數與回傳值放哪裡〉](./x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#五參數與回傳值放哪裡)），所以 `x/s $rdi` 能看到 `buf` 的內容。

（順帶一提：`run` 之前 `Breakpoint 1 at 0x1090`，那時 `libtxt.so` 還沒載入，gdb 只找得到執行檔裡的 `trim@plt` 跳板——`info symbol 0x1090` 印 `trim@plt in section .plt.sec`；
`.so` 載入後 gdb 自動把中斷點改到真正的 `trim`，`info breakpoints` 變成 `0x00007ffff7fb7119 in trim at trim.c:5`。PLT 跳板見[第 7 課課後討論第 4 章](./靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.md#4-plt-存函式位址got-存變數位址嗎plt-是跳板以及延遲綁定)。）

**原因二：`buf` 是 `main` 的區域變數，而你停在 `trim` 這一層。** 版本二（有 `-g`）：

```text
$ gdb -q -batch -ex 'break *trim' -ex run -ex 'print buf' -ex 'print s' -ex up -ex 'print buf' ./app_dyn
Breakpoint 1, trim (s=0x0) at trim.c:5
5	{
No symbol "buf" in current context.            ← 有 -g 也不行：trim 這層只有 s
$1 = 0x0                                       ← s 印出 0x0，是錯的（見下一段）
#1  0x00005555555551c2 in main () at main.c:9  ← up：切到呼叫者 main 那層
9	    printf("[%s]\n", trim(buf));
$2 = "   hello"                                ← 印出來了
```

`print` 只認得**目前選取的那一層**看得到的名字（那一層的參數、區域變數，加上全域變數）。`buf` 屬於 `main` 的堆疊框，要先切過去。

**`s=0x0` 是錯的**：`break *trim` 停在第一條指令，序言（prologue）還沒把 `%rdi` 存進堆疊那格，gdb 照除錯資訊去那格讀到舊值——跟[第 5 課第三節](./x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#三call-與-ret用-gdb-一條一條走)的 `add3 (a=0, b=0, c=0)` 同一回事。
要看參數就用 `break trim`（不加 `*`，gdb 會跳過序言）：

```text
$ gdb -q -batch -ex 'break trim' -ex run -ex 'print s' ./app_dyn
Breakpoint 1, trim (s=0x7fffffffd43f "   hello") at trim.c:6
$1 = 0x7fffffffd43f "   hello"
```

#### (2) 現在在哪一層：`frame`（不帶參數）

以下都是版本二，先停在 `is_space`，讓堆疊有三層。`is_space` 在 `libtxt.so` 裡，`run` 之前那個 `.so` 還沒載入，所以先 `start`（停在 `main`）再 `break is_space`、`continue`：

```sh
gdb -q -batch -ex start -ex 'break is_space' -ex continue -ex bt -ex frame -ex up -ex frame -ex bt -ex 'info frame' ./app_dyn
```

```text
(gdb) bt
#0  is_space (c=32 ' ') at space.c:2
#1  0x00007ffff7fb7141 in trim (s=0x7fffffffd43f "   hello") at trim.c:6
#2  0x00005555555551c2 in main () at main.c:9
(gdb) frame
#0  is_space (c=32 ' ') at space.c:2           ← 不帶參數：只印「目前選的這層」，不切換
2	int is_space(char c) { return c == ' ' || c == '\t' || c == '\n'; }
(gdb) up
#1  0x00007ffff7fb7141 in trim (s=0x7fffffffd43f "   hello") at trim.c:6
(gdb) frame
#1  0x00007ffff7fb7141 in trim (...)           ← up 之後變成 #1
(gdb) bt
#0  is_space (c=32 ' ') at space.c:2           ← 跟上面一模一樣：bt 不標出目前選的層
#1  0x00007ffff7fb7141 in trim (s=0x7fffffffd43f "   hello") at trim.c:6
#2  0x00005555555551c2 in main () at main.c:9
(gdb) info frame
Stack level 1, frame at 0x7fffffffd430:        ← 第一行就是層號
 rip = 0x7ffff7fb7141 in trim (trim.c:6); saved rip = 0x5555555551c2
 called by frame at 0x7fffffffd460, caller of frame at 0x7fffffffd410
 source language c.
 Arglist at 0x7fffffffd420, args: s=0x7fffffffd43f "   hello"
 Locals at 0x7fffffffd420, Previous frame's sp is 0x7fffffffd430
 Saved registers:
  rbp at 0x7fffffffd420, rip at 0x7fffffffd428
```

| 命令 | 印什麼 | 什麼時候用 |
|---|---|---|
| `frame`／`f`（不帶參數） | 目前選的那層：`#N`、函式、參數、停在哪行 | 只想知道「我現在在哪層」 |
| `info frame`／`info f` | `Stack level N` 加上這一框的細節：`saved rip`（返回位址）、呼叫者與被呼叫者的框位址、`rbp`／`rip` 存在哪一格 | 要看堆疊框本身長怎樣（[第 5 課第四節](./x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#四堆疊框pushq-rbp-movq-rsp-rbp-與-leave)） |
| `bt` | 整條呼叫鏈 | 看全貌；**不能**用來判斷目前選在哪層 |

`#0` 永遠是最內層，也就是程式真正停住的函式；數字越大越靠外（越接近 `main`）。
`up`／`down`／`frame` 只換「現在在看哪一層」，程式一步都沒動（[第七節](#七容易混淆的命令差在哪什麼時候用哪個)）——`continue`、`next` 仍然從 `#0` 停住的地方往下跑。

```text
                    ┌──────────────────────────────┐
  #2  main          │ buf = "   hello"             │   ← up 2、frame 2、frame function main
                    ├──────────────────────────────┤
  #1  trim          │ s = 0x7fffffffd43f           │   ← up、frame 1、frame function trim
                    ├──────────────────────────────┤
  #0  is_space      │ c = 32 ' '                   │   ← 程式真正停在這裡（%rip 在這層）
                    └──────────────────────────────┘
        往外 = up（數字變大）        往內 = down（數字變小）
```

圖的重點：選哪一層只決定 `print`、`info locals`、`list` 看的是誰的東西；三層的值同時都在記憶體裡。

#### (3) 跳到指定的層：`frame N`、`up N`／`down N`、`frame function`

```sh
gdb -q -batch -ex start -ex 'break is_space' -ex continue \
    -ex 'frame 2' -ex 'frame 5' -ex 'frame 0' -ex 'up 2' -ex 'down 2' -ex 'up 9' \
    -ex down -ex down -ex down -ex 'frame function trim' ./app_dyn
```

```text
(gdb) frame 2
#2  0x00005555555551c2 in main () at main.c:9          ← 絕對：直接到第 2 層
(gdb) frame 5
No frame at level 5.                                   ← 沒有這層：報錯，目前選的層不變
(gdb) frame 0
(gdb) up 2
#2  0x00005555555551c2 in main () at main.c:9          ← 相對：往外 2 層
(gdb) down 2
#0  is_space (c=32 ' ') at space.c:2                   ← 往內 2 層
(gdb) up 9
#2  0x00005555555551c2 in main () at main.c:9          ← 超過最外層：停在最外層，不報錯
(gdb) down
#1 …
(gdb) down
#0 …
(gdb) down
Bottom (innermost) frame selected; you cannot go down. ← 已經在 #0 再往內：報錯
(gdb) frame function trim
#1  0x00007ffff7fb7141 in trim (s=0x7fffffffd43f "   hello") at trim.c:6   ← 用函式名找
```

| 寫法 | 意思 | 不存在／超出時 |
|---|---|---|
| `frame N`（＝`frame level N`） | 選第 N 層（絕對） | `No frame at level N.` |
| `up [N]` | 往外 N 層，省略 N 就是 1 | 停在最外層，不報錯；已經在最外層時報 `Initial frame selected; you cannot go up.` |
| `down [N]` | 往內 N 層，省略 N 就是 1 | 已經在 `#0` 時報 `Bottom (innermost) frame selected; you cannot go down.` |
| `frame function 名字` | 選最內層的那個函式的框（遞迴時同名有好幾層，選最靠近 `#0` 的） | `Function "nosuch" not defined.` |

（`up` 在最外層的那句、`frame function` 找不到的那句也都實測過：先 `frame 2` 再 `up`、以及 `frame function nosuch`。）

`frame` 的子命令依 `help frame` 共 5 個：上面的 `level`、`function`，以及 `address`（用堆疊框的位址選，就是 `info frame` 印的 `frame at 0x…`）、`apply`（對好幾層各執行一個命令，下一段）、`view`（看一個不在目前 backtrace 裡的框，進階用途）。

#### (4) 一次看所有層：`bt`、`bt full`、`frame apply all`

```sh
gdb -q -batch -ex start -ex 'break is_space' -ex continue \
    -ex bt -ex 'bt full' -ex 'bt 1' -ex 'bt -1' -ex 'frame apply all info locals' ./app_dyn
```

```text
(gdb) bt                              ← 全名 backtrace，也可以打 where
#0  is_space (c=32 ' ') at space.c:2
#1  0x00007ffff7fb7141 in trim (s=0x7fffffffd43f "   hello") at trim.c:6
#2  0x00005555555551c2 in main () at main.c:9
(gdb) bt full                         ← 每層連區域變數一起印
#0  is_space (c=32 ' ') at space.c:2
No locals.
#1  0x00007ffff7fb7141 in trim (s=0x7fffffffd43f "   hello") at trim.c:6
No locals.
#2  0x00005555555551c2 in main () at main.c:9
        buf = "   hello"              ← 不用 up 就看到 main 的 buf
(gdb) bt 1                            ← 只印最內層 1 層
#0  is_space (c=32 ' ') at space.c:2
(gdb) bt -1                           ← 只印最外層 1 層
#2  0x00005555555551c2 in main () at main.c:9
(gdb) frame apply all info locals     ← 對每一層執行 info locals（縮寫 faas info locals）
#0  is_space (c=32 ' ') at space.c:2
No locals.
#1  …
No locals.
#2  0x00005555555551c2 in main () at main.c:9
buf = "   hello"
```

每一列的格式是 `#層號  返回位址 in 函式 (參數=值) at 檔案:行號`。`#0` 沒有位址欄，因為它就是此刻停住的地方；`#1` 以後的位址是「回到那一層後要接著執行的指令」，也就是被呼叫者框裡存的 `saved rip`（對照上面 `info frame` 的 `saved rip = 0x5555555551c2`，正是 `#2` 那列的位址）。
`trim` 的參數 `s` 不算區域變數，所以是 `No locals.`；參數另外用 `info args` 看。沒有 `-g` 時 `bt` 還是列得出函式名稱，但參數與區域變數都沒有。

`frame apply all` 比 `bt full` 通用：後面可以接任何命令（例如 `frame apply all info args`、`frame apply all p $rsp`）。
`bt` 的完整選項用 `help backtrace` 查（還有 `-full`、`-frame-arguments`、`-past-main` 等）；這裡只示範 `full`、`N`、`-N` 三種。

### 2. 在 gdb 裡查區段：位址在哪（`info files`）、內容是什麼（`x`、`disassemble`、`dump`）

> 2026-11-04 課後提問（接在第 1 章後面）：
> - 「從 gdb有辦法知道.bss, .text在哪裏嗎?可以直接查的指令」
> - 「那有辦法看區段的內容嗎?比方.text, .bss」
>
> 正文出處：[第五節〈`info` 的全部 65 個子命令〉](#五info-的全部-65-個子命令)表裡的 `info files`、`info symbol`、`info proc`（那裡每個只有一行說明），以及 [4.3 data](#43-data查看與修改資料22-個) 的 `x`、`disassemble`。

**一句話：gdb 沒有「印出某個區段」的專用命令——先用 `info files` 查出區段在記憶體裡的範圍（要在程式跑起來之後查，PIE 才換成執行時的位址），再用 `x`、`disassemble`、`dump` 去看那段記憶體。**

程式與編譯指令同[第 1 章〈實驗用的程式〉](#實驗用的程式)的版本二（有 `-g`）。位址是 gdb 預設關掉位址隨機化時的值。

#### (1) 位址在哪：`info files`、`maint info sections`、`info symbol`、`info proc mappings`

```sh
gdb -q -batch -ex 'info files' -ex start -ex 'info files' \
    -ex 'maint info sections .text .bss' -ex 'info symbol main' -ex 'info symbol trim' \
    -ex 'info symbol 0x555555558010' -ex 'info proc mappings' ./app_dyn
```

```text
(gdb) info files                                       ← run 之前（節錄，只留 .text、.data、.bss）
	Entry point: 0x10a0
	0x00000000000010a0 - 0x00000000000011f4 is .text
	0x0000000000004000 - 0x0000000000004010 is .data
	0x0000000000004010 - 0x0000000000004018 is .bss
(gdb) start
(gdb) info files                                       ← 跑起來之後（節錄）
	Entry point: 0x5555555550a0
	0x00005555555550a0 - 0x00005555555551f4 is .text
	0x0000555555558000 - 0x0000555555558010 is .data
	0x0000555555558010 - 0x0000555555558018 is .bss
	0x00007ffff7fc6000 - 0x00007ffff7ff0195 is .text in /lib64/ld-linux-x86-64.so.2
	0x00007ffff7fb7060 - 0x00007ffff7fb720c is .text in ./libtxt.so
	0x00007ffff7fba010 - 0x00007ffff7fba018 is .bss in ./libtxt.so
	0x00007ffff7c28800 - 0x00007ffff7db0039 is .text in /lib/x86_64-linux-gnu/libc.so.6
	0x00007ffff7e056e0 - 0x00007ffff7e12d90 is .bss in /lib/x86_64-linux-gnu/libc.so.6
	…（每個 .so 的每個區段都列，實際輸出有幾十行）
```

- `info files`（＝`info target`）列出每個區段的範圍 `起點 - 終點`（終點不含）。**要在程式跑起來之後查**：`run` 之前印的是檔案裡的位址（`0x10a0`，跟 `readelf -S` 一樣）；
  程式是 PIE（Position-Independent Executable，位置無關執行檔），執行時整份被搬到基底 `0x555555554000` 之後，`.text` 就變成 `0x5555555550a0`（＝基底＋`0x10a0`）。跑起來之後它還會把 `ld.so`、`libtxt.so`、`libc.so.6`、vDSO（輸出寫 `system-supplied DSO`）的區段都列出來。
- 這個程式的 `.bss` 只有 8 byte：`main.c` 沒有全域變數，那 8 byte 是 C 執行環境自己的變數（下面 `info symbol` 印出的 `completed`）。

輸出太長時，用另外三個命令縮小範圍：

```text
(gdb) maint info sections .text .bss                   ← 只看指定的區段（只看執行檔本身）
 [15]     0x5555555550a0->0x5555555551f4 at 0x000010a0: .text ALLOC LOAD READONLY CODE HAS_CONTENTS
 [25]     0x555555558010->0x555555558018 at 0x00003010: .bss ALLOC
(gdb) info symbol main                                 ← 反過來問：這個符號／位址在哪個區段
main in section .text of ./app_dyn
(gdb) info symbol trim
trim in section .text of ./libtxt.so
(gdb) info symbol 0x555555558010
completed in section .bss of ./app_dyn
(gdb) info proc mappings                               ← 分頁的對映（節錄）
          Start Addr           End Addr       Size     Offset  Perms  objfile
      0x555555555000     0x555555556000     0x1000     0x1000  r-xp   ./app_dyn     ← .text 0x5555555550a0 在這頁
      0x555555558000     0x555555559000     0x1000     0x3000  rw-p   ./app_dyn     ← .data、.bss 在這頁
      0x7ffff7fb7000     0x7ffff7fb8000     0x1000     0x1000  r-xp   ./libtxt.so   ← libtxt 的 .text
      0x7ffffffdd000     0x7ffffffff000    0x22000        0x0  rw-p   [stack]
```

- `maint info sections`（`maintenance` 前綴，第六節）每列是 `[編號] 執行時起點->終點 at 檔案偏移: 名字 旗標`。編號從 0 起算、不含 ELF 的空區段，所以比 `readelf -S` 的編號（`.text` 是 `[16]`）少 1。
  **`.bss` 只有 `ALLOC`、沒有 `HAS_CONTENTS`**：它在檔案裡不佔空間（`readelf -S` 的型別是 `NOBITS`），載入時才配置記憶體、填 0。`.text` 有 `HAS_CONTENTS`，內容就在檔案偏移 `0x10a0`。
- `info symbol` 回答「這個位址落在哪個符號、哪個區段、哪個檔案」，給位址或符號都可以。
- `info proc mappings` 看的是**分頁**（頁的權限 `r-xp`、`rw-p`），不是區段；區段被包在載入段裡、載入段再對映成頁（[第 7 課課後討論第 16 章](./靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.md#16-got-算一頁嗎區段載入段與頁寫入時複製一次複製一頁)）。所以 `.text` 落在 `r-xp` 那頁、`.bss` 落在 `rw-p` 那頁。

| 命令 | 回答什麼 | 範圍 |
|---|---|---|
| `info files`／`info target` | 每個區段的位址範圍 | 執行檔＋所有已載入的 `.so` |
| `maint info sections [名字…]` | 區段範圍、檔案偏移、旗標（`HAS_CONTENTS` 等） | 只有執行檔（加 `-all-objects` 才含 `.so`，見 `help maint info sections`） |
| `info symbol 位址或符號` | 這個位址屬於哪個符號、哪個區段 | 全部 |
| `info proc mappings` | 分頁的範圍與權限 | 整個行程（含 `[stack]`、`[heap]`、`[vdso]`） |

#### (2) 內容是什麼：`x`、`disassemble`、`dump`

用 `info files` 查到的範圍，就能用一般看記憶體的命令看區段內容。`.text`：`0x5555555550a0 - 0x5555555551f4`；`.bss`：`0x555555558010 - 0x555555558018`。

```sh
gdb -q -batch -ex start -ex 'x/5i 0x5555555550a0' -ex 'disassemble 0x5555555550a0,0x5555555550b0' \
    -ex 'x/16xb 0x5555555550a0' -ex 'x/8xb 0x555555558010' -ex 'p &__bss_start' -ex 'p &_end' \
    -ex 'x/1xg &__bss_start' -ex 'dump binary memory text.bin 0x5555555550a0 0x5555555551f4' ./app_dyn
```

```text
(gdb) x/5i 0x5555555550a0                         ← .text：當成指令看（i），5 條
   0x5555555550a0 <_start>:	endbr64
   0x5555555550a4 <_start+4>:	xor    %ebp,%ebp
   0x5555555550a6 <_start+6>:	mov    %rdx,%r9
   0x5555555550a9 <_start+9>:	pop    %rsi
   0x5555555550aa <_start+10>:	mov    %rsp,%rdx
(gdb) disassemble 0x5555555550a0,0x5555555550b0   ← 反組譯「起點,終點」（終點不含）
Dump of assembler code from 0x5555555550a0 to 0x5555555550b0:
   0x00005555555550a0 <_start+0>:	endbr64
   …
   0x00005555555550ad <_start+13>:	and    $0xfffffffffffffff0,%rsp
End of assembler dump.
(gdb) x/16xb 0x5555555550a0                       ← .text：原始位元組（16 個、十六進位、每個 1 byte）
0x5555555550a0 <_start>:	0xf3	0x0f	0x1e	0xfa	0x31	0xed	0x49	0x89
0x5555555550a8 <_start+8>:	0xd1	0x5e	0x48	0x89	0xe2	0x48	0x83	0xe4
(gdb) x/8xb 0x555555558010                        ← .bss：8 byte 全是 0
0x555555558010 <completed.0>:	0x00	0x00	0x00	0x00	0x00	0x00	0x00	0x00
(gdb) p &__bss_start                              ← 不想抄位址：連結器產生的符號
$1 = (<data variable, no debug info> *) 0x555555558010 <completed>
(gdb) p &_end
$2 = (char (*)[]) 0x555555558018
(gdb) x/1xg &__bss_start
0x555555558010 <completed.0>:	0x0000000000000000
```

- `.text` 有兩種看法，看的是同一段記憶體：`x/xb` 是原始位元組，`x/i`、`disassemble` 把它翻成指令。`f3 0f 1e fa` 就是 `endbr64`、`31 ed` 是 `xor %ebp,%ebp`。
  `x` 的 `/FMT` 規則見[第三節〈輸出格式 `/FMT`〉](#輸出格式-fmtprint-與-x-共用)；`disassemble` 的 `/r`（同時印機器碼）、`/s`（同時印原始碼）見 [4.3 data](#43-data查看與修改資料22-個)。
- `.text` 的開頭是 `_start`（C 執行環境的進入點，`Entry point` 就是它），`main` 在後面。
- `__bss_start`、`_end` 是連結器（linker）自動定義的符號：`.bss` 從 `__bss_start` 開始，`_end` 是整個程式資料（`.data`＋`.bss`）的結尾。這個程式剛好 `_end − __bss_start = 8`，就是 `.bss` 的大小。
  它們沒有除錯資訊的型別，所以 `p` 印出 `<data variable, no debug info>`；`x` 只要位址，不受影響。

**把整段存成檔案：`dump`**。`dump binary memory 檔名 起點 終點` 把那段記憶體原封不動寫進檔案。`help dump` 列出 7 個子命令：`binary`、`ihex`、`srec`、`tekhex`、`verilog` 是**檔案格式**，`memory`（一段記憶體）、`value`（一個表達式的值）是**要存什麼**；格式後面再接 `memory` 或 `value`，不寫格式（`dump memory …`）就是 raw binary。拿它跟直接從執行檔取出的 `.text` 比：

```text
$ ls -l text.bin
-rw-rw-r-- 1 set set 340 Nov  4 11:04 text.bin        ← 0x1f4 − 0xa0 = 0x154 = 340 byte
$ objcopy -O binary --only-section=.text app_dyn text_file.bin    ← 從檔案取出 .text 的內容
$ cmp text.bin text_file.bin && echo SAME
SAME
```

記憶體裡的 `.text` 跟檔案裡的一個 byte 都不差：載入只是把檔案那段對映進來（`info proc mappings` 那頁的 `Offset 0x1000`）。
**`.bss` 就不一樣**：它沒有 `HAS_CONTENTS`，檔案裡沒有它的內容。`objdump -s -j .bss app_dyn`（`objdump -s` 見[第 4 課第三節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#為什麼-l-叫-long-卻是-4-byte組合語言的大小名稱和-c-的型別是兩套)〈為什麼 `l` 叫 long 卻是 4 byte〉）實測印出
`4010 00000000 00000000`，但那 8 個 0 是 `objdump` 依「`.bss` 一律從 0 開始」**補出來的**，不是從檔案讀的——檔案偏移 `0x3010` 那裡放的其實是下一個區段 `.comment`（`xxd -s 0x3010 -l 8 app_dyn` 印出 `GCC: (Ub`，編譯器的版本字串），`.bss` 在檔案裡一個 byte 都沒有。
程式跑起來之後 `.bss` 裡的變數被改成多少，只能在**跑起來的行程**裡用 gdb 看。

| 想看 | 命令 | 說明 |
|---|---|---|
| 程式碼（`.text`）翻成指令 | `x/Ni 位址`、`disassemble 起點,終點` | `x/i` 給條數，`disassemble` 給範圍或函式名 |
| 任何區段的原始位元組 | `x/Nxb 位址`（或 `xg` 每 8 byte 一格） | 大小字母 `b`/`h`/`w`/`g` 見第三節 |
| 字串（`.rodata`） | `x/s 位址` | 印到 `\0` 為止 |
| 整段存成檔案 | `dump binary memory 檔 起點 終點` | 再用 `xxd`、`cmp` 等外部工具看 |
| 從**檔案**看（不用跑） | `objdump -s -j .text app_dyn`、`objdump -d` | `.bss` 只會印出補的 0；位址是檔案裡的位址 |

---

## 自我測驗

> 參考答案寫在每題下面（2026-10-06 使用者指定）。作答時先遮住答案、自己想過再對照。

（**可以翻筆記作答**：本篇是參考型筆記。題目考的是「差別」與「該用哪個」，不考選項字面。）

1. `next` 和 `step` 差在哪？如果不小心 `step` 進了一個不想看的函式（例如 `printf`），用哪個命令最快回到原本那一層？為什麼不是 `continue`？
   > **參考答案**【[第七節](../html/c/gdb完整命令參考.html#七容易混淆的命令差在哪什麼時候用哪個)】`next` 遇到函式呼叫時把整個呼叫當成一步、不進去；`step` 會進去（前提是那個函式有除錯資訊，見〈實驗：用 `avg.c` 看 `next` 與 `step` 的差別〉）。不小心進去時用 `finish`：跑完目前這個函式、停在呼叫它的地方，並印出回傳值。`continue` 不會在「回到上一層」時停下，會一直跑到下一個中斷點或程式結束，就錯過想看的地方了。
2. `info` 和 `show` 都是「顯示」，差別是什麼？`set` 為什麼同時出現在 data 類和第六節？
   > **參考答案**【[第五節](../html/c/gdb完整命令參考.html#五info-的全部-65-個子命令)、[第六節](../html/c/gdb完整命令參考.html#六setshowmaintenance數量為什麼不逐項列怎麼查)】`info` 看**被除錯程式**的狀態（中斷點、暫存器、堆疊框、區域變數……），`show` 看 **gdb 自己**的設定。`set` 有兩種用途：改 gdb 的設定（第六節，對應 `show`），以及改程式裡變數的值（`set var x = 3`，屬於 data 類），所以兩邊都有它。
3. `break`、`watch`、`catch` 三者分別讓程式在「什麼條件」停下？各舉一個它比另外兩個更適合的情境。
   > **參考答案**【[第七節](../html/c/gdb完整命令參考.html#七容易混淆的命令差在哪什麼時候用哪個)】`break`：程式**執行到某個位置**（行號、函式）時停下，適合已經知道問題大概在哪、想從那裡一步步看。`watch`：某個**運算式的值改變**時停下，不管是哪一行改的，適合不知道是哪一行改壞了變數。`catch`：發生某個**事件**時停下（系統呼叫、`fork`、`exec`、C++ 例外），適合「程式什麼時候呼叫了 `write`」這類問題——系統呼叫發生在 libc 內部，不知道是原始碼哪一行觸發的，`break` 很難設。
4. 為什麼 gdb 裡的 `print` 可以寫 `a[1] + n`、`*ptr` 這種 C 語法？如果程式沒有用 `-g` 編譯，`print sum` 會發生什麼事？
   > **參考答案**【[第一節](../html/c/gdb完整命令參考.html#一gdb-在做什麼為什麼要--g)】`-g` 會把**除錯資訊**寫進執行檔（`readelf -S` 看得到 `.debug_info`、`.debug_line` 等區段），內容包括每個變數的名字、型別、此刻放在哪裡，以及每段機器碼對應哪一行；gdb 讀了這些資訊，才能把 `a[1] + n` 當成 C 來算。沒有 `-g` 時，`print sum` 得到 `No symbol "sum" in current context.`。沒有 `-g` 仍能 `break average`，是因為函式名稱存在一般的符號表裡（`nm` 看得到 `T average`）；`-g` 補的是行號、參數、區域變數這些更細的資訊。
5. 情境題：你懷疑某個全域變數在程式某處被意外改掉了，但不知道是哪一行改的。你會用本篇的哪個命令找出兇手？如果那個變數在 `-O2` 下被最佳化成只放在暫存器，這個方法還可靠嗎？
   > **參考答案**【[第七節](../html/c/gdb完整命令參考.html#七容易混淆的命令差在哪什麼時候用哪個)〈實驗：`watch` 在 `-O0` 與 `-O2` 下抓全域變數〉】用 `watch 變數名`。對全域變數，`-O2` 下仍然可靠：它一定有記憶體位址，硬體監看點抓得到最後是哪個函式寫入的；但連續的變化可能被合併成一次、停下的行號不精確，要用 `bt` 確認是哪個函式。
6. 情境題：你在 gdb 裡 `break 20`，程式停下來後顯示的那一行跟你預期的完全不同，還出現 `warning: Source file is more recent than executable.`。發生了什麼事？程式實際停在哪裡？該怎麼處理？另外，為什麼 `.i` 檔有幾百行，gdb 的行號卻還是對應原始的 `.c`？
   > **參考答案**：（還沒作答；作答、批改完再補上參考答案。）
