# 2026-11-05 複習：函式庫、C資料、hello、GNU、objdump、陣列、組語

> 依[複習排程](../learning/複習排程.md)出今天到期的全部題目（每天出完所有到期題，2026-10-23 起的規則），共 **13 題**（32 個作答框），沒有過期的。
> 函式庫 2、4、8、10、11、13 是 11-02 答對升到盒子 2；C資料 9 是 11-02 大致對、留在盒子 2；hello 5、GNU 4、objdump 3、陣列 2、組語 6、組語 12 是 10-29 答對升到盒子 3。
> 題目依筆記分節、節內照題號排（盒子 2 的筆記排前面）；每題照原題複製，用到的程式碼與實驗步驟都附在題目裡，「過去的作答」連到之前每一次作答（含批改與參考答案）。
> **補問「為什麼」**（2026-11-04 起的出題規則，筆記〈自我測驗〉的原題同步改了）：函式庫 8-1（不是表的話它是什麼）、函式庫 10-1、C資料 9（什麼情況下會出錯、為什麼）、hello 5（`-static` 還會發生嗎、為什麼）、組語 6-3、組語 12-2。
> 目前在第 8 課的作業階段，到期題裡沒有跟第 8 課直接相關的，所以照盒子排。
>
> **怎麼寫**：闔上筆記，每題寫 1～2 句關鍵句就好。可以分幾次寫，寫完一部分就可以先說「幫我批改」。複習不重答，批改會直接附參考答案。
> 已拆小題的題目，每個小題一個作答框；以大題為單位排程，小題全對才算對。

## 批改結果（2026-11-05）

> 13 題全部批改完（2026-11-05）：對 10 題（函式庫 2、4、8、10、13〔13-4 改判〕，C資料 9，hello 5，GNU 4，組語 6、12）；大致對 2 題（objdump 3、陣列 2〔2-1〕）；不完整 1 題（函式庫 11〔11-4 錯〕）。不對的小題（函式庫 11-4、objdump 3、陣列 2-1）自我練習全部過關（11-05）。
> 函式庫 10-1 的題目「那個數字是誰、用哪兩樣東西算出來的」有歧義，筆記〈自我測驗〉第 10 題已改成明問「哪個程式、在什麼時候填進去」並加一條核心關鍵（`ld.so` 第一次呼叫時算出來寫進去），下次複習照新題目。

| 題目 | 結果 | 盒子 | 下次 | 錯在哪／缺什麼 | 過關 |
|---|---|---|---|---|---|
| 函式庫 2-1 | 對 | | | 抓到：讀到 `libtxt.a` 時清單上還沒有 `trim`，一個成員都不拉；讀到 `main.o` 才需要，`ld` 不回頭。小修：「丟棄」是 `.so` 的說法，`.a` 是「一個成員都不拉」 | — |
| 函式庫 2-2 | 對 | | | 抓到：`cannot find` 是找不到函式庫**檔案**，`undefined reference` 是檔案有找到、`trim` 這個**符號**沒被解決 | — |
| 函式庫 2-3 | 對 | 2 → 3（整題） | 2026-11-12 | 抓到：Ubuntu 預設 `--as-needed`，讀到 `-ltxt` 時沒人需要 `trim`，`.so` 被丟掉。小修：是命令列處理完才報錯，不是讀到 `main.o` 當下 | — |
| 函式庫 4-1 | 對 | | | 抓到：`ld.so` 印的；`-Llib` 只給連結時的 `ld`，執行檔只記名字，`lib/` 不在 `ld.so` 的搜尋路徑。小修：搜尋的不只預設目錄（跟 10-29、11-01、11-02 同一個小修，第三次） | — |
| 函式庫 4-2 | 對 | | | 抓到：`-Wl,-rpath,'$ORIGIN/lib'`；雙引號 → `$ORIGIN` 變空字串 → `/lib`。還補上 `ld.so` 執行時把 `$ORIGIN` 換成執行檔目錄。小修：寫進去的不含引號 | — |
| 函式庫 4-3 | 對 | 2 → 3（整題） | 2026-11-12 | 抓到：`RUNPATH` 換得掉、`RPATH` 換不掉，因為 `LD_LIBRARY_PATH` 排在兩者之間。小修：是 `ld.so` 的順序，不是 `RUNPATH`「優先讀」 | — |
| 函式庫 8-1 | 對 | | | 抓到：程式碼（跳板）不是表；`.got.plt` 的 `0x4008`；`0x107a + 0x2f8e`。小修：只跳一次，`*` 是讀那格的內容再跳，不是先跳到 `0x4008`（跟 11-02 同一個小修） | — |
| 函式庫 8-2 | 對 | | | 抓到：第一次 跳板 → `0x4008` 裡的 `0x1040` → `ld.so` 查、寫回 `0x1139`、跳進去；第二次直接跳。11-02 的「基底是誰的」小修改對了 | — |
| 函式庫 8-3 | 對 | 2 → 3（整題） | 2026-11-12 | 抓到：函式先經跳板可以先轉去查；變數只是一條 `mov`，沒地方攔。小修：`mov` 是從 GOT **讀出**位址放進 `%rax`（11-02 也提過） | — |
| 函式庫 10-1 | 對 | | | 抓到：不一樣；基底 ＋ `0x1119`；ASLR 讓基底不同。小修：要說是 `libshape.so` 的載入基底、算出的是虛擬位址（跟 11-02 同一個小修）。題目「是誰算出來的」有歧義，這次不扣，之後改題並加核心關鍵（`ld.so` 第一次呼叫時填） | — |
| 函式庫 10-4 | 對 | 2 → 3（整題） | 2026-11-12 | 抓到：gdb 關 ASLR → 基底相同；虛擬位址相同不代表同一個實體頁，不同也可能是同一個。11-02 的「有可能相同」改對了 | — |
| 函式庫 11-1 | 對 | | | 抓到：3、7 | — |
| 函式庫 11-2 | 對 | | | 抓到：會、`.bss`；`-fPIE` 用 `%rip` 直接讀，所以執行檔要有一份；`ld.so` 載入時複製初值。小修：連結時是 `ld` 留 4 byte 位置＋寫 `R_X86_64_COPY`，沒講出是誰、何時 | — |
| 函式庫 11-3 | 對 | | | 抓到：執行檔的 `COPY` 把初值複製進 `.bss`；`.so` 的 `GLOB_DAT` 由 `ld.so` 先找執行檔、填執行檔那份的位址；兩邊同一份。11-02 的「填位址」用詞改對了 | — |
| 函式庫 11-4 | 錯 | 2 → 1（整題） | 2026-11-06 | （使用者回報沒看到警告：他的 `setlevel` 已重新連結過、`level` 變 8 byte，不符題目「不重新連結」，維持原判）漏了 `ld.so` 的 `different size … consider re-linking` 警告（11-02 有寫）與原因（`.bss` 的 4 byte 是連結時定的）；改用 `eax`／`rax` 解釋，但 `get_level()` 仍回傳 `int`，而且 `1410065408` 是 `main` 直接讀、`COPY` 只複製 4 byte 的結果 | ✅ 過關（11-05） |
| 函式庫 13-1 | 對 | | | 抓到：不是空的（啟動檔已讀）；沒有 `trim`，`main.o` 還沒讀。可補具體符號 `main`、`__libc_start_main` | — |
| 函式庫 13-2 | 對 | | | 抓到：`--as-needed` 丟掉、`--no-as-needed` 記進 `NEEDED`、讀到 `main.o` 時用記住的定義解決、不是加進清單。小修：`--as-needed` 是 Ubuntu 的 gcc 預設 | — |
| 函式庫 13-3 | 對 | | | 抓到：`none_no` 有 `NEEDED libtxt.so`、`none_as` 沒有；沒用到就不必載入。理由較 11-02 單薄（可補：別台機器沒這個 `.so` 就跑不起來） | — |
| 函式庫 13-4 | 對 | 2 → 3（整題） | 2026-11-12 | 改判（11-05）：原判大致對（「為什麼」只寫規則）；使用者指出順序機制同大題 13-1 已答（讀到 `-ltxt` 時清單沒有 `trim`），13-4 的重點是選項只管 `.so`，意見合理。抓到：不會成功、`--no-as-needed` 對 `.a` 無效 | — |
| C資料 9 | 對 | 2 → 3 | 2026-11-12 | 抓到：`len < used` 時無號相減不會是負的、`> 0` 成立；`< 0` 永遠假才警告；UB → 化簡成 `y < 0`，`y` 為正時永遠抓不到溢位（11-02 的 `y = -2` 誤報這次改對了）。小修：講出「回繞成很大的正數」；「不管大於 0 還是小於 0 都是 true」措辭會讀成 `< 0` 也成立 | — |
| hello 5 | 對 | 3 → 4 | 2026-11-19 | 抓到：連結階段在 A 機器決定了版本、B 機器執行時 `ld.so` 找不到那個版本就報錯、`-static` 不會。小修：講明「版本需求寫進執行檔」；B 是**沒有**那個版本（太舊），不是「不一致」 | — |
| GNU 4 | 對 | 3 → 4 | 2026-11-19 | 抓到：修改並散布時要用同樣的 GPL、公開原始碼；LGPL 的 glibc 連結後散布不必公開、GPL 就要。小修：明講「不散布就沒義務」 | — |
| objdump 3 | 大致對 | 3 → 3 | 2026-11-12 | 連結時 `ld` 找到並寫 `NEEDED`、執行時 `ld.so` 只拿到名字找不到，對；缺「這時 `ldd` 顯示 `libfoo.so => not found`」——寫成 `ldd` 平常的特性（10-26 也漏同處，10-29 有寫） | ✅ 過關（11-05） |
| 陣列 2-1 | 大致對 | | | 40、8 與「參數是 `int *`」對；**寫錯**：說 `sizeof a` 量的是 `int (*)[10]`——那是 `&a` 的型別、大小是 8；`sizeof a` 量的是 `int[10]`。「先退化再恢復」跟 10-29 同一個小修 | ✅ 過關（11-05） |
| 陣列 2-2 | 對 | | | 抓到：只拿到指標，要另外傳長度 | — |
| 陣列 2-3 | 對 | 3 → 3（整題） | 2026-11-12 | 抓到：`a` 是 `int *`、+1 走 4；`&a` 是 `int (*)[10]`、+1 走 40。10-29 的 `a + 1 = a[1]` 這次沒再犯 | — |
| 組語 6-1 | 對 | | | 抓到：`l` 是歷史命名；C 的 `long` 標準只要求 ≥ 32 位元、本機 64 位元。10-29 的「32 byte」筆誤改對了 | — |
| 組語 6-2 | 對 | | | `q` | — |
| 組語 6-3 | 對 | | | 抓到：gdb `x` 沒有 `l` 這個大小字母（10-29 誤寫成「組合語言 FMT」，這次對了）。小修：講出同學把 AT&T 字尾與 gdb 大小字母兩套名字混在一起 | — |
| 組語 6-4 | 對 | 3 → 4（整題） | 2026-11-19 | 抓到：`x/1xw`，gdb 的 `w` ＝ 4 byte。小修：列出 `b`／`h`／`w`／`g` ＝ 1／2／4／8 | — |
| 組語 12-1 | 對 | | | 抓到：`0x11f4` 是檔案位址、執行時是基底 ＋ `0x11f4`；`*deref+4` 用符號重算。小修：原因是 **PIE** 不是 ASLR（gdb 關掉 ASLR 照樣失敗；跟 10-29 同一處，第二次） | — |
| 組語 12-2 | 對 | 3 → 4（整題） | 2026-11-19 | 抓到：`-no-pie` 位址固定、不搬，`objdump` 的位址可直接用。小修：是**連結時**定的，不是編譯時 | — |

## 今天的題目

排序：盒子小的在前；同一盒子照筆記與題號。

| # | 題目 | 盒子 | 原本該考的日期 |
|---|---|---|---|
| 1 | 函式庫 2（3 小題） | 2 | 2026-11-05 |
| 2 | 函式庫 4（3 小題） | 2 | 2026-11-05 |
| 3 | 函式庫 8（3 小題） | 2 | 2026-11-05 |
| 4 | 函式庫 10（2 小題） | 2 | 2026-11-05 |
| 5 | 函式庫 11（4 小題） | 2 | 2026-11-05 |
| 6 | 函式庫 13（4 小題） | 2 | 2026-11-05 |
| 7 | C資料 9 | 2 | 2026-11-05 |
| 8 | hello 5 | 3 | 2026-11-05 |
| 9 | GNU 4 | 3 | 2026-11-05 |
| 10 | objdump 3 | 3 | 2026-11-05 |
| 11 | 陣列 2（3 小題） | 3 | 2026-11-05 |
| 12 | 組語 6（4 小題） | 3 | 2026-11-05 |
| 13 | 組語 12（2 小題） | 3 | 2026-11-05 |

---

## 函式庫：[靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html)

### 函式庫 2. 順序：`-ltxt` 寫在 `main.c` 前面

> 盒子 2｜過去的作答：[s1-07 quiz.md 第 2 題](../small-project/s1-07-libraries/quiz.md#2-順序-ltxt-寫在-mainc-前面)（10-29 第一次作答）、[11-01 複習](./2026-11-01-函式庫-組語.md#函式庫-2-順序-ltxt-寫在-mainc-前面)、[11-02 複習](./2026-11-02-函式庫-interp-C資料-字元-陣列-組語-組語二-多檔.md#函式庫-2-順序-ltxt-寫在-mainc-前面)

一個靜態函式庫 `libtxt.a` 由下面的 `case.c`、`space.c`、`trim.c` 三個成員組成，`main.c` 只呼叫 `trim`（檔案同第 1 題）。`gcc -Wall -L. -ltxt main.c -o app` 會失敗，錯誤訊息是 `undefined reference to 'trim'`。用 `ld` 處理輸入檔的方式解釋為什麼。這個錯誤跟 `cannot find -ltxt` 差在哪（各是什麼東西找不到）？`cannot find -ltxt` 可以這樣產生：同樣的檔案，指令改成漏掉 `-L.` 的 `gcc -Wall main.c -ltxt -o app`。在 Ubuntu 上把 `libtxt.a` 換成 `libtxt.so`、命令一樣寫成 `-L. -ltxt main.c`，也會失敗，原因是什麼？

```c
/* case.c */
void to_upper_str(char *s) { for (; *s; s++) if (*s >= 'a' && *s <= 'z') *s -= 32; }
void to_lower_str(char *s) { for (; *s; s++) if (*s >= 'A' && *s <= 'Z') *s += 32; }
```
```c
/* space.c */
int is_space(char c) { return c == ' ' || c == '\t' || c == '\n'; }
```
```c
/* trim.c */
int is_space(char c);

char *trim(char *s)
{
    while (is_space(*s))
        s++;
    return s;
}
```
```c
/* main.c */
#include <stdio.h>

char *trim(char *s);

int main(void)
{
    char buf[] = "   hello";
    printf("[%s]\n", trim(buf));
    return 0;
}
```
建立與連結：`gcc -Wall -c case.c space.c trim.c && ar rcs libtxt.a case.o space.o trim.o && gcc -Wall main.c -L. -ltxt -o app`

#### 函式庫 2-1. `gcc -Wall -L. -ltxt main.c -o app` 會失敗，錯誤訊息是 `undefined reference to 'trim'`。用 `ld` 處理輸入檔的方式解釋為什麼。

```
因爲ld在讀到libtxt.a這個庫的時候,未定義的符號清單並沒有trim,因爲還沒有看到main.o,然後它會把libtxt.a丟棄,接下來纔讀到
main.o這個時候他就沒找不到trim的定義了,因此報出undefined reference to 'trim'
```

> **批改**：**對**。三個核心關鍵都有：讀到 `libtxt.a` 時「未定義的符號清單」裡還沒有 `trim`（`main.o` 還沒讀）→ 一個成員都不拉；之後讀到 `main.o` 才需要 `trim`，`ld` 不會回頭 → `undefined reference`。小修：「把 `libtxt.a` 丟棄」精確說是「一個成員都不拉」——`.a` 本身沒有被丟掉這回事，只是沒有成員被拉進來；「丟棄」是 2-3 的 `.so` 在 `--as-needed` 下的說法。
>
> **參考答案**【[第二節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#順序ld-由左往右只掃一次)〈順序：`ld` 由左往右只掃一次〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `ld` 照命令列順序處理輸入檔，手上維持一張「還沒找到定義的符號」清單。
> 2. 遇到 `libtxt.a` 時 `main` 的 `.o` 還沒讀，清單裡沒有 `trim`（只有啟動檔帶進來的 `main` 等），一個成員都不拉就略過。
> 3. 讀到 `main` 才多了 `trim`，命令列結束時還在清單上，`ld` 不會回頭重掃 `libtxt.a` → `undefined reference`。
>
> **詳細說明**：規則：函式庫寫在用到它的檔案後面（`main.c -L. -ltxt`）。函式庫之間互相需要時，寫兩次或用 `-Wl,--start-group … -Wl,--end-group`。


#### 函式庫 2-2. 這個錯誤跟 `cannot find -ltxt` 差在哪（各是什麼東西找不到）？`cannot find -ltxt` 可以這樣產生：同樣的檔案，指令改成漏掉 `-L.` 的 `gcc -Wall main.c -ltxt -o app`。

```
connot find -ltxt:是ld在預設的搜尋庫資料夾,找不到名字叫libtxt.so, libtxt.a的庫
上題的錯是有找到,但因爲命令的順序導致ld找不到trim的定義,情況不同
```

> **批改**：**對**。抓到：`cannot find -ltxt` 是 `ld` 在搜尋目錄裡找不到 `libtxt.so`／`libtxt.a` 這個**檔案**；2-1 是檔案找到了，`trim` 這個**符號**沒被解決。小修：錯字 `connot` → `cannot`；「預設的搜尋庫資料夾」這次是因為沒給 `-L.`，名字拼錯（`-ltx`）也是同一句錯誤。
>
> **參考答案**【[第三節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#三-l-與--l連結時去哪裡找找什麼名字)、[第二節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#順序ld-由左往右只掃一次)〈順序：`ld` 由左往右只掃一次〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `cannot find -ltxt`：**連結時** `ld` 在搜尋目錄裡找不到 `libtxt.so`／`libtxt.a` **這個檔案**（例如沒給 `-L.`、名字拼錯）。
> 2. `undefined reference to 'trim'`：檔案找到了，但 `trim` 這個**符號**到最後都沒有被解決（這題是因為順序，一個成員都沒拉）。
>
> **詳細說明**：本機實測（同樣四個檔案）：`gcc -Wall main.c -ltxt -o app` → `/usr/bin/ld: cannot find -ltxt: No such file or directory`；`-ltx`（拼錯）→ `cannot find -ltx`；`gcc -Wall -L. -ltxt main.c -o app` → `undefined reference to 'trim'`。前兩個在「找檔案」就停了，第三個是「找符號」失敗。兩者都是連結時 `ld` 的錯；執行時 `ld.so` 找不到 `.so` 是另一句 `cannot open shared object file`（第 4 題）。


#### 函式庫 2-3. 在 Ubuntu 上把 `libtxt.a` 換成 `libtxt.so`、命令一樣寫成 `-L. -ltxt main.c`，也會失敗，原因是什麼？

```
因爲ubuntu的gcc預設 --as-needed
.so只有被需要的時候纔會被寫入needed,而按照這個命令列的順序讀到libtxt.so的時候,ld不知道需要trim這個符號定義,
因爲main.o還沒讀到,所以ld會判斷不需要libtxt.so,最後到main又發現找不到定義了
```

> **批改**：**對**。兩個核心關鍵都有：Ubuntu 的 gcc 預設 `--as-needed`，`.so` 只有被需要時才寫進 `NEEDED`；讀到 `libtxt.so` 時 `main.o` 還沒讀、沒人需要 `trim` → 被丟掉 → 之後 `trim` 沒人提供。連續兩次（11-02、今天）都寫出 `--as-needed`，這點穩了。小修：「最後到 main 又發現找不到定義」——不是讀到 `main.o` 當下就報錯，是**命令列全部處理完**時 `trim` 還在清單上才報（2-1 也一樣）。
>
> **參考答案**【[第四節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#so-在-ubuntu-上也有順序問題)〈`.so` 在 Ubuntu 上也有順序問題〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. Ubuntu 的 gcc 預設傳 `--as-needed` 給 `ld`：`.so` 只有在**目前有人需要它的符號**時才寫進 `NEEDED`、才拿來解決符號。
> 2. 遇到 `-ltxt` 時 `main` 還沒讀、沒人需要 `trim`，`libtxt.so` 就被丟掉 → 之後 `main` 的 `trim` 沒人提供 → `undefined reference`。
>
> **詳細說明**：單純「由左往右」不會讓 `.so` 失敗：把 `-Wl,--no-as-needed` 放在 `-ltxt` 前面（`gcc -Wall -L. -Wl,--no-as-needed -ltxt main.c -o app4`）就成功，`readelf -d app4` 有 `NEEDED libtxt.so`。所以 `.a` 和 `.so` 失敗的機制不同（`.a` 是不拉成員、`.so` 是被 `--as-needed` 丟掉），但結論一樣：`-l` 一律寫在後面。


---
### 函式庫 4. 情境：連結成功、執行找不到 `libfoo.so`

> 盒子 2｜過去的作答：[s1-07 quiz.md 第 4 題](../small-project/s1-07-libraries/quiz.md#4-情境連結成功執行找不到-libfooso)（10-29 第一次作答）、[11-01 複習](./2026-11-01-函式庫-組語.md#函式庫-4-情境連結成功執行找不到-libfooso)、[11-02 複習](./2026-11-02-函式庫-interp-C資料-字元-陣列-組語-組語二-多檔.md#函式庫-4-情境連結成功執行找不到-libfooso)

情境題：你在 `/home/me/proj/` 下做了 `lib/libfoo.so` 和用它的 `app`，連結指令是 `gcc main.c -Llib -lfoo -o app`。連結成功，`./app` 卻印出 `error while loading shared libraries: libfoo.so: cannot open shared object file`。(1) 這個訊息是誰印的？為什麼連結時找得到、執行時找不到？(2) 你想讓 `app` 和 `lib/` 一起打包給同事，同事解壓到任何地方都能直接執行，連結指令要怎麼改？為什麼要用單引號？(3) 同事想臨時用 `LD_LIBRARY_PATH` 換成他自己編的 `libfoo.so` 來除錯，在你的做法（`RUNPATH`）下換得掉嗎？如果是 `RPATH` 呢？為什麼？

#### 函式庫 4-1. 這個訊息是誰印的？為什麼連結時找得到、執行時找不到？

```
這個是ld.so印的
因爲ld在連結的時候可以從命令行知道這個庫所在的路徑位址,也就是-Llib
但ld.so他只能從dynamic section 知道NEEDED寫libfoo.so卻不知道它到底在哪,所以他只能從預設的搜尋的路徑開始找,最後就是找不到
```

> **批改**：**對**。三個核心關鍵都有：是 `ld.so` 印的；連結時 `ld` 從命令列的 `-Llib` 知道資料夾；執行時 `ld.so` 從動態區段只拿到 `NEEDED libfoo.so` 這個名字、不知道在哪，只能照自己的搜尋順序找，`lib/` 不在裡面。小修（**第三次同一個**，10-29 以來每次都只寫「預設路徑」）：`ld.so` 找的不只預設目錄，依序是 `RPATH`、`LD_LIBRARY_PATH`、`RUNPATH`、`/etc/ld.so.cache`、預設目錄——4-2、4-3 考的正是前三項，把順序一起講出來，三個小題就串起來了。
>
> **參考答案**【[第五節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#ldso-去了哪些地方找)〈`ld.so` 去了哪些地方找〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 是動態連結器 `ld.so`（執行檔的 interpreter）在執行時印的。
> 2. `-Llib` 只告訴**連結時**的 `ld` 去哪找，不會寫進執行檔；執行檔的 `NEEDED` 只記名字 `libfoo.so`。
> 3. 執行時 `ld.so` 只找 `RPATH`、`LD_LIBRARY_PATH`、`RUNPATH`、`/etc/ld.so.cache`、預設目錄，`lib/`（和當前目錄）都不在裡面。
>
> **詳細說明**：`LD_DEBUG=libs ./app` 會印出它試過的每個路徑，最後放棄。


#### 函式庫 4-2. 你想讓 `app` 和 `lib/` 一起打包給同事，同事解壓到任何地方都能直接執行，連結指令要怎麼改？為什麼要用單引號？

```
加入runpath
gcc main.c -Llib -lfoo -Wl,-rpath,'$ORIGIN/lib' -o app

因爲單引號就是把裏面的內容視爲純字符串,不會展開像是變量$ORIGIN 得到的runpath就真的是 '$ORIGIN/lib', 這樣ld.so可知道,origin是在執行檔所在的位置,也就可以正確的找到庫的位置
用雙引號就會展開,最後因爲$ORIGIN是空字符串得到了/lib,找不到
```

> **批改**：**對**。指令正確；單引號的理由也對：雙引號時 shell 把 `$ORIGIN` 展開成空字串 → 寫進去變成 `/lib`；單引號原封不動寫進 `$ORIGIN/lib`。這次還補上了 11-02 沒寫的「`ld.so` 執行時把 `$ORIGIN` 換成執行檔所在的目錄」（題目沒問，不計分，但這是它能用的原因）。小修：寫進 `RUNPATH` 的是 `$ORIGIN/lib`，**不含單引號**——引號是給 shell 看的，shell 處理完就拿掉了（跟[第 8 課第二節〈實驗：把 `argv` 一格一格印出來〉](../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#實驗把-argv-一格一格印出來)的「`"two words"` 是一格……程式拿到的字串裡沒有引號」同一件事）。
>
> **參考答案**【[第五節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#方法二-wl-rpath把路徑寫進執行檔)〈方法二：`-Wl,-rpath`——把路徑寫進執行檔〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `gcc main.c -Llib -lfoo -Wl,-rpath,'$ORIGIN/lib' -o app`：把 `$ORIGIN/lib` 寫進執行檔的 `RUNPATH`。
> 2. 要讓 `$ORIGIN` **原封不動**寫進執行檔；不用單引號（包括用雙引號）時 shell 會先把 `$ORIGIN` 換成空字串，寫進去的就變成 `/lib`。
>
> **詳細說明**：為什麼要字面保留：`$ORIGIN` 不是給 shell 的變數，而是 `ld.so` 認得的記號，**執行時 `ld.so` 把它換成執行檔所在的目錄**（同事解壓到 `/tmp/x/`，就去找 `/tmp/x/lib/`），所以只要 `app` 和 `lib/` 的相對位置不變，解壓到哪裡都找得到。單引號裡 shell 什麼都不展開；一定要用雙引號時寫成 `"\$ORIGIN/lib"`。用 `readelf -d app | grep RUNPATH` 檢查實際寫進去的是什麼。


#### 函式庫 4-3. 同事想臨時用 `LD_LIBRARY_PATH` 換成他自己編的 `libfoo.so` 來除錯，在你的做法（`RUNPATH`）下換得掉嗎？如果是 `RPATH` 呢？為什麼？

```
runpath 可以
rpath不行

因爲runpath,會優先讀LD_LIBRARY_PATH, 但rpath會優先讀rpath
```

> **批改**：**對**。抓到：`RUNPATH` 換得掉、`RPATH` 換不掉，理由是兩者跟 `LD_LIBRARY_PATH` 的先後。小修用詞：「`runpath` 會優先讀 `LD_LIBRARY_PATH`」——讀的是 `ld.so`，不是 `RUNPATH`；精確說是「`ld.so` 的搜尋順序裡 `LD_LIBRARY_PATH` 排在 `RUNPATH` 前面、排在 `RPATH` 後面」。
>
> **參考答案**【[第五節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#rpath-和-runpath-差在哪跟-ld_library_path-誰優先)〈`RPATH` 和 `RUNPATH` 差在哪：跟 `LD_LIBRARY_PATH` 誰優先〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `RUNPATH` 換得掉：`ld.so` 的搜尋順序裡 `LD_LIBRARY_PATH`（第 2 步）排在 `RUNPATH`（第 3 步）前面，先找到同事自己的版本。
> 2. `RPATH` 換不掉：`RPATH`（第 1 步，只在沒有 `RUNPATH` 時用）排在 `LD_LIBRARY_PATH` 前面，先找到原本 `lib/` 裡的版本。
>
> **詳細說明**：筆記的 `v1`／`v2` 實驗：`LD_LIBRARY_PATH=v2 ./app_runpath` 印 1005（用了 v2），`./app_rpath` 印 5（還是 v1）。`RUNPATH` 是故意排在後面的，讓使用者能臨時換版本來除錯；所以 `ld` 預設寫 `RUNPATH`，加 `--disable-new-dtags` 才寫舊式的 `RPATH`。


---
### 函式庫 8. PLT 與 GOT：函式的位址存在哪、延遲綁定（10-26 追問新增）

> 盒子 2｜過去的作答：[s1-07 quiz.md 第 8 題](../small-project/s1-07-libraries/quiz.md#8-plt-與-got函式的位址存在哪延遲綁定10-26-追問新增)（10-29 第一次作答）、[11-01 複習](./2026-11-01-函式庫-組語.md#函式庫-8-plt-與-got函式的位址存在哪延遲綁定10-26-追問新增)、[11-02 複習](./2026-11-02-函式庫-interp-C資料-字元-陣列-組語-組語二-多檔.md#函式庫-8-plt-與-got函式的位址存在哪延遲綁定10-26-追問新增)

`liblogger.so` 的 `run()` 呼叫了 `log_msg()` 兩次，主程式 `runner.c` 只呼叫 `run()`。(1) 反組譯裡的 `log_msg@plt` 是一張存位址的表嗎？如果不是，它是什麼？`log_msg` 的位址存在哪個區段的哪一格？用 `jmp` 那行的數字算出來。(2) 第一次和第二次 `call 1070 <log_msg@plt>`，從 `call` 到進入 `log_msg` 各經過哪些地方？`ld.so` 在哪一次介入、做了什麼？(3) 全域變數為什麼不能用這種「第一次用到才查」的方式，一定要載入時就把 `.got` 那格填好？

```c
/* logger.c */
#include <stdio.h>

void log_msg(const char *s)
{
    printf("[log] %s\n", s);
}

void run(void)
{
    log_msg("start");
    log_msg("stop");
}
```
```c
/* runner.c */
void run(void);

int main(void)
{
    run();
    return 0;
}
```
編譯：`gcc -Wall -fPIC -c logger.c -o logger.pic.o && gcc -shared logger.pic.o -o liblogger.so && gcc -Wall runner.c -L. -llogger -Wl,-rpath,'$ORIGIN' -o runner`（檔案：[`logger.c`](../small-project/s1-07-libraries/examples/pic/logger.c)、[`runner.c`](../small-project/s1-07-libraries/examples/pic/runner.c)）。觀察到的反組譯與區段（`objdump -d --no-show-raw-insn liblogger.so`、`readelf -SW liblogger.so`、`readelf -r liblogger.so`）：
```asm
    1179:	call   1070 <log_msg@plt>        ← 第一次呼叫 log_msg（參數 "start" 已放進 %rdi）
    ...
    1188:	call   1070 <log_msg@plt>        ← 第二次呼叫 log_msg（參數 "stop"）

0000000000001070 <log_msg@plt>:
    1070:	endbr64                          ← 間接跳躍的保護標記，可以忽略
    1074:	jmp    *0x2f8e(%rip)        # 4008 <log_msg@@Base+0x2ecf>   ← 跳到「%rip 加 0x2f8e 那格裡存的位址」
    107a:	nopw   0x0(%rax,%rax,1)        ← 不做事的填充指令
```
```text
  [11] .plt              PROGBITS        0000000000001020 001020 000030 10  AX  0   0 16
  [13] .plt.sec          PROGBITS        0000000000001060 001060 000020 10  AX  0   0 16
  [22] .got              PROGBITS        0000000000003fc8 002fc8 000020 08  WA  0   0  8
  [23] .got.plt          PROGBITS        0000000000003fe8 002fe8 000028 08  WA  0   0  8
000000004008  000700000007 R_X86_64_JUMP_SLO 0000000000001139 log_msg + 0
```

#### 函式庫 8-1. 反組譯裡的 `log_msg@plt` 是一張存位址的表嗎？如果不是，它是什麼？`log_msg` 的位址存在哪個區段的哪一格？用 `jmp` 那行的數字算出來。

```
不是

他是一段code,讓程式碼執行跳到.got.plt的位址,然後在跳到log_msg所在的位址
它存在.got.plt 0x4008 的那格

0x107a + 0x2f8e = 0x4008
```

> **批改**：**對**。三個核心關鍵都有：不是表，是一段程式碼（跳板）；位址存在 `.got.plt` 的 `0x4008` 那格；`0x107a ＋ 0x2f8e ＝ 0x4008`（下一條指令的位址加位移）。小修（**跟 11-02 同一個**）：「讓程式跳到 `.got.plt` 的位址，然後再跳到 `log_msg`」聽起來像跳了兩次、先跳到 `0x4008` 本身。實際只有**一次** `jmp`：`jmp *0x2f8e(%rip)` 的 `*` 是「**讀出** `0x4008` 那格存的 8 byte，跳到那個位址」——CPU 從來沒有在 `0x4008` 執行過任何東西（`.got.plt` 是資料，權限 `WA`，不能執行）。
>
> **參考答案**【[第九節第 4 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#4-plt-存函式位址got-存變數位址嗎plt-是跳板以及延遲綁定)〈PLT 存函式位址、GOT 存變數位址嗎？PLT 是跳板，以及延遲綁定〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 不是：`log_msg@plt` 是**程式碼**（`.plt.sec` 裡的跳板，權限 `AX`），不存位址。
> 2. `log_msg` 的位址存在 `.got.plt`（`0x3fe8` 起）的 `0x4008` 那格（`R_X86_64_JUMP_SLOT`）。
> 3. `jmp *0x2f8e(%rip)` 的下一條是 `0x107a`：`0x107a ＋ 0x2f8e ＝ 0x4008`。
>
> **詳細說明**：`*` 表示「跳到記憶體裡存的那個位址」。函式和變數的位址都在 GOT 裡，只是函式那些放在 `.got.plt`。


#### 函式庫 8-2. 第一次和第二次 `call 1070 <log_msg@plt>`，從 `call` 到進入 `log_msg` 各經過哪些地方？`ld.so` 在哪一次介入、做了什麼？

```
第一次: 跳到log_msg@plt -> 跳到.got.plt 0x4008那格, 讀那格位址,一開始指向0x1040(.plt)-> 程式碼:叫ld.so去找出liblogger.so裏log_msg的位址(0x1139),在寫入到0x4008那格, 然後就跳入log_msg(0x1139)

第二次: 跳到log_msg@plt -> 跳到.got.plt 0x4008 讀那格得到0x1139 log_msg-> 跳入log_msg


ld.so在第一次介入,他的功能就是找出log_msg的位址,並寫到.got.plt(0x4008)那格
以上位址全都是檔案的位址,不是真正在記憶體的位址
```

> **批改**：**對**。兩次的路徑都對，`0x1040`、`0x1139` 也都寫出來：第一次 跳板 → 讀 `0x4008` 那格，裡面是 `.plt` 的 `0x1040` → 叫 `ld.so` 查出 `log_msg`、寫回 `0x4008`、跳進去；第二次 跳板 → 讀出 `0x1139` → 直接跳，`ld.so` 不介入。11-02 的小修（基底是誰的）這次改成「全都是檔案的位址」，說法正確。小修：同 8-1，「跳到 `.got.plt` `0x4008` 那格」改成「讀 `0x4008` 那格、照裡面的位址跳」——你後半句「讀那格位址」已經寫對了，前半句的「跳到」是多餘的。
>
> **參考答案**【[第九節第 4 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#4-plt-存函式位址got-存變數位址嗎plt-是跳板以及延遲綁定)〈PLT 存函式位址、GOT 存變數位址嗎？PLT 是跳板，以及延遲綁定〉、[第九節第 8 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#8-ldso-填進格子的是什麼這個行程裡的虛擬位址)〈`ld.so` 填進格子的是什麼：這個行程裡的虛擬位址〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 第一次：`call` → `log_msg@plt` 跳板 → `jmp *(0x4008 那格)`，那格載入時填的是 `.plt` 裡「叫 `ld.so` 去查」的程式碼 → `ld.so` 查出 `log_msg` 的位址、寫回 `0x4008`、跳進 `log_msg`。
> 2. 第二次：`call` → 跳板 → 照 `0x4008`（已經是 `log_msg` 的位址）直接跳進去，`ld.so` 不再介入。
>
> **詳細說明**：這叫延遲綁定（lazy binding）：好處是啟動時不必查完所有函式。跳板只是照格子跳，不判斷格子「空不空」。


#### 函式庫 8-3. 全域變數為什麼不能用這種「第一次用到才查」的方式，一定要載入時就把 `.got` 那格填好？

```
因爲全域變數是用mov把變數的位址寫入,mov根本沒辦法查

函式 基本都是用jmp,所以可以跳到ld.so查函式程式碼的位址,查到後在把原本存查程式碼的那格改成真正函式的位址
```

> **批改**：**對**。抓到機制：函式呼叫先經過 PLT 跳板（`jmp`），所以第一次可以先轉去叫 `ld.so` 查、查完改格子；變數就是一條 `mov`，中間沒有程式碼可以攔下來先查。小修（11-02 也提過）：「`mov` 把變數的位址寫入」——如果你的意思是「寫入 `%rax`」，那是對的；但要講完整：`mov` 從 `.got` 那格**讀出**變數的位址放進 `%rax`，再用 `(%rax)` 讀值。所以那格沒填好，讀出來的就是檔案裡的 0，`(%rax)` 當場 segfault——這就是「一定要載入時填好」的直接後果。
>
> **參考答案**【[第九節第 4 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#4-plt-存函式位址got-存變數位址嗎plt-是跳板以及延遲綁定)〈PLT 存函式位址、GOT 存變數位址嗎？PLT 是跳板，以及延遲綁定〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 延遲綁定靠跳板：呼叫函式時一定先經過 PLT 這段程式碼，第一次才有機會轉去叫 `ld.so` 查。
> 2. 讀變數就是一條 `mov`（從 GOT 拿位址、再照位址讀），中間沒有程式碼可以攔下來「先去查」，所以 `.got` 裡變數那格一定要在載入時就填好。
>
> **詳細說明**：如果 `.got` 那格沒填，`mov (%rax),…` 會直接拿檔案裡的 0 當位址去讀，當場 segfault。


---
### 函式庫 10. 共用的 `.so`：格子裡的位址與實體頁（10-26 追問新增）

> 盒子 2｜過去的作答：[s1-07 quiz.md 第 10 題](../small-project/s1-07-libraries/quiz.md#10-共用的-so格子裡的位址與實體頁10-26-追問新增)（10-29 第一次作答）、[11-01 複習](./2026-11-01-函式庫-組語.md#函式庫-10-共用的-so格子裡的位址與實體頁10-26-追問新增)、[11-02 複習](./2026-11-02-函式庫-interp-C資料-字元-陣列-組語-組語二-多檔.md#函式庫-10-共用的-so格子裡的位址與實體頁10-26-追問新增)
>
> 本題只考 10-1、10-4；10-2、10-3 延到第 ③ 站（2026-10-31 使用者決定）。

兩支程式 `m`、`n` 都連 `libshape.so`（兩支都沒有自己的 `area`），同時在跑。`libshape.so` 的 `calc()` 透過 `area@plt` 呼叫 `area()`。(1) 兩個行程第一次呼叫 `calc` 之後，各自 `.got.plt` 裡 `area` 那格裝的數字一樣嗎？為什麼？那個數字是誰、用哪兩樣東西算出來的？(2) 兩個行程執行的 `area` 程式碼是同一份實體記憶體嗎？`/proc/<pid>/smaps` 裡哪個欄位、出現什麼數值可以證明？(3) 跳到那格裡的位址時，虛擬位址由誰翻成實體位址？翻譯用的表又是誰準備的？(4) 同學分別用 gdb 跑 `m` 和 `n`，`p area` 兩邊都印 `0x7ffff7fb7119`，他說「看吧，兩個行程的 `area` 在同一個位址」。這個推論哪裡有問題？

```c
/* shape.c */
long area(long w, long h)
{
    return w * h;
}

long calc(long w, long h)
{
    return area(w, h) + 1;
}
```
```c
/* m.c */
#include <stdio.h>
#include <unistd.h>

long calc(long w, long h);

int main(void)
{
    printf("m: calc(3, 4) = %ld\n", calc(3, 4));
    fflush(stdout);
    sleep(5);
    return 0;
}
```
```c
/* n.c */
#include <stdio.h>
#include <unistd.h>

long calc(long w, long h);

int main(void)
{
    printf("n: calc(5, 6) = %ld\n", calc(5, 6));
    fflush(stdout);
    sleep(5);
    return 0;
}
```
編譯與執行：`gcc -Wall -fPIC -c shape.c -o shape.pic.o && gcc -shared shape.pic.o -o libshape.so && gcc -Wall m.c -L. -lshape -Wl,-rpath,'$ORIGIN' -o m && gcc -Wall n.c -L. -lshape -Wl,-rpath,'$ORIGIN' -o n && ./m & ./n & wait`（檔案：[`shape.c`](../small-project/s1-07-libraries/examples/share/shape.c)、[`m.c`](../small-project/s1-07-libraries/examples/share/m.c)、[`n.c`](../small-project/s1-07-libraries/examples/share/n.c)）。`libshape.so` 的反組譯與重定位（`objdump -d --no-show-raw-insn libshape.so`、`readelf -r libshape.so`）：
```asm
    1156:	call   1050 <area@plt>                  ← calc 裡呼叫 area
0000000000001050 <area@plt>:
    1050:	endbr64                             ← 間接跳躍的保護標記，可以忽略
    1054:	jmp    *0x2fa6(%rip)        # 4000 <area+0x2ee7>   ← 照 0x4000 那格裡存的位址跳
```
```text
000000004000  000600000007 R_X86_64_JUMP_SLO 0000000000001119 area + 0
```

#### 函式庫 10-1. 兩個行程第一次呼叫 `calc` 之後，各自 `.got.plt` 裡 `area` 那格裝的數字一樣嗎？為什麼？那個數字是誰、用哪兩樣東西算出來的？

```
不一樣
因爲每個行程根據ASLR,他們的位址基點都不一樣
那格數字就是area的位址+基點 = 0x1119 + base
```

> **批改**：**對**。三個核心關鍵都有：不一樣；那格是基底 ＋ `0x1119`；兩個行程的基底因為 ASLR 不同，所以數字不同。小修（**跟 11-02 同一個**）：要講清楚是 **`libshape.so` 在這個行程裡的載入基底**——`0x1119` 是 `area` 在 `libshape.so` 這個檔案裡的位置，所以加的是它的基底，不是 `m`、`n` 執行檔的；算出來的是**這個行程裡的虛擬位址**（11-02 你有寫「虛擬記憶體位址」，這次沒寫）。
>
> **題目的歧義（之後改題）**：「那個數字是誰、用哪兩樣東西算出來的」可以讀成「是誰的位址」，也可以讀成「是**誰（哪個程式）**算出來的」。目前的核心關鍵沒有要求後者，所以這次不扣分；但後者才是重點——**是 `ld.so` 在第一次呼叫 `area` 時（延遲綁定）算出來、寫進那格的**，不是連結時的 `ld`（`ld` 做 `.so` 時還不知道基底）。下次複習起題目改成明問「哪個程式、在什麼時候填進去」，並把這點加進核心關鍵（筆記〈自我測驗〉第 10 題已同步改）。
>
> **參考答案**【[第九節第 7 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#7-函式那邊plt-跳板共用gotplt-每個行程各一份)〈函式那邊：PLT 跳板共用、`.got.plt` 每個行程各一份〉、[第九節第 8 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#8-ldso-填進格子的是什麼這個行程裡的虛擬位址)〈`ld.so` 填進格子的是什麼：這個行程裡的虛擬位址〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 不一樣。
> 2. 那格裝的是**這個行程裡的虛擬位址**：`libshape.so` 在這個行程的載入基底 ＋ `area` 在檔案裡的位置 `0x1119`（`.dynsym`，也就是 `R_X86_64_JUMP_SLOT` 那行的 `0x1119`）。
> 3. 兩個行程的載入基底不同（ASLR），所以數字不同。
>
> **詳細說明**：`0x1119` 在做 `.so` 時就定了，但它只是「檔案內的位置」，還要加上基底；基底要到 `ld.so` 把檔案映射進來時才知道，每個行程不同。那格一共被寫兩次：載入時寫「基底 ＋ `.plt` 裡叫 `ld.so` 的那段程式碼的位置」，第一次呼叫時改成「基底 ＋ `0x1119`」。筆記第 7 章的 `q`、`r` 實測兩個行程的格子分別是 `0x7187194b9139`、`0x7125ada21139`。


#### 函式庫 10-4. 同學分別用 gdb 跑 `m` 和 `n`，`p area` 兩邊都印 `0x7ffff7fb7119`，他說「看吧，兩個行程的 `area` 在同一個位址」。這個推論哪裡有問題？

```
gdb 默認是關閉aslr的,所以他們的基點都相同
但是就算沒有這個問題,也不能只靠虛擬記憶體來判斷在實體記憶體是不是相同位置
虛擬記憶體不同也可能指向相同的位址
```

> **批改**：**對**。兩個核心關鍵都有：gdb 預設關閉 ASLR，所以兩邊的基底相同、印出同一個數字；而且虛擬位址本來就不能用來判斷實體記憶體——相同不代表同一個實體頁，不同也可能是同一個。11-02 的小修（「有可能相同」→ 一定相同）這次改成「基點都相同」，對了。小修：可以補一句**為什麼**虛擬位址不能比：兩個行程各有自己的位址空間和頁表，同一個數字在兩個行程裡由各自的頁表翻譯，對到哪裡互不相干。
>
> **參考答案**【[第九節第 9 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#9-虛擬位址怎麼變成實體位址核心準備頁表cpu-的-mmu-翻譯)〈虛擬位址怎麼變成實體位址：核心準備頁表，CPU 的 MMU 翻譯〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. gdb 預設關掉 ASLR，所以兩次的載入基底一樣，印出同一個數字只是這個原因，不能拿來推論。
> 2. 兩個行程各有自己的位址空間和頁表，虛擬位址只在自己的行程裡有意義；數字一樣不代表是同一個東西或同一個實體頁（反過來，數字不同也可能是同一個實體頁，像 9-1）。
>
> **詳細說明**：要證明兩個行程用的是同一份實體記憶體，看 `smaps` 的 `Shared_Clean`、`Pss`（10-2）。一般程式拿不到實體位址：`/proc/self/pagemap` 沒有 `CAP_SYS_ADMIN` 時 PFN 會被遮成 0（筆記第 9 章 `pfn.c`）。


---
### 函式庫 11. 主程式只寫 `extern`：變數放在哪裡（10-26 追問新增）

> 盒子 2｜過去的作答：[s1-07 quiz.md 第 11 題](../small-project/s1-07-libraries/quiz.md#11-主程式只寫-extern變數放在哪裡10-26-追問新增)（10-29 第一次作答）、[11-01 複習](./2026-11-01-函式庫-組語.md#函式庫-11-主程式只寫-extern變數放在哪裡10-26-追問新增)、[11-02 複習](./2026-11-02-函式庫-interp-C資料-字元-陣列-組語-組語二-多檔.md#函式庫-11-主程式只寫-extern變數放在哪裡10-26-追問新增)

`libcfg.so` 定義了 `int level = 3;` 與 `get_level()`，主程式 `setlevel.c` 只寫了 `extern int level;`。(1) 執行後兩行各印多少？(2) `nm setlevel` 會不會看到 `level`？在哪一種區段？為什麼原始碼只有 `extern` 卻會有？(3) 第一行印出的值是怎麼放進那裡的？`get_level()` 讀的是哪一份 `level`？用重定位的類型和 `ld.so` 找符號的順序說明。(4) 之後把 `cfg.c` 改成 `long level = 3;` 重做 `libcfg.so`，`setlevel` 不重新連結就執行，會發生什麼事、為什麼？

```c
/* cfg.c */
int level = 3;

int get_level(void)
{
    return level;
}
```
```c
/* setlevel.c */
#include <stdio.h>

extern int level;
int get_level(void);

int main(void)
{
    printf("一開始 level = %d\n", level);
    level = 7;
    printf("get_level() = %d\n", get_level());
    return 0;
}
```
編譯：`gcc -Wall -fPIC -c cfg.c -o cfg.pic.o && gcc -shared cfg.pic.o -o libcfg.so && gcc -Wall setlevel.c -L. -lcfg -Wl,-rpath,'$ORIGIN' -o setlevel`（檔案：[`cfg.c`](../small-project/s1-07-libraries/examples/pic/cfg.c)、[`setlevel.c`](../small-project/s1-07-libraries/examples/pic/setlevel.c)）。主程式是 Ubuntu 預設的 `-fPIE`。

#### 函式庫 11-1. 執行後兩行各印多少？

```
3,7
```

> **批改**：**對**。
>
> **參考答案**【[第九節第 3 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#3-主程式只寫-extern-int-counter變數放進執行檔複製重定位)〈主程式只寫 `extern int counter;`：變數放進執行檔（複製重定位）〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 第一行 `一開始 level = 3`，第二行 `get_level() = 7`。
>
> **詳細說明**：主程式和 `get_level()` 用的是同一份 `level`（11-3），所以主程式改成 7，`get_level()` 讀到 7。


#### 函式庫 11-2. `nm setlevel` 會不會看到 `level`？在哪一種區段？為什麼原始碼只有 `extern` 卻會有？

```
會, 在bss區段, 因爲主程式執行檔默認編譯使用-fPIE, 也就是主程式並不會使用got,他需要把level的定義複製到自己的.bss裏,
fPIE 在拿全域變數就是靠這樣算出%rip與level的相對距離直接用

然後ld.so會在執行的時候把把libcfg.so level的初值在寫進去主程式的.bss
```

> **批改**：**對**。三個核心關鍵都有：會看到、在執行檔的 `.bss`；主程式是 `-fPIE`，不經 GOT、用 `%rip` 加固定距離直接讀；所以執行檔自己要有一份 `level`。11-02 的小修（分兩個時間點）這次寫對了一半：「`ld.so` 執行時把初值寫進主程式的 `.bss`」是**載入時**那一步。另一半要補：**連結時**是 `ld` 在 `.bss` 留一個同樣大小（4 byte）的位置、並寫一個 `R_X86_64_COPY`——「把 `level` 的定義複製到自己的 `.bss`」這句沒有講出是誰、在什麼時候做的。這個「大小在連結時就定了」正是 11-4 的關鍵。
>
> **參考答案**【[第九節第 3 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#3-主程式只寫-extern-int-counter變數放進執行檔複製重定位)〈主程式只寫 `extern int counter;`：變數放進執行檔（複製重定位）〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 會看到，類型 `B`，在執行檔的 `.bss`。
> 2. 主程式是 `-fPIE`：編譯器假設全域變數都在執行檔自己裡面，用 `%rip` 加固定距離直接讀，不經 GOT。
> 3. 但 `level` 定義在 `.so`，距離不可能固定；`ld` 只好在執行檔的 `.bss` 替它留一個同樣大小的位置，並寫一個 `R_X86_64_COPY` 重定位（複製重定位）。
>
> **詳細說明**：本機實測：`nm setlevel` → `0000000000004010 B level`；`readelf -SW setlevel` 的 `.bss` 從 `0x4010` 開始；`readelf -r setlevel` → `000000004010 … R_X86_64_COPY … level + 0`。


#### 函式庫 11-3. 第一行印出的值是怎麼放進那裡的？`get_level()` 讀的是哪一份 `level`？用重定位的類型和 `ld.so` 找符號的順序說明。

```
第一行印出來的是主程式裏.bss level位址儲存的值, 執行時ld.so可以知道第一行的重定位類型是R_X86_64_COPY, 因此他就會把libcfg.so裏 level定義的初值複製到0x4010的地方(也就是主程式.bss區段內)

get_level讀得也是主程式的.bss level的值, 因爲ld.so會優先使用主程式的level定義,而且libcfg.so 標記了level的重定位是 R_X86_64_GLOB_DAT, 所以他會在libcfg.so的got裏代表level的位址寫上主程式裏level的位址
```

> **批改**：**對**。三個核心關鍵都有：`ld.so` 照執行檔的 `R_X86_64_COPY` 把 `libcfg.so` 的初值複製到執行檔 `.bss` 的 `0x4010`；`libcfg.so` 的 GOT 那格是 `R_X86_64_GLOB_DAT`，`ld.so` 先找執行檔、找到執行檔那份，把它的位址填進去；所以 `get_level()` 讀的也是執行檔那份。11-02 的用詞小修（`GLOB_DAT` 是「填位址」）這次寫成「寫上主程式裡 `level` 的位址」，對了。小修：「第一行的重定位類型是 `R_X86_64_COPY`」——`COPY` 是**執行檔的重定位表**（`readelf -r setlevel`）裡的那一筆，跟 `printf` 的第一行無關，寫成「執行檔對 `level` 的重定位是 `R_X86_64_COPY`」比較清楚。
>
> **參考答案**【[第九節第 3 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#3-主程式只寫-extern-int-counter變數放進執行檔複製重定位)〈主程式只寫 `extern int counter;`：變數放進執行檔（複製重定位）〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 第一行的 3：載入時 `ld.so` 處理執行檔的 `R_X86_64_COPY`，去 `libcfg.so` 找到 `level`，把初值 3 複製進執行檔 `.bss` 的 `level`；`main` 直接讀它。
> 2. `get_level()` 經過 `libcfg.so` 的 GOT 讀 `level`，那格是 `R_X86_64_GLOB_DAT level`；`ld.so` 照「先找執行檔、再找 `.so`」找到**執行檔那份**，把執行檔的 `&level` 填進去。
> 3. 所以兩邊用的都是執行檔那份；`main` 改成 7，`get_level()` 讀到 7。`libcfg.so` 自己那份 `level` 只在載入時被讀一次初值。
>
> **詳細說明**：本機實測 `LD_DEBUG=bindings ./setlevel` 的兩行就是這兩件事：`binding file ./setlevel to …/libcfg.so: normal symbol 'level'`（處理 `COPY`：去 `.so` 拿初值）、`binding file …/libcfg.so to ./setlevel: normal symbol 'level'`（處理 `GLOB_DAT`：`.so` 的 GOT 接到執行檔）。


#### 函式庫 11-4. 之後把 `cfg.c` 改成 `long level = 3;` 重做 `libcfg.so`，`setlevel` 不重新連結就執行，會發生什麼事、為什麼？

```
編譯libcfg.so沒有錯誤
但是原本libcfg.so返回是寫入eax, 變成寫入rax
但是setlevel還是只用eax的getlevel的回傳值,所以如果level的值大於32位會有問題,例如把初值設定成10000000000
得到的是
一開始 level = 1410065408
get_level() = 7
這是因爲雖然level已經寫到rax了,但是setlevel取值拿eax, 只看下面32位元
```

> **批改**：**錯**。兩個核心關鍵都沒有答到，而且原因錯了：
>
> 1. **漏了 `ld.so` 的警告**：執行時第一行就是 `./setlevel: Symbol `level' has different size in shared object, consider re-linking`（印在 `stderr`），程式照樣跑。11-02 你寫出來了，這次沒有。
> 2. **漏了真正的原因**：執行檔 `.bss` 留給 `level` 的大小（4 byte）是**連結時**照當時的 `.so` 決定的；`.so` 換成 8 byte 後兩邊對不上，要重新連結。
> 3. **「`eax`／`rax`」的解釋不成立**：`get_level()` 的回傳型別還是 `int`（題目只改了 `level`），回傳值本來就放在 `%eax`，呼叫端照 `int` 拿 `%eax` 是正常的，沒有對不上。而且你測到的 `一開始 level = 1410065408` 是 `main` **直接讀**執行檔裡的 `level` 印的，根本沒經過 `get_level()` 的回傳值。
>
> 你的實驗很好，我照做了一次（初值改成 `10000000000`），輸出跟你一樣，能直接證明正確的原因：
>
> ```text
> ./setlevel: Symbol `level' has different size in shared object, consider re-linking
> 一開始 level = 1410065408
> get_level() = 7
> ```
>
> - `1410065408` 就是 `10000000000` 的**低 32 位元**（`printf '%d\n' $((10000000000 & 0xffffffff))` 印出 `1410065408`）：`ld.so` 處理 `COPY` 時只複製兩邊大小**較小的** 4 byte 進執行檔的 `.bss`，上面 4 byte 沒有複製過來。`main` 讀的是這 4 byte，所以印出被截斷的值。
> - `get_level() = 7`：`main` 寫 7 進那 4 byte；`get_level()` 經過 GOT 到執行檔那格讀 **8 byte**（新的 `libcfg.so` 反組譯是 `mov (%rax),%rax`），多讀的 4 byte 是 `level` 後面的 `.bss` 空間，剛好是 0，所以讀到 7。這裡 `%rax` 讀 8 byte、回傳時只留 `%eax`，是 C 把 `long` 轉成 `int` 的正常轉換，不是錯誤的來源。
> - 所以錯在「大小在連結時寫死」：執行檔只有 4 byte 的位置，`.so` 卻以為有 8 byte。
>
> **補充（11-05，你回報「執行時沒有看到警告」之後查的）**：你 `small-project/s1-07-libraries/quiz/q11/` 裡的 `setlevel` 修改時間是 11:47，`libcfg.so` 是 12:34；`readelf -sW setlevel` 顯示執行檔裡的 `level` 已經是 **8 byte**——也就是 `setlevel` 在 `level` 改成 `long` 之後**重新連結過**，不是題目說的「不重新連結」。重新連結後兩邊都是 8 byte，大小對得上，`ld.so` 就不會警告。輸出剛好一樣，但原因不同：
>
> - 重新連結後，`COPY` 把 8 byte 全部複製進來（另寫一支 `extern long level;` 的程式印出來是 `0x2540be400`，就是 `10000000000`）。`main` 印 `1410065408`，是因為 `setlevel.c` 仍宣告 `extern int level;`、只讀前 4 byte（`0x540be400`）——兩個檔案對同一個變數的宣告型別不一樣，這本身就是錯的程式（C 的未定義行為）。
> - `get_level() = 7`：`main` 只寫了前 4 byte，後 4 byte 還是 `0x2`，`get_level()` 讀到 `0x200000007`，轉成 `int` 剩下 7。
>
> 所以判定不變（**維持「錯」**）：題目的情境是不重新連結，核心關鍵問的是那個情境下的警告與原因；而 `eax`／`rax` 的解釋在兩種情況下都不成立。要看到警告，照題目的順序做：先用 `int level = 3;` 做 `libcfg.so` 並連結 `setlevel`，之後**只**重做 `libcfg.so`（改成 `long`），不要再跑連結 `setlevel` 的那條指令。
>
> **參考答案**【[第九節第 3 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#3-主程式只寫-extern-int-counter變數放進執行檔複製重定位)〈主程式只寫 `extern int counter;`：變數放進執行檔（複製重定位）〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 執行時 `ld.so` 印警告 `Symbol 'level' has different size in shared object, consider re-linking`，程式照樣跑。
> 2. 原因：執行檔 `.bss` 留給 `level` 的大小（4 byte）是**連結時**照當時的 `.so` 決定的；現在 `.so` 裡的 `level` 是 8 byte，大小對不上，要重新連結。
>
> **詳細說明**：本機實測（`cfg.c` 改成 `long level = 3;` 重做 `libcfg.so`，`setlevel` 不動）：先印那行警告，接著照樣印 `一開始 level = 3`、`get_level() = 7`。印對是運氣：`COPY` 只複製較小的 4 byte；`get_level()` 經過 GOT 到執行檔那格讀 8 byte，多讀的 4 byte 是執行檔 `.bss` 裡 `level` 後面的空間，剛好是 0；初值超過 4 byte、或後面放了別的變數，就會讀錯、寫到別人。這是函式庫通常**不直接給外面用全域變數、而是提供函式**（`get_level()`）的原因之一：函式走 PLT，執行檔不必知道它的大小。

**自我練習**（不批改）

```
ld.so直接報./setlevel: Symbol `level' has different size in shared object, consider re-linking

因爲libcfg.so重新編譯過了,所以level會變成8byte,
但是setlevel沒有重新編譯,他的符號表還是記住level是4byte
所以在ld.so載入libcfg.so做符號連結與 複製lib的定義去主程式的時候,就會彙報這個警告,並希望你重新連結setlevel

直接執行可能會發生一些截斷的錯誤,不過以本題的情況來說結果不會錯,但如果level初值到1000000000
就會發現結果有問題
```


---
### 函式庫 13. `--as-needed`：讀到 `.so` 那一刻的清單

> 盒子 2｜過去的作答：[s1-07 quiz.md 第 13 題](../small-project/s1-07-libraries/quiz.md#13---as-needed讀到-so-那一刻的清單)（10-31 第一次作答）、[11-01 複習](./2026-11-01-函式庫-組語.md#函式庫-13---as-needed讀到-so-那一刻的清單)、[11-02 複習](./2026-11-02-函式庫-interp-C資料-字元-陣列-組語-組語二-多檔.md#函式庫-13---as-needed讀到-so-那一刻的清單)

`libtxt.so` 由下面的 `case.c`、`space.c`、`trim.c` 做成；`main.c` 只呼叫 `trim`，`none.c` 什麼都不呼叫。(1) 用 `gcc -Wall -L. -ltxt main.c -o app_bad`（`-ltxt` 寫在 `main.c` 前面）連結時，`gcc -v` 顯示 gcc 交給 `ld` 的順序是 `Scrt1.o crti.o crtbeginS.o … -ltxt <main.c 的 .o> … -lc …`。讀到 `-ltxt` 那一刻，「還沒找到定義的符號」清單是空的嗎？裡面有 `trim` 嗎？為什麼？(2) 預設下 `gcc -Wall -L. -ltxt main.c -o app_bad` 失敗；把 `-Wl,--no-as-needed` 放在 `-ltxt` 前面（`gcc -Wall -L. -Wl,--no-as-needed -ltxt main.c -o app_no`）就成功。讀到 `libtxt.so` 時兩種模式各做了什麼？`main.o` 要的 `trim` 最後是怎麼被解決的——是 `libtxt.so` 把 `trim` 加進了清單嗎？(3) `none.c` 用 `gcc -Wall none.c -L. -Wl,--no-as-needed -ltxt -o none_no` 和 `gcc -Wall none.c -L. -ltxt -o none_as` 各連結一次，`readelf -d` 的 `NEEDED` 會差在哪？Ubuntu 為什麼預設開 `--as-needed`？(4) 把 `libtxt.so` 換成同樣三個 `.o` 做成的 `libtxt.a`（並刪掉 `libtxt.so`），`gcc -Wall -L. -Wl,--no-as-needed -ltxt main.c -o app_a` 會成功嗎？為什麼？

```c
/* case.c */
void to_upper_str(char *s) { for (; *s; s++) if (*s >= 'a' && *s <= 'z') *s -= 32; }
void to_lower_str(char *s) { for (; *s; s++) if (*s >= 'A' && *s <= 'Z') *s += 32; }
```
```c
/* space.c */
int is_space(char c) { return c == ' ' || c == '\t' || c == '\n'; }
```
```c
/* trim.c */
int is_space(char c);

char *trim(char *s)
{
    while (is_space(*s))
        s++;
    return s;
}
```
```c
/* main.c */
#include <stdio.h>

char *trim(char *s);

int main(void)
{
    char buf[] = "   hello";
    printf("[%s]\n", trim(buf));
    return 0;
}
```
```c
/* none.c */
int main(void)
{
    return 0;            /* 完全不呼叫 libtxt 的函式 */
}
```
做 `.so`：`gcc -Wall -fPIC -shared case.c space.c trim.c -o libtxt.so`；做 `.a`：`gcc -Wall -c case.c space.c trim.c && ar rcs libtxt.a case.o space.o trim.o && rm libtxt.so`（檔案都在 [`examples/asneeded/`](../small-project/s1-07-libraries/examples/asneeded/)）。啟動檔 `Scrt1.o` 未定義的符號可以用 `nm -u $(gcc -print-file-name=Scrt1.o)` 看。

#### 函式庫 13-1. 用 `gcc -Wall -L. -ltxt main.c -o app_bad`（`-ltxt` 寫在 `main.c` 前面）連結時，`gcc -v` 顯示 gcc 交給 `ld` 的順序是 `Scrt1.o crti.o crtbeginS.o … -ltxt <main.c 的 .o> … -lc …`。讀到 `-ltxt` 那一刻，「還沒找到定義的符號」清單是空的嗎？裡面有 `trim` 嗎？為什麼？

```
不是
沒有trim
在讀Scrt1.o crti.o crtbeginS.o 這些的時候就會添加未定義的符號清單了,而讀到-ltxt那一刻,還沒讀到main.o所以不會有trim在未定義的符號清單裏面
```

> **批改**：**對**。兩個核心關鍵都有：不是空的，`Scrt1.o`、`crti.o`、`crtbeginS.o` 讀進來時已經放進未定義符號；沒有 `trim`，因為 `main.o` 排在 `-ltxt` 後面還沒讀。可以補（11-02 也建議過）：具體是哪些——`Scrt1.o` 帶進 `main`、`__libc_start_main`（`nm -u $(gcc -print-file-name=Scrt1.o)` 看得到）。
>
> **參考答案**【[第九節第 15 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#15-ld-還沒讀到-maino怎麼知道誰需要什麼未定義符號清單與---as-needed)〈`ld` 還沒讀到 `main.o`，怎麼知道誰需要什麼〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. **不是空的**：`-ltxt` 前面的啟動檔已經讀過，`Scrt1.o` 帶進 `main`、`__libc_start_main` 等未定義符號。
> 2. **沒有 `trim`**：需要 `trim` 的是 `main.c` 的 `.o`，它排在 `-ltxt` 後面、還沒讀到；`ld` 由左往右、不往後看，只用此刻的清單做決定。
>
> **詳細說明**：`nm -u $(gcc -print-file-name=Scrt1.o)` 列出 `U _GLOBAL_OFFSET_TABLE_`、`U __libc_start_main`、`U main`；`crtbeginS.o` 另外帶進 `U __TMC_END__`（`w` 開頭的是弱符號，找不到也不算錯）。所以正文第二節說「這時清單裡沒有 `calc_add`」，不是嚴格意義的空清單。


#### 函式庫 13-2. 預設下 `gcc -Wall -L. -ltxt main.c -o app_bad` 失敗；把 `-Wl,--no-as-needed` 放在 `-ltxt` 前面（`gcc -Wall -L. -Wl,--no-as-needed -ltxt main.c -o app_no`）就成功。讀到 `libtxt.so` 時兩種模式各做了什麼？`main.o` 要的 `trim` 最後是怎麼被解決的——是 `libtxt.so` 把 `trim` 加進了清單嗎？

```
一般的情況是 --as-needed, 動態庫只有在被需要的時候纔會寫入NEEDED,否則就丟棄,原本的寫法,讀到-ltxt,他都不被需要,因爲還沒讀到main.o,所以就被丟棄,導致報了undefined trim的錯誤

但是--no-as-needed,讀到ltxt不管你有沒有被需要,都加到需求單,然後看到main的trim的時候直接找到定義就解決了
libtxt.so沒有把trim加入清單
```

> **批改**：**對**。三個核心關鍵都有：`--as-needed` 讀到 `libtxt.so` 時沒人需要它 → 丟掉、不記進 `NEEDED` → 之後 `trim` 沒人提供；`--no-as-needed` 不管需不需要都記進 `NEEDED`，讀到 `main.o` 要 `trim` 時直接用它的定義解決；`libtxt.so` 沒有把 `trim` 加進清單。小修：「`--as-needed` 是一般的情況」要加「在 Ubuntu 上」——它是 Ubuntu 的 gcc 預設傳的，GNU `ld` 本身預設是 `--no-as-needed`（2-3 也是這個前提）。
>
> **參考答案**【[第九節第 15 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#15-ld-還沒讀到-maino怎麼知道誰需要什麼未定義符號清單與---as-needed)〈`ld` 還沒讀到 `main.o`，怎麼知道誰需要什麼〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `--as-needed`（預設）：讀到 `libtxt.so` 時看此刻的清單，沒人需要它的符號 → 整個丟掉、不記進 `NEEDED`；後面 `main.o` 要 `trim` 時沒人提供 → `undefined reference`。
> 2. `--no-as-needed`：不看清單，一律留下、記進 `NEEDED`，並**記住它提供的定義**。
> 3. `trim` 是讀到 `main.o` 時，`ld` 用已經記住的 `libtxt.so` 定義當場解決的；不是 `libtxt.so` 把 `trim` 加進清單——清單只放「還沒找到定義的符號」，`.so` 提供的是定義。
>
> **詳細說明**：`nm -D --defined-only libtxt.so` 看得到它提供的四個定義：`is_space`、`to_lower_str`、`to_upper_str`、`trim`。`.so` 本來就有「先記住、後面再用」的能力，順序問題完全是 `--as-needed` 在那一刻「丟掉」造成的。實測：`gcc -Wall -L. -Wl,--no-as-needed -ltxt main.c -o app_no` 成功，`LD_LIBRARY_PATH=. ./app_no` 印 `[hello]`，`readelf -d app_no` 有 `libtxt.so` 與 `libc.so.6` 兩個 `NEEDED`。


#### 函式庫 13-3. `none.c` 用 `gcc -Wall none.c -L. -Wl,--no-as-needed -ltxt -o none_no` 和 `gcc -Wall none.c -L. -ltxt -o none_as` 各連結一次，`readelf -d` 的 `NEEDED` 會差在哪？Ubuntu 為什麼預設開 `--as-needed`？

```
差別是 none_no有libtxt.so在needed, none_as沒有

因爲只有被需要的需要載入,不被需要的就沒必要用到
```

> **批改**：**對**。兩個核心關鍵都有：`none_no` 的 `NEEDED` 有 `libtxt.so`、`none_as` 沒有；用不到的 `.so` 沒必要載入。理由寫得比 11-02 單薄，可以補 11-02 批改提過的兩點：兩個都有 `NEEDED libc.so.6`（差別只有 `libtxt.so` 那一列）；多一個用不到的 `NEEDED`，換到沒裝 `libtxt.so` 的機器 `ld.so` 找不到它，程式整個跑不起來——不只是浪費。
>
> **參考答案**【[第九節第 15 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#15-ld-還沒讀到-maino怎麼知道誰需要什麼未定義符號清單與---as-needed)〈`ld` 還沒讀到 `main.o`，怎麼知道誰需要什麼〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `none_no` 有 `libtxt.so` 與 `libc.so.6` 兩個 `NEEDED`；`none_as` 只有 `libc.so.6`。
> 2. `--no-as-needed` 讓用不到的 `.so` 也記進 `NEEDED`，執行時 `ld.so` 就得多找、多載入一個；`--as-needed` 讓沒用到的自動消失。
>
> **詳細說明**：多出來的 `NEEDED` 不只是浪費：換到沒裝 `libtxt.so` 的機器，`ld.so` 找不到它，程式會整個跑不起來，即使根本沒用到。建置系統常把一長串 `-l` 套到每個程式上，`--as-needed` 讓沒用到的自動消失；代價就是 `-l` 一定要寫在用到它的檔案後面。


#### 函式庫 13-4. 把 `libtxt.so` 換成同樣三個 `.o` 做成的 `libtxt.a`（並刪掉 `libtxt.so`），`gcc -Wall -L. -Wl,--no-as-needed -ltxt main.c -o app_a` 會成功嗎？為什麼？

```
不會
--no-as-needed是給動態庫用的,靜態庫-ltxt一定要放到main.c後面
```

> **批改**：~~**大致對**~~ → **對**（2026-11-05 改判）。「不會成功」對，「`--no-as-needed` 只管動態庫」也對——這是這一小題真正要區分的點（`.a` 跟 `.so` 不同，選項救不了 `.a`）。原本扣分的理由是「為什麼失敗」只寫了規則「靜態庫 `-ltxt` 一定要放到 `main.c` 後面」、沒寫機制；使用者反對：題目要的「為什麼」是選項對 `.a` 無效，失敗是順序造成的，而順序的機制同一大題的 13-1 已經答過（讀到 `-ltxt` 時清單還沒有 `trim`）。**這個意見合理**：13-1～13-4 是同一大題的小題，13-1 的作答已經寫出「讀到 `-ltxt` 那一刻清單沒有 `trim`」，13-4 寫「因為順序的原因」接得上它；11-02 判對的作答也只多寫了「清單上還沒有 `trim`」這一句。所以改判為對。仍要注意：單獨一句「一定要放到後面」只是規則、不是原因；引用其他小題時寫明「同 13-1：讀到 `-ltxt` 時清單沒有 `trim`」，讀的人才不必猜。完整機制：`.a` 只拉此刻清單需要的成員 → 一個都不拉 → 讀到 `main.o` 也不回頭 → `undefined reference to 'trim'`。
>
> **參考答案**【[第九節第 15 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#15-ld-還沒讀到-maino怎麼知道誰需要什麼未定義符號清單與---as-needed)〈`ld` 還沒讀到 `main.o`，怎麼知道誰需要什麼〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. **不會成功**：`undefined reference to 'trim'`。
> 2. `--as-needed`／`--no-as-needed` 只管 `.so`；`.a` 讀到時只拉此刻清單需要的成員，此刻清單沒有 `trim` → 一個都不拉、也不記，之後不回頭。
>
> **詳細說明**：`.a` 的順序問題不管加什麼選項都存在，只能把 `-ltxt` 寫在 `main.c` 後面，或用 `--start-group … --end-group` 讓群組反覆掃（正文第二節）。`rm libtxt.so` 是因為 `-ltxt` 會先找 `.so`（正文第三節）。


---

## C資料：[C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html)

### C資料 9. 情境題：有人寫了 `if (len - used > 0)` 判斷「緩衝區還有沒有剩餘空間」，`len`、`used` 都是 `size_t`，用 `-Wall -Wextra` 編譯沒有任何警告。這個判斷在什麼情況下會出錯、為什麼？為什麼 gcc 對 `len - used < 0` 會警告、對 `> 0` 卻不會？另外，有人用 `int sum = x + y; if (sum < x)` 檢查「`x` 加上一個**正數** `y` 會不會溢位」，為什麼這個檢查在 `-O2` 下可能完全失效？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 9 題](../small-project/s1-02-data-repr/quiz.md#9-情境題有人寫了-if-len---used--0-判斷緩衝區還有沒有剩餘空間lenused-都是-size_t用--wall--wextra-編譯沒有任何警告這個判斷在什麼情況下會出錯為什麼-gcc-對-len---used--0-會警告對--0-卻不會另外為什麼-int-sum--x--y-if-sum--x-這種溢位檢查在--o2-下可能完全失效)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-30 複習](./2026-10-30-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語.md#c資料-9-情境題有人寫了-if-len---used--0-判斷緩衝區還有沒有剩餘空間lenused-都是-size_t用--wall--wextra-編譯沒有任何警告這個判斷在什麼情況下會出錯為什麼-gcc-對-len---used--0-會警告對--0-卻不會另外為什麼-int-sum--x--y-if-sum--x-這種溢位檢查在--o2-下可能完全失效)、[11-02 複習](./2026-11-02-函式庫-interp-C資料-字元-陣列-組語-組語二-多檔.md#c資料-9-情境題有人寫了-if-len---used--0-判斷緩衝區還有沒有剩餘空間lenused-都是-size_t用--wall--wextra-編譯沒有任何警告這個判斷在什麼情況下會出錯為什麼-gcc-對-len---used--0-會警告對--0-卻不會另外有人用-int-sum--x--y-if-sum--x-檢查x-加上一個正數-y-會不會溢位為什麼這個檢查在--o2-下可能完全失效)

```
在len < used會出現問題, 因爲不管是 大於0還是小於0 都是true 因爲類型是size_t unsigned long無號相減一定是 大於 0 或 等於0

因爲gcc 認爲無號運算結果也必須是無號, 所以 小於0永遠爲假,if沒意義. 但無號的結果有大於0與等於0,所以if是有意義的,因此不警告

-O2的最佳化,不會去檢查有沒有溢位,因爲溢位是UB的行爲,因此-O2會把sum <x 簡化成 x+y < x -> y < 0

if (y < 0) 

這個檢查時要檢查+正數y是否會溢位,但y爲正數,這個if是false,因此溢位檢查就失效了,永遠檢查不到溢位
```

> **批改**：**對**。三個核心關鍵都有：
>
> 1. `len < used` 時出錯：`size_t` 是無號，相減的結果不會是負數，所以 `> 0` 成立；
> 2. 無號數 `< 0` 永遠為假，`if` 沒意義，所以 gcc 警告；`> 0` 會有「大於 0」與「等於 0」兩種結果，是有意義的判斷，所以不警告；
> 3. 有號溢位是 UB，`-O2` 把 `x + y < x` 化簡成 `y < 0`；`y` 是正數，`y < 0` 永遠為假，**真的溢位時也永遠抓不到**——11-02 錯在舉 `y = -2` 的誤報，這次改成「`y` 為正，檢查永遠是 false」，改對了。
>
> 小修三處：
> - 「不管是大於 0 還是小於 0 都是 true」容易讀成「`< 0` 也是 true」——那正好跟第二問相反（`< 0` 永遠是 false）。要說的是：「不管數學上 `len - used` 是正是負，`> 0` 都成立」，因為負的結果會**回繞成一個很大的正數**（例如 `len = 2`、`used = 5` 時是 `18446744073709551613`），判斷就誤以為「還有空間」。
> - 「`-O2` 不會去檢查有沒有溢位」：精確說是「gcc 假設**有號**溢位不會發生（C 標準規定它是 UB）」，所以可以做 `x + y < x` → `y < 0` 這種只在不溢位時才成立的代數化簡；無號數的溢位有定義（回繞），不能這樣化簡。
> - 「這個檢查時要檢查」→「這個檢查是要檢查」（錯字）。
>
> **參考答案**【[第七節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#七陷阱有號和無號混在一起比較)〈`-Wtype-limits` 只抓「永遠成立／永遠不成立」的比較〉、[第八節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#八溢位無號回繞有號是未定義行為)〈另一種寫法：`x + y < x` 被改寫成 `y < 0`〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `len < used` 時，`len - used` 回繞成很大的正數，判斷誤以為「還有空間」。
> 2. 無號數 `< 0` 永遠為假，所以 gcc 警告；`> 0` 等於 `!= 0`，結果不固定，是合法寫法，所以不警告。
> 3. 有號溢位是 UB，gcc 假設它不會發生，把 `x + y < x` 改寫成 **`y < 0`**；真的溢位時（`y` 是正的），這個檢查抓不到。
>
> **詳細說明**：`len == used` 時結果是 0，`> 0` 為假，所以不是「永遠為真」，只有 `len < used` 會出錯。警告來自 `-Wtype-limits`（`-Wextra` 會開啟）。溢位那一問的記法：「`x + y < x`」在數學上（沒有溢位）跟「`y < 0`」完全等價，gcc 只是做了這個代數化簡，而這個化簡只在「不會溢位」時才成立。`sum < x` 這個寫法只在 `y ≥ 0` 時才是溢位檢查（`y` 是負數時，沒溢位也會成立），見[第十一節第 1 章](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#1-sum--x-這個溢位檢查前提化簡的根據怎麼確定-gcc-化簡了)。正確的溢位檢查要在運算**之前**，用不會溢位的式子判斷，例如 `y > 0 && x > INT_MAX - y`。

---

## hello：[hello.c怎麼變成能跑的程式-編譯的四個階段](../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html)

### hello 5. 情境題：你把一支動態連結的程式複製到另一台機器，執行時出現找不到 `libc.so.6` 某個版本符號的錯誤。從本課的哪個階段可以解釋這件事？如果改用 `-static` 編譯，這個問題還會發生嗎？為什麼？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-情境題你把一支動態連結的程式複製到另一台機器執行時出現找不到-libcso6-某個版本符號的錯誤從本課的哪個階段可以解釋這件事如果改用--static-編譯這個問題還會發生嗎)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#hello-5-情境題你把一支動態連結的程式複製到另一台機器執行時出現找不到-libcso6-某個版本符號的錯誤從本課的哪個階段可以解釋這件事如果改用--static-編譯這個問題還會發生嗎)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-29 複習](./2026-10-29-hello-GNU-objdump-陣列-組語.md#hello-5-情境題你把一支動態連結的程式複製到另一台機器執行時出現找不到-libcso6-某個版本符號的錯誤從本課的哪個階段可以解釋這件事如果改用--static-編譯這個問題還會發生嗎)

```
連結階段在a機械,符號連結時,找到了a機械libc.so.6的某個版本,因此建立了連結.但是執行檔移動到b機械執行的時候,ld.so去找這個庫用到的符號版本發現不一致的時候,就會報出這個錯誤

改-static不會,因爲他在連結的階段就把那個庫的靜態版本一起加入執行檔了,程式碼已經在裏面了
```

> **批改**：**對**（盒子 3 → 4）。三個核心關鍵都有：從**連結階段**解釋——在 A 機器連結時，`ld` 依 A 的 `libc.so.6` 決定了要哪個版本的符號；到 B 機器執行時，`ld.so` 檢查 B 的 `libc.so.6` 有沒有提供那個版本，沒有就報錯；`-static` 不會發生，因為程式碼在連結時就放進執行檔、不再找外部的 `libc.so.6`。10-23、10-26 都缺「是連結時決定的」，10-29 開始寫出來，這次也有。
>
> 小修（不扣分）：
> - 「建立了連結」講得太含糊，要說出**留下了什麼**：`ld` 把「要 `libc.so.6` 的 `GLIBC_2.34` 版」這個**版本需求寫進執行檔**（`readelf -V` 的 `.gnu.version_r` 看得到），執行檔帶著這張需求單到 B 機器。
> - 「發現不一致」→ 精確說是「B 的 `libc.so.6` **沒有提供**那個版本」：B 的 glibc 比 A 新是沒問題的（新版會保留舊的版本符號），只有 B 比較舊、缺那個版本時才報 `` version `GLIBC_2.34' not found ``。
> - `-static` 放進去的是 `libc.a` 裡**用到的那些成員（`.o`）**，不是整個庫（10-29 提過）。
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

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md#gnu-4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#gnu-4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-29 複習](./2026-10-29-hello-GNU-objdump-陣列-組語.md#gnu-4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)

```
1. 要求當你修改並且發佈的時候需要以同樣GPL的授權發佈,並且公開原始碼

2. LGPL:你的軟體連結了libc並且發佈,不需要公開原始碼
   GPL: 你的軟體連結了libc並且發佈,需要公開原始碼
```

> **批改**：**對**（盒子 3 → 4）。三個核心關鍵都有：copyleft 要求**修改並散布**時用同樣的 GPL、公開原始碼（「當你修改並且發佈的時候」也表示只自己用沒有義務）；glibc 是 LGPL，連結它的軟體散布時不必公開原始碼；如果是 GPL，連結它的軟體散布時就要公開。跟 10-29 一樣完整。
>
> 小修（不扣分）：第 1 點可以把「只自己用、不散布就沒有義務」明講出來，而不是只靠「發佈的時候」隱含——這是 copyleft 最常被誤會的地方（很多人以為「用了 GPL 軟體就要開源」）。第 2 點把對比的主角講清楚：差別在**被連結的那個庫**用哪種授權（glibc 是 LGPL；假設它是 GPL），不是你的軟體用哪種。
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

## objdump：[objdump能不能代替ldd找依賴庫-需求單與實際載入](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html)

### objdump 3. 一支程式連結時用了 `-L./lib -lfoo`，編譯成功；執行時卻出現 `error while loading shared libraries: libfoo.so`。用「連結時」與「執行時」分別是誰在找檔案，解釋為什麼連結找得到、執行卻找不到。這時 `objdump -p` 與 `ldd` 各會顯示什麼？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-一支程式連結時用了--llib--lfoo編譯成功執行時卻出現-error-while-loading-shared-libraries-libfooso用連結時與執行時分別是誰在找檔案解釋為什麼連結找得到執行卻找不到這時-objdump--p-與-ldd-各會顯示什麼)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-29 複習](./2026-10-29-hello-GNU-objdump-陣列-組語.md#objdump-3-一支程式連結時用了--llib--lfoo編譯成功執行時卻出現-error-while-loading-shared-libraries-libfooso用連結時與執行時分別是誰在找檔案解釋為什麼連結找得到執行卻找不到這時-objdump--p-與-ldd-各會顯示什麼)

```
連接時: ld 會根據命令行寫的 從./lib找到libfoo.so所以連結的時候不會有問題,ld還會把libfoo.so寫到NEEDED裏
執行時: ld.so看到NEEDED libfoo.so,然後從自己預設的庫資料夾查詢有沒有庫, 結果找不到

objdump -p 直接從執行檔的dynamic section 只會看到這個執行檔的NEEDED
ldd 使用預設的ld.so,實際查詢NEEDED,並且不止執行檔的,連相依的所有庫的NEEDED他都找
除此之外他還會顯示動態載入器的路徑, LINUX虛擬的函式庫
```

> **批改**：**大致對**（盒子 3 → 3）。前兩個核心關鍵都有：連結時 `ld` 照 `-L./lib` 找到 `libfoo.so`、在執行檔寫下 `NEEDED libfoo.so`；執行時 `ld.so` 只從 `NEEDED` 拿到名字、在自己的搜尋路徑裡找不到。**缺第 3 點的後半**：題目問的是「**這時**（找不到的情況下）`ldd` 會顯示什麼」——答案是 `libfoo.so => not found`。你寫的是 `ldd` 平常的特性（會遞迴查相依的庫、顯示動態載入器與 vDSO），都對，但沒回答這個情境下它會印出什麼；10-29 你寫了「`ldd` 在這個庫會顯示 not found」，這次漏了（10-26 也漏在同一處）。`objdump -p` 那半有了：照樣列出 `NEEDED libfoo.so`——可以補一句「所以從 `objdump -p` 看不出問題」，這正是兩個工具的差別。
>
> 小修：「從自己預設的庫資料夾查詢」完整說是 `RPATH`／`LD_LIBRARY_PATH`／`RUNPATH`／`/etc/ld.so.cache`／預設目錄（跟今天函式庫 4-1 同一個小修）。
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

**自我練習**（不批改）

```
連接時: ld 會根據命令行寫的 從./lib找到libfoo.so所以連結的時候不會有問題,ld還會把libfoo.so寫到NEEDED裏
執行時: ld.so看到NEEDED libfoo.so,然後從自己預設的庫資料夾查詢有沒有庫, 結果找不到

objdump -p 會直接看到libfoo.so
ldd 會看到 libfoo not found
```


---

## 陣列：[陣列、字串與指標運算-C不檢查邊界與用gdb找segfault](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html)

### 陣列 2. 陣列傳進函式之後

> 盒子 3｜過去的作答：[s1-03 quiz.md 第 2 題](../small-project/s1-03-arrays-pointers/quiz.md#2-1-int-a10-在-main-裡-sizeof-a-是-40傳進-void-fint-a-之後-sizeof-a-變成-8為什麼)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-29 複習](./2026-10-29-hello-GNU-objdump-陣列-組語.md#陣列-2-陣列傳進函式之後)

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
main宣告的a[10], a會退化成int 指標, 但是在用sizeof a 的時候他沒有退化,恢復成原來陣列的指標,int (*) [10] 因此是40byte

f的參數就是 int指標,因此在函式內用 sizeof a自然還是int指標
```

> **批改**：**大致對**。對的部分：`sizeof a` 是不退化的例外，量出 40；參數 `int a[]` 其實是 `int *`，所以函式裡是 8（核心關鍵 2）。**寫錯的地方**：「恢復成原來陣列的指標，`int (*)[10]`，因此是 40 byte」——`sizeof a` 量的是**陣列本身**（型別 `int[10]`），不是「指向陣列的指標」。`int (*)[10]` 是 `&a` 的型別（2-3），它是指標，大小是 8：
>
> ```c
> #include <stdio.h>
>
> int main(void)
> {
>     int a[10];
>     printf("sizeof a  = %zu   (型別 int[10])\n", sizeof a);
>     printf("sizeof &a = %zu   (型別 int (*)[10])\n", sizeof &a);
>     printf("sizeof (int (*)[10]) = %zu\n", sizeof(int (*)[10]));
>     return 0;
> }
> ```
>
> ```text
> sizeof a  = 40   (型別 int[10])
> sizeof &a = 8   (型別 int (*)[10])
> sizeof (int (*)[10]) = 8
> ```
>
> 所以「型別是 `int (*)[10]`」跟「40 byte」是互相矛盾的；把 2-3 的 `&a` 跟 2-1 的 `a` 混在一起了。另外「退化成 `int` 指標，但用 `sizeof` 時沒退化、恢復成……」跟 10-29 的小修同一處：`sizeof a` 時 `a` **從頭到尾沒有退化**，沒有「先退化再恢復」這回事。
>
> **參考答案**【[第三節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#三陣列名稱什麼時候變成指標什麼時候不會)〈函式參數寫成 `int a[]`，其實就是 `int *a`〉】
>
> **核心關鍵**：
>
> 1. 在 `main` 裡，`sizeof a` 是「陣列名稱不退化」的例外，量的是整個陣列：10 × 4 = 40 byte。
> 2. 參數寫成 `int a[]` 其實就是 `int *a`，`f(a)` 傳過去的只是 `a[0]` 的位址，所以 `f` 裡的 `sizeof a` 是指標的 8 byte。
>
> **詳細說明**：gcc 對這種寫法有 `-Wsizeof-array-argument` 警告。

**自我練習**（不批改）

```
main宣告的a[10], a會退化成int 指標, 但是在用sizeof a 的時候他沒有退化,指的就是原來的陣列 因此是40byte

f的參數就是 int指標,因此在函式內用 sizeof a自然還是int指標
```


#### 陣列 2-2. 這件事對「怎麼設計接收陣列的函式」有什麼影響？

> 題目：這件事（陣列傳進函式之後 `sizeof a` 從 40 變成 8）對「怎麼設計接收陣列的函式」有什麼影響？

```
因爲a 退化成了int指標,僅從這個參數無法知道陣列的大小,因此需要額外輸入這個陣列的長度,show纔有辦法知道陣列的範圍
```

> **批改**：**對**。抓到：參數只是 `int *`，函式不知道陣列多長，要另外傳長度。
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
a 如上所說,是退化成int指標,a+1,只位移了4byte

&a a跟sizeof a的情況相同,又沒退化了, 是int (*) [10] 指標,所以 &a + 1 相當位移了40byte
```

> **批改**：**對**（陣列 2 整題：2-1 大致對 → 盒子 3 → 3）。兩個核心關鍵都有：`a` 退化成 `int *`、`a + 1` 走 4 byte；`&a` 不退化、型別 `int (*)[10]`、`&a + 1` 走 40 byte。「`&a` 跟 `sizeof a` 情況相同」——兩者都是不退化的例外，這句對；但不退化之後兩者得到的東西不同：`sizeof a` 量的是陣列 `int[10]`，`&a` 得到的是指向陣列的指標 `int (*)[10]`（2-1 就是把這兩者混了）。10-29 的小修（`a + 1` 是 `&a[1]` 不是 `a[1]`）這次沒再犯。
>
> **參考答案**【[第三節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#三陣列名稱什麼時候變成指標什麼時候不會)〈`a` 和 `&a`：位址一樣，型別不一樣〉】
>
> **核心關鍵**：
>
> 1. 位址相同，但指的東西不同：`a` 退化成指向第一個元素（`int`）的指標，`&a` 是指向**整個陣列**的指標。
> 2. 所以 `a + 1` 走 4 byte（一個 `int`），`&a + 1` 走 40 byte（一整個陣列）。
>
> **詳細說明**：用型別的話說，`a` 是 `int *`，`&a` 是 `int (*)[10]`。記法：**退化 ＝ 變成指向第一個元素的指標；`sizeof`、`&` 是兩個不退化的例外**。


---

## 組語：[x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)

### 組語 6. 組合語言的大小名稱 vs C 的型別

> 盒子 3｜過去的作答：[s1-04 quiz.md 第 6 題](../small-project/s1-04-asm-basics/quiz.md#6-組合語言的大小名稱-vs-c-的型別2026-10-06-課後提問新增)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-29 複習](./2026-10-29-hello-GNU-objdump-陣列-組語.md#組語-6-組合語言的大小名稱-vs-c-的型別)

#### 組語 6-1. `movsbl` 的 `l` 叫 long，為什麼是 4 byte，而本機 C 的 `long` 是 8 byte？

```
組語l 叫long是4byte是歷史原因,爲了向下相容沿用至今

C標準定義long只要32位元以上就好,只是本機x86_64long是64位元,要看平臺具體是什麼
```

> **批改**：**對**。抓到：組合語言的 `l` 是歷史命名、為了相容沿用；C 的 `long` 標準只要求至少 32 位元，實際大小看平台，本機 x86-64 是 64 位元。10-29 的筆誤（「32 byte」）這次寫成「32 位元」，改對了。可以補上歷史的具體內容：8086 是 16 位元，word ＝ 2 byte，所以 32 位元叫 long（兩個 word）＝ 4 byte。
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

> **批改**：**對**。`q`。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈為什麼 `l` 叫 long 卻是 4 byte〉】
>
> **核心關鍵**：
>
> 1. `q`（quad word，8 byte）。


#### 組語 6-3. 這樣對嗎？為什麼？

> 題目：情境題：第 5 課你在 `gdb` 裡想看一個 `int` 變數的 4 個 byte，同學說「`int` 在組合語言是 `l`，所以用 `x/1l`」。這樣對嗎？為什麼？

```
gdb x 的功能 表達式沒有l 所以不對
```

> **批改**：**對**。抓到：不對，gdb `x` 的大小字母裡沒有 `l`（實際打 `x/1l` 會得到 `Undefined output format "l".`）。10-29 把「gdb 的 FMT」誤寫成「組合語言 FMT」，這次寫對了是 gdb。小修：題目補問了「為什麼」，可以把同學錯在哪講出來——他把**兩套不同的名字**混在一起：組合語言（AT&T）的字尾 `b`／`w`／`l`／`q` 跟 gdb `x` 的大小字母 `b`／`h`／`w`／`g` 是各自的命名，`l` 只屬於前者。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈為什麼 `l` 叫 long 卻是 4 byte〉的「第三套名字：`gdb` 的 `x` 命令」】
>
> **核心關鍵**：
>
> 1. 不對：gdb `x` 的大小字母是另一套，`l` 不是它的大小字母（實測 `Undefined output format "l".`）。


#### 組語 6-4. 應該用哪個大小字母，為什麼？

> 題目：（承上）在 gdb 裡用 `x` 看 `int` 變數的 4 個 byte，應該用哪個大小字母，為什麼？

```
x/1xw, 可以用w 代表4byte
```

> **批改**：**對**（組語 6 整題 盒子 3 → 4）。抓到：`x/1xw`，`w` 在 gdb 裡是 4 byte。小修（跟 10-29 同一個）：把 gdb 的四個大小字母一起寫出來，`b`／`h`／`w`／`g` ＝ 1／2／4／8 byte，才看得出為什麼是 `w`——同一個字 word，在 AT&T 字尾是 2 byte、在 gdb 是 4 byte。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈為什麼 `l` 叫 long 卻是 4 byte〉的「第三套名字：`gdb` 的 `x` 命令」】
>
> **核心關鍵**：
>
> 1. gdb `x` 的大小字母是 `b`／`h`／`w`／`g` ＝ 1／2／4／8 byte。
> 2. 一次看整個 `int` 用 **`w`**（4 byte，`x/1xw &v`）；用 `b` 要寫個數 4（`x/4xb &v`），逐 byte 印。
>
> **詳細說明**：同一個字 word：AT&T 是 2 byte，gdb 是 4 byte。記法是一律換算成 byte 數再想。


---
### 組語 12. 檔案裡的位址 vs 執行時的位址

> 用 `objdump -d paren` 查到 `deref+4` 在 `0x11f4`，`break *0x11f4` 之後 `run` 卻得到 `Cannot access memory at address 0x11f4`，`break *deref+4` 就沒問題。
>
> 盒子 3｜過去的作答：[s1-04 quiz.md 第 12 題](../small-project/s1-04-asm-basics/quiz.md#12-檔案裡的位址-vs-執行時的位址2026-10-06-課後實作新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-29 複習](./2026-10-29-hello-GNU-objdump-陣列-組語.md#組語-12-檔案裡的位址-vs-執行時的位址)

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

`-O2` 編出的 `deref`：

```asm
deref:
	endbr64                 # 控制流程保護標記（佔 4 byte，所以下一條是 deref+4）
	mov	(%rdi),%rax         # 把 %rdi 當位址，讀 8 byte 放進 %rax
	ret                     # 回傳 %rax
```

實驗步驟：

```sh
gcc -O2 -g paren.c -o paren
objdump -d --no-show-raw-insn paren        # deref 的 mov (%rdi),%rax 那一行印成 11f4
gdb -batch -ex 'break *0x11f4' -ex 'break *deref+4' -ex 'info breakpoints' -ex run -ex 'info breakpoints' ./paren
```

#### 組語 12-1. 為什麼？

> 題目：`break *0x11f4` 之後 `run` 得到 `Cannot access memory at address 0x11f4`，`break *deref+4` 卻沒問題，為什麼？

```
因爲實際上在0x11f4找不到對應的記憶體位址,這個是檔案的位址,實際執行的時候真是的記憶體位址根據ASLR
要 0x11f4 + base

*deref+4 沒問題是因爲 這是根據符號deref找到的位址 是真實的位址
```

> **批改**：**對**。兩個核心關鍵都有：`0x11f4` 是**檔案裡的位址**，執行時的位址是「基底 ＋ `0x11f4`」，所以 `0x11f4` 找不到記憶體；`*deref+4` 用符號，gdb 會用 `deref` 執行時的位址去算。小修（**跟 10-29 同一處，第二次**）：搬家的原因是 **PIE**，不是 ASLR。gdb 預設**關掉** ASLR，基底固定是 `0x555555554000`，`break *0x11f4` 照樣失敗——如果原因是 ASLR，關掉 ASLR 就該成功了，但沒有。ASLR 只是讓「每次執行的基底不一樣」；「需要加基底」是因為 PIE 的位址從 0 開始算、載入時才決定放在哪。12-2 的 `-no-pie` 正好是對照組：沒有 PIE 就不用加基底。下次請直接寫「因為 PIE」。另外錯字：「真是的記憶體位址」→「真實的」。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上面中斷點 4 的 `break *0x11f4`：為什麼 `run` 之後失敗？〉】
>
> **核心關鍵**：
>
> 1. `objdump -d` 印的是**檔案裡的位址**；PIE 執行時整支被搬到基底（gdb 裡是 `0x555555554000`），執行時位址 ＝ 基底 ＋ 檔案位址，`0x11f4` 在執行時沒有對映。
> 2. `*deref+4` 裡有符號名稱，程式載入後 gdb 用新的位址重算成 `0x5555555551f4`；寫死的 `0x11f4` 只是數字，不會重算。


#### 組語 12-2. 如果程式是用 `-no-pie` 編的，`break *` 加 `objdump` 查到的位址會不會成功？為什麼？

```
會, 因爲-no-pie就是編譯的時候就定了記憶體位址了,這個時候用break *(objdump 查到的位址) 是可以的
```

> **批改**：**對**（組語 12 整題 盒子 3 → 4）。抓到：會成功，因為非 PIE 的位址在建置時就定了，載入時不搬，`objdump` 的位址就是執行時的位址。小修：精確說是**連結時**（`ld` 把程式放在固定位址，例如 `0x401000` 附近）就定了，不是「編譯的時候」——編譯只產生 `.o`，位址還沒決定；`readelf -h` 的 `Type` 是 `EXEC`（PIE 是 `DYN`）。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上面中斷點 4 的 `break *0x11f4`：為什麼 `run` 之後失敗？〉】
>
> **核心關鍵**：
>
> 1. 會成功：非 PIE（`Type` 是 EXEC）固定放在檔案寫的位址，不會搬，檔案裡的位址就是執行時的位址。


---
