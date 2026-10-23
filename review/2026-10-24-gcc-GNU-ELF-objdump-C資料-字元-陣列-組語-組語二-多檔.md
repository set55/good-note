# 2026-10-24 複習：gcc、GNU、ELF、objdump、C資料、字元、陣列、組語、組語二、多檔

> 依[複習排程](../learning/複習排程.md)出今天到期的全部題目（每天出完所有到期題，2026-10-23 起的規則），共 **28 題**（61 個作答框），沒有過期的。
> 其中 24 題是 10-23 大複習沒拿到「對」、回到或留在盒子 1 的題目；另外 4 題（gcc 1、GNU 2、C資料 1、C資料 5）是 10-10 答對、排在今天的盒子 4。interp 3 原本也在今天，10-24 改判為對、移到 10-26；字元 2 是 10-23 作答時沒存到、10-24 補登為對，移到 11-06；多檔 1 在 10-24 改判為對，移到 10-26。
> 題目依筆記分節、節內照題號排；每題照原題複製，用到的程式碼與實驗步驟都附在題目裡，「過去的作答」連到之前每一次作答（含 10-23 的批改與參考答案）。
>
> **怎麼寫**：闔上筆記，每題寫 1～2 句關鍵句就好。可以分幾次寫，寫完一部分就可以先說「幫我批改」。複習不重答，批改會直接附參考答案。
> 已拆小題的題目，每個小題一個作答框；以大題為單位排程，小題全對才算對。

## 今天的題目

| # | 題目 | 盒子 | 原本該考的日期 |
|---|---|---|---|
| 1 | gcc 1 | 4 | 2026-10-24 |
| 2 | gcc 2 | 1 | 2026-10-24 |
| 3 | gcc 5 | 1 | 2026-10-24 |
| 4 | GNU 2 | 4 | 2026-10-24 |
| 5 | ELF 4 | 1 | 2026-10-24 |
| 6 | objdump 4 | 1 | 2026-10-24 |
| 7 | C資料 1 | 4 | 2026-10-24 |
| 8 | C資料 4 | 1 | 2026-10-24 |
| 9 | C資料 5 | 4 | 2026-10-24 |
| 10 | C資料 6 | 1 | 2026-10-24 |
| 11 | C資料 9 | 1 | 2026-10-24 |
| 12 | 字元 5 | 1 | 2026-10-24 |
| 13 | 陣列 4（2 小題） | 1 | 2026-10-24 |
| 14 | 陣列 5（3 小題） | 1 | 2026-10-24 |
| 15 | 陣列 7（4 小題） | 1 | 2026-10-24 |
| 16 | 組語 7（4 小題） | 1 | 2026-10-24 |
| 17 | 組語 9（2 小題） | 1 | 2026-10-24 |
| 18 | 組語 11（3 小題） | 1 | 2026-10-24 |
| 19 | 組語 15（3 小題） | 1 | 2026-10-24 |
| 20 | 組語二 1（4 小題） | 1 | 2026-10-24 |
| 21 | 組語二 2（3 小題） | 1 | 2026-10-24 |
| 22 | 組語二 3（4 小題） | 1 | 2026-10-24 |
| 23 | 組語二 4（2 小題） | 1 | 2026-10-24 |
| 24 | 多檔 2（2 小題） | 1 | 2026-10-24 |
| 25 | 多檔 3（3 小題） | 1 | 2026-10-24 |
| 26 | 多檔 4（4 小題） | 1 | 2026-10-24 |
| 27 | 多檔 5（3 小題） | 1 | 2026-10-24 |
| 28 | 多檔 6（3 小題） | 1 | 2026-10-24 |

---

## gcc：[gcc最佳化等級-O0到O3差在哪](../html/c/gcc最佳化等級-O0到O3差在哪.html)

### gcc 1. `-O0` 的 `O` 和 `0` 各是什麼？為什麼說它是「預設值」——不加任何 `-O` 時會發生什麼？

> 盒子 4｜過去的作答：[s1-01 quiz.md 第 1 題](../small-project/s1-01-hello-compile/quiz.md#1--o0-的-o-和-0-各是什麼為什麼說它是預設值不加任何--o-時會發生什麼)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md)

```
```

### gcc 2. 為什麼 `-O0` 的組合語言每一圈都在讀寫記憶體（`-4(%rbp)`），而 `-O2` 不會？這個差別對 `gdb` 有什麼影響？

> 盒子 1｜過去的作答：[s1-01 quiz.md gcc 第 2 題](../small-project/s1-01-hello-compile/quiz.md#2-為什麼--o0-的組合語言每一圈都在讀寫記憶體-4rbp而--o2-不會這個差別對-gdb-有什麼影響)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

> 盒子 1｜過去的作答：[s1-01 quiz.md 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-情境題你在自己的程式裡用--o2-編譯gdb-印某個變數時顯示-optimized-out你會怎麼處理如果是在-kernel-裡遇到同樣的狀況處理方式又有什麼不同)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## GNU：[GNU是什麼-Linux核心與GNU工具的分工](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html)

### GNU 2. 目標三元組 `x86_64-linux-gnu` 的三段各代表什麼？為什麼說它剛好說明了「核心」和「C 函式庫」是兩件事？

> 盒子 4｜過去的作答：[s1-01 quiz.md 第 2 題](../small-project/s1-01-hello-compile/quiz.md#2-目標三元組-x86_64-linux-gnu-的三段各代表什麼為什麼說它剛好說明了核心和c-函式庫是兩件事)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md)

```
```

---

## ELF：[ELF是什麼-可執行與可連結格式的兩種視角](../html/c/ELF是什麼-可執行與可連結格式的兩種視角.html)

### ELF 4. 區段（section）和載入段（segment）差在哪？為什麼核心載入時要把很多區段打包成少數幾個載入段？

> 盒子 1｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-區段section和載入段segment差在哪為什麼核心載入時要把很多區段打包成少數幾個載入段)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#elf-4-區段section和載入段segment差在哪為什麼核心載入時要把很多區段打包成少數幾個載入段)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## objdump：[objdump能不能代替ldd找依賴庫-需求單與實際載入](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html)

### objdump 4. 情境題：同事傳來一支 ARM 開發板用的執行檔，問你「它需要哪些函式庫」。你在自己的 x86-64 電腦上該用哪個工具？用 `ldd` 會發生什麼事，為什麼？

> 盒子 1｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-情境題同事傳來一支-arm-開發板用的執行檔問你它需要哪些函式庫你在自己的-x86-64-電腦上該用哪個工具用-ldd-會發生什麼事為什麼)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## C資料：[C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html)

### C資料 1. 一個 `int` 變數存著 `-1`，用 `%d` 印出 `-1`，轉成 `unsigned` 再用 `%u` 印出 `4294967295`。記憶體裡的內容有沒有變？為什麼同一個東西會印出兩個不同的數？

> 盒子 4｜過去的作答：[s1-02 quiz.md 第 1 題](../small-project/s1-02-data-repr/quiz.md#1-一個-int-變數存著--1用-d-印出--1轉成-unsigned-再用-u-印出-4294967295記憶體裡的內容有沒有變為什麼同一個東西會印出兩個不同的數)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md)

```
```

### C資料 4. `int y = x + 1; return y > x;` 在 `x = INT_MAX` 時，`-O0` 回傳 0、`-O2` 回傳 1。為什麼會這樣？這算不算 gcc 的 bug？kernel 用哪個選項避免這種情況，代價是什麼？

> 盒子 1｜過去的作答：[s1-02 quiz.md 第 4 題](../small-project/s1-02-data-repr/quiz.md#4-int-y--x--1-return-y--x-在-x--int_max-時-o0-回傳-0-o2-回傳-1為什麼會這樣這算不算-gcc-的-bugkernel-用哪個選項避免這種情況代價是什麼)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

### C資料 5. 情境題：你從網路封包裡讀到 4 個 byte，依序是 `00 00 01 00`。協定規定數字用大端序，所以它代表 256。如果直接用 `int *` 把這 4 個 byte 讀進本機（x86-64），會得到多少？為什麼？

> 盒子 4｜過去的作答：[s1-02 quiz.md 第 5 題](../small-project/s1-02-data-repr/quiz.md#5-情境題你從網路封包裡讀到-4-個-byte依序是-00-00-01-00協定規定數字用大端序所以它代表-256如果直接用-int--把這-4-個-byte-讀進本機x86-64會得到多少為什麼)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md)

```
```

### C資料 6. `printf("%p")` 印出 `0x7ffe50b23624`，只有 12 位十六進位。指標到底佔幾個 byte？印出來為什麼比較短？如果在 kernel 的錯誤訊息裡看到 `0xffff888102a4c000`，從位址的長相你能判斷什麼？

> 盒子 1｜過去的作答：[s1-02 quiz.md 第 6 題](../small-project/s1-02-data-repr/quiz.md#6-printfp-印出-0x7ffe50b23624只有-12-位十六進位指標到底佔幾個-byte印出來為什麼比較短如果在-kernel-的錯誤訊息裡看到-0xffff888102a4c000從位址的長相你能判斷什麼)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

### C資料 9. 情境題：有人寫了 `if (len - used > 0)` 判斷「緩衝區還有沒有剩餘空間」，`len`、`used` 都是 `size_t`，用 `-Wall -Wextra` 編譯沒有任何警告。這個判斷在什麼情況下會出錯？為什麼 gcc 對 `len - used < 0` 會警告、對 `> 0` 卻不會？另外，為什麼 `int sum = x + y; if (sum < x)` 這種溢位檢查在 `-O2` 下可能完全失效？

> 盒子 1｜過去的作答：[s1-02 quiz.md 第 9 題](../small-project/s1-02-data-repr/quiz.md#9-情境題有人寫了-if-len---used--0-判斷緩衝區還有沒有剩餘空間lenused-都是-size_t用--wall--wextra-編譯沒有任何警告這個判斷在什麼情況下會出錯為什麼-gcc-對-len---used--0-會警告對--0-卻不會另外為什麼-int-sum--x--y-if-sum--x-這種溢位檢查在--o2-下可能完全失效)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## 字元：[字元是什麼-char就是1byte的整數與ASCII和UTF-8](../html/c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.html)

### 字元 5. 有人說「『字』是 3 個字元」，另一個人說「是 1 個字元」，誰對？`strlen("字")` 和 `mbstowcs(NULL, "字", 0)` 各會得到多少？如果程式忘了呼叫 `setlocale`，後者會得到什麼，那個數字是怎麼來的？

> 盒子 1｜過去的作答：[s1-02 quiz.md 第 5 題](../small-project/s1-02-data-repr/quiz.md#5-有人說字是-3-個字元另一個人說是-1-個字元誰對strlen字-和-mbstowcsnull-字-0-各會得到多少如果程式忘了呼叫-setlocale後者會得到什麼那個數字是怎麼來的)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## 陣列：[陣列、字串與指標運算-C不檢查邊界與用gdb找segfault](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html)

### 陣列 4. 越界的三種下場

#### 陣列 4-1. 越界存取有三種下場：安靜出錯、`stack smashing detected`、segfault。分別是什麼決定會落到哪一種？

> 盒子 1｜過去的作答：[s1-03 quiz.md 4-1](../small-project/s1-03-arrays-pointers/quiz.md#4-1-越界存取有三種下場安靜出錯stack-smashing-detectedsegfault分別是什麼決定會落到哪一種)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

#### 陣列 4-2. 為什麼「程式沒當掉」不能證明沒有越界？

```
```

### 陣列 5. 情境題：當掉的那一行

#### 陣列 5-1. 同事的程式偶爾 segfault，gdb 的 `bt` 顯示當在 `parse()` 裡 `x = buf[k];` 這一行，`k` 的值是 523871，而 `buf` 只有 256 個元素。你接下來要怎麼查出真正的錯誤在哪裡？

> 盒子 1｜過去的作答：[s1-03 quiz.md 5-1](../small-project/s1-03-arrays-pointers/quiz.md#5-1-同事的程式偶爾-segfaultgdb-的-bt-顯示當在-parse-裡-x--bufk-這一行k-的值是-523871而-buf-只有-256-個元素你接下來要怎麼查出真正的錯誤在哪裡)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

#### 陣列 5-2. 為什麼不能直接修改這一行就算解決？

> 題目：同事的程式 segfault 在 `parse()` 的 `x = buf[k];`，`k` 是 523871、`buf` 只有 256 個元素。為什麼不能直接修改這一行就算解決？

```
```

#### 陣列 5-3. 如果想在「第一次越界的當下」就抓到它，要怎麼做？

> 題目：同上的 `x = buf[k];` segfault。如果想在「第一次越界的當下」就抓到它，要怎麼做？

```
```

### 陣列 7. 情境題：讀懂 ASan 的影子 byte

> ASan 報告的影子 byte 那一列裡，某個堆疊上的 `char` 陣列對應到 `00 00 06`，前後都是 `f2`。
>
> 盒子 1｜過去的作答：2026-10-11 在對話裡作答（筆記[第七節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#七addresssanitizer在第一次越界的當下抓到它)〈上圖的 `01`：影子 byte 的值是「個數」，不是資料〉的追問）、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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
> 盒子 1｜過去的作答：[s1-04 quiz.md 第 7 題](../small-project/s1-04-asm-basics/quiz.md#7-參數只佔暫存器的一部分2026-10-06-課後提問新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

### 組語 9. 符號擴展補的是什麼

> 盒子 1｜過去的作答：[s1-04 quiz.md 第 9 題](../small-project/s1-04-asm-basics/quiz.md#9-符號擴展補的是什麼2026-10-06-課後提問新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

#### 組語 9-1. `movsbl` 把 1 byte 擴展成 4 byte 時，新補的高 3 byte 是照哪一個位元補的？

```
```

#### 組語 9-2. 為什麼這樣補，`signed char` 的 −1 變成 `int` 之後才會還是 −1（用二補數的權重解釋）？

```
```

### 組語 11. gdb 的 `break *位址`

> `break deref+4` 會得到 `Function "deref+4" not defined.`，`break *deref+4` 卻可以。
>
> 盒子 1｜過去的作答：[s1-04 quiz.md 第 11 題](../small-project/s1-04-asm-basics/quiz.md#11-gdb-的-break-位址2026-10-06-課後提問新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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
> 盒子 1｜過去的作答：[s1-04 quiz.md 第 15 題](../small-project/s1-04-asm-basics/quiz.md#15-減法的借位與-cf2026-10-07-課後提問新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

---

## 組語二：[x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html)

### 組語二 1. `call` 與 `ret`

> 盒子 1｜過去的作答：[s1-05 quiz.md 第 1 題](../small-project/s1-05-asm-calls/quiz.md#1-call-與-ret)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

### 組語二 2. 堆疊框

> 盒子 1｜過去的作答：[s1-05 quiz.md 第 2 題](../small-project/s1-05-asm-calls/quiz.md#2-堆疊框)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

```asm
add3:                                 main:
  push   %rbp                           push   %rbp
  mov    %rsp,%rbp                      mov    %rsp,%rbp
  mov    %rdi,-0x18(%rbp)               sub    $0x10,%rsp
  ...（沒有 sub、沒有 call）              ...（有 call add3、call printf）
  pop    %rbp                           leave
  ret                                   ret
```

#### 組語二 2-1. 函式開頭的 `push %rbp` 和 `mov %rsp,%rbp` 各在做什麼？

```
```

#### 組語二 2-2. `main` 結尾的 `leave` 等於哪兩條指令？

```
```

#### 組語二 2-3. `add3` 把區域變數放在 `%rsp` 下面、沒有 `sub %rsp`，`main` 卻要 `sub $0x10,%rsp`，為什麼？

```
```

### 組語二 3. 8 個參數

> 盒子 1｜過去的作答：[s1-05 quiz.md 第 3 題](../small-project/s1-05-asm-calls/quiz.md#3-8-個參數)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

`args8.c` 的 `sum8` 有 8 個 `long` 參數。

完整原始碼（檔案：[`args8.c`](../small-project/s1-05-asm-calls/args8.c)，只有兩個函式、沒有 `main`，只編成 `.o` 看組合語言）與編譯指令：

```c
long sum8(long a, long b, long c, long d, long e, long f, long g, long h)
{
    return a + b + c + d + e + f + g + h;
}

long call8(void)
{
    return sum8(1, 2, 3, 4, 5, 6, 7, 8);
}
```

```sh
gcc -O0 -c -fno-asynchronous-unwind-tables args8.c -o args8.o
objdump -d --no-show-raw-insn args8.o
```

```asm
call8:                                sum8 裡讀第 7、8 個參數：
  push   %rbp                           mov    0x10(%rbp),%rax
  mov    %rsp,%rbp                      ...
  push   $0x8                           mov    0x18(%rbp),%rax
  push   $0x7
  mov    $0x6,%r9d
  ...（5～1 分別放進 %r8d、%ecx、%edx、%esi、%edi）
  call   sum8
  add    $0x10,%rsp
  leave
  ret
```

#### 組語二 3-1. 前 6 個參數放在哪裡？

```
```

#### 組語二 3-2. 第 7、8 個放在哪裡、`call8` 用什麼順序放？

```
```

#### 組語二 3-3. 在 `sum8` 裡第 7 個參數為什麼是 `0x10(%rbp)`，而不是 `0(%rbp)`？

```
```

#### 組語二 3-4. `call8` 在 `call` 之後為什麼要 `add $0x10,%rsp`？

```
```

### 組語二 4. callee-saved 與對齊

> 盒子 1｜過去的作答：[s1-05 quiz.md 第 4 題](../small-project/s1-05-asm-calls/quiz.md#4-callee-saved-與對齊)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

### 多檔 2. 定義的 `.c` 也要引入自己的標頭檔

> 盒子 1｜過去的作答：[s1-06 quiz.md 第 2 題](../small-project/s1-06-multi-file/quiz.md#2-定義的-c-也要引入自己的標頭檔)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

為什麼「定義函式的那個 `.c` 也要 `#include` 自己的標頭檔」？如果 `mathx.c` 沒有 `#include "mathx.h"`，有人把 `mathx.c` 的定義改成 `int square(int x)` 卻忘了改 `mathx.h`，會發生什麼事、在哪個階段（編譯、連結、執行）才發現？

```c
/* mathx.h */
#ifndef MATHX_H
#define MATHX_H

long square(long x);

#endif
```

#### 多檔 2-1. 為什麼「定義函式的那個 `.c` 也要 `#include` 自己的標頭檔」？

```
```

#### 多檔 2-2. 如果 `mathx.c` 沒有 `#include "mathx.h"`，有人把 `mathx.c` 的定義改成 `int square(int x)` 卻忘了改 `mathx.h`，會發生什麼事、在哪個階段（編譯、連結、執行）才發現？

> 題目補充（2026-10-22 批改後補上，原題沒寫清楚）：呼叫端 `main.c` **有** `#include "mathx.h"`，內容是：
>
> ```c
> #include <stdio.h>
> #include "mathx.h"
>
> int main(void)
> {
>     long r = square(100000);
>     printf("%ld\n", r);
>     return 0;
> }
> ```

> 題目：如果 `mathx.c` 沒有 `#include "mathx.h"`，有人把 `mathx.c` 的定義改成 `int square(int x)` 卻忘了改 `mathx.h`，會發生什麼事、在哪個階段（編譯、連結、執行）才發現？

```
```

### 多檔 3. `static` 的兩種意思

> 盒子 1｜過去的作答：[s1-06 quiz.md 第 3 題](../small-project/s1-06-multi-file/quiz.md#3-static-的兩種意思)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

> 盒子 1｜過去的作答：[s1-06 quiz.md 第 4 題](../small-project/s1-06-multi-file/quiz.md#4-標頭檔裡的-long-hits)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

### 多檔 5. 情境題：改了標頭檔，`make` 說不用重編

> 盒子 1｜過去的作答：[s1-06 quiz.md 第 5 題](../small-project/s1-06-multi-file/quiz.md#5-情境題改了標頭檔make-說不用重編)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

情境題：同事的專案用下面的 `Makefile`。他把 `config.h` 裡的 `#define BUF_SIZE 64` 改成 `128`，存檔後執行 `make`，`make` 只印了 `make: 'server' is up to date.`，跑起來的緩衝區還是 64。`make` 是怎麼判斷不用重編的？為什麼這個判斷錯了？你會怎麼改這個 `Makefile`，讓以後改任何標頭檔都會重編該重編的檔案？（不必寫出完整的 `Makefile`，說明要改哪些地方、各自的作用即可。）

```make
server: server.o net.o
	gcc -o server server.o net.o

server.o: server.c
	gcc -Wall -c server.c

net.o: net.c
	gcc -Wall -c net.c
```
（`server.c` 和 `net.c` 都 `#include "config.h"`；命令前面是 TAB。）

#### 多檔 5-1. `make` 是怎麼判斷不用重編的？

```
```

#### 多檔 5-2. 為什麼這個判斷錯了？

```
```

#### 多檔 5-3. 你會怎麼改這個 `Makefile`，讓以後改任何標頭檔都會重編該重編的檔案？（不必寫出完整的 `Makefile`，說明要改哪些地方、各自的作用即可。）

```
```

### 多檔 6. 符號是什麼：`t.o` 的符號表裡有誰

> 盒子 1｜過去的作答：[s1-06 quiz.md 第 6 題](../small-project/s1-06-multi-file/quiz.md#6-符號是什麼to-的符號表裡有誰)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

（2026-10-20 提問「符號代表什麼」追加，出處：[第一節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#一本課在做什麼編譯器一次只看一個-c)〈上面說的「符號」到底是什麼〉）下面的 `t.c` 用 `gcc -c t.c` 編成 `t.o`。哪些名字會出現在 `t.o` 的符號表裡？其中哪些是「這個檔案提供」、哪些是「需要別人提供」？`n`、`sum`、`i` 為什麼不在符號表裡？對連結器來說，符號表裡的一個符號代表什麼、拿來做什麼用？

```c
#include <stdio.h>

long total = 0;
extern long limit;

void add(long n)
{
    long sum = total + n;
    total = sum;
}

int main(void)
{
    for (int i = 0; i < 3; i++)
        add(i);
    if (total > limit)
        puts("big");
    return 0;
}
```

編譯：`gcc -c t.c`（只編不連結；`limit` 沒有定義，連結會失敗，這題不用連結）

#### 多檔 6-1. 哪些名字會出現在 `t.o` 的符號表裡？其中哪些是「這個檔案提供」、哪些是「需要別人提供」？

```
```

#### 多檔 6-2. `n`、`sum`、`i` 為什麼不在符號表裡？

```
```

#### 多檔 6-3. 對連結器來說，符號表裡的一個符號代表什麼、拿來做什麼用？

```
```
