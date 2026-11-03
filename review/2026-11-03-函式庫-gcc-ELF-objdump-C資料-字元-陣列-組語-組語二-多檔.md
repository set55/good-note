# 2026-11-03 複習：函式庫、gcc、ELF、objdump、C資料、字元、陣列、組語、組語二、多檔

> 依[複習排程](../learning/複習排程.md)出今天到期的全部題目（每天出完所有到期題，2026-10-23 起的規則），共 **22 題**（53 個作答框），沒有過期的。
> 函式庫 3、12 是 11-02 大致對／錯、留在盒子 1 的題目（不對的小題 11-02 已在自我練習框過關，今天再考一次）；函式庫 1、5、7 是第 7 課自我測驗 10-31 進排程後、盒子 2 的第一次複習；objdump 1 是 10-31 答對升到盒子 2；其餘 16 題是 10-27 作答後排在盒子 3 的題目（C資料 10 是大致對、留在盒子 3）。
> 題目依筆記分節、節內照題號排（函式庫有盒子 1 的題目，排在最前面）；每題照原題複製，用到的程式碼與實驗步驟都附在題目裡，「過去的作答」連到之前每一次作答（含批改與參考答案）。
> 目前在第 8 課的作業階段，到期題裡沒有跟第 8 課直接相關的，所以照盒子排。
>
> **怎麼寫**：闔上筆記，每題寫 1～2 句關鍵句就好。可以分幾次寫，寫完一部分就可以先說「幫我批改」。複習不重答，批改會直接附參考答案。
> 已拆小題的題目，每個小題一個作答框；以大題為單位排程，小題全對才算對。

## 批改結果（2026-11-03）

> 22 題全部批改完（2026-11-03）：對 15 題（函式庫 1，gcc 5，ELF 4〔改判〕，objdump 1、4，C資料 6，字元 5，陣列 4〔4-2 改判〕、7，組語 7、11，組語二 1、4，多檔 3、4）；大致對 6 題（函式庫 3〔3-3 改判〕、12、7，gcc 2，C資料 10，組語 15）；不完整 1 題（函式庫 5〔5-3 改判〕）。不對的小題自我練習全部過關（函式庫 3-2、12-2、12-3、5-2、7-1，gcc 2，C資料 10，組語 15-3，都在 11-03 過關）。

| 題目 | 結果 | 盒子 | 下次 | 錯在哪／缺什麼 | 過關 |
|---|---|---|---|---|---|
| 函式庫 3-1 | 對 | | | 抓到：錯誤提到 `hits`；原因是沒加 `-fPIC`（預設 `-fPIE`）時直接用 `%rip` 加固定距離讀它。小修：「會成功嗎？」沒有明講，要先寫「不會成功」 | — |
| 函式庫 3-2 | 大致對 | | | 「`static`、只有 `stats.c` 用得到」、「`%rip` 加固定距離直接讀」對；缺中間那一步：**所以它一定是這個 `.so` 自己那份，跟 `.text` 一起搬、距離在做 `.so` 時就固定**。跟 11-02 錯在同一處（11-02 自我練習有寫出「距離一定是固定的」，這次又漏了） | ✅ 過關（11-03） |
| 函式庫 3-3 | 對 | 1 → 1（整題） | 2026-11-04 | 改判（11-03）：原本把「基點不同 → 到 `hits` 距離不同」當成寫錯，使用者說明指的是 `.so` 到（主程式那份）`hits` 的距離，接在理由 1 後面成立。抓到：理由 1 符號插入、`ld.so` 先找執行檔、位址填進 GOT；理由 2 每個行程到 `hits` 的距離不同、一份共用的 `.text` 放不下，所以位址放 GOT。小修：可直接說出「`.text` 唯讀、改了每個行程要各複製一份」（跟 11-02 同一個小修） | — |
| 函式庫 12-1 | 對 | | | 抓到：前處理、找不到 `temp.h`、補 `-Iinclude`。小修：記住原文 `fatal error: temp.h: No such file or directory` | — |
| 函式庫 12-2 | 大致對 | | | 連結、補 `-ltemp` 對；**沒寫錯誤長相** `undefined reference to 'c_to_f'`（11-02 有寫）。小修：缺 `-rpath` 不是連結失敗的原因，是執行時才出事 | ✅ 過關（11-03） |
| 函式庫 12-3 | 大致對 | | | 「不對、前處理就處理掉」對（比 11-02 的符號插入進步）；「常量」要說**巨集**（`const` 變數真的會存在 `.so`）；缺「換成 `0.0` 編進 `main` 的程式碼、`.so` 裡沒有它」 | ✅ 過關（11-03） |
| 函式庫 12-4 | 對 | 1 → 1（整題） | 2026-11-04 | 抓到：`.h` 不用給（已編進 `app`）；`.so` 要給（執行時 `ld.so` 載入）。可補：放在 `app` 旁的 `lib/`（`RUNPATH $ORIGIN/lib`） | — |
| 函式庫 1-1 | 對 | | | 抓到：只有 `trim`、`is_space`；`main` 要 `trim` → `trim.o`，`trim.o` 要 `is_space` → `space.o`，`case.o` 不拉。小修：`ld` 看的是「未定義符號」清單 | — |
| 函式庫 1-2 | 對 | 2 → 3（整題） | 2026-11-10 | 抓到：直接寫的 `.o` 一律整個放進去（`case.o` 的兩個函式也進去）；`.a` 有人需要才拉。可補：執行結果一樣、檔案變大 | — |
| 函式庫 5-1 | 對 | | | 抓到：`NEEDED` 是 SONAME `libcalc.so.1` 不是 `libcalc.so`，所以刪 `.so` 沒事、刪 `.so.1` 找不到。可補：`libcalc.so` 只在連結時給 `-lcalc` 用 | — |
| 函式庫 5-2 | 不完整 | | | 動態做法對（`.so.1` 改指向 `.so.1.0.1`）；**靜態那一半沒答**：`.o` 已複製進執行檔，要用新 `libcalc.a` 重新連結。10-29 有答到 | ✅ 過關（11-03） |
| 函式庫 5-3 | 對 | 2 → 1（整題） | 2026-11-04 | 改判（11-03）：原本以「沒寫為什麼」判大致對；使用者說明「舊程式繼續用 `.so.1`」就是不會壞的原因，`NEEDED` 寫 `.so.1` 在同一題 5-1 已答，改判為對。抓到：新 SONAME `libcalc.so.2`；舊程式照舊用 `.so.1`、不會拿到新版。可補：`libcalc.so` 改指向新版給新程式連結 | — |
| 函式庫 7-1 | 大致對 | | | 「不會」對；理由**放錯階段**：`multiple definition` 是連結時 `ld` 的檢查，「`ld.so` 優先選執行檔」是執行時的事（7-3 的答案）。正確：`ld` 不把 `.so` 複製進輸出檔、只記 `NEEDED`，輸出檔只有一份 `verbose`（你 7-2 有寫到這個機制） | ✅ 過關（11-03） |
| 函式庫 7-2 | 對 | | | 抓到：會重複定義；`.o` 整個加進執行檔，兩份 `verbose` 同在 `app2`；`.so` 只查未定義符號、寫 `NEEDED` | — |
| 函式庫 7-3 | 對 | | | 抓到：3；`ld.so` 先找執行檔，把主程式那份的位址填進 `libconf.so` 的 GOT | — |
| 函式庫 7-4 | 對 | 2 → 2（整題） | 2026-11-06 | 抓到：0；`static` → 不經 GOT、`%rip` 直接讀。可補：讀的是 `.so` 自己那份（`LOCAL`、插入不了） | — |
| gcc 2 | 大致對 | 3 → 3 | 2026-11-10 | `-O2` 放暫存器對，`<optimized out>` 原因（不在堆疊、暫存器不保證是它）寫得好；缺 `-O0` 那一半（不最佳化、每個變數有固定記憶體位置、每條語句讀寫記憶體），gdb 影響漏了「行號會跳」。小修：「空間換時間」不是重點 | ✅ 過關（11-03） |
| gcc 5 | 對 | 3 → 4 | 2026-11-17 | 抓到：自己的程式改 `-O0` 重編；kernel 不能 `-O0`、改用 `printk`。**理由第三次寫歪**（10-24、10-27 同一句）：不是「死碼與迴圈擴展太多」，是 kernel **依賴**內嵌與刪除死碼才編得過（不在核心關鍵，不扣分）。小修：要加 `-g` | — |
| ELF 4 | 對 | 3 → 4 | 2026-11-17 | 改判（11-03）：原本以缺「區段是連結視角、給 `ld` 用」判大致對；使用者指出題目只問差在哪，沒問誰的視角。「區段小、各有功能／載入段依權限打包、配合核心」已答出差別，「視角」移到詳細說明。抓到：權限以頁為單位、同權限打包、不打包浪費 | — |
| objdump 1 | 對 | 2 → 3 | 2026-11-10 | 抓到：三類（vdso、interpreter、間接依賴）；`objdump` 只看 `curl` 的 dynamic section，`ldd` 請載入器一路看 `NEEDED` 的 `NEEDED`。10-30、10-31「直接與間接」的小修這次改過來了 | — |
| objdump 4 | 對 | 3 → 4 | 2026-11-17 | 抓到：用只讀檔案的工具看 `NEEDED`；`ldd` 靠本機 x86-64 interpreter、讀不了 ARM 執行檔，錯誤誤導人。小修：原文是 `not a dynamic executable`；首選 `readelf -d`（跟 10-27 同一處） | — |
| C資料 6 | 對 | 3 → 4 | 2026-11-17 | 抓到：8 byte；使用者空間上面 2 byte 一定是 0、`%p` 不印前導零（補出完整 `0x00007ffe50b23624`）；`ffff` 開頭是核心位址。10-23 缺的「為什麼一定是 0」這次有了。可補：根本原因是只用 48 bit（canonical address） | — |
| C資料 10 | 大致對 | 3 → 3 | 2026-11-10 | `a < b` 的「`long` 裝得下 `unsigned int`」這次有了（10-20、10-27 都缺）；警告理由對。`c < d` 沒寫轉成 **`unsigned long`**、−1 變最大值，依據寫成「`int` 裝不下」——應是「無號等級 ≥ 有號 → 轉無號」（10-27 自我練習有寫對） | ✅ 過關（11-03） |
| 字元 5 | 對 | 3 → 4 | 2026-11-17 | 抓到：兩人都對（`char` vs Unicode 字元）；`strlen` 3、`mbstowcs` 1；沒 `setlocale` → `"C"` locale → `(size_t)-1` ＝ 2⁶⁴ − 1。小修：不是「沒有對應的碼」，是 `0xe5` 被判成無效序列 | — |
| 陣列 4-1 | 對 | | | 抓到：合法記憶體 → 安靜；寫壞金絲雀、返回時查 → stack smashing；沒對映／沒權限 → segfault。小修：「金絲雀位址」第三次（是檢查值不是位址）；安靜出錯的例子（寫到返回位址）不對 | — |
| 陣列 4-2 | 對 | 3 → 4（整題） | 2026-11-17 | 改判（11-03）：原本以「拿紅區當理由」判大致對；使用者說明紅區只是舉例，重點是在合法的堆疊裡越界——紅區內寫壞區域變數確實是安靜出錯的一個實例，「這是屬於安靜的出錯」也接上 4-1。抓到：落在可存取的記憶體，C 與 kernel 都不會報錯，結果卻可能錯。小修：先講通則（C 不檢查邊界、任何可存取的記憶體都一樣），再舉例 | — |
| 陣列 7-1 | 對 | | | 抓到：8 ＋ 8 ＋ 6 ＝ 22 byte | — |
| 陣列 7-2 | 對 | | | 抓到：最後那組 8 byte 只有前 6 個可存取。小修：講明是「那一組」的前 6 個 | — |
| 陣列 7-3 | 對 | | | 抓到：22 − 1 ＝ 21 個字元 | — |
| 陣列 7-4 | 對 | 3 → 4（整題） | 2026-11-17 | 抓到：全部可以碰已經是 `00`，`08` 多餘 | — |
| 組語 7-1 | 對 | | | 抓到：`signed char` 1 byte 在 `%dil`；讀整個 `%rdi` 會帶進上面 7 byte 的垃圾 | — |
| 組語 7-2 | 對 | | | 抓到：`0x556677ff` | — |
| 組語 7-3 | 對 | | | 抓到：正數，最高位 `0x5` 的符號位是 0 | — |
| 組語 7-4 | 對 | 3 → 4（整題） | 2026-11-17 | 抓到：期望只看 `0xff`，讀 4 byte 帶進上面 3 byte 垃圾。小修：直接寫出差別——期望 −1、實際是正的 `0x556677ff` | — |
| 組語 11-1 | 對 | | | 抓到：`*` → 後面是算位址的式子，`deref` ＋ 4。可補：沒 `*` 會被當函式名（linespec） | — |
| 組語 11-2 | 對 | | | 抓到：不一樣；C 的 `*p` 取出指向的值，gdb 的 `*` 表示「後面是位址」 | — |
| 組語 11-3 | 對 | 3 → 4（整題） | 2026-11-17 | 抓到：`*deref` 停第一條 `endbr64`（`0x117b`）；`deref` 跳過開頭準備工作。小修：不是「跑到堆疊頂端」，是停在準備工作之後的那條指令 `0x1187`（跟 10-24、10-27 同一處） | — |
| 組語 15-1 | 對 | | | 抓到：需要借，CF ＝ 1。小修：加法 CF 記進位、減法 CF 記借位，不是「或」起來 | — |
| 組語 15-2 | 對 | | | 抓到：`0xfb` ＝ 251，差 256，因為借的 1 值 2⁸ | — |
| 組語 15-3 | 大致對 | 3 → 3（整題） | 2026-11-10 | 加法器算出 `0xfb`、沒進位對；**關係寫錯**：不是「CF ＝ 借位 ‖ 進位」，是減法時 CF ＝ **NOT** 加法器進位（反例 5 − 3：有進位但 CF ＝ 0） | ✅ 過關（11-03） |
| 組語二 1-1 | 對 | | | 抓到：`%rsp` 減 8 存返回位址、跳到 `add3`。小修：講出存的是 `0x1199`（下一條指令） | — |
| 組語二 1-2 | 對 | | | 抓到：取 `%rsp` 那格當位址跳過去、`%rsp` 加 8 | — |
| 組語二 1-3 | 對 | | | 抓到：`ret` 不知道，從 `%rsp` 那格拿到 `0x1199` | — |
| 組語二 1-4 | 對 | 3 → 4（整題） | 2026-11-17 | 抓到：不會回 `0x1199`、跳到被蓋上的值。小修：明講「`ret` 不檢查」；最常見結果是 segfault，「安靜出錯」少見 | — |
| 組語二 4-1 | 對 | | | 抓到：`x` 跨 `call` 要用、`%rdi` caller-saved → 搬到 callee-saved `%rbx`；`f` 借用要先 `push`、最後 `pop` | — |
| 組語二 4-2 | 對 | 3 → 4（整題） | 2026-11-17 | 抓到：`call` 時 `%rsp` 要 16 的倍數；`call f` 已推 8 byte；再減 8 湊回 | — |
| 多檔 3-1 | 對 | | | 抓到：外面 → `LOCAL`、只有本檔可見；裡面 → 不放堆疊、放 `.bss`／`.data`。小修：裡面的重點是儲存期間（一份、值留著），「變成 `LOCAL`」不是它帶來的改變 | — |
| 多檔 3-2 | 對 | | | 抓到：`LOCAL` 不配對；拿掉變兩個 `GLOBAL` 定義 → `multiple definition` | — |
| 多檔 3-3 | 對 | 3 → 4（整題） | 2026-11-17 | 抓到：`.bss` 值留著；堆疊每次從 0。小修：堆疊不是「被清除」，是每次重新執行 `n = 0` | — |
| 多檔 4-1 | 對 | | | 抓到：兩個 `.c` 都貼進 `hits.h`、各有一份定義 → 連結時重複定義。可補：`long hits;` 是暫定定義 | — |
| 多檔 4-2 | 對 | | | 抓到：guard 只擋同一個編譯單元；分開編譯各貼一次是正常的 | — |
| 多檔 4-3 | 對 | | | 抓到：標頭檔 `extern long hits;`、`hits.c` 定義 `long hits = 0;` | — |
| 多檔 4-4 | 對 | 3 → 4（整題） | 2026-11-17 | 抓到：舊版預設 `-fcommon` → `COM`、連結時合併。小修：標成 `COM` 是編譯時、合併是連結時；gcc 10 起改 `-fno-common` | — |

## 今天的題目

排序：盒子小的在前；同一盒子照上次作答日期、再照筆記與題號。

| # | 題目 | 盒子 | 原本該考的日期 |
|---|---|---|---|
| 1 | 函式庫 3（3 小題） | 1 | 2026-11-03 |
| 2 | 函式庫 12（4 小題） | 1 | 2026-11-03 |
| 3 | 函式庫 1（2 小題） | 2 | 2026-11-03 |
| 4 | 函式庫 5（3 小題） | 2 | 2026-11-03 |
| 5 | 函式庫 7（4 小題） | 2 | 2026-11-03 |
| 6 | objdump 1 | 2 | 2026-11-03 |
| 7 | gcc 2 | 3 | 2026-11-03 |
| 8 | gcc 5 | 3 | 2026-11-03 |
| 9 | ELF 4 | 3 | 2026-11-03 |
| 10 | objdump 4 | 3 | 2026-11-03 |
| 11 | C資料 6 | 3 | 2026-11-03 |
| 12 | C資料 10 | 3 | 2026-11-03 |
| 13 | 字元 5 | 3 | 2026-11-03 |
| 14 | 陣列 4（2 小題） | 3 | 2026-11-03 |
| 15 | 陣列 7（4 小題） | 3 | 2026-11-03 |
| 16 | 組語 7（4 小題） | 3 | 2026-11-03 |
| 17 | 組語 11（3 小題） | 3 | 2026-11-03 |
| 18 | 組語 15（3 小題） | 3 | 2026-11-03 |
| 19 | 組語二 1（4 小題） | 3 | 2026-11-03 |
| 20 | 組語二 4（2 小題） | 3 | 2026-11-03 |
| 21 | 多檔 3（3 小題） | 3 | 2026-11-03 |
| 22 | 多檔 4（4 小題） | 3 | 2026-11-03 |

---

## 函式庫：[靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html)

### 函式庫 3. `-fPIC`：哪個變數出問題、為什麼要 GOT

> 盒子 1｜過去的作答：[s1-07 quiz.md 第 3 題](../small-project/s1-07-libraries/quiz.md#3--fpic哪個變數出問題為什麼要-got)（10-29 第一次作答）、[11-01 複習](./2026-11-01-函式庫-組語.md#函式庫-3--fpic哪個變數出問題為什麼要-got)、[11-02 複習](./2026-11-02-函式庫-interp-C資料-字元-陣列-組語-組語二-多檔.md#函式庫-3--fpic哪個變數出問題為什麼要-got)

下面的 `stats.c` 不加 `-fPIC` 編成 `stats.o`，再用 `gcc -shared stats.o -o libstats.so`。會成功嗎？錯誤訊息會提到哪個變數？為什麼另一個變數不會有問題？為什麼 `.so` 存取全域變數一定要經過 GOT，而不能像執行檔那樣用 `%rip` 加固定距離直接讀（講出兩個理由）？

```c
/* stats.c */
int hits = 0;
static int misses = 0;

void hit(void)  { hits = hits + 1; }
void miss(void) { misses = misses + 1; }
```
編譯：`gcc -Wall -c stats.c -o stats.o && gcc -shared stats.o -o libstats.so`

#### 函式庫 3-1. 會成功嗎？錯誤訊息會提到哪個變數？

```
hits變數,因爲動態庫的全域變數需要用got, 用fPIE會直接而去算%rip與hits的距離
```

> **批改**：**對**。抓到重點：錯誤會提到 `hits`；原因是沒加 `-fPIC` 時（Ubuntu 預設 `-fPIE`）編譯器直接用 `%rip` 加固定距離讀 `hits`，做 `.so` 時 `ld` 不接受。小修：題目第一問「會成功嗎？」你沒有明講，要先寫「**不會成功**」，再講錯誤提到誰；`.so` 的全域變數「需要用 GOT」是 3-3 的內容，這題不必展開。
>
> **參考答案**【[第四節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#為什麼一定要--fpic先看不加會怎樣)〈為什麼一定要 `-fPIC`：先看不加會怎樣〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 不會成功。
> 2. 錯誤提到 `hits`：`relocation R_X86_64_PC32 against symbol 'hits' can not be used when making a shared object; recompile with -fPIC`。
>
> **詳細說明**：本機實測輸出：`warning: relocation against 'hits' in read-only section '.text'`、`relocation R_X86_64_PC32 against symbol 'hits' can not be used when making a shared object; recompile with -fPIC`、`final link failed: bad value`。沒加 `-fPIC`（Ubuntu 預設 `-fPIE`）時，編譯器假設 `hits` 就在附近、用 `R_X86_64_PC32` 直接讀；做 `.so` 時 `ld` 不接受對全域符號用這種重定位。

#### 函式庫 3-2. 為什麼另一個變數不會有問題？

```
misses是靜態變數,只有stats.o的函式會用到,可以直接用%rip算相對距離.
```

> **批改**：**大致對**。核心關鍵 1 有：`misses` 是 `static`，只有 `stats.c`（編成 `stats.o`）的函式用得到。結論「可以直接用 `%rip` 算相對距離」也對。**缺核心關鍵 2 的「為什麼」**：從「只有自己用」到「可以用 `%rip` 直接讀」中間少了一步——別人不能用、也不能用同名定義取代它，所以 `misses` **一定是這個 `.so` 自己那一份**，放在同一個 `.so` 的 `.bss`；`.so` 不管載到哪裡，`.text` 和 `.bss` 一起搬，**兩者的距離在做 `.so` 時就固定了**。這一步正好是 `hits` 不行的地方（`hits` 可能被換成主程式那份，距離做 `.so` 時不知道）。跟 11-02 錯在同一處；11-02 你的自我練習寫了「`%rip` 與他的距離一定是固定的」「一定是在 stats.o 的 .bss，不可能在別人那邊」，這次作答又漏掉——這兩句就是這題的答案。
>
> **參考答案**【[第四節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#為什麼不能直接用固定距離counter-最後不一定是這一份)〈為什麼不能直接用固定距離：`counter` 最後不一定是這一份〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `misses` 是 `static`（`LOCAL`），別的檔案不可能用到它、也不可能用同名定義插入。
> 2. 所以它一定是這個 `.so` 自己那一份，跟程式碼在同一個 `.so` 裡、距離固定，`%rip` 加固定距離就讀得到，不需要 GOT。
>
> **詳細說明**：筆記的 `count_static.c`（`static int counter`）不加 `-fPIC` 也做得成 `.so`，是同一個道理。`.so` 整個被載到哪裡都沒關係：`.text` 和 `.bss` 一起搬，兩者之間的距離不變。

**自我練習**（不批改）

```
因爲misses是靜態變數,靜態變數的符號是local,一定是libstats.so才能用所以可以是固定距離
```

#### 函式庫 3-3. 為什麼 `.so` 存取全域變數一定要經過 GOT，而不能像執行檔那樣用 `%rip` 加固定距離直接讀（講出兩個理由）？

```
1. hits不一定只有libstats.so會用到,執行檔的主程式也可能會用到,因爲ld.so會優先選擇主程式執行檔的定義,所以libstats.so需要靠got裏面hits的位址存取這個值

2. 如此一來,libstats.so 的程式碼 .text才能共用,因爲每個用他的行程位址基點都是不同的,到hits的距離也不同,直接把hits位址
寫進got,在相同的一份.text 可以值存取各自的hits
```

> **批改**：**對**（改判，11-03）。理由 1：`hits` 可能被主程式的同名定義取代，`ld.so` 先找執行檔，所以 `libstats.so` 只能透過 GOT 裡 `ld.so` 填好的位址存取。理由 2：`.text` 要給所有行程共用，而每個行程裡 `libstats.so` 到 `hits` 的距離不同，同一份 `.text` 放不下不同的距離，所以把位址放進 GOT。
> - **改判說明**：第一次批改把「每個行程位址基點不同，到 `hits` 的距離也不同」當成錯（以為指的是 `.so` 自己那份 `hits`，那樣距離不會變）。使用者說明指的是 `.so` 到 `hits`（理由 1 講的主程式那份）的距離——兩個模組各自載入、基點不同，距離確實每個行程都不一樣，這句成立，改判為對。
> - 小修：理由 2 可以直接說出「不能讓 `ld.so` 改 `.text`：它是唯讀的，改了每個行程要各複製一份」，比「`.text` 才能共用」更具體（跟 11-02 同一個小修，不扣分）。
> - 小修：「可以值存取」應是「可以直接存取」。
>
> **參考答案**【[第四節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#為什麼不能直接用固定距離counter-最後不一定是這一份)〈為什麼不能直接用固定距離：`counter` 最後不一定是這一份〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 符號插入：`.so` 的全域變數最後被使用的可能不是它自己那份（執行檔或先載入的 `.so` 有同名定義，`ld.so` 先找執行檔），做 `.so` 時不知道它在哪 → 只能等載入時由 `ld.so` 把真正位址填進 GOT。
> 2. 不能讓 `ld.so` 載入時直接改 `.text` 裡的距離：`.text` 是唯讀的，而且每個行程寫進自己的位址就得各複製一份，同一份 `.text` 實體頁給所有行程共用的好處就沒了；所以把要因行程而異的位址集中在可寫的 GOT。
>
> **詳細說明**：正文〈為什麼不能直接用固定距離〉的 `usecount` 實驗：主程式自己定義 `counter = 100`，`libcount.so` 的 `bump()` 加的是主程式那份，印出 `counter = 101`。第二個理由在同一節〈那為什麼不讓 `ld.so` 載入時直接改 `.text` 裡的距離就好？〉那段，第 9 題也考這件事。補充：基點不同只影響**絕對位址**，同一個 `.so` 內部的相對距離不變（所以 `static` 變數照樣用 `%rip` 直接讀）；距離會因行程而不同，是因為變數可能在**別的模組**裡。


---

### 函式庫 12. `.h` 與 `-l`：各在哪個階段用到（10-26 追問新增）

> 盒子 1｜過去的作答：[s1-07 quiz.md 第 12 題](../small-project/s1-07-libraries/quiz.md#12-h-與--l各在哪個階段用到10-26-追問新增)（10-29 第一次作答）、[11-01 複習](./2026-11-01-函式庫-組語.md#函式庫-12-h-與--l各在哪個階段用到10-26-追問新增)、[11-02 複習](./2026-11-02-函式庫-interp-C資料-字元-陣列-組語-組語二-多檔.md#函式庫-12-h-與--l各在哪個階段用到10-26-追問新增)

`libtemp.so` 的宣告放在 `include/temp.h`（裡面還定義了巨集 `FREEZING_C`），做好的 `libtemp.so` 放在 `lib/`。(1) `gcc -Wall main.c -Llib -ltemp -o app` 會失敗在哪個階段（前處理、編譯、連結、執行）？錯誤大概長怎樣、要補什麼？(2) `gcc -Wall -Iinclude main.c -Llib -o app` 呢？(3) 同事說「`FREEZING_C` 的值存在 `libtemp.so` 裡，執行時才讀進來」，對嗎？為什麼？(4) 編好的 `app`（連結時加了 `-Wl,-rpath,'$ORIGIN/lib'`）要拿給別人執行，`include/temp.h` 要一起給嗎？`lib/libtemp.so` 呢？

```c
/* include/temp.h */
#ifndef TEMP_H
#define TEMP_H

#define FREEZING_C 0.0

double c_to_f(double c);

#endif
```
```c
/* temp.c */
#include "temp.h"

double c_to_f(double c)
{
    return c * 9.0 / 5.0 + 32.0;
}
```
```c
/* main.c */
#include <stdio.h>
#include "temp.h"

int main(void)
{
    printf("%.1f\n", c_to_f(FREEZING_C));
    return 0;
}
```
做 `.so`：`gcc -Wall -fPIC -Iinclude -c temp.c -o temp.pic.o && gcc -shared temp.pic.o -o lib/libtemp.so`（檔案：[`include/temp.h`](../small-project/s1-07-libraries/examples/headers/temp/include/temp.h)、[`temp.c`](../small-project/s1-07-libraries/examples/headers/temp/temp.c)、[`main.c`](../small-project/s1-07-libraries/examples/headers/temp/main.c)）。

#### 函式庫 12-1. `gcc -Wall main.c -Llib -ltemp -o app` 會失敗在哪個階段（前處理、編譯、連結、執行）？錯誤大概長怎樣、要補什麼？

```
前處理

錯誤:找不到temp.h

補上-Iinclude
```

> **批改**：**對**。階段（前處理）、錯誤（找不到 `temp.h`）、要補 `-Iinclude` 都對。小修：錯誤原文是 `fatal error: temp.h: No such file or directory`，記住 `fatal error: xxx.h` 這個長相，一看就知道是前處理。
>
> **參考答案**【[第九節第 11 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#11-做-so-需要-h-嗎include-管編譯-l-管連結)〈做 `.so` 需要 `.h` 嗎？`#include` 管編譯、`-l` 管連結〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 失敗在**前處理**：`fatal error: temp.h: No such file or directory`。
> 2. `#include "temp.h"` 先找 `main.c` 所在的目錄、再找 `-I` 指定的目錄；`temp.h` 在 `include/`，要補 `-Iinclude`。
>
> **詳細說明**：本機實測輸出：`main.c:2:10: fatal error: temp.h: No such file or directory`、`compilation terminated.`。四個階段的錯誤長相不同：前處理是 `fatal error: xxx.h`、連結是 `undefined reference`／`cannot find -l`、執行是 `error while loading shared libraries`。

#### 函式庫 12-2. `gcc -Wall -Iinclude main.c -Llib -o app` 呢？

```
連接的時候出錯

差 -ltemp, 也差runpath的路徑

gcc -Wall -Iinclude main.c -Llib -ltemp -Wl,-rpath,'$ORIGIN/lib' -o app
```

> **批改**：**大致對**。階段（連結）和要補 `-ltemp` 對，指令也寫對了（`-ltemp` 在 `main.c` 後面）。**缺核心關鍵 1 的錯誤長相**：這題「呢？」問的跟 12-1 一樣是「哪個階段、錯誤大概長怎樣、要補什麼」，你沒寫錯誤——是 `undefined reference to 'c_to_f'`（`.h` 給了宣告所以編譯會過，`c_to_f` 的本體要到連結才找，找不到）。11-02 你有寫出來，這次漏了。小修：「也差 runpath」不是這個命令失敗的原因——沒有 `-rpath`，連結照樣成功，是**執行**時 `ld.so` 找不到 `libtemp.so`（`error while loading shared libraries`）。補上它沒錯，但要分清楚它解決的是哪個階段的問題。
>
> **參考答案**【[第九節第 11 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#11-做-so-需要-h-嗎include-管編譯-l-管連結)〈做 `.so` 需要 `.h` 嗎？`#include` 管編譯、`-l` 管連結〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 失敗在**連結**：`undefined reference to 'c_to_f'`。
> 2. 要補 `-ltemp`（寫在 `main.c` 後面），`ld` 才會去 `-Llib` 找 `libtemp.so`、解決 `c_to_f`。
>
> **詳細說明**：本機實測輸出：`main.c:(.text+0x15): undefined reference to 'c_to_f'`。為什麼前處理、編譯都會過：有 `-Iinclude` 找得到 `temp.h`，而 `.h` 只給 `c_to_f` 的宣告（型別），編譯器有宣告就能編；本體要到連結時才找。對照筆記第 11 章的 `sq.c`：有 `#include <math.h>`、沒 `-lm`，一樣是 `undefined reference to 'sqrt'`。`-Wl,-rpath,'$ORIGIN/lib'` 解決的是下一關（執行時找 `.so`），不是連結。

**自我練習**（不批改）

```
連接的時候出錯,錯誤是沒有定義c_to_f

差 -ltemp, 也差runpath的路徑

gcc -Wall -Iinclude main.c -Llib -ltemp -Wl,-rpath,'$ORIGIN/lib' -o app
```

#### 函式庫 12-3. 同事說「`FREEZING_C` 的值存在 `libtemp.so` 裡，執行時才讀進來」，對嗎？為什麼？

```
不對 FREEZING_C是常量,在前處理的時候就已經讀進來了
```

> **批改**：**大致對**。「不對」、「前處理時就處理掉了」這條主線對，比 11-02 的「符號連接、執行檔優先」進步很多。但核心關鍵 2 不完整，而且有一個用詞會讓理由變錯：
> - **「常量」要說「巨集」**：這不只是用詞。如果 `FREEZING_C` 是**常量變數**（例如 `temp.c` 裡寫 `const double FREEZING_C = 0.0;`、`.h` 寫 `extern const double FREEZING_C;`），它就是一個符號，值真的存在 `libtemp.so` 的 `.rodata`，同事的說法就**對**了。它不在 `.so` 裡，是因為它是 `#define` 巨集——前處理器做的是**文字替換**。
> - **缺「值最後在哪」**：「前處理的時候就已經讀進來了」沒講讀進哪裡。要講出：前處理把 `FREEZING_C` 換成 `0.0`，所以 `0.0` 編進 `main.c` 編出來的程式碼（`app` 裡），`libtemp.so` 裡根本沒有 `FREEZING_C`。這才回答了同事「存在 `libtemp.so` 裡」那半句。
>
> **參考答案**【[第九節第 11 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#11-做-so-需要-h-嗎include-管編譯-l-管連結)〈做 `.so` 需要 `.h` 嗎？`#include` 管編譯、`-l` 管連結〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 不對。
> 2. `FREEZING_C` 是巨集，前處理時就被換成 `0.0`，寫進 `main.c` 編出來的程式碼；`libtemp.so` 裡根本沒有這個東西。
>
> **詳細說明**：本機實測：`gcc -E -Iinclude main.c` 的最後幾行是 `printf("%.1f\n", c_to_f(0.0));`。所以改了 `temp.h` 的 `FREEZING_C`，只換 `.so` 沒用，要重新編譯 `main.c`。用 `nm -D lib/libtemp.so` 也只看得到 `c_to_f`，找不到 `FREEZING_C`。對照（本機實測）：改成 `temp.c` 裡定義 `const double FREEZING_C = 0.0;`、`main.c` 寫 `extern const double FREEZING_C;`，`nm -D` 看得到 `R FREEZING_C`（存在 `.so` 的 `.rodata`），`app` 有一條 `R_X86_64_COPY FREEZING_C`（載入時 `ld.so` 從 `.so` 複製過來，第 11 題的機制）；把 `.so` 改成 `5.0` 重做、`app` 不重新連結，印出 `5.0`——這時同事的說法就對了。

**自我練習**（不批改）

```
不對 FREEZING_C是巨集,在前處理的時候就已經讀進來了

main.c的FREEZING_C會被0.0替換掉
```

#### 函式庫 12-4. 編好的 `app`（連結時加了 `-Wl,-rpath,'$ORIGIN/lib'`）要拿給別人執行，`include/temp.h` 要一起給嗎？`lib/libtemp.so` 呢？

```
temp.h不用,前處理已經把內容都寫進去了

libtemp.so需要,執行時,ld.so載入庫需要
```

> **批改**：**對**。`.h` 不用給（前處理時內容已經編進 `app`）；`.so` 要給（執行時 `ld.so` 要載入它）。可補：`ld.so` 照 `NEEDED libtemp.so` 和 `RUNPATH $ORIGIN/lib` 找，所以要放在 `app` 旁邊的 `lib/`（跟 11-02 同一個可補）。
>
> **參考答案**【[第九節第 11 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#11-做-so-需要-h-嗎include-管編譯-l-管連結)〈做 `.so` 需要 `.h` 嗎？`#include` 管編譯、`-l` 管連結〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `include/temp.h` 不用給：它只在編譯呼叫端時用，內容已經編進 `app`。
> 2. `lib/libtemp.so` 要給：`c_to_f` 的程式碼在裡面，執行時 `ld.so` 照 `NEEDED` 和 `RUNPATH $ORIGIN/lib` 載入它，所以要放在 `app` 旁邊的 `lib/`。
>
> **詳細說明**：這就是 `zlib1g`（執行用，只有 `.so.1`）和 `zlib1g-dev`（開發用，`.h`、`.a`、`.so`）分成兩個套件的原因。


---

### 函式庫 1. 靜態函式庫：哪些函式會進執行檔

> 盒子 2｜過去的作答：[s1-07 quiz.md 第 1 題](../small-project/s1-07-libraries/quiz.md#1-靜態函式庫哪些函式會進執行檔)（10-29 第一次作答）

一個靜態函式庫 `libtxt.a` 由三個成員組成，`main.c` 只呼叫 `trim`。最後的執行檔裡會有哪些函式（`to_upper_str`、`to_lower_str`、`trim`、`is_space`）？為什麼？如果把連結指令改成直接寫三個 `.o`（`gcc main.c case.o trim.o space.o`），結果有什麼不同、為什麼？

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

#### 函式庫 1-1. 最後的執行檔裡會有哪些函式（`to_upper_str`、`to_lower_str`、`trim`、`is_space`）？為什麼？

```
trim, is_space
因爲ld在做連接的時候會先看符號清單,主程式用到那些,主程式有用到trim, 而trim(trim.o) 又有用到is_space(space.o)
所以實際上連結後的程式只會用trim.o與space.o
```

> **批改**：**對**。抓到重點：只有 `trim`、`is_space`；`main` 要 `trim` → 拉 `trim.o`，`trim.o` 又要 `is_space` → 拉 `space.o`，`case.o` 沒人要就不拉。比 10-29 多講了「以 `.o` 為單位」，很好。小修：`ld` 看的是「**還沒定義的符號**」清單（未定義符號），不是一般的符號清單；而且拉的單位是整個成員 `.o`，所以如果有人用到 `to_upper_str`，同一個 `case.o` 裡的 `to_lower_str` 也會跟著進來。
>
> **參考答案**【[第二節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#連結只拉需要的成員)〈連結：只拉需要的成員〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 執行檔裡只有 `trim` 和 `is_space`。
> 2. `ld` 從 `.a` 只拉能解決「目前還沒定義的符號」的成員：`main` 要 `trim` → 拉 `trim.o`；`trim.o` 要 `is_space` → 再拉 `space.o`；`case.o` 沒人要，整個不拉。
>
> **詳細說明**：拉的單位是**整個 `.o`**：如果有人用到 `to_upper_str`，同在 `case.o` 的 `to_lower_str` 也會跟著進來（筆記的 `both.c`）。成員在封存檔裡的順序不影響：本機實測 `ar t libtxt.a` 是 `case.o space.o trim.o`，`space.o` 排在 `trim.o` 前面，照樣拉得到——`ld` 在**同一個** `.a` 裡會反覆看成員，直到沒有新的符號被解決；「只掃一次、不回頭」是對命令列上前後不同的輸入檔而言（第 2 題）。驗證：`nm app | grep -E 'trim|is_space|to_'` 只列出 `trim`、`is_space`。

#### 函式庫 1-2. 如果把連結指令改成直接寫三個 `.o`（`gcc main.c case.o trim.o space.o`），結果有什麼不同、為什麼？

```
這個命令就會把所有的.o都打入到一個執行檔了,不管有沒有引入到.
與靜態函式庫不同,靜態函式庫是真的有用到,你纔會打包那個.o到執行檔
```

> **批改**：**對**。抓到重點：命令列上直接寫的 `.o` 不管有沒有人用，一律整個放進執行檔，所以 `case.o` 的 `to_upper_str`、`to_lower_str` 也進去了；`.a` 才是「有人需要才拉」。可補：執行結果一樣（還是印 `[hello]`），只是執行檔變大（實測 16136 → 16240 byte）。
>
> **參考答案**【[第二節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#連結只拉需要的成員)〈連結：只拉需要的成員〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 執行檔多了 `to_upper_str`、`to_lower_str`（執行結果一樣，檔案變大）。
> 2. 命令列上直接寫的 `.o` 一律整個放進輸出檔，不管有沒有人用；只有封存檔裡的成員才是「有人需要才拉」。
>
> **詳細說明**：本機實測：`nm app2` 四個函式都在；`app` 16136 byte、`app2` 16240 byte。


---

### 函式庫 5. SONAME 與函式庫更新

> 盒子 2｜過去的作答：[s1-07 quiz.md 第 5 題](../small-project/s1-07-libraries/quiz.md#5-soname-與函式庫更新)（10-29 第一次作答）

你維護 `libcalc`，SONAME 是 `libcalc.so.1`。系統上有 `libcalc.so -> libcalc.so.1`、`libcalc.so.1 -> libcalc.so.1.0.0` 兩個符號連結。(1) 一支程式動態連結了 `libcalc`，它的 `NEEDED` 寫的是哪個名字？為什麼刪掉 `libcalc.so` 它照樣能跑，刪掉 `libcalc.so.1` 就不行？(2) 你修了一個 bug（介面不變），做成 `libcalc.so.1.0.1`。已經動態連結的程式要怎樣才能用到修正？另一支用 `libcalc.a` 靜態連結的程式呢？(3) 下一版要把 `calc_add` 改成三個參數（不相容），SONAME 該怎麼辦？為什麼這樣舊程式不會壞？

#### 函式庫 5-1. 一支程式動態連結了 `libcalc`，它的 `NEEDED` 寫的是哪個名字？為什麼刪掉 `libcalc.so` 它照樣能跑，刪掉 `libcalc.so.1` 就不行？

```
是libcalc.so.1 題目寫明瞭這纔是他的solname, 因爲needed寫的是libcalc.so.1不是libcalc.so
```

> **批改**：**對**。抓到重點：`NEEDED` 寫的是 SONAME `libcalc.so.1`，不是 `libcalc.so`——所以執行時找的是 `.so.1`，`libcalc.so` 刪了無所謂、`.so.1` 刪了就找不到。可補兩句讓因果更完整：(a) `ld` 照 `-lcalc` 打開的是 `libcalc.so`，但記進 `NEEDED` 的是檔案裡寫的 SONAME；(b) `libcalc.so`（連結名稱）只在**連結時**給 `-lcalc` 用，執行時 `ld.so` 用不到它。小修：拼字是 SONAME，不是 solname。
>
> **參考答案**【[第六節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#sonameneeded-記的是哪個名字)〈SONAME：`NEEDED` 記的是哪個名字〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `NEEDED` 寫的是 SONAME `libcalc.so.1`：`ld` 照 `-lcalc` 打開 `libcalc.so`，但記下的是那個檔案裡寫的 SONAME，不是檔名。
> 2. `libcalc.so`（連結名稱）只在連結時給 `-lcalc` 用；執行時 `ld.so` 找的是 `libcalc.so.1`，所以刪掉它就找不到。
>
> **詳細說明**：這就是開發套件（`zlib1g-dev`，有 `libz.so`）和執行套件（`zlib1g`，只有 `libz.so.1`）分開的原因：只裝執行套件，程式能跑，但不能 `-lz` 連結。

#### 函式庫 5-2. 你修了一個 bug（介面不變），做成 `libcalc.so.1.0.1`。已經動態連結的程式要怎樣才能用到修正？另一支用 `libcalc.a` 靜態連結的程式呢？

```
ln -sf libcalc.so.1.0.1 libcalc.so.1
```

> **批改**：**不完整**。動態那一半的做法對：把 `libcalc.so.1` 改指向 `libcalc.so.1.0.1`。但這題問了兩件事，**靜態那一半沒答**：用 `libcalc.a` 靜態連結的程式，`.o` 在連結時已經**複製進執行檔**，換掉函式庫檔案影響不到它，要拿新的 `libcalc.a` **重新連結**才用得到修正（`main.c` 不必重編）。動態那一半也可以補一句「為什麼這樣就夠」：SONAME 不變，程式的 `NEEDED` 還是 `libcalc.so.1`，不用重新連結，重新執行時 `ld.so` 照連結找到的就是新檔。10-29 第一次作答兩半都有，這次漏了靜態。
>
> **參考答案**【[第六節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#sonameneeded-記的是哪個名字)〈SONAME：`NEEDED` 記的是哪個名字〉、[第六節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#換掉-so程式不用重新連結)〈換掉 `.so`：程式不用重新連結〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 動態：SONAME 不變（`libcalc.so.1`），把 `libcalc.so.1` 改指向 `libcalc.so.1.0.1`；程式不必重新連結，重新執行就用到修正。
> 2. 靜態：`libcalc.a` 的 `.o` 已經複製進執行檔，換函式庫影響不到它，要用新的 `libcalc.a` 重新連結。
>
> **詳細說明**：`ldconfig` 會照各 `.so` 的 SONAME 自動建立 SONAME 符號連結。已經在跑的行程要重開才會載入新檔。

**自我練習**（不批改）

```
ln -sf libcalc.so.1.0.1 libcalc.so.1

需要重現編譯
```

#### 函式庫 5-3. 下一版要把 `calc_add` 改成三個參數（不相容），SONAME 該怎麼辦？為什麼這樣舊程式不會壞？

```
編譯新的so 用新的soname libcalc.so.2
這樣舊的程式還是繼續用libcalc.so.1
```

> **批改**：**對**（改判，11-03）。核心關鍵 1：新版用新的 SONAME `libcalc.so.2`。核心關鍵 2：「舊的程式還是繼續用 `libcalc.so.1`」——舊程式載入的仍是舊版，不會拿到參數不相容的新版，這就是不會壞的原因；而「為什麼還找得到 `.so.1`」（`NEEDED` 寫的是 `libcalc.so.1`）你在同一題的 5-1 已經答了。第一次批改要求在 5-3 再寫一次 `NEEDED`，太嚴，改判為對。可補：舊檔要留著（兩版並存）；`libcalc.so` 改指向新版，新程式 `-lcalc` 才連到 `.so.2`。
>
> **參考答案**【[第六節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#sonameneeded-記的是哪個名字)〈SONAME：`NEEDED` 記的是哪個名字〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 新版 SONAME 改成 `libcalc.so.2`（`-Wl,-soname,libcalc.so.2`，真實檔例如 `libcalc.so.2.0.0`，`libcalc.so.2` 指向它，`libcalc.so` 改指向新版給新程式連結）。
> 2. 舊程式的 `NEEDED` 寫的是 `libcalc.so.1`，執行時照樣找到舊檔（舊檔留著、兩版並存），不會拿到參數不相容的新版；重新連結的新程式才記下 `libcalc.so.2`。
>
> **詳細說明**：反過來，如果不相容卻沒換 SONAME，舊程式會載入新版、用兩個參數呼叫三個參數的 `calc_add`，第三個參數是暫存器裡的垃圾值。


---

### 函式庫 7. 兩份同名全域變數：接 `.so` 為什麼不算重複定義（10-25 追問新增）

> 盒子 2｜過去的作答：[s1-07 quiz.md 第 7 題](../small-project/s1-07-libraries/quiz.md#7-兩份同名全域變數接-so-為什麼不算重複定義10-25-追問新增)（10-29 第一次作答）

`libconf.so` 和主程式 `app.c` 都定義了 `verbose`。(1) `gcc -Wall app.c -L. -lconf -Wl,-rpath,'$ORIGIN' -o app` 會不會出現 `multiple definition`？為什麼？改成 `gcc -Wall app.c conf.pic.o -o app2` 呢？差別在 `ld` 怎麼處理 `.so` 和 `.o`。(2) 能連結成功的那一個，`./app` 會印出 `get_verbose() = ` 多少？用執行時 `ld.so` 怎麼找符號解釋。(3) 把 `conf.c` 第一行改成 `static int verbose = 0;`，重做 `libconf.so`，不重新連結就執行 `./app`，印出多少？為什麼跟 (2) 不同？

```c
/* conf.c */
int verbose = 0;

int get_verbose(void)
{
    return verbose;
}
```
```c
/* app.c */
#include <stdio.h>

int verbose = 3;
int get_verbose(void);

int main(void)
{
    printf("get_verbose() = %d\n", get_verbose());
    return 0;
}
```
做函式庫：`gcc -Wall -fPIC -c conf.c -o conf.pic.o && gcc -shared conf.pic.o -o libconf.so`（檔案：[`conf.c`](../small-project/s1-07-libraries/examples/pic/conf.c)、[`app.c`](../small-project/s1-07-libraries/examples/pic/app.c)）

#### 函式庫 7-1. `gcc -Wall app.c -L. -lconf -Wl,-rpath,'$ORIGIN' -o app` 會不會出現 `multiple definition`？為什麼？

```
不會,因爲ld.so在動態載入庫的時候,會優先選擇執行檔的定義
```

> **批改**：**大致對**。「不會」對，但寫的理由**放錯階段**：`multiple definition` 是**連結時** `ld` 報的錯，`ld.so` 是**執行時**才登場——連結的時候 `ld.so` 根本還沒跑，「`ld.so` 優先選執行檔」解釋不了 `ld` 為什麼不報錯（那是 7-3 的答案）。正確的理由在連結階段：`ld` 不把 `.so` 的內容複製進輸出檔，只讀它的動態符號表、記進 `NEEDED`，所以 `app` 裡只有 `app.c` 那一份 `verbose`；`multiple definition` 只檢查放進同一個輸出檔的定義。你 7-2 第一段其實寫出了這個機制（`.so` → 查未定義符號、寫 `NEEDED`），只是沒拿來回答 7-1，所以判大致對、不判錯。
>
> **參考答案**【[第九節第 2 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#2-兩份-counter接-so-為什麼不算重複定義)〈兩份 `counter`：接 `.so` 為什麼不算重複定義〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 不會。
> 2. `ld` 不把 `.so` 的內容複製進輸出檔，只讀它的動態符號表（解決未定義符號）並記進 `NEEDED`；輸出檔裡只有 `app.c` 那份 `verbose`，`multiple definition` 只檢查放進同一個輸出檔的定義。
>
> **詳細說明**：連結成功後兩份 `verbose` 都還在，各在各的檔案裡，要到執行時才由 `ld.so` 挑一份。分清楚兩個階段：**連結時**（`ld`）決定會不會報 `multiple definition`；**執行時**（`ld.so`）決定用哪一份（7-3）。

**自我練習**（不批改）

```
不會,ld在做連接的時候只是去檢查app.c的的未定義的符號是否能與libconf.so連結,能的話就會記錄需求單
```

#### 函式庫 7-2. 改成 `gcc -Wall app.c conf.pic.o -o app2` 呢？差別在 `ld` 怎麼處理 `.so` 和 `.o`。

```
原本so是ld檢查未定義的符號清單,如果.so有,會把.so寫入到needed,然後verbose會優先用執行檔的

如果是.o的話,ld會直接做符號連結,並且把.o一起加入執行檔
但是兩個都有定義verbose所以會有重複定義的錯誤
```

> **批改**：**對**。抓到重點：會出現重複定義；`.o` 會被整個加進執行檔，`app.c` 和 `conf.pic.o` 的 `verbose` 都進了 `app2`。對照 `.so` 那段也對：`ld` 只查它能不能解決未定義符號、寫進 `NEEDED`。小修：「`ld` 會直接做符號連結」改成「把 `.o` 的內容複製進輸出檔」比較準；錯誤原文是 `multiple definition of 'verbose'`。
>
> **參考答案**【[第九節第 2 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#2-兩份-counter接-so-為什麼不算重複定義)〈兩份 `counter`：接 `.so` 為什麼不算重複定義〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 會出現 `multiple definition of 'verbose'`。
> 2. `.o` 的內容會被整個**複製進**輸出檔，`app.c` 和 `conf.pic.o` 的 `verbose` 都進了 `app2`，同一個輸出檔裡兩份定義。
>
> **詳細說明**：同樣兩份定義，接 `.o` 是連結錯誤、接 `.so` 是執行時的符號插入。

#### 函式庫 7-3. 能連結成功的那一個，`./app` 會印出 `get_verbose() = ` 多少？用執行時 `ld.so` 怎麼找符號解釋。

```
3

ld.so會優先選擇執行檔的verbose,並且把verbose在主程式的位址寫到.so的got,.so就可以透過got找到verbose了
```

> **批改**：**對**。抓到重點：印 3；`ld.so` 找符號先找執行檔，把主程式那份 `verbose` 的位址填進 `libconf.so` 的 GOT，`get_verbose` 經 GOT 讀到 3。可補：找不到才依 `NEEDED` 順序找各個 `.so`，第一個找到的贏（符號插入）。
>
> **參考答案**【[第四節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#為什麼不能直接用固定距離counter-最後不一定是這一份)〈為什麼不能直接用固定距離：`counter` 最後不一定是這一份〉、[第九節第 2 章](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#2-兩份-counter接-so-為什麼不算重複定義)〈兩份 `counter`：接 `.so` 為什麼不算重複定義〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 印 `get_verbose() = 3`。
> 2. `ld.so` 找符號**先找執行檔、再依 `NEEDED` 順序找各個 `.so`**，第一個找到的贏（符號插入）；所以 `libconf.so` 的 GOT 裡 `verbose` 那格填的是執行檔那份的位址，`get_verbose` 經過 GOT 讀到 3。
>
> **詳細說明**：`LD_DEBUG=bindings ./app` 會看到 `binding file …/libconf.so to ./app: normal symbol 'verbose'`。`libconf.so` 自己那份（初值 0）一直在記憶體裡，只是沒人用。

#### 函式庫 7-4. 把 `conf.c` 第一行改成 `static int verbose = 0;`，重做 `libconf.so`，不重新連結就執行 `./app`，印出多少？為什麼跟 7-3 不同？

```
0

.so在執行的時候 verbose是靜態變數,get_verbose裏面就不會用got了,而是直接用與%rip的相對距離
```

> **批改**：**對**。抓到重點：印 0；`static` 讓 `get_verbose` 不經 GOT，直接用 `%rip` 加固定距離讀，所以 `ld.so` 沒機會把它接到執行檔那份。可補：讀到的是 `.so` **自己那份**（初值 0），因為 `static` 是 `LOCAL`、別人插入不了；執行檔不必重新連結也會變，因為讀法寫在 `.so` 的 `get_verbose` 裡。
>
> **參考答案**【[第四節](../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#為什麼不能直接用固定距離counter-最後不一定是這一份)〈為什麼不能直接用固定距離：`counter` 最後不一定是這一份〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 印 `get_verbose() = 0`。
> 2. `static` 讓 `verbose` 變成 `LOCAL`，別人不可能插入：編譯器直接用 `%rip` 加固定距離讀 `.so` 自己那份，不經過 GOT，`ld.so` 也不會把它接到執行檔的 `verbose`。
>
> **詳細說明**：執行檔不用重新連結也會變：`get_verbose` 的程式碼在 `.so` 裡，換掉 `.so` 就換了讀法。


---


## gcc：[gcc最佳化等級-O0到O3差在哪](../html/c/gcc最佳化等級-O0到O3差在哪.html)

### gcc 2. 為什麼 `-O0` 的組合語言每一圈都在讀寫記憶體（`-4(%rbp)`），而 `-O2` 不會？這個差別對 `gdb` 有什麼影響？

> 盒子 3｜過去的作答：[s1-01 quiz.md gcc 第 2 題](../small-project/s1-01-hello-compile/quiz.md#2-為什麼--o0-的組合語言每一圈都在讀寫記憶體-4rbp而--o2-不會這個差別對-gdb-有什麼影響)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#gcc-2-為什麼--o0-的組合語言每一圈都在讀寫記憶體-4rbp而--o2-不會這個差別對-gdb-有什麼影響)

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

> 題目：為什麼 `-O0` 的組合語言每一圈都在讀寫記憶體（`-4(%rbp)`），而 `-O2` 不會？這個差別對 `gdb` 有什麼影響？

```
-O2相對於-O0的最佳化 是用空間換取時間的做法,他把迴圈擴展,並且捨棄了把區域變數放在記憶體堆疊,改採用寄存器儲存這些數字

gdb如果想去打印區域變數,很有可能遇到optimized out, 因爲變數已經不是放在函式的堆疊上了,而寄存器不能不保證當前存的就是這個變數
```

> **批改**：**大致對**。核心關鍵 2 對：`-O2` 不把區域變數放在堆疊，改放在暫存器。`<optimized out>` 的原因也寫得比 10-27 準：變數不在堆疊上、暫存器不保證此刻裝的是它（就是 10-27 小修說的「沒有固定的家」）。但有兩處沒答到：
> - **核心關鍵 1：`-O0` 那一半沒正面回答**。題目先問「為什麼 `-O0` 每一圈都在讀寫記憶體」，你只從 `-O2`「捨棄了放在堆疊」反推。要寫出：`-O0` 不最佳化、照原始碼字面翻譯，每個變數都有**固定的記憶體位置**（`i` 是 `-4(%rbp)`、`total` 是 `-8(%rbp)`），每條語句都從記憶體讀、算完再寫回去。10-27 你有寫到「`-O0` 沒有做任何最佳化，所以會把區域變數存在堆疊裡」。
> - **核心關鍵 3 少一半**：gdb 的影響只寫了 `<optimized out>`，漏了**行號會跳**（指令被重排、合併，單步時跳過某些行，例如第三節實測跳過第 3 行），也沒對照 `-O0` 每一行都對得到一段指令、變數隨時印得出來。
> - 小修：「用空間換取時間」不是這題的重點——`-O2` 這裡快的主要原因是**變數放暫存器、不碰記憶體**；而且 `-O2` 一般不會為了速度大幅加大程式碼（那比較像 `-O3`）。「寄存器」台灣用語是「暫存器」。
>
> **參考答案**【[第二節](../html/c/gcc最佳化等級-O0到O3差在哪.html#二實驗同一段程式不同等級編出什麼)、[第三節](../html/c/gcc最佳化等級-O0到O3差在哪.html#三為什麼除錯要用--o0)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `-O0` 不最佳化、照字面翻譯：每個變數都有**固定的記憶體位置**（`i` 是 `-4(%rbp)`、`total` 是 `-8(%rbp)`），每條語句都從記憶體讀、再寫回記憶體。
> 2. `-O2` 把變數留在**暫存器**（`i` 在 `%eax`、`total` 在 `%edx`），迴圈裡完全不碰記憶體。
> 3. 對 gdb 的影響：`-O0` 原始碼的每一行都對得到一段指令，變數隨時印得出來；`-O2` 行號會跳、指令被重排，變數可能顯示 `<optimized out>`。
>
> **詳細說明**：`-O2` 版裡的 `andl $1, %edi` 不是計數，是在進迴圈前判斷 `n` 的奇偶，好讓迴圈一圈加兩個數。第三節的實測：`-O2` 停下來時跳過了第 3 行；只走兩行，`total` 就已經是 3；被內嵌進 `main` 的那一份變數全是 `<optimized out>`。所以學習、單步除錯用 `-O0 -g`，日常開發用 `-Og -g`。`<optimized out>` 的原因是變數**沒有固定的家**：值只在某一段指令裡待在某個暫存器，之後那個暫存器拿去放別的、或整個被算掉，除錯資訊在那個位置就說不出它在哪。

**自我練習**（不批改）

```
-O0: 不做任何最佳化,直接把程式碼轉成相對應的組語,區域變數也是直觀的存在函式堆疊上,要用的時候存取
-O2: 做了迴圈擴展,與把所有變數都存在暫存器,提升速度

對gdb的影響是打印變數會獲得optimized out因爲暫存器無法保證此刻還是區域變數
而且-O2會造成行號跳號
```


---

### gcc 5. 情境題：你在自己的程式裡用 `-O2` 編譯，`gdb` 印某個變數時顯示 `<optimized out>`。你會怎麼處理？如果是在 kernel 裡遇到同樣的狀況，處理方式又有什麼不同？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-情境題你在自己的程式裡用--o2-編譯gdb-印某個變數時顯示-optimized-out你會怎麼處理如果是在-kernel-裡遇到同樣的狀況處理方式又有什麼不同)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md)、[10-16 複習](./2026-10-16-interp-ELF-gcc-GNU-陣列-C資料.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#gcc-5-情境題你在自己的程式裡用--o2-編譯gdb-印某個變數時顯示-optimized-out你會怎麼處理如果是在-kernel-裡遇到同樣的狀況處理方式又有什麼不同)

```
改成用-O0編譯,這樣才能正常的打印變數

kernel無法用-O0, 太多死碼與擴展迴圈的寫法,能做的只能用printk打印出想看的變數
```

> **批改**：**對**。抓到：自己的程式改用 `-O0` 重編，gdb 就印得出變數；kernel 不能用 `-O0`，改用 `printk` 把想看的值印出來。兩個核心關鍵都有。
>
> **要注意：kernel 不能 `-O0` 的理由第三次寫歪了**（10-24、10-27 都是這句）。不是「kernel 裡有太多死碼與迴圈擴展」——是 kernel 的程式碼**依賴**編譯器一定會做的最佳化才編得過：例如有些函式**必須**被內嵌（inline），有些只在某種設定下才用到的程式碼**要靠編譯器刪掉死碼**，不然會連到不存在的符號。跟迴圈展開無關，也不是「死碼太多」，而是「需要編譯器把死碼刪掉」。理由不在核心關鍵裡，不扣分，但同一句錯了三次，下次請寫成「kernel 依賴內嵌與刪除死碼」。
>
> 小修：自己的程式寫 `-O0 -g`（沒有 `-g` 就沒有除錯資訊，gdb 連變數名都不知道）；日常開發也可以用 `-Og -g`。不能重編時，加 `printf` 也是一條路。
>
> **參考答案**【[第三節](../html/c/gcc最佳化等級-O0到O3差在哪.html#三為什麼除錯要用--o0)、[第四節](../html/c/gcc最佳化等級-O0到O3差在哪.html#四跟-kernel-的關係)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 自己的程式：用 `-O0 -g`（或 `-Og -g`）重編，變數有固定位置，gdb 就印得出來；不能重編時，加 `printf` 把值印出來。
> 2. kernel：**不能用 `-O0` 編**（kernel 依賴編譯器一定會做的內嵌與刪除死碼才編得過），`<optimized out>` 是常態；只能改用 `printk`、tracing 這類方法觀察，並學會讀最佳化後的組合語言。
>
> **詳細說明**：kernel 不連結 libc，沒有 `printf`，也沒有 stdout；`printk` 的訊息寫進 kernel 的紀錄緩衝區，用 `dmesg` 看。一句話：自己的程式可以「改編譯方式」，kernel 只能「改觀察方式」。

**自我練習**（不批改）

```
kernel用-O0編不過,

他需要-O2 幫他刪除死碼與程式內嵌
```


---


## ELF：[ELF是什麼-可執行與可連結格式的兩種視角](../html/c/ELF是什麼-可執行與可連結格式的兩種視角.html)

### ELF 4. 區段（section）和載入段（segment）差在哪？為什麼核心載入時要把很多區段打包成少數幾個載入段？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-區段section和載入段segment差在哪為什麼核心載入時要把很多區段打包成少數幾個載入段)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#elf-4-區段section和載入段segment差在哪為什麼核心載入時要把很多區段打包成少數幾個載入段)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#elf-4-區段section和載入段segment差在哪為什麼核心載入時要把很多區段打包成少數幾個載入段)

```
section比較小,每個section都有自己獨特的功能
而segment是按section的權限打包而成的,目的是爲了符合核心管理記憶體的方式
核心管理記憶體是一頁一頁管理,而同一頁只能有一個權限,假如我們沒有把section打包稱segment,那可能就會產生很多頁,造成浪費(1頁最少要4096byte)
```

> **批改**：**對**（改判，11-03）。核心關鍵 1：區段比較小、依功能切（每個有自己的功能），載入段是依**權限**打包、為了配合核心管理記憶體——這就答出兩者差在哪。核心關鍵 2：權限以一頁（4096 byte）為單位、同一頁只能一種權限，所以同權限的區段打包，不然各占一頁會浪費。第一次批改要求寫出「區段是連結視角、給 `ld` 用」，但題目只問「差在哪」，沒有問誰的視角，這點移到詳細說明、不當核心關鍵（筆記的參考答案同步改了）。
>
> 小修：「打包稱 segment」應是「打包成」；不打包時的問題有兩面：權限不同的區段擠在同一頁就沒辦法各自設權限，各占一頁又浪費——你寫了後者。
>
> **參考答案**【[第四節](../html/c/ELF是什麼-可執行與可連結格式的兩種視角.html#四名字的關鍵兩種視角)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 區段（section）依**內容／功能**切得很細（`hello` 有 31 個）；載入段（segment）數量很少（4 個 `LOAD`），是把區段依**權限**打包、描述「哪一塊放進記憶體、權限是什麼」，給核心載入用。
> 2. 記憶體權限**最小只能以分頁（本機 4096 byte）為單位**設定，同一頁不能一半可執行、一半可寫；所以 `ld` 把權限相同的區段排在一起、打包成一個載入段，核心每塊映射一次、設一種權限就能把程式放進記憶體。
>
> **詳細說明**：核心載入時只在乎權限（可讀／可寫／可執行），不在乎區段叫 `.text` 還是 `.rodata`。如果不打包，權限不同的小區段擠在同一頁就沒辦法各自設權限；每個區段各占一頁又會浪費記憶體。（11-03 改判後移到詳細說明）兩者也是「兩種視角」：區段是**連結視角**，給 `ld`、`nm`、`objdump` 用；載入段是**執行視角**，給核心與動態載入器用。


---


## objdump：[objdump能不能代替ldd找依賴庫-需求單與實際載入](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html)

### objdump 1. `objdump -p /usr/bin/curl` 只列出 3 個 `NEEDED`，`ldd` 卻列出 33 行。多出來的是哪幾類東西？為什麼 `objdump` 看不到？

> 盒子 2｜過去的作答：[s1-01 quiz.md 第 1 題](../small-project/s1-01-hello-compile/quiz.md#1-objdump--p-usrbincurl-只列出-3-個-neededldd-卻列出-33-行多出來的是哪幾類東西為什麼-objdump-看不到)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#objdump-1-objdump--p-usrbincurl-只列出-3-個-neededldd-卻列出-33-行多出來的是哪幾類東西為什麼-objdump-看不到)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-30 複習](./2026-10-30-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語.md#objdump-1-objdump--p-usrbincurl-只列出-3-個-neededldd-卻列出-33-行多出來的是哪幾類東西為什麼-objdump-看不到)、[10-31 複習](./2026-10-31-objdump-組語二.md#objdump-1-objdump--p-usrbincurl-只列出-3-個-neededldd-卻列出-33-行多出來的是哪幾類東西為什麼-objdump-看不到)

```
linux-vdso.so.1 
interpreter
間接的函式庫

因爲objdump 只看 curl的 dynamic section
ldd會用預設的interpreter去看curl的dynamic section,而且interpreter還會去看這些needed的needed
```

> **批改**：**對**。三類都有：`linux-vdso.so.1`、interpreter（動態載入器）、間接依賴；這次只寫「間接」，10-30、10-31 那處「直接與間接」的小修改過來了。第二問也對：`objdump` 只看 `curl` 自己的 dynamic section（需求單裡只有直接依賴），`ldd` 請載入器去看，載入器還會一路看 `NEEDED` 的 `NEEDED`。小修：`ldd` 不只是「看」，是讓載入器**實際照載入的流程走一次**（找檔案、讀每個 `.so` 的需求單），所以列出的是實際會載入的清單和路徑；vdso 是核心執行時直接放進行程的，`curl` 的需求單裡當然沒有它，載入器路徑則寫在 `INTERP`、不在 `NEEDED`。
>
> **參考答案**【[第二節](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html#二為什麼會差這麼多兩者看的是不同時間點)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 多出來的三類：**間接依賴**（最大宗，例如 `libssl.so.3` 是 `libcurl.so.4` 要的，不是 `curl` 自己要的）、`linux-vdso.so.1`（核心提供的虛擬函式庫）、動態載入器本身。
> 2. `objdump -p` 只讀連結時寫進 `curl` 的需求單（`NEEDED`），裡面**只有直接依賴**；`ldd` 是請載入器實際跑一次，一路照每個函式庫的 `NEEDED` 找下去，才拿到完整清單。
>
> **詳細說明**：`NEEDED` 放在 dynamic section，不在檔頭。另外兩類也不會出現在 `curl` 的需求單裡：vdso 是核心在執行時直接放進行程的，沒有對應的檔案；載入器的路徑寫在 `INTERP`，不在 `NEEDED`。


---

### objdump 4. 情境題：同事傳來一支 ARM 開發板用的執行檔，問你「它需要哪些函式庫」。你在自己的 x86-64 電腦上該用哪個工具？用 `ldd` 會發生什麼事，為什麼？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-情境題同事傳來一支-arm-開發板用的執行檔問你它需要哪些函式庫你在自己的-x86-64-電腦上該用哪個工具用-ldd-會發生什麼事為什麼)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#objdump-4-情境題同事傳來一支-arm-開發板用的執行檔問你它需要哪些函式庫你在自己的-x86-64-電腦上該用哪個工具用-ldd-會發生什麼事為什麼)

```
可以用objdump -p看看

ldd用的是預設的interpreter去查,但是x86-64的預設interpreter沒辦法去讀arm的執行檔
會的到 not a executable的奇怪錯誤
```

> **批改**：**對**。抓到：用只讀檔案的工具（`objdump -p`）看需求單；`ldd` 靠本機 x86-64 的預設 interpreter，讀不了 ARM 的執行檔，印出讓人誤會的錯誤。這次寫的「interpreter 沒辦法讀 ARM 的執行檔」比 10-27 的「載入 ARM 的庫會有問題」準——卡在執行檔本身，不是它的函式庫。小修（不扣分）：
> - 錯誤原文是 `not a dynamic executable`（這支明明是動態連結的，所以說它誤導人）；寫成「not a executable」少了關鍵的 dynamic。
> - 工具首選 `readelf -d`：開發板上的檔案常被剝掉區段標頭，`objdump -p` 可能看不到，`readelf -d` 能透過程式標頭的 `DYNAMIC` 找到同一份需求單（跟 10-27 同一處小修）。
> - 「會的到」應是「會得到」。
>
> **參考答案**【[第三節](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html#三ldd-會執行東西這是它的代價)、[第五節](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html#五怎麼選)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 用 **`readelf -d`**（或 `objdump -p`）看 `NEEDED`：只讀檔案、不執行，任何架構的 ELF 都讀得了。
> 2. `ldd` 要靠**本機的 x86-64 載入器**實際載入，處理不了 ARM 的檔案，於是印出誤導人的 **`not a dynamic executable`**。
>
> **詳細說明**：`ldd` 平常列出 vdso、間接依賴的那整套機制，都建立在「本機載入器能載入它」上面；架構不同，第一步就失敗。`objdump -p` 平常也行，但開發板上的檔案常被剝掉區段標頭，`readelf -d` 能透過程式標頭的 `DYNAMIC` 找到同一份資料，比較保險。直接執行則會看到核心的 `exec format error`。


---


## C資料：[C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html)

### C資料 6. `printf("%p")` 印出 `0x7ffe50b23624`，只有 12 位十六進位。指標到底佔幾個 byte？印出來為什麼比較短？如果在 kernel 的錯誤訊息裡看到 `0xffff888102a4c000`，從位址的長相你能判斷什麼？

> 盒子 3｜過去的作答：[s1-02 quiz.md 第 6 題](../small-project/s1-02-data-repr/quiz.md#6-printfp-印出-0x7ffe50b23624只有-12-位十六進位指標到底佔幾個-byte印出來為什麼比較短如果在-kernel-的錯誤訊息裡看到-0xffff888102a4c000從位址的長相你能判斷什麼)、[10-07 複習](./2026-10-07-GNU-C資料-hello-objdump-字元.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#c資料-6-printfp-印出-0x7ffe50b23624只有-12-位十六進位指標到底佔幾個-byte印出來為什麼比較短如果在-kernel-的錯誤訊息裡看到-0xffff888102a4c000從位址的長相你能判斷什麼)

```
8byte

因爲這是使用者空間的位址,所以上面2byte都是0(kernel是1), 0不顯示, 完整應該是0x00007ffe50b23624

因爲上面2byte是ffff,可以判斷這是核心位址空間
```

> **批改**：**對**。三問都答到：(1) 指標 8 byte；(2) 這是使用者空間的位址，上面 2 byte 一定是 0，`%p` 不印前導零，完整是 `0x00007ffe50b23624`（補出完整寫法很好）；(3) 上面 2 byte 是 `ffff`，是核心位址空間。10-23 缺的「為什麼一定是 0」這次有了（因為是使用者空間），括號裡「kernel 是 1」也點出核心那一半上面全是 1。可補（不扣分）：上面 16 bit 會是全 0 或全 1，根本原因是 x86-64 的虛擬位址只用 **48 bit**，上面 16 bit 必須跟第 47 bit 一樣（canonical address），所以合法位址只有使用者、核心這兩段。
>
> **參考答案**【[第二節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#二指標就是位址)〈位址看起來只有 6 byte？——`%p` 不補零，而且最上面 2 byte 一定是 0〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 指標佔 8 byte。
> 2. x86-64 虛擬位址只用 48 bit，使用者空間的位址上面 2 byte 一定是 `00 00`，`%p` 不補前導零，所以只印出 12 位。
> 3. `0xffff888102a4c000` 整個是一個位址；上面 16 bit 全是 1，表示它在位址空間的核心那一半，所以是 kernel 的位址。
>
> **詳細說明**：canonical address 規定上面 16 bit 必須跟第 47 bit 相同，所以合法的位址只有兩段：`0x0000000000000000`～`0x00007fffffffffff`（使用者空間）與 `0xffff800000000000`～`0xffffffffffffffff`（核心），中間那一大段不合法。


---

### C資料 10. `long a = -1; unsigned int b = 1;` 與 `int c = -1; unsigned long d = 1;`，`a < b` 和 `c < d` 各是真還是假？兩邊各被轉成什麼型別，依據的是哪一條規則？為什麼 gcc 只對其中一個發出 `-Wsign-compare` 警告？

**題目用到的程式**（把題目的兩組宣告寫成可以編譯的完整程式 `cmp.c`，`-Wall -Wextra` 編譯）：

```c
#include <stdio.h>

int main(void)
{
    long a = -1;
    unsigned int b = 1;

    int c = -1;
    unsigned long d = 1;

    printf("a < b: %d\n", a < b);
    printf("c < d: %d\n", c < d);
    return 0;
}
```

```sh
gcc -Wall -Wextra cmp.c -o cmp && ./cmp
```

> 盒子 3｜過去的作答：[s1-02 quiz.md 第 10 題](../small-project/s1-02-data-repr/quiz.md#10-long-a---1-unsigned-int-b--1-與-int-c---1-unsigned-long-d--1a--b-和-c--d-各是真還是假兩邊各被轉成什麼型別依據的是哪一條規則為什麼-gcc-只對其中一個發出--wsign-compare-警告)、[10-05 複習](./2026-10-05-gcc-陣列-GNU-C資料-字元.md)、[10-09 複習](./2026-10-09-陣列-gcc-GNU-C資料-interp-ELF.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#c資料-10-long-a---1-unsigned-int-b--1-與-int-c---1-unsigned-long-d--1a--b-和-c--d-各是真還是假兩邊各被轉成什麼型別依據的是哪一條規則為什麼-gcc-只對其中一個發出--wsign-compare-警告)

```
a<b: long 有號範圍大於無號 unsigned int所以轉成long, true
c<d: 有號int無法包含unsigend long轉無號, c>d 所以false

gcc會對c<d發出警告,因爲這個轉換的來源類型是有號,有號轉換可能產出不一樣的結果
```

> **批改**：**大致對**（盒子 3 留在 3）。核心關鍵 1 這次對了：「`long` 的範圍大於（裝得下）`unsigned int`，所以轉成 `long`，為真」——10-20、10-27 都缺的「裝得下」有了。核心關鍵 3 也對：只對 `c < d` 警告，因為是有號的那邊被轉成無號，值會變。**核心關鍵 2 不完整**：
> - **沒寫轉成什麼型別**：題目問「兩邊各被轉成什麼型別」，你只寫「轉無號」。要寫出兩邊都轉成 **`unsigned long`**，`c` 的 −1 變成 18446744073709551615，所以 `c < d` 為假。
> - **依據寫錯條件**：你寫「有號 `int` 無法包含 `unsigned long` 所以轉無號」。這題適用的規則是**無號那邊（`unsigned long`）的等級 ≥ 有號那邊（`int`）→ 轉成那個無號型別**，跟「裝不裝得下」無關——等級夠就直接轉，不必再比範圍。「裝不下」是另一條規則（有號等級較高、卻裝不下無號值時，才轉成**有號那邊對應的無號型別**），套到這題會轉成 `unsigned int` 的思路，剛好型別一樣才沒出錯。10-27 自我練習你寫的「無號等級高，轉無號」才是對的依據。
>
> **參考答案**【[第四節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#四無號整數二進位與回繞)〈怎麼確定一個常數的型別？比較時以誰為準？〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `a < b` 為**真**：`long` 裝得下所有 `unsigned int` 的值 → 兩邊轉成 `long`，值不變，−1 < 1。
> 2. `c < d` 為**假**：無號那邊（`unsigned long`）的等級 ≥ 有號那邊（`int`）→ 兩邊轉成 `unsigned long`，−1 變成 18446744073709551615。
> 3. 只對 `c < d` 警告：只有它**把有號的值轉成無號**，負數的值會變；`a < b` 轉成 `long`，值不變。
>
> **詳細說明**：要看的是**轉換的方向**，不是「`<` 兩邊有一邊是 signed」（`a < b` 的 `a` 也是 signed，卻沒有警告）。有號轉無號不是 UB：位元不變，換成無號的讀法（[第六節](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#六有號與無號之間的轉換位元不變換個讀法)）；UB 是有號溢位。規則的順序：(1) 無號那邊等級 ≥ 有號那邊 → 轉成那個無號型別（`c < d`）；(2) 否則，有號那邊裝得下無號那邊所有值 → 轉成有號型別（`a < b`）；(3) 都不是 → 兩邊轉成有號那邊對應的無號型別（例如 `long` 只有 4 byte 的平台上的 `a < b`，變成 `unsigned long`）。

**自我練習**（不批改）

```
a<b: long 有號範圍大於無號 unsigned int所以轉成long, true
c<d: 有號int無法包含unsigend long, 使用看等級long > int, 轉成unsigend long, c>d 所以false

gcc會對c<d發出警告,因爲這個轉換的來源類型是有號,有號轉換可能產出不一樣的結果
```


---


## 字元：[字元是什麼-char就是1byte的整數與ASCII和UTF-8](../html/c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.html)

### 字元 5. 有人說「『字』是 3 個字元」，另一個人說「是 1 個字元」，誰對？`strlen("字")` 和 `mbstowcs(NULL, "字", 0)` 各會得到多少？如果程式忘了呼叫 `setlocale`，後者會得到什麼，那個數字是怎麼來的？

> 盒子 3｜過去的作答：[s1-02 quiz.md 第 5 題](../small-project/s1-02-data-repr/quiz.md#5-有人說字是-3-個字元另一個人說是-1-個字元誰對strlen字-和-mbstowcsnull-字-0-各會得到多少如果程式忘了呼叫-setlocale後者會得到什麼那個數字是怎麼來的)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#字元-5-有人說字是-3-個字元另一個人說是-1-個字元誰對strlen字-和-mbstowcsnull-字-0-各會得到多少如果程式忘了呼叫-setlocale後者會得到什麼那個數字是怎麼來的)

```
字元如果是char, 3個字元對
字元如果是指unicode的字元,那就是1個字元對

strlen: 3
mbstowcs: 1

setlocale沒呼叫,會使用c locale,因爲沒有"字"這個對應的碼,會返回size_t -1 = 2^64 - 1
```

> **批改**：**對**。三個核心關鍵都有：(1) 兩人都對，看「字元」指 `char`（byte）還是 Unicode 字元；(2) `strlen` 是 3、`mbstowcs` 是 1（10-23 錯成 4 的地方這次對了）；(3) 沒呼叫 `setlocale` 就用 `"C"` locale，回傳 `(size_t)-1` ＝ 2⁶⁴ − 1。小修（不扣分）：「沒有『字』這個對應的碼」可以講準一點——`"C"` locale 只認 ASCII，讀到第一個 byte `0xe5`（大於 `0x7f`）就判定是**無效的多位元組序列**，不是查不到「字」；`mbstowcs` 的 1 要在呼叫 `setlocale(LC_ALL, "")` 之後才成立。「返回」台灣多說「回傳」。
>
> **參考答案**【[第五節](../html/c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.html#五那字是-3-個字元嗎看你說的是哪一種字元)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 兩個人都對：以 `char`（byte）算是 **3** 個，以 Unicode 字元算是 **1** 個。
> 2. `strlen("字")` = 3；呼叫 `setlocale` 之後 `mbstowcs` = 1。
> 3. 忘了 `setlocale` → 用 `"C"` locale，看不懂 UTF-8 → 回傳 `(size_t)-1`，也就是 **18446744073709551615**（2⁶⁴ − 1）。
>
> **詳細說明**：`mbstowcs` 要先知道編碼，必須呼叫 `setlocale(LC_ALL, "")` 讀進環境變數 `LANG`。沒呼叫時，不管 `LANG` 設什麼，程式都用預設的 `"C"` locale，只認 ASCII，看到 `0xe5` 就判定是無效的多位元組序列。`size_t` 在本機是 8 byte 無號，`-1` 轉成無號就是全部位元為 1。


---


## 陣列：[陣列、字串與指標運算-C不檢查邊界與用gdb找segfault](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html)

### 陣列 4. 越界的三種下場

#### 陣列 4-1. 越界存取有三種下場：安靜出錯、`stack smashing detected`、segfault。分別是什麼決定會落到哪一種？

> 盒子 3｜過去的作答：[s1-03 quiz.md 4-1](../small-project/s1-03-arrays-pointers/quiz.md#4-1-越界存取有三種下場安靜出錯stack-smashing-detectedsegfault分別是什麼決定會落到哪一種)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#陣列-4-越界的三種下場)

```
安靜出錯: 在合理的記憶體堆疊,錯誤的寫入或蓋掉不對的區域,例如把返回位址寫到了區域變數的堆疊上,這種gcc, 或是kernel都不會報錯

stack smashing detected: 在函式堆疊塞入陣列,會在陣列的後面的記憶體位址寫入一個值,叫金絲雀位址,如果在函式返回的時候檢查到
金絲雀位址上的值產生變化,代表有人誤寫,C會噴錯

segfault: 如果存取到無權限的記憶體區段,或者存取了一個沒有對映位址的區段,kernel都會報segfault
```

> **批改**：**對**。三種都抓到：位址還在合法（可存取）的記憶體 → 安靜出錯；陣列後面的金絲雀被寫壞、函式返回時檢查到 → `stack smashing detected`；存取沒有對映或權限不允許的區段 → 核心送 segfault。小修（不扣分）：
> - **「金絲雀位址」第三次出現**（10-24、10-27 同一處小修）：金絲雀是放在陣列後面的一個**檢查值**，不是位址；檢查的是「那格的值有沒有被改」。
> - 安靜出錯舉的例子不對：「把返回位址寫到了區域變數的堆疊上」——越界寫入如果改到**返回位址**，通常會先被金絲雀抓到（金絲雀擋在它前面），或 `ret` 跳到亂七八糟的地方而當掉，不是「安靜」。安靜出錯的典型例子是越界讀到隔壁的變數、或寫壞隔壁的區域變數（還沒碰到金絲雀）。
> - 「C 會噴錯」更準確是「glibc 的 `__stack_chk_fail` 印出訊息並 `abort`」。
>
> **參考答案**【[第五節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#五c-不檢查邊界越界的三種下場)開頭的表格、〈下場一：安靜地出錯〉、〈下場二：被堆疊保護抓到〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 位址還在可存取的記憶體 → **安靜出錯**；越界**寫入**改掉金絲雀、函式返回前檢查 → **`stack smashing detected`**；分頁**沒有對映或權限不允許** → **segfault**。
>
> **詳細說明**：金絲雀是 gcc 放在區域陣列後面的檢查值，返回前才檢查，所以發現得比犯錯晚；只讀不寫不會改到它。segfault 是核心送出的 `SIGSEGV`。

#### 陣列 4-2. 為什麼「程式沒當掉」不能證明沒有越界？

```
這是屬於安靜的出錯,根據ABI的規定%rsp底下128byte 是紅區,別人不會寫入,但是自己可以隨意寫,如果自己本身在紅區內
錯誤的覆蓋區域變數之類的東西 kernel與C不會覺得有問題,但最後程式的結果很有可能出錯
```

> **批改**：**對**（改判，11-03）。核心關鍵：越界落在還能存取的記憶體時，C 與 kernel 都不會覺得有問題，程式照跑、結果卻可能錯——你開頭的「這是屬於安靜的出錯」接上 4-1 的定義，紅區內寫壞區域變數是其中一個實例（那塊記憶體合法可寫，沒有任何機制會檢查）。第一次批改把紅區當成你給的「理由」而扣分；使用者說明那只是舉例、重點是在合法的堆疊裡越界，改判為對。
> 小修（不扣分）：先寫通則再舉例，讀的人才不會以為只有紅區才會這樣——**C 不檢查邊界**，越界只要落在任何可存取的記憶體（隔壁的區域變數、全域變數、heap）都一樣安靜；會出聲的只有金絲雀（只抓寫壞那一格、返回時才查）和 segfault（kernel 只管分頁，不知道陣列邊界）。舉例時也挑典型的：`int a[4]` 寫到 `a[5]`、改到隔壁變數，比紅區直觀。
>
> **參考答案**【[第五節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#五c-不檢查邊界越界的三種下場)〈下場一：安靜地出錯〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 「沒當掉」不能證明沒越界：C 不檢查邊界（越界是未定義行為），落在還能存取的記憶體時沒有任何機制會報錯。
>
> **詳細說明**：「沒當掉」的時候可能讀到垃圾、寫壞別人的資料，程式照常結束（`oob.c` 結束狀態 0），錯誤很久以後才在別處出現。紅區（red zone）是另一回事：x86-64 System V ABI 讓葉函式可以直接使用 `%rsp` 底下 128 byte 而不調整 `%rsp`，訊號處理等不會去寫那一塊；它跟越界能不能被偵測無關。


---

### 陣列 7. 情境題：讀懂 ASan 的影子 byte

> ASan 報告的影子 byte 那一列裡，某個堆疊上的 `char` 陣列對應到 `00 00 06`，前後都是 `f2`。
>
> 盒子 3｜過去的作答：2026-10-11 在對話裡作答（筆記[第七節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#七addresssanitizer在第一次越界的當下抓到它)〈上圖的 `01`：影子 byte 的值是「個數」，不是資料〉的追問）、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#陣列-7-情境題讀懂-asan-的影子-byte)

#### 陣列 7-1. 這個陣列佔幾個 byte？

```
22byte
```

> **批改**：**對**。`00 00 06` ＝ 8 ＋ 8 ＋ 6 ＝ 22 byte。
>
> **參考答案**【[第七節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#七addresssanitizer在第一次越界的當下抓到它)〈上圖的 `01`：影子 byte 的值是「個數」，不是資料〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `00 00 06` ＝ 8 ＋ 8 ＋ 6 ＝ **22 byte**。
>
> **詳細說明**：每個影子 byte 管 8 個 byte 的應用程式記憶體：`00` 是 8 個全部可以碰。

#### 陣列 7-2. `06` 的 6 是什麼意思？

> 題目：（影子 byte 是 `00 00 06`，前後都是 `f2`）`06` 的 6 是什麼意思？

```
只有前6個byte可以存取
```

> **批改**：**對**。抓到：最後那一組 8 byte 裡只有前 6 個可以存取。小修（不扣分）：講明是「**那一組 8 byte 裡**」的前 6 個，不是整個陣列的前 6 個。
>
> **參考答案**【[第七節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#七addresssanitizer在第一次越界的當下抓到它)〈上圖的 `01`：影子 byte 的值是「個數」，不是資料〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 最後那一組 8 個 byte 裡，**只有前 6 個**可以碰，後 2 個是禁區。
>
> **詳細說明**：影子 byte 存的是計數，不是記憶體內容。可以碰的部分一定從一組的開頭算起，因為 ASan 把每個變數的起點對齊到 8 的倍數。

#### 陣列 7-3. 如果它是 `char s[] = "…";` 這樣用字串初始化的，字串本身（不含 `'\0'`）有幾個字元？

> 題目：（影子 byte 是 `00 00 06`）如果它是 `char s[] = "…";` 這樣用字串初始化的，字串本身（不含 `'\0'`）有幾個字元？

```
21個字元,要扣掉\0
```

> **批改**：**對**。22 − 1（`'\0'`）＝ 21 個字元。
>
> **參考答案**【[第七節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#七addresssanitizer在第一次越界的當下抓到它)〈上圖的 `01`：影子 byte 的值是「個數」，不是資料〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `char s[] = "…"` 的大小是字串長度 ＋ 1（`'\0'`），所以字串有 **21 個字元**。
>
> **詳細說明**：`char s[] = "…"` 沒寫大小時，編譯器照字串長度＋1 配置，所以陣列大小直接反推得出字串長度。

#### 陣列 7-4. 為什麼影子 byte 裡不會出現 `08`？

```
因爲08 跟 00意義相同,沒有必要這樣寫
```

> **批改**：**對**。8 個 byte 全部可以碰已經用 `00` 表示，`08` 意思一樣、不需要。可補：`01`～`07` 只用來表示「不滿 8 個」。
>
> **參考答案**【[第七節](../html/c/陣列、字串與指標運算-C不檢查邊界與用gdb找segfault.html#七addresssanitizer在第一次越界的當下抓到它)〈上圖的 `01`：影子 byte 的值是「個數」，不是資料〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 8 個 byte 全部可以碰已經寫成 `00`；`01`～`07` 只用來表示「不滿 8 個」，所以不需要 `08`。
>
> **詳細說明**：前後的 `f2` 是變數之間的中間禁區。反過來，宣告 `char s[22]` 就能預測影子 byte 是 `00 00 06`。


---


## 組語：[x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)

### 組語 7. 參數只佔暫存器的一部分

> `int widen_char(signed char c) { return c; }` 編出 `movsbl %dil, %eax`。
>
> 盒子 3｜過去的作答：[s1-04 quiz.md 第 7 題](../small-project/s1-04-asm-basics/quiz.md#7-參數只佔暫存器的一部分2026-10-06-課後提問新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#組語-7-參數只佔暫存器的一部分)

用到的程式（檔案：[`dil.c`](../small-project/s1-04-asm-basics/dil.c)）：

```c
#include <stdio.h>

__attribute__((noinline)) int widen_char(signed char c) { return c; }   /* noinline：不准內嵌，才看得到真的呼叫 */

int main(void)
{
    printf("%d\n", widen_char(-1));
    return 0;
}
```

```sh
gcc -O2 -g dil.c -o dil
objdump -d --no-show-raw-insn dil
```

`widen_char` 編出來的組合語言（`objdump` 節錄）：

```asm
widen_char:
	endbr64                # 給 CPU 的控制流程保護標記，跟計算無關（佔 4 byte，所以下一條是 widen_char+4）
	movsbl	%dil, %eax     # 讀 %dil（1 byte），有號擴展成 4 byte 放進 %eax
	ret                    # 回傳 %eax
```

筆記的 gdb 實驗：停在 `movsbl` 之前，把 `%rdi` 整個改掉，再執行一條指令看結果。

```sh
gdb -batch -ex 'break *widen_char+4' -ex run -ex 'p/x $rdi' \
    -ex 'set $rdi = 0x11223344556677ff' -ex 'p/x $rdi' -ex 'p/x $dil' \
    -ex stepi -ex 'p $eax' -ex 'p/x $rax' -ex continue ./dil
```

（`break *widen_char+4`：停在 `widen_char` 開頭往後 4 byte 那條指令；`set $rdi = …`：改暫存器；`stepi`：只執行一條機器指令。）

#### 組語 7-1. 在 64 位元的機器上，為什麼它只讀 `%dil`，而不讀整個 `%rdi`？

```
因爲c明寫他是 signed char類型,只有1byte,如果讀rdi(8byte),那有可能會把上面7byte的垃圾是數字一起讀進去
dil只有看1byte,是準的
```

> **批改**：**對**。抓到：`signed char` 只有 1 byte、放在 `%dil`；讀整個 `%rdi` 會把上面 7 byte 的垃圾一起讀進來。可補：上面 7 byte 是呼叫慣例**沒有約定**的部分，呼叫的人不保證它是什麼。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈讀懂實驗一的範例：為什麼寫 `1(%rdi)`、為什麼用 `%dil`〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 參數是 1 byte 的 `signed char`，只放在 `%rdi` 最低的 1 byte（`%dil`）。
> 2. 高 7 byte 呼叫的人沒有約定，**內容不保證**；讀整個 `%rdi` 會把垃圾一起讀進來。
>
> **詳細說明**：實例：`main` 寫 `%edi` 傳 −1，`%rdi` 是 `0x00000000ffffffff`，整個讀會得到 4294967295。

#### 組語 7-2. 之後 `%eax` 的十六進位是多少？

> 題目：假設 `widen_char` 改成讀 4 byte 的 `movl %edi, %eax`，在上面的 `gdb` 實驗裡把 `%rdi` 改成 `0x11223344556677ff`。之後 `%eax` 的十六進位是多少？

```
0x556677ff
```

> **批改**：**對**。`%rdi` 的低 4 byte。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈組合語言裡只有 `%edi`、`%dil`，為什麼 gdb 印得出 `$rdi`？〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `0x556677ff`（`%rdi` 的低 4 byte）。

#### 組語 7-3. 是正數還是負數？

> 題目：（承 7-2）`movl %edi, %eax` 之後的 `%eax` 是正數還是負數？

```
是正數,0x55,說明最高符號位爲0
```

> **批改**：**對**。最高的 hex 位 `5` ＝ `0101`，符號位是 0，所以是正數。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈`%dil` 是 `0xff`，為什麼 `%eax` 變成 `0xffffffff`〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 正數：最高位元（`0x5` ＝ `0101` 的第一個 bit）是 0。
>
> **詳細說明**：十進位是 1432778751。

#### 組語 7-4. 跟 C 期望的 −1 差在哪？

> 題目：（承 7-2）改成 `movl %edi, %eax` 之後得到的值，跟 C 期望的 −1 差在哪？

```
C期望的只有0xff這一段,但是因爲我們把%dil改成了%edi,那我們就有可能會讀到上面3byte的垃圾數據(這裏是我們自己手動寫垃圾進去)
所以最後的返回的值就有問題
```

> **批改**：**對**。抓到：C 期望的只有最低那 1 byte `0xff`；讀 4 byte 會把上面 3 byte 的垃圾（這裡是手動寫進去的 `0x556677`）一起讀進來，回傳值就錯了。小修（不扣分）：「差在哪」可以直接寫出差別——期望的是 `0xff` **有號擴展**成的 −1，實際得到的是**正的** `0x556677ff`（1432778751），連正負號都錯。另外「把 `%dil` 改成了 `%edi`」講的是指令改成讀 4 byte 的 `movl %edi, %eax`，暫存器本身沒變。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈讀懂實驗一的範例：為什麼寫 `1(%rdi)`、為什麼用 `%dil`〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. C 的 `c` 只在最低 1 byte（`0xff`，符號擴展後是 −1）；讀 4 byte 會把高 3 byte 的垃圾 `0x556677` 一起讀進來，得到正的 1432778751，不是 −1。
>
> **詳細說明**：所以參數有多大，指令就讀多大的名字：`signed char` 讀 `%dil`、`short` 讀 `%di`、`int` 讀 `%edi`、`long` 和指標讀 `%rdi`。


---

### 組語 11. gdb 的 `break *位址`

> `break deref+4` 會得到 `Function "deref+4" not defined.`，`break *deref+4` 卻可以。
>
> 盒子 3｜過去的作答：[s1-04 quiz.md 第 11 題](../small-project/s1-04-asm-basics/quiz.md#11-gdb-的-break-位址2026-10-06-課後提問新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#組語-11-gdb-的-break-位址)

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

`-O2` 編出的 `deref`：

```asm
deref:
	endbr64                 # 控制流程保護標記（佔 4 byte，所以下一條是 deref+4）
	mov	(%rdi),%rax         # 把 %rdi 當位址，讀 8 byte 放進 %rax
	ret                     # 回傳 %rax
```

#### 組語 11-1. `*` 讓 gdb 怎麼理解後面的東西？

```
* 要求gdb運算後面算式的位址, *deref+4 => 可以理解爲deref這個符號的位址 + 4 byte
```

> **批改**：**對**。抓到：`*` 讓 gdb 把後面當成**算出位址的式子**，`*deref+4` ＝ `deref` 的位址 ＋ 4。可補（不扣分）：對照沒有 `*` 的情況——後面被當成函式名或行號（linespec），`deref+4` 整串被當成一個函式名，所以 `Function "deref+4" not defined.`。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上面 `break *widen_char+4` 的 `*`：為什麼要加？〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `*` 表示後面是**一個算出位址的式子**：gdb 計算 `deref + 4`（函式開頭往後 4 byte），停在那個位址的機器指令。
> 2. 沒有 `*` 時，後面被當成函式名或行號（linespec），`deref+4` 整串被當成函式名，所以 `not defined`。

#### 組語 11-2. 這個 `*` 跟 C 的 `*p` 是同一個意思嗎？

```
不太一樣,C的*p 是取p這個指標指的那個位址那格存的數據
```

> **批改**：**對**。抓到：不一樣；C 的 `*p` 是取出 `p` 指向那格裡存的值，gdb `break` 的 `*` 是說「後面是位址」（你 11-1 寫的），不去讀那格。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上面 `break *widen_char+4` 的 `*`：為什麼要加？〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 不是：`break` 的 `*` 是 gdb 的語法記號，意思是「後面是位址」；C 的 `*p` 是取出指標指向的值。

#### 組語 11-3. 在 `-O0` 的程式上，`break deref` 和 `break *deref` 為什麼會停在不同的位址？

同一個 `paren.c` 改用 `-O0` 編，`deref` 的組合語言：

```sh
gcc -O0 -g paren.c -o paren0
objdump -d --no-show-raw-insn paren0
gdb -batch -ex 'break deref' -ex 'break *deref' ./paren0     # 看兩個中斷點各下在哪個位址
```

```asm
000000000000117b <deref>:
    117b:	endbr64                 # 控制流程保護標記
    117f:	push   %rbp             # 把呼叫者的 %rbp 存到堆疊（第 5 課的序言）
    1180:	mov    %rsp,%rbp        # %rbp = 目前堆疊頂，建立這個函式的堆疊框
    1183:	mov    %rdi,-0x8(%rbp)  # 把參數 p（在 %rdi）存進堆疊上的區域變數
    1187:	mov    -0x8(%rbp),%rax  # 從堆疊讀回 p 放進 %rax
    118b:	mov    (%rax),%rax      # *p：把 p 當位址讀 8 byte
    118e:	pop    %rbp             # 還原呼叫者的 %rbp
    118f:	ret                     # 回傳 %rax
```

> 題目：在 `-O0` 的程式上，`break deref` 和 `break *deref` 為什麼會停在不同的位址？

```
break *deref: 找出deref這個符號的位址,並計算,因爲沒有+任何數值,所以就是直接找這個符號的位址,也就是117b endbr64

break deref: 他會直接跳過這個函式前面處理的一些代碼,像是設定這個函式的堆疊頂端,直接跑到這個函式的堆疊頂端
```

> **批改**：**對**。抓到：`break *deref` 照符號的位址，停在第一條 `endbr64`（`0x117b`，實測）；`break deref` 跳過函式開頭設定堆疊框的那些準備工作。小修（不扣分）：
> - 「直接跑到這個函式的**堆疊頂端**」說錯了東西：中斷點是**程式碼**的位址，不是堆疊上的位置。應說「停在準備工作**之後的那條指令**」——實測 `0x1187`（有 `-g`，連 `mov %rdi,-0x8(%rbp)` 把參數存進堆疊也算準備工作；跟 10-24、10-27 同一處小修）。
> - 「跳過」不是不執行：`push %rbp` 那幾條照樣執行，只是停下來時已經跑完了。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#三指令的文法助憶符大小後綴運算元)〈上面 `break *widen_char+4` 的 `*`：為什麼要加？〉的「`break 函式名` 和 `break *函式名` 不一定停在同一個地方」】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `break deref`（函式名）會把中斷點**下在函式開頭的準備工作之後**（`-O0` 的 `push %rbp`、`mov %rsp,%rbp` 等；這些指令照常執行，停下時已經跑完）。有 `-g` 照行號表，連「把參數存進堆疊」也算進去，停在 `0x1187`；沒有 `-g` 停在 `0x1183`。
> 2. `break *deref` 不跳過任何東西，停在函式的**第一條指令** `endbr64`（實測 `0x117b`）。
>
> **詳細說明**：本機實測：`gdb -batch -ex 'break deref' -ex 'break *deref' ./paren0` 印出 `Breakpoint 1 at 0x1187: file paren.c, line 4.`、`Breakpoint 2 at 0x117b: file paren.c, line 4.`。


---

### 組語 15. 減法的借位與 CF

> 8 位元算 4 − 9。
>
> 盒子 3｜過去的作答：[s1-04 quiz.md 第 15 題](../small-project/s1-04-asm-basics/quiz.md#15-減法的借位與-cf2026-10-07-課後提問新增)、[10-13 複習](./2026-10-13-組語-字元.md)、[10-22 複習](./2026-10-22-組語-C資料-objdump-字元-陣列.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#組語-15-減法的借位與-cf)

#### 組語 15-1. 照直式減法，最高位（第 7 位元）需不需要往外借？CF 是多少？

```
需要,
CF = 1 只要有借位或往外進位就是1
```

> **批改**：**對**。需要往外借，CF ＝ 1。可補：原因是 4 < 9（無號），最高位不夠減、要往第 8 位元之外借。小修：「CF 只要有借位或往外進位就是 1」要分開講——**加法**時 CF 記往外進位、**減法**時 CF 記往外借位，不是兩個條件「或」起來（15-3 會看到這樣想會出錯）。
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面③：3 − 5 在 CPU 裡實際上是怎麼算的？〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 需要：4 < 9（無號），最高位要往第 8 位元之外借 → CF ＝ 1。

#### 組語 15-2. 留下的位元當無號讀是多少，它跟數學上的正確答案差了多少、為什麼剛好差這麼多？

> 題目：（8 位元算 4 − 9）留下的位元當無號讀是多少，它跟數學上的正確答案差了多少、為什麼剛好差這麼多？

```
0x04 - 0x09 = 0xfb = 251

251 - (-5) = 256

因爲有往外借位,借了256(0x100)
```

> **批改**：**對**。抓到：`0xfb` ＝ 251；跟 −5 差 256；因為往外借的那個 1 值 2⁸ ＝ 256（`0x100`）。
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面③：3 − 5 在 CPU 裡實際上是怎麼算的？〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 留下 `0xfb`，無號讀是 **251**。
> 2. 數學答案是 −5，**差 256**：往第 8 位元之外借的 1 價值 2⁸ ＝ 256，結果多了 256（251 ＝ 4 − 9 ＋ 256）。
>
> **詳細說明**：`0xfb` 當有號讀正好是 −5：同一串位元，有號讀法剛好把多出來的 256 扣回去。

#### 組語 15-3. 如果 CPU 是用加法器算（D + S 反轉 + 1），加法器有沒有往外進位，跟 CF 是什麼關係？

> 題目：（8 位元算 4 − 9）如果 CPU 是用加法器算（D + S 反轉 + 1），加法器有沒有往外進位，跟 CF 是什麼關係？

```
0x09 反轉: 00001001 -> 11110110 ->0xf6

0x04 + 0xf6 + 1 = 0xfb 與第15-2結果相同

沒有往外進位, CF = 1 -> 有借位||有往外進位 這裏的情況是有借位沒有往外進位 ->CF = 1
```

> **批改**：**大致對**。核心關鍵 1 對：9 反轉是 `0xf6`，`0x04 + 0xf6 + 1 = 0xfb`，跟 15-2 一樣，加法器**沒有**往外進位。**核心關鍵 2 錯**：你寫的關係是「CF ＝ 有借位 ‖ 有往外進位」，這不對。減法用加法器算時，**加法器的進位跟借位剛好相反**：
> - 這題：加法器沒進位 ⇔ 需要借位 → CF ＝ 1；
> - 反例 5 − 3：`0x05 + 0xfc + 1 = 0x102`，加法器**有**往外進位，但 5 ≥ 3 不用借 → CF ＝ **0**。照你的「或」，這裡會算出 CF ＝ 1，就錯了。
> 
> 所以 x86 做減法時，是把加法器的進位**反過來**存進 CF（CF ＝ NOT 進位 ＝ 借位）。這是這題要問的「關係」。
>
> **參考答案**【[第五節](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#五旗標與條件跳躍cmp-只負責比跳不跳由下一個指令決定)〈上面③：3 − 5 在 CPU 裡實際上是怎麼算的？〉的「電路上其實是用加法做的」】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 4 ＋ (9 反轉 ＝ 246) ＋ 1 ＝ 251，沒超過 255 → 加法器**沒有**往外進位。
> 2. 減法時進位跟借位**相反**：x86 把加法器的進位反過來存進 CF，所以「沒進位」→ CF ＝ 1（借位了）。
>
> **詳細說明**：對照 5 − 3：5 ＋ 252 ＋ 1 ＝ 258，有進位 → CF ＝ 0（不用借）。ARM 等架構減法時直接存加法器的進位，CF 的意思剛好相反。

**自我練習**（不批改）

```
0x09 反轉: 00001001 -> 11110110 ->0xf6

0x04 + 0xf6 + 1 = 0xfb 與第15-2結果相同

加法器做減法時 沒有往外進位, CF = 1 -> 有借位 ->沒有往外進位
```


---


## 組語二：[x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html)

### 組語二 1. `call` 與 `ret`

> 盒子 3｜過去的作答：[s1-05 quiz.md 第 1 題](../small-project/s1-05-asm-calls/quiz.md#1-call-與-ret)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#組語二-1-call-與-ret)

`frame.c` 的 `main` 裡有 `call 1149 <add3>`（位址 `0x1194`），下一條指令在 `0x1199`；`add3` 最後一條是 `ret`。

完整原始碼（檔案：[`frame.c`](../small-project/s1-05-asm-calls/frame.c)）與編譯指令：

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

```asm
# main
1194:	call   1149 <add3>
1199:	mov    %rax,-0x8(%rbp)
# add3 的最後
1177:	pop    %rbp
1178:	ret
```

#### 組語二 1-1. `call` 做了哪兩件事？

```
1. %rsp下移8byte,這個空間用來儲存返回位址
2. jmp到 add3
```

> **批改**：**對**。抓到：`%rsp` 減 8、在那格存返回位址；跳到 `add3`。小修（不扣分）：講出存進去的是**哪個值**——`call` 下一條指令的位址 `0x1199`，1-3 就是靠這一點。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#三call-與-ret用-gdb-一條一條走)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 把**返回位址**（`call` 下一條指令的位址 `0x1199`）推進堆疊：`%rsp` 減 8，寫進新的頂端。
> 2. 跳到 `add3`（`%rip` 變成 `add3` 的位址）。
>
> **詳細說明**：`frame.c` 實測：`call` 之後 `%rsp` 從 `d410` 變成 `d408`，`d408` 裡是 `0x555555555199`（PIE 載入後的 `0x1199`）。

#### 組語二 1-2. `ret` 做了什麼？

```
拿%rsp當前指的那一格,當作返回位址,%rsp上移8byte

跳到返回位址
```

> **批改**：**對**。抓到：把 `%rsp` 指的那格當返回位址、跳過去，`%rsp` 加 8。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#三call-與-ret用-gdb-一條一條走)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 把堆疊頂端那 8 byte 當成位址，放進 `%rip`（跳過去）。
> 2. `%rsp` 加 8（返回位址被拿走）。
>
> **詳細說明**：`ret` ＝ `pop` 到 `%rip`。實測 `ret` 之後 `%rsp` 回到 `d410`，跟 `call` 之前一模一樣。

#### 組語二 1-3. `ret` 怎麼知道要回到 `0x1199`？

```
ret不知道,但他從%rsp指的那格拿到0x1199,接下來如同1-2
```

> **批改**：**對**。抓到：`ret` 自己不知道，是從 `%rsp` 指的那格拿到 `0x1199`。可補：那格的 `0x1199` 是 `call` 推進去的（1-1），所以 `add3` 在 `ret` 前必須讓 `%rsp` 回到那一格（`pop %rbp` 拿走舊 `%rbp`）。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#三call-與-ret用-gdb-一條一條走)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `call` 把 `0x1199` 推進堆疊，存在 `%rsp` 指的那格記憶體裡。
> 2. `ret` 不知道呼叫者是誰，只是取堆疊頂端那 8 byte 當位址。
>
> **詳細說明**：所以 `add3` 在 `ret` 之前必須把 `%rsp` 還原到放返回位址的那一格：`pop %rbp` 把序言推的舊 `%rbp` 拿走，頂端就又是 `0x1199`。函式裡 `push` 和 `pop` 沒配好，`ret` 就會跳錯。

#### 組語二 1-4. 如果 `add3` 裡有個陣列越界寫入，剛好蓋掉了堆疊上的返回位址，`ret` 會發生什麼事？

```
我只能確定他不會jmp到返回位址,有可能越界,也有可能跳到合法的記憶體區段,安靜的出錯
```

> **批改**：**對**。抓到：不會回到 `0x1199`，而是跳到別的地方——被蓋上去的值指向哪就去哪。小修（不扣分）：
> - 直接講出機制：**`ret` 不檢查**，照樣把那格（已被蓋掉的值）當位址跳過去。
> - 結果最常見的是 **segfault**：那個值多半不是有對映、可執行的位址。「安靜的出錯」只有在剛好跳進合法的程式碼時才可能，而且通常也會很快亂掉；「有可能越界」意思不清楚，應該說「跳到沒有對映或不能執行的位址 → segfault」。
> - 有堆疊保護時，越界寫入要蓋到返回位址得先經過金絲雀，函式返回**之前**就會印 `stack smashing detected` 中止；如果蓋上去的值是攻擊者刻意挑的位址，`ret` 就會跳去執行攻擊者要的程式碼（緩衝區溢位攻擊）。
>
> **參考答案**【[第三節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#三call-與-ret用-gdb-一條一條走)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `ret` 不檢查，照樣把被蓋掉的值當成位址跳過去。
>
> **詳細說明**：跳到亂的地方，通常是 segfault 當掉（那個位址沒有對映，或不能執行）。有堆疊保護時，函式返回**之前**檢查金絲雀，印 `stack smashing detected` 中止；關掉保護就真的走到 `ret`，返回時 segfault。如果蓋上去的值是攻擊者刻意挑的位址，`ret` 就會跳去執行攻擊者要的程式碼——這是緩衝區溢位攻擊的原理。


---

### 組語二 4. callee-saved 與對齊

> 盒子 3｜過去的作答：[s1-05 quiz.md 第 4 題](../small-project/s1-05-asm-calls/quiz.md#4-callee-saved-與對齊)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#組語二-4-callee-saved-與對齊)

下面兩個函式用 `-O2` 編譯（`g` 只有宣告）。

```c
/* saved.c */                          /* noret.c */
long g(long x);                         long g(long x);

long f(long x)                          long f(long x)
{                                       {
    long y = g(x);                          long y = g(x);
    return x + y;                           return y * 2;
}                                       }
```

完整原始碼（檔案：[`saved.c`](../small-project/s1-05-asm-calls/saved.c)、[`noret.c`](../small-project/s1-05-asm-calls/noret.c)；`g` 只有宣告，只編成 `.o`）與編譯指令：

```sh
gcc -O2 -c -fno-asynchronous-unwind-tables saved.c -o saved.o
gcc -O2 -c -fno-asynchronous-unwind-tables noret.c -o noret.o
objdump -d --no-show-raw-insn saved.o noret.o
```

```asm
# saved.c 的 f                          # noret.c 的 f
push   %rbx                             sub    $0x8,%rsp
mov    %rdi,%rbx                        call   g
call   g                                add    $0x8,%rsp
add    %rbx,%rax                        add    %rax,%rax
pop    %rbx                             ret
ret
```

#### 組語二 4-1. 為什麼 `saved.c` 的 `f` 要 `push %rbx`，而不是讓 `x` 留在 `%rdi`？

```
因爲call完g以後x還要用, %rdi是caller saved,所以g有可能改他,故放到rbx 這種callee saved,可以不用擔心g改動
但因爲f自己也是別人的callee,rbx的原值他也要幫別人保存,所以先push %rbx,保存原值,等處理完所有事情在pop復原
```

> **批改**：**對**。三點都有：`x` 在 `call g` 之後還要用、`%rdi` 是 caller-saved 可能被 `g` 改；所以搬到 callee-saved 的 `%rbx`；`f` 自己也是別人的 callee，借用 `%rbx` 前先 `push` 原值、最後 `pop` 還原。小修：「復原」台灣多寫「還原」。
>
> **參考答案**【[第六節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#六誰負責保存暫存器以及-16-byte-對齊)〈實驗：要跨過 `call` 保存的值放哪裡〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `x` 在 `g` 回來後還要用，但 `%rdi` 是 caller-saved，`g` 可能改掉它。
> 2. 所以把 `x` 搬到 callee-saved 的 `%rbx`：ABI 保證 `g` 回來時 `%rbx` 不變。
> 3. `f` 借用 `%rbx` 也要對自己的呼叫者負責：開頭 `push %rbx`、返回前 `pop %rbx` 還原。
>
> **詳細說明**：對照 `noret.c`：`call` 之後不再用 `x`，就不需要 `%rbx`。`saved.c` 的 `push %rbx` 還順便把 `%rsp` 挪了 8 byte，同時滿足 16 byte 對齊（4-2）。

#### 組語二 4-2. `noret.c` 的 `f` 不需要保存任何值，為什麼還要 `sub $0x8,%rsp`？

```
因爲noret.c不知道g做了什麼,這裏只有宣告,根據ABI的規則,要確保call g的時候,%rsp的位址是16的倍數,
因爲call f的時候%rsp已經-8byte存返回位址了,所以只需要再-8byte就可以湊成16的倍數
```

> **批改**：**對**。三點都有：ABI 要求 `call` 的那一刻 `%rsp` 是 16 的倍數；`call f` 已經推了 8 byte 返回位址；所以再減 8 湊回 16 的倍數。小修（不扣分）：「不知道 `g` 做了什麼」不是必要條件——就算知道，ABI 照樣要求對齊；可補為什麼要 16：有些指令（例如 SSE 的 `movaps`）要求 16 byte 對齊的位址。
>
> **參考答案**【[第六節](../html/c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.html#六誰負責保存暫存器以及-16-byte-對齊)〈上面的 `sub $0x8,%rsp`：16 byte 對齊〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. ABI 規定：執行 `call` 的那一刻，`%rsp` 必須是 16 的倍數。
> 2. 呼叫 `f` 的人照規定對齊，但 `call f` 推了 8 byte 返回位址，進到 `f` 時 `%rsp` 是「16 的倍數 − 8」。
> 3. `f` 要 `call g`，必須再挪 8 byte：沒東西要存，就用 `sub $0x8,%rsp` 單純挪。
>
> **詳細說明**：要 16 的原因：有些指令（例如 SSE 的 `movaps`）要求記憶體位址是 16 的倍數，否則會當掉。


---


## 多檔：[多檔案的C程式-標頭檔、static與extern、連結錯誤與make](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html)

### 多檔 3. `static` 的兩種意思

> 盒子 3｜過去的作答：[s1-06 quiz.md 第 3 題](../small-project/s1-06-multi-file/quiz.md#3-static-的兩種意思)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#多檔-3-static-的兩種意思)

`static` 寫在函式外面和寫在函式裡面，各改變了什麼？用下面兩個例子說明：(a) `a.c`、`b.c` 各有一個 `static int helper(void)`，為什麼能一起連結，拿掉 `static` 為什麼就 `multiple definition`？(b) `next_id()` 呼叫三次回傳 1、2、3，`plain_id()` 每次都是 1，兩個 `n` 各放在記憶體的哪裡？

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

#### 多檔 3-1. `static` 寫在函式外面和寫在函式裡面，各改變了什麼？

```
函式外面, 這個符號變成了LOCAL,只有擁有這個函式的檔案可以使用,也不會跟別的檔案做符號連結
函式裏面, 一樣變成了LOCAL, 無法跟別人連結,也不是一般的區域變數存在函式堆疊裏,而是根據初值是否爲0,爲0存在.bss, 非0存在.data
```

> **批改**：**對**。抓到：函式外面 → 符號變 `LOCAL`、只有這個檔案用得到、不跟別的檔案配對（**可見範圍**）；函式裡面 → 不放堆疊，依初值放 `.bss`／`.data`（**儲存期間**）。小修（不扣分）：(1) 函式裡的 `static`，重點是**儲存期間**變了——整個程式執行期間只有一份、值留到下一次呼叫；「一樣變成 `LOCAL`」雖然沒錯（`nm` 看得到 `n.0` 之類的 `LOCAL` 符號），但它本來就是區域變數、別的檔案本來就碰不到，這不是 `static` 帶來的改變，名字也仍然只在那個函式裡看得到。(2) 可以用一句話對照：外面改「誰看得到」，裡面改「活多久、放哪裡」。
>
> **參考答案**【[第五節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#五static兩種完全不同的意思)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 函式外面：改的是名字的**可見範圍**——只在這個 `.c` 有效，符號變成 `LOCAL`，連結器不拿它跟別的檔案配對。
> 2. 函式裡面：改的是**儲存期間**——不放堆疊，放在固定記憶體（`.bss`／`.data`），整個程式執行期間只有一份，值留到下一次呼叫；名字仍然只在那個函式裡看得到。

#### 多檔 3-2. (a) `a.c`、`b.c` 各有一個 `static int helper(void)`，為什麼能一起連結，拿掉 `static` 為什麼就 `multiple definition`？

```
因爲a,b的helper都是local,不連接,也就不會產生這種因爲有多個定義global,而不知道該怎麼連接的錯誤

拿掉static 符號變成global, ld不知道helper到底該連接a版本還是b版本
```

> **批改**：**對**。抓到：兩個 `helper` 都是 `LOCAL`，不參與配對，所以同名不衝突；拿掉 `static` 變 `GLOBAL`，兩個 `.o` 各有一個同名的 `GLOBAL` 定義 → `multiple definition`。小修（不扣分）：「`ld` 不知道該連 a 版本還是 b 版本」是好的直覺，但規則更硬——就算兩份內容一模一樣，有兩個同名 `GLOBAL` 定義就是錯。
>
> **參考答案**【[第五節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#檔案層級的-static讓名字變成-local)〈檔案層級的 `static`〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 有 `static`：兩個 `helper` 都是 `LOCAL`，連結器做符號解析時只用 `GLOBAL` 配對，所以同名也不衝突。
> 2. 拿掉 `static`：兩個 `.o` 各有一個 `GLOBAL` 的 `helper` 定義 → 同名的 `GLOBAL` 定義有兩個 → `multiple definition`。
>
> **詳細說明**：兩份內容不一樣（回傳 1 和 2）不是重點；就算一模一樣，有兩個同名的 `GLOBAL` 定義也是錯。

#### 多檔 3-3. (b) `next_id()` 呼叫三次回傳 1、2、3，`plain_id()` 每次都是 1，兩個 `n` 各放在記憶體的哪裡？

```
因爲next_id 的n是static,固定存放在.bss裏,每次n+1都會存到.bss
但plain_id是n是存到函式堆疊裏,函式結束,堆疊就被清除了,所以每次都是從0開始
```

> **批改**：**對**。抓到：`next_id` 的 `n` 固定放在 `.bss`，值留著所以 1、2、3；`plain_id` 的 `n` 在堆疊框裡，每次呼叫都從 0 開始。小修（不扣分）：堆疊不是「被清除」——函式返回時那塊只是不再屬於它（`%rsp` 移回去），內容還在、之後被別的呼叫蓋掉；每次都是 1 的原因是**每次呼叫都重新執行 `n = 0`**。
>
> **參考答案**【[第五節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#函式裡的-static不放堆疊值留到下一次)〈函式裡的 `static`〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `next_id` 的 `n`：固定記憶體 `.bss`（初值 0），只初始化一次、值留著，所以 1、2、3。
> 2. `plain_id` 的 `n`：堆疊框裡（`-0x4(%rbp)`），每次呼叫都重新設成 0，所以每次都是 1。


---

### 多檔 4. 標頭檔裡的 `long hits;`

> 盒子 3｜過去的作答：[s1-06 quiz.md 第 4 題](../small-project/s1-06-multi-file/quiz.md#4-標頭檔裡的-long-hits)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md#多檔-4-標頭檔裡的-long-hits)

下面的 `hits.h` 被 `hits.c` 和 `hits_main.c` 都 `#include`，兩個 `.c` 都沒有再寫 `hits` 的定義。本機 gcc 13 連結時出現 `multiple definition of 'hits'`。為什麼？已經有 include guard 了，為什麼沒擋住？該怎麼改（標頭檔寫什麼、哪裡寫什麼）？為什麼同樣的程式碼在舊版 gcc 上可以連結？

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

#### 多檔 4-1. 為什麼？

```
因爲在compile hists.c 與 hits_main.c的時候 兩邊的前處理都直接把hits.h寫入了,所以兩邊各自都定義了hits

因此ld去連結hits.o hits_main.o就會出現重複定義的錯誤
```

> **批改**：**對**。抓到：前處理把 `hits.h` 貼進兩個 `.c`，兩邊各有一份 `hits` 的**定義**，連結時 `ld` 看到兩個同名定義 → `multiple definition`。可補（不扣分）：`long hits;`（沒 `extern`、沒初值）叫**暫定定義**，在 gcc 10 以後實際上就是定義；兩個 `.o` 各有一個 `GLOBAL` 的 `hits`。小修：「hists.c」是筆誤。
>
> **參考答案**【[第六節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#忘了-extern每個-c-都有一份定義)〈忘了 `extern`：每個 `.c` 都有一份定義〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `long hits;`（沒有 `extern`、沒有初值）是暫定定義，實際上就是**定義**。
> 2. 標頭檔被貼進兩個 `.c`，兩個 `.o` 各有一個 `GLOBAL` 的 `hits` → 連結器看到兩個同名定義 → `multiple definition`。

#### 多檔 4-2. 已經有 include guard 了，為什麼沒擋住？

```
include guard 只能擋住同一個檔案重複include hits.h
hits.c 與 hits_main.c都是分開編譯的,include沒辦法擋住
```

> **批改**：**對**。抓到：include guard 只擋同一個檔案（編譯單元）裡重複 include；`hits.c`、`hits_main.c` 分開編譯，各貼一次是正常的，guard 擋不到。可補：錯誤發生在**連結時**兩個 `.o` 之間，guard（前處理的巨集）本來就管不到那個階段。
>
> **參考答案**【[第六節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#忘了-extern每個-c-都有一份定義)〈忘了 `extern`：每個 `.c` 都有一份定義〉最後一段】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. include guard 只防止**同一個編譯單元**裡被貼兩次（`HITS_H` 這個巨集只在編譯這一個 `.c` 的期間有效）。
> 2. `hits.c` 和 `hits_main.c` 是分開編譯的兩個編譯單元，各貼一次是正常的；錯誤發生在連結時兩個 `.o` 之間，guard 管不到。

#### 多檔 4-3. 該怎麼改（標頭檔寫什麼、哪裡寫什麼）？

```
hits.h 改成 extern long hits;

hits.c 定義: long hits =0;
```

> **批改**：**對**。抓到：標頭檔寫 `extern long hits;`（宣告），定義只放在 `hits.c` 的 `long hits = 0;`。可補：`hits.c` 也要 `#include "hits.h"`（題目的程式已經有），讓編譯器檢查宣告與定義的型別一致。
>
> **參考答案**【[第六節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#六extern全域變數的宣告與定義)〈整理：全域變數放哪裡〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 標頭檔寫 `extern long hits;`：宣告，不配置空間，幾個 `.c` 引入都沒關係。
> 2. **恰好一個 `.c`**（`hits.c`）寫定義 `long hits = 0;`，而且這個 `.c` 也引入 `hits.h`，讓編譯器檢查宣告與定義的型別一致。

#### 多檔 4-4. 為什麼同樣的程式碼在舊版 gcc 上可以連結？

```
舊版的程式嗎gcc 默認打開-fcommon, 這樣在編譯的時候會把各自得hits符號標記爲COM, 連結的時候就會把全域變數的符號標記爲COM

ld在做連結的時候就會把他們統一
```

> **批改**：**對**。抓到：舊版 gcc 預設 `-fcommon`，沒初值的 `hits` 被標成 `COM`（common 符號），連結時 `ld` 把同名的 common 符號合併成一份。小修（不扣分）：(1) 標成 `COM` 是**編譯時**做的（`.o` 的符號表裡 `Ndx` 就是 `COM`），連結時是**合併**，你第二句「連結的時候就會把全域變數的符號標記為 COM」把兩個階段混在一起了；(2) 可以點出版本：gcc 10 起預設改成 `-fno-common`；(3) 正確修法仍是 4-3 的 `extern`，不是加 `-fcommon`。
>
> **參考答案**【[第六節](../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#忘了-extern每個-c-都有一份定義)〈`-fcommon`：舊程式為什麼這樣寫也能動〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. gcc 10 以前預設 `-fcommon`：沒有初值的全域變數被標成 **common 符號**（`Ndx` 是 `COM`，`nm` 印 `C`），不是一般的定義。
> 2. 連結器遇到好幾個同名的 common 符號會**合併成一份**，不報錯，兩個檔案共用同一個 `hits`。
>
> **詳細說明**：gcc 10 起預設改成 `-fno-common`。正確修法是補 `extern`（4-3），不是加 `-fcommon`。


---
