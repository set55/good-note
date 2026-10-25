# s1-07：靜態與動態函式庫——`ar` 與 `.a`、`-fPIC` 與 `.so`、`-L`／`-l`、執行時怎麼找到 `.so`

> 對應課程：第 ① 站第 7 課
> 課程筆記：[靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html)
> 自我測驗：寫在同資料夾的 [quiz.md](./quiz.md)

## 說明

這一課要**自己動手做**函式庫：用 `ar` 包 `.a`、用 `-fPIC -shared` 做 `.so`，然後讓程式在連結時、執行時都找得到它們。

- 每一題一個資料夾（`hw1/`～`hw5/`），檔案自己建。題目給的程式碼照打；題目給的指令照下，**指令的輸出要貼進作答框**。
- 編譯一律加 `-Wall`，**有警告就要處理**，不能忽略。
- 編譯產物（`.o`、`.a`、`.so`、執行檔）不會進 git（白名單），要留的輸出**貼進作答框**。
- 筆記裡的範例（`calc_add`、`counter`）可以參考，但每一題的檔案、情境都跟範例不同，照抄範例答不出來。

每題「解釋原因」的地方都標了答案的來源：
- 【筆記第 X 節】：本課教過，回筆記那一節找。
- 【新觀念，先猜】：還沒教過，寫下你的猜測就好，猜錯不算錯。

**「先預測」的題目，一定要先把預測寫進作答框，再執行。** 預測錯了沒關係，把錯在哪裡寫下來，這才是練習的重點。

**第 6 課反覆卡在同一件事：實驗沒跑到最後一步、沒貼證據就下結論。** 這一課每個小題都寫明「要貼什麼」，缺了就算不完整：
- 「沒有錯誤訊息」不代表結果對，一定要**執行**並貼上輸出；
- 改了檔案之後，確認你執行的是**這次重新做出來的**檔案（必要時先 `rm` 舊的產物）。

**怎麼作答**：每個小題下面有作答框，直接寫在裡面（把 `YYYY-MM-DD` 換成作答日期）。批改與參考答案寫在作答框正下方，不重答。

---

## 筆記的範例程式

課程筆記裡每個實驗的原始碼都放在 [`examples/`](./examples/)，一個實驗一個子資料夾，可以直接跟著筆記重做。**它們是範例，不是作業**；作業寫在 `hw1/`～`hw5/`，檔名跟範例不重複。

| 資料夾 | 檔案 | 筆記裡的位置 | 示範什麼 |
|---|---|---|---|
| [`examples/calc/`](./examples/calc/) | [`calc.h`](./examples/calc/calc.h)、[`add.c`](./examples/calc/add.c)、[`mul.c`](./examples/calc/mul.c)、[`main.c`](./examples/calc/main.c)、[`both.c`](./examples/calc/both.c) | [第一節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#一本課在做什麼函式庫就是先編好的-o)～[第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#四動態函式庫-shared-與--fpic) | `ar rcs` 做 `libcalc.a`、只拉需要的成員、`both.c` 整個 `.o` 被拉進來、`-lcalc` 寫在前面的 `undefined reference`、`-L`／`-l`／`-l:`／`-static`、做 `libcalc.so` 與執行時找不到 |
| [`examples/pic/`](./examples/pic/) | [`count.c`](./examples/pic/count.c)、[`count_static.c`](./examples/pic/count_static.c)、[`usecount.c`](./examples/pic/usecount.c)、[`callbump.c`](./examples/pic/callbump.c)、[`tally.c`](./examples/pic/tally.c)、[`conf.c`](./examples/pic/conf.c)、[`app.c`](./examples/pic/app.c)、[`greet.c`](./examples/pic/greet.c)、[`greet_main.c`](./examples/pic/greet_main.c)、[`logger.c`](./examples/pic/logger.c)、[`runner.c`](./examples/pic/runner.c) | [第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#四動態函式庫-shared-與--fpic)〈為什麼一定要 `-fPIC`〉 | 不加 `-fPIC` 時 `recompile with -fPIC`、GOT 間接存取、符號插入（主程式的 `counter` 贏）、`static` 變數不用 `-fPIC`、`callbump.c` 用 gdb 看 GOT 那格載入後裝 `&counter`（10-25 追問）、`tally.c` 是自我測驗第 6 題、`conf.c`／`app.c` 是第 7 題（10-25 追問「接 `.so` 為什麼不算重複定義」）、`greet.c`／`greet_main.c` 看 PLT 跳板與 `.got.plt` 的延遲綁定（10-26 追問）、`logger.c`／`runner.c` 是第 8 題 |
| [`examples/search/`](./examples/search/) | [`calc.h`](./examples/search/calc.h)、[`mul.c`](./examples/search/mul.c)、[`main.c`](./examples/search/main.c)、[`v1/add.c`](./examples/search/v1/add.c)、[`v2/add.c`](./examples/search/v2/add.c) | [第五節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#五執行時怎麼找到-so)〈`RPATH` 和 `RUNPATH` 差在哪〉、[第六節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#六換掉-so-不用重新連結以及-soname)〈換掉 `.so`〉 | `RUNPATH` 讓 `LD_LIBRARY_PATH` 優先、`RPATH` 不讓；換掉 `.so` 不用重新連結 |
| [`examples/soname/`](./examples/soname/) | [`calc.h`](./examples/soname/calc.h)、[`add.c`](./examples/soname/add.c)、[`mul.c`](./examples/soname/mul.c)、[`main.c`](./examples/soname/main.c) | 第六節〈SONAME〉 | `-Wl,-soname`、三個名字與符號連結、`NEEDED` 記的是 SONAME |

---

## 1. 做一個靜態函式庫，看連結器拉了哪些成員

在 `hw1/` 裡建下面五個檔案。這是一個溫度換算函式庫：溫度先算成「十分之一度」的整數，再由 `temp_round` 四捨五入成整數度。

`temp.h`：

```c
#ifndef TEMP_H
#define TEMP_H

int c_to_f(int c);
int f_to_c(int f);
int temp_round(int x10);

#endif
```

`ctof.c`：

```c
#include "temp.h"

int c_to_f(int c)
{
    return temp_round(c * 18 + 320);
}
```

`ftoc.c`：

```c
#include "temp.h"

int f_to_c(int f)
{
    return temp_round((f - 32) * 100 / 18);
}
```

`round.c`：

```c
#include "temp.h"

int temp_round(int x10)
{
    return (x10 + 5) / 10;
}
```

`hw1_main.c`（**只呼叫 `f_to_c`**）：

```c
#include <stdio.h>
#include "temp.h"

int main(void)
{
    printf("f_to_c(212) = %d\n", f_to_c(212));
    return 0;
}
```

把 `ctof.c`、`ftoc.c`、`round.c` 各自 `gcc -Wall -c` 成 `.o`，用 `ar rcs` 包成 `libtemp.a`，再用 `-L. -ltemp` 連結成 `hw1`。

### 1-1. 貼上你建立 `libtemp.a` 與連結 `hw1` 的指令、`./hw1` 的輸出、`ar t libtemp.a` 與 `nm -s libtemp.a` 的輸出。`nm -s` 印出的 `Archive index` 是什麼、誰會用到它？再用 `ar rcS libnoidx.a ctof.o ftoc.o round.o` 做一個沒有索引的版本、用 `-L. -lnoidx` 連結，貼上結果，並用 `ranlib` 修好它（貼上修好後的連結與執行結果）。【筆記[第二節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#二靜態函式庫用-ar-把-o-包成-a)〈實驗：做出 `libcalc.a`〉】

**第 1 次（YYYY-MM-DD）**
```
```

### 1-2. 先預測：`nm hw1 | grep -E 'c_to_f|f_to_c|temp_round'` 會列出這三個函式裡的哪幾個？寫下預測後再執行，貼上結果，並貼上 `gcc -Wall hw1_main.c -L. -ltemp -Wl,--verbose -o hw1 2>&1 | grep libtemp` 的輸出。`hw1_main.c` 沒有直接呼叫 `temp_round`，它為什麼（或為什麼沒有）在執行檔裡？`c_to_f` 呢？【筆記第二節〈連結：只拉需要的成員〉】

**第 1 次（YYYY-MM-DD）**
```
```

---

## 2. 兩個互相需要的靜態函式庫：順序怎麼排

在 `hw2/` 裡，把函式庫拆成兩個：`libconv.a` 放換算函式，`libround.a` 放四捨五入。這次 `temp_round` 會先呼叫 `temp_clamp`，把低於絕對零度（−273.2 度，也就是 −2732 個十分之一度）的值夾住，而 `temp_clamp` 放在 **`libconv.a`** 裡。

`temp.h`：

```c
#ifndef TEMP_H
#define TEMP_H

int c_to_f(int c);
int f_to_c(int f);
int temp_round(int x10);
int temp_clamp(int x10);

#endif
```

`ctof.c`、`ftoc.c`：跟第 1 題一樣（照第 1 題的內容再建一次）。

`limit.c`：

```c
#include "temp.h"

int temp_clamp(int x10)
{
    return x10 < -2732 ? -2732 : x10;
}
```

`round.c`（跟第 1 題不同：多了一行呼叫 `temp_clamp`）：

```c
#include "temp.h"

int temp_round(int x10)
{
    x10 = temp_clamp(x10);
    return (x10 + 5) / 10;
}
```

`hw2_main.c`：

```c
#include <stdio.h>
#include "temp.h"

int main(void)
{
    printf("f_to_c(100) = %d\n", f_to_c(100));
    return 0;
}
```

建立兩個函式庫：

```sh
gcc -Wall -c ctof.c ftoc.c limit.c round.c
ar rcs libconv.a ctof.o ftoc.o limit.o
ar rcs libround.a round.o
```

### 2-1. 先預測下面三個連結指令，哪些會成功、失敗的會說哪個符號找不到（是在哪個檔案裡需要它）？寫下預測後再逐一執行，貼上每一個的完整輸出（成功的要執行 `./hw2` 並貼上結果）。用「`ld` 由左往右掃，手上有一張還缺哪些符號的清單」的方式，**逐步**說明每個指令掃到每個檔案時拉了哪些成員、清單怎麼變。【筆記第二節〈順序：`ld` 由左往右只掃一次〉】

```sh
gcc -Wall hw2_main.c -L. -lround -lconv -o hw2           # (a)
gcc -Wall hw2_main.c -L. -lconv -lround -o hw2           # (b)
gcc -Wall hw2_main.c -L. -lconv -lround -lconv -o hw2    # (c)
```

**第 1 次（YYYY-MM-DD）**
```
```

### 2-2. 不重複寫 `-lconv`，改用 `-Wl,--start-group … -Wl,--end-group` 讓 (b) 的順序也能連結成功。貼上指令與執行結果。【筆記第二節〈順序〉最後一段】

**第 1 次（YYYY-MM-DD）**
```
```

### 2-3. 先猜再實驗：把兩個函式庫改做成 `.so`（指令如下），再用 (a)、(b) 兩種順序連結。各會成功還是失敗？貼上結果（成功的要執行）。為什麼 `.so` 版的 (b) 不必像 `.a` 那樣把 `-lconv` 寫兩次？(a) 為什麼還是失敗（或為什麼成功）？【新觀念，先猜；(a) 的部分可以用筆記[第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#四動態函式庫-shared-與--fpic)〈`.so` 在 Ubuntu 上也有順序問題〉推】

```sh
mkdir -p so
gcc -Wall -fPIC -shared ctof.c ftoc.c limit.c -o so/libconv.so
gcc -Wall -fPIC -shared round.c -o so/libround.so
gcc -Wall hw2_main.c -Lso -lround -lconv -Wl,-rpath,'$ORIGIN/so' -o hw2so   # (a)
gcc -Wall hw2_main.c -Lso -lconv -lround -Wl,-rpath,'$ORIGIN/so' -o hw2so   # (b)
```

**第 1 次（YYYY-MM-DD）**
```
```

---

## 3. 做一個 `.so`：`-fPIC`、GOT、讓執行檔找得到它

在 `hw3/` 裡。這次函式庫多了一個全域變數 `temp_calls`，記錄 `temp_round` 被呼叫了幾次。

`temp.h`：

```c
#ifndef TEMP_H
#define TEMP_H

extern int temp_calls;

int c_to_f(int c);
int f_to_c(int f);
int temp_round(int x10);

#endif
```

`ctof.c`、`ftoc.c`：跟第 1 題一樣。

`round.c`：

```c
#include "temp.h"

int temp_calls = 0;

int temp_round(int x10)
{
    temp_calls = temp_calls + 1;
    return (x10 + 5) / 10;
}
```

`hw3_main.c`：

```c
#include <stdio.h>
#include "temp.h"

int main(void)
{
    printf("%d %d\n", f_to_c(100), c_to_f(100));
    printf("temp_calls = %d\n", temp_calls);
    return 0;
}
```

### 3-1. 先預測：三個檔案都**不加** `-fPIC`，執行 `gcc -Wall -c ctof.c ftoc.c round.c && gcc -shared ctof.o ftoc.o round.o -o libtemp.so`，會成功嗎？如果失敗，錯誤訊息會指向哪個 `.o`、哪個符號？寫下預測後執行，貼上輸出。為什麼 `ctof.o`、`ftoc.o` 沒有被點名（它們也呼叫了別的檔案的 `temp_round`）？【筆記第四節〈為什麼一定要 `-fPIC`〉、〈為什麼不能直接用固定距離〉】

**第 1 次（YYYY-MM-DD）**
```
```

### 3-2. 改用 `-fPIC` 重編三個檔案、做出 `libtemp.so`。貼上 `objdump -dr --no-show-raw-insn round.o` 裡 `temp_round` 的內容，標出**哪幾行在存取 `temp_calls`**，逐行說明那幾行在做什麼（中間 `imul $0x66666667`、`shr`、`sar` 那幾行是 gcc 用乘法代替「除以 10」的寫法，這題不用解釋）。再貼上 `readelf -r libtemp.so | grep temp_calls`，說明那一列是留給誰、要它做什麼。【筆記第四節〈為什麼一定要 `-fPIC`〉】

**第 1 次（YYYY-MM-DD）**
```
```

### 3-3. 用 `gcc -Wall hw3_main.c -L. -ltemp -o hw3` 連結，執行 `./hw3`，貼上輸出與 `ldd ./hw3` 的輸出。這個錯誤是誰印的？為什麼連結成功了，執行卻找不到？接著用兩種方法讓它跑起來，各貼上指令與輸出：(a) `LD_LIBRARY_PATH`；(b) 重新連結，用 `-Wl,-rpath,'$ORIGIN'` 把路徑寫進執行檔（另外貼上 `readelf -d hw3 | grep -E 'NEEDED|RUNPATH'`）。輸出裡 `temp_calls` 是幾？為什麼是這個數字？【筆記[第五節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#五執行時怎麼找到-so)〈方法一〉、〈方法二〉】

**第 1 次（YYYY-MM-DD）**
```
```

### 3-4. 先預測：在 `hw3_main.c` 的 `#include "temp.h"` 下面加一行 `int temp_calls = 1000;`（`temp.h` 不改），照 3-3 (b) 的方式重新連結、執行，`temp_calls = ` 會印出多少？寫下預測後執行，貼上輸出與 `LD_DEBUG=bindings ./hw3 2>&1 | grep temp_calls`。為什麼？接著把 `round.o` 等三個 `-fPIC` 的 `.o` 用 `ar rcs libtemp.a ctof.o ftoc.o round.o` 包成 `.a`，用 `gcc -Wall hw3_main.c -L. -l:libtemp.a -o hw3s` 靜態連結，先預測會怎樣再執行、貼上結果；為什麼同樣兩份 `temp_calls`，動態和靜態的結果不一樣？做完把加的那一行刪掉。【筆記第四節〈為什麼不能直接用固定距離〉】

**第 1 次（YYYY-MM-DD）**
```
```

---

## 4. `.so` 依賴另一個 `.so`：`RUNPATH` 只管直接依賴

在 `hw4/` 裡。這次 `libtemp.so` 自己也需要另一個函式庫 `libround.so`，兩個都放在 `hw4/lib/`，執行檔 `hw4` 放在 `hw4/`。

`temp.h`、`ctof.c`、`ftoc.c`、`round.c`：跟**第 1 題**一樣（沒有 `temp_calls`、沒有 `temp_clamp`）。

`hw4_main.c`：

```c
#include <stdio.h>
#include "temp.h"

int main(void)
{
    printf("c_to_f(37) = %d\n", c_to_f(37));
    return 0;
}
```

建立與連結（照打；`-Wl,-rpath-link,lib` 是什麼見 4-3）：

```sh
mkdir -p lib
gcc -Wall -fPIC -shared round.c -o lib/libround.so
gcc -Wall -fPIC -shared ctof.c ftoc.c -Llib -lround -o lib/libtemp.so
gcc -Wall hw4_main.c -Llib -ltemp -Wl,-rpath-link,lib -Wl,-rpath,'$ORIGIN/lib' -o hw4
```

### 4-1. 先預測 `./hw4` 會成功嗎？寫下預測後執行，貼上輸出；再貼上 `readelf -d hw4 | grep -E 'NEEDED|RUNPATH'`、`readelf -d lib/libtemp.so | grep -E 'NEEDED|RUNPATH'`，以及 `LD_DEBUG=libs ./hw4 2>&1 | grep -E 'find library|search path'`。`hw4` 明明有 `RUNPATH`、`libround.so` 也跟 `libtemp.so` 放在同一個資料夾，為什麼找不到（或為什麼找得到）？【筆記第五節〈`ld.so` 去了哪些地方找〉的第 3 步、〈`RPATH` 和 `RUNPATH` 差在哪〉最後一段】

**第 1 次（YYYY-MM-DD）**
```
```

### 4-2. 用兩種方法修好，各貼上你改的指令、`readelf -d` 相關的列與 `./hw4` 的輸出：(a) 只重做 `lib/libtemp.so`、讓它自己帶 `RUNPATH`（路徑要用 `$ORIGIN` 寫，想一想 `libtemp.so` 的 `$ORIGIN` 是哪個資料夾）；(b) 不改 `libtemp.so`（先把它做回原本的樣子），改成重新連結 `hw4`、用 `RPATH`。兩種方法你會選哪一種？為什麼？【筆記第五節〈`RPATH` 和 `RUNPATH` 差在哪〉】

**第 1 次（YYYY-MM-DD）**
```
```

### 4-3. 先猜再實驗：把連結 `hw4` 的指令拿掉 `-Wl,-rpath-link,lib`，重新連結，貼上輸出。`hw4_main.c` 根本沒呼叫 `temp_round`，為什麼 `ld` 在**連結時**就要去找 `libround.so`？`-rpath-link` 跟 `-rpath` 差在哪？【`-rpath-link` 的作用見筆記[第七節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#七本課用到的工具選項與完整清單在哪)〈`ld`〉的表；`ld` 為什麼要在連結時檢查是新觀念，先猜】

**第 1 次（YYYY-MM-DD）**
```
```

---

## 5. 函式庫更新：靜態 vs 動態，以及 SONAME

在 `hw5/` 裡。`temp.h`、`ctof.c`、`ftoc.c` 跟第 1 題一樣，但**`round.c` 先故意寫錯**（無條件捨去，不是四捨五入）：

```c
#include "temp.h"

int temp_round(int x10)
{
    return x10 / 10;
}
```

`hw5_main.c`：

```c
#include <stdio.h>
#include "temp.h"

int main(void)
{
    printf("f_to_c(100) = %d\n", f_to_c(100));
    return 0;
}
```

做出靜態版與帶 SONAME 的動態版，各連結一支程式：

```sh
gcc -Wall -fPIC -c ctof.c ftoc.c round.c
ar rcs libtemp.a ctof.o ftoc.o round.o
gcc -shared -Wl,-soname,libtemp.so.1 ctof.o ftoc.o round.o -o libtemp.so.1.0.0
ln -sf libtemp.so.1.0.0 libtemp.so.1
ln -sf libtemp.so.1 libtemp.so
gcc -Wall hw5_main.c -L. -l:libtemp.a -o hw5_static
gcc -Wall hw5_main.c -L. -ltemp -Wl,-rpath,'$ORIGIN' -o hw5_dyn
```

### 5-1. 執行兩支程式，貼上輸出（100°F 正確是 37.8°C，四捨五入應該是 38）。貼上 `ls -l libtemp*`、`readelf -d hw5_dyn | grep -E 'NEEDED|RUNPATH'`。`ld` 照 `-ltemp` 打開的是 `libtemp.so`，`NEEDED` 卻寫 `libtemp.so.1`，為什麼？【筆記[第六節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#六換掉-so-不用重新連結以及-soname)〈SONAME〉】

**第 1 次（YYYY-MM-DD）**
```
```

### 5-2. 先預測：把 `round.c` 改成正確的四捨五入（`return (x10 + 5) / 10;`），然後**只更新函式庫、不重新連結兩支程式**：

```sh
gcc -Wall -fPIC -c round.c
ar rcs libtemp.a round.o
gcc -shared -Wl,-soname,libtemp.so.1 ctof.o ftoc.o round.o -o libtemp.so.1.0.1
ln -sf libtemp.so.1.0.1 libtemp.so.1
```

`./hw5_static`、`./hw5_dyn` 各會印出多少？寫下預測後執行，貼上輸出。為什麼兩支結果不同？`hw5_static` 要怎樣才能拿到修正？`ar rcs libtemp.a round.o` 之後 `ar t libtemp.a` 有幾個成員，為什麼？【筆記第六節〈換掉 `.so`：程式不用重新連結〉、第二節的 `ar` 文法（`r`）】

**第 1 次（YYYY-MM-DD）**
```
```

### 5-3. 先預測：下一版把 `f_to_c` 改成回傳「十分之一度」（不相容的改變），新增 `ftoc2.c`，做成 SONAME 是 `libtemp.so.2` 的新版，並把連結名稱指到新版：

```c
#include "temp.h"

int f_to_c(int f)
{
    return (f - 32) * 100 / 18;
}
```

```sh
gcc -Wall -fPIC -c ftoc2.c
gcc -shared -Wl,-soname,libtemp.so.2 ctof.o ftoc2.o round.o -o libtemp.so.2.0.0
ln -sf libtemp.so.2.0.0 libtemp.so.2
ln -sf libtemp.so.2 libtemp.so
gcc -Wall hw5_main.c -L. -ltemp -Wl,-rpath,'$ORIGIN' -o hw5_dyn2
```

**不重新連結**舊的 `hw5_dyn`，`./hw5_dyn` 和 `./hw5_dyn2` 各印出多少？寫下預測後執行，貼上輸出、`ls -l libtemp*` 與兩支程式的 `readelf -d … | grep NEEDED`。為什麼兩個版本可以同時存在、舊程式沒有被新版弄壞？如果 5-3 偷懶、新版沿用 SONAME `libtemp.so.1` 並把 `libtemp.so.1` 指過去，`hw5_dyn` 會印出什麼？（這一問只要推理，不用做。）【筆記第六節〈SONAME〉最後一段】

**第 1 次（YYYY-MM-DD）**
```
```
