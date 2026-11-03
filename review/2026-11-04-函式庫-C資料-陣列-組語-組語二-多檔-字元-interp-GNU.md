# 2026-11-04 複習：函式庫、C資料、陣列、組語、組語二、多檔、字元、interp、GNU

> 依[複習排程](../learning/複習排程.md)出今天到期的全部題目（每天出完所有到期題，2026-10-23 起的規則），共 **15 題**（39 個作答框），沒有過期的。
> 函式庫 3、12、5 是 11-03 大致對／不完整、留在盒子 1 的題目（不對的小題 11-03 已在自我練習框過關，今天再考一次）；函式庫 6、9 是 11-01 答對升到盒子 2；C資料 5、陣列 5、組語 9、組語二 3、多檔 2、5、6 是 10-28 答對升到盒子 3；字元 1、interp 1、GNU 3 是 10-21 答對升到盒子 4。
> 題目依筆記分節、節內先放盒子小的再照題號；每題照原題複製，用到的程式碼與實驗步驟都附在題目裡，「過去的作答」連到之前每一次作答（含批改與參考答案）。
> 目前在第 8 課的作業階段，到期題裡沒有跟第 8 課直接相關的，所以照盒子排。
>
> **怎麼寫**：闔上筆記，每題寫 1～2 句關鍵句就好。可以分幾次寫，寫完一部分就可以先說「幫我批改」。複習不重答，批改會直接附參考答案。
> 已拆小題的題目，每個小題一個作答框；以大題為單位排程，小題全對才算對。

## 今天的題目

排序：盒子小的在前；同一盒子照上次作答日期、再照筆記與題號。

| # | 題目 | 盒子 | 原本該考的日期 |
|---|---|---|---|
| 1 | 函式庫 3（3 小題） | 1 | 2026-11-04 |
| 2 | 函式庫 5（3 小題） | 1 | 2026-11-04 |
| 3 | 函式庫 12（4 小題） | 1 | 2026-11-04 |
| 4 | 函式庫 6（5 小題） | 2 | 2026-11-04 |
| 5 | 函式庫 9（3 小題） | 2 | 2026-11-04 |
| 6 | C資料 5 | 3 | 2026-11-04 |
| 7 | 陣列 5（3 小題） | 3 | 2026-11-04 |
| 8 | 組語 9（2 小題） | 3 | 2026-11-04 |
| 9 | 組語二 3（4 小題） | 3 | 2026-11-04 |
| 10 | 多檔 2（2 小題） | 3 | 2026-11-04 |
| 11 | 多檔 5（3 小題） | 3 | 2026-11-04 |
| 12 | 多檔 6（3 小題） | 3 | 2026-11-04 |
| 13 | 字元 1 | 4 | 2026-11-04 |
| 14 | interp 1 | 4 | 2026-11-04 |
| 15 | GNU 3 | 4 | 2026-11-04 |

---

## 函式庫：[靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html)

### 函式庫 3. `-fPIC`：哪個變數出問題、為什麼要 GOT

> 盒子 1｜過去的作答：[s1-07 quiz.md 第 3 題](../small-project/s1-07-libraries/quiz.md#3--fpic哪個變數出問題為什麼要-got)（10-29 第一次作答）、[11-01 複習](./2026-11-01-函式庫-組語.md#函式庫-3--fpic哪個變數出問題為什麼要-got)、[11-02 複習](./2026-11-02-函式庫-interp-C資料-字元-陣列-組語-組語二-多檔.md#函式庫-3--fpic哪個變數出問題為什麼要-got)、[11-03 複習](./2026-11-03-函式庫-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#函式庫-3--fpic哪個變數出問題為什麼要-got)

下面的 `stats.c` 不加 `-fPIC` 編成 `stats.o`，再用 `gcc -shared stats.o -o libstats.so`。會成功嗎？錯誤訊息會提到哪個變數？為什麼另一個變數不會有問題？為什麼 `.so` 存取全域變數一定要經過 GOT，而不能像執行檔那樣用 `%rip` 加固定距離直接讀（講出兩個理由）？

```c
/* stats.c */
int hits = 0;
static int misses = 0;

void hit(void)  { hits = hits + 1; }
void miss(void) { misses = misses + 1; }
```
編譯：`gcc -Wall -c stats.c -o stats.o && gcc -shared stats.o -o libstats.so`

#### 函式庫 3-1. 會成功嗎？錯誤訊息會提到哪個變數？

```
```

#### 函式庫 3-2. 為什麼另一個變數不會有問題？

```
```

#### 函式庫 3-3. 為什麼 `.so` 存取全域變數一定要經過 GOT，而不能像執行檔那樣用 `%rip` 加固定距離直接讀（講出兩個理由）？

```
```


---

### 函式庫 5. SONAME 與函式庫更新

> 盒子 1｜過去的作答：[s1-07 quiz.md 第 5 題](../small-project/s1-07-libraries/quiz.md#5-soname-與函式庫更新)（10-29 第一次作答）、[11-03 複習](./2026-11-03-函式庫-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#函式庫-5-soname-與函式庫更新)

你維護 `libcalc`，SONAME 是 `libcalc.so.1`。系統上有 `libcalc.so -> libcalc.so.1`、`libcalc.so.1 -> libcalc.so.1.0.0` 兩個符號連結。(1) 一支程式動態連結了 `libcalc`，它的 `NEEDED` 寫的是哪個名字？為什麼刪掉 `libcalc.so` 它照樣能跑，刪掉 `libcalc.so.1` 就不行？(2) 你修了一個 bug（介面不變），做成 `libcalc.so.1.0.1`。已經動態連結的程式要怎樣才能用到修正？另一支用 `libcalc.a` 靜態連結的程式呢？(3) 下一版要把 `calc_add` 改成三個參數（不相容），SONAME 該怎麼辦？為什麼這樣舊程式不會壞？

#### 函式庫 5-1. 一支程式動態連結了 `libcalc`，它的 `NEEDED` 寫的是哪個名字？為什麼刪掉 `libcalc.so` 它照樣能跑，刪掉 `libcalc.so.1` 就不行？

```
```

#### 函式庫 5-2. 你修了一個 bug（介面不變），做成 `libcalc.so.1.0.1`。已經動態連結的程式要怎樣才能用到修正？另一支用 `libcalc.a` 靜態連結的程式呢？

```
```

#### 函式庫 5-3. 下一版要把 `calc_add` 改成三個參數（不相容），SONAME 該怎麼辦？為什麼這樣舊程式不會壞？

```
```


---

### 函式庫 12. `.h` 與 `-l`：各在哪個階段用到（10-26 追問新增）

> 盒子 1｜過去的作答：[s1-07 quiz.md 第 12 題](../small-project/s1-07-libraries/quiz.md#12-h-與--l各在哪個階段用到10-26-追問新增)（10-29 第一次作答）、[11-01 複習](./2026-11-01-函式庫-組語.md#函式庫-12-h-與--l各在哪個階段用到10-26-追問新增)、[11-02 複習](./2026-11-02-函式庫-interp-C資料-字元-陣列-組語-組語二-多檔.md#函式庫-12-h-與--l各在哪個階段用到10-26-追問新增)、[11-03 複習](./2026-11-03-函式庫-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#函式庫-12-h-與--l各在哪個階段用到10-26-追問新增)

`libtemp.so` 的宣告放在 `include/temp.h`（裡面還定義了巨集 `FREEZING_C`），做好的 `libtemp.so` 放在 `lib/`。(1) `gcc -Wall main.c -Llib -ltemp -o app` 會失敗在哪個階段（前處理、編譯、連結、執行）？錯誤大概長怎樣、要補什麼？(2) `gcc -Wall -Iinclude main.c -Llib -o app` 呢？(3) 同事說「`FREEZING_C` 的值存在 `libtemp.so` 裡，執行時才讀進來」，對嗎？為什麼？(4) 編好的 `app`（連結時加了 `-Wl,-rpath,'$ORIGIN/lib'`）要拿給別人執行，`include/temp.h` 要一起給嗎？`lib/libtemp.so` 呢？

```c
/* include/temp.h */
#ifndef TEMP_H
#define TEMP_H

#define FREEZING_C 0.0

double c_to_f(double c);

#endif
```
```c
/* temp.c */
#include "temp.h"

double c_to_f(double c)
{
    return c * 9.0 / 5.0 + 32.0;
}
```
```c
/* main.c */
#include <stdio.h>
#include "temp.h"

int main(void)
{
    printf("%.1f\n", c_to_f(FREEZING_C));
    return 0;
}
```
做 `.so`：`gcc -Wall -fPIC -Iinclude -c temp.c -o temp.pic.o && gcc -shared temp.pic.o -o lib/libtemp.so`（檔案：[`include/temp.h`](../small-project/s1-07-libraries/examples/headers/temp/include/temp.h)、[`temp.c`](../small-project/s1-07-libraries/examples/headers/temp/temp.c)、[`main.c`](../small-project/s1-07-libraries/examples/headers/temp/main.c)）。

#### 函式庫 12-1. `gcc -Wall main.c -Llib -ltemp -o app` 會失敗在哪個階段（前處理、編譯、連結、執行）？錯誤大概長怎樣、要補什麼？

```
```

#### 函式庫 12-2. `gcc -Wall -Iinclude main.c -Llib -o app` 呢？

```
```

#### 函式庫 12-3. 同事說「`FREEZING_C` 的值存在 `libtemp.so` 裡，執行時才讀進來」，對嗎？為什麼？

```
```

#### 函式庫 12-4. 編好的 `app`（連結時加了 `-Wl,-rpath,'$ORIGIN/lib'`）要拿給別人執行，`include/temp.h` 要一起給嗎？`lib/libtemp.so` 呢？

```
```


---

### 函式庫 6. GOT：`D(%rip)` 的距離、GOT 那一格與變數本身（10-25 追問新增）

> 盒子 2｜過去的作答：[s1-07 quiz.md 第 6 題](../small-project/s1-07-libraries/quiz.md#6-gotdrip-的距離got-那一格與變數本身10-25-追問新增)（10-29 第一次作答）、[11-01 複習](./2026-11-01-函式庫-組語.md#函式庫-6-gotdrip-的距離got-那一格與變數本身10-25-追問新增)

看下面 `libtally.so` 的反組譯（`total = total + x;` 那一段）回答：(1) `111a` 那一行的 `0x2eaf` 是從哪裡到哪裡的距離？用這段的數字算出右邊註解的 `3fd0`。`112b` 那一行的 `D` 是 `0x2e9e`，跟 `0x2eaf` 不一樣，為什麼算出來還是 `3fd0`？(2) 註解 `<total-0x48>` 是說 `total` 放在 `0x3fd0` 嗎？`0x3fd0` 是什麼、`total` 本身在哪？(3) 執行完 `111a` 那一行，`%rax` 裡裝的是什麼？`1121` 讀到的又是什麼？為什麼用 `objdump -s -j .got libtally.so` 看，`0x3fd0` 那格在檔案裡是 0？

```c
/* tally.c */
int seen = 0;
long total = 0;

void add_sample(long x)
{
    seen = seen + 1;
    total = total + x;
}
```
編譯與觀察：`gcc -Wall -fPIC -c tally.c -o tally.pic.o && gcc -shared tally.pic.o -o libtally.so`，再 `objdump -d --no-show-raw-insn libtally.so` 與 `readelf -r libtally.so`，`total = total + x;` 那一段是：
```asm
    111a:	mov    0x2eaf(%rip),%rax        # 3fd0 <total-0x48>   ← 從 %rip 加 0x2eaf 的地方讀 8 byte 放進 %rax
    1121:	mov    (%rax),%rdx                                  ← 照 %rax 裡的位址讀 8 byte 放進 %rdx
    1124:	mov    -0x8(%rbp),%rax                              ← 從堆疊取出參數 x 放進 %rax
    1128:	add    %rax,%rdx                                    ← %rdx = %rdx + x
    112b:	mov    0x2e9e(%rip),%rax        # 3fd0 <total-0x48>   ← 同第一行，D 不同
    1132:	mov    %rdx,(%rax)                                  ← 把 %rdx 寫到 %rax 裡的位址
```
```text
000000003fd0  000700000006 R_X86_64_GLOB_DAT 0000000000004018 total + 0
000000003fd8  000600000006 R_X86_64_GLOB_DAT 0000000000004010 seen + 0
```
（檔案：[`tally.c`](../small-project/s1-07-libraries/examples/pic/tally.c)）

#### 函式庫 6-1. `111a` 那一行的 `0x2eaf` 是從哪裡到哪裡的距離？用這段的數字算出右邊註解的 `3fd0`。

```
```

#### 函式庫 6-2. `112b` 那一行的 `D` 是 `0x2e9e`，跟 `0x2eaf` 不一樣，為什麼算出來還是 `3fd0`？

```
```

#### 函式庫 6-3. 註解 `<total-0x48>` 是說 `total` 放在 `0x3fd0` 嗎？`0x3fd0` 是什麼、`total` 本身在哪？

```
```

#### 函式庫 6-4. 執行完 `111a` 那一行，`%rax` 裡裝的是什麼？`1121` 讀到的又是什麼？

```
```

#### 函式庫 6-5. 為什麼用 `objdump -s -j .got libtally.so` 看，`0x3fd0` 那格在檔案裡是 0？

```
```


---

### 函式庫 9. 兩支程式共用一個 `.so`：哪些是同一份、哪些各一份（10-26 追問新增）

> 盒子 2｜過去的作答：[s1-07 quiz.md 第 9 題](../small-project/s1-07-libraries/quiz.md#9-兩支程式共用一個-so哪些是同一份哪些各一份10-26-追問新增)（10-29 第一次作答）、[11-01 複習](./2026-11-01-函式庫-組語.md#函式庫-9-兩支程式共用一個-so哪些是同一份哪些各一份10-26-追問新增)

兩支程式 `x`、`y` 都連 `libhits.so`，同時在跑（各自停在 `sleep(5)`）。(1) `x` 印出的 `&hit` 和 `y` 印出的不一樣，這代表記憶體裡有兩份 `hit` 的程式碼嗎？`libhits.so` 的哪些部分兩個行程共用同一份實體記憶體、哪些是各自一份？(2) `y` 印出的 `hit()` 回傳值是多少？為什麼不是 `x`、`y` 加起來的 7？(3) 假設沒有 GOT，改成讓 `ld.so` 載入時把位址直接寫進 `.text`，用「寫入時複製」說明 `.text` 還能不能共用、為什麼。

```c
/* hits.c */
int hits = 0;

int hit(void)
{
    hits = hits + 1;
    return hits;
}
```
```c
/* x.c */
#include <stdio.h>
#include <unistd.h>

int hit(void);

int main(void)
{
    int r = 0;
    for (int i = 0; i < 2; i++)               /* x 呼叫 2 次 */
        r = hit();
    printf("x: &hit=%p  hit() 回傳 %d\n", (void *)hit, r);
    fflush(stdout);
    sleep(5);
    return 0;
}
```
```c
/* y.c */
#include <stdio.h>
#include <unistd.h>

int hit(void);

int main(void)
{
    int r = 0;
    for (int i = 0; i < 5; i++)               /* y 呼叫 5 次 */
        r = hit();
    printf("y: &hit=%p  hit() 回傳 %d\n", (void *)hit, r);
    fflush(stdout);
    sleep(5);
    return 0;
}
```
編譯：`gcc -Wall -fPIC -c hits.c -o hits.pic.o && gcc -shared hits.pic.o -o libhits.so && gcc -Wall x.c -L. -lhits -Wl,-rpath,'$ORIGIN' -o x && gcc -Wall y.c -L. -lhits -Wl,-rpath,'$ORIGIN' -o y`，執行：`./x & ./y & wait`（檔案：[`hits.c`](../small-project/s1-07-libraries/examples/share/hits.c)、[`x.c`](../small-project/s1-07-libraries/examples/share/x.c)、[`y.c`](../small-project/s1-07-libraries/examples/share/y.c)）。

#### 函式庫 9-1. `x` 印出的 `&hit` 和 `y` 印出的不一樣，這代表記憶體裡有兩份 `hit` 的程式碼嗎？`libhits.so` 的哪些部分兩個行程共用同一份實體記憶體、哪些是各自一份？

```
```

#### 函式庫 9-2. `y` 印出的 `hit()` 回傳值是多少？為什麼不是 `x`、`y` 加起來的 7？

```
```

#### 函式庫 9-3. 假設沒有 GOT，改成讓 `ld.so` 載入時把位址直接寫進 `.text`，用「寫入時複製」說明 `.text` 還能不能共用、為什麼。

```
```


---


## C資料：[C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html)

### C資料 5. 情境題：你從網路封包裡讀到 4 個 byte，依序是 `00 00 01 00`。協定規定數字用大端序，所以它代表 256。如果直接用 `int *` 把這 4 個 byte 讀進本機（x86-64），會得到多少？為什麼？

> 盒子 3｜過去的作答：[s1-02 quiz.md 第 5 題](../small-project/s1-02-data-repr/quiz.md#5-情境題你從網路封包裡讀到-4-個-byte依序是-00-00-01-00協定規定數字用大端序所以它代表-256如果直接用-int--把這-4-個-byte-讀進本機x86-64會得到多少為什麼)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-25 複習](./2026-10-25-C資料-陣列-組語-組語二-多檔.md)、[10-28 複習](./2026-10-28-ELF-C資料-陣列-組語-組語二-多檔.md#c資料-5-情境題你從網路封包裡讀到-4-個-byte依序是-00-00-01-00協定規定數字用大端序所以它代表-256如果直接用-int--把這-4-個-byte-讀進本機x86-64會得到多少為什麼)

```
```


---


## 陣列：[陣列、字串與指標運算-C不檢查邊界與用gdb找segfault](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html)

### 陣列 5. 情境題：當掉的那一行

#### 陣列 5-1. 同事的程式偶爾 segfault，gdb 的 `bt` 顯示當在 `parse()` 裡 `x = buf[k];` 這一行，`k` 的值是 523871，而 `buf` 只有 256 個元素。你接下來要怎麼查出真正的錯誤在哪裡？

> 盒子 3｜過去的作答：[s1-03 quiz.md 5-1](../small-project/s1-03-arrays-pointers/quiz.md#5-1-同事的程式偶爾-segfaultgdb-的-bt-顯示當在-parse-裡-x--bufk-這一行k-的值是-523871而-buf-只有-256-個元素你接下來要怎麼查出真正的錯誤在哪裡)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-25 複習](./2026-10-25-C資料-陣列-組語-組語二-多檔.md)、[10-28 複習](./2026-10-28-ELF-C資料-陣列-組語-組語二-多檔.md#陣列-5-情境題當掉的那一行)

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


---


## 組語：[x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)

### 組語 9. 符號擴展補的是什麼

> 盒子 3｜過去的作答：[s1-04 quiz.md 第 9 題](../small-project/s1-04-asm-basics/quiz.md#9-符號擴展補的是什麼2026-10-06-課後提問新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-25 複習](./2026-10-25-C資料-陣列-組語-組語二-多檔.md)、[10-28 複習](./2026-10-28-ELF-C資料-陣列-組語-組語二-多檔.md#組語-9-符號擴展補的是什麼)

#### 組語 9-1. `movsbl` 把 1 byte 擴展成 4 byte 時，新補的高 3 byte 是照哪一個位元補的？

```
```

#### 組語 9-2. 為什麼這樣補，`signed char` 的 −1 變成 `int` 之後才會還是 −1（用二補數的權重解釋）？

```
```


---


## 組語二：[x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html)

### 組語二 3. 8 個參數

> 盒子 3｜過去的作答：[s1-05 quiz.md 第 3 題](../small-project/s1-05-asm-calls/quiz.md#3-8-個參數)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-25 複習](./2026-10-25-C資料-陣列-組語-組語二-多檔.md)、[10-28 複習](./2026-10-28-ELF-C資料-陣列-組語-組語二-多檔.md#組語二-3-8-個參數)

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


---


## 多檔：[多檔案的C程式-標頭檔、static與extern、連結錯誤與make](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html)

### 多檔 2. 定義的 `.c` 也要引入自己的標頭檔

> 盒子 3｜過去的作答：[s1-06 quiz.md 第 2 題](../small-project/s1-06-multi-file/quiz.md#2-定義的-c-也要引入自己的標頭檔)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-25 複習](./2026-10-25-C資料-陣列-組語-組語二-多檔.md)、[10-28 複習](./2026-10-28-ELF-C資料-陣列-組語-組語二-多檔.md#多檔-2-定義的-c-也要引入自己的標頭檔)

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


---

### 多檔 5. 情境題：改了標頭檔，`make` 說不用重編

> 盒子 3｜過去的作答：[s1-06 quiz.md 第 5 題](../small-project/s1-06-multi-file/quiz.md#5-情境題改了標頭檔make-說不用重編)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-25 複習](./2026-10-25-C資料-陣列-組語-組語二-多檔.md)、[10-28 複習](./2026-10-28-ELF-C資料-陣列-組語-組語二-多檔.md#多檔-5-情境題改了標頭檔make-說不用重編)

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


---

### 多檔 6. 符號是什麼：`t.o` 的符號表裡有誰

> 盒子 3｜過去的作答：[s1-06 quiz.md 第 6 題](../small-project/s1-06-multi-file/quiz.md#6-符號是什麼to-的符號表裡有誰)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-25 複習](./2026-10-25-C資料-陣列-組語-組語二-多檔.md)、[10-28 複習](./2026-10-28-ELF-C資料-陣列-組語-組語二-多檔.md#多檔-6-符號是什麼to-的符號表裡有誰)

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


---


## 字元：[字元是什麼-char就是1byte的整數與ASCII和UTF-8](../html/c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.html)

### 字元 1. `char c = 'A';` 和 `char c = 65;` 存進記憶體的東西一樣嗎？為什麼 `printf` 用 `%c` 印出 `A`、用 `%d` 印出 65？

> 盒子 4｜過去的作答：[s1-02 quiz.md 字元第 1 題](../small-project/s1-02-data-repr/quiz.md#1-char-c--a-和-char-c--65-存進記憶體的東西一樣嗎為什麼-printf-用-c-印出-a用-d-印出-65)、[10-05 複習](./2026-10-05-gcc-陣列-GNU-C資料-字元.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md#字元-1-char-c--a-和-char-c--65-存進記憶體的東西一樣嗎為什麼-printf-用-c-印出-a用-d-印出-65)

```
```


---


## interp：[ELF的interpreter是什麼-動態載入器與井字號驚嘆號](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html)

### interp 1. 動態連結的執行檔裡，interpreter 的路徑寫在哪裡？`execve` 之後，核心先執行的是你的程式還是它？為什麼要這樣安排？

> 盒子 4｜過去的作答：[s1-01 quiz.md 第 1 題](../small-project/s1-01-hello-compile/quiz.md#1-動態連結的執行檔裡interpreter-的路徑寫在哪裡execve-之後核心先執行的是你的程式還是它為什麼要這樣安排)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md#interp-1-動態連結的執行檔裡interpreter-的路徑寫在哪裡execve-之後核心先執行的是你的程式還是它為什麼要這樣安排)

```
```


---


## GNU：[GNU是什麼-Linux核心與GNU工具的分工](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html)

### GNU 3. 情境題：一支 shell 腳本在你的 Ubuntu 上用 `bash` 測試完全正常，放到 OpenWrt 上卻出錯。從本篇的角度，最可能的原因是什麼？寫腳本時該怎麼預防？

> 盒子 4｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-情境題一支-shell-腳本在你的-ubuntu-上用-bash-測試完全正常放到-openwrt-上卻出錯從本篇的角度最可能的原因是什麼寫腳本時該怎麼預防)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md#gnu-3-情境題一支-shell-腳本在你的-ubuntu-上用-bash-測試完全正常放到-openwrt-上卻出錯從本篇的角度最可能的原因是什麼寫腳本時該怎麼預防)

```
```


---
