# s1-06 自我測驗作答

> 第 ① 站第 6 課的自我測驗寫在這裡。題目從筆記複製過來；批改完後，最新版的參考答案也寫在筆記每題下面。
>
> **怎麼寫**：直接在每題下面的空白區塊裡作答，**闔上筆記、用自己的話**（把 `YYYY-MM-DD` 換成作答日期）。寫完跟 Claude 說「幫我批改」。
> 批改與參考答案會寫在你的答案下方，**不重答**：答錯或不完整也直接附參考答案，沒弄懂的交給間隔複習再考。
>
> **複習**：間隔複習的作答寫在 repo 根目錄的 [`review/`](../../review/README.md)（每天一個檔），排程見 [複習排程](../../learning/複習排程.md)。本檔只放第一次作答。

## 目錄

| 測驗 | 對應筆記 | 狀態 |
|---|---|---|
| [第 6 課：多檔案的 C 程式](#已答第-6-課多檔案的-c-程式) | [多檔案的C程式-標頭檔、static與extern、連結錯誤與make](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html) | 已答、已批改（2026-10-22） |

---

## 已答：第 6 課：多檔案的 C 程式

> 題目來自 [多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html)。
> 每題拆成小題（`N-M`），每個小題各有一個作答框；題目用到的程式碼附在每個大題的開頭。批改寫在該小題的作答框正下方。

### 批改結果（2026-10-22）

> 18 個小題：對 8、大致對 6、不完整 3、錯 1（2-2 於 10-22 改判為大致對）。不重答，每題直接附參考答案（核心關鍵＋詳細說明）。以大題計：第 1、2、3、5 題大致對，第 6 題不完整，第 4 題錯，6 題都進複習盒子 1（2026-10-23）。

| 題目 | 結果 | 錯在哪／缺什麼 |
|---|---|---|
| 1-1 | 對 | 抓到重點：看不到宣告 → 隱式宣告成回傳 `int` → 只拿 `%eax`，算出 `0x540be400` |
| 1-2 | 大致對 | 「名字相同就接起來」對；缺「符號表根本沒有型別（`Type` 只有 `FUNC`／`NOTYPE`）」；第二句「兩邊都要引入」不對焦——這題只要 `main.c` 引入就好 |
| 1-3 | 大致對 | 只拿低 32 位元、是結果不是原因，都對；擴展描述錯：補的是高 32 位元、照第 31 位元**符號擴展**，不是「用二補數補 0」（最高位元是 1 時會變負數） |
| 2-1 | 大致對 | 「不一致在編譯階段報錯」對；沒講出「宣告和定義在同一個編譯單元，編譯器才比得到」與「呼叫端因此跟定義一致」（跟作業 1-1 卡在同一處） |
| 2-2 | 大致對 | **改判**（2026-10-22，原判錯）：題目沒寫 `main.c` 有沒有引入 `mathx.h`，是出題不清楚；照你的讀法（`main.c` 沒引入）推理正確——隱式宣告的 `int` 剛好跟 `int square(int)` 一致，沒有階段會發現。缺：沒點出 `main.c` 編譯時仍有隱式宣告警告；出題原意（`main.c` 有引入）的答案見參考答案 |
| 3-1 | 對 | 抓到重點：外面 → `LOCAL`、只限本檔；裡面 → 固定記憶體、不放堆疊 |
| 3-2 | 大致對 | `LOCAL` 不參與配對對；漏了後半：拿掉 `static` → 兩個 `GLOBAL` 同名定義 → `multiple definition` |
| 3-3 | 對 | 抓到重點：`next_id` 的 `n` 在 `.bss`（初值 0），`plain_id` 的 `n` 在堆疊框 |
| 4-1 | 對 | 抓到重點：沒 `extern` 就是定義，兩個 `.c` 各一份（作業 2-1 的錯這次說對了）；術語叫暫定定義 |
| 4-2 | 對 | 抓到重點：guard 只防同一個 `.c` 重複引入，兩個 `.c` 分開編譯管不到 |
| 4-3 | 不完整 | 只改標頭檔加 `extern`，漏了「恰好一個 `.c` 寫 `long hits = 0;`」——照你的改法會 `undefined reference to 'hits'`（實測） |
| 4-4 | 錯 | 說成兩份各讀寫各的；實際是 common 符號被**合併成一份**（兩份的話會印 `hits = 0`，實際印 3） |
| 5-1 | 對 | 抓到重點：先決條件比目標新才執行命令 |
| 5-2 | 對 | 抓到重點：`make` 只看 `Makefile` 寫的先決條件，`config.h` 沒列 |
| 5-3 | 大致對 | `-MMD`＋`-include` 主線對；`-MP` 說錯（是替標頭檔加空規則，防刪掉／改名時 `make` 停下來），`-include` 的 `-` 沒說（跟作業 5-1 卡在同一處） |
| 6-1 | 不完整 | `limit` 錯成「檔案提供」：`extern` 只是宣告，是 `UND`（需要別人提供）；其他四個對 |
| 6-2 | 對 | 抓到重點：在堆疊上，位址不用連結器決定，不需要符號（作業 1-3 漏的這次答到） |
| 6-3 | 不完整 | 只答到配對（符號解析）；沒答「符號代表什麼」（名字＋區段、偏移、大小），漏重定位；`puts` 把連結器 `ld` 和執行時的 `ld.so` 混在一起 |

### 1. 忘了宣告：`square(100000)` 印出 1410065408

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

#### 1-1. 為什麼編譯器會產生錯的程式？

**第 1 次（2026-10-22）**
```
問題就是因爲main.c 沒有引入 mathx.c,應該說標頭檔,他沒有mathx的宣告,所以會默認把回傳值當成int
因此組語出現了cltq, 想把eax 轉成rax, 0x186a0平方後是0x2540be400 超過4byte,eax只會拿0x540be400,最後的出來的數字就是1410065408
```

> **批改**：**對**。抓到重點：編 `main.c` 時沒有 `square` 的宣告 → 照隱式宣告猜回傳 `int` → 只拿 `%eax`；`0x2540be400` 被截成 `0x540be400` ＝ 1410065408，數字也算對。
>
> 小提醒：開頭的「沒有引入 mathx.c」你自己改口成標頭檔是對的——被 `#include` 的是宣告（標頭檔），`.c` 不會被引入。
>
> **參考答案**【[第二節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#實驗忘了宣告square100000-印出-1410065408)〈實驗：忘了宣告〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 編譯器一次只看一個 `.c`，編 `main.c` 時看不到 `square` 的宣告和定義。
> 2. 只好照舊規則**隱式宣告**成回傳 `int`，照 `int` 產生程式碼：回傳值只取 `%eax`（低 32 位元）。
>
> **詳細說明**：`square` 本身照 `long` 算，`%rax` ＝ `0x2_540be400`（10000000000）；`main` 只拿 `%eax` ＝ `0x540be400` ＝ 1410065408。**被呼叫的那邊是對的，呼叫的那邊用錯的型別解讀結果**。編譯器是忠實照自己的猜測產生程式，錯在猜測本身。
> gcc 13 對隱式宣告只給 `-Wimplicit-function-declaration` 警告，所以編譯要開 `-Wall`、不能忽略警告（據 gcc 14 的變更說明新版預設改成錯誤，本機未實測）。

#### 1-2. 為什麼連結器沒有發現兩邊型別不一樣？

**第 1 次（2026-10-22）**
```
readelf -s main.o mathx.o                                                                                   ok | base py | at 14:50:32

File: main.o

Symbol table '.symtab' contains 7 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS main.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 .rodata
     4: 0000000000000000    67 FUNC    GLOBAL DEFAULT    1 main
     5: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND square
     6: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND printf

File: mathx.o

Symbol table '.symtab' contains 4 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS mathx.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000    22 FUNC    GLOBAL DEFAULT    1 square
兩邊都是global 而且符號名稱也相同,所以有連起來.但之所以兩邊型別判斷有問題 
是因爲兩邊都需要引入標頭檔,都沒有做的話就是各自依自己的定義,main用int, mathx用long
```

> **批改**：**大致對**。「兩邊都是 `GLOBAL`、名字相同，所以接起來」就是連結器的配對方式，主線對；缺了關鍵的另一半：**符號表裡根本沒有 C 的型別**，連結器想比也沒得比。你貼的 `readelf -s` 就看得到：`square` 在 `main.o` 是 `NOTYPE`、在 `mathx.o` 是 `FUNC`，`Type` 欄只分函式／變數，沒有 `long(long)`、`int()` 這種資訊。
>
> 另外第二句「兩邊都需要引入標頭檔」放在這題不對焦：這題的 bug 只要 **`main.c`** 引入 `mathx.h` 就會消失（編譯器看到 `long square(long)`，照 `long` 產生程式）；`mathx.c` 引入自己的標頭檔防的是第 2 題那種錯。
>
> **參考答案**【[第四節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#四符號表o-之間靠名字接起來)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 連結器只比對符號的**名字**：`main.o` 的 `UND square` 配上 `mathx.o` 的 `GLOBAL square`。
> 2. `.o` 的符號表裡沒有 C 的型別（`Type` 只有 `FUNC`／`OBJECT`／`NOTYPE` 這種），所以兩邊型別不一樣，連結器無從發現。
>
> **詳細說明**：整理成一句：編譯 `main.c` 時看不到 `mathx.c`、編譯 `mathx.c` 時看不到 `main.c`、連結時看不到型別——**沒有任何一個階段同時看得到兩邊的型別**。標頭檔的作用就是在編譯階段把同一份型別資訊帶到兩邊（第三節）。

**自我練習**（不批改）

```
連接器根本沒有C的型別,想比也沒得比
連接器只是靠名字是否相同 bind是否爲global來決定是否連接
```

#### 1-3. `main.o` 裡的 `cltq` 在這個錯誤裡扮演什麼角色？

**第 1 次（2026-10-22）**
```
mathx使用long檔返回值,也就是rax
main則是認爲返回值是int 4byte也就是eax
cltq就是要把eax轉成rax,也就是main從square得到的回傳值,他只看低32爲,然後高8爲就用二補數的方式補0
真正轉換錯誤就發生在cltq(但他不是原因,是原因造成的行爲)
```

> **批改**：**大致對**。「`main` 以為是 `int` 只拿 `%eax`、`square` 放的是完整的 `%rax`、`cltq` 是原因造成的行為而不是原因」都對，最後那句判斷很好。錯在擴展的描述：
>
> - 補的是**高 32 位元**（`%eax` 32 位元 → `%rax` 64 位元），不是「高 8 位」。
> - `cltq` 是**符號擴展**：高 32 位元全部複製第 31 位元（`%eax` 的最高位元），不是「用二補數的方式補 0」。這題剛好補 0，是因為 `0x540be400` 的最高位元是 0（`0x5` ＝ `0101`）；截下來的最高位元是 1 時會補成全 1，變成**負數**（見下方詳細說明的 `square(50000)`）。這是[第 4 課](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html) `movslq` 的觀念。
>
> **參考答案**【[第二節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#實驗忘了宣告square100000-印出-1410065408)〈實驗：忘了宣告〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `cltq` 把 `%eax` **符號擴展**成 `%rax`（＝ `movslq %eax,%rax`）：只保留低 32 位元，高 32 位元照第 31 位元補。
> 2. 所以 `square` 留在 `%rax` 高 32 位元的正確結果（`0x2`）被蓋掉，截斷就發生在這一條；它是「猜成 `int`」的結果，不是根因。
>
> **詳細說明**：為什麼會有這一條：`main` 以為拿到的是 `int`，要存進 `long r`，`int` 轉 `long` 要保留數值（含正負號），所以編譯器插一條符號擴展。如果 `main` 有正確的宣告（回傳 `long`），就不會有 `cltq`，直接 `mov %rax,-0x8(%rbp)`。
>
> 本機實測：把題目 `main.c` 的 `square(100000)` 改成 `square(50000)`，其餘不變、照題目的指令編譯執行，印出 `-1794967296`。50000² ＝ 2500000000 ＝ `0x9502f900`，還沒超過 32 位元，但最高位元是 1（`0x9` ＝ `1001`），`cltq` 補成全 1 → 2500000000 − 2³² ＝ −1794967296。

**自我練習**（不批改）

```
寫錯了 是高32位
```

### 2. 定義的 `.c` 也要引入自己的標頭檔

為什麼「定義函式的那個 `.c` 也要 `#include` 自己的標頭檔」？如果 `mathx.c` 沒有 `#include "mathx.h"`，有人把 `mathx.c` 的定義改成 `int square(int x)` 卻忘了改 `mathx.h`，會發生什麼事、在哪個階段（編譯、連結、執行）才發現？

```c
/* mathx.h */
#ifndef MATHX_H
#define MATHX_H

long square(long x);

#endif
```

#### 2-1. 為什麼「定義函式的那個 `.c` 也要 `#include` 自己的標頭檔」？

**第 1 次（2026-10-22）**
```
標頭檔是做宣告,如果不用標頭檔,也就是不用宣告,那之後你在改函式定義的時候,連接器並不會知道錯誤,連接器使用符號名稱連接起來的,他不知道類型,就會發生第一題的情況
第二小題正是告訴我們如果.h 與.c 宣告與定義的型別不一致,這個時候就會噴錯,我們可以在編譯階段就知道問題並且修改
```

> **批改**：**大致對**。抓到結果：`.h` 與 `.c` 型別不一致時，編譯階段就報錯。理由講得繞：「不用標頭檔就是不用宣告，連接器不會知道錯誤」把兩件事混在一起。關鍵是**為什麼引入了就抓得到**：編譯器一次只看一個 `.c`，`mathx.c` 引入 `mathx.h` 之後，**宣告和定義出現在同一個編譯單元**，編譯器才有機會比對，不一致就報 `conflicting types`。也該講出另一半：`main.c` 引入的是同一份 `mathx.h`，所以定義通過這個檢查，就保證呼叫端用的宣告跟定義一致。
>
> （跟作業 1-1 卡在同一處：作業寫成讓宣告和定義「連接起來」；這次方向對了，但還沒講出「同一個編譯單元、由編譯器比對」。）
>
> **參考答案**【[第三節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#定義的那個-c-為什麼也要-include-自己的標頭檔)〈定義的那個 `.c` 為什麼也要 `#include` 自己的標頭檔〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 引入後，`mathx.c` 這個編譯單元裡同時有宣告（貼進來的 `mathx.h`）和定義，編譯器會比對，不一致就在**編譯時**報 `conflicting types`。
> 2. 呼叫端引入的是同一份 `mathx.h`，所以定義通過檢查，就保證呼叫端用的宣告跟定義一致。
>
> **詳細說明**：第三節的 `bad.c` 實測（`#include "mathx.h"` 後把定義寫成 `int square(int x)`）：
>
> ```text
> $ gcc -c bad.c
> bad.c:3:5: error: conflicting types for ‘square’; have ‘int(int)’
> mathx.h:4:6: note: previous declaration of ‘square’ with type ‘long int(long int)’
> ```
>
> 沒引入的話會怎樣，就是 2-2。

**自我練習**（不批改）

```
有引入的話,編譯階段如果.h與.c的類別衝突可以直接看到,也讓所有引入.h的檔案統一類別
```

#### 2-2. 如果 `mathx.c` 沒有 `#include "mathx.h"`，有人把 `mathx.c` 的定義改成 `int square(int x)` 卻忘了改 `mathx.h`，會發生什麼事、在哪個階段（編譯、連結、執行）才發現？

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

**第 1 次（2026-10-22）**
```
很有可能什麼都不會發生, 因爲如果根據第一題的main他也沒有引入mathx.h,那main很有可能會認爲返回值是int,參數也是int,因爲預設的關係,結果剛好跟int square(int x)一致,成爲一個巧合
```

> **批改**：~~錯~~ → **大致對**（2026-10-22 改判）。原本判錯的理由是「題目的 `main.c` 有引入 `mathx.h`」，但題目沒有寫出 `main.c`，也沒說它有沒有引入——這是出題不清楚，你照第 1 題的 `main.c`（沒引入）來讀是合理的。照你的讀法實測：
>
> ```text
> $ gcc -Wall -c main.c        # 第 1 題的 main.c，沒有引入 mathx.h
> main.c:5:14: warning: implicit declaration of function ‘square’ [-Wimplicit-function-declaration]
> $ gcc -Wall -c mathx.c && gcc -o calc main.o mathx.o && ./calc
> 1410065408
> ```
>
> 隱式宣告猜的 `int` 剛好跟新的 `int square(int)` 一致，「型別不一致」確實沒有任何階段會發現，你說的「巧合」對。缺的部分：`main.c` 編譯時仍然有隱式宣告的警告（那是第 1 題的問題，沒有消失）；印出 1410065408 是 `int` 乘法本身溢位，跟型別不一致無關。
>
> 出題原意是 `main.c` **有**引入 `mathx.h`（題目已補上），那個情境的答案在下面——複習時會考這個版本。
>
> 原本的批改（保留）：題目的情境是 `main.c` **有** `#include "mathx.h"`（標頭檔就是給呼叫端用的；第 1 題才是沒引入），所以 `main.c` 照標頭檔的 `long square(long)` 呼叫，不是照隱式宣告的 `int`，不會「剛好一致」。也沒回答題目問的「在哪個階段才發現」。
>
> 實測（新實驗，原始碼見下方詳細說明）：編譯、連結都沒有任何錯誤或警告（`-Wall` 也沒有），執行印出 `1410065408`——**執行時才發現，而且只有數字夠大時才看得出來**（`square(3)` 會正常印 9）。
>
> **參考答案**【[第三節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#定義的那個-c-為什麼也要-include-自己的標頭檔)〈定義的那個 `.c` 為什麼也要 `#include` 自己的標頭檔〉最後一段】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 編譯不報錯：`mathx.c` 沒引入 `mathx.h`，編譯器看不到舊宣告、沒東西可比；`main.c` 照標頭檔的 `long square(long)` 編，也沒錯。
> 2. 連結不報錯：連結器只比名字 `square`。
> 3. **執行時**才出錯（結果錯誤，而且可能沒人發現）：呼叫端照 `long` 傳參數、讀 `%rax`，定義端照 `int` 只用 `%edi`、只算 32 位元——又回到第 1 題「連結成功、結果錯誤」。
>
> **詳細說明**：實驗 `examples/quiz-noinclude/`：`mathx.h`（檔案：[`mathx.h`](examples/quiz-noinclude/mathx.h)）就是題目給的標頭檔；`mathx.c`（檔案：[`mathx.c`](examples/quiz-noinclude/mathx.c)）改成 `int`、而且**沒有**引入標頭檔：
>
> ```c
> /* 自我測驗 2-2：定義改成 int、而且沒有 #include "mathx.h" */
> int square(int x)
> {
>     return x * x;
> }
> ```
>
> `main.c`（檔案：[`main.c`](examples/quiz-noinclude/main.c)）有引入標頭檔：
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
>
> ```text
> $ gcc -Wall -c main.c && gcc -Wall -c mathx.c && gcc -o calc main.o mathx.o && ./calc
> 1410065408
> $ objdump -d --no-show-raw-insn mathx.o
> 0000000000000000 <square>:
>    0:	endbr64
>    4:	push   %rbp
>    5:	mov    %rsp,%rbp
>    8:	mov    %edi,-0x4(%rbp)      # 只存參數的低 32 位元
>    b:	mov    -0x4(%rbp),%eax
>    e:	imul   %eax,%eax            # 32 位元乘法，100000² 溢位，%eax = 0x540be400
>   11:	pop    %rbp
>   12:	ret
> ```
>
> `main` 照 `long` 讀整個 `%rax`；`square` 寫的是 `%eax`，寫 32 位元暫存器會把高 32 位元清成 0（[第 4 課](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)），所以 `main` 拿到 1410065408。（`int` 乘法溢位在 C 裡是未定義行為，這只是本機 `-O0` 的結果。）
>
> 這就是定義的 `.c` 也要引入自己標頭檔的理由：引入了，這個錯在編譯 `mathx.c` 時就是 `conflicting types`（2-1），根本到不了執行。

### 3. `static` 的兩種意思

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

#### 3-1. `static` 寫在函式外面和寫在函式裡面，各改變了什麼？

**第 1 次（2026-10-22）**
```
static 在函式外面,意味着這個檔案只有這個.c可以使用,readelf -s 看到bind是local無法跟其他檔案連接
static 在函式裏面 說明這個變數(以本題code爲例) 他會存在固定的記憶體區段.bss, 而不是像一般的區域變數存在函式的堆棧中
```

> **批改**：**對**。兩個意思都答到：函式外 → `LOCAL`、只有這個 `.c` 能用；函式裡 → 放在固定的記憶體區段、不放堆疊。補一句會更完整：因為在固定位置，**整個程式只有一份、值留到下一次呼叫**（3-3 你也答到了）。放 `.bss` 是因為初值 0、非 0 會放 `.data`——你寫了「以本題 code 為例」，有注意到這點。
>
> **參考答案**【[第五節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#五static兩種完全不同的意思)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 函式外面：改的是名字的**可見範圍**——只在這個 `.c` 有效，符號變成 `LOCAL`，連結器不拿它跟別的檔案配對。
> 2. 函式裡面：改的是**儲存期間**——不放堆疊，放在固定記憶體（`.bss`／`.data`），整個程式執行期間只有一份，值留到下一次呼叫；名字仍然只在那個函式裡看得到。
>
> **詳細說明**：同一個關鍵字、兩種完全不同的意思，要看寫在哪裡。只給自己檔案用的函式與全域變數一律加 `static`（kernel 絕大部分函式都是）。

#### 3-2. (a) `a.c`、`b.c` 各有一個 `static int helper(void)`，為什麼能一起連結，拿掉 `static` 為什麼就 `multiple definition`？

**第 1 次（2026-10-22）**
```
因爲static 函式的bind是local,在ld連接的時候不會參與連接,所以不會噴multiple definition
```

> **批改**：**大致對**。前半對：`static` → `LOCAL`，連結器配對時不看 `LOCAL`，兩個 `helper` 互不干擾。題目的後半沒答：**拿掉 `static` 為什麼就 `multiple definition`**——兩個 `helper` 都變成 `GLOBAL` 的定義，連結器看到兩個同名的 `GLOBAL` 定義，不知道要用哪個。
>
> 用詞：`LOCAL` 符號不是「不參與連結」——它們照樣被放進執行檔、各有自己的位址（`nm ab` 看得到兩個 `t helper`），只是不拿來跟別的檔案**配對**。
>
> **參考答案**【[第五節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#檔案層級的-static讓名字變成-local)〈檔案層級的 `static`〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 有 `static`：兩個 `helper` 都是 `LOCAL`，連結器做符號解析時只用 `GLOBAL` 配對，所以同名也不衝突。
> 2. 拿掉 `static`：兩個 `.o` 各有一個 `GLOBAL` 的 `helper` 定義 → 同名的 `GLOBAL` 定義有兩個 → `multiple definition`。
>
> **詳細說明**：第五節實測（`a2.c`、`b2.c` 是拿掉 `static` 的版本）：
>
> ```text
> $ readelf -s a2.o | grep helper
>      3: 0000000000000000    15 FUNC    GLOBAL DEFAULT    1 helper
> $ gcc -o ab2 ab_main.o a2.o b2.o
> /usr/bin/ld: b2.o: in function `helper':
> b2.c:(.text+0x0): multiple definition of `helper'; a2.o:a2.c:(.text+0x0): first defined here
> ```
>
> 兩份內容不一樣（回傳 1 和 2）不是重點；就算一模一樣，有兩個同名的 `GLOBAL` 定義也是錯（第四節 `mathx2.c` 的實驗）。

#### 3-3. (b) `next_id()` 呼叫三次回傳 1、2、3，`plain_id()` 每次都是 1，兩個 `n` 各放在記憶體的哪裡？

**第 1 次（2026-10-22）**
```
next_id 是放在.bss 因爲他的初始值是0
plain_id 是放在函式的堆棧裏(所以每次重新呼叫都是0)
```

> **批改**：**對**。`next_id` 的 `n` 在 `.bss`（初值 0 的理由也對）、`plain_id` 的 `n` 在堆疊框裡、每次重新設定。小補：`plain_id` 每次回傳的是 **1**（設成 0 再加 1），你寫的「每次重新呼叫都是 0」指的是初值，意思對。
>
> **參考答案**【[第五節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#函式裡的-static不放堆疊值留到下一次)〈函式裡的 `static`〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `next_id` 的 `n`：固定記憶體 `.bss`（初值 0），只初始化一次、值留著，所以 1、2、3。
> 2. `plain_id` 的 `n`：堆疊框裡（`-0x4(%rbp)`），每次呼叫都重新設成 0，所以每次都是 1。
>
> **詳細說明**：組語看得出差別：`next_id` 用 `0x0(%rip)` 存取（固定位置，連結時填），函式裡沒有任何設成 0 的指令（初值是載入時準備好的）；`plain_id` 每次都 `movl $0x0,-0x4(%rbp)`。符號表裡有 `LOCAL` 的 `n.0`（`.bss`、4 byte），`plain_id` 的 `n` 沒有符號。

### 4. 標頭檔裡的 `long hits;`

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

#### 4-1. 為什麼？

**第 1 次（2026-10-22）**
```
因爲 hits_main.c 與 hits.c都引入了hits.h,假如沒有加extern, 這兩個檔案連接起來 相當於重複定義了hits兩次
```

> **批改**：**對**。抓到重點：沒有 `extern` 的 `long hits;` 是定義，標頭檔被兩個 `.c` 引入就各有一份。作業 2-1 在 `logger.h` 犯的就是這個錯，這次說對了。補上術語：`long hits;` 叫**暫定定義**（同一個檔案沒有別的定義時就當成 `long hits = 0;`），所以「看起來像宣告，其實是定義」。
>
> **參考答案**【[第六節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#忘了-extern每個-c-都有一份定義)〈忘了 `extern`：每個 `.c` 都有一份定義〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `long hits;`（沒有 `extern`、沒有初值）是暫定定義，實際上就是**定義**。
> 2. 標頭檔被貼進兩個 `.c`，兩個 `.o` 各有一個 `GLOBAL` 的 `hits` → 連結器看到兩個同名定義 → `multiple definition`。
>
> **詳細說明**：第六節實測：
>
> ```text
> $ readelf -s hits.o hits_main.o | grep -E "File| hits$"
> File: hits.o
>      3: 0000000000000000     8 OBJECT  GLOBAL DEFAULT    4 hits
> File: hits_main.o
>      4: 0000000000000000     8 OBJECT  GLOBAL DEFAULT    4 hits
> $ gcc -o hits hits_main.o hits.o
> /usr/bin/ld: hits.o:(.bss+0x0): multiple definition of `hits'; hits_main.o:(.bss+0x0): first defined here
> ```
>
> 兩個 `.o` 都是 8 byte 的 `OBJECT`、`Ndx` 4（`.bss`）——各自有一份定義。

#### 4-2. 已經有 include guard 了，為什麼沒擋住？

**第 1 次（2026-10-22）**
```
首先他們是分別編譯成.o的, include guard只能防止,同一個.c重複include相同的標頭,分別編譯的他防止不了
```

> **批改**：**對**。include guard 只防同一個 `.c` 裡重複引入，兩個 `.c` 分別編譯各貼一次，它管不到。
>
> **參考答案**【[第六節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#忘了-extern每個-c-都有一份定義)〈忘了 `extern`：每個 `.c` 都有一份定義〉最後一段】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. include guard 只防止**同一個編譯單元**裡被貼兩次（`HITS_H` 這個巨集只在編譯這一個 `.c` 的期間有效）。
> 2. `hits.c` 和 `hits_main.c` 是分開編譯的兩個編譯單元，各貼一次是正常的；錯誤發生在連結時兩個 `.o` 之間，guard 管不到。
>
> **詳細說明**：guard 的機制見[第三節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#include-guard同一個標頭檔被貼兩次)〈include guard〉：第一次貼進來時定義 `HITS_H`，同一個 `.c` 第二次貼進來就整段丟掉。編譯下一個 `.c` 時前處理器從頭開始，`HITS_H` 又是沒定義的。

#### 4-3. 該怎麼改（標頭檔寫什麼、哪裡寫什麼）？

**第 1 次（2026-10-22）**
```
標頭檔 hits.h 的 long hits 前面加上 extern宣告全局唯一就好
```

> **批改**：**不完整**。標頭檔改成 `extern long hits;` 對，但只做這一半會變成另一個錯：`extern` 只是宣告、不配置空間，題目說兩個 `.c` 都沒有自己的定義，結果**整個程式沒有任何一份 `hits`**，連結時 `undefined reference to 'hits'`（實測見下方詳細說明）。
>
> 所以另一半一定要講：**恰好一個 `.c`**（自然是 `hits.c`）寫定義 `long hits = 0;`。「`extern` 宣告全局唯一」這個說法也不對：讓它「唯一」的是只有一個 `.c` 寫定義，`extern` 本身只是說「定義在別的地方」。
>
> **參考答案**【[第六節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#六extern全域變數的宣告與定義)〈整理：全域變數放哪裡〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 標頭檔寫 `extern long hits;`：宣告，不配置空間，幾個 `.c` 引入都沒關係。
> 2. **恰好一個 `.c`**（`hits.c`）寫定義 `long hits = 0;`，而且這個 `.c` 也引入 `hits.h`，讓編譯器檢查宣告與定義的型別一致。
>
> **詳細說明**：只改標頭檔、不補定義的實驗 `examples/quiz-extern-only/`：`hits.h`（檔案：[`hits.h`](examples/quiz-extern-only/hits.h)）
>
> ```c
> #ifndef HITS_H
> #define HITS_H
>
> extern long hits;
> void hit(void);
>
> #endif
> ```
>
> `hits.c`（檔案：[`hits.c`](examples/quiz-extern-only/hits.c)）
>
> ```c
> #include "hits.h"
>
> /* 自我測驗 4-3：只把標頭檔改成 extern，卻沒有任何 .c 寫定義 */
>
> void hit(void)
> {
>     hits = hits + 1;
> }
> ```
>
> `hits_main.c`（檔案：[`hits_main.c`](examples/quiz-extern-only/hits_main.c)）跟題目的一樣。
>
> ```text
> $ gcc -c hits.c hits_main.c && gcc -o hits hits_main.o hits.o
> /usr/bin/ld: hits_main.o: in function `main':
> hits_main.c:(.text+0x1a): undefined reference to `hits'
> /usr/bin/ld: hits.o: in function `hit':
> hits.c:(.text+0xb): undefined reference to `hits'
> ...
> collect2: error: ld returned 1 exit status
> ```
>
> （中間還有兩行 `relocation against 'hits' in read-only section` 的警告，是同一個原因的附帶訊息。）兩個 `.o` 都是 `UND hits`，沒有人提供 → `undefined reference`。在 `hits.c` 補上 `long hits = 0;` 就是第六節的正確版本，印 `hits = 3`。
>
> | 放在 | 寫 |
> |---|---|
> | 標頭檔 | `extern long hits;` |
> | 恰好一個 `.c` | `long hits = 0;` |
> | 只有自己檔案用 | `static long hits = 0;`，不寫進標頭檔 |

#### 4-4. 為什麼同樣的程式碼在舊版 gcc 上可以連結？

**第 1 次（2026-10-22）**
```
舊版會把兩個定義都讓他存在,但改假如有用到這個變數,改的地方會不一致就是hits_main會讀寫,他定義的
而hits.c也是讀寫他定義的
```

> **批改**：**錯**。舊版不是「兩份各自存在、各讀寫各的」，而是**合併成一份**。想想如果真的是兩份：`hit()` 加的是 `hits.c` 那份，`main` 印的是自己那份，會印 `hits = 0`；實際上舊版（等同本機加 `-fcommon`）印 `hits = 3`，兩個檔案用的是同一個變數。
>
> **參考答案**【[第六節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#忘了-extern每個-c-都有一份定義)〈`-fcommon`：舊程式為什麼這樣寫也能動〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. gcc 10 以前預設 `-fcommon`：沒有初值的全域變數被標成 **common 符號**（`Ndx` 是 `COM`，`nm` 印 `C`），不是一般的定義。
> 2. 連結器遇到好幾個同名的 common 符號會**合併成一份**，不報錯，兩個檔案共用同一個 `hits`。
>
> **詳細說明**：第六節實測（本機 gcc 13 用 `-fcommon` 模擬舊版行為；更舊的 gcc 本機沒有，未實測）：
>
> ```text
> $ gcc -fcommon -c hits.c hits_main.c
> $ readelf -s hits.o hits_main.o | grep -E "File| hits$"
> File: hits.o
>      3: 0000000000000008     8 OBJECT  GLOBAL DEFAULT  COM hits
> File: hits_main.o
>      4: 0000000000000008     8 OBJECT  GLOBAL DEFAULT  COM hits
> $ gcc -o hits hits_main.o hits.o && ./hits
> hits = 3
> ```
>
> gcc 10 起預設改成 `-fno-common`，所以舊專案升級 gcc 後冒出一堆 `multiple definition`。正確修法是補 `extern`（4-3），不是加 `-fcommon`。`-fcommon` 也只合併**沒有初值**的；有初值的定義照樣衝突（[第六節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#補充extern-的三個邊界情況2026-10-21-作業第-2-題批改時補教)〈補充：`extern` 的三個邊界情況〉，作業 2-3）。

**自我練習**（不批改）

```
舊版gcc 默認加上-fcommon, 沒有初值的全域變數會被標成com,然後就合併成一個
```

### 5. 情境題：改了標頭檔，`make` 說不用重編

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

#### 5-1. `make` 是怎麼判斷不用重編的？

**第 1 次（2026-10-22）**
```
他是根據目標檔的時間是否比 先決條件的時間 舊, 如果是它就會執行 ,不是就不會
```

> **批改**：**對**。規則講對：先決條件的修改時間比目標新就執行命令，否則跳過。套進這題：`server.o` 的先決條件只有 `server.c`，`server.c` 沒改、`server.o` 比它新 → 跳過；`net.o` 同理；兩個 `.o` 都沒更新，`server` 也比它們新 → `up to date`。另一個條件「目標不存在也執行」沒寫，不影響這題。
>
> **參考答案**【[第七節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#文法規則)〈文法：規則〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 目標不存在，或任何一個**先決條件的修改時間比目標新**，就執行命令；否則跳過（先決條件也是別的規則的目標時，先遞迴處理它）。
> 2. 本例 `server.o`、`net.o` 的先決條件只有各自的 `.c`，`.c` 都沒改 → 都不重編 → `server` 也不必重新連結。
>
> **詳細說明**：`make` 比的是檔案系統記錄的修改時間（`ls -l --time-style=full-iso` 看得到），它不看檔案內容。

#### 5-2. 為什麼這個判斷錯了？

**第 1 次（2026-10-22）**
```
因爲實際上.h等更新時間應該要比.o 或者server還要更新,理論上是要重編
但是這個Makefile的先決條件都沒有.h所以make也無從判斷到
```

> **批改**：**對**。抓到重點：`make` 只看 `Makefile` 寫的先決條件，`config.h` 不在裡面，它無從判斷。補上你沒寫出的「為什麼一定要重編」：`BUF_SIZE` 是巨集，前處理時就被換成 64 寫死在 `.o` 的機器碼裡，不重編就永遠是 64。
>
> **參考答案**【[第七節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#先決條件漏了標頭檔make-說不用重編結果是舊的)〈先決條件漏了標頭檔〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `make` 只看 `Makefile` 寫的先決條件，不會自己去讀 `.c` 裡 `#include` 了哪些檔案；`config.h` 沒列進去，它改了 `make` 不知道。
> 2. 可是 `config.h` 的內容在編譯時被貼進 `server.c`／`net.c`，它變了 `.o` 就該重編。
>
> **詳細說明**：第七節 `Makefile.nodeps` 是同一個情況：把 `mathx.h` 的 `DEFAULT_N` 從 7 改成 9，`make` 說 `'calc' is up to date.`，程式還是印 `square(7) = 49`。這種錯最難查：程式碼看起來是對的，跑出來的卻是舊版。

#### 5-3. 你會怎麼改這個 `Makefile`，讓以後改任何標頭檔都會重編該重編的檔案？（不必寫出完整的 `Makefile`，說明要改哪些地方、各自的作用即可。）

**第 1 次（2026-10-22）**
```
gcc 增加兩個選項
-MMD: 可以讓gcc產生.d檔 讓.c引入的使用者的header 可以寫入.o的先決條件,不包含系統級的標頭檔案
-MP:沒有先決條件不產生命令

再加上 -include $(OBJS:.o=.d), 把.d的條件一起加入到makefile
```

> **批改**：**大致對**。主線對：用 `-MMD` 讓 gcc 寫出 `.d`、用 `-include` 把它讀進 `Makefile`。缺／錯的地方（跟作業 5-1 卡在同一處：`-MP`、`-include` 的說明不足）：
>
> - `-MP` 說成「沒有先決條件不產生命令」不對。`-MP` 是替每個標頭檔**多寫一條沒有先決條件、沒有命令的空規則**（例如 `config.h:`），作用是：之後把 `config.h` 刪掉或改名，`make` 不會因為「`server.o` 依賴的 `config.h` 不存在、又沒有規則能產生它」而停下來報錯。
> - `-include` 前面的 `-` 沒說：「`.d` 不存在也不報錯」——第一次 `make` 時還沒有 `.d`（那時 `.o` 也不存在，反正都要編）。
> - 細節（不扣分）：這個 `Makefile` 沒有 `OBJS` 變數，要先定義；兩條 `gcc -Wall -c` 命令要真的加上 `-MMD -MP`。
>
> **參考答案**【[第七節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#實用的-makefile變數樣式規則自動產生相依)〈實用的 `Makefile`〉第 5、6 點】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 編譯命令加 `-MMD -MP`：編譯時順便寫出 `.d` 檔，內容是「這個 `.o` 依賴哪些使用者標頭檔」的規則（例如 `server.o: server.c config.h`）；`-MP` 再替每個標頭檔加一條空規則，標頭檔刪掉或改名時 `make` 不會停下來。
> 2. `Makefile` 加 `-include $(OBJS:.o=.d)`：把 `.d` 讀進來、跟原本的規則合併，`config.h` 就成了 `.o` 的先決條件；`-` 表示第一次還沒有 `.d` 時不報錯。
>
> **詳細說明**：改好的版本（命令前面是 TAB；`clean`／`.PHONY` 不是這題必要的，順便附上）：
>
> ```make
> CC     = gcc
> CFLAGS = -Wall -MMD -MP
> OBJS   = server.o net.o
>
> server: $(OBJS)
> 	$(CC) -o $@ $^
>
> %.o: %.c
> 	$(CC) $(CFLAGS) -c $< -o $@
>
> -include $(OBJS:.o=.d)
>
> clean:
> 	rm -f server $(OBJS) $(OBJS:.o=.d)
>
> .PHONY: clean
> ```
>
> 本機用三個小檔案實測（`config.h` 只有 `#define BUF_SIZE 64`；`net.c` 引入它、`int net_size(void) { return BUF_SIZE; }`；`server.c` 引入它和 `stdio.h`，`main` 印 `printf("server %d, net %d\n", BUF_SIZE, net_size());`）：
>
> ```text
> $ make && ./server
> gcc -Wall -MMD -MP -c server.c -o server.o
> gcc -Wall -MMD -MP -c net.c -o net.o
> gcc -o server server.o net.o
> server 64, net 64
> $ cat server.d
> server.o: server.c config.h
> config.h:
> $ sed -i 's/64/128/' config.h
> $ make && ./server
> gcc -Wall -MMD -MP -c server.c -o server.o
> gcc -Wall -MMD -MP -c net.c -o net.o
> gcc -o server server.o net.o
> server 128, net 128
> ```
>
> `server.d` 第 1 行就是原本漏掉的相依，第 2 行 `config.h:` 是 `-MP` 加的空規則。（只改這一次的話，手動把 `config.h` 加進兩條規則的先決條件也行，但之後每加一個 `#include` 都要記得改，所以讓 gcc 產生。）

### 6. 符號是什麼：`t.o` 的符號表裡有誰

（2026-10-20 提問「符號代表什麼」追加，出處：[第一節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#一本課在做什麼編譯器一次只看一個-c)〈上面說的「符號」到底是什麼〉）下面的 `t.c` 用 `gcc -c t.c` 編成 `t.o`。哪些名字會出現在 `t.o` 的符號表裡？其中哪些是「這個檔案提供」、哪些是「需要別人提供」？`n`、`sum`、`i` 為什麼不在符號表裡？對連結器來說，符號表裡的一個符號代表什麼、拿來做什麼用？

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

#### 6-1. 哪些名字會出現在 `t.o` 的符號表裡？其中哪些是「這個檔案提供」、哪些是「需要別人提供」？

**第 1 次（2026-10-22）**
```
add 檔案提供
main 檔案提供
limit 檔案提供
total 檔案提供
puts 別人提供
```

> **批改**：**不完整**。`add`、`main`、`total` 是這個檔案提供、`puts` 要別人提供，都對；**`limit` 錯了**：`extern long limit;` 只是宣告、不配置空間，`t.c` 裡沒有它的定義，所以它是 `UND`——**需要別人提供**。這正是第六節 `extern` 的重點（題目最後也提示了「`limit` 沒有定義，連結會失敗」）。本機實測的符號表：
>
> ```text
> $ readelf -s t.o
>    Num:    Value          Size Type    Bind   Vis      Ndx Name
>      0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
>      1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS t.c
>      2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
>      3: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 .rodata
>      4: 0000000000000000     8 OBJECT  GLOBAL DEFAULT    4 total
>      5: 0000000000000000    44 FUNC    GLOBAL DEFAULT    1 add
>      6: 000000000000002c    85 FUNC    GLOBAL DEFAULT    1 main
>      7: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND limit
>      8: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND puts
> ```
>
> **參考答案**【[第一節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#一本課在做什麼編譯器一次只看一個-c)〈上面說的「符號」到底是什麼〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 符號表裡有：`total`、`add`、`main`、`limit`、`puts`（加上 ELF 記帳用的 `FILE`、`SECTION` 列）。
> 2. 這個檔案提供（有定義，`Ndx` 是區段編號）：`total`（`.bss`）、`add`、`main`（`.text`）。
> 3. 需要別人提供（`UND`）：`limit`（只有 `extern` 宣告）、`puts`（libc）。
>
> **詳細說明**：`nm t.o` 是同一份的精簡版：`T add`、`T main`、`B total`（`.bss`，初值 0）、`U limit`、`U puts`。`extern` 宣告的變數跟呼叫了但沒定義的函式，在符號表裡是同一種東西：`UND`、`NOTYPE`（這個檔案只知道名字）。

**自我練習**（不批改）

```
limits 因爲只有宣告,所以需要別人提供,在.o是und
```

#### 6-2. `n`、`sum`、`i` 為什麼不在符號表裡？

**第 1 次（2026-10-22）**
```
因爲他們在函式運算階段就可以直接算出來(用函式的stack),不需要符號與特定記憶體位址
```

> **批改**：**對**。抓到重點：它們在函式的堆疊上，不需要符號與固定的記憶體位址。用詞小提醒：「在函式運算階段就可以直接算出來」不精確——不是「值」算得出來，是**位址**由編譯器自己寫成 `%rbp` 的偏移（`n` 在 `-0x18(%rbp)`、`sum` 在 `-0x8(%rbp)`），不用等連結器填。作業 1-3 漏的就是這一點，這次答到了。
>
> **參考答案**【[第一節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#一本課在做什麼編譯器一次只看一個-c)〈上面說的「符號」到底是什麼〉「區域變數不是符號」那段】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `n`（參數）、`sum`、`i` 是函式裡的變數，放在堆疊框（或暫存器），位址是執行時 `%rbp` 的偏移。
> 2. 這個偏移編譯器自己就寫得出來，不需要連結器決定位址，所以不需要名字、不進符號表。
>
> **詳細說明**：`objdump -dr t.o` 的 `add` 裡，`mov %rdi,-0x18(%rbp)`（存 `n`）、`mov %rax,-0x8(%rbp)`（存 `sum`）下面都沒有重定位項目；讀寫 `total` 的 `mov 0x0(%rip),%rdx` 下面才有 `R_X86_64_PC32 total-0x4`。有洞的地方才需要名字。

#### 6-3. 對連結器來說，符號表裡的一個符號代表什麼、拿來做什麼用？

**第 1 次（2026-10-22）**
```
符號用來做不同檔案的連接,假如是global 且符號相同就可以連接,
例如main檔案雖然有puts的符號,但是他不會知道puts的記憶體位址,實際上puts的記憶體位址在另外一個檔案,所以interpreter會在如libc,然後找出這個符號的記憶體位址在做連接,這樣檔程式跑到puts的時候才知道接下來要跳去那裏
```

> **批改**：**不完整**。答到「拿來在不同檔案之間配對（`GLOBAL`、同名）」——這是符號解析。缺兩點：
>
> - 題目問的「**符號代表什麼**」沒答：符號 ＝ 一個名字，加上它代表的東西**在哪裡**（哪個區段、區段裡第幾 byte、多大）；有定義的是「我提供」，`UND` 的是「我需要」。
> - 連結器配對完還要做**重定位**：算出每個符號的最終位址，填進機器碼裡留的洞（`objdump -dr` 看到的 `R_X86_64_PC32 total-0x4` 這種項目）。
>
> 另外 `puts` 那段把兩個工具混在一起了：interpreter（`ld.so`）是**執行時**的動態載入器，題目問的是**連結器**（`ld`，建置時）。`puts` 在 libc 這個共享函式庫裡，連結器建置時只確認 libc 有這個符號、留下「執行時再接」的記錄，真正的位址才由 `ld.so` 執行時填（[第 1 課第五節](../../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html#五連結把洞接上但-puts-還是沒接完)）。對 `add`、`total` 這種同一個程式裡的符號，位址是連結器自己填的，跟 interpreter 無關。
>
> **參考答案**【[第一節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#一本課在做什麼編譯器一次只看一個-c)〈上面說的「符號」到底是什麼〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 一個符號 ＝ 名字＋它在哪裡（區段、偏移、大小）；有定義的代表「這個檔案提供」，`UND` 代表「這個檔案需要」。
> 2. 連結器用它做**符號解析**：把每個 `UND` 跟別的 `.o`／函式庫的 `GLOBAL` 定義配對（只比名字）。
> 3. 再做**重定位**：算出每個符號的最終位址，填進機器碼裡標著那個符號名字的洞。
>
> **詳細說明**：`objdump -dr --no-show-raw-insn t.o` 裡有洞的指令（只截有重定位項目的幾行）：
>
> ```text
>    c:	mov    0x0(%rip),%rdx        # add 讀 total
> 			f: R_X86_64_PC32	total-0x4
>   49:	call   4e <main+0x22>        # main 呼叫 add
> 			4a: R_X86_64_PLT32	add-0x4
>   5f:	mov    0x0(%rip),%rax        # main 讀 limit
> 			62: R_X86_64_PC32	limit-0x4
>   75:	call   7a <main+0x4e>        # main 呼叫 puts
> 			76: R_X86_64_PLT32	puts-0x4
> ```
>
> 每個洞都標著一個符號名字，連結器的工作就是「找到這個名字的定義（解析）→ 算出位址填進去（重定位）」。`limit` 找不到任何定義，所以這題連結會 `undefined reference to 'limit'`。

**自我練習**（不批改）

```
符號 代表 一個名字 + 他的空間位址,與他的類型等, ld可以把重新定位這些符號的位址,有些符號位址可以直接出來,比方定義在當前檔案的,如果是要從別的檔案來的,這個符號就會是und 留洞,在執行的時候有ld.so把洞填滿
```
