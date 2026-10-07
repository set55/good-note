# 讀懂 x86-64 組合語言（一）—— 暫存器、`mov` 與 `lea`、`cmp` 與條件跳躍，逐行讀懂 `sum_to`

> 日期：2026-10-05
> 情境：系統化學習路線第 ① 站第 4 課。對應 CS:APP 第 3.1～3.6 節（程式的機器層級表示：資料格式、存取資訊、算術與邏輯運算、控制——條件碼與跳躍、迴圈）；
> 函式呼叫與堆疊框（3.7 節）留到第 5 課
> 適用範圍：實測本機 Ubuntu 24.04、x86-64、gcc 13.3.0、GNU as／objdump 2.42（binutils）、gdb 15.1；所有組合語言都是本機 gcc 實際產生的
> 對應檔案：每個實驗程式的完整原始碼都寫在它出現的那一節，可以直接複製編譯；同樣的程式也存成檔案放在
> [`small-project/s1-04-asm-basics/`](../small-project/s1-04-asm-basics/)（2026-10-07 起，每個範例旁邊都有檔案連結；會進 git，編譯產物不進），作業也在同一個資料夾
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

實驗用的程式就是 [gcc最佳化等級](./gcc最佳化等級-O0到O3差在哪.md) 的 `sum_to`（第 1 課作業資料夾裡的 [`sumto.c`](../small-project/s1-01-hello-compile/sumto.c)），左邊標的是行號，第六節會用到：

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

### 第 1 欄的位址從哪裡算：`.text` 區段

> 2026-10-05 課後提問：「第 1 欄說的 `.text` 是什麼？」這一段也更正了原本表格的錯誤：原本寫「從函式開頭算」，實際上是從 `.text` 區段開頭算。

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

#### `readelf` 說從 `0x40` 開始、`objdump` 說從 `0` 開始，誰對？

> 2026-10-05 課後提問：「`readelf -S` 看 `.text` 從 `0x40` 開始、大小 `0x70`；`objdump -d` 卻是從 `0x00` 開始、最後一行是 `0x6f`。兩個命令對程式碼在哪裡的判斷怎麼不一樣？」

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

所以 `movl %edi, %eax` 不只是寫 `%eax`，整個 `%rax` 都被設定好了（高 32 位元是 0）。用下面這個實驗看它的後果（檔案：[`ops.c`](../small-project/s1-04-asm-basics/ops.c)）：

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

**上表第一列 `(B)` 的 `(%rdi)`：`%rdi` 和 `(%rdi)` 差在哪？括號就是 C 的 `*`。**

> 2026-10-06 課後提問：「`(%rdi)` 的 `%rdi` 不是暫存器嗎？為什麼變成記憶體位址了？」

上表 `(B)` 那一列寫「位址 ＝ B」，例子是 `(%rdi)`。乍看會奇怪：`%rdi` 明明是暫存器，怎麼變成記憶體？
關鍵是：暫存器裡裝的永遠只是一個 64 位元的數字。同一個暫存器，寫法不同，指令拿這個數字做的事就不同：

| 寫法 | 指令拿到的是 | 對應 C |
|---|---|---|
| `%rdi` | 暫存器裡的**數字本身** | `x`（用變數的值） |
| `(%rdi)` | 把暫存器裡的數字**當成位址**，到記憶體的那個位址去讀（或寫）那裡的值 | `*p`（用指標指向的值） |

所以 `(%rdi)` 裡的 `%rdi` 仍然是暫存器；括號的意思是「它裝的數字是一個位址，請去記憶體的那裡」。這就是運算元種類表說的「**用的是那個位址裡的值**」。
上表其他寫法也一樣，只是位址不再只是 B，而是 D ＋ B ＋ I × S 算出來的數字，算完同樣去那個位址拿值。

實驗：兩個函式，一個回傳參數本身，一個回傳參數指向的值。`paren.c`（檔案：[`paren.c`](../small-project/s1-04-asm-basics/paren.c)）完整原始碼：

```c
#include <stdio.h>

__attribute__((noinline)) long copy_val(long x)  { return x; }    /* 回傳 x 本身 */
__attribute__((noinline)) long deref(long *p)    { return *p; }   /* 回傳 p 指向的值 */

int main(void)
{
    long v = 1234;
    printf("%ld %ld\n", copy_val(v), deref(&v));
    return 0;
}
```

（`__attribute__((noinline))` 的意思見下方〈讀懂實驗一的範例〉。）

```
$ gcc -O2 -g paren.c -o paren
$ objdump -d --no-show-raw-insn paren     （節錄）
00000000000011e0 <copy_val>:
    11e0:	endbr64
    11e4:	mov    %rdi,%rax       ← 沒有括號：把 %rdi 的數字複製到 %rax
    11e7:	ret
00000000000011f0 <deref>:
    11f0:	endbr64
    11f4:	mov    (%rdi),%rax     ← 有括號：把 %rdi 的數字當位址，去記憶體讀 8 byte 到 %rax
    11f7:	ret
```

兩行只差一對括號。用 `gdb` 停在 `deref` 的 `mov` 之前，看 `%rdi` 裡是什麼、那個位址裡又是什麼：

```
$ gdb -batch -ex 'break *deref+4' -ex run -ex 'p/x $rdi' -ex 'x/1gd $rdi' -ex stepi -ex 'p $rax' -ex kill ./paren
Breakpoint 1, deref (p=p@entry=0x7fffffffd420) at paren.c:4
$1 = 0x7fffffffd420           ← %rdi 裡的數字：main 的變數 v 的位址（&v，在堆疊上）
0x7fffffffd420:	1234          ← x/1gd：到這個位址看 1 個 8 byte（g）、用十進位（d）印 → v 的值
$2 = 1234                     ← 執行 mov (%rdi),%rax 之後，%rax 拿到的是記憶體裡的 1234，不是 %rdi 的數字
```

對照 `copy_val`：`%rdi` 裡就是 `1234` 這個數字（`v` 的值），`mov %rdi,%rax` 之後 `%rax` 也是 1234——它沒有碰記憶體。

如果 `%rdi` 裡的數字不是合法的位址，加了括號就會出事。在同一個中斷點把 `%rdi` 改成 1234 再執行：

```
$ gdb -batch -ex 'break *deref+4' -ex run -ex 'set $rdi = 1234' -ex stepi -ex kill ./paren
Program received signal SIGSEGV, Segmentation fault.
deref (p=p@entry=0x4d2) at paren.c:4
```

`(%rdi)` 要去讀位址 1234（`0x4d2`）的記憶體，那裡沒有對映任何分頁，就 segfault——跟
[第 3 課第五節](./陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#五c-不檢查邊界越界的三種下場)用壞掉的指標 `*p` 是同一件事。
（gdb 命令：`x/1gd 位址` 是「看記憶體：1 個、8 byte、十進位」，大小字母的意思見下方〈為什麼 `l` 叫 long 卻是 4 byte〉的 gdb 表；其他見 [gdb完整命令參考](./gdb完整命令參考.md)。）

**上表其他五列：每一種用一個實際的 C 函式對照。**

> 2026-10-06 課後提問：「`(B)` 看懂了，其他還是看不懂。」上表「本課實例」欄的例子大多出現在後面的章節和作業，在這裡讀不到，
> 所以下面另外寫一支程式，五種寫法各一個函式，並用 gdb 驗證「算出來的位址」就是 C 裡那個東西的位址。

讀每一種寫法時都照同一個步驟：**① 把括號裡的東西照「位址 ＝ D ＋ B ＋ I × S」算出一個數字（沒寫的當 0，S 沒寫當 1）；② 到記憶體的那個位址拿值。**
跟 `(B)` 一樣，差別只在第 ① 步怎麼算。`forms.c`（檔案：[`forms.c`](../small-project/s1-04-asm-basics/forms.c)）完整原始碼：

```c
#include <stdio.h>

long tbl[4] = {100, 200, 300, 400};   /* 全域陣列 */
long g = 42;                          /* 全域變數 */

__attribute__((noinline)) long second(long *p)      { return p[1]; }      /* D(B)     */
__attribute__((noinline)) int  get(int *a, long i)  { return a[i]; }      /* (B,I,S)  */
__attribute__((noinline)) int  get3(int *a, long i) { return a[i + 3]; }  /* D(B,I,S) */
__attribute__((noinline)) long get_tbl(long i)      { return tbl[i]; }    /* D(,I,S)  */
__attribute__((noinline)) long get_g(void)          { return g; }         /* D(%rip)  */

int main(void)
{
    long arr[2]  = {7, 8};
    int  nums[6] = {10, 11, 12, 13, 14, 15};
    printf("%ld %d %d %ld %ld\n", second(arr), get(nums, 1), get3(nums, 1), get_tbl(2), get_g());
    return 0;
}
```

```
$ gcc -O2 -g forms.c -o forms_pie && ./forms_pie
8 11 14 300 42
$ objdump -d --no-show-raw-insn forms_pie      （節錄五個函式）
0000000000001220 <second>:
    1224:	mov    0x8(%rdi),%rax
0000000000001230 <get>:
    1234:	mov    (%rdi,%rsi,4),%eax
0000000000001240 <get3>:
    1244:	mov    0xc(%rdi,%rsi,4),%eax
0000000000001250 <get_tbl>:
    1254:	lea    0x2de5(%rip),%rax        # 4040 <tbl>
    125b:	mov    (%rax,%rdi,8),%rax
0000000000001260 <get_g>:
    1264:	mov    0x2db5(%rip),%rax        # 4020 <g>
    126b:	ret
```

（每個函式第一行都是 `endbr64`，最後是 `ret`，這裡省略。`objdump` 把 `D` 印成十六進位：`0x8` 是 8、`0xc` 是 12；`gcc -S` 的 `.s` 會印十進位。）

以下 gdb 的做法都一樣：`break *函式名+4` 停在那條 `mov` 上（`*` 的意思見下方〈上面 `break *widen_char+4` 的 `*`：為什麼要加？〉），
用 `p/x` 照公式把位址算出來，再用 C 的寫法 `&…` 問 gdb 那個東西的位址，兩個應該相同；最後用 `x` 看那個位址裡的值
（`x/1gd`：1 個 8 byte、十進位；`x/1wd`：1 個 4 byte、十進位；大小字母見下方〈為什麼 `l` 叫 long 卻是 4 byte〉的 gdb 表）。

**`D(B)`：位址 ＝ B ＋ D。`second` 的 `mov 0x8(%rdi),%rax`。** `p` 在 `%rdi`，`p[1]` 在 `p` 往後一個 `long`，也就是往後 8 byte，所以 `D` ＝ 8、`B` ＝ `%rdi`：

```
$ gdb -batch -ex 'break *second+4' -ex run -ex 'p/x $rdi' -ex 'p/x $rdi + 8' -ex 'p/x &p[1]' -ex 'x/1gd $rdi + 8' -ex kill ./forms_pie
$1 = 0x7fffffffd400        ← B：%rdi 裡是 p（main 的 arr 的位址）
$2 = 0x7fffffffd408        ← 照公式算：B ＋ D
$3 = 0x7fffffffd408        ← C 的 &p[1]：一樣
0x7fffffffd408:	8          ← 那裡的值是 arr[1]，也就是 8
```

`D` 是寫死在指令裡的常數，常用在「起點加一個固定距離」：陣列的固定第幾格、結構的某個成員（第 ② 站之後會看到）。
`D` 也可以是負數，第六節的 `-4(%rbp)` 就是「`%rbp` 往前 4 byte」（`%rbp` 是什麼，第 5 課講）。

**`(B,I,S)`：位址 ＝ B ＋ I × S。`get` 的 `mov (%rdi,%rsi,4),%eax`。** `a` 在 `%rdi`（`B`）、`i` 在 `%rsi`（`I`），每個 `int` 4 byte，所以 `S` ＝ 4：

```
$ gdb -batch -ex 'break *get+4' -ex run -ex 'p/x $rdi' -ex 'p $rsi' -ex 'p/x $rdi + $rsi*4' -ex 'p/x &a[i]' -ex 'x/1wd $rdi + $rsi*4' -ex kill ./forms_pie
$1 = 0x7fffffffd410        ← B：a（main 的 nums）
$2 = 1                     ← I：i
$3 = 0x7fffffffd414        ← B ＋ I × S ＝ a ＋ 1 × 4
$4 = 0x7fffffffd414        ← C 的 &a[i]：一樣
0x7fffffffd414:	11         ← nums[1]
```

`D` 寫死在指令裡，`I` 是暫存器，所以 `I` 可以在執行時改變——這就是「第 `i` 個」的 `a[i]`。`S` 由元素的大小決定，這也是 `S` 只能是 1、2、4、8 的理由（見表後那段）。

**`D(B,I,S)`：位址 ＝ B ＋ I × S ＋ D。`get3` 的 `mov 0xc(%rdi,%rsi,4),%eax`。** `a[i + 3]` 是 `a ＋ (i ＋ 3) × 4 ＝ a ＋ i × 4 ＋ 12`，
前兩項是 `(B,I,S)`，固定的 `＋ 12` 就放進 `D`。注意 `D` 是 **12（byte）**，不是原始碼裡的 3（個元素）：

```
$ gdb -batch -ex 'break *get3+4' -ex run -ex 'p/x $rdi + $rsi*4 + 12' -ex 'p/x &a[i+3]' -ex 'x/1wd $rdi + $rsi*4 + 12' -ex kill ./forms_pie
$1 = 0x7fffffffd420        ← a ＋ 1 × 4 ＋ 12（a 一樣是 0x7fffffffd410）
$2 = 0x7fffffffd420        ← C 的 &a[i+3]：一樣
0x7fffffffd420:	14         ← nums[4]
```

**`D(%rip)`：位址 ＝ 下一條指令的位址 ＋ D。`get_g` 的 `mov 0x2db5(%rip),%rax`。** 全域變數 `g` 不在任何參數暫存器裡，要用 `%rip` 當基準來找。
`%rip` 是「下一條要執行的指令在哪裡」（[第二節](#二暫存器cpu-裡的-16-個變數)的 `%rip`），`mov` 從 `0x1264` 開始、長 7 byte，下一條 `ret` 在 `0x126b`：
`0x126b ＋ 0x2db5 ＝ 0x4020`，正是 `objdump` 在右邊註解的 `# 4020 <g>`。

```
$ gdb -batch -ex 'break *get_g+4' -ex run -ex 'p/x $pc' -ex 'p/x $pc + 7 + 0x2db5' -ex 'p/x &g' -ex 'x/1gd &g' -ex kill ./forms_pie
$1 = 0x555555555264        ← 停在 mov 上（gdb 的 $pc 是「這一條」的位址，所以下一條要再 ＋ 7）
$2 = 0x555555558020        ← 下一條指令的位址 ＋ D
$3 = 0x555555558020        ← C 的 &g：一樣
0x555555558020 <g>:	42
```

為什麼不直接把 `g` 的位址寫進 `D`？因為 PIE 程式執行時會被整支搬到基底位址上（見下方〈上面中斷點 4 的 `break *0x11f4`：為什麼 `run` 之後失敗？〉），`g` 的位址要執行時才知道；
但「`g` 跟這條指令相距多遠」（`0x2db5`）不管搬到哪裡都不變。用 `%rip` 加距離，程式放在哪裡都找得到 `g`——這就是表上說「PIE 用」的意思。

**`D(,I,S)`：位址 ＝ I × S ＋ D，沒有 `B`。** 上面 `get_tbl`（`tbl[i]`）在 PIE 下被拆成兩條：先 `lea 0x2de5(%rip),%rax` 用 `%rip` 算出 `tbl` 的位址放進 `%rax`
（`lea` 只算位址、不讀記憶體，[第四節](#四mov-與-lea搬資料-vs-算位址)），再用 `(%rax,%rdi,8)`，是 `(B,I,S)`。
要看到 `D(,I,S)`，得讓 `tbl` 的位址在編譯時就固定，也就是編成非 PIE：

```
$ gcc -O2 -g -fno-pie -no-pie forms.c -o forms_np
$ objdump -d --no-show-raw-insn forms_np      （節錄）
0000000000401240 <get_tbl>:
  401244:	mov    0x404060(,%rdi,8),%rax
$ gdb -batch -ex 'break *get_tbl+4' -ex run -ex 'p $rdi' -ex 'p/x 0x404060 + $rdi*8' -ex 'p/x &tbl[2]' -ex 'x/1gd 0x404060 + $rdi*8' -ex kill ./forms_np
$1 = 2                     ← I：i
$2 = 0x404070              ← D ＋ I × S ＝ 0x404060 ＋ 2 × 8
$3 = 0x404070              ← C 的 &tbl[2]：一樣
0x404070 <tbl+16>:	300
```

（`-fno-pie` 叫編譯器產生非 PIE 的程式碼、`-no-pie` 叫連結器產生非 PIE 的執行檔，兩個都要加；只加 `-no-pie`，本機 gcc 仍然產生上面 `lea …(%rip)` 那種寫法。）
這裡的 `D` ＝ `0x404060` 就是 `tbl` 固定的位址；括號裡逗號前面空著，代表沒有 `B`。所以 `D(,I,S)` 是「`D` 本身就是陣列的起點」：固定位址的陣列的第 `i` 格。

五種寫法整理：

| 寫法 | 本例 | ① 算位址 | 對應 C |
|---|---|---|---|
| `(B)` | `(%rdi)` | `p` | `*p` |
| `D(B)` | `0x8(%rdi)` | `p ＋ 8` | `p[1]`（`long`） |
| `(B,I,S)` | `(%rdi,%rsi,4)` | `a ＋ i × 4` | `a[i]`（`int`） |
| `D(B,I,S)` | `0xc(%rdi,%rsi,4)` | `a ＋ i × 4 ＋ 12` | `a[i + 3]`（`int`） |
| `D(,I,S)` | `0x404060(,%rdi,8)` | `0x404060 ＋ i × 8` | `tbl[i]`（固定位址的 `long` 陣列） |
| `D(%rip)` | `0x2db5(%rip)` | 下一條指令 ＋ `0x2db5` | `g`（全域變數） |

**S 只能是 1、2、4、8，正好是 `char`、`short`、`int`、`long`／指標的大小**——這就是硬體直接支援
[第 3 課第二節](./陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#二指標運算p--1-走一個元素不是一個-byte)的指標運算：`a + i` 走 `i × sizeof(*a)` 個 byte。

一條限制：**一個指令最多只能有一個記憶體運算元**。要把記憶體搬到記憶體，得先搬進暫存器再搬出去——這是第六節 `-O0` 裡一直出現 `movl -4(%rbp), %eax` 的原因之一。

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

**實驗一：C 的 `long` 編出來是 `q`，不是 `l`。** `suffix.c`（檔案：[`suffix.c`](../small-project/s1-04-asm-basics/suffix.c)）完整原始碼（只用 `-S`，不需要 `main`）：

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

**實驗二：組譯器的 `.byte`／`.word`／`.long`／`.quad` 也是這套大小。**

> 2026-10-06 使用者反映這段看不懂、不知道 `data.s` 從哪裡來，改寫成逐步說明。這個實驗只是要再證明一次「`.long` 是 4 byte」，
> 看不懂不影響主線；以 `.` 開頭的指示詞，[第六節](#六逐行讀-sum_to-的--o0-版)〈先看完整的 `.s` 檔：哪些是指令、哪些不是〉才正式介紹。

**`data.s` 是什麼、從哪裡來**：它**不是 gcc 產生的**，是為了這個實驗**自己手寫**的組合語言原始檔。平常的 `.s` 是 `gcc -S` 從 C 翻出來的
（[第 1 課第三節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#三編譯c-變成組合語言以及一個意外)）；這裡跳過 C，直接用編輯器新增一個 `data.s`，內容是下面這 5 行，
再把它交給組譯器 `as`（第 1 課四個階段的第 3 步：組合語言 → `.o`）。寫好的檔案：[`data.s`](../small-project/s1-04-asm-basics/data.s)。

```asm
	.data
a:	.byte 0x11
b:	.word 0x2222
c:	.long 0x33333333
d:	.quad 0x4444444444444444
```

逐行：

| 行 | 是什麼 | 意思 |
|---|---|---|
| `.data` | 指示詞（directive）：以 `.` 開頭，不是 CPU 指令，是給組譯器的命令 | 「接下來的東西放進 `.data` 區段」（`.data` 是放有初始值的資料的區段，區段見 [ELF 是什麼](./ELF是什麼-可執行與可連結格式的兩種視角.md)） |
| `a:` | 標籤（label）：名字後面加冒號 | 替「目前這個位置」取名字 `a`，作用像 C 的變數名；組譯器會把它記進符號表 |
| `.byte 0x11` | 指示詞 | 在這裡放 **1** byte，值是 `0x11` |
| `.word 0x2222` | 指示詞 | 放 **2** byte，值是 `0x2222` |
| `.long 0x33333333` | 指示詞 | 放 **4** byte |
| `.quad 0x4444444444444444` | 指示詞 | 放 **8** byte |

這 5 行大約等於 C 的 `char a = 0x11; short b = 0x2222; int c = 0x33333333; long d = 0x4444444444444444;`（四個全域變數），
差別是手寫的版本沒有對齊，四個資料一個緊接一個。實驗的想法：**如果 `.long` 是 4 byte，`d` 就應該從第 1 ＋ 2 ＋ 4 ＝ 7 個 byte 開始。**

**步驟一：組譯。**

```
$ as data.s -o data.o
（沒有輸出就是成功）
```

**步驟二：看每個標籤在第幾個 byte——符號表。** `readelf -s` 列出 `.o` 的符號表（symbol table），標籤就記在這裡；`-W` 是不截斷長行：

```
$ readelf -sW data.o
Symbol table '.symtab' contains 5 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT    2 a
     2: 0000000000000001     0 NOTYPE  LOCAL  DEFAULT    2 b
     3: 0000000000000003     0 NOTYPE  LOCAL  DEFAULT    2 c
     4: 0000000000000007     0 NOTYPE  LOCAL  DEFAULT    2 d
```

這裡只要看三欄：

| 欄 | 意思 | 本例 |
|---|---|---|
| `Value` | 標籤的位置：從所在區段的開頭算起第幾個 byte（`.o` 還沒連結，所以從 0 算） | `a`＝0、`b`＝1、`c`＝3、`d`＝7 |
| `Ndx` | 標籤在第幾號區段 | 都是 2；`readelf -S data.o` 列出區段表，`[ 2]` 就是 `.data`（見下） |
| `Name` | 名字 | `a`～`d` |

其他欄這個實驗用不到：第 0 列是 ELF 規定的空白項；`Size` 是 0，因為手寫標籤沒有宣告大小；`Type`、`Bind`、`Vis` 是符號的種類與可見範圍
（`LOCAL` 代表只在這個檔案裡看得到）。各欄完整說明見 [readelf完整選項參考](./readelf完整選項參考.md)〈2.3 符號〉。

相鄰兩個標籤相減，就是前一個資料佔幾個 byte：`b − a` ＝ 1（`.byte`）、`c − b` ＝ 2（`.word`）、`d − c` ＝ 4（**`.long`**）。
`.quad` 後面沒有標籤，改看區段大小：

```
$ readelf -SW data.o      （節錄）
  [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
  [ 2] .data             PROGBITS        0000000000000000 000040 00000f 00  WA  0   0  1
```

`.data` 的 `Size` 是 `0xf` ＝ 15 byte，`15 − 7` ＝ 8（`.quad`）。

**步驟三：看區段裡實際放了什麼 byte。** `objdump -s` 以十六進位傾印區段內容，`-j .data` 只看 `.data` 這一個區段：

```
$ objdump -s -j .data data.o

data.o:     file format elf64-x86-64

Contents of section .data:
 0000 11222233 33333344 44444444 444444    .""3333DDDDDDDD
```

這一行的讀法：最左邊 `0000` 是這一行從區段的第幾個 byte 開始；中間是內容，每 4 byte 一組（8 個十六進位字）；
最右邊是同樣的 byte 當成 ASCII 字元顯示（`0x22` 是 `"`、`0x33` 是 `3`、`0x44` 是 `D`，不能顯示的印成 `.`）。
把中間那段重新切開，就是寫進去的四個資料：

```
11 | 22 22 | 33 33 33 33 | 44 44 44 44 44 44 44 44
a    b       c             d
1    2       4             8   byte
```

**結論：`.long` 佔 4 byte，跟字尾 `l` 一致**——組譯器的資料指示詞和指令字尾是同一套名字。

**順便看小端序。** 上面每個資料的 byte 都一樣，看不出順序。把數值換成每個 byte 都不同的 `data2.s`（檔案：[`data2.s`](../small-project/s1-04-asm-basics/data2.s)）：

```asm
	.data
a:	.byte 0x11
b:	.word 0x1234
c:	.long 0x12345678
```

```
$ as data2.s -o data2.o && objdump -s -j .data data2.o
Contents of section .data:
 0000 11341278 563412                      .4.xV4.
```

切開是 `11 | 34 12 | 78 56 34 12`：`0x1234` 存成 `34 12`、`0x12345678` 存成 `78 56 34 12`，**低位的 byte 放在前面**，
就是[第 2 課第三節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#三位元組順序endianness)的小端序。
（`objdump` 的全部選項見[第八節](#八本課用到的工具選項與完整清單在哪)〈`objdump`：本課用到的選項〉。）

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

拿錯一套名字時 `gdb` 的反應（`xv.c`（檔案：[`xv.c`](../small-project/s1-04-asm-basics/xv.c)）完整原始碼：`int v = 0x12345678; int main(void){return 0;}`，用 `gcc -g xv.c -o xv` 編譯）：

```
$ gdb -batch -ex 'x/1l &v' -ex 'x/1xw &v' -ex 'x/1xh &v' ./xv
0x4010 <v>:	Undefined output format "l".     ← l 不是 gdb 的大小字母（gdb 把它當成格式字母，也不認得）
0x4010 <v>:	0x12345678                      ← w：4 byte，整個 int
0x4010 <v>:	0x5678                          ← h：2 byte，只有低位那一半（小端序，低位在前）
```

（`x` 的完整文法 `x/<個數><格式字母><大小字母> <位址>` 見 [gdb完整命令參考](./gdb完整命令參考.md)，第 5 課會實際用。）

### 讀懂實驗一的範例：為什麼寫 `1(%rdi)`、為什麼用 `%dil`

> 2026-10-06 課後提問：「為什麼 `rdi` 要寫成 `1(%rdi)`？為什麼要用 `dil`，我們不是在用 64 位元的系統嗎？」

**`leaq 1(%rdi), %rax` 是「`%rax = %rdi + 1`」，不是讀記憶體。** `1(%rdi)` 是記憶體運算元的寫法 `D(B,I,S)` 只寫了 `D` 和 `B`
（`D` ＝ 1、`B` ＝ `%rdi`，省略 `I`、`S`，見上面的 `D(B,I,S)` 表），算出來的數字是 `1 + %rdi`。
但指令是 `lea`，只算數字、不去那個位址讀東西（[第四節](#四mov-與-lea搬資料-vs-算位址)〈`lea` 被拿來做算術〉）。
`%rdi` 是第 1 個參數 `x`，`%rax` 是回傳值，所以這一行就是 `return x + 1;`。

為什麼不用 `add`：`add` 只能把結果寫回其中一個運算元。`addq $1, %rdi` 的結果在 `%rdi`，還得再 `movq %rdi, %rax` 搬到回傳值的位置，
要兩個指令；`lea` 可以讀 `%rdi`、把結果寫到**另一個**暫存器 `%rax`，一個指令做完。

**「64 位元系統」指的是暫存器和位址是 64 位元，不是每個資料都變成 64 位元。** C 的 `signed char` 仍然是 1 byte、`int` 仍然是 4 byte。
參數 `c` 是 `signed char`，放在第 1 個參數暫存器 `%rdi` 裡，但**只有最低的 1 byte 是 `c`**，這 1 byte 的名字就是 `%dil`（[第二節](#二暫存器cpu-裡的-16-個變數)的暫存器表）。
`movsbl %dil, %eax` 的意思是「讀 `%rdi` 最低的 1 byte，符號擴展成 4 byte，放進 `%eax`」。

**為什麼不能直接讀整個 `%rdi`：另外 7 個 byte 不是 `c`，裡面是什麼不保證。** 呼叫的人只負責把 `c` 放進最低的 1 byte，高位是多少沒有約定
（這是 System V ABI 的規定，第 5 課講呼叫慣例時再細看；這裡只實測 gcc 的行為）。下面用 `gdb` 實際看。`dil.c`（檔案：[`dil.c`](../small-project/s1-04-asm-basics/dil.c)）完整原始碼：

```c
#include <stdio.h>

__attribute__((noinline)) int widen_char(signed char c) { return c; }

int main(void)
{
    printf("%d\n", widen_char(-1));
    return 0;
}
```

（`__attribute__((noinline))` 是 gcc 的擴充語法，意思是「不要把這個函式內嵌」。不加的話，`-O2` 會把 `widen_char(-1)` 直接算成 `-1`
（[gcc最佳化等級第二節](./gcc最佳化等級-O0到O3差在哪.md#二實驗同一段程式不同等級編出什麼)〈「內嵌」是什麼〉），就看不到呼叫了。）

```
$ gcc -O2 -g dil.c -o dil
$ objdump -d --no-show-raw-insn dil     （節錄 main 與 widen_char）
0000000000001060 <main>:
    1060:	endbr64
    1064:	sub    $0x8,%rsp
    1068:	mov    $0xffffffff,%edi              ← 呼叫的人：把 -1 放進 %edi
    106d:	lea    0xf90(%rip),%rsi
    1074:	call   1180 <widen_char>
    ...
0000000000001180 <widen_char>:
    1180:	endbr64
    1184:	movsbl %dil,%eax                     ← 被呼叫的人：只讀最低 1 byte
    1188:	ret
```

`main` 寫的是 `%edi`（4 byte），所以 `%rdi` 的高 4 byte 被清成 0（[第二節](#二暫存器cpu-裡的-16-個變數)〈寫低位的名字時，高位會怎樣？〉），
整個 `%rdi` 是 `0x00000000ffffffff`。如果 `widen_char` 把整個 `%rdi` 當成數字，得到的是 4294967295，不是 −1。
（`main` 裡其他幾行是準備 `printf` 的參數，`printf` 在 `-O2` 下被換成檢查版的 `__printf_chk`，跟本題無關。）

接著在 `gdb` 裡故意把 `%rdi` 的高 7 byte 改成亂七八糟的值，只留最低 1 byte 是 `0xff`，看 `movsbl` 的結果會不會變：

```
$ gdb -batch -ex 'break *widen_char+4' -ex run -ex 'p/x $rdi' \
      -ex 'set $rdi = 0x11223344556677ff' -ex 'p/x $rdi' -ex 'p/x $dil' \
      -ex stepi -ex 'p $eax' -ex 'p/x $rax' -ex continue ./dil
Breakpoint 1 at 0x1184: file dil.c, line 3.
Breakpoint 1, widen_char (c=c@entry=-1 '\377') at dil.c:3
$1 = 0xffffffff                ← 呼叫時 %rdi 的真正內容：高 4 byte 是 0
$2 = 0x11223344556677ff        ← 故意把高 7 byte 改成垃圾
$3 = 0xff                      ← %dil 只看最低 1 byte，仍是 0xff
$4 = -1                        ← stepi 執行完 movsbl 之後，%eax = -1
$5 = 0xffffffff                ← 寫 %eax 時，%rax 的高 4 byte 被清成 0
-1                             ← continue 讓程式跑完，printf 印出 -1
```

用到的 `gdb` 命令：`break *widen_char+4`（在 `widen_char` 開頭往後 4 byte、也就是 `endbr64` 之後的那條指令下中斷點，`*` 代表後面是位址）、
`p/x $rdi`（用十六進位印暫存器）、`set $rdi = …`（改暫存器的值）、`stepi`（只執行一條機器指令，第 5 課詳講）、`continue`。
完整說明見 [gdb完整命令參考](./gdb完整命令參考.md)。

**上面 `break *widen_char+4` 的 `*`：為什麼要加？**

> 2026-10-06 課後提問：「`break *deref+4` 為什麼前面要加 `*`？」

`break` 後面的「位置」有兩種主要寫法（[gdb完整命令參考第三節](./gdb完整命令參考.md#三gdb-內部命令的共通文法)〈位置（LOCATION）〉的表）：

| 寫法 | 例子 | gdb 怎麼理解 |
|---|---|---|
| 行號規格（linespec） | `break deref`、`break 4` | 後面是**函式名或行號**，gdb 用除錯資訊找出那一行「該停的地方」 |
| 位址 | `break *deref`、`break *deref+4`、`break *0x11f4` | `*` 開頭：後面是一個**算出位址的式子**，就停在那個位址的那條機器指令 |

這裡的 `*` 是 `break` 的語法記號，意思是「後面是位址」，**不是 C 的取值 `*`**。`*` 後面的 `deref+4` 會當成式子計算：
`deref` 代表函式的開頭位址，`+4` 是往後 4 byte。本機 `paren.c`（上面〈上表第一列 `(B)`〉那段的程式）實測：

```
$ gdb -batch -ex 'break deref' -ex 'break *deref' -ex 'break *deref+4' -ex 'break deref+4' -ex 'break *0x11f4' ./paren
Breakpoint 1 at 0x11f0: file paren.c, line 4.          ← 函式名
Breakpoint 2 at 0x11f0: file paren.c, line 4.          ← * 函式開頭的位址
Breakpoint 3 at 0x11f4: file paren.c, line 4.          ← * 開頭往後 4 byte：endbr64 之後的 mov
Function "deref+4" not defined.                        ← 沒有 *：gdb 把 deref+4 整串當成函式名，找不到
Make breakpoint pending on future shared library load? (y or [n]) [answered N; input not from terminal]
Breakpoint 4 at 0x11f4: file paren.c, line 4.          ← * 直接寫數字：現在看起來跟 3 一樣，但 run 之後會失敗（見下方〈上面中斷點 4 的 `break *0x11f4`：為什麼 `run` 之後失敗？〉）
```

（`0x11f4` 是 `objdump -d` 印出的 `deref` 裡 `mov (%rdi),%rax` 那一行的位址。程式還沒執行時，gdb 顯示的是檔案裡的位址，所以 3 和 4 看起來一樣；**程式跑起來以後就不一樣了**，見下方。）

**為什麼實驗要用 `*…+4`，不用 `break deref`**：實驗要停在「`mov` 還沒執行」的那一刻，改 `%rdi` 之後再 `stepi` 執行 `mov`。
`-O2` 的 `break deref` 停在 `0x11f0`，也就是 `endbr64` 那一條；在那裡 `stepi` 執行的是 `endbr64`，不是 `mov`。
用 `*deref+4` 才能精確停在 `mov` 上。

**`break 函式名` 和 `break *函式名` 不一定停在同一個地方。** 上面 `-O2` 兩者剛好都是 `0x11f0`；同一個 `paren.c` 改用 `-O0` 編：

```
$ gcc -O0 -g paren.c -o paren0
$ objdump -d --no-show-raw-insn paren0      （節錄 deref）
000000000000117b <deref>:
    117b:	endbr64
    117f:	push   %rbp
    1180:	mov    %rsp,%rbp
    1183:	mov    %rdi,-0x8(%rbp)
    1187:	mov    -0x8(%rbp),%rax
    118b:	mov    (%rax),%rax
    118e:	pop    %rbp
    118f:	ret
$ gdb -batch -ex 'break deref' -ex 'break *deref' ./paren0
Breakpoint 1 at 0x1187: file paren.c, line 4.     ← 函式名：跳過開頭的準備工作，停在函式本體的第一條
Breakpoint 2 at 0x117b: file paren.c, line 4.     ← *：一定是函式的第一條指令 endbr64
```

`break deref` 會跳過函式開頭「把參數存進堆疊」這些準備指令（`push %rbp` 到 `mov %rdi,-0x8(%rbp)`，第 5 課〈堆疊框〉的內容），
停在原始碼那一行真正開始做事的地方，這對「一行一行看 C」最方便；`break *deref` 則不跳過任何東西。
**看 C 用函式名或行號；看組合語言、要精確控制停在哪一條指令時，用 `*位址`。**

**上面中斷點 4 的 `break *0x11f4`：為什麼 `run` 之後失敗？**

> 2026-10-06 使用者實際操作：用 `objdump -d paren` 查到 `deref+4` 是 `0x11f4`，`break *0x11f4` 之後 `run`，得到 `Cannot insert breakpoint 1. Cannot access memory at address 0x11f4`；
> 同樣的程式 `break *deref+4` 卻正常停下。上面把中斷點 4 說成「跟 3 是同一個地方」是錯的，已更正。

**`objdump -d` 印的是「檔案裡的位址」，不是「執行時的位址」。** 本機 gcc 預設產生 PIE（[ELF是什麼第三節](./ELF是什麼-可執行與可連結格式的兩種視角.md#三一種格式四種檔案)：`Type` 是 DYN），
PIE 的程式執行時會被整支搬到某個基底位址（base address）上，**執行時的位址 ＝ 基底 ＋ 檔案裡的位址**（[第一節](#一組合語言在哪裡一行文字一個機器指令)〈第 1 欄的位址從哪裡算：`.text` 區段〉的 `info proc mappings` 實驗看過同一件事）。
在 gdb 裡，基底是 `0x555555554000`：

```
$ gdb -batch -ex 'break *deref+4' -ex run -ex 'p deref' -ex 'info proc mappings' -ex kill ./paren
$1 = {long (long *)} 0x5555555551f0 <deref>          ← 執行時 deref 在 0x5555555551f0，不是 0x11f0
      0x555555554000     0x555555555000     0x1000        0x0  r--p   …/paren   ← 基底 0x555555554000
      0x555555555000     0x555555556000     0x1000     0x1000  r-xp   …/paren   ← 機器碼所在的分頁
      …
```

`0x555555554000 ＋ 0x11f4 ＝ 0x5555555551f4`，這才是 `deref+4` 執行時的位址；`0x11f4` 這個位址在執行時沒有對映任何分頁，所以 gdb 放不進中斷點。

兩種寫法的差別在於 gdb **什麼時候算位址**。兩個中斷點一起下、`run` 前後各看一次 `info breakpoints`：

```
$ gdb -batch -ex 'break *0x11f4' -ex 'break *deref+4' -ex 'info breakpoints' -ex run -ex 'info breakpoints' ./paren
Num     Type           Disp Enb Address            What
1       breakpoint     keep y   0x00000000000011f4 in deref at paren.c:4
2       breakpoint     keep y   0x00000000000011f4 in deref at paren.c:4     ← run 之前：兩個一樣
Warning:
Cannot insert breakpoint 1.
Cannot access memory at address 0x11f4

Num     Type           Disp Enb Address            What
1       breakpoint     keep y   0x00000000000011f4                           ← 寫死的數字：不會跟著搬
2       breakpoint     keep y   0x00005555555551f4 in deref at paren.c:4     ← 用名字寫的：程式載入後 gdb 重新算
```

- `break *deref+4`：位置裡有符號名稱 `deref`，程式載入後 gdb 用新的位址重新算一次，所以會自動跟著搬到 `0x5555555551f4`。
- `break *0x11f4`：只是一個數字，gdb 不知道它跟哪個函式有關，不會重新算。

**想直接寫數字時有兩個做法：**

1. 寫執行時的位址：`break *0x5555555551f4`（實測可以停下，`x/i $pc` 顯示 `=> 0x5555555551f4 <deref+4>: mov (%rdi),%rax`）。
   這個數字在 gdb 裡每次都一樣，因為 gdb 預設關掉位址隨機化（`show disable-randomization` 印出 `Disabling randomization of debuggee's virtual address space is on.`）；
   在 gdb 外面直接執行時，基底每次都不同（ASLR，Address Space Layout Randomization，位址空間配置隨機化）。
2. 編成非 PIE：`gcc -O2 -g -no-pie paren.c -o paren_nopie`，`objdump -d` 印的就是執行時的位址：

   ```
   $ objdump -d --no-show-raw-insn paren_nopie      （節錄）
   00000000004011e0 <deref>:
     4011e0:	endbr64
     4011e4:	mov    (%rdi),%rax
   $ gdb -batch -ex 'break *0x4011e4' -ex run -ex 'x/i $pc' -ex kill ./paren_nopie
   Breakpoint 1 at 0x4011e4: file paren.c, line 4.
   Breakpoint 1, deref (p=p@entry=0x7fffffffd410) at paren.c:4
   => 0x4011e4 <deref+4>:	mov    (%rdi),%rax
   ```

   非 PIE 的程式（`Type` 是 EXEC）固定放在檔案寫的位址，不會搬，所以檔案裡的位址就是執行時的位址。

**實務上最簡單的是第三種：用「函式名＋位移」（`break *deref+4`），讓 gdb 自己算。** 位移 `+4` 可以從 `objdump -d` 讀：`11f4 − 11f0 ＝ 4`。

不管高 7 byte 是什麼，結果都是 −1，因為 `movsbl` 只讀 `%dil`。**參數有多大，指令就讀多大的名字**：`signed char` 讀 `%dil`、`short` 讀 `%di`、
`int` 讀 `%edi`、`long` 和指標讀 `%rdi`。看組合語言時，從暫存器的名字就能反推參數的大小（作業第 5 題會用到）。

### 組合語言裡只有 `%edi`、`%dil`，為什麼 gdb 印得出 `$rdi`？

> 2026-10-06 課後提問：「組合語言完全沒有用到 `rdi`，為什麼 gdb 打印 `rdi` 出得來？文中也要我看 `rdi`。」

**因為 `%edi`、`%di`、`%dil` 不是另外的暫存器，它們都是 `%rdi` 這**一個**暫存器的一部分的名字**（[第二節](#二暫存器cpu-裡的-16-個變數)的表，同一列就是同一個暫存器）。
CPU 裡實際只有 16 個 64 位元的通用暫存器，`%rdi` 是其中一個；寫 `%edi` 就是寫它的低 4 byte，讀 `%dil` 就是讀它的最低 1 byte：

```
             %rdi（64 位元，8 byte）
┌────┬────┬────┬────┬────┬────┬────┬────┐
│ 11 │ 22 │ 33 │ 44 │ 55 │ 66 │ 77 │ 88 │   ← 高位在左（這是數值的寫法，不是記憶體裡的順序）
└────┴────┴────┴────┴────┴────┴────┴────┘
                    └──── %edi（低 4 byte）┘
                              └ %di（低 2 byte）┘
                                   └ %dil（最低 1 byte）┘
```

所以 `main` 的 `mov $0xffffffff,%edi` 寫的就是 `%rdi`（低 4 byte 寫 `ff ff ff ff`、高 4 byte 清成 0），`widen_char` 的 `movsbl %dil,%eax` 讀的也是 `%rdi`（最低 1 byte）。
組合語言沒寫出 `%rdi` 這個名字，只代表那個指令只碰它的一部分，暫存器本身一直都在。

**gdb 可以印任何一個暫存器，不管程式有沒有用到。** 暫存器是 CPU 裡固定存在的儲存格，程式停下來時，gdb 請核心把這個行程**整組**暫存器的值讀出來給它
（讀的方法是第 ② 站之後的內容），所以 `$rdi`、`$rsi`、`$rcx` 都印得出來；程式沒用到的暫存器裡，是之前留下來的值。

同一個暫存器、不同名字的實驗。接著上面的 `dil.c` 與 `dil`，停在 `widen_char` 裡，先把 `%rdi` 設成一個每個 byte 都不同的值，再用各種名字去看、去改：

```
$ gdb -batch -ex 'break *widen_char+4' -ex run \
      -ex 'set $rdi = 0x1122334455667788' \
      -ex 'p/x $rdi' -ex 'p/x $edi' -ex 'p/x $di' -ex 'p/x $dil' \
      -ex 'set $dil = 0xff' -ex 'p/x $rdi' \
      -ex 'set $edi = 0xaabbccdd' -ex 'p/x $rdi' \
      -ex 'info registers rdi rsi rcx' -ex kill ./dil
$1 = 0x1122334455667788        ← $rdi：整個 8 byte
$2 = 0x55667788                ← $edi：低 4 byte
$3 = 0x7788                    ← $di ：低 2 byte
$4 = 0x88                      ← $dil：最低 1 byte
$5 = 0x11223344556677ff        ← 改 $dil，$rdi 的最低 1 byte 跟著變：證明是同一個暫存器
$6 = 0x11223344aabbccdd        ← 改 $edi，$rdi 的低 4 byte 跟著變（注意高 4 byte 沒有被清成 0，見下面）
rdi            0x11223344aabbccdd  1234605617868164317
rsi            0x555555556004      93824992239620
rcx            0x555555557dc0      93824992247232     ← widen_char 沒用到 %rcx，照樣印得出來（是之前留下來的值）
```

（`info registers 名字…` 列出指定暫存器的十六進位與十進位值，不給名字就列出全部；`kill` 結束被除錯的程式。完整說明見 [gdb完整命令參考](./gdb完整命令參考.md)。）

**注意 `$6`：gdb 的 `set $edi = …` 沒有把高 4 byte 清成 0。** 「寫 32 位元的名字會把高 32 位元清成 0」是 **CPU 執行指令**時的規則（[第二節](#二暫存器cpu-裡的-16-個變數)〈寫低位的名字時，高位會怎樣？〉），
`main` 的 `mov $0xffffffff,%edi` 是 CPU 執行的指令，所以有清；gdb 的 `set` 不是 CPU 指令，是 gdb 自己改暫存器的值，它只改你指定的那幾個 byte。用 gdb 改暫存器做實驗時要記得這個差別。

### `%dil` 是 `0xff`，為什麼 `%eax` 變成 `0xffffffff`：符號擴展補的是第 7 位元

> 2026-10-06 課後提問：「`%dil` 已經是 `0xff`，從 `%eax` 出來為什麼是 `0xffffffff`？高 3 個 byte 怎麼自動用 1 填？」

**這就是 `movsbl` 的「s」：符號擴展（sign extension）。** `movsbl %dil, %eax` ＝ move **s**ign-extended **b**yte to **l**ong：
把 1 byte 放進 `%eax` 的最低 1 byte，**新多出來的高 3 byte，每一個 bit 都複製來源的最高位元（第 7 位元，也就是符號位）**。
`0xff` 是 `1111 1111`，第 7 位元是 1，所以高 3 byte 全補 1，得到 `0xffffffff`。C 層級的同一件事在
[第 2 課第六節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#六有號與無號之間的轉換位元不變換個讀法)〈**變寬（擴展，extension）**〉（`extend.c` 的 `a1`）。

**為什麼要補 1：為了讓值不變。** 參數是 `signed char`，`0xff` 用二補數讀是 −1（最高位的權重是 −2⁷）：
−128 + 127 = −1。要把它變成 `int` 而且仍然是 −1，`int` 的二補數裡 −1 就是 `0xffffffff`（−2³¹ + (2³¹ − 1) = −1）。
一般來說，在二補數最前面多補一個「跟原本最高位相同」的 bit，值不會變：
- 最高位是 0（正數或 0）：前面補 0，多出來的位元權重乘上 0，值當然不變。
- 最高位是 1（負數）：原本最高位的權重是 −2ⁿ，補一個 1 在更前面之後，新的最高位權重是 −2ⁿ⁺¹、原本那一位變成 +2ⁿ，合起來 −2ⁿ⁺¹ + 2ⁿ = −2ⁿ，跟原來一樣。補幾個都是同樣的道理。

對照**零擴展（zero extension）**的 `movzbl`（move **z**ero-extended **b**yte to **l**ong）：不管第 7 位元是什麼，高位一律補 0。
參數是 `unsigned char` 時 gcc 就用它，因為無號的 `0x80` 代表 128，補 0 才能保持 128。

實驗：同一個 byte，分別交給 `movsbl` 和 `movzbl`。`ext.c`（檔案：[`ext.c`](../small-project/s1-04-asm-basics/ext.c)）完整原始碼：

```c
#include <stdio.h>

__attribute__((noinline)) int widen_char(signed char c)    { return c; }
__attribute__((noinline)) int widen_uchar(unsigned char c) { return c; }

int main(void)
{
    printf("%d %d\n", widen_char(-1), widen_uchar(255));
    return 0;
}
```

```
$ gcc -O2 -g ext.c -o ext
$ objdump -d --no-show-raw-insn ext     （節錄）
0000000000001190 <widen_char>:
    1190:	endbr64
    1194:	movsbl %dil,%eax          ← signed char：符號擴展
    1198:	ret
00000000000011a0 <widen_uchar>:
    11a0:	endbr64
    11a4:	movzbl %dil,%eax          ← unsigned char：零擴展
    11a8:	ret
```

在 gdb 裡兩個函式各下一個中斷點，停下來時把 `%rdi` 設成高位是垃圾、最低 1 byte 是 `0x80` 的值，`stepi` 執行擴展指令，再印 `%eax`
（`x/i $pc` 印出「下一條要執行的指令」，用來確認停在哪個函式）：

```
$ gdb -batch -ex 'break *widen_char+4' -ex 'break *widen_uchar+4' -ex run \
      -ex 'x/i $pc' -ex 'set $rdi = 0x1122334455667780' -ex stepi -ex 'p/x $eax' -ex 'p $eax' \
      -ex continue \
      -ex 'x/i $pc' -ex 'set $rdi = 0x1122334455667780' -ex stepi -ex 'p/x $eax' -ex 'p $eax' \
      -ex kill ./ext
=> 0x5555555551a4 <widen_uchar+4>:	movzbl %dil,%eax
$1 = 0x80
$2 = 128
=> 0x555555555194 <widen_char+4>:	movsbl %dil,%eax
$3 = 0xffffff80
$4 = -128
```

（先停在 `widen_uchar`：C 標準沒有規定函式參數的求值順序，gcc 這次先算 `printf` 的第 3 個參數。）
把 `0x80` 換成 `0x7f`、`0xff` 各跑一次，結果整理成表：

| `%dil` | 二進位 | 第 7 位元 | `movsbl` 後的 `%eax` | `movzbl` 後的 `%eax` |
|---|---|---|---|---|
| `0x7f` | `0111 1111` | 0 | `0x0000007f` ＝ 127 | `0x0000007f` ＝ 127 |
| `0x80` | `1000 0000` | 1 | `0xffffff80` ＝ −128 | `0x00000080` ＝ 128 |
| `0xff` | `1111 1111` | 1 | `0xffffffff` ＝ −1 | `0x000000ff` ＝ 255 |

第 7 位元是 0 時兩個指令結果一樣；是 1 時才分出差別。CPU 不知道這個 byte「是」有號還是無號，**是 gcc 依 C 的型別選了 `movs` 還是 `movz`**——
跟[第五節](#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)「有號還是無號由條件碼決定」是同一個道理：型別只活在編譯器選的指令裡。

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

#### 上表的四個旗標在講什麼：用 10 組數字實際跑一次

> 2026-10-07 課後提問：「第5節旗標這邊完全看不懂在幹嘛？」（指上面〈四個旗標〉那張表）

上表「什麼時候是 1」那一欄很難懂，因為它同時在講**同一次運算的兩種解讀**。先抓住一件事：

**CPU 做一次加法或減法，只得到一串位元；四個旗標是從四個角度替「這一次運算」各記一筆。**

| 旗標 | 它在回答的問題 | 看的是什麼 |
|---|---|---|
| ZF | 「留下來的位元是不是全為 0？」 | 結果的位元 |
| SF | 「留下來的最高位元是不是 1？」 | 結果的位元（只是把最高位元抄一份） |
| CF | 「**把兩個數當無號數**算，正確答案裝得進這個大小嗎？」裝不下（加法超過最大值、減法變成負的）就是 1 | 運算過程 |
| OF | 「**把兩個數當有號數**算，正確答案裝得進這個大小嗎？」裝不下就是 1 | 運算過程 |

CPU 不知道你的資料是有號還是無號，所以 CF 和 OF **兩個都算、兩個都設**；上表最右欄「對誰有意義」的意思是：你的資料如果是無號數，就只有 CF 的答案對你有用，OF 是在回答一個跟你無關的問題，反之亦然。

為了心算方便，下面的實驗全部用 **8 位元（1 byte）**：無號能裝 0～255，有號能裝 −128～127（[第 2 課第五節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#五有號整數二補數)）。32、64 位元的道理完全一樣，只是範圍變大。

**實驗程式**：`flags.s`（檔案：[`flags.s`](../small-project/s1-04-asm-basics/flags.s)）做一次 `cmpb` 或 `addb`，把做完後的旗標整包回傳；`flagsmain.c`（檔案：[`flagsmain.c`](../small-project/s1-04-asm-basics/flagsmain.c)）餵 10 組數字、把四個旗標拆出來印。完整原始碼：

```asm
# flags.s：做一次 8 位元的 cmp 或 add，把做完後的旗標整包回傳
# 參數：%dil = D（第 1 個參數的最低 1 byte），%sil = S（第 2 個參數的最低 1 byte）
	.text
	.globl	flags_cmp
flags_cmp:                       # unsigned long flags_cmp(unsigned char d, unsigned char s)
	cmpb	%sil, %dil           # 算 D − S（8 位元），結果丟掉，只設旗標
	pushfq                       # 把整個 rflags 複製一份放到堆疊上
	popq	%rax                 # 再從堆疊取到 %rax ＝ 回傳值
	ret

	.globl	flags_add
flags_add:                       # unsigned long flags_add(unsigned char d, unsigned char s)
	addb	%sil, %dil           # 算 D + S（8 位元），結果寫回 %dil，同時設旗標
	pushfq
	popq	%rax
	ret

	.section	.note.GNU-stack,"",@progbits
```

逐行說明：

- `.text`、`.globl`：指示詞（[第六節](#先看完整的-s-檔哪些是指令哪些不是)〈先看完整的 `.s` 檔：哪些是指令、哪些不是〉），「下面是程式碼」、「這個名字給別的檔案用」。
- `cmpb %sil, %dil`：`b` 字尾是 1 byte；`%dil`、`%sil` 是 `%rdi`、`%rsi` 最低的 1 byte（[第二節](#二暫存器cpu-裡的-16-個變數)）。參數是 `unsigned char`，只佔那 1 byte（[第三節](#讀懂實驗一的範例為什麼寫-1rdi為什麼用-dil)〈讀懂實驗一的範例：為什麼寫 `1(%rdi)`、為什麼用 `%dil`〉）。
- `addb %sil, %dil`：同上，但做加法；結果寫回 `%dil`，函式不用它，我們只要旗標。
- `pushfq` ＋ `popq %rax`（**新指令，本課沒教**）：旗標不能用 `mov` 讀——本機寫 `mov %rflags, %rax` 會得到 `Error: bad register name '%rflags'`。`pushfq` 把整個 rflags 推到堆疊（stack）上，`popq %rax` 再把它取進 `%rax`，兩行合起來就是「把 rflags 複製到 `%rax`」。堆疊是什麼，第 5 課以後講；這裡只要知道這兩行的效果。`%rax` 是回傳值（[第二節](#二暫存器cpu-裡的-16-個變數)的暫存器表），所以 C 拿到的回傳值就是整包旗標。
- `pushfq`、`popq` 都不改旗標（[第五節〈哪些指令會改旗標〉](#哪些指令會改旗標)：只有算術與邏輯指令會改），所以存下來的就是 `cmpb`／`addb` 剛設好的旗標。
- `.section .note.GNU-stack,...`：告訴連結器「堆疊不需要能執行」。手寫 `.s` 沒加這行，本機連結時會出現 `warning: ... missing .note.GNU-stack section implies executable stack`；gcc 自己產生的 `.s` 都會自動帶這行。

```c
/* flagsmain.c：對幾組 8 位元的數字做 cmp／add，印出結果與四個旗標 */
#include <stdio.h>

unsigned long flags_cmp(unsigned char d, unsigned char s);   /* 在 flags.s */
unsigned long flags_add(unsigned char d, unsigned char s);

/* rflags 裡四個旗標的位置：CF 第 0 位元、ZF 第 6、SF 第 7、OF 第 11 */
static void show(const char *op, unsigned char d, unsigned char s)
{
    unsigned long f;
    unsigned char r;

    if (op[0] == 'c') { f = flags_cmp(d, s); r = (unsigned char)(d - s); }
    else              { f = flags_add(d, s); r = (unsigned char)(d + s); }

    printf("%s D=0x%02x S=0x%02x -> 0x%02x | 無號 %3u %c %3u -> %3u | 有號 %4d %c %4d -> %4d | ZF=%lu SF=%lu CF=%lu OF=%lu\n",
           op, d, s, r,
           d, op[0] == 'c' ? '-' : '+', s, r,
           (signed char)d, op[0] == 'c' ? '-' : '+', (signed char)s, (signed char)r,
           (f >> 6) & 1, (f >> 7) & 1, (f >> 0) & 1, (f >> 11) & 1);
}

int main(void)
{
    show("cmp", 5, 5);        /* 相等 */
    show("cmp", 5, 3);        /* 一般的大減小 */
    show("cmp", 3, 5);        /* 小減大 */
    show("cmp", 0xff, 1);     /* 第五節實驗那組：-1 和 1 */
    show("cmp", 1, 0xff);     /* 反過來 */
    show("cmp", 0x80, 1);     /* -128 - 1 */
    show("add", 0x7f, 1);     /* 127 + 1 */
    show("add", 0xff, 1);     /* 255 + 1，也就是 -1 + 1 */
    show("add", 200, 100);    /* 200 + 100 */
    show("add", 0x80, 0x80);  /* -128 + -128 */
    return 0;
}
```

`(f >> 6) & 1` 是「把第 6 位元移到最右邊、只留那一位」，取出 ZF；四個旗標在 rflags 裡的位置（CF 第 0、ZF 第 6、SF 第 7、OF 第 11）是 Intel 規定的，gdb 的 `info registers eflags` 也是照這個位置解讀。印出來的「無號」「有號」兩欄是把**同一串位元**用兩種方式讀（`(signed char)` 轉型，[第 2 課第五節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#五有號整數二補數)），`->` 後面是 8 位元**留下來的**結果，不一定是數學上的正確答案。

```sh
cd small-project/s1-04-asm-basics
gcc -O0 -g flagsmain.c flags.s -o flags
./flags
```

本機輸出（加上列號，方便下面引用）：

```
①  cmp D=0x05 S=0x05 -> 0x00 | 無號   5 -   5 ->   0 | 有號    5 -    5 ->    0 | ZF=1 SF=0 CF=0 OF=0
②  cmp D=0x05 S=0x03 -> 0x02 | 無號   5 -   3 ->   2 | 有號    5 -    3 ->    2 | ZF=0 SF=0 CF=0 OF=0
③  cmp D=0x03 S=0x05 -> 0xfe | 無號   3 -   5 -> 254 | 有號    3 -    5 ->   -2 | ZF=0 SF=1 CF=1 OF=0
④  cmp D=0xff S=0x01 -> 0xfe | 無號 255 -   1 -> 254 | 有號   -1 -    1 ->   -2 | ZF=0 SF=1 CF=0 OF=0
⑤  cmp D=0x01 S=0xff -> 0x02 | 無號   1 - 255 ->   2 | 有號    1 -   -1 ->    2 | ZF=0 SF=0 CF=1 OF=0
⑥  cmp D=0x80 S=0x01 -> 0x7f | 無號 128 -   1 -> 127 | 有號 -128 -    1 ->  127 | ZF=0 SF=0 CF=0 OF=1
⑦  add D=0x7f S=0x01 -> 0x80 | 無號 127 +   1 -> 128 | 有號  127 +    1 -> -128 | ZF=0 SF=1 CF=0 OF=1
⑧  add D=0xff S=0x01 -> 0x00 | 無號 255 +   1 ->   0 | 有號   -1 +    1 ->    0 | ZF=1 SF=0 CF=1 OF=0
⑨  add D=0xc8 S=0x64 -> 0x2c | 無號 200 + 100 ->  44 | 有號  -56 +  100 ->   44 | ZF=0 SF=0 CF=1 OF=0
⑩  add D=0x80 S=0x80 -> 0x00 | 無號 128 + 128 ->   0 | 有號 -128 + -128 ->    0 | ZF=1 SF=0 CF=1 OF=1
```

**一個旗標一個旗標看：**

**ZF（結果是 0）**——①、⑧、⑩ 是 1，三列的結果都是 `0x00`。
- ① 5 − 5 = 0，這是 `cmp` 用 ZF 判斷「相等」的原理：D − S 是 0，就是 D == S。
- ⑩ 是陷阱：128 + 128 的正確答案是 256，但 256 是 `0x100`，8 位元只留下 `0x00`，ZF 照樣是 1。**ZF 只看留下來的位元，不管數學上的答案。**

**SF（最高位元是 1）**——③、④、⑦ 是 1，結果分別是 `0xfe`、`0xfe`、`0x80`，二進位最左邊都是 1。
- SF 就是把結果的第 7 位元（8 位元的最高位元）抄一份，**它不判斷什麼**。當成有號數讀，最高位元是 1 就是負數，所以叫 sign flag。
- ⑦ 是陷阱：127 + 1 的正確答案是 +128，但留下的 `0x80` 當有號讀是 −128，SF = 1 說「負的」——**溢位時 SF 是錯的**。這就是下面 OF 存在的理由。

**CF（當成無號數，裝不下）**——只看「無號」那一欄：
- ③ 3 − 5：無號數沒有負數，小減大要向更高位借位（borrow），CF = 1。所以 `cmp` 之後 **CF = 1 就代表 D < S（無號）**，上表寫的「`cmp` 時就是 D 比 S 小」就是這個意思。
- ⑤ 1 − 255：一樣小減大，CF = 1。注意有號那一欄是 1 − (−1) = 2，完全正常——**CF 不管有號的解讀**。
- ④ 255 − 1 = 254：大減小，不用借位，CF = 0。有號那欄是 −1 − 1 = −2（負的），CF 還是 0。
- ⑧ 255 + 1、⑨ 200 + 100、⑩ 128 + 128：正確答案 256、300、256 都超過 255，最高位元往外進位（carry），CF = 1。
- ⑦ 127 + 1 = 128，沒超過 255，CF = 0。

**OF（當成有號數，裝不下）**——只看「有號」那一欄：
- ⑥ −128 − 1：正確答案 −129，比 −128 還小，裝不下 → OF = 1。留下的 `0x7f` 讀成 +127，正負號反了。
- ⑦ 127 + 1：正確答案 +128，比 127 大 → OF = 1（留下的讀成 −128）。
- ⑩ −128 + −128：正確答案 −256 → OF = 1。這一列 CF 也是 1：兩種解讀**都**裝不下。
- ⑧ −1 + 1 = 0、⑨ −56 + 100 = 44：有號看完全正常，OF = 0；但無號看都進位了，CF = 1。

**把兩個「裝不下」放在一起比**：⑦ 只有 OF、⑧⑨ 只有 CF、⑩ 兩個都有、①～④ 的減法裡 ③⑤ 只有 CF、⑥ 只有 OF。同一串位元，兩個旗標經常給相反的答案——**所以 CF 只對無號數有意義、OF 只對有號數有意義**，用錯了就是在聽一個跟你的資料無關的答案。

**回頭看條件碼表：為什麼有號的「小於」是 SF ≠ OF，而不是只看 SF？**
`cmp` 之後想知道 D < S，直覺是「D − S 是負的」，也就是 SF = 1。看三列 D < S（有號）的：
- ③ 3 < 5：SF = 1、OF = 0，直覺對。
- ④ −1 < 1：SF = 1、OF = 0，直覺對。
- ⑥ −128 < 1：**SF = 0、OF = 1**。−128 − 1 溢位了，SF 被翻成「正的」。只看 SF 會答錯，但 OF = 1 告訴你「SF 是反的」，所以要把 SF 再翻回來——SF ≠ OF 就是「沒溢位時看 SF；溢位時看 SF 的相反」。

無號的「小於」（`b`）就簡單：CF = 1。③ 3 < 5、⑤ 1 < 255 都是 CF = 1，其他列都是 0。
本節下面[〈實驗：同一個 `cmp`、同一組旗標，`setl` 說是、`setb` 說否〉](#實驗同一個-cmp同一組旗標setl-說是setb-說否)是 32 位元版的④：SF = 1、OF = 0、CF = 0，所以 `setl` 說是、`setb` 說否。

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

**兩個函式只差一個字母**。用 `gdb` 在 `set` 那一行停下，看 `cmp` 剛做完時的旗標（檔案：[`lt.c`](../small-project/s1-04-asm-basics/lt.c)、[`ltmain.c`](../small-project/s1-04-asm-basics/ltmain.c)）：

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

> 參考答案寫在每題下面（2026-10-06 使用者指定）。本篇的題目還沒作答，等第一次作答、批改完再補上。

1. `movl (%rdi,%rsi,4), %eax` 和 `leaq (%rdi,%rsi,4), %rax` 括號裡寫的一模一樣，兩個指令做的事差在哪？各對應 C 的什麼寫法？為什麼 gcc 會拿 `lea` 來算 `x * 3` 這種跟位址無關的東西？
2. `long widen_u(unsigned x) { return x; }` 只要一個 `movl %edi, %eax`，`long widen_s(int x) { return x; }` 卻要 `movslq %edi, %rax`。為什麼 `movl` 就足以完成前者？如果改寫 16 位元的 `%ax`，同樣的理由還成立嗎？
3. `lt_s` 和 `lt_u` 的 `cmpl %esi, %edi` 一模一樣，gdb 也看到兩者的旗標完全相同（`[ SF IF ]`），為什麼一個回傳 1、一個回傳 0？C 的「有號還是無號」在組合語言裡到底是在哪裡決定的？
4. `-O2` 的 `sum_to` 裡，`andl $1, %edi` 和 `je .L3` 中間夾了一個 `movl $1, %eax`。`je` 判斷的是哪個指令的結果？為什麼 gcc 可以這樣排？如果中間夾的是 `addl $1, %eax`，會怎樣？
5. 情境題：你在 kernel 的反組譯裡看到 `movq 24(%rdi,%rsi,8), %rax`，`%rdi` 是函式的第 1 個參數、`%rsi` 是第 2 個。推測它對應的 C 可能長怎樣：第 1 個參數最可能是指向什麼大小的元素的指標？存取的是第幾個元素（用 `i` 表示）？如果同一個位置看到的是 `leaq` 而不是 `movq`，C 又會是什麼？
6. （課後提問延伸）`movsbl` 的 `l` 叫 long，為什麼是 4 byte，而本機 C 的 `long` 是 8 byte？本機 C 的 `long` 在組合語言裡對到哪個字尾？
   情境題：第 5 課你在 `gdb` 裡想看一個 `int` 變數的 4 個 byte，同學說「`int` 在組合語言是 `l`，所以用 `x/1l`」。這樣對嗎？應該用哪個大小字母，為什麼？
7. （課後提問延伸）`int widen_char(signed char c) { return c; }` 編出 `movsbl %dil, %eax`。在 64 位元的機器上，為什麼它只讀 `%dil`，而不讀整個 `%rdi`？假設 `widen_char` 改成讀 4 byte 的 `movl %edi, %eax`，在筆記的 `gdb` 實驗裡把 `%rdi` 改成 `0x11223344556677ff` 之後，`%eax` 的十六進位是多少？是正數還是負數？跟 C 期望的 −1 差在哪？
8. （課後提問延伸）`dil.c` 的組合語言裡只出現 `%edi`、`%dil`，沒有 `%rdi`，為什麼 gdb 能印出 `$rdi`？在 gdb 裡 `set $edi = 0xaabbccdd`，`$rdi` 的高 4 byte 會不會被清成 0？跟 CPU 執行 `movl $0xaabbccdd, %edi` 有什麼不同？
9. （課後提問延伸）`movsbl` 把 1 byte 擴展成 4 byte 時，新補的高 3 byte 是照哪一個位元補的？為什麼這樣補，`signed char` 的 −1 變成 `int` 之後才會還是 −1（用二補數的權重解釋）？
10. （課後提問延伸）`mov %rdi,%rax` 和 `mov (%rdi),%rax` 只差一對括號，兩個指令各拿 `%rdi` 裡的數字做什麼？各對應 C 的什麼寫法？如果 `%rdi` 裡是 1234，哪一個會 segfault，為什麼？
11. （課後提問延伸）`break deref+4` 會得到 `Function "deref+4" not defined.`，`break *deref+4` 卻可以，`*` 讓 gdb 怎麼理解後面的東西？這個 `*` 跟 C 的 `*p` 是同一個意思嗎？在 `-O0` 的程式上，`break deref` 和 `break *deref` 為什麼會停在不同的位址？
12. （課後實作延伸）用 `objdump -d paren` 查到 `deref+4` 在 `0x11f4`，`break *0x11f4` 之後 `run` 卻得到 `Cannot access memory at address 0x11f4`，`break *deref+4` 就沒問題。為什麼？如果程式是用 `-no-pie` 編的，`break *` 加 `objdump` 查到的位址會不會成功？
13. （課後提問延伸）`get_g` 讀全域變數 `g` 用的是 `mov 0x2db5(%rip),%rax`，為什麼 PIE 程式不能直接把 `g` 的位址寫進指令，而要用 `%rip` ＋ 距離？同一個 `tbl[i]`，為什麼非 PIE 的程式可以寫成一條 `mov 0x404060(,%rdi,8),%rax`？
14. （課後提問延伸）8 位元的 `cmpb %sil, %dil`，`%dil` = `0x7f`、`%sil` = `0xff`。寫出做完後 ZF、SF、CF、OF 各是 0 還是 1，並說明 CF、OF 各是用哪一種解讀（無號／有號）算出來的。接著 `setl` 和 `setb` 各會得到 1 還是 0？為什麼有號的「小於」要看 SF ≠ OF，不能只看 SF？
   情境題：同學說「OF = 1 就代表這次加法的結果錯了」。這句話什麼時候對、什麼時候不對？用 8 位元的 `0x7f + 0x01` 說明。
