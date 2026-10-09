# s1-05：讀懂 x86-64 組合語言（二）——函式呼叫與堆疊框

> 對應課程：第 ① 站第 5 課
> 課程筆記：[x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md)
> 自我測驗：寫在同資料夾的 [quiz.md](./quiz.md)

## 說明

這一課的目標還是**讀得懂**：自己打一小段 C（題目給的程式碼照打，或照題目說明寫）→ 先預測 → 用 `gcc`／`objdump`／`gdb` 實際看 → 對照、解釋。

- 看組合語言用：`gcc -O0 -c -fno-asynchronous-unwind-tables 檔名.c`，再 `objdump -dr --no-show-raw-insn 檔名.o`（題目指定 `-O2` 或要連結成執行檔的除外）。
- 編譯產物不會進 git（白名單），要留的輸出**貼進作答框**。只貼題目要的那個函式。
- 筆記裡的範例可以參考，但每一題的參數個數、型別、情境都跟範例不同，照抄範例答不出來。

每題「解釋原因」的地方都標了答案的來源：
- 【筆記第 X 節】：本課教過，回筆記那一節找。
- 【第 4 課筆記第 X 節】：回 [x86-64組合語言入門（一）](../../c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md) 那一節找。
- 【新觀念，先猜】：還沒教過，寫下你的猜測就好，猜錯不算錯。

**「先預測」的題目，一定要先把預測寫進作答框，再編譯。** 預測錯了沒關係，把錯在哪裡寫下來，這才是練習的重點。

**怎麼作答**：每個小題下面有作答框，直接寫在裡面（把 `YYYY-MM-DD` 換成作答日期）。批改與參考答案寫在作答框正下方，不重答。

---

## 筆記的範例程式

課程筆記裡每個實驗的原始碼與 gdb 命令稿都放在這個資料夾，可以直接跟著筆記重做。**它們是範例，不是作業**；作業的檔名（`pick.c`、`nine.c`、`sum9.c`、`chain.c`、`keep.c`、`va.c`）跟它們不重複。

| 檔案 | 筆記裡的位置 | 示範什麼 |
|---|---|---|
| [`frame.c`](./frame.c) | [第三節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#三call-與-ret用-gdb-一條一條走)〈實驗：`frame.c`〉、第四節 | `main` 呼叫 `add3`：`call`／`ret`、堆疊框、紅區 |
| [`trace.gdb`](./trace.gdb) | 第三節〈實驗：`frame.c`〉 | 用 `stepi` 一條一條看 `call`、`push`、`ret` 時 `%rsp` 的變化 |
| [`who.gdb`](./who.gdb) | 第三節〈停在函式入口時，怎麼知道是誰呼叫的〉 | `x/gx $rsp` ＋ `info symbol` 找呼叫者 |
| [`leave.gdb`](./leave.gdb) | [第四節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#四堆疊框pushq-rbp-movq-rsp-rbp-與-leave)〈`leave`：一條指令做兩件事〉 | `leave` 前後的 `%rsp`、`%rbp` |
| [`redzone.gdb`](./redzone.gdb) | 第四節〈上圖 `add3` 的變數在 `%rsp` 的「下面」：紅區〉 | 葉函式的區域變數在 `%rsp` 下方 |
| [`args8.c`](./args8.c) | [第五節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#五參數與回傳值放哪裡)〈實驗：8 個參數〉 | 前 6 個參數的暫存器、第 7、8 個放堆疊 |
| [`saved.c`](./saved.c)、[`noret.c`](./noret.c) | [第六節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#六誰負責保存暫存器以及-16-byte-對齊) | callee-saved 的 `%rbx`；只為了對齊的 `sub $0x8,%rsp` |
| [`varargs.c`](./varargs.c) | [第七節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#七printf-前的-movl-0-eax可變參數函式的-al) | 可變參數函式呼叫前的 `%al` ＝ 0、1、2 |

---

## 1. 不同型別的參數放在哪裡

寫 `pick.c`：

```c
long pick(char a, short b, int c, long d, unsigned char *e, unsigned int f)
{
    return a + b + c + d + *e + f;
}
```

```
gcc -O0 -c -fno-asynchronous-unwind-tables pick.c
objdump -dr --no-show-raw-insn pick.o
```

### 1-1. 編譯前先預測：6 個參數各是放在哪個暫存器傳進來的？各用那個暫存器的哪個名字（64／32／16／8 位元）？寫成一張表。【筆記[第五節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#五參數與回傳值放哪裡)的表與表下第一段；[第 4 課筆記第二節](../../c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#二暫存器cpu-裡的-16-個變數)的暫存器名字表】

**第 1 次（YYYY-MM-DD）**
```

```

### 1-2. 編譯，貼上 `pick` 從開頭到「第一個 `movs` 開頭的指令」之前的那幾行（存參數的部分）。對照預測：每個參數被存到哪個 `-N(%rbp)`？用的是幾 byte 的 `mov`？哪裡跟你預測的不一樣？

**第 1 次（YYYY-MM-DD）**
```

```

### 1-3. `e` 是指標、`f` 是 `unsigned int`，為什麼一個用暫存器的 64 位元名字、一個用 32 位元名字？【筆記[第五節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#五參數與回傳值放哪裡)表下第一段；[第 2 課筆記第一節](../../c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#一位元組與型別大小)（型別大小）】

**第 1 次（YYYY-MM-DD）**
```

```

### 1-4. 把參數加起來之前，`a`、`b` 先經過哪兩個指令變成 32 位元？為什麼它們用 `movs` 開頭，而不是 `movz` 開頭？【[第 4 課筆記第二節](../../c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#二暫存器cpu-裡的-16-個變數)〈寫低位的名字時，高位會怎樣？〉的擴展指令表；[第 2 課筆記第一節](../../c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#一位元組與型別大小)〈`unsigned` 單獨寫就是 `unsigned int`：整數型別的所有寫法〉（`char` 在本機有沒有號）】

**第 1 次（YYYY-MM-DD）**
```

```

---

## 2. 9 個參數與 16 byte 對齊

寫兩個檔案。`nine.c` 只**宣告** `sum9`（跟筆記第六節的 `saved.c` 一樣，讓 gcc 不知道 `sum9` 長什麼樣子）：

```c
/* nine.c */
int sum9(int a, int b, int c, int d, int e, int f, int g, int h, int i);

int call9(void)
{
    return sum9(10, 20, 30, 40, 50, 60, 70, 80, 90);
}
```

`sum9.c` 是 `sum9` 本體：

```c
/* sum9.c */
int sum9(int a, int b, int c, int d, int e, int f, int g, int h, int i)
{
    return a + b + c + d + e + f + g + h + i;
}
```

```
gcc -O0 -c -fno-asynchronous-unwind-tables nine.c
gcc -O0 -c -fno-asynchronous-unwind-tables sum9.c
objdump -dr --no-show-raw-insn nine.o
objdump -dr --no-show-raw-insn sum9.o
```

### 2-1. 編譯前先預測 `call9`：哪幾個參數用 `push` 放上堆疊？推的順序是什麼？每個 `int` 參數在堆疊上佔幾 byte？【筆記[第五節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#五參數與回傳值放哪裡)的表、〈實驗：8 個參數，第 7、8 個去哪了〉】

**第 1 次（YYYY-MM-DD）**
```

```

### 2-2. 編譯前先預測 `sum9`：第 7、8、9 個參數（`g`、`h`、`i`）各在 `N(%rbp)` 的哪裡？說明這幾個數字是怎麼算出來的。【筆記第五節〈實驗：8 個參數，第 7、8 個去哪了〉的逐點說明；[第四節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#四堆疊框pushq-rbp-movq-rsp-rbp-與-leave)】

**第 1 次（YYYY-MM-DD）**
```

```

### 2-3. 編譯，貼上 `call9` 和 `sum9` 的本體，跟 2-1、2-2 的預測對照。

**第 1 次（YYYY-MM-DD）**
```

```

### 2-4. `call9` 在 `call` 之後用 `add` 把 `%rsp` 加回去的數字是多少？跟推上堆疊的參數總共幾 byte 一樣嗎？如果不一樣，多出來的是 `call9` 裡哪一行指令造成的？為什麼需要那一行？【筆記[第六節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#六誰負責保存暫存器以及-16-byte-對齊)〈上面的 `sub $0x8,%rsp`：16 byte 對齊〉】

**第 1 次（YYYY-MM-DD）**
```

```

### 2-5. 把 `nine.c` 裡 `sum9` 的宣告換成 `sum9.c` 的完整定義（兩個函式放在同一個檔案），重新編譯，看 `call9`。2-4 找到的那一行還在嗎？猜猜看為什麼。【新觀念，先猜】

**第 1 次（YYYY-MM-DD）**
```

```

---

## 3. 用 gdb 沿著堆疊找出兩層呼叫

寫 `chain.c`，編成有除錯資訊的執行檔：

```c
#include <stdio.h>

int inner(int y)
{
    return y * 3;
}

int outer(int x)
{
    return inner(x + 1) * 2;
}

int main(void)
{
    printf("%d\n", outer(4));
    return 0;
}
```

```
gcc -O0 -g chain.c -o chain
objdump -d --no-show-raw-insn chain
gdb ./chain
```

### 3-1. 在 gdb 裡 `break *inner`、`run`，停在 `inner` 的第一條指令。用本課的命令找出返回位址，用 `info symbol` 說出它在哪個函式的 `+` 幾；再到 `objdump` 的輸出裡，找出它是哪一條 `call` 的下一條指令。貼上你下的命令與輸出。【筆記[第三節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#三call-與-ret用-gdb-一條一條走)〈停在函式入口時，怎麼知道是誰呼叫的〉】

**第 1 次（YYYY-MM-DD）**
```

```

### 3-2. 接著 `stepi` 三次（執行完 `endbr64`、`push %rbp`、`mov %rsp,%rbp`），先用 `info registers rbp` 確認 `%rbp` 已經是 `inner` 的了。然後**只用 `x/2gx`**，從 `inner` 的 `%rbp` 出發，沿著 `%rbp` 鏈往上走兩層，找出 `outer` 的返回位址（它回到 `main` 的哪裡）。貼上命令與輸出，說明每一次 `x/2gx` 讀到的兩格各是什麼。【筆記[第四節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#四堆疊框pushq-rbp-movq-rsp-rbp-與-leave)〈畫出來：停在 `add3` 裡面時的堆疊〉】

**第 1 次（YYYY-MM-DD）**
```

```

### 3-3. 把 3-2 找到的東西畫成像筆記〈畫出來〉那樣的堆疊圖：從 `main` 的 `%rbp` 那一格，往下畫到 `inner` 的 `%rbp` 那一格。每一格寫位址（最後 3 位就好）、內容、屬於誰的堆疊框。（不知道內容的格子寫「不知道」就好，不必每格都填滿。）【筆記第四節〈畫出來〉】

**第 1 次（YYYY-MM-DD）**
```

```

### 3-4. 在同一個地方下 `bt`，貼上輸出。`bt` 印出的每一層位址，跟你在 3-1、3-2 找到的返回位址一致嗎？【[第 3 課筆記第六節](../../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#六用-gdb-找-segfault實際走一次)（`bt`）；筆記第三節〈停在函式入口時，怎麼知道是誰呼叫的〉最後一段】

**第 1 次（YYYY-MM-DD）**
```

```

---

## 4. 要撐過兩次 `call` 的值

寫 `keep.c`（`g` 只宣告，理由同第 2 題）：

```c
long g(long x);

long h(long a, long b)
{
    long x = g(a);
    long y = g(b);
    return x + y + a;
}
```

```
gcc -O2 -c -fno-asynchronous-unwind-tables keep.c
objdump -dr --no-show-raw-insn keep.o
```

### 4-1. 編譯前先預測：`a`、`b`、`x`、`y` 裡面，哪些值在某一次 `call` 之後還要用（每個值寫出它要撐過哪幾次 `call`）？`h` 會用到幾個 callee-saved 暫存器、開頭 `push` 幾次？需不需要額外的 `sub $0x8,%rsp` 來對齊？【筆記[第六節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#六誰負責保存暫存器以及-16-byte-對齊)】

**第 1 次（YYYY-MM-DD）**
```

```

### 4-2. 編譯，貼上 `h` 的本體，在每個 callee-saved 暫存器旁邊註明它存的是哪個值，跟預測對照。

**第 1 次（YYYY-MM-DD）**
```

```

### 4-3. 把 `return` 改成 `x + y`（不加 `a`），重新編譯，貼上本體。`push` 的次數、有沒有 `sub $0x8,%rsp` 各怎麼變？用「進函式時 `%rsp` 除以 16 餘幾、每個 `push` 挪幾 byte」算一次，解釋為什麼。【筆記第六節〈上面的 `sub $0x8,%rsp`：16 byte 對齊〉】

**第 1 次（YYYY-MM-DD）**
```

```

### 4-4. 4-2 的 `h` 裡，`pop` 的順序跟 `push` 的順序是什麼關係？為什麼一定要這樣？【筆記[第二節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#二堆疊rsp-指著頂端往低位址長)】

**第 1 次（YYYY-MM-DD）**
```

```

---

## 5. 可變參數函式的 `%al`

寫 `va.c`：

```c
#include <stdio.h>

void showd(double x)
{
    printf("%f\n", x);
}

int main(void)
{
    printf("%d %f %d %f %f\n", 1, 0.5, 2, 1.5, 2.5);
    showd(0.25);
    printf("%f %f %f %f %f %f %f %f %f\n",
           1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    return 0;
}
```

```
gcc -O0 -fno-asynchronous-unwind-tables va.c -o va
./va
objdump -d --no-show-raw-insn va
```

### 5-1. 編譯前先預測第一個 `printf`：6 個參數（含格式字串）各放在哪個暫存器？呼叫前 `%eax` 會被設成多少？【筆記[第五節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#五參數與回傳值放哪裡)表下第一段、[第七節](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#七printf-前的-movl-0-eax可變參數函式的-al)】

**第 1 次（YYYY-MM-DD）**
```

```

### 5-2. 編譯前先預測：`main` 在 `call showd` 之前會不會設 `%eax`？為什麼？【筆記第七節最後一段】

**第 1 次（YYYY-MM-DD）**
```

```

### 5-3. 編譯，貼上 `main` 裡第一個 `printf` 和 `showd` 這兩次呼叫前面設參數的那幾行，跟 5-1、5-2 的預測對照。

**第 1 次（YYYY-MM-DD）**
```

```

### 5-4. 第二個 `printf` 有 9 個 `double`，可是傳浮點參數的暫存器只有 `%xmm0`～`%xmm7` 八個。編譯前先猜：第 9 個 `double` 會放在哪裡？`%al` 會是多少？再對照實際輸出，貼上相關的幾行。【新觀念，先猜】

**第 1 次（YYYY-MM-DD）**
```

```
