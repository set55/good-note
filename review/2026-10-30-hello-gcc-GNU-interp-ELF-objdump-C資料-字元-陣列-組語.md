# 2026-10-30 複習：hello、gcc、GNU、interp、ELF、objdump、C資料、字元、陣列、組語

> 依[複習排程](../learning/複習排程.md)出今天到期的全部題目（每天出完所有到期題，2026-10-23 起的規則），共 **21 題**（33 個作答框），沒有過期的。
> 其中 3 題在盒子 2（C資料 4、C資料 9 是 10-27 大致對留在盒子 2；組語 18 是 10-27 改判為對、升到盒子 2）；其餘 18 題是 10-23 大複習後排在盒子 3 的題目（含 3 題大致對留在盒子 3：hello 2、hello 3、interp 6）。
> 題目依筆記分節、節內照題號排；每題照原題複製，用到的程式碼與實驗步驟都附在題目裡，「過去的作答」連到之前每一次作答（含批改與參考答案）。
>
> **怎麼寫**：闔上筆記，每題寫 1～2 句關鍵句就好。可以分幾次寫，寫完一部分就可以先說「幫我批改」。複習不重答，批改會直接附參考答案。
> 已拆小題的題目，每個小題一個作答框；以大題為單位排程，小題全對才算對。

## 批改結果（2026-10-30）

| 題目 | 結果 | 盒子 | 下次 | 錯在哪／缺什麼 | 過關 |
|---|---|---|---|---|---|
| hello 2 | 對 | 3 → 4 | 11-13 | 抓到：`libc.so.6`、執行時 `ld.so` 填 GOT（10-23 缺的這次有了）；本機 `hello` 是 `BIND_NOW`，載入時就填好、不是延遲綁定 | — |
| hello 3 | 對 | 3 → 4 | 11-13 | 抓到：編譯時不知道 `puts` 在哪先填 0、重定位項目 `R_X86_64_PLT32 puts-0x4`（10-23 缺的「為什麼」這次有了） | — |
| gcc 3 | 對 | 3 → 4 | 11-13 | 抓到：內嵌、常數摺疊 | — |
| gcc 4 | 對 | 3 → 4 | 11-13 | 抓到：不一定越小；速度 vs 大小；迴圈展開，用空間換時間 | — |
| GNU 5 | 對 | 3 → 4 | 11-13 | 抓到：DCO、GPL-2.0、查得到負責人；「讓你有權利」說反了，是**聲明**自己有權（跟 10-23 同樣的寫法） | — |
| GNU 6 | 大致對 | 3 → 3 | 11-06 | 前兩點對；判斷 libc 改成看系統有沒有 `libc.so.6` 檔，應看**程式本身**的 interpreter／`readelf -V`（10-23 答對過） | ✅ 過關（10-30） |
| interp 6 | 對 | 3 → 4 | 11-13 | 抓到：`sh` 印的、`sh` 自己讀檔沒 `r` 被擋、ELF 由核心直接映射（10-23 錯的這次改對了） | — |
| ELF 2 | 對 | 3 → 4 | 11-13 | 抓到：沒有執行視角；合併區段、打包 `LOAD`、寫程式標頭；打包依據是「權限相同」要講出來 | — |
| ELF 3 | 對 | 3 → 4 | 11-13 | 抓到：預設 PIE → DYN、`-static` 不是 PIE → EXEC | — |
| ELF 5 | 對 | 3 → 4 | 11-13 | 抓到：不是同一種問題，核心讀檔頭就擋、走不到 interpreter；`Machine` 欄 | — |
| objdump 1 | 不完整 | 3 → 1 | 10-31 | 三類對；第二問「為什麼 `objdump` 看不到」沒作答：`NEEDED` 只有直接依賴、`ldd` 讓載入器實際載入（10-23 答對過） | ✅ 過關（10-30） |
| C資料 4 | 對 | 2 → 3 | 11-06 | 抓到：UB、`-O0` 回繞／`-O2` 假設不溢位、不是 bug、`-fno-strict-overflow` 與代價（10-27 寫成「打標籤」的這次改對了） | — |
| C資料 7 | 對 | 3 → 4 | 11-13 | 抓到：十進位／十六進位的型別清單、兩個比較的結果與轉換 | — |
| C資料 9 | 大致對 | 2 → 2 | 11-02 | 前兩問對；第三問缺機制「UB → gcc 假設不溢位 → 化簡成 `y < 0`」，只寫了結果（10-27 有 `y < 0`、缺假設；這次兩個都沒寫） | ✅ 過關（10-30） |
| 字元 4 | 對 | 3 → 4 | 11-13 | 抓到：`char` 有號、超過 127 變負數永遠不成立；改 `unsigned char` | — |
| 陣列 1-1 | 對 | | | 抓到：`int *` 走 4、`unsigned char *` 走 1 | — |
| 陣列 1-2 | 對 | | | 抓到：`*(p + 3)` | — |
| 陣列 1-3 | 對 | 3 → 4（整題） | 11-13 | 抓到：`*(3 + p)`，加法可交換 | — |
| 陣列 6-1 | 對 | | | 抓到：24 byte | — |
| 陣列 6-2 | 對 | | | 抓到：6 個 | — |
| 陣列 6-3 | 對 | | | 抓到：索引 6 | — |
| 陣列 6-4 | 對 | 3 → 4（整題） | 11-13 | 抓到：3 個 `00` | — |
| 組語 2-1 | 對 | | | 抓到：寫 `%eax` 自動清高 32 位元 | — |
| 組語 2-2 | 對 | 3 → 4（整題） | 11-13 | 抓到：不成立、寫 16 位元不會清，要零擴展指令；原因要明寫「高位保留舊值」 | — |
| 組語 3-1 | 對 | | | 抓到：`setl` 看 SF、OF，`setb` 看 CF；旗標值與回傳值 | — |
| 組語 3-2 | 對 | 3 → 4（整題） | 11-13 | 抓到：編譯時 gcc 依型別挑指令，CPU 沒有有號無號 | — |
| 組語 5-1 | 對 | | | 抓到：8 byte 元素（`long`） | — |
| 組語 5-2 | 對 | | | 抓到：`a[i + 3]` | — |
| 組語 5-3 | 對 | 3 → 4（整題） | 11-13 | 抓到：`a + i + 3`（`&a[i + 3]`） | — |
| 組語 18-1 | 對 | | | 抓到：進位傳進第 7 位、沒有往外 | — |
| 組語 18-2 | 對 | | | 抓到：CF 0、OF 1；CF 只看第 7 位往外 | — |
| 組語 18-3 | 對 | | | 抓到：沒往外進位、沒溢位（「沒有借位」是減法的說法） | — |
| 組語 18-4 | 對 | 2 → 3（整題） | 11-06 | 抓到：進 1、出 1，XOR 得 0 | — |

> 總計 21 題：對 18、大致對 2（GNU 6、C資料 9）、不完整 1（objdump 1）。
> 不是「對」的 3 題要在各自批改下方的**自我練習框**重寫到算對才過關（`CLAUDE.md`「自我練習過關」），沒過關前會擋住上第 8 課。

## 今天的題目

排序：盒子小的在前（盒子 2 的 3 題），其餘依上次作答日期；同一天的照筆記順序。

| # | 題目 | 盒子 | 原本該考的日期 |
|---|---|---|---|
| 1 | C資料 4 | 2 | 2026-10-30 |
| 2 | C資料 9 | 2 | 2026-10-30 |
| 3 | 組語 18（4 小題） | 2 | 2026-10-30 |
| 4 | hello 2 | 3 | 2026-10-30 |
| 5 | hello 3 | 3 | 2026-10-30 |
| 6 | gcc 3 | 3 | 2026-10-30 |
| 7 | gcc 4 | 3 | 2026-10-30 |
| 8 | GNU 5 | 3 | 2026-10-30 |
| 9 | GNU 6 | 3 | 2026-10-30 |
| 10 | interp 6 | 3 | 2026-10-30 |
| 11 | ELF 2 | 3 | 2026-10-30 |
| 12 | ELF 3 | 3 | 2026-10-30 |
| 13 | ELF 5 | 3 | 2026-10-30 |
| 14 | objdump 1 | 3 | 2026-10-30 |
| 15 | C資料 7 | 3 | 2026-10-30 |
| 16 | 字元 4 | 3 | 2026-10-30 |
| 17 | 陣列 1（3 小題） | 3 | 2026-10-30 |
| 18 | 陣列 6（4 小題） | 3 | 2026-10-30 |
| 19 | 組語 2（2 小題） | 3 | 2026-10-30 |
| 20 | 組語 3（2 小題） | 3 | 2026-10-30 |
| 21 | 組語 5（3 小題） | 3 | 2026-10-30 |

---

## hello：[hello.c怎麼變成能跑的程式-編譯的四個階段](../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html)

### hello 2. `hello.i` 裡有 `printf` 的宣告，卻沒有它的程式碼。那它的程式碼在哪？是在哪個時間點、由誰把「呼叫 `printf`」和「`printf` 的程式碼」接起來的？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 2 題](../small-project/s1-01-hello-compile/quiz.md#2-helloi-裡有-printf-的宣告卻沒有它的程式碼那它的程式碼在哪是在哪個時間點由誰把呼叫-printf和printf-的程式碼接起來的)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
在libc.so.6裏面

由ld.so接起來

接起來的流程:執行檔在跑的時候, 程式碼跑到call puts,跳到puts@plt ,執行這個程式碼,會在跳到.got.plt裏記錄puts位址的地方,但這個時候這個位址只存着一個 '呼叫ld.so去找libc.so.6裏的puts位址的程式碼' 的位址,跳到那個位址執行程式碼,ld.so就會找puts在 libc.so.6裏的位址,接着把這個位址在.got.plt覆蓋掉原本呼叫ld.so的位址
```

> **批改**：**對**（盒子 3 → 4）。兩個核心關鍵都有：程式碼在 `libc.so.6`；**執行時**由 `ld.so` 把 `puts` 的位址填進 GOT 那一格（10-23 缺的「填進 `.got`」這次有了）。小修（不扣分）：你描述的是第 7 課的**延遲綁定**（第一次呼叫才經過 `ld.so` 填格子），但本機 gcc 編的 `hello` 不是這樣——`readelf -d hello` 有 `FLAGS BIND_NOW`（Ubuntu 預設連結時加 `-z now`），而且 `hello` 沒有 `.got.plt`，`puts` 那格在 `.got`（`0x3fd0`）。所以 `ld.so` 是在**載入時、跳到 `_start` 之前**就把 `puts` 那格填好，`call puts@plt` → `jmp *0x3fd0` 第一次就直接到 `libc` 的 `puts`。延遲綁定是第 7 課的 `libgreet.so` 才看得到（[第 7 課第九節第 4 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#4-plt-存函式位址got-存變數位址嗎plt-是跳板以及延遲綁定)）。兩種時機都是「執行時、由 `ld.so` 填 GOT」，所以不影響這題的判定。
>
> **參考答案**【[第五節](../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html#五連結把洞接上但-puts-還是沒接完)〈2026-09-29 補充〉、[第六節](../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html#六執行從-execve-到-write)、[第十節](../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html#十三個名字很像的檔案ldld-linux-x86-64so2libcso6)】
>
> **核心關鍵**：
>
> 1. 程式碼在 **`libc.so.6`**（`printf` 在本課被換成 `puts`）。
> 2. **執行時**由動態載入器 `ld-linux-x86-64.so.2` 接起來：`execve` 後它先跑，照需求單打開 `libc.so.6`、映射進記憶體，再把 `puts` 的真正位址填進 `.got`。
>
> **詳細說明**：這一步要成立，**連結時** `ld` 得先做好兩件事：把 `call` 後面的洞填成跳到 `puts@plt`，並在執行檔裡寫下需求單（`NEEDED libc.so.6`、interpreter 路徑、需要的符號版本）。CPU 執行 `call` 只是照那 4 個 byte 跳，不是「接」。`LD_DEBUG=bindings ./hello` 看得到載入器接線這一步。本機 `hello` 帶 `BIND_NOW`，所以 `puts` 那格在載入時就填好；沒有 `BIND_NOW` 的才是第一次呼叫時填（延遲綁定）。

### hello 3. `hello.o` 裡「呼叫 `puts`」的地方，要跳去的位址為什麼暫時是 0（空著）？什麼資訊讓連結器知道之後要把它填成什麼？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-helloo-裡-call-指令後面的四個-byte-為什麼是-00-00-00-00什麼資訊讓連結器知道要把它填成什麼)（舊題目文字）、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md#hello-3-helloo-裡-call-指令後面的四個-byte-為什麼是-00-00-00-00什麼資訊讓連結器知道要把它填成什麼)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

題目用到的程式 `hello.c`（檔案：[`hello.c`](../small-project/s1-01-hello-compile/hello.c)）完整原始碼：

```c
#include <stdio.h>

#define GREETING "hello, world"

int main(void)
{
    printf("%s\n", GREETING);
    return 0;
}
```

產生 `hello.o` 的指令（只編譯＋組譯、不連結）：

```sh
gcc -c hello.c -o hello.o
```

（原始碼寫的是 `printf`，gcc 把「`printf("%s\n", 字串)`」換成了 `puts(字串)`，所以 `hello.o` 裡呼叫的是 `puts`——見筆記第三節。）

> 題目：`hello.o` 裡「呼叫 `puts`」的地方，要跳去的位址為什麼暫時是 0（空著）？什麼資訊讓連結器知道之後要把它填成什麼？

```
因爲gcc在把hello.c編譯成hello.o的過程中不知道puts在哪裏,所以先寫成0,準備交給ld處理,讓ld去找這邊要填上哪個符號

13: R_X86_64_PLT32      puts-0x4, ld就可以知道這邊有個叫puts的函式要跳轉
```

> **批改**：**對**（盒子 3 → 4）。兩個核心關鍵都有：編譯成 `hello.o` 時不知道 `puts` 在哪（10-23 缺的「為什麼不知道」這次有了），先填 0；重定位項目 `13: R_X86_64_PLT32 puts-0x4` 讓連結器知道這裡要填 `puts`（10-23 寫成 `printf` 的錯也改了）。小修（不扣分）：(1) 精確說是**組譯器 `as`** 一次只處理一個檔案，`puts` 的程式碼在別的檔案（`libc.so.6`），所以不知道位址；(2) 那一行的名字叫「**重定位項目**（relocation entry）」，是組譯器留給 `ld` 的備忘：「位移 `0x13` 的 4 個 byte，將來填 `puts` 的位址」。
>
> **參考答案**【[第四節](../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html#四組譯機器碼裡的洞)】
>
> **核心關鍵**：
>
> 1. 組譯器一次只處理 `hello.s` 這一個檔案，`puts` 的程式碼在別的檔案（`libc.so.6`），組譯時不知道它的位址，只能先填 0 占位。
> 2. 組譯器同時記下一筆重定位項目 `13: R_X86_64_PLT32 puts-0x4`（「位移 0x13 的地方，將來填上 `puts` 的位址」）；連結器照這筆項目去找名叫 `puts` 的符號，把位址填進那 4 個 byte。
>
> **詳細說明**：`objdump -dr` 的 `-r` 會把重定位項目印在對應指令的下面。同一段裡的 `lea 0x0(%rip),%rax` 也是同樣情況：字串 `"hello, world"` 放在 `.rodata`，位址到連結時才決定，所以也有一筆 `R_X86_64_PC32 .rodata-0x4`。

---

## gcc：[gcc最佳化等級-O0到O3差在哪](../html/c/gcc最佳化等級-O0到O3差在哪.html)

### gcc 3. 在 `-O2` 下，`answer()` 連呼叫 `sum_to` 這件事都不見了，直接回傳 5050。編譯器做了哪兩件事才能做到？

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

`-O0` 與 `-O2` 的 `answer`（本機 gcc 13.3 實際輸出，刪掉組譯指示）：

```asm
# -O0
answer:
	endbr64
	pushq	%rbp
	movq	%rsp, %rbp
	movl	$100, %edi         # 第一個參數 = 100
	call	sum_to             # 呼叫 sum_to
	popq	%rbp
	ret                        # 回傳值就是 sum_to 留在 %eax 的結果

# -O2
answer:
	endbr64
	movl	$5050, %eax        # 回傳值直接設成 5050
	ret
```

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3--o2-下-answer-直接變成-movl-5050-eax編譯器做了哪兩件事才能做到)、[10-05 複習](./2026-10-05-gcc-陣列-GNU-C資料-字元.md)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
1. 程式內嵌: 他直接把整個sumto內嵌到answer裏了
2. 常量摺疊: 因爲sumto的參數是常量100, gcc直接把數字代進內嵌的程式碼把結果算出來
```

> **批改**：**對**（盒子 3 → 4）。兩個核心關鍵都有：內嵌（把 `sum_to` 整段複製進 `answer`）、常數摺疊（參數是常數 100，編譯時就把結果算出來）。小修（不扣分）：正式譯名是「常數摺疊」（constant folding）。
>
> **參考答案**【[第二節](../html/c/gcc最佳化等級-O0到O3差在哪.html#二實驗同一段程式不同等級編出什麼)〈「內嵌」是什麼〉】
>
> **核心關鍵**：
>
> 1. **內嵌（inline）**：把 `sum_to` 的函式本體整段複製進 `answer()`，取代「呼叫」，參數 `n` 換成常數 100。
> 2. **常數摺疊（constant folding）**：內嵌後所有數字在編譯時都已知，編譯器在**編譯時**就把迴圈算完，得到 5050。
>
> **詳細說明**：兩步缺一不可。單看 `sum_to(int n)`，`n` 要到執行時才知道，沒辦法事先算；是內嵌把 100 帶進來，才算得出來。給別人呼叫的那份 `sum_to` 仍然保留迴圈。

### gcc 4. 「最佳化等級越高，程式越小」對嗎？`-O2`／`-O3` 和 `-Os`／`-Oz` 的目標各是什麼？為什麼追求速度反而可能讓程式變大？

（2026-10-09 改寫的題目：原題「用本篇的數字說明」要求背數字，闔上筆記作答時無從引用。不用寫出確切的 byte 數。）

**題目用到的程式**：同上一題 gcc 3 的 `sumto.c`。同一份 `sumto.c` 用不同的最佳化等級各編成 `.o`，比 `.text` 區段（放機器碼的區段）的大小，例如：

```sh
gcc -O2 -c sumto.c -o sumto_O2.o   # -c：編譯成目的檔就停，不連結
size -A sumto_O2.o                 # -A：逐個區段列出大小，看 .text 那一列
```

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-最佳化等級越高程式越小對嗎用本篇的數字說明)、[10-05 複習](./2026-10-05-gcc-陣列-GNU-C資料-字元.md)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
1. 每個最佳化等級只說明 最佳化了什麼東西,程式不一定就越小

2.
-O2,-O3: 這兩個追求的是速度,一般使用空間換取時間的方式
-Os,-Oz: 追求空間, 可能是犧牲時間換取空間

3. 可能是用了迴圈擴展,增加單個迴圈的邏輯,一個迴圈多做1次之類
```

> **批改**：**對**（盒子 3 → 4）。三個核心關鍵都有：不對、程式不一定越小；`-O2`／`-O3` 求速度、`-Os`／`-Oz` 求大小；速度靠迴圈展開（一圈做好幾次的工作）這類改寫，用空間換時間。小修（不扣分）：第 1 點可以講得更直接——`-O2`／`-O3` 可能比 `-O0` **還大**，決定大小的是「目標」不是等級的數字；變大的原因除了迴圈展開，還有內嵌（把函式本體複製到每個呼叫點）與向量化。
>
> **參考答案**【[第二節](../html/c/gcc最佳化等級-O0到O3差在哪.html#二實驗同一段程式不同等級編出什麼)〈大小（`.text` 區段，`size -A` 實測）〉、〈`-O2`：答案在編譯時就算好了〉】
>
> **核心關鍵**：
>
> 1. 不對：`-O2`／`-O3` 編出來的程式可能比 `-O0` 還大。
> 2. `-O2`／`-O3` 的目標是**速度**；`-Os`／`-Oz` 的目標才是**大小**。決定大小的是最佳化的目標，不是等級的數字。
> 3. 為了跑得快，編譯器會改寫迴圈（展開、一次處理好幾個元素、向量化）、內嵌函式，用更多指令換來更少的迴圈判斷與呼叫，也就是用空間換時間。
>
> **詳細說明**：本機 `sumto.c` 的 `.text` 大小：`-Oz` 30、`-Os` 35、`-O0` 70、`-O2` 90、`-O3` 234 byte。`-O2` 版的 `sum_to` 一圈加兩個數，進迴圈前還要先處理奇偶，所以比 `-O0` 長。量的是 `.text` 區段（機器碼），不是整個檔案。

---

## GNU：[GNU是什麼-Linux核心與GNU工具的分工](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html)

### GNU 5. 你將來送 kernel patch 時要加的 `Signed-off-by:` 跟 GPL 有什麼關係？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-你將來送-kernel-patch-時要加的-signed-off-by-跟-gpl-有什麼關係)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md#gnu-5-你將來送-kernel-patch-時要加的-signed-off-by-跟-gpl-有什麼關係)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
這是DCO, 讓你有權利可以以GPL(kernel是GPL2)的授權發佈
而且有Signed-off-by 在出現問題的時候可以直接找到對應的負責人
```

> **批改**：**對**（盒子 3 → 4）。兩個核心關鍵都有：`Signed-off-by:` 是 DCO、跟 kernel 的 GPL-2.0 有關；出問題查得到是誰負責。小修（不扣分，但方向要擺正）：「**讓你有權利**以 GPL 發佈」說反了——簽名本身不會給你任何權利，它是你**聲明（certify）**「這段是我寫的，或我本來就有權以 GPL-2.0 貢獻它」。權利要事先就有，DCO 是把這個聲明留下紀錄。10-23 也是同樣的寫法。
>
> **參考答案**【[第五節](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html#五gplgnu-最深遠的影響)】
>
> **核心關鍵**：
>
> 1. `Signed-off-by:` 是 DCO（Developer Certificate of Origin，開發者原創聲明）：貢獻者聲明這段程式碼是自己寫的，或自己有權以專案的授權（kernel 是 GPL-2.0）貢獻它。
> 2. kernel 以 GPL-2.0 散布，每段併入的程式碼都必須能合法以 GPL-2.0 散布；DCO 讓每個 patch 都有人為來源負責，出了著作權爭議查得到是誰聲明的。
>
> **詳細說明**：DCO 是 kernel 社群的規定，不是 GPL 條文本身的要求，GPL 沒有規定發布時要署名。

### GNU 6. 情境題：同事說「我在這台機器上打 `uname -o` 看到 `GNU/Linux`，所以它一定用 glibc」。這個推論錯在哪？`uname` 的哪些欄位可以信、哪一欄不行，為什麼？你會改看什麼來判斷？

> 盒子 3｜過去的作答：[s1-01 quiz.md GNU 第 6 題](../small-project/s1-01-hello-compile/quiz.md#6-情境題同事說我在這台機器上打-uname--o-看到-gnulinux所以它一定用-glibc這個推論錯在哪uname-的哪些欄位可以信哪一欄不行為什麼你會改看什麼來判斷)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
uname -o 並沒有向kernel確認,而是uname自己打印出來的,不管在哪個環境都是gnu

可信: -s,-n,-v,-r,-m  這些都是跟kernel確認
不可信:-p,-i,-o

-p,-i 說明上明說了non-portable, 而且各家平臺對這兩個定義完全不同

-o 想正確知道的話,應該看c libarary的路徑
/lib/x86_64-linux-gnu/libc.so.6
```

> **批改**：**大致對**（盒子 3 → 3）。前兩個核心關鍵對：`-o` 是 `uname` 自己印的字串、沒問 kernel；`-s`／`-n`／`-r`／`-v`／`-m` 問 kernel 可以信，`-p`／`-i`／`-o` 不行。缺第 3 點：判斷 libc 要看**程式本身**，你改成看「系統上有沒有 `/lib/x86_64-linux-gnu/libc.so.6` 這個檔」。這不夠：檔案存在只代表裝了 glibc，不代表程式用它（例如 Alpine 可以裝 `gcompat`／glibc 相容套件，但系統的程式仍然連 musl）；同事問的是「這台機器（的程式）用哪個 libc」，要去看程式要求的 interpreter（`readelf -l /bin/ls | grep interpreter`：`ld-linux-*` 是 glibc、`ld-musl-*` 是 musl）或 `readelf -V` 有沒有 `GLIBC_` 版本需求。10-23 你答的正是這兩招。小修：「不管在哪個環境都是 gnu」——只在 Linux 上的 GNU `uname` 和 BusyBox `uname` 是這樣，別的系統的 `uname -o` 會印自己的名字（例如 FreeBSD）。
>
> **參考答案**【[第四節](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html#四不是-gnu-的-linux你已經遇過了)〈`uname -o` 印出 `GNU/Linux`，不代表它是 GNU〉】
>
> **核心關鍵**：
>
> 1. 錯在把 `-o` 當證據：它是 `uname` 程式寫死的字串，不是問 kernel 來的（BusyBox 版也印 `GNU/Linux`，實測用 musl 的 OpenWrt 一樣印）。
> 2. `-s`、`-n`、`-r`、`-v`、`-m` 來自 `uname()` 系統呼叫（kernel 回傳的 `struct utsname`），可以信；`-o` 不行，`-p`、`-i` 也是程式自己判斷的（說明裡標 non-portable）。
> 3. 判斷 libc 要看**程式本身**：`readelf -l /bin/ls | grep interpreter`（`ld-linux-*` 是 glibc、`ld-musl-*` 是 musl），或用 `readelf -V` 看有沒有 `GLIBC_` 版本需求。
>
> **詳細說明**：本機 `/bin/ls` 的 interpreter 是 `/lib64/ld-linux-x86-64.so.2`；`readelf -V /bin/ls` 列出 `File: libc.so.6`、`Name: GLIBC_2.38` 等項目。musl 不用符號版本，所以沒有 `GLIBC_` 項目。也可以 `ls /lib/ld-musl-*`，或直接執行載入器，看它怎麼自我介紹。只看「系統上有沒有某個 libc 檔」不夠，因為同一台機器可以同時裝兩種 libc。

**自我練習**（不批改）

```
1. uname -o 之 uname直接 把gnu/linux打印出來,並不是向kernel取得的

2. 可信:s,n,r,v,m 都是跟kernel拿的
   不可信: i,p,o
其中i,p明說是non portable,而i, p 在不同平臺 沒有統一標準,置於o是直接打印

3. strace ./hello
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
可以看到執行時打開的libc.so.6
```

---

## interp：[ELF的interpreter是什麼-動態載入器與井字號驚嘆號](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html)

### interp 6. 情境題：一支腳本權限是 `---x--x--x`（只有執行權限），第一行是 `#!/bin/sh`，執行時出現 `/bin/sh: ./x.sh: Permission denied`。這個錯誤是誰印的？為什麼核心讀得到 `#!` 那一行，直譯器卻讀不到腳本？同樣只有執行權限的 ELF 執行檔為什麼就能跑？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 6 題](../small-project/s1-01-hello-compile/quiz.md#6-情境題一支腳本權限是----x--x--x只有執行權限第一行是-binsh執行時出現-binsh-xsh-permission-denied這個錯誤是誰印的為什麼核心讀得到--那一行直譯器卻讀不到腳本同樣只有執行權限的-elf-執行檔為什麼就能跑)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
sh 報的

kernel不受到角色權限限制,但sh會,所以sh雖然有腳本路徑,但卻無法看裏面的內容,因此報permission denied

因爲kernel已經把elf映射到記憶體了,而ld.so可以直接從記憶體的映射直接看到他需要載入的庫等
```

> **批改**：**對**（盒子 3 → 4）。三個核心關鍵都有：錯誤是 `sh` 印的；`sh` 拿到的是腳本路徑、要自己讀內容，受你的權限限制，沒有 `r` 就被擋；ELF 是**核心直接映射進記憶體**（10-23 說成「interpreter 沒讀檔」，這次改對了）。小修（不扣分）：(1)「kernel 不受角色權限限制」說過頭了——核心在 `execve` 照樣檢查你有沒有 `x`，只是它讀 `#!` 那一行和映射 ELF **不需要 `r`**；(2) 補一句 `sh` 是「以你的身分跑的普通行程」，就把「為什麼 `sh` 會被擋」講完整了；(3) `ld.so` 那句跟這題無關（靜態 ELF 沒有 `ld.so` 一樣只要 `x` 就能跑）。
>
> **參考答案**【[第四節](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html#四跟腳本的--是同一個機制)〈交給直譯器的是「路徑」，不是內容：三個實驗〉的實驗三】
>
> **核心關鍵**：
>
> 1. 錯誤是 **`/bin/sh`** 印的（訊息開頭就是它的名字），不是核心印的。
> 2. 核心在 `execve` 裡自己讀 `#!` 那一行，只檢查 `x`、不受 `r` 限制；核心交給 `sh` 的是**腳本路徑**，`sh` 是以你的身分執行的普通行程，要自己 `open` 腳本，沒有 `r` 就被擋下。
> 3. 只有 `x` 的 ELF 能跑，是因為程式檔由**核心直接映射進記憶體**，不需要任何使用者行程打開它。
>
> **詳細說明**：所以腳本要 `r`＋`x`，ELF 只要 `x`。實驗三用 `#!/bin/cat` 做同一件事：`chmod 111 catscript.sh` 之後執行，錯誤開頭是 `/usr/bin/cat:`；`cp /usr/bin/true mytrue; chmod 111 mytrue` 則可以正常跑。靜態連結的 ELF 沒有 interpreter，一樣只要 `x`。

---

## ELF：[ELF是什麼-可執行與可連結格式的兩種視角](../html/c/ELF是什麼-可執行與可連結格式的兩種視角.html)

### ELF 2. `readelf -l hello.o` 說 `There are no program headers in this file`，這代表什麼？連結器做了什麼才讓執行檔有程式標頭？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 2 題](../small-project/s1-01-hello-compile/quiz.md#2-readelf--l-helloo-說-there-are-no-program-headers-in-this-file這代表什麼連結器做了什麼才讓執行檔有程式標頭)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

**題目用到的實驗**（[ELF 第四節](../html/c/ELF是什麼-可執行與可連結格式的兩種視角.html#四名字的關鍵兩種視角)）：把 `hello.c` 只編譯不連結得到 `hello.o`，對它跑 `readelf -l`（`-l`：印程式標頭表）；再連結成執行檔，對執行檔跑同一個命令對照。

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

```sh
$ gcc -c hello.c          # -c：編譯＋組譯，停在 .o，不連結
$ readelf -l hello.o      # 題目裡的訊息就是這一步印的
$ gcc -o hello hello.o    # 連結成執行檔
$ readelf -l hello        # 對照
```

> 題目：`readelf -l hello.o` 說 `There are no program headers in this file`，這代表什麼？連結器做了什麼才讓執行檔有程式標頭？

```
1. 這代表這hello.o並不是執行視角能看的檔案. 事實上,他是提供給連結視角也就是連結器的檔案

2. 連結器會把所有.o組合在一起,把section 合併稱 LOAD,重定位每一個有定義的符號,找不到定義的(不在.o裏的),寫上NEEDED,最後在
寫上program header
```

> **批改**：**對**（盒子 3 → 4）。兩個核心關鍵都有：沒有程式標頭 ＝ 沒有執行視角，`.o` 是給連結器的；連結器合併各 `.o` 的區段、打包成 `LOAD`、寫出程式標頭。小修（不扣分）：(1) 打包的依據要講出來——是把**權限相同**的區段排在一起打包成一個 `LOAD`（記憶體權限以分頁為單位，所以要按權限分組），不是把所有區段合成一個；(2)「找不到定義的寫上 `NEEDED`」不精確：`NEEDED` 記的是**函式庫的名字**（`libc.so.6`），不是一個個符號；`ld` 要在連結時給它的 `.so` 裡找到那些符號，找不到就報 `undefined reference`、不產生執行檔。
>
> **參考答案**【[第四節](../html/c/ELF是什麼-可執行與可連結格式的兩種視角.html#四名字的關鍵兩種視角)〈最有力的證據：`.o` 沒有執行視角〉】
>
> **核心關鍵**：
>
> 1. 沒有程式標頭表 ＝ 沒有執行視角：`.o` 只給連結器用，核心還不能載入執行它。
> 2. 連結器先把多個 `.o` 的同類區段**合併**，再把權限相同的區段打包成載入段 `LOAD`（依分頁對齊），最後寫出程式標頭表。
>
> **詳細說明**：合併之後還要補上位址（重定位）。動態連結時，程式標頭表裡另外有 `INTERP`（interpreter 的路徑）、`DYNAMIC`（指向 `.dynamic`，裡面有 `NEEDED`）等項，「可連結的」才變成「可執行的」。

### ELF 3. 本機一般 `gcc` 編出的執行檔，`Type` 為什麼是 DYN，跟 `libc.so.6` 同一類，而靜態版是 EXEC？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-本機一般-gcc-編出的執行檔type-為什麼是-dyn跟-libcso6-同一類而靜態版是-exec)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#elf-3-本機一般-gcc-編出的執行檔type-為什麼是-dyn跟-libcso6-同一類而靜態版是-exec)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
DYN: 因爲gcc 一般默認的是 PIE, 需要遵守ASLR, 都是位址都是動態決定的
EXEC: 靜態版就是 static 它包含了 -no-pie, 他們的位址都是固定的
```

> **批改**：**對**（盒子 3 → 4）。兩個核心關鍵都有：gcc 預設 PIE → DYN；`-static`（不是 PIE、位址固定）→ EXEC。小修（不扣分）：PIE 不是「需要遵守 ASLR」，而是「**可以**被放在任意位址」，ASLR 才有辦法每次挑不同位置；`Type` 只看是不是 PIE，跟有沒有動態連結無關（`-no-pie` 動態連結 → EXEC、`-static-pie` → DYN）。
>
> **參考答案**【[第三節](../html/c/ELF是什麼-可執行與可連結格式的兩種視角.html#三一種格式四種檔案)〈DYN／EXEC 跟「有沒有動態連結」無關：兩個反例〉】
>
> **核心關鍵**：
>
> 1. `Type` 看的是**是不是 PIE**（Position-Independent Executable，位置無關執行檔，每次執行由 ASLR 放在隨機位址）：本機 gcc 預設產生 PIE → DYN。
> 2. 不是 PIE（`-static` 版，固定位址）→ EXEC。
>
> **詳細說明**：PIE 跟共享函式庫一樣「可以放在任意位址」，所以跟 `libc.so.6` 同屬 DYN。`Type` 只看「是不是 PIE」，跟有沒有動態連結無關：`-no-pie` 動態連結 → EXEC，`-static-pie` 靜態連結 → DYN。

### ELF 5. 情境題：你把 Ubuntu（x86-64）上編好的程式複製到路由器（ARM），`file` 顯示它是 ELF，執行卻失敗。這跟 [interpreter 那篇](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html) 第五節「interpreter 不存在」是不是同一種問題？從檔頭的哪一欄可以看出來？

> 盒子 3｜過去的作答：[s1-01 quiz.md ELF 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-情境題你把-ubuntux86-64上編好的程式複製到路由器armfile-顯示它是-elf執行卻失敗這跟-interpreter-那篇-第五節interpreter-不存在是不是同一種問題從檔頭的哪一欄可以看出來)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

題目提到的「interpreter 不存在」實驗（interp 筆記[第五節](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html#五直譯器不存在時明明有檔案卻說找不到)）：把 `hello.c`（同上面 ELF 2）連結時故意把 interpreter 路徑改成不存在的檔案，再執行。
`-Wl,` 把後面的選項轉交給連結器 `ld`，`--dynamic-linker` 就是設定 `INTERP` 的選項：

```sh
$ gcc hello.c -o hello_badinterp -Wl,--dynamic-linker=/lib64/nope.so.2
$ ./hello_badinterp
```

> 題目：情境題：你把 Ubuntu（x86-64）上編好的程式複製到路由器（ARM），`file` 顯示它是 ELF，執行卻失敗。這跟 [interpreter 那篇](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html) 第五節「interpreter 不存在」是不是同一種問題？從檔頭的哪一欄可以看出來？

```
不同,kernel在執行這個檔案的時候會直接看到elf header的 machine寫的是x86_64, arm架構的核心會直接擋住,走不到interpreter

在machine欄位就可以
```

> **批改**：**對**（盒子 3 → 4）。兩個核心關鍵都有：不是同一種問題，機器碼是給 x86-64 的，核心讀檔頭就擋下、走不到找 interpreter 那一步；看檔頭的 `Machine` 欄。小修（不扣分）：可以補上兩種錯誤看起來不一樣——架構不符是 `Exec format error`（`ENOEXEC`），interpreter 不存在是 `not found`（`ENOENT`）。
>
> **參考答案**【[第五節](../html/c/ELF是什麼-可執行與可連結格式的兩種視角.html#五檔頭裡的其他欄位)】
>
> **核心關鍵**：
>
> 1. 不是同一種問題：ELF 格式在哪台機器上都讀得懂，但裡面的**機器碼是給特定 CPU 的**。
> 2. 看檔頭（`readelf -h`）的 **`Machine`** 欄位：它寫著 `Advanced Micro Devices X86-64`，跟 ARM 的 CPU 不符。
>
> **詳細說明**：kernel 在 `execve` 讀檔頭時發現架構不符，就回 `ENOEXEC`（`Exec format error`）；interpreter 不存在則是 `ENOENT`（`not found`），兩種錯誤訊息不同。動態連結的程式在路由器上確實**也**缺 interpreter，但架構不符時，kernel 讀到檔頭就停了，還沒走到「找 interpreter」那一步；就算把 x86-64 的載入器放到那個路徑，它本身也是 x86-64 的機器碼，一樣跑不了。`INTERP` 在程式標頭，不在檔頭；就算程式是靜態編譯、根本沒有 `INTERP`，x86-64 的機器碼一樣不能在 ARM 上跑。

---

## objdump：[objdump能不能代替ldd找依賴庫-需求單與實際載入](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html)

### objdump 1. `objdump -p /usr/bin/curl` 只列出 3 個 `NEEDED`，`ldd` 卻列出 33 行。多出來的是哪幾類東西？為什麼 `objdump` 看不到？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 1 題](../small-project/s1-01-hello-compile/quiz.md#1-objdump--p-usrbincurl-只列出-3-個-neededldd-卻列出-33-行多出來的是哪幾類東西為什麼-objdump-看不到)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#objdump-1-objdump--p-usrbincurl-只列出-3-個-neededldd-卻列出-33-行多出來的是哪幾類東西為什麼-objdump-看不到)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
1.
- interpreter
- 直接與間接依賴庫
- linux的虛擬函式庫
```

> **批改**：**不完整**（盒子 3 → 1）。第 1 點對：多出來的是間接依賴、vdso（`linux-vdso.so.1`）、動態載入器本身。題目的第二問「為什麼 `objdump` 看不到」**沒有作答**：`objdump -p` 只讀連結時寫進 `curl` 的需求單（`NEEDED`），裡面**只有直接依賴**；`ldd` 是請載入器實際載入一次，一路照每個函式庫的 `NEEDED` 找下去，才拿到完整清單。10-23 這一點答對過。小修：寫「直接與間接依賴庫」容易誤會——直接依賴（那 3 個）兩邊都看得到，**多出來的**是間接依賴。
>
> **參考答案**【[第二節](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html#二為什麼會差這麼多兩者看的是不同時間點)】
>
> **核心關鍵**：
>
> 1. 多出來的三類：**間接依賴**（最大宗，例如 `libssl.so.3` 是 `libcurl.so.4` 要的，不是 `curl` 自己要的）、`linux-vdso.so.1`（核心提供的虛擬函式庫）、動態載入器本身。
> 2. `objdump -p` 只讀連結時寫進 `curl` 的需求單（`NEEDED`），裡面**只有直接依賴**；`ldd` 是請載入器實際跑一次，才拿到完整清單。
>
> **詳細說明**：`NEEDED` 放在 dynamic section，不在檔頭。另外兩類也不會出現在 `curl` 的需求單裡：vdso 是核心在執行時直接放進行程的，沒有對應的檔案；載入器的路徑寫在 `INTERP`，不在 `NEEDED`。

**自我練習**（不批改）

```
1. 
- interpreter的路徑
- 間接的依賴庫
- linux虛擬庫

2. 
因爲objdump -p 只看這個執行檔的dynamic section的 NEEDED, 他不會去看相關依賴庫的NEEDED
```

---

## C資料：[C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html)

### C資料 4. `int y = x + 1; return y > x;` 在 `x = INT_MAX` 時，`-O0` 回傳 0、`-O2` 回傳 1。為什麼會這樣？這算不算 gcc 的 bug？kernel 用哪個選項避免這種情況，代價是什麼？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 4 題](../small-project/s1-02-data-repr/quiz.md#4-int-y--x--1-return-y--x-在-x--int_max-時-o0-回傳-0-o2-回傳-1為什麼會這樣這算不算-gcc-的-bugkernel-用哪個選項避免這種情況代價是什麼)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

題目用到的程式是筆記[第八節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#八溢位無號回繞有號是未定義行為)的 `ub.c`，完整原始碼：

```c
#include <stdio.h>
#include <limits.h>

int bigger(int x)
{
    int y = x + 1;
    return y > x;
}

int main(void)
{
    printf("bigger(INT_MAX) = %d\n", bigger(INT_MAX));
    return 0;
}
```

編譯與執行：`gcc -O0 ub.c -o ub && ./ub`、`gcc -O2 ub.c -o ub && ./ub`（結果如題目所述，分別印 0 與 1）。

> 題目：`int y = x + 1; return y > x;` 在 `x = INT_MAX` 時，`-O0` 回傳 0、`-O2` 回傳 1。為什麼會這樣？這算不算 gcc 的 bug？kernel 用哪個選項避免這種情況，代價是什麼？

```
-O0: 實際算出 INT_MAX +1 = INT_MIN所以回傳0
-O2: 溢位在c是未定義,所以c認爲溢位不會發生, 那 x+1 > x 爲真
因爲是ub,所以不能算是gcc的bug
-fno-strict-overflow: 這個時候溢位會變成定義的,所以可能發生溢位的地方就會實際計算
代價是失去溢位未定義最佳化的好處,計算時間拉長
```

> **批改**：**對**（盒子 2 → 3）。四個核心關鍵都有：有號溢位是 UB；`-O0` 照算回繞成 `INT_MIN` → 0、`-O2` 假設不會溢位 → 1；UB 所以不是 bug；`-fno-strict-overflow` 讓溢位變成有定義的回繞，代價是少了那些最佳化、可能變慢（10-27 把作用寫成「打標籤」，這次改對了）。小修（不扣分）：(1)「c 認爲溢位不會發生」——精確說是 C 標準**允許**編譯器假設它不會發生，做這個假設的是 gcc；(2) 不是 bug 的理由是「標準對發生 UB 的程式沒有任何要求，兩種結果都合法」；(3)「變成定義的」補一句「照二補數回繞」更完整。
>
> **參考答案**【[第八節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#八溢位無號回繞有號是未定義行為)】
>
> **核心關鍵**：
>
> 1. 有號溢位是未定義行為（UB），C 標準允許編譯器**假設它不會發生**。
> 2. `-O0` 照字面算，`INT_MAX + 1` 回繞成 `INT_MIN`，`y > x` 為假 → 0；`-O2` 依這個假設推論 `x + 1` 一定大於 `x`，直接回傳 1。
> 3. 不是 gcc 的 bug：標準對發生 UB 的程式沒有任何要求，兩種結果都合法。
> 4. kernel 用 `-fno-strict-overflow`（等於 `-fwrapv -fwrapv-pointer`），讓溢位照二補數回繞、變成有定義的行為；代價是少了依賴「不會溢位」的最佳化，程式可能稍慢。
>
> **詳細說明**：連 `-O0` 也不保證照字面算：第四節的 `overflow.c` 把同一件事寫成一行 `return x + 1 > x;`，`-O0` 也回傳 1，因為 gcc 在最佳化之前的化簡階段就把它改掉了（筆記第八節用 `-fdump-tree-original` 看到）。所以本題的 `bigger` 才故意拆成兩行。

### C資料 7. `2147483648` 和 `0x80000000` 是同一個數，為什麼一個的型別是 `long`、一個是 `unsigned int`？`-1 < 0x80000000` 和 `-1 < 2147483648` 的結果各是什麼？為什麼？

> 盒子 3｜過去的作答：[s1-02 quiz.md 第 7 題](../small-project/s1-02-data-repr/quiz.md#7-2147483648-和-0x80000000-是同一個數為什麼一個的型別是-long一個是-unsigned-int-1--0x80000000-和--1--2147483648-的結果各是什麼為什麼)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#c資料-7-2147483648-和-0x80000000-是同一個數為什麼一個的型別是-long一個是-unsigned-int-1--0x80000000-和--1--2147483648-的結果各是什麼為什麼)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
10進制規則: 先int -> long -> long long
16進制規則: int -> unsigned int -> long -> unsigned long -> long long -> unsigned long long
每個過程就看看自己的數字有沒有超過那個類別的範圍

2147483648 是long 因爲超過int 但沒超過long
0x80000000 是 unsigend int 沒超過 unsigned int

-1 < 0x80000000 false int 沒法包住 unsigned int 用無號, -1 是無號最大,所以false
-1 < 2147483648 true int與long 選long, 範圍大於int, -1還是-1, 故爲真
```

> **批改**：**對**（盒子 3 → 4）。四個核心關鍵都有：十進位清單 `int` → `long` → `long long`、十六進位清單中間夾無號型別；`2147483648` 是 `long`、`0x80000000` 是 `unsigned int`；`-1 < 0x80000000` 假（`-1` 轉成無號最大值）、`-1 < 2147483648` 真（轉成 `long` 仍是 −1）。小修（不扣分）：「int 沒法包住 unsigned int 用無號」的規則精確說是：`int` 和 `unsigned int` 等級（rank）相同時，**無號的贏**，`-1` 轉成 4294967295。
>
> **參考答案**【[第四節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#四無號整數二進位與回繞)〈常數也有型別：`0u` 的 `u` 是什麼〉、〈怎麼確定一個常數的型別？比較時以誰為準？〉】
>
> **核心關鍵**：
>
> 1. 沒有後綴的常數，依序嘗試、選第一個裝得下的型別。十進位的清單是 `int` → `long` → `long long`（永遠不變成無號），`2147483648` 超過 `INT_MAX`，所以是 `long`。
> 2. 十六進位的清單是 `int` → `unsigned int` → `long` → …，`0x80000000` 裝得進 `unsigned int`，所以是 `unsigned int`。
> 3. `-1 < 0x80000000` 為**假**：`int` 與 `unsigned int` 比，`-1` 轉成 4294967295。
> 4. `-1 < 2147483648` 為**真**：`int` 與 `long` 比，`-1` 轉成 `long` 還是 −1。
>
> **詳細說明**：十六進位清單多放無號型別，是因為寫十六進位的人通常在寫位元樣式（例如遮罩 `0x80000000`），要的是那個 bit 樣式，不是負數。

### C資料 9. 情境題：有人寫了 `if (len - used > 0)` 判斷「緩衝區還有沒有剩餘空間」，`len`、`used` 都是 `size_t`，用 `-Wall -Wextra` 編譯沒有任何警告。這個判斷在什麼情況下會出錯？為什麼 gcc 對 `len - used < 0` 會警告、對 `> 0` 卻不會？另外，為什麼 `int sum = x + y; if (sum < x)` 這種溢位檢查在 `-O2` 下可能完全失效？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 9 題](../small-project/s1-02-data-repr/quiz.md#9-情境題有人寫了-if-len---used--0-判斷緩衝區還有沒有剩餘空間lenused-都是-size_t用--wall--wextra-編譯沒有任何警告這個判斷在什麼情況下會出錯為什麼-gcc-對-len---used--0-會警告對--0-卻不會另外為什麼-int-sum--x--y-if-sum--x-這種溢位檢查在--o2-下可能完全失效)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
1. 在len < used會出錯, 因爲兩個都是unsigned long 類型,結果會是大於0, 判斷出錯

2. 因爲len, used都是unsigned long兩者運算結果需要是unsigned long 所以小於0會警告,無號不會小於0, 此判斷一定是false
大於或等於兩種不會,因爲是無號相減不是大於0就是等於0

3. 首先這個溢位檢查的前提必須是y 爲正值, 因爲數學上y爲正值 sum數學上一定大於x,如果小於x就是發生了溢位
-O2,溢位是未定義的,當x+y發生了溢位, sum應該要小於x,可是-O2 還是會把他判斷爲false 所以-O2檢查不到
```

> **批改**：**大致對**（盒子 2 → 2）。前兩問對：`len < used` 時回繞成很大的正數、誤判有空間；無號 `< 0` 永遠為假所以警告，`> 0` 結果不固定所以不警告。第三問只對一半：有說到「溢位是 UB」與「真的溢位時檢查抓不到」，但缺中間的機制——gcc **假設有號溢位不會發生**，於是把 `x + y < x` **代數化簡成 `y < 0`**；`y` 是正數，`y < 0` 為假，所以溢位了也抓不到。你寫的「`-O2` 還是會把它判斷為 false」只描述了結果，沒說是怎麼變成 false 的。10-27 寫出了「改寫成 `y < 0`」但缺「假設不溢位」，這次兩個都沒寫出來；這兩句要連在一起記：「**UB → gcc 假設不溢位 → 化簡成 `y < 0`**」。
>
> **參考答案**【[第七節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#七陷阱有號和無號混在一起比較)〈`-Wtype-limits` 只抓「永遠成立／永遠不成立」的比較〉、[第八節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#八溢位無號回繞有號是未定義行為)〈另一種寫法：`x + y < x` 被改寫成 `y < 0`〉】
>
> **核心關鍵**：
>
> 1. `len < used` 時，`len - used` 回繞成很大的正數，判斷誤以為「還有空間」。
> 2. 無號數 `< 0` 永遠為假，所以 gcc 警告；`> 0` 等於 `!= 0`，結果不固定，是合法寫法，所以不警告。
> 3. 有號溢位是 UB，gcc 假設它不會發生，把 `x + y < x` 改寫成 **`y < 0`**；真的溢位時（`y` 是正的），這個檢查抓不到。
>
> **詳細說明**：`len == used` 時結果是 0，`> 0` 為假，所以不是「永遠為真」，只有 `len < used` 會出錯。警告來自 `-Wtype-limits`（`-Wextra` 會開啟）。溢位那一問的記法：「`x + y < x`」在數學上（沒有溢位）跟「`y < 0`」完全等價，gcc 只是做了這個代數化簡，而這個化簡只在「不會溢位」時才成立。`sum < x` 這個寫法只在 `y ≥ 0` 時才是溢位檢查，見[第十一節第 1 章](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#1-sum--x-這個溢位檢查前提化簡的根據怎麼確定-gcc-化簡了)。正確的溢位檢查要在運算**之前**，用不會溢位的式子判斷，例如 `y > 0 && x > INT_MAX - y`。

**自我練習**（不批改）

```
因爲len, used都是unsigend long, 所以結果算出來一定是 >=0 ,所以在len < used一定會出錯,因爲數學上的結果是負值,可是算出來確實正值, 無法觸發<0的條件

因爲兩者都是無號,所以加減結果一定>=0 , < 0 永遠爲false,故警告
但是無號運算有 大於 與 等於 0的結果,因此>0的判斷會有兩種選項,不會警告

sum < x的前提是 y必須大於0, 用了最佳化, 且認爲溢位不會發生(沒定義),gcc把判斷簡化爲 y<0, 但必須要y>0纔會發生溢位,所以這個條件一定會失效
```

---

## 字元：[字元是什麼-char就是1byte的整數與ASCII和UTF-8](../html/c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.html)

### 字元 4. 情境題：你寫了一個函式，用 `char c` 一個一個 byte 讀一段 UTF-8 文字，並用 `if (c > 127)` 判斷「這是不是非 ASCII 的 byte」。在本機這個判斷會成立嗎？為什麼？該怎麼改？

> 盒子 3｜過去的作答：[s1-02 quiz.md 第 4 題](../small-project/s1-02-data-repr/quiz.md#4-情境題你寫了一個函式用-char-c-一個一個-byte-讀一段-utf-8-文字並用-if-c--127-判斷這是不是非-ascii-的-byte在本機這個判斷會成立嗎為什麼該怎麼改)、[2026-10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md#字元-4-情境題你寫了一個函式用-char-c-一個一個-byte-讀一段-utf-8-文字並用-if-c--127-判斷這是不是非-ascii-的-byte在本機這個判斷會成立嗎為什麼該怎麼改)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
本機的char是 sigend char,超過127就是負值了,所以不可能超過127,無法用c > 127判斷

把c改成unsigned char即可,因爲unsigend char最大是255
```

> **批改**：**對**（盒子 3 → 4）。兩個核心關鍵都有：本機 `char` 有號，超過 127 的 byte 讀成負數，`c > 127` 永遠不成立；改成 `unsigned char`。小修（不扣分）：可以點明 UTF-8 的非 ASCII byte 都在 `0x80`～`0xff`，正好全是會變負數的那一半；拼字 `signed`／`unsigned`。
>
> **參考答案**【[第四節](../html/c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.html#四中文字一個-byte-裝不下utf-8-用-3-個-byte)〈跟第 2 課的連結：`char` 有號，讀到中文會變負數〉、〈判斷「是不是非 ASCII 的 byte」：用哪個型別、為什麼是 127〉】
>
> **核心關鍵**：
>
> 1. 不會成立：本機 `char` 有號（−128～127），UTF-8 的非 ASCII byte 是 `0x80`～`0xff`，用 `char` 讀變成負數，`c > 127` 永遠為假。
> 2. 改成 `unsigned char c`（或比較時寫 `(unsigned char)c > 127`），byte 照 0～255 讀，`> 127` 的意思才正確。
>
> **詳細說明**：例如 `0xe5` 用 `char` 讀是 −27。`-Wextra` 的 `-Wtype-limits` 會警告「永遠為假」。不要改成 `unsigned int`：會先符號擴展成 `0xffffffe5`，判斷碰巧對、值卻是錯的；也不要寫 `c < 0`，它依賴 `char` 有號，換到 ARM 就失效。

---

## 陣列：[陣列、字串與指標運算-C不檢查邊界與用gdb找segfault](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html)

### 陣列 1. 指標的型別與指標運算

#### 陣列 1-1. `int *p` 和 `unsigned char *c` 存著同一個位址，為什麼 `p + 1` 和 `c + 1` 不一樣？

> 盒子 3｜過去的作答：[s1-03 quiz.md 1-1](../small-project/s1-03-arrays-pointers/quiz.md#1-1-int-p-和-unsigned-char-c-存著同一個位址為什麼-p--1-和-c--1-不一樣)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#陣列-1-1-int-p-和-unsigned-char-c-存著同一個位址為什麼-p--1-和-c--1-不一樣)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
int* p 位移1個單位是4byte
char *c 位移以個單位是1byte
不同
```

> **批改**：**對**。抓到：`int *` 加 1 走 4 byte、`unsigned char *` 走 1 byte。小修（不扣分）：講成通則是「`T *p` 的 `p + n` 走 `n × sizeof(T)` byte」，位址一樣、差在用什麼型別看它。
>
> **參考答案**【[第二節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#二指標運算p--1-走一個元素不是一個-byte)〈`p[i]` 就是 `*(p + i)`〉】
>
> **核心關鍵**：
>
> 1. `T *p` 的 `p + n` 走 `n × sizeof(T)` byte：`int *` 走 4，`unsigned char *` 走 1。

#### 陣列 1-2. `p[3]` 在 C 標準裡是怎麼定義的？

```
*(p+3)
```

> **批改**：**對**。抓到：`p[3]` 定義成 `*(p + 3)`。
>
> **參考答案**【[第二節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#二指標運算p--1-走一個元素不是一個-byte)〈`p[i]` 就是 `*(p + i)`〉】
>
> **核心關鍵**：
>
> 1. `p[3]` 的定義是 **`*(p + 3)`**（往後走 3 個元素，再取值）。
>
> **詳細說明**：C 標準把下標定義成 `E1[E2]` ≡ `*((E1) + (E2))`；`p + 3` 只是位址，要加 `*` 才是值。

#### 陣列 1-3. 為什麼 `3[p]` 也能編譯，而且值跟 `p[3]` 一樣？

```
3[p] = *(3+p) = *(p+3) = p[3]
```

> **批改**：**對**（陣列 1 整題 盒子 3 → 4）。抓到：`3[p]` ＝ `*(3 + p)`，加法可交換，等於 `p[3]`。
>
> **參考答案**【[第二節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#二指標運算p--1-走一個元素不是一個-byte)〈`p[i]` 就是 `*(p + i)`〉】
>
> **核心關鍵**：
>
> 1. `3[p]` ＝ `*(3 + p)`，加法可以交換，所以等於 `*(p + 3)` ＝ `p[3]`。
>
> **詳細說明**：`3[p]` 能編譯，證明 `[]` 只是「指標加法再取值」的縮寫。

### 陣列 6. 情境題：讀懂 ASan 的區間

> ASan 報告裡有一行 `[16, 40) 'buf' (line 7) <== Memory access at offset 40 overflows this variable`，上面寫著 `WRITE of size 4`。
>
> 盒子 3｜過去的作答：[s1-03 quiz.md 第 6 題](../small-project/s1-03-arrays-pointers/quiz.md#6-情境題讀懂-asan-的區間2026-09-30-新增)、[10-05 複習](./2026-10-05-gcc-陣列-GNU-C資料-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

#### 陣列 6-1. `buf` 佔幾個 byte？

```
24byte
```

> **批改**：**對**。抓到：半開區間，40 − 16 ＝ 24 byte。
>
> **參考答案**【[第七節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#七addresssanitizer在第一次越界的當下抓到它)〈`[96, 101)` 怎麼讀：半開區間、偏移與影子 byte〉】
>
> **核心關鍵**：
>
> 1. 區間以 byte 為單位、半開，長度 ＝ 40 − 16 ＝ **24 byte**。

#### 陣列 6-2. 如果它是 `int` 陣列，有幾個元素？

> 題目：ASan 報告 `[16, 40) 'buf' (line 7) <== Memory access at offset 40 overflows this variable`，`WRITE of size 4`。如果它是 `int` 陣列，有幾個元素？

```
6
```

> **批改**：**對**。抓到：24 ÷ 4 ＝ 6 個 `int`。
>
> **參考答案**【[第七節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#七addresssanitizer在第一次越界的當下抓到它)〈`[96, 101)` 怎麼讀：半開區間、偏移與影子 byte〉】
>
> **核心關鍵**：
>
> 1. 24 ÷ 4 ＝ **6 個** `int`。

#### 陣列 6-3. 這次寫到的是 `buf` 的第幾格（索引多少）？

> 題目：同上的 ASan 報告。這次寫到的是 `buf` 的第幾格（索引多少）？

```
6
```

> **批改**：**對**。抓到：(40 − 16) ÷ 4 ＝ 索引 6，剛好超出結尾一格。
>
> **參考答案**【[第七節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#七addresssanitizer在第一次越界的當下抓到它)〈`[96, 101)` 怎麼讀：半開區間、偏移與影子 byte〉】
>
> **核心關鍵**：
>
> 1. (40 − 16) ÷ 4 ＝ **索引 6**（`buf[6]`，剛好超出結尾一格）。
>
> **詳細說明**：索引 6 是典型的 `i <= n` 錯誤：迴圈多跑了一圈。

#### 陣列 6-4. 影子 byte 那一列裡，`buf` 會是幾個什麼樣的影子 byte？

> 題目：同上的 ASan 報告。影子 byte 那一列裡，`buf` 會是幾個什麼樣的影子 byte？

```
00 00 00
```

> **批改**：**對**（陣列 6 整題 盒子 3 → 4）。抓到：3 個 `00`（24 byte ÷ 每個影子 byte 管 8 byte ＝ 3 個，全部可以碰）。
>
> **參考答案**【[第七節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#七addresssanitizer在第一次越界的當下抓到它)〈`[96, 101)` 怎麼讀：半開區間、偏移與影子 byte〉】
>
> **核心關鍵**：
>
> 1. 1 個影子 byte 代表 8 個 byte；24 byte ＝ 3 組，全部可以碰 → **3 個 `00`**。
>
> **詳細說明**：影子 byte 的值 `00` 是「這 8 個 byte 都能碰」，`f1`、`f3` 是陣列左右的禁區。本機實測那一列是 `f1 f1 f1 f1 f1 f1 04 f2 00 00 00[f3]f3 f3 f3 f3`：`buf` 的 3 個 `00` 後面緊接右禁區 `f3`，`[f3]` 就是這次寫到的偏移 40。索引 6 是典型的 `i <= n` 錯誤：迴圈多跑了一圈。

---

## 組語：[x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)

### 組語 2. 寫 32 位元暫存器與擴展

> `long widen_u(unsigned x) { return x; }` 只要一個 `movl %edi, %eax`，`long widen_s(int x) { return x; }` 卻要 `movslq %edi, %rax`。
>
> 盒子 3｜過去的作答：[s1-04 quiz.md 第 2 題](../small-project/s1-04-asm-basics/quiz.md#2-寫-32-位元暫存器與擴展)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

程式（筆記檔案：[`ops.c`](../small-project/s1-04-asm-basics/ops.c) 的其中兩個函式，不需要 `main`）與編譯指令：

```c
long widen_s(int x)       { return x; }
long widen_u(unsigned x)  { return x; }
```

```sh
gcc -S -O2 -fno-asynchronous-unwind-tables ops.c -o ops.s
```

```asm
widen_s:
	endbr64                 # 控制流程保護標記，跟計算無關
	movslq	%edi, %rax      # 把 4 byte 的 %edi 有號擴展成 8 byte 放進 %rax
	ret                     # 回傳 %rax
widen_u:
	endbr64
	movl	%edi, %eax      # 把 %edi 的 4 byte 複製到 %eax
	ret
```

#### 組語 2-1. 為什麼 `movl` 就足以完成前者？

```
因爲按照規定, eax 32位元會自動用0補上32位
```

> **批改**：**對**。抓到：寫 32 位元的 `%eax` 時，CPU 自動把 `%rax` 的高 32 位元清成 0，`unsigned` → `long` 要的零擴展就順便完成了。小修（不扣分）：把前提講出來更完整——`unsigned` 變 `long` 需要的是**零擴展**（高位補 0），所以 CPU 的這條規則剛好夠用；`widen_s` 要補的是符號位元，CPU 不會自動補，才需要 `movslq`。
>
> **參考答案**【[第二節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#二暫存器cpu-裡的-16-個變數)〈寫低位的名字時，高位會怎樣？〉】
>
> **核心關鍵**：
>
> 1. `unsigned` → `long` 是**零擴展**：高 32 位元補 0。
> 2. 寫 32 位元的名字（`%eax`）時，CPU 會**自動把 `%rax` 的高 32 位元清成 0**，所以一個 `movl %edi, %eax` 就完成了零擴展。
>
> **詳細說明**：這也是擴展指令裡**沒有 `movzlq`** 的原因：4 → 8 byte 的零擴展，`movl` 就做完了。`widen_s` 要補符號位元，CPU 不會自動補，才需要 `movslq`。

#### 組語 2-2. 如果改寫 16 位元的 `%ax`，同樣的理由還成立嗎？

```
不成立, 應該要用movzwq
上面48位需要補0
```

> **批改**：**對**（組語 2 整題 盒子 3 → 4）。抓到：不成立，寫 16 位元時高位不會自動清 0，要用零擴展指令。小修（不扣分）：(1) 把原因直接寫成一句：「寫 `%ax`（或 `%al`）時，高位**保留原本的值**，只有寫 32 位元名字才會清 0」，不要只寫「需要補 0」；(2) `movzwq` 可以用，但 `movzwl %di, %eax` 就夠了——它寫的是 32 位元的 `%eax`，高 32 位元又會被自動清 0，所以 gcc 實際上都用 `movzwl`。
>
> **參考答案**【[第二節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#二暫存器cpu-裡的-16-個變數)〈寫低位的名字時，高位會怎樣？〉】
>
> **核心關鍵**：
>
> 1. 不成立。
> 2. 寫 16 位元（`%ax`）或 8 位元（`%al`）的名字時，高位**保留原本的值、不會自動清 0**；只有寫 32 位元名字才會清。所以要零擴展得用 `movzwl`／`movzbl` 這類指令。
>
> **詳細說明**：這條規則不對稱：寫 32 位元（`%eax`）→ 高 32 位元清成 0；寫 16 位元（`%ax`）、8 位元（`%al`）→ 高位不動，保留舊值。

### 組語 3. 有號與無號在哪裡決定

> `lt_s` 和 `lt_u` 的 `cmpl %esi, %edi` 一模一樣，gdb 也看到兩者的旗標完全相同（`[ SF IF ]`）。
>
> 盒子 3｜過去的作答：[s1-04 quiz.md 第 3 題](../small-project/s1-04-asm-basics/quiz.md#3-有號與無號在哪裡決定)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

實驗（筆記[第五節〈實驗：同一個 `cmp`、同一組旗標，`setl` 說是、`setb` 說否〉](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)）：用 `-1` 和 `1` 呼叫兩個函式，在 `cmp` 剛做完時用 gdb 看旗標。

檔案 [`lt.c`](../small-project/s1-04-asm-basics/lt.c)：

```c
/* lt.c：用 -O2 編 */
int lt_s(int a, int b)           { return a < b; }
int lt_u(unsigned a, unsigned b) { return a < b; }
```

檔案 [`ltmain.c`](../small-project/s1-04-asm-basics/ltmain.c)：

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

`lt.c` 編出的組合語言：

```asm
lt_s:
	endbr64                 # 控制流程保護標記
	xorl	%eax, %eax      # %eax = 0
	cmpl	%esi, %edi      # 算 a − b，只設旗標、不存結果
	setl	%al             # 依旗標把 %al 設成 1 或 0（看哪些旗標是本題要答的）
	ret                     # 回傳 %eax
lt_u:
	endbr64
	xorl	%eax, %eax
	cmpl	%esi, %edi      # 完全一樣的比較
	setb	%al             # 依旗標把 %al 設成 1 或 0（看哪些旗標是本題要答的）
	ret
```

#### 組語 3-1. 為什麼一個回傳 1、一個回傳 0？

```
lt_s:
setl 看有號
-1 -1 = -2 => SF = 1, OF=0 所以 a < b return 1

lt_u:
setb 看無號
(2^32 - 1) - 1 = 2^32 - 2 CF = 0 所以 a > b return 0
```

> **批改**：**對**。抓到：旗標一樣，差在 `setl`（有號）和 `setb`（無號）讀的旗標不同；SF ＝ 1、OF ＝ 0 → `setl` 成立回傳 1；CF ＝ 0 → `setb` 不成立回傳 0。小修（不扣分）：把條件寫出來更清楚——`setl` 看的是「SF ≠ OF」、`setb` 看的是「CF ＝ 1」；`-1 -1 = -2` 指的是 `a − b ＝ (−1) − 1`，寫成 `-1 - 1` 比較不會誤讀。
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈實驗：同一個 `cmp`、同一組旗標，`setl` 說是、`setb` 說否〉】
>
> **核心關鍵**：
>
> 1. 旗標相同，差在**讀旗標的條件碼**：`lt_s` 用 `setl`（看 SF ≠ OF），`lt_u` 用 `setb`（看 CF）。
> 2. 這次 SF = 1、OF = 0 → `setl` 成立，回傳 1；CF = 0 → `setb` 不成立，回傳 0。
>
> **詳細說明**：實驗的 `a` 是 −1（位元 `0xffffffff`）、`b` 是 1。有號看是 −1 < 1；無號看是 4294967295 < 1，不成立。`cmpl` 算出的位元與旗標只有一份，兩種讀法是 gcc 挑條件碼時決定的。

#### 組語 3-2. C 的「有號還是無號」在組合語言裡到底是在哪裡決定的？

```
組合語言沒有有號無號的概念, C語言纔有有號無號,C翻譯成組合語言的時候
根據有號無號,決定要用哪個組合的語法
```

> **批改**：**對**（組語 3 整題 盒子 3 → 4）。抓到：CPU／組合語言本身沒有有號無號，是**編譯時** gcc 依 C 的型別挑指令決定的。小修（不扣分）：「哪個組合的語法」要講具體——這題是比較之後挑哪個**條件碼**（有號 `l`／`g`、無號 `b`／`a`）；另一個看得出來的地方是變寬時挑 `movs` 還是 `movz`。
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈實驗：同一個 `cmp`、同一組旗標，`setl` 說是、`setb` 說否〉、[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈`%dil` 是 `0xff`，為什麼 `%eax` 變成 `0xffffffff`〉】
>
> **核心關鍵**：
>
> 1. 在**編譯時**由 gcc 依照 C 的型別選指令決定；CPU、暫存器、旗標裡都沒有「有號／無號」。
> 2. 這題的情境：比較之後挑哪個條件碼（有號 `l`／`g`，無號 `b`／`a`）。
>
> **詳細說明**：另一個看得出有號無號的地方是變寬時挑 `movs` 還是 `movz`。`cmp` 把兩種解讀需要的旗標（OF 給有號、CF 給無號）一次都設好，`add`、`sub` 的位元也不分有號無號。所以從組合語言反推型別時，只能看這兩種地方。

### 組語 5. 情境題：從一行反組譯推回 C

> 你在 kernel 的反組譯裡看到 `movq 24(%rdi,%rsi,8), %rax`，`%rdi` 是函式的第 1 個參數、`%rsi` 是第 2 個。推測它對應的 C 可能長怎樣。
>
> 盒子 3｜過去的作答：[s1-04 quiz.md 第 5 題](../small-project/s1-04-asm-basics/quiz.md#5-情境題從一行反組譯推回-c)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

#### 組語 5-1. 第 1 個參數最可能是指向什麼大小的元素的指標？

```
有可能是long,
因爲單位位移是8byte,而且24 剛好是3*8
```

> **批改**：**對**。抓到：比例 8 而且是 `movq`（讀 8 byte）→ 指向 8 byte 元素（`long`）的指標。小修（不扣分）：8 byte 的元素也可能是 `unsigned long` 或**指標**（kernel 很常見 `struct foo **tbl` 這種指標陣列）。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)「3. 運算元」的 `D(B,I,S)` 表】
>
> **核心關鍵**：
>
> 1. `S` ＝ 8 而且是 `movq`（讀 8 byte）→ 指向 8 byte 元素的指標（`long`、`unsigned long` 或指標）。
>
> **詳細說明**：kernel 裡很常見的是「指標的陣列」（例如 `struct foo **tbl`），元素同樣是 8 byte。

#### 組語 5-2. 存取的是第幾個元素（用 `i` 表示）？

> 題目：`movq 24(%rdi,%rsi,8), %rax` 存取的是第幾個元素（用 `i` 表示）？

```
*(a + i + 3)
```

> **批改**：**對**。抓到：`*(a + i + 3)` 就是 `a[i + 3]`（24 byte ÷ 8 byte ＝ 3 個元素）。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上表其他五列：每一種用一個實際的 C 函式對照〉】
>
> **核心關鍵**：
>
> 1. 位址 ＝ `a + i×8 + 24` ＝ `a + (i + 3)×8`，就是 `a[i + 3]`（`i` 是第 2 個參數）。
>
> **詳細說明**：D 換算成元素數要除以元素大小：24 byte ÷ 8 byte ＝ 3 個元素。筆記的 `0xc(%rdi,%rsi,4)` 是同一件事（12 ÷ 4 ＝ 3，`a[i + 3]` 的 `int` 版）。

#### 組語 5-3. 如果同一個位置看到的是 `leaq` 而不是 `movq`，C 又會是什麼？

> 題目：如果看到的是 `leaq 24(%rdi,%rsi,8), %rax` 而不是 `movq`，C 又會是什麼？

```
a + i + 3
```

> **批改**：**對**（組語 5 整題 盒子 3 → 4）。抓到：`a + i + 3`（＝ `&a[i + 3]`），結果是指標。小修（不扣分）：補一句原因——`lea` 只算位址、**不讀記憶體**，所以 C 少了那個 `*`。
>
> **參考答案**【[第四節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#四mov-與-lea搬資料-vs-算位址)〈`lea`：只算位址，不碰記憶體〉】
>
> **核心關鍵**：
>
> 1. `lea` 不讀記憶體，只算出位址 `a + i×8 + 24`。
> 2. 對應 `&a[i + 3]`（就是 `a + i + 3`），結果是指標。
>
> **詳細說明**：`movq` 版是 `return a[i + 3];`，`leaq` 版是 `return &a[i + 3];`。本機實測 `long *k(long *a, long i) { return &a[i + 3]; }` 編出 `leaq 24(%rdi,%rsi,8), %rax`。

### 組語 18. `0x01 + 0x7f` 的進位與 CF、OF

> 盒子 2｜過去的作答：2026-10-25 在對話裡作答（18-1 對、18-2 大致對、18-3 大致對、18-4 未作答），批改後的參考答案寫在[筆記第 18 題](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#自我測驗)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

#### 組語 18-1. 8 位元的 `0x01 + 0x7f`：計算過程中有沒有進位傳進第 7 位元（最高位）？有沒有從第 7 位元往外進位？

```
0x01 + 0x7f = 0x80
有傳進第7位元
沒有往外進位
```

> **批改**：**對**。抓到：進位一路傳進第 7 位、第 7 位沒有往外進位。
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面⑦：127 + 1 明明有「進位」，為什麼 CF 還是 0？〉】
>
> **核心關鍵**：
>
> 1. 有：`0x01 + 0x7f` 從第 0 位起每一位都「留 0、進 1」，進位一路傳進第 7 位。
> 2. 沒有：第 7 位是 0 + 0 + 1 ＝ 1，留下 1、不再往外進。

#### 組語 18-2. CF、OF 各是多少，為什麼「有進位」不等於 CF = 1？

> 題目：（承 18-1，8 位元的 `0x01 + 0x7f`）CF、OF 各是多少，為什麼「有進位」不等於 CF = 1？

```
CF = 0
OF = 1

因爲要往外進位纔算
```

> **批改**：**對**。抓到：CF ＝ 0、OF ＝ 1；CF 只算第 7 位往外的那一次進位。
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面⑦：127 + 1 明明有「進位」，為什麼 CF 還是 0？〉】
>
> **核心關鍵**：
>
> 1. CF ＝ 0：CF 只記**從第 7 位往外**的那一次進位，中間各位的進位不算；無號 127 + 1 ＝ 128 ≤ 255，裝得下。
> 2. OF ＝ 1。
>
> **詳細說明**：OF ＝ 1 的理由：當**有號數**算，127 + 1 ＝ +128 超出 −128～127，留下的 `0x80` 讀成 −128。

#### 組語 18-3. 再比較 `0x05 + 0x03`：它也有進位，為什麼兩個旗標都是 0？

```
首先CF = 0 , 因爲沒有借位,也沒有往外進位
在來OF = 0, 沒有溢位
```

> **批改**：**對**。抓到：沒有往外進位 → CF ＝ 0；有號沒溢位 → OF ＝ 0（跟 10-26、10-27 同樣判對）。小修（不扣分）：(1)「沒有借位」是減法的說法，加法用不到；(2) 題目強調「它也有進位」，可以補一句「進位只發生在第 1～3 位，沒傳到第 7 位，中間的進位不影響任何旗標」；OF 的理由寫成「有號 5 + 3 ＝ 8 裝得下」或「進第 7 位 0、出第 7 位 0」會更完整。
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面⑦：127 + 1 明明有「進位」，為什麼 CF 還是 0？〉】
>
> **核心關鍵**：
>
> 1. `0x05 + 0x03` 的進位只發生在第 1～3 位，**沒有傳到第 7 位**：進第 7 位 0、出第 7 位 0 → CF ＝ 0、OF ＝ 0。
> 2. 無號 8、有號 8 都裝得下；「中間有進位」本身不影響任何旗標。

#### 組語 18-4. 最後用「進到第 7 位的進位」和「第 7 位往外的進位」說明 `0xff + 0x01` 的 OF 為什麼是 0。

```
因爲進到第7位的進位是 1
第7位往外的進位也是1
1xor1 = 0 所以OF=0

也可以這樣看 0xff = -1, 0x01 = 1 -1 + 1 = 0 沒有溢位 故of = 0
```

> **批改**：**對**（組語 18 整題 盒子 2 → 3）。抓到：進第 7 位 1、出第 7 位 1，XOR 得 0；也用有號 −1 + 1 ＝ 0 沒溢位對照。
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面那條規則：OF 不是看溢位嗎？為什麼又變成看進位？〉】
>
> **核心關鍵**：
>
> 1. `0xff + 0x01`：進到第 7 位的進位是 1，第 7 位 1 + 0 + 1 ＝ 10，往外的進位也是 1；兩個一樣 → OF ＝ 0。
> 2. 對照有號的說法：−1 + 1 ＝ 0，裝得下，沒溢位；CF ＝ 1 只代表無號的 255 + 1 裝不下。
>
> **詳細說明**：OF 的定義一直是「當有號數裝不下」；「進 XOR 出」是 CPU 算出它的方法。
