# `-O0` 是什麼意思？—— gcc 最佳化等級，以及為什麼除錯要用 `-O0`

> 日期：2026-09-22
> 情境：第 ① 站第 1 課（[hello.c怎麼變成能跑的程式](./hello.c怎麼變成能跑的程式-編譯的四個階段.md)）用了 `gcc -S -O0`，追問 `-O0` 是什麼
> 適用範圍：實測本機 gcc 13.3.0（x86-64）；選項說明依本機 `man gcc` 的 Optimize Options 一節
> 相關：[hello.c怎麼變成能跑的程式-編譯的四個階段.md](./hello.c怎麼變成能跑的程式-編譯的四個階段.md)

**一句話結論：`-O0` 是大寫字母 O 加數字 0，意思是「最佳化等級 0」，也就是不最佳化，而且是 gcc 的
預設值。不最佳化時，C 的每個變數都老實放在記憶體、每一行都照順序執行，所以 `gdb` 看到的就是原始碼的樣子；
`-O2` 會重排、合併、刪除、甚至在編譯時就把答案算好，程式快很多，但除錯時變數會「不見」。
開發除錯用 `-O0`（或 `-Og`），正式發行用 `-O2`，Linux kernel 本身也是用 `-O2` 編的。**

---

## 一、文法：`-O<等級>`

```
gcc -O<等級> ...
     │  └─ 0、1、2、3、s、z、g、fast，或什麼都不寫
     └─ 大寫字母 O（Optimize），不是數字 0
```

| 寫法 | 意思（依 `man gcc`） |
|---|---|
| `-O0` | 減少編譯時間，讓除錯結果符合預期（*make debugging produce the expected results*）。**這是預設值** |
| `-O`、`-O1` | 最佳化。不做太花編譯時間的最佳化 |
| `-O2` | 再多一些：幾乎所有「不以程式大小換速度」的最佳化都開。發行版套件、kernel 的標準等級 |
| `-O3` | 在 `-O2` 之上再開更激進的迴圈最佳化（展開、向量化等），程式常會變大 |
| `-Os` | 以**大小**為目標：`-O2` 裡會讓程式變大的那幾項不開 |
| `-Oz` | 比 `-Os` 更積極地縮小，可能犧牲速度 |
| `-Og` | 以**除錯體驗**為目標：只開不妨礙除錯的最佳化。`man gcc` 建議「編輯－編譯－除錯」循環用它 |
| `-Ofast` | `-O3` 加上**不嚴格遵守標準**的最佳化（例如浮點數運算的精確度），要確定程式能接受才用 |

規則：
- **同一行出現好幾個 `-O`，以最後一個為準**（`man gcc`：*the last such option is the one that is effective*）。
- 單獨開某一個最佳化旗標（`-f...`）在 `-O0` 下多半沒用：`man gcc` 寫明 *Most optimizations are completely
  disabled at -O0 ... even if individual optimization flags are specified*。

每個等級實際開了哪些旗標，可以直接問 gcc：

```sh
gcc -O2 -Q --help=optimizers | grep enabled
```

本機數出來的 `[enabled]` 數量：`-O0` 52、`-Og` 80、`-O1` 92、`-Os`／`-Oz` 129、`-O2` 137、`-O3` 150。
（`-O0` 的 52 個大多是預設開著、但在 `-O0` 下實際不起作用的旗標，見上面那條規則。）

---

## 二、實驗：同一段程式，不同等級編出什麼

```c
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
```

### `-O0`：照字面，一步一步來

```asm
sum_to:
    pushq %rbp
    movq  %rsp, %rbp
    movl  %edi, -20(%rbp)      ← 參數 n 從暫存器存回堆疊（記憶體）
    movl  $0, -8(%rbp)         ← total = 0，放在記憶體
    movl  $1, -4(%rbp)         ← i = 1，放在記憶體
    ...
    movl  -4(%rbp), %eax       ← 每一圈：從記憶體讀 i
    addl  %eax, -8(%rbp)       ←         total += i，寫回記憶體
    addl  $1, -4(%rbp)         ←         i++，寫回記憶體
    ...
answer:
    movl  $100, %edi
    call  sum_to               ← 真的去呼叫 sum_to
```

每個變數都有固定的記憶體位置（`-4(%rbp)` 是 `i`、`-8(%rbp)` 是 `total`），每一圈都讀寫記憶體。
慢，但**原始碼的每一行都對得到一段組合語言**。

### `-O2`：答案在編譯時就算好了

```asm
answer:
    movl  $5050, %eax          ← 1+2+…+100 = 5050，編譯器自己算完了
    ret
```

`answer()` 裡的 `sum_to(100)` 被**內嵌（inline）**進來，參數是常數，整個迴圈就在編譯時算掉，
連呼叫都不見了。

**「內嵌」是什麼**（2026-09-22 補充，原本沒有定義這個詞）：把**被呼叫的函式本體整段複製**到呼叫它的地方，
取代「呼叫」這個動作。內嵌的對象是**函式**，不是數字。用 C 的樣子表示編譯器做的兩步（實際改寫發生在編譯器內部）：

```c
/* 原本 */
int answer(void) { return sum_to(100); }

/* 第 1 步：內嵌——把 sum_to 的本體搬進來，n 換成呼叫時給的 100 */
int answer(void) {
    int total = 0;
    for (int i = 1; i <= 100; i++)
        total += i;
    return total;
}

/* 第 2 步：常數摺疊（constant folding）——所有數字編譯時都已知，直接算出結果 */
int answer(void) { return 5050; }
```

第 2 步能做，是因為第 1 步之後迴圈的次數（100）在編譯時就知道了；如果只看 `sum_to(int n)` 本身，
`n` 是呼叫的人才知道的值，編譯器沒辦法事先算。**兩步缺一不可**。`sum_to` 本身（給其他人呼叫的版本）也被改寫：變數都留在暫存器裡，
迴圈一次處理兩個數。

### 大小（`.text` 區段，`size -A` 實測）

| 等級 | bytes | 說明 |
|---|---|---|
| `-Oz` | 30 | 最小 |
| `-Os` | 35 | |
| `-Og` | 43 | |
| `-O1` | 55 | |
| `-O0` | 70 | 沒最佳化，冗長 |
| `-O2` | 90 | 為了速度把迴圈改寫得比較長 |
| `-O3` | 234 | 迴圈展開、向量化，換來最大的程式 |

「最佳化」不等於「變小」：`-O2`、`-O3` 是用空間換時間。

---

## 三、為什麼除錯要用 `-O0`

同一個程式用 `-g`（加入除錯資訊）編兩次，在 `gdb` 裡停在 `sum_to`、往下走兩行、看區域變數：

```
=== -O0
Breakpoint 1, sum_to (n=10) at sum.c:3
3	    int total = 0;           ← 停在函式第一行，逐行往下
...
i = 1
total = 0                         ← 跟原始碼的進度一致

=== -O2
Breakpoint 1, sum_to (n=n@entry=10) at sum.c:2
2	{                            ← 停的位置就不一樣了
...
i = 2
total = 1                         ← 走了兩行，但迴圈已經跑到別的地方：指令被重排過
```

`-O2` 下，原始碼的「一行」已經不對應一段連續的指令，變數可能只活在暫存器裡、甚至被整個刪掉
（更複雜的程式裡，`gdb` 常印出 `<optimized out>`）。所以：

| 情境 | 用什麼 |
|---|---|
| 學習、看組合語言對照原始碼、用 `gdb` 單步 | `-O0 -g` |
| 日常開發，想除錯又不想太慢 | `-Og -g` |
| 正式發行 | `-O2`（需要時加 `-g` 另外保存除錯資訊） |
| 嵌入式、空間很緊（例如路由器韌體） | `-Os`／`-Oz` |

### `gdb` 是什麼（補教，2026-09-22）

> 本節是批改自我測驗時補上的：上面第三節直接給了 `gdb` 的輸出，卻沒有先介紹 `gdb`。
> 完整的 `gdb` 用法排在第 ① 站第 3 課，這裡只講看懂本篇需要的部分。

`gdb`（GNU Debugger，GNU 除錯器）讓你**把程式停在某一行、一行一行往下走、隨時看變數的值**。
它能把「機器碼的位址」對回「原始碼的第幾行、哪個變數」，靠的是編譯時加的 **`-g`**：
`-g` 會把**除錯資訊（debug information）**——每段機器碼對應哪一行、每個變數放在哪裡——寫進執行檔。
**`-g` 跟 `-O` 是兩回事**：`-g` 決定「有沒有對照表」，`-O` 決定「程式被改寫了多少」。

本篇用到的 `gdb` 命令如下。**這只是本機 gdb 173 個頂層命令中的 7 個**；全部命令、啟動選項與文法見
[gdb完整命令參考.md](./gdb完整命令參考.md)。

| 命令 | 作用 |
|---|---|
| `break <函式或行號>` | 設中斷點：程式跑到這裡就停下來 |
| `run` | 開始執行程式（跑到第一個中斷點停住） |
| `next` | 執行目前這一行，停在下一行（遇到函式呼叫不會進去） |
| `print <變數>` | 印出變數目前的值 |
| `info locals` | 印出目前函式所有區域變數 |
| `bt`（backtrace） | 印出「誰呼叫了誰」的呼叫鏈 |
| `continue` | 繼續跑，直到下一個中斷點或程式結束 |

本機實測一個算平均的小程式 `average(int *a, int n)`，三種編法的差別。

> 補充（2026-09-23）：原始的 `avg.c`、`avg2.c` 當初只放在實驗用的暫存目錄，沒有保存，筆記也沒附原始碼。
> 下面是依輸出重建、並重新實測過的版本：`avg.c` 的行號、呼叫位置、結果都與下方輸出一致；
> `avg2.c` 重建版的中斷點停在第 5 行（原輸出是第 6 行，原檔大概多了一行），其餘一致。
> 輸出裡的堆疊位址（例如 `a=0x7fffffffd1cc`）每次執行、每種環境都可能不同，不用在意。

`avg.c`（`average` 會被內嵌、資料是常數）：

```c
#include <stdio.h>

int average(int *a, int n)
{
    int sum = 0;                          /* 第 5 行 */
    for (int i = 0; i < n; i++)
        sum += a[i];
    int avg = sum / n;
    return avg;
}

int main(void)
{
    int a[] = {3, 6, 9};
    printf("%d\n", average(a, 3));        /* 第 15 行 */
    return 0;
}
```

`avg2.c`（跟 `avg.c` 只差兩處：`average` 加上 `__attribute__((noinline))` 禁止內嵌；
陣列內容改用 `argc` 計算，讓編譯器在編譯時不知道資料，沒辦法事先算好）：

```c
#include <stdio.h>

__attribute__((noinline))
int average(int *a, int n)
{
    int sum = 0;
    for (int i = 0; i < n; i++)
        sum += a[i];
    int avg = sum / n;
    return avg;
}

int main(int argc, char **argv)
{
    int a[] = {argc * 3, argc * 6, argc * 9};
    printf("%d\n", average(a, 3));
    return 0;
}
```

- `__attribute__((noinline))`：gcc 的擴充語法，意思是「這個函式不准內嵌」，放在函式定義前面。
- `argc`：執行時命令列參數的個數（不加參數執行時是 1），編譯時無法得知，所以 `a[]` 的內容要到執行時才確定。

自己重現的指令（`gdb -q -batch -ex '<命令>' ...` 是「不進互動模式，依序執行 `-ex` 給的命令」，見 [gdb完整命令參考](./gdb完整命令參考.md)）：

```
$ gcc -g -O0 avg.c  -o avg_O0          # 情況一
$ gcc -g -O2 avg.c  -o avg_O2          # 情況二
$ gcc -g -O2 avg2.c -o avg2_O2         # 情況三
$ gcc    -O0 avg.c  -o avg_nog         # 情況四：沒有 -g
$ gdb ./avg_O0                          # 進入 gdb 後依序打 break average、run、bt、next……
```


**`-g -O0`：一切都看得到**

```
Breakpoint 1, average (a=0x7fffffffd1cc, n=3) at avg.c:5
5	    int sum = 0;
#1  0x00005555555551fd in main () at avg.c:15      ← bt：是 main 的第 15 行呼叫進來的
...（next 三次）
i = 0
sum = 3
avg = 0                                             ← 還沒執行到 avg = sum / n，所以是記憶體裡原本的值
```

因為 `-O0` 讓每個變數都有固定的記憶體位置（第二節），`gdb` 隨時都能去那個位置讀值。

**`-g -O2`，而且函式被內嵌：中斷點根本停不下來**

```
Breakpoint 1 at 0x1180: file avg.c, line 4.
6                                ← 程式直接印出答案結束了，沒有停在 average
```

`average` 被內嵌進 `main`、參數又是常數，整個計算在編譯時就做完了（跟第二節的 `answer()` 一樣），
執行時根本沒有「呼叫 `average`」這件事。

**`-g -O2`，強制不內嵌（`__attribute__((noinline))`）、資料也改成執行時才知道：看到 `<optimized out>`**

```
Breakpoint 1, average (a=a@entry=0x7fffffffd1b8, n=n@entry=3) at avg2.c:6
sum = <optimized out>
avg = <optimized out>
$3 = 3                          ← n 還看得到
（重建版 avg2.c 重新實測：停在 avg2.c:5，print sum／avg 同樣是 <optimized out>，print n 是 3）
```

`sum`、`avg` 在 `-O2` 下只活在暫存器裡、而且那個暫存器馬上被拿去做別的事，
除錯資訊沒辦法告訴 `gdb`「此刻它的值在哪」，於是顯示 `<optimized out>`。**變數在原始碼裡存在，
不代表它在執行時有一個可以讀的地方。**

**沒有 `-g`：`gdb` 還能停，但對不回原始碼**

```
Breakpoint 1, 0x0000555555555171 in average ()     ← 只有位址，沒有行號、沒有參數
No symbol "sum" in current context.                ← 不知道有 sum 這個變數
```

遇到 `<optimized out>` 的處理順序：

1. 能重新編譯的話，改用 `-O0 -g`（或 `-Og -g`）重編，再重現一次問題。
2. 不能改編譯方式時：看 `info locals` 哪些變數還在、在變數被使用的那一行停下來看、
   或直接讀組合語言看值在哪個暫存器（`gdb` 的 `info registers`，第 3 課再講）。
3. 最後手段：在程式裡加 `printf` 印出來。

---

## 四、跟 kernel 的關係

Linux kernel 預設用 `-O2` 編譯（組態裡也可以改選以大小為目標），**不能**用 `-O0` 編——kernel
有些程式碼依賴編譯器一定會做的內嵌與刪除死碼（dead code）才能編過。這代表：之後在 kernel 裡除錯，
**看到的永遠是最佳化過的程式碼**，變數 `<optimized out>` 是常態，要學會讀最佳化後的組合語言，
並改用 `printk`、tracing 等方法觀察。（這一點會在第二段 K0 實際打開 kernel 的 Makefile 時驗證；本篇沒有實測。）

也因此第 1 課那個「`-O0` 也會把 `printf` 換成 `puts`」的例子值得記住：**即使是最不最佳化的等級，
編譯器也不保證照字面做**。

---

## 自我測驗

1. `-O0` 的 `O` 和 `0` 各是什麼？為什麼說它是「預設值」——不加任何 `-O` 時會發生什麼？
2. （**第 ① 站學完「讀懂組合語言」那一課後再答**，現在不需要答）為什麼 `-O0` 的組合語言每一圈都在讀寫記憶體（`-4(%rbp)`），而 `-O2` 不會？這個差別對 `gdb` 有什麼影響？
3. 在 `-O2` 下，`answer()` 連呼叫 `sum_to` 這件事都不見了，直接回傳 5050。編譯器做了哪兩件事才能做到？
4. 「最佳化等級越高，程式越小」對嗎？用本篇的數字說明。
5. 情境題：你在自己的程式裡用 `-O2` 編譯，`gdb` 印某個變數時顯示 `<optimized out>`。你會怎麼處理？如果是在 kernel 裡遇到同樣的狀況，處理方式又有什麼不同？
