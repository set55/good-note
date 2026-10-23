# 多檔案的 C 程式 —— 標頭檔、`static` 與 `extern`、三種連結錯誤、`make`

> 日期：2026-10-19
> 情境：系統化學習路線第 ① 站第 6 課。對應 CS:APP 第 7.1～7.6 節（連結：編譯器驅動程式、目的檔、符號與符號表、符號解析）；
> `make` 的部分對應 GNU make 手冊第 2、4、6、10 章（*An Introduction to Makefiles*、規則、變數、隱含規則）。
> 原本排在第 6 課的「靜態與動態函式庫」移到第 7 課（本課份量已經很多）。
> 適用範圍：實測本機 Ubuntu 24.04、x86-64、gcc 13.3.0、GNU make 4.3、GNU readelf／nm 2.42；所有輸出都是本機實際產生的
> 對應檔案：每個實驗的完整原始碼都寫在它出現的那一節；同樣的檔案放在
> [`small-project/s1-06-multi-file/examples/`](../small-project/s1-06-multi-file/examples/)（會進 git，編譯產物不進），一個實驗一個子資料夾；作業在 [`small-project/s1-06-multi-file/`](../small-project/s1-06-multi-file/)
> 相關：[hello.c怎麼變成能跑的程式](./hello.c怎麼變成能跑的程式-編譯的四個階段.md)（第 1 課：前處理、`.o` 裡的洞、符號解析與重定位、連結器只比對名字）、
> [readelf完整選項參考](./readelf完整選項參考.md)、[x86-64組合語言入門二](./x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md)（第 5 課）

**一句話結論：編譯器一次只看一個 `.c`，看不到別的檔案；它靠「宣告」知道別處的函式與變數長什麼樣子，再在 `.o` 的符號表留下「我需要 `X`」（`UND`）或「我提供 `X`」。
連結器只靠名字把這些需求和提供配對：找不到是 `undefined reference`，找到兩個是 `multiple definition`，型別不合它不會發現。
標頭檔就是「寫一次、讓定義的那邊和使用的那邊都 `#include`」的宣告，型別不合才會在編譯時被抓到。
檔案層級的 `static` 讓名字變成 `LOCAL`、別的檔案看不到；函式裡的 `static` 讓變數不放堆疊、值留到下一次呼叫。
全域變數在標頭檔要寫 `extern`（只宣告），整個程式只能有一個 `.c` 寫定義。
`make` 依「目標比先決條件舊就重做」決定要重編哪些檔案，所以先決條件要列完整（含標頭檔，用 `gcc -MMD -MP` 自動產生）。**

---

## 目錄

- [零、開課複習：第 5 課卡住的觀念](#零開課複習第-5-課卡住的觀念)
- [一、本課在做什麼：編譯器一次只看一個 `.c`](#一本課在做什麼編譯器一次只看一個-c)
- [二、宣告與定義：沒有宣告，編譯器只能猜](#二宣告與定義沒有宣告編譯器只能猜)
- [三、標頭檔：把宣告寫一次，讓兩邊都檢查](#三標頭檔把宣告寫一次讓兩邊都檢查)
- [四、符號表：`.o` 之間靠名字接起來](#四符號表o-之間靠名字接起來)
- [五、`static`：兩種完全不同的意思](#五static兩種完全不同的意思)
- [六、`extern`：全域變數的宣告與定義](#六extern全域變數的宣告與定義)
- [七、`make`：只重編改過的檔案](#七make只重編改過的檔案)
- [八、本課用到的工具、選項與完整清單在哪](#八本課用到的工具選項與完整清單在哪)
- [九、動手作業](#九動手作業)
- [自我測驗](#自我測驗)

---

## 零、開課複習：第 5 課卡住的觀念

第 5 課的作業與測驗批改後，記成「第 6 課開頭先複習」的觀念如下。今天的 10 題複習額度已經用完，這裡只講解、不另外出題；這些題目明天（10-20）會從複習排程考到。

1. **`%rsp` 本身 vs 它指的那格**：`%rsp` 裡放的是一個**位址**（例如 `0x7fffffffd408`）；返回位址是放在**那個位址的 8 byte 記憶體**裡。要看返回位址，是 `x/gx $rsp`，不是 `info registers rsp`。
   出處：[第 5 課第二節](./x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#二堆疊rsp-指著頂端往低位址長)、[第三節](./x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#三call-與-ret用-gdb-一條一條走)〈停在函式入口時，怎麼知道是誰呼叫的〉。
2. **紅區**：`%rsp` 下方 128 byte 保證不會被**別人**（例如訊號處理函式）蓋掉；葉函式自己也不 `push`／`call`，所以能直接把變數放在那裡，省一條 `sub`。會 `call` 的函式，`call` 會把返回位址寫進 `%rsp − 8`，所以 gcc 一律 `sub` 把變數放到 `%rsp` 上面。
   出處：[第 5 課第四節](./x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#四堆疊框pushq-rbp-movq-rsp-rbp-與-leave)〈上圖 `add3` 的變數在 `%rsp` 的「下面」：紅區〉與後面三個追問補充。
3. **callee-saved 的 `push %rbx`**：要撐過 `call` 的值不能留在 `%rdi` 這種 caller-saved 暫存器（被呼叫的函式可以隨便改），要搬到 `%rbx` 這種 callee-saved 暫存器；但 `%rbx` 也是**自己的呼叫者**的，借用前要先 `push` 存起來、結束前 `pop` 還回去。
   出處：[第 5 課第六節](./x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#六誰負責保存暫存器以及-16-byte-對齊)〈實驗：要跨過 `call` 保存的值放哪裡〉。
4. **停在第一條指令時 gdb 印錯參數**：`-O0` 的除錯資訊只記「參數在堆疊的哪一格」，序言還沒把 `%rdi` 存進去，所以讀到舊值；真正的參數在 `%rdi`、`%rsi`、`%rdx`。
   出處：[第 5 課第三節](./x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#三call-與-ret用-gdb-一條一條走)〈gdb 為什麼去堆疊讀參數，而不是讀暫存器〉。
5. **參數的正偏移從 `0x10(%rbp)` 開始**：`%rbp` 那格是舊 `%rbp`，`8(%rbp)` 是返回位址，第 7 個參數才在 `0x10(%rbp)`。
   出處：[第 5 課第五節](./x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#五參數與回傳值放哪裡)〈實驗：8 個參數，第 7、8 個去哪了〉。
6. **16 byte 對齊的算法**：規則是「執行 `call` 的那一刻 `%rsp` 是 16 的倍數」。進函式時 `call` 已經推了 8 byte，所以 `%rsp` 除以 16 餘 8；之後每個 `push`、`sub` 都要算進去，湊回餘 0 才能 `call`。
   出處：[第 5 課第六節](./x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md#六誰負責保存暫存器以及-16-byte-對齊)〈上面的 `sub $0x8,%rsp`：16 byte 對齊〉。
7. **畫堆疊圖時，區域變數與對齊用的空格也要佔格**：相鄰兩個 `%rbp` 相減再除以 8，就是中間應該有幾格。

本課回到 C 語言本身，組合語言只在第二節用來看一個 bug 的成因，讀到的指令都是第 4、5 課教過的。

---

## 一、本課在做什麼：編譯器一次只看一個 `.c`

到目前為止的程式都是一個 `.c` 檔。真正的專案（包括 Linux kernel，好幾萬個 `.c`）都是很多檔案。本課回答：

- `main.c` 呼叫 `mathx.c` 裡的函式，編譯 `main.c` 的時候編譯器怎麼知道那個函式長什麼樣子（第二、三節）；
- 兩個 `.o` 是怎麼接起來的，接不起來時錯誤訊息在說什麼（第四節）；
- 怎麼讓一個名字只在自己的檔案裡有效，怎麼讓一個變數被很多檔案共用（第五、六節）；
- 改了一個檔案，怎麼只重編需要重編的部分（第七節）。

關鍵事實只有一個：**gcc 編譯時一次只處理一個 `.c`**（加上它 `#include` 進來的內容），這個範圍叫**編譯單元**（translation unit，C 標準的用語，也譯「轉譯單元」）。
`gcc -c main.c` 時，gcc 完全不會去讀 `mathx.c`。檔案之間的連繫，要等到**連結**（第 1 課第五節）才發生，而連結器手上只有每個 `.o` 的**符號表**——一串名字。

---

**上面說的「符號」到底是什麼**

> 2026-10-20 提問：「在討論到ld連接時,常會提到符號,符號代表什麼?我只隱約感覺到是變數,function有關」（指上一段「連結器手上只有每個 `.o` 的符號表——一串名字」）

**符號（symbol）＝ 一個名字，加上「這個名字代表的那塊東西在哪裡」**：在哪個區段（`.text`、`.data`、`.bss`）、從區段開頭算第幾個 byte、佔幾個 byte。符號表（symbol table）就是一個 `.o` 裡所有符號的清單，像一本通訊錄：名字 → 地址。

在 C 裡，會變成符號的是**放在函式外面的東西**：

- **函式**（`twice`、`main`）——名字代表「這個函式第一條指令的位址」；
- **全域變數**（`counter`、`buf`）——名字代表「這個變數在記憶體裡的位址」；
- 呼叫了、但不在這個檔案裡的函式與變數（`printf`、`extern` 宣告的 `shared`）——也是符號，只是標成「未定義」，意思是「我要用這個名字，請連結器幫我找」。

**區域變數不是符號。** 為什麼？因為連結器的工作是「把機器碼裡對某個名字的參照，換成真正的位址」，只有**位址要等連結才知道**的東西才需要名字。區域變數放在堆疊上（第 5 課），位址是「執行時的 `%rbp` 減某個數」，編譯器自己就能寫成 `-0x4(%rbp)`，用不到連結器，所以不留名字。

實驗：一個檔案裡放函式、兩種全域變數、一個 `extern` 變數和一個區域變數，看哪些變成符號。`sym.c`（檔案：[`sym.c`](../small-project/s1-06-multi-file/examples/symbols/sym.c)）完整原始碼：

```c
#include <stdio.h>

int counter = 3;          /* 全域變數，有初值 */
int buf[100];             /* 全域變數，沒有初值 */
extern int shared;        /* 只有宣告：定義在 other.c */

int twice(int x)          /* 函式定義 */
{
    return 2 * x;
}

int main(void)
{
    int local = 5;        /* 區域變數（放在堆疊上） */
    printf("%d\n", twice(local) + counter + buf[0] + shared);
    return 0;
}
```

`other.c`（檔案：[`other.c`](../small-project/s1-06-multi-file/examples/symbols/other.c)）只有一行：

```c
int shared = 10;          /* shared 的定義 */
```

```sh
gcc -c sym.c -o sym.o
gcc -c other.c -o other.o
gcc sym.o other.o -o sym
readelf -s sym.o          # 符號表（各欄的意思見第四節）
nm sym.o                  # 同一份符號表的精簡版（第 1 課教過）
nm sym                    # 連結後的符號表
objdump -dr sym.o         # 機器碼加上重定位項目
```

`sym.o` 的符號表：

```
$ readelf -s sym.o
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS sym.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 .rodata
     4: 0000000000000000     4 OBJECT  GLOBAL DEFAULT    3 counter
     5: 0000000000000000   400 OBJECT  GLOBAL DEFAULT    4 buf
     6: 0000000000000000    18 FUNC    GLOBAL DEFAULT    1 twice
     7: 0000000000000012    82 FUNC    GLOBAL DEFAULT    1 main
     8: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND shared
     9: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND printf

$ nm sym.o
0000000000000000 B buf
0000000000000000 D counter
0000000000000012 T main
                 U printf
                 U shared
0000000000000000 T twice
```

逐列看（欄位的完整解釋在[第四節](#四符號表o-之間靠名字接起來)，這裡只看「誰變成了符號」）：

| C 原始碼裡的東西 | 在符號表裡 | 這個符號代表什麼 |
|---|---|---|
| `int counter = 3;` | 第 4 列 `OBJECT`，4 byte，`nm` 的 `D` | 「`.data` 區段（第 3 個區段）開頭第 0 byte 起的 4 個 byte」 |
| `int buf[100];` | 第 5 列 `OBJECT`，400 byte（100 個 `int`），`nm` 的 `B` | 「`.bss` 區段開頭起的 400 byte」；沒有初值的全域變數初值是 0，放 `.bss`（第五節〈函式裡的 `static`〉會細講 `.bss`） |
| `int twice(int x) {...}` | 第 6 列 `FUNC`，18 byte，`nm` 的 `T` | 「`.text` 第 0 byte 起的 18 byte 機器碼」 |
| `int main(void) {...}` | 第 7 列 `FUNC`，`Value` 是 `0x12` | 「`.text` 第 0x12（18）byte 起」——緊接在 `twice` 後面 |
| `extern int shared;` | 第 8 列 `UND`，`nm` 的 `U` | 「我要用叫 `shared` 的東西，這個檔案沒有」 |
| `printf(...)` | 第 9 列 `UND` | 同上，等 libc 提供 |
| `int local = 5;` | **沒有** | 堆疊上的變數，不需要名字 |
| `"%d\n"` 字串 | **沒有自己的名字**，用第 3 列的區段符號 `.rodata` 代表 | 下面說明 |

第 0～3 列是 ELF 的記帳用符號：第 0 列固定是空的、`FILE` 記原始檔名、`SECTION` 代表區段本身。

**符號用在哪裡：機器碼的洞上標的就是符號名字。** `objdump -dr sym.o` 看 `main`（只截跟符號有關的幾行；`#` 後面是說明）：

```
  1e:	c7 45 fc 05 00 00 00 	movl   $0x5,-0x4(%rbp)      # local = 5：直接寫堆疊，沒有洞
  2a:	e8 00 00 00 00       	call   2f <main+0x1d>       # 呼叫 twice：跳到哪還不知道，填 0
			2b: R_X86_64_PLT32	twice-0x4                  #   ← 洞上標「twice」
  2f:	8b 15 00 00 00 00    	mov    0x0(%rip),%edx       # 讀 counter：位址還不知道，填 0
			31: R_X86_64_PC32	counter-0x4
  37:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax       # 讀 buf[0]
			39: R_X86_64_PC32	buf-0x4
  3f:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax       # 讀 shared
			41: R_X86_64_PC32	shared-0x4
  49:	48 8d 05 00 00 00 00 	lea    0x0(%rip),%rax       # 取 "%d\n" 字串的位址
			4c: R_X86_64_PC32	.rodata-0x4                #   ← 用區段符號 .rodata
  58:	e8 00 00 00 00       	call   5d <main+0x4b>       # 呼叫 printf
			59: R_X86_64_PLT32	printf-0x4
```

每個洞（第 1 課第四節的 `00 00 00 00`）下面那一行重定位項目，寫的都是「這裡要填**某個符號**的位址」。連結器做的事就是：

1. 對每個符號名字找到它的定義（`shared` 在 `other.o` 找到、`printf` 在 libc）——**符號解析**；
2. 決定每個區段最後放在哪，算出每個符號的最終位址，填進洞裡——**重定位**。

而 `local = 5` 那一行（`movl $0x5,-0x4(%rbp)`）下面沒有重定位項目：沒有洞，也就沒有名字。

（`twice` 明明就在同一個 `.o`，為什麼呼叫它也留洞？因為 `twice` 是 `GLOBAL` 符號，組譯器把它交給連結器決定；這跟共享函式庫裡「同名符號可以被替換」有關，第 7 課再講，這裡不展開。）

連結之後，符號的 `Value` 從「區段裡的偏移」變成**程式裡的位址**，`other.o` 的 `shared` 也接進來了：

```
$ nm sym | grep -E ' (counter|buf|shared|twice|main|printf)'
0000000000004040 B buf
0000000000004010 D counter
000000000000115b T main
                 U printf@GLIBC_2.2.5
0000000000004014 D shared
0000000000001149 T twice
```

`main` 是 `0x115b`、`twice` 是 `0x1149`，差 `0x12`，跟 `.o` 裡 `main` 的 `Value` 一樣——整段 `.text` 被搬到 `0x1149` 附近，裡面的相對位置不變。`printf` 仍然是 `U`，原因見第 1 課第五節（動態連結，執行時才接）。

**符號從哪裡來：組合語言的標籤。** `gcc -S sym.c` 看得到每個 C 的函式、全域變數都變成一個標籤（`counter:`、`twice:`、`main:`）加一行 `.globl`（第 1 課第三節看過 `.globl main`）。組譯器把標籤變成符號表裡的符號，`.globl` 讓它變成 `GLOBAL`。字串的標籤叫 `.LC0`：組譯器規定 `.L` 開頭的是**區域標籤（local label）**，只在組譯時用，不寫進符號表，所以上面的符號表裡找不到 `.LC0`，只能用 `.rodata` 加偏移來指它。

一句話：**符號就是「連結器看得到的名字」——函式和全域變數的名字，加上它在哪個區段、哪個位置；區域變數沒有，因為它的位址不用連結器決定。**

*（追問補充結束，以下回到主線）*

---

kernel 也是一樣：每個 `.c` 各自編成 `.o`，大部分函式都寫成 `static`（只給自己的檔案用），要給別的模組用的才特別匯出；kernel 的建置系統（Kbuild）也是建立在 `make` 上的（本課只提一句，K0 才會實際碰到）。

---

## 二、宣告與定義：沒有宣告，編譯器只能猜

先分清兩個詞：

| | 是什麼 | 例子 | 整個程式可以有幾個 |
|---|---|---|---|
| **定義**（definition） | 真正產生東西：函式的本體（機器碼）、變數的儲存空間 | `long square(long x) { return x * x; }` | 一個 |
| **宣告**（declaration） | 只告訴編譯器「有這個名字，它的型別是這樣」，不產生東西 | `long square(long x);`（結尾是分號、沒有 `{}`） | 幾個都可以（型別要一致） |

函式的宣告也叫**原型**（prototype）。定義同時也是一個宣告（看到定義，編譯器當然也知道型別了）。

### 實驗：忘了宣告，`square(100000)` 印出 1410065408

`examples/implicit/`：`mathx.c`（檔案：[`mathx.c`](../small-project/s1-06-multi-file/examples/implicit/mathx.c)）

```c
long square(long x)
{
    return x * x;
}
```

`main.c`（檔案：[`main.c`](../small-project/s1-06-multi-file/examples/implicit/main.c)），**沒有** `square` 的宣告：

```c
#include <stdio.h>

int main(void)
{
    long r = square(100000);
    printf("%ld\n", r);
    return 0;
}
```

```
$ gcc -c main.c
main.c: In function ‘main’:
main.c:5:14: warning: implicit declaration of function ‘square’ [-Wimplicit-function-declaration]
    5 |     long r = square(100000);
      |              ^~~~~~
$ gcc -c mathx.c
$ gcc -o calc main.o mathx.o && ./calc
1410065408
```

100000 × 100000 ＝ 10000000000（一百億），印出來卻是 1410065408。沒有任何錯誤，只有一個警告。

**原因**：編譯 `main.c` 時，gcc 沒看過 `square`，只好照舊式 C 的規則**猜**它回傳 `int`（隱式宣告，implicit declaration；第 1 課[第九節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#九作業解說四題背後的原因)〈第 2 題〉看過同一件事）。看 `main.o` 的組合語言就知道它怎麼用這個猜測：

```
$ objdump -dr --no-show-raw-insn main.o
0000000000000000 <main>:
   0:	endbr64
   4:	push   %rbp
   5:	mov    %rsp,%rbp
   8:	sub    $0x10,%rsp
   c:	mov    $0x186a0,%edi
  11:	mov    $0x0,%eax
  16:	call   1b <main+0x1b>
			17: R_X86_64_PLT32	square-0x4
  1b:	cltq
  1d:	mov    %rax,-0x8(%rbp)
  ...
```

| 指令 | 在做什麼 |
|---|---|
| `mov $0x186a0,%edi` | 第 1 個參數 ＝ 100000（`0x186a0`）。猜成 `int` 參數，所以只寫 `%edi`（寫 32 位元會清高位，第 4 課） |
| `mov $0x0,%eax` | 沒有原型的函式，gcc 照可變參數函式的規矩把 `%al` 設成 0（第 5 課第七節） |
| `call 1b` ＋ `R_X86_64_PLT32 square` | 呼叫 `square`；位址還沒填，是重定位項目（第 1 課第四節的「洞」，`-r` 第 5 課教過） |
| **`cltq`** | 把 `%eax` **符號擴展**成 `%rax`，等於第 4 課的 `movslq %eax,%rax`（`cltq` 是 AT&T 對這個特定組合的專用寫法：convert long to quad） |
| `mov %rax,-0x8(%rbp)` | 存進 `r` |

`square` 本身是照 `long` 寫的，`imul` 算出完整的 10000000000 放在 `%rax`（`0x2540be400`）。可是 `main` 以為回傳值是 `int`，`cltq` **只拿 `%eax` 的低 32 位元**，再擴展回 64 位元：

```
10000000000 = 0x2_540be400
低 32 位元   =   0x540be400 = 1410065408（最高位元是 0，擴展後還是正數）
```

**被呼叫的那邊是對的，呼叫的那邊用錯的型別解讀結果**。編譯 `main.c` 時看不到 `mathx.c`，連結時連結器只比對名字 `square`（第 1 課），沒有任何一個階段知道兩邊型別不一樣。

（本機 gcc 13 預設 C17，隱式宣告只是警告；據 gcc 14 的變更說明，新版預設會變成錯誤，本機未實測。這也是為什麼編譯時一定要開 `-Wall` 而且不能忽略警告。）

---

## 三、標頭檔：把宣告寫一次，讓兩邊都檢查

修法是讓 `main.c` 看到正確的宣告。宣告直接寫在 `main.c` 裡也行，但有 10 個檔案要用 `square` 就要寫 10 次，改型別時要改 10 個地方。所以把宣告放進一個**標頭檔**（header file），要用的檔案 `#include` 它。

`examples/header/`：`mathx.h`（檔案：[`mathx.h`](../small-project/s1-06-multi-file/examples/header/mathx.h)）

```c
#ifndef MATHX_H
#define MATHX_H

long square(long x);

#endif
```

（`#ifndef`／`#define`／`#endif` 這三行是 include guard，本節後面解釋。）

`mathx.c`（檔案：[`mathx.c`](../small-project/s1-06-multi-file/examples/header/mathx.c)）——**定義的那個檔案自己也 `#include` 自己的標頭檔**：

```c
#include "mathx.h"

long square(long x)
{
    return x * x;
}
```

`main.c`（檔案：[`main.c`](../small-project/s1-06-multi-file/examples/header/main.c)）：

```c
#include <stdio.h>
#include "mathx.h"

int main(void)
{
    long r = square(100000);
    printf("%ld\n", r);
    return 0;
}
```

```
$ gcc -Wall -c main.c mathx.c
$ gcc -o calc main.o mathx.o && ./calc
10000000000
```

`gcc -c` 後面可以接好幾個 `.c`，它會**各自**編成同名的 `.o`（還是一次一個編譯單元，只是一個指令做完）。

### `#include` 只是貼上

第 1 課[第二節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#二前處理include-只是複製貼上)教過：前處理器把 `#include` 那一行換成檔案的內容。用 `gcc -E` 看 `main.c` 前處理後的最後幾行：

```
$ gcc -E main.c | tail -11

# 4 "mathx.h"
long square(long x);
# 3 "main.c" 2

int main(void)
{
    long r = square(100000);
    printf("%ld\n", r);
    return 0;
}
```

（全部 827 行，前面幾百行都是 `stdio.h` 貼進來的。`# 4 "mathx.h"` 這種行是前處理器留給編譯器的位置標記，讓錯誤訊息能指回原本的檔名與行號。）

所以真正讓編譯器知道型別的，是貼進來的那一行 `long square(long x);`。標頭檔沒有任何魔法，它就是**被很多 `.c` 共用的一段文字**。

### 定義的那個 `.c` 為什麼也要 `#include` 自己的標頭檔

這樣編譯器在編 `mathx.c` 時，會同時看到宣告和定義，**兩者不一致就報錯**。故意把定義改成 `int`：`bad.c`（檔案：[`bad.c`](../small-project/s1-06-multi-file/examples/header/bad.c)）

```c
#include "mathx.h"

int square(int x)
{
    return x * x;
}
```

```
$ gcc -c bad.c
bad.c:3:5: error: conflicting types for ‘square’; have ‘int(int)’
    3 | int square(int x)
      |     ^~~~~~
In file included from bad.c:1:
mathx.h:4:6: note: previous declaration of ‘square’ with type ‘long int(long int)’
    4 | long square(long x);
      |      ^~~~~~
```

這就是標頭檔的價值：**使用的那邊（`main.c`）和定義的那邊（`mathx.c`）看的是同一份宣告**，任何一邊跟它不一致，編譯時就會被抓到。
如果 `mathx.c` 沒有 `#include "mathx.h"`，改了定義卻忘了改標頭檔，`main.c` 會照舊的宣告呼叫——又回到第二節「連結成功、結果錯誤」的情況。

**標頭檔放什麼**：宣告（函式原型、`extern` 變數，第六節）、型別定義（`struct`、`typedef`）、巨集（`#define`）。
**不放什麼**：函式的定義、變數的定義——標頭檔會被貼進很多個 `.c`，定義跟著被貼了幾次，連結時就有幾份（第四、六節會看到這個錯誤）。

### `"..."` 與 `<...>`：去哪裡找標頭檔

| 寫法 | 先找哪裡 | 再找哪裡 | 用在 |
|---|---|---|---|
| `#include "mathx.h"` | **寫這行 `#include` 的那個檔案所在的資料夾** | `-iquote` 指定的資料夾，然後跟 `<...>` 一樣 | 自己專案的標頭檔 |
| `#include <stdio.h>` | `-I` 指定的資料夾（照命令列的順序） | 系統資料夾（下面這份清單） | 系統與函式庫的標頭檔 |

系統資料夾可以問 gcc（`-v` 印出詳細過程，`-E` 只做前處理，`-x c -` 表示從標準輸入讀 C 程式，`echo` 給它一個空的程式）：

```
$ echo | gcc -v -E -x c - 2>&1 | sed -n '/#include "..." search/,/End of search/p'
#include "..." search starts here:
#include <...> search starts here:
 /usr/lib/gcc/x86_64-linux-gnu/13/include
 /usr/local/include
 /usr/include/x86_64-linux-gnu
 /usr/include
End of search list.
```

`#include "..."` 那一段是空的，因為沒給 `-iquote`；「目前檔案所在的資料夾」是固定先找的，不列在這裡。

把 `mathx.h` 用角括號引入會找不到：`inc.c`（檔案：[`inc.c`](../small-project/s1-06-multi-file/examples/header/inc.c)）

```c
#include <mathx.h>
```

```
$ gcc -c inc.c
inc.c:1:10: fatal error: mathx.h: No such file or directory
    1 | #include <mathx.h>
      |          ^~~~~~~~~
compilation terminated.
$ gcc -I. -c inc.c && echo ok
ok
```

`-I.` 把目前資料夾（`.`）加進 `<...>` 的搜尋清單，就找得到了。大專案常把所有標頭檔放在 `include/` 資料夾，編譯時加 `-Iinclude`（kernel 也是這樣）。

### include guard：同一個標頭檔被貼兩次

函式的宣告重複幾次都沒關係，但 `struct` 的**定義**在同一個編譯單元裡只能出現一次。標頭檔又會引入別的標頭檔，很容易「繞了兩條路貼進來兩次」。

`examples/guard/without/`：`point.h`（檔案：[`point.h`](../small-project/s1-06-multi-file/examples/guard/without/point.h)），沒有 guard：

```c
struct point {
    long x;
    long y;
};
```

`draw.h`（檔案：[`draw.h`](../small-project/s1-06-multi-file/examples/guard/without/draw.h)）與 `move.h`（檔案：[`move.h`](../small-project/s1-06-multi-file/examples/guard/without/move.h)）都需要 `struct point`：

```c
#include "point.h"
void draw(struct point p);
```

```c
#include "point.h"
struct point move_right(struct point p, long dx);
```

`guard_main.c`（檔案：[`guard_main.c`](../small-project/s1-06-multi-file/examples/guard/without/guard_main.c)）兩個都要用：

```c
#include "draw.h"
#include "move.h"

int main(void)
{
    struct point p = { 1, 2 };
    (void)p;
    return 0;
}
```

（`(void)p;` 只是「用一下 `p`」，避免 `-Wall` 警告變數沒用到。）

```
$ gcc -c guard_main.c
In file included from move.h:1,
                 from guard_main.c:2:
point.h:1:8: error: redefinition of ‘struct point’
    1 | struct point {
      |        ^~~~~
In file included from draw.h:1,
                 from guard_main.c:1:
point.h:1:8: note: originally defined here
    1 | struct point {
      |        ^~~~~
```

錯誤訊息把兩條路都印出來了：`guard_main.c` 第 1 行 → `draw.h` → `point.h`，以及 `guard_main.c` 第 2 行 → `move.h` → `point.h`。`gcc -E guard_main.c` 也看得到 `struct point { ... }` 被貼了兩次。

`examples/guard/with/point.h`（檔案：[`point.h`](../small-project/s1-06-multi-file/examples/guard/with/point.h)）加上 guard，其他三個檔案不變：

```c
#ifndef POINT_H
#define POINT_H

struct point {
    long x;
    long y;
};

#endif
```

```
$ gcc -c guard_main.c && echo OK
OK
$ gcc -E guard_main.c | grep -c "struct point {"
1
```

三行的意思（都是前處理器的指令，第 1 課第二節）：

| 行 | 意思 |
|---|---|
| `#ifndef POINT_H` | 如果 `POINT_H` 這個巨集**還沒定義**（if not defined），保留到 `#endif` 為止的內容；已經定義了就整段丟掉 |
| `#define POINT_H` | 定義它（內容是空的，只是做個記號） |
| `#endif` | 結束 `#ifndef` 的範圍 |

第一次貼進來時 `POINT_H` 還沒定義，內容保留，同時定義了 `POINT_H`；第二次貼進來時 `POINT_H` 已經定義，整段被丟掉。
巨集名字自己取，慣例是檔名大寫、`.` 換成 `_`，只要整個專案裡不重複就好。**每個標頭檔都要加**，不要等出錯才加。
（另一種寫法是在檔案開頭寫 `#pragma once`，gcc 支援，但它不是 C 標準的一部分；kernel 用的是 `#ifndef` 這種寫法。）

---

## 四、符號表：`.o` 之間靠名字接起來

第 1 課[第四節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#四組譯機器碼裡的洞)用 `nm` 看過符號表：`T` 是「這個檔案提供」，`U` 是「這個檔案需要別人提供」。本課改用 `readelf -s`，它多一欄 **Bind**，第五節要用。

```
$ readelf -s main.o mathx.o

File: main.o

Symbol table '.symtab' contains 7 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS main.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 .rodata
     4: 0000000000000000    60 FUNC    GLOBAL DEFAULT    1 main
     5: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND square
     6: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND printf

File: mathx.o

Symbol table '.symtab' contains 4 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS mathx.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000    22 FUNC    GLOBAL DEFAULT    1 square
```

每一欄（`readelf` 的全部選項見 [readelf完整選項參考](./readelf完整選項參考.md)，`-s` 在[第 2.3 節〈符號〉](./readelf完整選項參考.md#23-符號)）：

| 欄 | 意思 | 本例 |
|---|---|---|
| `Num` | 第幾個符號 | 第 0 個永遠是空的（ELF 規定的佔位） |
| `Value` | 在 `.o` 裡是「在它所屬區段裡的偏移」；連結後的執行檔裡是位址 | `main`、`square` 都在各自 `.text` 的開頭，所以是 0 |
| `Size` | 佔幾 byte | `main` 60 byte、`square` 22 byte |
| `Type` | `FUNC` 函式、`OBJECT` 變數、`SECTION` 區段本身、`FILE` 原始檔名、`NOTYPE` 不知道（未定義的符號，這個檔案只知道名字） | `square` 在 `main.o` 是 `NOTYPE`，在 `mathx.o` 是 `FUNC` |
| `Bind` | **`GLOBAL`：別的檔案看得到、可以拿來配對；`LOCAL`：只在這個檔案裡有效**（還有一種 `WEAK`，本課不用） | 目前都是 `GLOBAL`；第五節的 `static` 會變 `LOCAL` |
| `Vis` | 可見度，給共享函式庫用的進一步限制 | 本課都是 `DEFAULT`，第 7 課才會用到 |
| `Ndx` | 屬於第幾個區段（`readelf -S` 的編號）；**`UND` ＝ undefined，這個檔案沒有定義、需要別人提供**；`ABS` ＝ 不屬於任何區段；`COM` 第六節 | `main.o` 的 `square`、`printf` 是 `UND` |
| `Name` | 名字 | |

連結器做的「符號解析」就是：把每個 `.o` 的 `UND` 拿去跟別的 `.o` 的 `GLOBAL` 定義配對。**它只比對 `Name` 這一欄**——`.o` 的符號表裡根本沒有 C 的型別（`Type` 只分函式、變數，沒有 `long(long)` 這種資訊）。這就是第二節的 bug 沒有被連結器發現的原因。

### 三種錯誤，各發生在哪一步

| 錯誤訊息 | 誰報的 | 原因 | 例子 |
|---|---|---|---|
| `conflicting types for ‘square’` | **編譯器**（`gcc -c` 時） | 同一個編譯單元裡，兩個宣告（或宣告與定義）的型別不一樣 | 第三節 `bad.c` |
| `undefined reference to ‘square’` | **連結器**（`ld`） | 有 `UND`，但所有 `.o` 和函式庫裡都沒有 `GLOBAL` 的定義 | 下面實驗 1 |
| `multiple definition of ‘square’` | **連結器** | 兩個 `.o` 都有 `GLOBAL` 的定義，不知道要用哪個 | 下面實驗 2 |

實驗 1：只連結 `main.o`，不給 `mathx.o`：

```
$ gcc -o x main.o
/usr/bin/ld: main.o: in function `main':
main.c:(.text+0x12): undefined reference to `square'
collect2: error: ld returned 1 exit status
```

`/usr/bin/ld` 開頭表示是連結器報的（`collect2` 是 gcc 用來呼叫 `ld` 的小程式，最後一行只是轉告「`ld` 失敗了」）。`main.c:(.text+0x12)` 是「`main.o` 的 `.text` 第 `0x12` byte 需要 `square`」——就是第二節 `objdump -dr` 看到的那個重定位項目的位置（`call` 指令從 `0x11` 開始，要填的 4 byte 從 `0x12` 開始）。

實驗 2：`mathx2.c`（檔案：[`mathx2.c`](../small-project/s1-06-multi-file/examples/header/mathx2.c)）是 `mathx.c` 原封不動的複製，一起連結：

```
$ gcc -o x main.o mathx.o mathx2.c
/usr/bin/ld: /tmp/ccVohvCq.o: in function `square':
mathx2.c:(.text+0x0): multiple definition of `square'; mathx.o:mathx.c:(.text+0x0): first defined here
collect2: error: ld returned 1 exit status
```

（`gcc` 的命令列可以混著給 `.o` 和 `.c`；`.c` 會先編成 `/tmp/` 底下的暫存 `.o`，所以第一行是一個隨機的暫存檔名。）兩份定義內容一模一樣也不行：連結器不比較內容，有兩個同名的 `GLOBAL` 定義就是錯。

**看錯誤訊息反推原因**：開頭是 `檔名:行:欄: error:` → 編譯時，看那一行的 C；開頭是 `/usr/bin/ld:` → 連結時，問題在「哪些 `.o` 一起連結」、「誰定義了什麼」，用 `readelf -s`／`nm` 查每個 `.o` 的符號表。

---

## 五、`static`：兩種完全不同的意思

`static` 這個關鍵字寫在不同位置，意思完全不一樣：

| 寫在哪裡 | 例子 | 意思 |
|---|---|---|
| **檔案層級**（函式外面）的函式或變數前面 | `static int helper(void) { ... }` | 這個名字**只在這個 `.c` 裡有效**，符號變成 `LOCAL`，別的檔案看不到、也不會跟別的檔案的同名符號衝突 |
| **函式裡面**的區域變數前面 | `static int n = 0;` | 這個變數**不放在堆疊上**，整個程式執行期間只有一份，值會留到下一次呼叫 |

### 檔案層級的 `static`：讓名字變成 `LOCAL`

`examples/static/`：`a.c`（檔案：[`a.c`](../small-project/s1-06-multi-file/examples/static/a.c)）與 `b.c`（檔案：[`b.c`](../small-project/s1-06-multi-file/examples/static/b.c)）各有一個叫 `helper` 的函式：

```c
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
static int helper(void)
{
    return 2;
}

int b_value(void)
{
    return helper();
}
```

`ab_main.c`（檔案：[`ab_main.c`](../small-project/s1-06-multi-file/examples/static/ab_main.c)）：

```c
#include <stdio.h>

int a_value(void);
int b_value(void);

int main(void)
{
    printf("a_value() = %d, b_value() = %d\n", a_value(), b_value());
    return 0;
}
```

（這裡為了少一個檔案，把宣告直接寫在 `ab_main.c`；正式專案應該放進 `a.h`、`b.h`。）

```
$ gcc -c a.c b.c ab_main.c && gcc -o ab ab_main.o a.o b.o && ./ab
a_value() = 1, b_value() = 2
$ readelf -s a.o
...
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS a.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000    15 FUNC    LOCAL  DEFAULT    1 helper
     4: 000000000000000f    15 FUNC    GLOBAL DEFAULT    1 a_value
$ nm a.o
000000000000000f T a_value
0000000000000000 t helper
```

`helper` 的 Bind 是 **`LOCAL`**；`nm` 用**小寫** `t` 表示（小寫 ＝ local，大寫 ＝ global；`nm` 符號字母的完整清單見 `man nm`）。
連結器配對時不看 `LOCAL` 符號，所以兩個 `helper` 互不干擾。連結後的執行檔裡兩個都還在，各自有位址：

```
$ nm ab | grep -E "helper|_value"
0000000000001194 T a_value
00000000000011b2 T b_value
0000000000001185 t helper
00000000000011a3 t helper
```

把兩個 `static` 拿掉（`sed 's/^static //' a.c > a2.c`，`b2.c` 同樣做法），就變成兩個 `GLOBAL` 的 `helper`：

```
$ gcc -c a2.c b2.c
$ readelf -s a2.o | grep helper
     3: 0000000000000000    15 FUNC    GLOBAL DEFAULT    1 helper
$ gcc -o ab2 ab_main.o a2.o b2.o
/usr/bin/ld: b2.o: in function `helper':
b2.c:(.text+0x0): multiple definition of `helper'; a2.o:a2.c:(.text+0x0): first defined here
collect2: error: ld returned 1 exit status
```

反過來，別的檔案想呼叫 `static` 的 `helper` 也不行：`callhelper.c`（檔案：[`callhelper.c`](../small-project/s1-06-multi-file/examples/static/callhelper.c)）

```c
int helper(void);       /* 想呼叫 a.c 裡的 static helper */

int main(void)
{
    return helper();
}
```

```
$ gcc -o ch callhelper.c a.c
/usr/bin/ld: /tmp/cc92gUUI.o: in function `main':
callhelper.c:(.text+0x9): undefined reference to `helper'
collect2: error: ld returned 1 exit status
```

`a.o` 裡明明有 `helper`，但它是 `LOCAL`，不能拿來配對，所以是 `undefined reference`。

**什麼時候用**：只給自己檔案用的函式與全域變數**一律加 `static`**。好處是名字不會跟別的檔案撞、別人不會依賴你的內部細節、讀程式的人一看就知道「這個只在本檔案用」。kernel 的程式碼裡絕大部分函式都是 `static`。

### 函式裡的 `static`：不放堆疊，值留到下一次

`examples/staticlocal/nextid.c`（檔案：[`nextid.c`](../small-project/s1-06-multi-file/examples/staticlocal/nextid.c)）：

```c
#include <stdio.h>

int next_id(void)
{
    static int n = 0;   /* 只初始化一次，值留到下一次呼叫 */
    n = n + 1;
    return n;
}

int plain_id(void)
{
    int n = 0;          /* 每次呼叫都重新放在堆疊上、重新設成 0 */
    n = n + 1;
    return n;
}

int main(void)
{
    for (int i = 0; i < 3; i++)
        printf("next_id() = %d, plain_id() = %d\n", next_id(), plain_id());
    return 0;
}
```

```
$ gcc -g -O0 -o nextid nextid.c && ./nextid
next_id() = 1, plain_id() = 1
next_id() = 2, plain_id() = 1
next_id() = 3, plain_id() = 1
```

看兩個函式的組合語言（`gcc -O0 -c nextid.c`，`objdump -d --no-show-raw-insn nextid.o`）：

```
0000000000000000 <next_id>:
   0:	endbr64
   4:	push   %rbp
   5:	mov    %rsp,%rbp
   8:	mov    0x0(%rip),%eax        # e <next_id+0xe>
   e:	add    $0x1,%eax
  11:	mov    %eax,0x0(%rip)        # 17 <next_id+0x17>
  17:	mov    0x0(%rip),%eax        # 1d <next_id+0x1d>
  1d:	pop    %rbp
  1e:	ret

000000000000001f <plain_id>:
  1f:	endbr64
  23:	push   %rbp
  24:	mov    %rsp,%rbp
  27:	movl   $0x0,-0x4(%rbp)
  2e:	addl   $0x1,-0x4(%rbp)
  32:	mov    -0x4(%rbp),%eax
  35:	pop    %rbp
  36:	ret
```

- `plain_id` 的 `n` 在 `-0x4(%rbp)`：**堆疊框裡**（第 5 課第四節），每次呼叫都 `movl $0x0` 重新設成 0，函式回傳後那塊空間就不屬於它了。
- `next_id` 的 `n` 用 `0x0(%rip)` 存取：`D(%rip)` 是「相對於目前指令位址」的寫法（[第 4 課第三節](./x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.md#三指令的文法助憶符大小後綴運算元)〈上表其他五列：每一種用一個實際的 C 函式對照〉），指向**固定的一塊記憶體**，不在堆疊上。`0x0` 是還沒填的洞（連結時才填）。而且函式裡**沒有任何把它設成 0 的指令**——初值 0 是程式載入時就準備好的，之後每次呼叫只是讀、加 1、寫回。

符號表裡看得到它：

```
$ readelf -s nextid.o
...
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     ...
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    4 .bss
     4: 0000000000000000     4 OBJECT  LOCAL  DEFAULT    4 n.0
     ...
     6: 0000000000000000    31 FUNC    GLOBAL DEFAULT    1 next_id
     7: 000000000000001f    24 FUNC    GLOBAL DEFAULT    1 plain_id
     8: 0000000000000037    79 FUNC    GLOBAL DEFAULT    1 main
```

- `n.0`：4 byte 的 `OBJECT`（變數）、`LOCAL`，放在第 4 個區段 **`.bss`**。名字是 gcc 自己取的（加 `.0` 避免跟別的函式裡同名的 `static` 變數撞名）。
- `plain_id` 的 `n` 不在符號表裡：堆疊上的變數只是 `-0x4(%rbp)` 這種偏移，沒有名字可以給連結器。
- **`.bss`**：放「初值是 0」的全域變數與 `static` 變數。這些變數全部是 0，檔案裡不必存內容，只記「需要幾 byte」（`readelf -S` 看它的型別是 `NOBITS`），程式載入時才配一塊全是 0 的記憶體。初值不是 0 的（例如 `static int n = 5;`）放 `.data`，內容要存在檔案裡。（第 1 課〈ELF〉的圖裡看過這兩個區段名，`.data`／`.bss` 都在可寫的 `LOAD` 段。）

所以函式裡的 `static` 改的是**儲存期間**（storage duration）：普通區域變數是「自動」（automatic），呼叫時才在堆疊上出現、回傳就消失；`static` 區域變數是「靜態」（static），整個程式執行期間都在。它的**名字**仍然只在那個函式裡看得到。

---

## 六、`extern`：全域變數的宣告與定義

函式的宣告和定義很好分（有沒有 `{}`）。全域變數就不好分了：`long hits;` 看起來像宣告，其實是定義。要明確寫「只是宣告」，要加 **`extern`**。

| 寫法（寫在函式外面） | 是什麼 |
|---|---|
| `extern long hits;` | **宣告**：有一個 `long` 叫 `hits`，定義在別的地方。不配置空間 |
| `long hits = 0;` | **定義**：配置 8 byte，初值 0 |
| `long hits;` | **暫定定義**（tentative definition）：同一個檔案裡沒有別的定義時，就當成 `long hits = 0;`。所以實際上還是**定義** |

正確做法：**標頭檔寫 `extern` 宣告，恰好一個 `.c` 寫定義**。

`examples/extern/`：`hits.h`（檔案：[`hits.h`](../small-project/s1-06-multi-file/examples/extern/hits.h)）

```c
#ifndef HITS_H
#define HITS_H

extern long hits;       /* 宣告：hits 這個變數存在，定義在別的檔案 */
void hit(void);

#endif
```

`hits.c`（檔案：[`hits.c`](../small-project/s1-06-multi-file/examples/extern/hits.c)）

```c
#include "hits.h"

long hits = 0;          /* 定義：真正配置 8 byte 的地方，整個程式只能有一個 */

void hit(void)
{
    hits = hits + 1;
}
```

`hits_main.c`（檔案：[`hits_main.c`](../small-project/s1-06-multi-file/examples/extern/hits_main.c)）

```c
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

```
$ gcc -Wall -c hits.c hits_main.c && gcc -o hits hits_main.o hits.o && ./hits
hits = 3
$ readelf -s hits_main.o hits.o | grep -E "File| hits?$"
File: hits_main.o
     5: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND hit
     6: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND hits
File: hits.o
     3: 0000000000000000     8 OBJECT  GLOBAL DEFAULT    4 hits
     4: 0000000000000000    29 FUNC    GLOBAL DEFAULT    1 hit
```

`hits_main.o` 只有 `UND hits`（需要），`hits.o` 有 8 byte 的 `OBJECT hits`（提供，在第 4 區段 `.bss`，因為初值是 0）。跟函式完全同一套規則。

### 忘了 `extern`：每個 `.c` 都有一份定義

`examples/extern/bad/`：`hits.h`（檔案：[`hits.h`](../small-project/s1-06-multi-file/examples/extern/bad/hits.h)）把 `extern` 拿掉：

```c
#ifndef HITS_H
#define HITS_H

long hits;
void hit(void);

#endif
```

`hits.c`（檔案：[`hits.c`](../small-project/s1-06-multi-file/examples/extern/bad/hits.c)）也不寫自己的定義，只靠標頭檔：

```c
#include "hits.h"

/* 這一版沒有自己寫 long hits = 0;，只靠標頭檔裡的 long hits; */

void hit(void)
{
    hits = hits + 1;
}
```

`hits_main.c`（檔案：[`hits_main.c`](../small-project/s1-06-multi-file/examples/extern/bad/hits_main.c)）跟上面的一樣。

```
$ gcc -c hits.c hits_main.c
$ readelf -s hits.o hits_main.o | grep -E "File| hits$"
File: hits.o
     3: 0000000000000000     8 OBJECT  GLOBAL DEFAULT    4 hits
File: hits_main.o
     4: 0000000000000000     8 OBJECT  GLOBAL DEFAULT    4 hits
$ gcc -o hits hits_main.o hits.o
/usr/bin/ld: hits.o:(.bss+0x0): multiple definition of `hits'; hits_main.o:(.bss+0x0): first defined here
collect2: error: ld returned 1 exit status
```

標頭檔被貼進兩個 `.c`，每個 `.c` 各有一個暫定定義，各自變成一份 8 byte 的 `hits`——兩個 `GLOBAL` 定義，`multiple definition`。
注意 include guard 救不了這個：guard 只防止**同一個 `.c`** 裡貼兩次，兩個不同的 `.c` 本來就各貼一次。

**`-fcommon`：舊程式為什麼這樣寫也能動**

舊版 gcc 對「沒有初值的全域變數」有另一種處理：把它標成 **common** 符號（`Ndx` 是 `COM`，`nm` 印 `C`），連結器遇到好幾個同名的 common 符號時，會合併成一份，不報錯。這個行為現在要用 `-fcommon` 才會打開：

```
$ gcc -fcommon -c hits.c hits_main.c
$ readelf -s hits.o hits_main.o | grep -E "File| hits$"
File: hits.o
     3: 0000000000000008     8 OBJECT  GLOBAL DEFAULT  COM hits
File: hits_main.o
     4: 0000000000000008     8 OBJECT  GLOBAL DEFAULT  COM hits
$ gcc -o hits hits_main.o hits.o && ./hits
hits = 3
$ gcc -Q --help=common | grep -- "-fcommon"
  -fcommon                    		[disabled]
```

（`gcc -Q --help=common` 列出「各語言共通的選項」與目前的設定值，`-Q` 是「印出設定值」；`COM` 符號的 `Value` 欄存的是對齊要求，不是偏移。）
本機 gcc 13 預設是關閉的（`-fno-common`）；據 gcc 10 的變更說明，預設是從 gcc 10 開始改成關閉的（本機沒有更舊的 gcc，未實測）。所以很多舊專案升級 gcc 後突然出現一堆 `multiple definition`，就是這個原因。正確的修法是補上 `extern`，不是加 `-fcommon`。

**整理：全域變數放哪裡**

| 放在 | 寫 |
|---|---|
| 標頭檔 | `extern long hits;`（宣告，多少個 `.c` 引入都沒關係） |
| 恰好一個 `.c` | `long hits = 0;`（定義，這個 `.c` 也引入同一個標頭檔，讓編譯器檢查型別一致） |
| 只有自己檔案用 | `static long hits = 0;`（第五節，`LOCAL`，不寫進標頭檔） |

### 補充：`extern` 的三個邊界情況（2026-10-21 作業第 2 題批改時補教）

作業第 2 題碰到三件上面沒講的事，用 `examples/extern-rules/` 重做一次。`hits.h`、`hits.c`、`hits_main.c` 跟上面 `examples/extern/` 的一模一樣（檔案：[`hits.h`](../small-project/s1-06-multi-file/examples/extern-rules/hits.h)、[`hits.c`](../small-project/s1-06-multi-file/examples/extern-rules/hits.c)、[`hits_main.c`](../small-project/s1-06-multi-file/examples/extern-rules/hits_main.c)），原始碼見上面。

**1. 只有宣告、沒用到的變數，不會出現在符號表**

`unused.c`（檔案：[`unused.c`](../small-project/s1-06-multi-file/examples/extern-rules/unused.c)）引入了 `hits.h`，但完全沒用到 `hits`：

```c
#include "hits.h"       /* 看得到 extern long hits; 的宣告，但完全沒用到 hits */

int main(void)
{
    return 0;
}
```

```
$ gcc -Wall -c unused.c && readelf -s unused.o

Symbol table '.symtab' contains 4 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS unused.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000    15 FUNC    GLOBAL DEFAULT    1 main
```

沒有 `hits`，連 `UND` 都沒有。`UND` 是「機器碼裡有個洞要填這個名字」，沒有任何指令用到 `hits`，就沒有洞、也不需要跟連結器要它。**宣告只是告訴編譯器型別，本身不產生符號**；函式的宣告也一樣（`hits.h` 裡的 `hit` 在 `unused.o` 裡也沒有）。

**2. `-fcommon` 只合併「沒有初值」的定義**

`init_main.c`（檔案：[`init_main.c`](../small-project/s1-06-multi-file/examples/extern-rules/init_main.c)）自己又寫了一份有初值的定義：

```c
#include <stdio.h>
#include "hits.h"

long hits = 5;          /* 第二份「有初值」的定義：hits.c 已經有 long hits = 0; */

int main(void)
{
    hit();
    printf("hits = %ld\n", hits);
    return 0;
}
```

```
$ gcc -Wall -fcommon -c hits.c init_main.c
$ readelf -s hits.o init_main.o | grep -E "File| hits$"
File: hits.o
     3: 0000000000000000     8 OBJECT  GLOBAL DEFAULT    4 hits
File: init_main.o
     4: 0000000000000000     8 OBJECT  GLOBAL DEFAULT    3 hits
$ gcc -o x init_main.o hits.o
/usr/bin/ld: hits.o:(.bss+0x0): multiple definition of `hits'; init_main.o:(.data+0x0): first defined here
collect2: error: ld returned 1 exit status
```

加了 `-fcommon`，兩個 `hits` 的 `Ndx` 還是 `4`（`.bss`）、`3`（`.data`），不是 `COM`。上面〈`-fcommon`〉那段的 `COM` 只發生在 `long hits;` 這種**沒寫初值**的暫定定義上；只要寫了初值（就算是 `= 0`），就是一份確定的定義，`-fcommon` 不管它，兩份照樣 `multiple definition`。要合併也合理不了：兩份初值不一樣（5 和 0）時，連結器不知道該用哪個。

**3. 標頭檔說 `extern`、`.c` 又寫 `static`：編譯時就錯**

`static_h.c`（檔案：[`static_h.c`](../small-project/s1-06-multi-file/examples/extern-rules/static_h.c)）：

```c
#include "hits.h"       /* 標頭檔說 extern long hits;（別的檔案看得到的 hits） */

static long hits = 0;   /* 又說 hits 只在本檔案有效 */

void hit(void)
{
    hits = hits + 1;
}
```

```
$ gcc -Wall -c static_h.c
static_h.c:3:13: error: static declaration of ‘hits’ follows non-static declaration
    3 | static long hits = 0;   /* 又說 hits 只在本檔案有效 */
      |             ^~~~
In file included from static_h.c:1:
hits.h:4:13: note: previous declaration of ‘hits’ with type ‘long int’
    4 | extern long hits;       /* 宣告：hits 這個變數存在，定義在別的檔案 */
      |             ^~~~
```

同一個編譯單元裡，`hits` 先被宣告成「別的檔案看得到的」，又被定義成「只在本檔案有效」，兩個說法矛盾，編譯器直接拒絕——跟第三節 `conflicting types` 是同一個道理：**定義的 `.c` 引入自己的標頭檔，矛盾在編譯時就抓到**。

如果不引入標頭檔，矛盾就拖到連結時才出現。`static_noh.c`（檔案：[`static_noh.c`](../small-project/s1-06-multi-file/examples/extern-rules/static_noh.c)）：

```c
void hit(void);         /* 不引入 hits.h，自己寫 hit 的宣告 */

static long hits = 0;   /* 只在本檔案有效 → LOCAL */

void hit(void)
{
    hits = hits + 1;
}
```

```
$ gcc -Wall -c static_noh.c hits_main.c
$ readelf -s static_noh.o hits_main.o | grep -E "File| hits$"
File: static_noh.o
     4: 0000000000000000     8 OBJECT  LOCAL  DEFAULT    4 hits
File: hits_main.o
     6: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND hits
$ gcc -o y hits_main.o static_noh.o
/usr/bin/ld: hits_main.o: warning: relocation against `hits' in read-only section `.text'
/usr/bin/ld: hits_main.o: in function `main':
hits_main.c:(.text+0x1a): undefined reference to `hits'
/usr/bin/ld: warning: creating DT_TEXTREL in a PIE
collect2: error: ld returned 1 exit status
```

`static_noh.o` 的 `hits` 是 `LOCAL`，不能拿來配對，`hits_main.o` 的 `UND hits` 找不到 `GLOBAL` 的定義，所以是 `undefined reference`——跟第五節 `callhelper.c` 呼叫不到 `static helper` 是同一件事，只是這次換成變數。（中間兩行 `warning: relocation ... DT_TEXTREL in a PIE` 是連結失敗時順帶出現的，跟位置無關程式碼（PIE）有關，第 7 課講共享函式庫時再說，這裡只看 `undefined reference` 那一行。）

---

## 七、`make`：只重編改過的檔案

檔案一多，每次都 `gcc -c` 全部重編很慢（kernel 全部重編要幾十分鐘）。分開編譯的好處是：**改了 `mathx.c`，只要重編 `mathx.o` 再重新連結**，`main.o` 不必動。`make` 就是自動做這個判斷的工具。

### 文法：規則

`Makefile` 由**規則**（rule）組成：

```
目標 … : 先決條件 …
<TAB>命令
<TAB>命令
```

| 位置 | 是什麼 |
|---|---|
| **目標**（target） | 這條規則要產生的東西，通常是檔名（`calc`、`main.o`）；冒號左邊可以有好幾個 |
| `:` | 分隔目標與先決條件（固定符號） |
| **先決條件**（prerequisites） | 產生目標需要用到的檔案，空格分開；可以是空的 |
| **命令**（recipe） | 怎麼從先決條件產生目標，交給 shell 執行的指令。**每一行都必須用 TAB 開頭**（不能用空格），可以有好幾行 |

**`make` 怎麼決定要不要執行命令**：目標不存在，或任何一個先決條件的**修改時間比目標新**，就執行命令；否則跳過。先決條件本身如果也是別的規則的目標，先遞迴處理它。

### 實驗：第一個 `Makefile`

`examples/make/`：`mathx.h`（檔案：[`mathx.h`](../small-project/s1-06-multi-file/examples/make/mathx.h)），比第三節多一個巨集：

```c
#ifndef MATHX_H
#define MATHX_H

#define DEFAULT_N 7     /* main 要算平方的數字 */

long square(long x);

#endif
```

`mathx.c`（檔案：[`mathx.c`](../small-project/s1-06-multi-file/examples/make/mathx.c)）與第三節相同：

```c
#include "mathx.h"

long square(long x)
{
    return x * x;
}
```

`main.c`（檔案：[`main.c`](../small-project/s1-06-multi-file/examples/make/main.c)）：

```c
#include <stdio.h>
#include "mathx.h"

int main(void)
{
    printf("square(%d) = %ld\n", DEFAULT_N, square(DEFAULT_N));
    return 0;
}
```

先寫一個每個檔案都寫死的版本（這個版本只在本節示範，沒有另外存檔；命令前面是 TAB）：

```make
calc: main.o mathx.o
	gcc -o calc main.o mathx.o

main.o: main.c mathx.h
	gcc -Wall -c main.c

mathx.o: mathx.c mathx.h
	gcc -Wall -c mathx.c

clean:
	rm -f calc main.o mathx.o

.PHONY: clean
```

```
$ make
gcc -Wall -c main.c
gcc -Wall -c mathx.c
gcc -o calc main.o mathx.o
$ make
make: 'calc' is up to date.
$ touch mathx.c
$ make
gcc -Wall -c mathx.c
gcc -o calc main.o mathx.o
$ touch mathx.h
$ make
gcc -Wall -c main.c
gcc -Wall -c mathx.c
gcc -o calc main.o mathx.o
$ make mathx.o
gcc -Wall -c mathx.c
```

（`touch` 把檔案的修改時間改成現在，內容不變，等於「假裝改過」。）

1. 第一次 `make`：沒給目標名字，就做檔案裡**第一條規則的目標** `calc`。`calc` 需要 `main.o`、`mathx.o`，兩個都不存在，先做它們，再連結。`make` 會先把每一行命令印出來再執行。
2. 第二次：`calc` 比 `main.o`、`mathx.o` 新，`main.o` 比 `main.c`、`mathx.h` 新……全部都不用做。
3. `touch mathx.c`：只有 `mathx.o` 比它舊，重編 `mathx.o`；`mathx.o` 變新了，`calc` 也要重新連結；`main.o` 不動。
4. `touch mathx.h`：兩個 `.o` 的先決條件都有 `mathx.h`，兩個都重編。
5. `make mathx.o`：命令列指定目標，只做這一個。

**命令一定要用 TAB**：把 TAB 換成 4 個空格，`make` 會報

```
Makefile:2: *** missing separator.  Stop.
```

（`Makefile:2` 是第 2 行。很多編輯器會自動把 TAB 換成空格，寫 `Makefile` 時要注意。）

### 先決條件漏了標頭檔：`make` 說不用重編，結果是舊的

`Makefile.nodeps`（檔案：[`Makefile.nodeps`](../small-project/s1-06-multi-file/examples/make/Makefile.nodeps)）跟上面一樣，只是先決條件**沒寫 `mathx.h`**：

```make
calc: main.o mathx.o
	gcc -o calc main.o mathx.o

main.o: main.c
	gcc -Wall -c main.c

mathx.o: mathx.c
	gcc -Wall -c mathx.c
```

```
$ make -f Makefile.nodeps && ./calc
gcc -Wall -c main.c
gcc -Wall -c mathx.c
gcc -o calc main.o mathx.o
square(7) = 49
$ sed -i 's/DEFAULT_N 7 /DEFAULT_N 9 /' mathx.h
$ grep DEFAULT_N mathx.h | head -1
#define DEFAULT_N 9     /* main 要算平方的數字 */
$ make -f Makefile.nodeps
make: 'calc' is up to date.
$ ./calc
square(7) = 49
```

（`-f 檔名` 指定要讀哪個 makefile，不給就找 `GNUmakefile`、`makefile`、`Makefile`。`sed -i` 直接修改檔案。）

標頭檔改成 9 了，`make` 卻說不用做，程式還印 7。因為 `make` **只看 `Makefile` 寫的先決條件**，它不會自己去讀 `main.c` 有 `#include` 哪些檔案。`main.o` 的先決條件只有 `main.c`，`main.c` 沒變，就不重編。
`DEFAULT_N` 是巨集，前處理時就被換成數字寫死在 `main.o` 裡，不重編就永遠是 7。

這種錯最難查：程式碼看起來是對的，跑出來的卻是舊版。**先決條件一定要列完整**；但手動維護「每個 `.c` 引入了哪些 `.h`」很容易漏，所以讓 gcc 自動產生**這份相依清單**（不是整個 `Makefile`：gcc 只寫出「`main.o` 依賴哪些檔案」的 `.d` 檔，再由 `Makefile` 讀進來，見下一小節[〈實用的 `Makefile`〉](#實用的-makefile變數樣式規則自動產生相依)第 5、6 點）。

### 實用的 `Makefile`：變數、樣式規則、自動產生相依

`Makefile`（檔案：[`Makefile`](../small-project/s1-06-multi-file/examples/make/Makefile)），命令前面是 TAB：

```make
CC     = gcc
CFLAGS = -Wall -g -O0 -MMD -MP
OBJS   = main.o mathx.o

calc: $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

clean:
	rm -f calc $(OBJS) $(OBJS:.o=.d)

.PHONY: clean
```

（先把 `mathx.h` 改回 7，`rm -f *.o calc` 清掉舊的產物。）

```
$ make && ./calc
gcc -Wall -g -O0 -MMD -MP -c main.c -o main.o
gcc -Wall -g -O0 -MMD -MP -c mathx.c -o mathx.o
gcc -o calc main.o mathx.o
square(7) = 49
$ cat main.d
main.o: main.c mathx.h
mathx.h:
$ sed -i 's/DEFAULT_N 7 /DEFAULT_N 9 /' mathx.h
$ make && ./calc
gcc -Wall -g -O0 -MMD -MP -c main.c -o main.o
gcc -Wall -g -O0 -MMD -MP -c mathx.c -o mathx.o
gcc -o calc main.o mathx.o
square(9) = 81
```

這次改了標頭檔，兩個 `.o` 都重編了。逐行解釋：

**1. 變數**：`CC = gcc` 定義變數，`$(CC)` 取用（也可以寫 `${CC}`）。`CC`、`CFLAGS` 是慣用的名字（C 編譯器、C 編譯選項），`make` 的內建規則也用這兩個名字。
變數的賦值有 5 種寫法（GNU make 手冊 6.2、6.5～6.6 節）：

| 寫法 | 意思 |
|---|---|
| `X = 值` | **遞迴展開**：存的是「文字」，每次 `$(X)` 被用到時才展開裡面的變數（所以可以引用後面才定義的變數） |
| `X := 值`（或 `::=`） | **立即展開**：定義的那一刻就把裡面的變數展開，之後不再變 |
| `X ?= 值` | 只有 `X` 還沒定義時才設（常用來給預設值，讓命令列或環境變數可以蓋過） |
| `X += 值` | 在原本的值後面加一個空格再接上 |
| `X != 指令` | 執行 shell 指令，把輸出存進來 |

命令列也可以蓋掉變數：`make CFLAGS=-O2` 會讓 `Makefile` 裡的 `CFLAGS = ...` 失效（命令列優先）。

**2. 自動變數**：在命令裡代表「這條規則的某個部分」，`make` 執行每條規則時自動設好。全部 8 個（GNU make 手冊 10.5.3 節）：

| 自動變數 | 代表 | 本例 |
|---|---|---|
| `$@` | 目標 | `calc`、`main.o` |
| `$<` | 第一個先決條件 | `main.c` |
| `$^` | 全部先決條件（去掉重複） | `main.o mathx.o` |
| `$+` | 全部先決條件（保留重複，照原順序） | — |
| `$?` | 比目標新的那些先決條件 | — |
| `$*` | 樣式規則裡 `%` 對到的部分（「主幹」，stem） | `main`（做 `main.o` 時） |
| `$\|` | 「只看存在、不看時間」的先決條件（order-only，寫在 `\|` 後面） | — |
| `$%` | 目標是函式庫成員 `lib.a(x.o)` 時的成員名 | —（第 7 課的靜態函式庫才會碰到） |

每個都可以加 `D`／`F` 取資料夾或檔名部分，例如 `$(@D)`、`$(<F)`。

**3. 樣式規則**（pattern rule）：`%.o: %.c` 的 `%` 對到任何字串，意思是「任何 `X.o` 都可以從 `X.c` 這樣產生」。有了它，每多一個 `.c` 只要加進 `OBJS`，不必再寫一條規則。
其實 `make` 本身就內建了這條規則（`make -p` 印出內部資料庫，`-f /dev/null` 表示不讀任何 makefile，只看內建的）：

```
$ make -p -f /dev/null 2>/dev/null | grep -A3 '^%.o: %.c'
%.o: %.c
#  recipe to execute (built-in):
	$(COMPILE.c) $(OUTPUT_OPTION) $<
...
$ make -p -f /dev/null 2>/dev/null | grep -E "^COMPILE.c|^OUTPUT_OPTION|^CC ="
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
OUTPUT_OPTION = -o $@
CC = cc
```

內建的 `CC` 是 `cc`（在 Ubuntu 上 `cc` 是指向 gcc 的連結）。自己寫一次是為了讓規則看得見，也避免依賴內建規則（`make -r` 可以關掉所有內建規則）。

**4. `$(OBJS:.o=.d)`**：**替換參照**（substitution reference），把 `OBJS` 裡每個字結尾的 `.o` 換成 `.d`，得到 `main.d mathx.d`。

**5. `-MMD -MP`：讓 gcc 寫出相依檔**。這兩個是 gcc 的選項，跟 `make` 無關：

| 選項 | 做什麼 |
|---|---|
| `-MMD` | 編譯的同時，另外寫一個 `.d` 檔，內容是 `make` 的規則格式：「這個 `.o` 依賴哪些檔案」。**只列使用者的標頭檔**，不列系統標頭檔（`stdio.h` 那些） |
| `-MD` | 跟 `-MMD` 一樣，但連系統標頭檔也列（本機實測 `main.c` 的 `.d` 有 24 行，全是 `/usr/include/...`） |
| `-MP` | 替每個標頭檔多寫一條**沒有先決條件、沒有命令的規則**（上面 `main.d` 的第 2 行 `mathx.h:`）。萬一之後把 `mathx.h` 刪掉或改名，`make` 不會因為「`main.o` 依賴的 `mathx.h` 不存在、又沒有規則能產生它」而停下來 |

`main.d` 的第 1 行 `main.o: main.c mathx.h` 就是我們在 `Makefile.nodeps` 漏掉的那條相依。gcc 在前處理時本來就知道 `#include` 了哪些檔案，由它寫出來最準。（gcc 的 `-M` 系列還有 `-M`、`-MM`、`-MF`、`-MT`、`-MQ`、`-MG` 等，完整說明在 `man gcc` 的 *Preprocessor Options*。）

**6. `-include $(OBJS:.o=.d)`**：把 `main.d`、`mathx.d` 的內容讀進來，當成 `Makefile` 的一部分。`include` 前面的 `-` 表示「檔案不存在也不要報錯」——第一次 `make` 時 `.d` 還沒產生（那時 `.o` 也不存在，反正都要編）。
同一個目標可以出現在好幾條規則裡：`%.o: %.c` 給了命令，`main.d` 的 `main.o: main.c mathx.h` 補上更多先決條件，`make` 會把它們合起來。

**7. `clean` 與 `.PHONY`**：`clean` 不是檔案，是「清掉產物」這個動作的名字，所以沒有先決條件。`.PHONY: clean` 宣告它是**假目標**（phony target）：不管有沒有叫 `clean` 的檔案，`make clean` 都要執行命令。
不宣告會怎樣：`Makefile.nophony`（檔案：[`Makefile.nophony`](../small-project/s1-06-multi-file/examples/make/Makefile.nophony)，就是上面的 `Makefile` 拿掉 `.PHONY: clean` 那一行）

```
$ touch clean
$ make -f Makefile.nophony clean
make: 'clean' is up to date.
$ make clean
rm -f calc main.o mathx.o main.d mathx.d
```

剛好有一個叫 `clean` 的檔案，它沒有先決條件、檔案又存在，`make` 判斷「已經是最新的」，什麼都不做。有 `.PHONY` 的版本照樣執行。

### 命令列：`make [選項] [目標] ...`

| 位置 | 是什麼 | 省略時 |
|---|---|---|
| 選項 | 下表 | — |
| 目標 | 要做哪些目標，可以好幾個，照順序做 | 做 `Makefile` 裡第一條規則的目標（叫「預設目標」，default goal） |
| （也可以放）`變數=值` | 蓋掉 `Makefile` 裡的變數，例如 `make CFLAGS=-O2` | — |

`make --help` 的全部選項（本機 GNU Make 4.3，共 32 項，照 `--help` 的順序；它沒有再分組）：

| 選項 | 做什麼 |
|---|---|
| `-b`、`-m` | 不做任何事，只是為了相容舊版 `make` 而接受 |
| `-B`、`--always-make` | 不管時間，所有目標都重做 |
| `-C 資料夾`、`--directory=資料夾` | 先切換到該資料夾再開始（kernel 編外部模組就是 `make -C <kernel 原始碼> M=$PWD`） |
| `-d` | 印出大量除錯訊息（為什麼決定做或不做每個目標） |
| `--debug[=FLAGS]` | 只印某幾類除錯訊息（`a` 全部、`b` 基本、`v` 詳細、`i` 隱含規則、`j` 子程序、`m` 重讀 makefile、`n` 都不印） |
| `-e`、`--environment-overrides` | 讓環境變數蓋過 `Makefile` 裡的變數（預設是 `Makefile` 優先） |
| `-E 字串`、`--eval=字串` | 把字串當成 `Makefile` 的一行先執行 |
| `-f 檔案`、`--file=檔案`、`--makefile=檔案` | 讀指定的檔案，而不是預設的 `GNUmakefile`／`makefile`／`Makefile` |
| `-h`、`--help` | 印出說明 |
| `-i`、`--ignore-errors` | 命令失敗也繼續（預設是一個命令失敗就停） |
| `-I 資料夾`、`--include-dir=資料夾` | `include` 其他 makefile 時也去這個資料夾找（跟 gcc 的 `-I` 無關） |
| `-j [N]`、`--jobs[=N]` | 同時執行 N 個命令（平行編譯）；不給 N 就不限數量 |
| `-k`、`--keep-going` | 某個目標失敗時，不相干的目標繼續做（一次看到所有錯誤） |
| `-l [N]`、`--load-average[=N]`、`--max-load[=N]` | 系統負載高於 N 時不開新的平行工作 |
| `-L`、`--check-symlink-times` | 符號連結比較時間時，用連結本身和指向的檔案中較新的那個 |
| `-n`、`--just-print`、`--dry-run`、`--recon` | **只印出會執行的命令，不真的執行**（檢查 `Makefile` 最常用） |
| `-o 檔案`、`--old-file=檔案`、`--assume-old=檔案` | 把這個檔案當成很舊，不重做它（也不因為它而重做別人） |
| `-O[TYPE]`、`--output-sync[=TYPE]` | 平行執行時，把每個工作的輸出整理在一起再印，不交錯 |
| `-p`、`--print-data-base` | 印出內部資料庫（全部變數與規則，含內建的） |
| `-q`、`--question` | 不執行命令，只用結束狀態回答「是不是最新的」（0 ＝ 最新） |
| `-r`、`--no-builtin-rules` | 關掉內建的隱含規則（例如內建的 `%.o: %.c`） |
| `-R`、`--no-builtin-variables` | 關掉內建的變數（例如內建的 `CC = cc`） |
| `-s`、`--silent`、`--quiet` | 不印出命令本身（只印命令的輸出） |
| `--no-silent` | 印出命令（取消 `-s`） |
| `-S`、`--no-keep-going`、`--stop` | 取消 `-k` |
| `-t`、`--touch` | 不執行命令，只把目標 `touch` 成最新 |
| `--trace` | 印出每個目標為什麼要做、做了什麼 |
| `-v`、`--version` | 印出版本 |
| `-w`、`--print-directory` | 印出目前所在的資料夾（遞迴呼叫 `make` 時有用） |
| `--no-print-directory` | 不印資料夾（取消 `-w`） |
| `-W 檔案`、`--what-if=檔案`、`--new-file=檔案`、`--assume-new=檔案` | 把這個檔案當成剛改過（配合 `-n` 可以問「改了這個檔案會重做什麼」） |
| `--warn-undefined-variables` | 用到沒定義的變數時警告（打錯變數名字會變成空字串，這個選項抓得到） |

`Makefile` 本身的文法比這裡教的多很多：函式（`$(wildcard *.c)`、`$(patsubst ...)` 等，手冊第 8 章）、條件（`ifeq`、`ifdef`，第 7 章）、命令前加 `@`（不印這行）或 `-`（這行失敗也繼續）、遞迴 `make` 等。本課只教了規則、變數、自動變數、樣式規則、`include`、`.PHONY` 與替換參照；完整文法見 `info make`（GNU make 手冊，本機要先 `sudo apt install make-doc`，或看線上版 <https://www.gnu.org/software/make/manual/>）。

---

## 八、本課用到的工具、選項與完整清單在哪

### `gcc`：本課新用到的選項

gcc 的選項有上千個，完整見 `man gcc`；`gcc --help=common`、`--help=warnings` 等可以分類列出。本課新用到 7 個：

| 選項 | 做什麼 | 出處 |
|---|---|---|
| `-I 資料夾` | 把資料夾加進 `#include <...>`（以及 `"..."` 的後半段）的搜尋清單 | 第三節 |
| `-v` | 印出 gcc 執行的每個步驟與搜尋路徑 | 第三節 |
| `-MMD`、`-MD`、`-MP` | 編譯時順便寫出 `make` 用的相依檔 `.d` | 第七節 |
| `-fcommon`（預設 `-fno-common`） | 沒有初值的全域變數變成 common 符號，同名的會被合併 | 第六節 |
| `-Q --help=common` | 列出共通選項與目前的設定值 | 第六節 |

`-c`、`-E`、`-o`、`-g`、`-O0`、`-Wall` 前幾課教過（第 1 課[第七節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#七本課用到的工具與選項)）。

### 觀察工具

- `readelf -s`：符號表。`readelf` 的全部選項見 [readelf完整選項參考](./readelf完整選項參考.md)；本課用到 `-s`、`-S` 兩個。各欄的意思見第四節的表。
- `nm`：第 1 課教過（`nm --help` 共 36 個選項，完整見 `man nm`）；本課多了小寫字母（`t` ＝ local 的程式碼、`b` ＝ local 的 `.bss`）與 `C`（common）。符號字母的完整清單在 `man nm`。
- `objdump -dr`：第 5 課教過。
- `touch`：把檔案的修改時間改成現在（不存在就建一個空檔）。本課只用這個最基本的用法；它還能指定時間（`-d`、`-t`）、只改存取時間（`-a`）等，完整見 `touch --help`。
- `make`：全部 32 個選項見第七節〈命令列〉。

### C 語言：本課的關鍵字

- `static`：兩種意思，第五節。
- `extern`：宣告「定義在別處」，第六節。（`extern` 也能用在函式宣告上，但函式宣告本來就預設是 `extern`，寫不寫都一樣。）
- 前處理指令 `#ifndef`／`#define`／`#endif`：第三節。前處理指令的完整清單見 `info cpp`（GNU C 前處理器手冊；第 1 課第二節只教了 `#include` 與 `#define`）。

---

## 九、動手作業

作業在 [`small-project/s1-06-multi-file/`](../small-project/s1-06-multi-file/README.md)，共 5 題。題目、要寫的檔案與作答框都在該資料夾的 `README.md`。

---

## 自我測驗

> 參考答案寫在每題下面，第一次作答批改完才會補上。作答時先遮住答案、自己想過再對照。作答寫在 [`s1-06-multi-file/quiz.md`](../small-project/s1-06-multi-file/quiz.md)。

1. `main.c` 沒有 `square` 的宣告，`gcc -c main.c` 只有一個警告，連結也成功，但 `square(100000)` 印出 1410065408 而不是 10000000000。為什麼編譯器會產生錯的程式？為什麼連結器沒有發現兩邊型別不一樣？`main.o` 裡的 `cltq` 在這個錯誤裡扮演什麼角色？
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
   > **參考答案**（核心關鍵＋詳細說明）
   >
   > **1-1**【[第二節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#實驗忘了宣告square100000-印出-1410065408)〈實驗：忘了宣告〉】核心關鍵：
   >
   > 1. 編譯器一次只看一個 `.c`，編 `main.c` 時看不到 `square` 的宣告和定義。
   > 2. 只好照舊規則**隱式宣告**成回傳 `int`，照 `int` 產生程式碼：回傳值只取 `%eax`（低 32 位元）。
   >
   > 詳細說明：`square` 本身照 `long` 算，`%rax` ＝ `0x2_540be400`（10000000000）；`main` 只拿 `%eax` ＝ `0x540be400` ＝ 1410065408。**被呼叫的那邊是對的，呼叫的那邊用錯的型別解讀結果**。編譯器是忠實照自己的猜測產生程式，錯在猜測本身。
   > gcc 13 對隱式宣告只給 `-Wimplicit-function-declaration` 警告，所以編譯要開 `-Wall`、不能忽略警告（據 gcc 14 的變更說明新版預設改成錯誤，本機未實測）。
   >
   > **1-2**【[第四節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#四符號表o-之間靠名字接起來)】核心關鍵：
   >
   > 1. 連結器只比對符號的**名字**：`main.o` 的 `UND square` 配上 `mathx.o` 的 `GLOBAL square`。
   > 2. `.o` 的符號表裡沒有 C 的型別（`Type` 只有 `FUNC`／`OBJECT`／`NOTYPE` 這種），所以兩邊型別不一樣，連結器無從發現。
   >
   > 詳細說明：整理成一句：編譯 `main.c` 時看不到 `mathx.c`、編譯 `mathx.c` 時看不到 `main.c`、連結時看不到型別——**沒有任何一個階段同時看得到兩邊的型別**。標頭檔的作用就是在編譯階段把同一份型別資訊帶到兩邊（第三節）。
   >
   > **1-3**【[第二節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#實驗忘了宣告square100000-印出-1410065408)〈實驗：忘了宣告〉】核心關鍵：
   >
   > 1. `cltq` 把回傳的 `%eax` 擴展成 `%rax` 存進 `long r`：只保留低 32 位元。（2026-10-24 改判：「符號擴展」移到詳細說明，題目的 100000 補 0 補 1 結果一樣）
   > 2. 所以 `square` 留在 `%rax` 高 32 位元的正確結果（`0x2`）被蓋掉，截斷就發生在這一條；它是「猜成 `int`」的結果，不是根因。
   >
   > 詳細說明：`cltq` 是**符號擴展**（＝ `movslq %eax,%rax`），高 32 位元照第 31 位元補。為什麼會有這一條：`main` 以為拿到的是 `int`，要存進 `long r`，`int` 轉 `long` 要保留數值（含正負號），所以編譯器插一條符號擴展。如果 `main` 有正確的宣告（回傳 `long`），就不會有 `cltq`，直接 `mov %rax,-0x8(%rbp)`。
   >
   > 本機實測：把題目 `main.c` 的 `square(100000)` 改成 `square(50000)`，其餘不變、照題目的指令編譯執行，印出 `-1794967296`。50000² ＝ 2500000000 ＝ `0x9502f900`，還沒超過 32 位元，但最高位元是 1（`0x9` ＝ `1001`），`cltq` 補成全 1 → 2500000000 − 2³² ＝ −1794967296。
   >
   > （2026-10-24 使用者追問「印出來是負數好像不對」，重新實測確認）負數只出現在這一題的寫法（沒有宣告、有 `cltq`）。同樣是 `square(50000)`，換成多檔 2-2 的寫法（`main.c` 引入 `long square(long)` 的標頭檔、`mathx.c` 定義成 `int square(int)`）印的是 **2500000000**：`imul %eax,%eax` 留下 `0x9502f900`，寫 `%eax` 時高 32 位元被清成 0，`main` 照 `long` 讀整個 `%rax`，沒有 `cltq` 去補 1。兩個寫法的 `100000` 都印 1410065408，因為 `0x540be400` 的第 31 位是 0（`0x5` ＝ `0101`），補 0 補 1 結果一樣。差別只有第 31 位是 1 時才看得出來。
2. 為什麼「定義函式的那個 `.c` 也要 `#include` 自己的標頭檔」？如果 `mathx.c` 沒有 `#include "mathx.h"`，有人把 `mathx.c` 的定義改成 `int square(int x)` 卻忘了改 `mathx.h`，會發生什麼事、在哪個階段（編譯、連結、執行）才發現？
   ```c
   /* mathx.h */
   #ifndef MATHX_H
   #define MATHX_H

   long square(long x);

   #endif
   ```
   呼叫端 `main.c` 有引入 `mathx.h`（2026-10-22 補上，原題沒寫清楚）：
   ```c
   #include <stdio.h>
   #include "mathx.h"

   int main(void)
   {
       long r = square(100000);
       printf("%ld\n", r);
       return 0;
   }
   ```
   > **參考答案**（核心關鍵＋詳細說明）
   >
   > **2-1**【[第三節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#定義的那個-c-為什麼也要-include-自己的標頭檔)〈定義的那個 `.c` 為什麼也要 `#include` 自己的標頭檔〉】核心關鍵：
   >
   > 1. 引入後，`mathx.c` 這個編譯單元裡同時有宣告（貼進來的 `mathx.h`）和定義，編譯器會比對，不一致就在**編譯時**報 `conflicting types`。
   > 2. 呼叫端引入的是同一份 `mathx.h`，所以定義通過檢查，就保證呼叫端用的宣告跟定義一致。
   >
   > 詳細說明：第三節的 `bad.c` 實測（`#include "mathx.h"` 後把定義寫成 `int square(int x)`）：
   >
   > ```text
   > $ gcc -c bad.c
   > bad.c:3:5: error: conflicting types for ‘square’; have ‘int(int)’
   > mathx.h:4:6: note: previous declaration of ‘square’ with type ‘long int(long int)’
   > ```
   >
   > 沒引入的話會怎樣，就是 2-2。
   >
   > **2-2**【[第三節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#定義的那個-c-為什麼也要-include-自己的標頭檔)〈定義的那個 `.c` 為什麼也要 `#include` 自己的標頭檔〉最後一段】核心關鍵：
   >
   > 1. 編譯不報錯：`mathx.c` 沒引入 `mathx.h`，編譯器看不到舊宣告、沒東西可比；`main.c` 照標頭檔的 `long square(long)` 編，也沒錯。
   > 2. 連結不報錯：連結器只比名字 `square`。
   > 3. **執行時**才出錯（結果錯誤，而且可能沒人發現）：呼叫端照 `long` 傳參數、讀 `%rax`，定義端照 `int` 只用 `%edi`、只算 32 位元——又回到第 1 題「連結成功、結果錯誤」。
   >
   > 詳細說明：實驗 `../small-project/s1-06-multi-file/examples/quiz-noinclude/`：`mathx.h`（檔案：[`mathx.h`](../small-project/s1-06-multi-file/examples/quiz-noinclude/mathx.h)）就是題目給的標頭檔；`mathx.c`（檔案：[`mathx.c`](../small-project/s1-06-multi-file/examples/quiz-noinclude/mathx.c)）改成 `int`、而且**沒有**引入標頭檔：
   >
   > ```c
   > /* 自我測驗 2-2：定義改成 int、而且沒有 #include "mathx.h" */
   > int square(int x)
   > {
   >     return x * x;
   > }
   > ```
   >
   > `main.c`（檔案：[`main.c`](../small-project/s1-06-multi-file/examples/quiz-noinclude/main.c)）有引入標頭檔：
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
   > `main` 照 `long` 讀整個 `%rax`；`square` 寫的是 `%eax`，寫 32 位元暫存器會把高 32 位元清成 0（[第 4 課](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)），所以 `main` 拿到 1410065408。（`int` 乘法溢位在 C 裡是未定義行為，這只是本機 `-O0` 的結果。）
   >
   > 這就是定義的 `.c` 也要引入自己標頭檔的理由：引入了，這個錯在編譯 `mathx.c` 時就是 `conflicting types`（2-1），根本到不了執行。
3. `static` 寫在函式外面和寫在函式裡面，各改變了什麼？用下面兩個例子說明：(a) `a.c`、`b.c` 各有一個 `static int helper(void)`，為什麼能一起連結，拿掉 `static` 為什麼就 `multiple definition`？(b) `next_id()` 呼叫三次回傳 1、2、3，`plain_id()` 每次都是 1，兩個 `n` 各放在記憶體的哪裡？
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
   > **參考答案**（核心關鍵＋詳細說明）
   >
   > **3-1**【[第五節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#五static兩種完全不同的意思)】核心關鍵：
   >
   > 1. 函式外面：改的是名字的**可見範圍**——只在這個 `.c` 有效，符號變成 `LOCAL`，連結器不拿它跟別的檔案配對。
   > 2. 函式裡面：改的是**儲存期間**——不放堆疊，放在固定記憶體（`.bss`／`.data`），整個程式執行期間只有一份，值留到下一次呼叫；名字仍然只在那個函式裡看得到。
   >
   > 詳細說明：同一個關鍵字、兩種完全不同的意思，要看寫在哪裡。只給自己檔案用的函式與全域變數一律加 `static`（kernel 絕大部分函式都是）。
   >
   > **3-2**【[第五節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#檔案層級的-static讓名字變成-local)〈檔案層級的 `static`〉】核心關鍵：
   >
   > 1. 有 `static`：兩個 `helper` 都是 `LOCAL`，連結器做符號解析時只用 `GLOBAL` 配對，所以同名也不衝突。
   > 2. 拿掉 `static`：兩個 `.o` 各有一個 `GLOBAL` 的 `helper` 定義 → 同名的 `GLOBAL` 定義有兩個 → `multiple definition`。
   >
   > 詳細說明：第五節實測（`a2.c`、`b2.c` 是拿掉 `static` 的版本）：
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
   >
   > **3-3**【[第五節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#函式裡的-static不放堆疊值留到下一次)〈函式裡的 `static`〉】核心關鍵：
   >
   > 1. `next_id` 的 `n`：固定記憶體 `.bss`（初值 0），只初始化一次、值留著，所以 1、2、3。
   > 2. `plain_id` 的 `n`：堆疊框裡（`-0x4(%rbp)`），每次呼叫都重新設成 0，所以每次都是 1。
   >
   > 詳細說明：組語看得出差別：`next_id` 用 `0x0(%rip)` 存取（固定位置，連結時填），函式裡沒有任何設成 0 的指令（初值是載入時準備好的）；`plain_id` 每次都 `movl $0x0,-0x4(%rbp)`。符號表裡有 `LOCAL` 的 `n.0`（`.bss`、4 byte），`plain_id` 的 `n` 沒有符號。
4. 下面的 `hits.h` 被 `hits.c` 和 `hits_main.c` 都 `#include`，兩個 `.c` 都沒有再寫 `hits` 的定義。本機 gcc 13 連結時出現 `multiple definition of 'hits'`。為什麼？已經有 include guard 了，為什麼沒擋住？該怎麼改（標頭檔寫什麼、哪裡寫什麼）？為什麼同樣的程式碼在舊版 gcc 上可以連結？
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
   > **參考答案**（核心關鍵＋詳細說明）
   >
   > **4-1**【[第六節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#忘了-extern每個-c-都有一份定義)〈忘了 `extern`：每個 `.c` 都有一份定義〉】核心關鍵：
   >
   > 1. `long hits;`（沒有 `extern`、沒有初值）是暫定定義，實際上就是**定義**。
   > 2. 標頭檔被貼進兩個 `.c`，兩個 `.o` 各有一個 `GLOBAL` 的 `hits` → 連結器看到兩個同名定義 → `multiple definition`。
   >
   > 詳細說明：第六節實測：
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
   >
   > **4-2**【[第六節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#忘了-extern每個-c-都有一份定義)〈忘了 `extern`：每個 `.c` 都有一份定義〉最後一段】核心關鍵：
   >
   > 1. include guard 只防止**同一個編譯單元**裡被貼兩次（`HITS_H` 這個巨集只在編譯這一個 `.c` 的期間有效）。
   > 2. `hits.c` 和 `hits_main.c` 是分開編譯的兩個編譯單元，各貼一次是正常的；錯誤發生在連結時兩個 `.o` 之間，guard 管不到。
   >
   > 詳細說明：guard 的機制見[第三節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#include-guard同一個標頭檔被貼兩次)〈include guard〉：第一次貼進來時定義 `HITS_H`，同一個 `.c` 第二次貼進來就整段丟掉。編譯下一個 `.c` 時前處理器從頭開始，`HITS_H` 又是沒定義的。
   >
   > **4-3**【[第六節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#六extern全域變數的宣告與定義)〈整理：全域變數放哪裡〉】核心關鍵：
   >
   > 1. 標頭檔寫 `extern long hits;`：宣告，不配置空間，幾個 `.c` 引入都沒關係。
   > 2. **恰好一個 `.c`**（`hits.c`）寫定義 `long hits = 0;`，而且這個 `.c` 也引入 `hits.h`，讓編譯器檢查宣告與定義的型別一致。
   >
   > 詳細說明：只改標頭檔、不補定義的實驗 `../small-project/s1-06-multi-file/examples/quiz-extern-only/`：`hits.h`（檔案：[`hits.h`](../small-project/s1-06-multi-file/examples/quiz-extern-only/hits.h)）
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
   > `hits.c`（檔案：[`hits.c`](../small-project/s1-06-multi-file/examples/quiz-extern-only/hits.c)）
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
   > `hits_main.c`（檔案：[`hits_main.c`](../small-project/s1-06-multi-file/examples/quiz-extern-only/hits_main.c)）跟題目的一樣。
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
   >
   > **4-4**【[第六節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#忘了-extern每個-c-都有一份定義)〈`-fcommon`：舊程式為什麼這樣寫也能動〉】核心關鍵：
   >
   > 1. gcc 10 以前預設 `-fcommon`：沒有初值的全域變數被標成 **common 符號**（`Ndx` 是 `COM`，`nm` 印 `C`），不是一般的定義。
   > 2. 連結器遇到好幾個同名的 common 符號會**合併成一份**，不報錯，兩個檔案共用同一個 `hits`。
   >
   > 詳細說明：第六節實測（本機 gcc 13 用 `-fcommon` 模擬舊版行為；更舊的 gcc 本機沒有，未實測）：
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
   > gcc 10 起預設改成 `-fno-common`，所以舊專案升級 gcc 後冒出一堆 `multiple definition`。正確修法是補 `extern`（4-3），不是加 `-fcommon`。`-fcommon` 也只合併**沒有初值**的；有初值的定義照樣衝突（[第六節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#補充extern-的三個邊界情況2026-10-21-作業第-2-題批改時補教)〈補充：`extern` 的三個邊界情況〉，作業 2-3）。
5. 情境題：同事的專案用下面的 `Makefile`。他把 `config.h` 裡的 `#define BUF_SIZE 64` 改成 `128`，存檔後執行 `make`，`make` 只印了 `make: 'server' is up to date.`，跑起來的緩衝區還是 64。`make` 是怎麼判斷不用重編的？為什麼這個判斷錯了？你會怎麼改這個 `Makefile`，讓以後改任何標頭檔都會重編該重編的檔案？（不必寫出完整的 `Makefile`，說明要改哪些地方、各自的作用即可。）
   ```make
   server: server.o net.o
   	gcc -o server server.o net.o

   server.o: server.c
   	gcc -Wall -c server.c

   net.o: net.c
   	gcc -Wall -c net.c
   ```
   （`server.c` 和 `net.c` 都 `#include "config.h"`；命令前面是 TAB。）
   > **參考答案**（核心關鍵＋詳細說明）
   >
   > **5-1**【[第七節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#文法規則)〈文法：規則〉】核心關鍵：
   >
   > 1. 目標不存在，或任何一個**先決條件的修改時間比目標新**，就執行命令；否則跳過（先決條件也是別的規則的目標時，先遞迴處理它）。
   > 2. 本例 `server.o`、`net.o` 的先決條件只有各自的 `.c`，`.c` 都沒改 → 都不重編 → `server` 也不必重新連結。
   >
   > 詳細說明：`make` 比的是檔案系統記錄的修改時間（`ls -l --time-style=full-iso` 看得到），它不看檔案內容。
   >
   > **5-2**【[第七節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#先決條件漏了標頭檔make-說不用重編結果是舊的)〈先決條件漏了標頭檔〉】核心關鍵：
   >
   > 1. `make` 只看 `Makefile` 寫的先決條件，不會自己去讀 `.c` 裡 `#include` 了哪些檔案；`config.h` 沒列進去，它改了 `make` 不知道。
   > 2. 可是 `config.h` 的內容在編譯時被貼進 `server.c`／`net.c`，它變了 `.o` 就該重編。
   >
   > 詳細說明：第七節 `Makefile.nodeps` 是同一個情況：把 `mathx.h` 的 `DEFAULT_N` 從 7 改成 9，`make` 說 `'calc' is up to date.`，程式還是印 `square(7) = 49`。這種錯最難查：程式碼看起來是對的，跑出來的卻是舊版。
   >
   > **5-3**【[第七節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#實用的-makefile變數樣式規則自動產生相依)〈實用的 `Makefile`〉第 5、6 點】核心關鍵：
   >
   > 1. 編譯命令加 `-MMD -MP`：編譯時順便寫出 `.d` 檔，內容是「這個 `.o` 依賴哪些使用者標頭檔」的規則（例如 `server.o: server.c config.h`）；`-MP` 再替每個標頭檔加一條空規則，標頭檔刪掉或改名時 `make` 不會停下來。
   > 2. `Makefile` 加 `-include $(OBJS:.o=.d)`：把 `.d` 讀進來、跟原本的規則合併，`config.h` 就成了 `.o` 的先決條件；`-` 表示第一次還沒有 `.d` 時不報錯。
   >
   > 詳細說明：改好的版本（命令前面是 TAB；`clean`／`.PHONY` 不是這題必要的，順便附上）：
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
6. （2026-10-20 提問「符號代表什麼」追加，出處：[第一節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#一本課在做什麼編譯器一次只看一個-c)〈上面說的「符號」到底是什麼〉）下面的 `t.c` 用 `gcc -c t.c` 編成 `t.o`。哪些名字會出現在 `t.o` 的符號表裡？其中哪些是「這個檔案提供」、哪些是「需要別人提供」？`n`、`sum`、`i` 為什麼不在符號表裡？對連結器來說，符號表裡的一個符號代表什麼、拿來做什麼用？
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
   > **參考答案**（核心關鍵＋詳細說明）
   >
   > **6-1**【[第一節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#一本課在做什麼編譯器一次只看一個-c)〈上面說的「符號」到底是什麼〉】核心關鍵：
   >
   > 1. 符號表裡有：`total`、`add`、`main`、`limit`、`puts`（加上 ELF 記帳用的 `FILE`、`SECTION` 列）。
   > 2. 這個檔案提供（有定義，`Ndx` 是區段編號）：`total`（`.bss`）、`add`、`main`（`.text`）。
   > 3. 需要別人提供（`UND`）：`limit`（只有 `extern` 宣告）、`puts`（libc）。
   >
   > 詳細說明：`nm t.o` 是同一份的精簡版：`T add`、`T main`、`B total`（`.bss`，初值 0）、`U limit`、`U puts`。`extern` 宣告的變數跟呼叫了但沒定義的函式，在符號表裡是同一種東西：`UND`、`NOTYPE`（這個檔案只知道名字）。
   >
   > **6-2**【[第一節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#一本課在做什麼編譯器一次只看一個-c)〈上面說的「符號」到底是什麼〉「區域變數不是符號」那段】核心關鍵：
   >
   > 1. `n`（參數）、`sum`、`i` 是函式裡的變數，放在堆疊框（或暫存器），位址是執行時 `%rbp` 的偏移。
   > 2. 這個偏移編譯器自己就寫得出來，不需要連結器決定位址，所以不需要名字、不進符號表。
   >
   > 詳細說明：`objdump -dr t.o` 的 `add` 裡，`mov %rdi,-0x18(%rbp)`（存 `n`）、`mov %rax,-0x8(%rbp)`（存 `sum`）下面都沒有重定位項目；讀寫 `total` 的 `mov 0x0(%rip),%rdx` 下面才有 `R_X86_64_PC32 total-0x4`。有洞的地方才需要名字。
   >
   > **6-3**【[第一節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#一本課在做什麼編譯器一次只看一個-c)〈上面說的「符號」到底是什麼〉】核心關鍵：
   >
   > 1. 一個符號 ＝ 名字＋它在哪裡（區段、偏移、大小）；有定義的代表「這個檔案提供」，`UND` 代表「這個檔案需要」。
   > 2. 連結器用它做**符號解析**：把每個 `UND` 跟別的 `.o`／函式庫的 `GLOBAL` 定義配對（只比名字）。
   > 3. 再做**重定位**：算出每個符號的最終位址，填進機器碼裡標著那個符號名字的洞。
   >
   > 詳細說明：`objdump -dr --no-show-raw-insn t.o` 裡有洞的指令（只截有重定位項目的幾行）：
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
