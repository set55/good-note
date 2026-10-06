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
| 第 1 欄 | `1b:` | 這個指令的**位址**（十六進位）。在 `.o` 裡就是「從 `.text` 區段開頭算起第幾個 byte」，**不是從函式開頭算**——`sum_to` 剛好是 `.text` 裡的第一個函式，才會從 0 開始。`.text` 是什麼、連結後這個數字怎麼變，見下面的〈第 1 欄的位址從哪裡算：`.text` 區段〉 |
| 第 2 欄 | `8b 45 fc` | **機器碼**：CPU 真正讀的 byte。指令長度不固定，1～15 byte 都有（x86 的特色） |
| 第 3 欄 | `mov -0x4(%rbp),%eax` | 同一個指令的**文字版**（反組譯的結果），就是組合語言 |

**機器碼本身也看得出第 2 課的東西**：

- `c7 45 f8 00 00 00 00`（`movl $0x0,-0x8(%rbp)`）後面四個 byte 是常數 0；`c7 45 fc 01 00 00 00` 是常數 1——**小端序**，低位的 `01` 在前面。
- `7e ee` 是 `jle 1b`。`ee` 不是目標位址，是**相對距離**，用有號的二補數解讀：`0xee` 當 `signed char` 是 −18。
  跳躍距離是從「下一個指令的位置」算起：下一個指令在 `0x2d`，`0x2d − 18 = 0x1b`，正好是跳過去的地方。
  同理 `eb 0a`（`jmp 25`）：下一個指令在 `0x1b`，`0x1b + 10 = 0x25`。

**讀組合語言時不需要背機器碼**，`objdump` 會幫你翻成文字。這裡只是要看到：組合語言跟機器碼是一對一的，不是另一種「比較高階的語言」。

### 第 1 欄的位址從哪裡算：`.text` 區段（2026-10-05 課後提問）

> 使用者問：「第 1 欄說的 `.text` 是什麼？」這一段也更正了原本表格的錯誤：原本寫「從函式開頭算」，實際上是從 `.text` 區段開頭算。

**`.text` 是 ELF 檔裡放「機器碼」的那個區段（section）。** [ELF是什麼第四節](./ELF是什麼-可執行與可連結格式的兩種視角.md#四名字的關鍵兩種視角)講過，
ELF 檔的內容被切成很多有名字的區段：程式碼放 `.text`、字串常數放 `.rodata`（[第 3 課第四節](./陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#四字串以-0-結尾的-char-陣列)）、
有初值的全域變數放 `.data`……。名字叫「text」是歷史慣例（早期 Unix 就把程式碼段叫 text），跟文字無關。

它跟 `.s` 檔裡的 `.text` 指示詞是同一個東西：gcc 在 `sum_to:` 前面寫 `.text`，就是叫 `as`「接下來的指令，機器碼放進 `.text` 區段」；
`main` 前面的字串則先切到 `.section .rodata`，寫完再切回 `.text`（見[第六節](#六逐行讀-sum_to-的--o0-版)）。

**證據一：`.o` 裡三個函式都在同一個 `.text` 裡，位址接著排下去。** 用的是本節開頭的 `sumto.c` 與 `s0.o`：

```sh
objdump -d s0.o | grep "section\|>:"
```

```
Disassembly of section .text:          ← objdump 告訴你：下面反組譯的是 .text 區段
0000000000000000 <sum_to>:
0000000000000032 <answer>:             ← answer 從 0x32 開始，不是 0
0000000000000046 <main>:
```

所以第 1 欄是「`.text` 開頭往後第幾個 byte」。`sum_to` 佔 `0x00`～`0x31`（最後的 `ret` 在 `0x31`），`answer` 緊接在 `0x32`。
`nm s0.o` 印的符號位址也是同一套數字：`0000000000000000 T sum_to`、`0000000000000032 T answer`、`0000000000000046 T main`（`T` 就是「在 text 區段的符號」）。

**證據二：區段表裡的 `.text`。**

```sh
readelf -S -W s0.o
```

```
  [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
  [ 1] .text             PROGBITS        0000000000000000 000040 000070 00  AX  0   0  1
  [ 2] .rela.text        RELA            0000000000000000 000270 000060 18   I 11   1  8
  ...
  [ 5] .rodata           PROGBITS        0000000000000000 0000b0 00000b 00   A  0   0  1
```

| 欄 | `.text` 的值 | 意思 |
|---|---|---|
| `Address` | `0` | 還沒連結，還不知道會被放到哪裡，所以 `.o` 裡每個區段都從 0 算 |
| `Off` | `0x40` | 在 `.o` **檔案**裡從第 64 個 byte 開始（前 64 byte 是 ELF 檔頭） |
| `Size` | `0x70` | 112 byte ＝ `sum_to`（0x32）＋ `answer`（0x14）＋ `main`（0x2a），三個函式的機器碼全部 |
| `Flg` | `AX` | A（alloc）：執行時要載入記憶體；X（execute）：可以執行。對照 `.rodata` 只有 `A`，不能執行也不能寫 |

`.rela.text` 是 `.text` 的重定位項目——[第 1 課第四節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#四組譯機器碼裡的洞)那些「洞」（`call printf` 後面的 `00 00 00 00`）的清單。

#### `readelf` 說從 `0x40` 開始、`objdump` 說從 `0` 開始，誰對？（2026-10-05 課後提問）

> 使用者問：「`readelf -S` 看 `.text` 從 `0x40` 開始、大小 `0x70`；`objdump -d` 卻是從 `0x00` 開始、最後一行是 `0x6f`。兩個命令對程式碼在哪裡的判斷怎麼不一樣？」

兩個都對，因為**它們講的是兩套不同的座標**，加上一個區間的讀法：

**1. `0x40` 是「在檔案裡的位置」，`0` 是「位址」。** `readelf -S` 一行裡同時有兩欄：

| 欄 | `.text` 的值 | 座標系 | 誰用它 |
|---|---|---|---|
| `Off`（offset） | `0x40` | **檔案位置**：從 `.o` 檔第 0 個 byte 數起 | 讀檔的程式（`readelf`、`ld`）要知道去檔案哪裡拿 |
| `Address` | `0` | **位址**：程式載入記憶體後在哪裡（`.o` 還沒決定，所以是 0） | 指令、跳躍、`call`、`objdump -d` 的第 1 欄 |

`objdump -d` 第 1 欄印的是 `Address` 那一套，所以從 0 開始；你看到的 `0x40` 是 `Off` 那一欄。
`objdump -h` 也把兩者並列，叫 `VMA`（Virtual Memory Address，虛擬記憶體位址）和 `File off`：

```sh
objdump -h s0.o
```

```
Idx Name          Size      VMA               LMA               File off  Algn
  0 .text         00000070  0000000000000000  0000000000000000  00000040  2**0
```

**同一個東西，三個工具三種叫法**（2026-10-05 第四個追問：「位址是指 `Address`、`VirtAddr`、`PhysAddr` 嗎？檔案位置是 `Off`、`Offset` 嗎？」）：

| 本筆記的說法 | `readelf -S`（區段標頭） | `readelf -l`（程式標頭） | `objdump -h` | 意思 |
|---|---|---|---|---|
| **位址** | `Address` | `VirtAddr` | `VMA` | 虛擬位址：程式在**自己的位址空間**裡看到的位址，指令、指標、gdb 用的都是它 |
| **檔案位置** | `Off` | `Offset` | `File off` | 從檔案第 0 個 byte 數起，第幾個 byte |
| （不用） | — | `PhysAddr` | `LMA` | 實體位址（physical address）：一般 Linux 程式用不到，見下面 |

- 區段標頭和程式標頭是兩張表（[ELF是什麼第四節](./ELF是什麼-可執行與可連結格式的兩種視角.md#四名字的關鍵兩種視角)的兩種視角），各自有位址和檔案位置兩欄，只是欄名不同；
  同一塊內容在兩張表裡的數字是一致的（例如 `.init_array` 的 `Address 0x3db8`／`Off 0x2db8`，就是它所在 `LOAD` 的 `VirtAddr 0x3db8`／`Offset 0x2db8`）。
- **「位址」一律指虛擬位址，不是實體位址。** 程式碼和 gdb 看到的位址都是虛擬的，實際落在 RAM 的哪裡由核心的分頁表決定，程式自己看不到也不需要知道。
- **`PhysAddr` 為什麼存在、又為什麼可以忽略**：ELF 也用在沒有虛擬記憶體的環境（微控制器、開機程式），那裡的程式要被放到某個固定的實體位址，這一欄才有意義。
  Linux 核心載入一般程式時只看 `VirtAddr`、不看這一欄，所以連結器直接填成跟 `VirtAddr` 一樣（本機每一行都一樣）。
  `objdump -h` 的 `LMA`（Load Memory Address，載入位址）是同一個概念的 binutils 叫法。
- 同理，`readelf -l` 還有 `FileSiz`／`MemSiz` 兩個大小：檔案裡佔多少、載入記憶體後佔多少。第 4 個 `LOAD` 是 `0x258`／`0x260`，差的 8 byte 就是 `.bss`（`readelf -S` 的 `Type` 是 `NOBITS`：檔案裡不存內容，載入時直接補 0）。

要 `objdump` 把兩套一起印出來，加 `-F`（`--file-offsets`）：

```sh
objdump -d -F s0.o | grep "<sum_to>\|<answer>\|<main>"
```

```
0000000000000000 <sum_to> (File Offset: 0x40):
0000000000000032 <answer> (File Offset: 0x72):
0000000000000046 <main> (File Offset: 0x86):
```

每個函式都是「檔案位置 ＝ 位址 ＋ `0x40`」。直接去檔案的 `0x40` 看那幾個 byte，就是 `sum_to` 的第一個指令：

```sh
xxd -s 0x40 -l 16 s0.o      # 從檔案第 0x40 個 byte 開始，印 16 個
xxd -s 0xa8 -l 8 s0.o       # 從 0xa8 開始印 8 個，看 .text 的結尾
```

```
00000040: f30f 1efa 5548 89e5 897d ecc7 45f8 0000  ....UH...}..E...
000000a8: 00b8 0000 0000 5dc3                      ......].
```

`f3 0f 1e fa` 是 `endbr64`、`55` 是 `push %rbp`、`48 89 e5` 是 `mov %rsp,%rbp`，跟 `objdump -d` 位址 `0`～`5` 的機器碼一樣；
結尾的 `5d c3`（`pop %rbp`、`ret`）在檔案的 `0xae`、`0xaf`，就是 `objdump` 位址 `0x6e`、`0x6f` 那兩行。

**2. 大小 `0x70`、最後一行 `0x6f`：區間是半開的。** `0x70` 是 **byte 數**（112 個），從位址 0 開始的 112 個 byte 佔 `0x00`～`0x6f`，
寫成 `[0x00, 0x70)`——跟[第 3 課第七節](./陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#七addresssanitizer在第一次越界的當下抓到它)〈`[96, 101)` 怎麼讀〉的 ASan 區間同一種讀法，右端是「第一個不屬於它的 byte」。
最後一個指令 `ret`（`c3`）只有 1 byte，剛好在 `0x6f`，所以 `.text` 用完 `0x70` 個 byte 一個不差。
在檔案裡就是 `[0x40, 0xb0)`；`readelf -S` 裡下一個區段 `.data` 的 `Off` 正好是 `0xb0`，緊接在後。

**連結後的執行檔**：`sumto0` 的 `.text` 是 `Address 0x1060`、`Off 0x1060`，兩欄剛好一樣，但這只是巧合，不是規則。

> 2026-10-05 第三個追問：「從可寫那一塊開始位址多了 `0x1000`，這是怎麼看出來的？」——原本這裡只節錄了 4 行，`.init_array` 根本不在節錄裡，也沒附看出「另一個分頁」的證據。以下補上完整輸出，分三步看。

**第一步：`readelf -S` 逐行比 `Address` 和 `Off`。** 本機 `readelf -S -W sumto0` 從 `.text` 到 `.bss` 的部分（中間沒有省略）：

```
  [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
  [16] .text             PROGBITS        0000000000001060 001060 000159 00  AX  0   0 16
  [17] .fini             PROGBITS        00000000000011bc 0011bc 00000d 00  AX  0   0  4
  [18] .rodata           PROGBITS        0000000000002000 002000 00000f 00   A  0   0  4
  [19] .eh_frame_hdr     PROGBITS        0000000000002010 002010 000044 00   A  0   0  4
  [20] .eh_frame         PROGBITS        0000000000002058 002058 0000ec 00   A  0   0  8
  [21] .init_array       INIT_ARRAY      0000000000003db8 002db8 000008 08  WA  0   0  8   ← 從這行開始 Address ≠ Off
  [22] .fini_array       FINI_ARRAY      0000000000003dc0 002dc0 000008 08  WA  0   0  8
  [23] .dynamic          DYNAMIC         0000000000003dc8 002dc8 0001f0 10  WA  7   0  8
  [24] .got              PROGBITS        0000000000003fb8 002fb8 000048 08  WA  0   0  8
  [25] .data             PROGBITS        0000000000004000 003000 000010 00  WA  0   0  8
  [26] .bss              NOBITS          0000000000004010 003010 000008 00  WA  0   0  1
```

`[20]` 以前每一行 `Address` 和 `Off` 都一樣；`[21] .init_array` 起，`Address` 都比 `Off` 多 `0x1000`（`0x3db8` vs `0x2db8`）。
同一行的 `Flg` 欄也是從 `[21]` 開始出現 `W`——這就是「從可寫的那一塊開始」的意思。到這裡只是**看到**了差 `0x1000`，原因要看第二步。

**第二步：`readelf -l` 看載入段（segment），權限是以「段」為單位、對映是以「分頁」為單位。**

```sh
readelf -l -W sumto0
```

```
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  LOAD           0x000000 0x0000000000000000 0x0000000000000000 0x000628 0x000628 R   0x1000
  LOAD           0x001000 0x0000000000001000 0x0000000000001000 0x0001c9 0x0001c9 R E 0x1000
  LOAD           0x002000 0x0000000000002000 0x0000000000002000 0x000144 0x000144 R   0x1000
  LOAD           0x002db8 0x0000000000003db8 0x0000000000003db8 0x000258 0x000260 RW  0x1000
  ...
 Section to Segment mapping:
  Segment Sections...
   02     .interp .note.gnu.property ... .rela.plt
   03     .init .plt .plt.got .plt.sec .text .fini
   04     .rodata .eh_frame_hdr .eh_frame
   05     .init_array .fini_array .dynamic .got .data .bss
```

（`Offset`／`VirtAddr` 就是第一步的 `Off`／`Address`，程式標頭表用這兩個名字；`Flg` 的 `R`／`W`／`E` 是讀／寫／執行；
程式標頭表的完整說明見 [ELF是什麼第四節](./ELF是什麼-可執行與可連結格式的兩種視角.md#四名字的關鍵兩種視角)。）

`.init_array` 到 `.bss` 被打包成第 4 個 `LOAD`，權限 `RW`；它前面那個 `LOAD`（`.rodata` 那一塊）權限只有 `R`。
**記憶體權限是以分頁（4096 ＝ `0x1000` byte）為單位設定的，一頁只能有一種權限**。現在用位址算算看「如果不加 `0x1000` 會怎樣」：

| | 唯讀段（`R`） | 可寫段（`RW`） |
|---|---|---|
| 檔案位置 | `0x2000` 起，`0x144` byte → 結束在 `0x2144` | `0x2db8` 起 |
| 如果位址 ＝ 檔案位置 | 佔 `0x2000`～`0x2fff` 這一頁 | 從 `0x2db8` 開始，**也落在 `0x2000`～`0x2fff` 這一頁** → 同一頁要同時是 `R` 和 `RW`，辦不到 |
| 實際位址 | `0x2000`（那一頁是 `R`） | `0x3db8`（換到 `0x3000`～`0x3fff` 這一頁，可以設成 `RW`） |

所以連結器把可寫段的**位址**往後推一頁。**檔案位置**卻不用動：檔案只是存放內容，不需要權限，緊接著放就好，不必在檔案裡空出 4096 byte。

為什麼剛好推 `0x1000`、而不是推到 `0x3000` 整數？因為載入時是「整頁整頁」從檔案對映進記憶體（`Align 0x1000`），
規則是**位址和檔案位置除以 `0x1000` 的餘數要一樣**（都是 `0xdb8`），這樣檔案的某一頁才能原封不動地對到記憶體的某一頁。
（至於為什麼從 `0xdb8` 這個不整齊的地方開始：`0x3db8 + 0x248 = 0x4000`，連結器刻意讓 `.init_array`～`.got` 這一塊剛好結束在分頁邊界，`.data` 從 `0x4000` 整頁開始。
`.init_array`～`.got` 在動態載入器填完之後會被改成唯讀，叫 RELRO（RELocation Read-Only），就是 `readelf -l` 裡 `GNU_RELRO` 那一行；第 5 課之後碰到 `.got` 再詳講。）

**第三步：執行起來用 gdb 確認。** 就是上面證據三用過的 `info proc mappings`。它要看**正在跑的行程**，所以要先讓程式停住；`sumto0` 沒有 `-g`，不能用行號下中斷點，要用函式名稱：

```sh
gdb -q ./sumto0
(gdb) break sum_to        # 或用 start（停在 main）、starti（停在第一個指令）
(gdb) run
(gdb) info proc mappings
```

（直接 `run` 會一路跑完，再下 `info proc mappings` 只會得到 `No current process: you must name one.`；用 `break 19` 這種行號會跑到 glibc 的檔案去，
見 [gdb完整命令參考第八節〈沒加 `-g` 就用行號下中斷點〉](./gdb完整命令參考.md#沒加--g-就用行號下中斷點中斷點跑到-glibc-裡永遠不會停2026-10-05-使用者實際遇到)。）
`sumto0` 自己的 5 行（後面還有 `[heap]`、libc、`ld.so`、`[stack]` 等，跟這裡無關）：

```
        Start Addr           End Addr       Size     Offset  Perms  objfile
    0x555555554000     0x555555555000     0x1000        0x0  r--p   .../sumto0
    0x555555555000     0x555555556000     0x1000     0x1000  r-xp   .../sumto0
    0x555555556000     0x555555557000     0x1000     0x2000  r--p   .../sumto0
    0x555555557000     0x555555558000     0x1000     0x2000  r--p   .../sumto0
    0x555555558000     0x555555559000     0x1000     0x3000  rw-p   .../sumto0
```

**這張表裡同時有兩套座標，要分開讀**（2026-10-05 第八個追問：「我找不到 `0x4000` 在哪個 offset」——`0x4000` 不在 `Offset` 欄，它是**位址**）：

- `Start Addr` 是**位址**，但加上了載入起點。減掉起點 `0x555555554000`，才是 `readelf -S` 的 `Address`／`readelf -l` 的 `VirtAddr`。
- `Offset` 是**檔案位置**：這一頁的內容從檔案第幾個 byte 搬來，對應 `readelf` 的 `Off`／`Offset`。

把每一行換算出來，再對照第一步的區段表：

| `Start Addr` | 減掉起點＝**位址** | `Offset`＝**檔案位置** | `Perms` | 這一頁裡有哪些區段（第一步的 `Address`） |
|---|---|---|---|---|
| `0x555555554000` | `0x0000` | `0x0` | `r--p` | ELF 檔頭、`.interp`～`.rela.plt`（第 1 個 `LOAD`） |
| `0x555555555000` | `0x1000` | `0x1000` | `r-xp` | `.init`、`.plt`、`.text`（`0x1060`）、`.fini` |
| `0x555555556000` | `0x2000` | `0x2000` | `r--p` | `.rodata`（`0x2000`）、`.eh_frame_hdr`、`.eh_frame` |
| `0x555555557000` | `0x3000` | **`0x2000`** | `r--p` | `.init_array`（`0x3db8`）、`.fini_array`、`.dynamic`、`.got`——RELRO 那一塊 |
| `0x555555558000` | **`0x4000`** | `0x3000` | `rw-p` | `.data`（`0x4000`）、`.bss`（`0x4010`） |

- 前三行「位址 ＝ 檔案位置」；後兩行位址比檔案位置多 `0x1000`，跟第一步 `readelf -S` 看到的完全一致。
- 「`.data` 所在的 `0x4000` 那一頁」指的是**位址** `0x4000`（`0x555555558000 − 0x555555554000`），它在檔案裡的位置是 `0x3000`——正好是第一步 `.data` 那一行的 `Off 0x3000`。

**檔案的 `0x2000` 那一頁被對映了兩次**：一次到位址 `0x2000`（給 `.rodata`），一次到位址 `0x3000`（給 `.init_array` 等，檔案位置 `0x2db8` 就在這一頁裡）。
這就是「檔案裡緊接著放、位址上跳一頁」實際的樣子。第二次對映那一頁現在顯示 `r--p` 而不是 `rw-p`，是因為 RELRO：載入器填完後已經把它改成唯讀；`.data` 所在的那一頁（**位址** `0x4000`、檔案位置 `0x3000`）才是 `rw-p`。

**一句話：位址 ≠ 檔案位置，是因為權限以分頁為單位，可寫段不能跟唯讀段共用一頁；連結器只在位址上跳一頁、檔案裡不留空，靠「餘數相同」讓分頁對得上。**

`xxd` 是把檔案印成十六進位的工具（跟 Vim 一起裝的）。這裡只用了 2 個選項，本機 `xxd -h`（版本 2023-10-25）**全部 19 個**如下：

| 選項 | 做什麼 |
|---|---|
| `-s [+][-]seek` | 從檔案第 `seek` 個 byte 開始（`+` 表示相對目前位置，`-` 表示從檔尾往前算） |
| `-l len` | 印 `len` 個 byte 就停 |
| `-c cols` | 每行幾個 byte（預設 16） |
| `-g bytes` | 每幾個 byte 一組（預設 2，`-e` 時 4） |
| `-e` | 以小端序分組顯示（每組倒過來印） |
| `-b` | 印成二進位，不是十六進位 |
| `-d` | 左邊的位置用十進位 |
| `-o off` | 顯示的位置全部加上 `off`（不改讀的位置） |
| `-u` | 十六進位用大寫字母 |
| `-a` | 連續全 0 的行用一個 `*` 代替 |
| `-E` | 右邊的字元欄用 EBCDIC 解讀（預設 ASCII） |
| `-ps` | 只印連續的十六進位，沒有位置和字元欄 |
| `-i` | 印成 C 的陣列宣告（可以 `#include` 的格式） |
| `-C` | 配合 `-i`：變數名稱用大寫 |
| `-n name` | 配合 `-i`：指定變數名稱 |
| `-r` | 反過來：把十六進位文字轉回二進位檔（可以拿來改檔）；`-r -s off` 是轉回時位置加上 `off` |
| `-R when` | 上色：`always`、`auto`（預設）、`never` |
| `-h` | 印說明 |
| `-v` | 印版本 |

**證據三：連結之後，`objdump -d` 的第 1 欄（第一節前面「三欄怎麼讀」那張表的第 1 欄，`1b:` 那一欄）變成執行檔裡的位址。** 同一個指令 `endbr64`，在 `s0.o` 裡印成 `0:`，在執行檔 `sumto0` 裡印成 `1149:`：

```sh
gcc -O0 sumto.c -o sumto0
objdump -d sumto0 | grep -A1 "<sum_to>:"
readelf -S -W sumto0 | grep " .text"
```

```
0000000000001149 <sum_to>:
    1149:	f3 0f 1e fa          	endbr64
  [16] .text             PROGBITS        0000000000001060 001060 000159 00  AX  0   0 16
```

- 連結器把所有 `.o` 的 `.text` 合併成一個，排在 `0x1060`。合併後最前面是啟動碼（`nm sumto0` 看到 `_start` 在 `0x1060`，[第 1 課](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#第-1-題o-15k連結後為什麼變成-16k新觀念elf-的區段與分頁對齊)提過的啟動碼符號），我們的 `sum_to` 被排到 `0x1149`。
- `.text` 從 112 byte 變成 `0x159` ＝ 345 byte，多出來的就是啟動碼那些函式。
- 這是 PIE（[ELF是什麼第三節](./ELF是什麼-可執行與可連結格式的兩種視角.md#三一種格式四種檔案)），`0x1149` 還只是「離載入起點多遠」；
  真正執行時要再加上載入位址，所以在 gdb 裡看到的是 `0x555555555149`（gdb 關掉 ASLR，起點固定在 `0x555555554000`）。本機實測：

  ```sh
  gdb -q -batch -ex 'break sum_to' -ex run -ex 'info symbol $pc' -ex 'info proc mappings' ./sumto0
  ```

  ```
  Breakpoint 1, 0x0000555555555151 in sum_to ()
  sum_to + 8 in section .text of .../sumto0          ← 停在 sum_to 開頭往後 8 byte，所以 sum_to 在 0x555555555149
        Start Addr           End Addr       Size     Offset  Perms  objfile
    0x555555554000     0x555555555000     0x1000        0x0  r--p   .../sumto0
    0x555555555000     0x555555556000     0x1000     0x1000  r-xp   .../sumto0   ← .text 被載入到這一頁：可讀、可執行、不可寫
    0x555555556000     0x555555557000     0x1000     0x2000  r--p   .../sumto0
  ```

  （`break sum_to` 停在 `+8`，是因為 gdb 會跳過函式開頭設定堆疊框的指令——沒有 `-g` 時它自己分析開頭的 `endbr64; push; mov`；
  `info symbol` 直接回答「這個位址在哪個符號、哪個區段」。）`.text` 的 `AX` 旗標到了執行時，就變成這一頁的 `r-xp` 權限。

**一句話：第 1 欄是「這個指令在 `.text` 區段裡的位址」。`.o` 還沒連結，`.text` 從 0 算；連結後，它是執行檔裡的位址。不管哪一種，都不是「從函式開頭算」——函式內的偏移要看 `<sum_to+0x1b>` 那種寫法。**
（`objdump` 的跳躍目標就寫成 `jle 1b <sum_to+0x1b>`：`1b` 是區段裡的位址，`sum_to+0x1b` 是「從 `sum_to` 開頭算第 0x1b 個 byte」。在 `sum_to` 裡兩者剛好一樣，在 `answer` 裡就不一樣了。）

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

（`unsigned` 單獨寫就是 `unsigned int`，見[第 2 課第一節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#一位元組與型別大小)〈`unsigned` 單獨寫就是 `unsigned int`：整數型別的所有寫法〉。）

兩個函式都是「把 32 位元的參數變成 64 位元的 `long` 回傳」，就是[第 2 課第六節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#六有號與無號之間的轉換位元不變換個讀法)〈**變寬（擴展，extension）**〉那一段講的擴展：

- `widen_u`（`unsigned` → `long`）要**零擴展**：高位補 0。剛好「寫 `%eax` 就把高 32 位元清 0」，所以一個普通的 `movl` 就做完了。
- `widen_s`（`int` → `long`）要**符號擴展**：高位補符號位元。普通的 `mov` 做不到，要用專門的 `movslq`。

擴展用的指令有一套命名規則：`movs` 或 `movz`，接「來源大小」，再接「目的大小」。大小用一個字母表示：`b`＝1、`w`＝2、`l`＝4、`q`＝8 byte（完整說明在[第三節](#三指令的文法助憶符大小後綴運算元)的大小後綴表；**這裡的 long 是 4 byte，不是 C 的 `long`**，原因見第三節〈[為什麼 `l` 叫 long 卻是 4 byte](#為什麼-l-叫-long-卻是-4-byte組合語言的大小名稱和-c-的型別是兩套)〉）：

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

### 為什麼 `l` 叫 long 卻是 4 byte：組合語言的大小名稱和 C 的型別是兩套

> 2026-10-06 課後提問：「第二節的 `movsbl` 說 long 是 4 byte，跟 C 的 `long` 是 8 byte 不一樣，這正常嗎？」

**正常。組合語言的 `l`（long）和 C 的 `long` 只是剛好同名，是兩套各自定下來、後來沒有一起改的名字。**

- **組合語言這套（byte／word／long／quad）是按 x86 的歷史定的**：x86 最早的 CPU（8086）是 16 位元的，所以「一個 word」＝ 2 byte。
  後來 CPU 變成 32 位元，32 位元的資料就叫「兩個 word」——Intel 的手冊叫 double word（dword），AT&T 語法叫 long，字尾寫 `l`；
  再變成 64 位元時叫 quad word（四個 word），字尾寫 `q`。為了讓舊的組合語言程式還能用，**名字一旦定了就不改**，
  所以到了 64 位元的今天，word 仍然是 2 byte、long 仍然是 4 byte。
- **C 的 `long` 是按平台的慣例定的**：C 標準只規定 `long` 至少 32 位元，確切大小由平台決定。
  本機 x86-64 Linux 採用 LP64（[第 2 課第一節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#一位元組與型別大小)），`long` 是 8 byte；
  64 位元 Windows 的 `long` 是 4 byte。所以連 C 的 `long` 自己，換平台都不一定一樣大。

因此讀組合語言時，**看到字尾的 `l` 就想「4 byte」，不要想成 C 的 `long`**。本機 C 型別對到字尾的關係是：
`char` → `b`、`short` → `w`、`int`／`unsigned` → `l`、**C 的 `long`、指標 → `q`**。

**實驗：C 的 `long` 編出來是 `q`，不是 `l`。** `suffix.c` 完整原始碼（只用 `-S`，不需要 `main`）：

```c
long  add1_long(long x)          { return x + 1; }
int   add1_int(int x)            { return x + 1; }
int   widen_char(signed char c)  { return c; }
long  widen_char_l(signed char c){ return c; }
```

```
$ gcc -S -O2 -fno-asynchronous-unwind-tables suffix.c -o suffix.s
（只列函式本體）
add1_long:
	endbr64
	leaq	1(%rdi), %rax        ← C 的 long：字尾 q（8 byte），64 位元暫存器 %rax
	ret
add1_int:
	endbr64
	leal	1(%rdi), %eax        ← C 的 int：字尾 l（4 byte），32 位元暫存器 %eax
	ret
widen_char:
	endbr64
	movsbl	%dil, %eax           ← signed char → int：byte 擴展成 "long"（4 byte）
	ret
widen_char_l:
	endbr64
	movsbq	%dil, %rax           ← signed char → C 的 long：byte 擴展成 quad（8 byte）
	ret
```

（`%dil` 是 `%rdi` 最低的 1 byte，見[第二節](#二暫存器cpu-裡的-16-個變數)的暫存器表。）
`widen_char` 回傳 C 的 `int`，用的是 `movsbl`；`widen_char_l` 回傳 C 的 `long`，用的卻是 `movsbq`。
指令名字裡的 `l` 跟 C 的 `long` 對不上，對上 C 的 `long` 的是 `q`。

**組譯器的資料指示詞也用同一套名字。** 第六節的 `.s` 檔尾出現過 `.long`。下面直接用組譯器 `as` 組譯一個只放資料的 `.s` 檔，看每個指示詞佔幾個 byte。`data.s` 完整內容：

```asm
	.data
a:	.byte 0x11
b:	.word 0x2222
c:	.long 0x33333333
d:	.quad 0x4444444444444444
```

（`.data` 是「接下來的東西放進 `.data` 區段」；`a:`、`b:` 這些是標籤，代表那個位置的位址。）

```
$ as data.s -o data.o
$ readelf -sW data.o | grep -E ' [abcd]$'
     1: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT    2 a
     2: 0000000000000001     0 NOTYPE  LOCAL  DEFAULT    2 b
     3: 0000000000000003     0 NOTYPE  LOCAL  DEFAULT    2 c
     4: 0000000000000007     0 NOTYPE  LOCAL  DEFAULT    2 d
$ objdump -s -j .data data.o
Contents of section .data:
 0000 11222233 33333344 44444444 444444    .""3333DDDDDDDD
```

`a` 在 0、`b` 在 1、`c` 在 3、`d` 在 7：`.byte` 佔 1、`.word` 佔 2、`.long` 佔 4、`.quad` 佔 8 byte，總共 15 byte。
`.long` 一樣是 4 byte。（`readelf -s` 列符號表、`objdump -s -j .data` 以十六進位傾印 `.data` 區段的內容，兩個工具的完整選項見
[readelf完整選項參考](./readelf完整選項參考.md) 與第八節〈`objdump`：本課用到的選項〉。傾印裡的 `2222` 和 `3333…` 看不出順序，換成 `0x12345678` 會看到小端序，見第 2 課第三節。）

**第三套名字：`gdb` 的 `x` 命令。** 第 5 課會用 `gdb` 的 `x` 看記憶體，它的大小字母**又是另一套**。本機 `gdb 15.1` 的 `help x`：

```
$ gdb -batch -ex 'help x'
...
Size letters are b(byte), h(halfword), w(word), g(giant, 8 bytes).
```

| byte 數 | AT&T 字尾／`as` 指示詞 | Intel 手冊 | `gdb` 的 `x` | 本機 C 型別 |
|---|---|---|---|---|
| 1 | `b`／`.byte` | byte | `b` | `char` |
| 2 | `w`／`.word`（word） | word | `h`（halfword） | `short` |
| 4 | `l`／`.long`（long） | doubleword | **`w`（word）** | `int` |
| 8 | `q`／`.quad`（quad） | quadword | `g`（giant） | `long`、指標 |

**同一個字 word，在 AT&T 語法是 2 byte，在 `gdb` 的 `x` 是 4 byte。** 三套名字各有各的歷史，沒有誰對誰錯；
記法是**一律換算成 byte 數再想**，不要拿名字去對 C 的型別。

拿錯一套名字時 `gdb` 的反應（`xv.c` 完整原始碼：`int v = 0x12345678; int main(void){return 0;}`，用 `gcc -g xv.c -o xv` 編譯）：

```
$ gdb -batch -ex 'x/1l &v' -ex 'x/1xw &v' -ex 'x/1xh &v' ./xv
0x4010 <v>:	Undefined output format "l".     ← l 不是 gdb 的大小字母（gdb 把它當成格式字母，也不認得）
0x4010 <v>:	0x12345678                      ← w：4 byte，整個 int
0x4010 <v>:	0x5678                          ← h：2 byte，只有低位那一半（小端序，低位在前）
```

（`x` 的完整文法 `x/<個數><格式字母><大小字母> <位址>` 見 [gdb完整命令參考](./gdb完整命令參考.md)，第 5 課會實際用。）

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
6. （課後提問延伸）`movsbl` 的 `l` 叫 long，為什麼是 4 byte，而本機 C 的 `long` 是 8 byte？本機 C 的 `long` 在組合語言裡對到哪個字尾？
   情境題：第 5 課你在 `gdb` 裡想看一個 `int` 變數的 4 個 byte，同學說「`int` 在組合語言是 `l`，所以用 `x/1l`」。這樣對嗎？應該用哪個大小字母，為什麼？
