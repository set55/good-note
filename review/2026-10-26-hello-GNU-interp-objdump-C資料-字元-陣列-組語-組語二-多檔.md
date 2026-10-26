# 2026-10-26 複習：hello、GNU、interp、objdump、C資料、字元、陣列、組語、組語二、多檔

> 依[複習排程](../learning/複習排程.md)出今天到期的全部題目（每天出完所有到期題，2026-10-23 起的規則），共 **20 題**（41 個作答框），沒有過期的。
> 其中 17 題是 10-23 大複習後排在盒子 2 的題目（含 4 題大致對留在盒子 2：hello 5、組語 10、陣列 2、陣列 3）；組語 4、組語 13 是 10-19 答對、排在今天的盒子 3；組語 18 是 10-25 追問新增、在對話裡第一次作答後進盒子 1。
> 題目依筆記分節、節內照題號排；每題照原題複製，用到的程式碼與實驗步驟都附在題目裡，「過去的作答」連到之前每一次作答（含批改與參考答案）。
>
> **怎麼寫**：闔上筆記，每題寫 1～2 句關鍵句就好。可以分幾次寫，寫完一部分就可以先說「幫我批改」。複習不重答，批改會直接附參考答案。
> 已拆小題的題目，每個小題一個作答框；以大題為單位排程，小題全對才算對。

## 批改結果（2026-10-26）

| 題目 | 結果 | 盒子 | 下次 | 錯在哪／缺什麼 |
|---|---|---|---|---|
| hello 5 | 大致對 | 2 → 2 | 10-29 | 缺「版本需求是**連結時** `ld` 寫進執行檔的」（跟 10-23 缺在同一處）；`libc.so.6` 找得到，缺的是版本符號 |
| GNU 4 | 大致對 | 2 → 2 | 10-29 | 缺 copyleft 的「散布時要用**同樣的授權（GPL）**」，只寫了公開原始碼（10-23 有寫） |
| interp 2 | 對 | 2 → 3 | 11-02 | 抓到：`ld` 連結時合併、重定位、寫需求單；`ld.so` 執行時載入、接 `.got`、跳 `_start` |
| interp 3 | 對 | 2 → 3 | 11-02 | 抓到：先啟動直譯器、寫的地方不同、交的是路徑；自己用 `/bin/cat ./catscript` 對照證明 |
| objdump 3 | 大致對 | 2 → 2 | 10-29 | 缺這個情境下 `ldd` 的結果 `libfoo.so => not found`，只寫了 `ldd` 一般做什麼 |
| C資料 3 | 對 | 2 → 3 | 11-02 | 抓到：轉成無號 2³² − 1；`-Wall` 不會、要 `-Wextra`（`-Wsign-compare`） |
| C資料 8 | 大致對 | 2 → 2 | 10-29 | 來源型別寫成 `int`，應是 `signed char`（10-23 寫對過）；其餘對 |
| C資料 11 | 對 | 2 → 3 | 11-02 | 抓到：三個型別、`char` 是第三個獨立型別、`long unsigned int` 就是 `unsigned long` |
| 字元 3 | 對 | 2 → 3 | 11-02 | 抓到：4、2、4 與各自的理由 |
| 陣列 2-1 | 大致對 | | | 40／8 對；但說 `sizeof a` 時 `a`「退化」，`sizeof` 正是**不退化**的例外 |
| 陣列 2-2 | 對 | | | 抓到：要另外傳長度 |
| 陣列 2-3 | 大致對 | 2 → 2（整題） | 10-29 | 型別對；缺 `a + 1` 走 4 byte、`&a + 1` 走 40 byte；`&a` 又說成「退化」 |
| 陣列 3-1 | 對 | | | 抓到：堆疊、`.rodata` |
| 陣列 3-2 | 對 | | | 抓到：堆疊可寫、`.rodata` 沒有 `w`（兩次說成「堆疊在 `.data`」這次改正） |
| 陣列 3-3 | 對 | 2 → 3（整題） | 11-02 | 抓到：拿位址、`info proc mappings` 看權限 |
| 組語 1-1 | 對 | | | 抓到：`mov` 讀記憶體、`lea` 只存位址 |
| 組語 1-2 | 對 | | | 抓到：`*(p + i)` 與 `p + i`，沒有多乘 4 |
| 組語 1-3 | 對 | 2 → 3（整題） | 11-02 | 抓到：`lea` 只算數字、`x + 2x` |
| 組語 4-1 | 對 | | | 抓到：`andl $1, %edi` |
| 組語 4-2 | 對 | | | 抓到：中間不能夾改旗標的指令（`mov` 不改） |
| 組語 4-3 | 對 | 3 → 4（整題） | 11-09 | 抓到：變成判斷 `addl` 的結果 |
| 組語 6-1 | 對 | | | 抓到：歷史命名；C 的 `long` 由平台決定 |
| 組語 6-2 | 對 | | | 抓到：`q` |
| 組語 6-3 | 對 | | | 抓到：不對，`l` 不是 gdb 的大小字母 |
| 組語 6-4 | 大致對 | 2 → 2（整題） | 10-29 | `w` 對；缺「為什麼」：gdb 的 `b/h/w/g` ＝ 1/2/4/8，`w` 是 4 byte |
| 組語 10-1 | 對 | | | 抓到：複製數字 vs 當位址去讀 |
| 組語 10-2 | 對 | | | 抓到：`a` 與 `*a` |
| 組語 10-3 | 對 | 2 → 3（整題） | 11-02 | 抓到：`deref` 會、1234 沒有對映（10-23 缺的這次有了） |
| 組語 12-1 | 大致對 | | | 檔案位址執行時沒對映對；缺「`*deref+4` 有符號、載入後 gdb 用新位址重算」（跟 10-13、10-22 缺在同一處） |
| 組語 12-2 | 對 | 2 → 2（整題） | 10-29 | 抓到：非 PIE 不搬，會成功 |
| 組語 13-1 | 對 | | | 抓到：ASLR 位址不固定、相對距離固定 |
| 組語 13-2 | 對 | 3 → 4（整題） | 11-09 | 抓到：非 PIE 位址固定，直接當 `D` |
| 組語 18-1 | 對 | | | 抓到：進位傳進第 7 位、沒有往外 |
| 組語 18-2 | 大致對 | | | 值對；缺理由：CF 只看第 7 位往外、OF 是有號 127 + 1 裝不下（10-25 也缺） |
| 組語 18-3 | 對 | | | 抓到：沒有往外進位、沒有溢位 |
| 組語 18-4 | 不完整 | 1 → 1（整題） | 10-27 | 沒用兩個進位說明；「進 XOR 出」是 CPU 算出有號溢位的方法，不是另一個意思 |
| 組語二 5-1 | 對 | | | 抓到：序言還沒跑、參數在 `%rdi`／`%rsi`／`%rdx` |
| 組語二 5-2 | 對 | 2 → 3（整題） | 11-02 | 抓到：`%rsp` 那格是返回位址 |
| 多檔 1-1 | 對 | | | 抓到：不知道型別、預設 `int`、只拿 32 位元 |
| 多檔 1-2 | 對 | | | 抓到：只靠名字、沒有型別 |
| 多檔 1-3 | 對 | 2 → 3（整題） | 11-02 | 抓到：`cltq` 把 `%eax` 擴展成 `long`，是猜成 `int` 的結果 |

> 總計 20 題：對 12、大致對 7（hello 5、GNU 4、objdump 3、C資料 8、陣列 2、組語 6、組語 12）、不完整 1（組語 18）。

## 今天的題目

| # | 題目 | 盒子 | 原本該考的日期 |
|---|---|---|---|
| 1 | 組語 18（4 小題） | 1 | 2026-10-26 |
| 2 | hello 5 | 2 | 2026-10-26 |
| 3 | GNU 4 | 2 | 2026-10-26 |
| 4 | interp 2 | 2 | 2026-10-26 |
| 5 | interp 3 | 2 | 2026-10-26 |
| 6 | objdump 3 | 2 | 2026-10-26 |
| 7 | C資料 3 | 2 | 2026-10-26 |
| 8 | C資料 8 | 2 | 2026-10-26 |
| 9 | C資料 11 | 2 | 2026-10-26 |
| 10 | 字元 3 | 2 | 2026-10-26 |
| 11 | 陣列 2（3 小題） | 2 | 2026-10-26 |
| 12 | 陣列 3（3 小題） | 2 | 2026-10-26 |
| 13 | 組語 1（3 小題） | 2 | 2026-10-26 |
| 14 | 組語 6（4 小題） | 2 | 2026-10-26 |
| 15 | 組語 10（3 小題） | 2 | 2026-10-26 |
| 16 | 組語 12（2 小題） | 2 | 2026-10-26 |
| 17 | 組語二 5（2 小題） | 2 | 2026-10-26 |
| 18 | 多檔 1（3 小題） | 2 | 2026-10-26 |
| 19 | 組語 4（3 小題） | 3 | 2026-10-26 |
| 20 | 組語 13（2 小題） | 3 | 2026-10-26 |

---

## hello：[hello.c怎麼變成能跑的程式-編譯的四個階段](../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html)

### hello 5. 情境題：你把一支動態連結的程式複製到另一台機器，執行時出現找不到 `libc.so.6` 某個版本符號的錯誤。從本課的哪個階段可以解釋這件事？如果改用 `-static` 編譯，這個問題還會發生嗎？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-情境題你把一支動態連結的程式複製到另一台機器執行時出現找不到-libcso6-某個版本符號的錯誤從本課的哪個階段可以解釋這件事如果改用--static-編譯這個問題還會發生嗎)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#hello-5-情境題你把一支動態連結的程式複製到另一台機器執行時出現找不到-libcso6-某個版本符號的錯誤從本課的哪個階段可以解釋這件事如果改用--static-編譯這個問題還會發生嗎)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
執行階段,interpreter會從dynamic section發現需要填補的NEEDED 庫,然後會從interpreter預設的資料夾路徑去找
這個庫,另外symbol table (readelf -s) 也會顯示要引入的庫版本, 顯然在另外一臺機器應該是找不到libc.so.6的相容版本

不會,static會連接的時候直接把用到的.a直接一起合併到執行檔,不需要動態載入so
```

> **批改**：**大致對**。`-static` 不會發生（libc 的程式碼已經在執行檔裡、不再找外部的 `libc.so.6`），執行時由動態載入器檢查、報錯，這兩點對。缺第 1 個核心關鍵：版本需求是**連結時** `ld` 寫進執行檔的——你寫「symbol table 也會顯示要引入的庫版本」，看得出你知道它在執行檔裡，但沒講是誰、在什麼時候寫進去的（跟 10-23 缺在同一處；10-23 的自我練習裡你寫對了，這次闔上筆記又沒寫出來）。另外「找不到 `libc.so.6` 的相容版本」說得不精確：`libc.so.6` 是**找得到**的，只是它沒有提供執行檔要求的那個**版本符號**（例如 `GLIBC_2.34`）。小修（不扣分）：版本需求是 `readelf -V` 看的 `.gnu.version_r`，`readelf -s` 只會在符號名後面帶 `@GLIBC_…`。
>
> **參考答案**【[第九節](../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html#九作業解說四題背後的原因)〈自我測驗第 5 題延伸：「找不到版本符號」是怎麼回事〉】
>
> **核心關鍵**：
>
> 1. **連結時**：`ld` 把需要的符號版本（例如 `GLIBC_2.34`）寫進執行檔。
> 2. **執行時**：動態載入器檢查目標機器的 `libc.so.6` 有沒有提供這個版本，沒有就報 `` version `GLIBC_2.34' not found ``。
> 3. `-static` 不會發生：libc 的程式碼已經整份複製進執行檔，不再找外部的 `libc.so.6`。
>
> **詳細說明**：`readelf -V hello` 看得到版本需求；真正的門檻是 2.34（`__libc_start_main`），不是 `puts` 的 2.2.5。`-static` 的代價是檔案大，而且 libc 的安全修補要重新編譯才會帶進去。

---

## GNU：[GNU是什麼-Linux核心與GNU工具的分工](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html)

### GNU 4. GPL 的 copyleft 要求什麼？glibc 用 LGPL 而不是 GPL，對「商業軟體能不能連結 libc」有什麼差別？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md#gnu-4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#gnu-4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
要求修改並且發佈(傳播)到外面時,需要公開原始碼

LGPL 允許開發人員可以連接glibc 開發人員的程式可以發佈而不用公開源代碼
但GPL要求一旦連接,只要發佈出去就需要公開原始碼
```

> **批改**：**大致對**。「散布出去才要公開原始碼」、連結 LGPL 的 glibc 散布時不必公開、連結 GPL 的庫就要公開，這幾點對。缺 copyleft 最核心的那一半：散布時必須**用同樣的授權（GPL）**——不只是「公開原始碼」，拿到的人也要能以 GPL 再修改、再散布（10-23 你寫了「一樣是要以 gpl 的授權」，這次漏了）。只寫「公開原始碼」的話，跟「改完要附原始碼、但可以加別的限制」的授權分不出來；copyleft 的重點是自由不能在傳遞途中被拿走。
>
> **參考答案**【[第五節](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html#五gplgnu-最深遠的影響)】
>
> **核心關鍵**：
>
> 1. copyleft：可以自由使用、修改、散布，但**散布**修改後（或以它為基礎）的版本時，必須用**同樣的授權（GPL）**並提供原始碼。
> 2. 只是使用、或改了自己用、不給別人，沒有義務。
> 3. glibc 用 LGPL：允許非自由的程式**連結**它而不必開放自己的原始碼，所以商業軟體都能連結 `libc.so.6`；如果 glibc 是 GPL，連結它的程式在散布時就得整個用 GPL 開放。
>
> **詳細說明**：copyleft 的目的是讓「自由不能在傳遞途中被拿走」。例子：公司內部改了 GPL 的工具自己用，不必公開；把改過的版本賣給客戶，就要給客戶原始碼，並讓客戶同樣能以 GPL 再散布。

---

## interp：[ELF的interpreter是什麼-動態載入器與井字號驚嘆號](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html)

### interp 2. `ld` 和 `ld.so`（`ld-linux-x86-64.so.2`）各在什麼時間點、做什麼事？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 2 題](../small-project/s1-01-hello-compile/quiz.md#2-ld-和-ldsold-linux-x86-64so2各在什麼時間點做什麼事)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md#interp-2-ld-和-ldsold-linux-x86-64so2各在什麼時間點做什麼事)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#interp-2-ld-和-ldsold-linux-x86-64so2各在什麼時間點做什麼事)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
ld 在連接時,負責把所有.o組合起來, section(區段)按權限組成load(載入段),重定位符號,分配位址.若有動態分享庫沒法定位,則在dynamic section寫上需求單 並且寫上program header

ld.so在執行時, 會給每個符號,程式碼定位真實的記憶體位址,並且按需求單 載入需要用到的庫,並連到.got,最後在跳到_start的函式
```

> **批改**：**對**。抓到：`ld` 在連結時合併 `.o`、把區段依權限組成載入段、決定位址並做重定位，填不了的共享函式庫符號只在動態區段寫需求單；`ld.so` 在執行時載入需求單上的函式庫、把位址接進 `.got`，最後跳到 `_start`。小修（不扣分）：(1) `ld.so` 是**核心先啟動**的，這句可以寫出來；(2)「給每個符號、程式碼定位真實的記憶體位址」——`ld.so` 不搬、不定位程式碼，函式庫映射進記憶體後程式碼就在那裡，它填的是 `.got` 裡「去哪裡找」的位址。
>
> **參考答案**【[第二節](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html#二執行時發生什麼事)〈「跳進程式」是什麼意思：進入點（entry point）〉、[第 1 課第五節](../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html#五連結把洞接上但-puts-還是沒接完)】
>
> **核心關鍵**：
>
> 1. `ld`：**連結時**（`gcc` 的最後一步，只做一次）合併各 `.o` 的區段、決定每個符號的位址、把 `call` 的洞填上。
> 2. 位址在共享函式庫裡的符號（例如 `puts`），`ld` 填不了真實位址，只寫需求單：`NEEDED libc.so.6`、`INTERP` 路徑。連結完 `ld` 就結束，執行時不會出現。
> 3. `ld.so`：**執行時**（每次執行都做）由核心先啟動，依 `NEEDED` 找到函式庫（含間接依賴）映射進記憶體。
> 4. `ld.so` 把 `puts` 等符號的真實位址填進 `.got`（接線），最後跳到程式的進入點 `_start`（不是 `main`）。
>
> **詳細說明**：`ld.so` 不搬程式碼，函式庫的程式碼映射進記憶體就在那裡了；它填的是「去哪裡找那段程式碼」的位址。載入時也會檢查函式庫版本（`GLIBC_2.34` 這種需求）。

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

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-腳本的-binsh-和-elf-的-interpreter哪裡相同哪裡不同用-bincat-的實驗說明核心把什麼交給直譯器)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md#interp-3-腳本的-binsh-和-elf-的-interpreter哪裡相同哪裡不同用-bincat-的實驗說明核心把什麼交給直譯器)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#interp-3-腳本的-binsh-和-elf-的-interpreter哪裡相同哪裡不同用-bincat-的實驗說明核心把什麼交給直譯器)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
相同的是,他們都是核心一開始執行的程式
不同的是,核心看的啓動文字不同,sh是用#!, interpreter是把路徑寫在program header
做的事不同 核心給sh 腳本路徑,sh讀並執行腳本, interpreter啓動後,根據dynamic section 裏的需求單載入庫

 ~/work/Projects/github/good-note/review | on main >17 !2 ?1  ./catscript                                                                                                         ok | base py | at 10:45:42
#!/bin/cat
這一行會被 cat 印出來


~/work/Projects/github/good-note/review | on main >17 !2 ?1  /bin/cat ./catscript                                                                                                ok | base py | at 10:46:39
#!/bin/cat
這一行會被 cat 印出來

兩者的輸出相同


根據實驗結果, 我們可以看出來核心在啓動 /bin/cat後,是將腳本路徑交給了cat
```

> **批改**：**對**。抓到：相同點（核心先啟動直譯器，而不是檔案本身）、寫的地方不同（`#!` vs 程式標頭）、核心交給 `cat` 的是**腳本的路徑**。這次還自己多做了一步對照：`./catscript` 和 `/bin/cat ./catscript` 輸出一模一樣，正好證明核心實際執行的就是 `/bin/cat ./catscript`——比只跑一次更有說服力。小修（不扣分）：ELF 那邊寫在程式標頭的是 `INTERP` 那一項；另外 `review/catscript` 是你的實驗檔，目前沒進 git，要不要留看你。
>
> **參考答案**【[第四節](../html/c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.html#四跟腳本的--是同一個機制)、〈交給直譯器的是「路徑」，不是內容：三個實驗〉】
>
> **核心關鍵**：
>
> 1. 相同：核心在 `execve` 時讀到「請先執行這支程式」，先啟動直譯器，而不是檔案本身。
> 2. 不同：寫的地方（文字檔第一行的 `#!` vs 程式標頭的 `INTERP`）。
> 3. 核心交給直譯器的是**腳本的路徑（當作命令列參數）**：實際執行的是 `/bin/cat ./catscript`，`cat` 用這個路徑自己打開檔案，才把整個檔案（含第一行）印出來。
>
> **詳細說明**：另一個不同點是直譯器拿到檔案後做的事：`sh` 逐行讀文字、執行命令；`ld.so` 載入函式庫、接線後跳進程式本身，之後由 CPU 執行程式的指令。兩個實驗可以證明交的是路徑、不是內容：`#!/usr/bin/echo` 的腳本印出的是 `./echoscript.sh` 這個路徑（`echo` 只印參數，不讀檔）；把腳本改成 `chmod 111`（只能執行、不能讀），錯誤訊息是 `/usr/bin/cat: ./catscript.sh: Permission denied`——`cat` 自己開檔被擋。

---

## objdump：[objdump能不能代替ldd找依賴庫-需求單與實際載入](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html)

### objdump 3. 一支程式連結時用了 `-L./lib -lfoo`，編譯成功；執行時卻出現 `error while loading shared libraries: libfoo.so`。用「連結時」與「執行時」分別是誰在找檔案，解釋為什麼連結找得到、執行卻找不到。這時 `objdump -p` 與 `ldd` 各會顯示什麼？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-一支程式連結時用了--llib--lfoo編譯成功執行時卻出現-error-while-loading-shared-libraries-libfooso用連結時與執行時分別是誰在找檔案解釋為什麼連結找得到執行卻找不到這時-objdump--p-與-ldd-各會顯示什麼)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
連接時, ld 可以根據命令列給他的參數 -L 的路徑去找 libfoo.so
但執行時,ld.so是從dynamic section 裏面看需求單,而需求單只有寫明庫的名稱,ld.so需要自己去預設的搜尋路徑去找,而這些預設路徑裏面沒有libfoo.so

objudmp -p 會顯示這個執行檔直接用到的shared object file,也就是dynamic section的需求單的函式庫
ldd啓動interpreter後,會去看執行檔的dynamic section 需求單的函式庫
也會去看這些so(so本身也是elf)的dynamic section 找他們需要的函式庫(間接的函式庫,對於執行檔),然後繼續遞迴找
還會顯示interpreter
與linux預設的虛擬函式庫
```

> **批改**：**大致對**。連結時 `ld` 照 `-L` 找到、執行時 `ld.so` 只看需求單上的**名字**、去自己的預設路徑找不到，`objdump -p` 列出 `NEEDED`，這幾點都對（「需求單只寫名稱」這次主動寫出來了）。缺第 3 個核心關鍵的後半：題目問的是**這個情境下** `ldd` 會顯示什麼——答案是 `libfoo.so => not found`。你寫的是 `ldd` 一般會做什麼（遞迴找間接依賴、列出 interpreter 和 vDSO），都對，但沒有回到題目的結果（10-23 你有寫「最後會顯示 not found」）。小修（不扣分）：「linux 預設的虛擬函式庫」正式名稱是 vDSO（`linux-vdso.so.1`），它是核心直接映射進來的，不是檔案。
>
> **參考答案**【[第四節](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html#四找不到函式庫時兩者各看到什麼)、[第二節](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html#二為什麼會差這麼多兩者看的是不同時間點)】
>
> **核心關鍵**：
>
> 1. 連結時是 **`ld`** 在找：照 `-L./lib` 找到 `libfoo.so`，在執行檔寫下 `NEEDED libfoo.so`，**只有名字、沒有路徑**。
> 2. 執行時是**動態載入器**在找：它不知道當初的 `-L`，只找自己的預設路徑（加上 `LD_LIBRARY_PATH` 等），所以找不到。
> 3. `objdump -p` 照樣列出 `NEEDED libfoo.so`（看不出問題）；`ldd` 顯示 `libfoo.so => not found`。
>
> **詳細說明**：執行時的搜尋順序是：執行檔裡的 `RUNPATH`、`LD_LIBRARY_PATH`、`/etc/ld.so.cache`、預設目錄（`/lib`、`/usr/lib`）。修法是連結時加 `-Wl,-rpath,…` 把路徑寫進執行檔，或執行時設 `LD_LIBRARY_PATH`。

---

## C資料：[C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html)

### C資料 3. `int n = -1; unsigned int len = 5;`，`n < len` 為什麼是假？只加 `-Wall` 編譯，gcc 會不會提醒你？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 3 題](../small-project/s1-02-data-repr/quiz.md#3-int-n---1-unsigned-int-len--5n--len-為什麼是假只加--wall-編譯gcc-會不會提醒你)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

**題目用到的程式**：筆記[第七節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#七陷阱有號和無號混在一起比較) `signs.c` 的最後一段（`n < len` 那段），抽出來成為能單獨編譯的版本：

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

```sh
$ gcc -Wall -o signcmp signcmp.c
$ ./signcmp
```

> 題目：`int n = -1; unsigned int len = 5;`，`n < len` 為什麼是假？只加 `-Wall` 編譯，gcc 會不會提醒你？

```
有號要轉成無號, 所以 -1 -> 2^32 -1 > 5因此爲假

不會,要加-Wextra, 因爲Wextra包含了-Wsign-compare
```

> **批改**：**對**。抓到：有號和無號比較時 `n` 被轉成無號，`-1` 變成 2³² − 1 > 5；只加 `-Wall` 不會提醒，要 `-Wextra`（裡面的 `-Wsign-compare`，選項名這次寫對了）。
>
> **參考答案**【[第四節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#四無號整數二進位與回繞)〈怎麼確定一個常數的型別？比較時以誰為準？〉、[第六節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#六有號與無號之間的轉換位元不變換個讀法)、[第七節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#七陷阱有號和無號混在一起比較)】
>
> **核心關鍵**：
>
> 1. 比較之前做一般算術轉換：`int` 和 `unsigned int` 等級相同，無號贏，`n` 被轉成 `unsigned int`。
> 2. 轉換時位元不變、換成無號讀法：`-1`（`0xffffffff`）變成 4294967295 > 5，所以 `n < len` 為假。
> 3. 只加 `-Wall` **不會**提醒（C 的 `-Wall` 不含 `-Wsign-compare`），要加 `-Wextra`。
>
> **詳細說明**：這個轉換是定義好的，不是溢位，也不是未定義行為；未定義行為的是有號整數**運算**溢位（第八節）。加 `-Wextra` 印出 `comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]`。

### C資料 8. `signed char c = -1; unsigned int u = c;` 之後 `u` 是多少？擴展時新補的高位是 0 還是 1，由哪個型別決定？如果把 `c` 改宣告成 `unsigned char c = 0xff;`，`u` 又是多少？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 8 題](../small-project/s1-02-data-repr/quiz.md#8-signed-char-c---1-unsigned-int-u--c-之後-u-是多少擴展時新補的高位是-0-還是-1由哪個型別決定如果把-c-改宣告成-unsigned-char-c--0xffu-又是多少)、[10-05 複習](./2026-10-05-gcc-陣列-GNU-C資料-字元.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
u = 2^32 -1

1, 由來源類型 int決定

255, unsigned 擴展補0
```

> **批改**：**大致對**。`u` ＝ 2³² − 1、補 1、「由來源型別決定」、`unsigned char` 補 0 得 255，這幾點對。錯在一處：來源型別是 **`signed char`**，不是 `int`——`c` 宣告成 `signed char`，擴展是從 1 byte 擴成 4 byte，符號位是 `c` 自己的第 7 位元。如果來源已經是 `int`，就沒有「擴展」這回事了（`int` 轉 `unsigned int` 位元一個都不動）。10-23 你寫對了「signed char」。
>
> **參考答案**【[第六節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#六有號與無號之間的轉換位元不變換個讀法)】
>
> **核心關鍵**：
>
> 1. `u` = **4294967295**（`0xffffffff`，2³² − 1）。
> 2. 擴展時補什麼由**原本的型別**決定：`signed char` 是有號的，做符號擴展，符號位是 1 就補 1；補完再以無號讀。
> 3. 改成 `unsigned char c = 0xff`：原本的型別無號 → 零擴展補 0 → `0x000000ff` = **255**。
>
> **詳細說明**：順序是「先依來源型別擴展到 32 bit，再換成目標型別的讀法」。目標型別 `unsigned int` 是無號的，並不會讓擴展改成補 0。

### C資料 11. `unsigned x`、`unsigned int x`、`signed x` 各是什麼型別？為什麼 `char` 和 `signed char` 在本機的範圍完全一樣，gcc 卻說它們是不同型別、宣告衝突？情境題：同事在錯誤訊息裡看到 `long unsigned int`，說「我明明寫的是 `unsigned long`，gcc 是不是把型別搞錯了？」你怎麼回答？

> 盒子 2｜過去的作答：2026-10-06 在對話裡作答、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#c資料-11-unsigned-xunsigned-int-xsigned-x-各是什麼型別為什麼-char-和-signed-char-在本機的範圍完全一樣gcc-卻說它們是不同型別宣告衝突情境題同事在錯誤訊息裡看到-long-unsigned-int說我明明寫的是-unsigned-longgcc-是不是把型別搞錯了你怎麼回答)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
unsigned = unsigned int
unsigned int就是無號整數類型
signed = signed int = int

因爲char在不同的平臺有可能被當作無號/有號,所以C標準把char當成第三個類型

沒有搞錯,因爲long 其實就是long int,另外順序也沒有一定
```

> **批改**：**對**。抓到：`unsigned`＝`unsigned int`、`signed`＝`int`；`char` 的有號無號由平台決定，所以是第三個獨立的型別；`long unsigned int` 就是 `unsigned long`，順序不重要、`int` 可以省略。小修（不扣分）：可以補一句「型別相不相同看標準的規定，不看範圍」，這正是題目「範圍一樣卻衝突」要點破的地方。
>
> **參考答案**【[第一節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#一位元組與型別大小)〈`unsigned` 單獨寫就是 `unsigned int`：整數型別的所有寫法〉】
>
> **核心關鍵**：
>
> 1. `unsigned x`、`unsigned int x` 都是 `unsigned int`；`signed x` 是 `int`（＝ `signed int`）。
> 2. `char` 有號還是無號，C 標準交給平台決定（x86-64 有號、ARM 通常無號），標準沒辦法把它定成 `signed char` 或 `unsigned char` 其中之一，所以 `char` 是**第三個獨立的型別**。型別相不相同看標準的規定，不看範圍。
> 3. gcc 沒搞錯：`unsigned long` 和 `long unsigned int` 是同一個型別。指定字的順序不重要、有其他指定字時 `int` 可以省略，gcc 印型別時用它自己的固定寫法（大小在前、有號無號居中、`int` 放最後）。

---

## 字元：[字元是什麼-char就是1byte的整數與ASCII和UTF-8](../html/c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.html)

### 字元 3. `sizeof('A')`、`sizeof("A")`、`sizeof("字")` 在本機各是多少？分別解釋為什麼。

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 3 題](../small-project/s1-02-data-repr/quiz.md#3-sizeofasizeofasizeof字-在本機各是多少分別解釋為什麼)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

```
'A' 是 int 4byte
"A" 是字符串,也就是1char+\0 char = 2byte
"字" UTF8是3個char + \0 char = 4byte
```

> **批改**：**對**。抓到：`'A'` 是 `int` 所以 4；`"A"` 是 `'A'`＋`'\0'` 共 2；「字」的 UTF-8 是 3 byte＋`'\0'` 共 4。連續兩次答對了。
>
> **參考答案**【[第三節](../html/c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.html#三c-的字元常數單引號而且型別是-int)、[第四節](../html/c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.html#四中文字一個-byte-裝不下utf-8-用-3-個-byte)】
>
> **核心關鍵**：
>
> 1. `sizeof('A')` = 4：在 C 裡，字元常數 `'A'` 的型別是 `int`（C++ 才是 `char`）。
> 2. `sizeof("A")` = 2：`'A'` 加上結尾的 `'\0'`。
> 3. `sizeof("字")` = 4：UTF-8 用 3 byte 表示「字」，再加上 `'\0'`。
>
> **詳細說明**：字串常數是 `char` 陣列，`sizeof` 量的是整個陣列，含 `'\0'`；`strlen` 才不算 `'\0'`（`strlen("字")` 是 3）。「字」的 UTF-8 是 `e5 ad 97`。

---

## 陣列：[陣列、字串與指標運算-C不檢查邊界與用gdb找segfault](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html)

### 陣列 2. 陣列傳進函式之後

> 盒子 2｜過去的作答：[s1-03 quiz.md 第 2 題](../small-project/s1-03-arrays-pointers/quiz.md#2-1-int-a10-在-main-裡-sizeof-a-是-40傳進-void-fint-a-之後-sizeof-a-變成-8為什麼)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

**題目用到的程式**：筆記[第三節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#三陣列名稱什麼時候變成指標什麼時候不會)的 `decay.c`（筆記用 `int a[5]`，題目用 `int a[10]`，道理相同）：

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

```sh
$ gcc -Wall -Wextra -g -O0 -o decay decay.c
$ ./decay
```

#### 陣列 2-1. `int a[10];` 在 `main` 裡 `sizeof a` 是 40，傳進 `void f(int a[])` 之後 `sizeof a` 變成 8。為什麼？

```
宣告 int a[10] -> 用 sizeof a, a會退化成整個陣列,因此size of a 這時=40

參數 int a[] 相當於指標,因此是8byte
```

> **批改**：**大致對**。40 是整個陣列、參數 `int a[]` 其實是指標所以 8，這兩點對。但「`sizeof a` 時 `a` 會**退化**成整個陣列」把詞用反了：**退化（decay）**指的是陣列名稱變成「指向第一個元素的指標」；`sizeof a` 正好是**不退化**的例外，`a` 還是陣列本身，所以才量得到 40。這跟 10-23 的 2-3 小修、今天的 2-3 是同一個混淆（見 2-3 的批改）。
>
> **參考答案**【[第三節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#三陣列名稱什麼時候變成指標什麼時候不會)〈函式參數寫成 `int a[]`，其實就是 `int *a`〉】
>
> **核心關鍵**：
>
> 1. 在 `main` 裡，`sizeof a` 是「陣列名稱不退化」的例外，量的是整個陣列：10 × 4 = 40 byte。
> 2. 參數寫成 `int a[]` 其實就是 `int *a`，`f(a)` 傳過去的只是 `a[0]` 的位址，所以 `f` 裡的 `sizeof a` 是指標的 8 byte。
>
> **詳細說明**：gcc 對這種寫法有 `-Wsizeof-array-argument` 警告。

#### 陣列 2-2. 這件事對「怎麼設計接收陣列的函式」有什麼影響？

> 題目：這件事（陣列傳進函式之後 `sizeof a` 從 40 變成 8）對「怎麼設計接收陣列的函式」有什麼影響？

```
需要新增陣列長度作爲參數,因爲int a[]只有指標的功能
```

> **批改**：**對**。抓到：函式只拿到指標、不知道長度，要另外傳長度參數。
>
> **參考答案**【[第三節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#三陣列名稱什麼時候變成指標什麼時候不會)〈函式參數寫成 `int a[]`，其實就是 `int *a`〉】
>
> **核心關鍵**：
>
> 1. 函式不知道陣列多長，**長度一定要另外當參數傳**（例如 `long sum(int *p, int n)`）。
>
> **詳細說明**：呼叫端用 `sizeof a / sizeof a[0]` 算長度，標準函式庫的 `memcpy`、`read` 也是這樣設計的；字串是例外，靠結尾的 `\0`。

#### 陣列 2-3. `a` 和 `&a` 印出來的位址一樣，它們差在哪裡？

> 題目：`a` 和 `&a` 印出來的位址一樣，它們差在哪裡？（`int a[10];`）

```
a 的類型 還是int* a int的指標
*(a + 1) = a[1]

&a 會退化成 int[10]* 類型變成int[10] 的指標了
```

> **批改**：**大致對**。型別這一半對：`a` 是指向 `int` 的指標，`&a` 是指向整個 `int[10]` 的指標。缺第 2 個核心關鍵：型別不同**造成的差別**——`a + 1` 走 4 byte、`&a + 1` 走 40 byte（10-23 你有寫 40 byte，這次沒寫；題目問「差在哪」，這就是印出來看得到的差別）。另外 `&a` 又寫成「退化」：`&a` 跟 `sizeof a` 一樣是**不退化**的例外，`&` 直接作用在陣列本身，才得到「指向整個陣列」的指標；會退化的是單獨寫的 `a`（變成 `&a[0]`）。兩次都把「退化」用在不退化的地方，建議記成：**退化 ＝ 變成指向第一個元素的指標；`sizeof`、`&` 是兩個不退化的例外**。
>
> **參考答案**【[第三節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#三陣列名稱什麼時候變成指標什麼時候不會)〈`a` 和 `&a`：位址一樣，型別不一樣〉】
>
> **核心關鍵**：
>
> 1. 位址相同，但指的東西不同：`a` 退化成指向第一個元素（`int`）的指標，`&a` 是指向**整個陣列**的指標。
> 2. 所以 `a + 1` 走 4 byte（一個 `int`），`&a + 1` 走 40 byte（一整個陣列）。
>
> **詳細說明**：用型別的話說，`a` 是 `int *`，`&a` 是 `int (*)[10]`。

### 陣列 3. `char s1[]` 與 `char *s2`

> 盒子 2｜過去的作答：[s1-03 quiz.md 第 3 題](../small-project/s1-03-arrays-pointers/quiz.md#3-1-char-s1--hi-和-char-s2--hi-的-hi-分別在記憶體的哪裡)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

**題目用到的程式**：`strings.c`（檔案：[`strings.c`](../small-project/s1-03-arrays-pointers/strings.c)，筆記[第四節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#四字串以-0-結尾的-char-陣列)）：

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

```sh
$ gcc -Wall -g -O0 -o strings strings.c
$ ./strings
$ gdb ./strings          # 3-3 用
```

#### 陣列 3-1. `char s1[] = "hi";` 和 `char *s2 = "hi";` 的 `"hi"` 分別在記憶體的哪裡？

```
s1: 放在stack上
s2: 放在.rodata上
```

> **批改**：**對**。抓到：`s1` 的 `"hi"` 在堆疊、`s2` 指向的在 `.rodata`。
>
> **參考答案**【[第四節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#四字串以-0-結尾的-char-陣列)】
>
> **核心關鍵**：
>
> 1. `char s1[] = "hi";`：`"hi"` 在**堆疊**上（`s1` 是一個 3 byte 的區域陣列）。
> 2. `char *s2 = "hi";`：`"hi"` 是字串常數，在 **`.rodata`**；`s2` 只是指向它的指標。
>
> **詳細說明**：`s1` 的 3 個 byte 是從字串常數**複製**進陣列的，是 `s1` 自己的一份，所以能改。

#### 陣列 3-2. 為什麼改 `s1[0]` 沒事，改 `s2[0]` 卻 segfault？

> 題目：為什麼改 `s1[0]` 沒事，改 `s2[0]` 卻 segfault？

```
stack的權限有w
.rodata沒有
```

> **批改**：**對**。抓到：堆疊那一頁有 `w`、`.rodata` 沒有，寫入被拒絕（10-09、10-23 兩次說成「堆疊在 `.data`」，這次改正了）。小修（不扣分）：可以補完因果鏈——`.rodata` 那頁**有對映、只是不能寫**，CPU 拒絕寫入，核心送出 `SIGSEGV`。
>
> **參考答案**【[第五節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#五c-不檢查邊界越界的三種下場)開頭的表格】
>
> **核心關鍵**：
>
> 1. `s1` 的副本在堆疊上，那一頁是 `rw-p`，可以寫。
> 2. `s2` 指向 `.rodata`，分頁權限是 `r--p`：**有對映**但**不允許寫**，CPU 拒絕，核心送出 `SIGSEGV`。
>
> **詳細說明**：C 標準只說修改字串常數是未定義行為；本機會 segfault，是因為 gcc 把字串常數放進唯讀分頁。

#### 陣列 3-3. 你會怎麼在 gdb 裡確認 3-2 的答案？

> 題目：你會怎麼在 gdb 裡確認 3-2 的答案（改 `s1[0]` 沒事、改 `s2[0]` 卻 segfault 的原因）？

```
先找出 s1, s2存的記憶體位址,然後在用info proc mappings去看落得區段,看權限情況
```

> **批改**：**對**（陣列 3 整題盒子 2 → 3）。抓到：拿到 `s1`、`s2` 的位址，用 `info proc mappings` 看落在哪一段、權限有沒有 `w`。
>
> **參考答案**【[第四節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#四字串以-0-結尾的-char-陣列)】
>
> **核心關鍵**：
>
> 1. 讓程式當在 `s2[0] = 'H';`，用 `print s2` 拿到位址。
> 2. 用 `info proc mappings` 找到那個位址所在的那一段，看它的**權限是 `r--p`**。
>
> **詳細說明**：拿來對照的話，`print &s1` 落在 `[stack]`（`rw-p`）。`x s2` 是去看 `s2` 指向的內容，要拿位址用 `print s2` 比較直接。

---

## 組語：[x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)

### 組語 1. `mov` 與 `lea`

> `movl (%rdi,%rsi,4), %eax` 和 `leaq (%rdi,%rsi,4), %rax` 括號裡寫的一模一樣。
>
> 盒子 2｜過去的作答：[s1-04 quiz.md 第 1 題](../small-project/s1-04-asm-basics/quiz.md#1-mov-與-lea)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

（`%rdi` 是第 1 個參數、`%rsi` 是第 2 個參數。）

#### 組語 1-1. 兩個指令做的事差在哪？

```
mov 把前面算出的記憶體位址存的那格數值 寫入eax
lea 把前面算得記憶體位址直接存到eax
```

> **批改**：**對**。抓到：`mov` 拿算出來的位址去記憶體讀那一格，`lea` 只把位址本身存進去。
>
> **參考答案**【[第四節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#四mov-與-lea搬資料-vs-算位址)〈`lea`：只算位址，不碰記憶體〉】
>
> **核心關鍵**：
>
> 1. `movl` 先算出位址 `a + i×4`，再**去記憶體讀**那個位址的 4 byte，放進 `%eax`。
> 2. `leaq` 只把算出來的**位址數字本身**放進 `%rax`，**不讀記憶體**。
>
> **詳細說明**：括號寫法一樣，代表兩個指令算出同一個數字 D ＋ B ＋ I × S；差別只在要不要拿這個數字去存取記憶體。因為 `lea` 根本不存取，位址不合法也不會 segfault。

#### 組語 1-2. 各對應 C 的什麼寫法？

> 題目：`movl (%rdi,%rsi,4), %eax` 和 `leaq (%rdi,%rsi,4), %rax` 各對應 C 的什麼寫法？

```
*(rdi + rsi) 

rdi + rsi
```

> **批改**：**對**。抓到：一個多了 `*`、一個沒有，而且沒有再多乘 4（`*(p + i)` 的指標加法本身就會乘 4）。小修（不扣分）：題目問 C 的寫法，用 C 的變數名比較清楚，例如 `int *a` 在 `%rdi`、`long i` 在 `%rsi` 時是 `a[i]`（＝`*(a + i)`）與 `&a[i]`（＝`a + i`）；寫成暫存器名字時要記得 `rdi` 必須是 `int *`，`rdi + rsi` 才會自動乘 4。
>
> **參考答案**【[第四節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#四mov-與-lea搬資料-vs-算位址)〈`lea`：只算位址，不碰記憶體〉】
>
> **核心關鍵**：
>
> 1. `movl (%rdi,%rsi,4), %eax` ↔ `a[i]`（就是 `*(a + i)`，`a` 是 `int *`）。
> 2. `leaq (%rdi,%rsi,4), %rax` ↔ `&a[i]`（就是 `a + i`，結果是指標）。
>
> **詳細說明**：括號裡的 `a + i×4` 是 `a + i` 換算成 byte 的算法；`mov` 比 `lea` 多做的就是那個 `*`。

#### 組語 1-3. 為什麼 gcc 會拿 `lea` 來算 `x * 3` 這種跟位址無關的東西？

`long times3(long x) { return x * 3; }` 用 `gcc -S -O2` 編出來是：

```asm
times3:
	endbr64                       # 給 CPU 的控制流程保護標記，跟計算無關
	leaq	(%rdi,%rdi,2), %rax       # 這一行就是本題要解釋的
	ret                           # 回傳 %rax
```

> 題目：為什麼 gcc 會拿 `lea` 來算 `x * 3` 這種跟位址無關的東西？

```
把記憶體運算的記過直接寫入eax, 相當於純數字計算
而 x*3 = x + 2x, 可以lea直接算完
```

> **批改**：**對**（組語 1 整題盒子 2 → 3）。抓到：`lea` 只算數字、不碰記憶體，`x * 3` ＝ `x + 2x` 一條 `lea` 就算完。小修（不扣分）：「記過」是「結果」的錯字；另外 `lea` **不改旗標**、結果能放到另一個暫存器，是它比 `imul`／`add` 好用的兩個附帶理由。
>
> **參考答案**【[第四節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#四mov-與-lea搬資料-vs-算位址)〈`lea` 被拿來做算術〉】
>
> **核心關鍵**：
>
> 1. `lea` 只算 D ＋ B ＋ I × S 這個數字、不碰記憶體，所以算的東西不必是位址。
> 2. `x * 3` ＝ `x + x×2`，正好寫成 `leaq (%rdi,%rdi,2), %rax`，一個指令做完。
>
> **詳細說明**：另外兩個好處：`lea` **不改旗標**（`add` 會），而且結果可以放到另一個暫存器。能一次做到的倍數是 1、2、3、4、5、8、9 倍。

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

> 盒子 3｜過去的作答：[s1-04 quiz.md 第 4 題](../small-project/s1-04-asm-basics/quiz.md#4-旗標與指令的排列)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)

#### 組語 4-1. `je` 判斷的是哪個指令的結果？

```
andl $1, %edi  
```

> **批改**：**對**。抓到：`andl $1, %edi`。
>
> **參考答案**【[第七節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#七逐行讀-sum_to-的--o2-版)〈逐行〉】
>
> **核心關鍵**：
>
> 1. `andl $1, %edi`：它是 `je` 之前最後一個會改旗標的指令。
>
> **詳細說明**：`je` 看 ZF：`n & 1` 是 0（偶數）就跳到 `.L3`。

#### 組語 4-2. 為什麼 gcc 可以這樣排？

```
計算旗標的運算元 與 je這類的指令之間, 不能再放其他也是計算旗標的運算元
```

> **批改**：**對**。抓到：設旗標的指令和 `je` 之間不能夾別的會改旗標的指令——言下之意是 `movl` 不改旗標，所以可以夾。小修（不扣分）：直接把那句寫出來最清楚：「**`mov` 不改旗標**，夾在中間不會蓋掉 `andl` 設的 ZF」；「運算元」在這裡應該是「指令」（運算元是指令後面的 `$1`、`%edi`）。
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈哪些指令會改旗標〉】
>
> **核心關鍵**：
>
> 1. `mov` 不改旗標，夾在 `andl` 和 `je` 中間不會蓋掉 `andl` 設的旗標。
>
> **詳細說明**：`mov` 系列、`lea`、跳躍都不改旗標；算術和邏輯指令（`add`、`sub`、`and`、`xor`、`cmp`、`test`）會改。gcc 利用這點把不相干的指令塞進比較和跳躍之間。

#### 組語 4-3. 如果中間夾的是 `addl $1, %eax`，會怎樣？

```
變成判斷addl的旗標結果
```

> **批改**：**對**（組語 4 整題盒子 3 → 4）。抓到：`addl` 會改旗標，`je` 判斷的就變成 `addl` 的結果。小修（不扣分）：可以補後果——判斷的變成「`%eax + 1` 是不是 0」，不再是「`n` 是不是偶數」，程式邏輯就錯了。
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈哪些指令會改旗標〉】
>
> **核心關鍵**：
>
> 1. `addl` 會改旗標，`je` 判斷的就變成 `addl`（`%eax + 1` 是不是 0）的結果，不再是 `n` 是不是偶數。
>
> **詳細說明**：程式邏輯就錯了，所以 gcc 不會把會改旗標的指令排在 `andl` 和 `je` 中間。

### 組語 6. 組合語言的大小名稱 vs C 的型別

> 盒子 2｜過去的作答：[s1-04 quiz.md 第 6 題](../small-project/s1-04-asm-basics/quiz.md#6-組合語言的大小名稱-vs-c-的型別2026-10-06-課後提問新增)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

#### 組語 6-1. `movsbl` 的 `l` 叫 long，為什麼是 4 byte，而本機 C 的 `long` 是 8 byte？

```
組合語言的long是4byte 是因爲歷史原因,而爲了向下兼容沿用至今

C標準只要求long至少32位元,而我的本機ubuntu是定8byte
```

> **批改**：**對**。抓到：組合語言的 `l` 是歷史命名、為了相容沿用；C 標準只要求 `long` 至少 32 位元，本機是 8 byte。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈為什麼 `l` 叫 long 卻是 4 byte〉】
>
> **核心關鍵**：
>
> 1. 組合語言的 `b`／`w`／`l`／`q` 是 **x86 的歷史命名**：8086 是 16 位元，word ＝ 2 byte；32 位元叫 long（兩個 word）＝ 4 byte；為了相容，名字一直沒改。
> 2. C 的 `long` 大小**由平台決定**（標準只要求至少 32 位元），本機 x86-64 Linux 是 LP64，`long` ＝ 8 byte。兩者只是剛好同名。

#### 組語 6-2. 本機 C 的 `long` 在組合語言裡對到哪個字尾？

```
q
```

> **批改**：**對**。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈為什麼 `l` 叫 long 卻是 4 byte〉】
>
> **核心關鍵**：
>
> 1. `q`（quad word，8 byte）。

#### 組語 6-3. 這樣對嗎？

> 題目：情境題：第 5 課你在 `gdb` 裡想看一個 `int` 變數的 4 個 byte，同學說「`int` 在組合語言是 `l`，所以用 `x/1l`」。這樣對嗎？

```
不對 gdb裏面沒有l, 他把gdb跟組合語言搞混了
```

> **批改**：**對**。抓到：不對，`l` 是組合語言的字尾，不是 gdb `x` 的大小字母。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈為什麼 `l` 叫 long 卻是 4 byte〉的「第三套名字：`gdb` 的 `x` 命令」】
>
> **核心關鍵**：
>
> 1. 不對：gdb `x` 的大小字母是另一套，`l` 不是它的大小字母（實測 `Undefined output format "l".`）。

#### 組語 6-4. 應該用哪個大小字母，為什麼？

> 題目：（承上）在 gdb 裡用 `x` 看 `int` 變數的 4 個 byte，應該用哪個大小字母，為什麼？

```
可以用w
x/1xw可以這樣看看
```

> **批改**：**大致對**。用 `w`、`x/1xw` 對。缺題目問的「為什麼」：gdb `x` 的大小字母是 `b`／`h`／`w`／`g` ＝ 1／2／4／8 byte，**在 gdb 裡 `w` 是 4 byte**，正好是一個 `int`（10-23 你有寫「4 byte」）。這點正是 6-3 陷阱的解法：同一個字 word，在 AT&T 組合語言是 2 byte、在 gdb 是 4 byte，只記字母不記 byte 數就會搞混。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈為什麼 `l` 叫 long 卻是 4 byte〉的「第三套名字：`gdb` 的 `x` 命令」】
>
> **核心關鍵**：
>
> 1. gdb `x` 的大小字母是 `b`／`h`／`w`／`g` ＝ 1／2／4／8 byte。
> 2. 一次看整個 `int` 用 **`w`**（4 byte，`x/1xw &v`）；用 `b` 要寫個數 4（`x/4xb &v`），逐 byte 印。
>
> **詳細說明**：同一個字 word：AT&T 是 2 byte，gdb 是 4 byte。記法是一律換算成 byte 數再想。

### 組語 10. 暫存器 vs 記憶體運算元

> `mov %rdi,%rax` 和 `mov (%rdi),%rax` 只差一對括號。
>
> 盒子 2｜過去的作答：[s1-04 quiz.md 第 10 題](../small-project/s1-04-asm-basics/quiz.md#10-暫存器-vs-記憶體運算元2026-10-06-課後提問新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

用到的程式（檔案：[`paren.c`](../small-project/s1-04-asm-basics/paren.c)）：

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

```sh
gcc -O2 -g paren.c -o paren
objdump -d --no-show-raw-insn paren
```

```asm
copy_val:
	endbr64
	mov	%rdi,%rax       # 沒有括號（第 1 個參數在 %rdi）
	ret                 # 回傳 %rax
deref:
	endbr64
	mov	(%rdi),%rax     # 有括號
	ret
```

#### 組語 10-1. 兩個指令各拿 `%rdi` 裡的數字做什麼？

```
mov %rdi, %rax => 把rdi寄存器的值直接寫到rax
mov (%rdi), %rax => 把rdi存的記憶體位址上的那格值寫到rax
```

> **批改**：**對**。抓到：第一個複製數字本身，第二個把它當位址去記憶體讀那一格。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上表第一列 `(B)` 的 `(%rdi)`：`%rdi` 和 `(%rdi)` 差在哪？〉】
>
> **核心關鍵**：
>
> 1. `mov %rdi,%rax`：把 `%rdi` 裡的**數字本身**複製到 `%rax`，不碰記憶體。
> 2. `mov (%rdi),%rax`：把那個數字**當成位址**，去記憶體讀那裡的 8 byte，放進 `%rax`。

#### 組語 10-2. 各對應 C 的什麼寫法？

> 題目：`mov %rdi,%rax` 和 `mov (%rdi),%rax` 各對應 C 的什麼寫法？

```
a
*a
```

> **批改**：**對**。抓到：`a` 與 `*a`——括號就是 C 的 `*`。小修（不扣分）：對照 `paren.c` 的話是 `return x;` 與 `return *p;`。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上表第一列 `(B)` 的 `(%rdi)`：`%rdi` 和 `(%rdi)` 差在哪？〉】
>
> **核心關鍵**：
>
> 1. `%rdi` ↔ 用變數的值 `x`（`return x;`）。
> 2. `(%rdi)` ↔ 用指標指向的值 `*p`（`return *p;`）。
>
> **詳細說明**：括號就是 C 的 `*`。

#### 組語 10-3. 如果 `%rdi` 裡是 1234，哪一個會 segfault，為什麼？

```
deref

rdi 1234 在記憶體區段很有可能無對映
```

> **批改**：**對**（組語 10 整題盒子 2 → 3）。抓到：`deref` 那條 `mov (%rdi),%rax` 會 segfault，因為 1234 這個位址沒有對映（10-23 缺的「沒有對映分頁」這次有了）。小修（不扣分）：可以講得更肯定——1234（`0x4d2`）落在位址空間最底下，Linux 刻意不讓那一區對映（`vm.mmap_min_addr` 預設 65536），所以不是「很有可能」，是一定沒有。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上表第一列 `(B)` 的 `(%rdi)`：`%rdi` 和 `(%rdi)` 差在哪？〉】
>
> **核心關鍵**：
>
> 1. `mov (%rdi),%rax` 會 segfault；第一個只是複製數字，不會。
> 2. 它把 1234（`0x4d2`）當成位址去讀，行程在那個位址**沒有對映任何分頁**，所以收到 SIGSEGV。
>
> **詳細說明**：筆記實測：`set $rdi = 1234` 後 `stepi`，得到 `Program received signal SIGSEGV, Segmentation fault.`。

### 組語 12. 檔案裡的位址 vs 執行時的位址

> 用 `objdump -d paren` 查到 `deref+4` 在 `0x11f4`，`break *0x11f4` 之後 `run` 卻得到 `Cannot access memory at address 0x11f4`，`break *deref+4` 就沒問題。
>
> 盒子 2｜過去的作答：[s1-04 quiz.md 第 12 題](../small-project/s1-04-asm-basics/quiz.md#12-檔案裡的位址-vs-執行時的位址2026-10-06-課後實作新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

實驗步驟（程式同組語 11 的 `paren.c`）：

```sh
gcc -O2 -g paren.c -o paren
objdump -d --no-show-raw-insn paren        # deref 的 mov (%rdi),%rax 那一行印成 11f4
gdb -batch -ex 'break *0x11f4' -ex 'break *deref+4' -ex 'info breakpoints' -ex run -ex 'info breakpoints' ./paren
```

#### 組語 12-1. 為什麼？

> 題目：`break *0x11f4` 之後 `run` 得到 `Cannot access memory at address 0x11f4`，`break *deref+4` 卻沒問題，為什麼？

```
因爲對gdb來說break * 後面是運算式,算出要跳的位址

而0x11f4 是執行檔還沒啓動時,的虛擬位址,實際上可能根本找不到映射的區段
```

> **批改**：**大致對**。`0x11f4` 是執行前（檔案裡）的位址、執行時那裡沒有對映，這半對。缺第 2 個核心關鍵：為什麼 `*deref+4` 就沒問題——因為它**有符號名稱**，程式載入後 gdb 用 `deref` 的**新位址**（PIE 被搬到基底 `0x555555554000` 之後）重算成 `0x5555555551f4`；寫死的 `0x11f4` 只是數字，不會重算。你寫的「`*` 後面是運算式」是 11 題的觀念，對，但兩個中斷點都是運算式，差別在**運算式裡有沒有符號**（跟 10-13、10-22 缺在同一處，10-23 寫到了、這次又漏）。另外第一半也可以點名原因：是 **PIE** 執行時整支被搬到基底，才讓檔案位址失效。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上面中斷點 4 的 `break *0x11f4`：為什麼 `run` 之後失敗？〉】
>
> **核心關鍵**：
>
> 1. `objdump -d` 印的是**檔案裡的位址**；PIE 執行時整支被搬到基底（gdb 裡是 `0x555555554000`），執行時位址 ＝ 基底 ＋ 檔案位址，`0x11f4` 在執行時沒有對映。
> 2. `*deref+4` 裡有符號名稱，程式載入後 gdb 用新的位址重算成 `0x5555555551f4`；寫死的 `0x11f4` 只是數字，不會重算。

#### 組語 12-2. 如果程式是用 `-no-pie` 編的，`break *` 加 `objdump` 查到的位址會不會成功？

```
那就沒問題了,因爲-no-pie的記憶體位址在編譯階段就定了,所以執行檔加載入記憶體,位址不會變
```

> **批改**：**對**。抓到：非 PIE 的位址連結時就定了，載入時不搬，檔案位址就是執行時位址。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上面中斷點 4 的 `break *0x11f4`：為什麼 `run` 之後失敗？〉】
>
> **核心關鍵**：
>
> 1. 會成功：非 PIE（`Type` 是 EXEC）固定放在檔案寫的位址，不會搬，檔案裡的位址就是執行時的位址。

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

> 盒子 3｜過去的作答：[s1-04 quiz.md 第 13 題](../small-project/s1-04-asm-basics/quiz.md#13-drip-與-dis2026-10-06-課後提問新增)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)

#### 組語 13-1. `get_g` 讀全域變數 `g` 用的是 `mov 0x2db5(%rip),%rax`，為什麼 PIE 程式不能直接把 `g` 的位址寫進指令，而要用 `%rip` ＋ 距離？

```
因爲PIE遵守ASLR, 每個符號的位址不是固定的,但相對位址是固定的,所以我們可以找g距離%rip的距離,然後算出來就好了
```

> **批改**：**對**。抓到：PIE 的基底每次不同（ASLR），絕對位址連結時不知道；但指令到 `g` 的距離固定，用 `%rip` ＋ 距離算。小修（不扣分）：`%rip` 是**下一條**指令的位址，距離是從那裡算的。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上表其他五列：每一種用一個實際的 C 函式對照〉】
>
> **核心關鍵**：
>
> 1. PIE 每次執行被放在不同的基底（ASLR），連結時不知道 `g` 的絕對位址，沒辦法寫死。
> 2. 同一個檔案裡，這條指令到 `g` 的**距離固定**，所以用 `%rip`（下一條指令的位址）＋ 距離算出來。
>
> **詳細說明**：`%rip` 用的是**下一條**指令的位址，所以筆記的 `get_g` 實驗要算 `$pc + 7 + 0x2db5`（`mov` 是 7 byte）。

#### 組語 13-2. 同一個 `tbl[i]`，為什麼非 PIE 的程式可以寫成一條 `mov 0x404060(,%rdi,8),%rax`？

```
因爲tbl的位址在編譯的時候就決定了,所以他可以直接用tbl的位址0x404060
```

> **批改**：**對**（組語 13 整題盒子 3 → 4）。抓到：非 PIE 連結時位址就固定，`tbl` 的 `0x404060` 直接當 `D` 寫進指令。小修（不扣分）：精確說是**連結**時決定（編譯成 `.o` 時還不知道）。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上表其他五列：每一種用一個實際的 C 函式對照〉】
>
> **核心關鍵**：
>
> 1. 非 PIE 在連結時位址就固定（`tbl` 在 `0x404060`），可以直接當常數 `D` 寫進指令：`D(,I,S)` ＝ `0x404060 + i×8`。
>
> **詳細說明**：PIE 的程式不能這樣寫，要先用 `lea …(%rip)` 算出 `tbl` 的位址放進暫存器，再用 `(B,I,S)`，至少兩條指令。

### 組語 18. `0x01 + 0x7f` 的進位與 CF、OF

> 盒子 1｜過去的作答：2026-10-25 在對話裡作答（18-1 對、18-2 大致對、18-3 大致對、18-4 未作答），批改後的參考答案寫在[筆記第 18 題](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#自我測驗)

#### 組語 18-1. 8 位元的 `0x01 + 0x7f`：計算過程中有沒有進位傳進第 7 位元（最高位）？有沒有從第 7 位元往外進位？

```
有
沒有
```

> **批改**：**對**。抓到：有進位一路傳進第 7 位；第 7 位沒有再往外進。
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
```

> **批改**：**大致對**。CF ＝ 0、OF ＝ 1 兩個值都對。缺題目問的「為什麼」：(1) CF 只記**從第 7 位往外**的那一次進位，中間各位的進位不算——18-1 你自己答了「沒有從第 7 位往外進」，這就是 CF ＝ 0 的理由，要把兩件事接起來；(2) OF ＝ 1 是因為當**有號數**算 127 + 1 ＝ +128，超出 −128～127，留下的 `0x80` 被讀成 −128。（10-25 在對話裡也是值對、理由缺。）
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面⑦：127 + 1 明明有「進位」，為什麼 CF 還是 0？〉】
>
> **核心關鍵**：
>
> 1. CF ＝ 0：CF 只記**從第 7 位往外**的那一次進位，中間各位的進位不算；無號 127 + 1 ＝ 128 ≤ 255，裝得下。
> 2. OF ＝ 1：當**有號數**算，127 + 1 ＝ +128 超出 −128～127，留下的 `0x80` 讀成 −128。

#### 組語 18-3. 再比較 `0x05 + 0x03`：它也有進位，為什麼兩個旗標都是 0？

```
沒有第7爲往外進位 CF = 0
沒有溢位 OF = 0
```

> **批改**：**對**。抓到：沒有從第 7 位往外進位 → CF ＝ 0；當有號數也沒溢位 → OF ＝ 0。小修（不扣分）：可以再點出「`0x05 + 0x03` 的進位只在第 1～3 位，**根本沒傳到第 7 位**」，這是它跟 18-1 的 `0x01 + 0x7f` 不同的地方。
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面⑦：127 + 1 明明有「進位」，為什麼 CF 還是 0？〉】
>
> **核心關鍵**：
>
> 1. `0x05 + 0x03` 的進位只發生在第 1～3 位，**沒有傳到第 7 位**：進第 7 位 0、出第 7 位 0 → CF ＝ 0、OF ＝ 0。
> 2. 無號 8、有號 8 都裝得下；「中間有進位」本身不影響任何旗標。

#### 組語 18-4. 最後用「進到第 7 位的進位」和「第 7 位往外的進位」說明 `0xff + 0x01` 的 OF 為什麼是 0。

```
不理解進位跟溢位的關係
在我的理解OF就是看有沒有溢位
```

> **批改**：**不完整**（組語 18 整題回到盒子 1）。「OF 就是看有沒有（有號）溢位」這句是對的，那是 OF 的**定義**；但題目要的是用兩個進位說明 `0xff + 0x01` 的 OF 為什麼是 0，這部分沒有作答。這個疑問 10-25 已經補進筆記〈上面那條規則：OF 不是看溢位嗎？為什麼又變成看進位？〉，重點只有一句：**「進 XOR 出」不是 OF 的另一個意思，是 CPU 算出「有沒有有號溢位」的方法**——CPU 只有一個加法器、手上沒有「有號的正確答案」可比，只看得到各位的進位。`ofrule.c` 把 65536 組全部比過，兩種算法 0 組不一致。
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面那條規則：OF 不是看溢位嗎？為什麼又變成看進位？〉】
>
> **核心關鍵**：
>
> 1. `0xff + 0x01`：進到第 7 位的進位是 1，第 7 位 1 + 0 + 1 ＝ 10，往外的進位也是 1；兩個一樣 → OF ＝ 0。
> 2. 對照有號的說法：−1 + 1 ＝ 0，裝得下，沒溢位；CF ＝ 1 只代表無號的 255 + 1 裝不下。
>
> **詳細說明**：OF 的定義一直是「當有號數裝不下」；「進 XOR 出」是 CPU 算出它的方法。為什麼會一致：同號相加時「進 ≠ 出」剛好就是正負號被改壞（兩個正數 `0+0` 卻收到進位，第 7 位變 1 ＝ 結果變負；兩個負數 `1+1` 卻沒收到進位，第 7 位留 0 ＝ 結果變正）；一正一負相加時第 7 位是 `1+0`，收到進位就往外進、沒收到就不進，「進」和「出」一定相同，也一定不溢位。拿 18 題的三組對一次：`0x01+0x7f` 進 1 出 0 → OF 1；`0x05+0x03` 進 0 出 0 → OF 0；`0xff+0x01` 進 1 出 1 → OF 0。

---

## 組語二：[x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html)

### 組語二 5. 情境題：停在函式的第一條指令

> 盒子 2｜過去的作答：[s1-05 quiz.md 第 5 題](../small-project/s1-05-asm-calls/quiz.md#5-情境題停在函式的第一條指令)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

你用 `break *add3` 停在 `add3` 的第一條指令（還沒執行 `push %rbp`），gdb 顯示 `add3 (a=0, b=0, c=0)`，可是 `main` 明明傳的是 1、2、3。

完整原始碼（檔案：[`frame.c`](../small-project/s1-05-asm-calls/frame.c)）與編譯指令（同組語二 1）：

```c
#include <stdio.h>

long add3(long a, long b, long c)
{
    long s = a + b;
    return s + c;
}

int main(void)
{
    long r = add3(1, 2, 3);
    printf("%ld\n", r);
    return 0;
}
```

```sh
gcc -O0 -g -fno-asynchronous-unwind-tables frame.c -o frame
objdump -d --no-show-raw-insn frame
```

`break *add3` 的 gdb 指令：`gdb -batch -ex 'break *add3' -ex run ./frame`

```asm
add3:
  endbr64                 ← 停在這裡
  push   %rbp
  mov    %rsp,%rbp
  mov    %rdi,-0x18(%rbp)
  mov    %rsi,-0x20(%rbp)
  mov    %rdx,-0x28(%rbp)
  ...
```

#### 組語二 5-1. 為什麼 gdb 印錯？這時候要看真正的參數，該看哪裡？

```
因爲在這個時候區域變數(stack)都還沒還有準備好, 要看實際參數只能先看寄存器rdi, rsi, rdx
```

> **批改**：**對**。抓到：序言還沒跑，參數還沒存進堆疊；真正的值在 `%rdi`、`%rsi`、`%rdx`。小修（不扣分）：精確地說，gdb 照除錯資訊去 `-0x18(%rbp)` 這幾格讀參數，這時那幾格還沒被寫入，讀到的是舊值——「區域變數還沒準備好」換成「gdb 去堆疊讀參數、但參數還沒存進去」會更準。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#三call-與-ret用-gdb-一條一條走)】
>
> **核心關鍵**：
>
> 1. gdb 照除錯資訊去堆疊上的 `-0x18(%rbp)` 等位置讀參數（`-O0` 的除錯資訊只記堆疊那一格）。
> 2. 停在第一條指令時序言還沒跑，參數還沒存進那幾格 → 讀到的是舊值。
> 3. 真正的參數在 `%rdi`、`%rsi`、`%rdx`（`info registers rdi rsi rdx`）。

#### 組語二 5-2. 同一個時間點，你想知道「是誰呼叫了 `add3`」，可是不能用 `bt`。根據本課，你會看哪個暫存器指的位址？看到的值代表什麼？

```
直接插rsp那格存的位址,因爲他是返回位址,可以知道他返回到哪,也就是call他的函式
```

> **批改**：**對**（組語二 5 整題盒子 2 → 3）。抓到：看 `%rsp` 指的那格，是返回位址，從它落在哪個函式知道是誰 `call` 的。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#三call-與-ret用-gdb-一條一條走)〈停在函式入口時，怎麼知道是誰呼叫的〉】
>
> **核心關鍵**：
>
> 1. 看 `%rsp` 指的那 8 byte（`x/gx $rsp`）。
> 2. 那是返回位址：`ret` 會跳回去的地方，也就是呼叫者裡 `call` 下一條指令的位址。
>
> **詳細說明**：`info symbol *(long *)$rsp` → `main + 32`。只有停在**第一條指令**時頂端才是返回位址；`push %rbp` 之後返回位址在 `8(%rbp)`。

---

## 多檔：[多檔案的C程式-標頭檔、static與extern、連結錯誤與make](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html)

### 多檔 1. 忘了宣告：`square(100000)` 印出 1410065408

> 盒子 2｜過去的作答：[s1-06 quiz.md 第 1 題](../small-project/s1-06-multi-file/quiz.md#1-忘了宣告square100000-印出-1410065408)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)

`main.c` 沒有 `square` 的宣告，`gcc -c main.c` 只有一個警告，連結也成功，但 `square(100000)` 印出 1410065408 而不是 10000000000。為什麼編譯器會產生錯的程式？為什麼連結器沒有發現兩邊型別不一樣？`main.o` 裡的 `cltq` 在這個錯誤裡扮演什麼角色？

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

#### 多檔 1-1. 為什麼編譯器會產生錯的程式？

```
因爲main不知道square的類別,他只能用預設的int,所以雖然 square的計算是用8byte
但main卻會把他截斷成32位元
```

> **批改**：**對**。抓到：編 `main.c` 時不知道 `square` 的型別，只能照預設當成回傳 `int`；`square` 照 8 byte 算對了，`main` 卻只拿 32 位元。小修（不扣分）：根本原因是編譯器一次只看一個 `.c`，可以點出來。
>
> **參考答案**【[第二節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#實驗忘了宣告square100000-印出-1410065408)〈實驗：忘了宣告〉】
>
> **核心關鍵**：
>
> 1. 編譯器一次只看一個 `.c`，編 `main.c` 時看不到 `square` 的宣告和定義。
> 2. 只好照舊規則**隱式宣告**成回傳 `int`，照 `int` 產生程式碼：回傳值只取 `%eax`（低 32 位元）。
>
> **詳細說明**：`square` 本身照 `long` 算，`%rax` ＝ `0x2_540be400`；`main` 只拿 `%eax` ＝ `0x540be400` ＝ 1410065408。**被呼叫的那邊是對的，呼叫的那邊用錯的型別解讀結果**。

#### 多檔 1-2. 為什麼連結器沒有發現兩邊型別不一樣？

```
連接器只能依據符號名去連接兩個檔案相同的符號
他不知道C的型別是否符合
```

> **批改**：**對**。抓到：連結器只靠符號名稱把兩邊接起來，不知道 C 的型別。小修（不扣分）：原因是 `.o` 的符號表**根本沒有** C 的型別資訊（`Type` 只有 `FUNC`／`OBJECT`）。
>
> **參考答案**【[第四節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#四符號表o-之間靠名字接起來)】
>
> **核心關鍵**：
>
> 1. 連結器只比對符號的**名字**：`main.o` 的 `UND square` 配上 `mathx.o` 的 `GLOBAL square`。
> 2. `.o` 的符號表裡沒有 C 的型別（`Type` 只有 `FUNC`／`OBJECT`／`NOTYPE` 這種），所以兩邊型別不一樣，連結器無從發現。
>
> **詳細說明**：編譯 `main.c` 時看不到 `mathx.c`、編譯 `mathx.c` 時看不到 `main.c`、連結時看不到型別——**沒有任何一個階段同時看得到兩邊的型別**。

#### 多檔 1-3. `main.o` 裡的 `cltq` 在這個錯誤裡扮演什麼角色？

```
因爲main認爲自己從square拿到的返回值是32位元, 但r卻是long,因此他要做cltq, sigend的int轉成long,
是上述的原因造成這個行爲
```

> **批改**：**對**（多檔 1 整題盒子 2 → 3）。抓到：`main` 以為拿到 32 位元的 `int`、要存進 `long r`，所以用 `cltq` 把 `%eax` 擴展成 `long`；它是「猜成 `int`」造成的結果，不是根因。小修（不扣分）：可以明講截斷**就發生在這一條**——`cltq` 用 `%eax` 重寫整個 `%rax`，`square` 留在高 32 位元的 `0x2` 就在這裡被蓋掉。
>
> **參考答案**【[第二節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#實驗忘了宣告square100000-印出-1410065408)〈實驗：忘了宣告〉】
>
> **核心關鍵**：
>
> 1. `cltq` 把回傳的 `%eax` 擴展成 `%rax` 存進 `long r`：只保留低 32 位元。
> 2. 所以 `square` 留在 `%rax` 高 32 位元的正確結果（`0x2`）被蓋掉，截斷就發生在這一條；它是「猜成 `int`」的結果，不是根因。
>
> **詳細說明**：`cltq` 是**符號擴展**（＝ `movslq %eax,%rax`），高 32 位元照第 31 位元補。`main` 以為拿到的是 `int`，要存進 `long r`，`int` 轉 `long` 要保留數值（含正負號），所以編譯器插一條符號擴展。如果 `main` 有正確的宣告，就不會有 `cltq`。題目的 100000² 低 32 位元 `0x540be400` 第 31 位是 0，補 0 補 1 結果一樣。
