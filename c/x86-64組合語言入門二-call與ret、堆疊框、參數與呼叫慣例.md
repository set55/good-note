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

**gdb 在第 1 段結尾印的 `add3 (a=0, b=0, c=0)` 是錯的**：那時 `stepi` 剛進到 `add3` 的第一條指令。gdb 照除錯資訊去 `-0x18(%rbp)` 這些位置讀 `a`、`b`、`c`，可是參數還在暫存器裡、還沒存進去，而且 `%rbp` 還是 `main` 的值，所以讀到的是不相干的記憶體。
到了第 5 段，堆疊框已經建好、參數也存好了，gdb 就印出正確的 `a=1, b=2, c=3`。這正是 `break add3`（不加 `*`）要跳過開頭準備指令的原因（零、第 2 點）。

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
`main` 要呼叫 `add3` 和 `printf`，`call` 會把返回位址推到 `%rsp` 下面，蓋掉那一區，所以 `main` 必須老老實實 `sub $0x10,%rsp`。

> kernel 的程式碼不能用紅區：在 kernel 裡，中斷發生時 CPU 會直接把資料推到目前的堆疊上，會蓋掉 `%rsp` 下面的東西。所以 kernel 編譯時加了 `-mno-red-zone`（Linux 原始碼 `arch/x86/Makefile`；本機未實測）。

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
| `x/FMT 位址` | `x/2gx $rsp`：看堆疊頂端兩格；`x/i $pc`：看下一條指令 | 4.3 data；格式字母全表見第三節〈輸出格式 `/FMT`〉 |
| `info registers 名字…` | 只印指定的暫存器 | 第五節（`info` 子命令） |
| `info symbol 位址` | 位址落在哪個符號＋偏移 | 第五節（`info` 子命令） |
| `print` | `print &a`：看變數的位址 | 4.3 data |
| `echo 文字` | 在命令稿裡印標題（本課只當分隔用） | 4.7 support |
| `set 設定 值` | `set disassemble-next-line off`：`stepi` 停下時不要自動印出下一條指令的反組譯（這本來就是預設值，寫出來是確保輸出一致；命令稿用 `x/i $pc` 自己印） | 4.3 data；`set` 底下的子命令多到上千個，見[第六節](./gdb完整命令參考.md#六setshowmaintenance數量為什麼不逐項列怎麼查) |

`stepi` 和 `nexti` 的差別：`nexti` 遇到 `call` 不跟進去，整個被呼叫的函式當成一步執行完。想看 `call` 推返回位址的那一刻，要用 `stepi`。

**命令稿**：`gdb -q -batch -x 檔案 程式` 從檔案一行一行讀命令執行（`-x`），執行完就結束（`-batch`），不印開場說明（`-q`）。gdb 的全部命令列選項列在 [gdb完整命令參考第二節](./gdb完整命令參考.md#二啟動-gdb命令列文法與全部選項)。

### `gcc`、`objdump`：本課新用到的選項

- `gcc`：沒有新選項，用的都是第 4 課[第八節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#八本課用到的工具選項與完整清單在哪)列過的 `-O0`、`-O2`、`-c`、`-g`、`-fno-asynchronous-unwind-tables`。
- `objdump`：新用到 `-r`（`--reloc`）：顯示重定位項目，和 `-d` 一起用時印在相關的指令下面。`-d`、`--no-show-raw-insn` 第 4 課教過；「顯示什麼」一組的全部 31 項在 [objdump能不能代替ldd第六節](./objdump能不能代替ldd找依賴庫-需求單與實際載入.md#六本篇用到的工具選項)。

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
