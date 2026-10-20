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
| [`examples/make/`](./examples/make/) | [`Makefile`](./examples/make/Makefile)、[`Makefile.nodeps`](./examples/make/Makefile.nodeps)、[`Makefile.nophony`](./examples/make/Makefile.nophony)、[`mathx.h`](./examples/make/mathx.h)、[`mathx.c`](./examples/make/mathx.c)、[`main.c`](./examples/make/main.c) | [第七節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#七make只重編改過的檔案) | 規則與時間判斷、漏了標頭檔相依、`-MMD -MP`、`.PHONY` |
| [`examples/symbols/`](./examples/symbols/) | [`sym.c`](./examples/symbols/sym.c)、[`other.c`](./examples/symbols/other.c) | [第一節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#一本課在做什麼編譯器一次只看一個-c)〈上面說的「符號」到底是什麼〉（2026-10-20 提問） | 函式、全域變數、`extern` 變成符號，區域變數不是；機器碼的洞上標的是符號名 |

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

**第 1 次（YYYY-MM-DD）**
```
```

### 1-2. 用 `readelf -s` 看 `strstat.o` 和 `hw1_main.o`，貼上 `is_digit`、`count_digits`、`length`、`main`、`printf` 這幾列（在哪個 `.o` 出現就貼哪個）。每一列的 `Bind` 和 `Ndx` 是什麼？說明為什麼是這樣。【筆記[第四節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#四符號表o-之間靠名字接起來)的欄位表、第五節】

**第 1 次（YYYY-MM-DD）**
```
```

### 1-3. `msg` 這個字串在 `hw1_main.o` 的符號表裡有沒有一列叫 `msg`？先預測，再用 `readelf -s` 確認。為什麼？【筆記第五節〈函式裡的 `static`〉（哪些變數有符號、哪些沒有）】

**第 1 次（YYYY-MM-DD）**
```
```

### 1-4. 先預測再實驗：把 `strstat.c` 裡 `length` 的定義改成 `int length(const char *s)`（標頭檔不改）。(a) 重新 `gcc -Wall -c strstat.c`，會怎樣？是編譯還是連結階段？貼上訊息。(b) 再把 `strstat.c` 的 `#include "strstat.h"` 拿掉，重新編譯、連結、執行，會怎樣？為什麼 (a) 和 (b) 的結果不一樣？做完把檔案改回來。【筆記第三節〈定義的那個 `.c` 為什麼也要 `#include` 自己的標頭檔〉、第四節〈三種錯誤，各發生在哪一步〉】

**第 1 次（YYYY-MM-DD）**
```
```

---

## 2. 讓三個檔案共用一個全域變數

在 `hw2/` 裡寫一個小程式，有一個全域變數 `int log_level`，`main` 設定它，另外兩個檔案讀它：

- `logger.h`：`log_level` 的宣告，以及 `void log_msg(int level, const char *text);` 的宣告。要有 include guard。
- `logger.c`：`log_level` 的**定義，初值是 2**；`log_msg` 的定義：`level <= log_level` 時印出 `[level] text`，否則什麼都不印。
- `worker.h`、`worker.c`：`void do_work(void);`，裡面呼叫 `log_msg(1, "work start")`、`log_msg(3, "work detail")`。
- `hw2_main.c`：先呼叫一次 `do_work()`，把 `log_level` 改成 3，再呼叫一次 `do_work()`。

### 2-1. 貼上 `logger.h`、`logger.c` 的內容與執行結果。說明 `log_level` 在 `logger.h` 和 `logger.c` 各寫成什麼、為什麼要這樣分。【筆記[第六節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#六extern全域變數的宣告與定義)〈整理：全域變數放哪裡〉】

**第 1 次（YYYY-MM-DD）**
```
```

### 2-2. 先預測：`log_level` 在 `logger.o` 的符號表裡，`Ndx` 是 `.bss` 還是 `.data` 的編號？在 `worker.o`、`hw2_main.o` 裡呢？用 `readelf -s` 與 `readelf -S` 確認，貼上相關的列。為什麼跟筆記 `hits` 的例子在不同的區段？【筆記第五節〈函式裡的 `static`〉的 `.bss` 說明、第六節】

**第 1 次（YYYY-MM-DD）**
```
```

### 2-3. 先猜再實驗：在 `worker.c` 裡**也**寫一行 `int log_level = 1;`（有初值），然後用 `gcc -fcommon` 編譯全部檔案再連結。筆記說 `-fcommon` 會合併同名的全域變數，這次會成功嗎？貼上結果。做完把那一行刪掉。【新觀念，先猜】

**第 1 次（YYYY-MM-DD）**
```
```

### 2-4. 先猜再實驗：把 `logger.c` 的定義改成 `static int log_level = 2;`（`logger.h` 不改），只編譯 `logger.c`。會怎樣？是哪個階段報錯？如果這一步沒有報錯，你覺得連結時會出現什麼錯誤？貼上訊息，說明你的推理。做完改回來。【新觀念，先猜】（推理可以用筆記第五、六節：`static` 讓名字只在本檔案有效、`extern` 宣告「定義在別處」）

**第 1 次（YYYY-MM-DD）**
```
```

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
