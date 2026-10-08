# 2026-10-08 複習：字元、C資料、objdump、hello、ELF、陣列

> 依 [複習排程](../learning/複習排程.md) 選出今天到期的 10 題。今天不上新課（s1-04 作業剛批改、有 9 題待重答，自我測驗還沒交），
> 所以不放「跟新課相關」的題目，全部依**過期最久 → 盒子小 → 上次作答較早**挑選：
> 10-05 到期的 11 題（全部盒子 1、上次作答都是 10-04）依排程表順序取前 10 題；**陣列 5** 沒選到，順延到明天。
> 今天到期的共 37 題（10-05 到期 11、10-06 到期 9、10-07 到期 10、10-08 到期 7），沒選到的 27 題順延，日期不動。
>
> **怎麼寫**：闔上筆記，每題寫 1～2 句關鍵句就好。寫完說「幫我批改」。複習不重答，批改會直接附參考答案。
> 陣列 1、陣列 4 已拆成小題，每個小題一個作答框。

## 今天的題目

| # | 題目 | 盒子 | 原本該考的日期 | 為什麼選 |
|---|---|---|---|---|
| 1 | 字元 5 | 1 | 2026-10-05 | 過期最久，排程表順序 |
| 2 | C資料 9 | 1 | 2026-10-05 | 同上 |
| 3 | objdump 3 | 1 | 2026-10-05 | 同上 |
| 4 | objdump 4 | 1 | 2026-10-05 | 同上 |
| 5 | hello 5 | 1 | 2026-10-05 | 同上 |
| 6 | objdump 1 | 1 | 2026-10-05 | 同上 |
| 7 | ELF 3 | 1 | 2026-10-05 | 同上 |
| 8 | ELF 4 | 1 | 2026-10-05 | 同上 |
| 9 | 陣列 1（3 小題） | 1 | 2026-10-05 | 同上 |
| 10 | 陣列 4（2 小題） | 1 | 2026-10-05 | 同上 |

---

## 字元：[字元是什麼-char就是1byte的整數與ASCII和UTF-8](../c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.md)

### 字元 5. 有人說「『字』是 3 個字元」，另一個人說「是 1 個字元」，誰對？`strlen("字")` 和 `mbstowcs(NULL, "字", 0)` 各會得到多少？如果程式忘了呼叫 `setlocale`，後者會得到什麼，那個數字是怎麼來的？

> 盒子 1｜過去的作答：[s1-02 quiz.md 第 5 題](../small-project/s1-02-data-repr/quiz.md#5-有人說字是-3-個字元另一個人說是-1-個字元誰對strlen字-和-mbstowcsnull-字-0-各會得到多少如果程式忘了呼叫-setlocale後者會得到什麼那個數字是怎麼來的)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)

```

```

---

## C資料：[C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號](../c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md)

### C資料 9. 情境題：有人寫了 `if (len - used > 0)` 判斷「緩衝區還有沒有剩餘空間」，`len`、`used` 都是 `size_t`，用 `-Wall -Wextra` 編譯沒有任何警告。這個判斷在什麼情況下會出錯？為什麼 gcc 對 `len - used < 0` 會警告、對 `> 0` 卻不會？另外，為什麼 `int sum = x + y; if (sum < x)` 這種溢位檢查在 `-O2` 下可能完全失效？

> 盒子 1｜過去的作答：[s1-02 quiz.md 第 9 題](../small-project/s1-02-data-repr/quiz.md#9-情境題有人寫了-if-len---used--0-判斷緩衝區還有沒有剩餘空間lenused-都是-size_t用--wall--wextra-編譯沒有任何警告這個判斷在什麼情況下會出錯為什麼-gcc-對-len---used--0-會警告對--0-卻不會另外為什麼-int-sum--x--y-if-sum--x-這種溢位檢查在--o2-下可能完全失效)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)

```

```

---

## objdump：[objdump能不能代替ldd找依賴庫-需求單與實際載入](../c/objdump能不能代替ldd找依賴庫-需求單與實際載入.md)

### objdump 3. 一支程式連結時用了 `-L./lib -lfoo`，編譯成功；執行時卻出現 `error while loading shared libraries: libfoo.so`。用「連結時」與「執行時」分別是誰在找檔案，解釋為什麼連結找得到、執行卻找不到。這時 `objdump -p` 與 `ldd` 各會顯示什麼？

> 盒子 1｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-一支程式連結時用了--llib--lfoo編譯成功執行時卻出現-error-while-loading-shared-libraries-libfooso用連結時與執行時分別是誰在找檔案解釋為什麼連結找得到執行卻找不到這時-objdump--p-與-ldd-各會顯示什麼)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)

```

```

### objdump 4. 情境題：同事傳來一支 ARM 開發板用的執行檔，問你「它需要哪些函式庫」。你在自己的 x86-64 電腦上該用哪個工具？用 `ldd` 會發生什麼事，為什麼？

> 盒子 1｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-情境題同事傳來一支-arm-開發板用的執行檔問你它需要哪些函式庫你在自己的-x86-64-電腦上該用哪個工具用-ldd-會發生什麼事為什麼)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)

```

```

### objdump 1. `objdump -p /usr/bin/curl` 只列出 3 個 `NEEDED`，`ldd` 卻列出 33 行。多出來的是哪幾類東西？為什麼 `objdump` 看不到？

> 盒子 1｜過去的作答：[s1-01 quiz.md 第 1 題](../small-project/s1-01-hello-compile/quiz.md#1-objdump--p-usrbincurl-只列出-3-個-neededldd-卻列出-33-行多出來的是哪幾類東西為什麼-objdump-看不到)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)

```

```

---

## hello：[hello.c怎麼變成能跑的程式-編譯的四個階段](../c/hello.c怎麼變成能跑的程式-編譯的四個階段.md)

### hello 5. 情境題：你把一支動態連結的程式複製到另一台機器，執行時出現找不到 `libc.so.6` 某個版本符號的錯誤。從本課的哪個階段可以解釋這件事？如果改用 `-static` 編譯，這個問題還會發生嗎？

> 盒子 1｜過去的作答：[s1-01 quiz.md 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-情境題你把一支動態連結的程式複製到另一台機器執行時出現找不到-libcso6-某個版本符號的錯誤從本課的哪個階段可以解釋這件事如果改用--static-編譯這個問題還會發生嗎)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)

```

```

---

## ELF：[ELF是什麼-可執行與可連結格式的兩種視角](../c/ELF是什麼-可執行與可連結格式的兩種視角.md)

### ELF 3. 本機一般 `gcc` 編出的執行檔，`Type` 為什麼是 DYN，跟 `libc.so.6` 同一類，而靜態版是 EXEC？

> 盒子 1｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-本機一般-gcc-編出的執行檔type-為什麼是-dyn跟-libcso6-同一類而靜態版是-exec)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)

```

```

### ELF 4. 區段（section）和載入段（segment）差在哪？為什麼核心載入時要把很多區段打包成少數幾個載入段？

> 盒子 1｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-區段section和載入段segment差在哪為什麼核心載入時要把很多區段打包成少數幾個載入段)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)

```

```

---

## 陣列：[陣列、字串與指標運算-C不檢查邊界與用gdb找segfault](../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md)

### 陣列 1. 指標的型別與指標運算

#### 陣列 1-1. `int *p` 和 `unsigned char *c` 存著同一個位址，為什麼 `p + 1` 和 `c + 1` 不一樣？

> 盒子 1｜過去的作答：[s1-03 quiz.md 1-1](../small-project/s1-03-arrays-pointers/quiz.md#1-1-int-p-和-unsigned-char-c-存著同一個位址為什麼-p--1-和-c--1-不一樣)

```

```

#### 陣列 1-2. `p[3]` 在 C 標準裡是怎麼定義的？

> 過去的作答：[s1-03 quiz.md 1-2](../small-project/s1-03-arrays-pointers/quiz.md#1-2-p3-在-c-標準裡是怎麼定義的)

```

```

#### 陣列 1-3. 為什麼 `3[p]` 也能編譯，而且值跟 `p[3]` 一樣？

> 過去的作答：[s1-03 quiz.md 1-3](../small-project/s1-03-arrays-pointers/quiz.md#1-3-為什麼-3p-也能編譯而且值跟-p3-一樣)

```

```

### 陣列 4. 越界的三種下場

#### 陣列 4-1. 越界存取有三種下場：安靜出錯、`stack smashing detected`、segfault。分別是什麼決定會落到哪一種？

> 盒子 1｜過去的作答：[s1-03 quiz.md 4-1](../small-project/s1-03-arrays-pointers/quiz.md#4-1-越界存取有三種下場安靜出錯stack-smashing-detectedsegfault分別是什麼決定會落到哪一種)

```

```

#### 陣列 4-2. 為什麼「程式沒當掉」不能證明沒有越界？

> 過去的作答：[s1-03 quiz.md 4-2](../small-project/s1-03-arrays-pointers/quiz.md#4-2-為什麼程式沒當掉不能證明沒有越界)

```

```
