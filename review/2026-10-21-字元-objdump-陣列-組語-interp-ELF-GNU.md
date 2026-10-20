# 2026-10-21 複習：字元、objdump、陣列、組語、interp、ELF、GNU

> 依 [複習排程](../learning/複習排程.md) 選出今天到期的 10 題（2026-10-20 先建好，使用者隔天直接作答）。今天只複習、不上新課（第 6 課的作業與自我測驗還在等回覆），
> 所以全部依**過期最久 → 盒子小 → 上次作答較早**挑選：10-12 到期剩下的字元 1；10-13 到期的 8 題全選（盒子 1 的 objdump 4、陣列 4、組語 1、組語 6，
> 盒子 3 的 interp 1、ELF 1、GNU 3，盒子 4 的 interp 5）；10-14 到期、盒子 1 的 7 題組語（上次作答都是 10-13）依排程表順序選組語 7，湊滿 10 題。
> 今天到期的共 59 題（含 10-20 剛考完、排在 10-21 的 C資料 6、C資料 9 等），沒選到的 49 題順延，日期不動。
>
> **怎麼寫**：闔上筆記，每題寫 1～2 句關鍵句就好。寫完說「幫我批改」。複習不重答，批改會直接附參考答案。
> 陣列 4、組語 1、組語 6、組語 7 已拆成小題，每個小題一個作答框。

## 今天的題目

| # | 題目 | 盒子 | 原本該考的日期 | 為什麼選 |
|---|---|---|---|---|
| 1 | 字元 1 | 3 | 2026-10-12 | 過期最久 |
| 2 | objdump 4 | 1 | 2026-10-13 | 次久、盒子 1 |
| 3 | 陣列 4（2 小題） | 1 | 2026-10-13 | 同上 |
| 4 | 組語 1（3 小題） | 1 | 2026-10-13 | 同上 |
| 5 | 組語 6（4 小題） | 1 | 2026-10-13 | 同上 |
| 6 | interp 1 | 3 | 2026-10-13 | 次久、盒子 3 |
| 7 | ELF 1 | 3 | 2026-10-13 | 同上 |
| 8 | GNU 3 | 3 | 2026-10-13 | 同上 |
| 9 | interp 5 | 4 | 2026-10-13 | 次久、盒子 4 |
| 10 | 組語 7（4 小題） | 1 | 2026-10-14 | 盒子 1，排程表順序 |

---

## 字元：[字元是什麼-char就是1byte的整數與ASCII和UTF-8](../html/c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.html)

### 字元 1. `char c = 'A';` 和 `char c = 65;` 存進記憶體的東西一樣嗎？為什麼 `printf` 用 `%c` 印出 `A`、用 `%d` 印出 65？

> 盒子 3｜過去的作答：[s1-02 quiz.md 字元第 1 題](../small-project/s1-02-data-repr/quiz.md#1-char-c--a-和-char-c--65-存進記憶體的東西一樣嗎為什麼-printf-用-c-印出-a用-d-印出-65)、[10-05 複習](./2026-10-05-gcc-陣列-GNU-C資料-字元.md)

```
```

---

## objdump：[objdump能不能代替ldd找依賴庫-需求單與實際載入](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html)

### objdump 4. 情境題：同事傳來一支 ARM 開發板用的執行檔，問你「它需要哪些函式庫」。你在自己的 x86-64 電腦上該用哪個工具？用 `ldd` 會發生什麼事，為什麼？

> 盒子 1｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-情境題同事傳來一支-arm-開發板用的執行檔問你它需要哪些函式庫你在自己的-x86-64-電腦上該用哪個工具用-ldd-會發生什麼事為什麼)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)

```
```

---

## 陣列：[陣列、字串與指標運算-C不檢查邊界與用gdb找segfault](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html)

### 陣列 4. 越界的三種下場

#### 陣列 4-1. 越界存取有三種下場：安靜出錯、`stack smashing detected`、segfault。分別是什麼決定會落到哪一種？

> 盒子 1｜過去的作答：[s1-03 quiz.md 4-1](../small-project/s1-03-arrays-pointers/quiz.md#4-1-越界存取有三種下場安靜出錯stack-smashing-detectedsegfault分別是什麼決定會落到哪一種)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)

```
```

#### 陣列 4-2. 為什麼「程式沒當掉」不能證明沒有越界？

```
```

---

## 組語：[x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)

### 組語 1. `mov` 與 `lea`

> `movl (%rdi,%rsi,4), %eax` 和 `leaq (%rdi,%rsi,4), %rax` 括號裡寫的一模一樣。
>
> 盒子 1｜過去的作答：[s1-04 quiz.md 第 1 題](../small-project/s1-04-asm-basics/quiz.md#1-mov-與-lea)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)

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

### 組語 6. 組合語言的大小名稱 vs C 的型別

> 盒子 1｜過去的作答：[s1-04 quiz.md 第 6 題](../small-project/s1-04-asm-basics/quiz.md#6-組合語言的大小名稱-vs-c-的型別2026-10-06-課後提問新增)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)

#### 組語 6-1. `movsbl` 的 `l` 叫 long，為什麼是 4 byte，而本機 C 的 `long` 是 8 byte？

```
```

#### 組語 6-2. 本機 C 的 `long` 在組合語言裡對到哪個字尾？

```
```

> 情境題：第 5 課你在 `gdb` 裡想看一個 `int` 變數的 4 個 byte，同學說「`int` 在組合語言是 `l`，所以用 `x/1l`」。

#### 組語 6-3. 這樣對嗎？

```
```

#### 組語 6-4. 應該用哪個大小字母，為什麼？

> 題目：（承上）在 gdb 裡用 `x` 看 `int` 變數的 4 個 byte，應該用哪個大小字母，為什麼？

```
```

### 組語 7. 參數只佔暫存器的一部分

> `int widen_char(signed char c) { return c; }` 編出 `movsbl %dil, %eax`。
>
> 盒子 1｜過去的作答：[s1-04 quiz.md 第 7 題](../small-project/s1-04-asm-basics/quiz.md#7-參數只佔暫存器的一部分2026-10-06-課後提問新增)、[10-13 複習](./2026-10-13-組語-字元.md)

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

> 假設 `widen_char` 改成讀 4 byte 的 `movl %edi, %eax`，在上面的 `gdb` 實驗裡把 `%rdi` 改成 `0x11223344556677ff`。

#### 組語 7-2. 之後 `%eax` 的十六進位是多少？

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

---

## interp：[ELF的interpreter是什麼-動態載入器與井字號驚嘆號](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html)

### interp 1. 動態連結的執行檔裡，interpreter 的路徑寫在哪裡？`execve` 之後，核心先執行的是你的程式還是它？為什麼要這樣安排？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 1 題](../small-project/s1-01-hello-compile/quiz.md#1-動態連結的執行檔裡interpreter-的路徑寫在哪裡execve-之後核心先執行的是你的程式還是它為什麼要這樣安排)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md)

```
```

### interp 5. 情境題：你把 Ubuntu 上編好的程式複製到一台 Alpine Linux，`ls -l` 看得到檔案、也有 x 權限，執行卻說 `not found`。最可能的原因是什麼？你會用哪兩個步驟確認？用第 1 課學過的哪一種編譯方式可以避開這個問題？

> 盒子 4｜過去的作答：[s1-01 quiz.md 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-情境題你把-ubuntu-上編好的程式複製到一台-alpine-linuxls--l-看得到檔案也有-x-權限執行卻說-not-found最可能的原因是什麼你會用哪兩個步驟確認用第-1-課學過的哪一種編譯方式可以避開這個問題)

```
```

---

## ELF：[ELF是什麼-可執行與可連結格式的兩種視角](../html/c/ELF是什麼-可執行與可連結格式的兩種視角.html)

### ELF 1. ELF 三個字母各代表什麼？為什麼說「Executable」和「Linkable」對應兩種不同的讀者？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 1 題](../small-project/s1-01-hello-compile/quiz.md#1-elf-三個字母各代表什麼為什麼說executable和linkable對應兩種不同的讀者)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md)

```
```

---

## GNU：[GNU是什麼-Linux核心與GNU工具的分工](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html)

### GNU 3. 情境題：一支 shell 腳本在你的 Ubuntu 上用 `bash` 測試完全正常，放到 OpenWrt 上卻出錯。從本篇的角度，最可能的原因是什麼？寫腳本時該怎麼預防？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-情境題一支-shell-腳本在你的-ubuntu-上用-bash-測試完全正常放到-openwrt-上卻出錯從本篇的角度最可能的原因是什麼寫腳本時該怎麼預防)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md)

```
```
