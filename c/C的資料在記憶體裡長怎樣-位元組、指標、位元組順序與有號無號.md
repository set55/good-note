# C 的資料在記憶體裡長怎樣？—— 位元組、指標、位元組順序與有號／無號

> 日期：2026-09-24
> 情境：系統化學習路線第 ① 站第 2 課，對應 CS:APP 第 2 章 2.1～2.3 節（資訊的儲存、整數的表示、整數運算）。
> 第 1 課 [ELF 是什麼](./ELF是什麼-可執行與可連結格式的兩種視角.md#五檔頭裡的其他欄位) 裡 `readelf -h` 的
> `Data: 2's complement, little endian` 這一行，兩個詞都在這一課講
> 適用範圍：實測本機 Ubuntu 24.04、x86-64、gcc 13.3.0、glibc 2.39、gdb 15.1；所有輸出都是本機實際執行結果
> 對應檔案：實驗程式放在 scratchpad，不入 git；作業在 [`small-project/s1-02-data-repr/`](../small-project/s1-02-data-repr/)（會進 git）
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

各型別佔幾個 byte，用 `sizeof` 問編譯器（`sizes.c`，`%zu` 是印 `size_t` 的格式，見第九節）：

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
叫**解參考（dereference）**。實驗程式 `addr.c`：

```c
int x = 0x12345678;
int *p = &x;                              /* p 存著 x 的位址 */
unsigned char *b = (unsigned char *)&x;   /* 同一個位址，但宣告成「指向 1 個 byte」 */

printf("&x = %p\n", (void *)&x);
printf("*p = 0x%x\n", *p);                /* 從那個位址讀 4 個 byte，當成 int */
printf("*b = 0x%02x\n", *b);              /* 從同一個位址只讀 1 個 byte */
*p = 0x11223344;                          /* 透過指標寫回去 */
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
  不寫 cast 的話，gcc 不加任何選項就會警告：`initialization of ‘unsigned char *’ from incompatible pointer type ‘int *’ [-Wincompatible-pointer-types]`。
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

### 位址看起來只有 6 byte？——`%p` 不補零，而且最上面 2 byte 一定是 0

> 2026-09-24 課後提問：「`0x7fffbc2b1274` 只有 12 位十六進位，也就是 6 byte，為什麼說指標是 8 byte？」

指標**確實佔 8 byte**，只是 `%p` 跟 `%x` 一樣不印開頭的 0。強制補滿 16 位就看得出來（`ptr8.c`）：

```
%p      : 0x7fff6a6d956c
%#018lx : 0x00007fff6a6d956c      ← 補滿 16 位十六進位＝8 byte，最上面是 00 00
sizeof(p) = 8
```

（`%#018lx`：旗標 `#` 加 `0x`、旗標 `0` 補零、寬度 18＝`0x` 兩個字元加 16 位數、`l` 表示參數是 `long`；完整文法見第九節。）

用 `gdb` 直接看**指標變數 `p` 自己**佔的那 8 個 byte：

```
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

範圍是 0 到 2³²−1。**超出範圍時，C 標準規定結果是對 2³² 取餘數**，也就是**回繞（wrap around）**，像里程表跑過 99999 回到 00000：

```
UINT_MAX + 1 = 0
0u - 1 = 4294967295
```

（下面 `overflow.c` 的實測輸出。）這是**有定義的行為**，任何最佳化等級、任何平台結果都一樣。

---

## 五、有號整數：二補數

`int` 也是 32 個 bit，但要能表示負數。現代所有 CPU 都用**二補數（two's complement）**，規則只改一個地方：
**最高位（第 31 位）的權重是 −2³¹，不是 +2³¹**，其他位不變。

```
0x7fffffff = 0111 1111 ... 1111 = 2³¹ − 1          = 2147483647  = INT_MAX
0x80000000 = 1000 0000 ... 0000 = −2³¹              = −2147483648 = INT_MIN
0xffffffff = 1111 1111 ... 1111 = −2³¹ + (2³¹ − 1) = −1
```

實測（`signs.c`）：

```
m = -1, as %u = 4294967295, as %x = 0xffffffff, first byte = 0xff
INT_MIN = -2147483648 = 0x80000000
INT_MAX = 2147483647 = 0x7fffffff
```

由規則推出的幾個性質：

- **最高位是 1 就是負數**，所以最高位又叫**符號位（sign bit）**。
- **範圍不對稱**：−2147483648 到 2147483647，負數多一個。`INT_MIN` 取負號沒有對應的正數。
- **取負數的算法：每個 bit 反過來，再加 1**。例：`1` 是 `0x00000001`，反過來是 `0xfffffffe`，加 1 得 `0xffffffff`，就是 −1。
- 加法電路**跟無號完全一樣**：`0xffffffff + 1` 的位元結果是 `0x00000000`，當無號看是「回繞到 0」，當有號看是「−1 + 1 = 0」。
  這是二補數勝出的原因——CPU 不需要兩套加法器。

這就是 `readelf -h` 那一行的前半 `2's complement`：這個檔案裡的有號數用二補數表示。

---

## 六、有號與無號之間的轉換：位元不變，換個讀法

同樣寬度的有號、無號互相轉換時，**位元一個都沒變，只是換了解讀方式**：

```
unsigned int u = -1;           → u = 4294967295         （0xffffffff 當無號讀）
int big = 3000000000u;         → big = -1294967296      （3000000000 = 0xb2d05e00，最高位是 1，當有號讀是負數）
```

**變窄（截斷，truncation）**：只留下低位的 byte，高位直接丟掉。

```
unsigned char c = 300;         → c = 44
```

300 是 `0x12c`，`unsigned char` 只有 8 bit，留下 `0x2c` = 44。gcc 連 `-Wall` 都不用就會警告：

```
signs.c:18:23: warning: unsigned conversion from ‘int’ to ‘unsigned char’ changes value from ‘300’ to ‘44’ [-Woverflow]
```

**變寬（擴展，extension）**：要補上新的高位，補什麼取決於**原本的型別**：

```
(int)(signed char)-1   = -1  = 0xffffffff     ← 符號擴展（sign extension）：用符號位補，負數補 1
(int)(unsigned char)0xff = 255 = 0x000000ff   ← 零擴展（zero extension）：一律補 0
```

兩者原本都是同一個 byte `0xff`，擴展成 `int` 後一個是 −1、一個是 255。這就是第一節說「處理原始位元組用 `unsigned char`」的理由：
用 `char` 讀到 `0xff`，擴展後變成 −1，拿去當陣列索引或跟 255 比較都會出錯。

---

## 七、陷阱：有號和無號混在一起比較

```c
if (-1 < 0u)  ...     /* 結果：假 */

int n = -1;
unsigned int len = 5;
if (n < len)  ...     /* 結果：假 */
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
最後一欄那個迴圈是無號回繞的經典 bug：`i` 從 0 再減 1 變成 4294967295，`i >= 0` 永遠成立，迴圈停不下來（作業第 4 題）。

---

## 八、溢位：無號回繞，有號是未定義行為

### 有號溢位在 C 標準裡是「未定義行為」

無號溢位有規定（回繞）；**有號溢位，C 標準沒有規定結果**，叫**未定義行為（undefined behavior，UB）**。
意思不是「結果不確定」，而是**標準對這支程式完全沒有要求**：編譯器可以**假設它永遠不會發生**，並依這個假設改寫程式。

實驗（`ub.c`）：

```c
int bigger(int x)
{
    int y = x + 1;
    return y > x;      /* x + 1 當然比 x 大……除非溢位 */
}
/* main 裡：printf("bigger(INT_MAX) = %d\n", bigger(INT_MAX)); */
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

**連 `-O0` 也不保證照字面算。** 把同一件事寫成一行 `return x + 1 > x;`，`-O0` 也回傳 1。
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

UBSan 的實測輸出：

```
$ gcc -O0 -fsanitize=undefined ub.c -o ub && ./ub
ub.c:6:9: runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'
bigger(INT_MAX) = 0
```

**UBSan 請搭配 `-O0` 或 `-Og` 用。** 實測 `-Og` 有報錯，但 `-O1`、`-O2` **沒有報錯**：`-fdump-tree-optimized` 顯示
`bigger` 裡確實有檢查（`.UBSAN_CHECK_ADD`），但 `main` 裡的呼叫已經被內嵌、在編譯時算成 `printf(..., 0)`，檢查根本沒機會執行。

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

作業在 [`small-project/s1-02-data-repr/`](../small-project/s1-02-data-repr/README.md)：自己寫程式看位元組、判斷位元組順序、
重現有號無號比較的陷阱與有號溢位的 `-O0`／`-O2` 差異。

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
