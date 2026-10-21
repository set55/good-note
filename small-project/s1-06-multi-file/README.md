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

## 批改結果（2026-10-21～22）

第 1～4 題 2026-10-21 批改，第 5 題 2026-10-22 批改；全部 5 題已批改完。

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
| 3-1 | 對 | 兩個 `static` 變數與初值都對，輸出 5／15／12、1／2／3 實測吻合 |
| 3-2 | 對 | 抓到：`LOCAL`、`total` 在 `.bss`、`count` 在 `.data`，因為初值 0 vs 100。小地方：名字預測漏了 gcc 加的編號（`total.1`、`count.0`） |
| 3-3 | 大致對 | `0x0(%rip)` 是固定位址＋連結時填、拿掉 `static` 改放堆疊、結果 5／10／−3 都對；預測的 `mov 0x0, -0x10(%rbp)` 寫錯：常數要 `$0x0`（不加 `$` 是位址 0），且 `mov` 不能兩邊都是記憶體，應為 `movq $0x0,-0x8(%rbp)` |
| 4a-1 | 不完整 | 連結階段、`multiple definition` 對；原因說成「沒寫 `extern` 所以是定義」是錯的：函式是不是定義看有沒有本體 `{}`，`extern` 是全域變數的規則；include guard 漏了關鍵：只在同一個 `.c` 內有效，兩個 `.c` 分開編譯各貼一次 |
| 4a-2 | 大致對 | `clamp.h` 只留宣告、結果 `100 0` 對；放定義的檔案沒貼（標成 `volume.c` 的是 `a_main.c`）；照原則應該新增 `clamp.c` 並引入 `clamp.h`，而不是寫進 `volume.c` |
| 4a-3 | 對 | 抓到：能連結、兩個 `clamp` 都是 `LOCAL`、代價是每個 `.o` 各一份。用詞：多的只有這個函式的機器碼，不是整個程式 2 倍 |
| 4b-1 | 大致對 | 連結階段、名字打成 `print_reprot`、只宣告不定義是合法的，都對；少了 `b_main.o` 的 `UND print_report` 那一半證據 |
| 4c-1 | 不完整 | 第一問（編譯階段 `too few arguments`）對；第二問猜成編譯錯（隱式宣告在筆記第二節教過）；沒有連結、執行，漏了「連結也過、執行時印出垃圾數字」——跟 1-4 (b)、2-3 一樣，停在沒報錯就下結論 |
| 5-1 | 不完整 | `Makefile` 本身完全符合要求；但沒貼第一次 `make` 的輸出，`-MMD -MP` 沒說明，`-include` 說成「放到 `.o` 裡」（應是把 `.d` 讀進 `Makefile` 當規則，`-` 是不存在不報錯），`.PHONY` 漏了「有同名檔案也照樣執行」 |
| 5-2 | 大致對 | 實際輸出、「引入 `worker.h` 的都重編、再連結」、`logger.h` 全部重編都對；沒貼 `worker.d`／`hw2_main.d` 當證據，預測時漏了 `hw2_main.o` |
| 5-3 | 錯 | 用了 `make logger.o`，那是真的編譯一次；只印不執行是 `make -n`（筆記第七節選項表） |
| 5-4 | 對 | 新觀念，猜中不會重編，`-B` 對；補教：`make` 只比時間、不記選項；命令列 `CFLAGS=` 會整個換掉，連 `-MMD -MP -g` 一起不見 |

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

**第 1 次（2026-10-21）**
````
```
#include <stdio.h>
long running_total(long x)
{
    static long total = 0;
    total += x;
    return total;
}

int calls(void)
{
    static int count = 100;
    count++;
    return count - 100;
}

int main(void)
{
    printf("running_total(5): %ld\n", running_total(5));
    printf("running_total(10): %ld\n", running_total(10));
    printf("running_total(-3): %ld\n", running_total(-3));

    printf("calls(): %d\n", calls());
    printf("calls(): %d\n", calls());
    printf("calls(): %d\n", calls());
    return 0;
}
```

~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw3 | on main >3 !1  ./total                                                                                                          ok | base py | at 21:55:29
running_total(5): 5
running_total(10): 15
running_total(-3): 12
calls(): 1
calls(): 2
calls(): 3
````


> **批改（2026-10-21）**：**對**。兩個函式都用函式裡的 `static`、初值照題目（`0` 與 `100`），`calls` 回傳 `count - 100`；我在本機用你的 `total.c` 重編（`gcc -Wall -O0 -c total.c && gcc total.o -o total`），沒有警告，輸出跟你貼的一模一樣：5、15、12 與 1、2、3。
> 小建議：`total.c` 最後一行沒有換行（`git diff` 顯示 `\ No newline at end of file`），C 標準要求原始檔以換行結尾，gcc 不會抱怨，但 `git` 和一些工具會標出來，存檔時補一個換行就好。
>
> **參考答案**【[第五節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#函式裡的-static不放堆疊值留到下一次)〈函式裡的 `static`〉】
>
> 核心關鍵：
> 1. 兩個變數都寫成函式裡的 `static`（`static long total = 0;`、`static int count = 100;`），初始化只發生一次，值留到下一次呼叫。
> 2. 輸出是 `5`、`15`、`12`（累加）與 `1`、`2`、`3`。
>
> 詳細說明：`running_total` 第三次是 15 + (−3) = 12；`calls` 的 `count` 依序變成 101、102、103，減 100 得 1、2、3。`static int count = 100;` 這一行**不是每次呼叫都執行的指令**，它是「程式載入時這塊記憶體已經是 100」，所以不會每次被設回 100（3-3 的組合語言會看到函式裡完全沒有設初值的指令）。

### 3-2. 先預測：兩個 `static` 變數在 `total.o` 的符號表裡叫什麼名字、`Bind` 是什麼、各在哪個區段？用 `readelf -s`、`readelf -S` 確認，貼上相關的列。兩個變數為什麼在不同的區段？【筆記第五節〈函式裡的 `static`〉】

**第 1 次（2026-10-21）**
```
total 與 count
bind都是 local
total在.bss
count 在.data

3: 0000000000000000     0 SECTION LOCAL  DEFAULT    3 .data
4: 0000000000000000     0 SECTION LOCAL  DEFAULT    4 .bss
5: 0000000000000000     8 OBJECT  LOCAL  DEFAULT    4 total.1
6: 0000000000000000     4 OBJECT  LOCAL  DEFAULT    3 count.0

因爲.bss是初始值爲0的全域變數或static
.data是初始值不爲0的全域變數或static
```


> **批改（2026-10-21）**：**對**。`LOCAL`、`total` 在 `.bss`、`count` 在 `.data`、原因（初值 0 vs 不是 0）都對；用 `SECTION` 那兩列（`3 .data`、`4 .bss`）對出 `Ndx` 的編號也是有效的證據。
> 小地方：名字的預測寫成 `total`、`count`，實際是 gcc 加了編號的 `total.1`、`count.0`——筆記第五節的 `n.0` 講過，編號是為了避免跟別的函式裡同名的 `static` 變數撞名。你貼的結果已經看到了，下次預測時記得這一點。
>
> **參考答案**【[第五節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#函式裡的-static不放堆疊值留到下一次)〈函式裡的 `static`〉】
>
> 核心關鍵：
> 1. 名字是 gcc 加上編號的 `total.1`、`count.0`，`Bind` 都是 `LOCAL`（名字只在這個檔案內，連結器不拿它跟別的檔案配對）。
> 2. `total.1` 在 `.bss`（`Ndx` 4）、`count.0` 在 `.data`（`Ndx` 3）。
> 3. 原因：初值是 0 的放 `.bss`，檔案裡不存內容只記大小（`NOBITS`），載入時配一塊全 0 的記憶體；初值不是 0（100）的放 `.data`，值要存在檔案裡。
>
> 詳細說明：本機 `readelf -s total.o` 與 `readelf -S -W total.o` 的相關列：
> ```text
>      5: 0000000000000000     8 OBJECT  LOCAL  DEFAULT    4 total.1
>      6: 0000000000000000     4 OBJECT  LOCAL  DEFAULT    3 count.0
>   [ 3] .data             PROGBITS        0000000000000000 000154 000004 00  WA  0   0  4
>   [ 4] .bss              NOBITS          0000000000000000 000158 000008 00  WA  0   0  8
> ```
> - `Size` 8 與 4 分別是 `long` 與 `int`；`.data` 的 `Size` 是 4（只有 `count.0` 的 4 byte 內容 `100`），`.bss` 的 `Size` 8 但型別是 `NOBITS`——檔案裡不佔空間。
> - `.data` 與 `.bss` 的 `Flg` 都是 `WA`（可寫、載入時配記憶體），所以兩個變數都可以在執行時被改。
> - 編號 `.0`、`.1` 只保證不撞名，**不是照宣告順序**（這裡後宣告的 `count` 反而是 `.0`），不要拿編號推斷任何事。

### 3-3. 用 `objdump -d --no-show-raw-insn total.o` 看 `running_total`，貼上它的本體。`static` 變數用什麼寫法存取？如果把 `static` 拿掉，你預期這幾行會變成什麼寫法、執行結果變成什麼？（只要預測，不必真的改。）【筆記第五節〈函式裡的 `static`〉的組合語言對照；[第 5 課筆記第四節](../../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#四堆疊框pushq-rbp-movq-rsp-rbp-與-leave)】

**第 1 次（2026-10-21）**
```
0000000000000000 <running_total>:
   0:   endbr64
   4:   push   %rbp
   5:   mov    %rsp,%rbp
   8:   mov    %rdi,-0x8(%rbp)
   c:   mov    0x0(%rip),%rdx        # 13 <running_total+0x13>
  13:   mov    -0x8(%rbp),%rax
  17:   add    %rdx,%rax
  1a:   mov    %rax,0x0(%rip)        # 21 <running_total+0x21>
  21:   mov    0x0(%rip),%rax        # 28 <running_total+0x28>
  28:   pop    %rbp
  29:   ret

mov    0x0(%rip),%rdx , 這裏的0x0是因爲還沒分配好記憶體位址,但簡而言之,ld把符號的位址填好後,這裏就可以計算出來距離rip,下一個指令的位址距離多少了

變成普通的區域變數,應該會在stack分配記憶體給他

可能會變成
mov    %rdi,-0x8(%rbp) 這裏放x
mov    0x0, -0x10(%rbp) 這格放total
mov    -0x8(%rbp),%rax
mov    -0x10(%rbp), %rdx
add    %rdx,%rax
pop    %rbp
ret

running_total(5): 5
running_total(10): 10
running_total(-3): -3
```


> **批改（2026-10-21）**：**大致對**。抓到的重點都對：`static` 變數用 `0x0(%rip)` 存取（相對於 `%rip` 的固定位址），`0x0` 是等連結器填的洞；拿掉 `static` 會放到堆疊用 `-N(%rbp)`；執行結果 5、10、−3 也對。
> 錯在預測的組合語言：`mov    0x0, -0x10(%rbp)` 有兩個錯——(1) 常數要加 `$`，寫成 `$0x0`；不加 `$` 的 `0x0` 在 AT&T 語法裡是「**位址 0 那塊記憶體**」，不是數字 0（[第 4 課第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)的運算元表〈立即數 `$` 開頭〉）；(2) 這樣寫變成記憶體搬到記憶體，`mov` 不允許兩個運算元都是記憶體（[第 4 課第四節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#四mov-與-lea搬資料-vs-算位址)〈`mov`：把來源複製到目的〉），組譯器會直接拒絕。另外大小要明確：目的是記憶體、來源是立即數時，要寫 `movq` 才知道是 8 byte。正確寫法是 `movq   $0x0,-0x8(%rbp)`。
> 偏移量（`-0x10` vs 實際 `-0x8`、`x` 在 `-0x18`）由編譯器決定，預測不到不扣分。
>
> **參考答案**【[第五節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#函式裡的-static不放堆疊值留到下一次)〈函式裡的 `static`〉的 `next_id`／`plain_id` 對照；[第 4 課第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)、[第 5 課第四節](../../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#四堆疊框pushq-rbp-movq-rsp-rbp-與-leave)】
>
> 核心關鍵：
> 1. `static` 變數用 `0x0(%rip)` 存取：指向一塊固定的記憶體（`.bss`），不在堆疊上；`0x0` 是連結時才填的洞；函式裡沒有設初值的指令。
> 2. 拿掉 `static`：變數放在堆疊框，用 `-N(%rbp)` 存取，而且每次呼叫開頭都有一個 `movq $0x0,-N(%rbp)` 把它設回 0。
> 3. 所以執行結果變成 `5`、`10`、`-3`——每次都是 0 + x。
>
> 詳細說明：用 `objdump -dr` 加 `-r` 看得到洞上標的是誰（`#` 後面是說明）：
> ```text
>    c:   mov    0x0(%rip),%rdx        # 讀 total：rdx = total
>             f: R_X86_64_PC32  .bss-0x4
>   13:   mov    -0x8(%rbp),%rax       # rax = x（x 從 %rdi 存到堆疊的那格）
>   17:   add    %rdx,%rax             # rax = x + total
>   1a:   mov    %rax,0x0(%rip)        # 寫回 total
>             1d: R_X86_64_PC32  .bss-0x4
>   21:   mov    0x0(%rip),%rax        # 再讀一次 total 當回傳值（-O0 不省）
>             24: R_X86_64_PC32  .bss-0x4
> ```
> 洞上標的是 `.bss`（區段符號）而不是 `total.1`：`LOCAL` 的符號不會被別的檔案用到，組譯器直接寫成「`.bss` 開頭加偏移」，跟筆記第一節 `"%d\n"` 字串用 `.rodata-0x4` 是同一個做法（這是新觀念，不在扣分範圍）。`-0x4` 是因為 `%rip` 指的是**下一個指令**的位址，洞本身離下一個指令還有 4 byte。
>
> 我實際把 `static` 拿掉重編（`sed 's/static long total/long total/' total.c > nost.c`，`gcc -O0 -c nost.c`，`objdump -d --no-show-raw-insn nost.o`），`running_total` 變成：
> ```text
>    8:   mov    %rdi,-0x18(%rbp)      # x 存到堆疊
>    c:   movq   $0x0,-0x8(%rbp)       # total = 0：每次呼叫都重設
>   14:   mov    -0x18(%rbp),%rax      # rax = x
>   18:   add    %rax,-0x8(%rbp)       # total += x（記憶體當目的、暫存器當來源，合法）
>   1c:   mov    -0x8(%rbp),%rax       # 回傳 total
> ```
> 執行結果 `5`、`10`、`-3`（`calls` 沒動，仍是 1、2、3）。注意 `add %rax,-0x8(%rbp)` 只有一邊是記憶體，所以可以；你寫的 `mov 0x0, -0x10(%rbp)` 兩邊都是記憶體，不行。

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

**第 1 次（2026-10-21）**
```
/usr/bin/ld: a_main.o: in function `clamp':
a_main.c:(.text+0x0): multiple definition of `clamp'; volume.o:volume.c:(.text+0x0): first defined here
collect2: error: ld returned 1 exit status

是連接階段

clamp重複定義了
因爲clamp.h沒有宣告 extern在clamp, 所以clamp已經是默認爲定義了,而每個有這個.h的.c都會重複定義他, include guard,只能
幫助確定有沒有include這個標頭當,他無法不然.c重複定義
```


> **批改（2026-10-21）**：**不完整**。錯誤訊息、連結階段、「每個引入 `.h` 的 `.c` 都有一份定義」都對。兩個問題：
> 1. 原因講錯：`clamp` 是定義，是因為它**有函式本體 `{ ... }`**，跟有沒有寫 `extern` 無關。函式宣告本來就預設 `extern`（[第八節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#八本課用到的工具選項與完整清單在哪)〈C 語言：本課的關鍵字〉），`extern int clamp(...) { ... }` 照樣是定義、照樣 `multiple definition`。「沒寫 `extern` 就變成定義」是**全域變數**的規則（`long hits;` 是暫定定義，[第六節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#六extern全域變數的宣告與定義)），不要套到函式上。
> 2. include guard 為什麼擋不住，只寫了「擋不住」，沒說原因：guard 只在**同一個 `.c`（編譯單元）裡**有效。`volume.c` 和 `a_main.c` 是分開編譯的，各自從頭開始，`CLAMP_H` 在兩次編譯裡都還沒定義，所以兩個檔案都各貼了一份。
>
> **參考答案**【[第四節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#三種錯誤各發生在哪一步)〈三種錯誤，各發生在哪一步〉、[第三節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#三標頭檔把宣告寫一次讓兩邊都檢查)〈標頭檔放什麼〉、[第六節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#忘了-extern每個-c-都有一份定義)〈忘了 `extern`〉最後一段】
>
> 核心關鍵：
> 1. 連結階段（訊息開頭是 `/usr/bin/ld:`）的 `multiple definition of 'clamp'`。
> 2. 原因：標頭檔裡放了函式的**定義**（有本體），被貼進 `volume.c` 和 `a_main.c`，兩個 `.o` 各有一個 `GLOBAL` 的 `clamp`，連結器不知道用哪一個。
> 3. include guard 只防同一個 `.c` 裡貼兩次；兩個 `.c` 分開編譯，各貼一次，guard 管不到。
>
> 詳細說明：我照題目重做，`readelf -s volume.o a_main.o | grep -E 'File|clamp'` 看到兩份定義：
> ```text
> File: volume.o
>      3: 0000000000000000    48 FUNC    GLOBAL DEFAULT    1 clamp
> File: a_main.o
>      4: 0000000000000000    48 FUNC    GLOBAL DEFAULT    1 clamp
> ```
> 兩列都是 `GLOBAL`，`Ndx` 是 1（`.text`，有內容），也就是兩個 `.o` 都說「我提供 `clamp`」。編譯時沒有報錯，因為每個 `.c` 裡都只有一份定義；問題要等連結器把兩個 `.o` 放在一起時才看得到。

**自我練習**（不批改）

```
連接階段錯誤,原因是因爲重複定義clamp

y因爲他們兩個是分開編譯的,兩個都各自引入標頭檔案,所以都定義到了
```

#### 4a-2. 照筆記的原則修好它（要動哪些檔案、各放什麼），貼上修好後的檔案與執行結果。

**第 1 次（2026-10-21）**
````
我改動了clamp.h, 不再將完整的定義放在標頭,並且用上了extern
clamp的定義就寫在volume.c

```volume.c
#include <stdio.h>
#include "clamp.h"

int set_volume(int v);

int main(void)
{
    printf("%d %d\n", set_volume(150), clamp(-5, 0, 10));
    return 0;
}
```

```clamp.h
#ifndef CLAMP_H
#define CLAMP_H

extern int clamp(int v, int lo, int hi);

#endif
```

./a                                                                                                      127 err | base py | at 22:46:21
100 0
````


> **批改（2026-10-21）**：**大致對**。`clamp.h` 只留宣告是對的，`100 0` 也對。缺的地方：
> 1. 你標成 `volume.c` 貼上來的那一段其實是 `a_main.c` 的內容（有 `main`、沒有 `clamp` 的定義），所以「`clamp` 的定義放在哪、長什麼樣」沒有貼出來。你資料夾裡的檔案後來已經改成 4a-3 的版本，我也看不到你修好的版本。
> 2. 定義放在 `volume.c` 可以連結，但不是筆記的做法：`clamp` 是一個通用工具函式，照「一個 `.h` 配一個 `.c`」的原則，應該**新增 `clamp.c`**，在裡面 `#include "clamp.h"` 並寫定義，連結時多加 `clamp.o`。放在 `volume.c` 的話，之後別的程式只想用 `clamp`，也得把整個 `volume.o` 連進來。
>
> `extern int clamp(...);` 的 `extern` 沒有錯，只是多寫了，函式宣告本來就預設 `extern`。
>
> 另外，題目要求每個小專案各開一個子資料夾（`a/`、`b/`、`c/`）。你全部放在 `hw4/` 裡，結果 4a-3 把 4a-2 修好的檔案蓋掉了，4c 的 `c_main.c` 也只剩註解掉標頭檔的版本。下次分開放，每一題的檔案才會留著。
>
> **參考答案**【[第三節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#三標頭檔把宣告寫一次讓兩邊都檢查)〈標頭檔放什麼〉、[第六節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#忘了-extern每個-c-都有一份定義)〈忘了 `extern`〉最後一段】
>
> 核心關鍵：
> 1. `clamp.h` 只放宣告 `int clamp(int v, int lo, int hi);`（保留 include guard）。
> 2. 定義放在**一個** `.c` 裡（慣例是新增 `clamp.c`），這個 `.c` 也要 `#include "clamp.h"`，讓編譯器檢查宣告和定義一致。
> 3. 三個 `.o` 一起連結，結果是 `100 0`。
>
> 詳細說明：修好後的檔案：
>
> `clamp.h`：
> ```c
> #ifndef CLAMP_H
> #define CLAMP_H
>
> int clamp(int v, int lo, int hi);
>
> #endif
> ```
> `clamp.c`：
> ```c
> #include "clamp.h"
>
> int clamp(int v, int lo, int hi)
> {
>     if (v < lo)
>         return lo;
>     if (v > hi)
>         return hi;
>     return v;
> }
> ```
> `volume.c`、`a_main.c` 不用改。
> ```text
> gcc -Wall -c clamp.c volume.c a_main.c
> gcc -o a clamp.o volume.o a_main.o
> ./a
> 100 0
> ```
> `set_volume(150)` 被夾到上限 100，`clamp(-5, 0, 10)` 被夾到下限 0。

#### 4a-3. 另一種「修法」：不搬檔案，只在 `clamp.h` 的 `int clamp(...)` 前面加 `static`。先預測能不能連結，再試。如果能，用 `readelf -s` 看兩個 `.o`，說明這樣做的代價是什麼。【筆記第五節〈檔案層級的 `static`〉】

**第 1 次（2026-10-21）**
```
能夠連接,也可以測試

~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw4 | on main >4 !4 ?1  readelf -s a_main.o                                                                                           ok | base py | at 22:49:35

Symbol table '.symtab' contains 8 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS a_main.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000    48 FUNC    LOCAL  DEFAULT    1 clamp
     4: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 .rodata
     5: 0000000000000030    80 FUNC    GLOBAL DEFAULT    1 main
     6: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND set_volume
     7: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND printf

 ~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw4 | on main >4 !4 ?1  readelf -s volume.o                                                                                           ok | base py | at 22:49:47

Symbol table '.symtab' contains 5 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS volume.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000    48 FUNC    LOCAL  DEFAULT    1 clamp
     4: 0000000000000030    37 FUNC    GLOBAL DEFAULT    1 set_volume

兩個都變成local自己可以用,但這樣就變成 兩個.o要定義兩次,但因爲是local所以連接時不會有問題,只是佔用記憶體的容量也變2倍了
```


> **批改（2026-10-21）**：**對**。預測能連結是對的；`readelf -s` 兩個 `.o` 都貼了，也看出兩個 `clamp` 都是 `LOCAL`；代價也講到了：每個 `.o` 各有一份。
> 用詞修正：不是「記憶體容量變 2 倍」，只有 `clamp` 這個函式的機器碼多了一份（這裡是 48 byte），程式其他部分不會跟著變大。引入的 `.c` 越多，就多越多份。
>
> **參考答案**【[第五節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#檔案層級的-static讓名字變成-local)〈檔案層級的 `static`〉】
>
> 核心關鍵：
> 1. 能連結：`static` 讓兩個 `clamp` 都變成 `LOCAL`，連結器不拿 `LOCAL` 的名字去配對，所以不會撞名。
> 2. 證據：`volume.o` 和 `a_main.o` 各有一列 `48 FUNC LOCAL ... 1 clamp`，兩份都是定義（`Ndx` 1）。
> 3. 代價：每個引入 `clamp.h` 的 `.c` 都會複製一份 `clamp` 的機器碼，執行檔裡有好幾份同樣的函式。
>
> 詳細說明（後兩點是新觀念，不在扣分範圍）：
> - 我實測執行，結果一樣是 `100 0`。
> - 引入了但沒用到的 `.c` 會收到警告：只寫 `#include "clamp.h"` 和一個不相干函式的 `unused.c`，`gcc -Wall -c unused.c` 會印
>   `clamp.h:4:12: warning: 'clamp' defined but not used [-Wunused-function]`。
> - 兩份是**不同的函式**，位址也不一樣。如果函式裡有 `static` 變數，每個 `.c` 也會各有一份，彼此不共用。
> - 實務上真的想把小函式放在標頭檔時，寫法是 `static inline`：`inline` 讓編譯器傾向直接展開，沒用到也不警告。kernel 的標頭檔裡很多這種寫法（之後的課才會細講）。

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

**第 1 次（2026-10-21）**
```
/usr/bin/ld: b_main.o: in function `main':
b_main.c:(.text+0x9): undefined reference to `print_report'
collect2: error: ld returned 1 exit status

連接的時候出了錯

Symbol table '.symtab' contains 6 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS report.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 .rodata
     4: 0000000000000000    26 FUNC    GLOBAL DEFAULT    1 print_reprot
     5: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND puts

可以看到print_report 寫成print_reprot 與b_main自然連接不起來

因爲c 認爲print_reprot 與 print_report是沒有關係的定義與宣告,h的宣告,在.c沒有定義也不會因此有錯
```


> **批改（2026-10-21）**：**大致對**。連結階段、`undefined reference`、從 `report.o` 看出定義的名字是 `print_reprot`、「只宣告不定義不會錯」都對。缺的地方：題目要求看**兩個** `.o`，你只貼了 `report.o`。另一半證據是 `b_main.o` 裡那一列 `UND print_report`（要的名字），兩邊並排，才看得出「要的」和「給的」差了兩個字母。
>
> **參考答案**【[第四節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#三種錯誤各發生在哪一步)〈三種錯誤，各發生在哪一步〉、[第二節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#二宣告與定義沒有宣告編譯器只能猜)】
>
> 核心關鍵：
> 1. 連結階段的 `undefined reference to 'print_report'`。
> 2. 證據：`b_main.o` 有 `NOTYPE GLOBAL ... UND print_report`（需要），`report.o` 只有 `FUNC GLOBAL ... 1 print_reprot`（提供）；連結器只比對名字，兩個名字不同，所以沒有人提供 `print_report`。
> 3. 編譯時不會錯：只有宣告、沒有定義是合法的，編譯器以為定義在別的檔案。它一次只看一個 `.c`，不會檢查每個宣告都有對應的定義；`print_reprot` 對它來說只是另一個函式。
>
> 詳細說明：本機的兩列：
> ```text
> b_main.o:  4: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND print_report
> report.o:  4: 0000000000000000    26 FUNC    GLOBAL DEFAULT    1 print_reprot
> ```
> 引入自己的標頭檔只能抓到「同一個名字、型別不同」（1-4 的 `conflicting types`），抓不到打錯名字，因為打錯的名字就是另一個新的函式。`-Wall` 也不警告，因為 `print_reprot` 沒有 `static`，編譯器無法知道別的檔案不會呼叫它。
> 另外，你貼的符號表裡是 `UND puts` 而不是 `printf`：`printf("report ok\n")` 的格式字串裡沒有 `%`、又以 `\n` 結尾，gcc 會把它換成比較快的 `puts("report ok")`（新觀念，看到不要以為貼錯了）。

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

**第 1 次（2026-10-21）**
```
c_main.c: In function ‘main’:
c_main.c:6:21: error: too few arguments to function ‘area’
    6 |     printf("%ld\n", area(3));
      |                     ^~~~
In file included from c_main.c:2:
area.h:4:6: note: declared here
    4 | long area(long w, long h);
      |      ^~~~

編譯階段就錯了,少了一個參數

一樣編譯階段, 應該是undefined function gcc找不到這個函式

c_main.c: In function ‘main’:
c_main.c:6:21: warning: implicit declaration of function ‘area’ [-Wimplicit-function-declaration]
    6 |     printf("%ld\n", area(3));
      |                     ^~~~
c_main.c:6:15: warning: format ‘%ld’ expects argument of type ‘long int’, but argument 2 has type ‘int’ [-Wformat=]
    6 |     printf("%ld\n", area(3));
      |             ~~^     ~~~~~~~
      |               |     |
      |               |     int
      |               long int
      |             %d

實際結果,沒有報錯只有警告

看起來他有隱含宣告的意思,而且默認的返回值還是int
```


> **批改（2026-10-21）**：**不完整**。第一問對：編譯階段的 `too few arguments to function 'area'`，原因是少一個參數。第二問：
> 1. 猜「編譯階段就錯、找不到函式」是錯的。這不是新觀念：[第二節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#實驗忘了宣告square100000-印出-1410065408)〈實驗：忘了宣告〉的 `square` 就是同一件事，沒有宣告時編譯器會**隱式宣告**成回傳 `int`，照樣編得過，只有警告。
> 2. 你實際試出「沒報錯、只有警告、預設回傳 `int`」，這個觀察是對的。但只做到 `-c`，沒有連結、沒有執行，所以漏了最重要的部分：**連結也過、執行時印出錯的數字**。這跟 1-4 (b)、2-3 是同一種問題：「沒報錯」不代表結果對，要跑到最後一步才知道。
>
> **參考答案**【[第二節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#實驗忘了宣告square100000-印出-1410065408)〈實驗：忘了宣告〉、[第四節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#三種錯誤各發生在哪一步)〈三種錯誤，各發生在哪一步〉】
>
> 核心關鍵：
> 1. 有引入 `area.h`：編譯階段就報 `too few arguments`，因為宣告讓編譯器知道 `area` 要兩個參數。
> 2. 沒有引入：編譯只有警告（`implicit declaration`，隱式宣告成回傳 `int`、參數不檢查），編譯得過。
> 3. 連結也會過：連結器只比對名字 `area`，不比對參數個數和型別。
> 4. 執行時結果是錯的：`area` 照它的定義從 `%rsi` 讀 `h`，但呼叫的那邊沒放 `h`，讀到的是垃圾；回傳值也被當成 `int`，只取低 32 位元。沒有任何一個階段抓得到。
>
> 詳細說明：我把你的 `c_main.c`（`#include "area.h"` 註解掉的版本）實際連結、執行：
> ```text
> gcc -Wall -c area.c c_main.c      # 跟你一樣的兩個警告
> gcc -o c area.o c_main.o          # 連結成功，沒有任何訊息
> ./c
> 2899668792
> ./c
> 3459328120
> ```
> 每次執行的數字都不一樣，而且都不是 `3 × 某個數` 的合理面積。`objdump -d --no-show-raw-insn c_main.o` 看呼叫的那邊（`#` 後面是說明）：
> ```text
>    8:   mov    $0x3,%edi      # 第一個參數 w = 3
>    d:   mov    $0x0,%eax      # 沒有原型時，當成可變參數函式呼叫：%al = 0（第 5 課）
>   12:   call   17 <main+0x17> # 呼叫 area；%rsi 完全沒設，裡面是之前留下的值
>   17:   mov    %eax,%esi      # 只拿回傳值的低 32 位元（當成 int），放進 printf 的第二個參數
> ```
> `area` 那邊照定義算 `%rdi × %rsi`，`%rsi` 是呼叫之前剩下的值（每次執行不同，因為位址隨機化），所以結果每次都變。`mov %eax,%esi` 寫 32 位元暫存器會把高 32 位元清成 0（第 4 課），`printf` 的 `%ld` 讀整個 `%rsi`，所以印出來的一定是 0～4294967295 之間的數。
> 第二個警告 `format '%ld' expects ... 'int'` 也是同一個原因：編譯器以為 `area` 回傳 `int`。這就是筆記一直強調的：**一定要引入標頭檔**，讓錯誤在編譯時就被抓到。

---

## 5. 自己寫 `Makefile`

把第 2 題的檔案（`logger.h`、`logger.c`、`worker.h`、`worker.c`、`hw2_main.c`）複製到 `hw5/`，替它寫一個 `Makefile`。要求：

- 用變數 `CC`、`CFLAGS`、`OBJS`；`CFLAGS` 要有 `-Wall -g -O0`。
- 用一條樣式規則把任何 `.c` 編成 `.o`，命令裡用自動變數，不能寫死檔名。
- 用 `gcc` 自動產生標頭檔相依，並在 `Makefile` 裡讀進來。
- 預設目標是連結出 `logdemo`；有 `clean`，而且是假目標。

### 5-1. 貼上 `Makefile` 與第一次 `make` 的輸出、執行結果。逐行說明 `Makefile` 每一行在做什麼。【筆記[第七節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#七make只重編改過的檔案)〈實用的 `Makefile`〉】

**第 1 次（2026-10-22）**
````
```
CC = gcc   # 決定編譯器
CFLAGS = -Wall -g -O0 -MMD -MP # 編譯器命令用的flag
OBJS = logger.o worker.o hw2_main.o # 要變出來的.o

logdemo: $(OBJS) # 默認的make指令, 直接就把執行檔變出來 先決條件是objs
	$(CC) $(CFLAGS) -o logdemo $(OBJS) # 編譯命令

%.o: %.c 決定要編那個.o
	$(CC) $(CFLAGS) -c $< -o $@ # $<:代表第一個先決條件, $@是目標

-include $(OBJS:.o=.d) #要把產生的.d全部放到make .o裏面

clean:  #清除的命令
	rm -f $(OBJS) logdemo *.d

.PHONY: clean, #每次都執行
```

~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw5 | on main >4 !4 ?2  ./logdemo                                                                                                     ok | base py | at 00:39:36
[1] Work started
[1] Work started
[3] Work detail
````


> **批改（2026-10-22）**：**不完整**。`Makefile` 本身是對的：我讀了你 `hw5/` 裡的檔案，四個要求都做到了，複製到 scratchpad 跑 `make` 也正常。扣分在說明和缺少的輸出：
> 1. **沒貼第一次 `make` 的輸出**，只貼了 `./logdemo` 的結果。題目要的是 `make` 印出來的三個編譯命令和一個連結命令。
> 2. **`-MMD -MP` 沒說明**，只寫了「編譯器命令用的 flag」。這兩個是這個 `Makefile` 能自動追蹤標頭檔的關鍵。
> 3. **`-include` 說錯**：它不是「把 `.d` 放到 `.o` 裡面」，而是把 `.d` 的內容**讀進 `Makefile`**，當成額外的規則（例如 `worker.o: worker.c worker.h logger.h`）；前面的 `-` 是「檔案不存在也不要報錯」。
> 4. **`.PHONY` 不完整**：重點不是「每次都執行」，而是 `clean` 不是檔案，宣告成假目標後，就算目錄裡剛好有一個叫 `clean` 的檔案，`make clean` 也照樣執行。
> 5. 樣式規則只寫了「決定要編哪個 `.o`」，要講出 `%` 的意思：**任何** `X.o` 都可以從 `X.c` 用這條命令產生。
>
> 另外兩個要小心的地方（你實際的檔案沒有這兩個問題，只出現在貼上來加了說明的版本裡）：
> - `%.o: %.c 決定要編那個.o` 少了 `#`。在 `Makefile` 裡，冒號後面的字全部都是先決條件，`make` 會去找一個叫 `決定要編那個.o` 的檔案。我實測同樣的寫法（`all: x 說明`），結果是 `No rule to make target 'x', needed by 'all'.  Stop.`。
> - `.PHONY: clean,` 多了逗號。`make` 的清單用空白分隔，這樣宣告的假目標叫 `clean,`，`clean` 本身沒有被宣告。我實測：先 `touch clean` 再 `make clean`，得到 `make: 'clean' is up to date.`，什麼都沒刪。
>
> **參考答案**【[第七節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#實用的-makefile變數樣式規則自動產生相依)〈實用的 `Makefile`〉】
>
> 核心關鍵：
> 1. `Makefile` 符合四個要求：有 `CC`／`CFLAGS`／`OBJS`、`%.o: %.c` 的命令用 `$<`／`$@`、`-MMD -MP` 加上 `-include $(OBJS:.o=.d)`、第一條規則是 `logdemo`，而且 `.PHONY: clean`。
> 2. 第一次 `make`：三個 `.c` 各編一次，再連結一次；執行結果是 `[1] Work started`、`[1] Work started`、`[3] Work detail`。
> 3. 說得出三個關鍵行在做什麼：樣式規則（任何 `X.o` 從 `X.c` 產生）、`-MMD -MP`（gcc 編譯時順便寫出 `.d` 相依檔）、`-include`（把 `.d` 讀進來當規則，不存在也不報錯）。
>
> 詳細說明：你的 `Makefile`（`hw5/Makefile` 的實際內容，命令前面是 TAB）：
> ```make
> CC = gcc
> CFLAGS = -Wall -g -O0 -MMD -MP
> OBJS = logger.o worker.o hw2_main.o
>
> logdemo: $(OBJS)
> 	$(CC) $(CFLAGS) -o logdemo $(OBJS)
>
> %.o: %.c
> 	$(CC) $(CFLAGS) -c $< -o $@
>
> -include $(OBJS:.o=.d)
>
> clean:
> 	rm -f $(OBJS) logdemo *.d
>
> .PHONY: clean
> ```
> 逐行說明：
> - `CC = gcc`：C 編譯器的變數；之後用 `$(CC)` 取用。
> - `CFLAGS = -Wall -g -O0 -MMD -MP`：編譯選項。`-Wall` 開警告、`-g` 留除錯資訊、`-O0` 不最佳化；`-MMD` 讓 gcc 編譯時另外寫一個 `X.d`，列出 `X.o` 依賴哪些**使用者的**標頭檔；`-MP` 替每個標頭檔多寫一條空規則（`.d` 裡的 `worker.h:`），標頭檔刪掉或改名時 `make` 才不會報 `No rule to make target`。
> - `OBJS = ...`：要連結的 `.o` 清單。
> - `logdemo: $(OBJS)`：第一條規則，所以是預設目標（`make` 不加目標就做它）；先決條件是三個 `.o`，任何一個比 `logdemo` 新就重新連結。
> - 下一行的命令：連結。可以寫成 `$(CC) -o $@ $^`，跟筆記一樣不寫死檔名。連結時帶上 `$(CFLAGS)` 不會出錯，但 `-MMD -MP -O0` 在連結時沒有作用。
> - `%.o: %.c`：樣式規則，`%` 對到任何字串；`make` 要做 `worker.o` 時，`%` 就是 `worker`，先決條件是 `worker.c`。
> - 命令 `$(CC) $(CFLAGS) -c $< -o $@`：`$<` 是第一個先決條件（`worker.c`），`$@` 是目標（`worker.o`）。
> - `-include $(OBJS:.o=.d)`：`$(OBJS:.o=.d)` 把每個 `.o` 換成 `.d`，得到 `logger.d worker.d hw2_main.d`；`include` 把它們的內容讀進來，補上「`.o` 也依賴標頭檔」這些先決條件；`-` 讓第一次 `make`（還沒有 `.d`）不報錯。
> - `clean:` 和 `rm -f ...`：清掉產物；`-f` 讓檔案不存在時也不報錯。
> - `.PHONY: clean`：宣告 `clean` 是假目標，不管有沒有叫 `clean` 的檔案都執行。
>
> 我在 scratchpad 用你的檔案跑出的第一次 `make`：
> ```text
> gcc -Wall -g -O0 -MMD -MP -c logger.c -o logger.o
> gcc -Wall -g -O0 -MMD -MP -c worker.c -o worker.o
> gcc -Wall -g -O0 -MMD -MP -c hw2_main.c -o hw2_main.o
> gcc -Wall -g -O0 -MMD -MP -o logdemo logger.o worker.o hw2_main.o
> ```
> 執行結果裡 `[1] Work started` 出現兩次，`[3] Work detail` 只出現一次：第一次 `do_work()` 時 `log_level` 是 2，等級 3 的訊息被擋掉；`main` 把 `log_level` 改成 3 之後，第二次兩則都印。

### 5-2. 先預測：`touch worker.h` 之後 `make`，哪些 `.o` 會重編、會不會重新連結？再實際跑，貼上輸出，並貼上 `worker.d` 與 `hw2_main.d` 的內容當證據。換成 `touch logger.h` 呢？【筆記第七節〈實驗：第一個 `Makefile`〉的逐點說明、〈`-MMD -MP`〉】

**第 1 次（2026-10-22）**
```
應該是worker.o會重編,也會重新連接

~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw5 | on main >4 !4 ?2  make                                                                                                          ok | base py | at 00:46:00
gcc -Wall -g -O0 -MMD -MP -c worker.c -o worker.o
gcc -Wall -g -O0 -MMD -MP -c hw2_main.c -o hw2_main.o
gcc -Wall -g -O0 -MMD -MP -o logdemo logger.o worker.o hw2_main.o

觀察到實際 worker.o, hw2_main.o, logdemo全都重編了,看來只要有include worker.h都會重編, logdemo因爲是hw2_main跟logger連接的產物,所以重編也正常

換成touch logger.h 那全都會重編,因爲全都有引用
```


> **批改（2026-10-22）**：**大致對**。實際跑的輸出對；看到 `hw2_main.o` 也重編之後，解釋「有引入 `worker.h` 的都重編、連結產物跟著重做」也對；`touch logger.h` 全部重編的推論也對（三個 `.c` 都引入了 `logger.h`）。缺的地方：
> 1. **沒貼 `worker.d` 和 `hw2_main.d`**，題目要你拿它們當證據。「哪些 `.o` 會重編」不是看你記得誰 `#include` 了誰，而是看 `.d` 裡寫了什麼，因為 `make` 只知道 `.d` 告訴它的事。
> 2. 預測漏了 `hw2_main.o`。你的 `hw2_main.c` 第一行就是 `#include "worker.h"`。預測前先看一下每個 `.c` 引入了哪些標頭檔。
>
> **參考答案**【[第七節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#實驗第一個-makefile)〈實驗：第一個 `Makefile`〉的逐點說明、[第七節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#實用的-makefile變數樣式規則自動產生相依)〈實用的 `Makefile`〉〈`-MMD -MP`〉】
>
> 核心關鍵：
> 1. `touch worker.h`：`worker.o` 和 `hw2_main.o` 重編（兩個 `.c` 都引入了 `worker.h`），`logger.o` 不動；有 `.o` 變新，所以 `logdemo` 重新連結。
> 2. 證據是 `.d`：`worker.d` 和 `hw2_main.d` 的規則裡都有 `worker.h`，`logger.d` 沒有。
> 3. `touch logger.h`：三個 `.d` 都有 `logger.h`，三個 `.o` 全部重編，再重新連結。
>
> 詳細說明：你 `hw5/` 裡的 `.d`：
> ```text
> $ cat worker.d
> worker.o: worker.c worker.h logger.h
> worker.h:
> logger.h:
> $ cat hw2_main.d
> hw2_main.o: hw2_main.c worker.h logger.h
> worker.h:
> logger.h:
> $ cat logger.d
> logger.o: logger.c logger.h
> logger.h:
> ```
> 每個 `.d` 的第一行就是「這個 `.o` 依賴哪些檔案」，`-include` 把它併進 `%.o: %.c` 那條規則。`touch worker.h` 讓 `worker.h` 比 `worker.o`、`hw2_main.o` 新，這兩個就重編。我實測 `touch logger.h` 後 `make`：
> ```text
> gcc -Wall -g -O0 -MMD -MP -c logger.c -o logger.o
> gcc -Wall -g -O0 -MMD -MP -c worker.c -o worker.o
> gcc -Wall -g -O0 -MMD -MP -c hw2_main.c -o hw2_main.o
> gcc -Wall -g -O0 -MMD -MP -o logdemo logger.o worker.o hw2_main.o
> ```

### 5-3. `touch logger.c` 之後，用 `make` 的某個選項**只印出會執行的命令、不真的執行**，貼上命令與輸出。接著真的 `make` 一次。【筆記第七節〈命令列〉的選項表】

**第 1 次（2026-10-22）**
```
 ~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw5 | on main >4 !4 ?2  make logger.o                                                                                                 ok | base py | at 00:57:56
gcc -Wall -g -O0 -MMD -MP -c logger.c -o logger.o

~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw5 | on main >4 !4 ?2  make                                                                                                         INT | base py | at 00:58:39
gcc -Wall -g -O0 -MMD -MP -o logdemo logger.o worker.o hw2_main.o
```


> **批改（2026-10-22）**：**錯**。`make logger.o` 不是「只印不執行」，它是指定目標 `logger.o`，**真的把 `logger.c` 編譯了一次**。所以接下來的 `make` 只剩連結要做，`logger.o` 已經是新的了。題目要的是 `-n` 選項（也可以寫 `--just-print`、`--dry-run`、`--recon`），筆記第七節〈命令列〉的選項表有這一列：「只印出會執行的命令，不真的執行」。
>
> **參考答案**【[第七節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#命令列make-選項-目標-)〈命令列〉的選項表】
>
> 核心關鍵：
> 1. 用 `make -n`（`--just-print`／`--dry-run`／`--recon`）：只印出會執行的命令，不真的執行，所以檔案不會被改到。
> 2. `touch logger.c` 之後，`-n` 印出兩行：重編 `logger.o`、重新連結 `logdemo`；`worker.o`、`hw2_main.o` 不動。
> 3. 之後真的 `make`，執行的就是剛剛印出來的那兩行（因為 `-n` 什麼都沒做，`logger.c` 仍然比 `logger.o` 新）。
>
> 詳細說明：我實測：
> ```text
> $ touch logger.c
> $ make -n
> gcc -Wall -g -O0 -MMD -MP -c logger.c -o logger.o
> gcc -Wall -g -O0 -MMD -MP -o logdemo logger.o worker.o hw2_main.o
> $ make
> gcc -Wall -g -O0 -MMD -MP -c logger.c -o logger.o
> gcc -Wall -g -O0 -MMD -MP -o logdemo logger.o worker.o hw2_main.o
> ```
> 兩次印出來的一模一樣，這就是 `-n` 的用處：改 `Makefile` 之後，先看一下它打算做什麼，確定沒問題再真的跑。
> 你的寫法 `make <目標>` 也有用（例如只想重編一個 `.o`），但它是「做這個目標」，不是「預覽」。

### 5-4. 先猜再實驗：什麼檔案都不改，直接執行 `make CFLAGS="-Wall -O2"`。`make` 會用 `-O2` 重編嗎？貼上輸出。為什麼？如果你真的想用新的選項全部重編，有什麼辦法？【新觀念，先猜】（第二問可以用筆記第七節〈命令列〉的選項表）

**第 1 次（2026-10-22）**
```
不行
 ~/w/Proj/g/good-note/s/s1-06-multi-file/hw/hw5 | on main >4 !4 ?2  make CFLAGS="-Wall -O2"                                                                                       ok | base py | at 00:58:57
make: 'logdemo' is up to date.

看輸出似乎是logdemo已經是最新的了

用 -B
```


> **批改（2026-10-22）**：**對**。新觀念先猜：「不會重編」猜對了，輸出也貼了；第二問的 `-B` 對。「為什麼」只寫了「`logdemo` 已經是最新的」，那是 `make` 的說法；背後的原因補在下面（新觀念，不扣分）。
>
> **參考答案**【新觀念；第二問用[第七節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#命令列make-選項-目標-)〈命令列〉的選項表】
>
> 核心關鍵：
> 1. 不會重編，輸出 `make: 'logdemo' is up to date.`。
> 2. 原因：`make` 只比**檔案的修改時間**，不記得上次是用什麼選項編的。所有 `.o` 都比 `.c`／`.h` 新、`logdemo` 比 `.o` 新，所以什麼都不做；`CFLAGS` 變了，它不知道。
> 3. 要全部重編：`make -B CFLAGS="-Wall -O2"`（`-B` 不管時間，所有目標都重做），或者先 `make clean` 再 `make CFLAGS="-Wall -O2"`。
>
> 詳細說明：我實測 `make -B CFLAGS="-Wall -O2"`：
> ```text
> gcc -Wall -O2 -c logger.c -o logger.o
> gcc -Wall -O2 -c worker.c -o worker.o
> gcc -Wall -O2 -c hw2_main.c -o hw2_main.o
> gcc -Wall -O2 -o logdemo logger.o worker.o hw2_main.o
> ```
> 注意命令裡**不見了 `-g -O0 -MMD -MP`**：命令列的 `CFLAGS=...` 是把整個變數換掉，不是在後面追加（筆記第七節〈1. 變數〉：「命令列優先」）。少了 `-MMD`，這次就沒有重寫 `.d`。我看了時間，三個 `.d` 都比新的 `.o` 舊，裡面還是上一次的內容。少了 `-g`，gdb 也看不到原始碼行號。
> 所以實務上，自動產生相依的選項不放在 `CFLAGS`，另外放一個變數（例如 `DEPFLAGS = -MMD -MP`，命令寫成 `$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@`），使用者在命令列換 `CFLAGS` 也不會把它弄丟。
> 想讓 `make` 自動發現「選項變了」也做得到，例如把選項寫進一個檔案，讓每個 `.o` 都依賴它（kernel 的 Kbuild 就是這樣記錄每個 `.o` 的編譯命令，存在 `.X.o.cmd` 檔裡，K0 會看到）。
