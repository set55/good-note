# s1-03：陣列、字串與指標運算

> 對應課程：第 ① 站第 3 課
> 課程筆記：[陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md)
> 自我測驗：寫在同資料夾的 [quiz.md](./quiz.md)

## 說明

程式碼都在這個資料夾裡**自己寫**，每支程式都用 `gcc -Wall -Wextra -g -O0` 編譯（題目另有指定的除外）。
筆記裡的範例可以參考寫法，但每一題的型別、數值、情境都跟範例不同，照抄範例答不出來。
每題「解釋原因」的地方都標了答案的來源：
- 【筆記第 X 節】：本課教過，回筆記那一節找。
- 【第 2 課筆記第 X 節】：上一課教過，回 [C的資料在記憶體裡長怎樣](../../c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md) 那一節找。
- 【新觀念，先猜】：還沒教過，寫下你的猜測就好，猜錯不算錯。

**「先預測」的題目，一定要先把預測寫進作答框，再執行程式。** 預測錯了沒關係，把錯在哪裡寫下來，這才是練習的重點。

**怎麼作答**：每個小題下面有作答框，直接寫在裡面。批改寫在作答框正下方；要重答時，批改下方會預先放好下一個作答框。

---

## 1. `short` 與 `long` 的指標運算

寫 `stride.c`，宣告：

```c
short s[6] = {1, 2, 3, 4, 5, 6};
long  l[3] = {10, 20, 30};
```

印出 `s`、`s + 3`、`l`、`l + 2`、`&s + 1` 的位址（`%p`），以及 `&s[5] - &s[1]`、`(char *)(l + 2) - (char *)l` 兩個相減的結果。

### 1-1. 執行前先預測：以 `s` 和 `l` 的位址為基準，`s + 3`、`l + 2`、`&s + 1` 各往後多幾個 byte？兩個相減的結果各是多少？寫出每一個的算法。【筆記[第二節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#二指標運算p--1-走一個元素不是一個-byte)、[第三節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#三陣列名稱什麼時候變成指標什麼時候不會)】

**第 1 次（2026-09-29）**
```
s+3: 6 byte  2byte*3 = 6
l+2: 16 byte 8byte*2 = 16
&s+1: 12 byte 1*12byte = 12

&s[5] - &s[1] = 差4各元素

(char *)(l + 2) - (char *)l = 2*8byte = 16byte

```

> **批改**：**對**，五個預測全對，算法也都寫出來了：`s + 3` 是 3 × 2 = 6 byte、`l + 2` 是 2 × 8 = 16 byte；
> `&s + 1` 是 12 byte——`&s` 的型別是「指向整個 `short[6]` 的指標」，一步走整個陣列 6 × 2 = 12（筆記[第三節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#三陣列名稱什麼時候變成指標什麼時候不會)〈`a` 和 `&a`〉）。
> `&s[5] - &s[1]` 差 4 個元素（「各」應是「個」）；`(char *)` 相減是 16 byte。

### 1-2. 編譯執行，貼上輸出，跟預測對照。兩個相減的結果要用哪個 `printf` 格式印？為什麼不是 `%d` 也不是 `%zu`？【筆記[第二節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#二指標運算p--1-走一個元素不是一個-byte)】

**第 1 次（2026-09-29）**
```
s address = 0x7fffe3fe7c44
s+3 address = 0x7fffe3fe7c4a 差 s 6byte
l address = 0x7fffe3fe7c50
l+2 address = 0x7fffe3fe7c60 差 l 16byte
&s + 1 address = 0x7fffe3fe7c50 差 s 12 byte剛好一個數組的長度
&s[5] - &s[1] = 4
(char *)(l + 2) - (char *)l = 16

用td
因爲兩個指標相減的性別是ptrdiff_t
```

> **批改**：**不完整**。輸出正確、跟預測完全一致（本機重跑也一樣），`%td`、型別 `ptrdiff_t` 都對（「性別」應是「型別」）。
> 但題目的後半「為什麼**不是** `%d`、也**不是** `%zu`」沒有回答——只說出它叫 `ptrdiff_t`，沒說出 `ptrdiff_t` 的哪兩個性質讓另外兩個格式不適用。
>
> **提示**【筆記[第二節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#二指標運算p--1-走一個元素不是一個-byte)〈兩個指標相減：得到「差幾個元素」，型別是 `ptrdiff_t`〉】：
> 本機 `ptrdiff_t` 是幾 byte？`%d` 對應的 `int` 是幾 byte？`%zu` 對應的 `size_t` 是有號還是無號？`&s[1] - &s[5]` 會得到什麼？

**第 2 次（重答，2026-09-29）**
> 題目：編譯執行，貼上輸出，跟預測對照。兩個相減的結果要用哪個 `printf` 格式印？為什麼不是 `%d` 也不是 `%zu`？
```
s address = 0x7fffe3fe7c44
s+3 address = 0x7fffe3fe7c4a 差 s 6byte
l address = 0x7fffe3fe7c50
l+2 address = 0x7fffe3fe7c60 差 l 16byte
&s + 1 address = 0x7fffe3fe7c50 差 s 12 byte剛好一個數組的長度
&s[5] - &s[1] = 4
(char *)(l + 2) - (char *)l = 16

ptrdiff_t 8byte 有號整數
int是4byte
zu是無號

&s[5] - &s[1] = 4 這個出來的型別是ptrdiff_t 不是int 也不是size_t 至於爲什麼我不確定
```

> **批改（第 2 次）**：**大致對**。答案需要的兩個性質你都寫出來了——`ptrdiff_t` 是 **8 byte**、**有號**；`int` 是 4 byte；`%zu` 是無號——只差最後一步：把這兩個性質接到「所以不能用」。
> 已經兩次作答，公布參考答案。本機實測（`fmt.c`，印 `&s[1] - &s[5]`，應該是 −4）：
> ```
> $ gcc -Wall -Wextra -g -O0 fmt.c -o fmt
> fmt.c:8:20: warning: format '%d' expects argument of type 'int', but argument 2 has type 'long int' [-Wformat=]
> $ ./fmt
> %td: -4
> %zu: 18446744073709551612      ← 把 −4 的 8 個 byte 當成無號讀：2⁶⁴ − 4
> %d : -4                        ← 這次碰巧對，但 gcc 已經警告型別不符
> ```
> `fmt.c` 原始碼：
> ```c
> #include <stdio.h>
>
> int main(void)
> {
>     short s[6] = {1, 2, 3, 4, 5, 6};
>     printf("%%td: %td\n", &s[1] - &s[5]);
>     printf("%%zu: %zu\n", &s[1] - &s[5]);
>     printf("%%d : %d\n", &s[1] - &s[5]);
>     return 0;
> }
> ```
>
> **參考答案**：用 **`%td`**，因為相減的結果型別是 `ptrdiff_t`（`t` 就是它的長度修飾字）。
> - 不用 `%d`：`%d` 要的是 4 byte 的 `int`，`ptrdiff_t` 在本機是 8 byte 的 `long`，**大小不符**——gcc 的 `-Wformat` 會警告；差距超出 `int` 範圍時印出來的數字就是錯的。
> - 不用 `%zu`：`%zu` 要的是**無號**的 `size_t`，但指標相減**可以是負的**（`&s[1] - &s[5]` 是 −4），用 `%zu` 印會變成 18446744073709551612（第 2 課的「同寬換讀法」）。

### 1-3. `&s[5] - &s[1]` 和 `(char *)&s[5] - (char *)&s[1]` 差在哪裡？為什麼同樣兩個位址相減，會得到兩個不同的數？【筆記[第二節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#二指標運算p--1-走一個元素不是一個-byte)】

**第 1 次（2026-09-29）**
```
數組內指標相減 得到的類型是ptrdiff_t,一個long類型,相差的是元素個數
第二個因爲轉成char 單位是1byte,所以應該是4 * 2 * 1 = 8byte,第二個有轉換類型
```

> **批改**：**對**。指標相減的結果是「差幾個元素」：兩個位址差 8 byte，除以元素大小——`short *` 除以 2 得 4，`char *` 除以 1 得 8。
> 同樣兩個位址，相減時用哪個型別的指標，就決定「一步」多大。（作答原本寫在作答框外面，已搬進框裡。）

---

## 2. 不用 `[]` 把陣列倒過來

寫 `rev.c`，實作：

```c
void reverse(int *first, int n);
```

把從 `first` 開始的 `n` 個 `int` 原地倒轉（例如 `{1, 2, 3, 4, 5}` 變成 `{5, 4, 3, 2, 1}`）。
**函式裡不准出現 `[]`**，只能用指標運算（`+`、`-`、`++`、`--`、`*`、指標比較）。
`main` 裡分別用 5 個元素和 6 個元素的陣列測試，印出倒轉前後的內容。

### 2-1. 貼上你的 `reverse` 函式和執行結果。奇數個和偶數個元素，你的迴圈分別在什麼條件下停止？【筆記[第二節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#二指標運算p--1-走一個元素不是一個-byte)】

**第 1 次（2026-09-29）**
```
#include <stdio.h>

void reverse(int *first, int n) {
    int start = 0;
    int end = n - 1;
    while (start < end) {
        int* getfirst = first + start;
        int* getend = first + end;

        int temp = *getfirst;
        *getfirst = *getend;
        *getend = temp;

        start++;
        end--;
    }
}

int main() {
    int arr[] = {1, 2, 3, 4, 5};
    int arr2[] = {1, 2, 3, 4, 5, 6};
    reverse(arr, 5);
    reverse(arr2, 6);
    printf("arr reversed: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    printf("arr2 reversed: ");
    for (int i = 0; i < 6; i++) {
        printf("%d ", arr2[i]);
    }
    printf("\n");
    return 0;
}

arr reversed: 5 4 3 2 1 
arr2 reversed: 6 5 4 3 2 1

當start > end 的時候迴圈結束,都交換完了
```

> **批改**：程式**對**，兩組都正確倒轉，也沒有用到 `[]`。停止條件**不完整**：題目要你**分開**說奇數和偶數，你只寫了一個「`start > end`」，而這對奇數個是錯的。
> 另外一個寫法上的建議（不扣分）：你用兩個整數 `start`、`end` 當索引，每圈再算 `first + start`——這其實是把 `a[i]` 換個寫法。
> 題目想練的是**直接移動兩根指標**：一根從頭往後 `++`、一根從尾往前 `--`，比較兩根指標決定何時停（筆記第二節：同一個陣列裡的指標可以比大小）。有興趣可以改寫看看。
>
> **提示**：用 5 個元素手動走一次你的迴圈：每一圈結束時 `start`、`end` 各是多少？最後一次檢查 `start < end` 時，兩者的值是什麼、是「大於」還是「等於」？再用 6 個元素走一次。中間那個元素（奇數時）需要交換嗎？

**第 2 次（重答，2026-09-29）**
> 題目：貼上你的 `reverse` 函式和執行結果。奇數個和偶數個元素，你的迴圈分別在什麼條件下停止？
```
我不太確定題目想讓我怎麼寫,可以直接寫出來嗎?
```

> **批改（第 2 次）**：還沒作答，你問題目要怎麼寫——這裡把題目說清楚。程式那一半第 1 次已經對了，**只剩停止條件**：
> 你的迴圈是 `while (start < end)`，它在 `start < end` 變成**不成立**的那一刻停。「不成立」有兩種可能：`start == end`，或 `start > end`。
> 題目問的是：5 個元素時是哪一種？6 個元素時是哪一種？
>
> **怎麼寫**：照下面的格式，把你的迴圈**用手**走一次（或在 `while` 裡加一行 `printf` 印出 `start`、`end`），填完兩張表，最後各用一句話回答「停的時候 `start` 和 `end` 是什麼關係」、以及「奇數時中間那一格有沒有被換、需不需要換」。
> ```
> n = 5：
> 檢查第幾次   start   end   start < end ?
> 1            0       4     成立 → 交換
> 2            ?       ?     ?
> ...
> n = 6：
> （同上）
> ```

> **參考答案**（2026-09-30 改為重答一次即公布）：迴圈 `while (start < end)` 在條件不成立時停，兩種情況停的方式不同：
> ```
> n = 5：檢查 (start, end) = (0,4) 交換 → (1,3) 交換 → (2,2) 不成立，停     ← start == end
> n = 6：檢查 (start, end) = (0,5) 交換 → (1,4) 交換 → (2,3) 交換 → (3,2) 不成立，停   ← start > end
> ```
> - **奇數個**：停在 `start == end`，兩個索引剛好碰在中間那一格（`arr[2]`）。中間那格跟自己交換沒有意義，**不需要換**，所以停在這裡正好。
> - **偶數個**：沒有中間那格，最後一次交換 `(2,3)` 之後兩者**交錯**，停在 `start > end`（`start == end + 1`）。
> - 所以條件寫 `start < end` 兩種情況都對；寫成 `start != end` 的話，偶數個時兩者會直接交錯過去、永遠不相等，一路換到陣列外面。

### 2-2. 在 `reverse` 裡加一行印出 `sizeof first`，再把參數改寫成 `int first[]`，重新編譯。各印出多少？編譯器說了什麼？為什麼 `reverse` 一定要有 `n` 這個參數？【筆記[第三節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#三陣列名稱什麼時候變成指標什麼時候不會)】

**第 1 次（2026-09-29）**
```
size of first: 8
參數改了, size of first: 8
warning: ‘sizeof’ on array function parameter ‘first’ will return size of ‘int *’ [-Wsizeof-array-argument]
    6 |     printf("size of first: %zu\n", sizeof first);
把他當成了 int指標了, 所以還是8byte

因爲指標進去 我們還是不知道陣列長度,不帶就不知道陣列邊界在哪裏
```

> **批改**：**對**。兩次都是 8；`int first[]` 寫在參數裡其實就是 `int *first`，gcc 用 `-Wsizeof-array-argument` 提醒你 `sizeof` 拿到的是指標大小；
> 陣列傳進函式只剩起點位址，長度資訊不見了，所以一定要另外傳 `n`（筆記[第三節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#三陣列名稱什麼時候變成指標什麼時候不會)〈函式參數寫成 `int a[]`，其實就是 `int *a`〉）。

### 2-3. 在 `main` 裡故意把 5 個元素的陣列用 `reverse(a, 6)` 呼叫。**先預測**：用一般方式編譯執行，會當掉嗎？再加 `-fsanitize=address` 編譯執行，貼上報告的前 5 行，說明它是 `READ` 還是 `WRITE`、幾個 byte、發生在哪一行，以及為什麼一般編譯的版本可能看不出錯。【筆記[第五節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#五c-不檢查邊界越界的三種下場)、[第七節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#七addresssanitizer在第一次越界的當下抓到它)】

**第 1 次（2026-09-29）**
```
1. 不會
2. 
 gcc -Wall -Wextra -fsanitize=address -O0 -o rev rev.c    ok | base py | at 14:47:54 
rev.c: In function ‘main’:
rev.c:31:23: warning: comparison of integer expressions of different signedness: ‘int’ and ‘long unsigned int’ [-Wsign-compare]
   31 |     for (int i = 0; i < sizeof arr2 / sizeof arr2[0]; i++) {
      |      
報告只有出這個

可能沒有到金絲雀記憶體位址,也沒有到main入口點村的rip位址
```

> **批改**：預測「不會當掉」**對**（本機一般編譯跑完 exit 0）。「只越界一格，碰不到金絲雀和返回位址」方向也對。
> 但 ASan 那部分**還沒做**：你貼的是**編譯**時的警告（第 31 行 `int` 和 `sizeof` 比較，跟這題無關），不是 ASan 報告。**ASan 的報告要等程式執行時才出現**——編譯完要再跑 `./rev`。
> 我跑了你資料夾裡那支 `rev`，報告確實有出來，只是你沒執行。還有兩點：
> - 題目說明要求用 `-g` 編譯，你這次沒加。沒有 `-g`，報告裡只有 `reverse+0x1350` 這種偏移，**看不到行號**（筆記[第七節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#七addresssanitizer在第一次越界的當下抓到它)）。
> - 一般編譯的結果其實**不是**完全正確：看看印出來的 `arr` 第一個數字是多少，它是從哪裡來的？
>
> **提示**：用 `gcc -Wall -Wextra -g -O0 -fsanitize=address -o rev rev.c` 重新編譯，**再執行 `./rev`**，貼上前 5 行，回答題目的四件事（`READ`／`WRITE`、幾個 byte、哪一行、為什麼一般版本看不出錯）。
> 再用一般方式編譯執行一次，貼上 `arr reversed:` 那一行，說明第一個數字和第六個數字各是怎麼來的。

**第 2 次（重答，2026-09-29）**
> 題目：在 `main` 裡故意把 5 個元素的陣列用 `reverse(a, 6)` 呼叫。**先預測**：用一般方式編譯執行，會當掉嗎？再加 `-fsanitize=address` 編譯執行，貼上報告的前 5 行，說明它是 `READ` 還是 `WRITE`、幾個 byte、發生在哪一行，以及為什麼一般編譯的版本可能看不出錯。
```
size of first: 8
=================================================================
==9732==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x777b5bb00034 at pc 0x5942bee5c351 bp 0x7ffda5d08460 sp 0x7ffda5d08450
READ of size 4 at 0x777b5bb00034 thread T0
    #0 0x5942bee5c350 in reverse /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/rev.c:12
    #1 0x5942bee5c700 in main /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/rev.c:23
    #2 0x777b5da2a1c9 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
    #3 0x777b5da2a28a in __libc_start_main_impl ../csu/libc-start.c:360
    #4 0x5942bee5c1a4 in _start (/home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/rev+0x11a4) (BuildId: 39769f27c794178662c6263c65a077f98c280931)

Address 0x777b5bb00034 is located in stack of thread T0 at offset 52 in frame
    #0 0x5942bee5c38d in main /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/rev.c:20           

是read出錯,在程式12行
*getfirst = *getend;

看錯誤像是超出stack, 但具體原因可能要麻煩你跟我說
主要是因爲arr只有5個,但是我們直接只向第6個byte的位址所以stack overflow?
```

> **批改（第 2 次）**：這次**有執行、有加 `-g`**，報告有行號了。（作答原本寫在空白框下面，已搬進框裡。）
> - `READ`、第 12 行 `*getfirst = *getend;` **對**。幾個 byte 沒寫出來：報告寫 `READ of size 4`——4 byte，剛好一個 `int`。
> - 「指向第 6 個 **byte**」**不對**：是第 6 個**元素**（`arr[5]`），位址是 `arr` + 5 × 4 = 偏移 20 byte。這正是第 1 題練的「`p + 1` 走一個元素」。
> - 你問「`stack-buffer-overflow` 是不是 stack overflow」：**不是**，這兩個是不同的東西。報告的名字要拆開讀：**stack** 上的一個 **buffer**（緩衝區，就是陣列）被 **overflow**（寫／讀超過結尾）——
>   也就是「**堆疊上的陣列越界**」（筆記[第七節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#七addresssanitizer在第一次越界的當下抓到它)那張表的第一列）。
>   「stack overflow」（堆疊溢位）則是整個堆疊用完了，例如遞迴太深——第 5 題會碰到。
> - 報告的下半段你沒有貼：本機重跑是 `[32, 52) 'arr' (line 21) <== Memory access at offset 52 overflows this variable`。`arr` 佔偏移 32～51（5 × 4 = 20 byte），讀的是偏移 52，**剛好超出結尾**。
> - 「為什麼一般版本看不出錯」這次沒答（第 1 次答的方向是對的）。
>
> **提示**：
> 1. 第 12 行 `*getfirst = *getend;` 同時有讀和寫，第 13 行 `*getend = temp;` 是寫。越界的那一格**先被讀、後被寫**，為什麼報告只說 `READ`，沒有 `WRITE`？【筆記[第七節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#七addresssanitizer在第一次越界的當下抓到它)那張表最後一列】
> 2. 一般編譯（不加 ASan）執行一次，貼上 `arr reversed:` 那一行。第一個數字是什麼、從哪裡來？原本 `arr` 裡的 `1` 被搬到哪裡去了？這說明一般版本是「沒有錯」還是「錯了但沒當掉」？【筆記[第五節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#五c-不檢查邊界越界的三種下場)〈下場一：安靜地出錯〉】

> **參考答案**（2026-09-30 改為重答一次即公布）：
> - 一般編譯**不會當掉**。ASan 報告：`stack-buffer-overflow`（堆疊上的陣列越界）、**`READ of size 4`**（讀了 4 byte，一個 `int`）、發生在 **`rev.c:12`** 的 `*getfirst = *getend;`，
>   由 `main` 第 23 行 `reverse(arr, 6)` 呼叫；`[32, 52) 'arr' <== Memory access at offset 52`：`arr` 佔 20 byte，讀的是**剛好超出結尾**的下一格，也就是第 6 個元素 `arr[5]`。
> - 為什麼只有 `READ`：第一次 swap（`start=0, end=5`）時，第 12 行**先讀** `arr[5]`，第 13 行才寫它。ASan 在**第一個錯誤就停**（筆記[第七節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#七addresssanitizer在第一次越界的當下抓到它)表格最後一列），第 13 行的越界寫入根本沒機會執行。
> - 一般版本本機實測印出 `arr reversed: 0 5 4 3 2 1`：第一個數字 `0` 是 `arr` 後面那 4 byte 原本的內容（這裡是空隙），被換進了 `arr[0]`；
>   原本的 `1` 則被**寫到 `arr` 外面**。程式不是「沒有錯」，而是**錯了但沒當掉**（筆記[第五節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#五c-不檢查邊界越界的三種下場)〈下場一：安靜地出錯〉）。
>   只越界一格，碰到的是空隙或別的區域變數，碰不到金絲雀與返回位址，所以堆疊保護也不會觸發。

---

## 3. 自己寫 `strlen`

寫 `mystrlen.c`，實作：

```c
size_t my_strlen(char *s);
```

**只能用指標往前走**：用另一個指標從 `s` 開始走到 `'\0'`，最後用兩個指標相減得到長度（不准用 `strlen`、不准用 `[]`、不准另外用整數計數）。

### 3-1. 貼上你的 `my_strlen`，並用 `"Linux"`、`""`（空字串）、`"字"` 三個字串測試，貼上結果。`"字"` 的結果為什麼不是 1？【筆記[第二節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#二指標運算p--1-走一個元素不是一個-byte)、[第四節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#四字串以-0-結尾的-char-陣列)；〈字元是什麼〉[第四節](../../c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.md#四中文字一個-byte-裝不下utf-8-用-3-個-byte)】

**第 1 次（2026-09-29）**
```
#include <stdio.h>

size_t my_strlen(char *s){
    char *start = s;
    while (*s != '\0') {
        s++;
    }
    return s - start;
}

int main() {
    char li[] = "Linux";
    char empty[] = ""; 
    char word[] = "字";
    printf("Length of '%s' is %zu\n", li, my_strlen(li));
    printf("Length of '%s' is %zu\n", empty, my_strlen(empty));
    printf("Length of '%s' is %zu\n", word, my_strlen(word));
    return 0;
}

Length of 'Linux' is 5
Length of '' is 0
Length of '字' is 3

因爲根據unicode編碼字是三個字元
```

> **批改**：程式**對**：一根指標走到 `'\0'`、兩根指標相減，符合題目限制，三個結果 5、0、3 都正確（本機重跑一樣）。
> 「為什麼不是 1」**錯**：「字是三個**字元**」正是〈字元是什麼〉第 5 題卡住的那個混淆——「字」是**一個**字元（一個 Unicode code point），3 是別的東西。
> 另外，把中文變成 3 的那套編碼規則叫什麼？不是「Unicode 編碼」這個籠統的說法。
>
> **提示**【〈字元是什麼〉[第四節](../../c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.md#四中文字一個-byte-裝不下utf-8-用-3-個-byte)、第五節】：`my_strlen` 每走一步，`s` 前進的是一個什麼（看 `s` 的型別）？
> 它數的是字元數，還是別的單位？「字」在檔案裡是用哪一種編碼存的、佔幾個那種單位？

**第 2 次（重答，2026-09-29）**
> 題目：貼上你的 `my_strlen`，並用 `"Linux"`、`""`（空字串）、`"字"` 三個字串測試，貼上結果。`"字"` 的結果為什麼不是 1？
```
我直接回答錯的部分
是因爲我的my_strlen 是一個字元一個字元算,到\0這個字元停下,所以走了3次,因此判斷"字"是三個字元
```

> **批改（第 2 次）**：**不完整**。「`my_strlen` 一格一格走、走到 `\0` 停，所以走了 3 步」**對**。
> 但「一個字元一個字元算」和「字是三個字元」還是把兩種「字元」混在一起了：`my_strlen` 一步走的是一個 **`char`，也就是 1 個 byte**，不是一個字元（字）。
> 答案需要的「byte」「UTF-8」兩個詞都沒有出現。已經兩次作答，公布參考答案——這跟〈字元是什麼〉第 5 題是同一個觀念，明天（10/1）以後複習那篇時會再考。
>
> **參考答案**：`my_strlen` 的指標是 `char *`，每次 `s++` 前進 **1 個 byte**，所以它數的是 **byte 數**，不是字元數。
> 「字」是**一個**字元（一個 Unicode code point，U+5B57），原始碼存成 **UTF-8**，一個中文字用 **3 個 byte**（`e5 ad 97`）表示，
> 後面才是 `\0`，所以走 3 步。C 的 `char` 叫「字元」，其實是「1 byte 的整數」（〈字元是什麼〉[第一節](../../c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.md#一字元就是一個數字)），這個名字就是混淆的來源。

### 3-2. 宣告 `char name[16] = "Linux";`。**先預測** `sizeof name` 和 `my_strlen(name)` 各是多少，再執行驗證。為什麼兩個數字不一樣？`name[5]` 到 `name[15]` 裡是什麼？【筆記[第四節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#四字串以-0-結尾的-char-陣列)；後半【新觀念，先猜】】

**第 1 次（2026-09-29）**
```
sizeof name 預測是 16(byte)
my_strlen(name) 預測是5
sizeof name是name指標指向整個陣列,所以是整個陣列的大小
mystren到\0就會停下來了,只有5

name[5]是\0
所以剩餘的我猜都是\0
```

> **批改**：預測 16 和 5 **對**，`my_strlen` 走到 `\0` 就停也對。但有兩個問題：
> - **沒有執行驗證**：題目要求預測完再執行，`strlen.c` 裡沒有 `name`，作答框也沒有輸出。
> - **「`name` 指標指向整個陣列」不對**：`name` 是**陣列**，不是指標。`sizeof name` 得到 16，正是因為 `sizeof` 作用在陣列名稱上時**不會**退化成指標（筆記[第三節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#三陣列名稱什麼時候變成指標什麼時候不會)）；
>   如果它是指標，`sizeof` 會是 8——跟你在第 2-2 題看到的一樣。
>
> 後半（【新觀念，先猜】）：猜「`name[5]` 是 `\0`，剩下都是 `\0`」**猜對了**。規則是：陣列的初始值比陣列短時，沒給到的格子一律補 0。
> 已補教進筆記第四節〈[陣列大小跟字串長度不一樣時：比較長、剛好、比較短](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#陣列大小跟字串長度不一樣時比較長剛好比較短)〉（附實測）。
>
> **提示**：在程式裡加上 `name`，實際印出 `sizeof name` 與 `my_strlen(name)` 貼上來；再用一句話重寫「為什麼兩個數字不一樣」——`sizeof` 量的是什麼、`my_strlen` 量的是什麼，並把「指標」這個錯字眼拿掉。

**第 2 次（重答，2026-09-29）**
> 題目：宣告 `char name[16] = "Linux";`。**先預測** `sizeof name` 和 `my_strlen(name)` 各是多少，再執行驗證。為什麼兩個數字不一樣？`name[5]` 到 `name[15]` 裡是什麼？
```
其餘的描述同上,我這邊改寫了strlen.c 並且把輸出貼上
Length of 'Linux' is 5
Length of '' is 0
Length of '字' is 3
Length of 'LinuxLinux' is 10
Length of 'LinuLinuxLinux' is 14
Length of 'Linux' is 16
Length of 'Linux' is 5

sizeof name 預測是 16(byte)
my_strlen(name) 預測是5
sizeof name是整個陣列,所以是整個陣列的大小


```

> **批改（第 2 次）**：**對**。實際執行得到 16 和 5，跟預測一致；「`sizeof name` 是整個陣列的大小」這次把「指標」拿掉了，說法正確。
> 小提醒：你印 `sizeof name` 那行的標籤也寫成 `Length of`，自己回頭看輸出時容易搞混，標籤寫 `sizeof` 比較好。
>
> **參考答案**：`sizeof name` 量的是**陣列佔多少 byte**（宣告時決定，16），`my_strlen` 量的是**從開頭到第一個 `\0` 有幾個 byte**（5）。兩個量的東西不同。
> `name[5]` 到 `name[15]` 全是 0：初始值比陣列短時，沒給到的格子補 0（補教見筆記第四節〈陣列大小跟字串長度不一樣時〉）。

### 3-3. 再宣告 `char t[5] = "Linux";` 和 `char u[4] = "Linux";`。**先預測**：`-Wall -Wextra` 會對哪一行警告？（【新觀念，先猜】）然後對 `t` 呼叫 `my_strlen`，分別用一般方式和 `-fsanitize=address` 編譯執行，貼上結果。`t` 少了什麼？為什麼一般編譯可能印出看起來正確的 5？【筆記[第四節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#四字串以-0-結尾的-char-陣列)、[第五節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#五c-不檢查邊界越界的三種下場)、[第七節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#七addresssanitizer在第一次越界的當下抓到它)】

**第 1 次（2026-09-29）**
```
兩行都會,長度都不夠

-Wall -Wextra
~/w/Proj/g/good-note/s/s1-03-arrays-pointers | on main !1 ?3  gcc -Wall -Wextra -O0 -o strlen strlen.c                 ok | base py | at 15:00:06 
strlen.c: In function ‘main’:
strlen.c:16:17: warning: initializer-string for array of ‘char’ is too long
   16 |     char u[4] = "Linux";
      |                 ^~~~~~

輸出結果:
Length of 'Linux' is 5
Length of '' is 0
Length of '字' is 3
Length of 'LinuxLinux' is 10
Length of 'LinuLinuxLinux' is 14

gcc -Wall -Wextra -O0 -fsanitize=address -o strlen strlen.c
strlen.c: In function ‘main’:
strlen.c:16:17: warning: initializer-string for array of ‘char’ is too long
   16 |     char u[4] = "Linux";
      |                 ^~~~~~~
輸出
~/w/Proj/g/good-note/s/s1-03-arrays-pointers | on main !1 ?3  ./strlen                                                 ok | base py | at 15:19:35 
Length of 'Linux' is 5
Length of '' is 0
Length of '字' is 3
=================================================================
==48796==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x7548bf500065 at pc 0x59e273b042dc bp 0x7ffe44f8d8f0 sp 0x7ffe44f8d8e0
READ of size 1 at 0x7548bf500065 thread T0
    #0 0x59e273b042db in my_strlen (/home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/strlen+0x12db) (BuildId: 31e3859f785c2e80912216ed533a9b8ba6afb932)
    #1 0x59e273b045b8 in main (/home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/strlen+0x15b8) (BuildId: 31e3859f785c2e80912216ed533a9b8ba6afb932)
    #2 0x7548c142a1c9 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
    #3 0x7548c142a28a in __libc_start_main_impl ../csu/libc-start.c:360
    #4 0x59e273b041c4 in _start (/home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/strlen+0x11c4) (BuildId: 31e3859f785c2e80912216ed533a9b8ba6afb932)

Address 0x7548bf500065 is located in stack of thread T0 at offset 101 in frame
    #0 0x59e273b04300 in main (/home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/strlen+0x1300) (BuildId: 31e3859f785c2e80912216ed533a9b8ba6afb932)

  This frame has 5 object(s):
    [48, 49) 'empty' (line 13)
    [64, 68) 'word' (line 14)
    [80, 84) 'u' (line 16)
    [96, 101) 't' (line 15) <== Memory access at offset 101 overflows this variable
    [128, 134) 'li' (line 12)
HINT: this may be a false positive if your program uses some custom stack unwind mechanism, swapcontext or vfork
      (longjmp and C++ exceptions *are* supported)
SUMMARY: AddressSanitizer: stack-buffer-overflow (/home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/strlen+0x12db) (BuildId: 31e3859f785c2e80912216ed533a9b8ba6afb932) in my_strlen
Shadow bytes around the buggy address:
  0x7548bf4ffd80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7548bf4ffe00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7548bf4ffe80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7548bf4fff00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7548bf4fff80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
=>0x7548bf500000: f1 f1 f1 f1 f1 f1 01 f2 04 f2 04 f2[05]f2 f2 f2
  0x7548bf500080: 06 f3 f3 f3 00 00 00 00 00 00 00 00 00 00 00 00
  0x7548bf500100: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7548bf500180: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7548bf500200: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7548bf500280: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==48796==ABORTING

 ~/w/Proj/g/good-note/s/s1-03-arrays-pointers | on main !1 ?3                                                        1 err | base py | at 15:20:23 
```

> **批改**：**實驗做完、輸出也對，但題目問的兩個「為什麼」都沒有回答。**
> - 預測「兩行都會警告」**猜錯，不算錯**（新觀念）：實際只有 `u[4]` 那行。`t[5]` 剛好放得下 5 個字元、放不下 `\0`，C 允許這樣寫，本機 gcc 13.3 開 `-Wall -Wextra` 也**不警告**——這正是它危險的地方。
>   已補教進筆記第四節〈[陣列大小跟字串長度不一樣時：比較長、剛好、比較短](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#陣列大小跟字串長度不一樣時比較長剛好比較短)〉。
> - 你的一般編譯輸出很有價值：`t` 得到 **10**（`LinuxLinux`）、`u` 得到 **14**（`LinuLinuxLinux`）。我印了三個陣列的位址，它們在堆疊上**緊緊相連**：`u` 後面就是 `t`、`t` 後面就是 `li`，
>   所以 `my_strlen(t)` 讀過 `t` 的結尾、一路讀完 `li` 的 `Linux\0` 才停（實測與原始碼見筆記那一小節）。
> - ASan 報告你貼了，但沒有解讀；而且又沒加 `-g`，第一行只有 `my_strlen+0x12db`，沒有行號。加了 `-g` 本機會顯示 `my_strlen strlen.c:5`、`main strlen.c:20`。
>
> **提示**：重答時回答三件事：
> 1. `t` 的 5 格裡放了什麼、**少了什麼**？這跟 `my_strlen` 的停止條件有什麼關係？【筆記[第四節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#四字串以-0-結尾的-char-陣列)】
> 2. 看你貼的 ASan 報告：是 `READ` 還是 `WRITE`、幾個 byte？`[96, 101) 't'` 和 `offset 101` 說明讀到了哪裡？【筆記[第七節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#七addresssanitizer在第一次越界的當下抓到它)】
> 3. 你的 `t` 印出 10 而不是 5。什麼情況下會印出「看起來正確」的 5？印出 5 代表程式是對的嗎？【筆記[第五節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#五c-不檢查邊界越界的三種下場)〈下場一：安靜地出錯〉】

**第 2 次（重答，2026-09-29）**
> 題目：再宣告 `char t[5] = "Linux";` 和 `char u[4] = "Linux";`。**先預測**：`-Wall -Wextra` 會對哪一行警告？（【新觀念，先猜】）然後對 `t` 呼叫 `my_strlen`，分別用一般方式和 `-fsanitize=address` 編譯執行，貼上結果。`t` 少了什麼？為什麼一般編譯可能印出看起來正確的 5？
```
其他輸出同上
t的5格
0x4c    0x69    0x6e    0x75    0x78
少了\0 = 0x00

Start Addr           End Addr       Size     Offset  Perms  objfile
      0x555555554000     0x555555555000     0x1000        0x0  r--p   /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/strlen
      0x555555555000     0x555555556000     0x1000     0x1000  r-xp   /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/strlen
      0x555555556000     0x555555557000     0x1000     0x2000  r--p   /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/strlen
      0x555555557000     0x555555558000     0x1000     0x2000  r--p   /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/strlen
      0x555555558000     0x555555559000     0x1000     0x3000  rw-p   /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/strlen
      0x555555559000     0x55555557a000    0x21000        0x0  rw-p   [heap]
      0x7ffff7c00000     0x7ffff7c28000    0x28000        0x0  r--p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7c28000     0x7ffff7db1000   0x189000    0x28000  r-xp   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7db1000     0x7ffff7e00000    0x4f000   0x1b1000  r--p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7e00000     0x7ffff7e04000     0x4000   0x1ff000  r--p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7e04000     0x7ffff7e06000     0x2000   0x203000  rw-p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7e06000     0x7ffff7e13000     0xd000        0x0  rw-p
      0x7ffff7f9e000     0x7ffff7fa1000     0x3000        0x0  rw-p
      0x7ffff7fbb000     0x7ffff7fbd000     0x2000        0x0  rw-p
      0x7ffff7fbd000     0x7ffff7fc1000     0x4000        0x0  r--p   [vvar]
      0x7ffff7fc1000     0x7ffff7fc3000     0x2000        0x0  r--p   [vvar_vclock]
      0x7ffff7fc3000     0x7ffff7fc5000     0x2000        0x0  r-xp   [vdso]
      0x7ffff7fc5000     0x7ffff7fc6000     0x1000        0x0  r--p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7fc6000     0x7ffff7ff1000    0x2b000     0x1000  r-xp   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7ff1000     0x7ffff7ffb000     0xa000    0x2c000  r--p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7ffb000     0x7ffff7ffd000     0x2000    0x36000  r--p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7ffd000     0x7ffff7fff000     0x2000    0x38000  rw-p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffffffdd000     0x7ffffffff000    0x22000        0x0  rw-p   [stack]
  0xffffffffff600000 0xffffffffff601000     0x1000        0x0  --xp   [vsyscall]

  (gdb) x t
0x7fffffffd5f5: 0x4c
(gdb) x u
0x7fffffffd5f1: 0x4c
(gdb) x li
0x7fffffffd5fa: 0x4c
(gdb) x/6xb li
0x7fffffffd5fa: 0x4c    0x69    0x6e    0x75    0x78    0x00
(gdb) x empty
0x7fffffffd5ec: 0x00
(gdb) x word
0x7fffffffd5ed: 0xe5
(gdb) x/11xb u
0x7fffffffd5f1: 0x4c    0x69    0x6e    0x75    0x4c    0x69    0x6e    0x75
0x7fffffffd5f9: 0x78    0x4c    0x69
(gdb) x/12xb u
0x7fffffffd5f1: 0x4c    0x69    0x6e    0x75    0x4c    0x69    0x6e    0x75
0x7fffffffd5f9: 0x78    0x4c    0x69    0x6e
(gdb) x/13xb u
0x7fffffffd5f1: 0x4c    0x69    0x6e    0x75    0x4c    0x69    0x6e    0x75
0x7fffffffd5f9: 0x78    0x4c    0x69    0x6e    0x75
(gdb) x/15xb u
0x7fffffffd5f1: 0x4c    0x69    0x6e    0x75    0x4c    0x69    0x6e    0x75
0x7fffffffd5f9: 0x78    0x4c    0x69    0x6e    0x75    0x78    0x00
我的沒有印出5, t,li在記憶體位址連在一起了

中間也沒有/0做區隔

如果長度剛好是5個,而且雖然沒有\0的那個格,c允許他這樣寫
```

> **批改（第 2 次）**：提示的三件事：1 **對**、2 **未答**、3 **不完整**。
> - 1：`t` 的 5 格是 `4c 69 6e 75 78`（`Linux`），**少了 `\0`** ——對。你還自己用 gdb 的 `x/15xb u` 把 `u`（`d5f1`）→ `t`（`d5f5`）→ `li`（`d5fa`）一路讀到 `li` 的 `00`，
>   親眼證實三個陣列緊緊相連、中間沒有 `\0`，所以 `t` 得到 10。這個實驗做得很好，也跟筆記補教的位址實測吻合。（`info proc mappings` 這題用不到，可以不貼。）
> - 2：ASan 報告沒有解讀。
> - 3：「C 允許剛好 5 格的寫法」是**為什麼沒有警告**的答案，不是「什麼時候會印出 5」的答案。
>
> **提示**：
> 1. 看你第 1 次貼的報告：`READ of size 1` 和 `[96, 101) 't' (line 15) <== Memory access at offset 101 overflows this variable`。`t` 佔哪幾個偏移？讀的是哪一個偏移、那是 `t` 的第幾格？
>    是**讀**還是**寫**？（對照你在 2-3 讀過的 `[32, 52) 'arr'`，是同一種看法。）
> 2. 你的 gdb 輸出裡，`t` 後面緊接的那個 byte（`0x7fffffffd5fa`）是什麼？`my_strlen(t)` 要在第 5 步就停下來、印出 5，那個 byte 必須是什麼？
>    如果印出了 5，代表 `t` 是一個正確的字串嗎？

> **參考答案**（2026-09-30 改為重答一次即公布）：
> - `t` 的 5 格是 `L i n u x`，**少了結尾的 `\0`**。`my_strlen` 只靠「碰到 `\0` 才停」判斷結尾，所以會走出 `t`。
> - ASan：**`READ of size 1`**，`[96, 101) 't'` 表示 `t` 佔偏移 96～100，讀的是偏移 **101**——`t` 後面的第一個 byte（`t[5]`），`my_strlen` 在第 5 行 `while (*s != '\0')` 讀它時被抓到。
> - 你的一般編譯得到 10：`t` 後面緊接的是 `li` 的 `Linux\0`，一路讀到 `li` 的 `\0` 才停（你的 `x/15xb u` 已經證實）。
>   只有當 `t` 後面**碰巧**是一個 `0` byte（例如對齊留下的空隙、換個最佳化等級或變數順序），才會在第 5 步停下、印出「看起來正確」的 5。
> - 印出 5 **不代表程式對**：它照樣讀了陣列外面的記憶體，只是碰巧讀到 0。結果對、程式錯——正是筆記[第五節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#五c-不檢查邊界越界的三種下場)〈下場一：安靜地出錯〉，也是要用 ASan 的原因。

---

## 4. 用 gdb 找出 segfault 的真正原因

下面這支程式**有 bug**（這是題目，不是解答）。把它複製成 `find.c`，**不要先改**：

```c
#include <stdio.h>

int data[8] = {3, 1, 4, 1, 5, 9, 2, 6};

int *find(int *p, int target)
{
    int *start = p;             /* 只是為了在 gdb 裡算「走了多遠」 */
    while (*p != target)        /* 第 8 行 */
        p++;
    (void)start;                /* 告訴編譯器「我知道 start 沒被用到」，避免 -Wextra 的警告 */
    return p;
}

int main(void)
{
    printf("找 5：第 %td 個\n", find(data, 5) - data);
    printf("找 7：第 %td 個\n", find(data, 7) - data);   /* 第 17 行 */
    return 0;
}
```

注意 `data` 宣告在**函式外面**（全域變數），不在堆疊上。

### 4-1. 編譯執行，貼上輸出與結束狀態（`echo $?`）。第一行為什麼是對的？結束狀態代表什麼？【筆記[第五節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#五c-不檢查邊界越界的三種下場)】

**第 1 次（2026-09-30）**
```
~/w/Proj/g/good-note/s/s1-03-arrays-pointers | on main >8 ?1  ./find; echo $?                                                                         ok | base py | at 14:17:29
找 5：第 4 個
[1]    29761 segmentation fault (core dumped)  ./find
139

因爲第一行在陣列第4個元素就可以找到了,沒有越界
Starting program: /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/find

This GDB supports auto-downloading debuginfo from the following URLs:
  <https://debuginfod.ubuntu.com>
Enable debuginfod for this session? (y or [n]) y
Debuginfod has been enabled.
To make this setting permanent, add 'set debuginfod enabled on' to .gdbinit.
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
找 5：第 4 個

Program received signal SIGSEGV, Segmentation fault.
0x000055555555516b in find (p=0x55555557a000, target=7) at find.c:8
8           while (*p != target)        /* 第 8 行 */
(gdb) info proc mapping
process 30310
Mapped address spaces:

          Start Addr           End Addr       Size     Offset  Perms  objfile
      0x555555554000     0x555555555000     0x1000        0x0  r--p   /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/find
      0x555555555000     0x555555556000     0x1000     0x1000  r-xp   /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/find
      0x555555556000     0x555555557000     0x1000     0x2000  r--p   /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/find
      0x555555557000     0x555555558000     0x1000     0x2000  r--p   /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/find
      0x555555558000     0x555555559000     0x1000     0x3000  rw-p   /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/find
      0x555555559000     0x55555557a000    0x21000        0x0  rw-p   [heap]
      0x7ffff7c00000     0x7ffff7c28000    0x28000        0x0  r--p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7c28000     0x7ffff7db1000   0x189000    0x28000  r-xp   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7db1000     0x7ffff7e00000    0x4f000   0x1b1000  r--p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7e00000     0x7ffff7e04000     0x4000   0x1ff000  r--p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7e04000     0x7ffff7e06000     0x2000   0x203000  rw-p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7e06000     0x7ffff7e13000     0xd000        0x0  rw-p
      0x7ffff7f9e000     0x7ffff7fa1000     0x3000        0x0  rw-p
      0x7ffff7fbb000     0x7ffff7fbd000     0x2000        0x0  rw-p
      0x7ffff7fbd000     0x7ffff7fc1000     0x4000        0x0  r--p   [vvar]
      0x7ffff7fc1000     0x7ffff7fc3000     0x2000        0x0  r--p   [vvar_vclock]
      0x7ffff7fc3000     0x7ffff7fc5000     0x2000        0x0  r-xp   [vdso]
      0x7ffff7fc5000     0x7ffff7fc6000     0x1000        0x0  r--p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7fc6000     0x7ffff7ff1000    0x2b000     0x1000  r-xp   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7ff1000     0x7ffff7ffb000     0xa000    0x2c000  r--p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7ffb000     0x7ffff7ffd000     0x2000    0x36000  r--p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7ffd000     0x7ffff7fff000     0x2000    0x38000  rw-p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffffffdd000     0x7ffffffff000    0x22000        0x0  rw-p   [stack]
  0xffffffffff600000 0xffffffffff601000     0x1000        0x0  --xp   [vsyscall]
(gdb) info frame
Stack level 0, frame at 0x7fffffffd620:
 rip = 0x55555555516b in find (find.c:8); saved rip = 0x5555555551cd
 called by frame at 0x7fffffffd630
 source language c.
 Arglist at 0x7fffffffd610, args: p=0x55555557a000, target=7
 Locals at 0x7fffffffd610, Previous frame's sp is 0x7fffffffd620
 Saved registers:
  rbp at 0x7fffffffd610, rip at 0x7fffffffd618
(gdb) x/8xw data
0x555555558020 <data>:  0x00000003      0x00000001      0x00000004      0x00000001
0x555555558030 <data+16>:       0x00000005      0x00000009      0x00000002      0x00000006

data[0] 的指標是0x555555558020, 當target 設定爲7的時候,data裏面沒有,那在遇到符合7的值之前 p指標會一直以4byte移動,最後移到0x7fffffffd618 kernel踩到程式停止的位址
最後就會變成139,139具體的名字我有點忘了


```

> **批改**：輸出與結束狀態 139 **對**；「第一行在第 4 個元素就找到、沒越界」**對**。兩處要修：
> - **139 代表什麼沒答**（筆記[第五節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#五c-不檢查邊界越界的三種下場)〈結束狀態 139 和 134 是什麼〉）。
> - 「`p` 最後移到 `0x7fffffffd618`」**不對**：那是 `info frame` 裡**返回位址存放的位置**（`rip at 0x7fffffffd618`），在堆疊上，跟 `p` 無關。`p` 當掉時的值就在同一段輸出的第一行：`find (p=0x55555557a000, ...)`。
>
> **提示**：139 = 128 + 幾？那個數字是哪一個訊號？

**第 2 次（重答，2026-09-30）**
> 題目：編譯執行，貼上輸出與結束狀態（`echo $?`）。第一行為什麼是對的？結束狀態代表什麼？
```
139是區段違規 因爲0x55555557a000是rw_p分頁的邊界
```

> **批改（第 2 次）**：**大致對**。「區段違規」就是 segmentation violation（SIGSEGV）；邊界更精確地說是 `[heap]` 的結尾。沒寫出 139 是怎麼算的。
>
> **參考答案**：輸出第一行「找 5：第 4 個」正確，因為 5 在 `data[4]`，迴圈在陣列內就停了，沒有越界。第二行找 7，`data` 裡沒有 7，`p` 一路讀出界，碰到沒對映的記憶體就當掉。
> 結束狀態 **139 = 128 + 11**：shell 用「128 + 訊號編號」表示程式被訊號殺死，11 是 **SIGSEGV**（segmentation fault，區段違規），見筆記[第五節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#五c-不檢查邊界越界的三種下場)〈結束狀態 139 和 134 是什麼〉。

### 4-2. 用 gdb 執行到當掉，貼上 `bt` 的輸出。當在哪個函式、哪一行？是從 `main` 的哪一行呼叫進來的？參數 `target` 是多少？【筆記[第六節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#六用-gdb-找-segfault實際走一次)】

**第 1 次（2026-09-30）**
```
find 第8行, target=7
```

> **批改**：`find`、第 8 行、`target=7` **對**。但**沒有貼 `bt` 的輸出**，也沒答「從 `main` 的哪一行呼叫進來」——這一半正是 `bt` 才看得到的（第 4-1 題貼的是當掉時的那一行，不是 `bt`）。
>
> **提示**：在 gdb 當掉後打 `bt`，看 `#1` 那一行（筆記[第六節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#六用-gdb-找-segfault實際走一次)第 2 步）。

**第 2 次（重答，2026-09-30）**
> 題目：用 gdb 執行到當掉，貼上 `bt` 的輸出。當在哪個函式、哪一行？是從 `main` 的哪一行呼叫進來的？參數 `target` 是多少？
```
find 第8行, target=7
(gdb) run
Starting program: /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/find

This GDB supports auto-downloading debuginfo from the following URLs:
  <https://debuginfod.ubuntu.com>
Enable debuginfod for this session? (y or [n]) y
Debuginfod has been enabled.
To make this setting permanent, add 'set debuginfod enabled on' to .gdbinit.
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
找 5：第 4 個

Program received signal SIGSEGV, Segmentation fault.
0x000055555555516b in find (p=0x55555557a000, target=7) at find.c:8
8           while (*p != target)        /* 第 8 行 */
(gdb) bt
#0  0x000055555555516b in find (p=0x55555557a000, target=7) at find.c:8
#1  0x00005555555551cd in main () at find.c:17
```

> **批改（第 2 次）**：**對**。`bt` 貼了：當在 `find` 第 8 行（`#0`），由 `main` 第 17 行呼叫進來（`#1`），`target=7`。

### 4-3. 在當掉的地方用 `print p - start` 算出 `p` 已經走過幾個元素，再用 `info proc mappings` 找出 `p` 的位址落在哪一段的邊界上。貼上相關的輸出，並用筆記[第六節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#六用-gdb-找-segfault實際走一次)的方式驗算一次（起點位址 + 元素個數 × 每個元素的大小 = 當掉的位址）。【筆記[第六節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#六用-gdb-找-segfault實際走一次)】

**第 1 次（2026-09-30）**
```
(gdb) print p -start
$2 = 34808

(gdb) x p
0x55555557a000: Cannot access memory at address 0x55555557a00

(gdb) info proc mapping
process 30310
Mapped address spaces:

          Start Addr           End Addr       Size     Offset  Perms  objfile
      0x555555554000     0x555555555000     0x1000        0x0  r--p   /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/find
      0x555555555000     0x555555556000     0x1000     0x1000  r-xp   /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/find
      0x555555556000     0x555555557000     0x1000     0x2000  r--p   /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/find
      0x555555557000     0x555555558000     0x1000     0x2000  r--p   /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/find
      0x555555558000     0x555555559000     0x1000     0x3000  rw-p   /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/find
      0x555555559000     0x55555557a000    0x21000        0x0  rw-p   [heap]
      0x7ffff7c00000     0x7ffff7c28000    0x28000        0x0  r--p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7c28000     0x7ffff7db1000   0x189000    0x28000  r-xp   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7db1000     0x7ffff7e00000    0x4f000   0x1b1000  r--p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7e00000     0x7ffff7e04000     0x4000   0x1ff000  r--p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7e04000     0x7ffff7e06000     0x2000   0x203000  rw-p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7e06000     0x7ffff7e13000     0xd000        0x0  rw-p
      0x7ffff7f9e000     0x7ffff7fa1000     0x3000        0x0  rw-p
      0x7ffff7fbb000     0x7ffff7fbd000     0x2000        0x0  rw-p
      0x7ffff7fbd000     0x7ffff7fc1000     0x4000        0x0  r--p   [vvar]
      0x7ffff7fc1000     0x7ffff7fc3000     0x2000        0x0  r--p   [vvar_vclock]
      0x7ffff7fc3000     0x7ffff7fc5000     0x2000        0x0  r-xp   [vdso]
      0x7ffff7fc5000     0x7ffff7fc6000     0x1000        0x0  r--p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7fc6000     0x7ffff7ff1000    0x2b000     0x1000  r-xp   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7ff1000     0x7ffff7ffb000     0xa000    0x2c000  r--p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7ffb000     0x7ffff7ffd000     0x2000    0x36000  r--p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7ffd000     0x7ffff7fff000     0x2000    0x38000  rw-p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffffffdd000     0x7ffffffff000    0x22000        0x0  rw-p   [stack]
  0xffffffffff600000 0xffffffffff601000     0x1000        0x0  --xp   [vsyscall]

  落在這裏0x55555557a000剛好是 rw-p分頁的邊界

  (gdb) x/8xw data
0x555555558020 <data>:  0x00000003      0x00000001      0x00000004      0x00000001
0x555555558030 <data+16>:       0x00000005      0x00000009      0x00000002      0x00000006

起點在0x555555558020 + 4 * 34808 = 0x55555557a000(我沒實際算,16進制我不知道該怎麼計算)

```

> **批改**：**大致對**。`p - start` = 34808、`p` 停在 `0x55555557a000`、式子都對。邊界要講名字：`0x55555557a000` 是 **`[heap]` 的結尾**（End Addr，不含）。
> 十六進位不必手算，交給工具（已補教進筆記〈[作業第 4、5 題的補教](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#作業第-45-題的補教當掉的位置由哪裡沒有對映決定)〉第三件）：
> ```
> $ printf '%#x\n' $((0x555555558020 + 4*34808))
> 0x55555557a000
> (gdb) print/x 0x555555558020 + 4*34808
> $3 = 0x55555557a000
> ```
> 驗算成立：34808 × 4 = 139232 = `0x21fe0`，`0x555555558020 + 0x21fe0 = 0x55555557a000`。

### 4-4. 用 `print &data` 找出 `data` 在哪一段（看 `info proc mappings` 的那一段是什麼、權限是什麼）。`p` 當掉的位置**不是** `data` 所在那一段的結尾，而是再往後另一段的結尾。那一段叫什麼名字？為什麼 `p` 走出 `data` 那一段的時候沒有當掉？【前半筆記[第六節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#六用-gdb-找-segfault實際走一次)；後半【新觀念，先猜】】

**第 1 次（2026-09-30）**
```
上面題目的回答都有貼,我用上面的輸出回答先
rw_p

我不確定那一段叫什麼名字?你是說heap嗎?

應該是因爲沒有踩到金絲雀, 與結束點的記憶體
```

> **批改**：
> - 前半**不完整**：`rw-p` 對，但要說是**哪一段**——`data` 在 `0x555555558000`～`0x555555559000`、檔名是 `find` 的那一段，也就是執行檔自己的 `.data` 所在的載入段。
> - 那一段的名字：**`[heap]`**，你猜對了。
> - 後半【新觀念，先猜】：猜「沒踩到金絲雀」不對，但猜錯不算錯。補教（已寫進筆記〈[作業第 4、5 題的補教](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#作業第-45-題的補教當掉的位置由哪裡沒有對映決定)〉第一件）：
>
> **參考答案**：`.data` 那一段結束在 `0x555555559000`，**`[heap]` 剛好從同一個位址開始**，中間沒有空隙；`p` 走出 `.data` 時踏進的是有對映的 `[heap]`，所以沒當，
> 一直讀到 `[heap]` 結尾才碰到沒對映的地方。金絲雀只保護堆疊上的返回位址，跟全域變數無關。
> **而且這只在 gdb 裡成立**：gdb 預設關掉 ASLR，`[heap]` 才緊貼在後面。本機用 `set disable-randomization off` 實測三次，`[heap]` 都離很遠，`p - start` 都是 **1016**——一走出 `.data` 那一頁（`0x1000 − 0x20` = 4064 byte）就當。

### 4-5. 真正的 bug 在哪一行？為什麼當掉的那一行（第 8 行）不是犯錯的那一行？把 `find` 改成不會越界的版本（提示：想想第三節說的「長度要另外傳」；找不到時要回傳什麼，你自己決定並說明），貼上改好的程式與執行結果。【筆記[第三節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#三陣列名稱什麼時候變成指標什麼時候不會)、[第六節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#六用-gdb-找-segfault實際走一次)】

**第 1 次（2026-09-30）**
```
第9行
因爲0x55555557a000+1 第9行越界,第8行的時候還是0x55555557a000 只是剛好在邊界
./find_fix                                                                           ok | base py | at 15:29:51
找 5：第 4 個
找 7：未找到
程式在find_fix.c
```

> **批改**：改好的程式**對**：`len-- > 0` 讓迴圈最多走 `len` 格，找不到時回傳 `data + 8`（剛好超出結尾的位置，C 允許指到這裡、但不能讀它），`main` 用索引 `< 8` 判斷找到沒有；本機用 `-fsanitize=address` 跑也沒有報錯。
> 小地方：題目要你**說明找不到時回傳什麼**，你沒寫；編譯時有兩個 `-Wsign-compare` 警告（`ptrdiff_t` 跟 `sizeof` 的 `size_t` 比較——正是筆記第三節〈算元素個數再跑迴圈〉的陷阱）；`find` 每個數字呼叫了兩次，可以先存進變數。
> 「真正的 bug 在哪一行」**錯**：第 9 行 `p++` 只是照指示往前走；而且當掉是在第 8 行**讀** `*p` 的時候，不是第 9 行。
>
> **提示**【筆記[第六節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#六用-gdb-找-segfault實際走一次)〈當掉的那一行 ≠ 犯錯的那一行〉】：`find` 從哪裡得知 `data` 有幾個元素？是誰在「不知道長度」的情況下叫它去找一個**不存在**的值？（看 `bt` 的 `#1`。）當掉的第 8 行只是「第一次碰到沒對映記憶體」的地方。

**第 2 次（重答，2026-09-30）**
> 題目：真正的 bug 在哪一行？為什麼當掉的那一行（第 8 行）不是犯錯的那一行？（程式與執行結果已經對了，這次只回答這兩問，並補一句「找不到時回傳什麼、為什麼這樣設計」。）
```
第17行
因爲第17行沒有給他範圍

找不到時回傳null,如果是因爲他不可能是null,我已經在find_fix.c改寫了
```

> **批改（第 2 次）**：「bug 在第 17 行、因為沒給範圍」**大致對**；「為什麼第 8 行不是犯錯的那一行」沒有正面回答。回傳 `NULL` 表示找不到，這個設計**好**（`NULL` 不可能是陣列裡的有效位置，呼叫者一看就知道）。
> 但**新版 `find_fix.c` 多了一個越界讀取**：找不到時迴圈結束，`p` 已經是 `data + 8`（剛好超出結尾），第 11 行 `if (*p != target)` 又去**讀**它。本機 ASan 實測：
> ```
> $ gcc -Wall -Wextra -g -O0 -fsanitize=address find_fix.c -o find_fix && ./find_fix
> 找 5：第 4 個
> ==48768==ERROR: AddressSanitizer: global-buffer-overflow on address 0x59f1158b8040 ...
> READ of size 4 at 0x59f1158b8040 thread T0
>     #0 0x59f1158b52b3 in find find_fix.c:11
>     #1 0x59f1158b5352 in main find_fix.c:25
> 0x59f1158b8040 is located 0 bytes after global variable 'data' defined in 'find_fix.c:3:5' (0x59f1158b8020) of size 32
> ```
> 一般編譯看不出來（只多讀 4 byte，碰到的是有對映的記憶體），印出的結果也對——又是「結果對、程式錯」。這就是為什麼改完要用 ASan 再**執行**一次。
> （另外第 11 行後面那段註解是原本給 `(void)start;` 的，搬錯位置了。）
>
> **參考答案**：
> - 真正的 bug 是 **`find` 的設計**：它只拿到起點，**不知道陣列有多長**，只能一直往前讀到碰巧遇到 `target`；而 `main` 第 17 行叫它找一個陣列裡**不存在**的 7，它就停不下來。錯在「沒有長度資訊」與第 17 行的呼叫，不在 `find` 的某一行運算。
> - 第 8 行只是「**第一次讀到沒對映記憶體**」的地方：`p` 從走出 `data` 那一刻就已經錯了，前面三萬多次越界讀取都沒當，當在哪裡只取決於旁邊的記憶體有沒有對映（筆記[第六節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#六用-gdb-找-segfault實際走一次)〈當掉的那一行 ≠ 犯錯的那一行〉）。
> - 修法：傳入長度，迴圈最多走 `len` 格；找不到時回傳 `NULL`。判斷「找到沒有」要看**迴圈是不是走完了全部 `len` 格**（例如記下結尾位置 `data + len`，比較 `p` 有沒有到達那裡），**不能再去讀 `*p`**——走完時 `p` 已經指在陣列外面。

---

## 5. 倒著走的迴圈與 `size_t`

寫 `back.c`：宣告 `int a[5] = {1, 2, 3, 4, 5};`，用下面這個迴圈**倒著**印出每個元素：

```c
for (size_t i = sizeof a / sizeof a[0] - 1; i >= 0; i--)
    printf("%d\n", a[i]);
```

### 5-1. **先預測**：只用 `-Wall` 編譯會不會警告？加上 `-Wextra` 呢？如果有警告，它在說什麼、為什麼？預測完再編譯驗證。【第 2 課筆記[第七節](../../c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#七陷阱有號和無號混在一起比較)〈[`-Wtype-limits`](../../c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#-wtype-limits-只抓永遠成立永遠不成立的比較)〉】

**第 1 次（2026-09-30）**
```
不會
有
sizeof 的類型是unsigned long , -1 就是+ -1 因爲C語言沒有負數, 但-1 原始型別是int, int跟 unsigned long比,最大值是unsigend long最大,所以會以他的爲主,那-1就是最大的unsigned long,二補數回補,那就會永遠大於0了

back.c: In function ‘main’:
back.c:6:51: warning: comparison of unsigned expression in ‘>= 0’ is always true [-Wtype-limits]
    6 |     for (size_t i = sizeof a / sizeof a[0] - 1; i >= 0; i--)
      |
```

> **批改**：兩個預測（`-Wall` 不警告、`-Wextra` 會）**對**，警告內容也貼了。原因**大致對**但混進了一個錯誤：
> - 對的部分：`size_t` 是無號，**無號數永遠 ≥ 0**，所以 `i >= 0` 永遠成立，`-Wextra` 裡的 `-Wtype-limits` 抓的就是這種「永遠成立」的比較。
> - 不對的部分：初始值 `sizeof a / sizeof a[0] - 1` 是 **4**（5 − 1），不是「最大的 unsigned long」。這裡的 `1` 會轉成 `unsigned long` 的 1，5 − 1 = 4，沒有回繞。回繞是之後 `i` 從 0 再 `i--` 時才發生（第 5-3 題）。

### 5-2. **先預測**印完 `5 4 3 2 1` 之後會發生什麼，再執行 `./back | head -10`、`./back > /dev/null; echo $?`、`./back | wc -l`，貼上結果。印了大約幾行才停？結束狀態代表什麼？（「為什麼剛好是這麼多行才當掉」：【新觀念，先猜】，提示：`ulimit -s` 會印出一個數字，單位是 KB）

**第 1 次（2026-09-30）**
```
會瘋狂迴圈,指針不斷不斷移動4byte,直到移動到邊界才停


 ~/w/Proj/g/good-note/s/s1-03-arrays-pointers | on main >8 !1 ?3  ./back | head -10                                                                   INT | base py | at 15:41:59
5
4
3
2
1
-1
-2
0
0
25028

~/w/Proj/g/good-note/s/s1-03-arrays-pointers | on main >8 !1 ?3  ./back > /dev/null; echo $?                                                   PIPE|0 ok | base py | at 15:42:11
[1]    40865 segmentation fault (core dumped)  ./back > /dev/null
139

~/w/Proj/g/good-note/s/s1-03-arrays-pointers | on main >8 !1 ?3  ./back | wc -l                                                         ok | took 5m 20s | base py | at 15:49:50
2093475

我猜是指標跑到了記憶體分頁邊界,只是這是往回跑

```

> **批改**：預測「一直跑到邊界才停」**對**；三個指令的結果都貼了：約 **209 萬行**（2093475），結束狀態 139。139 的意思同第 4-1 題。
> 「為什麼剛好這麼多行」【新觀念，先猜】：猜「往回跑到記憶體邊界」**方向對**，缺的是那個邊界為什麼在這裡。補教（筆記〈[作業第 4、5 題的補教](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#作業第-45-題的補教當掉的位置由哪裡沒有對映決定)〉第二件）：
>
> **參考答案**：`a` 在堆疊上，迴圈是往**低位址**倒著讀。堆疊往低位址長，起初只對映 136 KB，但往下讀時**核心會自動把堆疊延長**，直到上限 `ulimit -s`（本機 8192 KB ＝ 8 MB）。
> 209 萬 × 4 byte ≈ 8.37 MB，約等於 8 MB 的上限（`a` 上面還有環境變數等），超過上限核心不再延長，才 segfault。

### 5-3. 用 `-fsanitize=address` 編譯執行，貼上報告的前 5 行。報告說的是 `underflow`（在陣列**前面**），不是 `overflow`。解釋：(1) `i` 是 0 的時候再做 `i--`，`i` 變成多少？【第 2 課筆記[第四節](../../c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#四無號整數二進位與回繞)】 (2) 為什麼 `a[i]` 碰到的是 `a` 前面緊鄰的那一格，而不是很遠很遠的地方？【新觀念，先猜；提示：`a[i]` 是 `*(a + i)`，位址是 `a` 加上 `i × 4`，而位址只有 64 bit】

**第 1 次（2026-09-30）**
```
==41806==ERROR: AddressSanitizer: stack-buffer-underflow on address 0x7f318340001c at pc 0x566d642d543a bp 0x7fff61e70e40 sp 0x7fff61e70e30
READ of size 4 at 0x7f318340001c thread T0
    #0 0x566d642d5439 in main /home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/back.c:7
    #1 0x7f318542a1c9 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
    #2 0x7f318542a28a in __libc_start_main_impl ../csu/libc-start.c:360
    #3 0x566d642d5164 in _start (/home/set/work/Projects/github/good-note/small-project/s1-03-arrays-pointers/back+0x1164) (BuildId: 684fe8e707909772cfd48dafb2a2e6bbfba5e24b)

(1) 18446744073709551615
(2) 不確定

```

> **批改**：ASan 報告**對**（`stack-buffer-underflow`、`READ of size 4`、`back.c:7`）。(1) **對**：`SIZE_MAX` = 2⁶⁴ − 1。
> (2)【新觀念，先猜】沒猜，直接補教（筆記〈[作業第 4、5 題的補教](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#作業第-45-題的補教當掉的位置由哪裡沒有對映決定)〉第二件）：
>
> **參考答案**：`a[i]` 的位址是 `a + i × 4`。`i = 2⁶⁴ − 1` 時，`i × 4 = 2⁶⁶ − 4`，位址只有 64 bit，超出的部分被丟掉（回繞），剩下 2⁶⁴ − 4，也就是「−4」。
> 所以 `a + i × 4` 等於 **`a − 4`**，正是 `a[-1]`：`a` 前面緊鄰的那一格。這跟第 2 課「無號整數回繞」是同一件事，只是發生在位址運算上。

### 5-4. 把迴圈改成正確的版本，**`i` 仍然用 `size_t`**，並說明你的改法為什麼不會再出錯。用 `-Wall -Wextra` 和 `-fsanitize=address` 驗證，貼上程式與結果。（沒有唯一的答案；想不出來可以先寫一個用 `int` 的版本，並說明 `int` 版本在什麼情況下會不夠用。）

**第 1 次（2026-09-30）**
```
我改在back_fix.c
因爲當i小於0在減,會回退,變更大,i是unsigned long,所以>=0永遠成立,但我只需要在加一個條件 不然i的範圍超過4就好
 ~/w/Proj/g/good-note/s/s1-03-arrays-pointers | on main >8 !1 ?3  gcc -Wall -Wextra -g -O0 -o back_fix back_fix.c                                   1 err | base py | at 15:52:42
back_fix.c: In function ‘main’:
back_fix.c:6:51: warning: comparison of unsigned expression in ‘>= 0’ is always true [-Wtype-limits]
    6 |     for (size_t i = sizeof a / sizeof a[0] - 1; i >= 0 && i < sizeof a / sizeof a[0]; i--)
      |                                                   ^~

 ~/w/Proj/g/good-note/s/s1-03-arrays-pointers | on main >8 !1 ?4  gcc -Wall -Wextra -g -O0 -fsanitize=address -o back_fix back_fix.c                   ok | base py | at 16:16:36
back_fix.c: In function ‘main’:
back_fix.c:6:51: warning: comparison of unsigned expression in ‘>= 0’ is always true [-Wtype-limits]
    6 |     for (size_t i = sizeof a / sizeof a[0] - 1; i >= 0 && i < sizeof a / sizeof a[0]; i--)
```

> **批改**：**對**。加上 `i < 5`：`i` 從 0 再減 1 回繞成 `SIZE_MAX`，`i < 5` 不成立，迴圈就停——正是利用回繞本身來判斷。本機用 ASan 跑，印出 5 4 3 2 1、exit 0，沒有報錯。
> 兩個建議：
> - 你沒有貼 ASan 版**執行**的結果，只有編譯（跟 2-3 題一樣，ASan 要執行才會檢查）。
> - 那個警告還在，因為 `i >= 0` 還留著。它永遠成立、沒有作用，刪掉只留 `i < 5`，警告就消失了。另一種常見寫法是 `for (size_t i = n; i-- > 0; )`：先比較、再減，`i` 在迴圈本體裡是 4、3、2、1、0。
