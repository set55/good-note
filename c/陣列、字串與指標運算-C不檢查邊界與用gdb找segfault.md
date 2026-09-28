# 陣列、字串與指標運算 —— C 不檢查邊界：越界為什麼有時當掉、有時安靜出錯，怎麼用 gdb 找到它

> 日期：2026-09-27
> 情境：系統化學習路線第 ① 站第 3 課。對應 CS:APP 第 3.8 節（Array Allocation and Access）的 C 語意部分
> （該節的組合語言留到第 4、5 課），以及 3.10.3～3.10.4（越界存取與緩衝區溢位、堆疊保護）
> 適用範圍：實測本機 Ubuntu 24.04、x86-64、gcc 13.3.0、glibc 2.39、gdb 15.1；所有輸出都是本機實際執行結果
> （位址每次執行不同，因為 ASLR；在 gdb 裡跑的位址固定，因為 gdb 預設關掉 ASLR，見[第 2 課第二節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#二指標就是位址)的〈每次執行位址都不一樣〉）
> 對應檔案：每個實驗程式的完整原始碼都寫在它出現的那一節，可以直接複製編譯；
> 作業在 [`small-project/s1-03-arrays-pointers/`](../small-project/s1-03-arrays-pointers/)（會進 git）
> 相關：[C的資料在記憶體裡長怎樣](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md)（第 2 課：位元組、指標、有號無號）、
> [gdb完整命令參考](./gdb完整命令參考.md)（本課用到的 gdb 命令都在那裡）、
> [ELF是什麼](./ELF是什麼-可執行與可連結格式的兩種視角.md)（`.rodata`、載入段與權限）

**一句話結論：陣列是一段連續、同型別的格子；`p + 1` 走的是「一個元素」，不是一個 byte，`p[i]` 就是 `*(p + i)` 的縮寫。
陣列名稱在大多數地方會變成「指向第 0 個元素的指標」，所以傳進函式後就不知道自己多長——長度要另外傳。
字串只是「以 `\0` 結尾的 `char` 陣列」。C 完全不檢查邊界：越界存取是未定義行為，結果可能是**安靜地讀到垃圾、改掉別的資料**、
被**堆疊保護**抓到、或**碰到沒對映的分頁才 segfault**——當掉的那一行通常不是犯錯的那一行。
所以不能等它當掉：用 `gdb` 看「當在哪、當時的值」，用 AddressSanitizer 在**第一次越界的當下**抓到它。**

---

## 目錄

- [零、開課複習：第 2 課卡住的三件事，在本課會再出現](#零開課複習第-2-課卡住的三件事在本課會再出現)
- [一、陣列：一段連續、同型別的格子](#一陣列一段連續同型別的格子)
- [二、指標運算：`p + 1` 走一個元素，不是一個 byte](#二指標運算p--1-走一個元素不是一個-byte)
- [三、陣列名稱什麼時候變成指標、什麼時候不會](#三陣列名稱什麼時候變成指標什麼時候不會)
- [四、字串：以 `\0` 結尾的 `char` 陣列](#四字串以-0-結尾的-char-陣列)
- [五、C 不檢查邊界：越界的三種下場](#五c-不檢查邊界越界的三種下場)
- [六、用 gdb 找 segfault：實際走一次](#六用-gdb-找-segfault實際走一次)
- [七、AddressSanitizer：在第一次越界的當下抓到它](#七addresssanitizer在第一次越界的當下抓到它)
- [八、本課用到的工具與選項](#八本課用到的工具與選項)
- [九、動手作業](#九動手作業)

---

## 零、開課複習：第 2 課卡住的三件事，在本課會再出現

1. **截斷 vs. 同寬換讀法**：同寬的有號↔無號轉換，位元一個都不動，只換讀法（`-1` ↔ `4294967295`）；
   **寬變窄**才是截斷（丟掉高位 byte）。「只換讀法」這句話只在同寬時成立。
2. **常數也有型別**：十進位常數沒有後綴時，依序試 `int`、`long`、`long long`，**沒有無號型別**；
   十六進位／八進位常數會試無號型別。所以 `2147483648` 是 `long`、`0x80000000` 是 `unsigned int`。
   比較時照「一般算術轉換」決定以誰為準（[第 2 課第四節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#四無號整數二進位與回繞)〈怎麼確定一個常數的型別？比較時以誰為準？〉）。
3. **`char` 在 x86-64 上是有號的**：讀到 UTF-8 的 `0xe5` 會變成 −27。**把記憶體當一格一格的 byte 看時，用 `unsigned char`**。

這三件事本課都會用到：第二節用 `unsigned char *` 一格一格走記憶體；`sizeof` 的結果是 `size_t`（無號），
兩個指標相減的結果 `ptrdiff_t` 是**有號**的，兩者混在一起比較又會碰到[第 2 課第七節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#七陷阱有號和無號混在一起比較)的陷阱（本課第三節、作業第 5 題）。

---

## 一、陣列：一段連續、同型別的格子

`int a[5];` 宣告一個**陣列（array）**：5 個 `int` **緊密相連**地放在記憶體裡，中間沒有空隙。
`a[i]` 是第 `i` 個元素，編號從 0 開始，最後一個是 `a[4]`。

`arr.c`：

```c
#include <stdio.h>

int main(void)
{
    int a[5] = {10, 20, 30, 40, 50};

    printf("sizeof a    = %zu\n", sizeof a);
    printf("sizeof a[0] = %zu\n", sizeof a[0]);
    printf("元素個數    = %zu\n", sizeof a / sizeof a[0]);
    for (int i = 0; i < 5; i++)
        printf("a[%d] = %d  位址 %p\n", i, a[i], (void *)&a[i]);
    return 0;
}
```

```
$ gcc -Wall -Wextra -g -O0 -o arr arr.c && ./arr
sizeof a    = 20
sizeof a[0] = 4
元素個數    = 5
a[0] = 10  位址 0x7ffdbf471f30
a[1] = 20  位址 0x7ffdbf471f34
a[2] = 30  位址 0x7ffdbf471f38
a[3] = 40  位址 0x7ffdbf471f3c
a[4] = 50  位址 0x7ffdbf471f40
```

- `sizeof a` 是**整個陣列**的大小：5 × 4 = 20 byte。
- 相鄰元素的位址差 4：正好是一個 `int` 的大小，證明它們緊密相連。
- `sizeof a / sizeof a[0]` 是求元素個數的慣用寫法——**只在 `a` 真的是陣列時有效**（第三節會看到它什麼時候失效）。
- 位址在 `0x7ff…` 開頭：區域變數放在**堆疊（stack）**上（[第 2 課第二節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#二指標就是位址)的〈`0x7fffffffd254` 在哪裡？〉那一段：用 `info proc mappings` 看出位址落在 `[stack]`）。

陣列有兩個性質是後面所有事情的根源：

1. **長度在編譯時就固定**，而且**只存在於型別裡**（`int [5]`），記憶體裡沒有任何一個 byte 記著「這個陣列長 5」。
2. **C 不在執行時檢查 `i` 有沒有超出 0～4**。`a[7]` 照樣會算出一個位址、照樣去讀寫（第五節）。

---

## 二、指標運算：`p + 1` 走一個元素，不是一個 byte

第 2 課講過：指標就是一個位址，**指標的型別決定從那個位址讀幾個 byte、怎麼解讀**。
本課再加一條：**指標的型別也決定 `+ 1` 要走幾個 byte**。

**指標運算（pointer arithmetic）** 的規則：`p` 的型別是 `T *` 時，`p + n` 的位址是 `p` 的位址加上 `n × sizeof(T)` 個 byte。

`ptrarith.c`：

```c
#include <stdio.h>
#include <stddef.h>

int main(void)
{
    int a[4] = {10, 20, 30, 40};
    int *p = a;                 /* 等於 &a[0] */
    unsigned char *c = (unsigned char *)a;
    double d[2] = {1.5, 2.5};
    double *q = d;

    printf("p     = %p   p + 1 = %p\n", (void *)p, (void *)(p + 1));
    printf("c     = %p   c + 1 = %p\n", (void *)c, (void *)(c + 1));
    printf("q     = %p   q + 1 = %p\n", (void *)q, (void *)(q + 1));

    printf("*(p + 2) = %d   p[2] = %d   2[p] = %d\n", *(p + 2), p[2], 2[p]);

    int *first = &a[0];
    int *last = &a[3];
    ptrdiff_t n = last - first;
    ptrdiff_t back = first - last;
    printf("last - first = %td   first - last = %td\n", n, back);
    printf("以 byte 算：%td\n", (unsigned char *)last - (unsigned char *)first);
    printf("sizeof(ptrdiff_t) = %zu\n", sizeof(ptrdiff_t));
    return 0;
}
```

```
$ gcc -Wall -Wextra -g -O0 -o ptrarith ptrarith.c && ./ptrarith
p     = 0x7ffc40825f30   p + 1 = 0x7ffc40825f34
c     = 0x7ffc40825f30   c + 1 = 0x7ffc40825f31
q     = 0x7ffc40825f40   q + 1 = 0x7ffc40825f48
*(p + 2) = 30   p[2] = 30   2[p] = 30
last - first = 3   first - last = -3
以 byte 算：12
sizeof(ptrdiff_t) = 8
```

逐行看：

- **`p` 和 `c` 存的是同一個位址**（都是 `...5f30`），但 `p + 1` 多 4、`c + 1` 多 1、`q + 1` 多 8——各自是 `sizeof(int)`、`sizeof(unsigned char)`、`sizeof(double)`。
  位址本身沒有變，變的是「你用什麼型別看它」。這就是第 2 課「型別決定讀法」的延伸。
- **想一個 byte 一個 byte 地走記憶體，就轉成 `unsigned char *`**（開課複習第 3 點）：它的 `+ 1` 剛好是 1 byte，讀出來也不會變負數。

### `p[i]` 就是 `*(p + i)`

C 標準對下標運算的**定義**就是：`E1[E2]` 等於 `*((E1) + (E2))`。所以：

- `p[2]` = `*(p + 2)` = 「從 `p` 往後走 2 個元素，讀那一格」。
- 加法可以交換，`*(p + 2)` = `*(2 + p)`，所以 `2[p]` 也合法而且是同一個值（輸出第 4 行）。
  沒有人會這樣寫程式；它的價值是證明 **`[]` 不是陣列專屬的語法，只是指標加法再取值的縮寫**。
- 因此 `a[i]` 也是 `*(a + i)`——這裡 `a` 先變成指向 `a[0]` 的指標（第三節）。

### 兩個指標相減：得到「差幾個元素」，型別是 `ptrdiff_t`

- 指向**同一個陣列**的兩個指標可以相減，結果是**中間差幾個元素**，不是差幾個 byte：`last - first` 是 3。
  轉成 `unsigned char *` 再減，才是 byte 數 12（= 3 × 4）。
- 結果的型別是 `<stddef.h>` 裡的 **`ptrdiff_t`**（pointer difference type）：本機是 8 byte 的**有號**整數（`long`），
  因為相減可以是負的（`first - last` 是 −3）。`printf` 用 **`%td`** 印（`t` 是 `ptrdiff_t` 的長度修飾字，見[第 2 課第九節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#printf-的格式完整文法)〈`printf` 的格式：完整文法〉）。
- 對照 `sizeof` 的結果 `size_t`：**無號**，用 `%zu`。一個有號一個無號，混在一起比較就會踩到[第 2 課第七節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#七陷阱有號和無號混在一起比較)的陷阱。
- 兩個指標**相加沒有意義**，C 不允許（`p + q` 編譯錯誤）；指向**不同陣列**的指標相減是未定義行為。

---

## 三、陣列名稱什麼時候變成指標、什麼時候不會

`int a[5];` 的 `a` 在**大多數**運算式裡，會自動轉成「指向 `a[0]` 的指標」（型別 `int *`）。
這個轉換叫做**陣列退化成指標（array-to-pointer decay）**。例外只有幾個，最重要的是：

- `sizeof a`：得到整個陣列的大小，不退化。
- `&a`：得到「指向整個陣列」的指標，型別是 `int (*)[5]`，不退化。
- 用字串常數初始化 `char` 陣列（`char s[] = "hi";`，第四節）。

`decay.c`：

```c
#include <stdio.h>

void show(int a[])
{
    printf("函式裡 sizeof a = %zu\n", sizeof a);
}

int main(void)
{
    int a[5] = {0};
    int *p = a;

    printf("main 裡 sizeof a = %zu, sizeof p = %zu\n", sizeof a, sizeof p);
    show(a);
    printf("a      = %p   a + 1  = %p\n", (void *)a, (void *)(a + 1));
    printf("&a     = %p   &a + 1 = %p\n", (void *)&a, (void *)(&a + 1));
    return 0;
}
```

```
$ gcc -Wall -Wextra -g -O0 -o decay decay.c
decay.c: In function ‘show’:
decay.c:5:46: warning: ‘sizeof’ on array function parameter ‘a’ will return size of ‘int *’ [-Wsizeof-array-argument]
    5 |     printf("函式裡 sizeof a = %zu\n", sizeof a);
      |                                              ^
decay.c:3:15: note: declared here
    3 | void show(int a[])
      |           ~~~~^~~~~
$ ./decay
main 裡 sizeof a = 20, sizeof p = 8
函式裡 sizeof a = 8
a      = 0x7fffd4ec3200   a + 1  = 0x7fffd4ec3204
&a     = 0x7fffd4ec3200   &a + 1 = 0x7fffd4ec3214
```

### 函式參數寫成 `int a[]`，其實就是 `int *a`

- `main` 裡 `sizeof a` 是 20（陣列），`sizeof p` 是 8（指標）。
- 傳進 `show` 之後，`sizeof a` 變成 8：**函式參數宣告成 `int a[]`（甚至 `int a[5]`），編譯器一律把它當成 `int *a`**。
  呼叫 `show(a)` 時，`a` 已經退化成指標，傳過去的只有一個 8 byte 的位址，**陣列的長度沒有跟著傳過去**。
- gcc 知道這很容易誤會，所以 `-Wsizeof-array-argument`（預設就開）直接警告「這會回傳 `int *` 的大小」。

**結論：在 C 裡，把陣列交給函式時，長度一定要另外傳**（例如 `long sum(int *p, int n)`，第五、六節）。
C 標準函式庫也是這樣設計的：`memcpy(dst, src, n)`、`read(fd, buf, count)` 都帶長度。字串是唯一的例外——它靠結尾的 `\0` 知道自己在哪結束（第四節）。

### `a` 和 `&a`：位址一樣，型別不一樣

輸出最後兩行：`a` 和 `&a` 印出**同一個位址** `...3200`，但 `a + 1` 多 4、`&a + 1` 多 **20**（`0x14`）。
套用第二節的規則就懂了：`a` 退化成 `int *`，`+ 1` 走一個 `int`；`&a` 的型別是 `int (*)[5]`（指向「5 個 `int` 的陣列」），
`+ 1` 走一整個陣列 = 20 byte。**又一次：位址相同，型別決定怎麼走。**

### 算元素個數再跑迴圈：又碰到有號無號（[第 2 課第七節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#七陷阱有號和無號混在一起比較)）

`count.c`：

```c
#include <stdio.h>

int main(void)
{
    int a[5] = {1, 2, 3, 4, 5};
    int total = 0;

    for (int i = 0; i < sizeof a / sizeof a[0]; i++)   /* 第 8 行 */
        total += a[i];
    printf("total = %d\n", total);
    return 0;
}
```

```
$ gcc -Wall -o count count.c && echo "(-Wall 沒有警告)"
(-Wall 沒有警告)
$ gcc -Wall -Wextra -o count count.c
count.c: In function ‘main’:
count.c:8:23: warning: comparison of integer expressions of different signedness: ‘int’ and ‘long unsigned int’ [-Wsign-compare]
    8 |     for (int i = 0; i < sizeof a / sizeof a[0]; i++)   /* 第 8 行 */
      |                       ^
$ ./count
total = 15
```

`sizeof a / sizeof a[0]` 的型別是 `size_t`（`unsigned long`），`i` 是 `int`，比較時 `i` 被轉成 `unsigned long`。
這裡 `i` 從 0 開始、不會是負的，所以結果正確；但同樣的寫法只要 `i` 可能是負數就會出錯（[第 2 課第七節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#七陷阱有號和無號混在一起比較)）。
常見的改法是把 `i` 宣告成 `size_t`——但往回數的迴圈改成 `size_t` 又有另一個陷阱（作業第 5 題）。

---

## 四、字串：以 `\0` 結尾的 `char` 陣列

C 沒有「字串型別」。**字串（string）** 是一串 `char`，最後接一個值為 0 的 byte（`'\0'`，稱為**結尾空字元（null terminator）**）。
`"hi"` 在記憶體裡是 3 個 byte：`'h'`、`'i'`、`'\0'`（〈字元是什麼〉第三節的 `sizeof("A")` 是 2 就是這個原因）。
`strlen` 從開頭一個一個數，數到 `\0` 停，**不算 `\0` 本身**。它沒有其他辦法知道字串多長。

同樣寫 `"hi"`，放進陣列和交給指標是兩回事。`strings.c`：

```c
#include <stdio.h>
#include <string.h>

int main(void)
{
    char s1[] = "hi";       /* 陣列：在堆疊上有自己的 3 個 byte */
    char *s2 = "hi";        /* 指標：指向字串常數 */

    printf("sizeof s1 = %zu, strlen(s1) = %zu\n", sizeof s1, strlen(s1));
    printf("sizeof s2 = %zu, strlen(s2) = %zu\n", sizeof s2, strlen(s2));
    printf("s1 在 %p\n", (void *)s1);
    printf("s2 指向 %p\n", (void *)s2);

    s1[0] = 'H';
    printf("改完 s1 = %s\n", s1);

    s2[0] = 'H';            /* 第 17 行：寫進唯讀記憶體 */
    printf("改完 s2 = %s\n", s2);
    return 0;
}
```

```
$ gcc -Wall -Wextra -g -O0 -o strings strings.c
$ ./strings; echo "exit=$?"
sizeof s1 = 3, strlen(s1) = 2
sizeof s2 = 8, strlen(s2) = 2
s1 在 0x7ffd6d4f7645
s2 指向 0x579d88111008
改完 s1 = Hi
exit=139
```

（互動式的 shell 還會另外印一行 segmentation fault 的訊息，字樣依 shell 而定；這裡用 `echo exit=$?` 看結束狀態，139 的意思見第五節。）

- **`char s1[] = "hi";`**：宣告一個 3 byte 的陣列，把 `h`、`i`、`\0` **複製**進去。`sizeof s1` 是 3（陣列大小，含 `\0`），
  位址在 `0x7ff…`（堆疊上）。它是你自己的記憶體，可以改。
- **`char *s2 = "hi";`**：`s2` 只是一個 8 byte 的指標，指向編譯器放在**執行檔裡**的字串常數（string literal）。
  `sizeof s2` 是 8（指標大小），位址在 `0x5…`（執行檔被載入的區域）。改它就當掉。

字串常數放在哪裡？用 `readelf -p`（把一個區段當字串印出來，見 [readelf完整選項參考](./readelf完整選項參考.md)）看 `.rodata`（read-only data，唯讀資料）區段：

```
$ readelf -p .rodata strings

String dump of section '.rodata':
  [     8]  hi
  [    10]  sizeof s1 = %zu, strlen(s1) = %zu\n
  [    38]  sizeof s2 = %zu, strlen(s2) = %zu\n
  ...
```

（後面幾行含中文的格式字串，`readelf` 把非 ASCII 的 byte 印成亂碼，省略。）`"hi"` 在 `.rodata` 的偏移 8——
`s2` 指向的 `0x...008` 結尾正是它。`printf` 的格式字串也全都在這裡。

在 gdb 裡跑，當掉時看它指向的那一頁的權限：

```
$ gdb -q ./strings
(gdb) run
Program received signal SIGSEGV, Segmentation fault.
0x0000555555555262 in main () at strings.c:17
17	    s2[0] = 'H';            /* 第 17 行：寫進唯讀記憶體 */
(gdb) print s2
$1 = 0x555555556008 "hi"
(gdb) info proc mappings
          Start Addr           End Addr       Size     Offset  Perms  objfile
      0x555555554000     0x555555555000     0x1000        0x0  r--p   /…/strings
      0x555555555000     0x555555556000     0x1000     0x1000  r-xp   /…/strings
      0x555555556000     0x555555557000     0x1000     0x2000  r--p   /…/strings
      ...
      0x7ffffffdd000     0x7ffffffff000    0x22000        0x0  rw-p   [stack]
```

`0x555555556008` 落在 `0x555555556000`～`0x555555557000` 這一頁，權限是 **`r--p`：可讀、不可寫、不可執行**。
這就是 [ELF是什麼](./ELF是什麼-可執行與可連結格式的兩種視角.md) 講的：連結器把權限相同的區段打包成載入段，核心**以分頁為單位**設定權限。
對唯讀的分頁寫入，CPU 會拒絕，核心就送出 SIGSEGV（第五節）。**C 標準本身只說「修改字串常數是未定義行為」；
在 Linux／gcc 上，這個未定義行為的實際表現就是 segfault。**

**所以：要修改的字串，用 `char s[] = "..."`（有自己的副本）；只讀不改的，用指標也可以。**

> 補充一個 I/O 的陷阱（第 ⑤ 站會正式講）：上面的輸出是在終端機上跑的。如果把輸出導向檔案或管線（`./strings > out.txt`），
> `printf` 會先把字存在程式自己的緩衝區裡，還沒寫出去程式就當掉，結果**什麼都看不到**。當掉的程式印得少，不代表它當在比較前面。

---

## 五、C 不檢查邊界：越界的三種下場

**越界存取（out-of-bounds access）**：讀寫陣列範圍以外的位置，例如 `int a[4]` 的 `a[4]`、`a[-1]`。
C 標準規定這是**未定義行為（UB，Undefined Behavior）**（[第 2 課第八節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#八溢位無號回繞有號是未定義行為)）：標準不規定會發生什麼，編譯器也不必檢查。
實際上 `a[4]` 就是 `*(a + 4)`——算出一個位址，照樣去讀寫。會發生什麼事，取決於**那個位址上剛好有什麼**：

| 下場 | 什麼時候 | 本節的例子 |
|---|---|---|
| **安靜地出錯** | 那個位址還在程式可以存取的記憶體裡（例如堆疊上的其他資料） | `oob.c`：讀到垃圾、寫壞別的東西，程式照常結束 |
| **被保護機制抓到** | 剛好蓋到編譯器放的「檢查值」 | `smash.c`：`*** stack smashing detected ***` |
| **segfault** | 那個位址所在的分頁**沒有對映**，或權限不允許這種存取 | `segv.c`（第六節）、`strings.c`（第四節） |

### 下場一：安靜地出錯

`oob.c`：

```c
#include <stdio.h>

int main(void)
{
    int a[4] = {1, 2, 3, 4};

    /* 讀：a[4] 已經超出陣列 */
    printf("a[4] = %d\n", a[4]);            /* 第 8 行 */

    /* 寫：i <= 4 多寫了一格 */
    for (int i = 0; i <= 4; i++)
        a[i] = 99;                          /* 第 12 行 */

    printf("寫完了，a[0] = %d\n", a[0]);
    return 0;
}
```

```
$ gcc -Wall -Wextra -g -O0 -o oob oob.c
$ ./oob; echo "exit=$?"
a[4] = -83499216
寫完了，a[0] = 99
exit=0
```

- `-Wall -Wextra` 在 `-O0` **一個警告都沒有**。
- `a[4]` 讀到一個垃圾值（每次執行都不同），多寫的那一格寫到了堆疊上 `a` 後面的某個東西，**程式照常結束，結束狀態 0**。
- 這是最危險的一種：錯了卻沒有任何跡象。那個被蓋掉的東西如果是另一個變數，錯誤會在很久以後、很遠的地方才出現。

改用 `-O2` 編譯，gcc 反而會警告：

```
$ gcc -Wall -Wextra -g -O2 -o oob2 oob.c
oob.c: In function ‘main’:
oob.c:12:14: warning: iteration 4 invokes undefined behavior [-Waggressive-loop-optimizations]
   12 |         a[i] = 99;                          /* 第 12 行 */
      |         ~~~~~^~~~
oob.c:11:23: note: within this loop
   11 |     for (int i = 0; i <= 4; i++)
      |                     ~~^~~~
oob.c:8:5: warning: array subscript 4 is above array bounds of ‘int[4]’ [-Warray-bounds=]
    8 |     printf("a[4] = %d\n", a[4]);            /* 第 8 行 */
      |     ^~~~~~~~~~~~~~~~~~~~~~~~~~~
oob.c:5:9: note: while referencing ‘a’
    5 |     int a[4] = {1, 2, 3, 4};
      |         ^
oob.c:12:10: warning: array subscript 4 is above array bounds of ‘int[4]’ [-Warray-bounds=]
   12 |         a[i] = 99;                          /* 第 12 行 */
      |         ~^~~
oob.c:5:9: note: while referencing ‘a’
    5 |     int a[4] = {1, 2, 3, 4};
      |         ^
$ ./oob2; echo "exit=$?"
a[4] = 1353146480
寫完了，a[0] = 99
exit=0
```

- `-Warray-bounds` 和 `-Waggressive-loop-optimizations` 是**最佳化過程順便做的分析**：`-O0` 不做這些分析，所以不會警告
  （`-Warray-bounds` 其實已經被 `-Wall` 打開了，只是沒有分析就沒有東西可報）。
- 第一個警告直接說出第 2 課的觀念：「第 4 圈會觸發未定義行為」——gcc 知道 `a[4]` 是 UB，就可以假設迴圈跑不到第 4 圈，並依此最佳化。
- **這些警告只抓得到編譯時看得出來的越界**（索引是常數或迴圈範圍固定）。索引來自使用者輸入、函式參數時，編譯器什麼都看不出來（第六節的 `segv.c` 就沒有任何警告）。

### 下場二：被堆疊保護抓到

`smash.c`：往 8 byte 的 `char` 陣列寫 32 個 `'A'`：

```c
#include <stdio.h>

int main(void)
{
    char buf[8];
    int i;

    printf("buf 在 %p，i 在 %p\n", (void *)buf, (void *)&i);

    /* 往 8 byte 的陣列寫 32 個 byte */
    for (i = 0; i < 32; i++)
        buf[i] = 'A';                       /* 第 12 行 */

    printf("迴圈結束時 i = %d\n", i);
    return 0;                               /* 第 15 行 */
}                                           /* 第 16 行 */
```

```
$ gcc -Wall -Wextra -g -O0 -o smash smash.c
$ ./smash; echo "exit=$?"
buf 在 0x7fffc9f4b330，i 在 0x7fffc9f4b32c
迴圈結束時 i = 32
*** stack smashing detected ***: terminated
exit=134
```

- 迴圈**完整跑完**、`printf` 也印了——越界寫入的當下沒有人發現。
- 到 `main` 要**結束時**，才出現 `*** stack smashing detected ***`，程式被終止。

這是 gcc 的**堆疊保護（stack protector）**：編譯器在函式的區域陣列**後面**（較高位址）放一個隨機的**檢查值（canary，金絲雀值**，
取名自礦坑裡用來提早發現毒氣的金絲雀），函式**返回前**檢查它有沒有被改掉；被改了就呼叫 glibc 的 `__stack_chk_fail`，印出這行訊息並中止程式。
Ubuntu 的 gcc **預設**就開了 `-fstack-protector-strong`（本機實測）：

```
$ gcc -Q --help=common | grep fstack-protector
  -fstack-protector           		[disabled]
  -fstack-protector-all       		[disabled]
  -fstack-protector-explicit  		[disabled]
  -fstack-protector-strong    		[enabled]
```

它只在**函式返回時**檢查一次，所以：抓到的時間點晚於犯錯的時間點；而且只抓得到「一路寫到檢查值」的越界——只多寫 1 格、剛好沒碰到檢查值（像 `oob.c`）就抓不到。

**為什麼 `i` 沒被蓋掉？** 輸出第一行：`i` 的位址比 `buf` **低** 4 byte，而越界是往高位址寫，所以沒碰到 `i`。
這是 gcc 在這次編譯的排法，**不是保證**；換個編譯器或選項，被蓋掉的可能就是 `i`，迴圈就會出現莫名其妙的行為。

### 關掉保護，看看下面是什麼：返回時 segfault

```
$ gcc -Wall -Wextra -g -O0 -fno-stack-protector -o smash_np smash.c
$ ./smash_np; echo "exit=$?"
buf 在 0x7ffe2ea6f4e8，i 在 0x7ffe2ea6f4e4
迴圈結束時 i = 32
exit=139
```

一樣跑完迴圈、印完字，然後以 139 結束（segfault）。用 gdb 看當在哪：

```
$ gdb -q ./smash_np
(gdb) run
Program received signal SIGSEGV, Segmentation fault.
0x00005555555551b7 in main () at smash.c:16
16	}                                           /* 第 16 行 */
```

**當在第 16 行 `}`——函式結束的地方，不是寫壞東西的第 12 行。** 在第 15 行停下來，看被蓋掉的是什麼：

```
(gdb) break 15
(gdb) run
Breakpoint 1, main () at smash.c:15
15	    return 0;                               /* 第 15 行 */
(gdb) print &buf
$1 = (char (*)[8]) 0x7fffffffd308
(gdb) x/32xb buf
0x7fffffffd308:	0x41	0x41	0x41	0x41	0x41	0x41	0x41	0x41
0x7fffffffd310:	0x41	0x41	0x41	0x41	0x41	0x41	0x41	0x41
0x7fffffffd318:	0x41	0x41	0x41	0x41	0x41	0x41	0x41	0x41
0x7fffffffd320:	0x41	0x41	0x41	0x41	0x41	0x41	0x41	0x41
(gdb) info frame
Stack level 0, frame at 0x7fffffffd320:
 rip = 0x5555555551b1 in main (smash.c:15); saved rip = 0x4141414141414141
 ...
 Saved registers:
  rbp at 0x7fffffffd310, rip at 0x7fffffffd318
```

- `print &buf` 的型別是 `char (*)[8]`——第三節的「指向整個陣列的指標」。
- `x/32xb` 看到從 `buf` 開始 32 個 byte 全是 `0x41`（`'A'` 的 ASCII 碼）。
- `info frame` 說：**「`saved rip` 存在 `0x7fffffffd318`，值是 `0x4141414141414141`」**。
  用中文說：一個函式結束時，要跳回「呼叫它的那個地方」繼續執行，**那個返回位址就存在堆疊上**，就在區域變數的上面（較高位址）。
  這裡它在 `buf` 往後 16 byte 的地方，被 `'A'` 蓋成了 `0x4141414141414141`。
  （`rip` 是 CPU 裡記錄「下一個要執行的指令在哪」的暫存器；`saved rip` 就是存起來、等返回時放回去的那個值。暫存器與堆疊框是第 4、5 課的內容，這裡只要知道「返回位址存在堆疊上」。）
- `main` 在第 16 行返回，CPU 跳到 `0x4141414141414141`——這個位址連 canonical 都不是（[第 2 課第二節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#二指標就是位址)〈位址看起來只有 6 byte？〉：最上面 16 bit 必須全 0 或全 1），
  更不可能有對映，於是 segfault。

這就是**緩衝區溢位（buffer overflow）**：往陣列寫超過它的大小，蓋掉旁邊的資料。蓋到返回位址時，
如果寫進去的不是 `'A'` 而是精心挑選的位址，程式就會跳去攻擊者想要的地方——這是歷史上最經典的漏洞類型，也是堆疊保護存在的原因。

### 結束狀態 139 和 134 是什麼

程式被**訊號（signal）** 終止時，shell 把結束狀態設成 **128 + 訊號編號**：

| 結束狀態 | 訊號 | 誰送的、為什麼 |
|---|---|---|
| 139 = 128 + 11 | `SIGSEGV`（segmentation violation，區段違規） | 核心：程式存取了沒對映、或權限不允許的位址 |
| 134 = 128 + 6 | `SIGABRT`（abort，中止） | 程式自己：`__stack_chk_fail` 發現檢查值被改，呼叫 `abort()` |

用 `kill -l <編號>` 可以把編號換成名字（本機 zsh：`kill -l 11` 印 `SEGV`；`kill -l 6` 印 `IOT`，是 `ABRT` 的舊別名）。
訊號是第 ② 站（行程）的主題，這裡只需要知道：**segfault 不是 C 語言的機制，是 CPU 與核心的機制**——
CPU 查分頁表發現這個位址不能這樣存取，交給核心處理，核心對這個行程送出 `SIGSEGV`，預設動作是終止行程。

---

## 六、用 gdb 找 segfault：實際走一次

最常見的 segfault 情境：函式收到的長度是錯的。`segv.c`：

```c
#include <stdio.h>

long sum(int *p, int n)
{
    long s = 0;
    for (int i = 0; i < n; i++)
        s += p[i];              /* 第 7 行 */
    return s;
}

int main(void)
{
    int a[10];
    for (int i = 0; i < 10; i++)
        a[i] = i;

    printf("sum(a, 10) = %ld\n", sum(a, 10));
    printf("sum(a, 1000000) = %ld\n", sum(a, 1000000));   /* 第 18 行：長度傳錯 */
    return 0;
}
```

```
$ gcc -Wall -Wextra -g -O0 -o segv segv.c        ← 沒有任何警告
$ ./segv; echo "exit=$?"
sum(a, 10) = 45
exit=139
```

只知道「當了」，不知道當在哪。用 gdb 一步一步問：

**第 1 步：跑起來，讓它當在 gdb 裡。**

```
$ gdb -q ./segv
(gdb) run
Program received signal SIGSEGV, Segmentation fault.
0x000055555555519d in sum (p=0x7fffffffd2f0, n=1000000) at segv.c:7
7	        s += p[i];              /* 第 7 行 */
```

gdb 攔下了 `SIGSEGV`，程式停在出事的那一刻：`sum` 函式、第 7 行，參數 `n=1000000`。

**第 2 步：`backtrace`（`bt`）——誰呼叫了誰。**

```
(gdb) bt
#0  0x000055555555519d in sum (p=0x7fffffffd2f0, n=1000000) at segv.c:7
#1  0x000055555555522a in main () at segv.c:18
```

`#0` 是當掉的那一層，`#1` 是呼叫它的：`main` 的第 18 行。**錯誤的長度就是從那裡來的。**

**第 3 步：看當時的值。**

```
(gdb) print i
$1 = 1860
(gdb) print &p[i]
$2 = (int *) 0x7ffffffff000
(gdb) print &p[i] - p
$3 = 1860
(gdb) x/4xb &p[i]
0x7ffffffff000:	Cannot access memory at address 0x7ffffffff000
```

- 已經讀到第 1860 個元素——陣列只有 10 個，**前面 1850 次越界讀取都沒有當掉**，只是安靜地讀了垃圾（下場一）。
- 要讀的位址是 `0x7ffffffff000`，gdb 自己也讀不到（`Cannot access memory`）。
- `print` 裡可以寫 C 的指標運算：`&p[i] - p` 得到 1860，跟第二節「相減得到元素個數」一致。

**第 4 步：這個位址在哪？**

```
(gdb) info proc mappings
          Start Addr           End Addr       Size     Offset  Perms  objfile
      ...
      0x7ffffffdd000     0x7ffffffff000    0x22000        0x0  rw-p   [stack]
  0xffffffffff600000 0xffffffffff601000     0x1000        0x0  --xp   [vsyscall]
```

`[stack]` 的結尾（End Addr，不含）正好是 `0x7ffffffff000`。陣列 `a` 在堆疊上，一路往高位址讀，
**讀到堆疊區域的盡頭、踏進沒有對映的分頁的那一刻，才 segfault**。
驗算：`p` 是 `0x7fffffffd2f0`，1860 × 4 = 7440 = `0x1d10`，`0x7fffffffd2f0 + 0x1d10 = 0x7ffffffff000`。

**第 5 步：切到呼叫者那一層看。**

```
(gdb) ptype p
type = int *
(gdb) print *p@10
$4 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
(gdb) frame 1
#1  0x000055555555522a in main () at segv.c:18
18	    printf("sum(a, 1000000) = %ld\n", sum(a, 1000000));   /* 第 18 行：長度傳錯 */
(gdb) ptype a
type = int [10]
(gdb) print a
$5 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
(gdb) print sizeof a
$6 = 40
```

- 在 `sum` 裡，`p` 的型別是 `int *`：`print p` 只會印出位址，**gdb 也不知道它指向幾個元素**（第三節：長度沒有跟著傳進來）。
  要看內容，用 gdb 的 **`@` 運算子**：`*p@10` 的意思是「從 `*p` 開始，把連續 10 個同型別的東西當成陣列印出來」。
  `@` 是 gdb 自己的語法，不是 C（`help print` 有說明）。
- `frame 1` 切到 `main` 那一層（程式本身沒有動），在那裡 `a` 還是 `int [10]`，`print a` 直接印出整個陣列，`sizeof a` 是 40。

### 當掉的那一行 ≠ 犯錯的那一行

| 例子 | 當在 | 犯錯在 |
|---|---|---|
| `segv.c` | 第 7 行 `s += p[i]`（`sum` 裡） | 第 18 行：傳了錯的長度（`main` 裡） |
| `smash.c`（關掉保護） | 第 16 行 `}` | 第 12 行：寫超過 8 byte |
| `smash.c`（有保護） | `main` 返回時 | 第 12 行 |

segfault 發生在**第一次碰到不能碰的分頁**的時候，那往往離真正的錯誤很遠。所以 gdb 的用法是：
`bt` 找出當掉的位置**和一路呼叫過來的路徑**，在每一層用 `print` 看參數與變數，**往回推「這個錯的值是從哪裡來的」**。

---

## 七、AddressSanitizer：在第一次越界的當下抓到它

第五節的三種下場有個共同問題：**越界的當下沒有人發現**。
**AddressSanitizer（ASan，位址消毒器）** 是 gcc／clang 內建的工具：加 `-fsanitize=address` 編譯，編譯器會在**每一次**讀寫記憶體前插入檢查，
並在每個陣列的前後放一段**禁區（redzone）**。一碰到禁區就立刻停下來，報告**哪一行、讀還是寫、幾個 byte、越過了哪個變數**。

它跟第 2 課的 UBSan（`-fsanitize=undefined`）是同一家族：UBSan 抓有號溢位這類 UB，ASan 抓記憶體存取錯誤。

拿第五節的 `oob.c` 來試（原始碼同上，不用改）：

```
$ gcc -Wall -Wextra -g -O0 -fsanitize=address -o oob_asan oob.c
$ ./oob_asan; echo "exit=$?"
=================================================================
==118093==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x701fac900030 at pc 0x5660c03173f9 bp 0x7ffd5c286a00 sp 0x7ffd5c2869f0
READ of size 4 at 0x701fac900030 thread T0
    #0 0x5660c03173f8 in main /…/oob.c:8
    #1 0x701faea2a1c9 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
    #2 0x701faea2a28a in __libc_start_main_impl ../csu/libc-start.c:360
    #3 0x5660c0317184 in _start (/…/oob_asan+0x1184) (BuildId: 21d3ca8f73b8fb972ee2af64ced150aab2712db2)

Address 0x701fac900030 is located in stack of thread T0 at offset 48 in frame
    #0 0x5660c0317258 in main /…/oob.c:4

  This frame has 1 object(s):
    [32, 48) 'a' (line 5) <== Memory access at offset 48 overflows this variable
HINT: this may be a false positive if your program uses some custom stack unwind mechanism, swapcontext or vfork
      (longjmp and C++ exceptions *are* supported)
SUMMARY: AddressSanitizer: stack-buffer-overflow /…/oob.c:8 in main
Shadow bytes around the buggy address:
  ...
=>0x701fac900000: f1 f1 f1 f1 00 00[f3]f3 00 00 00 00 00 00 00 00
  ...
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  ...（共 18 種標記，省略其餘）
==118093==ABORTING
exit=1
```

（路徑以 `/…/` 省略，`Shadow bytes` 上下各 5 行全 `00` 的列省略。）逐段讀：

| 報告的哪裡 | 意思 |
|---|---|
| `stack-buffer-overflow` | 錯誤種類：堆疊上的陣列溢位（其他種類如 `heap-buffer-overflow`、`stack-buffer-underflow`） |
| `READ of size 4` | **讀**了 **4 byte**（一個 `int`）；寫入會是 `WRITE` |
| `#0 ... in main /…/oob.c:8` | 發生在 `oob.c` **第 8 行**——就是 `a[4]` 那一行，不是之後某個地方 |
| `[32, 48) 'a' (line 5) <== Memory access at offset 48 overflows this variable` | 這個堆疊框裡有一個變數 `a`（第 5 行宣告），佔偏移 32～47；存取的是偏移 48——**剛好超出 `a` 的結尾** |
| `Shadow bytes` 那一列 | ASan 用 1 個「影子 byte」記錄每 8 個 byte 能不能碰：`00` 可以、`f1`／`f3` 是陣列左右的禁區；`[f3]` 是這次碰到的那一格 |
| `ABORTING`、`exit=1` | **第一個錯誤就停**，後面第 12 行的越界寫入沒有機會報告 |

再拿 `smash.c` 試——剛才堆疊保護要等到返回時才抓到，ASan 在第一次越界時就抓到：

```
$ gcc -g -O0 -fsanitize=address -o smash_asan smash.c
$ ./smash_asan
...
WRITE of size 1 at 0x7236a2a00048 thread T0
    #0 0x576bbeb843c6 in main smash.c:12
  ...
  This frame has 2 object(s):
    [48, 52) 'i' (line 6)
    [64, 72) 'buf' (line 5) <== Memory access at offset 72 overflows this variable
SUMMARY: AddressSanitizer: stack-buffer-overflow smash.c:12 in main
```

`WRITE of size 1`（寫 1 個 `char`）、**第 12 行**、越過 `buf` 的結尾——正是犯錯的那一行。

**ASan 的限制與用法：**

- 要**重新編譯**才有效，而且要加 `-g` 才有行號。報告是在執行時產生的：**沒被執行到的程式碼路徑，它抓不到**。
- 會讓程式變慢、用更多記憶體（官方文件說典型的減速約 2 倍，本機未量測），所以**用在開發與測試，不用在正式版**。
- 它的執行時行為可以用環境變數 `ASAN_OPTIONS` 調整。本機 gcc 13.3 的 ASan 用 `ASAN_OPTIONS=help=1 ./程式` 列出 **132 個**選項；
  本課**全部用預設值，一個都沒用到**，所以不逐項列，需要時用那個指令查。
- kernel 也有同樣的東西：**KASAN（Kernel Address Sanitizer）**，是 kernel 開發時找記憶體錯誤的主要工具（第二段 K0 以後會用到）。

---

## 八、本課用到的工具與選項

### `gcc`：本課新用到的選項

`gcc` 的選項有上千個，**這裡只列本課新用到的 7 個**（`-Wall`、`-Wextra`、`-g`、`-O0`、`-O2`、`-Wsign-compare` 見前兩課），
完整清單見 `man gcc`（警告在 Warning Options、`-fsanitize` 與 `-fstack-protector*` 在 Instrumentation Options）。
警告全部 427 個的查法見 [第 2 課第九節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#九本課用到的工具與選項)。

| 選項 | 作用 | 本機狀態 |
|---|---|---|
| `-Wsizeof-array-argument` | 對「宣告成陣列的函式參數」用 `sizeof` 時警告 | 預設開 |
| `-Warray-bounds` | 編譯時看得出來的越界警告；**需要最佳化分析（`-O2`）才報得出來** | `-Wall` 開（等級 1） |
| `-Waggressive-loop-optimizations` | 迴圈某一圈確定會觸發 UB 時警告（同樣要 `-O2`） | 預設開 |
| `-fstack-protector-strong` | 對含陣列等的函式插入檢查值，返回時檢查 | **Ubuntu 預設開** |
| `-fno-stack-protector` | 關掉堆疊保護（本課只為了示範） | — |
| `-fsanitize=address` | AddressSanitizer：每次記憶體存取都檢查，越界立刻報告 | — |
| `-Q --help=common` | 列出通用選項與目前狀態（本課用來確認 `-fstack-protector-strong` 預設開啟） | — |

### `gdb`：本課用到的命令

**這是全部 173 個頂層命令裡的 9 個**（加上 `info` 的 65 個子命令裡的 2 個），完整清單與每個命令的說明見
[gdb完整命令參考](./gdb完整命令參考.md) 第四、五節。

| 命令 | 本課怎麼用 | 參考 |
|---|---|---|
| `run` | 開始執行，當掉時 gdb 會攔下訊號、停在那一行 | 4.1 running |
| `break 15` | 在第 15 行設中斷點 | 4.2 breakpoints |
| `backtrace`（`bt`） | 列出呼叫路徑：`#0` 是目前這層，`#1` 是呼叫它的 | 4.4 stack |
| `frame 1` | 切換到 `#1` 那一層，才看得到那層的區域變數 | 4.4 stack |
| `print`（`p`） | 印出 C 表達式的值，可以寫 `&p[i] - p`、`sizeof a`；`*p@10` 是 gdb 的「連續 10 個」語法 | 4.3 data、第三節〈表達式〉 |
| `ptype` | 印出型別：`int *` vs `int [10]` | 4.3 data |
| `x/32xb 位址` | 從位址開始看 32 個 byte、十六進位 | 4.3 data、第三節〈`/FMT`〉 |
| `info proc mappings` | 行程的記憶體對映：每一段的位址範圍與權限（`r`／`w`／`x`） | 第五節 `info` |
| `info frame` | 目前堆疊框的詳細資訊，本課只看 `saved rip`（返回位址存在哪、值是多少） | 第五節 `info` |

### 其他

| 工具 | 本課用法 | 完整參考 |
|---|---|---|
| `readelf -p .rodata` | 把 `.rodata` 區段當字串印出來，看字串常數在哪 | [readelf完整選項參考](./readelf完整選項參考.md) |
| `printf` 的 `%td` | 印 `ptrdiff_t` | [第 2 課第九節 `printf` 完整文法](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#printf-的格式完整文法) |
| `ASAN_OPTIONS=help=1 ./程式` | 列出 ASan 全部 132 個執行時選項（本課沒用任何一個） | 同左 |
| `kill -l 11` | 把訊號編號換成名字 | `man 7 signal`（第 ② 站） |

---

## 九、動手作業

作業在 [`small-project/s1-03-arrays-pointers/`](../small-project/s1-03-arrays-pointers/README.md)，
自我測驗作答在同資料夾的 [`quiz.md`](../small-project/s1-03-arrays-pointers/quiz.md)。

---

## 自我測驗

1. `int *p` 和 `unsigned char *c` 存著同一個位址，為什麼 `p + 1` 和 `c + 1` 不一樣？`p[3]` 在 C 標準裡是怎麼定義的？為什麼 `3[p]` 也能編譯，而且值一樣？
2. `int a[10];` 在 `main` 裡 `sizeof a` 是 40，傳進 `void f(int a[])` 之後 `sizeof a` 變成 8。為什麼？這件事對「怎麼設計接收陣列的函式」有什麼影響？另外，`a` 和 `&a` 印出來的位址一樣，它們差在哪裡？
3. `char s1[] = "hi";` 可以改 `s1[0]`，`char *s2 = "hi";` 改 `s2[0]` 卻 segfault。兩者的 `"hi"` 分別在記憶體的哪裡？為什麼寫入一個會當、一個不會？你會怎麼在 gdb 裡確認？
4. 本課看到越界存取有三種下場：安靜出錯、`stack smashing detected`、segfault。分別是什麼決定會落到哪一種？為什麼「程式沒當掉」不能證明沒有越界？
5. 情境題：同事的程式偶爾 segfault，gdb 的 `bt` 顯示當在 `parse()` 裡 `x = buf[k];` 這一行，`k` 的值是 523871，而 `buf` 只有 256 個元素。你接下來要怎麼查出真正的錯誤在哪裡？為什麼不能直接修改這一行就算解決？如果想在「第一次越界的當下」就抓到它，要怎麼做？
