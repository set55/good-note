# 2026-11-06 複習：函式庫、GNU、C資料、組語、字元、hello

> 依[複習排程](../learning/複習排程.md)出今天到期的全部題目（每天出完所有到期題，2026-10-23 起的規則），共 **8 題**（17 個作答框），沒有過期的。
> 函式庫 11 是 11-05 不完整（11-4 錯）、回到盒子 1；函式庫 7 是 11-03 大致對、留在盒子 2；GNU 6（大致對，留盒子 3）、C資料 4、組語 18（對，升盒子 3）是 10-30 作答；字元 2、hello 1、C資料 2 是 10-23 答對升到盒子 4。
> 題目依筆記分節、節內照題號排（盒子小的筆記排前面）；每題照原題複製，用到的程式碼與實驗步驟都附在題目裡，「過去的作答」連到之前每一次作答（含批改與參考答案）。
> **補問「為什麼」**（2026-11-04 起的出題規則，筆記〈自我測驗〉的原題同步改了）：C資料 2（本機與大端序的 `*b` 各是為什麼）。
> 目前在第 8 課的作業階段，到期題裡沒有跟第 8 課直接相關的，所以照盒子排。
>
> **怎麼寫**：闔上筆記，每題寫 1～2 句關鍵句就好。可以分幾次寫，寫完一部分就可以先說「幫我批改」。複習不重答，批改會直接附參考答案。
> 已拆小題的題目，每個小題一個作答框；以大題為單位排程，小題全對才算對。

## 今天的題目

排序：盒子小的在前；同一盒子照筆記與題號。

| # | 題目 | 盒子 | 原本該考的日期 |
|---|---|---|---|
| 1 | 函式庫 11（4 小題） | 1 | 2026-11-06 |
| 2 | 函式庫 7（4 小題） | 2 | 2026-11-06 |
| 3 | GNU 6 | 3 | 2026-11-06 |
| 4 | C資料 4 | 3 | 2026-11-06 |
| 5 | 組語 18（4 小題） | 3 | 2026-11-06 |
| 6 | C資料 2 | 4 | 2026-11-06 |
| 7 | 字元 2 | 4 | 2026-11-06 |
| 8 | hello 1 | 4 | 2026-11-06 |

---

## 函式庫：[靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html)

### 函式庫 7. 兩份同名全域變數：接 `.so` 為什麼不算重複定義（10-25 追問新增）

> 盒子 2｜過去的作答：[s1-07 quiz.md 第 7 題](../small-project/s1-07-libraries/quiz.md#7-兩份同名全域變數接-so-為什麼不算重複定義10-25-追問新增)（10-29 第一次作答）、[11-03 複習](./2026-11-03-函式庫-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#函式庫-7-兩份同名全域變數接-so-為什麼不算重複定義10-25-追問新增)

`libconf.so` 和主程式 `app.c` 都定義了 `verbose`。(1) `gcc -Wall app.c -L. -lconf -Wl,-rpath,'$ORIGIN' -o app` 會不會出現 `multiple definition`？為什麼？改成 `gcc -Wall app.c conf.pic.o -o app2` 呢？差別在 `ld` 怎麼處理 `.so` 和 `.o`。(2) 能連結成功的那一個，`./app` 會印出 `get_verbose() = ` 多少？用執行時 `ld.so` 怎麼找符號解釋。(3) 把 `conf.c` 第一行改成 `static int verbose = 0;`，重做 `libconf.so`，不重新連結就執行 `./app`，印出多少？為什麼跟 (2) 不同？

```c
/* conf.c */
int verbose = 0;

int get_verbose(void)
{
    return verbose;
}
```
```c
/* app.c */
#include <stdio.h>

int verbose = 3;
int get_verbose(void);

int main(void)
{
    printf("get_verbose() = %d\n", get_verbose());
    return 0;
}
```
做函式庫：`gcc -Wall -fPIC -c conf.c -o conf.pic.o && gcc -shared conf.pic.o -o libconf.so`（檔案：[`conf.c`](../small-project/s1-07-libraries/examples/pic/conf.c)、[`app.c`](../small-project/s1-07-libraries/examples/pic/app.c)）

#### 函式庫 7-1. `gcc -Wall app.c -L. -lconf -Wl,-rpath,'$ORIGIN' -o app` 會不會出現 `multiple definition`？為什麼？

```
不會,因爲動態載入器 會在.so與主程式之間挑一個,主程式優先,以本題爲例,會用app.c的那格verbose, 而ld.so就會在libconf.so的got
把app.c verbose的位址填入.got裏
```

#### 函式庫 7-2. 改成 `gcc -Wall app.c conf.pic.o -o app2` 呢？差別在 `ld` 怎麼處理 `.so` 和 `.o`。

```
這個就有雙重定義的錯誤了,因爲 兩個 app.o , conf.pic.o 兩個同時都有verbose的定義,ld不知道改選哪一個,這兩個.o都是要放進去執行檔的
```

#### 函式庫 7-3. 能連結成功的那一個，`./app` 會印出 `get_verbose() = ` 多少？用執行時 `ld.so` 怎麼找符號解釋。

```
3,
ld.so 看到verbose這個符號,會去看誰有定義,順序是先主程式,纔是so, 主程式有定義verbose 是 3, 因此挑這個3的位址,寫到了conf.c
的got
```

#### 函式庫 7-4. 把 `conf.c` 第一行改成 `static int verbose = 0;`，重做 `libconf.so`，不重新連結就執行 `./app`，印出多少？為什麼跟 7-3 不同？

```
0
因爲.so的verbose已經是靜態變數了,不需要在用got了,組語會直接寫,找%rip到 static verbose的距離,因此是0
一定是這樣因爲在編.so的時候還沒有app.c的verbose,所以最後一定是找跟static verbose的距離
```

---

### 函式庫 11. 主程式只寫 `extern`：變數放在哪裡（10-26 追問新增）

> 盒子 1｜過去的作答：[s1-07 quiz.md 第 11 題](../small-project/s1-07-libraries/quiz.md#11-主程式只寫-extern變數放在哪裡10-26-追問新增)（10-29 第一次作答）、[11-01 複習](./2026-11-01-函式庫-組語.md#函式庫-11-主程式只寫-extern變數放在哪裡10-26-追問新增)、[11-02 複習](./2026-11-02-函式庫-interp-C資料-字元-陣列-組語-組語二-多檔.md#函式庫-11-主程式只寫-extern變數放在哪裡10-26-追問新增)、[11-05 複習](./2026-11-05-函式庫-C資料-hello-GNU-objdump-陣列-組語.md#函式庫-11-主程式只寫-extern變數放在哪裡10-26-追問新增)

`libcfg.so` 定義了 `int level = 3;` 與 `get_level()`，主程式 `setlevel.c` 只寫了 `extern int level;`。(1) 執行後兩行各印多少？(2) `nm setlevel` 會不會看到 `level`？在哪一種區段？為什麼原始碼只有 `extern` 卻會有？(3) 第一行印出的值是怎麼放進那裡的？`get_level()` 讀的是哪一份 `level`？用重定位的類型和 `ld.so` 找符號的順序說明。(4) 之後把 `cfg.c` 改成 `long level = 3;` 重做 `libcfg.so`，`setlevel` 不重新連結就執行，會發生什麼事、為什麼？

```c
/* cfg.c */
int level = 3;

int get_level(void)
{
    return level;
}
```
```c
/* setlevel.c */
#include <stdio.h>

extern int level;
int get_level(void);

int main(void)
{
    printf("一開始 level = %d\n", level);
    level = 7;
    printf("get_level() = %d\n", get_level());
    return 0;
}
```
編譯：`gcc -Wall -fPIC -c cfg.c -o cfg.pic.o && gcc -shared cfg.pic.o -o libcfg.so && gcc -Wall setlevel.c -L. -lcfg -Wl,-rpath,'$ORIGIN' -o setlevel`（檔案：[`cfg.c`](../small-project/s1-07-libraries/examples/pic/cfg.c)、[`setlevel.c`](../small-project/s1-07-libraries/examples/pic/setlevel.c)）。主程式是 Ubuntu 預設的 `-fPIE`。

#### 函式庫 11-1. 執行後兩行各印多少？

```
3, 7
```

#### 函式庫 11-2. `nm setlevel` 會不會看到 `level`？在哪一種區段？為什麼原始碼只有 `extern` 卻會有？

```
會
在bss
因爲主程式在編譯的時候使用fPIE, 不像fPIC可以用got動態指定level的位址, 所以原始碼一定是把level定義在自己這邊的位址,這樣
%rip與level的距離就是固定的,libcfg是fPIC,所以他的got只要存住setlevel的level位址就好
```

#### 函式庫 11-3. 第一行印出的值是怎麼放進那裡的？`get_level()` 讀的是哪一份 `level`？用重定位的類型和 `ld.so` 找符號的順序說明。

```
level 在 setlevel的重定位類型是R_X86_64_COPY, 所以ld.so看到以後就會把libcfg.so裏面的level初值複製到 這裏
而在libcfg.so 在執行到get_level的時候會去.got找level的位址,因爲level的重定位是R_X86_64_GLOB_DAT,ld.so也會把level在setlevel的位址寫到libcfg.so的got
```

#### 函式庫 11-4. 之後把 `cfg.c` 改成 `long level = 3;` 重做 `libcfg.so`，`setlevel` 不重新連結就執行，會發生什麼事、為什麼？

```
./setlevel: Symbol `level' has different size in shared object, consider re-linking
一開始 level = 3
get_level() = 7

因爲libcfg.so有重新編,所以他的level大小是8byte, 而setlevel他的level還是維持4byte,所以會有這個警告
本題因爲level太小看不出來,但當level夠大,就會發現有截斷的情況
```

---

## GNU：[GNU是什麼-Linux核心與GNU工具的分工](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html)

### GNU 6. 情境題：同事說「我在這台機器上打 `uname -o` 看到 `GNU/Linux`，所以它一定用 glibc」。這個推論錯在哪？`uname` 的哪些欄位可以信、哪一欄不行，為什麼？你會改看什麼來判斷？

> 盒子 3｜過去的作答：[s1-01 quiz.md GNU 第 6 題](../small-project/s1-01-hello-compile/quiz.md#6-情境題同事說我在這台機器上打-uname--o-看到-gnulinux所以它一定用-glibc這個推論錯在哪uname-的哪些欄位可以信哪一欄不行為什麼你會改看什麼來判斷)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-30 複習](./2026-10-30-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語.md#gnu-6-情境題同事說我在這台機器上打-uname--o-看到-gnulinux所以它一定用-glibc這個推論錯在哪uname-的哪些欄位可以信哪一欄不行為什麼你會改看什麼來判斷)

```
```

---

## C資料：[C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html)

### C資料 2. `int x = 0x12345678; unsigned char *b = (unsigned char *)&x;` 在本機 `*b` 是多少？換到大端序的機器上是多少？各是為什麼？另外，為什麼 `int *p = &x;` 的 `*p` 在兩種機器上都是 `0x12345678`？

> 盒子 4｜過去的作答：[s1-02 quiz.md 第 2 題](../small-project/s1-02-data-repr/quiz.md#2-int-x--0x12345678-unsigned-char-b--unsigned-char-x-在本機-b-是多少換到大端序的機器上是多少為什麼-int-p--x-的-p-在兩種機器上都是-0x12345678)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#c資料-2-int-x--0x12345678-unsigned-char-b--unsigned-char-x-在本機-b-是多少換到大端序的機器上是多少各是為什麼另外為什麼-int-p--x-的-p-在兩種機器上都是-0x12345678)

**題目用到的程式**：筆記[第二節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#二指標就是位址)的 `addr.c`：

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

```sh
$ gcc -Wall -g -O0 -o addr addr.c
$ ./addr
```

> 題目：`int x = 0x12345678; unsigned char *b = (unsigned char *)&x;` 在本機 `*b` 是多少？換到大端序的機器上是多少？各是為什麼？另外，為什麼 `int *p = &x;` 的 `*p` 在兩種機器上都是 `0x12345678`？

```
```

---

### C資料 4. `int y = x + 1; return y > x;` 在 `x = INT_MAX` 時，`-O0` 回傳 0、`-O2` 回傳 1。為什麼會這樣？這算不算 gcc 的 bug？kernel 用哪個選項避免這種情況，代價是什麼？

> 盒子 3｜過去的作答：[s1-02 quiz.md 第 4 題](../small-project/s1-02-data-repr/quiz.md#4-int-y--x--1-return-y--x-在-x--int_max-時-o0-回傳-0-o2-回傳-1為什麼會這樣這算不算-gcc-的-bugkernel-用哪個選項避免這種情況代價是什麼)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-30 複習](./2026-10-30-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語.md#c資料-4-int-y--x--1-return-y--x-在-x--int_max-時-o0-回傳-0-o2-回傳-1為什麼會這樣這算不算-gcc-的-bugkernel-用哪個選項避免這種情況代價是什麼)

題目用到的程式是筆記[第八節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#八溢位無號回繞有號是未定義行為)的 `ub.c`，完整原始碼：

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

編譯與執行：`gcc -O0 ub.c -o ub && ./ub`、`gcc -O2 ub.c -o ub && ./ub`（結果如題目所述，分別印 0 與 1）。

> 題目：`int y = x + 1; return y > x;` 在 `x = INT_MAX` 時，`-O0` 回傳 0、`-O2` 回傳 1。為什麼會這樣？這算不算 gcc 的 bug？kernel 用哪個選項避免這種情況，代價是什麼？

```
```

---

## 組語：[x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)

### 組語 18. `0x01 + 0x7f` 的進位與 CF、OF

> 盒子 3｜過去的作答：2026-10-25 在對話裡作答（18-1 對、18-2 大致對、18-3 大致對、18-4 未作答），批改後的參考答案寫在[筆記第 18 題](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#自我測驗)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-30 複習](./2026-10-30-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語.md#組語-18-0x01--0x7f-的進位與-cfof)

#### 組語 18-1. 8 位元的 `0x01 + 0x7f`：計算過程中有沒有進位傳進第 7 位元（最高位）？有沒有從第 7 位元往外進位？

```
```

#### 組語 18-2. CF、OF 各是多少，為什麼「有進位」不等於 CF = 1？

> 題目：（承 18-1，8 位元的 `0x01 + 0x7f`）CF、OF 各是多少，為什麼「有進位」不等於 CF = 1？

```
```

#### 組語 18-3. 再比較 `0x05 + 0x03`：它也有進位，為什麼兩個旗標都是 0？

```
```

#### 組語 18-4. 最後用「進到第 7 位的進位」和「第 7 位往外的進位」說明 `0xff + 0x01` 的 OF 為什麼是 0。

```
```

---

## 字元：[字元是什麼-char就是1byte的整數與ASCII和UTF-8](../html/c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.html)

### 字元 2. 使用者輸入的字元是 `'7'`，你想得到數值 7。直接把它當數字用會得到多少？該怎麼轉，這個方法為什麼成立？

> 盒子 4｜過去的作答：[s1-02 quiz.md 字元第 2 題](../small-project/s1-02-data-repr/quiz.md#2-使用者輸入的字元是-7你想得到數值-7直接把它當數字用會得到多少該怎麼轉這個方法為什麼成立)、[10-05 複習](./2026-10-05-gcc-陣列-GNU-C資料-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#字元-2-使用者輸入的字元是-7你想得到數值-7直接把它當數字用會得到多少該怎麼轉這個方法為什麼成立)

```
```

---

## hello：[hello.c怎麼變成能跑的程式-編譯的四個階段](../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html)

### hello 1. `gcc hello.c` 背後實際是哪幾個程式在接力？為什麼說 `gcc` 是「總指揮」？

> 盒子 4｜過去的作答：[s1-01 quiz.md 第 1 題](../small-project/s1-01-hello-compile/quiz.md#1-gcc-helloc-背後實際是哪幾個程式在接力為什麼說-gcc-是總指揮)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#hello-1-gcc-helloc-背後實際是哪幾個程式在接力為什麼說-gcc-是總指揮)

```
```
