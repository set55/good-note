# 2026-10-26 複習：hello、GNU、interp、objdump、C資料、字元、陣列、組語、組語二、多檔

> 依[複習排程](../learning/複習排程.md)出今天到期的全部題目（每天出完所有到期題，2026-10-23 起的規則），共 **20 題**（41 個作答框），沒有過期的。
> 其中 17 題是 10-23 大複習後排在盒子 2 的題目（含 4 題大致對留在盒子 2：hello 5、組語 10、陣列 2、陣列 3）；組語 4、組語 13 是 10-19 答對、排在今天的盒子 3；組語 18 是 10-25 追問新增、在對話裡第一次作答後進盒子 1。
> 題目依筆記分節、節內照題號排；每題照原題複製，用到的程式碼與實驗步驟都附在題目裡，「過去的作答」連到之前每一次作答（含批改與參考答案）。
>
> **怎麼寫**：闔上筆記，每題寫 1～2 句關鍵句就好。可以分幾次寫，寫完一部分就可以先說「幫我批改」。複習不重答，批改會直接附參考答案。
> 已拆小題的題目，每個小題一個作答框；以大題為單位排程，小題全對才算對。

## 今天的題目

| # | 題目 | 盒子 | 原本該考的日期 |
|---|---|---|---|
| 1 | 組語 18（4 小題） | 1 | 2026-10-26 |
| 2 | hello 5 | 2 | 2026-10-26 |
| 3 | GNU 4 | 2 | 2026-10-26 |
| 4 | interp 2 | 2 | 2026-10-26 |
| 5 | interp 3 | 2 | 2026-10-26 |
| 6 | objdump 3 | 2 | 2026-10-26 |
| 7 | C資料 3 | 2 | 2026-10-26 |
| 8 | C資料 8 | 2 | 2026-10-26 |
| 9 | C資料 11 | 2 | 2026-10-26 |
| 10 | 字元 3 | 2 | 2026-10-26 |
| 11 | 陣列 2（3 小題） | 2 | 2026-10-26 |
| 12 | 陣列 3（3 小題） | 2 | 2026-10-26 |
| 13 | 組語 1（3 小題） | 2 | 2026-10-26 |
| 14 | 組語 6（4 小題） | 2 | 2026-10-26 |
| 15 | 組語 10（3 小題） | 2 | 2026-10-26 |
| 16 | 組語 12（2 小題） | 2 | 2026-10-26 |
| 17 | 組語二 5（2 小題） | 2 | 2026-10-26 |
| 18 | 多檔 1（3 小題） | 2 | 2026-10-26 |
| 19 | 組語 4（3 小題） | 3 | 2026-10-26 |
| 20 | 組語 13（2 小題） | 3 | 2026-10-26 |

---

## hello：[hello.c怎麼變成能跑的程式-編譯的四個階段](../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html)

### hello 5. 情境題：你把一支動態連結的程式複製到另一台機器，執行時出現找不到 `libc.so.6` 某個版本符號的錯誤。從本課的哪個階段可以解釋這件事？如果改用 `-static` 編譯，這個問題還會發生嗎？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-情境題你把一支動態連結的程式複製到另一台機器執行時出現找不到-libcso6-某個版本符號的錯誤從本課的哪個階段可以解釋這件事如果改用--static-編譯這個問題還會發生嗎)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#hello-5-情境題你把一支動態連結的程式複製到另一台機器執行時出現找不到-libcso6-某個版本符號的錯誤從本課的哪個階段可以解釋這件事如果改用--static-編譯這個問題還會發生嗎)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## GNU：[GNU是什麼-Linux核心與GNU工具的分工](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html)

### GNU 4. GPL 的 copyleft 要求什麼？glibc 用 LGPL 而不是 GPL，對「商業軟體能不能連結 libc」有什麼差別？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md#gnu-4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#gnu-4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## interp：[ELF的interpreter是什麼-動態載入器與井字號驚嘆號](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html)

### interp 2. `ld` 和 `ld.so`（`ld-linux-x86-64.so.2`）各在什麼時間點、做什麼事？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 2 題](../small-project/s1-01-hello-compile/quiz.md#2-ld-和-ldsold-linux-x86-64so2各在什麼時間點做什麼事)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md#interp-2-ld-和-ldsold-linux-x86-64so2各在什麼時間點做什麼事)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#interp-2-ld-和-ldsold-linux-x86-64so2各在什麼時間點做什麼事)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

### interp 3. 腳本的 `#!/bin/sh` 和 ELF 的 interpreter，哪裡相同、哪裡不同？用 `#!/bin/cat` 的實驗說明「核心把什麼交給直譯器」。

題目說的 `#!/bin/cat` 實驗（筆記[第四節](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html#四跟腳本的--是同一個機制)開頭）：寫一支第一行是 `#!/bin/cat` 的腳本，給它執行權限，再用 `./` 直接執行它。

腳本 `catscript` 的完整內容（兩行）：

```text
#!/bin/cat
這一行會被 cat 印出來
```

建檔與執行的指令：

```sh
printf '#!/bin/cat\n這一行會被 cat 印出來\n' > catscript
chmod +x catscript
./catscript
```

（執行結果不附，因為會洩漏答案。）

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-腳本的-binsh-和-elf-的-interpreter哪裡相同哪裡不同用-bincat-的實驗說明核心把什麼交給直譯器)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md#interp-3-腳本的-binsh-和-elf-的-interpreter哪裡相同哪裡不同用-bincat-的實驗說明核心把什麼交給直譯器)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#interp-3-腳本的-binsh-和-elf-的-interpreter哪裡相同哪裡不同用-bincat-的實驗說明核心把什麼交給直譯器)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## objdump：[objdump能不能代替ldd找依賴庫-需求單與實際載入](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html)

### objdump 3. 一支程式連結時用了 `-L./lib -lfoo`，編譯成功；執行時卻出現 `error while loading shared libraries: libfoo.so`。用「連結時」與「執行時」分別是誰在找檔案，解釋為什麼連結找得到、執行卻找不到。這時 `objdump -p` 與 `ldd` 各會顯示什麼？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-一支程式連結時用了--llib--lfoo編譯成功執行時卻出現-error-while-loading-shared-libraries-libfooso用連結時與執行時分別是誰在找檔案解釋為什麼連結找得到執行卻找不到這時-objdump--p-與-ldd-各會顯示什麼)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## C資料：[C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html)

### C資料 3. `int n = -1; unsigned int len = 5;`，`n < len` 為什麼是假？只加 `-Wall` 編譯，gcc 會不會提醒你？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 3 題](../small-project/s1-02-data-repr/quiz.md#3-int-n---1-unsigned-int-len--5n--len-為什麼是假只加--wall-編譯gcc-會不會提醒你)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

**題目用到的程式**：筆記[第七節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#七陷阱有號和無號混在一起比較) `signs.c` 的最後一段（`n < len` 那段），抽出來成為能單獨編譯的版本：

```c
#include <stdio.h>

int main(void)
{
    int n = -1;
    unsigned int len = 5;

    if (n < len)
        printf("n < len\n");
    else
        printf("n < len is FALSE (n=%d, len=%u)\n", n, len);
    return 0;
}
```

```sh
$ gcc -Wall -o signcmp signcmp.c
$ ./signcmp
```

> 題目：`int n = -1; unsigned int len = 5;`，`n < len` 為什麼是假？只加 `-Wall` 編譯，gcc 會不會提醒你？

```
```

### C資料 8. `signed char c = -1; unsigned int u = c;` 之後 `u` 是多少？擴展時新補的高位是 0 還是 1，由哪個型別決定？如果把 `c` 改宣告成 `unsigned char c = 0xff;`，`u` 又是多少？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 8 題](../small-project/s1-02-data-repr/quiz.md#8-signed-char-c---1-unsigned-int-u--c-之後-u-是多少擴展時新補的高位是-0-還是-1由哪個型別決定如果把-c-改宣告成-unsigned-char-c--0xffu-又是多少)、[10-05 複習](./2026-10-05-gcc-陣列-GNU-C資料-字元.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

### C資料 11. `unsigned x`、`unsigned int x`、`signed x` 各是什麼型別？為什麼 `char` 和 `signed char` 在本機的範圍完全一樣，gcc 卻說它們是不同型別、宣告衝突？情境題：同事在錯誤訊息裡看到 `long unsigned int`，說「我明明寫的是 `unsigned long`，gcc 是不是把型別搞錯了？」你怎麼回答？

> 盒子 2｜過去的作答：2026-10-06 在對話裡作答、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#c資料-11-unsigned-xunsigned-int-xsigned-x-各是什麼型別為什麼-char-和-signed-char-在本機的範圍完全一樣gcc-卻說它們是不同型別宣告衝突情境題同事在錯誤訊息裡看到-long-unsigned-int說我明明寫的是-unsigned-longgcc-是不是把型別搞錯了你怎麼回答)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## 字元：[字元是什麼-char就是1byte的整數與ASCII和UTF-8](../html/c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.html)

### 字元 3. `sizeof('A')`、`sizeof("A")`、`sizeof("字")` 在本機各是多少？分別解釋為什麼。

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 3 題](../small-project/s1-02-data-repr/quiz.md#3-sizeofasizeofasizeof字-在本機各是多少分別解釋為什麼)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## 陣列：[陣列、字串與指標運算-C不檢查邊界與用gdb找segfault](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html)

### 陣列 2. 陣列傳進函式之後

> 盒子 2｜過去的作答：[s1-03 quiz.md 第 2 題](../small-project/s1-03-arrays-pointers/quiz.md#2-1-int-a10-在-main-裡-sizeof-a-是-40傳進-void-fint-a-之後-sizeof-a-變成-8為什麼)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

**題目用到的程式**：筆記[第三節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#三陣列名稱什麼時候變成指標什麼時候不會)的 `decay.c`（筆記用 `int a[5]`，題目用 `int a[10]`，道理相同）：

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

```sh
$ gcc -Wall -Wextra -g -O0 -o decay decay.c
$ ./decay
```

#### 陣列 2-1. `int a[10];` 在 `main` 裡 `sizeof a` 是 40，傳進 `void f(int a[])` 之後 `sizeof a` 變成 8。為什麼？

```
```

#### 陣列 2-2. 這件事對「怎麼設計接收陣列的函式」有什麼影響？

> 題目：這件事（陣列傳進函式之後 `sizeof a` 從 40 變成 8）對「怎麼設計接收陣列的函式」有什麼影響？

```
```

#### 陣列 2-3. `a` 和 `&a` 印出來的位址一樣，它們差在哪裡？

> 題目：`a` 和 `&a` 印出來的位址一樣，它們差在哪裡？（`int a[10];`）

```
```

### 陣列 3. `char s1[]` 與 `char *s2`

> 盒子 2｜過去的作答：[s1-03 quiz.md 第 3 題](../small-project/s1-03-arrays-pointers/quiz.md#3-1-char-s1--hi-和-char-s2--hi-的-hi-分別在記憶體的哪裡)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

**題目用到的程式**：`strings.c`（檔案：[`strings.c`](../small-project/s1-03-arrays-pointers/strings.c)，筆記[第四節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#四字串以-0-結尾的-char-陣列)）：

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

```sh
$ gcc -Wall -g -O0 -o strings strings.c
$ ./strings
$ gdb ./strings          # 3-3 用
```

#### 陣列 3-1. `char s1[] = "hi";` 和 `char *s2 = "hi";` 的 `"hi"` 分別在記憶體的哪裡？

```
```

#### 陣列 3-2. 為什麼改 `s1[0]` 沒事，改 `s2[0]` 卻 segfault？

> 題目：為什麼改 `s1[0]` 沒事，改 `s2[0]` 卻 segfault？

```
```

#### 陣列 3-3. 你會怎麼在 gdb 裡確認 3-2 的答案？

> 題目：你會怎麼在 gdb 裡確認 3-2 的答案（改 `s1[0]` 沒事、改 `s2[0]` 卻 segfault 的原因）？

```
```

---

## 組語：[x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)

### 組語 1. `mov` 與 `lea`

> `movl (%rdi,%rsi,4), %eax` 和 `leaq (%rdi,%rsi,4), %rax` 括號裡寫的一模一樣。
>
> 盒子 2｜過去的作答：[s1-04 quiz.md 第 1 題](../small-project/s1-04-asm-basics/quiz.md#1-mov-與-lea)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

（`%rdi` 是第 1 個參數、`%rsi` 是第 2 個參數。）

#### 組語 1-1. 兩個指令做的事差在哪？

```
```

#### 組語 1-2. 各對應 C 的什麼寫法？

> 題目：`movl (%rdi,%rsi,4), %eax` 和 `leaq (%rdi,%rsi,4), %rax` 各對應 C 的什麼寫法？

```
```

#### 組語 1-3. 為什麼 gcc 會拿 `lea` 來算 `x * 3` 這種跟位址無關的東西？

`long times3(long x) { return x * 3; }` 用 `gcc -S -O2` 編出來是：

```asm
times3:
	endbr64                       # 給 CPU 的控制流程保護標記，跟計算無關
	leaq	(%rdi,%rdi,2), %rax       # 這一行就是本題要解釋的
	ret                           # 回傳 %rax
```

> 題目：為什麼 gcc 會拿 `lea` 來算 `x * 3` 這種跟位址無關的東西？

```
```

### 組語 4. 旗標與指令的排列

> `-O2` 的 `sum_to` 裡，`andl $1, %edi` 和 `je .L3` 中間夾了一個 `movl $1, %eax`。

用到的程式（第 1 課的檔案：[`sumto.c`](../small-project/s1-01-hello-compile/sumto.c)）：

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

```sh
gcc -S -O2 -fno-asynchronous-unwind-tables sumto.c -o sumto2.s
```

`sum_to` 的 `-O2` 組合語言（`%edi` 是參數 `n`；註解只寫每行算什麼，不寫哪些指令改旗標，那是題目要問的）：

```asm
sum_to:
	endbr64
	testl	%edi, %edi            # 算 n & n
	jle	.L4                   # ≤ 0 就跳到 .L4（回傳 0）
	leal	1(%rdi), %ecx         # %ecx = %rdi + 1
	xorl	%edx, %edx            # %edx = 0（total）
	andl	$1, %edi              # %edi = %edi & 1
	movl	$1, %eax              # %eax = 1（i）
	je	.L3                   # 依旗標決定要不要跳到 .L3
	movl	$2, %eax              # %eax = 2
	movl	$1, %edx              # %edx = 1
	cmpl	%ecx, %eax            # 比較 %eax 與 %ecx
	je	.L1                   # 相等就跳 .L1
.L3:
	leal	1(%rdx,%rax,2), %edx  # %edx = %edx + %rax×2 + 1（一圈加兩個數）
	addl	$2, %eax              # %eax += 2
	cmpl	%ecx, %eax            # 比較 %eax 與 %ecx
	jne	.L3                   # 不相等就再一圈
.L1:
	movl	%edx, %eax            # 回傳值 = %edx
	ret
.L4:
	xorl	%edx, %edx
	movl	%edx, %eax            # 回傳 0
	ret
```

> 盒子 3｜過去的作答：[s1-04 quiz.md 第 4 題](../small-project/s1-04-asm-basics/quiz.md#4-旗標與指令的排列)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)

#### 組語 4-1. `je` 判斷的是哪個指令的結果？

```
```

#### 組語 4-2. 為什麼 gcc 可以這樣排？

```
```

#### 組語 4-3. 如果中間夾的是 `addl $1, %eax`，會怎樣？

```
```

### 組語 6. 組合語言的大小名稱 vs C 的型別

> 盒子 2｜過去的作答：[s1-04 quiz.md 第 6 題](../small-project/s1-04-asm-basics/quiz.md#6-組合語言的大小名稱-vs-c-的型別2026-10-06-課後提問新增)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

#### 組語 6-1. `movsbl` 的 `l` 叫 long，為什麼是 4 byte，而本機 C 的 `long` 是 8 byte？

```
```

#### 組語 6-2. 本機 C 的 `long` 在組合語言裡對到哪個字尾？

```
```

#### 組語 6-3. 這樣對嗎？

> 題目：情境題：第 5 課你在 `gdb` 裡想看一個 `int` 變數的 4 個 byte，同學說「`int` 在組合語言是 `l`，所以用 `x/1l`」。這樣對嗎？

```
```

#### 組語 6-4. 應該用哪個大小字母，為什麼？

> 題目：（承上）在 gdb 裡用 `x` 看 `int` 變數的 4 個 byte，應該用哪個大小字母，為什麼？

```
```

### 組語 10. 暫存器 vs 記憶體運算元

> `mov %rdi,%rax` 和 `mov (%rdi),%rax` 只差一對括號。
>
> 盒子 2｜過去的作答：[s1-04 quiz.md 第 10 題](../small-project/s1-04-asm-basics/quiz.md#10-暫存器-vs-記憶體運算元2026-10-06-課後提問新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

用到的程式（檔案：[`paren.c`](../small-project/s1-04-asm-basics/paren.c)）：

```c
#include <stdio.h>

__attribute__((noinline)) long copy_val(long x)  { return x; }    /* 回傳 x 本身 */
__attribute__((noinline)) long deref(long *p)    { return *p; }   /* 回傳 p 指向的值 */

int main(void)
{
    long v = 1234;
    printf("%ld %ld\n", copy_val(v), deref(&v));
    return 0;
}
```

```sh
gcc -O2 -g paren.c -o paren
objdump -d --no-show-raw-insn paren
```

```asm
copy_val:
	endbr64
	mov	%rdi,%rax       # 沒有括號（第 1 個參數在 %rdi）
	ret                 # 回傳 %rax
deref:
	endbr64
	mov	(%rdi),%rax     # 有括號
	ret
```

#### 組語 10-1. 兩個指令各拿 `%rdi` 裡的數字做什麼？

```
```

#### 組語 10-2. 各對應 C 的什麼寫法？

> 題目：`mov %rdi,%rax` 和 `mov (%rdi),%rax` 各對應 C 的什麼寫法？

```
```

#### 組語 10-3. 如果 `%rdi` 裡是 1234，哪一個會 segfault，為什麼？

```
```

### 組語 12. 檔案裡的位址 vs 執行時的位址

> 用 `objdump -d paren` 查到 `deref+4` 在 `0x11f4`，`break *0x11f4` 之後 `run` 卻得到 `Cannot access memory at address 0x11f4`，`break *deref+4` 就沒問題。
>
> 盒子 2｜過去的作答：[s1-04 quiz.md 第 12 題](../small-project/s1-04-asm-basics/quiz.md#12-檔案裡的位址-vs-執行時的位址2026-10-06-課後實作新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

實驗步驟（程式同組語 11 的 `paren.c`）：

```sh
gcc -O2 -g paren.c -o paren
objdump -d --no-show-raw-insn paren        # deref 的 mov (%rdi),%rax 那一行印成 11f4
gdb -batch -ex 'break *0x11f4' -ex 'break *deref+4' -ex 'info breakpoints' -ex run -ex 'info breakpoints' ./paren
```

#### 組語 12-1. 為什麼？

> 題目：`break *0x11f4` 之後 `run` 得到 `Cannot access memory at address 0x11f4`，`break *deref+4` 卻沒問題，為什麼？

```
```

#### 組語 12-2. 如果程式是用 `-no-pie` 編的，`break *` 加 `objdump` 查到的位址會不會成功？

```
```

### 組語 13. `D(%rip)` 與 `D(,I,S)`

> 題目來自筆記[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上表其他五列：每一種用一個實際的 C 函式對照〉。

用到的程式（檔案：[`forms.c`](../small-project/s1-04-asm-basics/forms.c)）：

```c
#include <stdio.h>

long tbl[4] = {100, 200, 300, 400};   /* 全域陣列 */
long g = 42;                          /* 全域變數 */

__attribute__((noinline)) long second(long *p)      { return p[1]; }      /* D(B)     */
__attribute__((noinline)) int  get(int *a, long i)  { return a[i]; }      /* (B,I,S)  */
__attribute__((noinline)) int  get3(int *a, long i) { return a[i + 3]; }  /* D(B,I,S) */
__attribute__((noinline)) long get_tbl(long i)      { return tbl[i]; }    /* D(,I,S)  */
__attribute__((noinline)) long get_g(void)          { return g; }         /* D(%rip)  */

int main(void)
{
    long arr[2]  = {7, 8};
    int  nums[6] = {10, 11, 12, 13, 14, 15};
    printf("%ld %d %d %ld %ld\n", second(arr), get(nums, 1), get3(nums, 1), get_tbl(2), get_g());
    return 0;
}
```

編成 PIE（gcc 預設）與非 PIE 兩種，看 `get_g`、`get_tbl` 的指令（`objdump` 節錄，每個函式開頭的 `endbr64` 省略）：

```sh
gcc -O2 -g forms.c -o forms_pie
objdump -d --no-show-raw-insn forms_pie
gcc -O2 -g -fno-pie -no-pie forms.c -o forms_np
objdump -d --no-show-raw-insn forms_np
```

```asm
# forms_pie（PIE）
0000000000001250 <get_tbl>:
    1254:	lea    0x2de5(%rip),%rax        # 4040 <tbl>    %rax = tbl 的位址
    125b:	mov    (%rax,%rdi,8),%rax                       # %rax = 讀 %rax + i×8 那 8 byte
0000000000001260 <get_g>:
    1264:	mov    0x2db5(%rip),%rax        # 4020 <g>      %rax = 讀 %rip + 0x2db5 那 8 byte
    126b:	ret

# forms_np（非 PIE）
0000000000401240 <get_tbl>:
  401244:	mov    0x404060(,%rdi,8),%rax                   # %rax = 讀 0x404060 + i×8 那 8 byte
```

> 盒子 3｜過去的作答：[s1-04 quiz.md 第 13 題](../small-project/s1-04-asm-basics/quiz.md#13-drip-與-dis2026-10-06-課後提問新增)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)

#### 組語 13-1. `get_g` 讀全域變數 `g` 用的是 `mov 0x2db5(%rip),%rax`，為什麼 PIE 程式不能直接把 `g` 的位址寫進指令，而要用 `%rip` ＋ 距離？

```
```

#### 組語 13-2. 同一個 `tbl[i]`，為什麼非 PIE 的程式可以寫成一條 `mov 0x404060(,%rdi,8),%rax`？

```
```

### 組語 18. `0x01 + 0x7f` 的進位與 CF、OF

> 盒子 1｜過去的作答：2026-10-25 在對話裡作答（18-1 對、18-2 大致對、18-3 大致對、18-4 未作答），批改後的參考答案寫在[筆記第 18 題](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#自我測驗)

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

## 組語二：[x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html)

### 組語二 5. 情境題：停在函式的第一條指令

> 盒子 2｜過去的作答：[s1-05 quiz.md 第 5 題](../small-project/s1-05-asm-calls/quiz.md#5-情境題停在函式的第一條指令)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

你用 `break *add3` 停在 `add3` 的第一條指令（還沒執行 `push %rbp`），gdb 顯示 `add3 (a=0, b=0, c=0)`，可是 `main` 明明傳的是 1、2、3。

完整原始碼（檔案：[`frame.c`](../small-project/s1-05-asm-calls/frame.c)）與編譯指令（同組語二 1）：

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

`break *add3` 的 gdb 指令：`gdb -batch -ex 'break *add3' -ex run ./frame`

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

#### 組語二 5-1. 為什麼 gdb 印錯？這時候要看真正的參數，該看哪裡？

```
```

#### 組語二 5-2. 同一個時間點，你想知道「是誰呼叫了 `add3`」，可是不能用 `bt`。根據本課，你會看哪個暫存器指的位址？看到的值代表什麼？

```
```

---

## 多檔：[多檔案的C程式-標頭檔、static與extern、連結錯誤與make](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html)

### 多檔 1. 忘了宣告：`square(100000)` 印出 1410065408

> 盒子 2｜過去的作答：[s1-06 quiz.md 第 1 題](../small-project/s1-06-multi-file/quiz.md#1-忘了宣告square100000-印出-1410065408)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

`main.c` 沒有 `square` 的宣告，`gcc -c main.c` 只有一個警告，連結也成功，但 `square(100000)` 印出 1410065408 而不是 10000000000。為什麼編譯器會產生錯的程式？為什麼連結器沒有發現兩邊型別不一樣？`main.o` 裡的 `cltq` 在這個錯誤裡扮演什麼角色？

```c
/* mathx.c */
long square(long x)
{
    return x * x;
}
```
```c
/* main.c */
#include <stdio.h>

int main(void)
{
    long r = square(100000);
    printf("%ld\n", r);
    return 0;
}
```
```asm
# objdump -dr --no-show-raw-insn main.o（main 的一部分）
  c:	mov    $0x186a0,%edi         # 第 1 個參數 = 100000
 11:	mov    $0x0,%eax
 16:	call   1b <main+0x1b>        # 呼叫 square（位址連結時才填）
			17: R_X86_64_PLT32	square-0x4
 1b:	cltq                         # 把 %eax 符號擴展成 %rax（= movslq %eax,%rax）
 1d:	mov    %rax,-0x8(%rbp)       # 存進 r
```
編譯與執行：`gcc -c main.c && gcc -c mathx.c && gcc -o calc main.o mathx.o && ./calc`

#### 多檔 1-1. 為什麼編譯器會產生錯的程式？

```
```

#### 多檔 1-2. 為什麼連結器沒有發現兩邊型別不一樣？

```
```

#### 多檔 1-3. `main.o` 裡的 `cltq` 在這個錯誤裡扮演什麼角色？

```
```
