# C 的資料在記憶體裡長怎樣？—— 位元組、指標、位元組順序與有號／無號

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

### 常數也有型別：`0u` 的 `u` 是什麼

> 2026-09-24 課後提問：「`0u` 是什麼？」

`0u` 是**型別為 `unsigned int` 的 0**。程式裡直接寫的數字叫**整數常數（integer constant）**，它跟變數一樣有型別，
尾巴的字母叫**後綴（suffix）**，用來指定型別。用 C11 的 `_Generic`（依運算式的型別選一個結果）讓 gcc 回報型別。
`TYPE` 和 `SHOW` 是前處理器的巨集（第 1 課第二節：前處理就是文字替換），`#e` 把參數原封不動變成字串，所以能印出原始寫法。`suffix.c` 完整原始碼：

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

---

## 五、有號整數：二補數

`int` 也是 32 個 bit，但要能表示負數。現代所有 CPU 都用**二補數（two's complement）**，規則只改一個地方：
**最高位（第 31 位）的權重是 −2³¹，不是 +2³¹**，其他位不變。

```
0x7fffffff = 0111 1111 ... 1111 = 2³¹ − 1          = 2147483647  = INT_MAX
0x80000000 = 1000 0000 ... 0000 = −2³¹              = −2147483648 = INT_MIN
0xffffffff = 1111 1111 ... 1111 = −2³¹ + (2³¹ − 1) = −1
```

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

C 的規則（usual arithmetic conversions，一般算術轉換）：**`int` 和 `unsigned int` 放在同一個運算裡，`int` 會先被轉成 `unsigned int`**。
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

## 自我測驗

1. 一個 `int` 變數存著 `-1`，用 `%d` 印出 `-1`，轉成 `unsigned` 再用 `%u` 印出 `4294967295`。記憶體裡的內容有沒有變？為什麼同一個東西會印出兩個不同的數？
2. `int x = 0x12345678; unsigned char *b = (unsigned char *)&x;` 在本機 `*b` 是多少？換到大端序的機器上是多少？
   為什麼 `int *p = &x;` 的 `*p` 在兩種機器上都是 `0x12345678`？
3. `int n = -1; unsigned int len = 5;`，`n < len` 為什麼是假？只加 `-Wall` 編譯，gcc 會不會提醒你？
4. `int y = x + 1; return y > x;` 在 `x = INT_MAX` 時，`-O0` 回傳 0、`-O2` 回傳 1。為什麼會這樣？這算不算 gcc 的 bug？
   kernel 用哪個選項避免這種情況，代價是什麼？
5. 情境題：你從網路封包裡讀到 4 個 byte，依序是 `00 00 01 00`。協定規定數字用大端序，所以它代表 256。
   如果直接用 `int *` 把這 4 個 byte 讀進本機（x86-64），會得到多少？為什麼？
6. `printf("%p")` 印出 `0x7ffe50b23624`，只有 12 位十六進位。指標到底佔幾個 byte？印出來為什麼比較短？
   如果在 kernel 的錯誤訊息裡看到 `0xffff888102a4c000`，從位址的長相你能判斷什麼？
7. `2147483648` 和 `0x80000000` 是同一個數，為什麼一個的型別是 `long`、一個是 `unsigned int`？
   `-1 < 0x80000000` 和 `-1 < 2147483648` 的結果各是什麼？為什麼？
8. `signed char c = -1; unsigned int u = c;` 之後 `u` 是多少？擴展時新補的高位是 0 還是 1，由哪個型別決定？
   如果把 `c` 改宣告成 `unsigned char c = 0xff;`，`u` 又是多少？
9. 情境題：有人寫了 `if (len - used > 0)` 判斷「緩衝區還有沒有剩餘空間」，`len`、`used` 都是 `size_t`，用 `-Wall -Wextra` 編譯沒有任何警告。
   這個判斷在什麼情況下會出錯？為什麼 gcc 對 `len - used < 0` 會警告、對 `> 0` 卻不會？
   另外，為什麼 `int sum = x + y; if (sum < x)` 這種溢位檢查在 `-O2` 下可能完全失效？
