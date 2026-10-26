# 2026-10-27 複習：gcc、ELF、objdump、C資料、字元、陣列、組語、組語二、多檔

> 依[複習排程](../learning/複習排程.md)出今天到期的全部題目（每天出完所有到期題，2026-10-23 起的規則），共 **19 題**（55 個作答框），沒有過期的。
> 其中 17 題是 10-24 答對、排在今天的盒子 2；C資料 10 是 10-20 答對、排在今天的盒子 3；組語 18 是 10-26 不完整、回到盒子 1（18-4 用兩個進位說明 OF）。
> 題目依筆記分節、節內照題號排；每題照原題複製，用到的程式碼與實驗步驟都附在題目裡，「過去的作答」連到之前每一次作答（含批改與參考答案）。
>
> **怎麼寫**：闔上筆記，每題寫 1～2 句關鍵句就好。可以分幾次寫，寫完一部分就可以先說「幫我批改」。複習不重答，批改會直接附參考答案。
> 已拆小題的題目，每個小題一個作答框；以大題為單位排程，小題全對才算對。

## 今天的題目

| # | 題目 | 盒子 | 原本該考的日期 |
|---|---|---|---|
| 1 | 組語 18（4 小題） | 1 | 2026-10-27 |
| 2 | gcc 2 | 2 | 2026-10-27 |
| 3 | gcc 5 | 2 | 2026-10-27 |
| 4 | ELF 4 | 2 | 2026-10-27 |
| 5 | objdump 4 | 2 | 2026-10-27 |
| 6 | C資料 4 | 2 | 2026-10-27 |
| 7 | C資料 6 | 2 | 2026-10-27 |
| 8 | C資料 9 | 2 | 2026-10-27 |
| 9 | 字元 5 | 2 | 2026-10-27 |
| 10 | 陣列 4（2 小題） | 2 | 2026-10-27 |
| 11 | 陣列 7（4 小題） | 2 | 2026-10-27 |
| 12 | 組語 7（4 小題） | 2 | 2026-10-27 |
| 13 | 組語 11（3 小題） | 2 | 2026-10-27 |
| 14 | 組語 15（3 小題） | 2 | 2026-10-27 |
| 15 | 組語二 1（4 小題） | 2 | 2026-10-27 |
| 16 | 組語二 4（2 小題） | 2 | 2026-10-27 |
| 17 | 多檔 3（3 小題） | 2 | 2026-10-27 |
| 18 | 多檔 4（4 小題） | 2 | 2026-10-27 |
| 19 | C資料 10 | 3 | 2026-10-27 |

---

## gcc：[gcc最佳化等級-O0到O3差在哪](../html/c/gcc最佳化等級-O0到O3差在哪.html)

### gcc 2. 為什麼 `-O0` 的組合語言每一圈都在讀寫記憶體（`-4(%rbp)`），而 `-O2` 不會？這個差別對 `gdb` 有什麼影響？

> 盒子 2｜過去的作答：[s1-01 quiz.md gcc 第 2 題](../small-project/s1-01-hello-compile/quiz.md#2-為什麼--o0-的組合語言每一圈都在讀寫記憶體-4rbp而--o2-不會這個差別對-gdb-有什麼影響)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

**題目用到的程式**：`sumto.c`（檔案：[`sumto.c`](../small-project/s1-01-hello-compile/sumto.c)），
用 `gcc -O0 -S sumto.c` 與 `gcc -O2 -S sumto.c` 產生組合語言（`-S`：只編譯到組合語言 `.s` 就停）。

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

`-O0` 的 `sum_to`（本機 gcc 13.3 實際輸出，刪掉 `.cfi`、`.type` 這類組譯指示，右邊是逐行說明）：

```asm
sum_to:
	endbr64                    # 間接跳躍的保護標記（CET），跟這題無關
	pushq	%rbp               # 存起呼叫者的 %rbp
	movq	%rsp, %rbp         # %rbp = 這個函式的堆疊框底
	movl	%edi, -20(%rbp)    # 參數 n（放在 %edi 傳進來）寫到記憶體 -20(%rbp)
	movl	$0, -8(%rbp)       # total = 0，寫到記憶體 -8(%rbp)
	movl	$1, -4(%rbp)       # i = 1，寫到記憶體 -4(%rbp)
	jmp	.L2                # 先跳去檢查條件
.L3:                               # 迴圈本體
	movl	-4(%rbp), %eax     # 從記憶體讀 i 到 %eax
	addl	%eax, -8(%rbp)     # 記憶體裡的 total += %eax
	addl	$1, -4(%rbp)       # 記憶體裡的 i += 1
.L2:                               # 迴圈條件
	movl	-4(%rbp), %eax     # 從記憶體讀 i
	cmpl	-20(%rbp), %eax    # 比較 i 與記憶體裡的 n（算 i − n、設旗標）
	jle	.L3                # i <= n（有號）就回去跑本體
	movl	-8(%rbp), %eax     # 從記憶體讀 total 當回傳值
	popq	%rbp               # 還原 %rbp
	ret
```

`-O2` 的 `sum_to`（同上，實際輸出）：

```asm
sum_to:
	endbr64
	testl	%edi, %edi         # n & n，只為了設旗標
	jle	.L4                # n <= 0 就跳去回傳 0
	leal	1(%rdi), %ecx      # %ecx = n + 1（迴圈的終點）
	xorl	%edx, %edx         # %edx = 0（total）
	andl	$1, %edi           # %edi = n & 1（n 是奇數還是偶數）
	movl	$1, %eax           # %eax = 1（i）
	je	.L3                # n 是偶數就直接進迴圈
	movl	$2, %eax           # n 是奇數：先處理掉 i = 1，i 從 2 開始
	movl	$1, %edx           #           total = 1
	cmpl	%ecx, %eax         #           i 已經到 n + 1 了嗎（n = 1 時）
	je	.L1
.L3:                               # 迴圈本體：一圈加兩個數
	leal	1(%rdx,%rax,2), %edx  # total = total + 2×i + 1（＝ total + i + (i+1)）
	addl	$2, %eax           # i += 2
	cmpl	%ecx, %eax         # i 到 n + 1 了嗎
	jne	.L3
.L1:
	movl	%edx, %eax         # 回傳 total
	ret
.L4:
	xorl	%edx, %edx
	movl	%edx, %eax         # 回傳 0
	ret
```

> 題目：為什麼 `-O0` 的組合語言每一圈都在讀寫記憶體（`-4(%rbp)`），而 `-O2` 不會？這個差別對 `gdb` 有什麼影響？

```
```

### gcc 5. 情境題：你在自己的程式裡用 `-O2` 編譯，`gdb` 印某個變數時顯示 `<optimized out>`。你會怎麼處理？如果是在 kernel 裡遇到同樣的狀況，處理方式又有什麼不同？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-情境題你在自己的程式裡用--o2-編譯gdb-印某個變數時顯示-optimized-out你會怎麼處理如果是在-kernel-裡遇到同樣的狀況處理方式又有什麼不同)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## ELF：[ELF是什麼-可執行與可連結格式的兩種視角](../html/c/ELF是什麼-可執行與可連結格式的兩種視角.html)

### ELF 4. 區段（section）和載入段（segment）差在哪？為什麼核心載入時要把很多區段打包成少數幾個載入段？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-區段section和載入段segment差在哪為什麼核心載入時要把很多區段打包成少數幾個載入段)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#elf-4-區段section和載入段segment差在哪為什麼核心載入時要把很多區段打包成少數幾個載入段)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## objdump：[objdump能不能代替ldd找依賴庫-需求單與實際載入](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html)

### objdump 4. 情境題：同事傳來一支 ARM 開發板用的執行檔，問你「它需要哪些函式庫」。你在自己的 x86-64 電腦上該用哪個工具？用 `ldd` 會發生什麼事，為什麼？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-情境題同事傳來一支-arm-開發板用的執行檔問你它需要哪些函式庫你在自己的-x86-64-電腦上該用哪個工具用-ldd-會發生什麼事為什麼)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## C資料：[C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html)

### C資料 4. `int y = x + 1; return y > x;` 在 `x = INT_MAX` 時，`-O0` 回傳 0、`-O2` 回傳 1。為什麼會這樣？這算不算 gcc 的 bug？kernel 用哪個選項避免這種情況，代價是什麼？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 4 題](../small-project/s1-02-data-repr/quiz.md#4-int-y--x--1-return-y--x-在-x--int_max-時-o0-回傳-0-o2-回傳-1為什麼會這樣這算不算-gcc-的-bugkernel-用哪個選項避免這種情況代價是什麼)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

### C資料 6. `printf("%p")` 印出 `0x7ffe50b23624`，只有 12 位十六進位。指標到底佔幾個 byte？印出來為什麼比較短？如果在 kernel 的錯誤訊息裡看到 `0xffff888102a4c000`，從位址的長相你能判斷什麼？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 6 題](../small-project/s1-02-data-repr/quiz.md#6-printfp-印出-0x7ffe50b23624只有-12-位十六進位指標到底佔幾個-byte印出來為什麼比較短如果在-kernel-的錯誤訊息裡看到-0xffff888102a4c000從位址的長相你能判斷什麼)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

### C資料 9. 情境題：有人寫了 `if (len - used > 0)` 判斷「緩衝區還有沒有剩餘空間」，`len`、`used` 都是 `size_t`，用 `-Wall -Wextra` 編譯沒有任何警告。這個判斷在什麼情況下會出錯？為什麼 gcc 對 `len - used < 0` 會警告、對 `> 0` 卻不會？另外，為什麼 `int sum = x + y; if (sum < x)` 這種溢位檢查在 `-O2` 下可能完全失效？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 9 題](../small-project/s1-02-data-repr/quiz.md#9-情境題有人寫了-if-len---used--0-判斷緩衝區還有沒有剩餘空間lenused-都是-size_t用--wall--wextra-編譯沒有任何警告這個判斷在什麼情況下會出錯為什麼-gcc-對-len---used--0-會警告對--0-卻不會另外為什麼-int-sum--x--y-if-sum--x-這種溢位檢查在--o2-下可能完全失效)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

### C資料 10. `long a = -1; unsigned int b = 1;` 與 `int c = -1; unsigned long d = 1;`，`a < b` 和 `c < d` 各是真還是假？兩邊各被轉成什麼型別，依據的是哪一條規則？為什麼 gcc 只對其中一個發出 `-Wsign-compare` 警告？

**題目用到的程式**（把題目的兩組宣告寫成可以編譯的完整程式 `cmp.c`，`-Wall -Wextra` 編譯）：

```c
#include <stdio.h>

int main(void)
{
    long a = -1;
    unsigned int b = 1;

    int c = -1;
    unsigned long d = 1;

    printf("a < b: %d\n", a < b);
    printf("c < d: %d\n", c < d);
    return 0;
}
```

```sh
gcc -Wall -Wextra cmp.c -o cmp && ./cmp
```

> 盒子 3｜過去的作答：[s1-02 quiz.md 第 10 題](../small-project/s1-02-data-repr/quiz.md#10-long-a---1-unsigned-int-b--1-與-int-c---1-unsigned-long-d--1a--b-和-c--d-各是真還是假兩邊各被轉成什麼型別依據的是哪一條規則為什麼-gcc-只對其中一個發出--wsign-compare-警告)、[10-05 複習](./2026-10-05-gcc-陣列-GNU-C資料-字元.md)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)

```
```

---

## 字元：[字元是什麼-char就是1byte的整數與ASCII和UTF-8](../html/c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.html)

### 字元 5. 有人說「『字』是 3 個字元」，另一個人說「是 1 個字元」，誰對？`strlen("字")` 和 `mbstowcs(NULL, "字", 0)` 各會得到多少？如果程式忘了呼叫 `setlocale`，後者會得到什麼，那個數字是怎麼來的？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 5 題](../small-project/s1-02-data-repr/quiz.md#5-有人說字是-3-個字元另一個人說是-1-個字元誰對strlen字-和-mbstowcsnull-字-0-各會得到多少如果程式忘了呼叫-setlocale後者會得到什麼那個數字是怎麼來的)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## 陣列：[陣列、字串與指標運算-C不檢查邊界與用gdb找segfault](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html)

### 陣列 4. 越界的三種下場

#### 陣列 4-1. 越界存取有三種下場：安靜出錯、`stack smashing detected`、segfault。分別是什麼決定會落到哪一種？

> 盒子 2｜過去的作答：[s1-03 quiz.md 4-1](../small-project/s1-03-arrays-pointers/quiz.md#4-1-越界存取有三種下場安靜出錯stack-smashing-detectedsegfault分別是什麼決定會落到哪一種)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

#### 陣列 4-2. 為什麼「程式沒當掉」不能證明沒有越界？

```
```

### 陣列 7. 情境題：讀懂 ASan 的影子 byte

> ASan 報告的影子 byte 那一列裡，某個堆疊上的 `char` 陣列對應到 `00 00 06`，前後都是 `f2`。
>
> 盒子 2｜過去的作答：2026-10-11 在對話裡作答（筆記[第七節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#七addresssanitizer在第一次越界的當下抓到它)〈上圖的 `01`：影子 byte 的值是「個數」，不是資料〉的追問）、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

#### 陣列 7-1. 這個陣列佔幾個 byte？

```
```

#### 陣列 7-2. `06` 的 6 是什麼意思？

> 題目：（影子 byte 是 `00 00 06`，前後都是 `f2`）`06` 的 6 是什麼意思？

```
```

#### 陣列 7-3. 如果它是 `char s[] = "…";` 這樣用字串初始化的，字串本身（不含 `'\0'`）有幾個字元？

> 題目：（影子 byte 是 `00 00 06`）如果它是 `char s[] = "…";` 這樣用字串初始化的，字串本身（不含 `'\0'`）有幾個字元？

```
```

#### 陣列 7-4. 為什麼影子 byte 裡不會出現 `08`？

```
```

---

## 組語：[x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)

### 組語 7. 參數只佔暫存器的一部分

> `int widen_char(signed char c) { return c; }` 編出 `movsbl %dil, %eax`。
>
> 盒子 2｜過去的作答：[s1-04 quiz.md 第 7 題](../small-project/s1-04-asm-basics/quiz.md#7-參數只佔暫存器的一部分2026-10-06-課後提問新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

用到的程式（檔案：[`dil.c`](../small-project/s1-04-asm-basics/dil.c)）：

```c
#include <stdio.h>

__attribute__((noinline)) int widen_char(signed char c) { return c; }   /* noinline：不准內嵌，才看得到真的呼叫 */

int main(void)
{
    printf("%d\n", widen_char(-1));
    return 0;
}
```

```sh
gcc -O2 -g dil.c -o dil
objdump -d --no-show-raw-insn dil
```

`widen_char` 編出來的組合語言（`objdump` 節錄）：

```asm
widen_char:
	endbr64                # 給 CPU 的控制流程保護標記，跟計算無關（佔 4 byte，所以下一條是 widen_char+4）
	movsbl	%dil, %eax     # 讀 %dil（1 byte），有號擴展成 4 byte 放進 %eax
	ret                    # 回傳 %eax
```

筆記的 gdb 實驗：停在 `movsbl` 之前，把 `%rdi` 整個改掉，再執行一條指令看結果。

```sh
gdb -batch -ex 'break *widen_char+4' -ex run -ex 'p/x $rdi' \
    -ex 'set $rdi = 0x11223344556677ff' -ex 'p/x $rdi' -ex 'p/x $dil' \
    -ex stepi -ex 'p $eax' -ex 'p/x $rax' -ex continue ./dil
```

（`break *widen_char+4`：停在 `widen_char` 開頭往後 4 byte 那條指令；`set $rdi = …`：改暫存器；`stepi`：只執行一條機器指令。）

#### 組語 7-1. 在 64 位元的機器上，為什麼它只讀 `%dil`，而不讀整個 `%rdi`？

```
```

#### 組語 7-2. 之後 `%eax` 的十六進位是多少？

> 題目：假設 `widen_char` 改成讀 4 byte 的 `movl %edi, %eax`，在上面的 `gdb` 實驗裡把 `%rdi` 改成 `0x11223344556677ff`。之後 `%eax` 的十六進位是多少？

```
```

#### 組語 7-3. 是正數還是負數？

> 題目：（承 7-2）`movl %edi, %eax` 之後的 `%eax` 是正數還是負數？

```
```

#### 組語 7-4. 跟 C 期望的 −1 差在哪？

> 題目：（承 7-2）改成 `movl %edi, %eax` 之後得到的值，跟 C 期望的 −1 差在哪？

```
```

### 組語 11. gdb 的 `break *位址`

> `break deref+4` 會得到 `Function "deref+4" not defined.`，`break *deref+4` 卻可以。
>
> 盒子 2｜過去的作答：[s1-04 quiz.md 第 11 題](../small-project/s1-04-asm-basics/quiz.md#11-gdb-的-break-位址2026-10-06-課後提問新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

`-O2` 編出的 `deref`：

```asm
deref:
	endbr64                 # 控制流程保護標記（佔 4 byte，所以下一條是 deref+4）
	mov	(%rdi),%rax         # 把 %rdi 當位址，讀 8 byte 放進 %rax
	ret                     # 回傳 %rax
```

#### 組語 11-1. `*` 讓 gdb 怎麼理解後面的東西？

```
```

#### 組語 11-2. 這個 `*` 跟 C 的 `*p` 是同一個意思嗎？

```
```

#### 組語 11-3. 在 `-O0` 的程式上，`break deref` 和 `break *deref` 為什麼會停在不同的位址？

同一個 `paren.c` 改用 `-O0` 編，`deref` 的組合語言：

```sh
gcc -O0 -g paren.c -o paren0
objdump -d --no-show-raw-insn paren0
gdb -batch -ex 'break deref' -ex 'break *deref' ./paren0     # 看兩個中斷點各下在哪個位址
```

```asm
000000000000117b <deref>:
    117b:	endbr64                 # 控制流程保護標記
    117f:	push   %rbp             # 把呼叫者的 %rbp 存到堆疊（第 5 課的序言）
    1180:	mov    %rsp,%rbp        # %rbp = 目前堆疊頂，建立這個函式的堆疊框
    1183:	mov    %rdi,-0x8(%rbp)  # 把參數 p（在 %rdi）存進堆疊上的區域變數
    1187:	mov    -0x8(%rbp),%rax  # 從堆疊讀回 p 放進 %rax
    118b:	mov    (%rax),%rax      # *p：把 p 當位址讀 8 byte
    118e:	pop    %rbp             # 還原呼叫者的 %rbp
    118f:	ret                     # 回傳 %rax
```

> 題目：在 `-O0` 的程式上，`break deref` 和 `break *deref` 為什麼會停在不同的位址？

```
```

### 組語 15. 減法的借位與 CF

> 8 位元算 4 − 9。
>
> 盒子 2｜過去的作答：[s1-04 quiz.md 第 15 題](../small-project/s1-04-asm-basics/quiz.md#15-減法的借位與-cf2026-10-07-課後提問新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

#### 組語 15-1. 照直式減法，最高位（第 7 位元）需不需要往外借？CF 是多少？

```
```

#### 組語 15-2. 留下的位元當無號讀是多少，它跟數學上的正確答案差了多少、為什麼剛好差這麼多？

> 題目：（8 位元算 4 − 9）留下的位元當無號讀是多少，它跟數學上的正確答案差了多少、為什麼剛好差這麼多？

```
```

#### 組語 15-3. 如果 CPU 是用加法器算（D + S 反轉 + 1），加法器有沒有往外進位，跟 CF 是什麼關係？

> 題目：（8 位元算 4 − 9）如果 CPU 是用加法器算（D + S 反轉 + 1），加法器有沒有往外進位，跟 CF 是什麼關係？

```
```

### 組語 18. `0x01 + 0x7f` 的進位與 CF、OF

> 盒子 1｜過去的作答：2026-10-25 在對話裡作答（18-1 對、18-2 大致對、18-3 大致對、18-4 未作答），批改後的參考答案寫在[筆記第 18 題](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#自我測驗)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

### 組語二 1. `call` 與 `ret`

> 盒子 2｜過去的作答：[s1-05 quiz.md 第 1 題](../small-project/s1-05-asm-calls/quiz.md#1-call-與-ret)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

`frame.c` 的 `main` 裡有 `call 1149 <add3>`（位址 `0x1194`），下一條指令在 `0x1199`；`add3` 最後一條是 `ret`。

完整原始碼（檔案：[`frame.c`](../small-project/s1-05-asm-calls/frame.c)）與編譯指令：

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

```asm
# main
1194:	call   1149 <add3>
1199:	mov    %rax,-0x8(%rbp)
# add3 的最後
1177:	pop    %rbp
1178:	ret
```

#### 組語二 1-1. `call` 做了哪兩件事？

```
```

#### 組語二 1-2. `ret` 做了什麼？

```
```

#### 組語二 1-3. `ret` 怎麼知道要回到 `0x1199`？

```
```

#### 組語二 1-4. 如果 `add3` 裡有個陣列越界寫入，剛好蓋掉了堆疊上的返回位址，`ret` 會發生什麼事？

```
```

### 組語二 4. callee-saved 與對齊

> 盒子 2｜過去的作答：[s1-05 quiz.md 第 4 題](../small-project/s1-05-asm-calls/quiz.md#4-callee-saved-與對齊)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

下面兩個函式用 `-O2` 編譯（`g` 只有宣告）。

```c
/* saved.c */                          /* noret.c */
long g(long x);                         long g(long x);

long f(long x)                          long f(long x)
{                                       {
    long y = g(x);                          long y = g(x);
    return x + y;                           return y * 2;
}                                       }
```

完整原始碼（檔案：[`saved.c`](../small-project/s1-05-asm-calls/saved.c)、[`noret.c`](../small-project/s1-05-asm-calls/noret.c)；`g` 只有宣告，只編成 `.o`）與編譯指令：

```sh
gcc -O2 -c -fno-asynchronous-unwind-tables saved.c -o saved.o
gcc -O2 -c -fno-asynchronous-unwind-tables noret.c -o noret.o
objdump -d --no-show-raw-insn saved.o noret.o
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

#### 組語二 4-1. 為什麼 `saved.c` 的 `f` 要 `push %rbx`，而不是讓 `x` 留在 `%rdi`？

```
```

#### 組語二 4-2. `noret.c` 的 `f` 不需要保存任何值，為什麼還要 `sub $0x8,%rsp`？

```
```

---

## 多檔：[多檔案的C程式-標頭檔、static與extern、連結錯誤與make](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html)

### 多檔 3. `static` 的兩種意思

> 盒子 2｜過去的作答：[s1-06 quiz.md 第 3 題](../small-project/s1-06-multi-file/quiz.md#3-static-的兩種意思)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

`static` 寫在函式外面和寫在函式裡面，各改變了什麼？用下面兩個例子說明：(a) `a.c`、`b.c` 各有一個 `static int helper(void)`，為什麼能一起連結，拿掉 `static` 為什麼就 `multiple definition`？(b) `next_id()` 呼叫三次回傳 1、2、3，`plain_id()` 每次都是 1，兩個 `n` 各放在記憶體的哪裡？

```c
/* a.c（b.c 一樣，只是 helper 回傳 2、函式叫 b_value） */
static int helper(void)
{
    return 1;
}

int a_value(void)
{
    return helper();
}
```
```c
/* nextid.c 的兩個函式 */
int next_id(void)
{
    static int n = 0;
    n = n + 1;
    return n;
}

int plain_id(void)
{
    int n = 0;
    n = n + 1;
    return n;
}
```

#### 多檔 3-1. `static` 寫在函式外面和寫在函式裡面，各改變了什麼？

```
```

#### 多檔 3-2. (a) `a.c`、`b.c` 各有一個 `static int helper(void)`，為什麼能一起連結，拿掉 `static` 為什麼就 `multiple definition`？

```
```

#### 多檔 3-3. (b) `next_id()` 呼叫三次回傳 1、2、3，`plain_id()` 每次都是 1，兩個 `n` 各放在記憶體的哪裡？

```
```

### 多檔 4. 標頭檔裡的 `long hits;`

> 盒子 2｜過去的作答：[s1-06 quiz.md 第 4 題](../small-project/s1-06-multi-file/quiz.md#4-標頭檔裡的-long-hits)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

下面的 `hits.h` 被 `hits.c` 和 `hits_main.c` 都 `#include`，兩個 `.c` 都沒有再寫 `hits` 的定義。本機 gcc 13 連結時出現 `multiple definition of 'hits'`。為什麼？已經有 include guard 了，為什麼沒擋住？該怎麼改（標頭檔寫什麼、哪裡寫什麼）？為什麼同樣的程式碼在舊版 gcc 上可以連結？

```c
/* hits.h */
#ifndef HITS_H
#define HITS_H

long hits;
void hit(void);

#endif
```
```c
/* hits.c */
#include "hits.h"

void hit(void)
{
    hits = hits + 1;
}
```
```c
/* hits_main.c */
#include <stdio.h>
#include "hits.h"

int main(void)
{
    hit();
    hit();
    hit();
    printf("hits = %ld\n", hits);
    return 0;
}
```
編譯與連結：`gcc -c hits.c hits_main.c && gcc -o hits hits_main.o hits.o`

#### 多檔 4-1. 為什麼？

```
```

#### 多檔 4-2. 已經有 include guard 了，為什麼沒擋住？

```
```

#### 多檔 4-3. 該怎麼改（標頭檔寫什麼、哪裡寫什麼）？

```
```

#### 多檔 4-4. 為什麼同樣的程式碼在舊版 gcc 上可以連結？

```
```

