# 2026-10-16 複習：interp、ELF、gcc、GNU、陣列、C資料

> 依 [複習排程](../learning/複習排程.md) 選出今天到期的 10 題。今天只複習、不上新課（第 5 課作業與測驗還在等回覆），
> 所以全部依**過期最久 → 盒子小 → 上次作答較早**挑選：10-09 到期的 3 題（interp 6、ELF 2、gcc 5）全選；
> 10-10 到期的 7 題（盒子 1 的 GNU 6、陣列 2、陣列 3、gcc 2、ELF 5，盒子 2 的 C資料 2、C資料 3）全選，剛好 10 題。
> 今天到期的共 63 題，沒選到的 53 題順延，日期不動。
>
> **怎麼寫**：闔上筆記，每題寫 1～2 句關鍵句就好。寫完說「幫我批改」。複習不重答，批改會直接附參考答案。
> 陣列 2、陣列 3 已拆成小題，每個小題一個作答框。

## 批改結果（2026-10-16）

| 題目 | 結果 | 盒子 | 下次 | 錯在哪／缺什麼 |
|---|---|---|---|---|
| interp 6 | 對 | 2 → 3 | 10-23 | 抓到：`sh` 印的、核心讀 `#!` 不受 `r` 限制、ELF 由核心映射沒有人讀檔（10-06 說成 kernel 執行程式碼，這次改正） |
| ELF 2 | 大致對 | 2 → 2 | 10-19 | 打包成 `LOAD`、寫 `NEEDED` 對；漏「先合併多個 `.o` 的同類區段」（跟 10-06 缺同一點） |
| gcc 5 | 不完整 | 2 → 1 | 10-17 | 自己的程式對；kernel 說成「無法重編」（應是不能用 `-O0`）、印到 stdout（應是 `printk`）（跟 10-06 卡在同一處） |
| gcc 2 | 大致對 | 1 → 1 | 10-17 | 記憶體 vs 暫存器對；又缺「`-O0` 照字面翻譯、固定記憶體位置」的理由，gdb 缺行號會跳（跟 10-09 缺同一點） |
| GNU 6 | 對（補答後） | 1 → 2 | 10-19 | 前兩問對（10-09 漏的可信欄位補上）；第三問原本漏看，補答用 `readelf -V` 看 `GLIBC_` 版本需求，對 |
| 陣列 2-1 | 對 | | | 抓到：`main` 裡是整個陣列，`f` 裡是指標 |
| 陣列 2-2 | 對 | | | 抓到：長度要另外當參數傳 |
| 陣列 2-3 | 對（改判） | 1 → 2（整題） | 10-19 | 抓到：`a` 指向一個元素、`&a` 代表整個陣列，步長 4 vs 40。原判因沒寫 `int (*)[10]` 給大致對；題目沒要求寫型別，改判 |
| 陣列 3-1 | 對（改判） | | | 抓到：堆疊、`.rodata`。原判因沒講「複製」給大致對；題目只問在哪裡，改判 |
| 陣列 3-2 | 對 | | | 抓到：堆疊可寫、`.rodata` 沒有 `w` |
| 陣列 3-3 | 對（改判） | 1 → 2（整題） | 10-19 | 抓到：用 `info proc mappings` 找位址落在哪一段（權限在同一列，接 3-2 的前提）。原判因沒寫「看權限欄」給大致對，改判 |
| ELF 5 | 對 | 1 → 2 | 10-19 | 抓到：不是同一種問題、看檔頭 `Machine`（10-09 答錯，這次改正） |
| C資料 2 | 對（改判） | 2 → 3 | 10-23 | 抓到：兩個值、小端從後往前／大端從前往後讀所以一樣。原判因「`p` 是 4 byte」的用詞和沒明講「寫讀抵消」給大致對；使用者說明前者是用詞、後者是同一件事，改判 |
| C資料 3 | 錯 | 2 → 1 | 10-17 | 把轉換說成溢位；第二問答成「溢位是 UB」，應是 `-Wall` 不會提醒、要 `-Wextra`（跟 10-05 C資料 10 誤以為有號轉無號是 UB 同） |

以大題計：對 6（interp 6、ELF 5、陣列 2〔改判〕、GNU 6〔補答後〕、陣列 3〔改判〕、C資料 2〔改判〕）、大致對 2（ELF 2、gcc 2）、不完整 1（gcc 5）、錯 1（C資料 3）。
升級：interp 6、C資料 2 → 盒子 3（10-23）、ELF 5、陣列 2、GNU 6、陣列 3 → 盒子 2（10-19）；ELF 2 留盒子 2（10-19）；gcc 2 留盒子 1，gcc 5、C資料 3 回盒子 1，明天（10-17）再考。
**進步**：interp 6 不再說成 kernel 執行程式碼、ELF 5 改看 `Machine`、GNU 6 補上可信的欄位、gcc 2 不再把 `andl $1` 當計數、陣列 3-1 不再把堆疊說成 data。
**反覆卡住**：gcc 5 kernel 不能用 `-O0`（第三次）、ELF 2 的「合併」、C資料 3 把有號轉無號當成溢位／UB。

## 今天的題目

| # | 題目 | 盒子 | 原本該考的日期 | 為什麼選 |
|---|---|---|---|---|
| 1 | interp 6 | 2 | 2026-10-09 | 過期最久，排程表順序 |
| 2 | ELF 2 | 2 | 2026-10-09 | 同上 |
| 3 | gcc 5 | 2 | 2026-10-09 | 同上 |
| 4 | GNU 6 | 1 | 2026-10-10 | 次久、盒子 1 |
| 5 | 陣列 2（3 小題） | 1 | 2026-10-10 | 同上 |
| 6 | 陣列 3（3 小題） | 1 | 2026-10-10 | 同上 |
| 7 | gcc 2 | 1 | 2026-10-10 | 同上 |
| 8 | ELF 5 | 1 | 2026-10-10 | 同上 |
| 9 | C資料 2 | 2 | 2026-10-10 | 次久、盒子 2 |
| 10 | C資料 3 | 2 | 2026-10-10 | 同上 |

---

## interp：[ELF的interpreter是什麼-動態載入器與井字號驚嘆號](../c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md)

### interp 6. 情境題：一支腳本權限是 `---x--x--x`（只有執行權限），第一行是 `#!/bin/sh`，執行時出現 `/bin/sh: ./x.sh: Permission denied`。這個錯誤是誰印的？為什麼核心讀得到 `#!` 那一行，直譯器卻讀不到腳本？同樣只有執行權限的 ELF 執行檔為什麼就能跑？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 6 題](../small-project/s1-01-hello-compile/quiz.md#6-情境題一支腳本權限是----x--x--x只有執行權限第一行是-binsh執行時出現-binsh-xsh-permission-denied這個錯誤是誰印的為什麼核心讀得到--那一行直譯器卻讀不到腳本同樣只有執行權限的-elf-執行檔為什麼就能跑)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md)

```
是sh印出來的,因爲sh要讀腳本,但這個腳本只有執行權限
核心有最高權限,不受userspace的角色限制,所以他讀得到腳本
因爲核心可以讀他的elf header,然後載入interpreter,interpreter這個時候會去加載其他函式庫,然後最後從start開始,接下來執行的過程就是cpu在做,中間除了核心沒有人讀檔案,還有cpu會去一行一行執行程式碼
```

> **批改**：**對**（盒子 2 → 3）。三問都答到了：錯誤是 `sh` 印的；核心自己讀 `#!`，不受 `r` 限制，`sh` 卻要自己讀腳本；ELF「中間除了核心沒有人讀檔案」就是關鍵。10-06 說成「執行程式碼的是 kernel」，這次改成 CPU 執行，已經改正。兩個用詞提醒：
> - 「核心有最高權限」說得太大：`execve` 讀檔時只是不檢查 `r`，`x` 還是會檢查，沒有 `x` 一樣執行不了。
> - ELF 那一問不必扯到 interpreter：靜態連結的程式沒有 interpreter，一樣只要 `x` 就能跑。重點只有「程式檔由核心直接映射進記憶體」（interpreter 本身也是核心映射的）。
>
> **參考答案**【[interp 第四節](../c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md#四跟腳本的--是同一個機制)〈交給直譯器的是「路徑」，不是內容：三個實驗〉的實驗三】
>
> 核心關鍵：
> 1. 錯誤是 **`/bin/sh`** 印的（訊息開頭就是它的名字），不是核心印的。
> 2. 核心在 `execve` 裡自己讀 `#!` 那一行，只檢查 `x`、不受 `r` 限制；核心交給 `sh` 的是**腳本路徑**，`sh` 是以你的身分執行的普通行程，要自己 `open` 腳本，沒有 `r` 就被擋下。
> 3. 只有 `x` 的 ELF 能跑，是因為程式檔由**核心直接映射進記憶體**，不需要任何使用者行程打開它。
>
> 詳細說明：所以腳本要 `r`＋`x`，ELF 只要 `x`。筆記的實驗三用 `#!/bin/cat` 做同一件事：`chmod 111 catscript.sh` 之後執行，錯誤開頭是 `/usr/bin/cat:`；`cp /usr/bin/true mytrue; chmod 111 mytrue` 則可以正常跑。

---

## ELF：[ELF是什麼-可執行與可連結格式的兩種視角](../c/ELF是什麼-可執行與可連結格式的兩種視角.md)

### ELF 2. `readelf -l hello.o` 說 `There are no program headers in this file`，這代表什麼？連結器做了什麼才讓執行檔有程式標頭？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 2 題](../small-project/s1-01-hello-compile/quiz.md#2-readelf--l-helloo-說-there-are-no-program-headers-in-this-file這代表什麼連結器做了什麼才讓執行檔有程式標頭)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md)

**題目用到的實驗**（[ELF 第四節](../c/ELF是什麼-可執行與可連結格式的兩種視角.md#四名字的關鍵兩種視角)）：把 `hello.c` 只編譯不連結得到 `hello.o`，對它跑 `readelf -l`（`-l`：印程式標頭表）；再連結成執行檔，對執行檔跑同一個命令對照。

`hello.c`（檔案：[`hello.c`](../small-project/s1-01-hello-compile/hello.c)）：

```c
#include <stdio.h>

#define GREETING "hello, world"

int main(void)
{
	printf("%s\n", GREETING);
	return 0;
}
```

```
$ gcc -c hello.c          # -c：編譯＋組譯，停在 .o，不連結
$ readelf -l hello.o      # 題目裡的訊息就是這一步印的
$ gcc -o hello hello.o    # 連結成執行檔
$ readelf -l hello        # 對照
```

```
這代表這個檔案還沒有經過連接器連接,所以還沒有寫好program header

ld 他會去看section header,把同權限的element,組合在相同的載入段LOAD,他也會檢查缺了那個函式庫的程式碼,並把需要導入的函式庫名稱,寫入到dynamic section並標註函式庫needed
```

> **批改**：**大致對**（盒子 2 → 2）。抓到了：權限相同的區段打包成 `LOAD`、把需要的函式庫寫進 `.dynamic` 的 `NEEDED`。跟 10-06 缺同一點：
> - 漏了第一步：**先把多個 `.o` 的同類區段合併**（例如每個 `.o` 的 `.text` 併成執行檔裡的一個 `.text`），合併、排好位址之後才打包成載入段。
> - 前半「還沒經過連結」是事實，但題目問的是「代表什麼」：沒有程式標頭表，就是沒有**執行視角**。`.o` 只給連結器讀，核心不能載入它。
>
> 用詞：「element」應該是**區段（section）**。另外 `NEEDED` 寫在 `.dynamic` 沒錯，但在程式標頭表裡對應的是 `DYNAMIC` 這一項；動態連結時連結器還會加上 `INTERP`。
>
> **參考答案**【[ELF 第四節](../c/ELF是什麼-可執行與可連結格式的兩種視角.md#四名字的關鍵兩種視角)〈最有力的證據：`.o` 沒有執行視角〉】
>
> 核心關鍵：
> 1. 沒有程式標頭表 ＝ 沒有執行視角：`.o` 只給連結器用，核心還不能載入執行它。
> 2. 連結器先把多個 `.o` 的同類區段**合併**，再把權限相同的區段打包成載入段 `LOAD`（依分頁對齊），最後寫出程式標頭表。
>
> 詳細說明：合併之後還要補上位址（重定位）。動態連結時，程式標頭表裡另外有 `INTERP`（interpreter 的路徑）、`DYNAMIC`（指向 `.dynamic`，裡面有 `NEEDED`）等項。2026-10-16 在本機重跑：`readelf -l hello.o` 印 `There are no program headers in this file.`；`readelf -l hello` 有 `PHDR`、`INTERP`、4 個 `LOAD`、`DYNAMIC`、2 個 `NOTE`，以及 `GNU_PROPERTY`、`GNU_EH_FRAME`、`GNU_STACK`、`GNU_RELRO`。

---

## gcc：[gcc最佳化等級-O0到O3差在哪](../c/gcc最佳化等級-O0到O3差在哪.md)

### gcc 5. 情境題：你在自己的程式裡用 `-O2` 編譯，`gdb` 印某個變數時顯示 `<optimized out>`。你會怎麼處理？如果是在 kernel 裡遇到同樣的狀況，處理方式又有什麼不同？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-情境題你在自己的程式裡用--o2-編譯gdb-印某個變數時顯示-optimized-out你會怎麼處理如果是在-kernel-裡遇到同樣的狀況處理方式又有什麼不同)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md)

```
-O2編譯時,變數很有可能是存在寄存器,而寄存器可能不會一直存着他,所以我們可以使用-O0 變數會存到記憶體的stack中

在kernel的時候也無法重新編譯,可以試試看直接把參數打印到stdout
```

> **批改**：**不完整**（盒子 2 → 1）。自己的程式那一半對：用 `-O0` 重編，理由「暫存器不會一直保留它、`-O0` 放在堆疊上」也對。kernel 那一半跟 09-30、10-06 卡在同一處，這次還多了兩個錯：
> - 「kernel 無法重新編譯」**錯**：kernel 當然可以重編（改了程式碼就要重編），不能的是**用 `-O0` 編**。kernel 依賴編譯器一定會做的內嵌與刪除死碼，用 `-O0` 編不過，所以 `<optimized out>` 在 kernel 裡是常態。
> - 「印到 stdout」**錯**：stdout 是使用者行程的 fd 1，kernel 裡沒有這個東西；kernel 也不連結 libc，沒有 `printf`。要用 `printk`。
>
> **參考答案**【[gcc 第三節](../c/gcc最佳化等級-O0到O3差在哪.md#三為什麼除錯要用--o0)、[第四節](../c/gcc最佳化等級-O0到O3差在哪.md#四跟-kernel-的關係)】
>
> 核心關鍵：
> 1. 自己的程式：用 `-O0 -g`（或 `-Og -g`）重編，變數有固定位置，gdb 就印得出來；不能重編時，加 `printf` 把值印出來。
> 2. kernel：**不能用 `-O0` 編**（要靠內嵌與刪除死碼才編得過），`<optimized out>` 是常態；只能改用 `printk`、tracing 這類方法觀察，並學會讀最佳化後的組合語言。
>
> 詳細說明：一句話：自己的程式可以「改編譯方式」，kernel 只能「改觀察方式」。`printk` 的訊息寫進 kernel 的紀錄緩衝區，用 `dmesg` 看（這點是補充，筆記沒有展開）。

### gcc 2. 為什麼 `-O0` 的組合語言每一圈都在讀寫記憶體（`-4(%rbp)`），而 `-O2` 不會？這個差別對 `gdb` 有什麼影響？

> 盒子 1｜過去的作答：[s1-01 quiz.md gcc 第 2 題](../small-project/s1-01-hello-compile/quiz.md#2-為什麼--o0-的組合語言每一圈都在讀寫記憶體-4rbp而--o2-不會這個差別對-gdb-有什麼影響)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)

**題目用到的程式**：`sumto.c`（檔案：[`sumto.c`](../small-project/s1-01-hello-compile/sumto.c)），
用 `gcc -O0 -S sumto.c` 與 `gcc -O2 -S sumto.c` 產生組合語言（`-S`：只編譯到組合語言 `.s` 就停）。

```c
#include <stdio.h>

int sum_to(int n)
{
    int total = 0;
    for (int i = 1; i <= n; i++)
        total += i;
    return total;
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

`-O0` 的 `sum_to`（本機 gcc 13.3 實際輸出，刪掉 `.cfi`、`.type` 這類組譯指示，右邊是逐行說明）：

```asm
sum_to:
	endbr64                    # 間接跳躍的保護標記（CET），跟這題無關
	pushq	%rbp               # 存起呼叫者的 %rbp
	movq	%rsp, %rbp         # %rbp = 這個函式的堆疊框底
	movl	%edi, -20(%rbp)    # 參數 n（放在 %edi 傳進來）寫到記憶體 -20(%rbp)
	movl	$0, -8(%rbp)       # total = 0，寫到記憶體 -8(%rbp)
	movl	$1, -4(%rbp)       # i = 1，寫到記憶體 -4(%rbp)
	jmp	.L2                # 先跳去檢查條件
.L3:                               # 迴圈本體
	movl	-4(%rbp), %eax     # 從記憶體讀 i 到 %eax
	addl	%eax, -8(%rbp)     # 記憶體裡的 total += %eax
	addl	$1, -4(%rbp)       # 記憶體裡的 i += 1
.L2:                               # 迴圈條件
	movl	-4(%rbp), %eax     # 從記憶體讀 i
	cmpl	-20(%rbp), %eax    # 比較 i 與記憶體裡的 n（算 i − n、設旗標）
	jle	.L3                # i <= n（有號）就回去跑本體
	movl	-8(%rbp), %eax     # 從記憶體讀 total 當回傳值
	popq	%rbp               # 還原 %rbp
	ret
```

`-O2` 的 `sum_to`（同上，實際輸出）：

```asm
sum_to:
	endbr64
	testl	%edi, %edi         # n & n，只為了設旗標
	jle	.L4                # n <= 0 就跳去回傳 0
	leal	1(%rdi), %ecx      # %ecx = n + 1（迴圈的終點）
	xorl	%edx, %edx         # %edx = 0（total）
	andl	$1, %edi           # %edi = n & 1（n 是奇數還是偶數）
	movl	$1, %eax           # %eax = 1（i）
	je	.L3                # n 是偶數就直接進迴圈
	movl	$2, %eax           # n 是奇數：先處理掉 i = 1，i 從 2 開始
	movl	$1, %edx           #           total = 1
	cmpl	%ecx, %eax         #           i 已經到 n + 1 了嗎（n = 1 時）
	je	.L1
.L3:                               # 迴圈本體：一圈加兩個數
	leal	1(%rdx,%rax,2), %edx  # total = total + 2×i + 1（＝ total + i + (i+1)）
	addl	$2, %eax           # i += 2
	cmpl	%ecx, %eax         # i 到 n + 1 了嗎
	jne	.L3
.L1:
	movl	%edx, %eax         # 回傳 total
	ret
.L4:
	xorl	%edx, %edx
	movl	%edx, %eax         # 回傳 0
	ret
```

```
-O0 是把i這個變數都存入到堆棧-4(%rbp),不斷更新i的值
-O2 是把total, i, n+1 都用在寄存器

當gdb想要讀取變數的時候有可能會的到optimization out
這是因爲寄存器可能已經不是變數的值了
```

> **批改**：**大致對**（盒子 1 → 1）。進步的地方：`-O2` 把 `total`、`i`、`n + 1` 都放在暫存器，這點對；10-09 把 `andl $1` 說成計數，這次沒再犯。gdb 可能印出 `<optimized out>` 也對。跟 10-09 缺同一點：
> - 沒講**為什麼** `-O0` 每一圈都要讀寫記憶體：`-O0` 不最佳化、照字面翻譯，每個變數分到一個**固定的堆疊位置**（`i` 在 `-4(%rbp)`、`total` 在 `-8(%rbp)`），每條語句都「從記憶體讀 → 算 → 寫回」。你寫的是「它這樣做」，不是「為什麼這樣做」。
> - gdb 那一半只講了 `<optimized out>`，少了另一面：`-O0` 每一行原始碼都對得到一段指令，可以一行一行單步走；`-O2` 的指令被重排，行號會跳。
>
> **參考答案**【[gcc 第二節](../c/gcc最佳化等級-O0到O3差在哪.md#二實驗同一段程式不同等級編出什麼)、[第三節](../c/gcc最佳化等級-O0到O3差在哪.md#三為什麼除錯要用--o0)】
>
> 核心關鍵：
> 1. `-O0` 不最佳化、照字面翻譯：每個變數都有**固定的記憶體位置**（`i` 是 `-4(%rbp)`、`total` 是 `-8(%rbp)`），每條語句都從記憶體讀、再寫回記憶體。
> 2. `-O2` 把變數留在**暫存器**（`i` 在 `%eax`、`total` 在 `%edx`），迴圈裡完全不碰記憶體。
> 3. 對 gdb 的影響：`-O0` 原始碼的每一行都對得到一段指令，變數隨時印得出來；`-O2` 行號會跳、指令被重排，變數可能顯示 `<optimized out>`。
>
> 詳細說明：`-O0` 的下一條語句不假設暫存器裡還留著上一次的值，所以每一圈都重新讀寫 `-4(%rbp)`。筆記第三節的實測：`-O2` 版停下來時跳過了第 3 行；只走兩行，`total` 就已經是 3；被內嵌進 `main` 的那一份，變數全是 `<optimized out>`；中斷點顯示 `(2 locations)`。所以學習、單步除錯用 `-O0 -g`，日常開發用 `-Og -g`。

---

## GNU：[GNU是什麼-Linux核心與GNU工具的分工](../linux/GNU是什麼-Linux核心與GNU工具的分工.md)

### GNU 6. 情境題：同事說「我在這台機器上打 `uname -o` 看到 `GNU/Linux`，所以它一定用 glibc」。這個推論錯在哪？`uname` 的哪些欄位可以信、哪一欄不行，為什麼？你會改看什麼來判斷？

> 盒子 1｜過去的作答：[s1-01 quiz.md GNU 第 6 題](../small-project/s1-01-hello-compile/quiz.md#6-情境題同事說我在這台機器上打-uname--o-看到-gnulinux所以它一定用-glibc這個推論錯在哪uname-的哪些欄位可以信哪一欄不行為什麼你會改看什麼來判斷)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)

```
實際上uname -o並沒有問核心,是uname自己直接打印出GNU/Linux的,

-s,-n, -r, -v, -m都可以, 這些都是uname去請求核心問的
但是-p,-i,-o不行 -o沒有問核心, 而-p -i是因爲 這個不是公共定義,在每個平臺定義都不太一樣,所以不可信 
```

> **批改**：**大致對**（盒子 1 → 1）。前兩問對：`-o` 是 `uname` 自己印的；`-s -n -r -v -m` 是向 kernel 問來的，可以信，`-o`、`-p`、`-i` 不行。10-09 漏掉的「哪些可以信」這次補上了。但**第三問「你會改看什麼來判斷」沒有作答**，偏偏是 10-09 答對的那一問。
>
> **參考答案**【[GNU 第四節](../linux/GNU是什麼-Linux核心與GNU工具的分工.md#四不是-gnu-的-linux你已經遇過了)〈`uname -o` 印出 `GNU/Linux`，不代表它是 GNU〉】
>
> 核心關鍵：
> 1. 錯在把 `-o` 當證據：它是 `uname` 程式寫死的字串，不是問 kernel 來的（BusyBox 版也印 `GNU/Linux`，實測用 musl 的 OpenWrt 一樣印）。
> 2. `-s`、`-n`、`-r`、`-v`、`-m` 來自 `uname()` 系統呼叫（kernel 回傳的 `struct utsname`），可以信；`-o` 不行，`-p`、`-i` 也是程式自己判斷的（說明裡標 non-portable）。
> 3. 判斷 libc 要看**程式本身**：`readelf -l /bin/ls | grep interpreter`（`ld-linux-*` 是 glibc、`ld-musl-*` 是 musl），或用 `readelf -V` 看有沒有 `GLIBC_` 版本需求。
>
> 詳細說明：本機 `/bin/ls` 的 interpreter 是 `/lib64/ld-linux-x86-64.so.2`；`readelf -V /bin/ls` 列出 `File: libc.so.6`、`Name: GLIBC_2.38` 等項目。musl 不用符號版本，所以沒有 `GLIBC_` 項目。

**第三問補答（2026-10-16，使用者說明是漏看第三問、補答前沒看參考答案，所以算第一次作答的一部分，不是重答）**

````
可以直接用readelf -V 直接去找函式庫的版本,或者找到函式庫名稱:可以用readelf -d找,然後找她的路徑,可以看到gnu
/usr/lib/x86_64-linux-gnu/libc.so.6
/usr/lib/i386-linux-gnu/libc.so.6

（附上的 readelf -V 輸出）
Version symbols section '.gnu.version' contains 7 entries:
 Addr: 0x000000000000050e  Offset: 0x0000050e  Link: 6 (.dynsym)
  000:   0 (*local*)       2 (GLIBC_2.34)    1 (*global*)      3 (GLIBC_2.2.5)
  004:   1 (*global*)      1 (*global*)      3 (GLIBC_2.2.5)

Version needs section '.gnu.version_r' contains 1 entry:
 Addr: 0x0000000000000520  Offset: 0x00000520  Link: 7 (.dynstr)
  000000: Version: 1  File: libc.so.6  Cnt: 2
  0x0010:   Name: GLIBC_2.2.5  Flags: none  Version: 3
  0x0020:   Name: GLIBC_2.34  Flags: none  Version: 2
````

> **批改（第三問）**：**對**。`readelf -V` 看到 `File: libc.so.6` 底下有 `GLIBC_2.2.5`、`GLIBC_2.34` 這些版本需求，就是 glibc 的證據（musl 不用符號版本），這正是核心關鍵 3 的第二種方法。整題改成**對（補答後）**，盒子 1 → 2。
>
> 第二種方法（`readelf -d` 找到 `libc.so.6`，再看它的路徑有沒有 `gnu`）可以當旁證，但不夠可靠：
> - `readelf -d` 只給**名字**（`NEEDED  [libc.so.6]`），不給路徑。你列出的兩個路徑是自己去目錄裡找到的，不一定是程式實際載入的那一個；實際載入哪一個由動態載入器決定（`ldd` 看得到，見 objdump 那篇）。
> - 兩個路徑裡的 `i386-linux-gnu` 是 32 位元的 libc，64 位元的程式不會用它。
> - 路徑裡的 `x86_64-linux-gnu` 是目標三元組（GNU 2 教過，第三段 `gnu` 就是 C 函式庫／ABI 那一欄），Ubuntu 用它當函式庫目錄名，所以在 Ubuntu 上確實指向 glibc。但這只是發行版**擺檔案的慣例**（補充，筆記沒教：Fedora 的 glibc 放在 `/usr/lib64/libc.so.6`，路徑裡沒有 `gnu`），看不到 `gnu` 也不代表不是 glibc。
>
> 結論：看程式本身的 interpreter（`readelf -l`）或版本需求（`readelf -V`）最直接，路徑只能當旁證。

---

## 陣列：[陣列、字串與指標運算-C不檢查邊界與用gdb找segfault](../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md)

### 陣列 2. 陣列傳進函式之後

> 盒子 1｜過去的作答：[s1-03 quiz.md 第 2 題](../small-project/s1-03-arrays-pointers/quiz.md#2-1-int-a10-在-main-裡-sizeof-a-是-40傳進-void-fint-a-之後-sizeof-a-變成-8為什麼)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)

**題目用到的程式**：筆記[第三節](../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#三陣列名稱什麼時候變成指標什麼時候不會)的 `decay.c`（筆記用 `int a[5]`，題目用 `int a[10]`，道理相同）：

```c
#include <stdio.h>

void show(int a[])
{
    printf("函式裡 sizeof a = %zu\n", sizeof a);
}

int main(void)
{
    int a[5] = {0};
    int *p = a;

    printf("main 裡 sizeof a = %zu, sizeof p = %zu\n", sizeof a, sizeof p);
    show(a);
    printf("a      = %p   a + 1  = %p\n", (void *)a, (void *)(a + 1));
    printf("&a     = %p   &a + 1 = %p\n", (void *)&a, (void *)(&a + 1));
    return 0;
}
```

```
$ gcc -Wall -Wextra -g -O0 -o decay decay.c
$ ./decay
```

#### 陣列 2-1. `int a[10];` 在 `main` 裡 `sizeof a` 是 40，傳進 `void f(int a[])` 之後 `sizeof a` 變成 8。為什麼？

```
在main宣告 a是int陣列, sizeof a, 的意思是a指向的整個陣列的大小, 10個int也就是40byte
但傳入f函式後,a就只是個指標,大小是8byte
```

> **批改**：**對**。抓到了：`main` 裡量的是整個陣列，`f` 裡的 `a` 其實是指標。小用詞：在 `main` 裡 `a` 就是陣列本身，不是「`a` 指向的陣列」。
>
> **參考答案**【[第 3 課第三節](../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#三陣列名稱什麼時候變成指標什麼時候不會)〈函式參數寫成 `int a[]`，其實就是 `int *a`〉】
>
> 核心關鍵：
> 1. 在 `main` 裡，`sizeof a` 是「陣列名稱不退化」的例外，量的是整個陣列：10 × 4 = 40 byte。
> 2. 參數寫成 `int a[]` 其實就是 `int *a`，`f(a)` 傳過去的只是 `a[0]` 的位址，所以 `f` 裡的 `sizeof a` 是指標的 8 byte。
>
> 詳細說明：gcc 對這種寫法有 `-Wsizeof-array-argument` 警告（預設就開）。

#### 陣列 2-2. 這件事對「怎麼設計接收陣列的函式」有什麼影響？

> 題目：這件事（陣列傳進函式之後 `sizeof a` 從 40 變成 8）對「怎麼設計接收陣列的函式」有什麼影響？

```
因爲f只能知道a指標的大小,所以如果要知道陣列的長度,只能一樣把長度當參數傳入
```

> **批改**：**對**。
>
> **參考答案**【[第 3 課第三節](../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#三陣列名稱什麼時候變成指標什麼時候不會)〈函式參數寫成 `int a[]`，其實就是 `int *a`〉】
>
> 核心關鍵：
> 1. 函式不知道陣列多長，**長度一定要另外當參數傳**（例如 `long sum(int *p, int n)`）。
>
> 詳細說明：呼叫端用 `sizeof a / sizeof a[0]` 算長度（這時 `a` 還是陣列）；`memcpy`、`read` 也是這樣設計的。字串是例外，靠結尾的 `\0` 判斷在哪裡結束。

#### 陣列 2-3. `a` 和 `&a` 印出來的位址一樣，它們差在哪裡？

> 題目：`a` 和 `&a` 印出來的位址一樣，它們差在哪裡？（`int a[10];`）

```
a是int指標,所以直接印就是a指標指的內存位址,如果a+1 移動的是4byte跳到下一個參數

&a 雖然也是相同的內存位址,但是他是指a這個陣列開頭位址,如果做&a+1,那就是加40byte,跳到原本的陣列之外指向下一個位址
```

> **批改**：**對（改判）**。原判「大致對」，理由是沒寫出 `&a` 的型別 `int (*)[10]`。使用者指出題目只問「差在哪裡」，沒有要求寫出型別，改判。
> 判斷依據：你寫出了兩者的差別：`a` 是 `int` 指標，`+ 1` 走一個元素（4 byte）；`&a` 代表整個陣列，`+ 1` 走 40 byte、跳出原本的陣列。這就是「型別不同」的實質內容。
> `int (*)[10]` 這個寫法是細節，原本卻放進了核心關鍵，這是參考答案的問題，已經把它移到詳細說明（筆記同步）。用詞：`a + 1` 跳到的是下一個**元素**，不是「參數」。
>
> **參考答案**【[第 3 課第三節](../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#三陣列名稱什麼時候變成指標什麼時候不會)〈`a` 和 `&a`：位址一樣，型別不一樣〉】（2026-10-16 改判後修正核心關鍵）
>
> 核心關鍵：
> 1. 位址相同，但指的東西不同：`a` 退化成指向第一個元素（`int`）的指標，`&a` 是指向**整個陣列**的指標。
> 2. 所以 `a + 1` 走 4 byte（一個 `int`），`&a + 1` 走 40 byte（一整個陣列）。
>
> 詳細說明：用型別的話說，`a` 是 `int *`，`&a` 是 `int (*)[10]`。括號讓 `*` 先跟名字結合，宣告這種指標變數要寫成 `int (*p)[10] = &a;`；`int *p[10]` 沒有括號，是「10 個 `int *` 組成的陣列」。

### 陣列 3. `char s1[]` 與 `char *s2`

> 盒子 1｜過去的作答：[s1-03 quiz.md 第 3 題](../small-project/s1-03-arrays-pointers/quiz.md#3-1-char-s1--hi-和-char-s2--hi-的-hi-分別在記憶體的哪裡)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)

**題目用到的程式**：`strings.c`（檔案：[`strings.c`](../small-project/s1-03-arrays-pointers/strings.c)，筆記[第四節](../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#四字串以-0-結尾的-char-陣列)）：

```c
#include <stdio.h>
#include <string.h>

int main(void)
{
    char s1[] = "hi";       /* 陣列：在堆疊上有自己的 3 個 byte */
    char *s2 = "hi";        /* 指標：指向字串常數 */

    printf("sizeof s1 = %zu, strlen(s1) = %zu\n", sizeof s1, strlen(s1));
    printf("sizeof s2 = %zu, strlen(s2) = %zu\n", sizeof s2, strlen(s2));
    printf("s1 在 %p\n", (void *)s1);
    printf("s2 指向 %p\n", (void *)s2);

    s1[0] = 'H';
    printf("改完 s1 = %s\n", s1);

    s2[0] = 'H';            /* 第 17 行：寫進唯讀記憶體 */
    printf("改完 s2 = %s\n", s2);
    return 0;
}
```

```
$ gcc -Wall -g -O0 -o strings strings.c
$ ./strings
$ gdb ./strings          # 3-3 用
```

#### 陣列 3-1. `char s1[] = "hi";` 和 `char *s2 = "hi";` 的 `"hi"` 分別在記憶體的哪裡？

```
s1是陣列,s1的是存在堆棧上的

s2是指標,這個指標是指向.rodata的某一個位址
```

> **批改**：**對（改判）**。`s1` 的 `"hi"` 在堆疊、`s2` 指向的在 `.rodata`，兩個位置都答對了（10-09 把堆疊說成 data，這次沒再犯）。原判「大致對」，理由是沒講 `s1` 是**複製**進去的；使用者指出題目只問「在記憶體的哪裡」，改判。「複製」解釋的是**為什麼** `s1` 的那一份在堆疊上、可以改，3-2 才用得到，不是這一問的必要點，已經從核心關鍵移到詳細說明（筆記同步）。
>
> **參考答案**【[第 3 課第四節](../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#四字串以-0-結尾的-char-陣列)】
>
> 核心關鍵：
> 1. `char s1[] = "hi";`：`"hi"` 在**堆疊**上（`s1` 是一個 3 byte 的區域陣列）。
> 2. `char *s2 = "hi";`：`"hi"` 是字串常數，在 **`.rodata`**；`s2` 只是指向它的指標。
>
> 詳細說明：`s1` 的 3 個 byte（`h`、`i`、`\0`）是**複製**進陣列的，是 `s1` 自己的一份，所以能改；`s2` 本身是一個 8 byte 的指標變數（它自己在堆疊上），指向唯讀的那一份。（2026-10-16 改判後，「複製」從核心關鍵移到這裡）

#### 陣列 3-2. 為什麼改 `s1[0]` 沒事，改 `s2[0]` 卻 segfault？

> 題目：為什麼改 `s1[0]` 沒事，改 `s2[0]` 卻 segfault？

```
s1[0]是存在堆棧上,是由w權限的
但是s2[0]是存在.rodata上的,他是沒有w權限的
```

> **批改**：**對**。抓到了：堆疊那一頁可寫，`.rodata` 那一頁沒有 `w`。
>
> **參考答案**【[第 3 課第四節](../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#四字串以-0-結尾的-char-陣列)、[第五節](../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#五c-不檢查邊界越界的三種下場)開頭的表格】
>
> 核心關鍵：
> 1. `s1` 的副本在堆疊上，那一頁是 `rw-p`，可以寫。
> 2. `s2` 指向 `.rodata`，分頁權限是 `r--p`：**有對映**但**不允許寫**，CPU 拒絕，核心送出 `SIGSEGV`。
>
> 詳細說明：C 標準只說「修改字串常數是未定義行為」；在本機會 segfault，是因為 gcc 把字串常數放進唯讀分頁。

#### 陣列 3-3. 你會怎麼在 gdb 裡確認 3-2 的答案？

> 題目：你會怎麼在 gdb 裡確認 3-2 的答案（改 `s1[0]` 沒事、改 `s2[0]` 卻 segfault 的原因）？

```
用info proc mappings 確認每一個載入段的記憶體位址範圍
然後在檢查s1 的位址,與s2的位址
在哪個載入段的範圍
```

> **批改**：**對（改判）**。方法對：用 `info proc mappings` 找出 `s1`、`s2` 的位址各落在哪一段。原判「大致對」，理由是沒寫「看權限欄」。使用者指出 3-2 已經講明兩邊權限不同，3-3 是在這個前提下去找位址落在哪一段；而 `info proc mappings` 每一段的權限就印在同一列，找到那一段就看到權限，改判。
> 提醒（不扣分）：小心「`s2` 的位址」：`&s2` 是指標變數本身的位址，在堆疊上；要查的是 `s2` 的**值**（它指向的位址），用 `print s2`。用詞上，`info proc mappings` 列的是記憶體對映（mapping），不是 ELF 的「載入段」，兩者相關，但不是同一張表。
>
> **參考答案**【[第 3 課第四節](../c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.md#四字串以-0-結尾的-char-陣列)】
>
> 核心關鍵：
> 1. 讓程式當在 `s2[0] = 'H';`，用 `print s2` 拿到 `s2` 指向的位址。
> 2. 用 `info proc mappings` 找到那個位址所在的那一段，看它的**權限是 `r--p`**。
>
> 詳細說明：拿來對照的話，`print &s1` 會落在 `[stack]`，權限是 `rw-p`。

---

## ELF（續）

### ELF 5. 情境題：你把 Ubuntu（x86-64）上編好的程式複製到路由器（ARM），`file` 顯示它是 ELF，執行卻失敗。這跟 [interpreter 那篇](../c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md) 第五節「interpreter 不存在」是不是同一種問題？從檔頭的哪一欄可以看出來？

> 盒子 1｜過去的作答：[s1-01 quiz.md ELF 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-情境題你把-ubuntux86-64上編好的程式複製到路由器armfile-顯示它是-elf執行卻失敗這跟-interpreter-那篇-第五節interpreter-不存在是不是同一種問題從檔頭的哪一欄可以看出來)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)

題目提到的「interpreter 不存在」實驗（interp 筆記[第五節](../c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md#五直譯器不存在時明明有檔案卻說找不到)）：把 `hello.c`（同上面 ELF 2）連結時故意把 interpreter 路徑改成不存在的檔案，再執行。
`-Wl,` 把後面的選項轉交給連結器 `ld`，`--dynamic-linker` 就是設定 `INTERP` 的選項：

```
$ gcc hello.c -o hello_badinterp -Wl,--dynamic-linker=/lib64/nope.so.2
$ ./hello_badinterp
```

```
不是同一個問題
從elf當頭可以知道machine是x86_64,而路由器是arm所以當然執行不了

從elf當頭的machine欄位
```

> **批改**：**對**（盒子 1 → 2）。不是同一種問題、看檔頭的 `Machine` 欄位，兩點都答到了。10-09 說成同一種問題、去看程式標頭的 `INTERP`，這次改正了。用詞：「檔頭」（ELF header），不是「當頭」。
>
> 註：筆記的題目在 10-09 改寫過，明講問的是「讓它失敗的原因」，並加了提示；本檔誤貼了舊版題目，不影響這次判定。參考答案的核心關鍵也順便調整：`ENOEXEC`／`ENOENT` 這兩個錯誤碼不是題目問的，移到詳細說明（筆記同步更新）。
>
> **參考答案**【[ELF 第五節](../c/ELF是什麼-可執行與可連結格式的兩種視角.md#五檔頭裡的其他欄位)】
>
> 核心關鍵：
> 1. 不是同一種問題：ELF 格式在哪台機器上都讀得懂，但裡面的**機器碼是給特定 CPU 的**。
> 2. 看檔頭（`readelf -h`）的 **`Machine`** 欄位：它寫著 `Advanced Micro Devices X86-64`，跟 ARM 的 CPU 不符。
>
> 詳細說明：kernel 在 `execve` 讀檔頭時發現架構不符，就回 `ENOEXEC`（`Exec format error`）；interpreter 不存在則是 `ENOENT`（`not found`），兩種錯誤訊息不同。動態連結的程式在路由器上確實**也**缺 interpreter，但架構不符時，kernel 讀到檔頭就停了，還沒走到「找 interpreter」那一步；就算把 x86-64 的載入器放到那個路徑，它本身也是 x86-64 的機器碼，一樣跑不了。`INTERP` 在程式標頭，不在檔頭；靜態編譯的程式根本沒有 `INTERP`，x86-64 的機器碼一樣不能在 ARM 上跑。

---

## C資料：[C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號](../c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md)

### C資料 2. `int x = 0x12345678; unsigned char *b = (unsigned char *)&x;` 在本機 `*b` 是多少？換到大端序的機器上是多少？為什麼 `int *p = &x;` 的 `*p` 在兩種機器上都是 `0x12345678`？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 2 題](../small-project/s1-02-data-repr/quiz.md#2-int-x--0x12345678-unsigned-char-b--unsigned-char-x-在本機-b-是多少換到大端序的機器上是多少為什麼-int-p--x-的-p-在兩種機器上都是-0x12345678)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)

**題目用到的程式**：筆記[第二節](../c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#二指標就是位址)的 `addr.c`：

```c
#include <stdio.h>

int main(void)
{
    int x = 0x12345678;
    int *p = &x;                              /* p 存著 x 的位址 */
    unsigned char *b = (unsigned char *)&x;   /* 同一個位址，但宣告成指向 1 個 byte */

    printf("x        = 0x%x (%d)\n", x, x);
    printf("&x       = %p\n", (void *)&x);
    printf("p        = %p\n", (void *)p);
    printf("*p       = 0x%x\n", *p);              /* 從那個位址讀 4 個 byte，當成 int */
    printf("b        = %p\n", (void *)b);
    printf("*b       = 0x%02x\n", *b);            /* 從同一個位址只讀 1 個 byte */
    *p = 0x11223344;                              /* 第 15 行：透過指標寫回去 */
    printf("after *p = 0x11223344, x = 0x%x\n", x);
    return 0;
}
```

```
$ gcc -Wall -g -O0 -o addr addr.c
$ ./addr
```

```
0x78

0x12

p是4byte的指標,所以都是讀4byte
大小端序的唯一差別只是數的方向小端序是從後開始往前數,大端序是從前往後數,做一最後的道德結果都是一樣
```

> **批改**：**對（改判）**（盒子 2 → 3）。原判「大致對」，理由有兩個，使用者爭論後兩個都撤回：
> - 「`p` 是 4 byte 的指標」：使用者說明意思是「`*p` 讀 4 byte、`p + 1` 走 4 byte」，只是用詞不精準；10-07 寫過「不是指標的大小，是從起點 byte 開始數 4 個 byte」，可見觀念是對的。用詞提醒：`p` 本身是 8 byte，4 是它**指向的 `int`** 的大小。
> - 「沒說為什麼一樣」：「小端從後往前數、大端從前往後數」，配上前面 `*b` 的答案（小端第一個 byte 是 `0x78`、大端是 `0x12`），就是在說：每台機器照自己存的順序去讀，所以拼回來都是 `0x12345678`。這正是「同一套順序寫、同一套順序讀，一來一回抵消」，只是講法不同。照核心關鍵的判定（用詞不同不扣分），改判為對。
>
> **參考答案**【[C資料 第三節](../c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#三位元組順序endianness)〈什麼時候要在意位元組順序？〉】
>
> 核心關鍵：
> 1. 本機（小端序）`*b` 是 `0x78`，大端序機器是 `0x12`。
> 2. `*p` 兩邊都一樣，是因為每台機器都用**自己那一套順序寫、再用同一套順序讀**（小端把第一個 byte 當最低位，也就是從後往前拼；大端把第一個 byte 當最高位，從前往後拼），一來一回抵消。
>
> 詳細說明：小端機器寫成 `78 56 34 12`，大端機器寫成 `12 34 56 78`。看得出順序的只有兩種情況：用 `unsigned char *` 一個 byte 一個 byte 看，或者一台機器寫、另一種順序的機器讀（網路封包、檔案）。

### C資料 3. `int n = -1; unsigned int len = 5;`，`n < len` 為什麼是假？只加 `-Wall` 編譯，gcc 會不會提醒你？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 3 題](../small-project/s1-02-data-repr/quiz.md#3-int-n---1-unsigned-int-len--5n--len-為什麼是假只加--wall-編譯gcc-會不會提醒你)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)

**題目用到的程式**：筆記[第七節](../c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#七陷阱有號和無號混在一起比較) `signs.c` 的最後一段（`n < len` 那段），抽出來成為能單獨編譯的版本：

```c
#include <stdio.h>

int main(void)
{
    int n = -1;
    unsigned int len = 5;

    if (n < len)
        printf("n < len\n");
    else
        printf("n < len is FALSE (n=%d, len=%u)\n", n, len);
    return 0;
}
```

```
$ gcc -Wall -o signcmp signcmp.c
$ ./signcmp
```

```
因爲轉成unsigned int -1 就溢位了, 應該是n>len

因爲溢位在在gcc是未定義行爲,他不管溢位是否發生
```

> **批改**：**錯**（盒子 2 → 1）。
> - 第一問只答到「轉成 `unsigned int`」。「`-1` 就溢位了」**錯**：`-1` 轉成 `unsigned int` 不是溢位，是 C 規定好的轉換，位元不變、換成無號讀法，`0xffffffff` 讀成 4294967295，比 5 大。也沒講**為什麼**是 `n` 被轉：一般算術轉換時，`int` 和 `unsigned int` 等級相同，等級相同時無號那邊贏（10-07 說成「無號等級比較大」，這次沒提到）。
> - 第二問**錯**：題目問會不會提醒，答案是**不會**。C 的 `-Wall` 不含 `-Wsign-compare`，要加 `-Wextra` 才有（10-07 答對過）。「溢位是未定義行為」跟這題無關，也不對：有號轉無號是**定義好的**（模 2³² 回繞），有號整數**運算**溢位才是未定義行為（[第八節](../c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#八溢位無號回繞有號是未定義行為)）。跟 10-05 C資料 10「以為有號轉無號是 UB」錯在同一處。
>
> 2026-10-16 在本機用上面的 `signcmp.c` 實測：
>
> ```
> $ gcc -Wall -o signcmp signcmp.c && echo "(-Wall 無警告)"
> (-Wall 無警告)
> $ gcc -Wall -Wextra -o signcmp signcmp.c
> signcmp.c: In function ‘main’:
> signcmp.c:8:11: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
>     8 |     if (n < len)
>       |           ^
> $ ./signcmp
> n < len is FALSE (n=-1, len=5)
> ```
>
> **參考答案**【[C資料 第四節](../c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#四無號整數二進位與回繞)〈怎麼確定一個常數的型別？比較時以誰為準？〉、[第六節](../c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#六有號與無號之間的轉換位元不變換個讀法)、[第七節](../c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#七陷阱有號和無號混在一起比較)】
>
> 核心關鍵：
> 1. 比較之前做一般算術轉換：`int` 和 `unsigned int` 等級相同，無號贏，`n` 被轉成 `unsigned int`。
> 2. 轉換時位元不變、換成無號讀法：`-1`（`0xffffffff`）變成 4294967295 > 5，所以 `n < len` 為假。
> 3. 只加 `-Wall` **不會**提醒（C 的 `-Wall` 不含 `-Wsign-compare`），要加 `-Wextra`。
>
> 詳細說明：這個轉換是定義好的，不是溢位，也不是未定義行為；未定義行為的是有號整數**運算**溢位（例如 `INT_MAX + 1`）。C++ 的 `-Wall` 就包含 `-Wsign-compare`，C 沒有。
