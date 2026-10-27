# C 的資料在記憶體裡長怎樣？—— 位元組、指標、位元組順序與有號／無號

> 課程：[Linux 與系統程式系統化學習](../learning/課程目錄.md#linux-與系統程式系統化學習)｜第 ① 站第 2 課
> 日期：2026-09-24
> 情境：系統化學習路線第 ① 站第 2 課，對應 CS:APP 第 2 章 2.1～2.3 節（資訊的儲存、整數的表示、整數運算）。
> 第 1 課 [ELF 是什麼](./ELF是什麼-可執行與可連結格式的兩種視角.md#五檔頭裡的其他欄位) 裡 `readelf -h` 的
> `Data: 2's complement, little endian` 這一行，兩個詞都在這一課講
> 適用範圍：實測本機 Ubuntu 24.04、x86-64、gcc 13.3.0、glibc 2.39、gdb 15.1；所有輸出都是本機實際執行結果
> 對應檔案：每個實驗程式的完整原始碼都寫在它出現的那一節，可以直接複製編譯；作業在 [`small-project/s1-02-data-repr/`](../small-project/s1-02-data-repr/)（會進 git）
> 相關：[gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md)（第九節的 `-O0`／`-O2` 差異）、
> [gdb完整命令參考](./gdb完整命令參考.md)（本課用 `x` 看記憶體）

**一句話結論：記憶體裡只有一格一格的位元組（byte），本身沒有「型別」。同一串位元組是 `-1` 還是 `4294967295`、
要讀 1 個還是 4 個 byte，全由 C 程式碼裡的**型別**決定。指標就是一個位址（x86-64 上 8 個 byte 的數字），
指標的型別決定從那個位址讀幾個 byte、怎麼解讀。x86-64 是**小端序**：多位元組數字的**低位 byte 放在低位址**。
有號整數用**二補數**表示；有號與無號混在一起比較時，有號的會被轉成無號，`-1` 就變成最大的數。
無號溢位會回繞（有定義），**有號溢位是未定義行為**，`-O2` 會利用這一點改掉你的程式——所以 kernel 加了 `-fno-strict-overflow`。**

---

## 目錄

- [一、位元組與型別大小](#一位元組與型別大小)
- [二、指標就是位址](#二指標就是位址)
- [三、位元組順序（endianness）](#三位元組順序endianness)
- [四、無號整數：二進位與回繞](#四無號整數二進位與回繞)
- [五、有號整數：二補數](#五有號整數二補數)
- [六、有號與無號之間的轉換：位元不變，換個讀法](#六有號與無號之間的轉換位元不變換個讀法)
- [七、陷阱：有號和無號混在一起比較](#七陷阱有號和無號混在一起比較)
- [八、溢位：無號回繞，有號是未定義行為](#八溢位無號回繞有號是未定義行為)
- [九、本課用到的工具與選項](#九本課用到的工具與選項)
- [十、動手作業](#十動手作業)
- [十一、課後討論](#十一課後討論)

---

## 一、位元組與型別大小

**位元（bit）** 是一個 0 或 1。**位元組（byte）** 是 8 個位元，是記憶體**最小的定址單位**：每一個 byte 都有自己的位址，
沒辦法只取「某個位址的第 3 個 bit」，要先把整個 byte 讀出來。

一個 byte 可以寫成兩位**十六進位（hexadecimal）**：一位十六進位剛好是 4 個 bit（`0`～`f` 對應 `0000`～`1111`），
所以 `0x12345678` 一眼就看得出是 4 個 byte：`12`、`34`、`56`、`78`。這是整課一直用十六進位的原因。

各型別佔幾個 byte，用 `sizeof` 問編譯器（`%zu` 是印 `size_t` 的格式，見第九節）。`sizes.c` 完整原始碼：

```c
#include <stdio.h>
#include <limits.h>

int main(void)
{
    printf("char        %zu\n", sizeof(char));
    printf("short       %zu\n", sizeof(short));
    printf("int         %zu\n", sizeof(int));
    printf("long        %zu\n", sizeof(long));
    printf("long long   %zu\n", sizeof(long long));
    printf("float       %zu\n", sizeof(float));
    printf("double      %zu\n", sizeof(double));
    printf("int *       %zu\n", sizeof(int *));
    printf("char *      %zu\n", sizeof(char *));
    printf("size_t      %zu\n", sizeof(size_t));
    printf("CHAR_MIN %d  CHAR_MAX %d\n", CHAR_MIN, CHAR_MAX);
    printf("INT_MIN %d  INT_MAX %d\n", INT_MIN, INT_MAX);
    printf("UINT_MAX %u\n", UINT_MAX);
    printf("LONG_MAX %ld\n", LONG_MAX);
    return 0;
}
```

```
$ gcc -Wall -Wextra sizes.c -o sizes && ./sizes
char        1
short       2
int         4
long        8
long long   8
float       4
double      8
int *       8
char *      8
size_t      8
CHAR_MIN -128  CHAR_MAX 127
INT_MIN -2147483648  INT_MAX 2147483647
UINT_MAX 4294967295
LONG_MAX 9223372036854775807
```

要注意的三件事：

- **C 標準沒有規定 `int`、`long` 的確切大小**，只規定最小範圍。這裡的數字是 **x86-64 Linux** 的慣例，叫 **LP64**
  （Long and Pointer are 64-bit，`long` 和指標是 64 位元）。64 位元 Windows 的 `long` 是 4 byte，同一份程式碼換平台可能就不一樣。
  kernel 大量用 `u32`、`u64` 這種寫明大小的型別，就是為了不受這個影響（使用者空間對應的是 `<stdint.h>` 的 `uint32_t` 等）。
- **所有指標都是 8 byte**，不管指向 `char` 還是 `int`。指標存的是位址，而 x86-64 的位址是 64 位元（第二節）。
- **`char` 在這台機器上是有號的**（`CHAR_MIN` 是 -128）。C 標準讓平台自己決定 `char` 有號還是無號（ARM 上通常是無號），
  所以要處理「原始位元組」時，一律寫 `unsigned char`。

`CHAR_MIN`、`INT_MAX` 這些常數來自 `<limits.h>`（第九節）。

---

### `unsigned` 單獨寫就是 `unsigned int`：整數型別的所有寫法

> 2026-10-06 補教：使用者讀第 4 課的 `long widen_u(unsigned x)` 時問「`unsigned` 是什麼類型」。本課的表只寫了 `int`、`long` 這些名字，
> 沒說明 C 的整數型別可以有好幾種寫法，是教學上的漏洞。

**結論：單獨寫 `unsigned`，就是 `unsigned int`**（本機 4 byte，0～4294967295）。C 的整數型別名字是由幾個**型別指定字（type specifier）**
組合起來的：`signed`／`unsigned`（有號／無號）、`short`／`long`／`long long`（大小）、`int`、`char`。C 標準（C11 6.7.2）規定了三件事：

- **有了其他指定字時，`int` 可以省略**：`unsigned` ＝ `unsigned int`、`short` ＝ `short int`、`long` ＝ `long int`。
- **沒寫 `signed`／`unsigned` 時，預設有號**：`int` ＝ `signed int` ＝ `signed`、`long` ＝ `signed long`。**唯一的例外是 `char`**（見下面）。
- 指定字的**順序不重要**：`long unsigned int`、`int unsigned long`、`unsigned long` 都是同一個型別（但沒人這樣亂寫，慣例是 `unsigned long`）。

| 型別 | 所有合法寫法（同一列都是同一個型別） | 本機大小 |
|---|---|---|
| `char` | `char` | 1 |
| `signed char` | `signed char` | 1 |
| `unsigned char` | `unsigned char` | 1 |
| `short` | `short`、`signed short`、`short int`、`signed short int` | 2 |
| `unsigned short` | `unsigned short`、`unsigned short int` | 2 |
| `int` | `int`、`signed`、`signed int` | 4 |
| `unsigned int` | `unsigned`、`unsigned int` | 4 |
| `long` | `long`、`signed long`、`long int`、`signed long int` | 8 |
| `unsigned long` | `unsigned long`、`unsigned long int` | 8 |
| `long long` | `long long`、`signed long long`、`long long int`、`signed long long int` | 8 |
| `unsigned long long` | `unsigned long long`、`unsigned long long int` | 8 |

（這張表是 C 的**全部**標準整數型別。另外還有 `_Bool`（C23 改叫 `bool`），以及浮點的 `float`、`double`、`long double`；
`<stdint.h>` 的 `uint32_t` 這些不是新型別，而是上表某一個型別的**別名**（`typedef`），例如本機 `uint32_t` 就是 `unsigned int`。）

**`char` 是例外：`char`、`signed char`、`unsigned char` 是三個不同的型別。** 本機的 `char` 雖然是有號的（上面 `CHAR_MIN` 是 -128），
值的範圍跟 `signed char` 一樣，但在型別系統裡它們**不是同一個型別**。這是因為 C 標準讓平台決定 `char` 有號還是無號，
所以 `char` 不能被定義成兩者之一。

**怎麼驗證「同一個型別」：讓 gcc 告訴你。** 同一個函式宣告兩次時，型別完全相同就不報錯，不同就報 `conflicting types`，
而且錯誤訊息會用 gcc 自己的寫法印出型別。`same.c` 完整原始碼（只編譯不連結，所以不需要 `main`）：

```c
unsigned int f(unsigned int x);
unsigned     f(unsigned x);          /* 第 2 行：跟第 1 行同一個型別，不報錯 */
long int unsigned f2(void);
unsigned long     f2(void);          /* 第 4 行：順序不同、省略 int，仍是同一個型別 */
int          f(unsigned x);          /* 第 5 行：回傳型別改成 int，跟第 1 行衝突 */
signed char  h(void);
char         h(void);                /* 第 7 行：本機 char 有號，但 char 和 signed char 是不同型別 */
```

```
$ gcc -c same.c -o same.o
same.c:5:14: error: conflicting types for ‘f’; have ‘int(unsigned int)’
    5 | int          f(unsigned x);          /* 第 5 行：回傳型別改成 int，跟第 1 行衝突 */
      |              ^
same.c:2:14: note: previous declaration of ‘f’ with type ‘unsigned int(unsigned int)’
    2 | unsigned     f(unsigned x);          /* 第 2 行：跟第 1 行同一個型別，不報錯 */
      |              ^
same.c:7:14: error: conflicting types for ‘h’; have ‘char(void)’
    7 | char         h(void);                /* 第 7 行：本機 char 有號，但 char 和 signed char 是不同型別 */
      |              ^
same.c:6:14: note: previous declaration of ‘h’ with type ‘signed char(void)’
    6 | signed char  h(void);
      |              ^
```

逐行看：

- 第 2 行沒有報錯，代表 `unsigned f(unsigned x)` 跟第 1 行的 `unsigned int f(unsigned int x)` 是同一個型別。而且 gcc 在 note 裡**把第 2 行印成**
  `unsigned int(unsigned int)`：你寫的是 `unsigned`，gcc 心裡的名字是 `unsigned int`。（`unsigned int(unsigned int)` 是 gcc 印函式型別的寫法：
  括號外是回傳型別，括號內是參數型別。）
- 第 4 行沒有報錯：`long int unsigned` 和 `unsigned long` 是同一個型別。
- 第 5 行報錯：回傳型別從 `unsigned int` 改成 `int`，有號無號不同就是不同型別。
- 第 7 行報錯：`char` 和 `signed char` 在本機範圍一樣，型別卻不同。

gcc 自己偏好的寫法會把 `int` 放在最後、大小放在有號無號之前，例如 `unsigned long` 會被印成 `long unsigned int`、
`unsigned long long` 印成 `long long unsigned int`。之後在錯誤訊息或 `gdb` 的 `ptype` 裡看到這種寫法，它跟你寫的 `unsigned long` 是同一個型別。

所以第 4 課的 `long widen_u(unsigned x)` 就是「參數是 `unsigned int`（4 byte 無號）、回傳 `long`（8 byte 有號）」：
從 4 byte 變成 8 byte 要擴展，來源是無號的所以做零擴展（第六節），編出來就是一個 `movl %edi, %eax`。
把參數改寫成 `unsigned int x`，編出來的組合語言一模一樣。

*（追問補充結束，以下回到主線）*

---

## 二、指標就是位址

變數放在記憶體的某個位址上。`&x` 取得 `x` 的**位址**；存位址的變數叫**指標（pointer）**；`*p` 是「到 `p` 存的那個位址去讀寫」，
叫**解參考（dereference）**。實驗程式 `addr.c` 完整原始碼：

```c
#include <stdio.h>

int main(void)
{
    int x = 0x12345678;
    int *p = &x;                              /* p 存著 x 的位址 */
    unsigned char *b = (unsigned char *)&x;   /* 同一個位址，但宣告成指向 1 個 byte */

    printf("x        = 0x%x (%d)\n", x, x);
    printf("&x       = %p\n", (void *)&x);
    printf("p        = %p\n", (void *)p);
    printf("*p       = 0x%x\n", *p);              /* 從那個位址讀 4 個 byte，當成 int */
    printf("b        = %p\n", (void *)b);
    printf("*b       = 0x%02x\n", *b);            /* 從同一個位址只讀 1 個 byte */
    *p = 0x11223344;                              /* 第 15 行：透過指標寫回去 */
    printf("after *p = 0x11223344, x = 0x%x\n", x);
    return 0;
}
```

```
$ gcc -Wall -Wextra -g -O0 addr.c -o addr && ./addr
x        = 0x12345678 (305419896)
&x       = 0x7fffbc2b1274
p        = 0x7fffbc2b1274
*p       = 0x12345678
b        = 0x7fffbc2b1274
*b       = 0x78
after *p = 0x11223344, x = 0x11223344
```

逐項看：

- **`&x`、`p`、`b` 印出來是同一個數字**：指標的值就是一個位址，一個 8 byte 的整數，沒有別的東西。
- **`p` 和 `b` 指向同一個位址，讀出來卻不一樣**：`*p` 讀到 `0x12345678`，`*b` 只讀到 `0x78`。
  差別只在宣告的型別：`int *` 代表「從這裡讀 4 個 byte、當成 `int`」，`unsigned char *` 代表「從這裡讀 1 個 byte」。
  **記憶體本身不知道那裡放的是 `int`，是程式碼的型別決定怎麼讀。** 這是整課最重要的一句話。
- `(unsigned char *)&x` 的括號叫**強制轉型（cast）**：告訴編譯器「我知道型別不一樣，把這個位址當成 `unsigned char *` 用」。
  不寫 cast 的話，gcc 不加任何選項就會警告。對照實驗 `nocast.c` 完整原始碼：
  
  ```c
  #include <stdio.h>
  
  int main(void)
  {
      int x = 1;
      unsigned char *b = &x;      /* 故意不寫 (unsigned char *) */
  
      printf("%d\n", *b);
      return 0;
  }
  ```
  
  ```
  $ gcc nocast.c -o nocast
  nocast.c:6:24: warning: initialization of ‘unsigned char *’ from incompatible pointer type ‘int *’ [-Wincompatible-pointer-types]
  ```
- `%p` 規定要配 `void *`，所以印之前轉成 `(void *)`。`void *` 是「只有位址、不說指向什麼型別」的指標。
- 寫入 `*p` 就改到了 `x`：`p` 和 `x` 是同一塊記憶體的兩個名字。

**每次執行位址都不一樣**：

```
$ ./addr | grep '&x'
&x       = 0x7ffe50b23624          ← 上一次是 0x7fffbc2b1274
```

這是 ASLR（Address Space Layout Randomization，位址空間配置隨機化）：核心每次都把堆疊放在隨機的位置，
讓攻擊者猜不到位址。在 `gdb` 裡跑，位址每次都一樣（`0x7fffffffd254`），因為 `gdb` 預設**關掉** ASLR 方便除錯
（`gdb` 的 `set disable-randomization`，見 [gdb完整命令參考](./gdb完整命令參考.md)）。
行程的位址空間是第 ③ 站的主題，這裡只要知道：位址就是一個數字，而那個數字每次執行可能不同。

---

**`0x7fffffffd254` 在哪裡？**（2026-09-24 課後提問）它是在 `gdb` 裡 `print &x` 得到的 `x` 的位址，跑幾次都一樣。
用 `gdb` 的 `info proc mappings` 看這個行程的記憶體分成哪幾塊（[gdb完整命令參考](./gdb完整命令參考.md) 的 `info proc`）：

```
(gdb) info proc mappings
          Start Addr           End Addr       Size     Offset  Perms  objfile
      0x7ffffffdd000     0x7ffffffff000    0x22000        0x0  rw-p   [stack]
      （其他列省略：程式本身、libc、載入器各佔幾塊）
```

六個欄位：這一塊的起點、終點（不含）、大小、對應到檔案的哪個位置（`[stack]` 不是檔案，所以是 0）、
權限（`r` 可讀、`w` 可寫、第三格 `-` 代表不可執行、`p` 代表 private，自己的私有副本）、這一塊是什麼（檔案路徑，或 `[stack]` 這種特殊名稱）。

`0x7fffffffd254` 落在 `[stack]`（**堆疊**，函式的區域變數放在這裡）那一塊。關掉 ASLR 時，堆疊的頂端就在 `0x7ffffffff000`，
幾乎是使用者位址空間的最頂端（`0x00007fffffffffff`，下一小節〈位址看起來只有 6 byte？〉會解釋這個上限）。`x` 在頂端往下 `0x1dac`（7596）byte 的地方：
堆疊從高位址往低位址長，`x` 上面還放著環境變數、命令列參數等資料（第 ② 站）。
開著 ASLR 時，核心會把堆疊頂端往下移一段隨機的距離，所以直接執行時看到的是 `0x7fffbc2b1274`、`0x7ffe50b23624` 這種每次不同的數字。

*（追問補充結束，以下回到主線）*

---

### 位址看起來只有 6 byte？——`%p` 不補零，而且最上面 2 byte 一定是 0

> 2026-09-24 課後提問：「`0x7fffbc2b1274` 只有 12 位十六進位，也就是 6 byte，為什麼說指標是 8 byte？」

指標**確實佔 8 byte**，只是 `%p` 跟 `%x` 一樣不印開頭的 0。強制補滿 16 位就看得出來。`ptr8.c` 完整原始碼：

```c
#include <stdio.h>

int main(void)
{
    int x = 42;
    int *p = &x;

    printf("%%p      : %p\n", (void *)p);
    printf("%%#018lx : %#018lx\n", (unsigned long)p);
    printf("sizeof(p) = %zu\n", sizeof(p));       /* 第 10 行 */
    return 0;
}
```

```
$ gcc -Wall -Wextra -g -O0 ptr8.c -o ptr8 && ./ptr8
%p      : 0x7fff6a6d956c
%#018lx : 0x00007fff6a6d956c      ← 補滿 16 位十六進位＝8 byte，最上面是 00 00
sizeof(p) = 8
```

（`%#018lx`：旗標 `#` 加 `0x`、旗標 `0` 補零、寬度 18＝`0x` 兩個字元加 16 位數、`l` 表示參數是 `long`；完整文法見第九節。）

用 `gdb` 直接看**指標變數 `p` 自己**佔的那 8 個 byte（停在 `ptr8.c` 標註的第 10 行）：

```
$ gdb -q ./ptr8
(gdb) break 10
(gdb) run
(gdb) print p
$1 = (int *) 0x7fffffffd25c
(gdb) x/8xb &p
0x7fffffffd260:	0x5c	0xd2	0xff	0xff	0xff	0x7f	0x00	0x00
(gdb) x/1xg &p          ← g＝giant，8 byte 一個單位
0x7fffffffd260:	0x00007fffffffd25c
```

記憶體裡確實有 8 個 byte，而且是小端序（第三節）：最低位的 `5c` 在最前面，**最高位的兩個 `00` 在最後面**。

**為什麼最上面兩個 byte 總是 0？**（補教，這部分是第 ③ 站虛擬記憶體的內容，這裡先講結論）
x86-64 的指標雖然有 64 bit，CPU 實際只用低 48 bit 當虛擬位址（virtual address）：

```
$ grep -m1 'address sizes' /proc/cpuinfo
address sizes	: 39 bits physical, 48 bits virtual
```

另外 16 bit 規定要跟第 47 bit 一樣，全 0 或全 1，這種位址叫 **canonical address（標準形式位址）**。
Linux 把位址空間切成兩半：

| 範圍 | 給誰用 | 開頭長相 |
|---|---|---|
| `0x0000000000000000` ～ `0x00007fffffffffff` | 使用者程式 | 最上面 2 byte 是 `00 00`，`%p` 省略後看起來像 6 byte |
| `0xffff800000000000` ～ `0xffffffffffffffff` | 核心 | 最上面是 `ff ff`，印出來是完整的 16 位 |

所以**使用者程式裡印出的位址最多只有 12 位十六進位**；堆疊放在使用者區的最頂端附近，所以開頭是 `0x7ff...`。
將來在 kernel 的錯誤訊息裡看到 `ffff8881...` 這種完整 16 位的位址，那就是核心那一半。
（較新的 CPU 支援 5 層分頁，虛擬位址可以到 57 bit；本機 `/proc/cpuinfo` 的 flags 裡沒有 `la57`，所以是 48 bit。）

*（追問補充結束，以下回到主線）*

---

### 為什麼位址空間要切成「使用者」和「核心」兩半？

> 2026-10-20 複習時提問：「爲什麼記憶體空間分成核心的記憶體位址，還有使用者空間的記憶體位址？」（指上一小節「Linux 把位址空間切成兩半」那張表）

（補教，完整機制是第 ③ 站虛擬記憶體的內容，這裡先講結論。）

**先釐清「切成兩半」切的是什麼**：切的是**每個行程自己的虛擬位址空間**，不是實體記憶體（RAM）。
每個行程都以為自己擁有 `0` ～ `0xffffffffffffffff` 這一整段位址；虛擬位址實際對到 RAM 的哪裡，由核心替每個行程維護的
**分頁表（page table）**決定。上表的意思是：在**每一個**行程的位址空間裡，下半段放這個行程自己的東西，上半段放核心。

```
 行程 A 的位址空間            行程 B 的位址空間
┌──────────────────┐ 0xffff…  ┌──────────────────┐
│ 核心（大家都一樣）│ ←───────→ │ 核心（同一份）    │   上半：每個行程都對到同一份核心
├──────────────────┤          ├──────────────────┤
│   （不合法的洞）   │          │   （不合法的洞）   │
├──────────────────┤ 0x00007f…├──────────────────┤
│ A 的堆疊、程式碼…  │          │ B 的堆疊、程式碼…  │   下半：每個行程各自不同
└──────────────────┘ 0x0      └──────────────────┘
```

**為什麼核心要放進每個行程的位址空間？** 因為程式隨時都會進核心：每次系統呼叫（`write`、`read`，第 1 課用 `strace` 看到的那些）、
每次時鐘中斷、每次缺頁，CPU 都要立刻跳去執行核心的程式碼、讀寫核心的資料。CPU 存取記憶體時用的是**目前這張分頁表**——
如果核心不在這張表裡，每次進核心都得先換成另一張分頁表、出來再換回去，換分頁表會讓 CPU 的位址翻譯快取（TLB，Translation Lookaside Buffer，轉譯後備緩衝區）失效，很慢。
所以 Linux 把核心**固定對應在每個行程的同一段位址**：不管現在跑的是哪個行程，進核心時不用換表，核心的位址也都一樣。

**為什麼要切成兩段固定範圍，而不是混在一起？**

1. **保護**：分頁表的每一項都有一個位元標明「使用者模式能不能碰」。核心那一段全部標成「只有核心模式能存取」，
   所以它雖然**在**你的位址空間裡，你的程式卻碰不到；碰了 CPU 就產生錯誤，核心送 `SIGSEGV`。劃一條清楚的界線，這件事才好管理：
   系統呼叫傳進來的指標，核心只要檢查「它是不是落在使用者那一半」，就能擋掉想騙核心去讀寫核心資料的程式。
2. **兩邊各自長大不打架**：使用者程式從低位址往上用（程式碼在低處、堆疊在使用者區頂端），核心固定在最高處，
   兩邊中間隔著 canonical address 規定的那一大段不合法的洞，誰都不會長到對方的範圍裡。
3. **判斷很便宜**：以 canonical address 來說，看第 47 bit（等於看最上面是 `0000` 還是 `ffff`）就知道是哪一半，上一小節「從位址長相判斷是不是 kernel」就是這樣來的。

（32 位元的 Linux 也是同樣的設計，只是總共只有 4 GB，傳統切法是下面 3 GB 給使用者、`0xC0000000` 以上 1 GB 給核心。）

**實際跑一次：使用者程式去讀核心那一半的位址。** `kaddr.c`（檔案：[`kaddr.c`](../small-project/s1-02-data-repr/kaddr.c)）完整原始碼：

```c
#include <stdio.h>

int main(void)
{
    int local = 42;
    printf("local 的位址：%p\n", (void *)&local);
    printf("main  的位址：%p\n", (void *)main);

    int *k = (int *)0xffff888100000000;   /* 核心那一半的位址 */
    printf("準備讀 %p ...\n", (void *)k);
    fflush(stdout);
    printf("讀到 %d\n", *k);               /* 第 13 行 */
    return 0;
}
```

```
$ gcc -Wall -Wextra -g -O0 kaddr.c -o kaddr && ./kaddr; echo "exit=$?"
local 的位址：0x7fffa2e4259c          ← 使用者那一半（堆疊）
main  的位址：0x60474b476189          ← 使用者那一半（程式碼）
準備讀 0xffff888100000000 ...
exit=139                              ← 128 + 11，被 SIGSEGV（11 號訊號）殺掉，第 13 行沒印出來

$ strace -e trace=none ./kaddr
準備讀 0xffff888100000000 ...
--- SIGSEGV {si_signo=SIGSEGV, si_code=SEGV_MAPERR, si_addr=0xffff888100000000} ---
+++ killed by SIGSEGV (core dumped) +++
```

（`strace -e trace=none`：不印任何系統呼叫，只印訊號；`si_addr` 是出錯的那個位址。）
`0xffff888100000000` 是核心直接對應實體記憶體的那一段（kernel 錯誤訊息常見的 `ffff888...` 就在這裡），核心裡那個位址很可能真的有東西，
但使用者模式碰不到，程式直接被殺掉。`si_code` 寫的是 `SEGV_MAPERR`（「沒有對應」），而不是 `SEGV_ACCERR`（「有對應但沒權限」）：
使用者模式碰核心位址時，核心一律回報成 `SEGV_MAPERR`，不透露那裡到底有沒有東西。

用 `/proc/self/maps` 看一個行程的位址空間，列出來的也幾乎全是使用者那一半，最後一行例外：

```
$ tail -3 /proc/self/maps
7f420243a000-7f420243c000 rw-p 00038000 103:06 16420454   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
7fff8fb00000-7fff8fb22000 rw-p 00000000 00:00 0           [stack]
ffffffffff600000-ffffffffff601000 --xp 00000000 00:00 0   [vsyscall]
```

`[vsyscall]` 是早期為了加速 `gettimeofday` 這類呼叫、放在核心那一半的一小頁舊機制，為了相容保留著；除了它，核心那一半不會出現在 `maps` 裡。

**補一個例外：KPTI。** 2018 年公開的 Meltdown 漏洞讓使用者程式能利用 CPU 的推測執行，**偷看**核心那一半的內容（權限檢查擋得住結果，擋不住推測執行留下的痕跡）。
受影響的 CPU 上，Linux 改用 KPTI（Kernel Page-Table Isolation，核心分頁表隔離）：使用者模式時換成一張**幾乎不含核心**的分頁表，進核心才換回完整的表——
等於為了安全，付出上面說的「每次進出核心都換表」的代價。位址的切法不變，核心仍然在上半段。本機 `cat /sys/devices/system/cpu/vulnerabilities/meltdown` 印出 `Not affected`（i9-14900HX 不受影響），所以沒有開 KPTI。

*（追問補充結束，以下回到主線）*

---

`p + 1` 是什麼、陣列和指標的關係，是第 3 課的內容。

---

## 三、位元組順序（endianness）

上一節 `*b` 讀到 `0x78`：`x` 的**第一個 byte**（位址最低的那個）是 `0x78`，也就是數字的**最低位**。
用 `gdb` 的 `x`（examine，檢查記憶體）看全部 4 個 byte：

```
$ gdb -q ./addr
(gdb) break 15
(gdb) run
(gdb) print &x
$1 = (int *) 0x7fffffffd254
(gdb) x/4xb &x          ← 從 &x 開始，印 4 個單位，十六進位（x），每單位 1 byte（b）
0x7fffffffd254:	0x78	0x56	0x34	0x12
(gdb) x/1xw &x          ← 印 1 個單位，每單位 4 byte（w＝word）
0x7fffffffd254:	0x12345678
```

---

**怎麼讀 `x` 的輸出**（2026-09-24 課後提問：「都是印 `&x`，為什麼位址不同？」）：冒號**左邊**是位址，而且只印這一行**第一個**單位的位址；
冒號**右邊**是記憶體裡存的**內容**，`0x78` 這些是值，不是位址。每個 byte 其實都有自己的位址，`x` 只是沒逐個印出來：

| 位址 | 內容 |
|---|---|
| `0x7fffffffd254` | `0x78` |
| `0x7fffffffd255` | `0x56` |
| `0x7fffffffd256` | `0x34` |
| `0x7fffffffd257` | `0x12` |

所以 `print &x` 和 `x/4xb &x` 冒號左邊是同一個位址。另外要分清楚 `print p` 和 `x/8xb &p`（上一節）：
`p` 是指標**存的值**（`x` 的位址），`&p` 是指標變數**自己**的位址，那兩個數字本來就不同。

*（追問補充結束，以下回到主線）*

---

`break 15` 的第 15 行是 `addr.c` 裡標註的 `*p = 0x11223344;`：`gdb` 停在這一行**執行之前**，所以 `x` 還是 `0x12345678`。

`x/4xb` 的 `4`、`x`、`b` 分別是「數量、格式、單位大小」，完整的格式字母與大小字母見
[gdb完整命令參考〈輸出格式 `/FMT`〉](./gdb完整命令參考.md#輸出格式-fmtprint-與-x-共用)。

記憶體裡從低位址到高位址是 `78 56 34 12`，**倒過來**。多位元組的數字在記憶體裡有兩種放法：

| | 低位址 → 高位址 | 誰用 |
|---|---|---|
| **小端序（little endian）** | `78 56 34 12`（最低位的 byte 放最前面） | x86、x86-64；ARM 與 RISC-V 通常也設定成小端 |
| **大端序（big endian）** | `12 34 56 78`（跟我們寫數字的順序一樣） | 網路協定（所以又叫**網路位元組順序**，network byte order）、一些舊的 CPU（SPARC、PowerPC 的某些設定） |

這就是 `readelf -h` 那一行的後半：

```
$ readelf -h addr | grep -E 'Class|Data'
  Class:                             ELF64
  Data:                              2's complement, little endian
```

ELF 檔頭寫明這個檔案裡的多位元組數字用哪種順序，讀檔的工具才知道要怎麼拼回來。

**什麼時候要在意位元組順序？** 只要程式**用正確的型別讀寫自己的變數**，就永遠不會看到順序問題：
上面 `*p` 在大端和小端的機器上都讀到 `0x12345678`，CPU 自己知道怎麼拼。
**會出事的是「把原始位元組當成數字讀」的時候**：

- 用 `unsigned char *` 一個一個 byte 看（像 `*b` 在小端機器是 `0x78`，在大端機器會是 `0x12`）。
- 讀別的機器寫的檔案或網路封包。封包裡的數字是大端序，直接用 `int *` 讀進小端的 x86-64，四個 byte 的順序就反了。
  所以網路程式要用 `ntohl`／`htons` 之類的函式轉換（第 ⑥ 站）。

---

## 四、無號整數：二進位與回繞

`unsigned int` 是 32 個 bit，每個 bit 代表一個 2 的次方，從最低位起是 1、2、4、8……、2³¹。數值就是「是 1 的那些位加起來」：

```
0x0000000b = ... 0000 1011 = 8 + 2 + 1 = 11
0xffffffff = 32 個 1       = 2³² − 1 = 4294967295 = UINT_MAX
```

範圍是 0 到 2³²−1。**超出範圍時，C 標準規定結果是對 2³² 取餘數**，也就是**回繞（wrap around）**，像里程表跑過 99999 回到 00000。
`overflow.c` 完整原始碼：

```c
#include <stdio.h>
#include <limits.h>

int bigger(int x)
{
    return x + 1 > x;
}

int main(void)
{
    unsigned int u = UINT_MAX;
    u = u + 1;
    printf("UINT_MAX + 1 = %u\n", u);

    unsigned int z = 0;
    z = z - 1;
    printf("0u - 1 = %u\n", z);

    int s = INT_MAX;
    s = s + 1;
    printf("INT_MAX + 1 = %d\n", s);

    printf("bigger(INT_MAX) = %d\n", bigger(INT_MAX));
    return 0;
}
```

```
$ gcc overflow.c -o overflow && ./overflow
UINT_MAX + 1 = 0                  ← 無號回繞
0u - 1 = 4294967295               ← 無號回繞
INT_MAX + 1 = -2147483648         ← 有號溢位：第八節
bigger(INT_MAX) = 1               ← 有號溢位：第八節
```

前兩行是無號回繞，這是**有定義的行為**，任何最佳化等級、任何平台結果都一樣。後兩行是有號的溢位，情況完全不同，留到第八節。

---

### 常數也有型別：`0u` 的 `u` 是什麼

> 2026-09-24 課後提問：「`0u` 是什麼？」

`0u` 是**型別為 `unsigned int` 的 0**。程式裡直接寫的數字叫**整數常數（integer constant）**，它跟變數一樣有型別，
尾巴的字母叫**後綴（suffix）**，用來指定型別。用 C11 的 `_Generic`（依運算式的型別選一個結果）讓 gcc 回報型別。
`TYPE` 和 `SHOW` 是前處理器的巨集（[第 1 課第二節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#二前處理include-只是複製貼上)：前處理就是文字替換），`#e` 把參數原封不動變成字串，所以能印出原始寫法。`suffix.c` 完整原始碼：

```c
#include <stdio.h>

#define TYPE(e) _Generic((e), \
    int: "int", unsigned int: "unsigned int", \
    long: "long", unsigned long: "unsigned long", \
    long long: "long long", unsigned long long: "unsigned long long", \
    default: "other")

#define SHOW(e) printf("%-14s -> %s\n", #e, TYPE(e))

int main(void)
{
    SHOW(0);
    SHOW(0u);
    SHOW(0U);
    SHOW(0l);
    SHOW(0ul);
    SHOW(0lu);
    SHOW(0ll);
    SHOW(0ull);
    SHOW(2147483647);
    SHOW(2147483648);
    SHOW(3000000000);
    SHOW(3000000000u);
    SHOW(0x7fffffff);
    SHOW(0x80000000);
    SHOW(0xffffffff);
    SHOW(0x100000000);
    SHOW(-1);
    return 0;
}
```

```
$ gcc -Wall -Wextra suffix.c -o suffix && ./suffix     （先看前 8 行）
0              -> int
0u             -> unsigned int
0U             -> unsigned int
0l             -> long
0ul            -> unsigned long
0lu            -> unsigned long
0ll            -> long long
0ull           -> unsigned long long
```

**整數常數的後綴，全部列出**（C 標準）：

| 後綴 | 意思 | 寫法規則 |
|---|---|---|
| （沒有） | 依下面的規則，自動選第一個裝得下的型別 | — |
| `u`、`U` | 無號（unsigned） | 大小寫都可以 |
| `l`、`L` | 至少是 `long` | 小寫 `l` 很像數字 `1`，建議寫大寫 `L` |
| `ll`、`LL` | 至少是 `long long` | 兩個字母要同樣大小寫：`lL` 會報錯 `invalid suffix "lL" on integer constant` |
| `u` 與 `l`／`ll` 組合 | 無號的 `long`／`long long` | 順序隨意：`ul`、`lu`、`ULL`、`LLU`、`0Lu` 都合法 |
| `wb`、`WB`、`uwb`、`UWB` | C23 新增，`_BitInt(N)` 型別（指定位元數的整數） | **本機 gcc 13.3 不支援**，連 `-std=c2x` 都報 `invalid suffix "wb"`（gcc 14 起才有） |

浮點數常數的後綴（`1.0f`、`1.0L`）是另一套，本課不用。

**沒有後綴時，十進位和十六進位的規則不一樣**，這是容易踩的地方：

| 寫法 | 依序嘗試的型別（選第一個裝得下的） |
|---|---|
| 十進位（`2147483648`） | `int` → `long` → `long long`（**永遠不會變成無號**） |
| 十六進位、八進位（`0x80000000`） | `int` → `unsigned int` → `long` → `unsigned long` → `long long` → `unsigned long long` |

同一支 `suffix.c` 的後 9 行輸出：

```
2147483647     -> int             ← INT_MAX，int 裝得下
2147483648     -> long            ← 超過 INT_MAX，十進位跳過無號，直接變 long
3000000000     -> long
3000000000u    -> unsigned int    ← 有 u，而且 unsigned int 裝得下
0x7fffffff     -> int
0x80000000     -> unsigned int    ← 同樣是 2³¹，十六進位會先試 unsigned int
0xffffffff     -> unsigned int
0x100000000    -> long            ← 超過 32 bit
-1             -> int
```

最後一列要注意：C **沒有負數常數**。`-1` 是「對常數 `1` 做負號運算」，`1` 是 `int`，結果也是 `int`。

這會影響第七節的陷阱。同樣是 2³¹，只因為寫法不同，比較結果就相反。`cmp.c` 完整原始碼：

```c
#include <stdio.h>

int main(void)
{
    printf("-1 < 0x80000000 -> %d\n", -1 < 0x80000000);
    printf("-1 < 2147483648 -> %d\n", -1 < 2147483648);
    return 0;
}
```

```
$ gcc -Wall -Wextra cmp.c -o cmp && ./cmp
cmp.c:5:42: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
-1 < 0x80000000 -> 0      ← 0x80000000 是 unsigned int，-1 被轉成 4294967295
-1 < 2147483648 -> 1      ← 2147483648 是 long，-1 轉成 long 還是 -1
```

（`-Wall -Wextra` 對前者有 `-Wsign-compare` 警告；但第七節的 `-1 < 0u` 卻沒有警告，常數之間的比較不一定抓得到。）
第七節的 `-1 < 0u`、第六節的 `int big = 3000000000u` 用 `u`，就是為了**刻意製造一個無號常數**來示範轉換。

*（追問補充結束，以下回到主線）*

---

### 怎麼確定一個常數的型別？比較時以誰為準？

> 2026-09-27 自我測驗第 7 題追問：「這種常數比較到底要怎麼比？我要怎麼確定常數的型別，是要以誰的為準？」

分成兩步：**先決定每一邊各自是什麼型別，再決定兩邊要轉成哪個共同型別**。

**第一步：常數自己的型別**，照上一小節的規則，順序是：

1. 有後綴就照後綴（`u`、`l`、`ul`……）。後綴只規定「至少」，裝不下時一樣往更大的型別走。
2. 沒有後綴時，照上面那張「依序嘗試」的表，選**第一個裝得下**的型別：十進位只試有號型別（`int` → `long` → `long long`），
   十六進位／八進位會穿插試無號型別（`int` → `unsigned int` → `long` → …）。
3. 負號不屬於常數：`-1` 是 `int` 的 `1` 取負號，型別還是 `int`。
4. 不確定時**讓 gcc 告訴你**：用上一小節的 `_Generic`（`suffix.c`）印出型別，或看 `-Wsign-compare` 警告裡寫的型別名稱。

所以第 7 題的兩個常數：`2147483648` 十進位，`int` 裝不下（`INT_MAX` 是 2147483647），下一個試 `long`，本機 `long` 是 8 byte、最大 9223372036854775807，裝得下 → `long`。
`0x80000000` 十六進位，`int` 裝不下，下一個試 `unsigned int`（最大 4294967295），裝得下 → `unsigned int`。兩者差在**嘗試清單不同**，不是差在誰比較大。

**第二步：兩邊型別不同時，轉成哪一個**——這是 C 標準的一般算術轉換（usual arithmetic conversions），整數的部分依序套用，套到第一條成立的就停：

| 順序 | 條件 | 結果 | 例子（本機） |
|---|---|---|---|
| 0 | 先做**整數提升**（integer promotion）：比 `int` 小的 `char`、`short` 先變成 `int` | — | `char` 與 `char` 相比，其實是兩個 `int` 在比 |
| 1 | 兩邊型別相同 | 不用轉 | `int` 與 `int` |
| 2 | 兩邊都有號，或都無號 | 轉成**等級（rank）較高**的那個（`long long` > `long` > `int`） | `int` 與 `long` → `long` |
| 3 | 無號那邊的等級 **≥** 有號那邊 | 轉成那個**無號**型別 | `int` 與 `unsigned int` → `unsigned int`；`int` 與 `unsigned long` → `unsigned long`；`long` 與 `unsigned long` → `unsigned long` |
| 4 | 有號那邊**裝得下無號那邊的所有值** | 轉成那個**有號**型別 | `long`（8 byte）與 `unsigned int`（4 byte）→ `long` |
| 5 | 以上都不成立 | 轉成**有號那邊對應的無號型別** | 本機沒有這種組合（`long` 與 `long long` 都是 8 byte 時才會遇到，例如 `long long` 與 `unsigned long`） |

口訣：**同號取大；無號不比較小就聽無號的；有號裝得下無號就聽有號的**。一旦結果是無號，負數就會被換成很大的正數（第六節），比較就可能反過來。

實測。`TYPE((a) + (b))` 用加法問 gcc「兩邊轉完的共同型別」（比較和加法用的是同一套轉換）。`common.c` 完整原始碼：

```c
#include <stdio.h>

#define TYPE(e) _Generic((e), \
    int: "int", unsigned int: "unsigned int", \
    long: "long", unsigned long: "unsigned long", \
    default: "other")

#define SHOW(a, b) printf("%-24s -> %-14s  %s < %s = %d\n", \
    #a " 與 " #b, TYPE((a) + (b)), #a, #b, (a) < (b))

int main(void)
{
    SHOW(-1, 1);             /* int           與 int           */
    SHOW(-1, 1u);            /* int           與 unsigned int  */
    SHOW(-1, 1L);            /* int           與 long          */
    SHOW(-1L, 1u);           /* long          與 unsigned int  */
    SHOW(-1, 1UL);           /* int           與 unsigned long */
    SHOW(-1L, 1UL);          /* long          與 unsigned long */
    SHOW(-1, 0x80000000);    /* int           與 unsigned int（十六進位常數）*/
    SHOW(-1, 2147483648);    /* int           與 long（十進位常數）*/
    return 0;
}
```

```
$ gcc -Wall -Wextra common.c -o common && ./common
common.c:9:48: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
common.c:9:48: warning: comparison of integer expressions of different signedness: ‘int’ and ‘long unsigned int’ [-Wsign-compare]
common.c:9:48: warning: comparison of integer expressions of different signedness: ‘long int’ and ‘long unsigned int’ [-Wsign-compare]
common.c:9:48: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
-1 與 1                 -> int             -1 < 1 = 1       ← 規則 1
-1 與 1u                -> unsigned int    -1 < 1u = 0      ← 規則 3：-1 變 4294967295
-1 與 1L                -> long            -1 < 1L = 1      ← 規則 2
-1L 與 1u               -> long            -1L < 1u = 1     ← 規則 4：long 裝得下所有 unsigned int，不會出事，也沒有警告
-1 與 1UL               -> unsigned long   -1 < 1UL = 0     ← 規則 3：-1 變 18446744073709551615
-1L 與 1UL              -> unsigned long   -1L < 1UL = 0    ← 規則 3
-1 與 0x80000000        -> unsigned int    -1 < 0x80000000 = 0   ← 第一步決定是 unsigned int，第二步套規則 3
-1 與 2147483648        -> long            -1 < 2147483648 = 1   ← 第一步決定是 long，第二步套規則 2
```

（警告的行號都是 9，因為每個 `SHOW` 展開後的比較都寫在巨集定義那一行。）
4 則警告剛好對應結果變成無號的 4 列；`-1L` 與 `1u` 轉成 `long`，值不會變，gcc 也就不警告。
實務上的結論：**不要讓有號和無號混在一起比較**；非比不可時，自己先轉型（例如 `(long)i < (long)len`），並固定用 `-Wall -Wextra` 讓 `-Wsign-compare` 提醒你。

*（追問補充結束，以下回到主線）*

---

## 五、有號整數：二補數

`int` 也是 32 個 bit，但要能表示負數。現代所有 CPU 都用**二補數（two's complement）**，規則只改一個地方：
**最高位（第 31 位）的權重是 −2³¹，不是 +2³¹**，其他位不變。

```
0x7fffffff = 0111 1111 ... 1111 = 2³¹ − 1          = 2147483647  = INT_MAX
0x80000000 = 1000 0000 ... 0000 = −2³¹              = −2147483648 = INT_MIN
0xffffffff = 1111 1111 ... 1111 = −2³¹ + (2³¹ − 1) = −1
```

---

**上面說的「權重」是什麼：8 位元的最高位為什麼是 −2⁷，不是 −2⁸**

> 2026-10-25 提問：「可以先請教一下什麼是 2 補數權重嗎？」（批改 10-24 複習組語 9-2 時，作答把 8 位元的最高位權重寫成 −2⁸）

**權重（weight）＝ 一個位元「是 1 的時候」代表多少。** 一串位元的值，就是「每個是 1 的位元的權重」加起來；是 0 的位元貢獻 0。
跟十進位一樣：`523` 的 5 在百位，權重是 100，所以代表 500。二進位每往左一位，權重乘 2。

**位元編號從 0 數起，第 i 位的權重是 2ⁱ。** 最右邊（最低位）是第 0 位，權重 2⁰ ＝ 1；8 位元的最左邊是**第 7 位**，權重 2⁷ ＝ 128。
所以 n 位元的最高位是第 n − 1 位：8 位元是 2⁷、32 位元是 2³¹。2⁸ ＝ 256 是「第 8 位」的權重，那一位在 8 位元裡根本不存在
（它就是減法往外借位時借的那一位，見[第 4 課第五節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面③：3 − 5 在 CPU 裡實際上是怎麼算的？〉）。

| 位元編號 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
|---|---|---|---|---|---|---|---|---|
| 無號的權重 | +128（+2⁷） | 64 | 32 | 16 | 8 | 4 | 2 | 1 |
| 二補數的權重 | **−128（−2⁷）** | 64 | 32 | 16 | 8 | 4 | 2 | 1 |

**無號與二補數只差最高位那一格的正負號**，其他位一模一樣。逐列讀這張表：

- 無號那一列：8 個全是 1 時是 128 ＋ 64 ＋ … ＋ 1 ＝ 255 ＝ 2⁸ − 1，這就是 `unsigned char` 的最大值。
- 二補數那一列：最高位換成 −128，其他 7 位最多加到 64 ＋ … ＋ 1 ＝ 127 ＝ 2⁷ − 1。所以全 1 是 −128 ＋ 127 ＝ −1；
  最高位是 1 的時候，後面 7 位最多只能補回 127，結果一定是負的——這就是「最高位是 1 就是負數」的由來。

實驗：把幾個 byte 拆成位元，用兩列權重各加一次，再跟 C 的 `(signed char)` 轉換比對。
`weights.c`（檔案：[`weights.c`](../small-project/s1-02-data-repr/weights.c)）完整原始碼：

```c
#include <stdio.h>

/* 把 1 byte 的每一位拆開，用兩種權重各算一次 */
static void show(unsigned char b)
{
    int as_unsigned = 0, as_signed = 0;

    printf("0x%02x = ", b);
    for (int i = 7; i >= 0; i--)
        putchar((b >> i) & 1 ? '1' : '0');

    for (int i = 0; i <= 7; i++) {
        if ((b >> i) & 1) {
            as_unsigned += 1 << i;                       /* 第 i 位的權重：+2^i */
            as_signed   += (i == 7) ? -(1 << 7) : (1 << i); /* 二補數：只有第 7 位是 -2^7 */
        }
    }
    printf("  無號權重和 = %4d  二補數權重和 = %4d  (signed char) = %4d\n",
           as_unsigned, as_signed, (signed char)b);
}

int main(void)
{
    unsigned char samples[] = { 0x01, 0x05, 0x7f, 0x80, 0x81, 0xfb, 0xff };

    printf("bit:   76543210   ← 位元編號，從 0 數起\n");
    for (unsigned i = 0; i < sizeof samples; i++)
        show(samples[i]);
    return 0;
}
```

```
$ gcc -Wall -Wextra weights.c -o weights && ./weights
bit:   76543210   ← 位元編號，從 0 數起
0x01 = 00000001  無號權重和 =    1  二補數權重和 =    1  (signed char) =    1
0x05 = 00000101  無號權重和 =    5  二補數權重和 =    5  (signed char) =    5
0x7f = 01111111  無號權重和 =  127  二補數權重和 =  127  (signed char) =  127
0x80 = 10000000  無號權重和 =  128  二補數權重和 = -128  (signed char) = -128
0x81 = 10000001  無號權重和 =  129  二補數權重和 = -127  (signed char) = -127
0xfb = 11111011  無號權重和 =  251  二補數權重和 =   -5  (signed char) =   -5
0xff = 11111111  無號權重和 =  255  二補數權重和 =   -1  (signed char) =   -1
```

程式第 15 行只對第 7 位用 −2⁷，算出來的「二補數權重和」每一列都跟 C 自己的 `(signed char)` 一樣，表示 CPU 讀有號 byte 就是這樣算的。逐列看：

- `0x01`、`0x05`、`0x7f`：最高位是 0，那一格的正負號用不到，兩種讀法一樣。`0x7f` ＝ 127 是 `signed char` 的最大值。
- `0x80`：只有第 7 位是 1，無號讀 ＋128、二補數讀 −128（`signed char` 的最小值）。兩者剛好差 256 ＝ 2 × 128：同一個位元，從 ＋128 變成 −128。
- `0x81`：−128 ＋ 1 ＝ −127。
- `0xfb`：−128 ＋ 64 ＋ 32 ＋ 16 ＋ 8 ＋ 2 ＋ 1 ＝ −5（第 4 課 4 − 9 留下的那個 byte）。
- `0xff`：−128 ＋ 127 ＝ −1。

**每一列「無號 − 二補數」都是 0 或 256**：最高位是 1 時，同一個位元從 ＋2⁷ 換成 −2⁷，差 2 × 2⁷ ＝ 2⁸。
所以[第六節](#六有號與無號之間的轉換位元不變換個讀法)「位元不變、換個讀法」時，負數換成無號讀會多出 2⁸（32 位元多出 2³²）：`int` 的 −1 用 `%u` 讀成 4294967295 ＝ −1 ＋ 2³²，就是這個道理。

常見寫錯：寫成「−2⁸ ＋ (2⁸ − 1) ＝ −1」。算出來剛好也是 −1，但那是 **9 位元**的權重（第 8 位是 −2⁸，下面 8 位最多 2⁸ − 1）。
8 位元是 −2⁷ ＋ (2⁷ − 1)、32 位元是 −2³¹ ＋ (2³¹ − 1)。記法：**n 位元，最高位是第 n − 1 位**。

*（追問補充結束，以下回到主線）*

---

實測。這支 `signs.c` 第五、六、七節共用，每一段輸出對應哪幾行，在各節裡說明。`signs.c` 完整原始碼：

```c
#include <stdio.h>
#include <limits.h>

int main(void)
{
    int m = -1;
    unsigned int u = m;
    unsigned char *b = (unsigned char *)&m;

    printf("m = %d, as %%u = %u, as %%x = 0x%x, first byte = 0x%02x\n", m, (unsigned)m, (unsigned)m, *b);
    printf("u = %u\n", u);
    printf("INT_MIN = %d = 0x%x\n", INT_MIN, (unsigned)INT_MIN);
    printf("INT_MAX = %d = 0x%x\n", INT_MAX, (unsigned)INT_MAX);

    int big = 3000000000u;
    printf("int big = 3000000000u -> %d\n", big);

    unsigned char c = 300;
    printf("unsigned char c = 300 -> %d\n", c);

    signed char sc = -1;
    unsigned char uc = 0xff;
    int a1 = sc, a2 = uc;
    printf("(int)(signed char)-1 = %d = 0x%x ; (int)(unsigned char)0xff = %d = 0x%x\n", a1, (unsigned)a1, a2, (unsigned)a2);

    if (-1 < 0u)
        printf("-1 < 0u is true\n");
    else
        printf("-1 < 0u is FALSE\n");

    int n = -1;
    unsigned int len = 5;
    if (n < len)
        printf("n < len\n");
    else
        printf("n < len is FALSE (n=%d, len=%u)\n", n, len);
    return 0;
}
```

```
$ gcc signs.c -o signs && ./signs
m = -1, as %u = 4294967295, as %x = 0xffffffff, first byte = 0xff      ← 第 10 行（本節）
u = 4294967295                                                         ← 第 11 行（第六節）
INT_MIN = -2147483648 = 0x80000000                                     ← 第 12 行（本節）
INT_MAX = 2147483647 = 0x7fffffff                                      ← 第 13 行（本節）
int big = 3000000000u -> -1294967296                                   ← 第 16 行（第六節）
unsigned char c = 300 -> 44                                            ← 第 19 行（第六節）
(int)(signed char)-1 = -1 = 0xffffffff ; (int)(unsigned char)0xff = 255 = 0xff   ← 第 24 行（第六節）
-1 < 0u is FALSE                                                       ← 第 26～29 行（第七節）
n < len is FALSE (n=-1, len=5)                                         ← 第 33～36 行（第七節）
```

（行號是從 `#include <stdio.h>` 那一行算第 1 行。）

由規則推出的幾個性質：

- **最高位是 1 就是負數**，所以最高位又叫**符號位（sign bit）**。
- **範圍不對稱**：−2147483648 到 2147483647，負數多一個。`INT_MIN` 取負號沒有對應的正數。
- **取負數的算法：每個 bit 反過來，再加 1**。例：`1` 是 `0x00000001`，反過來是 `0xfffffffe`，加 1 得 `0xffffffff`，就是 −1。
- 加法電路**跟無號完全一樣**：`0xffffffff + 1` 的位元結果是 `0x00000000`，當無號看是「回繞到 0」，當有號看是「−1 + 1 = 0」。
  這是二補數勝出的原因——CPU 不需要兩套加法器。

這就是 `readelf -h` 那一行的前半 `2's complement`：這個檔案裡的有號數用二補數表示。

---

## 六、有號與無號之間的轉換：位元不變，換個讀法

同樣寬度的有號、無號互相轉換時，**位元一個都沒變，只是換了解讀方式**（第五節 `signs.c` 的第 7、15 行）：

```
unsigned int u = -1;           → u = 4294967295         （0xffffffff 當無號讀）
int big = 3000000000u;         → big = -1294967296      （3000000000 = 0xb2d05e00，最高位是 1，當有號讀是負數）
```

**變窄（截斷，truncation）**：只留下低位的 byte，高位直接丟掉。

```
unsigned char c = 300;         → c = 44
```

300 是 `0x12c`，`unsigned char` 只有 8 bit，留下 `0x2c` = 44（`signs.c` 第 18 行）。gcc 連 `-Wall` 都不用就會警告：

```
signs.c:18:23: warning: unsigned conversion from ‘int’ to ‘unsigned char’ changes value from ‘300’ to ‘44’ [-Woverflow]
```

**變寬（擴展，extension）**：要補上新的高位，補什麼取決於**原本的型別**，也就是**轉換之前、比較窄的那個型別**，
不是轉換之後的型別：原本是有號就補符號位，原本是無號就補 0。

---

> 2026-09-24 課後提問：「擴展裡說的原本的型別是指哪個型別？」——下面用實驗把轉換前後的每個 byte 印出來。

`extend.c` 完整原始碼：

```c
#include <stdio.h>

int main(void)
{
    signed char sc = -1;        /* 1 byte：0xff */
    unsigned char uc = 0xff;    /* 1 byte：0xff，跟 sc 一模一樣 */

    int a1 = sc;                /* 原本的型別：signed char   → 轉成 int */
    int a2 = uc;                /* 原本的型別：unsigned char → 轉成 int */
    unsigned int a3 = sc;       /* 原本的型別：signed char   → 轉成 unsigned int */

    printf("a1 = %d, a2 = %d, a3 = %u\n", a1, a2, a3);    /* 第 12 行 */
    return 0;
}
```

```
$ gcc -Wall -Wextra -g -O0 extend.c -o extend && ./extend
a1 = -1, a2 = 255, a3 = 4294967295

$ gdb -q ./extend
(gdb) break 12          ← 標註的第 12 行，這時三個轉換都做完了
(gdb) run
(gdb) x/1xb &sc
0x7fffffffd262:	0xff
(gdb) x/1xb &uc
0x7fffffffd263:	0xff
(gdb) x/4xb &a1
0x7fffffffd264:	0xff	0xff	0xff	0xff     ← 原本 0xff，新補的 3 個 byte 是 ff
(gdb) x/4xb &a2
0x7fffffffd268:	0xff	0x00	0x00	0x00     ← 原本 0xff，新補的 3 個 byte 是 00
(gdb) x/4xb &a3
0x7fffffffd26c:	0xff	0xff	0xff	0xff     ← 轉成無號型別，補的還是 ff
```

（小端序：第一個 byte 是原本那個 `0xff`，後面三個是新補的高位。）

| | 原本的型別 | 轉成 | 補的高位 | 名稱 | 結果 |
|---|---|---|---|---|---|
| `a1` | `signed char`（有號） | `int` | 符號位是 1 → 補 `ff ff ff` | **符號擴展（sign extension）** | −1 |
| `a2` | `unsigned char`（無號） | `int` | 一律補 `00 00 00` | **零擴展（zero extension）** | 255 |
| `a3` | `signed char`（有號） | `unsigned int` | 還是補 `ff ff ff` | 符號擴展 | 4294967295 |

- **`a1` 與 `a2`**：轉成的型別都是 `int`，只有原本的型別不同，補的東西就不同。所以決定補什麼的是原本的型別。
- **`a3`**：轉成的是**無號**型別，補的卻還是 1。這證明轉換後的型別不影響補什麼。
  實際上是分兩步：先照原本的 `signed char` 做符號擴展，把 −1 保持成 −1；再依第六節開頭的規則，位元不變、換成無號的讀法，變成 4294967295。
- **為什麼要這樣設計**：擴展的目的是**保持數值不變**。`signed char` 的 `0xff` 代表 −1，補 1 之後 `0xffffffff` 在 `int` 裡還是 −1；
  `unsigned char` 的 `0xff` 代表 255，補 0 之後 `0x000000ff` 還是 255。要保持原本的值，就只能看原本的型別。

*（追問補充結束，以下回到主線）*

---

兩者原本都是同一個 byte `0xff`，擴展成 `int` 後一個是 −1、一個是 255。這就是第一節說「處理原始位元組用 `unsigned char`」的理由：
用 `char` 讀到 `0xff`，擴展後變成 −1，拿去當陣列索引或跟 255 比較都會出錯。

---

## 七、陷阱：有號和無號混在一起比較

第五節 `signs.c` 的最後兩段（第 26～36 行）：

```c
    if (-1 < 0u)
        printf("-1 < 0u is true\n");
    else
        printf("-1 < 0u is FALSE\n");

    int n = -1;
    unsigned int len = 5;
    if (n < len)
        printf("n < len\n");
    else
        printf("n < len is FALSE (n=%d, len=%u)\n", n, len);
```

```
-1 < 0u is FALSE
n < len is FALSE (n=-1, len=5)
```

C 的規則（usual arithmetic conversions，一般算術轉換）：**`int` 和 `unsigned int` 放在同一個運算裡，`int` 會先被轉成 `unsigned int`**。（完整的規則表見第四節〈怎麼確定一個常數的型別？比較時以誰為準？〉。）
轉換照第六節是「位元不變、換讀法」，於是 `-1` 變成 4294967295，比 5 大。

這在真實程式裡很常見，因為 `sizeof` 與 `strlen` 回傳的都是無號的 `size_t`：`if (i < strlen(s))` 裡的 `i` 如果是負的 `int`，就會踩到。

**編譯器能幫你抓多少？**

| 編譯方式 | `if (n < len)`（變數） | `if (-1 < 0u)`（常數） | `for (unsigned i = 3; i >= 0; i--)` |
|---|---|---|---|
| 不加警告選項 | 沒警告 | 沒警告 | 沒警告 |
| `-Wall` | **沒警告** | 沒警告 | **沒警告** |
| `-Wall -Wextra` | `-Wsign-compare` 警告 | 沒警告 | `-Wtype-limits` 警告 |

實測的兩則警告：

```
signs.c:33:11: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
loop.c:6:32: warning: comparison of unsigned expression in ‘>= 0’ is always true [-Wtype-limits]
```

**`-Wall` 不是「全部警告」**，只是一組常用的。C 程式請固定加 `-Wall -Wextra`（名稱的由來與數量見第九節）。
最後一欄那個迴圈是無號回繞的經典 bug：`i` 從 0 再減 1 變成 4294967295，`i >= 0` 永遠成立，迴圈停不下來。
為了讓示範能結束，加了一個印 6 次就 `break` 的計數器。`loop.c` 完整原始碼：

```c
#include <stdio.h>

int main(void)
{
    int count = 0;
    for (unsigned int i = 3; i >= 0; i--) {
        printf("%u\n", i);
        if (++count == 6)
            break;
    }
    return 0;
}
```

```
$ gcc loop.c -o loop && ./loop
3
2
1
0
4294967295          ← 0 減 1 回繞成最大值，i >= 0 還是成立
4294967294
```

### `-Wtype-limits` 只抓「永遠成立／永遠不成立」的比較

> 2026-09-27 作業第 4 題補教：`unsigned int a = 3, b = 5;` 用 `if (a - b > 0)` 判斷 a 是否比 b 大，`-Wall -Wextra` 完全沒警告，
> 但改成 `a - b < 0` 就有警告。

同一個無號運算式跟 0 做五種比較，`tl.c` 完整原始碼：

```c
#include <stdio.h>

int main(void)
{
    unsigned int a = 3, b = 5;
    if (a - b > 0)  printf("> 0\n");     /* 第 6 行 */
    if (a - b < 0)  printf("< 0\n");     /* 第 7 行 */
    if (a - b >= 0) printf(">= 0\n");    /* 第 8 行 */
    if (a - b <= 0) printf("<= 0\n");    /* 第 9 行 */
    if (a - b != 0) printf("!= 0\n");    /* 第 10 行 */
    return 0;
}
```

```
$ gcc -Wall -Wextra tl.c -o tl && ./tl
tl.c:7:15: warning: comparison of unsigned expression in ‘< 0’ is always false [-Wtype-limits]
tl.c:8:15: warning: comparison of unsigned expression in ‘>= 0’ is always true [-Wtype-limits]
> 0
>= 0
!= 0
```

只有第 7、8 行被警告。`man gcc` 對 `-Wtype-limits` 的說明是：比較的結果**因為型別的範圍有限而永遠成立或永遠不成立**時才警告
（例子正是「無號變數用 `<` 或 `>=` 跟 0 比」），這個警告包含在 `-Wextra` 裡。

| 比較 | 對無號數來說等於 | 結果固定嗎 | 警告 |
|---|---|---|---|
| `u < 0` | 永遠假 | 固定 | 有 |
| `u >= 0` | 永遠真 | 固定 | 有 |
| `u > 0` | `u != 0` | **不固定**（`u` 是 0 時為假） | 沒有 |
| `u <= 0` | `u == 0` | **不固定** | 沒有 |

`a - b > 0` 在語法上是完全正常的程式碼（「`u` 不是 0 嗎？」），gcc 無從知道你**心裡想的**是「a 比 b 大」。
所以編譯器抓得到「寫出來就一定錯」的比較，抓不到「寫出來合法、但意思跟你想的不一樣」的比較。
無號數比大小的正確寫法是直接比 `a > b`，不要相減再跟 0 比。

> 課後討論：[第十一節第 1 章〈`sum < x` 這個溢位檢查：前提、化簡的根據、怎麼確定 gcc 化簡了〉](#1-sum--x-這個溢位檢查前提化簡的根據怎麼確定-gcc-化簡了)（第 (6) 小節：`>= 0` 也會被警告）

---

## 八、溢位：無號回繞，有號是未定義行為

### 有號溢位在 C 標準裡是「未定義行為」

無號溢位有規定（回繞）；**有號溢位，C 標準沒有規定結果**，叫**未定義行為（undefined behavior，UB）**。
意思不是「結果不確定」，而是**標準對這支程式完全沒有要求**：編譯器可以**假設它永遠不會發生**，並依這個假設改寫程式。

實驗（`bigger` 故意分成兩行寫，原因見本節後面）。`ub.c` 完整原始碼：

```c
#include <stdio.h>
#include <limits.h>

int bigger(int x)
{
    int y = x + 1;
    return y > x;
}

int main(void)
{
    printf("bigger(INT_MAX) = %d\n", bigger(INT_MAX));
    return 0;
}
```

```
$ gcc -O0 ub.c -o ub && ./ub
bigger(INT_MAX) = 0
$ gcc -O2 ub.c -o ub && ./ub
bigger(INT_MAX) = 1
```

同一份程式碼，兩種答案：

- **`-O0`**：照字面算。CPU 的加法照二補數回繞，`INT_MAX + 1` 得到 `INT_MIN`，`INT_MIN > INT_MAX` 是假，回傳 0。
- **`-O2`**：編譯器推理「有號溢位不會發生，所以 `x + 1 > x` 永遠成立」，直接把函式換成回傳 1，根本不做加法。

這**不是 gcc 的 bug**。按照標準，程式一發生有號溢位就沒有「正確答案」，兩種結果都合法。
（`-O2` 把函式內嵌並在編譯時算出結果，就是 [gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md) 講的內嵌加常數計算。）

**連 `-O0` 也不保證照字面算。** 第四節的 `overflow.c` 把同一件事寫成一行 `return x + 1 > x;`（它的第 6 行），`-O0` 也回傳 1（第四節輸出的最後一行）。
用 `-fdump-tree-original` 把 gcc 內部剛讀完原始碼時的樣子印出來（gcc 內部的中間表示，長得像 C，不是組合語言）：

```
$ gcc -O0 -fdump-tree-original -c overflow.c -o /dev/null
$ cat overflow.c.005t.original       （檔名中間的數字依 gcc 版本而定）
;; Function bigger (null)
{
  return 1;
}
```

gcc 在**任何最佳化開始之前**，就已經把 `x + 1 > x` 化簡成 `1`。所以「用 `-O0` 就安全」是錯的，UB 就是 UB。

### 三種應對方式

| 做法 | `bigger(INT_MAX)` | 說明 |
|---|---|---|
| `-fwrapv` | `-O2` 也回傳 **0** | 告訴 gcc「有號溢位照二補數回繞」，把 UB 變成有定義的行為，gcc 就不能再做上面的推理 |
| `-fsanitize=undefined`（UBSan，Undefined Behavior Sanitizer，未定義行為檢查器） | 回傳 0，**並在執行時報錯** | 在每個可能溢位的運算旁插入檢查，發生時印出位置 |
| 改寫程式碼 | — | 在運算前先檢查會不會溢位，例如 `if (x == INT_MAX)`；或改用無號型別 |

UBSan 的實測輸出（`ub.c:6:9` 是「第 6 行第 9 欄」，第 6 行就是 `int y = x + 1;`）：

```
$ gcc -O0 -fsanitize=undefined ub.c -o ub && ./ub
ub.c:6:9: runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'
bigger(INT_MAX) = 0
```

**UBSan 請搭配 `-O0` 或 `-Og` 用。** 實測 `-Og` 有報錯，但 `-O1`、`-O2` **沒有報錯**：`-fdump-tree-optimized` 顯示
`bigger` 裡確實有檢查（`.UBSAN_CHECK_ADD`），但 `main` 裡的呼叫已經被內嵌、在編譯時算成 `printf(..., 0)`，檢查根本沒機會執行。

### 另一種寫法：`x + y < x` 被改寫成 `y < 0`

> 2026-09-27 作業第 5 題補教：「加了正數反而變小就是溢位」這種檢查，`-O2` 下會失效。

`overflow_check.c` 完整原始碼：

```c
#include <stdio.h>
#include <limits.h>

int will_overflow(int x, int y)
{
    int sum = x + y;          /* 第 6 行 */
    return sum < x;           /* 加了正數反而變小，就代表溢位了？ */
}

int main(void)
{
    printf("will_overflow(INT_MAX, 1): %d\n", will_overflow(INT_MAX, 1));
    return 0;
}
```

```
$ gcc -O0 overflow_check.c -o oc && ./oc
will_overflow(INT_MAX, 1): 1
$ gcc -O2 overflow_check.c -o oc && ./oc
will_overflow(INT_MAX, 1): 0
$ gcc -O2 -fwrapv overflow_check.c -o oc && ./oc
will_overflow(INT_MAX, 1): 1
```

`-O2` 下 gcc 把函式改成什麼樣子（`-fdump-tree-optimized` 印出最佳化後的中間表示，只節錄 `will_overflow` 與 `main`）：

```
$ gcc -O2 -fdump-tree-optimized -c overflow_check.c -o /dev/null
$ cat overflow_check.c.*.optimized
int will_overflow (int x, int y)
{
  _1 = y_3(D) < 0;            ← 加法不見了，整個函式變成「y 是不是負的」
  _4 = (int) _1;
  return _4;
}

int main ()
{
  __printf_chk (2, "will_overflow(INT_MAX, 1): %d\n", 0);   ← 內嵌後直接算成 0
  return 0;
}
```

（`y_3(D)` 就是參數 `y`，後面的 `_3`、`(D)` 是 gcc 內部的編號，不用管。）

推理過程：**假設有號溢位不會發生**，`x + y` 就是數學上的加法，那麼 `x + y < x` 兩邊同減 `x` 就等於 `y < 0`。
在數學上這完全正確，可是這個檢查**本來就是要抓溢位**——編譯器用「溢位不會發生」當前提，檢查就被改寫成一個永遠抓不到溢位的式子。
結論：**溢位檢查要在做運算之前、用不會溢位的式子判斷**，例如 `y > 0 && x > INT_MAX - y`。
寫這種檢查時，還要注意**檢查式本身**會不會溢位：`INT_MAX - y` 在 `y` 是正數時安全，但 `INT_MAX - x` 在 `x` 是負數時就溢位了。

> 課後討論：[第十一節第 1 章〈`sum < x` 這個溢位檢查：前提、化簡的根據、怎麼確定 gcc 化簡了〉](#1-sum--x-這個溢位檢查前提化簡的根據怎麼確定-gcc-化簡了)

**UBSan 在 `-O1`、`-O2` 下報不報錯，不能預測。** 上一段的 `ub.c` 在 `-O2 -fsanitize=undefined` 下沒有報錯，
這支 `overflow_check.c` 卻在 `-O0`、`-O1`、`-O2` 都有報錯（輸出都是 `overflow_check.c:6:9: runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'`，
而且 `-O2` 印出的結果是 1，不是沒加 UBSan 時的 0）。`-fdump-tree-optimized` 顯示差別在 `main`：`ub.c` 的 `main` 已經被算成 `printf(..., 0)`，
這支的 `main` 還保留著 `.UBSAN_CHECK_ADD (2147483647, 1)`，檢查會真的執行。
兩支程式都是 `INT_MAX + 1`，最佳化器有沒有把檢查一起算掉，要看程式的寫法，使用者無法預先判斷。
所以結論不變：**用 UBSan 找錯時固定用 `-O0` 或 `-Og`**，高最佳化等級沒報錯不代表沒有 UB。

### kernel 怎麼處理：`-fno-strict-overflow`

本機裝的 kernel 標頭裡就有 kernel 的頂層 `Makefile`：

```
$ grep -n -B1 strict-overflow /usr/src/linux-headers-7.0.0-34-generic/Makefile
1106-# disable invalid "can't wrap" optimizations for signed / pointers
1107:KBUILD_CFLAGS	+= -fno-strict-overflow
```

`man gcc` 對它的說明：`-fstrict-overflow` 的反面**等於 `-fwrapv -fwrapv-pointer`**，也就是有號整數**和指標**的運算都照回繞處理。
kernel 選擇「整個專案關掉這種推理」，而不是靠每個開發者自己記得避開 UB：kernel 有幾千萬行程式碼，
一個被最佳化掉的溢位檢查就可能是安全漏洞。
**代價是少了一部分最佳化**：編譯器不能再假設「有號數加 1 一定變大」，某些靠這個假設才能化簡的運算與迴圈就得照字面做，程式可能稍慢。
kernel 的取捨是：寧可慢一點，也要行為可預期。這也是為什麼要讀 kernel 程式碼，得先知道它是用哪些 gcc 選項編出來的。

---

## 九、本課用到的工具與選項

### `gcc`：本課用到的選項

`gcc` 的選項有上千個，**這裡只列本課新用到的 10 個**（`-g`、`-O0`、`-O2` 等見前兩篇），完整清單見 `man gcc`（警告在 Warning Options、`-fsanitize` 在
Instrumentation Options、`-fwrapv` 在 Code Gen Options、`-fdump-tree-*` 在 Developer Options）。
第 1 課的階段控制選項見 [第 1 課筆記第七節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#七本課用到的工具與選項)。

| 選項 | 作用 |
|---|---|
| `-Wall` | 開啟一組常用警告（**不是**全部） |
| `-Wextra` | 在 `-Wall` 之外再開一組，包括本課的 `-Wsign-compare`、`-Wtype-limits` |
| `-Wsign-compare` | 有號與無號比較時警告（C 語言中由 `-Wextra` 開啟） |
| `-Wtype-limits` | 比較結果因型別範圍永遠成立或永遠不成立時警告（由 `-Wextra` 開啟） |
| `-Woverflow` | 常數轉型後值改變時警告（預設就開） |
| `-fwrapv` | 有號整數溢位照二補數回繞，變成有定義的行為 |
| `-fno-strict-overflow` | 等於 `-fwrapv -fwrapv-pointer`，kernel 用這個 |
| `-fsanitize=undefined` | 插入執行時的 UB 檢查（UBSan）；搭配 `-O0`／`-Og` |
| `-fdump-tree-original`／`-fdump-tree-optimized` | 把 gcc 內部的中間表示在「剛讀完原始碼」／「最佳化做完」時存成檔案 |

**警告到底有幾個？可以問 gcc**：`gcc -Q --help=warnings` 列出每個警告選項與目前狀態。本機 gcc 13.3 共列出 427 個；
數標成 `[enabled]` 的：

```
$ gcc -Q --help=warnings | grep -c '\[enabled\]'              → 103   （什麼都不加）
$ gcc -Q --help=warnings -Wall | grep -c '\[enabled\]'        → 156
$ gcc -Q --help=warnings -Wall -Wextra | grep -c '\[enabled\]' → 172
```

所以 `-Wall` 只多開了 53 個，還有一半以上的警告沒開。想知道某個警告有沒有開，就把它 `grep` 出來看。

### `printf` 的格式：完整文法

```
%[旗標][寬度][.精度][長度修飾]轉換字元
```

只有**轉換字元**是必要的，其他四個位置都可以省略。以本課的 `%02x` 為例：旗標 `0`（補零）、寬度 `2`、沒有精度、沒有長度修飾、轉換字元 `x`。
以下依 `man 3 printf`（本機有安裝）**全部列出**。

**轉換字元（22 個）**：

| 字元 | 參數型別 → 輸出 |
|---|---|
| `d`、`i` | `int` → 有號十進位 |
| `o`、`u`、`x`、`X` | `unsigned int` → 無號的八進位、十進位、十六進位（小寫／大寫） |
| `e`、`E` | `double` → 科學記號 `1.5e+03` |
| `f`、`F` | `double` → 小數 `1500.000000` |
| `g`、`G` | `double` → 依數值大小自動選 `f` 或 `e` 格式，並去掉尾端的 0 |
| `a`、`A` | `double` → 十六進位浮點數 |
| `c` | `int` → 一個字元 |
| `s` | `const char *` → 字串 |
| `C`、`S` | 分別等於 `lc`、`ls`（寬字元），不是 C 標準，不建議用 |
| `p` | `void *` → 位址（十六進位） |
| `n` | 不輸出；把「到目前為止印了幾個字元」寫進參數指向的 `int` |
| `m` | glibc 擴充：印出 `strerror(errno)`，不需要參數 |
| `%` | 印出 `%` 本身 |

**長度修飾（10 個）**：放在轉換字元前面，說明參數實際的型別。

| 修飾 | 搭配整數轉換時，參數是 |
|---|---|
| `hh` | `signed char`／`unsigned char` |
| `h` | `short`／`unsigned short` |
| `l` | `long`／`unsigned long`（本課 `%ld` 印 `LONG_MAX`） |
| `ll` | `long long`／`unsigned long long` |
| `q` | 等於 `ll`，BSD 的舊寫法 |
| `L` | 搭配浮點轉換時是 `long double` |
| `j` | `intmax_t`／`uintmax_t` |
| `z` | `size_t`（本課 `%zu` 印 `sizeof` 的結果） |
| `Z` | 等於 `z`，比 `z` 更早的舊寫法 |
| `t` | `ptrdiff_t`（兩個指標相減的型別，第 3 課） |

**旗標（7 個）**：`#`（替代格式，例如 `%#x` 印出 `0x` 前綴）、`0`（用 0 補滿寬度）、`-`（靠左對齊）、
空白（正數前面留一個空白）、`+`（正數也印 `+`）、`'`（依 locale 加千分位）、`I`（依 locale 用替代的數字字形）。

**寬度**是最少印幾個字元；**精度**對整數是最少幾位數、對浮點數是小數點後幾位、對字串是最多印幾個字元。
兩者都可以寫 `*`，改從參數取值。

格式與參數型別不合（例如用 `%d` 印 `unsigned int` 的 4294967295）是未定義行為；本課把 `-1` 轉成 `(unsigned)` 再用 `%u`／`%x` 印，就是為了避免這個。
`-Wall` 裡的 `-Wformat` 會抓大部分這類錯誤。

### `<limits.h>`：本課用到的常數

本課用了 `CHAR_MIN`、`CHAR_MAX`、`INT_MIN`、`INT_MAX`、`UINT_MAX`、`LONG_MAX`。每個整數型別都有對應的
`_MIN`／`_MAX`（無號型別只有 `_MAX`），另有 `CHAR_BIT`（一個 byte 幾個 bit，Linux 上是 8）。
本機**沒有**安裝這個標頭檔的 man page（`man limits.h` 回 `No manual entry`），完整清單直接看標頭檔：`/usr/include/limits.h`。

### 觀察工具

| 工具 | 本課的用法 | 完整說明 |
|---|---|---|
| `gdb` | `print &x`、`print sizeof(p)`、`x/4xb &x`、`x/1xw &x` | [gdb完整命令參考](./gdb完整命令參考.md)（`x` 的格式字母見〈輸出格式 `/FMT`〉） |
| `readelf` | `readelf -h` 看 `Data:` 那一行 | [readelf完整選項參考](./readelf完整選項參考.md) |

---

## 十、動手作業

作業在 [`small-project/s1-02-data-repr/`](../small-project/s1-02-data-repr/README.md)：比 `int` 小的型別的範圍、`long` 的 8 byte 位元組順序、
一組轉換的先預測後驗證、另外兩種有號無號比較陷阱、一個錯誤的溢位檢查。題目刻意跟本篇範例不同，要用本篇的觀念推理，照抄範例答不出來。

---

## 十一、課後討論

課堂與複習之後的追問整理在這裡，一個主題一章，新的章加在最後。正文被問到的地方有一行「課後討論」連過來。

- [第 1 章](#1-sum--x-這個溢位檢查前提化簡的根據怎麼確定-gcc-化簡了)：`sum < x` 這個溢位檢查：前提、化簡的根據、怎麼確定 gcc 化簡了

### 1. `sum < x` 這個溢位檢查：前提、化簡的根據、怎麼確定 gcc 化簡了

> 2026-10-27 課後提問（10-27 複習批改[自我測驗](#自我測驗)第 9 題之後的一連串追問）：
> - 「另外第二個原因,我覺得你強調溢位在o2是Ub這樣沒什麼不好,但是這個理由感覺不是用來判斷y<0的憑據 題目之所以會用sum < x判斷已經意味這寫code的人知道溢位不會發生,所以他用這個判斷是來判斷溢位是否發生 最強而有力的證據就是sum < x的溢位判斷是在y <0的前提,所以當y>0發生溢位他自然無法抓到」
> - 「好像還是不對, sum < x爲什麼可以當作溢位檢查? 加入x = 5, y = -2那不就是sum < x嗎?這怎麼算是溢位?」
> - 「那我完全不理解這個問題在問什麼」
> - 「那我的問題就是爲什麼假設溢位不會發生的前提下,可以做x+y < x 化簡爲 y<0?」
> - 「你怎麼確定gcc有做化簡的?」
> - 「另外第二小題一定要強調大於0相當於不等於0嗎?無號運算結果是>= 0 不是很正常,gcc不警告就很正常」
>
> 正文出處：[第八節〈另一種寫法：`x + y < x` 被改寫成 `y < 0`〉](#另一種寫法x--y--x-被改寫成-y--0)（前五則）、[第七節〈`-Wtype-limits` 只抓「永遠成立／永遠不成立」的比較〉](#-wtype-limits-只抓永遠成立永遠不成立的比較)（最後一則）。

**一句話整理：`sum < x` 只在 `y ≥ 0` 時才是溢位檢查；它跟 `y < 0` 唯一的差別就是「有沒有溢位」，gcc 假設溢位不會發生，就把這個差別、連同整個檢查一起化簡掉了。**

以下照討論的順序排。用到的 `overflow_check.c` 就是正文〈另一種寫法〉那一支（檔案：[`overflow_check.c`](../small-project/s1-02-data-repr/examples/fold-dump/overflow_check.c)），為了閱讀方便再貼一次：

```c
#include <stdio.h>
#include <limits.h>

int will_overflow(int x, int y)
{
    int sum = x + y;          /* 第 6 行 */
    return sum < x;           /* 加了正數反而變小，就代表溢位了？ */
}

int main(void)
{
    printf("will_overflow(INT_MAX, 1): %d\n", will_overflow(INT_MAX, 1));
    return 0;
}
```

#### (1) 為什麼「只有 `-O2`」抓不到：不能只用「`y < 0` 抓不到 `y > 0`」解釋

第一則追問的論點是：`sum < x` 本身就是「`y < 0`」的判斷，所以 `y > 0` 又溢位時自然抓不到。這句描述的是 gcc **改寫之後**的樣子，但解釋不了正文的實測：

```
$ gcc -O0 overflow_check.c -o oc && ./oc
will_overflow(INT_MAX, 1): 1
$ gcc -O2 overflow_check.c -o oc && ./oc
will_overflow(INT_MAX, 1): 0
$ gcc -O2 -fwrapv overflow_check.c -o oc && ./oc
will_overflow(INT_MAX, 1): 1
```

如果 `sum < x` 本來就只能判斷 `y < 0`，`-O0` 也應該回傳 0；實際上 `-O0` 抓到了（`INT_MAX + 1` 回繞成 `INT_MIN`，`INT_MIN < INT_MAX` 成立）。
同一份程式碼，**只有**「`-O2` 而且溢位是未定義行為」時才抓不到，加了 `-fwrapv`（把有號溢位定義成二補數回繞）又抓得到。
所以這個差別要用 [第八節〈有號溢位在 C 標準裡是「未定義行為」〉](#有號溢位在-c-標準裡是未定義行為) 解釋：gcc 憑 UB 假設溢位不會發生，才能把 `sum < x` 改寫成 `y < 0`。
`-O2` 編出來的 `will_overflow` 只剩三行（本機 gcc 13.3，`gcc -O2 -S -fno-asynchronous-unwind-tables overflow_check.c`）：

```asm
will_overflow:
	endbr64                # 控制流程保護標記，跟計算無關
	movl	%esi, %eax     # %eax = y（第二個參數在 %esi）
	shrl	$31, %eax      # 右移 31 位，只留下 y 的符號位：y 是負的得 1，否則得 0
	ret                    # 回傳的就是 y < 0；加法和 x 都不見了
```

#### (2) `sum < x` 只在 `y ≥ 0` 時才是溢位檢查

`x = 5, y = -2`：`sum = 3`，`sum < x` 成立，可是根本沒有溢位。所以 `sum < x` **不是通用的溢位檢查**，它有一個沒寫出來的前提：寫的人知道 `y ≥ 0`（例如 `y` 是長度、個數）。
正文範例的註解寫的就是「**加了正數**反而變小」。在這個前提下，推理是：加一個非負數，數學上只會變大或不變，所以 `sum < x` 只可能是溢位回繞造成的。

`y` 可以是負數時，這招從頭就不對：

- `y < 0` 時，`sum < x` 本來就是正常結果（5 + (−2) = 3）；
- 往負的方向溢位（`INT_MIN + (−1)`）會回繞成 `INT_MAX`，結果反而變**大**，`sum < x` 抓不到。

`naive_builtin.c`（檔案：[`naive_builtin.c`](../small-project/s1-02-data-repr/examples/premise/naive_builtin.c)）把這種寫法跟 gcc 的內建函式 `__builtin_add_overflow`（真的溢位才回傳 1，結果寫進第三個參數）放在一起比：

```c
#include <stdio.h>
#include <limits.h>

__attribute__((noinline)) int naive(int x, int y)
{
    int sum = x + y;
    return sum < x;                         /* 只在 y >= 0 時才是溢位檢查 */
}

__attribute__((noinline)) int builtin(int x, int y)
{
    int sum;
    return __builtin_add_overflow(x, y, &sum);   /* 真的溢位才回傳 1 */
}

int main(void)
{
    int xs[] = { 5, INT_MAX, INT_MIN };
    int ys[] = { -2, 1, -1 };
    for (int i = 0; i < 3; i++)
        printf("x=%d y=%d  naive=%d  builtin=%d\n",
               xs[i], ys[i], naive(xs[i], ys[i]), builtin(xs[i], ys[i]));
    return 0;
}
```

（`__attribute__((noinline))`：不准 gcc 把函式內嵌進 `main`，否則 `-O2` 會在編譯時就把結果算好。）

```
$ gcc -O0 naive_builtin.c -o nb && ./nb
x=5 y=-2  naive=1  builtin=0
x=2147483647 y=1  naive=1  builtin=1
x=-2147483648 y=-1  naive=0  builtin=1
$ gcc -O2 naive_builtin.c -o nb && ./nb
x=5 y=-2  naive=1  builtin=0
x=2147483647 y=1  naive=0  builtin=1
x=-2147483648 y=-1  naive=1  builtin=1
```

| 這一列 | `naive` 錯在哪 |
|---|---|
| `5, -2`（兩種等級） | 沒溢位卻回傳 1：`y` 是負的，不符合前提 |
| `INT_MAX, 1`，`-O0` | 對：符合前提，照字面算、回繞，抓到 |
| `INT_MAX, 1`，`-O2` | 抓不到：被改寫成 `y < 0`，`y` 是 1 |
| `INT_MIN, -1`，`-O0` | 抓不到：往負的方向溢位，結果變大 |
| `INT_MIN, -1`，`-O2` | 回傳 1 是**碰巧**：改寫後只看 `y < 0`，`y` 剛好是負的 |

所以第一則追問說的「前提」方向反了：前提是 **`y ≥ 0`**，不是 `y < 0`。在 `y ≥ 0` 的前提下，`-O2` 改寫出來的 `y < 0` **永遠為假**，檢查一次都不會觸發，這就是自我測驗第 9 題說的「完全失效」。
兩個方向都要顧的正確寫法是在運算**之前**判斷：`(y > 0 && x > INT_MAX - y) || (y < 0 && x < INT_MIN - y)`，或直接用 `__builtin_add_overflow`。

#### (3) 這題到底在問什麼

自我測驗第 9 題第三問（當時的題目是「為什麼 `int sum = x + y; if (sum < x)` 這種溢位檢查在 `-O2` 下可能完全失效？」，沒寫出 `y` 是正數；這次討論後補上了這個前提），拆開來是這樣：

1. **寫的人想做什麼**：`y` 是正數，他擔心 `x + y` 超過 `INT_MAX`。他知道 `int` 溢位會繞回負數，所以想「加了正數反而變小，就是溢位了」，寫出 `if (sum < x)`。照二補數算，`INT_MAX + 1` 變成 `INT_MIN`，檢查成立，這個想法本身沒錯。
2. **gcc 在 `-O2` 怎麼看**：C 標準說有號溢位是 UB，gcc 可以假設它**不會發生**。在這個假設下，`x + y` 就是數學上的加法，`x + y < x` 等於 `y < 0`，gcc 就改寫成 `if (y < 0)`。
3. **結果**：寫的人的 `y` 一定是正的，`y < 0` 永遠為假，檢查還在原始碼裡，編出來卻等於沒寫。

一句話：**用來抓溢位的檢查，被「溢位不會發生」這個假設刪掉了。**

#### (4) 為什麼「不會溢位」就能把 `x + y < x` 化簡成 `y < 0`

靠的是數學上的規則：**不等式兩邊同時減同一個數，大小關係不變**。

```text
x + y < x
  兩邊同減 x
y < 0
```

白話：「`x` 加上 `y` 之後比 `x` 小」等於「`y` 是負的」——加正數只會變大、加 0 不變、只有加負數才變小。
這條規則的前提是 `x + y` 真的是**數學上的和**。有回繞時就不是：`INT_MAX + 1` 數學上是 2147483648，回繞後是 −2147483648，於是 `x + y < x` 為真、`y < 0` 為假，兩邊不一樣了。
「不會溢位」保證 C 算出來的 `x + y` 等於數學上的和，規則就能用。

`simp8.c`（檔案：[`simp8.c`](../small-project/s1-02-data-repr/examples/simplify8/simp8.c)）用 8 位元有號數（−128～127）把全部 65536 組 `x`、`y` 都試一次，用回繞後的結果比較兩個式子：

```c
#include <stdio.h>

/* 用 8 位元有號數（-128～127）把所有 x、y 組合都試一次 */
int main(void)
{
    int total = 0, overflow = 0, differ = 0, differ_no_ovf = 0;
    for (int x = -128; x <= 127; x++)
        for (int y = -128; y <= 127; y++) {
            int math = x + y;                    /* 數學上的和（int 夠大，不會溢位） */
            signed char wrapped = (signed char)math;  /* 8 位元 CPU 實際留下的結果（回繞） */
            int ovf = (math < -128 || math > 127);
            int a = wrapped < x;                 /* 原本的檢查 x + y < x */
            int b = y < 0;                       /* 化簡後的 y < 0 */
            total++;
            overflow += ovf;
            if (a != b) {
                differ++;
                if (!ovf) differ_no_ovf++;
            }
        }
    printf("全部組合: %d\n", total);
    printf("會溢位的組合: %d\n", overflow);
    printf("x+y<x 與 y<0 結果不同的組合: %d\n", differ);
    printf("其中「沒溢位」卻不同的: %d\n", differ_no_ovf);
    return 0;
}
```

```
$ gcc -O0 simp8.c -o simp8 && ./simp8
全部組合: 65536
會溢位的組合: 16384
x+y<x 與 y<0 結果不同的組合: 16384
其中「沒溢位」卻不同的: 0
```

- 沒溢位的組合，兩個式子**全部一樣**（0 組不同）：只要不溢位，化簡永遠正確。
- 會溢位的 16384 組，兩個式子**全部不同**：差別剛好就是「有沒有溢位」。

寫檢查的人要的正是這個差別；gcc 假設溢位不會發生，等於把這個差別當成不存在，化簡後就只剩 `y < 0`。

#### (5) 怎麼確定 gcc 真的化簡了：用 `-fdump-tree-all` 看每一關

(1) 的組合語言只證明了**結果**是 `y < 0`。要看到 gcc 在哪一步改的，用正文〈另一種寫法〉用過的 `-fdump-tree-…`：gcc 先把程式轉成內部的中間表示（GIMPLE），再一關一關（pass）做最佳化；
`-fdump-tree-all` 把**每一關之後**的樣子各存成一個檔，`-fdump-tree-<關卡名>`（例如 `-fdump-tree-optimized`）只存那一關。
這裡只用到這兩種寫法；`-fdump-tree-` 後面還能接 `-details` 這類修飾、gcc 還有 `-fdump-rtl-*`、`-fdump-ipa-*` 等其他傾印，完整清單見 `man gcc` 的〈GCC Developer Options〉。

```
$ gcc -O2 -c -fdump-tree-all overflow_check.c -o /dev/null
$ ls overflow_check.c.* | wc -l
122
```

122 個檔，檔名中間的數字是關卡順序（`overflow_check.c.034t.ccp1`、`overflow_check.c.035t.forwprop1`……）。逐檔找 `y < 0` 第一次出現在哪一關，是第 35 關 `forwprop1`。跟前一關並排（`sum_4`、`x_2(D)` 後面的 `_4`、`(D)` 是 gcc 內部的編號）：

```text
=== 第 34 關 ccp1 之後（還是原樣）      === 第 35 關 forwprop1 之後
sum_4 = x_2(D) + y_3(D);               sum_4 = x_2(D) + y_3(D);
_1 = sum_4 < x_2(D);                   _1 = y_3(D) < 0;          ← 在這一關被改寫
_5 = (int) _1;                         _5 = (int) _1;
return _5;                             return _5;
```

`forwprop` 是 forward propagation（往前代入）：把 `sum_4` 的定義 `x + y` 代進 `sum_4 < x`，得到 `x + y < x`，再化簡成 `y < 0`。
改寫後 `sum_4` 沒人用了，後面的關卡把它當死碼刪掉，最後一關 `optimized` 就是正文印過的樣子（`_1 = y_3(D) < 0;`，加法不見了）。

對照組：加 `-fwrapv`，同一關不化簡。

```
$ gcc -O2 -fwrapv -c -fdump-tree-forwprop1 -fdump-tree-optimized overflow_check.c -o /dev/null
```

```text
=== forwprop1 之後                       === optimized
sum_4 = x_2(D) + y_3(D);               sum_4 = x_2(D) + y_3(D);
_1 = x_2(D) > sum_4;                   _1 = x_2(D) > sum_4;
```

`-fwrapv` 下 `forwprop1` 只把 `sum < x` 換成等價的 `x > sum`，加法保留到最後。同一個 `-O2`、同一關，只差在溢位有沒有定義，所以是**因為溢位是 UB** 才化簡，不是因為 `-O2` 本身。

#### (6) 為什麼 `> 0` 不警告：不是因為「無號的結果 ≥ 0 很正常」

自我測驗第 9 題第二問（為什麼 gcc 對 `len - used < 0` 警告、對 `> 0` 不警告），最後一則追問的理由是「無號運算結果 ≥ 0 很正常，所以不警告」。這個理由分不出 `> 0` 和 `>= 0`：
正文〈`-Wtype-limits` 只抓…〉的 `tl.c` 實測，gcc 對 `a - b >= 0` **一樣警告**（`comparison of unsigned expression in '>= 0' is always true`）。

`-Wtype-limits` 只看一件事：**比較的結果是不是在編譯時就決定了**。

- 「無號結果不可能是負的」解釋的是 `< 0`（永遠假）和 `>= 0`（永遠真）為什麼被警告；
- `> 0` 不被警告，是因為它的結果**不固定**：值是 0 時為假，其他時候為真，是一個有意義的判斷。

「`> 0` 等於 `!= 0`」只是這件事的一種講法；寫成「`> 0` 在值為 0 時是假的，不是恆真」也一樣算答到。

---

## 自我測驗

> 參考答案寫在每題下面（2026-10-06 使用者指定）。作答時先遮住答案、自己想過再對照。

1. 一個 `int` 變數存著 `-1`，用 `%d` 印出 `-1`，轉成 `unsigned` 再用 `%u` 印出 `4294967295`。記憶體裡的內容有沒有變？為什麼同一個東西會印出兩個不同的數？
   > **參考答案**【[第五節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#五有號整數二補數)、[第六節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#六有號與無號之間的轉換位元不變換個讀法)】（2026-10-10 改寫成核心關鍵格式）
   >
   > 核心關鍵：
   > 1. 沒有變：記憶體裡始終是 `ff ff ff ff`。同樣寬度的有號、無號轉換只換讀法、不動位元。
   > 2. `%d` 用二補數讀（最高位權重是 −2³¹），得到 −1；`%u` 把每一位都當正的權重，得到 2³² − 1 = 4294967295。
   >
   > 詳細說明：是 `printf` 的 `%d`／`%u` 決定用哪一種讀法解讀那 32 個 bit；變數的型別決定 C 在運算、比較時用哪一種讀法。
2. `int x = 0x12345678; unsigned char *b = (unsigned char *)&x;` 在本機 `*b` 是多少？換到大端序的機器上是多少？
   為什麼 `int *p = &x;` 的 `*p` 在兩種機器上都是 `0x12345678`？
   > **參考答案**【[第三節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#三位元組順序endianness)〈什麼時候要在意位元組順序？〉】（2026-10-16 改寫成核心關鍵格式）
   >
   > 核心關鍵：
   > 1. 本機（小端序）`*b` 是 `0x78`，大端序機器是 `0x12`。
   > 2. `*p` 兩邊都一樣，是因為每台機器都用**自己那一套順序寫、再用同一套順序讀**（小端把第一個 byte 當最低位，也就是從後往前拼；大端把第一個 byte 當最高位，從前往後拼），一來一回抵消。
   >
   > 詳細說明：小端機器寫成 `78 56 34 12`，大端機器寫成 `12 34 56 78`。`*p` 讀 4 byte 是因為 `p` 指向 `int`；`p` 本身在 x86-64 上是 8 byte。看得出順序的只有兩種情況：用 `unsigned char *` 一個 byte 一個 byte 看，或者一台機器寫、另一種順序的機器讀（網路封包、檔案，第 5 題）。
3. `int n = -1; unsigned int len = 5;`，`n < len` 為什麼是假？只加 `-Wall` 編譯，gcc 會不會提醒你？
   > **參考答案**【[第四節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#四無號整數二進位與回繞)〈怎麼確定一個常數的型別？比較時以誰為準？〉、[第六節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#六有號與無號之間的轉換位元不變換個讀法)、[第七節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#七陷阱有號和無號混在一起比較)】（2026-10-16 改寫成核心關鍵格式）
   >
   > 核心關鍵：
   > 1. 比較之前做一般算術轉換：`int` 和 `unsigned int` 等級相同，無號贏，`n` 被轉成 `unsigned int`。
   > 2. 轉換時位元不變、換成無號讀法：`-1`（`0xffffffff`）變成 4294967295 > 5，所以 `n < len` 為假。
   > 3. 只加 `-Wall` **不會**提醒（C 的 `-Wall` 不含 `-Wsign-compare`），要加 `-Wextra`。
   >
   > 詳細說明：這個轉換是定義好的，不是溢位，也不是未定義行為；未定義行為的是有號整數**運算**溢位（第八節，例如 `INT_MAX + 1`）。2026-10-16 實測：`gcc -Wall` 沒有輸出，加 `-Wextra` 印出 `comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]`。
4. `int y = x + 1; return y > x;` 在 `x = INT_MAX` 時，`-O0` 回傳 0、`-O2` 回傳 1。為什麼會這樣？這算不算 gcc 的 bug？
   kernel 用哪個選項避免這種情況，代價是什麼？
   > **參考答案**【[第八節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#八溢位無號回繞有號是未定義行為)】（2026-10-11 改寫成核心關鍵格式）
   >
   > 核心關鍵：
   > 1. 有號溢位是未定義行為（UB），C 標準允許編譯器**假設它不會發生**。
   > 2. `-O0` 照字面算，`INT_MAX + 1` 回繞成 `INT_MIN`，`y > x` 為假 → 0；`-O2` 依這個假設推論 `x + 1` 一定大於 `x`，直接回傳 1。
   > 3. 不是 gcc 的 bug：標準對發生 UB 的程式沒有任何要求，兩種結果都合法。
   > 4. kernel 用 `-fno-strict-overflow`（等於 `-fwrapv -fwrapv-pointer`），讓溢位照二補數回繞、變成有定義的行為；代價是少了依賴「不會溢位」的最佳化，程式可能稍慢。
   >
   > 詳細說明：連 `-O0` 也不保證照字面算：第四節的 `overflow.c` 把同一件事寫成一行 `return x + 1 > x;`，`-O0` 也回傳 1，因為 gcc 在最佳化之前的化簡階段就把它改掉了（筆記第八節用 `-fdump-tree-original` 看到）。所以本題的 `bigger` 才故意拆成兩行。
5. 情境題：你從網路封包裡讀到 4 個 byte，依序是 `00 00 01 00`。協定規定數字用大端序，所以它代表 256。
   如果直接用 `int *` 把這 4 個 byte 讀進本機（x86-64），會得到多少？為什麼？
   > **參考答案**【[第三節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#三位元組順序endianness)】（2026-10-10 改寫成核心關鍵格式）
   >
   > 核心關鍵：
   > 1. 會得到 65536（`0x00010000`），不是協定要的 256。
   > 2. x86-64 是小端序：位址最低的 byte 當最低位，記憶體從低到高的 `00 00 01 00` 拼回來是 `0x00010000`。
   >
   > 詳細說明：反過來排的單位是**整個 byte**（兩個十六進位數字一組），byte 裡面的 `01` 不會變成 `10`（2026-10-25 批改 10-24 複習後補上）。網路上的位元組順序（network byte order）是大端序，所以讀網路資料要用 `ntohl`（network to host long）之類的函式轉換。在大端序的機器上這個函式什麼都不做，在 x86-64 上會把 4 個 byte 反過來。
6. `printf("%p")` 印出 `0x7ffe50b23624`，只有 12 位十六進位。指標到底佔幾個 byte？印出來為什麼比較短？
   如果在 kernel 的錯誤訊息裡看到 `0xffff888102a4c000`，從位址的長相你能判斷什麼？
   > **參考答案**【[第二節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#二指標就是位址)〈位址看起來只有 6 byte？——`%p` 不補零，而且最上面 2 byte 一定是 0〉】（2026-10-11 改寫成核心關鍵格式）
   >
   > 核心關鍵：
   > 1. 指標佔 8 byte。
   > 2. x86-64 虛擬位址只用 48 bit，使用者空間的位址上面 2 byte 一定是 `00 00`，`%p` 不補前導零，所以只印出 12 位。
   > 3. `0xffff888102a4c000` 整個是一個位址；上面 16 bit 全是 1，表示它在位址空間的核心那一半，所以是 kernel 的位址。
   >
   > 詳細說明：canonical address 規定上面 16 bit 必須跟第 47 bit 相同，所以合法的位址只有兩段：`0x0000000000000000`～`0x00007fffffffffff`（使用者空間，第 47 bit 是 0，上面全 0）與 `0xffff800000000000`～`0xffffffffffffffff`（核心，第 47 bit 是 1，上面全 1），中間那一大段不合法。「看起來像符號擴展」沒錯，形式上就是把第 47 bit 往上複製；但這是 CPU 對位址格式的規定，跟 C 的有號、無號型別無關。
7. `2147483648` 和 `0x80000000` 是同一個數，為什麼一個的型別是 `long`、一個是 `unsigned int`？
   `-1 < 0x80000000` 和 `-1 < 2147483648` 的結果各是什麼？為什麼？
   > **參考答案**【[第四節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#四無號整數二進位與回繞)〈常數也有型別：`0u` 的 `u` 是什麼〉、〈怎麼確定一個常數的型別？比較時以誰為準？〉】（2026-10-10 改寫成核心關鍵格式）
   >
   > 核心關鍵：
   > 1. 沒有後綴的常數，依序嘗試、選第一個裝得下的型別。十進位的清單是 `int` → `long` → `long long`（永遠不變成無號），`2147483648` 超過 `INT_MAX`，所以是 `long`。
   > 2. 十六進位的清單是 `int` → `unsigned int` → `long` → …，`0x80000000` 裝得進 `unsigned int`，所以是 `unsigned int`。
   > 3. `-1 < 0x80000000` 為**假**：`int` 與 `unsigned int` 比，`-1` 轉成 4294967295。
   > 4. `-1 < 2147483648` 為**真**：`int` 與 `long` 比，`-1` 轉成 `long` 還是 −1。
   >
   > 詳細說明：十六進位清單多放無號型別，是因為寫十六進位的人通常在寫位元樣式（例如遮罩 `0x80000000`），要的是那個 bit 樣式，不是負數。
8. `signed char c = -1; unsigned int u = c;` 之後 `u` 是多少？擴展時新補的高位是 0 還是 1，由哪個型別決定？
   如果把 `c` 改宣告成 `unsigned char c = 0xff;`，`u` 又是多少？
   > **參考答案**【[第六節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#六有號與無號之間的轉換位元不變換個讀法)】（2026-10-20 改寫成核心關鍵格式）
   >
   > 核心關鍵：
   > 1. `u` = **4294967295**（`0xffffffff`，2³² − 1）。
   > 2. 擴展時補什麼由**原本的型別**決定：`signed char` 是有號的，做符號擴展，符號位是 1 就補 1；補完再以無號讀。
   > 3. 改成 `unsigned char c = 0xff`：原本的型別無號 → 零擴展補 0 → `0x000000ff` = **255**。
   >
   > 詳細說明：順序是「先依來源型別擴展到 32 bit，再換成目標型別的讀法」。目標型別 `unsigned int` 是無號的，並不會讓擴展改成補 0——同樣是 `-1`，來源 `signed char` 就補 1。n 個 bit 全是 1 ＝ 2ⁿ − 1，所以 32 bit 全 1 是 2³² − 1；2³¹ − 1（`0x7fffffff`）是有號 `int` 的最大值，最高位是 0，不是這題的答案。
9. 情境題：有人寫了 `if (len - used > 0)` 判斷「緩衝區還有沒有剩餘空間」，`len`、`used` 都是 `size_t`，用 `-Wall -Wextra` 編譯沒有任何警告。
   這個判斷在什麼情況下會出錯？為什麼 gcc 對 `len - used < 0` 會警告、對 `> 0` 卻不會？
   另外，有人用 `int sum = x + y; if (sum < x)` 檢查「`x` 加上一個**正數** `y` 會不會溢位」，為什麼這個檢查在 `-O2` 下可能完全失效？
   > **參考答案**【[第七節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#七陷阱有號和無號混在一起比較)〈`-Wtype-limits` 只抓「永遠成立／永遠不成立」的比較〉、[第八節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#八溢位無號回繞有號是未定義行為)〈另一種寫法：`x + y < x` 被改寫成 `y < 0`〉】（2026-10-11 改寫成核心關鍵格式）
   >
   > 核心關鍵：
   > 1. `len < used` 時，`len - used` 回繞成很大的正數，判斷誤以為「還有空間」。
   > 2. 無號數 `< 0` 永遠為假，所以 gcc 警告；`> 0` 等於 `!= 0`，結果不固定，是合法寫法，所以不警告。
   > 3. 有號溢位是 UB，gcc 假設它不會發生，把 `x + y < x` 改寫成 **`y < 0`**；真的溢位時（`y` 是正的），這個檢查抓不到。
   >
   > 詳細說明：`len == used` 時結果是 0，`> 0` 為假，所以不是「永遠為真」，只有 `len < used` 會出錯。警告來自 `-Wtype-limits`（`-Wextra` 會開啟）。溢位那一問的記法：「`x + y < x`」在數學上（沒有溢位）跟「`y < 0`」完全等價，gcc 只是做了這個代數化簡，而這個化簡只在「不會溢位」時才成立。`sum < x` 這個寫法只在 `y ≥ 0` 時才是溢位檢查（`y` 是負數時，沒溢位也會成立；2026-10-27 題目補上了「正數」的前提），見[第十一節第 1 章](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#1-sum--x-這個溢位檢查前提化簡的根據怎麼確定-gcc-化簡了)。正確的溢位檢查要在運算**之前**，用不會溢位的式子判斷，例如 `y > 0 && x > INT_MAX - y`。
10. `long a = -1; unsigned int b = 1;` 與 `int c = -1; unsigned long d = 1;`，`a < b` 和 `c < d` 各是真還是假？兩邊各被轉成什麼型別，依據的是哪一條規則？為什麼 gcc 只對其中一個發出 `-Wsign-compare` 警告？
    > **參考答案**【[第四節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#四無號整數二進位與回繞)〈怎麼確定一個常數的型別？比較時以誰為準？〉】（2026-10-09 改寫成核心關鍵格式）
    >
    > 核心關鍵：
    > 1. `a < b` 為**真**：`long` 裝得下所有 `unsigned int` 的值 → 兩邊轉成 `long`，值不變，−1 < 1。
    > 2. `c < d` 為**假**：無號那邊（`unsigned long`）的等級 ≥ 有號那邊（`int`）→ 兩邊轉成 `unsigned long`，−1 變成 18446744073709551615。
    > 3. 只對 `c < d` 警告：只有它**把有號的值轉成無號**，負數的值會變；`a < b` 轉成 `long`，值不變。
    >
    > 詳細說明：要看的是**轉換的方向**，不是「`<` 兩邊有一邊是 signed」（`a < b` 的 `a` 也是 signed，卻沒有警告）：`c < d` 的轉換來源是有號的 `int`、目標是無號，`a < b` 的轉換來源是無號的 `unsigned int`、目標是有號的 `long`。`a < b` 的規則也不只是「`long` 等級比較高」：在 `long` 只有 4 byte 的 32 位元平台，`long` 裝不下所有 `unsigned int` 的值，兩邊會改成轉 `unsigned long`（C 標準的規則，本機未實測）。有號轉無號不是 UB，C 標準規定得很清楚：位元不變，換成無號的讀法（[第六節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#六有號與無號之間的轉換位元不變換個讀法)）；UB 是有號溢位。
11. `unsigned x`、`unsigned int x`、`signed x` 各是什麼型別？為什麼 `char` 和 `signed char` 在本機的範圍完全一樣，gcc 卻說它們是不同型別、宣告衝突？
   情境題：同事在錯誤訊息裡看到 `long unsigned int`，說「我明明寫的是 `unsigned long`，gcc 是不是把型別搞錯了？」你怎麼回答？
    > **參考答案**【[第一節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#一位元組與型別大小)〈`unsigned` 單獨寫就是 `unsigned int`：整數型別的所有寫法〉】（2026-10-10 改寫成核心關鍵格式）
    >
    > 核心關鍵：
    > 1. `unsigned x`、`unsigned int x` 都是 `unsigned int`；`signed x` 是 `int`（＝ `signed int`）。
    > 2. `char` 有號還是無號，C 標準交給平台決定（x86-64 有號、ARM 通常無號），標準沒辦法把它定成 `signed char` 或 `unsigned char` 其中之一，所以 `char` 是**第三個獨立的型別**。型別相不相同看標準的規定，不看範圍。
    > 3. gcc 沒搞錯：`unsigned long` 和 `long unsigned int` 是同一個型別。指定字的順序不重要、有其他指定字時 `int` 可以省略，gcc 印型別時用它自己的固定寫法（大小在前、有號無號居中、`int` 放最後）。
    >
    > 詳細說明：如果把 `char` 定義成 `signed char`，換到 ARM 這種 `char` 無號的平台就錯了，定成 `unsigned char` 則在 x86-64 錯，所以標準只好讓它自成一個型別。同一個函式先宣告成吃 `char`、再定義成吃 `signed char`，gcc 會報 `conflicting types`。`same.c` 實驗裡 `unsigned f(unsigned x)` 也被 gcc 印成 `unsigned int(unsigned int)`。
12. 每個行程有自己的位址空間，為什麼 Linux 還要把核心放進**每一個**行程的位址空間（上半段）？既然核心就在你的位址空間裡，為什麼使用者程式讀不到它？
   情境題：有個程式執行 `printf("%d\n", *(int *)0xffff888100000000);`，結果會怎樣？`strace` 顯示的 `si_code` 是 `SEGV_MAPERR` 還是 `SEGV_ACCERR`，為什麼？
