# s1-04：讀懂 x86-64 組合語言（一）

> 對應課程：第 ① 站第 4 課
> 課程筆記：[x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)
> 自我測驗：寫在同資料夾的 [quiz.md](./quiz.md)

## 說明

這一課的目標是**讀得懂**，不是會寫組合語言。每一題都是：自己寫一小段 C → 先預測 gcc 會產生什麼 → 用 `gcc -S` 實際編出來 → 對照、解釋。

- 看組合語言一律用：`gcc -S -O2 -fno-asynchronous-unwind-tables 檔名.c -o 檔名.s`（題目指定 `-O0` 的除外），然後打開 `.s` 檔看。
  不需要 `main`，因為 `-S` 不連結。
- `.s` 檔不會進 git（白名單），要留的輸出**貼進作答框**。貼的時候只要函式本體（從 `函式名:` 到 `ret`），以 `.p2align`、`.globl` 這類 `.` 開頭的指示詞可以刪掉。
- 筆記裡的範例可以參考，但每一題的型別、數值、情境都跟範例不同，照抄範例答不出來。

每題「解釋原因」的地方都標了答案的來源：
- 【筆記第 X 節】：本課教過，回筆記那一節找。
- 【第 2 課筆記第 X 節】：回 [C的資料在記憶體裡長怎樣](../../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html) 那一節找。
- 【新觀念，先猜】：還沒教過，寫下你的猜測就好，猜錯不算錯。

**「先預測」的題目，一定要先把預測寫進作答框，再編譯。** 預測錯了沒關係，把錯在哪裡寫下來，這才是練習的重點。

**怎麼作答**：每個小題下面有作答框，直接寫在裡面。批改寫在作答框正下方；要重答時，批改下方會預先放好下一個作答框。

---

## 筆記的範例程式

課程筆記裡每個實驗的原始碼都存成檔案放在這個資料夾（2026-10-07 起），可以直接編譯、跟著筆記重做。**它們是範例，不是作業**；作業的檔名（`mul.c`、`elem.c`、`pick.c`、`sumarr.c`、`mystery.c`）跟它們不重複。編譯命令寫在筆記對應的那一段。

| 檔案 | 筆記裡的位置 | 示範什麼 |
|---|---|---|
| [`ops.c`](./ops.c) | [第二節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#二暫存器cpu-裡的-16-個變數)〈寫低位的名字時，高位會怎樣？〉、第四、五節 | `widen_s`／`widen_u`、`get`／`addr`、`times3`、`lt_s`／`lt_u` |
| [`paren.c`](./paren.c) | [第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上表第一列 `(B)` 的 `(%rdi)`〉、〈`break *widen_char+4` 的 `*`〉 | `%rdi` vs `(%rdi)`；`break *位址` |
| [`forms.c`](./forms.c) | 第三節〈上表其他五列〉 | `D(B)`、`(B,I,S)`、`D(B,I,S)`、`D(,I,S)`、`D(%rip)` |
| [`suffix.c`](./suffix.c) | 第三節〈為什麼 `l` 叫 long 卻是 4 byte〉實驗一 | C 的 `long` 編成 `q` 字尾 |
| [`data.s`](./data.s)、[`data2.s`](./data2.s) | 同上，實驗二 | 手寫的 `.s`：`.byte`／`.word`／`.long`／`.quad` 的大小、小端序 |
| [`xv.c`](./xv.c) | 同上，gdb 的 `x` | `x/1l` 的錯誤與 `w`／`h` |
| [`dil.c`](./dil.c) | 第三節〈讀懂實驗一的範例〉、〈`$rdi`〉 | 參數只佔 `%rdi` 最低 1 byte；暫存器的名字 |
| [`ext.c`](./ext.c) | 第三節〈符號擴展補的是第 7 位元〉 | `movsbl` vs `movzbl` |
| [`lt.c`](./lt.c)、[`ltmain.c`](./ltmain.c) | [第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈實驗：同一個 `cmp`、同一組旗標〉 | `setl` vs `setb` 的旗標 |
| [`cmptest.s`](./cmptest.s)、[`cmptestmain.c`](./cmptestmain.c) | 第五節〈上表「典型用途」欄的 `i`、`n` 是誰？〉 | `sum_to` 的 `cmpl`（i − n）與 `testl`（n & n）的旗標 |
| [`andtest.s`](./andtest.s)、[`andtestmain.c`](./andtestmain.c) | [第七節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#七逐行讀-sum_to-的--o2-版)〈上表的 `andl` 和 `testl`：看起來一樣，差在哪？〉 | `and` 寫回結果、`test` 只設旗標；旗標相同 |
| [`flags.s`](./flags.s)、[`flagsmain.c`](./flagsmain.c) | [第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上表的四個旗標在講什麼〉 | 10 組 8 位元 `cmpb`／`addb` 的 ZF/SF/CF/OF；手寫 `.s` |
| [`countdown.c`](./countdown.c) | 第五節〈上面這段十進位暖身：十位也是 0，98 是從哪裡冒出來的？〉 | 8 位元從 3 倒數 5 下，翻過 0 變 255、254 |
| [`borrow.c`](./borrow.c)（配 `flags.s`） | [第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面③：3 − 5 在 CPU 裡實際上是怎麼算的？〉 | 直式減法逐位借位、CF＝最高位往外借；加法器 D + ~S + 1 |
| [`carries.c`](./carries.c) | [第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面⑦：127 + 1 明明有「進位」，為什麼 CF 還是 0？〉 | 逐位印出收到的進位；CF＝第 7 位往外的進位、OF＝進第 7 位 XOR 出第 7 位 |
| [`ofrule.c`](./ofrule.c) | [第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面那條規則：OF 不是看溢位嗎？為什麼又變成看進位？〉 | 窮舉 65536 組：有號溢位 ＝ 進第 7 位 XOR 出第 7 位；第 7 位的 8 種情況 |
| [`clamp0.c`](./clamp0.c) | [第六節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#六逐行讀-sum_to-的--o0-版)〈`if` 怎麼翻〉（2026-10-08 批改作業 3-3 時補教） | `-O0` 的 `if`：條件跳躍檢查 C 條件的反面（`jns`） |
| [`ivopts.c`](./ivopts.c) | [第七節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#七逐行讀-sum_to-的--o2-版)〈陣列迴圈的 `i` 為什麼不見了〉（2026-10-08 批改作業 4-4 時補教） | `-O2` 與 `-O2 -fno-ivopts`：`i` 被換成往前走的指標 |
| [`../s1-01-hello-compile/sumto.c`](../s1-01-hello-compile/sumto.c) | 第一、六、七節 | `sum_to` 的 `-O0`／`-O2`（第 1 課的檔案） |

---

## 批改結果（2026-10-08）

| 題目 | 結果 | 錯在哪／缺什麼 |
|---|---|---|
| 1-1 | 大致對 | 重答後：`mul5`／`mul9` 可以、`mul6`／`mul7` 不行，對；缺「`B` 也放 `x`，算 `x + x×S`，所以 5 ＝ 1 + 4、9 ＝ 1 + 8」 |
| 1-2 | 不完整 | 重答後仍只寫「沒有猜錯的」，沒拿第 1 次預測對照（猜成四個都一個 `lea`、`S` 寫 5、括號裡加 `$`）；已附參考答案 |
| 1-3 | 對 | 抓到重點：`mul6` ＝ 3 倍再 ×2，`mul7` ＝ 8 倍減 1 倍 |
| 2-1 | 錯 | 預測題：指令名字、`B` 的暫存器、`S`、`D`、`$` 都跟實際不同；不重答，錯在哪寫進 2-2 |
| 2-2 | 大致對 | 重答後：四行的錯都找到了（`%rdi`、`S`、`D`、`movswl`／`movzbl`）；新誤會：`%rsi(%rdi)` 不合法，`D` 只能是常數 |
| 2-3 | 對 | 重答後：`0xffff` 第 15 位元是 1，符號擴展成 −1；看的是來源型別 |
| 2-4 | 大致對 | 16、8 對；少了「位址以 byte 為單位，2 個 `long` ＝ 2×8 byte」這一步 |
| 3-1 | 對 | 重答後：`jnb` 只看 CF = 0，相等時也不借位，已經包含在內 |
| 3-2 | 大致對 | 回傳值 2、1 都對；沒講到「同一組旗標，有號讀 SF/OF、無號讀 CF」，運算元順序不是原因 |
| 3-3 | 對 | 後半對（跳到 `return 2`）；前半是新觀念，已補教（筆記第六節〈`if` 怎麼翻〉） |
| 3-4 | 不完整 | 重答後：`nb` 看 CF 對，但沒翻成 C；應為 `b >= a`（＝`!(a > b)`），`unsigned` 的 `b - a >= 0` 永遠成立；已附參考答案 |
| 4-1 | 大致對 | 位置與迴圈條件都對；`addq %rdx, %rax` 之後是 `&a[i]` 不是 `a[i]`；有幾行沒註解 |
| 4-2 | 對 | 抓到重點：`-O0` 的 `a`、`i` 在記憶體，`B`、`I` 只能是暫存器 |
| 4-3 | 大致對 | 漏了 `%rdx`（`&a[n]`）；`%rdi` 是每圈 +8 的指標，不只是 `a`；`testq`／`jle` 沒說明 |
| 4-4 | 對 | 前半對：拿 `%rdi` 跟終點 `%rdx` 比；後半是新觀念，已補教（筆記第七節〈歸納變數最佳化〉） |
| 5-1 | 對 | 重答後：`i` 改 `long`，`movslq` 消失，本體一模一樣（實測） |
| 5-2 | 對 | 重答後：`!(a < b)` 一模一樣（實測）；更直接是 `a >= b` |
| 5-3 | 不完整 | 重答後：`i` 是 8 byte 對，但線索是「沒有擴展」不是「用了 `%rsi`」；`xorl %eax, %eax` 清的是整個 `%rax`，回傳 `long` 也一樣；「無法確定」的型別沒答出（已附參考答案） |

最終（含重答）：**對 8**（1-3、2-3、3-1、3-3、4-2、4-4、5-1、5-2）、**大致對 6**（1-1、2-2、2-4、3-2、4-1、4-3）、**不完整 3**（1-2、3-4、5-3）、**錯 1**（2-1，預測題不重答）。
第 1 次：對 4、大致對 4、錯 1、待重答 9（第 1 次的結果：錯 3〔1-1、5-1、5-2〕、不完整 6〔1-2、2-2、2-3、3-1、3-4、5-3〕）；重答 2026-10-08 批改完。
**還沒弄懂的觀念**：無號比較的條件跳躍翻回 C（CF ＝ 借位 ⇔ 無號的 `<`）；組合語言什麼時候看得出有號無號（只有擴展與比較）；寫 32 位元暫存器會清掉高 32 位元。

---

## 1. 用 `lea` 做乘法

寫 `mul.c`，裡面四個函式（都是 `long`）：

```c
long mul5(long x) { return x * 5; }
long mul9(long x) { return x * 9; }
long mul6(long x) { return x * 6; }
long mul7(long x) { return x * 7; }
```

### 1-1. 編譯前先預測：四個裡面，哪幾個 gcc 可以只用**一個** `lea` 做完？把你預測的那一行 `lea` 寫出來（暫存器用 `%rdi`、`%rax`）。另外幾個為什麼不行？【筆記[第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)的 `D(B,I,S)` 表、[第四節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#四mov-與-lea搬資料-vs-算位址)〈`lea` 被拿來做算術〉】

**第 1 次（2026-10-08）**
```
leaq 0x0(,%rdi,$5), %rax
leaq 0x0(,%rdi,$9), %rax
leaq 0x0(,%rdi,$6), %rax
leaq 0x0(,%rdi,$7), %rax
```

> **批改**：**錯**。
> - 預測「四個都能一個 `lea` 做完」不對：從你 1-2 貼的輸出就看得到，只有兩個是一個 `lea`，另外兩個都是兩個指令。
> - 你寫的 `0x0(,%rdi,$5)` 有兩個文法問題：① `$` 是**立即數**（immediate）的記號，只用在「整個運算元就是一個數字」的時候（例如 `$1`）；記憶體運算元括號裡的 `D`、`S` 不加 `$`。② `S` 不是隨便一個數字都可以。
> - 題目後半「另外幾個為什麼不行」沒有回答。
>
> **提示**：回[筆記第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元) `D(B,I,S)` 表下面，看 `S` 只能是哪幾個數。再想：`lea` 一次能算的是 `D + B + I×S`，如果 `B`、`I` 都放 `%rdi`（也就是 `x`），能算出 `x` 的幾倍？把所有能湊出的倍數列出來，就知道哪幾個做得到、哪幾個做不到。重答時寫出：能一個 `lea` 做完的那幾行（正確文法），以及其他幾個為什麼不行。

**第 2 次（重答，2026-10-08）**

> 題目：編譯前先預測：四個裡面，哪幾個 gcc 可以只用**一個** `lea` 做完？把你預測的那一行 `lea` 寫出來（暫存器用 `%rdi`、`%rax`）。另外幾個為什麼不行？

```
leaq (%rdi, %rdi, 4)
leaq (%rdi, %rdi, 8)
mul5, mul9 可以
mul6, mul7 不行,因爲S只能是1,2,4,8
```

> **批改（第 2 次）**：**大致對**。`mul5`、`mul9` 可以、`mul6`、`mul7` 不行，兩行 `lea` 的括號也寫對了（少了目的地 `, %rax`）。但「因為 `S` 只能是 1、2、4、8」只說了一半：照這句話，5 和 9 也不在 1、2、4、8 裡，為什麼它們可以？缺的是「`B` 也放 `x`，所以算的是 `x + x×S`」這一步。
>
> **參考答案**【[筆記第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元) `D(B,I,S)` 表、[第四節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#四mov-與-lea搬資料-vs-算位址)〈`lea` 被拿來做算術〉】`S` 只能是 1、2、4、8。一個 `lea` 能算 `D + B + I×S`，`B`、`I` 都放 `x` 時，做得出 `x×S`（1、2、4、8 倍）或 `x + x×S`（2、3、5、9 倍）。5 ＝ 1 + 4、9 ＝ 1 + 8，所以 `mul5` 是 `leaq (%rdi,%rdi,4), %rax`、`mul9` 是 `leaq (%rdi,%rdi,8), %rax`；6 和 7 不在這些倍數裡，一個 `lea` 做不到。

### 1-2. 用 `-O2` 編譯，貼上四個函式的本體，跟預測對照：哪裡猜對、哪裡猜錯？

**第 1 次（2026-10-08）**
```
mul5:
	endbr64
	leaq	(%rdi,%rdi,4), %rax
	ret
	.size	mul5, .-mul5
	.p2align 4
	.globl	mul9
	.type	mul9, @function
mul9:
	endbr64
	leaq	(%rdi,%rdi,8), %rax
	ret
	.size	mul9, .-mul9
	.p2align 4
	.globl	mul6
	.type	mul6, @function
mul6:
	endbr64
	leaq	(%rdi,%rdi,2), %rax
	addq	%rax, %rax
	ret
	.size	mul6, .-mul6
	.p2align 4
	.globl	mul7
	.type	mul7, @function
mul7:
	endbr64
	leaq	0(,%rdi,8), %rax
	subq	%rdi, %rax
	ret
	.size	mul7, .-mul7
	.ident	"GCC: (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
```

> **批改**：**不完整**。輸出貼對了（`.size` 以後的指示詞可以刪掉，題目只要本體），但題目要的「跟預測對照：哪裡猜對、哪裡猜錯」完全沒寫。
>
> **提示**：逐個函式對照 1-1：你猜一個 `lea` 的，實際上是一個還是兩個指令？你寫的 `0x0(,%rdi,$5)` 跟實際的 `mul5` 那一行比，`B`、`I`、`S` 各差在哪？為什麼實際的寫法能用合法的 `S` 算出 5 倍？

**第 2 次（重答，2026-10-08）**

> 題目：用 `-O2` 編譯，貼上四個函式的本體，跟預測對照：哪裡猜對、哪裡猜錯？

```
重答後, 沒有猜錯的
```

> **批改（第 2 次）**：**不完整**。「重答後沒有猜錯的」沒有回答題目：題目要的是拿**第 1 次**的預測跟實際輸出對照。第 1 次你預測四個都能一個 `lea` 做完，也寫了不合法的 `S`，這些都是猜錯的地方，要寫出來。重答只有這一次，所以直接附參考答案。
>
> **參考答案**：猜錯的有三處：① 預測四個都是一個 `lea`，實際上只有 `mul5`、`mul9` 是，`mul6`、`mul7` 各要兩個指令；② 寫成 `0x0(,%rdi,$5)`，`S` 用了不合法的 5、6、7、9，實際的寫法是把 `B` 也放 `%rdi`，用 `x + x×4`、`x + x×8` 湊出 5 倍、9 倍；③ 括號裡加了 `$`，記憶體運算元裡的數字不加 `$`。猜對的：用 `leaq`、結果放 `%rax`。

### 1-3. 不是一個 `lea` 就做完的那幾個，逐行寫出每個指令之後 `%rax` 等於 `x` 的幾倍，說明 gcc 是怎麼湊出來的。【筆記[第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)（`D(,I,S)` 這種省略形式）、[第四節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#四mov-與-lea搬資料-vs-算位址)〈本課的算術與邏輯指令〉】

**第 1 次（2026-10-08）**
```
mul6:
	endbr64
	leaq	(%rdi,%rdi,2), %rax # rax =rdi + 2*rdi = 3rdi
	addq	%rax, %rax          # rax = rax + rax =6*rdi = 6*x

mul7:
	endbr64
	leaq	0(,%rdi,8), %rax    # rax = 8*rdi
	subq	%rdi, %rax          # rax = rax - rdi = 7*rdi

```

> **批改**：**對**。`mul6` 是先 3 倍再自己加自己變 6 倍；`mul7` 是先 8 倍再減掉一個 `x`。兩個都逐行寫出了 `%rax` 是幾倍。
>
> **參考答案**【[筆記第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)（`D(,I,S)` 這種省略形式）、[第四節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#四mov-與-lea搬資料-vs-算位址)〈本課的算術與邏輯指令〉】
> - `mul6`：`leaq (%rdi,%rdi,2), %rax` → `x + x×2` ＝ **3x**；`addq %rax, %rax` → 3x + 3x ＝ **6x**。
> - `mul7`：`leaq 0(,%rdi,8), %rax`（省略 `B`）→ `x×8` ＝ **8x**；`subq %rdi, %rax` → 8x − x ＝ **7x**。
> - gcc 的湊法：一個 `lea` 只做得出 1、2、3、4、5、8、9 倍（見 1-1 參考答案）。6 和 7 不在裡面，就拆成「先用 `lea` 做出 3 倍或 8 倍，再補一個 `add`／`sub`」，兩個便宜的指令仍然比一個乘法指令划算。

---

## 2. 不同型別的陣列存取

寫 `elem.c`：

```c
int   get_short(short *a, long i)         { return a[i]; }
int   get_uchar(unsigned char *a, long i) { return a[i]; }
long  get_long2(long *a, long i)          { return a[i + 2]; }
long *next_long(long *a, long i)          { return &a[i + 1]; }
```

### 2-1. 編譯前先預測：每個函式的本體（`endbr64` 和 `ret` 以外）各是哪一個指令？寫出完整的一行，包括：用 `mov` 系列還是 `lea`、大小後綴或擴展指令的名字、記憶體運算元的 `D`、`B`、`I`、`S` 各是什麼。【筆記[第二節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#二暫存器cpu-裡的-16-個變數)〈寫低位的名字時，高位會怎樣？〉的擴展指令表、[第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)、[第四節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#四mov-與-lea搬資料-vs-算位址)】

**第 1 次（2026-10-08）**
```
get_short:
movw %rsi(%di) %eax

get_uchar:
movb %rsi(%dil) %eax

get_long2:
movq  $2(%rdi, %rsi, $1) %rax

next_long:
leaq  $1(%rdi, %rsi, $1) $rax
```

> **批改**：**錯**（預測題）。四行都跟實際輸出不同。預測錯沒關係，這題**不重答**；錯在哪裡，請寫在下面 2-2 的重答框（那才是這題的練習重點）。參考答案在 2-2 批改後一起補。
>
> **參考答案**：見 2-2 第 2 次的參考答案（實際的四行與第 1 次預測錯在哪）。

### 2-2. 用 `-O2` 編譯，貼上輸出，跟預測對照。

**第 1 次（2026-10-08）**
```
get_short:
	endbr64
	movswl	(%rdi,%rsi,2), %eax
	ret
	.size	get_short, .-get_short
	.p2align 4
	.globl	get_uchar
	.type	get_uchar, @function
get_uchar:
	endbr64
	movzbl	(%rdi,%rsi), %eax
	ret
	.size	get_uchar, .-get_uchar
	.p2align 4
	.globl	get_long2
	.type	get_long2, @function
get_long2:
	endbr64
	movq	16(%rdi,%rsi,8), %rax
	ret
	.size	get_long2, .-get_long2
	.p2align 4
	.globl	next_long
	.type	next_long, @function
next_long:
	endbr64
	leaq	8(%rdi,%rsi,8), %rax
	ret
```

> **批改**：**不完整**。輸出貼對了，但題目要的「跟預測對照」沒寫。
>
> **提示**：一行一行對照你在 2-1 的預測，至少檢查這五個地方：
> 1. **指令名字**：你寫 `movw`／`movb`，實際是什麼？回[筆記第二節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#二暫存器cpu-裡的-16-個變數)〈寫低位的名字時，高位會怎樣？〉的擴展指令表：元素比回傳的 `int` 小時，為什麼不能只搬那 1、2 byte？
> 2. **`B` 放哪個暫存器**：你寫 `(%di)`、`(%dil)`。`B` 放的是指標 `a`，指標是幾 byte？所以要用哪個名字？（[第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)）
> 3. **`I` 和 `S`**：`I` 是誰？`S` 跟元素的型別有什麼關係？
> 4. **`D`**：`get_long2`、`next_long` 的 `D` 你寫 `$2`、`$1`，實際是多少（2-4 你已經答對了）？
> 5. **`$`**：記憶體運算元裡能不能出現 `$`（見 1-1 的批改）？還有運算元之間要用逗號隔開。

**第 2 次（重答，2026-10-08）**

> 題目：用 `-O2` 編譯，貼上輸出，跟預測對照。

```
int   get_short(short *a, long i)         { return a[i]; }
movw %rsi(%di) %eax
movswl	(%rdi,%rsi,2), %eax

我的預測錯誤了,首先我應該用rdi,因爲指標大小是8byte, 記憶體位址我也忽略了 long i 需要乘 2(byte)
其次這是有號的然後形態是short to int
用movswl, w是2byte short, l是4byte int


int   get_uchar(unsigned char *a, long i) { return a[i]; }
movb %rsi(%dil) %eax
movzbl	(%rdi,%rsi), %eax

rdi也是與前面同樣的問題,指標是8byte
movb 也漏掉了這是unsigned類型轉 signed int,故高位補0 用movzbl, b是char 1byte, l是int 4byte
但記憶體位址我覺得如果寫成%rsi(%rdi)好像也沒有問題

long  get_long2(long *a, long i)          { return a[i + 2]; }
movq  $2(%rdi, %rsi, $1) %rax
movq	16(%rdi,%rsi,8), %rax

不應該是,1, 2 應該改換成類型的byte大小, 是long類型,所以應該分別是8byte, 16byte

long *next_long(long *a, long i)          { return &a[i + 1]; }
leaq  $1(%rdi, %rsi, $1) $rax
leaq	8(%rdi,%rsi,8), %rax

一樣的問題,換成long 8byte就好
```

> **批改（第 2 次）**：**大致對**。四行的錯都找到了：`B` 要用 64 位元的 `%rdi`（指標是 8 byte），`S` 要乘元素大小（`short` 2、`long` 8），`D` 要換成 byte（16、8），擴展指令要用 `movswl`／`movzbl`，名字的兩個字母依序是來源大小和目的大小。
> 有一個新的誤會：「寫成 `%rsi(%rdi)` 好像也沒問題」**不對**。`D` 只能是**常數**，不能是暫存器；要放暫存器只能放進括號裡當 `I`。實測 `as` 會拒絕：`Error: junk '(%rdi)' after register`。另外 `get_uchar` 是「無號 `unsigned char` 轉 `int`」，你寫成了「`unsigned` 類型轉 signed int」，意思是對的。
>
> **參考答案**【[筆記第二節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#二暫存器cpu-裡的-16-個變數)擴展指令表、[第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)】實際是 `movswl (%rdi,%rsi,2), %eax`、`movzbl (%rdi,%rsi), %eax`（`S` = 1 可以省略）、`movq 16(%rdi,%rsi,8), %rax`、`leaq 8(%rdi,%rsi,8), %rax`。第 1 次預測錯在：① 指令名字：元素比 `int` 小，要用擴展指令（`movs`／`movz`＋來源大小＋目的大小），不能只用 `movw`／`movb` 搬 1、2 byte；② `B` 放指標 `a`，要用 64 位元的 `%rdi`；③ `I` 是 `%rsi`（`i`），`S` 是元素大小 2、1、8、8；④ `D` 是 byte 數 2×8 ＝ 16、1×8 ＝ 8；⑤ 記憶體運算元裡不加 `$`，運算元之間要用逗號隔開。

### 2-3. `get_short` 和 `get_uchar` 都回傳 `int`，為什麼一個用 `movs` 開頭的指令、一個用 `movz` 開頭的？如果陣列裡那個元素的位元是 `0xff`（`get_uchar`）或 `0xffff`（`get_short`），兩個函式各回傳多少？【筆記[第二節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#二暫存器cpu-裡的-16-個變數)；[第 2 課筆記第六節](../../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#六有號與無號之間的轉換位元不變換個讀法)〈**變寬（擴展，extension）**〉】

**第 1 次（2026-10-08）**
```
因爲get_short 是signed 轉sigend所以要用movs,而get_uchar a是無號的轉成int以後高位補0所以用movz

get_uchar: 255
get_short: 65535

```

> **批改**：**不完整**。
> - 第一問**方向對**：`short` 是有號，用 `movs`（符號擴展）；`unsigned char` 是無號，用 `movz`（零擴展）。補一句更精確的說法：決定補什麼的是**來源**（元素）型別的有號無號，不是目的地。
> - `get_uchar` 回傳 **255，對**。
> - `get_short` 回傳 65535，**錯**。65535 是把 `0xffff` 當無號讀、高位補 0 的結果，但 `get_short` 用的不是 `movz`。
>
> **提示**：`movswl` 的 `s` 代表哪一種擴展？`0xffff` 的最高位元（第 15 位元）是 0 還是 1？照[筆記第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈`%dil` 是 `0xff`，為什麼 `%eax` 變成 `0xffffffff`〉的道理（那裡是 1 byte 看第 7 位元，這裡是 2 byte 看第 15 位元），補完之後 32 位元的十六進位是什麼？當 `int` 讀是多少？

**第 2 次（重答，2026-10-08）**

> 題目：`get_short` 和 `get_uchar` 都回傳 `int`，為什麼一個用 `movs` 開頭的指令、一個用 `movz` 開頭的？如果陣列裡那個元素的位元是 `0xff`（`get_uchar`）或 `0xffff`（`get_short`），兩個函式各回傳多少？

```
int   get_short(short *a, long i)         { return a[i]; }
movswl	(%rdi,%rsi,2), %eax

int   get_uchar(unsigned char *a, long i) { return a[i]; }
movzbl	(%rdi,%rsi), %eax

get_short 來源類型是short, 目標類型是int, 有號,2byte用sw,
get_uchar 來源是無號,char, 所以用zb

0xff -> 255
0xffff -> 最高位是1, 故 -> 0xffffffff, -1
```

> **批改（第 2 次）**：**對**。`0xffff` 的最高位元（第 15 位元）是 1，`movswl` 照它補 1 → `0xffffffff` ＝ **−1**。決定用 `movs` 還是 `movz` 的是**來源**型別的有號無號，這次也說清楚了。
>
> **參考答案**【[筆記第二節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#二暫存器cpu-裡的-16-個變數)；第 2 課第六節〈變寬〉】`short` 是有號，要保住正負號，用符號擴展 `movswl`（照第 15 位元補高位）；`unsigned char` 是無號，用零擴展 `movzbl`（高位補 0）。`0xff` → `get_uchar` 回傳 **255**；`0xffff` → `get_short` 回傳 **−1**（`0xffffffff`）。

### 2-4. `get_long2` 和 `next_long` 裡的 `D`（位移）各是多少？為什麼是這個數，而不是 C 原始碼裡的 `2` 和 `1`？【筆記[第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)；[第 3 課筆記第二節](../../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#二指標運算p--1-走一個元素不是一個-byte)】

**第 1 次（2026-10-08）**
```
get_long2: 16byte
next_long: 8byte

因爲兩個的a都是long指標
```

> **批改**：**大致對**。`D` = 16、8 都對，「因為 `a` 是 `long` 指標」也抓到方向，但少了中間那一步：**為什麼 `long` 指標就會變成 16 和 8**。
>
> **參考答案**【[筆記第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)；[第 3 課筆記第二節](../../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#二指標運算p--1-走一個元素不是一個-byte)】組合語言的位址以 **byte** 為單位、沒有型別。C 的 `a[i + 2]` 是「從 `a` 往後 `i + 2` 個 `long`」，gcc 要換算成 byte：`a + (i + 2)×8` = `a + i×8 + 16`，所以 `S` = 8、`D` = 2 × `sizeof(long)` = **16**。`&a[i + 1]` 同理：`D` = 1 × 8 = **8**。C 寫的 2 和 1 是「幾個元素」，組合語言的 `D` 是「幾個 byte」。

---

## 3. 有號與無號的分支

寫 `pick.c`，兩個函式只差在參數型別：

```c
int pick_s(int a, int b)           { if (a > b) return 1; return 2; }
int pick_u(unsigned a, unsigned b) { if (a > b) return 1; return 2; }
```

這題用 **`-O0`** 編譯：`gcc -S -O0 -fno-asynchronous-unwind-tables pick.c -o pick.s`（`-O2` 會把 `if` 改寫掉，看不到跳躍）。

### 3-1. 貼上兩個函式的本體。找出各自的 `cmp` 和條件跳躍那兩行，寫出：`cmp` 算的是「誰減誰」（用 `a`、`b` 表示），條件跳躍的條件碼是什麼、它在什麼情況下跳。【筆記[第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈`cmp` 與 `test`〉與條件碼表、[第六節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#六逐行讀-sum_to-的--o0-版)】

**第 1 次（2026-10-08）**
```
pick_s:
	endbr64
	pushq	%rbp
	movq	%rsp, %rbp
	movl	%edi, -4(%rbp)   # int a
	movl	%esi, -8(%rbp)   # int b
	movl	-4(%rbp), %eax   # a的值寫入ax
	cmpl	-8(%rbp), %eax   # $eax - (-8(%rbp)) => a - b (cmp` 算的是「誰減誰」)
	jle	.L2					 # if SF != OF || ZF = 1(a <= b) 跳 L2 (條件跳躍的條件碼是什麼、它在什麼情況下跳)
	movl	$1, %eax
	jmp	.L3
.L2:
	movl	$2, %eax
.L3:
	popq	%rbp
	ret
	.size	pick_s, .-pick_s
	.globl	pick_u
	.type	pick_u, @function


pick_u:
	endbr64
	pushq	%rbp
	movq	%rsp, %rbp
	movl	%edi, -4(%rbp)   # unsigned a
	movl	%esi, -8(%rbp)   # unsigned b
	movl	-4(%rbp), %eax   # eax = a
	cmpl	%eax, -8(%rbp)   # $(-8(%rbp)) - %eax => b - a (cmp` 算的是「誰減誰」)
	jnb	.L5					 # ZF = 1 OR CF = 0 (條件跳躍的條件碼是什麼、它在什麼情況下跳)
	movl	$1, %eax
	jmp	.L6
.L5:
	movl	$2, %eax
.L6:
	popq	%rbp
	ret
	.size	pick_u, .-pick_u
	.ident	"GCC: (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
```

> **批改**：**不完整**。
> - `pick_s` **全對**：`cmpl -8(%rbp), %eax` 算 `a − b`；`jle` 在 ZF = 1 或 SF ≠ OF 時跳，也就是 `a <= b`（有號）時跳到 `.L2`。
> - `pick_u`：`cmpl %eax, -8(%rbp)` 算 **`b − a`**，對；條件碼 `nb`，對。但「ZF = 1 OR CF = 0」**錯**：多了 ZF。
>
> **提示**：回[筆記第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈條件碼：全部 16 種〉找 `nb` 那一列：它有哪些別名、看哪幾個旗標？再想一下：`b == a` 時算 `b − a`，需不需要往外借位？CF 是多少？光看 CF 是不是就已經把「相等」包含進去了？重答時把 `pick_u` 的部分重寫一次（`pick_s` 不用重寫）。

**第 2 次（重答，2026-10-08）**

> 題目：貼上兩個函式的本體。找出各自的 `cmp` 和條件跳躍那兩行，寫出：`cmp` 算的是「誰減誰」（用 `a`、`b` 表示），條件跳躍的條件碼是什麼、它在什麼情況下跳。

```
我修正這邊的解釋: jnb	.L5					 # ZF = 1 OR CF = 0 (條件跳躍的條件碼是什麼、它在什麼情況下跳)
應該是CF=0就可以了,就已經代表b >= a了 因爲b == a 也符合CF=0
```

> **批改（第 2 次）**：**對**。`jnb` 只看 CF = 0；`b == a` 時 `b − a` 不需要借位，CF 本來就是 0，所以「相等」已經包含在裡面，不必另外看 ZF。
>
> **參考答案**【[筆記第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈`cmp` 與 `test`〉、條件碼表】`pick_s`：`cmpl -8(%rbp), %eax` 算 `a − b`；`jle .L2` 在 ZF = 1 或 SF ≠ OF 時跳，也就是 `a <= b`（有號）時跳到 `return 2`。`pick_u`：`cmpl %eax, -8(%rbp)` 算 `b − a`；`jnb .L5`（＝`jae`）在 CF = 0 時跳，也就是 `b >= a`（無號）時跳到 `return 2`。

### 3-2. 兩個函式的條件跳躍用了不同的條件碼。為什麼？如果 `a` 的位元是 `0xffffffff`、`b` 是 1，兩個函式各回傳什麼？【筆記[第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈實驗：同一個 `cmp`、同一組旗標，`setl` 說是、`setb` 說否〉】

**第 1 次（2026-10-08）**
```
pick_s是有號比較, 另外他cmp S, D S是b D是a, a-b <= 0 就跳L2 所以有號找小於等於用le
pick_u 是無號, 他的S 是a, D是b, b-a >= 0 就跳L5 所以無號找大於等於用nb

pick_s return 2
pick_u return 1

```

> **批改**：**大致對**。兩個回傳值都對：`pick_s` 回傳 **2**（`a` 當 `int` 讀是 −1，−1 > 1 不成立），`pick_u` 回傳 **1**（`a` 當 `unsigned` 讀是 4294967295，大於 1）。「一個有號、一個無號」也是對的核心。
> 缺的是旗標這一層的「為什麼」：你的說明主要在講兩邊的運算元順序，但順序不同**不是**條件碼不同的原因（`pick_u` 也可以寫成 `a − b` 再用 `be`）。
>
> **參考答案**【[筆記第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈實驗：同一個 `cmp`、同一組旗標，`setl` 說是、`setb` 說否〉】`cmp` 只做一次減法，把有號和無號兩種解讀需要的旗標（ZF、SF、OF、CF）一次全部設好，它不知道資料是有號還是無號。有號的條件碼（`l`、`le`、`g`、`ge`）讀 SF、OF，無號的（`b`、`be`、`a`、`ae`）讀 CF。gcc 依照 C 的型別挑條件碼：`int` 用 `le`，`unsigned` 用 `nb`（＝`ae`）。
> 套到 `a` = `0xffffffff`、`b` = 1：`pick_s` 算 `a − b` ＝ −1 − 1 ＝ −2，SF = 1、OF = 0 → SF ≠ OF，`jle` 跳 → 回傳 **2**；`pick_u` 算 `b − a` ＝ 1 − 4294967295，要往外借位，CF = 1 → `jnb` 不跳 → 回傳 **1**。

### 3-3. C 寫的是 `a > b`，但 `pick_s` 的條件跳躍檢查的卻是「小於或等於」。為什麼 gcc 要用**相反**的條件？跳過去的地方是 `return 1` 還是 `return 2`？【新觀念，先猜】

**第 1 次（2026-10-08）**
```
猜不出來

return 2
```

> **批改**：**對**（後半）。跳過去的 `.L2` 是 `return 2`，對。前半「為什麼用相反的條件」是**新觀念**，猜不出來不算錯，已補教並寫進筆記：[第六節〈`if` 怎麼翻：條件跳躍檢查的是 C 條件的「反面」〉](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#if-怎麼翻條件跳躍檢查的是-c-條件的反面)。
>
> **參考答案**【補教，[第六節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#if-怎麼翻條件跳躍檢查的是-c-條件的反面)〈`if` 怎麼翻〉】條件跳躍只有「跳」和「不跳（往下一行）」兩條路。`-O0` 照原始碼順序把 `if` 的本體（`return 1`）放在條件跳躍的正下方：條件成立時要執行本體，所以**不跳**、直接往下；條件不成立時才要**跳過**本體。所以跳躍檢查的是「什麼時候該跳過」＝ C 條件的反面 `!(a > b)`，也就是 `a <= b` → `jle .L2`，跳過去的 `.L2` 是 `return 2`。對照迴圈：往回跳是「再跑一次」，用原本的條件；往前跳是「跳過」，用反面的條件。

### 3-4. `pick_u` 的 `cmp` 兩個運算元的順序跟 `pick_s` 不一樣，條件碼也不是 `be`。把 `pick_u` 的「`cmp` ＋ 條件跳躍」翻成一句 C 的條件（用 `a`、`b`），說明它跟 `!(a > b)` 是同一件事。【筆記[第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)（AT&T 的順序、條件碼表裡的別名）】

**第 1 次（2026-10-08）**
```
用c翻譯: b - a >= 0

證明是同一件事: !(a>b) => !(a - b > 0) => a - b <= 0 => b - a >= 0
```

> **批改**：**不完整**。「`cmp` 算 `b − a`」對，推導的方向（把 `!(a > b)` 改寫成 `a <= b`）也對，但最後翻成的 C 條件 `b - a >= 0` 有問題：
> - 在 C 裡 `a`、`b` 都是 `unsigned`，`b - a` 也是 `unsigned`。**無號數 `>= 0` 什麼時候會是假的？**所以 `b - a >= 0` 跟 `jnb` 不是同一件事。
> - `jnb` 看的也不是「相減的結果是正是負」。
>
> **提示**：回[筆記第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)：`nb` 看的是哪一個旗標（3-1 的提示）？那個旗標在算 `b − a` 時代表什麼事（〈上面③：3 − 5 在 CPU 裡實際上是怎麼算的？〉）？什麼時候它是 0？用 `a`、`b` **直接比大小**寫出 C 條件，不要寫成減法；再說明它為什麼等於 `!(a > b)`。順便查條件碼表：`nb` 的別名是什麼，讀法是什麼意思？

**第 2 次（重答，2026-10-08）**

> 題目：`pick_u` 的 `cmp` 兩個運算元的順序跟 `pick_s` 不一樣，條件碼也不是 `be`。把 `pick_u` 的「`cmp` ＋ 條件跳躍」翻成一句 C 的條件（用 `a`、`b`），說明它跟 `!(a > b)` 是同一件事。

```
nb 看CF, 判斷正確答案是否unsigned 容不下,容不下代表溢位 CF = 1

想不出
```

> **批改（第 2 次）**：**不完整**。前半對：`nb` 看 CF，CF = 1 代表無號的答案「容不下」（往外借位）。但題目要的「翻成一句 C 的條件」和「說明它等於 `!(a > b)`」沒有答出來（「想不出」）。重答只有這一次，直接附參考答案。
>
> **參考答案**【[筆記第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面③：3 − 5 在 CPU 裡實際上是怎麼算的？〉、條件碼表】`cmpl %eax, -8(%rbp)` 算 `b − a`。CF = 1 代表 `b − a` 要往外借位，也就是無號的 `b < a`；`jnb` 在 CF = 0 時跳，也就是**沒有借位 ⇔ `b >= a`**。所以這兩行翻成 C 是 **`if (b >= a) goto .L5;`**（`nb` 的別名是 `ae`，above or equal，無號的 ≥）。`b >= a` 就是 `a <= b`，也就是 `!(a > b)`：條件跳躍檢查 C 條件的反面（第六節〈`if` 怎麼翻〉），跳過 `return 1`。
> 為什麼不能寫成 `b - a >= 0`：在 C 裡 `b - a` 也是 `unsigned`，永遠 `>= 0`，那句話永遠成立；CPU 判斷「夠不夠減」看的是**有沒有借位（CF）**，不是相減結果的正負。

---

## 4. 讀一個陣列加總迴圈：`-O0` 與 `-O2`

寫 `sumarr.c`：

```c
long sum_arr(long *a, long n)
{
    long t = 0;
    for (long i = 0; i < n; i++)
        t += a[i];
    return t;
}
```

### 4-1. 用 `-O0` 編譯，貼上本體，**每一行**旁邊寫它對應哪一句 C、在做什麼（像筆記第六節的逐行表那樣，可以寫成 `#` 註解）。寫完再用 `-fverbose-asm` 編一次對答案。另外找出 `a`、`n`、`t`、`i` 各住在哪個記憶體位置。【筆記[第六節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#六逐行讀-sum_to-的--o0-版)】

**第 1 次（2026-10-08）**
```
	.file	"sumarr.c"
	.text
	.globl	sum_arr
	.type	sum_arr, @function
sum_arr:
	endbr64
	pushq	%rbp
	movq	%rsp, %rbp
	movq	%rdi, -24(%rbp)		# long *a
	movq	%rsi, -32(%rbp)     # long n
	movq	$0, -16(%rbp)		# long t = 0
	movq	$0, -8(%rbp)		# long i = 0
	jmp	.L2
.L3:
	movq	-8(%rbp), %rax   	# rax 此時是 i
	leaq	0(,%rax,8), %rdx	# rdx此時是 8*i byte(代表陣列位移)
	movq	-24(%rbp), %rax		# rax這個時候變成 指標a
	addq	%rdx, %rax			# a+8*i => a[i]
	movq	(%rax), %rax		# a[i]取值
	addq	%rax, -16(%rbp)		# t += a[i]
	addq	$1, -8(%rbp)		# i++
.L2:
	movq	-8(%rbp), %rax		
	cmpq	-32(%rbp), %rax 	# cmp n i
	jl	.L3						# i - n < 0 => i < n
	movq	-16(%rbp), %rax     return t
	popq	%rbp
	ret


記憶體位址
a: -24(%rbp)
n: -32(%rbp)
t: -16(%rbp)
i: -8(%rbp)


sum_arr:
	endbr64	
	pushq	%rbp	#
	movq	%rsp, %rbp	#,
	movq	%rdi, -24(%rbp)	# a, a
	movq	%rsi, -32(%rbp)	# n, n
# sumarr.c:3:     long t = 0;
	movq	$0, -16(%rbp)	#, t
# sumarr.c:4:     for (long i = 0; i < n; i++)
	movq	$0, -8(%rbp)	#, i
# sumarr.c:4:     for (long i = 0; i < n; i++)
	jmp	.L2	#
.L3:
# sumarr.c:5:         t += a[i];
	movq	-8(%rbp), %rax	# i, i.0_1
	leaq	0(,%rax,8), %rdx	#, _2
	movq	-24(%rbp), %rax	# a, tmp88
	addq	%rdx, %rax	# _2, _3
	movq	(%rax), %rax	# *_3, _4
# sumarr.c:5:         t += a[i];
	addq	%rax, -16(%rbp)	# _4, t
# sumarr.c:4:     for (long i = 0; i < n; i++)
	addq	$1, -8(%rbp)	#, i
.L2:
# sumarr.c:4:     for (long i = 0; i < n; i++)
	movq	-8(%rbp), %rax	# i, tmp89
	cmpq	-32(%rbp), %rax	# n, tmp89
	jl	.L3	#,
# sumarr.c:6:     return t;
	movq	-16(%rbp), %rax	# t, _10
# sumarr.c:7: }
	popq	%rbp	#
	ret	



```

> **批改**：**大致對**。`a`、`n`、`t`、`i` 的位置全對（`-24`、`-32`、`-16`、`-8(%rbp)`），迴圈的條件 `i − n < 0` → `i < n` 也對。兩個地方要修：
> - `addq %rdx, %rax` 的註解寫「`a+8*i => a[i]`」不精確：這時 `%rax` 是 `a + 8×i`，是 **`&a[i]`（位址）**，還不是 `a[i]`；下一行 `movq (%rax), %rax` 的括號才是去記憶體把 `a[i]` 讀出來（[筆記第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈`%rdi` 和 `(%rdi)` 差在哪？括號就是 C 的 `*`〉）。
> - 題目要求**每一行**都寫，`jmp .L2`、`.L2:` 第一行的 `movq -8(%rbp), %rax`、還有開頭的 `endbr64`、`pushq`、`movq %rsp, %rbp` 沒有註解。
>
> **參考答案**【[筆記第六節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#六逐行讀-sum_to-的--o0-版)】`movq %rdi, -24(%rbp)`／`movq %rsi, -32(%rbp)`：參數 `a`、`n` 存進記憶體；`movq $0, -16(%rbp)`：`t = 0`；`movq $0, -8(%rbp)`：`i = 0`；`jmp .L2`：`for` 一開始先跳到底部檢查條件。`.L3`：讀 `i` → `leaq 0(,%rax,8), %rdx` 算 `i×8`（byte 位移）→ 讀 `a` → `addq` 得到 `&a[i]` → `movq (%rax), %rax` 讀出 `a[i]` → `addq %rax, -16(%rbp)`：`t += a[i]` → `addq $1, -8(%rbp)`：`i++`。`.L2`：讀 `i`，`cmpq -32(%rbp), %rax` 算 `i − n`，`jl .L3`：`i < n`（有號）就回去再跑一圈。最後 `movq -16(%rbp), %rax`：`return t`。位置：`a` 在 `-24(%rbp)`、`n` 在 `-32(%rbp)`、`t` 在 `-16(%rbp)`、`i` 在 `-8(%rbp)`。

### 4-2. `-O0` 版裡，`a[i]` 用了好幾個指令才讀到。筆記第四節的 `get` 只用一個 `movl (%rdi,%rsi,4), %eax`，為什麼 `-O0` 不這樣寫？（提示：先看 `a` 和 `i` 在 `-O0` 時住在哪裡。）【筆記[第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)（`B`、`I` 只能是什麼、一個指令最多幾個記憶體運算元）、[第六節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#六逐行讀-sum_to-的--o0-版)】

**第 1 次（2026-10-08）**
```
因爲本題,a,i是放在記憶體位址,get的a,i是放在寄存器, 而B,I只能是寄存器
```

> **批改**：**對**。抓到重點：`-O0` 的 `a`、`i` 住在記憶體，而 `B`、`I` 只能是暫存器。
>
> **參考答案**【[筆記第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)、[第六節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#六逐行讀-sum_to-的--o0-版)】`movq (%rdi,%rsi,8), %rax` 這種寫法要求 `a` 和 `i` 已經在暫存器裡（`B`、`I` 只能放暫存器），而且一個指令最多只能有一個記憶體運算元。`-O0` 把每個變數都放在記憶體（`a` 在 `-24(%rbp)`、`i` 在 `-8(%rbp)`），所以必須先用 `movq` 把它們各自讀進暫存器，再算位址、再讀 `a[i]`，就變成好幾個指令。`get` 是 `-O2`，參數一直留在 `%rdi`、`%rsi`，一個指令就夠。

### 4-3. 用 `-O2` 編譯，貼上本體。像筆記第七節〈先找出「誰是誰」〉那樣列表：每個用到的暫存器代表什麼、你怎麼看出來的。【筆記[第七節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#七逐行讀-sum_to-的--o2-版)】

**第 1 次（2026-10-08）**
```
sum_arr:
	endbr64
	testq	%rsi, %rsi
	jle	.L4
	leaq	(%rdi,%rsi,8), %rdx
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
.L3:
	addq	(%rdi), %rax
	addq	$8, %rdi
	cmpq	%rdx, %rdi
	jne	.L3
	ret
	.p2align 4,,10
	.p2align 3
.L4:
	xorl	%eax, %eax
	ret

n: %rsi, 他是第二個參數
a: %rdi 第一個參數
t: %rax t是return的值 而且 addq	(%rdi), %rax => t += a[i]

不用i 理由見4-4
```

> **批改**：**大致對**。`n` = `%rsi`、`t` = `%rax` 都對，理由也對（第 2 個參數；`addq (%rdi), %rax` 是 `t +=`，最後當回傳值）。缺的：
> - `%rdx` 沒列進表（你在 4-4 寫了它是 `a[n]` 的位址，但這題要列出**每個**用到的暫存器）。
> - `%rdi` 不只是 `a`：每圈 `addq $8, %rdi`，它是一個一直往前走的指標，第 k 圈指向 `a[k]`。
> - 開頭的 `testq %rsi, %rsi`／`jle .L4` 沒說明。
>
> **參考答案**【[筆記第七節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#七逐行讀-sum_to-的--o2-版)〈先找出「誰是誰」〉】
>
> | 暫存器 | 代表 | 怎麼看出來的 |
> |---|---|---|
> | `%rdi` | 一開始是 `a`；之後是**目前元素的位址** `&a[i]` | 第 1 個參數；每圈 `addq $8, %rdi`（一個 `long`），`addq (%rdi), %rax` 讀它指的元素 |
> | `%rsi` | `n` | 第 2 個參數；`testq %rsi, %rsi`／`jle .L4`：`n <= 0` 就直接回傳 0 |
> | `%rdx` | `&a[n]`，迴圈的終點 | `leaq (%rdi,%rsi,8), %rdx` ＝ `a + n×8` |
> | `%rax` | `t` | `xorl %eax, %eax` 清成 0；每圈加上一個元素；最後直接 `ret` 當回傳值 |

### 4-4. `-O2` 版裡找不到 `i` 了。迴圈是靠什麼知道「該結束了」？gcc 為什麼要這樣改寫？【前半：筆記[第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)、[第七節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#七逐行讀-sum_to-的--o2-版)；後半「為什麼」：新觀念，先猜】

**第 1 次（2026-10-08）**
```
不用i
a指標的位移a[i] => addq	$8, %rdi
rdx就是leaq	(%rdi,%rsi,8), %rdx , a[n]的記憶體位址
cmpq	%rdx, %rdi 
jne	.L3
直接判斷記憶體位址到了沒,靠這個確定迴圈是否結束


避免使用記憶體位址,所以只要rdx存住陣列的最後address就可以決定迴圈是否結束
```

> **批改**：**對**（前半）。迴圈靠「`%rdi` 這個往前走的位址有沒有走到終點 `%rdx`（`&a[n]`）」判斷結束：`cmpq %rdx, %rdi`／`jne .L3`。說得完整。
> 後半「為什麼」是**新觀念**，猜錯不算錯。你猜「避免使用記憶體位址」不太對：兩個版本都要從記憶體讀 `a[i]`，被改寫掉的是 `i`，不是記憶體存取。已補教並寫進筆記：[第七節〈陣列迴圈的 `i` 為什麼不見了：歸納變數最佳化〉](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#陣列迴圈的-i-為什麼不見了歸納變數最佳化)，裡面用 `-fno-ivopts` 編出「保留 `i`」的版本對照。
>
> **參考答案**【前半：[筆記第七節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#七逐行讀-sum_to-的--o2-版)；後半：補教，[第七節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#陣列迴圈的-i-為什麼不見了歸納變數最佳化)〈陣列迴圈的 `i` 為什麼不見了〉】迴圈前先用 `leaq (%rdi,%rsi,8), %rdx` 算出終點 `&a[n]`；每圈 `%rdi += 8` 指向下一個元素，`cmpq %rdx, %rdi`／`jne` 判斷走到終點沒。等於把 C 改寫成 `for (p = a; p != a + n; p++) t += *p;`。為什麼：`i` 只拿來算位址和判斷結束，gcc 的歸納變數最佳化（`-fivopts`）把它跟 `a` 合併成一個每圈 +8 的指標——少維護一個變數、少用一個暫存器，記憶體運算元也從 `(%rdi,%rcx,8)` 變成簡單的 `(%rdi)`。這種把「每圈重算 `a + i×8`」換成「每圈加 8」的改寫叫強度削減（strength reduction）。

---

## 5. 從組合語言反推 C

下面兩段是某個 C 檔用 `gcc -S -O2 -fno-asynchronous-unwind-tables` 編出來的（指示詞已刪掉）：

```asm
mystery1:
	endbr64
	movl	(%rdi,%rsi,4), %eax
	leal	3(%rax,%rax,4), %eax
	ret
mystery2:
	endbr64
	xorl	%eax, %eax
	cmpl	%esi, %edi
	setnb	%al
	ret
```

### 5-1. 寫出 `mystery1` 的 C（包括參數和回傳值的型別）。存成 `mystery.c`，用同樣的命令編譯，直到本體跟上面**一模一樣**。貼上你的 C 和編出來的結果。【筆記[第二～四節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#二暫存器cpu-裡的-16-個變數)】

**第 1 次（2026-10-08）**
```
int mystery1(int *a, int i) {return 5*a[i] + 3;}
int mystery2(unsigned a, unsigned b) {return a < b ;}

	.file	"mystery.c"
	.text
	.p2align 4
	.globl	mystery1
	.type	mystery1, @function
mystery1:
	endbr64
	movslq	%esi, %rsi
	movl	(%rdi,%rsi,4), %eax
	leal	3(%rax,%rax,4), %eax
	ret
	.size	mystery1, .-mystery1
	.p2align 4
	.globl	mystery2
	.type	mystery2, @function
mystery2:
	endbr64
	xorl	%eax, %eax
	cmpl	%edi, %esi
	setb	%al
	ret
	.size	mystery2, .-mystery2
	.ident	"GCC: (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
```

> **批改**：**錯**。你編出來的本體比題目**多了一行** `movslq %esi, %rsi`，題目要求一模一樣，所以還沒完成。（另外：資料夾裡的 `mystery.c` 現在是 `unsigned mystery1(unsigned *a, int i)`，跟作答框裡的 `int` 版不同；兩個都會多出那一行。）`5*a[i] + 3`、`a` 指向 4 byte 元素、回傳 4 byte，這些都推對了。
>
> **提示**：`movslq` 是[筆記第二節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#二暫存器cpu-裡的-16-個變數)擴展指令表裡的哪一種、把幾 byte 變成幾 byte？gcc 為什麼一定要先做這一步，才能把 `%rsi` 當 `(%rdi,%rsi,4)` 的 `I` 用（`I` 必須是幾位元的暫存器）？題目的組合語言**沒有**這一行，代表 `i` 傳進來的時候就已經是幾 byte 了？改完重新編譯，貼上 C 和**一模一樣**的結果。

**第 2 次（重答，2026-10-08）**

> 題目：寫出 `mystery1` 的 C（包括參數和回傳值的型別）。存成 `mystery.c`，用同樣的命令編譯，直到本體跟上面**一模一樣**。貼上你的 C 和編出來的結果。

```
unsigned mystery1(unsigned *a, long i) {return 5*a[i] + 3;}
mystery1:
	endbr64
	movl	(%rdi,%rsi,4), %eax
	leal	3(%rax,%rax,4), %eax
	ret
```

> **批改（第 2 次）**：**對**。`i` 改成 `long` 之後 `movslq` 消失，本體跟題目一模一樣（本機重新編譯 `mystery.c` 確認過）。`unsigned` 換成 `int` 也會得到一模一樣的結果，這正是 5-3 要問的。
>
> **參考答案**【[筆記第二～四節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#二暫存器cpu-裡的-16-個變數)】`int mystery1(int *a, long i) { return 5 * a[i] + 3; }`（`int` 換成 `unsigned`、`long` 換成 `unsigned long` 也一模一樣）。`movl (%rdi,%rsi,4), %eax` 讀 `a[i]`（4 byte 元素）；`leal 3(%rax,%rax,4), %eax` 算 `x + x×4 + 3` ＝ `5x + 3`。

### 5-2. 寫出 `mystery2` 的 C，同樣編譯到一模一樣為止，貼上 C 和結果。【筆記[第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)】

**第 1 次（2026-10-08）**
```
已經寫在5-1裏面
```

> **批改**：**錯**。`a > b` 編出來是 `cmpl %edi, %esi`＋`setb`，題目是 `cmpl %esi, %edi`＋`setnb`，不一樣。你在 5-3 自己寫了「`setnb` 是無號的大於等於」，但 C 寫成了 `>`。參數是兩個 `unsigned` 這點是對的。
>
> **提示**：照 `cmp S, D` 算的是 `D − S`：題目的 `cmpl %esi, %edi` 算的是哪個參數減哪個參數？`setnb` 在這個 `D − S` 之後代表 `D` 和 `S` 什麼關係（[筆記第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)條件碼表的最後一欄）？

**第 2 次（重答，2026-10-08）**

> 題目：寫出 `mystery2` 的 C，同樣編譯到一模一樣為止，貼上 C 和結果。

```
int mystery2(unsigned a, unsigned b) {return !(a < b);}

mystery2:
	endbr64
	xorl	%eax, %eax
	cmpl	%esi, %edi
	setnb	%al
	ret
```

> **批改（第 2 次）**：**對**。`!(a < b)` 編出來跟題目一模一樣（本機重新編譯確認過）。更直接的寫法是 `a >= b`，編出來也一模一樣。
>
> **參考答案**【[筆記第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)】`int mystery2(unsigned a, unsigned b) { return a >= b; }`。`cmpl %esi, %edi` 算 `a − b`（`D − S`），`setnb` 是「沒有借位」，也就是無號的 `a >= b`，成立時 `%al` = 1。`xorl %eax, %eax` 先把整個回傳暫存器清成 0，因為 `set` 只寫最低的 1 byte。

### 5-3. 說明你是怎麼從組合語言推出 5-1、5-2 的**型別**的：每個參數、回傳值的型別，各是看哪個線索（暫存器名字、`S`、條件碼……）決定的？有沒有哪個型別是組合語言**無法**確定的？【筆記[第二節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#二暫存器cpu-裡的-16-個變數)、[第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)、[第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)】

**第 1 次（2026-10-08）**
```
5-1 
return 值用了eax,並且用了 movl 的 l 所以return 類型是4byte, 有可能是int or unsigned 都可以,我只需要隨機選一個
再來 用了movl 與 S = 4, 看起來很像指標rdi位移4byte * rsi, 並且movl是在記憶體讀寫值,寫入eax 寄存器
4byte符合int or unsigned類型大小,所以猜測第一個參數是int, unsigned選一個, 第二個參數我沒法確定,但因爲他使用rsi所以 我認爲只要低於64元位應該都行


5-2 
第一與第二個參數都用了 edi, esi 先猜測4byte
另外條件判斷用了 setnb, nb是無號 的 大於等於判斷, 所以連個參數我都用unsigned

回傳值 用了 setnb %al, al是1byte, 而且set是條件達成寫入0x1, 沒有寫入0x0, 故大於1byte都可以,隨便就選了int

float或double,我猜測的

```

> **批改**：**不完整**。推對的部分：`mystery1` 回傳 `%eax`＋`l` → 4 byte，`int` 或 `unsigned` 都行；`a` 的元素看 `S` = 4 和 `movl` → 4 byte；`mystery2` 兩個參數用 `%edi`、`%esi` → 4 byte，`setnb` 是無號的條件碼 → `unsigned`。這幾點都對。有問題的：
> - `mystery1` 的 `i`：「沒法確定，低於 64 位元都行」**錯**。這個型別組合語言其實看得出來（見 5-1 的提示）。
> - `mystery2` 的回傳值：「`%al` 是 1 byte，所以大於 1 byte 都可以」方向接近，但理由沒有用到最關鍵的那一行。
> - 「float 或 double」：浮點數本課沒教（它們用另一組暫存器，之後的課會講），不算錯，但這不是題目要的答案。
>
> **提示**：
> 1. `i`：同 5-1 的提示，有沒有擴展指令就是線索。
> 2. `mystery2` 的回傳值：`setnb %al` 前面為什麼要先 `xorl %eax, %eax`？它清掉的是幾 byte？（[筆記第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)條件碼表下方 `set<cc>` 那一列）如果回傳型別只有 1 byte，gcc 還需要這一行嗎？
> 3. 「無法確定」的意思是：**換成另一個型別，編出來的組合語言一模一樣**。哪些地方的組合語言根本沒有用到「有號還是無號」這個資訊（沒有擴展、沒有比較）？

**第 2 次（重答，2026-10-08）**

> 題目：說明你是怎麼從組合語言推出 5-1、5-2 的**型別**的：每個參數、回傳值的型別，各是看哪個線索（暫存器名字、`S`、條件碼……）決定的？有沒有哪個型別是組合語言**無法**確定的？

```
i 因爲用了rsi 推測是8byte 故爲long

補上回傳值之所以用int 是因爲xorl eax eax 清掉的是4byte,所以判斷是int

哪個型別無法確定請告訴我答案

```

> **批改（第 2 次）**：**不完整**。
> - `i` 是 8 byte，**結論對**，但理由「因為用了 `%rsi`」不夠：你第 1 次寫 `int i` 時也用了 `%rsi`。真正的線索是 **`%rsi` 沒有經過擴展就直接拿來當 `I`**；如果 `i` 是 4 byte，gcc 必須先用 `movslq` 把它擴展成 64 位元。
> - 回傳值「`xorl %eax, %eax` 清 4 byte，所以是 `int`」**不對**：寫 32 位元暫存器會把高 32 位元也清成 0（[筆記第二節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#二暫存器cpu-裡的-16-個變數)〈寫低位的名字時，高位會怎樣？〉），所以這一行其實清掉了整個 `%rax`。實測回傳 `long` 也編出一模一樣的結果；回傳 `_Bool` 或 `char` 時，`xorl` 那一行就**不見了**。
> - 「哪個型別無法確定」沒有答出來（你請我直接給答案），這次附上參考答案。
>
> **參考答案**【[筆記第二節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#二暫存器cpu-裡的-16-個變數)、[第三節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)、[第五節](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)】「無法確定」的意思是：換成另一個型別，編出來的組合語言一模一樣。
>
> | 位置 | 看得出來的 | 看不出來的 | 線索 |
> |---|---|---|---|
> | `mystery1` 的 `a` | 指向 **4 byte** 元素的指標 | 元素是 `int` 還是 `unsigned` | `S` = 4、`movl` 讀 4 byte、沒有擴展；後面的 `leal` 對有號和無號算出的位元一樣 |
> | `mystery1` 的 `i` | **8 byte** 整數 | `long` 還是 `unsigned long` | `%rsi` 直接當 `I`，沒有 `movslq` 擴展 |
> | `mystery1` 的回傳值 | **4 byte**（`%eax`、`leal` 的 `l`） | `int` 還是 `unsigned` | 加法和乘法的位元結果不分有號無號 |
> | `mystery2` 的 `a`、`b` | 4 byte（`%edi`、`%esi`），而且是**無號** | ——（這裡看得出來） | 有比較，`setnb` 是無號的條件碼（有號會是 `setge`） |
> | `mystery2` 的回傳值 | 至少 4 byte 的整數 | `int`、`unsigned`、`long`、`unsigned long` 都一樣 | `xorl %eax, %eax` 清掉整個 `%rax`；回傳 `_Bool`／`char` 時 gcc 不會產生這一行（實測） |
>
> 規則：**只有遇到擴展（`movs`／`movz`）或比較（條件碼）時，組合語言才看得出有號無號**；單純的搬移、加減、乘法，對有號和無號產生的位元一樣，型別就分不出來。
