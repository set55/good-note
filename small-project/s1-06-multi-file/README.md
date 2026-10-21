# s1-06：多檔案的 C 程式——標頭檔、`static`／`extern`、連結錯誤、`make`

> 對應課程：第 ① 站第 6 課
> 課程筆記：[多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html)
> 自我測驗：寫在同資料夾的 [quiz.md](./quiz.md)

## 說明

這一課要**自己動手寫**：把程式拆成好幾個檔案、看符號表、讀連結錯誤、寫 `Makefile`。

- 每一題一個資料夾（`hw1/`～`hw5/`），檔案自己建。題目給的程式碼照打；題目只給說明的，照說明自己寫。
- 編譯一律加 `-Wall`，**有警告就要處理**，不能忽略。
- 看符號表用 `readelf -s 檔名.o`（只貼題目問的那幾列就好）。
- 編譯產物不會進 git（白名單），要留的輸出**貼進作答框**。
- 筆記裡的範例可以參考，但每一題的檔案、型別、情境都跟範例不同，照抄範例答不出來。

每題「解釋原因」的地方都標了答案的來源：
- 【筆記第 X 節】：本課教過，回筆記那一節找。
- 【新觀念，先猜】：還沒教過，寫下你的猜測就好，猜錯不算錯。

**「先預測」的題目，一定要先把預測寫進作答框，再編譯。** 預測錯了沒關係，把錯在哪裡寫下來，這才是練習的重點。

**怎麼作答**：每個小題下面有作答框，直接寫在裡面（把 `YYYY-MM-DD` 換成作答日期）。批改與參考答案寫在作答框正下方，不重答。

---

## 筆記的範例程式

課程筆記裡每個實驗的原始碼都放在 [`examples/`](./examples/)，一個實驗一個子資料夾，可以直接跟著筆記重做。**它們是範例，不是作業**；作業寫在 `hw1/`～`hw5/`，檔名跟範例不重複。

| 資料夾 | 檔案 | 筆記裡的位置 | 示範什麼 |
|---|---|---|---|
| [`examples/implicit/`](./examples/implicit/) | [`main.c`](./examples/implicit/main.c)、[`mathx.c`](./examples/implicit/mathx.c) | [第二節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#二宣告與定義沒有宣告編譯器只能猜)〈實驗：忘了宣告〉 | 沒有宣告，`square(100000)` 印出 1410065408（`cltq` 只取低 32 位元） |
| [`examples/header/`](./examples/header/) | [`mathx.h`](./examples/header/mathx.h)、[`mathx.c`](./examples/header/mathx.c)、[`main.c`](./examples/header/main.c)、[`bad.c`](./examples/header/bad.c)、[`inc.c`](./examples/header/inc.c)、[`mathx2.c`](./examples/header/mathx2.c) | [第三節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#三標頭檔把宣告寫一次讓兩邊都檢查)、[第四節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#四符號表o-之間靠名字接起來) | 標頭檔、`conflicting types`、`<>` 找不到與 `-I`、符號表、`undefined reference`、`multiple definition` |
| [`examples/guard/without/`](./examples/guard/without/)、[`examples/guard/with/`](./examples/guard/with/) | `point.h`、`draw.h`、`move.h`、`guard_main.c` | 第三節〈include guard〉 | 沒有 guard 時 `redefinition of 'struct point'`；有 guard 只貼一次 |
| [`examples/static/`](./examples/static/) | [`a.c`](./examples/static/a.c)、[`b.c`](./examples/static/b.c)、[`ab_main.c`](./examples/static/ab_main.c)、[`callhelper.c`](./examples/static/callhelper.c) | [第五節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#五static兩種完全不同的意思)〈檔案層級的 `static`〉 | 兩個 `static helper` 變 `LOCAL` 不衝突；別的檔案呼叫不到 |
| [`examples/staticlocal/`](./examples/staticlocal/) | [`nextid.c`](./examples/staticlocal/nextid.c) | 第五節〈函式裡的 `static`〉 | `static` 區域變數在 `.bss`、值留到下一次 |
| [`examples/extern/`](./examples/extern/)、[`examples/extern/bad/`](./examples/extern/bad/) | `hits.h`、`hits.c`、`hits_main.c` | [第六節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#六extern全域變數的宣告與定義) | 標頭檔寫 `extern`；忘了 `extern` 時 `multiple definition`、`-fcommon` |
| [`examples/extern-rules/`](./examples/extern-rules/) | `hits.h`、`hits.c`、`hits_main.c`、[`unused.c`](./examples/extern-rules/unused.c)、[`init_main.c`](./examples/extern-rules/init_main.c)、[`static_h.c`](./examples/extern-rules/static_h.c)、[`static_noh.c`](./examples/extern-rules/static_noh.c) | [第六節〈補充：`extern` 的三個邊界情況〉]({SUP})（作業第 2 題批改時補教） | 沒用到的宣告不產生符號；`-fcommon` 不合併有初值的定義；`extern` 後接 `static` 編譯錯、不引入標頭檔則 `undefined reference` |
| [`examples/make/`](./examples/make/) | [`Makefile`](./examples/make/Makefile)、[`Makefile.nodeps`](./examples/make/Makefile.nodeps)、[`Makefile.nophony`](./examples/make/Makefile.nophony)、[`mathx.h`](./examples/make/mathx.h)、[`mathx.c`](./examples/make/mathx.c)、[`main.c`](./examples/make/main.c) | [第七節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#七make只重編改過的檔案) | 規則與時間判斷、漏了標頭檔相依、`-MMD -MP`、`.PHONY` |
| [`examples/symbols/`](./examples/symbols/) | [`sym.c`](./examples/symbols/sym.c)、[`other.c`](./examples/symbols/other.c) | [第一節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#一本課在做什麼編譯器一次只看一個-c)〈上面說的「符號」到底是什麼〉（2026-10-20 提問） | 函式、全域變數、`extern` 變成符號，區域變數不是；機器碼的洞上標的是符號名 |

---

## 批改結果（2026-10-21）

第 1、2 題已批改（2026-10-21）；第 3 題以後還沒作答。

| 題目 | 結果 | 錯在哪／缺什麼 |
|---|---|---|
| 1-1 | 大致對 | 三個檔案、include guard、`static is_digit`、輸出都對，`static` 的理由也對；引入自己標頭檔的理由講錯：不是讓宣告和定義「連接起來」，是讓**編譯器**同時看到兩者、不一致就報錯 |
| 1-2 | 對 | 抓到：`is_digit` 因為 `static` 是 `LOCAL`；有定義的 `Ndx` 1 ＝ `.text`；`hw1_main.o` 裡三個是 `UND`，定義在別的檔案，要等連結。小錯：`UND` 那幾列的 `GLOBAL` 不是「全域可調用」，是「要拿去跟別人的 `GLOBAL` 定義配對」 |
| 1-3 | 大致對 | 預測「沒有」、堆疊都對；漏了關鍵：堆疊上的變數用 `-0x18(%rbp)` 這種偏移存取，**位址不用連結器填，所以不留名字** |
| 1-4 | 不完整 | (a) 編譯階段、`conflicting types` 都對；(b) 沒有實際跑、沒貼結果，「不會有問題」是錯的：能編、能連、印出 17，但那是**碰巧對**——連結器只比名字不比型別，兩邊型別不一致沒有任何階段抓到 |
| 2-1 | 錯 | `logger.h` 寫成 `int log_level;`，那是暫定定義（＝定義），三個 `.c` 都引入就有三份 → `multiple definition`。標頭檔要寫 `extern int log_level;`；也沒寫說明 |
| 2-2 | 對 | 預測與確認都對：`logger.o` 的 `Ndx` 3 ＝ `.data`、`hw2_main.o` 是 `UND`；抓到跟 `hits` 不同區段是因為初值 2 vs 0。`worker.o` 沒有 `log_level` 這一列（沒用到就不產生符號）是新觀念，已補教 |
| 2-3 | 不完整 | 新觀念，「會成功」猜錯不計分；扣在實驗沒做成：照題目做，連結會報 `multiple definition`（`-fcommon` 只合併沒初值的定義），貼上的執行結果是舊的 `dowork`，不是這次編出來的 |
| 2-4 | 對 | 新觀念先猜，猜中「編譯時就錯」，訊息也貼了，理由（`extern` 與 `static` 互相矛盾）對。補教：沒引入標頭檔時會拖到連結才 `undefined reference` |

---

## 1. 把一個檔案拆成三個

下面是一個單檔程式 `wordcount.c`（這份不用存，拆完的三個檔案才是作業）：

```c
#include <stdio.h>

int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

long count_digits(const char *s)
{
    long n = 0;
    for (; *s != '\0'; s++)
        if (is_digit(*s))
            n++;
    return n;
}

long length(const char *s)
{
    long n = 0;
    while (s[n] != '\0')
        n++;
    return n;
}

int main(void)
{
    const char *msg = "room 101, floor 3";
    printf("length = %ld, digits = %ld\n", length(msg), count_digits(msg));
    return 0;
}
```

在 `hw1/` 裡把它拆成三個檔案：

- `strstat.h`：`count_digits`、`length` 的宣告，要有 include guard。
- `strstat.c`：`is_digit`、`count_digits`、`length` 的定義。`is_digit` 只給這個檔案自己用。
- `hw1_main.c`：`main`。

各自用 `gcc -Wall -c` 編成 `.o`，再連結成 `wordcount`。

### 1-1. 貼上三個檔案的內容與執行結果。`is_digit` 你怎麼處理？為什麼？`strstat.c` 有沒有 `#include "strstat.h"`？為什麼要（或不要）？【筆記[第三節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#三標頭檔把宣告寫一次讓兩邊都檢查)〈定義的那個 `.c` 為什麼也要 `#include` 自己的標頭檔〉、[第五節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#五static兩種完全不同的意思)〈什麼時候用〉】

**第 1 次（2026-10-21）**
````
```strstat.h
#ifndef STRSTAT_H
#define STRSTAT_H

long count_digits(const char *s);

long length(const char *s);

#endif // STRSTAT_H
```

```strstat.c
#include "strstat.h"

static int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

long count_digits(const char *s)
{
    long n = 0;
    for (; *s != '\0'; s++)
        if (is_digit(*s))
            n++;
    return n;
}

long length(const char *s)
{
    long n = 0;
    while (s[n] != '\0')
        n++;
    return n;
}
```

```hw1_main.c
#include "strstat.h"
#include <stdio.h>

int main(void)
{
    const char *msg = "room 101, floor 3";
    printf("length = %ld, digits = %ld\n", length(msg), count_digits(msg));
    return 0;
}   
```

 ~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw1 | on main >1 !2 ?3  ./wordcount                                                                                                   ok | base py | at 15:11:49
length = 17, digits = 4

is_digits 用了static, 這樣這個函式就會變成local,其他檔案無法使用,連接器也不會連接他

strstat.c有strstat.h,這樣這個標頭的declaration and definition才能夠連接起來                                                
````

> **批改**：**大致對**。三個檔案拆得對：`strstat.h` 有 include guard、只放兩個宣告；`is_digit` 加了 `static`、沒寫進標頭檔；`hw1_main.c` 只有 `main`。我在 scratchpad 用你的檔案 `gcc -Wall -c` 重編，沒有警告，輸出 `length = 17, digits = 4` 正確。
> `is_digit` 的理由對（`static` → `LOCAL`，別的檔案用不到，連結器不拿它配對）。
> 引入自己標頭檔的理由**講錯了機制**：宣告和定義不需要誰去「連接」——把 `count_digits` 的呼叫接到定義，是連結器靠**名字**做的，有沒有引入標頭檔都一樣會接上。引入標頭檔的作用發生在**編譯** `strstat.c` 的時候：編譯器同時看到宣告與定義，**兩者不一致就報錯**（你在 1-4 (a) 自己做出了這個錯誤）。
>
> **參考答案**【筆記[第三節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#三標頭檔把宣告寫一次讓兩邊都檢查)〈[定義的那個 `.c` 為什麼也要 `#include` 自己的標頭檔](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#定義的那個-c-為什麼也要-include-自己的標頭檔)〉、[第五節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#五static兩種完全不同的意思)〈什麼時候用〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `is_digit` 加 `static`、不放進標頭檔：只給本檔案用的函式一律 `static`，符號變成 `LOCAL`，別的檔案看不到、也不會跟別處同名的函式撞。
> 2. `strstat.c` 要 `#include "strstat.h"`：這樣編譯 `strstat.c` 時編譯器同時看到宣告和定義，型別不一致就在**編譯時**報 `conflicting types`；不引入的話，改了定義忘了改標頭檔，`hw1_main.c` 照舊的宣告呼叫，沒有人發現。
>
> **詳細說明**：`strstat.c` 其實不靠標頭檔也能編得過（它自己就有定義），所以引入不是「為了能編譯」，而是「為了讓編譯器幫你檢查」。標頭檔是使用的那邊（`hw1_main.c`）和定義的那邊（`strstat.c`）共同看的那一份說明，兩邊都對照它，才保證兩邊一致。

**自我練習**（不批改）

```
使用static, 這樣這個函式才能變成只有strstat自己能用 local

有, 這樣gcc才能檢查你的定義與宣告的型別是否相同,否則就算編的過,main還是按舊的宣告呼叫
也就是說重點不是能不能編的過(沒h也能編的過)而是要在編譯時檢查main與strstat是否一致
```

### 1-2. 用 `readelf -s` 看 `strstat.o` 和 `hw1_main.o`，貼上 `is_digit`、`count_digits`、`length`、`main`、`printf` 這幾列（在哪個 `.o` 出現就貼哪個）。每一列的 `Bind` 和 `Ndx` 是什麼？說明為什麼是這樣。【筆記[第四節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#四符號表o-之間靠名字接起來)的欄位表、第五節】

**第 1 次（2026-10-21）**
```
~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw1 | on main >1 !4 ?3  readelf -sW strstat.o                                                                                         ok | base py | at 15:18:20

Symbol table '.symtab' contains 6 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS strstat.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000    39 FUNC    LOCAL  DEFAULT    1 is_digit # local只有strstat可以使用
     4: 0000000000000027    74 FUNC    GLOBAL DEFAULT    1 count_digits
     5: 0000000000000071    51 FUNC    GLOBAL DEFAULT    1 length

is_digit 是local 代表只有strstat.c可以使用,另外他的ndx是1, 說明他屬於.text的區段
count_digits是global全域,所有人都可以調用 它也屬於.text
length與count_digits一樣

~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw1 | on main >1 !4 ?3  readelf -sW hw1_main.o                                                                                        ok | base py | at 15:18:25

Symbol table '.symtab' contains 8 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS hw1_main.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 .rodata
     4: 0000000000000000    88 FUNC    GLOBAL DEFAULT    1 main
     5: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND count_digits
     6: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND length
     7: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND printf

main 是global,全域可以調用,他的ndx是1,與.text相同,代表它也是.text區段
count_digits, length, printf 都是global,全域可調用,但是ndx都是und, 因爲hw1_main.o尚未連接,他不知道在其他檔案他們算是什麼區段
```

> **批改**：**對**。五個符號都找到，`Bind`、`Ndx` 都讀對：`is_digit` 是 `LOCAL`（因為 `static`），`count_digits`、`length`、`main` 是 `GLOBAL`，`Ndx` 1 是 `.text`（我用 `readelf -S` 確認過兩個 `.o` 的第 1 個區段都是 `.text`）；`hw1_main.o` 裡三個是 `UND`，因為定義在別的檔案。
> 一個用詞要修正：`UND` 那幾列的 `GLOBAL` **不是「全域可調用」**——`hw1_main.o` 根本沒有提供它們。`UND` ＋ `GLOBAL` 的意思是「我需要一個叫這個名字的東西，連結時去找別人的 `GLOBAL` 定義來配對」。另外 `printf` 不在任何你寫的 `.o` 裡，連結時是從 C 函式庫（libc）找到的。
>
> **參考答案**【筆記[第四節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#四符號表o-之間靠名字接起來)的欄位表、[第五節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#檔案層級的-static讓名字變成-local)〈檔案層級的 `static`〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `strstat.o`：`is_digit` 是 `LOCAL`（加了 `static`，只在本檔有效）；`count_digits`、`length` 是 `GLOBAL`（別的檔案可以拿來配對）；三個的 `Ndx` 都是 `.text` 的編號，因為函式的機器碼放在 `.text`。
> 2. `hw1_main.o`：`main` 是 `GLOBAL`、`Ndx` 是 `.text`；`count_digits`、`length`、`printf` 是 `UND`，因為這個檔案只有宣告、沒有定義，要等連結時由 `strstat.o`（和 libc 的 `printf`）提供。
>
> **詳細說明**：`UND` 的符號 `Type` 是 `NOTYPE`、`Size` 是 0，因為這個檔案只知道名字，不知道它多大、是函式還是變數。連結器做的事就是把每個 `UND` 跟別處同名的 `GLOBAL` 定義配起來；`LOCAL` 的 `is_digit` 不參加配對，所以就算別的檔案也有一個 `is_digit` 也不會衝突。

### 1-3. `msg` 這個字串在 `hw1_main.o` 的符號表裡有沒有一列叫 `msg`？先預測，再用 `readelf -s` 確認。為什麼？【筆記第五節〈函式裡的 `static`〉（哪些變數有符號、哪些沒有）】

**第 1 次（2026-10-21）**
```
沒有
~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw1 | on main >1 !4 ?3  readelf -sW hw1_main.o                                                                                        ok | base py | at 15:18:25

Symbol table '.symtab' contains 8 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS hw1_main.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 .rodata
     4: 0000000000000000    88 FUNC    GLOBAL DEFAULT    1 main
     5: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND count_digits
     6: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND length
     7: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND printf

因爲msg他並沒有宣告static,所以會用堆疊把這個變量存着
```

> **批改**：**大致對**。預測「沒有」對，`readelf` 也確認了；「沒加 `static` 的區域變數放在堆疊上」也對。漏了一個關鍵：**為什麼放堆疊就沒有符號**。符號是給連結器用的——只有「位址要等連結才知道」的東西才需要名字。堆疊上的變數位址是「執行時的 `%rbp` 減某個數」，編譯器自己就寫得出來。我用 `objdump -dr` 看了你的 `hw1_main.o`，`main` 裡 `msg` 就是 `mov %rax,-0x18(%rbp)`，沒有任何重定位項目指向它。
>
> **參考答案**【筆記[第五節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#函式裡的-static不放堆疊值留到下一次)〈函式裡的 `static`〉（「`plain_id` 的 `n` 不在符號表裡」那一點）、[第一節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#一本課在做什麼編譯器一次只看一個-c)〈上面說的「符號」到底是什麼〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 沒有叫 `msg` 的符號。
> 2. `msg` 是一般的區域變數，放在堆疊上，用 `-0x18(%rbp)` 這種相對 `%rbp` 的偏移存取。
> 3. 這種位址編譯器自己算得出來、不需要連結器填，所以不留名字；只有全域變數、函式、`static` 變數這種位址要等連結才知道的東西才有符號。
>
> **詳細說明**：`msg` 這個**指標**在堆疊上，它指向的**字串內容** `"room 101, floor 3"` 則在 `.rodata`（符號表第 3 列那個 `SECTION .rodata`）。字串也沒有自己的名字：組合語言裡它的標籤是 `.LC0`，`.L` 開頭的區域標籤不寫進符號表，機器碼用「`.rodata` ＋ 偏移」指它（`objdump -dr` 看得到 `R_X86_64_PC32 .rodata-0x4`）。

**自我練習**（不批改）

```
沒有
msg 沒有加static, 這樣msg是存在stack的
存在stack代表他不需要連接器去把符號跟位址連接起來,編譯器自己就可以算出他的位址 用stack -D(%rbp)這種方式
```

### 1-4. 先預測再實驗：把 `strstat.c` 裡 `length` 的定義改成 `int length(const char *s)`（標頭檔不改）。(a) 重新 `gcc -Wall -c strstat.c`，會怎樣？是編譯還是連結階段？貼上訊息。(b) 再把 `strstat.c` 的 `#include "strstat.h"` 拿掉，重新編譯、連結、執行，會怎樣？為什麼 (a) 和 (b) 的結果不一樣？做完把檔案改回來。【筆記第三節〈定義的那個 `.c` 為什麼也要 `#include` 自己的標頭檔〉、第四節〈三種錯誤，各發生在哪一步〉】

**第 1 次（2026-10-21）**
```
a. 會出錯,因爲宣告預定義的類型不同了,在編譯階段
~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw1 | on main >1 !4 ?3  gcc -Wall -c strstat.c                                                                                        ok | base py | at 15:19:16
strstat.c:17:5: error: conflicting types for ‘length’; have ‘int(const char *)’
   17 | int length(const char *s)
      |     ^~~~~~
In file included from strstat.c:1:
strstat.h:6:6: note: previous declaration of ‘length’ with type ‘long int(const char *)’
    6 | long length(const char *s);

b. 那就不會出錯了,因爲對於strstat.c宣告消失了,只剩下定義,不會有問題

a的話你include 標頭檔,宣告與定義兩者要符合,b你沒有宣告不會衝突
```

> **批改**：**不完整**。(a) 全對：編譯階段、`conflicting types`，訊息也貼了。(b) 題目要你「重新編譯、連結、執行」，但作答框裡沒有貼任何結果，只有預測。「不會出錯」對了一半，「不會有問題」是錯的——這題要的正是「**不出錯 ≠ 沒問題**」。
> 我在 scratchpad 照做了一次（拿掉 `#include`、`length` 改回傳 `int`，`hw1_main.c` 不動）：
>
> ```text
> $ gcc -Wall -c s2.c && gcc -o wc2 s2.o hw1_main.o && ./wc2
> length = 17, digits = 4
> ```
>
> 能編、能連、結果還是對的，但那是**碰巧**：`main.c` 照標頭檔的 `long length(...)` 呼叫，`call length` 之後讀整個 `%rax`（`mov %rax,%rsi`）；定義那邊回傳 `int`，依呼叫慣例只保證 `%eax`（低 32 位元）是對的。這次 `-O0` 的 `length` 剛好用 `mov -0x8(%rbp),%rax` 把整個 8 byte 搬進來，高 32 位元才沒有亂掉；換一個寫法或最佳化等級，`main` 拿到的高 32 位元可能是別的東西。
>
> **參考答案**【筆記[第三節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#定義的那個-c-為什麼也要-include-自己的標頭檔)〈定義的那個 `.c` 為什麼也要 `#include` 自己的標頭檔〉、[第四節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#三種錯誤各發生在哪一步)〈三種錯誤，各發生在哪一步〉；`%eax` 回傳值見 [第 5 課筆記第五節](../../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#五參數與回傳值放哪裡)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. (a) 編譯 `strstat.c` 時就報 `conflicting types for 'length'`（編譯階段）：編譯器在同一個編譯單元裡同時看到標頭檔的 `long length(...)` 宣告和 `int length(...)` 定義。
> 2. (b) 拿掉 `#include` 後編譯、連結都成功，也能執行：編譯 `strstat.c` 時沒有宣告可以比對；連結器只比對**名字** `length`，`.o` 的符號表裡沒有型別。
> 3. 但兩邊型別其實不一致：`main` 照 `long` 讀回傳值，定義只回傳 `int`，這個錯誤**沒有任何階段抓到**，結果對不對全看運氣——這就是定義的 `.c` 要引入自己標頭檔的原因。
>
> **詳細說明**：(b) 就是筆記第二節「沒有宣告，`square(100000)` 印出 1410065408」的反方向：那次是**呼叫的那邊**型別錯，這次是**定義的那邊**型別錯，共同點都是「每個 `.c` 各自編譯時都沒看到矛盾、連結器又不看型別」。這次數字很小又剛好 `-O0` 搬整個 `%rax`，所以看起來對；不能把「印出來是對的」當成程式是對的。

**自我練習**（不批改）

```
b 編譯連接執行都ok 但實際上main他的型別是用long會正確只是碰巧
```

---

## 2. 讓三個檔案共用一個全域變數

在 `hw2/` 裡寫一個小程式，有一個全域變數 `int log_level`，`main` 設定它，另外兩個檔案讀它：

- `logger.h`：`log_level` 的宣告，以及 `void log_msg(int level, const char *text);` 的宣告。要有 include guard。
- `logger.c`：`log_level` 的**定義，初值是 2**；`log_msg` 的定義：`level <= log_level` 時印出 `[level] text`，否則什麼都不印。
- `worker.h`、`worker.c`：`void do_work(void);`，裡面呼叫 `log_msg(1, "work start")`、`log_msg(3, "work detail")`。
- `hw2_main.c`：先呼叫一次 `do_work()`，把 `log_level` 改成 3，再呼叫一次 `do_work()`。

### 2-1. 貼上 `logger.h`、`logger.c` 的內容與執行結果。說明 `log_level` 在 `logger.h` 和 `logger.c` 各寫成什麼、為什麼要這樣分。【筆記[第六節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#六extern全域變數的宣告與定義)〈整理：全域變數放哪裡〉】

**第 1 次（2026-10-21）**
````
```logger.h
#ifndef LOGGER_H
#define LOGGER_H

int log_level;

void log_msg(int level, const char *text);

#endif // LOGGER_H
```

```logger.c
#include "logger.h"

int log_level = 2;

void log_msg(int level, const char *text)
{
    if (level <= log_level) {
        printf("[%d] %s\n", level, text);
    }
}
```

我在連接的時候出錯了
 ~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw2 | on main >1 !4 ?3  gcc hw2_main.o logger.o worker.o -o dowork                                                                    ok | base py | at 15:52:11
/usr/bin/ld: logger.o:(.data+0x0): multiple definition of `log_level'; hw2_main.o:(.bss+0x0): first defined here
/usr/bin/ld: worker.o:(.bss+0x0): multiple definition of `log_level'; hw2_main.o:(.bss+0x0): first defined here
collect2: error: ld returned 1 exit status
````

> **批改**：**錯**。`logger.c` 寫得對（`int log_level = 2;` 是定義、`log_msg` 的條件對；貼上來的版本少了 `#include <stdio.h>`，但你資料夾裡的檔案有）。錯在 `logger.h`：`int log_level;` 寫在函式外面是**暫定定義**（tentative definition），實際上就是定義。`logger.h` 被 `logger.c`、`worker.c`、`hw2_main.c` 三個檔案引入，每個 `.o` 各有一份 `GLOBAL` 的 `log_level`，連結器就報 `multiple definition`。說明（`log_level` 在兩個檔案各寫成什麼、為什麼這樣分）也沒寫。
> 錯誤訊息怎麼讀：`logger.o:(.data+0x0): multiple definition of 'log_level'; hw2_main.o:(.bss+0x0): first defined here` ＝「`logger.o` 的 `.data` 第 0 byte 有一份 `log_level`，但 `hw2_main.o` 的 `.bss` 第 0 byte 已經先有一份」；第二行同理是 `worker.o` 那份。為什麼 `logger.o` 那份在 `.data`、另外兩份在 `.bss`，正好是 2-2 要你想的事。
> 這跟筆記第六節〈忘了 `extern`〉的 `hits` 是同一個錯，include guard 救不了：guard 只防同一個 `.c` 貼兩次，三個不同的 `.c` 本來就各貼一次。
> **請你自己把 `logger.h` 改好**（改法見下面參考答案），重新編譯、連結、執行，2-2 之後的題目都要用改好的版本。我在 scratchpad 用你的檔案只改了 `logger.h` 那一行，執行結果是：
>
> ```text
> [1] Work started
> [1] Work started
> [3] Work detail
> ```
>
> （題目要求的字串是 `"work start"`，你寫成 `"Work started"`，不影響這題。）
>
> **參考答案**【筆記[第六節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#六extern全域變數的宣告與定義)〈整理：全域變數放哪裡〉、〈[忘了 `extern`](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#忘了-extern每個-c-都有一份定義)〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `logger.h` 寫 `extern int log_level;`：這是**宣告**，只說「有一個 `int` 叫 `log_level`，定義在別處」，不配置空間，多少個 `.c` 引入都沒關係。
> 2. `logger.c` 寫 `int log_level = 2;`：這是**定義**，整個程式只能有恰好一份；`logger.c` 也引入 `logger.h`，讓編譯器檢查宣告與定義型別一致。
> 3. 標頭檔不能寫 `int log_level;`：那是暫定定義＝定義，標頭檔被貼進幾個 `.c` 就有幾份，連結時 `multiple definition`。
>
> **詳細說明**：改好之後，`worker.c`、`hw2_main.c` 編出來的 `.o` 裡 `log_level` 只會是 `UND`（需要別人提供），`logger.o` 有唯一的定義，連結器把三邊配起來，`main` 裡 `log_level = 3;` 改到的跟 `log_msg` 讀到的是同一塊記憶體——所以第二次 `do_work()` 才會多印 `[3] Work detail`。不要用 `-fcommon` 讓它「連得過」，正確的修法是補 `extern`。

### 2-2. 先預測：`log_level` 在 `logger.o` 的符號表裡，`Ndx` 是 `.bss` 還是 `.data` 的編號？在 `worker.o`、`hw2_main.o` 裡呢？用 `readelf -s` 與 `readelf -S` 確認，貼上相關的列。為什麼跟筆記 `hits` 的例子在不同的區段？【筆記第五節〈函式裡的 `static`〉的 `.bss` 說明、第六節】

**第 1 次（2026-10-21）**
```
.data
und

 ~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw2 | on main >1 !8 ?3  readelf -s logger.o                                                                                           ok | base py | at 16:38:45

Symbol table '.symtab' contains 7 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS logger.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 .rodata
     4: 0000000000000000     4 OBJECT  GLOBAL DEFAULT    3 log_level
     5: 0000000000000000    62 FUNC    GLOBAL DEFAULT    1 log_msg
     6: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND printf

 ~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw2 | on main >1 !8 ?3  readelf -S logger.o                                                                                           ok | base py | at 16:43:36
There are 14 section headers, starting at offset 0x2c0:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .text             PROGBITS         0000000000000000  00000040
       000000000000003e  0000000000000000  AX       0     0     1
  [ 2] .rela.text        RELA             0000000000000000  000001e8
       0000000000000048  0000000000000018   I      11     1     8
  [ 3] .data             PROGBITS         0000000000000000  00000080
       0000000000000004  0000000000000000  WA       0     0     4
  [ 4] .bss              NOBITS           0000000000000000  00000084
       0000000000000000  0000000000000000  WA       0     0     1
  [ 5] .rodata           PROGBITS         0000000000000000  00000084
       0000000000000009  0000000000000000   A       0     0     1
  [ 6] .comment          PROGBITS         0000000000000000  0000008d
       000000000000002e  0000000000000001  MS       0     0     1
  [ 7] .note.GNU-stack   PROGBITS         0000000000000000  000000bb
       0000000000000000  0000000000000000           0     0     1
  [ 8] .note.gnu.pr[...] NOTE             0000000000000000  000000c0
       0000000000000020  0000000000000000   A       0     0     8
  [ 9] .eh_frame         PROGBITS         0000000000000000  000000e0
       0000000000000038  0000000000000000   A       0     0     8
  [10] .rela.eh_frame    RELA             0000000000000000  00000230
       0000000000000018  0000000000000018   I      11     9     8
  [11] .symtab           SYMTAB           0000000000000000  00000118
       00000000000000a8  0000000000000018          12     4     8
  [12] .strtab           STRTAB           0000000000000000  000001c0
       0000000000000023  0000000000000000           0     0     1
  [13] .shstrtab         STRTAB           0000000000000000  00000248
       0000000000000074  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  D (mbind), l (large), p (processor specific)

 ~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw2 | on main >1 !8 ?3  readelf -s worker.o                                                                                           ok | base py | at 16:44:18

Symbol table '.symtab' contains 6 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS worker.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 .rodata
     4: 0000000000000000    51 FUNC    GLOBAL DEFAULT    1 do_work
     5: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND log_msg

~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw2 | on main >1 !8 ?3  readelf -s hw2_main.o                                                                                         ok | base py | at 16:44:47

Symbol table '.symtab' contains 6 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS hw2_main.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000    35 FUNC    GLOBAL DEFAULT    1 main
     4: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND do_work
     5: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND log_level

確認了log_level的ndx是3,也就是.data worker.c沒有用到log_level, main有用到,但在爲連接前log_level是未定義的

因爲筆記hits的初始值是0,而log_level初始值是2
```

> **批改**：**對**。預測「`.data`」「`UND`」都對，`readelf -S` 確認了第 3 個區段是 `.data`。區段不同的原因也抓到了：`hits` 初值 0、`log_level` 初值 2。（貼整份 `readelf -S` 也可以，題目說的「相關的列」只要 `[ 3] .data` 和 `[ 4] .bss` 那兩列。）
> 你注意到 `worker.o` **根本沒有** `log_level` 這一列，這是對的觀察，而且是筆記沒教的事（題目寫「另外兩個檔案讀它」，但照題目的規格 `worker.c` 只呼叫 `log_msg`，沒有讀 `log_level`，題目在這裡寫得不精確）：只有宣告、沒有任何指令用到的名字不會變成符號，連 `UND` 都沒有。已補進筆記[第六節〈補充：`extern` 的三個邊界情況〉](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#補充extern-的三個邊界情況2026-10-21-作業第-2-題批改時補教)第 1 點。
>
> **參考答案**【筆記[第五節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#函式裡的-static不放堆疊值留到下一次)〈函式裡的 `static`〉的 `.bss` 說明、[第六節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#六extern全域變數的宣告與定義)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `logger.o`：`log_level` 的 `Ndx` 是 3，`readelf -S` 對照是 `.data`。
> 2. `hw2_main.o`：`UND`（只有 `extern` 宣告，定義在 `logger.o`）；`worker.o` 沒用到 `log_level`，所以沒有這一列。
> 3. `log_level` 初值是 2（不是 0），值要存在檔案裡，所以放 `.data`；筆記的 `hits` 初值是 0，放 `.bss`，檔案裡只記「要幾 byte」（`NOBITS`），載入時才配一塊全 0 的記憶體。
>
> **詳細說明**：`readelf -S` 也看得到這個差別：`.data` 的 `Size` 是 `4`（就是 `log_level` 那 4 byte 的初值 `02 00 00 00`），型別是 `PROGBITS`（內容存在檔案裡）；`.bss` 的 `Size` 是 0、型別 `NOBITS`，而且 `.bss` 和 `.rodata` 的 `Offset` 都是 `0x84`——`.bss` 在檔案裡不佔空間。

### 2-3. 先猜再實驗：在 `worker.c` 裡**也**寫一行 `int log_level = 1;`（有初值），然後用 `gcc -fcommon` 編譯全部檔案再連結。筆記說 `-fcommon` 會合併同名的全域變數，這次會成功嗎？貼上結果。做完把那一行刪掉。【新觀念，先猜】

**第 1 次（2026-10-21）**
```
可以成功
結果就是編譯成功,執行也成功 我只能貼執行出來,因爲編譯沒有任何輸出
~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw2 | on main >1 !8 ?3  ./dowork                                                                                                 127 err | base py | at 16:54:25
[1] Work started
[1] Work started
[3] Work detail
```

> **批改**：**不完整**。這題是【新觀念，先猜】，「可以成功」猜錯不計分。扣分的地方是**實驗沒做成**，貼上的結果不是這次實驗的結果。我用你的檔案在 scratchpad 照題目做了一次（`worker.c` 加 `int log_level = 1;`，全部用 `-fcommon` 重編）：
>
> ```text
> $ gcc -Wall -fcommon -c hw2_main.c logger.c worker.c
> $ gcc -o dowork hw2_main.o logger.o worker.o
> /usr/bin/ld: worker.o:(.data+0x0): multiple definition of `log_level'; logger.o:(.data+0x0): first defined here
> collect2: error: ld returned 1 exit status
> ```
>
> 編譯沒有輸出是對的，但**連結會失敗**。連結失敗時不會產生新的 `dowork`，所以你執行到的是**之前編好的舊 `dowork`**；你貼的提示列上，前一個指令顯示 `127 err`（找不到指令），表示連結那一步很可能根本沒有跑成功。而且這個輸出本來就分辨不出來：就算 `log_level` 初值真的是 1，第一次 `do_work()` 也一樣只印 `[1]`。
> 以後遇到「看起來成功」的實驗，先確認產物是剛產生的（`ls -l` 看時間），或先 `rm *.o dowork` 再編，才不會被舊檔案騙。
>
> **參考答案**【新觀念，已補教：筆記[第六節〈補充：`extern` 的三個邊界情況〉](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#補充extern-的三個邊界情況2026-10-21-作業第-2-題批改時補教)第 2 點】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 不會成功：編譯通過，連結時報 `multiple definition of 'log_level'`。
> 2. `-fcommon` 只把**沒寫初值**的暫定定義（`int log_level;`）變成 `COM`、讓連結器合併；`int log_level = 1;` 和 `int log_level = 2;` 都有初值，是兩份確定的定義（都在 `.data`），`-fcommon` 不管它們。
>
> **詳細說明**：反過來想就知道為什麼不能合併：兩份初值不一樣（1 和 2），連結器沒辦法決定要用哪一個。common 符號能合併，是因為它們沒有初值，合併成一塊全 0 的記憶體不會有矛盾。`readelf -s worker.o` 可以驗證：加了 `-fcommon`，有初值的 `log_level` 的 `Ndx` 仍然是 `.data` 的編號，不是 `COM`。
 先猜再實驗：把 `logger.c` 的定義改成 `static int log_level = 2;`（`logger.h` 不改），只編譯 `logger.c`。會怎樣？是哪個階段報錯？如果這一步沒有報錯，你覺得連結時會出現什麼錯誤？貼上訊息，說明你的推理。做完改回來。【新觀念，先猜】（推理可以用筆記第五、六節：`static` 讓名字只在本檔案有效、`extern` 宣告「定義在別處」）

**第 1 次（2026-10-21）**
```
我猜測在編譯就會報錯了,因爲h 宣告他是全域變數,但是 logger.c又說他是local

 ~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw2 | on main >1 !8 ?3  gcc -Wall -c logger.c                                                                                         ok | base py | at 16:54:27
logger.c:4:12: error: static declaration of ‘log_level’ follows non-static declaration
    4 | static int log_level = 2;
      |            ^~~~~~~~~
In file included from logger.c:1:
logger.h:4:12: note: previous declaration of ‘log_level’ with type ‘int’
    4 | extern int log_level;

果然編譯就錯了,h跟c的用法 static, extern本身就衝突了
```

> **批改**：**對**。【新觀念，先猜】，而且猜中了：編譯 `logger.c` 時就報 `static declaration of 'log_level' follows non-static declaration`，理由也對——標頭檔先說 `log_level` 是別的檔案看得到的，`logger.c` 又說它只在本檔案有效，同一個編譯單元裡兩個說法矛盾。
> 題目的第二問（「如果這一步沒有報錯，連結時會出現什麼錯誤」）你沒有回答，因為編譯就錯了，這裡補上。另外，**`logger.c` 目前還是 `static int log_level = 2;`，還沒改回來**，做第 3、5 題前記得改回 `int log_level = 2;`（第 5 題要把這些檔案複製到 `hw5/`）。
>
> **參考答案**【新觀念，已補教：筆記[第六節〈補充：`extern` 的三個邊界情況〉](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#補充extern-的三個邊界情況2026-10-21-作業第-2-題批改時補教)第 3 點；推理用[第五節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#檔案層級的-static讓名字變成-local)〈檔案層級的 `static`〉、[第六節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#六extern全域變數的宣告與定義)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 編譯 `logger.c` 時就報錯（`static declaration ... follows non-static declaration`）：引入的 `logger.h` 說 `extern int log_level;`（外部可見），定義又寫 `static`（只在本檔案），同一個編譯單元裡互相矛盾。
> 2. 如果編譯沒抓到（例如 `logger.c` 沒引入 `logger.h`）：`logger.o` 的 `log_level` 是 `LOCAL`，不能配對，`hw2_main.o` 的 `UND log_level` 找不到 `GLOBAL` 定義，連結時報 `undefined reference to 'log_level'`。
>
> **詳細說明**：第二點我在 scratchpad 實測過（`logger.c` 把 `#include "logger.h"` 換成只宣告 `log_msg`），`readelf -s logger.o` 顯示 `LOCAL ... log_level`，連結報 `hw2_main.c:(.text+0xf): undefined reference to 'log_level'`。這跟 1-1、1-4 是同一個教訓：定義的 `.c` 引入自己的標頭檔，矛盾才會在編譯時就被抓到，而不是拖到連結、甚至執行時才出現。

---

## 3. 函式裡的 `static`

在 `hw3/` 寫 `total.c`，裡面有：

- `long running_total(long x)`：回傳**到目前為止所有呼叫傳進來的 `x` 的總和**。不能用全域變數，要用函式裡的 `static` 變數，初值 0。
- `int calls(void)`：回傳**自己被呼叫了第幾次**（第一次回傳 1），同樣用函式裡的 `static`，但這個變數的初值設成 **100**，回傳時再減掉 100（也就是 `static int count = 100;`，每次加 1 後回傳 `count - 100`）。
- `main`：依序呼叫 `running_total(5)`、`running_total(10)`、`running_total(-3)`，各印出回傳值；再呼叫 `calls()` 三次，各印出回傳值。

用 `gcc -Wall -O0 -c total.c` 編譯，再連結執行。

### 3-1. 貼上 `total.c` 與執行結果。【筆記[第五節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#五static兩種完全不同的意思)〈函式裡的 `static`〉】

**第 1 次（YYYY-MM-DD）**
```
```

### 3-2. 先預測：兩個 `static` 變數在 `total.o` 的符號表裡叫什麼名字、`Bind` 是什麼、各在哪個區段？用 `readelf -s`、`readelf -S` 確認，貼上相關的列。兩個變數為什麼在不同的區段？【筆記第五節〈函式裡的 `static`〉】

**第 1 次（YYYY-MM-DD）**
```
```

### 3-3. 用 `objdump -d --no-show-raw-insn total.o` 看 `running_total`，貼上它的本體。`static` 變數用什麼寫法存取？如果把 `static` 拿掉，你預期這幾行會變成什麼寫法、執行結果變成什麼？（只要預測，不必真的改。）【筆記第五節〈函式裡的 `static`〉的組合語言對照；[第 5 課筆記第四節](../../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#四堆疊框pushq-rbp-movq-rsp-rbp-與-leave)】

**第 1 次（YYYY-MM-DD）**
```
```

---

## 4. 讀連結錯誤

下面三個小專案各有一個錯。每一個都在 `hw4/` 底下開一個子資料夾（`a/`、`b/`、`c/`），照打、照指令編譯，然後回答。

### 4a：函式寫在標頭檔裡

`clamp.h`：

```c
#ifndef CLAMP_H
#define CLAMP_H

int clamp(int v, int lo, int hi)
{
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

#endif
```

`volume.c`：

```c
#include "clamp.h"

int set_volume(int v)
{
    return clamp(v, 0, 100);
}
```

`a_main.c`：

```c
#include <stdio.h>
#include "clamp.h"

int set_volume(int v);

int main(void)
{
    printf("%d %d\n", set_volume(150), clamp(-5, 0, 10));
    return 0;
}
```

```
gcc -Wall -c volume.c a_main.c
gcc -o a volume.o a_main.o
```

#### 4a-1. 貼上錯誤訊息。是編譯還是連結階段？原因是什麼？已經有 include guard 了，為什麼沒擋住？【筆記[第四節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#四符號表o-之間靠名字接起來)〈三種錯誤，各發生在哪一步〉、[第三節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#三標頭檔把宣告寫一次讓兩邊都檢查)〈標頭檔放什麼〉、第六節〈忘了 `extern`〉最後一段】

**第 1 次（YYYY-MM-DD）**
```
```

#### 4a-2. 照筆記的原則修好它（要動哪些檔案、各放什麼），貼上修好後的檔案與執行結果。

**第 1 次（YYYY-MM-DD）**
```
```

#### 4a-3. 另一種「修法」：不搬檔案，只在 `clamp.h` 的 `int clamp(...)` 前面加 `static`。先預測能不能連結，再試。如果能，用 `readelf -s` 看兩個 `.o`，說明這樣做的代價是什麼。【筆記第五節〈檔案層級的 `static`〉】

**第 1 次（YYYY-MM-DD）**
```
```

### 4b：名字打錯

`report.h`：

```c
#ifndef REPORT_H
#define REPORT_H

void print_report(void);

#endif
```

`report.c`：

```c
#include <stdio.h>
#include "report.h"

void print_reprot(void)
{
    printf("report ok\n");
}
```

`b_main.c`：

```c
#include "report.h"

int main(void)
{
    print_report();
    return 0;
}
```

```
gcc -Wall -c report.c b_main.c
gcc -o b report.o b_main.o
```

#### 4b-1. 貼上錯誤訊息。是哪個階段？用 `readelf -s`（或 `nm`）看兩個 `.o`，找出證據說明連結器為什麼配不起來。為什麼 `report.c` 明明 `#include` 了 `report.h`，編譯時卻沒有報錯？【筆記第四節；第二節（宣告與定義）】

**第 1 次（YYYY-MM-DD）**
```
```

### 4c：參數個數不對

`area.h`：

```c
#ifndef AREA_H
#define AREA_H

long area(long w, long h);

#endif
```

`area.c`：

```c
#include "area.h"

long area(long w, long h)
{
    return w * h;
}
```

`c_main.c`：

```c
#include <stdio.h>
#include "area.h"

int main(void)
{
    printf("%ld\n", area(3));
    return 0;
}
```

```
gcc -Wall -c area.c c_main.c
```

#### 4c-1. 貼上錯誤訊息。是哪個階段？如果 `c_main.c` 沒有 `#include "area.h"`（也沒有自己的宣告），先猜：會在哪個階段出問題、還是根本不會報錯？為什麼？（可以實際試試看，`-Wall` 的警告也貼上。）【筆記第二節〈實驗：忘了宣告〉、第四節】

**第 1 次（YYYY-MM-DD）**
```
```

---

## 5. 自己寫 `Makefile`

把第 2 題的檔案（`logger.h`、`logger.c`、`worker.h`、`worker.c`、`hw2_main.c`）複製到 `hw5/`，替它寫一個 `Makefile`。要求：

- 用變數 `CC`、`CFLAGS`、`OBJS`；`CFLAGS` 要有 `-Wall -g -O0`。
- 用一條樣式規則把任何 `.c` 編成 `.o`，命令裡用自動變數，不能寫死檔名。
- 用 `gcc` 自動產生標頭檔相依，並在 `Makefile` 裡讀進來。
- 預設目標是連結出 `logdemo`；有 `clean`，而且是假目標。

### 5-1. 貼上 `Makefile` 與第一次 `make` 的輸出、執行結果。逐行說明 `Makefile` 每一行在做什麼。【筆記[第七節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#七make只重編改過的檔案)〈實用的 `Makefile`〉】

**第 1 次（YYYY-MM-DD）**
```
```

### 5-2. 先預測：`touch worker.h` 之後 `make`，哪些 `.o` 會重編、會不會重新連結？再實際跑，貼上輸出，並貼上 `worker.d` 與 `hw2_main.d` 的內容當證據。換成 `touch logger.h` 呢？【筆記第七節〈實驗：第一個 `Makefile`〉的逐點說明、〈`-MMD -MP`〉】

**第 1 次（YYYY-MM-DD）**
```
```

### 5-3. `touch logger.c` 之後，用 `make` 的某個選項**只印出會執行的命令、不真的執行**，貼上命令與輸出。接著真的 `make` 一次。【筆記第七節〈命令列〉的選項表】

**第 1 次（YYYY-MM-DD）**
```
```

### 5-4. 先猜再實驗：什麼檔案都不改，直接執行 `make CFLAGS="-Wall -O2"`。`make` 會用 `-O2` 重編嗎？貼上輸出。為什麼？如果你真的想用新的選項全部重編，有什麼辦法？【新觀念，先猜】（第二問可以用筆記第七節〈命令列〉的選項表）

**第 1 次（YYYY-MM-DD）**
```
```
