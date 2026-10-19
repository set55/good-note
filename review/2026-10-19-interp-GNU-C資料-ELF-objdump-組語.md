# 2026-10-19 複習：interp、GNU、C資料、ELF、objdump、組語

> 依 [複習排程](../learning/複習排程.md) 選出今天到期的 10 題。今天只複習、不上新課（第 5 課的自我測驗還在等回覆），
> 所以全部依**過期最久 → 盒子小 → 上次作答較早**挑選：10-11 到期的 9 題（盒子 1 的 interp 2、interp 3、GNU 4、C資料 11、C資料 7，
> 盒子 2 的 ELF 4、objdump 1、組語 4、組語 13）全選；10-12 到期的盒子 1 有 6 題（上次作答都是 10-11），依排程表順序選 GNU 5，湊滿 10 題。
> 今天到期的共 62 題，沒選到的 52 題順延，日期不動。
>
> **怎麼寫**：闔上筆記，每題寫 1～2 句關鍵句就好。寫完說「幫我批改」。複習不重答，批改會直接附參考答案。
> 組語 4、組語 13 已拆成小題，每個小題一個作答框。

## 批改結果（2026-10-19）

| 題目 | 結果 | 盒子 | 下次 | 錯在哪／缺什麼 |
|---|---|---|---|---|
| interp 2 | 不完整 | 1 → 1 | 10-20 | `ld` 合併、寫 `NEEDED` 對；`ld.so` 又說成「補上缺的程式碼」（應是把位址填進 `.got`）、漏跳到 `_start`（跟 10-10 卡在同一處） |
| interp 3 | 大致對 | 1 → 1 | 10-20 | 抓到：交給直譯器的是**路徑**（10-10 說成內容，這次改正）；缺「寫的地方不同」（`#!` vs `INTERP`），也沒把 `cat` 用路徑自己開檔接回實驗 |
| GNU 4 | 大致對 | 1 → 1 | 10-20 | copyleft「修改並散布要用 GPL」、LGPL 可連結不開源都對；GPL 那句又寫成「有使用就要公開」，義務是**散布**才有（跟 10-10 同一個用詞） |
| GNU 5 | 對 | 1 → 2 | 10-22 | 抓到：DCO、聲明自己有權以 kernel 的 GPL 貢獻、出事查得到誰負責（10-11 方向說反，這次改正） |
| C資料 11 | 不完整 | 1 → 1 | 10-20 | 第一、三問對；第二問錯：本機 `char` 是**有號**的，不同型別的原因是 `char` 是標準規定的**第三個獨立型別**，不看範圍（跟 10-10 卡在同一處） |
| C資料 7 | 對 | 1 → 2 | 10-22 | 抓到：十進位清單沒有無號型別、十六進位有；兩個比較的轉換都對（10-10 理由錯，這次改正） |
| ELF 4 | 不完整 | 2 → 1 | 10-20 | 載入段是打包後的產物對；沒講兩種視角各給誰看，第二問「核心的操作機制」沒說出**權限以分頁為單位**（跟 09-27、09-29 卡在同一處） |
| objdump 1 | 大致對 | 2 → 2 | 10-22 | 三類都列出；理由的位置錯（`NEEDED` 在 dynamic section，不在檔頭），也沒說出需求單**只有直接依賴**、`ldd` 是讓載入器實際載入 |
| 組語 4-1 | 對 | | | 抓到：`andl`，`je` 前最後一個改旗標的指令 |
| 組語 4-2 | 對 | | | 抓到：`movl` 不改旗標 |
| 組語 4-3 | 對 | 2 → 3（整題） | 10-26 | 抓到：`addl` 會改旗標，`je` 變成判斷它的結果 |
| 組語 13-1 | 對 | | | 抓到：ASLR 位址會變、指令到 `g` 的相對距離固定 |
| 組語 13-2 | 對 | 2 → 3（整題） | 10-26 | 抓到：非 PIE 位址連結時固定，`0x404060` 直接當 `D` |

以大題計：對 4（GNU 5、C資料 7、組語 4、組語 13）、大致對 3（interp 3、GNU 4、objdump 1）、不完整 3（interp 2、C資料 11、ELF 4）。
升級：組語 4、組語 13 → 盒子 3（10-26）；GNU 5、C資料 7 → 盒子 2（10-22）。objdump 1 留盒子 2（10-22）；interp 3、GNU 4 留盒子 1；ELF 4 回盒子 1，interp 2、C資料 11 留盒子 1，明天（10-20）再考。
**進步**：interp 3 第一次答出「路徑」、GNU 5 的 DCO 方向改正、C資料 7 常數型別的理由改正。
**反覆卡住**：interp 2 `ld.so` 做的是「填位址」不是「補程式碼」（第二次）；C資料 11 `char` 是第三個型別（第二次，這次還把本機 `char` 說成無號）；ELF 4 權限以**分頁**為單位（第三次）。

## 今天的題目

| # | 題目 | 盒子 | 原本該考的日期 | 為什麼選 |
|---|---|---|---|---|
| 1 | interp 2 | 1 | 2026-10-11 | 過期最久、盒子 1 |
| 2 | interp 3 | 1 | 2026-10-11 | 同上 |
| 3 | GNU 4 | 1 | 2026-10-11 | 同上 |
| 4 | C資料 11 | 1 | 2026-10-11 | 同上 |
| 5 | C資料 7 | 1 | 2026-10-11 | 同上 |
| 6 | ELF 4 | 2 | 2026-10-11 | 過期最久、盒子 2 |
| 7 | objdump 1 | 2 | 2026-10-11 | 同上 |
| 8 | 組語 4（3 小題） | 2 | 2026-10-11 | 同上 |
| 9 | 組語 13（2 小題） | 2 | 2026-10-11 | 同上 |
| 10 | GNU 5 | 1 | 2026-10-12 | 次久、盒子 1，排程表順序 |

---

## interp：[ELF的interpreter是什麼-動態載入器與井字號驚嘆號](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html)

### interp 2. `ld` 和 `ld.so`（`ld-linux-x86-64.so.2`）各在什麼時間點、做什麼事？

> 盒子 1｜過去的作答：[s1-01 quiz.md 第 2 題](../small-project/s1-01-hello-compile/quiz.md#2-ld-和-ldsold-linux-x86-64so2各在什麼時間點做什麼事)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md#interp-2-ld-和-ldsold-linux-x86-64so2各在什麼時間點做什麼事)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#interp-2-ld-和-ldsold-linux-x86-64so2各在什麼時間點做什麼事)

```
ld 在連接時,負責將多個.o相同整合,並將相同權限的放到同一個載入段load,另外缺的函式庫寫入需求單(NEEDED),還有完成program header
ld.so是interpreter, 在執行時,kernel會把他載入,然後他會根據前面的需求單(NEEDED)載入函式庫,補上原本缺的程式碼
```

> **批改**：**不完整**（盒子 1 → 1）。時間點都對（連結時／執行時），`ld` 合併 `.o`、寫 `NEEDED`，`ld.so` 由核心載入、依 `NEEDED` 載入函式庫也都對。缺的兩點：
> - `ld` 最主要的工作沒寫：**決定每個符號的位址、把 `call` 的洞填上**。（「同權限放進同一個載入段」是對的，屬於詳細說明。）
> - `ld.so` 又寫成「補上原本缺的**程式碼**」，跟 10-10 錯在同一處：函式庫的程式碼映射進記憶體就在那裡了，`ld.so` 填的是 `puts` 的**位址**（填進 `.got`）。最後一步「跳到 `_start`」也又漏了。
>
> **參考答案**【[第二節](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html#二執行時發生什麼事)〈「跳進程式」是什麼意思：進入點（entry point）〉、[第 1 課第五節](../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html#五連結把洞接上但-puts-還是沒接完)】
>
> 核心關鍵：
> 1. `ld`：**連結時**（`gcc` 的最後一步，只做一次）合併各 `.o` 的區段、決定每個符號的位址、把 `call` 的洞填上。
> 2. 位址在共享函式庫裡的符號（例如 `puts`），`ld` 填不了真實位址，只寫需求單：`NEEDED libc.so.6`、`INTERP` 路徑。連結完 `ld` 就結束，執行時不會出現。
> 3. `ld.so`：**執行時**（每次執行都做）由核心先啟動，依 `NEEDED` 找到函式庫（含間接依賴）映射進記憶體。
> 4. `ld.so` 把 `puts` 等符號的真實位址填進 `.got`（接線），最後跳到程式的進入點 `_start`（不是 `main`）。
>
> 詳細說明：`ld` 也會產生程式標頭表，把同權限的區段打包成載入段；`ld.so` 載入時會檢查函式庫版本（`GLIBC_2.34` 這種需求）。「填入程式碼」的誤會：`ld.so` 不搬程式碼，函式庫的程式碼映射進記憶體就在那裡了；它填的是「去哪裡找那段程式碼」的位址。

### interp 3. 腳本的 `#!/bin/sh` 和 ELF 的 interpreter，哪裡相同、哪裡不同？用 `#!/bin/cat` 的實驗說明「核心把什麼交給直譯器」。

題目說的 `#!/bin/cat` 實驗（筆記[第四節](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html#四跟腳本的--是同一個機制)開頭）：寫一支第一行是 `#!/bin/cat` 的腳本，給它執行權限，再用 `./` 直接執行它。

腳本 `catscript` 的完整內容（兩行）：

```text
#!/bin/cat
這一行會被 cat 印出來
```

建檔與執行的指令：

```sh
printf '#!/bin/cat\n這一行會被 cat 印出來\n' > catscript
chmod +x catscript
./catscript
```

（執行結果不附，因為會洩漏答案。）

> 盒子 1｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-腳本的-binsh-和-elf-的-interpreter哪裡相同哪裡不同用-bincat-的實驗說明核心把什麼交給直譯器)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md#interp-3-腳本的-binsh-和-elf-的-interpreter哪裡相同哪裡不同用-bincat-的實驗說明核心把什麼交給直譯器)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#interp-3-腳本的-binsh-和-elf-的-interpreter哪裡相同哪裡不同用-bincat-的實驗說明核心把什麼交給直譯器)

```
相同的是kernel首先都是執行他們
不同的是,interpreter負責執行載入函式庫的工作,sh負責執行腳本

腳本的路徑
```

> **批改**：**大致對**（盒子 1 → 1）。最關鍵的一點答對了：核心交給直譯器的是**腳本的路徑**（10-10 寫成「內容」，這次改正）。相同點「核心先執行的是直譯器」也對。缺的：
> - 不同點只講了直譯器拿到檔案後做的事，沒講**寫在哪裡**：腳本寫在文字檔第一行的 `#!`，ELF 寫在程式標頭的 `INTERP`。
> - 題目要你「用 `#!/bin/cat` 的實驗說明」：要接回實驗——實際執行的是 `/bin/cat ./catscript`，`cat` 拿著路徑**自己打開檔案**，所以整個檔案（連 `#!/bin/cat` 那一行）都被印出來。
>
> **參考答案**【[第四節](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html#四跟腳本的--是同一個機制)〈交給直譯器的是「路徑」，不是內容：三個實驗〉】
>
> 核心關鍵：
> 1. 相同：核心在 `execve` 時讀到「請先執行這支程式」，先啟動直譯器，而不是檔案本身。
> 2. 不同：寫的地方（文字檔第一行的 `#!` vs 程式標頭的 `INTERP`）；直譯器拿到檔案後做的事（`sh` 逐行讀文字、執行命令 vs `ld.so` 載入函式庫、接線後跳進程式本身，之後由 CPU 執行程式的指令）。
> 3. 核心交給直譯器的是**腳本的路徑（當作命令列參數）**：實際執行的是 `/bin/cat ./catscript`，`cat` 用這個路徑自己打開檔案，才把整個檔案（含第一行）印出來。
>
> 詳細說明：兩個實驗可以證明交的是路徑、不是內容：`#!/usr/bin/echo` 的腳本印出的是 `./echoscript.sh` 這個路徑（`echo` 只印參數，不讀檔）；把腳本改成 `chmod 111`（只能執行、不能讀），錯誤訊息是 `/usr/bin/cat: ./catscript.sh: Permission denied`——`cat` 自己開檔被擋。如果核心交的是內容，`cat` 根本不必開檔，這個錯誤就不會發生。

---

## GNU：[GNU是什麼-Linux核心與GNU工具的分工](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html)

### GNU 4. GPL 的 copyleft 要求什麼？glibc 用 LGPL 而不是 GPL，對「商業軟體能不能連結 libc」有什麼差別？

> 盒子 1｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md#gnu-4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#gnu-4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)

```
copyleft 要求 當你修改與發佈後, 你還是要以gpl的授權發佈

商業軟體如果連接libc, 如果該軟體有給消費者使用,不需要進行公開源碼. gpl的話,假如你有使用,你的商業軟體也要遵守gpl的條款,以gpl的授權發佈公開源碼
```

> **批改**：**大致對**（盒子 1 → 1）。copyleft 的觸發條件「修改**並散布**後要用 GPL 發佈」寫對了；LGPL 讓商業軟體連結 libc、散布給客戶也不必公開原始碼，也對。扣在第二段的「gpl 的話，**假如你有使用**，你的商業軟體也要……公開源碼」：跟 10-10 是同一個用詞問題。只是「使用」（自己跑、公司內部用）沒有任何義務；要是 glibc 是 GPL，觸發義務的是**連結它並把程式散布出去**。第一段你自己講對了，第二段要用同一個條件。
>
> **參考答案**【[第五節](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html#五gplgnu-最深遠的影響)】
>
> 核心關鍵：
> 1. copyleft：可以自由使用、修改、散布，但**散布**修改後（或以它為基礎）的版本時，必須用**同樣的授權（GPL）**並提供原始碼。
> 2. 只是使用、或改了自己用、不給別人，沒有義務。
> 3. glibc 用 LGPL：允許非自由的程式**連結**它而不必開放自己的原始碼，所以商業軟體都能連結 `libc.so.6`；如果 glibc 是 GPL，連結它的程式在散布時就得整個用 GPL 開放。
>
> 詳細說明：copyleft 的目的是讓「自由不能在傳遞途中被拿走」：拿到 GPL 軟體的人，也要拿得到原始碼、有同樣的權利。例子：公司內部改了 GPL 的工具自己用，不必公開；把改過的版本賣給客戶，就要給客戶原始碼，並讓客戶同樣能以 GPL 再散布。

### GNU 5. 你將來送 kernel patch 時要加的 `Signed-off-by:` 跟 GPL 有什麼關係？

> 盒子 1｜過去的作答：[s1-01 quiz.md 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-你將來送-kernel-patch-時要加的-signed-off-by-跟-gpl-有什麼關係)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md#gnu-5-你將來送-kernel-patch-時要加的-signed-off-by-跟-gpl-有什麼關係)

```
這個是dco, 是說明我們開發者在進行修改與發佈,有權用這個專案的授權協議進行發佈,kernel是gpl的話,我們就有權利用gpl的授權發佈
,這在後來如果代碼有問題我們也可以找到是由誰負責.
```

> **批改**：**對**（盒子 1 → 2）。兩個核心關鍵都答到：`Signed-off-by:` 是 DCO，聲明自己**有權**以專案的授權（kernel 的 GPL）貢獻這段程式碼；出了問題查得到是誰聲明的。10-11 把方向說反，這次改正了。小提醒：kernel 的授權精確說是 **GPL-2.0**（不是「GPL 任何版本」）；「代碼有問題」指的是**著作權／授權**上的問題，不是 bug。
>
> **參考答案**【[第五節](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html#五gplgnu-最深遠的影響)】
>
> 核心關鍵：
> 1. `Signed-off-by:` 是 DCO（Developer Certificate of Origin，開發者原創聲明）：貢獻者聲明這段程式碼是自己寫的，或自己有權以專案的授權（kernel 是 GPL-2.0）貢獻它。
> 2. kernel 以 GPL-2.0 散布，每段併入的程式碼都必須能合法以 GPL-2.0 散布；DCO 讓每個 patch 都有人為來源負責，出了著作權爭議查得到是誰聲明的。
>
> 詳細說明：DCO 是 kernel 社群的規定，不是 GPL 條文本身的要求，GPL 沒有規定發布時要署名。

---

## C資料：[C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html)

### C資料 11. `unsigned x`、`unsigned int x`、`signed x` 各是什麼型別？為什麼 `char` 和 `signed char` 在本機的範圍完全一樣，gcc 卻說它們是不同型別、宣告衝突？情境題：同事在錯誤訊息裡看到 `long unsigned int`，說「我明明寫的是 `unsigned long`，gcc 是不是把型別搞錯了？」你怎麼回答？

> 盒子 1｜過去的作答：2026-10-06 在對話裡作答、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#c資料-11-unsigned-xunsigned-int-xsigned-x-各是什麼型別為什麼-char-和-signed-char-在本機的範圍完全一樣gcc-卻說它們是不同型別宣告衝突情境題同事在錯誤訊息裡看到-long-unsigned-int說我明明寫的是-unsigned-longgcc-是不是把型別搞錯了你怎麼回答)

```
unsigned, unsigned int 都是無號int
signed 是有號int

這是要看平臺,在當前linux (ubuntu)這個平臺 char是 unsigned char 所以型別不同

沒有long, long long 都是整數型別, 也可以叫他們 long int, int long順序無所謂
也可以把unsigned sigend加入 一樣順序無關
```

> **批改**：**不完整**（盒子 1 → 1）。第一問對。第三問大致對：「順序無所謂」答到了 gcc 沒搞錯的理由，最好再明說一句「`unsigned long` 和 `long unsigned int` 是**同一個型別**，gcc 只是用它自己固定的寫法印出來」。第二問**錯**，跟 10-10 卡在同一處，這次還多了一個事實錯誤：
> - 本機（x86-64）的 `char` 是**有號**的，不是 `unsigned char`。題目說「範圍跟 `signed char` 完全一樣」就是這個意思：範圍一樣，型別還是不同。
> - 所以原因不是「有號無號不同」，而是 C 標準規定 `char` 是**第三個獨立的型別**：它有號還是無號交給平台決定，標準沒辦法把它定成 `signed char` 或 `unsigned char` 其中之一。型別相不相同看標準怎麼規定，**不看範圍**。
>
> **參考答案**【[第一節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#一位元組與型別大小)〈`unsigned` 單獨寫就是 `unsigned int`：整數型別的所有寫法〉】
>
> 核心關鍵：
> 1. `unsigned x`、`unsigned int x` 都是 `unsigned int`；`signed x` 是 `int`（＝ `signed int`）。
> 2. `char` 有號還是無號，C 標準交給平台決定（x86-64 有號、ARM 通常無號），標準沒辦法把它定成 `signed char` 或 `unsigned char` 其中之一，所以 `char` 是**第三個獨立的型別**。型別相不相同看標準的規定，不看範圍。
> 3. gcc 沒搞錯：`unsigned long` 和 `long unsigned int` 是同一個型別。指定字的順序不重要、有其他指定字時 `int` 可以省略，gcc 印型別時用它自己的固定寫法（大小在前、有號無號居中、`int` 放最後）。
>
> 詳細說明：如果把 `char` 定義成 `signed char`，換到 ARM 這種 `char` 無號的平台就錯了，定成 `unsigned char` 則在 x86-64 錯，所以標準只好讓它自成一個型別。同一個函式先宣告成吃 `char`、再定義成吃 `signed char`，gcc 會報 `conflicting types`。`same.c` 實驗裡 `unsigned f(unsigned x)` 也被 gcc 印成 `unsigned int(unsigned int)`。

### C資料 7. `2147483648` 和 `0x80000000` 是同一個數，為什麼一個的型別是 `long`、一個是 `unsigned int`？`-1 < 0x80000000` 和 `-1 < 2147483648` 的結果各是什麼？為什麼？

> 盒子 1｜過去的作答：[s1-02 quiz.md 第 7 題](../small-project/s1-02-data-repr/quiz.md#7-2147483648-和-0x80000000-是同一個數為什麼一個的型別是-long一個是-unsigned-int-1--0x80000000-和--1--2147483648-的結果各是什麼為什麼)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#c資料-7-2147483648-和-0x80000000-是同一個數為什麼一個的型別是-long一個是-unsigned-int-1--0x80000000-和--1--2147483648-的結果各是什麼為什麼)

```
2147483648 是long是因爲十進制, 找型別的順序是int, long, long long, int的最大值只有2147483647, 所以是long
0x80000000 16進制是 int, unsigend int, long, unsigned long ...一樣的判斷方式int最大值是0x7fffffff 所以選unsigned int

-1 < 0x80000000 false, -1 轉成unsigned int 就是0xffffffff 所以是false
-1 < 2147483648 true, long的-1還是-1
```

> **批改**：**對**（盒子 1 → 2）。四個核心關鍵全部答到：十進位清單 `int` → `long` → `long long`、十六進位清單中間有 `unsigned int`，兩個比較的結果與轉換也都對。10-10 的理由錯，這次改正了。
>
> **參考答案**【[第四節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#四無號整數二進位與回繞)〈常數也有型別：`0u` 的 `u` 是什麼〉、〈怎麼確定一個常數的型別？比較時以誰為準？〉】
>
> 核心關鍵：
> 1. 沒有後綴的常數，依序嘗試、選第一個裝得下的型別。十進位的清單是 `int` → `long` → `long long`（永遠不變成無號），`2147483648` 超過 `INT_MAX`，所以是 `long`。
> 2. 十六進位的清單是 `int` → `unsigned int` → `long` → …，`0x80000000` 裝得進 `unsigned int`，所以是 `unsigned int`。
> 3. `-1 < 0x80000000` 為**假**：`int` 與 `unsigned int` 比，`-1` 轉成 4294967295。
> 4. `-1 < 2147483648` 為**真**：`int` 與 `long` 比，`-1` 轉成 `long` 還是 −1。
>
> 詳細說明：十六進位清單多放無號型別，是因為寫十六進位的人通常在寫位元樣式（例如遮罩 `0x80000000`），要的是那個 bit 樣式，不是負數。

---

## ELF：[ELF是什麼-可執行與可連結格式的兩種視角](../html/c/ELF是什麼-可執行與可連結格式的兩種視角.html)

### ELF 4. 區段（section）和載入段（segment）差在哪？為什麼核心載入時要把很多區段打包成少數幾個載入段？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-區段section和載入段segment差在哪為什麼核心載入時要把很多區段打包成少數幾個載入段)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#elf-4-區段section和載入段segment差在哪為什麼核心載入時要把很多區段打包成少數幾個載入段)

```
section 並沒有把要執行的要素依據權限分類打包成一塊,segment就是打包成一塊後的產物

這與核心對記憶體的操作機制有關
```

> **批改**：**不完整**（盒子 2 → 1）。第一問只答到一半：「載入段是同權限的區段打包後的產物」對，但沒講兩者是**兩種視角、給兩種讀者**——區段給連結器（`ld`、`objdump`）看、切得細；載入段給核心與動態載入器看、數量少。第二問「與核心對記憶體的操作機制有關」太模糊，等於沒答：關鍵字是**分頁**——記憶體權限最小只能以一頁（4096 byte）為單位設定，同一頁不能一半可執行、一半可寫。這一點 09-27、09-29 也卡過，是第三次。
>
> **參考答案**【[第四節](../html/c/ELF是什麼-可執行與可連結格式的兩種視角.html#四名字的關鍵兩種視角)】（2026-10-19 改寫成核心關鍵格式）
>
> 核心關鍵：
> 1. 區段（section）是**連結視角**的單位，切得很細（`hello` 有 31 個），給 `ld`、`nm`、`objdump` 用；載入段（segment）是**執行視角**的單位，數量很少（4 個 `LOAD`），給核心與動態載入器用，描述「哪一塊放進記憶體、權限是什麼」。
> 2. 記憶體權限**最小只能以分頁（本機 4096 byte）為單位**設定，同一頁不能一半可執行、一半可寫；所以 `ld` 把權限相同的區段排在一起、打包成一個載入段，核心每塊映射一次、設一種權限就能把程式放進記憶體。
>
> 詳細說明：核心載入時只在乎權限（可讀／可寫／可執行），不在乎區段叫 `.text` 還是 `.rodata`。如果不打包，權限不同的小區段擠在同一頁就沒辦法各自設權限；每個區段各占一頁又會浪費記憶體。所以打包的工作在連結時由 `ld` 做好，核心照程式標頭表映射就好。

---

## objdump：[objdump能不能代替ldd找依賴庫-需求單與實際載入](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html)

### objdump 1. `objdump -p /usr/bin/curl` 只列出 3 個 `NEEDED`，`ldd` 卻列出 33 行。多出來的是哪幾類東西？為什麼 `objdump` 看不到？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 1 題](../small-project/s1-01-hello-compile/quiz.md#1-objdump--p-usrbincurl-只列出-3-個-neededldd-卻列出-33-行多出來的是哪幾類東西為什麼-objdump-看不到)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#objdump-1-objdump--p-usrbincurl-只列出-3-個-neededldd-卻列出-33-行多出來的是哪幾類東西為什麼-objdump-看不到)

```
多出kernel的虛擬函式庫
間接函式庫
與interpreter

因爲objdump -p 只看file header,header沒有寫就沒有
```

> **批改**：**大致對**（盒子 2 → 2）。三類都列出了（間接依賴、vdso、載入器）。理由的主線「`objdump` 只看檔案裡寫的，沒寫就沒有」對，但缺兩點、錯一個位置：
> - 位置：`NEEDED` 不在檔頭（file header／ELF header），在 dynamic section（`.dynamic`）；`objdump -p` 印的是程式標頭表與 dynamic section。
> - 缺「為什麼沒寫」：連結時寫進 `curl` 的需求單**只有它自己直接要的** 3 個；間接依賴寫在 `libcurl.so.4` 自己的需求單裡。
> - 缺對照：`ldd` 是請**動態載入器實際載入一次**，才拿到完整清單——這是兩者差異的根本（題目在問「看的是不同時間點」）。
>
> **參考答案**【[第二節](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html#二為什麼會差這麼多兩者看的是不同時間點)】（2026-10-19 改寫成核心關鍵格式）
>
> 核心關鍵：
> 1. 多出來的三類：**間接依賴**（最大宗，例如 `libssl.so.3` 是 `libcurl.so.4` 要的，不是 `curl` 自己要的）、`linux-vdso.so.1`（核心提供的虛擬函式庫）、動態載入器本身。
> 2. `objdump -p` 只讀連結時寫進 `curl` 的需求單（`NEEDED`），裡面**只有直接依賴**；`ldd` 是請載入器實際跑一次，才拿到完整清單。
>
> 詳細說明：`NEEDED` 放在 dynamic section，不在檔頭。另外兩類也不會出現在 `curl` 的需求單裡：vdso 是核心在執行時直接放進行程的，沒有對應的檔案；載入器的路徑寫在 `INTERP`，不在 `NEEDED`。

---

## 組語：[x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)

### 組語 4. 旗標與指令的排列

> `-O2` 的 `sum_to` 裡，`andl $1, %edi` 和 `je .L3` 中間夾了一個 `movl $1, %eax`。

用到的程式（第 1 課的檔案：[`sumto.c`](../small-project/s1-01-hello-compile/sumto.c)）：

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

```sh
gcc -S -O2 -fno-asynchronous-unwind-tables sumto.c -o sumto2.s
```

`sum_to` 的 `-O2` 組合語言（`%edi` 是參數 `n`；註解只寫每行算什麼，不寫哪些指令改旗標，那是題目要問的）：

```asm
sum_to:
	endbr64
	testl	%edi, %edi            # 算 n & n
	jle	.L4                   # ≤ 0 就跳到 .L4（回傳 0）
	leal	1(%rdi), %ecx         # %ecx = %rdi + 1
	xorl	%edx, %edx            # %edx = 0（total）
	andl	$1, %edi              # %edi = %edi & 1
	movl	$1, %eax              # %eax = 1（i）
	je	.L3                   # 依旗標決定要不要跳到 .L3
	movl	$2, %eax              # %eax = 2
	movl	$1, %edx              # %edx = 1
	cmpl	%ecx, %eax            # 比較 %eax 與 %ecx
	je	.L1                   # 相等就跳 .L1
.L3:
	leal	1(%rdx,%rax,2), %edx  # %edx = %edx + %rax×2 + 1（一圈加兩個數）
	addl	$2, %eax              # %eax += 2
	cmpl	%ecx, %eax            # 比較 %eax 與 %ecx
	jne	.L3                   # 不相等就再一圈
.L1:
	movl	%edx, %eax            # 回傳值 = %edx
	ret
.L4:
	xorl	%edx, %edx
	movl	%edx, %eax            # 回傳 0
	ret
```

> 盒子 2｜過去的作答：[s1-04 quiz.md 第 4 題](../small-project/s1-04-asm-basics/quiz.md#4-旗標與指令的排列)

#### 組語 4-1. `je` 判斷的是哪個指令的結果？

```
andl
他是看最近一個對旗標操作的命令
```

> **批改**：**對**。`andl $1, %edi` 是 `je` 之前最後一個改旗標的指令。
>
> **參考答案**【[第七節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#七逐行讀-sum_to-的--o2-版)〈逐行〉】
>
> 核心關鍵：
> 1. `andl $1, %edi`：它是 `je` 之前最後一個會改旗標的指令。
>
> 詳細說明：`je` 看 ZF：`n & 1` 是 0（偶數）就跳到 `.L3`。

#### 組語 4-2. 為什麼 gcc 可以這樣排？

```
因爲movl不會改動旗標,所以je還是可以判斷andl
```

> **批改**：**對**。`mov` 不改旗標，夾在中間不會蓋掉 `andl` 設的旗標。
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈哪些指令會改旗標〉】
>
> 核心關鍵：
> 1. `mov` 不改旗標，夾在 `andl` 和 `je` 中間不會蓋掉 `andl` 設的旗標。
>
> 詳細說明：`mov` 系列、`lea`、跳躍都不改旗標；算術和邏輯指令（`add`、`sub`、`and`、`xor`、`cmp`、`test`）會改。gcc 利用這點把不相干的指令塞進比較和跳躍之間。

#### 組語 4-3. 如果中間夾的是 `addl $1, %eax`，會怎樣？

```
那這樣就會變成判斷addl的旗標結果
```

> **批改**：**對**（組語 4 整題盒子 2 → 3）。`addl` 會改旗標，`je` 判斷的就變成 `addl` 的結果。可以再多說一句後果：判斷的不再是「`n` 是不是偶數」，程式邏輯就錯了。
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈哪些指令會改旗標〉】
>
> 核心關鍵：
> 1. `addl` 會改旗標，`je` 判斷的就變成 `addl`（`%eax + 1` 是不是 0）的結果，不再是 `n` 是不是奇數。
>
> 詳細說明：程式邏輯就錯了，所以 gcc 不會把會改旗標的指令排在 `andl` 和 `je` 中間。

### 組語 13. `D(%rip)` 與 `D(,I,S)`

> 題目來自筆記[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上表其他五列：每一種用一個實際的 C 函式對照〉。

用到的程式（檔案：[`forms.c`](../small-project/s1-04-asm-basics/forms.c)）：

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

編成 PIE（gcc 預設）與非 PIE 兩種，看 `get_g`、`get_tbl` 的指令（`objdump` 節錄，每個函式開頭的 `endbr64` 省略）：

```sh
gcc -O2 -g forms.c -o forms_pie
objdump -d --no-show-raw-insn forms_pie
gcc -O2 -g -fno-pie -no-pie forms.c -o forms_np
objdump -d --no-show-raw-insn forms_np
```

```asm
# forms_pie（PIE）
0000000000001250 <get_tbl>:
    1254:	lea    0x2de5(%rip),%rax        # 4040 <tbl>    %rax = tbl 的位址
    125b:	mov    (%rax,%rdi,8),%rax                       # %rax = 讀 %rax + i×8 那 8 byte
0000000000001260 <get_g>:
    1264:	mov    0x2db5(%rip),%rax        # 4020 <g>      %rax = 讀 %rip + 0x2db5 那 8 byte
    126b:	ret

# forms_np（非 PIE）
0000000000401240 <get_tbl>:
  401244:	mov    0x404060(,%rdi,8),%rax                   # %rax = 讀 0x404060 + i×8 那 8 byte
```

> 盒子 2｜過去的作答：[s1-04 quiz.md 第 13 題](../small-project/s1-04-asm-basics/quiz.md#13-drip-與-dis2026-10-06-課後提問新增)

#### 組語 13-1. `get_g` 讀全域變數 `g` 用的是 `mov 0x2db5(%rip),%rax`，為什麼 PIE 程式不能直接把 `g` 的位址寫進指令，而要用 `%rip` ＋ 距離？

```
因爲pie根據aslr, 這些變數實際在記憶體的位置會隨着每一次執行一直變化,但只有這個程式碼到常數的記憶體相對位址是不變的,所以要用rip+相對距離
```

> **批改**：**對**。ASLR 讓位址每次執行都變、指令到 `g` 的相對距離固定，兩點都答到。用詞小提醒：`g` 是全域**變數**（在 `.data`），不是常數。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上表其他五列：每一種用一個實際的 C 函式對照〉】
>
> 核心關鍵：
> 1. PIE 每次執行被放在不同的基底（ASLR），連結時不知道 `g` 的絕對位址，沒辦法寫死。
> 2. 同一個檔案裡，這條指令到 `g` 的**距離固定**，所以用 `%rip`（下一條指令的位址）＋ 距離算出來。
>
> 詳細說明：`%rip` 用的是**下一條**指令的位址，所以筆記的 `get_g` 實驗要算 `$pc + 7 + 0x2db5`（`mov` 是 7 byte）。

#### 組語 13-2. 同一個 `tbl[i]`，為什麼非 PIE 的程式可以寫成一條 `mov 0x404060(,%rdi,8),%rax`？

```
從這個數字可以看出來tbl的記憶體位址就是0x404060然後隨着index直接決定記憶體要位移多少
爲什麼?因爲這是非pie的程式,也就是程式碼,常數這些記憶體位址都是固定的,直接拿來用就好
```

> **批改**：**對**（組語 13 整題盒子 2 → 3）。非 PIE 的位址在連結時就固定，`tbl` 的位址 `0x404060` 直接當 `D`，`%rdi` 乘 8 當位移。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上表其他五列：每一種用一個實際的 C 函式對照〉】
>
> 核心關鍵：
> 1. 非 PIE 在連結時位址就固定（`tbl` 在 `0x404060`），可以直接當常數 `D` 寫進指令：`D(,I,S)` ＝ `0x404060 + i×8`。
>
> 詳細說明：PIE 的程式不能這樣寫，要先用 `lea …(%rip)` 算出 `tbl` 的位址放進暫存器，再用 `(B,I,S)`，至少兩條指令。
