# 2026-10-29 複習：hello、GNU、objdump、陣列、組語

> 依[複習排程](../learning/複習排程.md)出今天到期的全部題目（每天出完所有到期題，2026-10-23 起的規則），共 **7 題**（16 個作答框），沒有過期的。
> 其中 6 題是 10-26 大致對、留在盒子 2 的題目（hello 5、GNU 4、objdump 3、陣列 2、組語 6、組語 12）；objdump 2 是 10-22 答對、排在今天的盒子 3。
> 題目依筆記分節、節內照題號排；每題照原題複製，用到的程式碼與實驗步驟都附在題目裡，「過去的作答」連到之前每一次作答（含批改與參考答案）。
>
> **怎麼寫**：闔上筆記，每題寫 1～2 句關鍵句就好。可以分幾次寫，寫完一部分就可以先說「幫我批改」。複習不重答，批改會直接附參考答案。
> 已拆小題的題目，每個小題一個作答框；以大題為單位排程，小題全對才算對。

## 今天的題目

| # | 題目 | 盒子 | 原本該考的日期 |
|---|---|---|---|
| 1 | hello 5 | 2 | 2026-10-29 |
| 2 | 陣列 2（3 小題） | 2 | 2026-10-29 |
| 3 | GNU 4 | 2 | 2026-10-29 |
| 4 | objdump 3 | 2 | 2026-10-29 |
| 5 | 組語 6（4 小題） | 2 | 2026-10-29 |
| 6 | 組語 12（2 小題） | 2 | 2026-10-29 |
| 7 | objdump 2 | 3 | 2026-10-29 |

---

## hello：[hello.c怎麼變成能跑的程式-編譯的四個階段](../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html)

### hello 5. 情境題：你把一支動態連結的程式複製到另一台機器，執行時出現找不到 `libc.so.6` 某個版本符號的錯誤。從本課的哪個階段可以解釋這件事？如果改用 `-static` 編譯，這個問題還會發生嗎？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-情境題你把一支動態連結的程式複製到另一台機器執行時出現找不到-libcso6-某個版本符號的錯誤從本課的哪個階段可以解釋這件事如果改用--static-編譯這個問題還會發生嗎)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#hello-5-情境題你把一支動態連結的程式複製到另一台機器執行時出現找不到-libcso6-某個版本符號的錯誤從本課的哪個階段可以解釋這件事如果改用--static-編譯這個問題還會發生嗎)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## GNU：[GNU是什麼-Linux核心與GNU工具的分工](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html)

### GNU 4. GPL 的 copyleft 要求什麼？glibc 用 LGPL 而不是 GPL，對「商業軟體能不能連結 libc」有什麼差別？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md#gnu-4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#gnu-4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## objdump：[objdump能不能代替ldd找依賴庫-需求單與實際載入](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html)

### objdump 2. `ldd` 是怎麼得到依賴清單的？從這個機制解釋：為什麼對來路不明的執行檔用 `ldd` 有風險，`objdump -p` 卻沒有？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 2 題](../small-project/s1-01-hello-compile/quiz.md#2-ldd-是怎麼得到依賴清單的從這個機制解釋為什麼對來路不明的執行檔用-ldd-有風險objdump--p-卻沒有)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)

```
```

### objdump 3. 一支程式連結時用了 `-L./lib -lfoo`，編譯成功；執行時卻出現 `error while loading shared libraries: libfoo.so`。用「連結時」與「執行時」分別是誰在找檔案，解釋為什麼連結找得到、執行卻找不到。這時 `objdump -p` 與 `ldd` 各會顯示什麼？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-一支程式連結時用了--llib--lfoo編譯成功執行時卻出現-error-while-loading-shared-libraries-libfooso用連結時與執行時分別是誰在找檔案解釋為什麼連結找得到執行卻找不到這時-objdump--p-與-ldd-各會顯示什麼)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## 陣列：[陣列、字串與指標運算-C不檢查邊界與用gdb找segfault](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html)

### 陣列 2. 陣列傳進函式之後

> 盒子 2｜過去的作答：[s1-03 quiz.md 第 2 題](../small-project/s1-03-arrays-pointers/quiz.md#2-1-int-a10-在-main-裡-sizeof-a-是-40傳進-void-fint-a-之後-sizeof-a-變成-8為什麼)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

---

## 組語：[x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)

### 組語 6. 組合語言的大小名稱 vs C 的型別

> 盒子 2｜過去的作答：[s1-04 quiz.md 第 6 題](../small-project/s1-04-asm-basics/quiz.md#6-組合語言的大小名稱-vs-c-的型別2026-10-06-課後提問新增)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

### 組語 12. 檔案裡的位址 vs 執行時的位址

> 用 `objdump -d paren` 查到 `deref+4` 在 `0x11f4`，`break *0x11f4` 之後 `run` 卻得到 `Cannot access memory at address 0x11f4`，`break *deref+4` 就沒問題。
>
> 盒子 2｜過去的作答：[s1-04 quiz.md 第 12 題](../small-project/s1-04-asm-basics/quiz.md#12-檔案裡的位址-vs-執行時的位址2026-10-06-課後實作新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

`-O2` 編出的 `deref`：

```asm
deref:
	endbr64                 # 控制流程保護標記（佔 4 byte，所以下一條是 deref+4）
	mov	(%rdi),%rax         # 把 %rdi 當位址，讀 8 byte 放進 %rax
	ret                     # 回傳 %rax
```

實驗步驟：

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
