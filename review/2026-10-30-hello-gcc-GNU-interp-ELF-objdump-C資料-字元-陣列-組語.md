# 2026-10-30 複習：hello、gcc、GNU、interp、ELF、objdump、C資料、字元、陣列、組語

> 依[複習排程](../learning/複習排程.md)出今天到期的全部題目（每天出完所有到期題，2026-10-23 起的規則），共 **21 題**（33 個作答框），沒有過期的。
> 其中 3 題在盒子 2（C資料 4、C資料 9 是 10-27 大致對留在盒子 2；組語 18 是 10-27 改判為對、升到盒子 2）；其餘 18 題是 10-23 大複習後排在盒子 3 的題目（含 3 題大致對留在盒子 3：hello 2、hello 3、interp 6）。
> 題目依筆記分節、節內照題號排；每題照原題複製，用到的程式碼與實驗步驟都附在題目裡，「過去的作答」連到之前每一次作答（含批改與參考答案）。
>
> **怎麼寫**：闔上筆記，每題寫 1～2 句關鍵句就好。可以分幾次寫，寫完一部分就可以先說「幫我批改」。複習不重答，批改會直接附參考答案。
> 已拆小題的題目，每個小題一個作答框；以大題為單位排程，小題全對才算對。

## 今天的題目

排序：盒子小的在前（盒子 2 的 3 題），其餘依上次作答日期；同一天的照筆記順序。

| # | 題目 | 盒子 | 原本該考的日期 |
|---|---|---|---|
| 1 | C資料 4 | 2 | 2026-10-30 |
| 2 | C資料 9 | 2 | 2026-10-30 |
| 3 | 組語 18（4 小題） | 2 | 2026-10-30 |
| 4 | hello 2 | 3 | 2026-10-30 |
| 5 | hello 3 | 3 | 2026-10-30 |
| 6 | gcc 3 | 3 | 2026-10-30 |
| 7 | gcc 4 | 3 | 2026-10-30 |
| 8 | GNU 5 | 3 | 2026-10-30 |
| 9 | GNU 6 | 3 | 2026-10-30 |
| 10 | interp 6 | 3 | 2026-10-30 |
| 11 | ELF 2 | 3 | 2026-10-30 |
| 12 | ELF 3 | 3 | 2026-10-30 |
| 13 | ELF 5 | 3 | 2026-10-30 |
| 14 | objdump 1 | 3 | 2026-10-30 |
| 15 | C資料 7 | 3 | 2026-10-30 |
| 16 | 字元 4 | 3 | 2026-10-30 |
| 17 | 陣列 1（3 小題） | 3 | 2026-10-30 |
| 18 | 陣列 6（4 小題） | 3 | 2026-10-30 |
| 19 | 組語 2（2 小題） | 3 | 2026-10-30 |
| 20 | 組語 3（2 小題） | 3 | 2026-10-30 |
| 21 | 組語 5（3 小題） | 3 | 2026-10-30 |

---

## hello：[hello.c怎麼變成能跑的程式-編譯的四個階段](../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html)

### hello 2. `hello.i` 裡有 `printf` 的宣告，卻沒有它的程式碼。那它的程式碼在哪？是在哪個時間點、由誰把「呼叫 `printf`」和「`printf` 的程式碼」接起來的？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 2 題](../small-project/s1-01-hello-compile/quiz.md#2-helloi-裡有-printf-的宣告卻沒有它的程式碼那它的程式碼在哪是在哪個時間點由誰把呼叫-printf和printf-的程式碼接起來的)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

### hello 3. `hello.o` 裡「呼叫 `puts`」的地方，要跳去的位址為什麼暫時是 0（空著）？什麼資訊讓連結器知道之後要把它填成什麼？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-helloo-裡-call-指令後面的四個-byte-為什麼是-00-00-00-00什麼資訊讓連結器知道要把它填成什麼)（舊題目文字）、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md#hello-3-helloo-裡-call-指令後面的四個-byte-為什麼是-00-00-00-00什麼資訊讓連結器知道要把它填成什麼)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

題目用到的程式 `hello.c`（檔案：[`hello.c`](../small-project/s1-01-hello-compile/hello.c)）完整原始碼：

```c
#include <stdio.h>

#define GREETING "hello, world"

int main(void)
{
    printf("%s\n", GREETING);
    return 0;
}
```

產生 `hello.o` 的指令（只編譯＋組譯、不連結）：

```sh
gcc -c hello.c -o hello.o
```

（原始碼寫的是 `printf`，gcc 把「`printf("%s\n", 字串)`」換成了 `puts(字串)`，所以 `hello.o` 裡呼叫的是 `puts`——見筆記第三節。）

> 題目：`hello.o` 裡「呼叫 `puts`」的地方，要跳去的位址為什麼暫時是 0（空著）？什麼資訊讓連結器知道之後要把它填成什麼？

```
```

---

## gcc：[gcc最佳化等級-O0到O3差在哪](../html/c/gcc最佳化等級-O0到O3差在哪.html)

### gcc 3. 在 `-O2` 下，`answer()` 連呼叫 `sum_to` 這件事都不見了，直接回傳 5050。編譯器做了哪兩件事才能做到？

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

`-O0` 與 `-O2` 的 `answer`（本機 gcc 13.3 實際輸出，刪掉組譯指示）：

```asm
# -O0
answer:
	endbr64
	pushq	%rbp
	movq	%rsp, %rbp
	movl	$100, %edi         # 第一個參數 = 100
	call	sum_to             # 呼叫 sum_to
	popq	%rbp
	ret                        # 回傳值就是 sum_to 留在 %eax 的結果

# -O2
answer:
	endbr64
	movl	$5050, %eax        # 回傳值直接設成 5050
	ret
```

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3--o2-下-answer-直接變成-movl-5050-eax編譯器做了哪兩件事才能做到)、[10-05 複習](./2026-10-05-gcc-陣列-GNU-C資料-字元.md)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

### gcc 4. 「最佳化等級越高，程式越小」對嗎？`-O2`／`-O3` 和 `-Os`／`-Oz` 的目標各是什麼？為什麼追求速度反而可能讓程式變大？

（2026-10-09 改寫的題目：原題「用本篇的數字說明」要求背數字，闔上筆記作答時無從引用。不用寫出確切的 byte 數。）

**題目用到的程式**：同上一題 gcc 3 的 `sumto.c`。同一份 `sumto.c` 用不同的最佳化等級各編成 `.o`，比 `.text` 區段（放機器碼的區段）的大小，例如：

```sh
gcc -O2 -c sumto.c -o sumto_O2.o   # -c：編譯成目的檔就停，不連結
size -A sumto_O2.o                 # -A：逐個區段列出大小，看 .text 那一列
```

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-最佳化等級越高程式越小對嗎用本篇的數字說明)、[10-05 複習](./2026-10-05-gcc-陣列-GNU-C資料-字元.md)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## GNU：[GNU是什麼-Linux核心與GNU工具的分工](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html)

### GNU 5. 你將來送 kernel patch 時要加的 `Signed-off-by:` 跟 GPL 有什麼關係？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-你將來送-kernel-patch-時要加的-signed-off-by-跟-gpl-有什麼關係)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md#gnu-5-你將來送-kernel-patch-時要加的-signed-off-by-跟-gpl-有什麼關係)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

### GNU 6. 情境題：同事說「我在這台機器上打 `uname -o` 看到 `GNU/Linux`，所以它一定用 glibc」。這個推論錯在哪？`uname` 的哪些欄位可以信、哪一欄不行，為什麼？你會改看什麼來判斷？

> 盒子 3｜過去的作答：[s1-01 quiz.md GNU 第 6 題](../small-project/s1-01-hello-compile/quiz.md#6-情境題同事說我在這台機器上打-uname--o-看到-gnulinux所以它一定用-glibc這個推論錯在哪uname-的哪些欄位可以信哪一欄不行為什麼你會改看什麼來判斷)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## interp：[ELF的interpreter是什麼-動態載入器與井字號驚嘆號](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html)

### interp 6. 情境題：一支腳本權限是 `---x--x--x`（只有執行權限），第一行是 `#!/bin/sh`，執行時出現 `/bin/sh: ./x.sh: Permission denied`。這個錯誤是誰印的？為什麼核心讀得到 `#!` 那一行，直譯器卻讀不到腳本？同樣只有執行權限的 ELF 執行檔為什麼就能跑？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 6 題](../small-project/s1-01-hello-compile/quiz.md#6-情境題一支腳本權限是----x--x--x只有執行權限第一行是-binsh執行時出現-binsh-xsh-permission-denied這個錯誤是誰印的為什麼核心讀得到--那一行直譯器卻讀不到腳本同樣只有執行權限的-elf-執行檔為什麼就能跑)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## ELF：[ELF是什麼-可執行與可連結格式的兩種視角](../html/c/ELF是什麼-可執行與可連結格式的兩種視角.html)

### ELF 2. `readelf -l hello.o` 說 `There are no program headers in this file`，這代表什麼？連結器做了什麼才讓執行檔有程式標頭？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 2 題](../small-project/s1-01-hello-compile/quiz.md#2-readelf--l-helloo-說-there-are-no-program-headers-in-this-file這代表什麼連結器做了什麼才讓執行檔有程式標頭)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

**題目用到的實驗**（[ELF 第四節](../html/c/ELF是什麼-可執行與可連結格式的兩種視角.html#四名字的關鍵兩種視角)）：把 `hello.c` 只編譯不連結得到 `hello.o`，對它跑 `readelf -l`（`-l`：印程式標頭表）；再連結成執行檔，對執行檔跑同一個命令對照。

`hello.c`（檔案：[`hello.c`](../small-project/s1-01-hello-compile/hello.c)）：

```c
#include <stdio.h>

#define GREETING "hello, world"

int main(void)
{
	printf("%s\n", GREETING);
	return 0;
}
```

```sh
$ gcc -c hello.c          # -c：編譯＋組譯，停在 .o，不連結
$ readelf -l hello.o      # 題目裡的訊息就是這一步印的
$ gcc -o hello hello.o    # 連結成執行檔
$ readelf -l hello        # 對照
```

> 題目：`readelf -l hello.o` 說 `There are no program headers in this file`，這代表什麼？連結器做了什麼才讓執行檔有程式標頭？

```
```

### ELF 3. 本機一般 `gcc` 編出的執行檔，`Type` 為什麼是 DYN，跟 `libc.so.6` 同一類，而靜態版是 EXEC？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-本機一般-gcc-編出的執行檔type-為什麼是-dyn跟-libcso6-同一類而靜態版是-exec)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#elf-3-本機一般-gcc-編出的執行檔type-為什麼是-dyn跟-libcso6-同一類而靜態版是-exec)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

### ELF 5. 情境題：你把 Ubuntu（x86-64）上編好的程式複製到路由器（ARM），`file` 顯示它是 ELF，執行卻失敗。這跟 [interpreter 那篇](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html) 第五節「interpreter 不存在」是不是同一種問題？從檔頭的哪一欄可以看出來？

> 盒子 3｜過去的作答：[s1-01 quiz.md ELF 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-情境題你把-ubuntux86-64上編好的程式複製到路由器armfile-顯示它是-elf執行卻失敗這跟-interpreter-那篇-第五節interpreter-不存在是不是同一種問題從檔頭的哪一欄可以看出來)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

題目提到的「interpreter 不存在」實驗（interp 筆記[第五節](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html#五直譯器不存在時明明有檔案卻說找不到)）：把 `hello.c`（同上面 ELF 2）連結時故意把 interpreter 路徑改成不存在的檔案，再執行。
`-Wl,` 把後面的選項轉交給連結器 `ld`，`--dynamic-linker` 就是設定 `INTERP` 的選項：

```sh
$ gcc hello.c -o hello_badinterp -Wl,--dynamic-linker=/lib64/nope.so.2
$ ./hello_badinterp
```

> 題目：情境題：你把 Ubuntu（x86-64）上編好的程式複製到路由器（ARM），`file` 顯示它是 ELF，執行卻失敗。這跟 [interpreter 那篇](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html) 第五節「interpreter 不存在」是不是同一種問題？從檔頭的哪一欄可以看出來？

```
```

---

## objdump：[objdump能不能代替ldd找依賴庫-需求單與實際載入](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html)

### objdump 1. `objdump -p /usr/bin/curl` 只列出 3 個 `NEEDED`，`ldd` 卻列出 33 行。多出來的是哪幾類東西？為什麼 `objdump` 看不到？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 1 題](../small-project/s1-01-hello-compile/quiz.md#1-objdump--p-usrbincurl-只列出-3-個-neededldd-卻列出-33-行多出來的是哪幾類東西為什麼-objdump-看不到)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#objdump-1-objdump--p-usrbincurl-只列出-3-個-neededldd-卻列出-33-行多出來的是哪幾類東西為什麼-objdump-看不到)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## C資料：[C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html)

### C資料 4. `int y = x + 1; return y > x;` 在 `x = INT_MAX` 時，`-O0` 回傳 0、`-O2` 回傳 1。為什麼會這樣？這算不算 gcc 的 bug？kernel 用哪個選項避免這種情況，代價是什麼？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 4 題](../small-project/s1-02-data-repr/quiz.md#4-int-y--x--1-return-y--x-在-x--int_max-時-o0-回傳-0-o2-回傳-1為什麼會這樣這算不算-gcc-的-bugkernel-用哪個選項避免這種情況代價是什麼)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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

### C資料 7. `2147483648` 和 `0x80000000` 是同一個數，為什麼一個的型別是 `long`、一個是 `unsigned int`？`-1 < 0x80000000` 和 `-1 < 2147483648` 的結果各是什麼？為什麼？

> 盒子 3｜過去的作答：[s1-02 quiz.md 第 7 題](../small-project/s1-02-data-repr/quiz.md#7-2147483648-和-0x80000000-是同一個數為什麼一個的型別是-long一個是-unsigned-int-1--0x80000000-和--1--2147483648-的結果各是什麼為什麼)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#c資料-7-2147483648-和-0x80000000-是同一個數為什麼一個的型別是-long一個是-unsigned-int-1--0x80000000-和--1--2147483648-的結果各是什麼為什麼)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

### C資料 9. 情境題：有人寫了 `if (len - used > 0)` 判斷「緩衝區還有沒有剩餘空間」，`len`、`used` 都是 `size_t`，用 `-Wall -Wextra` 編譯沒有任何警告。這個判斷在什麼情況下會出錯？為什麼 gcc 對 `len - used < 0` 會警告、對 `> 0` 卻不會？另外，為什麼 `int sum = x + y; if (sum < x)` 這種溢位檢查在 `-O2` 下可能完全失效？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 9 題](../small-project/s1-02-data-repr/quiz.md#9-情境題有人寫了-if-len---used--0-判斷緩衝區還有沒有剩餘空間lenused-都是-size_t用--wall--wextra-編譯沒有任何警告這個判斷在什麼情況下會出錯為什麼-gcc-對-len---used--0-會警告對--0-卻不會另外為什麼-int-sum--x--y-if-sum--x-這種溢位檢查在--o2-下可能完全失效)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## 字元：[字元是什麼-char就是1byte的整數與ASCII和UTF-8](../html/c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.html)

### 字元 4. 情境題：你寫了一個函式，用 `char c` 一個一個 byte 讀一段 UTF-8 文字，並用 `if (c > 127)` 判斷「這是不是非 ASCII 的 byte」。在本機這個判斷會成立嗎？為什麼？該怎麼改？

> 盒子 3｜過去的作答：[s1-02 quiz.md 第 4 題](../small-project/s1-02-data-repr/quiz.md#4-情境題你寫了一個函式用-char-c-一個一個-byte-讀一段-utf-8-文字並用-if-c--127-判斷這是不是非-ascii-的-byte在本機這個判斷會成立嗎為什麼該怎麼改)、[2026-10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md#字元-4-情境題你寫了一個函式用-char-c-一個一個-byte-讀一段-utf-8-文字並用-if-c--127-判斷這是不是非-ascii-的-byte在本機這個判斷會成立嗎為什麼該怎麼改)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

---

## 陣列：[陣列、字串與指標運算-C不檢查邊界與用gdb找segfault](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html)

### 陣列 1. 指標的型別與指標運算

#### 陣列 1-1. `int *p` 和 `unsigned char *c` 存著同一個位址，為什麼 `p + 1` 和 `c + 1` 不一樣？

> 盒子 3｜過去的作答：[s1-03 quiz.md 1-1](../small-project/s1-03-arrays-pointers/quiz.md#1-1-int-p-和-unsigned-char-c-存著同一個位址為什麼-p--1-和-c--1-不一樣)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#陣列-1-1-int-p-和-unsigned-char-c-存著同一個位址為什麼-p--1-和-c--1-不一樣)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
```

#### 陣列 1-2. `p[3]` 在 C 標準裡是怎麼定義的？

```
```

#### 陣列 1-3. 為什麼 `3[p]` 也能編譯，而且值跟 `p[3]` 一樣？

```
```

### 陣列 6. 情境題：讀懂 ASan 的區間

> ASan 報告裡有一行 `[16, 40) 'buf' (line 7) <== Memory access at offset 40 overflows this variable`，上面寫著 `WRITE of size 4`。
>
> 盒子 3｜過去的作答：[s1-03 quiz.md 第 6 題](../small-project/s1-03-arrays-pointers/quiz.md#6-情境題讀懂-asan-的區間2026-09-30-新增)、[10-05 複習](./2026-10-05-gcc-陣列-GNU-C資料-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

#### 陣列 6-1. `buf` 佔幾個 byte？

```
```

#### 陣列 6-2. 如果它是 `int` 陣列，有幾個元素？

> 題目：ASan 報告 `[16, 40) 'buf' (line 7) <== Memory access at offset 40 overflows this variable`，`WRITE of size 4`。如果它是 `int` 陣列，有幾個元素？

```
```

#### 陣列 6-3. 這次寫到的是 `buf` 的第幾格（索引多少）？

> 題目：同上的 ASan 報告。這次寫到的是 `buf` 的第幾格（索引多少）？

```
```

#### 陣列 6-4. 影子 byte 那一列裡，`buf` 會是幾個什麼樣的影子 byte？

> 題目：同上的 ASan 報告。影子 byte 那一列裡，`buf` 會是幾個什麼樣的影子 byte？

```
```

---

## 組語：[x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)

### 組語 2. 寫 32 位元暫存器與擴展

> `long widen_u(unsigned x) { return x; }` 只要一個 `movl %edi, %eax`，`long widen_s(int x) { return x; }` 卻要 `movslq %edi, %rax`。
>
> 盒子 3｜過去的作答：[s1-04 quiz.md 第 2 題](../small-project/s1-04-asm-basics/quiz.md#2-寫-32-位元暫存器與擴展)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

程式（筆記檔案：[`ops.c`](../small-project/s1-04-asm-basics/ops.c) 的其中兩個函式，不需要 `main`）與編譯指令：

```c
long widen_s(int x)       { return x; }
long widen_u(unsigned x)  { return x; }
```

```sh
gcc -S -O2 -fno-asynchronous-unwind-tables ops.c -o ops.s
```

```asm
widen_s:
	endbr64                 # 控制流程保護標記，跟計算無關
	movslq	%edi, %rax      # 把 4 byte 的 %edi 有號擴展成 8 byte 放進 %rax
	ret                     # 回傳 %rax
widen_u:
	endbr64
	movl	%edi, %eax      # 把 %edi 的 4 byte 複製到 %eax
	ret
```

#### 組語 2-1. 為什麼 `movl` 就足以完成前者？

```
```

#### 組語 2-2. 如果改寫 16 位元的 `%ax`，同樣的理由還成立嗎？

```
```

### 組語 3. 有號與無號在哪裡決定

> `lt_s` 和 `lt_u` 的 `cmpl %esi, %edi` 一模一樣，gdb 也看到兩者的旗標完全相同（`[ SF IF ]`）。
>
> 盒子 3｜過去的作答：[s1-04 quiz.md 第 3 題](../small-project/s1-04-asm-basics/quiz.md#3-有號與無號在哪裡決定)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

實驗（筆記[第五節〈實驗：同一個 `cmp`、同一組旗標，`setl` 說是、`setb` 說否〉](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)）：用 `-1` 和 `1` 呼叫兩個函式，在 `cmp` 剛做完時用 gdb 看旗標。

檔案 [`lt.c`](../small-project/s1-04-asm-basics/lt.c)：

```c
/* lt.c：用 -O2 編 */
int lt_s(int a, int b)           { return a < b; }
int lt_u(unsigned a, unsigned b) { return a < b; }
```

檔案 [`ltmain.c`](../small-project/s1-04-asm-basics/ltmain.c)：

```c
/* ltmain.c：另外一個檔，這樣 -O2 的 lt.c 不會被內嵌進 main */
#include <stdio.h>

int lt_s(int a, int b);
int lt_u(unsigned a, unsigned b);

int main(void)
{
    printf("lt_s(-1, 1) = %d\n", lt_s(-1, 1));
    printf("lt_u(-1, 1) = %d\n", lt_u(-1, 1));
    return 0;
}
```

```sh
gcc -O2 -g -c lt.c
gcc -O0 -g ltmain.c lt.o -o lt
./lt
gdb -q -batch -ex 'break *lt_s+8' -ex 'break *lt_u+8' -ex run \
    -ex 'x/i $pc' -ex 'info registers eflags' -ex continue \
    -ex 'x/i $pc' -ex 'info registers eflags' ./lt
```

`lt.c` 編出的組合語言：

```asm
lt_s:
	endbr64                 # 控制流程保護標記
	xorl	%eax, %eax      # %eax = 0
	cmpl	%esi, %edi      # 算 a − b，只設旗標、不存結果
	setl	%al             # 依旗標把 %al 設成 1 或 0（看哪些旗標是本題要答的）
	ret                     # 回傳 %eax
lt_u:
	endbr64
	xorl	%eax, %eax
	cmpl	%esi, %edi      # 完全一樣的比較
	setb	%al             # 依旗標把 %al 設成 1 或 0（看哪些旗標是本題要答的）
	ret
```

#### 組語 3-1. 為什麼一個回傳 1、一個回傳 0？

```
```

#### 組語 3-2. C 的「有號還是無號」在組合語言裡到底是在哪裡決定的？

```
```

### 組語 5. 情境題：從一行反組譯推回 C

> 你在 kernel 的反組譯裡看到 `movq 24(%rdi,%rsi,8), %rax`，`%rdi` 是函式的第 1 個參數、`%rsi` 是第 2 個。推測它對應的 C 可能長怎樣。
>
> 盒子 3｜過去的作答：[s1-04 quiz.md 第 5 題](../small-project/s1-04-asm-basics/quiz.md#5-情境題從一行反組譯推回-c)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

#### 組語 5-1. 第 1 個參數最可能是指向什麼大小的元素的指標？

```
```

#### 組語 5-2. 存取的是第幾個元素（用 `i` 表示）？

> 題目：`movq 24(%rdi,%rsi,8), %rax` 存取的是第幾個元素（用 `i` 表示）？

```
```

#### 組語 5-3. 如果同一個位置看到的是 `leaq` 而不是 `movq`，C 又會是什麼？

> 題目：如果看到的是 `leaq 24(%rdi,%rsi,8), %rax` 而不是 `movq`，C 又會是什麼？

```
```

### 組語 18. `0x01 + 0x7f` 的進位與 CF、OF

> 盒子 2｜過去的作答：2026-10-25 在對話裡作答（18-1 對、18-2 大致對、18-3 大致對、18-4 未作答），批改後的參考答案寫在[筆記第 18 題](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#自我測驗)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

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
