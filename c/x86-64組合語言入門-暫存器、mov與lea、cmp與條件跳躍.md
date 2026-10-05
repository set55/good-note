# 讀懂 x86-64 組合語言（一）—— 暫存器、`mov` 與 `lea`、`cmp` 與條件跳躍，逐行讀懂 `sum_to`

> 日期：2026-10-05
> 情境：系統化學習路線第 ① 站第 4 課。對應 CS:APP 第 3.1～3.6 節（程式的機器層級表示：資料格式、存取資訊、算術與邏輯運算、控制——條件碼與跳躍、迴圈）；
> 函式呼叫與堆疊框（3.7 節）留到第 5 課
> 適用範圍：實測本機 Ubuntu 24.04、x86-64、gcc 13.3.0、GNU as／objdump 2.42（binutils）、gdb 15.1；所有組合語言都是本機 gcc 實際產生的
> 對應檔案：每個實驗程式的完整原始碼都寫在它出現的那一節，可以直接複製編譯；
> 作業在 [`small-project/s1-04-asm-basics/`](../small-project/s1-04-asm-basics/)（會進 git）
> 相關：[hello.c怎麼變成能跑的程式](./hello.c怎麼變成能跑的程式-編譯的四個階段.md)（第 1 課：`.s` 是怎麼來的）、
> [gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md)（`sum_to` 第一次出現的地方）、
> [C的資料在記憶體裡長怎樣](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md)（第 2 課：二補數、符號擴展）、
> [陣列、字串與指標運算](./陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md)（第 3 課：`p[i]` 就是 `*(p + i)`）

**一句話結論：組合語言就是機器碼的文字版，一行一個指令。讀它只要抓三件事：
(1) **資料在哪**——16 個通用暫存器（`%rax`、`%rdi`…，同一個暫存器有 64／32／16／8 位元四個名字）或記憶體（`D(B,I,S)` ＝ 位址 `D + B + I×S`，正好就是 `a[i]` 的 `a + i×sizeof`）；
(2) **做什麼**——`mov` 搬資料（可能碰記憶體），`lea` 只**算位址不碰記憶體**（所以被拿來做加法和乘法），`add`／`sub`／`xor`／`and` 算術；
(3) **怎麼決定走哪條路**——`cmp`／`test` 只設**旗標**（ZF、SF、CF、OF），後面的 `jle`、`jb`、`setl` 才去讀旗標。
**有號還是無號，不是 `cmp` 決定的，是後面那個指令挑哪幾個旗標決定的**：`l`／`g`（less／greater）是有號，`b`／`a`（below／above）是無號——C 的型別到了組合語言，就只剩這個字母。**

---

## 目錄

- [零、開課複習：今天的複習題跟本課的關係](#零開課複習今天的複習題跟本課的關係)
- [一、組合語言在哪裡：一行文字＝一個機器指令](#一組合語言在哪裡一行文字一個機器指令)
- [二、暫存器：CPU 裡的 16 個變數](#二暫存器cpu-裡的-16-個變數)
- [三、指令的文法：助憶符、大小後綴、運算元](#三指令的文法助憶符大小後綴運算元)
- [四、`mov` 與 `lea`：搬資料 vs 算位址](#四mov-與-lea搬資料-vs-算位址)
- [五、旗標與條件跳躍：`cmp` 只負責比，跳不跳由下一個指令決定](#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)
- [六、逐行讀 `sum_to` 的 `-O0` 版](#六逐行讀-sum_to-的--o0-版)
- [七、逐行讀 `sum_to` 的 `-O2` 版](#七逐行讀-sum_to-的--o2-版)
- [八、本課用到的工具、選項與完整清單在哪](#八本課用到的工具選項與完整清單在哪)
- [九、動手作業](#九動手作業)
- [自我測驗](#自我測驗)

---

## 零、開課複習：今天的複習題跟本課的關係

今天的複習檔 [`review/2026-10-05-gcc-陣列-GNU-C資料-字元.md`](../review/2026-10-05-gcc-陣列-GNU-C資料-字元.md) 的前 5 題特意挑了跟本課相關的：

| 複習題 | 在本課哪裡再出現 |
|---|---|
| gcc 3（`-O2` 的 `answer()` 直接回傳 5050） | [第七節](#七逐行讀-sum_to-的--o2-版)：`answer:` 就只剩 `movl $5050, %eax` 和 `ret` 兩行 |
| gcc 4（最佳化不等於變小） | [第七節](#七逐行讀-sum_to-的--o2-版)：`-O2` 的 `sum_to` 為了一圈做兩個數，指令比 `-O0` 還多 |
| 陣列 2（`a` 與 `&a` 的型別） | [第四節](#四mov-與-lea搬資料-vs-算位址)：`a[i]` 在組合語言裡就是 `(%rdi,%rsi,4)`，那個 `4` 就是「`+ 1` 走幾個 byte」 |
| 陣列 3（`"hi"` 在 `.rodata`） | [第六節](#六逐行讀-sum_to-的--o0-版)：`main` 裡的 `"answer:%d\n"` 被放進 `.section .rodata`，用 `leaq .LC0(%rip)` 取位址 |
| 陣列 6（ASan 的半開區間） | 間接相關：本課的位址都用「基準＋偏移」表示，跟 ASan 的 `offset` 是同一種想法 |

建議**先寫完複習**，再讀下面的內容。

另外，[gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md) 的自我測驗第 2 題原本寫著「學完讀懂組合語言那一課後再答」，讀完本課的第六、七節就能回答了，它會在本課之後放回複習排程。

---

## 一、組合語言在哪裡：一行文字＝一個機器指令

[第 1 課](./hello.c怎麼變成能跑的程式-編譯的四個階段.md)看過：`gcc -S` 停在「編譯」之後，產生 `.s` 檔，`as` 再把它組譯成 `.o`。
`.s` 裡的每一行指令，在 `.o` 裡都對應**一串固定的 byte**（機器碼）。用 `objdump -d` 可以兩邊對照著看。

實驗用的程式就是 [gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md) 的 `sum_to`（第 1 課作業資料夾裡的 `sumto.c`），左邊標的是行號，第六節會用到：

```c
/* sumto.c */
#include <stdio.h>                          /* 第 1 行 */
                                            /* 第 2 行 */
int sum_to(int n)                           /* 第 3 行 */
{                                           /* 第 4 行 */
    int total = 0;                          /* 第 5 行 */
    for (int i = 1; i <= n; i++)            /* 第 6 行 */
        total += i;                         /* 第 7 行 */
    return total;                           /* 第 8 行 */
}

int answer(void)
{
    return sum_to(100);
}

int main(void)
{
    printf("answer:%d\n", answer());
    return 0;
}
```

（上面的 `/* 第 N 行 */` 是為了標行號才加的；實際檔案裡沒有這些註解，行號一樣。）

```sh
gcc -O0 -c sumto.c -o s0.o
objdump -d s0.o
```

```
0000000000000000 <sum_to>:
   0:	f3 0f 1e fa          	endbr64
   4:	55                   	push   %rbp
   5:	48 89 e5             	mov    %rsp,%rbp
   8:	89 7d ec             	mov    %edi,-0x14(%rbp)
   b:	c7 45 f8 00 00 00 00 	movl   $0x0,-0x8(%rbp)
  12:	c7 45 fc 01 00 00 00 	movl   $0x1,-0x4(%rbp)
  19:	eb 0a                	jmp    25 <sum_to+0x25>
  1b:	8b 45 fc             	mov    -0x4(%rbp),%eax
  1e:	01 45 f8             	add    %eax,-0x8(%rbp)
  21:	83 45 fc 01          	addl   $0x1,-0x4(%rbp)
  25:	8b 45 fc             	mov    -0x4(%rbp),%eax
  28:	3b 45 ec             	cmp    -0x14(%rbp),%eax
  2b:	7e ee                	jle    1b <sum_to+0x1b>
  2d:	8b 45 f8             	mov    -0x8(%rbp),%eax
  30:	5d                   	pop    %rbp
  31:	c3                   	ret
```

三欄怎麼讀：

| 欄 | 例子 | 是什麼 |
|---|---|---|
| 第 1 欄 | `1b:` | 這個指令在 `.text` 區段裡的**位置**（十六進位，從函式開頭算）。`.o` 還沒連結，所以從 0 開始 |
| 第 2 欄 | `8b 45 fc` | **機器碼**：CPU 真正讀的 byte。指令長度不固定，1～15 byte 都有（x86 的特色） |
| 第 3 欄 | `mov -0x4(%rbp),%eax` | 同一個指令的**文字版**（反組譯的結果），就是組合語言 |

**機器碼本身也看得出第 2 課的東西**：

- `c7 45 f8 00 00 00 00`（`movl $0x0,-0x8(%rbp)`）後面四個 byte 是常數 0；`c7 45 fc 01 00 00 00` 是常數 1——**小端序**，低位的 `01` 在前面。
- `7e ee` 是 `jle 1b`。`ee` 不是目標位址，是**相對距離**，用有號的二補數解讀：`0xee` 當 `signed char` 是 −18。
  跳躍距離是從「下一個指令的位置」算起：下一個指令在 `0x2d`，`0x2d − 18 = 0x1b`，正好是跳過去的地方。
  同理 `eb 0a`（`jmp 25`）：下一個指令在 `0x1b`，`0x1b + 10 = 0x25`。

**讀組合語言時不需要背機器碼**，`objdump` 會幫你翻成文字。這裡只是要看到：組合語言跟機器碼是一對一的，不是另一種「比較高階的語言」。

### 兩種寫法：AT&T 與 Intel

同一個指令有兩種文字寫法。gcc、`objdump`、`gdb` 在 Linux 上預設都用 **AT&T 語法**，Linux kernel 原始碼裡的組合語言也是 AT&T，所以本課全部用 AT&T。
Intel 的手冊和 Windows 世界用的是 **Intel 語法**，兩邊都會遇到，要認得。用 `-masm=intel` 讓 gcc 改產生 Intel 語法：

```sh
gcc -S -O0 -masm=intel -fno-asynchronous-unwind-tables sumto.c -o sumto0_intel.s
```

| | AT&T（gcc 預設） | Intel（`-masm=intel`） |
|---|---|---|
| 運算元順序 | `mov 來源, 目的` | `mov 目的, 來源`（**反過來**） |
| 暫存器 | 加 `%`：`%eax` | 不加：`eax` |
| 常數 | 加 `$`：`$1` | 不加：`1` |
| 大小 | 寫在指令後綴：`movl` | 寫在記憶體運算元前：`DWORD PTR` |
| 記憶體 | `-4(%rbp)`、`(%rdi,%rsi,4)` | `-4[rbp]`、`[rdi+rsi*4]` |
| 本機實例 | `movl $1, -4(%rbp)` | `mov DWORD PTR -4[rbp], 1` |
| 本機實例 | `cmpl -20(%rbp), %eax` | `cmp eax, DWORD PTR -20[rbp]` |

**最容易出錯的是順序**：看到 `cmp`、`sub` 時，AT&T 的「目的」在**右邊**。

---

## 二、暫存器：CPU 裡的 16 個變數

暫存器（register）是 CPU 內部的儲存格，存取不用經過記憶體，所以最快。x86-64 有 **16 個通用暫存器（general-purpose register）**，
每個 64 位元（8 byte）。**同一個暫存器的低位部分有自己的名字**：

| 64 位元 | 低 32 位元 | 低 16 位元 | 低 8 位元 | 慣例用途（System V ABI，第 5 課詳講） |
|---|---|---|---|---|
| `%rax` | `%eax` | `%ax` | `%al` | **回傳值** |
| `%rbx` | `%ebx` | `%bx` | `%bl` | 被呼叫者要保存（callee-saved） |
| `%rcx` | `%ecx` | `%cx` | `%cl` | 第 4 個參數 |
| `%rdx` | `%edx` | `%dx` | `%dl` | 第 3 個參數 |
| `%rsi` | `%esi` | `%si` | `%sil` | **第 2 個參數** |
| `%rdi` | `%edi` | `%di` | `%dil` | **第 1 個參數** |
| `%rbp` | `%ebp` | `%bp` | `%bpl` | 基準指標（base pointer）：`-O0` 用它當「這個函式的區域變數從哪裡算起」 |
| `%rsp` | `%esp` | `%sp` | `%spl` | **堆疊指標（stack pointer）**：永遠指著堆疊頂端，不能拿來亂用 |
| `%r8` | `%r8d` | `%r8w` | `%r8b` | 第 5 個參數 |
| `%r9` | `%r9d` | `%r9w` | `%r9b` | 第 6 個參數 |
| `%r10`、`%r11` | `%r10d`… | `%r10w`… | `%r10b`… | 暫用（呼叫者要保存，caller-saved） |
| `%r12`～`%r15` | `%r12d`… | `%r12w`… | `%r12b`… | 被呼叫者要保存 |

`%rax`、`%rbx`、`%rcx`、`%rdx` 另外還有 `%ah`、`%bh`、`%ch`、`%dh`（第 8～15 位元），是 16 位元時代留下來的，gcc 很少用。

本課只要記住四個：**第 1 個參數在 `%rdi`、第 2 個在 `%rsi`、回傳值放 `%rax`、`%rsp` 是堆疊頂端**。為什麼是這些暫存器、呼叫時堆疊怎麼變，是第 5 課的內容。

除了 16 個通用暫存器，本課還會碰到兩個特殊的：

- **`%rip`（instruction pointer，指令指標）**：存著「下一個要執行的指令在哪裡」。不能直接 `mov` 進去，只有跳躍、`call`、`ret` 會改它。
  可以拿它當基準算位址（`.LC0(%rip)`，見[第六節](#六逐行讀-sum_to-的--o0-版)）。
- **`rflags`（旗標暫存器）**：一堆 1 位元的旗標，由運算指令順便設定，條件跳躍去讀。見[第五節](#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)。

（還有浮點／SIMD 用的 `%xmm0`～`%xmm15` 等暫存器，依[課程規劃](../learning/學習進度.md)不在範圍內。）

### 寫低位的名字時，高位會怎樣？

這條規則很重要，而且不對稱：

| 寫入的大小 | 高位會怎樣 |
|---|---|
| 寫 32 位元名字（`%eax`） | **高 32 位元自動清成 0** |
| 寫 16 位元（`%ax`）或 8 位元（`%al`） | 高位**不動**，保留原本的值 |

所以 `movl %edi, %eax` 不只是寫 `%eax`，整個 `%rax` 都被設定好了（高 32 位元是 0）。用下面這個實驗看它的後果：

```c
/* ops.c：只用 -S 看組合語言，不需要 main */
long widen_s(int x)       { return x; }
long widen_u(unsigned x)  { return x; }
int  get(int *a, long i)  { return a[i]; }
int *addr(int *a, long i) { return &a[i]; }
long times3(long x)       { return x * 3; }
int  lt_s(int a, int b)           { return a < b; }
int  lt_u(unsigned a, unsigned b) { return a < b; }
```

```sh
gcc -S -O2 -fno-asynchronous-unwind-tables ops.c -o ops.s
```

（以下只列每個函式的本體，省略以 `.` 開頭的指示詞，它們是什麼見[第六節](#六逐行讀-sum_to-的--o0-版)。`endbr64` 是什麼也在第六節。）

```asm
widen_s:
	endbr64
	movslq	%edi, %rax
	ret
widen_u:
	endbr64
	movl	%edi, %eax
	ret
```

兩個函式都是「把 32 位元的參數變成 64 位元的 `long` 回傳」，就是[第 2 課第六節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#六有號與無號之間的轉換位元不變換個讀法)〈**變寬（擴展，extension）**〉那一段講的擴展：

- `widen_u`（`unsigned` → `long`）要**零擴展**：高位補 0。剛好「寫 `%eax` 就把高 32 位元清 0」，所以一個普通的 `movl` 就做完了。
- `widen_s`（`int` → `long`）要**符號擴展**：高位補符號位元。普通的 `mov` 做不到，要用專門的 `movslq`。

擴展用的指令有一套命名規則：`movs` 或 `movz`，接「來源大小」，再接「目的大小」：

| 指令 | 讀法 | 意思 |
|---|---|---|
| `movsbw`、`movsbl`、`movsbq` | move sign-extended byte to word／long／quad | 1 byte 符號擴展成 2／4／8 byte |
| `movswl`、`movswq` | … word to long／quad | 2 byte 符號擴展成 4／8 byte |
| `movslq` | … long to quad | 4 byte 符號擴展成 8 byte（Intel 語法叫 `movsxd`） |
| `movzbw`、`movzbl`、`movzbq` | move zero-extended byte to … | 1 byte 零擴展 |
| `movzwl`、`movzwq` | move zero-extended word to … | 2 byte 零擴展 |

沒有 `movzlq`：因為「寫 32 位元自動清高位」，`movl` 就是 4 → 8 byte 的零擴展了。

**C 的型別到這裡，只剩下「選哪個指令」**：`int` 和 `unsigned` 在暫存器裡都是同樣的 32 個位元，差別只在要變寬時用 `movs` 還是 `movz`。

---

## 三、指令的文法：助憶符、大小後綴、運算元

AT&T 語法的一行指令：

```
    addl    %eax, -8(%rbp)
    │  │    │     └─ 目的運算元（destination）：結果寫到這裡
    │  │    └─────── 來源運算元（source）
    │  └──────────── 大小後綴（suffix）：l ＝ 4 byte
    └─────────────── 助憶符（mnemonic）：指令的名字
```

骨架是 `<助憶符>[大小後綴] [來源], [目的]`。逐個位置：

**1. 助憶符**：指令的名字（`mov`、`add`、`cmp`…）。x86-64 有上千個，本課教的見[第八節](#八本課用到的工具選項與完整清單在哪)。

**2. 大小後綴**：這個指令處理幾個 byte。

| 後綴 | 名稱 | byte 數 | C 型別（本機） |
|---|---|---|---|
| `b` | byte | 1 | `char` |
| `w` | word | 2 | `short` |
| `l` | long（double word） | 4 | `int`、`unsigned` |
| `q` | quad word | 8 | `long`、指標 |

（「word」是 2 byte，是 16 位元時代的命名留下來的。注意 AT&T 的 `l` 是 4 byte，**跟 C 的 `long`（8 byte）不一樣**。）

- 運算元裡**有暫存器**時，大小其實看暫存器名字就知道（`%eax` 一定是 4 byte），後綴可以省略。`objdump` 就常省略：上面的 `mov -0x4(%rbp),%eax`。
- 運算元裡**沒有暫存器**時（例如 `movl $0x0,-0x8(%rbp)`：常數搬到記憶體），後綴就**一定要有**——否則不知道要寫 1、2、4 還是 8 個 byte。所以 `objdump` 在這種地方會印出 `movl`、`addl`。
- 省略的規則：有暫存器可以推斷大小時才該省略。兩邊都推不出大小時，本機 `as` 2.42 不會報錯，而是發出警告 ``Warning: no instruction mnemonic suffix given and no register operands; using default for `mov'``，自己挑一個預設大小（實測 `mov $0, -8(%rbp)` 被組譯成 `movl`，4 byte）——**這很危險**，你想寫 8 byte 時它只寫了 4 byte。自己寫組合語言時永遠加後綴。

**3. 運算元（operand）**：有三種，用前綴區分。

| 種類 | 寫法 | 例子 | 意思 |
|---|---|---|---|
| 立即數（immediate） | `$` 開頭 | `$1`、`$0x64`、`$5050` | 常數本身 |
| 暫存器 | `%` 開頭 | `%eax`、`%rdi` | 暫存器裡的值 |
| 記憶體 | `D(B,I,S)` 或它的省略形式 | `-4(%rbp)`、`(%rdi,%rsi,4)` | 算出一個位址，**用的是那個位址裡的值** |

記憶體運算元的完整形式是 `D(B,I,S)`，**位址 ＝ D ＋ B ＋ I × S**：

| 位置 | 名稱 | 可以是什麼 | 省略時 |
|---|---|---|---|
| `D` | 位移（displacement） | 常數（可以是負數）或符號名稱（`.LC0`） | 0 |
| `B` | 基底（base） | 一個 64 位元暫存器 | 0 |
| `I` | 索引（index） | 一個 64 位元暫存器（不能是 `%rsp`） | 0 |
| `S` | 倍數（scale） | **只能是 1、2、4、8** | 1 |

常見的省略形式：

| 寫法 | 位址 | 典型用途 | 本課實例 |
|---|---|---|---|
| `(B)` | B | `*p` | `addq (%rdi), %rax`（作業第 4 題） |
| `D(B)` | B ＋ D | 區域變數、結構成員 | `-4(%rbp)`：`%rbp` 往下 4 byte，就是 `i` |
| `(B,I,S)` | B ＋ I × S | `a[i]` | `(%rdi,%rsi,4)` |
| `D(B,I,S)` | B ＋ I × S ＋ D | `a[i + 常數]` | `1(%rdx,%rax,2)`（第七節，拿來算數） |
| `D(,I,S)` | I × S ＋ D | 沒有基底，只要倍數 | `0(,%rdi,8)`（作業第 1 題會看到） |
| `D(%rip)` | 下一個指令的位址 ＋ D | 取全域資料的位址（PIE 用） | `.LC0(%rip)` |

**S 只能是 1、2、4、8，正好是 `char`、`short`、`int`、`long`／指標的大小**——這就是硬體直接支援
[第 3 課第二節](./陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#二指標運算p--1-走一個元素不是一個-byte)的指標運算：`a + i` 走 `i × sizeof(*a)` 個 byte。

一條限制：**一個指令最多只能有一個記憶體運算元**。要把記憶體搬到記憶體，得先搬進暫存器再搬出去——這是第六節 `-O0` 裡一直出現 `movl -4(%rbp), %eax` 的原因之一。

---

## 四、`mov` 與 `lea`：搬資料 vs 算位址

### `mov`：把來源複製到目的

`mov S, D`：D ＝ S。來源和目的可以是暫存器或記憶體（不能兩個都是記憶體），來源也可以是立即數。
**有記憶體運算元時，`mov` 真的會去讀或寫那個位址**。

### `lea`：只算位址，不碰記憶體

`lea`（load effective address）的來源**寫法跟記憶體運算元一樣**，但它**只把算出來的位址放進目的暫存器，不去讀那個位址**。
對照 `ops.c` 的兩個函式（[第 3 課](./陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#二指標運算p--1-走一個元素不是一個-byte)：`a[i]` 就是 `*(a + i)`，`&a[i]` 就是 `a + i`）：

```asm
get:                                  # int  get(int *a, long i)  { return a[i]; }
	endbr64
	movl	(%rdi,%rsi,4), %eax       # %eax = 記憶體[a + i×4] 裡的 4 byte → 真的讀記憶體
	ret
addr:                                 # int *addr(int *a, long i) { return &a[i]; }
	endbr64
	leaq	(%rdi,%rsi,4), %rax       # %rax = a + i×4 這個數字本身 → 不讀記憶體
	ret
```

- 第 1 個參數 `a` 在 `%rdi`，第 2 個參數 `i` 在 `%rsi`，倍數 `4` 是 `sizeof(int)`。
- `get` 回傳 `int`（4 byte），用 `movl` 和 `%eax`；`addr` 回傳指標（8 byte），用 `leaq` 和 `%rax`。
- **兩行的括號一模一樣**，差別是 `mov` 會去那個位址取值（`*`），`lea` 只要位址本身（`&`）。所以 `lea` 碰到不合法的位址也不會 segfault，因為它根本沒去存取。

### `lea` 被拿來做算術

`lea` 本質上是「一個指令算出 `D + B + I×S`」，跟位址無關的計算也能借用它：

```asm
times3:                               # long times3(long x) { return x * 3; }
	endbr64
	leaq	(%rdi,%rdi,2), %rax       # %rax = x + x×2 = 3x
	ret
```

`x` 不是位址，但 `(%rdi,%rdi,2)` 算出來就是 `x + 2x`。gcc 很愛這樣用：一個指令就做完「乘一個小常數再加東西」，
而且**不會改旗標**（`add` 會），也可以把結果放到另一個暫存器（`add` 只能寫回其中一個運算元）。第七節 `-O2` 的迴圈本體就是一個 `lea`。

**判斷原則：看到 `lea`，不要假設它跟位址有關；問自己「它算出什麼數字」就好。**

### 本課的算術與邏輯指令

| 指令 | 效果（AT&T：`op S, D`） | 改不改旗標 | 本課在哪出現 |
|---|---|---|---|
| `add S, D` | D ＝ D ＋ S | 改 | `addl %eax, -8(%rbp)`（`total += i`） |
| `sub S, D` | D ＝ D − S | 改 | 作業第 1 題（`x * 7`） |
| `imul S, D` | D ＝ D × S（有號乘法） | 改 | 本課沒出現，gcc 能用 `lea`／`add` 時就不用它 |
| `and S, D` | D ＝ D & S（逐位元且） | 改 | `andl $1, %edi`（第七節：`n` 是不是奇數） |
| `xor S, D` | D ＝ D ^ S（逐位元互斥或） | 改 | `xorl %eax, %eax`（清成 0） |
| `mov`、`movs*`、`movz*`、`lea` | 見上面 | **不改** | |

**`xorl %eax, %eax` 是「把 `%eax` 設成 0」的慣用寫法**：任何數跟自己 xor 都是 0。它的機器碼只有 2 byte（`31 c0`），
比 `movl $0, %eax`（5 byte）短，而且寫的是 32 位元名字，連 `%rax` 的高位也一起清掉。`-O2` 幾乎都這樣寫；`-O0` 則照字面寫 `movl $0, ...`。

---

## 五、旗標與條件跳躍：`cmp` 只負責比，跳不跳由下一個指令決定

C 的 `if`、`for`、`while` 在組合語言裡都變成兩步：

1. **`cmp` 或 `test`**：做一次運算，**只設旗標，不存結果**。
2. **條件跳躍 `j<條件>`**（或 `set<條件>`）：讀旗標，決定跳不跳（或把 0／1 寫進一個 byte）。

### 四個旗標

`rflags` 裡跟比較有關的四個旗標，都是「上一個會改旗標的指令」順手設定的：

| 旗標 | 名稱 | 什麼時候是 1 | 對誰有意義 |
|---|---|---|---|
| ZF | Zero Flag | 結果是 0 | 兩者都適用（相等） |
| SF | Sign Flag | 結果的最高位元是 1（當成有號數是負的） | 有號 |
| CF | Carry Flag | 當成**無號數**運算時，進位或借位了（`cmp` 時就是「D 比 S 小（無號）」） | **無號** |
| OF | Overflow Flag | 當成**有號數**運算時溢位了（結果超出範圍，[第 2 課第八節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#八溢位無號回繞有號是未定義行為)的那種溢位） | **有號** |

（`rflags` 還有 PF、AF 等其他旗標，`gdb` 的 `info registers eflags` 會把設成 1 的都列出來，例如 `IF` 是「允許中斷」，在使用者程式裡永遠是 1，跟比較無關。）

### `cmp` 與 `test`

| 指令 | 算什麼 | 結果 | 典型用途 |
|---|---|---|---|
| `cmp S, D` | D − S | **丟掉**，只設旗標 | 比大小：`cmpl -20(%rbp), %eax` 就是比 `i` 和 `n` |
| `test S, D` | D & S | **丟掉**，只設旗標（CF、OF 清成 0） | `testl %edi, %edi`：看 `n` 是 0、負數還是正數 |

**AT&T 的順序要特別小心**：`cmp S, D` 算的是 **D − S**，所以 `cmpl %esi, %edi; jl` 的意思是「`%edi` < `%esi` 就跳」，**右邊的那個在比較式的左邊**。

### 條件碼：全部 16 種

條件跳躍 `j<cc>` 與條件設定 `set<cc>` 用同一套條件碼（condition code）`<cc>`。x86 的條件總共 16 種，很多有好幾個別名（同一個機器碼），**gcc 和 objdump 會挑其中一個印**：

| 條件碼（含別名） | 讀法 | 看哪些旗標 | 在 `cmp S, D` 之後的意思 | 類別 |
|---|---|---|---|---|
| `e`、`z` | equal／zero | ZF = 1 | D == S | 都適用 |
| `ne`、`nz` | not equal | ZF = 0 | D != S | 都適用 |
| `l`、`nge` | less | SF ≠ OF | D < S | **有號** |
| `le`、`ng` | less or equal | ZF = 1 或 SF ≠ OF | D <= S | **有號** |
| `g`、`nle` | greater | ZF = 0 且 SF = OF | D > S | **有號** |
| `ge`、`nl` | greater or equal | SF = OF | D >= S | **有號** |
| `b`、`nae`、`c` | below | CF = 1 | D < S | **無號** |
| `be`、`na` | below or equal | CF = 1 或 ZF = 1 | D <= S | **無號** |
| `a`、`nbe` | above | CF = 0 且 ZF = 0 | D > S | **無號** |
| `ae`、`nb`、`nc` | above or equal | CF = 0 | D >= S | **無號** |
| `s` | sign | SF = 1 | 結果是負的 | 單一旗標 |
| `ns` | not sign | SF = 0 | 結果不是負的 | 單一旗標 |
| `o` | overflow | OF = 1 | 有號溢位了 | 單一旗標 |
| `no` | not overflow | OF = 0 | 沒有有號溢位 | 單一旗標 |
| `p`、`pe` | parity even | PF = 1 | 結果最低 byte 有偶數個 1 | 單一旗標（浮點比較會用到） |
| `np`、`po` | parity odd | PF = 0 | 有奇數個 1 | 單一旗標 |

`n` 開頭就是「not」：`jnb`（not below）＝ `jae`（above or equal），同一個指令。gcc 有時印 `setnb`、`jnb`，看到時翻成「≥（無號）」就好。

**記法：less／greater 是有號，below／above 是無號。**

| 用到條件碼的指令 | 做什麼 |
|---|---|
| `j<cc> 標籤` | 條件成立就跳到標籤，不成立就往下執行 |
| `jmp 標籤` | 無條件跳躍 |
| `set<cc> 8位元暫存器` | 條件成立就把那個 byte 設成 1，否則 0。常配合 `xorl %eax, %eax` 先清 0，因為 `set` 只寫最低的 1 byte |

（還有 `cmov<cc>`：條件成立才搬資料，`-O2` 會用它取代短的 `if`。本課沒出現，第 5 課之後遇到再講。）

### 實驗：同一個 `cmp`、同一組旗標，`setl` 說是、`setb` 說否

`ops.c` 的 `lt_s` 和 `lt_u` 只差在參數是 `int` 還是 `unsigned`：

```asm
lt_s:                                 # int lt_s(int a, int b) { return a < b; }
	endbr64
	xorl	%eax, %eax                # %eax = 0
	cmpl	%esi, %edi                # 算 a − b，只設旗標
	setl	%al                       # 有號 a < b 就把 %al 設成 1
	ret
lt_u:                                 # int lt_u(unsigned a, unsigned b) { return a < b; }
	endbr64
	xorl	%eax, %eax
	cmpl	%esi, %edi                # 完全一樣的比較
	setb	%al                       # 無號 a < b（below）就設成 1
	ret
```

**兩個函式只差一個字母**。用 `gdb` 在 `set` 那一行停下，看 `cmp` 剛做完時的旗標：

```c
/* lt.c：用 -O2 編，跟 ops.c 的 lt_s、lt_u 相同 */
int lt_s(int a, int b)           { return a < b; }
int lt_u(unsigned a, unsigned b) { return a < b; }
```

```c
/* ltmain.c：另外一個檔，這樣 -O2 的 lt.c 不會被內嵌進 main */
#include <stdio.h>

int lt_s(int a, int b);
int lt_u(unsigned a, unsigned b);

int main(void)
{
    printf("lt_s(-1, 1) = %d\n", lt_s(-1, 1));
    printf("lt_u(-1, 1) = %d\n", lt_u(-1, 1));
    return 0;
}
```

```sh
gcc -O2 -g -c lt.c
gcc -O0 -g ltmain.c lt.o -o lt
./lt
gdb -q -batch -ex 'break *lt_s+8' -ex 'break *lt_u+8' -ex run \
    -ex 'x/i $pc' -ex 'info registers eflags' -ex continue \
    -ex 'x/i $pc' -ex 'info registers eflags' ./lt
```

```
lt_s(-1, 1) = 1
lt_u(-1, 1) = 0
```

```
Breakpoint 1, 0x00005555555551b8 in lt_s (a=-1, b=1) at lt.c:1
=> 0x5555555551b8 <lt_s+8>:	setl   %al
eflags         0x282               [ SF IF ]
Breakpoint 2, 0x00005555555551c8 in lt_u (a=4294967295, b=1) at lt.c:2
=> 0x5555555551c8 <lt_u+8>:	setb   %al
eflags         0x282               [ SF IF ]
```

（`break *lt_s+8` 的 `*` 表示「在這個**位址**停」，而不是在某一行原始碼；`+8` 是 `setl` 在函式裡的位置，從 `gdb -batch -ex 'disassemble lt_s' ./lt` 看來的：
`<+0>` `endbr64`、`<+4>` `xor`、`<+6>` `cmp`、`<+8>` `setl`。位址一定要對準某個指令的**開頭**：本機第一次試時打成 `+10`，停在 `setl` 的中間，gdb 插進去的中斷點 byte 把指令弄壞了，程式印出 `lt_s(-1, 1) = 256`——指令長度不固定，就有這種陷阱。`info registers` 與 `x/i` 的完整說明見 [gdb完整命令參考](./gdb完整命令參考.md)，第 5 課會正式用它們觀察暫存器。）

逐步算：

1. `-1` 的位元是 `0xffffffff`（[第 2 課第五節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#五有號整數二補數)），兩個函式拿到的 `%edi` **位元完全一樣**；gdb 只是依照除錯資訊的型別，一個印成 `a=-1`、一個印成 `a=4294967295`。
2. `cmpl %esi, %edi` 算 `0xffffffff − 0x00000001 = 0xfffffffe`：
   - 結果不是 0 → ZF = 0
   - 最高位元是 1 → **SF = 1**
   - 無號看：4294967295 − 1，不用借位 → **CF = 0**
   - 有號看：−1 − 1 = −2，沒溢位 → **OF = 0**
3. gdb 印的 `[ SF IF ]` 就是「只有 SF（和無關的 IF）是 1」，**兩個函式的旗標一模一樣**。
4. `setl` 看 SF ≠ OF：1 ≠ 0 → 成立 → 回傳 1（有號：−1 < 1）。
   `setb` 看 CF = 1：CF 是 0 → 不成立 → 回傳 0（無號：4294967295 < 1 不成立）。

**`cmp` 不知道也不在乎你的資料是有號還是無號，它把兩種解讀需要的旗標一次都設好；是 gcc 依照 C 的型別，挑了 `l` 或 `b` 去讀。**
這就是[第 2 課第七節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#七陷阱有號和無號混在一起比較)「`-1 < 0u` 為假」在機器上的樣子：`int` 遇上 `unsigned` 被轉成無號，gcc 就改用 `b`。

### 哪些指令會改旗標

**算術和邏輯指令（`add`、`sub`、`and`、`xor`、`cmp`、`test`…）會改旗標；`mov` 系列、`lea`、跳躍不會。**
所以 gcc 可以把 `cmp`／`and` 和真正讀旗標的 `j<cc>` 中間夾幾個 `mov`，旗標不會被破壞——第七節 `-O2` 就這樣排。
反過來，`cmp` 和 `j<cc>` 之間只要夾了一個 `add`，旗標就變成 `add` 的結果了。

---

## 六、逐行讀 `sum_to` 的 `-O0` 版

### 先看完整的 `.s` 檔：哪些是指令、哪些不是

```sh
gcc -S -O0 sumto.c -o sumto0_raw.s
```

`sum_to` 那一段（完整檔案還有 `answer`、`main` 與檔尾的 `.note.gnu.property`）：

```asm
	.file	"sumto.c"
	.text
	.globl	sum_to
	.type	sum_to, @function
sum_to:
.LFB0:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, -20(%rbp)
	...
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	sum_to, .-sum_to
```

一個 `.s` 檔裡有三種行：

| 種類 | 長相 | 例子 | 是什麼 |
|---|---|---|---|
| **標籤（label）** | 名字加冒號，頂格寫 | `sum_to:`、`.L2:`、`.LFB0:` | 給「這個位置」取名字，讓跳躍、`call` 可以指到這裡。`.L` 開頭的是 gcc 自動產生的**區域標籤**，不會進符號表（`nm` 看不到） |
| **指示詞（directive，也叫 pseudo-op）** | `.` 開頭 | `.text`、`.globl`、`.cfi_offset` | **給組譯器 `as` 的指令**，不是 CPU 的指令，不會變成機器碼（或只變成資料） |
| **指令（instruction）** | 助憶符開頭 | `movl`、`ret` | CPU 要執行的東西，每一行變成一段機器碼 |

本課的 `.s` 檔裡出現過的指示詞（全部列出）：

| 指示詞 | 意思 |
|---|---|
| `.file "sumto.c"` | 記錄原始檔名（給除錯資訊用） |
| `.text` | 接下來的東西放進 `.text` 區段（程式碼） |
| `.section 名稱[,旗標,型別]` | 接下來放進指定的區段，例如 `.section .rodata`（[ELF是什麼](./ELF是什麼-可執行與可連結格式的兩種視角.md)的區段） |
| `.globl sum_to` | 這個符號要讓其他 `.o` 看得到（對應 C 裡沒加 `static` 的函式）；沒寫的就只在本檔可見 |
| `.type sum_to, @function` | 在符號表標明這個符號是函式（`nm` 顯示 `T` 時靠它） |
| `.size sum_to, .-sum_to` | 符號的大小：`.` 是「目前位置」，`.-sum_to` 就是「從 `sum_to` 到這裡的 byte 數」 |
| `.string "answer:%d\n"` | 放一個以 `\0` 結尾的字串（[第 3 課第四節](./陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#四字串以-0-結尾的-char-陣列)） |
| `.p2align 4`、`.p2align 4,,10` | 對齊：把目前位置墊到 2⁴ ＝ 16 的倍數（墊的是什麼都不做的 `nop` 指令）；第三個參數是「最多墊幾個 byte，超過就不墊」 |
| `.align 8`、`.long 數值` | 對齊到 8；放一個 4 byte 的整數（出現在檔尾的 `.note.gnu.property`） |
| `.ident "GCC: ..."` | 記錄是哪個編譯器產生的（放進 `.comment` 區段） |
| `.cfi_startproc`、`.cfi_endproc`、`.cfi_def_cfa_offset`、`.cfi_offset`、`.cfi_def_cfa_register`、`.cfi_def_cfa` | CFI（Call Frame Information，呼叫框資訊）：告訴除錯器和例外處理「這個函式此刻的堆疊框長怎樣」，`gdb` 的 `bt` 靠它往回找呼叫者。跟程式邏輯無關 |

讀程式邏輯時，`.cfi_*` 只是干擾，可以用 `-fno-asynchronous-unwind-tables` 叫 gcc 不要產生（程式碼完全不變，只是少了那些資訊）。下面都這樣編。
完整的指示詞清單在 GNU as 手冊，見[第八節](#八本課用到的工具選項與完整清單在哪)。

### 對照原始碼：`-fverbose-asm`

`-fverbose-asm` 讓 gcc 在 `.s` 裡加註解：每段來自原始碼哪一行，以及每個運算元對應哪個變數。

```sh
gcc -S -O0 -fverbose-asm -fno-asynchronous-unwind-tables sumto.c -o sumto0_v.s
```

```asm
sum_to:
	endbr64	
	pushq	%rbp	#
	movq	%rsp, %rbp	#,
	movl	%edi, -20(%rbp)	# n, n
# sumto.c:5:     int total = 0;
	movl	$0, -8(%rbp)	#, total
# sumto.c:6:     for (int i = 1; i <= n; i++)
	movl	$1, -4(%rbp)	#, i
# sumto.c:6:     for (int i = 1; i <= n; i++)
	jmp	.L2	#
.L3:
# sumto.c:7:         total += i;
	movl	-4(%rbp), %eax	# i, tmp84
	addl	%eax, -8(%rbp)	# tmp84, total
# sumto.c:6:     for (int i = 1; i <= n; i++)
	addl	$1, -4(%rbp)	#, i
.L2:
# sumto.c:6:     for (int i = 1; i <= n; i++)
	movl	-4(%rbp), %eax	# i, tmp85
	cmpl	-20(%rbp), %eax	# n, tmp85
	jle	.L3	#,
# sumto.c:8:     return total;
```

`# n, n`、`# i, tmp84` 是「來源, 目的」對應的變數名；`tmp84` 是 gcc 內部的暫存值，沒有對應的 C 變數。

### 逐行

| 指令 | 做什麼 | 對應的 C |
|---|---|---|
| `endbr64` | 「合法的跳躍目標」標記。Intel CET（Control-flow Enforcement Technology，控制流程強制技術）的一部分：開了這個保護的 CPU 上，間接跳躍只能落在 `endbr64` 上，用來擋攻擊者亂跳。Ubuntu 的 gcc 預設 `-fcf-protection` 會在每個函式開頭放一個；舊 CPU 上它什麼都不做 | 無 |
| `pushq %rbp` | 把呼叫者的 `%rbp` 存到堆疊上 | 無（函式開頭的固定套路，第 5 課） |
| `movq %rsp, %rbp` | 讓 `%rbp` 指著目前的堆疊頂端，之後的區域變數都用「`%rbp` 往下幾 byte」表示 | 無（同上） |
| `movl %edi, -20(%rbp)` | 第 1 個參數 `n` 在 `%edi`，存到 `%rbp − 20` | 參數 `n` 放進記憶體 |
| `movl $0, -8(%rbp)` | `%rbp − 8` 寫 4 byte 的 0 | `int total = 0;` |
| `movl $1, -4(%rbp)` | `%rbp − 4` 寫 4 byte 的 1 | `int i = 1;` |
| `jmp .L2` | **先跳到迴圈底部去檢查條件** | `for` 一開始先檢查 `i <= n` |
| `.L3:` `movl -4(%rbp), %eax` | 讀 `i` 進 `%eax` | 迴圈本體開始 |
| `addl %eax, -8(%rbp)` | 記憶體裡的 `total` 加上 `%eax` | `total += i;` |
| `addl $1, -4(%rbp)` | 記憶體裡的 `i` 加 1 | `i++` |
| `.L2:` `movl -4(%rbp), %eax` | 再讀一次 `i` | 檢查條件 |
| `cmpl -20(%rbp), %eax` | 算 `i − n`，設旗標 | `i <= n` 的比較 |
| `jle .L3` | **有號**小於或等於就跳回本體 | 條件成立就再跑一圈（`int` 是有號，所以用 `le` 不是 `be`） |
| `movl -8(%rbp), %eax` | 讀 `total` 進 `%eax`——回傳值放 `%eax` | `return total;` |
| `popq %rbp` | 把呼叫者的 `%rbp` 拿回來 | 無（第 5 課） |
| `ret` | 回到呼叫者 | 函式結束 |

從這張表看出 `-O0` 的三個特徵：

1. **每個變數都住在記憶體裡的固定位置**：`n` 在 `-20(%rbp)`、`total` 在 `-8(%rbp)`、`i` 在 `-4(%rbp)`。每次用都從記憶體讀，改完就寫回去，即使下一行馬上又要用（`.L3` 的 `addl $1, -4(%rbp)` 寫回 `i`，下一行 `.L2` 又把它讀出來）。
2. **每一行 C 都對得到一小段組合語言**，順序也一樣。
3. **迴圈的形狀**：`for` 被翻成「先 `jmp` 到底部檢查，條件成立再跳回頂端」。這樣每圈只需要一個條件跳躍。用 C 寫出來就是：

```c
    i = 1;
    goto test;
loop:
    total += i;
    i++;
test:
    if (i <= n) goto loop;
```

`-O0` 這樣寫是為了**除錯**：變數一直在記憶體裡，`gdb` 隨時 `print total` 都讀得到正確的值，停在任何一行也都對應得到原始碼（[gcc最佳化等級第三節](./gcc最佳化等級-O0到O3差在哪.md#三為什麼除錯要用--o0)）。代價是每圈有 5 個指令要碰記憶體（其中兩個 `addl` 還是先讀再寫）。

### 順便看 `answer` 和 `main`

```asm
answer:
	endbr64
	pushq	%rbp
	movq	%rsp, %rbp
	movl	$100, %edi              # 第 1 個參數 = 100
	call	sum_to                  # 呼叫 sum_to，回傳值會在 %eax
	popq	%rbp
	ret                             # %eax 原封不動，直接當 answer 的回傳值
	.section	.rodata
.LC0:
	.string	"answer:%d\n"           # 字串常數放進 .rodata（第 3 課第四節）
	.text
main:
	endbr64
	pushq	%rbp
	movq	%rsp, %rbp
	call	answer                  # %eax = answer()
	movl	%eax, %esi              # 第 2 個參數 = answer() 的結果
	leaq	.LC0(%rip), %rax        # %rax = 字串的位址（不讀字串內容，所以是 lea）
	movq	%rax, %rdi              # 第 1 個參數 = 字串的位址
	movl	$0, %eax                # 可變參數函式的規定，第 5 課講
	call	printf@PLT
	movl	$0, %eax                # return 0;
	popq	%rbp
	ret
```

- 參數照順序放 `%edi`（`%rdi`）、`%esi`（`%rsi`），回傳值在 `%eax`——第二節表格的四個暫存器全部出現了。
- `"answer:%d\n"` 被 `.section .rodata` 放進唯讀資料區段，正是[第 3 課第四節](./陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#四字串以-0-結尾的-char-陣列)說的「字串常數在 `.rodata`」。
- 取它的位址用 `leaq .LC0(%rip)`：以「下一個指令的位址」為基準加一個距離。因為程式是 PIE（[ELF是什麼第三節](./ELF是什麼-可執行與可連結格式的兩種視角.md#三一種格式四種檔案)），載入位址每次不同，不能寫死絕對位址，但「字串離這行程式碼多遠」是固定的。
- `call printf@PLT`：經過 PLT 呼叫共享函式庫裡的函式，[第 1 課](./hello.c怎麼變成能跑的程式-編譯的四個階段.md)見過；`call`／`ret` 怎麼運作是第 5 課。

---

## 七、逐行讀 `sum_to` 的 `-O2` 版

```sh
gcc -S -O2 -fno-asynchronous-unwind-tables sumto.c -o sumto2.s
```

```asm
sum_to:
	endbr64
	testl	%edi, %edi
	jle	.L4
	leal	1(%rdi), %ecx
	xorl	%edx, %edx
	andl	$1, %edi
	movl	$1, %eax
	je	.L3
	movl	$2, %eax
	movl	$1, %edx
	cmpl	%ecx, %eax
	je	.L1
	.p2align 4,,10
	.p2align 3
.L3:
	leal	1(%rdx,%rax,2), %edx
	addl	$2, %eax
	cmpl	%ecx, %eax
	jne	.L3
.L1:
	movl	%edx, %eax
	ret
	.p2align 4,,10
	.p2align 3
.L4:
	xorl	%edx, %edx
	movl	%edx, %eax
	ret
	...
answer:
	endbr64
	movl	$5050, %eax
	ret
```

`answer` 只剩兩行：`sum_to(100)` 被內嵌後在編譯時算成 5050（[gcc最佳化等級第二節](./gcc最佳化等級-O0到O3差在哪.md#二實驗同一段程式不同等級編出什麼)）。下面讀 `sum_to` 本身。

### 先找出「誰是誰」

`-O2` 沒有 `-20(%rbp)` 這種記憶體位置了，**變數全部住在暫存器裡**。第一步是從用法推出每個暫存器代表什麼：

| 暫存器 | 代表 | 怎麼看出來的 |
|---|---|---|
| `%edi` | `n` | 第 1 個參數 |
| `%eax` | `i` | 從 1（或 2）開始，每圈 `+2`，跟 `%ecx` 比 |
| `%edx` | `total` | 從 0 開始，最後 `movl %edx, %eax` 當回傳值 |
| `%ecx` | `n + 1` | `leal 1(%rdi), %ecx`：迴圈的終點 |

### 逐行

| 指令 | 做什麼 | 對應的意思 |
|---|---|---|
| `testl %edi, %edi` | `n & n`（就是 `n`），設旗標 | 看 `n` 是正、零還是負 |
| `jle .L4` | ZF = 1 或 SF ≠ OF 就跳；`test` 會把 OF 清 0，所以等於「結果是 0 或負的」 | `n <= 0` 時迴圈一圈都不跑，直接跳去回傳 0 |
| `leal 1(%rdi), %ecx` | `%ecx = n + 1`（`lea` 拿來做加法） | 終點：`i` 到 `n + 1` 就停 |
| `xorl %edx, %edx` | `%edx = 0` | `total = 0` |
| `andl $1, %edi` | `%edi = n & 1`，設旗標（結果是 0 時 ZF = 1） | `n` 是不是奇數 |
| `movl $1, %eax` | `%eax = 1`，**不改旗標** | `i = 1` |
| `je .L3` | 看的是 **`andl` 設的** ZF：`n & 1 == 0`（偶數）就直接進迴圈 | 偶數：從 `i = 1` 開始兩個兩個加 |
| `movl $2, %eax` | `%eax = 2` | 奇數：先把 `i = 1` 這一個加掉…… |
| `movl $1, %edx` | `%edx = 1` | ……`total = 1`，下一個從 `i = 2` 開始 |
| `cmpl %ecx, %eax` `je .L1` | `i == n + 1` 就結束 | `n == 1` 時已經加完了 |
| `.L3:` `leal 1(%rdx,%rax,2), %edx` | `%edx = total + i×2 + 1` | `total += i + (i + 1)`：**一圈加兩個數** |
| `addl $2, %eax` | `i += 2` | |
| `cmpl %ecx, %eax` `jne .L3` | `i != n + 1` 就再一圈 | 用「不等於終點」取代 `i <= n` |
| `.L1:` `movl %edx, %eax` `ret` | 回傳 `total` | |
| `.L4:` `xorl %edx, %edx` … | 回傳 0 | `n <= 0` 的出口 |
| `.p2align 4,,10` 等 | 對齊，讓迴圈開頭落在 16 byte 邊界（`objdump` 會看到中間墊了 `nopl` 之類的空指令） | CPU 讀取指令比較有效率 |

**`movl $1, %eax` 夾在 `andl` 和 `je` 中間**，這是[第五節](#哪些指令會改旗標)「`mov` 不改旗標」的實際應用：gcc 把不相干的指令塞進比較和跳躍之間，`je` 讀到的仍然是 `andl` 的結果。

### 手動追蹤：`n = 4` 和 `n = 5`

| 步驟 | `n = 4`（偶數） | `n = 5`（奇數） |
|---|---|---|
| 進迴圈前 | `%ecx = 5`；`4 & 1 = 0` → `je` 成立；`i = 1`、`total = 0` | `%ecx = 6`；`5 & 1 = 1` → `je` 不成立；`i = 2`、`total = 1`；`2 != 6` |
| 第 1 圈 | `total = 0 + 1×2 + 1 = 3`（1 + 2）；`i = 3`；`3 != 5` 繼續 | `total = 1 + 2×2 + 1 = 6`（+2 +3）；`i = 4`；`4 != 6` 繼續 |
| 第 2 圈 | `total = 3 + 3×2 + 1 = 10`（+3 +4）；`i = 5`；`5 == 5` 結束 | `total = 6 + 4×2 + 1 = 15`（+4 +5）；`i = 6`；`6 == 6` 結束 |
| 回傳 | 10 ＝ 1+2+3+4 ✓ | 15 ＝ 1+2+3+4+5 ✓ |

### `-O0` 與 `-O2` 對照

| | `-O0` | `-O2` |
|---|---|---|
| 變數住哪 | 記憶體（`-4(%rbp)` 等） | 暫存器（`%eax`、`%edx`） |
| 每圈碰記憶體的指令 | 5 個 | 0 個 |
| 每圈加幾個數 | 1 個 | 2 個（先處理奇數多出來的那一個） |
| 迴圈條件 | `i <= n`（`jle`） | `i != n + 1`（`jne`） |
| `sum_to` 的指令數 | 16 個（`endbr64` 到 `ret`） | 21 個（不含對齊用的指示詞）——**變多了**，換來每圈比較少的工作 |
| 原始碼對得上嗎 | 一行對一段 | 重排過，`i`、`total` 有時不在任何固定的地方 |

最後一列就是 [gcc最佳化等級第三節](./gcc最佳化等級-O0到O3差在哪.md#三為什麼除錯要用--o0)說的 `<optimized out>` 的來源：`-O2` 的 `total` 只存在 `%edx` 裡，而且迴圈裡它一次跳兩個數，`gdb` 停在某一圈時，「`total` 現在是多少」在原始碼的意義上可能根本不存在。

---

## 八、本課用到的工具、選項與完整清單在哪

### `gcc`：本課用到的選項

gcc 的選項有上千個，下面只列本課用到的 6 個，完整清單見 `man gcc`（第 1 課的 [hello.c 筆記](./hello.c怎麼變成能跑的程式-編譯的四個階段.md)與 [gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md)有其他常用的）：

| 選項 | 做什麼 |
|---|---|
| `-S` | 編譯到組合語言就停，輸出 `.s` |
| `-O0`、`-O2` | 最佳化等級（[gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md)） |
| `-fno-asynchronous-unwind-tables` | 不產生 `.cfi_*` 指示詞（程式碼不變），讓 `.s` 比較好讀 |
| `-fverbose-asm` | 在 `.s` 裡加註解：原始碼行號與每個運算元對應的變數 |
| `-masm=intel` | 改用 Intel 語法（預設 `-masm=att`） |
| `-fcf-protection=none` | 不產生 `endbr64`（Ubuntu 預設開著 `-fcf-protection`）。本課沒用，想讓輸出再短一點時可以加 |

### `objdump`：本課用到的選項

| 選項 | 做什麼 |
|---|---|
| `-d` | 反組譯有程式碼的區段 |
| `--no-show-raw-insn` | 不印機器碼那一欄，只留位置和指令 |
| `-M intel` | 反組譯成 Intel 語法（`-M` 是「傳給反組譯器的選項」） |
| `-S` | 原始碼和反組譯交錯顯示（要用 `-g` 編譯） |
| `--visualize-jumps` | 用 ASCII 線畫出跳躍的來源和目標，讀迴圈很方便 |

`objdump` 的「顯示什麼」一組（`-d`、`-S` 在內）全部 31 項已經列在 [objdump能不能代替ldd第六節](./objdump能不能代替ldd找依賴庫-需求單與實際載入.md#六本篇用到的工具選項)。
這裡補上 `-M` 在 x86 上能接的**全部** 15 個值（`objdump --help` 最後一段實測）：

| `-M` 的值 | 做什麼 |
|---|---|
| `x86-64`、`i386`、`i8086` | 以 64／32／16 位元模式反組譯 |
| `att`、`intel` | AT&T（預設）／Intel 語法 |
| `att-mnemonic`、`intel-mnemonic` | 在 AT&T 語法下，助憶符用 AT&T／Intel 的名字（例如 `movslq` vs `movsxd`） |
| `addr64`、`addr32`、`addr16` | 假設位址大小是 64／32／16 位元 |
| `data32`、`data16` | 假設資料大小是 32／16 位元 |
| `suffix` | AT&T 語法下**一律**印大小後綴（預設只在推不出大小時印） |
| `amd64`、`intel64` | 依 AMD64 或 Intel64 的指令集解讀（兩家有少數指令不同） |

### 指令與指示詞的完整清單

- **指令**：x86-64 的助憶符有上千個（還會隨著新的指令集擴充增加，所以沒有一個固定的數字）。本課教了 13 類：
  `mov`、`movs*`、`movz*`、`lea`、`add`、`sub`、`imul`、`and`、`xor`、`cmp`、`test`、`jmp`／`j<cc>`、`set<cc>`；
  另外看到但留到第 5 課的有 `push`、`pop`、`call`、`ret`，以及 `endbr64`、`nop`。
  完整清單與每個指令的精確定義在 Intel 的 *Intel® 64 and IA-32 Architectures Software Developer's Manual* 第 2 卷（指令集參考）；
  線上可查的整理版：<https://www.felixcloutier.com/x86/>。**讀 gcc 的輸出很少需要本課以外的指令**，遇到不認得的再查。
- **指示詞**：GNU as 手冊的〈Assembler Directives〉一章：<https://sourceware.org/binutils/docs/as/Pseudo-Ops.html>。
  線上最新版的通用指示詞清單有 115 項，`.cfi_*` 另外在〈CFI directives〉一頁（數出 27 個），x86 專用的又在〈i386-Dependent〉一章。
  本機沒有裝 `as` 的 info 手冊（`info as` 找不到），本機版本是 2.42，跟線上最新版可能差幾項。本課出現的已經在[第六節](#先看完整的-s-檔哪些是指令哪些不是)全部列出。
- **`gdb` 看組合語言的命令**（`disassemble`、`x/i`、`stepi`、`info registers`）：[gdb完整命令參考](./gdb完整命令參考.md)，第 5 課會實際用。

---

## 九、動手作業

作業在 [`small-project/s1-04-asm-basics/`](../small-project/s1-04-asm-basics/README.md)，共 5 題：
`lea` 當乘法、不同型別的陣列存取、有號／無號的分支、讀一個陣列加總迴圈的 `-O0` 與 `-O2`、從組合語言反推 C。

---

## 自我測驗

1. `movl (%rdi,%rsi,4), %eax` 和 `leaq (%rdi,%rsi,4), %rax` 括號裡寫的一模一樣，兩個指令做的事差在哪？各對應 C 的什麼寫法？為什麼 gcc 會拿 `lea` 來算 `x * 3` 這種跟位址無關的東西？
2. `long widen_u(unsigned x) { return x; }` 只要一個 `movl %edi, %eax`，`long widen_s(int x) { return x; }` 卻要 `movslq %edi, %rax`。為什麼 `movl` 就足以完成前者？如果改寫 16 位元的 `%ax`，同樣的理由還成立嗎？
3. `lt_s` 和 `lt_u` 的 `cmpl %esi, %edi` 一模一樣，gdb 也看到兩者的旗標完全相同（`[ SF IF ]`），為什麼一個回傳 1、一個回傳 0？C 的「有號還是無號」在組合語言裡到底是在哪裡決定的？
4. `-O2` 的 `sum_to` 裡，`andl $1, %edi` 和 `je .L3` 中間夾了一個 `movl $1, %eax`。`je` 判斷的是哪個指令的結果？為什麼 gcc 可以這樣排？如果中間夾的是 `addl $1, %eax`，會怎樣？
5. 情境題：你在 kernel 的反組譯裡看到 `movq 24(%rdi,%rsi,8), %rax`，`%rdi` 是函式的第 1 個參數、`%rsi` 是第 2 個。推測它對應的 C 可能長怎樣：第 1 個參數最可能是指向什麼大小的元素的指標？存取的是第幾個元素（用 `i` 表示）？如果同一個位置看到的是 `leaq` 而不是 `movq`，C 又會是什麼？
