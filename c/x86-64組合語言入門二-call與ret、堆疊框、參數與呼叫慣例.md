# 讀懂 x86-64 組合語言（二）—— `call` 與 `ret`、堆疊框、參數放哪裡、誰負責保存暫存器

> 日期：2026-10-09
> 情境：系統化學習路線第 ① 站第 5 課。對應 CS:APP 第 3.7 節（程序：執行時堆疊、轉移控制、資料傳送、區域變數、暫存器的使用慣例）；
> 補完第 4 課留下的兩個問題：`-O0` 函式開頭的 `pushq %rbp; movq %rsp, %rbp` 在做什麼、`main` 呼叫 `printf` 前為什麼要 `movl $0, %eax`
> 適用範圍：實測本機 Ubuntu 24.04、x86-64、gcc 13.3.0、GNU objdump 2.42、gdb 15.1；所有組合語言與 gdb 輸出都是本機實際產生的。
> 呼叫慣例的規則（哪個參數放哪個暫存器、誰保存哪些暫存器、對齊）出自 *System V AMD64 ABI*（Linux 在 x86-64 上用的規範），本篇用實際編出來的程式驗證其中一部分，沒驗證到的地方會註明
> 對應檔案：每個實驗程式與 gdb 腳本的完整內容都寫在它出現的那一節；同樣的檔案也放在
> [`small-project/s1-05-asm-calls/`](../small-project/s1-05-asm-calls/)（會進 git，編譯產物不進），作業也在同一個資料夾
> 相關：[x86-64組合語言入門（一）](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md)（第 4 課：暫存器、`mov`／`lea`、旗標）、
> [陣列、字串與指標運算](./陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md)（第 3 課：堆疊上的陣列、`stack smashing detected`）、
> [gdb完整命令參考](./gdb完整命令參考.md)

**一句話結論：函式呼叫在機器層級只是「堆疊」加上四個指令。
`call f` ＝ 把**返回位址**（`call` 下一條指令的位址）推進堆疊，再跳到 `f`；`ret` ＝ 從堆疊頂端取出一個位址，跳回去。
`push`／`pop` 讓 `%rsp` 每次移動 8 byte，堆疊往**低位址**長。
每個函式用 `%rbp` 標出自己那一塊（堆疊框），區域變數用 `-8(%rbp)` 這種寫法存取。
參數照順序放 `%rdi`、`%rsi`、`%rdx`、`%rcx`、`%r8`、`%r9`，第 7 個起放堆疊，回傳值放 `%rax`。
`%rbx`、`%rbp`、`%r12`～`%r15` 由**被呼叫的函式**負責還原（所以要跨過 `call` 保存的值會放在這些暫存器裡），其他的呼叫完可能被改掉。**

---

## 目錄

- [零、開課複習：第 4 課卡住的五個觀念](#零開課複習第-4-課卡住的五個觀念)
- [一、本課在做什麼：函式呼叫只是幾條指令](#一本課在做什麼函式呼叫只是幾條指令)
- [二、堆疊：`%rsp` 指著頂端，往低位址長](#二堆疊rsp-指著頂端往低位址長)
- [三、`call` 與 `ret`：用 gdb 一條一條走](#三call-與-ret用-gdb-一條一條走)
- [四、堆疊框：`pushq %rbp; movq %rsp, %rbp` 與 `leave`](#四堆疊框pushq-rbp-movq-rsp-rbp-與-leave)
- [五、參數與回傳值放哪裡](#五參數與回傳值放哪裡)
- [六、誰負責保存暫存器，以及 16 byte 對齊](#六誰負責保存暫存器以及-16-byte-對齊)
- [七、`printf` 前的 `movl $0, %eax`：可變參數函式的 `%al`](#七printf-前的-movl-0-eax可變參數函式的-al)
- [八、本課用到的工具、選項與完整清單在哪](#八本課用到的工具選項與完整清單在哪)
- [九、動手作業](#九動手作業)
- [自我測驗](#自我測驗)

---

## 零、開課複習：第 4 課卡住的五個觀念

第 4 課的作業與測驗批改後，有五個觀念記成「第 5 課開頭先複習」。本課會一直用到前兩個，所以先講回來（今天的 10 題複習額度已經用完，這裡只講解、不另外出題）：

1. **寫 32 位元暫存器會清掉高 32 位元，寫 16／8 位元時高位不動**。這是 CPU 指令的規則：`movl $1, %eax` 之後整個 `%rax` 是 1；`movw $1, %ax` 只改低 16 位元。
   gdb 的 `set $edi = 1234` 不是 CPU 指令，它只改那 32 位元，**不會**清高位。所以本課看到 `mov $0x1,%edi` 傳一個 `long` 參數 1，是靠「寫 `%edi` 會清高位」，整個 `%rdi` 就是 1。
   出處：[第 4 課第二節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#二暫存器cpu-裡的-16-個變數)〈寫低位的名字時，高位會怎樣？〉。
2. **`break 函式名` 會跳過函式開頭的準備指令，`break *函式名` 停在第一條指令**。本課第三節就是用 `break *` 停在 `call` 上、看 `%rsp` 怎麼變，也會看到「停在第一條指令時 gdb 印出的參數是錯的」。
   出處：[第 4 課第三節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#三指令的文法助憶符大小後綴運算元)〈上面 `break *widen_char+4` 的 `*`：為什麼要加？〉。
3. **OF 要用有號的解讀來算**：兩個有號數相加減，數學上的結果超出範圍才是 OF ＝ 1。**`setl`／`jl` 看的是 SF ≠ OF**，不是只看 SF。
   出處：[第 4 課第五節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈回頭看條件碼表：為什麼有號的「小於」是 SF ≠ OF，而不是只看 SF？〉。
4. **CF 是無號減法的「借位」**：`a − b` 需要借位 ⇔ 無號的 `a < b`，所以 `jb`（below）看 CF。加法器把「進位」直接存成 CF，減法時剛好反過來，存的是「借位」。
   出處：同上〈上面③：3 − 5 在 CPU 裡實際上是怎麼算的？〉。
5. **`and` 會把結果寫回目的運算元，`test` 只設旗標、不寫回**。之後還要用到結果時用 `and`，只想判斷時用 `test`。
   出處：[第 4 課第七節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#七逐行讀-sum_to-的--o2-版)〈上表的 `andl` 和 `testl`：看起來一樣，差在哪？〉。

---

## 一、本課在做什麼：函式呼叫只是幾條指令

第 4 課讀懂了函式**裡面**的指令。本課讀懂函式**之間**怎麼接起來：

- 呼叫的時候怎麼跳過去，跳完怎麼回得來（第二、三節：堆疊、`call`／`ret`）；
- 每個函式的區域變數放在哪裡，為什麼 `-O0` 開頭都是 `pushq %rbp; movq %rsp, %rbp`（第四節）；
- 參數和回傳值怎麼傳（第五節）；
- 呼叫別人之後，自己暫存器裡的值還在不在（第六節）。

第 3、4 兩點的規則叫**呼叫慣例**（calling convention）。它不是 CPU 規定的，是所有編譯器、函式庫共同遵守的約定（Linux x86-64 用的是 System V AMD64 ABI，ABI 是 Application Binary Interface，應用程式二進位介面）。
大家都照同一套規則，gcc 編的程式才能呼叫別人編好的 `libc.so.6` 裡的 `printf`。

這也是 kernel 維護者每天會用到的東西：kernel 當掉時印出的 call trace，就是從堆疊上一個一個找回「返回位址」，倒推出是誰呼叫了誰。

---

## 二、堆疊：`%rsp` 指著頂端，往低位址長

**堆疊**（stack）是 kernel 在程式啟動時給的一段記憶體（[第 3 課](./陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md)在 `info proc mappings` 裡看過的 `[stack]`）。
CPU 用 `%rsp`（stack pointer，堆疊指標）記住**目前的頂端**在哪裡。兩個規則：

1. **堆疊往低位址長**：放東西進去時，`%rsp` 變小；拿出來時，`%rsp` 變大。
2. **`%rsp` 指著最後放進去的那一格**（頂端那格本身，不是它上面的空位）。

`push` 和 `pop` 就是照這兩條規則動作的指令，每次搬 8 byte。它們等於下面兩條指令合在一起（這是 Intel 手冊對 `push`／`pop` 的定義，下一節用 gdb 驗證）：

| 指令 | 等於 | 白話 |
|---|---|---|
| `push %rbp` | `sub $8, %rsp`，再 `mov %rbp, (%rsp)` | 頂端往下挪 8 byte，把 `%rbp` 寫進新的頂端 |
| `pop %rbp` | `mov (%rsp), %rbp`，再 `add $8, %rsp` | 把頂端那 8 byte 讀進 `%rbp`，頂端往上挪 8 byte |

`(%rsp)` 是第 4 課的寫法：括號就是 C 的 `*`，「`%rsp` 存的位址上的那個值」。

---

## 三、`call` 與 `ret`：用 gdb 一條一條走

`call` 和 `ret` 也是用堆疊做事：

| 指令 | 等於 | 白話 |
|---|---|---|
| `call f` | `push 返回位址`，再 `jmp f` | 返回位址 ＝ `call` **下一條**指令的位址。先記下來，再跳過去 |
| `ret` | `pop` 到 `%rip` | 把堆疊頂端那 8 byte 當成位址，跳過去 |

`%rip` 是第 4 課提過的「下一條要執行的指令的位址」（instruction pointer，gdb 裡也叫 `$pc`）。沒有指令能直接 `mov` 到 `%rip`，`ret` 等於是「從堆疊取一個值放進 `%rip`」。

所以 `ret` **不知道**自己是被誰呼叫的，它只是盲目地相信堆疊頂端那 8 byte。第 3 課看到的 `stack smashing detected` 保護的就是這件事：如果陣列越界把返回位址蓋掉，`ret` 會跳到一個亂七八糟的地方。

### 實驗：`frame.c`

`frame.c`（檔案：[`frame.c`](../small-project/s1-05-asm-calls/frame.c)）完整原始碼：

```c
#include <stdio.h>

long add3(long a, long b, long c)
{
    long s = a + b;
    return s + c;
}

int main(void)
{
    long r = add3(1, 2, 3);
    printf("%ld\n", r);
    return 0;
}
```

```
$ gcc -O0 -g -fno-asynchronous-unwind-tables frame.c -o frame
$ ./frame
6
$ objdump -d --no-show-raw-insn frame
```

`objdump` 輸出中的 `add3` 和 `main`（右邊的中文說明是我加的）：

```asm
0000000000001149 <add3>:
    1149:	endbr64                      # 間接跳躍保護的標記（第 4 課第六節），跟本課無關
    114d:	push   %rbp                  # 存起 main 的 %rbp
    114e:	mov    %rsp,%rbp             # %rbp = 這個函式的堆疊框基準
    1151:	mov    %rdi,-0x18(%rbp)      # 參數 a（在 %rdi）存到 -0x18(%rbp)
    1155:	mov    %rsi,-0x20(%rbp)      # 參數 b（在 %rsi）存到 -0x20(%rbp)
    1159:	mov    %rdx,-0x28(%rbp)      # 參數 c（在 %rdx）存到 -0x28(%rbp)
    115d:	mov    -0x18(%rbp),%rdx      # %rdx = a
    1161:	mov    -0x20(%rbp),%rax      # %rax = b
    1165:	add    %rdx,%rax             # %rax = a + b
    1168:	mov    %rax,-0x8(%rbp)       # s = a + b，存到 -0x8(%rbp)
    116c:	mov    -0x8(%rbp),%rdx       # %rdx = s
    1170:	mov    -0x28(%rbp),%rax      # %rax = c
    1174:	add    %rdx,%rax             # %rax = s + c：回傳值放在 %rax
    1177:	pop    %rbp                  # 還原 main 的 %rbp
    1178:	ret                          # 回到 main

0000000000001179 <main>:
    1179:	endbr64
    117d:	push   %rbp                  # 存起呼叫 main 的那個函式的 %rbp
    117e:	mov    %rsp,%rbp
    1181:	sub    $0x10,%rsp            # 替區域變數 r 留空間（為什麼是 16 不是 8：第六節）
    1185:	mov    $0x3,%edx             # 第 3 個參數 = 3（寫 %edx 會清掉 %rdx 的高 32 位元）
    118a:	mov    $0x2,%esi             # 第 2 個參數 = 2
    118f:	mov    $0x1,%edi             # 第 1 個參數 = 1
    1194:	call   1149 <add3>           # 呼叫 add3
    1199:	mov    %rax,-0x8(%rbp)       # r = 回傳值
    119d:	mov    -0x8(%rbp),%rax
    11a1:	mov    %rax,%rsi             # printf 的第 2 個參數 = r
    11a4:	lea    0xe59(%rip),%rax      # %rax = 字串 "%ld\n" 的位址
    11ab:	mov    %rax,%rdi             # printf 的第 1 個參數 = 字串位址
    11ae:	mov    $0x0,%eax             # 第七節
    11b3:	call   1050 <printf@plt>
    11b8:	mov    $0x0,%eax             # return 0;
    11bd:	leave                        # 第四節
    11be:	ret
```

`call` 那一行在 `main+27`（`0x1194 − 0x1179 = 0x1b = 27`），它的下一條指令在 `0x1199`。所以**返回位址應該是 `0x1199`**（執行時再加上載入位址）。用 gdb 驗證。

`trace.gdb`（檔案：[`trace.gdb`](../small-project/s1-05-asm-calls/trace.gdb)），gdb 的命令稿，一行一個命令，`echo` 只是印標題：

```
set disassemble-next-line off
break *main+27
run
echo \n=== 1. call 之前\n
x/i $pc
info registers rdi rsi rdx rsp rbp
x/2gx $rsp
stepi
echo \n=== 2. call 之後（進到 add3 第一條）\n
x/i $pc
info registers rsp rip
x/gx $rsp
stepi
stepi
echo \n=== 3. push %rbp 之後\n
x/i $pc
info registers rsp rbp
x/2gx $rsp
stepi
echo \n=== 4. mov %rsp,%rbp 之後\n
info registers rsp rbp
break *add3+47
continue
echo \n=== 5. 停在 ret（pop %rbp 已執行）\n
x/i $pc
info registers rax rsp rbp
x/gx $rsp
stepi
echo \n=== 6. ret 之後，回到 main\n
x/i $pc
info registers rsp rip
continue
```

用到的 gdb 命令（第八節有完整清單的位置）：`break *位址` 停在那一條指令；`stepi` 執行剛好一條機器指令（遇到 `call` 會跟進去）；
`x/i $pc` 印出下一條要執行的指令；`info registers 名字…` 印出指定的暫存器；`x/2gx $rsp` 從 `%rsp` 指的位址開始，印 2 個 `g`（8 byte）、用 `x`（十六進位）顯示。

---

**上面的 `info registers`：輸出的三欄各是什麼**

> 2026-10-09 課後提問：「在 gdb 這三欄是什麼意思」（貼的是 `info registers` 印出的 `rax`～`rsi` 五列）

拿上面的 `frame.c`（`gcc -O0 -g -o frame frame.c`）停在 `main` 的第一條指令，不給名字，列出全部暫存器：

```
$ gdb -batch -ex 'break *main' -ex run -ex 'info registers' ./frame
rax            0x555555555179      93824992235897
rbx            0x7fffffffd548      140737488344392
rcx            0x555555557dc0      93824992247232
rdx            0x7fffffffd558      140737488344408
rsi            0x7fffffffd548      140737488344392
rdi            0x1                 1
rbp            0x7fffffffd4c0      0x7fffffffd4c0
rsp            0x7fffffffd428      0x7fffffffd428
r8             0x0                 0
…（r9～r15 省略，格式同 r8）
rip            0x555555555179      0x555555555179 <main>
eflags         0x246               [ PF ZF IF ]
cs             0x33                51
…（ss、ds、es、fs、gs、fs_base、gs_base 省略）
```

| 欄 | 是什麼 | 例子 |
|---|---|---|
| 第 1 欄 | 暫存器名字（gdb 裡寫成 `$rax`） | `rax` |
| 第 2 欄 | 暫存器裡的值，一律用**十六進位**印（raw，原始位元） | `0x555555555179` |
| 第 3 欄 | **同一個值**，用 gdb 認為這個暫存器「天然的」格式（natural format）再印一次，格式依暫存器的**型別**決定 | 見下 |

第 2、3 欄是同一個值的兩種寫法，不是兩個不同的數。第 3 欄的格式由 gdb 給每個暫存器的型別決定（用 `ptype $rax` 這種命令問得到）：

- `rax`、`rbx`、`rdi`、`r8`… 型別是 `int64_t`（整數），第 3 欄印**十進位**：`0x1` 印成 `1`，`0x555555555179` 印成 `93824992235897`。
- `rbp`、`rsp` 型別是 `void *`（指標），指標的天然格式就是十六進位，所以第 3 欄跟第 2 欄**看起來一樣**。
- `rip` 型別是 `void (*)()`（指向程式碼的指標），除了十六進位，還在 `<>` 裡標出它指在哪個函式：`<main>`。停在 `main+27` 時就會印 `<main+27>`。
- `eflags` 型別是一組旗標，第 3 欄印出**哪些旗標是 1**：`0x246` 是 `[ PF ZF IF ]`（第 4 課[第五節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)講過的旗標）。

所以讀 `rax` 那一列時看第 2 欄就好：十進位的 `93824992235897` 沒有意義，因為這個值其實是位址。`info symbol $rax` 印出 `main in section .text`，也就是說剛好是 `main` 的位址（是呼叫 `main` 的 glibc 留下來的，不是 `main` 自己算的）。

這一頁也能用本課的觀念讀：停在 `main` 的第一條指令時，`main` 剛被呼叫，所以參數暫存器裡是 `main` 的參數（`main` 其實會收到 `argc`、`argv`、`envp` 三個參數，`frame.c` 寫成 `main(void)` 只是不用它們）：

- `rdi = 1`：`argc`，命令列只有程式名稱一個字串。
- `rsi = 0x7fffffffd548`：`argv`，指向字串指標的陣列。`x/s *(char **)$rsi` 印出程式的完整路徑。
- `rdx = 0x7fffffffd558`：`envp`，指向環境變數的陣列。`x/s *(char **)$rdx` 印出第一個環境變數。它比 `argv` 大 16，因為中間隔著 `argv[0]` 和結尾的 `NULL` 兩個 8 byte 指標。

`info registers` 只列整數暫存器，`info all-registers` 連浮點與 `%xmm` 暫存器都列。完整說明見 [gdb完整命令參考第五節](./gdb完整命令參考.md#五info-的全部-65-個子命令)。

*（追問補充結束，以下回到主線）*

---

```
$ gdb -q -batch -x trace.gdb ./frame
```

實際輸出（刪掉 gdb 啟動時的 `[Thread debugging ...]` 訊息）：

```
Breakpoint 1 at 0x1194: file frame.c, line 11.

Breakpoint 1, 0x0000555555555194 in main () at frame.c:11
11	    long r = add3(1, 2, 3);

=== 1. call 之前
=> 0x555555555194 <main+27>:	call   0x555555555149 <add3>
rdi            0x1                 1
rsi            0x2                 2
rdx            0x3                 3
rsp            0x7fffffffd410      0x7fffffffd410
rbp            0x7fffffffd420      0x7fffffffd420
0x7fffffffd410:	0x00007fffffffd500	0x00007fffffffd548
add3 (a=0, b=0, c=0) at frame.c:4
4	{

=== 2. call 之後（進到 add3 第一條）
=> 0x555555555149 <add3>:	endbr64
rsp            0x7fffffffd408      0x7fffffffd408
rip            0x555555555149      0x555555555149 <add3>
0x7fffffffd408:	0x0000555555555199
0x000055555555514d	4	{
0x000055555555514e	4	{

=== 3. push %rbp 之後
=> 0x55555555514e <add3+5>:	mov    %rsp,%rbp
rsp            0x7fffffffd400      0x7fffffffd400
rbp            0x7fffffffd420      0x7fffffffd420
0x7fffffffd400:	0x00007fffffffd420	0x0000555555555199
0x0000555555555151	4	{

=== 4. mov %rsp,%rbp 之後
rsp            0x7fffffffd400      0x7fffffffd400
rbp            0x7fffffffd400      0x7fffffffd400
Breakpoint 2 at 0x555555555178: file frame.c, line 7.

Breakpoint 2, 0x0000555555555178 in add3 (a=1, b=2, c=3) at frame.c:7
7	}

=== 5. 停在 ret（pop %rbp 已執行）
=> 0x555555555178 <add3+47>:	ret
rax            0x6                 6
rsp            0x7fffffffd408      0x7fffffffd408
rbp            0x7fffffffd420      0x7fffffffd420
0x7fffffffd408:	0x0000555555555199
0x0000555555555199 in main () at frame.c:11
11	    long r = add3(1, 2, 3);

=== 6. ret 之後，回到 main
=> 0x555555555199 <main+32>:	mov    %rax,-0x8(%rbp)
rsp            0x7fffffffd410      0x7fffffffd410
rip            0x555555555199      0x555555555199 <main+32>
6
```

逐段讀：

- **1. `call` 之前**：三個參數已經在 `%rdi`、`%rsi`、`%rdx`（1、2、3）。`%rsp` 是 `0x…d410`。程式是 PIE，載入位址是 `0x555555554000`，所以 `objdump` 的 `0x1194` 在這裡是 `0x555555555194`。
- **2. `call` 之後**：`%rsp` 從 `d410` 變成 **`d408`，少了 8**；新的頂端 `d408` 裡放的是 **`0x555555555199`**，就是 `call` 下一條指令 `main+32` 的位址。`%rip` 變成 `add3` 的開頭。這就是「`push 返回位址` + `jmp add3`」。
- **3. `push %rbp` 之後**：`%rsp` 再少 8，變成 `d400`；頂端 `d400` 裡是 `0x7fffffffd420`，就是 `main` 的 `%rbp`。`x/2gx` 印的第二格 `d408` 還是返回位址：兩樣東西疊在一起，越晚放的位址越低。
- **4. `mov %rsp,%rbp` 之後**：`%rbp` 也變成 `d400`。從這裡開始，`add3` 用 `%rbp` 當基準存取自己的變數（第四節）。
- **5. 停在 `ret`**：`pop %rbp` 已經執行，`%rbp` 還原成 `main` 的 `d420`，`%rsp` 回到 `d408`，頂端又是返回位址。回傳值 6 已經在 `%rax`。
- **6. `ret` 之後**：`%rip` 變成 `0x555555555199`（`main+32`），`%rsp` 回到 **`d410`，跟 `call` 之前一模一樣**。`call` 推進去的 8 byte 被 `ret` 拿走了。

**gdb 在第 1 段結尾印的 `add3 (a=0, b=0, c=0)` 是錯的**：那時 `stepi` 剛進到 `add3` 的第一條指令。gdb 照除錯資訊去堆疊上 `a`、`b`、`c` 的位置讀（序言跑完後就是 `-0x18(%rbp)`、`-0x20(%rbp)`、`-0x28(%rbp)` 那三格），可是那時參數還在暫存器裡、還沒存進去，讀到的是那幾格裡原本的舊值。（2026-10-19 更正：原本還寫了「而且 `%rbp` 還是 `main` 的值」，這是錯的——gdb 不是用 `%rbp` 算位置，位址本身是對的，見下方追問補充。）
到了第 5 段，堆疊框已經建好、參數也存好了，gdb 就印出正確的 `a=1, b=2, c=3`。這正是 `break add3`（不加 `*`）要跳過開頭準備指令的原因（零、第 2 點）。

---

**gdb 為什麼去堆疊讀參數，而不是讀暫存器**

> 2026-10-19 課後提問：「gdb 照除錯資訊，去 -0x18(%rbp)、-0x20(%rbp)、-0x28(%rbp) 讀 a、b、c。爲什麼gdb是去讀堆疊上的參數,而不是寄存器?」（指上一段「gdb 在第 1 段結尾印的 `add3 (a=0, b=0, c=0)` 是錯的」）

**1. gdb 不看程式碼，只照除錯資訊說的位置去讀。** `-g` 產生的除錯資訊（格式叫 DWARF，存在執行檔裡一組 `.debug_*` 區段）替每個變數記了一筆「它放在哪裡」（`DW_AT_location`）。
gdb 印 `a` 時不會去分析 `add3` 的指令、推算「現在 `a` 應該還在 `%rdi`」，它只照這一筆去讀。所以問題變成：**gcc 為什麼把 `a` 的位置記成堆疊？**

用 `readelf --debug-dump=info` 看 `frame.c`（`gcc -g -O0 -o frame frame.c`）的除錯資訊，只留名字與位置：

```
$ readelf --debug-dump=info frame | grep -A8 "DW_TAG_subprogram\|DW_TAG_formal_parameter\|DW_TAG_variable" | grep -E "DW_TAG|DW_AT_name|DW_AT_location|frame_base"
...
 <1><be>: Abbrev Number: 12 (DW_TAG_subprogram)
    <bf>   DW_AT_name        : (indirect string, offset: 0x12): add3
 <2><dc>: Abbrev Number: 2 (DW_TAG_formal_parameter)
    <dd>   DW_AT_name        : a
    <e4>   DW_AT_location    : 2 byte block: 91 58 	(DW_OP_fbreg: -40)
 <2><e7>: Abbrev Number: 2 (DW_TAG_formal_parameter)
    <e8>   DW_AT_name        : b
    <ef>   DW_AT_location    : 2 byte block: 91 50 	(DW_OP_fbreg: -48)
 <2><f2>: Abbrev Number: 2 (DW_TAG_formal_parameter)
    <f3>   DW_AT_name        : c
    <fa>   DW_AT_location    : 2 byte block: 91 48 	(DW_OP_fbreg: -56)
 <2><fd>: Abbrev Number: 3 (DW_TAG_variable)
    <fe>   DW_AT_name        : s
    <105>   DW_AT_location    : 2 byte block: 91 68 	(DW_OP_fbreg: -24)
```

（`DW_TAG_formal_parameter` 是參數、`DW_TAG_variable` 是區域變數；`readelf` 的 `-w`／`--debug-dump` 全部子項見 [readelf完整選項參考.md](./readelf完整選項參考.md#26-除錯資訊dwarf)。）

`a` 的位置只有**一筆**：`DW_OP_fbreg: -40`，意思是「堆疊框基準（frame base）往下 40 byte」，整個 `add3` 從頭到尾都用這一筆。
原因是 `-O0` 的做法：序言一跑完，參數就存進堆疊，之後**每次用 `a` 都回堆疊讀寫**（第四節的組合語言 `mov %rdi,-0x18(%rbp)` 之後只用 `-0x18(%rbp)`），`%rdi` 隨時可以拿去做別的事。
所以堆疊那格才是 `a` 的「家」，gcc 就只記這一筆；序言前那兩三條指令「`a` 其實還在 `%rdi`」的空檔，`-O0` 不另外記。

**2. 「堆疊框基準」不是 `%rbp`，是 CFA。** 同一份除錯資訊裡 `add3` 有一行 `DW_AT_frame_base : (DW_OP_call_frame_cfa)`：基準是 **CFA**（Canonical Frame Address，標準堆疊框位址），定義是「呼叫者執行 `call` 之前的 `%rsp`」。
每一條指令的 CFA 怎麼算，記在 `.eh_frame` 區段（第 4 課 `readelf -S` 看過這個區段名），`readelf --debug-dump=frames-interp` 可以看：

```
$ readelf --debug-dump=frames-interp frame
...
00000088 000000000000001c 0000008c FDE cie=00000000 pc=0000000000001149..0000000000001179
   LOC           CFA      rbp   ra    
0000000000001149 rsp+8    u     c-8   
000000000000114e rsp+16   c-16  c-8   
0000000000001151 rbp+16   c-16  c-8   
0000000000001178 rsp+8    c-16  c-8   
```

`pc=1149..1179` 是 `add3`。每一列是「從這個位址開始，CFA 怎麼算」：在 `1149`（第一條指令 `endbr64`）時 CFA ＝ `%rsp + 8`（`call` 推了 8 byte，加回去就是 `call` 之前的 `%rsp`）；`push %rbp` 之後變 `%rsp + 16`；`mov %rsp,%rbp` 之後改用 `%rbp + 16`。（`ra` 欄 `c-8` 是返回位址存在 CFA − 8，`rbp` 欄 `c-16` 是舊 `%rbp` 存在 CFA − 16，`u` 是還沒存。）
序言跑完後，`a` ＝ CFA − 40 ＝ `%rbp` ＋ 16 − 40 ＝ `%rbp` − 24 ＝ **`-0x18(%rbp)`**，跟組合語言一致。

所以停在第一條指令時，**gdb 算出的位址是對的**，就是之後 `a` 會被存進去的那一格；印錯只是因為那格還沒寫進 `a`，裡面是舊的值。用 `where-a.gdb`（檔案：[`where-a.gdb`](../small-project/s1-05-asm-calls/where-a.gdb)）確認：

```
break *add3
run
info registers rsp rbp rdi rsi rdx
info address a
info address c
print &a
print &c
print a
info frame
```

```
$ gdb -q -batch -x where-a.gdb ./frame
Breakpoint 1 at 0x1149: file frame.c, line 4.

Breakpoint 1, add3 (a=0, b=0, c=0) at frame.c:4
4	{
rsp            0x7fffffffd448      0x7fffffffd448
rbp            0x7fffffffd460      0x7fffffffd460
rdi            0x1                 1
rsi            0x2                 2
rdx            0x3                 3
Symbol "a" is a complex DWARF expression:
     0: DW_OP_fbreg -40
.
Symbol "c" is a complex DWARF expression:
     0: DW_OP_fbreg -56
.
$1 = (long *) 0x7fffffffd428
$2 = (long *) 0x7fffffffd418
$3 = 0
Stack level 0, frame at 0x7fffffffd450:
 rip = 0x555555555149 in add3 (frame.c:4); saved rip = 0x555555555199
 called by frame at 0x7fffffffd470
 source language c.
 Arglist at 0x7fffffffd440, args: a=0, b=0, c=0
 Locals at 0x7fffffffd440, Previous frame's sp is 0x7fffffffd450
 Saved registers:
  rip at 0x7fffffffd448
```

（這次執行的堆疊位址跟本節前面差了 `0x40`，堆疊起點會隨執行環境不同而變，比的是同一次執行裡的相對位置。）

- `info frame` 第一行 `frame at 0x7fffffffd450` 就是 CFA ＝ `%rsp`（`d448`）＋ 8。
- `&a` ＝ `d428` ＝ `d450 − 40`，是照 CFA 算的。如果照 `%rbp`（還是 `main` 的 `d460`）算 `-0x18(%rbp)`，會是 `d448`——那是返回位址那格，值不是 0。所以 gdb 沒有用 `%rbp`。
- 真正的值在 `%rdi`／`%rsi`／`%rdx`：1、2、3。

**3. 對照：`-O2` 的除錯資訊就會記暫存器。** 最佳化後變數常常只待在暫存器裡，gcc 會改記「位置清單」（location list）：哪段位址範圍在哪個暫存器。用 `-fno-inline` 讓 `add3` 不被併進 `main`：

```
$ gcc -g -O2 -fno-inline -o frame2 frame.c
$ objdump -d --no-show-raw-insn frame2 | sed -n '/<add3>:/,/ret/p'
0000000000001190 <add3>:
    1190:	endbr64
    1194:	add    %rsi,%rdi
    1197:	lea    (%rdi,%rdx,1),%rax
    119b:	ret
```

`add3` 完全不碰堆疊：`add %rsi,%rdi` 把 `a + b` 存回 `%rdi`（`s` 跟 `a` 共用 `%rdi`），`lea (%rdi,%rdx,1),%rax` 算 `%rdi + %rdx` 放進回傳值 `%rax`（`lea` 只算位址不讀記憶體，第 4 課教過）。除錯資訊也跟著變：

```
$ readelf --debug-dump=info frame2 | sed -n '/DW_AT_name.*add3/,/DW_TAG_variable/p' | grep -E "name|location|frame_base"
    <12e>   DW_AT_name        : (indirect string, offset: 0xb0): add3
    <148>   DW_AT_frame_base  : 1 byte block: 9c 	(DW_OP_call_frame_cfa)
    <14f>   DW_AT_name        : a
    <158>   DW_AT_location    : 0x40 (location list)
    <161>   DW_AT_name        : b
    <168>   DW_AT_location    : 1 byte block: 54 	(DW_OP_reg4 (rsi))
    <16b>   DW_AT_name        : c
    <172>   DW_AT_location    : 1 byte block: 51 	(DW_OP_reg1 (rdx))
```

`b`、`c` 直接記成暫存器 `rsi`、`rdx`（`DW_OP_reg4` 的 4 是 DWARF 自己替暫存器排的編號，`readelf` 在括號裡翻成名字）；`a` 因為 `%rdi` 在 `1194` 之後被改成 `a + b`，記成一張位置清單。`where-a-O2.gdb`（檔案：[`where-a-O2.gdb`](../small-project/s1-05-asm-calls/where-a-O2.gdb)）：

```
break *add3
run
info address a
print a
print b
print c
```

```
$ gdb -q -batch -x where-a-O2.gdb ./frame2
Breakpoint 1 at 0x1190: file frame.c, line 4.

Breakpoint 1, add3 (a=a@entry=1, b=b@entry=2, c=c@entry=3) at frame.c:4
4	{
Symbol "a" is multi-location:
  Base address 0x1190  Range 0x555555555190-0x555555555197: a variable in $rdi
  Range 0x555555555197-0x55555555519c: a complex DWARF expression:
     0: DW_OP_entry_value
       2: DW_OP_reg5 [$rdi]
     3: DW_OP_stack_value

.
$1 = 1
$2 = 2
$3 = 3
```

- `1190`～`1197`：`a` 在 `$rdi`，所以停在第一條指令也印對 `a=1`。
- `1197` 以後：`%rdi` 已經變成 `a + b`，除錯資訊改說「`a` ＝ 進函式那一刻 `%rdi` 的值」（`DW_OP_entry_value`），gdb 要從呼叫者那邊推回來；印出來的 `a@entry=1` 的 `@entry` 就是「進函式時的值」。

結論：gdb 讀哪裡，**完全由編譯器寫的除錯資訊決定**。`-O0` 只記堆疊那一格（因為那是變數的家），所以序言前讀到舊值；`-O2` 記了暫存器與位置清單，反而在第一條指令就讀得對。

（本段用到 gdb 的 `info address`、`info frame`，完整的 `info` 子命令見 [gdb完整命令參考.md](./gdb完整命令參考.md)；`readelf` 用到 `--debug-dump` 的 `info`、`frames-interp` 兩個子項。）

*（追問補充結束，以下回到主線）*

---

### 停在函式入口時，怎麼知道是誰呼叫的

既然 `call` 一進來，頂端就是返回位址，那麼停在函式**第一條指令**時，`x/gx $rsp` 就能看出呼叫者。`who.gdb`（檔案：[`who.gdb`](../small-project/s1-05-asm-calls/who.gdb)）：

```
break *add3
run
x/gx $rsp
info symbol *(long *)$rsp
```

`info symbol 位址` 會回答「這個位址在哪個符號裡、偏移多少」；`*(long *)$rsp` 是 C 的寫法：把 `$rsp` 當成 `long *`、取出它指的 8 byte。

```
$ gdb -q -batch -x who.gdb ./frame
Breakpoint 1 at 0x1149: file frame.c, line 4.

Breakpoint 1, add3 (a=0, b=0, c=0) at frame.c:4
4	{
0x7fffffffd408:	0x0000555555555199
main + 32 in section .text of /tmp/…/frame
```

（最後一行的路徑是本機的暫存目錄，這裡省略。）返回位址在 `main + 32`，所以呼叫者是 `main`。gdb 的 `bt`（backtrace）做的事情本質上一樣：沿著堆疊一層一層找返回位址。

---

## 四、堆疊框：`pushq %rbp; movq %rsp, %rbp` 與 `leave`

**堆疊框**（stack frame）是一個函式在堆疊上用的那一塊：存起來的舊 `%rbp`、區域變數、要傳給別人的多餘參數。`-O0` 的函式開頭都是同兩條指令，叫**序言**（prologue）：

```asm
push   %rbp          # 1. 把呼叫者的 %rbp 存在堆疊上（之後要還原）
mov    %rsp,%rbp     # 2. 把「現在的頂端」當成自己的基準點
```

之後整個函式裡，`%rsp` 可能還會動（`push`、`sub`），但 `%rbp` 不動，所以區域變數用 `%rbp` 加一個固定的負數存取最方便：`-0x8(%rbp)` 永遠是 `s`。
結尾的**尾聲**（epilogue）把這兩步倒回來：只用到 `push` 的函式寫 `pop %rbp`，有 `sub` 過 `%rsp` 的函式寫 `leave`。

### `leave`：一條指令做兩件事

`leave` 等於 `mov %rbp, %rsp`，再 `pop %rbp`：先把 `%rsp` 拉回基準點（中間用 `sub` 開的空間一次丟掉），再還原呼叫者的 `%rbp`。
`main` 有 `sub $0x10,%rsp`，所以用 `leave`；`add3` 沒動過 `%rsp`，`pop %rbp` 就夠了。

`leave.gdb`（檔案：[`leave.gdb`](../small-project/s1-05-asm-calls/leave.gdb)），停在 `main` 的 `leave`（`0x11bd − 0x1179 = 68`）：

```
break *main+68
run
x/i $pc
info registers rsp rbp
x/gx $rbp
stepi
x/i $pc
info registers rsp rbp
x/gx $rsp
continue
```

```
$ gdb -q -batch -x leave.gdb ./frame
Breakpoint 1 at 0x11bd: file frame.c, line 14.

Breakpoint 1, main () at frame.c:14
14	}
=> 0x5555555551bd <main+68>:	leave
rsp            0x7fffffffd410      0x7fffffffd410
rbp            0x7fffffffd420      0x7fffffffd420
0x7fffffffd420:	0x00007fffffffd4c0
0x00005555555551be	14	}
=> 0x5555555551be <main+69>:	ret
rsp            0x7fffffffd428      0x7fffffffd428
rbp            0x7fffffffd4c0      0x7fffffffd4c0
0x7fffffffd428:	0x00007ffff7c2a1ca
6
```

- `leave` 之前：`%rbp` 是 `d420`，那一格存著呼叫 `main` 的函式的 `%rbp`：`0x7fffffffd4c0`。
- `leave` 之後：`%rsp` ＝ `d420 + 8` ＝ **`d428`**（先拉回 `d420`，再 `pop` 走 8 byte），`%rbp` ＝ **`d4c0`**（剛才那格的值）。頂端 `d428` 是 `main` 自己的返回位址 `0x7ffff7c2a1ca`，在 libc 裡：`main` 也是被別人 `call` 的（第 1 課：程式不是從 `main` 開始的）。

### 畫出來：停在 `add3` 裡面時的堆疊

把第三節與本節的數字畫在一起（位址只寫最後 3 位，`d4xx` 都是 `0x7fffffffd4xx`）。上面是高位址，往下長：

```
位址     內容                            誰的
d428    0x7ffff7c2a1ca（返回 libc）       ┐
d420    0x7fffffffd4c0（舊 %rbp）  ← main 的 %rbp
d418    r（-0x8(%rbp)）                    │ main 的堆疊框
d410    （對齊用，沒用到）                  ┘
d408    0x555555555199（返回 main+32）    ┐
d400    0x7fffffffd420（main 的 %rbp） ← add3 的 %rbp 和 %rsp
d3f8    s       （-0x8(%rbp)）              │ add3 的堆疊框
d3f0    （沒用到）                          │
d3e8    a = 1   （-0x18(%rbp)）             │
d3e0    b = 2   （-0x20(%rbp)）             │
d3d8    c = 3   （-0x28(%rbp)）             ┘
```

每個堆疊框的 `%rbp` 那一格都存著**上一層的 `%rbp`**：`d400` 裡是 `d420`，`d420` 裡是 `d4c0`……像一串鏈子。所以從任何一層出發，都可以一路往回找到每一層的堆疊框和返回位址。

### 上圖 `add3` 的變數在 `%rsp` 的「下面」：紅區

仔細看上圖：`add3` 的 `%rsp` 是 `d400`，可是 `a`、`b`、`c`、`s` 放在 `d3d8`～`d3f8`，**比 `%rsp` 還低**，照[第二節](#二堆疊rsp-指著頂端往低位址長)的兩條規則推下去，那裡不屬於「堆疊裡」（第二節沒有直接寫這句，怎麼推出來見本小節最後的追問補充）。用 `redzone.gdb` 確認（檔案：[`redzone.gdb`](../small-project/s1-05-asm-calls/redzone.gdb)），停在 `s` 剛存完之後（`add3+35`，`0x116c`）：

```
break *add3+35
run
info registers rsp rbp
print &a
print &s
x/5gx $rbp-0x28
```

```
$ gdb -q -batch -x redzone.gdb ./frame
Breakpoint 1 at 0x116c: file frame.c, line 6.

Breakpoint 1, add3 (a=1, b=2, c=3) at frame.c:6
6	    return s + c;
rsp            0x7fffffffd400      0x7fffffffd400
rbp            0x7fffffffd400      0x7fffffffd400
$1 = (long *) 0x7fffffffd3e8
$2 = (long *) 0x7fffffffd3f8
0x7fffffffd3d8:	0x0000000000000003	0x0000000000000002
0x7fffffffd3e8:	0x0000000000000001	0x0000000000000000
0x7fffffffd3f8:	0x0000000000000003
```

`&a` ＝ `d3e8`、`&s` ＝ `d3f8`，都比 `%rsp`（`d400`）低；記憶體裡依序是 `c` ＝ 3、`b` ＝ 2、`a` ＝ 1、一格 0、`s` ＝ 3。

這是 System V AMD64 ABI 的**紅區**（red zone）規定：`%rsp` 下方 128 byte 保留給目前這個函式，不會被別人（例如訊號處理函式）蓋掉。
`add3` 是**葉函式**（leaf function，不再呼叫別人的函式），用的空間在 128 byte 以內，乾脆不 `sub %rsp`，省一條指令。
`main` 要呼叫 `add3` 和 `printf`，`call` 會把返回位址推到 `%rsp` 下面，蓋掉那一區；放在那裡、`call` 之後還要用的變數就會壞掉，所以 gcc 替會呼叫別人的函式一律 `sub $0x10,%rsp`，把變數放到 `%rsp` 上面（`frame.c` 這個 `main` 其實拿掉也能跑，見本小節最後第三個追問補充）。

> kernel 的程式碼不能用紅區：在 kernel 裡，中斷發生時 CPU 會直接把資料推到目前的堆疊上，會蓋掉 `%rsp` 下面的東西。所以 kernel 編譯時加了 `-mno-red-zone`（Linux 原始碼 `arch/x86/Makefile`；本機未實測）。

---

**紅區保證的是「誰」不會蓋：`main` 不是不遵守，是保證本來就不包含自己的 `call`**

> 2026-10-25 提問：「紅區的問題我問一下 你說ABI保證紅區不會被覆蓋,也就是rsp下面128byte不會被覆蓋,但是main爲什麼不遵守這個」（指上面「`%rsp` 下方 128 byte 保留給目前這個函式，不會被別人蓋掉」與「`call` 會把返回位址推到 `%rsp` 下面，蓋掉那一區」兩句）

上面兩句看起來互相矛盾，是因為「不會被蓋掉」少講了主詞。ABI（System V AMD64 psABI 第 3.2.2 節〈The Stack Frame〉）的原文是：

> The 128-byte area beyond the location pointed to by %rsp is considered to be reserved and shall not be modified by signal or interrupt handlers. Therefore, functions may use this area for temporary data that is not needed across function calls. In particular, leaf functions may use this area for their entire stack frame.

（引自規格書原文；本機沒有這份文件，未在本機查證。）逐句翻：

1. `%rsp` 下方 128 byte 是保留區，**訊號處理函式或中斷處理函式不可以改它**。
2. 所以函式可以把「**不需要跨過函式呼叫**」的暫存資料放在這裡。
3. 特別是葉函式，可以把整個堆疊框都放在這裡。

**保證的對象只有「突然插進來的人」**。訊號（signal）是程式跑到一半、核心強行插入去執行處理函式；如果沒有這條規定，處理函式會從目前的 `%rsp` 往下用堆疊，剛好蓋掉這一區。這種插入什麼時候發生、程式自己控制不了，所以要靠 ABI 約束對方。

**函式自己做的 `push`／`call` 不在保證範圍內**，因為那是函式自己決定要寫的。紅區是「保留給目前這個函式」的空間，函式自己往裡面寫，本來就是它的權利。`main` 執行 `call add3`，等於 `main` 自己往 `%rsp − 8` 寫返回位址，寫的是**自己的**紅區，沒有違反任何規定。

接著進到 `add3` 之後，`%rsp` 已經往下移了。紅區永遠是「**目前** `%rsp` 下方 128 byte」，跟著 `%rsp` 走，所以 `main` 原本那塊紅區，現在變成 `add3` 的堆疊框和紅區。被呼叫的函式本來就會使用呼叫者 `%rsp` 下面的空間，這也不是任何人違規。

所以原文第 2 句才特別寫「not needed across function calls」：

- 放在紅區的值，只要中間**不 `call`**，就不會被任何人蓋掉；
- 一 `call`，自己寫的返回位址加上被呼叫者的堆疊框，就會把它蓋掉。

`add3` 不呼叫別人，所以整個堆疊框都能放紅區。`main` 會 `call`，而 `call` 之後還要用的變數不能放紅區，所以 gcc 用 `sub` 把它們移到 `%rsp` 上面，那裡才是「已經放進堆疊」、別人不會寫的地方。

一句話：**紅區防的是「別人突然插進來」，不防「自己 `call` 出去」。**

*（追問補充結束，以下回到主線）*

---

**那自我測驗 2-3 為什麼要提紅區：`main` 要 `sub` 跟紅區無關，紅區解釋的是 `add3` 為什麼可以不 `sub`**

> 2026-10-25 提問：「那這樣2-3爲什麼解釋要提到紅區?main要sub的原因跟紅區有關係嗎?」（指自我測驗 2-3 的參考答案把「紅區」列為核心關鍵）

**`main` 要 `sub` 的理由跟紅區沒有關係**：`main` 會 `call`，`call` 會寫 `%rsp` 下面，`call` 之後還要用的變數就得放到 `%rsp` 上面。就算 ABI 沒有紅區這條規定，`main` 一樣要 `sub`。

紅區回答的是 2-3 的**另一半**：`add3` 憑什麼可以**不** `sub`。只說「`add3` 不 `call`，所以 `%rsp` 下面沒人會寫」還不夠，因為訊號隨時可能插進來，處理函式會從目前的 `%rsp` 往下用堆疊。沒有紅區的保證，連葉函式放在 `%rsp` 下面也不安全，每個函式都得 `sub`。所以 2-3 問的是「為什麼一個要、一個不用」，兩邊各需要一個理由：

| | 要不要 `sub` | 理由 |
|---|---|---|
| `main` | 要 | 自己會 `call`，`call` 寫 `%rsp` 下面（跟紅區無關） |
| `add3` | 不用 | 自己不 `push`／`call`，**加上**紅區保證訊號處理函式不寫 `%rsp` 下面 |

實驗：用 `-mno-red-zone` 告訴 gcc「不准使用紅區」（kernel 就是這樣編的，見上面的引言區塊），看兩個函式各有什麼變化。`frame.c`（檔案：[`frame.c`](../small-project/s1-05-asm-calls/frame.c)）完整原始碼：

```c
#include <stdio.h>

long add3(long a, long b, long c)
{
    long s = a + b;
    return s + c;
}

int main(void)
{
    long r = add3(1, 2, 3);
    printf("%ld\n", r);
    return 0;
}
```

```sh
gcc -O0 -g -fno-asynchronous-unwind-tables frame.c -o frame
gcc -O0 -g -fno-asynchronous-unwind-tables -mno-red-zone frame.c -o frame-norz
objdump -d --no-show-raw-insn frame        # 再對 frame-norz 做一次
```

本機實測（節錄序言與結尾，右邊是說明）：

```asm
# 預設（可以用紅區）                        # -mno-red-zone
<add3>:                                     <add3>:
  endbr64                                     endbr64
  push   %rbp                                 push   %rbp
  mov    %rsp,%rbp                            mov    %rsp,%rbp
                                              sub    $0x28,%rsp      # 多了：把變數空間挪到 %rsp 上面
  mov    %rdi,-0x18(%rbp)                     mov    %rdi,-0x18(%rbp)
  ...                                         ...
  pop    %rbp                                 leave                  # 有 sub 就要 leave 拉回 %rsp
  ret                                         ret

<main>:                                     <main>:
  endbr64                                     endbr64
  push   %rbp                                 push   %rbp
  mov    %rsp,%rbp                            mov    %rsp,%rbp
  sub    $0x10,%rsp                           sub    $0x10,%rsp      # 一模一樣
  ...（call add3、call printf）               ...（同左）
  leave                                       leave
  ret                                         ret
```

- **`main` 兩邊完全一樣**，都是 `sub $0x10,%rsp`：拿掉紅區對它沒有影響，證明它 `sub` 的理由不是紅區。
- **`add3` 只有在不准用紅區時才多了 `sub $0x28,%rsp`**，結尾也從 `pop %rbp` 變成 `leave`：證明 `add3` 可以不 `sub`，靠的就是紅區。`0x28` ＝ 40 byte，剛好涵蓋最低的變數 `-0x28(%rbp)`。

*（追問補充結束，以下回到主線）*

---

---

**「比 `%rsp` 低就不屬於堆疊裡」：第二節哪裡有這個意思**

> 2026-10-10 課後提問：「仔細看上圖：add3 的 %rsp 是 d400，可是 a、b、c、s 放在 d3d8～d3f8，比 %rsp 還低，照第二節的規則那裡不屬於「堆疊裡」。請問第二節的哪裏有這個意思?」

第二節**沒有直接寫**這句話，它是把第二節的兩條規則合起來推出來的：

1. 規則 1「堆疊往低位址長」：越晚放進去的東西，位址越低。
2. 規則 2「`%rsp` 指著最後放進去的那一格（頂端那格本身）」：`%rsp` 指的那格是**最晚**放進去的。

兩條合起來：最晚放進去的那格在 `%rsp`，比它更晚的東西不存在，所以**比 `%rsp` 低的位址還沒放任何東西進去**。
目前「在堆疊裡」的資料只有從 `%rsp` 往高位址那一段；`%rsp` 往下是**空位**，是下一次放東西時要用的地方。

第二節的 `push` 表可以直接驗證這點：`push %rbp` 是先 `sub $8, %rsp`、再寫進新的 `(%rsp)`，也就是說**下一次 `push` 會寫在 `%rsp - 8`**。
放在 `add3` 的情況：`%rsp` 是 `d400`，下一次 `push`（或 `call` 推返回位址）會寫在 `d3f8`——正好是 `s` 所在的那格。
照一般規則，這一區隨時可能被下一次 `push`／`call` 蓋掉，所以不該拿來放變數；這也是為什麼上面需要「紅區」這條額外規定：
ABI 保證 `%rsp` 下方 128 byte 不會被**別人**蓋掉，而 `add3` 是葉函式，自己也不會再 `push`／`call`，那裡才安全。

*（追問補充結束，以下回到主線）*

---

**`call` 只是把 `%rsp` 減 8，為什麼算「蓋掉 `%rsp` 下面」？`sub` 又怎麼「收進堆疊」**

> 2026-10-19 課後提問：「不好意思我看不懂爲什麼會需要sub, call以後 rsp不是改到rsp - 8 的位址嗎?這能算是蓋掉rsp下面的區域嗎?另外 sub跟收進堆疊裏看不懂」（指本節「`main` 要呼叫 `add3` 和 `printf`，`call` 會把返回位址推到 `%rsp` 下面，蓋掉那一區」那段，以及自我測驗 2-3 的參考答案）

**1. `call` 不只是「`%rsp` 減 8」，還會「寫」。** `call` ＝ `push 返回位址` ＋ `jmp`，而 `push` 是兩步（[第二節](#二堆疊rsp-指著頂端往低位址長)的 `push` 表）：

1. `%rsp` 減 8；
2. 把返回位址**寫進**新的 `%rsp` 指的那 8 byte。

新的 `%rsp` 就是「舊 `%rsp` − 8」，所以第 2 步寫進去的地方，正是 `call` 之前 `%rsp` **正下方那一格**。減 8 本身不會蓋掉任何東西，蓋掉東西的是第 2 步的**寫入**。
第三節的實測就是這樣：`call add3` 之前 `%rsp` ＝ `d410`，之後 `%rsp` ＝ `d408`，`d408` 裡放的是返回位址 `0x555555555199`——`d408` 就是 `call` 之前 `d410` 正下方那一格。
那一格原本如果放著變數，值就沒了。

**2. 「收進堆疊」是什麼意思：讓變數那一格在 `%rsp` 的「上面」。** 照本節最上面那張圖，`main` 的 `r` 放在 `-0x8(%rbp)` ＝ `d418`。比較 `main` 有沒有 `sub`：

| | `push %rbp; mov %rsp,%rbp` 之後 `%rsp` | `r` 那格（`d418`）在 `%rsp` 的 | `call` 的返回位址寫在 | `r` 會不會被蓋 |
|---|---|---|---|---|
| 沒有 `sub` | `d420` | 下面（`%rsp − 8`） | `d420 − 8` ＝ **`d418`** | 那格會被寫入，`r` 當時若已經有值就被蓋掉 |
| 有 `sub $0x10,%rsp` | `d420 − 0x10` ＝ `d410` | 上面 | `d410 − 8` ＝ `d408` | 不會 |

`sub $0x10,%rsp` 只是把 `%rsp` 往低位址挪 16 byte，**不寫任何東西**；效果是把「頂端」移到 `r` 的下面。
照第二節的規則，`%rsp` 以上算「已經放進堆疊裡的東西」，之後的 `push`／`call` 只會寫在新的 `%rsp` 下面，`r` 那格就安全了。
「收進堆疊」指的就是這個：用 `sub` 把變數的位置圈到 `%rsp` 以上。函式結束時 `leave`（`mov %rbp,%rsp`）再把 `%rsp` 拉回去，整塊一次丟掉。

`add3` 不需要這一步，是因為它從頭到尾沒有 `push`／`call`，`%rsp` 下面那一區沒有人會去寫（紅區保證別人不寫，它自己也不寫）。

**3. 實驗：故意不 `sub`，看變數被蓋掉。** `frame.c` 的 `r` 剛好是 `call add3` **回來之後**才寫進去，不 `sub` 也看不出問題；所以這裡手寫一個 `main`，把 `x = 42` 存在 `-8(%rbp)`，**中間 `call` 一個什麼都不做的函式**，再把 `x` 印出來。同一份檔案用 `-DSUB` 決定有沒有 `sub`（`.S` 大寫副檔名會先經過 C 前置處理器，`#ifdef` 才有效）。

`nosub.S`（檔案：[`nosub.S`](../small-project/s1-05-asm-calls/nosub.S)）完整原始碼：

```asm
/* nosub.S：main 把變數 x 放在 -8(%rbp)，比較「有 sub」和「沒 sub」時，
   中間 call 一次 noop 之後 x 還在不在。
   編譯：gcc -g -o nosub nosub.S            （沒 sub）
         gcc -g -DSUB -o withsub nosub.S    （有 sub） */

        .section .rodata
fmt:    .string "x = %#lx\n"

        .text
noop:                           /* 什麼都不做的函式 */
        ret

        .globl main
main:
        push    %rbp
        mov     %rsp, %rbp
#ifdef SUB
        sub     $0x10, %rsp     /* 把 x 那格收進堆疊：%rsp 移到 x 的下面 */
#endif
        movq    $42, -8(%rbp)   /* x = 42，放在 %rbp - 8 */
        call    noop            /* 推返回位址到 %rsp - 8，再跳去 noop */
        mov     -8(%rbp), %rsi  /* printf 的第 2 個參數 = x */
        lea     fmt(%rip), %rdi /* printf 的第 1 個參數 = 字串位址 */
        mov     $0, %eax        /* 沒有 double 參數（第七節） */
        call    printf@PLT
        mov     $0, %eax        /* return 0 */
        leave
        ret

        .section .note.GNU-stack,"",@progbits
```

最後一行只是告訴連結器「這個檔案不需要可執行的堆疊」，少了它 `ld` 會印警告，跟本實驗無關。

```
$ gcc -g -DSUB -o withsub nosub.S
$ gcc -g -o nosub nosub.S
$ ./withsub
x = 0x2a
$ ./nosub
x = 0x55555555514b
```

有 `sub` 時印出 `0x2a`（42）；沒 `sub` 時 `x` 變成一個很像位址的數字。用 `objdump -d --no-show-raw-insn nosub` 看沒 `sub` 的版本：

```
000000000000113a <main>:
    113a:	push   %rbp
    113b:	mov    %rsp,%rbp
    113e:	movq   $0x2a,-0x8(%rbp)
    1146:	call   1139 <noop>
    114b:	mov    -0x8(%rbp),%rsi
    ...
```

`call` 的下一條指令在 `0x114b`——`x` 被換成的就是 `call noop` 的**返回位址**（執行時再加上載入位址 `0x555555554000`）。

用 gdb 停在 `call noop` 上，比較 `call` 前後。`nosub.gdb`（檔案：[`nosub.gdb`](../small-project/s1-05-asm-calls/nosub.gdb)），`main+12` 是 `call noop`（`0x1146 − 0x113a = 12`）：

```
break *main+12
run
x/i $pc
info registers rsp rbp
x/gx $rbp-8
stepi
x/i $pc
info registers rsp
x/gx $rbp-8
continue
```

```
$ gdb -q -batch -x nosub.gdb ./nosub
Breakpoint 1 at 0x1146: file nosub.S, line 21.

Breakpoint 1, main () at nosub.S:21
21	        call    noop            /* 推返回位址到 %rsp - 8，再跳去 noop */
=> 0x555555555146 <main+12>:	call   0x555555555139 <noop>
rsp            0x7fffffffd450      0x7fffffffd450
rbp            0x7fffffffd450      0x7fffffffd450
0x7fffffffd448:	0x000000000000002a
noop () at nosub.S:11
11	        ret
=> 0x555555555139 <noop>:	ret
rsp            0x7fffffffd448      0x7fffffffd448
0x7fffffffd448:	0x000055555555514b
x = 0x55555555514b
```

- `call` 之前：`%rsp` ＝ `d450`，`x` 在 `%rbp − 8` ＝ `d448`，也就是 `%rsp` 正下方那一格，值是 `0x2a`。
- `call` 之後：`%rsp` ＝ `d448`，返回位址 `0x55555555514b` 寫進 `d448`——`x` 被蓋掉了。

有 `sub` 的版本換成 `withsub.gdb`（檔案：[`withsub.gdb`](../small-project/s1-05-asm-calls/withsub.gdb)，內容只有第一行改成 `break *main+16`，因為多了 4 byte 的 `sub` 指令）：

```
$ gdb -q -batch -x withsub.gdb ./withsub
...
=> 0x55555555514a <main+16>:	call   0x555555555139 <noop>
rsp            0x7fffffffd430      0x7fffffffd430
rbp            0x7fffffffd440      0x7fffffffd440
0x7fffffffd438:	0x000000000000002a
...
=> 0x555555555139 <noop>:	ret
rsp            0x7fffffffd428      0x7fffffffd428
0x7fffffffd438:	0x000000000000002a
x = 0x2a
```

- `sub` 之後 `%rsp` ＝ `d430`，`x`（`d438`）在 `%rsp` 上面。
- `call` 把返回位址寫在 `d430 − 8` ＝ `d428`，`x` 不受影響。

（兩個程式的 `%rbp` 差 `0x10`，是兩個執行檔被載入時堆疊起點不同，跟 `sub` 無關；要比的是同一個程式裡 `x` 和 `%rsp` 的相對位置。）

所以 2-3 的完整答案是：`%rsp` 下面那一區，只有「自己不會 `push`／`call`」的葉函式才能放心用（再加上紅區保證別人不寫）；
會 `call` 的函式，每次 `call` 都會往 `%rsp − 8` 寫返回位址，所以要先 `sub`，把變數放到 `%rsp` 以上。
`frame.c` 的 `r` 雖然剛好是 `call` 回來才寫，但編譯器不會逐一去算「這個變數在 `call` 時有沒有在用」，會呼叫別人的函式就一律用 `sub` 開空間。

*（追問補充結束，以下回到主線）*

---

**那 `frame.c` 的 `main` 拿掉 `sub $0x10,%rsp` 會怎樣：其實照樣能跑**

> 2026-10-19 課後提問：「我實際去找main的組語了,但是我看不出來sub 0x10的必要性」（指 `frame.c` 的 `main`，`objdump -d` 的那一段）

看得很準：**單看 `frame.c` 這個 `main`，`sub $0x10,%rsp` 不是必要的**，拿掉照樣印出 6。上一段說「`call` 會蓋掉 `%rsp` 下面」沒錯，但只有「變數的值在 `call` 之後還要用」時才會出事，這個 `main` 剛好沒有這種變數。

`nosub-frame.sh`（檔案：[`nosub-frame.sh`](../small-project/s1-05-asm-calls/nosub-frame.sh)）完整內容，在 `frame.c` 所在的資料夾執行 `sh nosub-frame.sh`：

```sh
#!/bin/sh
# nosub-frame.sh：把 frame.c 編成組合語言，刪掉 main 的 subq $16, %rsp，再組譯、執行
# 用法：在 frame.c 所在的資料夾執行 sh nosub-frame.sh
set -e
gcc -O0 -S frame.c -o frame.s                       # 跟筆記一樣用 -O0，只停在組合語言
grep -n 'subq' frame.s                              # 確認整個檔案只有 main 這一條 subq
sed '/subq\t\$16, %rsp/d' frame.s > frame_nosub.s   # 刪掉那一行，其他不動
diff frame.s frame_nosub.s || true                  # 列出差別（diff 有差別時結束狀態是 1）
gcc -o frame_nosub frame_nosub.s                    # 組譯＋連結
./frame_nosub
```

```
$ sh nosub-frame.sh
45:	subq	$16, %rsp
45d44
< 	subq	$16, %rsp
6
```

（`gcc -S` 產生的是 AT&T 語法加大小字尾的寫法：`subq $16, %rsp` 就是 objdump 裡的 `sub $0x10,%rsp`，第 4 課教過字尾 `q` ＝ 8 byte。）

照時間順序走一次沒有 `sub` 的 `main`（`%rbp` 那格以下，`r` 在 `-0x8(%rbp)`，也就是 `%rsp` 正下方那格）：

1. `call add3`：返回位址寫進 `-0x8(%rbp)`。這時 `r` 還沒有值，**沒有東西被蓋掉**；`add3` 的 `ret` 把它拿走。
2. `mov %rax,-0x8(%rbp)`：`r` ＝ 6，寫進那格。
3. `mov -0x8(%rbp),%rax`、`mov %rax,%rsi`：`r` 的值**已經複製到 `%rsi`**（`printf` 的第 2 個參數）。
4. `call printf`：返回位址再寫進 `-0x8(%rbp)`，`r` 確實被蓋掉了——但之後 `main` 再也沒讀過 `-0x8(%rbp)`，`printf` 拿的是 `%rsi` 裡的 6。

對齊也沒問題：進 `main` 時 `%rsp` 是 16 的倍數 ＋ 8，`push %rbp` 之後剛好是 16 的倍數，不 `sub` 也滿足[第六節](#六誰負責保存暫存器以及-16-byte-對齊)「`call` 之前 `%rsp` 是 16 的倍數」的規定。

**那 gcc 為什麼還是加了 `sub`？** 因為 `-O0` 不做這種分析。從本課看到的兩個函式，`-O0` 的做法是：

- 葉函式（`add3`）：變數放紅區，不 `sub`；
- 會 `call` 別人的函式（`main`）：每個區域變數都在 `%rsp` 上面給一格固定位置，用 `sub` 開出來（大小湊成 16 的倍數，`r` 8 byte ＋ 對齊 8 byte ＝ `0x10`）。

它不去判斷「`r` 在哪次 `call` 時還要用」，一律這樣安排，才能保證**任何**程式都對——像上一段 `nosub.S` 的 `x`，在 `call` 之前寫入、之後還要讀，不 `sub` 就錯。所以 `sub` 是 gcc 的「保險做法」，不是這個 `main` 的值非靠它不可。

*（追問補充結束，以下回到主線）*

---

## 五、參數與回傳值放哪裡

System V AMD64 ABI 規定的整數與指標參數（`char`、`int`、`long`、指標……）：

| 第幾個參數 | 1 | 2 | 3 | 4 | 5 | 6 | 7 以後 |
|---|---|---|---|---|---|---|---|
| 放在哪 | `%rdi` | `%rsi` | `%rdx` | `%rcx` | `%r8` | `%r9` | 堆疊（呼叫者推進去，每個佔 8 byte） |

**回傳值**放在 `%rax`。參數比 8 byte 小的時候，用同一個暫存器較小的名字（`int` 用 `%edi`、`char` 用 `%dil`，第 4 課第二節的表）。
浮點數（`double`）的參數走另一組暫存器 `%xmm0`～`%xmm7`，回傳值在 `%xmm0`；本路線不學浮點指令，第七節只會看到它們出現。

### 實驗：8 個參數，第 7、8 個去哪了

`args8.c`（檔案：[`args8.c`](../small-project/s1-05-asm-calls/args8.c)）完整原始碼。不需要 `main`，因為只編成 `.o` 看組合語言：

```c
long sum8(long a, long b, long c, long d, long e, long f, long g, long h)
{
    return a + b + c + d + e + f + g + h;
}

long call8(void)
{
    return sum8(1, 2, 3, 4, 5, 6, 7, 8);
}
```

```
$ gcc -O0 -c -fno-asynchronous-unwind-tables args8.c -o args8.o
$ objdump -dr --no-show-raw-insn args8.o
```

`-r` 會把**重定位項目**（relocation）印在相關的指令下面（第 1 課[第四節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#四組譯機器碼裡的洞)：`.o` 裡 `call` 的目標還是 0，要等連結器填）。輸出：

```asm
0000000000000000 <sum8>:
   0:	endbr64
   4:	push   %rbp
   5:	mov    %rsp,%rbp
   8:	mov    %rdi,-0x8(%rbp)       # a（第 1 個，在 %rdi）
   c:	mov    %rsi,-0x10(%rbp)      # b
  10:	mov    %rdx,-0x18(%rbp)      # c
  14:	mov    %rcx,-0x20(%rbp)      # d
  18:	mov    %r8,-0x28(%rbp)       # e
  1c:	mov    %r9,-0x30(%rbp)       # f（第 6 個，在 %r9）
  20:	mov    -0x8(%rbp),%rdx       # 以下依序加總
  24:	mov    -0x10(%rbp),%rax
  28:	add    %rax,%rdx
  2b:	mov    -0x18(%rbp),%rax
  2f:	add    %rax,%rdx
  32:	mov    -0x20(%rbp),%rax
  36:	add    %rax,%rdx
  39:	mov    -0x28(%rbp),%rax
  3d:	add    %rax,%rdx
  40:	mov    -0x30(%rbp),%rax
  44:	add    %rax,%rdx
  47:	mov    0x10(%rbp),%rax       # g（第 7 個）：在 %rbp 的「上面」0x10
  4b:	add    %rax,%rdx
  4e:	mov    0x18(%rbp),%rax       # h（第 8 個）：在 %rbp 上面 0x18
  52:	add    %rdx,%rax             # 回傳值在 %rax
  55:	pop    %rbp
  56:	ret

0000000000000057 <call8>:
  57:	endbr64
  5b:	push   %rbp
  5c:	mov    %rsp,%rbp
  5f:	push   $0x8                  # 第 8 個參數先推
  61:	push   $0x7                  # 第 7 個參數後推（所以在比較低的位址）
  63:	mov    $0x6,%r9d             # 第 6 個
  69:	mov    $0x5,%r8d             # 第 5 個
  6f:	mov    $0x4,%ecx
  74:	mov    $0x3,%edx
  79:	mov    $0x2,%esi
  7e:	mov    $0x1,%edi             # 第 1 個
  83:	call   88 <call8+0x31>       # 目標還沒填，看下一行的重定位項目
			84: R_X86_64_PLT32	sum8-0x4
  88:	add    $0x10,%rsp            # 呼叫完，丟掉剛才推的兩個參數（2 × 8 byte）
  8c:	leave
  8d:	ret
```

逐點讀：

- **前 6 個參數**完全照表：`%rdi`、`%rsi`、`%rdx`、`%rcx`、`%r8`、`%r9`。`call8` 寫的是 `%r9d`、`%ecx` 這些 32 位元的名字，因為常數小，寫 32 位元就會清掉高位（零、第 1 點），整個 64 位元就是那個值，而且指令比較短。
- **第 7、8 個參數**用 `push` 推進堆疊，**從最後一個開始推**。這樣第 7 個在比較低的位址，離 `sum8` 的堆疊框最近。
- **在 `sum8` 裡為什麼是 `0x10(%rbp)`**：`call` 又推了返回位址，`sum8` 的序言又推了 `%rbp`。所以從 `sum8` 的 `%rbp` 往上看：`0(%rbp)` 是舊 `%rbp`、`0x8(%rbp)` 是返回位址、`0x10(%rbp)` 才是第 7 個參數、`0x18(%rbp)` 是第 8 個。**參數用正的偏移（在呼叫者那邊），區域變數用負的偏移（在自己這邊）。**
- **誰清掉堆疊上的參數**：`call8` 自己推的，`call` 回來後由 `call8` 用 `add $0x10,%rsp` 丟掉。被呼叫的 `sum8` 用 `ret` 返回時只拿走返回位址。
- `call 88 <call8+0x31>`：`.o` 裡 `call` 的位移還是 0，所以 `objdump` 把目標算成「下一條指令」`0x88`，這個數字沒有意義。真正的目標是下一行的重定位項目 `R_X86_64_PLT32 sum8-0x4`，連結時才填上。

### 補教：`char`、`short` 參數為什麼先 `mov %edi,%edx` 再存 `%dl`

（2026-10-12 批改作業 1-2 時補教。）參數比 8 byte 小時，只佔暫存器最低的幾個 byte，但 `-O0` 存它的時候常常**先用 32 位元的名字把整個暫存器複製一份**，再只存低位。作業的 `pick.c` 完整原始碼：

```c
long pick(char a, short b, int c, long d, unsigned char *e, unsigned int f)
{
    return a + b + c + d + *e + f;
}
```

```
gcc -O0 -c -fno-asynchronous-unwind-tables pick.c
objdump -dr --no-show-raw-insn pick.o
```

存參數的部分（本機實測）：

```asm
   8:	mov    %esi,%eax           # b 在 %si：先把整個 %esi 複製到暫存用的 %eax
   a:	mov    %edx,-0xc(%rbp)     # c（int，4 byte）
   d:	mov    %rcx,-0x18(%rbp)    # d（long，8 byte）
  11:	mov    %r8,-0x20(%rbp)     # e（指標，8 byte）
  15:	mov    %r9d,-0x10(%rbp)    # f（unsigned int，4 byte）
  19:	mov    %edi,%edx           # a 在 %dil：先把整個 %edi 複製到 %edx
  1b:	mov    %dl,-0x4(%rbp)      # 只存最低 1 byte：a
  1e:	mov    %ax,-0x8(%rbp)      # 只存最低 2 byte：b
```

- `a` 確實是用 `%dil`、`b` 確實是用 `%si` 傳進來的；`mov %edi,%edx`、`mov %esi,%eax` 只是 gcc 在 `-O0` 下的搬法：先整個複製到別的暫存器，再用 8／16 位元的名字（`%dl`、`%ax`）存進堆疊。
- 複製過去的高位是垃圾（[第 4 課第三節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#三指令的文法助憶符大小後綴運算元)〈讀懂實驗一的範例〉：參數只佔最低幾個 byte，高位不保證），但最後只存低位，所以不影響結果。
- 所以判斷「參數用哪個名字傳進來」要看**最後存進堆疊或拿去運算時用的名字**（`%dl`、`%ax`），不要被中間的 32 位元複製騙了。
- `-O2` 不存堆疊，直接 `movsbl %dil,%edi`、`movswl %si,%esi`，看得更清楚。

---

## 六、誰負責保存暫存器，以及 16 byte 對齊

`call` 之後，被呼叫的函式也要用暫存器。如果它把呼叫者還要用的值蓋掉怎麼辦？ABI 把 16 個暫存器分成兩組：

| 分類 | 暫存器 | 規則 |
|---|---|---|
| **呼叫者保存**（caller-saved） | `%rax`、`%rcx`、`%rdx`、`%rsi`、`%rdi`、`%r8`～`%r11` | 被呼叫的函式**可以隨便改**。呼叫者如果 `call` 之後還要用裡面的值，要自己先存起來 |
| **被呼叫者保存**（callee-saved） | `%rbx`、`%rbp`、`%r12`～`%r15`（以及 `%rsp`） | 被呼叫的函式如果要用，**必須先存起來、返回前還原**。所以呼叫者可以相信 `call` 回來後它們沒變 |

這張表是 ABI 的規定（本機沒有 ABI 文件可以對照，下面用一個實際例子驗證其中的 `%rbx`）。記法：傳參數的 6 個、回傳值 `%rax`、再加上 `%r10`、`%r11`，全都是「用完就丟」的；剩下的 6 個是「借了要還」的。

### 實驗：要跨過 `call` 保存的值放哪裡

`saved.c`（檔案：[`saved.c`](../small-project/s1-05-asm-calls/saved.c)）完整原始碼。`g` 只有宣告、沒有定義，這樣 gcc 不知道 `g` 會做什麼，只能照 ABI 假設它會亂改 caller-saved 暫存器：

```c
long g(long x);

long f(long x)
{
    long y = g(x);
    return x + y;
}
```

```
$ gcc -O2 -c -fno-asynchronous-unwind-tables saved.c -o saved.o
$ objdump -dr --no-show-raw-insn saved.o
```

```asm
0000000000000000 <f>:
   0:	endbr64
   4:	push   %rbx                  # rbx 是 callee-saved：f 要用它，先替呼叫 f 的人存起來
   5:	mov    %rdi,%rbx             # x 從 %rdi 搬到 %rbx
   8:	call   d <f+0xd>             # 呼叫 g（參數 x 還在 %rdi）
			9: R_X86_64_PLT32	g-0x4
   d:	add    %rbx,%rax             # %rax = g 的回傳值 + x
  10:	pop    %rbx                  # 還原呼叫 f 的人的 %rbx
  11:	ret
```

`x` 在 `g` 回來之後還要用（`x + y`），可是它原本在 `%rdi`，是 caller-saved，`g` 可能改掉它。gcc 的做法：把 `x` 搬到 callee-saved 的 `%rbx`，因為 ABI 保證 `g` 回來後 `%rbx` 不變。
但 `f` 自己對呼叫 `f` 的人也有同樣的義務，所以 `f` 一開頭 `push %rbx`，返回前 `pop %rbx` 還原。

對照組 `noret.c`（檔案：[`noret.c`](../small-project/s1-05-asm-calls/noret.c)），`call` 之後不再用 `x`：

```c
long g(long x);

long f(long x)
{
    long y = g(x);
    return y * 2;
}
```

```
$ gcc -O2 -c -fno-asynchronous-unwind-tables noret.c -o noret.o
$ objdump -dr --no-show-raw-insn noret.o
```

```asm
0000000000000000 <f>:
   0:	endbr64
   4:	sub    $0x8,%rsp             # 不是為了存東西：為了對齊（下面說明）
   8:	call   d <f+0xd>
			9: R_X86_64_PLT32	g-0x4
   d:	add    $0x8,%rsp
  11:	add    %rax,%rax             # y * 2 寫成 y + y
  14:	ret
```

不需要跨過 `call` 保存任何值，就不用 `%rbx`，也沒有 `push`／`pop %rbx`。（`-O2` 不建 `%rbp` 堆疊框：`%rbp` 只是讓人和除錯器好讀，最佳化時省掉。）

### 上面的 `sub $0x8,%rsp`：16 byte 對齊

`noret.c` 什麼都不存，為什麼還要把 `%rsp` 往下挪 8 byte、回來再加回去？因為 ABI 還有一條規定：**執行 `call` 的那一刻，`%rsp` 必須是 16 的倍數。**

算一次：

1. 呼叫 `f` 的人照規定，`call f` 時 `%rsp` 是 16 的倍數。
2. `call` 推進 8 byte 的返回位址，進到 `f` 時 `%rsp` 變成「16 的倍數 − 8」。
3. `f` 要 `call g`，必須再挪 8 byte 才會回到 16 的倍數。`saved.c` 的 `push %rbx` 剛好挪了 8 byte，一舉兩得；`noret.c` 沒有東西要存，就用 `sub $0x8,%rsp` 單純挪 8 byte。

`frame.c` 的 `main` 也一樣：進 `main` 時 `%rsp` 是 `d428`（`0x…28`，除以 16 餘 8），`push %rbp` 後是 `d420`，`sub $0x10` 後是 `d410`（16 的倍數）。區域變數 `r` 只要 8 byte，gcc 卻留了 16 byte，就是為了這條規定。第三節實測 `call add3` 之前 `%rsp` ＝ `0x7fffffffd410`，確實是 16 的倍數。

為什麼要 16：有些指令（例如一次搬 16 byte 的 SSE 指令 `movaps`）要求記憶體位址是 16 的倍數，否則會當掉。ABI 規定進函式時的對齊狀態是固定的，函式裡的區域變數就能直接排成 16 byte 對齊。

### 補教：被呼叫的函式就在同一個檔案時，`sub $0x8,%rsp` 為什麼不見了

（2026-10-16 批改作業 2-5 時補教。）上面的對齊是**為了被呼叫的函式**：讓它可以放心用 `movaps` 這類要求 16 byte 對齊的指令。所以如果 gcc **看得到**被呼叫的函式、確定它不需要對齊，這一次呼叫就可以不補。

`align9.c`（檔案：[`align9.c`](../small-project/s1-05-asm-calls/align9.c)）完整原始碼，`sum9` 有 9 個參數、第 7～9 個要放堆疊（3 × 8 ＝ 24 byte，單靠它們湊不成 16 的倍數）：

```c
/* align9.c：sum9 和 call9 放在同一個檔案（作業 2-5 的情境） */
int sum9(int a, int b, int c, int d, int e, int f, int g, int h, int i)
{
    return a + b + c + d + e + f + g + h + i;
}

int call9(void)
{
    return sum9(10, 20, 30, 40, 50, 60, 70, 80, 90);
}
```

```
gcc -O0 -c -fno-asynchronous-unwind-tables align9.c
objdump -dr --no-show-raw-insn align9.o
gcc -O0 -c -fno-asynchronous-unwind-tables -fno-ipa-stack-alignment align9.c -o align9-noipa.o
objdump -dr --no-show-raw-insn align9-noipa.o
```

兩次的 `call9` 並排（本機 gcc 13.3 實測；左邊是預設，右邊加了 `-fno-ipa-stack-alignment`）：

```
預設                                    -fno-ipa-stack-alignment
  49: endbr64                             49: endbr64
  4d: push   %rbp                         4d: push   %rbp
  4e: mov    %rsp,%rbp                    4e: mov    %rsp,%rbp
                                          51: sub    $0x8,%rsp      # 對齊用的 8 byte 回來了
  51: push   $0x5a     # i = 90           55: push   $0x5a
  53: push   $0x50     # h = 80           57: push   $0x50
  55: push   $0x46     # g = 70           59: push   $0x46
  57: mov    $0x3c,%r9d                   5b: mov    $0x3c,%r9d
  ...（%r8d、%ecx、%edx、%esi、%edi 同上）
  77: call   7c <call9+0x33>              7b: call   80 <call9+0x37>
  7c: add    $0x18,%rsp  # 只丟 24 byte    80: add    $0x20,%rsp  # 丟 32 byte
  80: leave                               84: leave
  81: ret                                 85: ret
```

如果 `sum9` 只有**宣告**、本體在另一個檔案（作業 2 的 `nine.c`），結果跟右邊一樣，有 `sub $0x8,%rsp`。

為什麼：

1. **只有宣告時**，gcc 不知道 `sum9` 裡面做什麼，只能照 ABI 假設它需要 16 byte 對齊，補上 `sub $0x8,%rsp`。用上一小節的算法：進 `call9` 餘 8 → `push %rbp` 餘 0 → `sub` 餘 8 → 推 24 byte 餘 0 → `call` 時對齊。
2. **本體在同一個檔案時**，gcc 先分析 `sum9`：它不再 `call` 別人、沒有要求對齊的指令，只需要 8 byte 對齊。於是這一次 `call sum9` 不補，`call` 時餘 8 也沒關係；`add` 也從 `0x20` 變成剛好 3 個參數的 `0x18`。
3. 這個分析是 gcc 的 `-fipa-stack-alignment`（IPA，Interprocedural Analysis，跨函式分析：看別的函式的本體來決定這個函式怎麼編）。`man gcc` 的說明是 "Reduce stack alignment on call sites if possible. Enabled by default."，`gcc -Q --help=optimizers -O0 | grep ipa-stack` 實測在 `-O0` 也是 `[enabled]`。加 `-fno-ipa-stack-alignment` 就關掉，`sub` 回來。

注意這只影響**看得到本體的那一次呼叫**。`sum9` 是一般的全域函式，別的檔案呼叫它時，那邊的 gcc 只看得到宣告，照樣會補對齊；`sum9` 自己的程式碼也不變。

gcc 的最佳化選項很多（本機 gcc 13.3 的 `gcc -Q --help=optimizers -O0` 列出 260 個 `-f` 選項），這裡只用到 `-fipa-stack-alignment` 這一個；完整清單用 `gcc --help=optimizers`（加 `-Q` 和 `-O0`／`-O2` 會顯示在那個等級是否開啟）或 `man gcc` 的 "Options That Control Optimization" 一節查。

---

## 七、`printf` 前的 `movl $0, %eax`：可變參數函式的 `%al`

第 4 課看到 `main` 呼叫 `printf` 前有一行 `movl $0, %eax`，當時說「第 5 課講」。這是 ABI 對**可變參數函式**（variadic function，參數個數不固定，例如 `printf(const char *fmt, ...)`）的規定：
**呼叫前把 `%al`（`%rax` 最低的 1 byte）設成「這次用了幾個向量暫存器（`%xmm0`～`%xmm7`）傳參數」**，給 `printf` 決定要不要把那些暫存器存起來。

`varargs.c`（檔案：[`varargs.c`](../small-project/s1-05-asm-calls/varargs.c)）完整原始碼：

```c
#include <stdio.h>

int main(void)
{
    printf("%d\n", 1);
    printf("%f\n", 1.5);
    printf("%f %f\n", 1.5, 2.5);
    return 0;
}
```

```
$ gcc -O0 -fno-asynchronous-unwind-tables varargs.c -o varargs
$ ./varargs
1
1.500000
1.500000 2.500000
$ objdump -d --no-show-raw-insn varargs
```

`main` 的部分：

```asm
0000000000001149 <main>:
    1149:	endbr64
    114d:	push   %rbp
    114e:	mov    %rsp,%rbp
    # printf("%d\n", 1)
    1151:	mov    $0x1,%esi                    # 第 2 個參數 1（整數，走 %rsi）
    1156:	lea    0xeab(%rip),%rax             # 字串 "%d\n" 的位址
    115d:	mov    %rax,%rdi                    # 第 1 個參數
    1160:	mov    $0x0,%eax                    # %al = 0：沒有用到向量暫存器
    1165:	call   1050 <printf@plt>
    # printf("%f\n", 1.5)
    116a:	mov    0xea7(%rip),%rax             # 從 .rodata 讀出 1.5 的 8 byte（double）
    1171:	movq   %rax,%xmm0                   # 放進 %xmm0：double 參數走向量暫存器
    1176:	lea    0xe8f(%rip),%rax             # 字串 "%f\n" 的位址
    117d:	mov    %rax,%rdi
    1180:	mov    $0x1,%eax                    # %al = 1：用了 1 個（%xmm0）
    1185:	call   1050 <printf@plt>
    # printf("%f %f\n", 1.5, 2.5)
    118a:	movsd  0xe8e(%rip),%xmm0            # 從 .rodata 讀出 2.5，暫放 %xmm0
    1192:	mov    0xe7f(%rip),%rax             # 讀出 1.5
    1199:	movapd %xmm0,%xmm1                  # 2.5 搬到 %xmm1（第 2 個 double 參數）
    119d:	movq   %rax,%xmm0                   # 1.5 放進 %xmm0（第 1 個 double 參數）
    11a2:	lea    0xe67(%rip),%rax             # 字串 "%f %f\n" 的位址
    11a9:	mov    %rax,%rdi
    11ac:	mov    $0x2,%eax                    # %al = 2：用了 2 個（%xmm0、%xmm1）
    11b1:	call   1050 <printf@plt>
    11b6:	mov    $0x0,%eax                    # return 0;
    11bb:	pop    %rbp
    11bc:	ret
```

`%eax` 依序設成 0、1、2，正好是每次呼叫用掉的 `%xmm` 暫存器個數。寫的是 `%eax` 而不是 `%al`，是因為 `mov $0x0,%eax` 一次清掉整個 `%rax`（零、第 1 點）。
可以注意兩件事：整數參數 `1` 和 `double` 參數各走各的暫存器序列：`%rdi`、`%rsi` 是整數的第 1、2 個，`%xmm0`、`%xmm1` 是浮點的第 1、2 個，互不影響。
`movq`、`movsd`、`movapd` 是搬浮點資料的指令，本路線不需要記，看得出「把 double 放進 `%xmm`」就夠了。

**只有可變參數函式需要設 `%al`。** 呼叫一般函式（例如 `add3`）時，`main` 沒有寫這一行（第三節的 `main` 在 `call add3` 前沒有 `mov $0x0,%eax`）。

### 補教：超過 8 個 `double`，第 9 個放哪裡、`%al` 是多少

（2026-10-19 批改作業 5-4 時補教。）浮點參數的暫存器只有 `%xmm0`～`%xmm7` 八個。第 9 個 `double` 跟第 7 個以後的整數參數一樣，**放到堆疊上**（[第五節](#五參數與回傳值放哪裡)的表最右欄），每個佔 8 byte。`%al` 數的是**用了幾個向量暫存器**，不是有幾個 `double`，所以最多就是 8。

`float9.c`（檔案：[`float9.c`](../small-project/s1-05-asm-calls/float9.c)）完整原始碼：

```c
/* float9.c：9 個 double 傳給 printf，第 9 個放不進 %xmm0～%xmm7 */
#include <stdio.h>

int main(void)
{
    printf("%.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f\n",
           1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    return 0;
}
```

```
$ gcc -O0 -g -fno-asynchronous-unwind-tables float9.c -o float9
$ ./float9
1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0
$ objdump -d --no-show-raw-insn float9
$ objdump -s -j .rodata float9
```

`-g` 是為了下面用 gdb 看，加不加 `-g` 編出來的指令都一樣（本機對照過，位址相同）。`objdump -s -j .rodata` 是第 4 課[第三節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#為什麼-l-叫-long-卻是-4-byte組合語言的大小名稱和-c-的型別是兩套)〈為什麼 `l` 叫 long 卻是 4 byte〉的步驟三教過的「印出某個區段的原始內容」（`-s` 印內容、`-j` 指定區段），用來查每個常數是多少：

```
Contents of section .rodata:
 2000 01000200 00000000 252e3166 20252e31  ........%.1f %.1
 ...
 2030 252e3166 0a000000 00000000 00002040  %.1f.......... @
 2040 00000000 00001c40 00000000 00001840  .......@.......@
 2050 00000000 00001440 00000000 00001040  .......@.......@
 2060 00000000 00000840 00000000 00000040  .......@.......@
 2070 00000000 0000f03f 00000000 00002240  .......?......"@
```

每個 `double` 佔 8 byte，小端序（第 2 課[第三節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#三位元組順序endianness)），所以要倒過來讀：`2038` 開始的 `00 00 00 00 00 00 20 40` 是 `0x4020000000000000`，也就是 8.0。依同樣的讀法：`2040` ＝ 7.0（`0x401c…`）、`2048` ＝ 6.0、`2050` ＝ 5.0、`2058` ＝ 4.0、`2060` ＝ 3.0、`2068` ＝ 2.0（`0x4000…`）、`2070` ＝ 1.0（`0x3ff0…`）、`2078` ＝ 9.0（`0x4022…`）。`double` 的位元怎麼對應到數值（IEEE 754 格式）本路線不教，這裡只要知道「這 8 byte 是哪個數」；下面用 gdb 的 `x/fg` 直接把它印成浮點數，不必自己換算。

`main` 的部分（`#` 後面是說明）：

```asm
0000000000001149 <main>:
    1149:	endbr64
    114d:	push   %rbp                  # 進 main 時餘 8，push 後 %rsp 是 16 的倍數
    114e:	mov    %rsp,%rbp
    1151:	movsd  0xedf(%rip),%xmm7     # 2038：8.0 → %xmm7（第 8 個 double）
    1159:	movsd  0xedf(%rip),%xmm6     # 2040：7.0 → %xmm6
    1161:	movsd  0xedf(%rip),%xmm5     # 2048：6.0 → %xmm5
    1169:	movsd  0xedf(%rip),%xmm4     # 2050：5.0 → %xmm4
    1171:	movsd  0xedf(%rip),%xmm3     # 2058：4.0 → %xmm3
    1179:	movsd  0xedf(%rip),%xmm2     # 2060：3.0 → %xmm2
    1181:	movsd  0xedf(%rip),%xmm1     # 2068：2.0 → %xmm1
    1189:	mov    0xee0(%rip),%rax      # 2070：1.0 先暫放 %rax（%xmm0 等一下要先借來搬 9.0）
    1190:	sub    $0x8,%rsp             # 對齊用：只有 1 個 8 byte 的堆疊參數，先補 8 byte
    1194:	movsd  0xedc(%rip),%xmm0     # 2078：9.0 → %xmm0（暫時借用）
    119c:	lea    -0x8(%rsp),%rsp       # %rsp = %rsp - 8：挪出放第 9 個參數的一格
    11a1:	movsd  %xmm0,(%rsp)          # 9.0 寫進堆疊頂端：第 9 個 double 在堆疊上
    11a6:	movq   %rax,%xmm0            # 1.0 放回 %xmm0（第 1 個 double）
    11ab:	lea    0xe56(%rip),%rax      # 格式字串的位址
    11b2:	mov    %rax,%rdi             # 第 1 個參數（整數序列的第 1 個）
    11b5:	mov    $0x8,%eax             # %al = 8：用了 8 個向量暫存器
    11ba:	call   1050 <printf@plt>
    11bf:	add    $0x10,%rsp            # 呼叫者清掉：堆疊參數 8 ＋ 對齊 8 ＝ 16
    11c3:	mov    $0x0,%eax             # return 0;
    11c8:	leave
    11c9:	ret
```

逐段看：

1. **前 8 個 `double` 照順序放 `%xmm0`～`%xmm7`**：1.0 在 `%xmm0`、2.0 在 `%xmm1`……8.0 在 `%xmm7`。gcc 從 `%xmm7` 倒著填，順序不重要，要看的是 `call` 那一刻每個暫存器裡是什麼。1.0 繞了一圈（先放 `%rax`、最後才搬進 `%xmm0`），因為 gcc 要先借 `%xmm0` 把 9.0 從 `.rodata` 搬到堆疊（`-O0` 不會想辦法省這一步）。
2. **第 9 個放堆疊**：`lea -0x8(%rsp),%rsp` 把 `%rsp` 減 8（`lea` 只算位址、不讀記憶體，見第 4 課[第四節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#四mov-與-lea搬資料-vs-算位址)；這裡的效果跟 `sub $0x8,%rsp` 一樣），`movsd %xmm0,(%rsp)` 把 9.0 寫到新的頂端。`call` 推進返回位址後，9.0 就在 `printf` 看到的「返回位址上面那一格」，跟[第五節](#五參數與回傳值放哪裡)第 7 個整數參數放的位置一樣。
3. **`sub $0x8,%rsp` 是對齊**，跟[第六節](#上面的-sub-0x8rsp16-byte-對齊)同一條規則：進 `main` 時餘 8，`push %rbp` 後餘 0；堆疊參數只有 1 個（8 byte），放上去會變成餘 8，所以先補 8 byte，`call` 時才是 16 的倍數。作業第 2 題的 `call9` 推 3 個 `int`（24 byte）要補 8，也是同一個道理。
4. **`%al` ＝ 8**：放在堆疊的第 9 個不算，因為 `%al` 是給 `printf` 決定「要把幾個 `%xmm` 暫存器存起來」，堆疊上的參數本來就在記憶體裡，不必存。
5. **`add $0x10,%rsp`**：呼叫者清掉自己放的東西（第五節），8 byte 參數加 8 byte 對齊，一次加 16。

用 gdb 確認 `call` 那一刻的狀態。`float9.gdb`（檔案：[`float9.gdb`](../small-project/s1-05-asm-calls/float9.gdb)），`main+113` 是 `call printf` 那一行（`0x11ba − 0x1149 = 0x71 = 113`）：

```
break *main
break *main+113
run
echo == 進 main 時 ==\n
info registers rsp
continue
echo == 執行 call printf 之前 ==\n
info registers rsp
x/gx $rsp
x/fg $rsp
p $xmm0.v2_double[0]
p $xmm7.v2_double[0]
p $rax
```

```
$ gdb -q -batch -x float9.gdb ./float9
Breakpoint 1 at 0x1149: file float9.c, line 5.
Breakpoint 2 at 0x11ba: file float9.c, line 6.

Breakpoint 1, main () at float9.c:5
5	{
== 進 main 時 ==
rsp            0x7fffffffd4b8      0x7fffffffd4b8

Breakpoint 2, 0x00005555555551ba in main () at float9.c:6
6	    printf("%.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f\n",
== 執行 call printf 之前 ==
rsp            0x7fffffffd4a0      0x7fffffffd4a0
0x7fffffffd4a0:	0x4022000000000000
0x7fffffffd4a0:	9
$1 = 1
$2 = 8
$3 = 8
```

逐行對照：

- 進 `main` 時 `%rsp` ＝ `…d4b8`（除以 16 餘 8）；到 `call` 前是 `…d4a0`（餘 0）。中間少了 24 byte：`push %rbp` 8、對齊 8、第 9 個參數 8。
- `x/gx $rsp` 印出頂端 8 byte 是 `0x4022000000000000`，跟 `.rodata` 的 `2078` 一樣；`x/fg $rsp` 把同樣的 8 byte 當浮點數印，就是 `9`。格式字母 `f`（浮點數）、大小字母 `g`（8 byte）的全表見 [gdb完整命令參考第三節](./gdb完整命令參考.md#三gdb-內部命令的共通文法)〈輸出格式 `/FMT`：`print` 與 `x` 共用〉。
- `p` 是 `print` 的縮寫。`%xmm` 暫存器是 16 byte，gdb 把它看成好幾種排法（`v2_double` ＝ 當成 2 個 `double`、`v4_float` ＝ 4 個 `float`……），`movsd` 只用最低的 8 byte，就是 `v2_double[0]`。`%xmm0` ＝ 1、`%xmm7` ＝ 8，跟上面的推論一樣。
- `p $rax` ＝ 8：`mov $0x8,%eax` 把整個 `%rax` 設成 8，`%al` 就是 8。

---

## 八、本課用到的工具、選項與完整清單在哪

### `gdb`：本課用到的命令

gdb 的頂層命令共 173 個，本課用到其中 9 個（`info registers`、`info symbol` 是 `info` 這一個頂層命令的兩個子命令）；全部依 gdb 自己的 13 類列在 [gdb完整命令參考第四節](./gdb完整命令參考.md#四全部-173-個頂層命令依-gdb-的-13-類)，`info` 的全部 65 個子命令在[第五節](./gdb完整命令參考.md#五info-的全部-65-個子命令)。

| 命令 | 本課怎麼用 | 在參考筆記的哪一類 |
|---|---|---|
| `break *位址` | `break *main+27`：停在某一條指令上（不跳過序言） | 4.2 breakpoints；`*位址` 的寫法見第三節〈位置（LOCATION）〉 |
| `run` | 開始執行 | 4.1 running |
| `stepi`（`si`） | 剛好執行一條機器指令，遇到 `call` 會跟進去 | 4.1 running |
| `continue`（`c`） | 繼續跑到下一個中斷點 | 4.1 running |
| `x/FMT 位址` | `x/2gx $rsp`：看堆疊頂端兩格；`x/i $pc`：看下一條指令；`x/fg $rsp`：把頂端 8 byte 當浮點數印（第七節補教） | 4.3 data；格式字母全表見第三節〈輸出格式 `/FMT`〉 |
| `info registers 名字…` | 只印指定的暫存器 | 第五節（`info` 子命令） |
| `info symbol 位址` | 位址落在哪個符號＋偏移 | 第五節（`info` 子命令） |
| `print` | `print &a`：看變數的位址；`p $xmm0.v2_double[0]`：把 `%xmm0` 最低 8 byte 當 `double` 印（第七節補教，`p` 是縮寫） | 4.3 data |
| `echo 文字` | 在命令稿裡印標題（本課只當分隔用） | 4.7 support |
| `set 設定 值` | `set disassemble-next-line off`：`stepi` 停下時不要自動印出下一條指令的反組譯（這本來就是預設值，寫出來是確保輸出一致；命令稿用 `x/i $pc` 自己印） | 4.3 data；`set` 底下的子命令多到上千個，見[第六節](./gdb完整命令參考.md#六setshowmaintenance數量為什麼不逐項列怎麼查) |

`stepi` 和 `nexti` 的差別：`nexti` 遇到 `call` 不跟進去，整個被呼叫的函式當成一步執行完。想看 `call` 推返回位址的那一刻，要用 `stepi`。

**命令稿**：`gdb -q -batch -x 檔案 程式` 從檔案一行一行讀命令執行（`-x`），執行完就結束（`-batch`），不印開場說明（`-q`）。gdb 的全部命令列選項列在 [gdb完整命令參考第二節](./gdb完整命令參考.md#二啟動-gdb命令列文法與全部選項)。

### `gcc`、`objdump`：本課新用到的選項

- `gcc`：主線沒有新選項（2026-10-16 批改作業時的補教用到 `-fno-ipa-stack-alignment`，見第六節〈補教：被呼叫的函式就在同一個檔案時，`sub $0x8,%rsp` 為什麼不見了〉），用的都是第 4 課[第八節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#八本課用到的工具選項與完整清單在哪)列過的 `-O0`、`-O2`、`-c`、`-g`、`-fno-asynchronous-unwind-tables`。
- `objdump`：新用到 `-r`（`--reloc`）：顯示重定位項目，和 `-d` 一起用時印在相關的指令下面。`-d`、`--no-show-raw-insn` 第 4 課教過，第七節補教用到的 `-s`、`-j` 也是第 4 課教過的；「顯示什麼」一組的全部 31 項在 [objdump能不能代替ldd第六節](./objdump能不能代替ldd找依賴庫-需求單與實際載入.md#六本篇用到的工具選項)。

### 本課的指令

本課新教的指令是 6 個：`push`、`pop`、`call`、`ret`、`leave`、`sub`（`sub` 第 4 課教過算術，這裡用來挪 `%rsp`）。看到但不需要學的：`endbr64`（第 4 課）、搬浮點資料的 `movq`／`movsd`／`movapd`。
完整的指令清單與精確定義見第 4 課第八節〈指令與指示詞的完整清單〉列的 Intel 手冊與 <https://www.felixcloutier.com/x86/>。
呼叫慣例的完整規則（結構體怎麼傳、浮點數、超過 16 byte 的回傳值……本課都沒有教）在 *System V Application Binary Interface, AMD64 Architecture Processor Supplement* 第 3.2 節。

---

## 九、動手作業

作業在 [`small-project/s1-05-asm-calls/`](../small-project/s1-05-asm-calls/README.md)，共 5 題：
不同型別的參數放進哪個暫存器的哪個名字、9 個參數的堆疊位置、用 gdb 找出兩層呼叫的返回位址與 `%rbp` 鏈、要跨過兩次 `call` 保存的值與對齊、可變參數函式的 `%al`。

---

## 自我測驗

> 參考答案寫在每題下面，第一次作答批改完才會補上。作答時先遮住答案、自己想過再對照。作答寫在 [`s1-05-asm-calls/quiz.md`](../small-project/s1-05-asm-calls/quiz.md)。

1. `frame.c` 的 `main` 裡有 `call 1149 <add3>`（位址 `0x1194`），下一條指令在 `0x1199`；`add3` 最後一條是 `ret`。`call` 做了哪兩件事？`ret` 做了什麼？`ret` 怎麼知道要回到 `0x1199`？如果 `add3` 裡有個陣列越界寫入，剛好蓋掉了堆疊上的返回位址，`ret` 會發生什麼事？
   ```asm
   # main
   1194:	call   1149 <add3>
   1199:	mov    %rax,-0x8(%rbp)
   # add3 的最後
   1177:	pop    %rbp
   1178:	ret
   ```
   > **參考答案**（核心關鍵＋詳細說明）
   >
   > **1-1**【[第三節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#三call-與-ret用-gdb-一條一條走)】核心關鍵：
   >
   > 1. 把**返回位址**（`call` 下一條指令的位址 `0x1199`）推進堆疊：`%rsp` 減 8，寫進新的頂端。
   > 2. 跳到 `add3`（`%rip` 變成 `add3` 的位址）。
   >
   > 詳細說明：`call add3` ＝ `push 返回位址` ＋ `jmp add3`。`frame.c` 實測：`call` 之後 `%rsp` 從 `d410` 變成 `d408`，`d408` 裡是 `0x555555555199`（PIE 載入後的 `0x1199`）。
   >
   > **1-2**【[第三節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#三call-與-ret用-gdb-一條一條走)】核心關鍵：
   >
   > 1. 把堆疊頂端那 8 byte 當成位址，放進 `%rip`（跳過去）。
   > 2. `%rsp` 加 8（返回位址被拿走）。
   >
   > 詳細說明：`ret` ＝ `pop` 到 `%rip`。沒有指令能直接 `mov` 到 `%rip`，`ret` 就是「從堆疊取一個值放進 `%rip`」。實測 `ret` 之後 `%rsp` 回到 `d410`，跟 `call` 之前一模一樣。
   >
   > **1-3**【[第三節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#三call-與-ret用-gdb-一條一條走)】核心關鍵：
   >
   > 1. `call` 把 `0x1199` 推進堆疊，存在 `%rsp` 指的那格記憶體裡。
   > 2. `ret` 不知道呼叫者是誰，只是取堆疊頂端那 8 byte 當位址。
   >
   > 詳細說明：所以 `add3` 在 `ret` 之前必須把 `%rsp` 還原到放返回位址的那一格：`pop %rbp` 把序言推的舊 `%rbp` 拿走，頂端就又是 `0x1199`（實測第 5 段：停在 `ret` 時 `%rsp` ＝ `d408`、裡面是 `0x555555555199`）。函式裡 `push` 和 `pop` 沒配好，`ret` 就會跳錯。
   >
   > **1-4**【[第三節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#三call-與-ret用-gdb-一條一條走)】核心關鍵：
   >
   > 1. `ret` 不檢查，照樣把被蓋掉的值當成位址跳過去。
   >
   > 詳細說明：跳到亂的地方，通常是 segfault 當掉（那個位址沒有對映，或不能執行）；題目只問 `ret` 的行為，這點不列入核心關鍵（2026-10-25 改判後調整）。第 3 課實際看過兩種下場：有堆疊保護時，函式返回**之前**檢查金絲雀，印 `stack smashing detected` 中止；關掉保護（`-fno-stack-protector`）就真的走到 `ret`，返回時 segfault（[第 3 課第五節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#五c-不檢查邊界越界的三種下場)〈關掉保護，看看下面是什麼：返回時 segfault〉）。如果蓋上去的值是攻擊者刻意挑的位址，`ret` 就會跳去執行攻擊者要的程式碼——這是緩衝區溢位攻擊的原理，也是堆疊保護存在的理由。
2. 函式開頭的 `push %rbp` 和 `mov %rsp,%rbp` 各在做什麼？`main` 結尾的 `leave` 等於哪兩條指令？`add3` 把區域變數放在 `%rsp` 下面、沒有 `sub %rsp`，`main` 卻要 `sub $0x10,%rsp`，為什麼？
   ```asm
   add3:                                 main:
     push   %rbp                           push   %rbp
     mov    %rsp,%rbp                      mov    %rsp,%rbp
     mov    %rdi,-0x18(%rbp)               sub    $0x10,%rsp
     ...（沒有 sub、沒有 call）              ...（有 call add3、call printf）
     pop    %rbp                           leave
     ret                                   ret
   ```
   > **參考答案**（核心關鍵＋詳細說明）
   >
   > **2-1**【[第四節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#四堆疊框pushq-rbp-movq-rsp-rbp-與-leave)】核心關鍵：
   >
   > 1. `push %rbp`：把**呼叫者的** `%rbp` 存到堆疊上，返回前要還原。
   > 2. `mov %rsp,%rbp`：把現在的頂端定成自己堆疊框的**基準點**，之後區域變數用 `%rbp` 加固定的負偏移存取。
   >
   > 詳細說明：實測：`add3` 的 `push %rbp` 之後，`d400` 裡是 `main` 的 `%rbp`（`d420`）；`mov` 之後 `%rbp` ＝ `d400`。每個框的 `%rbp` 那格都存著上一層的 `%rbp`，串成一條鏈，`bt` 就是沿著它往回找。
   >
   > **2-2**【[第四節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#四堆疊框pushq-rbp-movq-rsp-rbp-與-leave)〈`leave`：一條指令做兩件事〉】核心關鍵：
   >
   > 1. `mov %rbp,%rsp`：把 `%rsp` 拉回基準點，`sub` 開的空間一次丟掉。
   > 2. `pop %rbp`：還原呼叫者的 `%rbp`。
   >
   > 詳細說明：`leave.gdb` 實測：`leave` 前 `%rsp` ＝ `d410`、`%rbp` ＝ `d420`；之後 `%rsp` ＝ `d428`（拉回 `d420` 再 `pop` 走 8 byte）、`%rbp` ＝ `d4c0`。只 `push` 沒 `sub` 的函式（`add3`）用 `pop %rbp` 就夠了。
   >
   > **2-3**【[第四節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#四堆疊框pushq-rbp-movq-rsp-rbp-與-leave)〈上圖 `add3` 的變數在 `%rsp` 的「下面」：紅區〉】核心關鍵：
   >
   > 1. **紅區**：ABI 保證 `%rsp` 下方 128 byte 不會被別人蓋掉；`add3` 是葉函式，自己也不會 `push`／`call`，所以直接用、省一條 `sub`。
   > 2. `main` 要 `call`：`call` 推返回位址會寫在 `%rsp` 下面、蓋掉那一區，`call` 之後還要用的變數就壞了，所以 gcc 對會 `call` 的函式一律 `sub`，把變數空間放進堆疊（`%rsp` 上面）。
   >
   > 詳細說明：兩個核心關鍵各管一半：`main` 要 `sub` 跟紅區無關（只因為它會 `call`），紅區解釋的是 `add3` 為什麼可以不 `sub`；用 `-mno-red-zone` 編，`main` 不變、`add3` 多出 `sub $0x28,%rsp`（第四節〈那自我測驗 2-3 為什麼要提紅區〉，2026-10-25 補上）。`sub` 的大小是 `0x10` 不是 8，才是 16 byte 對齊的緣故（[第六節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#六誰負責保存暫存器以及-16-byte-對齊)〈上面的 `sub $0x8,%rsp`：16 byte 對齊〉：進 `main` 時 `d428`，`push` 後 `d420`，`sub $0x10` 後 `d410` 是 16 的倍數）。`frame.c` 的 `r` 碰巧在 `call add3` 回來才寫入、`call printf` 前已複製到 `%rsi`，所以拿掉 `sub` 其實也能跑；gcc `-O0` 不做這種分析，會 `call` 的函式一律 `sub`（[第四節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#四堆疊框pushq-rbp-movq-rsp-rbp-與-leave)〈那 `frame.c` 的 `main` 拿掉 `sub $0x10,%rsp` 會怎樣：其實照樣能跑〉）。kernel 不能用紅區（中斷會直接推資料到目前的堆疊上），所以編譯時加 `-mno-red-zone`。
3. `args8.c` 的 `sum8` 有 8 個 `long` 參數。前 6 個參數放在哪裡？第 7、8 個放在哪裡、`call8` 用什麼順序放？在 `sum8` 裡第 7 個參數為什麼是 `0x10(%rbp)`，而不是 `0(%rbp)`？`call8` 在 `call` 之後為什麼要 `add $0x10,%rsp`？
   ```c
   long sum8(long a, long b, long c, long d, long e, long f, long g, long h)
   {
       return a + b + c + d + e + f + g + h;
   }

   long call8(void)
   {
       return sum8(1, 2, 3, 4, 5, 6, 7, 8);
   }
   ```
   ```asm
   call8:                                sum8 裡讀第 7、8 個參數：
     push   %rbp                           mov    0x10(%rbp),%rax
     mov    %rsp,%rbp                      ...
     push   $0x8                           mov    0x18(%rbp),%rax
     push   $0x7
     mov    $0x6,%r9d
     ...（5～1 分別放進 %r8d、%ecx、%edx、%esi、%edi）
     call   sum8
     add    $0x10,%rsp
     leave
     ret
   ```
   > **參考答案**（核心關鍵＋詳細說明）
   >
   > **3-1**【[第五節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#五參數與回傳值放哪裡)】核心關鍵：
   >
   > 1. 照順序放 `%rdi`、`%rsi`、`%rdx`、`%rcx`、`%r8`、`%r9`。
   >
   > 詳細說明：`call8` 寫的是 `%r9d`、`%ecx` 這些 32 位元的名字：常數小，寫 32 位元會清掉高 32 位元，整個 64 位元就是那個值，指令還比較短。回傳值放 `%rax`。
   >
   > **3-2**【[第五節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#五參數與回傳值放哪裡)〈實驗：8 個參數，第 7、8 個去哪了〉】核心關鍵：
   >
   > 1. 放在堆疊上（呼叫者用 `push` 推進去，每個 8 byte）。
   > 2. 從最後一個開始推：先 `push $0x8` 再 `push $0x7`，所以第 7 個在較低的位址。
   >
   > 詳細說明：倒著推，第 7 個才會離 `sum8` 的堆疊框最近（在 `0x10(%rbp)`），第 8 個接在上面（`0x18(%rbp)`），參數個數不固定時也能從固定位置往上數。
   >
   > **3-3**【[第五節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#五參數與回傳值放哪裡)〈實驗：8 個參數，第 7、8 個去哪了〉】核心關鍵：
   >
   > 1. `call8` 推完參數後，`call` 又推了返回位址、`sum8` 的序言又推了舊 `%rbp`。
   > 2. 所以從 `sum8` 的 `%rbp` 往上：`0(%rbp)` 舊 `%rbp`、`0x8(%rbp)` 返回位址、`0x10(%rbp)` 才是第 7 個。
   >
   > 詳細說明：記法：**參數用正的偏移（在呼叫者那邊），區域變數用負的偏移（在自己這邊）**。作業 2-2 預測成負偏移，這次答對了。
   >
   > **3-4**【[第五節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#五參數與回傳值放哪裡)〈實驗：8 個參數，第 7、8 個去哪了〉】核心關鍵：
   >
   > 1. 第 7、8 個參數是 `call8` 自己 `push` 的（2 × 8 ＝ 16 byte）。
   > 2. `sum8` 的 `ret` 只拿走返回位址、不清參數，所以由呼叫者 `call8` 在 `call` 回來後用 `add $0x10,%rsp` 丟掉。
   >
   > 詳細說明：這裡 `add` 之後緊接著 `leave`，其實 `leave` 的 `mov %rbp,%rsp` 也會把 `%rsp` 拉回去；`-O0` 照規則先清掉參數，不管後面有沒有 `leave`。如果 `call8` 接著還要呼叫別人，不清的話推過的參數就會一直堆在堆疊上。
4. 下面兩個函式用 `-O2` 編譯（`g` 只有宣告）。為什麼 `saved.c` 的 `f` 要 `push %rbx`，而不是讓 `x` 留在 `%rdi`？`noret.c` 的 `f` 不需要保存任何值，為什麼還要 `sub $0x8,%rsp`？
   ```c
   /* saved.c */                          /* noret.c */
   long g(long x);                         long g(long x);

   long f(long x)                          long f(long x)
   {                                       {
       long y = g(x);                          long y = g(x);
       return x + y;                           return y * 2;
   }                                       }
   ```
   ```asm
   # saved.c 的 f                          # noret.c 的 f
   push   %rbx                             sub    $0x8,%rsp
   mov    %rdi,%rbx                        call   g
   call   g                                add    $0x8,%rsp
   add    %rbx,%rax                        add    %rax,%rax
   pop    %rbx                             ret
   ret
   ```
   > **參考答案**（核心關鍵＋詳細說明）
   >
   > **4-1**【[第六節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#六誰負責保存暫存器以及-16-byte-對齊)〈實驗：要跨過 `call` 保存的值放哪裡〉】核心關鍵：
   >
   > 1. `x` 在 `g` 回來後還要用，但 `%rdi` 是 caller-saved，`g` 可能改掉它。
   > 2. 所以把 `x` 搬到 callee-saved 的 `%rbx`：ABI 保證 `g` 回來時 `%rbx` 不變。
   > 3. `f` 借用 `%rbx` 也要對自己的呼叫者負責：開頭 `push %rbx`、返回前 `pop %rbx` 還原。
   >
   > 詳細說明：對照 `noret.c`：`call` 之後不再用 `x`，就不需要 `%rbx`，也沒有 `push`／`pop %rbx`。`saved.c` 的 `push %rbx` 還順便把 `%rsp` 挪了 8 byte，同時滿足 16 byte 對齊，一舉兩得（4-2）。
   >
   > **4-2**【[第六節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#六誰負責保存暫存器以及-16-byte-對齊)〈上面的 `sub $0x8,%rsp`：16 byte 對齊〉】核心關鍵：
   >
   > 1. ABI 規定：執行 `call` 的那一刻，`%rsp` 必須是 16 的倍數。
   > 2. 呼叫 `f` 的人照規定對齊，但 `call f` 推了 8 byte 返回位址，進到 `f` 時 `%rsp` 是「16 的倍數 − 8」。
   > 3. `f` 要 `call g`，必須再挪 8 byte：沒東西要存，就用 `sub $0x8,%rsp` 單純挪。
   >
   > 詳細說明：`saved.c` 不需要 `sub`，因為 `push %rbx` 剛好也挪了 8 byte。要 16 的原因：有些指令（例如 SSE 的 `movaps`）要求記憶體位址是 16 的倍數，否則會當掉。
5. 情境題：你用 `break *add3` 停在 `add3` 的第一條指令（還沒執行 `push %rbp`），gdb 顯示 `add3 (a=0, b=0, c=0)`，可是 `main` 明明傳的是 1、2、3。
   - 為什麼 gdb 印錯？這時候要看真正的參數，該看哪裡？
   - 同一個時間點，你想知道「是誰呼叫了 `add3`」，可是不能用 `bt`。根據本課，你會看哪個暫存器指的位址？看到的值代表什麼？
   ```c
   long add3(long a, long b, long c)
   {
       long s = a + b;
       return s + c;
   }
   ```
   ```asm
   add3:
     endbr64                 ← 停在這裡
     push   %rbp
     mov    %rsp,%rbp
     mov    %rdi,-0x18(%rbp)
     mov    %rsi,-0x20(%rbp)
     mov    %rdx,-0x28(%rbp)
     ...
   ```
   > **參考答案**（核心關鍵＋詳細說明）
   >
   > **5-1**【[第三節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#三call-與-ret用-gdb-一條一條走)】核心關鍵：
   >
   > 1. gdb 照除錯資訊去堆疊上的 `-0x18(%rbp)` 等位置讀參數（`-O0` 的除錯資訊只記堆疊那一格）。
   > 2. 停在第一條指令時序言還沒跑，參數還沒存進那幾格 → 讀到的是舊值。（2026-10-19 更正：原本寫「`%rbp` 也還是 `main` 的」是錯的，gdb 用 CFA 算位址，位址是對的，見[第三節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#三call-與-ret用-gdb-一條一條走)〈gdb 為什麼去堆疊讀參數，而不是讀暫存器〉。）
   > 3. 真正的參數在 `%rdi`、`%rsi`、`%rdx`（`info registers rdi rsi rdx`）。
   >
   > 詳細說明：到序言跑完、參數存好之後（例如停在 `ret` 前），gdb 就印出正確的 `a=1, b=2, c=3`。這也是 `break add3`（不加 `*`）要跳過開頭準備指令的原因——第 4 課組語 11-3 卡過的同一件事。
   >
   > **5-2**【[第三節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#三call-與-ret用-gdb-一條一條走)〈停在函式入口時，怎麼知道是誰呼叫的〉】核心關鍵：
   >
   > 1. 看 `%rsp` 指的那 8 byte（`x/gx $rsp`）。
   > 2. 那是返回位址：`ret` 會跳回去的地方，也就是呼叫者裡 `call` 下一條指令的位址。
   >
   > 詳細說明：要真的查出呼叫者，再看這個位址落在哪個函式（`info symbol *(long *)$rsp` → `main + 32`），那個函式就是呼叫者；題目沒有要求寫出這一步（2026-10-19 改判後從核心關鍵移到這裡）。只有停在**第一條指令**時頂端才是返回位址；`push %rbp` 之後返回位址在 `8(%rbp)`。`bt` 做的事本質上一樣：沿著堆疊一層一層找返回位址。
6. （2026-10-25 提問延伸）ABI 說 `%rsp` 下方 128 byte 的紅區「不會被蓋掉」，可是 `main` 一執行 `call add3`，返回位址就寫在 `%rsp` 下面。這算不算 `main` 違反了紅區的規定？紅區保證的是「誰」不會蓋掉它？為什麼放在紅區的值「不能跨過函式呼叫」？
   `frame.c`（檔案：[`frame.c`](../small-project/s1-05-asm-calls/frame.c)）完整原始碼與編譯指令：
   ```c
   #include <stdio.h>

   long add3(long a, long b, long c)
   {
       long s = a + b;
       return s + c;
   }

   int main(void)
   {
       long r = add3(1, 2, 3);
       printf("%ld\n", r);
       return 0;
   }
   ```
   ```sh
   gcc -O0 -g -fno-asynchronous-unwind-tables frame.c -o frame
   objdump -d --no-show-raw-insn frame
   ```
   ```asm
   add3:                                 main:
     push   %rbp                           push   %rbp
     mov    %rsp,%rbp                      mov    %rsp,%rbp
     mov    %rdi,-0x18(%rbp)               sub    $0x10,%rsp     # 把 %rsp 往下挪 16 byte
     ...（變數都放在 %rsp 下面）               ...（有 call add3、call printf）
     pop    %rbp                           leave
     ret                                   ret
   ```
