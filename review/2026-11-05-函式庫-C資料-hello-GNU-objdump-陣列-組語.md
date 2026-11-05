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

> 批改中：已批改 1 題（函式庫 2）——對 1 題。其餘 12 題待作答。

| 題目 | 結果 | 盒子 | 下次 | 錯在哪／缺什麼 | 過關 |
|---|---|---|---|---|---|
| 函式庫 2-1 | 對 | | | 抓到：讀到 `libtxt.a` 時清單上還沒有 `trim`，一個成員都不拉；讀到 `main.o` 才需要，`ld` 不回頭。小修：「丟棄」是 `.so` 的說法，`.a` 是「一個成員都不拉」 | — |
| 函式庫 2-2 | 對 | | | 抓到：`cannot find` 是找不到函式庫**檔案**，`undefined reference` 是檔案有找到、`trim` 這個**符號**沒被解決 | — |
| 函式庫 2-3 | 對 | 2 → 3（整題） | 2026-11-12 | 抓到：Ubuntu 預設 `--as-needed`，讀到 `-ltxt` 時沒人需要 `trim`，`.so` 被丟掉。小修：是命令列處理完才報錯，不是讀到 `main.o` 當下 | — |

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

#### 函式庫 4-2. 你想讓 `app` 和 `lib/` 一起打包給同事，同事解壓到任何地方都能直接執行，連結指令要怎麼改？為什麼要用單引號？

```
加入runpath
gcc main.c -Llib -lfoo -Wl,-rpath,'$ORIGIN/lib' -o app

因爲單引號就是把裏面的內容視爲純字符串,不會展開像是變量$ORIGIN 得到的runpath就真的是 '$ORIGIN/lib', 這樣ld.so可知道,origin是在執行檔所在的位置,也就可以正確的找到庫的位置
用雙引號就會展開,最後因爲$ORIGIN是空字符串得到了/lib,找不到
```

#### 函式庫 4-3. 同事想臨時用 `LD_LIBRARY_PATH` 換成他自己編的 `libfoo.so` 來除錯，在你的做法（`RUNPATH`）下換得掉嗎？如果是 `RPATH` 呢？為什麼？

```
runpath 可以
rpath不行

因爲runpath,會優先讀LD_LIBRARY_PATH, 但rpath會優先讀rpath
```

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
```

#### 函式庫 8-2. 第一次和第二次 `call 1070 <log_msg@plt>`，從 `call` 到進入 `log_msg` 各經過哪些地方？`ld.so` 在哪一次介入、做了什麼？

```
```

#### 函式庫 8-3. 全域變數為什麼不能用這種「第一次用到才查」的方式，一定要載入時就把 `.got` 那格填好？

```
```

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
```

#### 函式庫 10-4. 同學分別用 gdb 跑 `m` 和 `n`，`p area` 兩邊都印 `0x7ffff7fb7119`，他說「看吧，兩個行程的 `area` 在同一個位址」。這個推論哪裡有問題？

```
```

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
```

#### 函式庫 11-2. `nm setlevel` 會不會看到 `level`？在哪一種區段？為什麼原始碼只有 `extern` 卻會有？

```
```

#### 函式庫 11-3. 第一行印出的值是怎麼放進那裡的？`get_level()` 讀的是哪一份 `level`？用重定位的類型和 `ld.so` 找符號的順序說明。

```
```

#### 函式庫 11-4. 之後把 `cfg.c` 改成 `long level = 3;` 重做 `libcfg.so`，`setlevel` 不重新連結就執行，會發生什麼事、為什麼？

```
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
```

#### 函式庫 13-2. 預設下 `gcc -Wall -L. -ltxt main.c -o app_bad` 失敗；把 `-Wl,--no-as-needed` 放在 `-ltxt` 前面（`gcc -Wall -L. -Wl,--no-as-needed -ltxt main.c -o app_no`）就成功。讀到 `libtxt.so` 時兩種模式各做了什麼？`main.o` 要的 `trim` 最後是怎麼被解決的——是 `libtxt.so` 把 `trim` 加進了清單嗎？

```
```

#### 函式庫 13-3. `none.c` 用 `gcc -Wall none.c -L. -Wl,--no-as-needed -ltxt -o none_no` 和 `gcc -Wall none.c -L. -ltxt -o none_as` 各連結一次，`readelf -d` 的 `NEEDED` 會差在哪？Ubuntu 為什麼預設開 `--as-needed`？

```
```

#### 函式庫 13-4. 把 `libtxt.so` 換成同樣三個 `.o` 做成的 `libtxt.a`（並刪掉 `libtxt.so`），`gcc -Wall -L. -Wl,--no-as-needed -ltxt main.c -o app_a` 會成功嗎？為什麼？

```
```

---

## C資料：[C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號](../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html)

### C資料 9. 情境題：有人寫了 `if (len - used > 0)` 判斷「緩衝區還有沒有剩餘空間」，`len`、`used` 都是 `size_t`，用 `-Wall -Wextra` 編譯沒有任何警告。這個判斷在什麼情況下會出錯、為什麼？為什麼 gcc 對 `len - used < 0` 會警告、對 `> 0` 卻不會？另外，有人用 `int sum = x + y; if (sum < x)` 檢查「`x` 加上一個**正數** `y` 會不會溢位」，為什麼這個檢查在 `-O2` 下可能完全失效？

> 盒子 2｜過去的作答：[s1-02 quiz.md 第 9 題](../small-project/s1-02-data-repr/quiz.md#9-情境題有人寫了-if-len---used--0-判斷緩衝區還有沒有剩餘空間lenused-都是-size_t用--wall--wextra-編譯沒有任何警告這個判斷在什麼情況下會出錯為什麼-gcc-對-len---used--0-會警告對--0-卻不會另外為什麼-int-sum--x--y-if-sum--x-這種溢位檢查在--o2-下可能完全失效)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-24 複習](./2026-10-24-gcc-GNU-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-27 複習](./2026-10-27-gcc-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-30 複習](./2026-10-30-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語.md#c資料-9-情境題有人寫了-if-len---used--0-判斷緩衝區還有沒有剩餘空間lenused-都是-size_t用--wall--wextra-編譯沒有任何警告這個判斷在什麼情況下會出錯為什麼-gcc-對-len---used--0-會警告對--0-卻不會另外為什麼-int-sum--x--y-if-sum--x-這種溢位檢查在--o2-下可能完全失效)、[11-02 複習](./2026-11-02-函式庫-interp-C資料-字元-陣列-組語-組語二-多檔.md#c資料-9-情境題有人寫了-if-len---used--0-判斷緩衝區還有沒有剩餘空間lenused-都是-size_t用--wall--wextra-編譯沒有任何警告這個判斷在什麼情況下會出錯為什麼-gcc-對-len---used--0-會警告對--0-卻不會另外有人用-int-sum--x--y-if-sum--x-檢查x-加上一個正數-y-會不會溢位為什麼這個檢查在--o2-下可能完全失效)

```
```

---

## hello：[hello.c怎麼變成能跑的程式-編譯的四個階段](../html/c/hello.c怎麼變成能跑的程式-編譯的四個階段.html)

### hello 5. 情境題：你把一支動態連結的程式複製到另一台機器，執行時出現找不到 `libc.so.6` 某個版本符號的錯誤。從本課的哪個階段可以解釋這件事？如果改用 `-static` 編譯，這個問題還會發生嗎？為什麼？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 5 題](../small-project/s1-01-hello-compile/quiz.md#5-情境題你把一支動態連結的程式複製到另一台機器執行時出現找不到-libcso6-某個版本符號的錯誤從本課的哪個階段可以解釋這件事如果改用--static-編譯這個問題還會發生嗎)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md#hello-5-情境題你把一支動態連結的程式複製到另一台機器執行時出現找不到-libcso6-某個版本符號的錯誤從本課的哪個階段可以解釋這件事如果改用--static-編譯這個問題還會發生嗎)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-29 複習](./2026-10-29-hello-GNU-objdump-陣列-組語.md#hello-5-情境題你把一支動態連結的程式複製到另一台機器執行時出現找不到-libcso6-某個版本符號的錯誤從本課的哪個階段可以解釋這件事如果改用--static-編譯這個問題還會發生嗎)

```
```

---

## GNU：[GNU是什麼-Linux核心與GNU工具的分工](../html/linux/GNU是什麼-Linux核心與GNU工具的分工.html)

### GNU 4. GPL 的 copyleft 要求什麼？glibc 用 LGPL 而不是 GPL，對「商業軟體能不能連結 libc」有什麼差別？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 4 題](../small-project/s1-01-hello-compile/quiz.md#4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-06 複習](./2026-10-06-字元-interp-ELF-gcc-GNU.md#gnu-4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-10 複習](./2026-10-10-C資料-GNU-interp-hello-gcc.md#gnu-4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)、[10-19 複習](./2026-10-19-interp-GNU-C資料-ELF-objdump-組語.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-29 複習](./2026-10-29-hello-GNU-objdump-陣列-組語.md#gnu-4-gpl-的-copyleft-要求什麼glibc-用-lgpl-而不是-gpl對商業軟體能不能連結-libc有什麼差別)

```
```

---

## objdump：[objdump能不能代替ldd找依賴庫-需求單與實際載入](../html/c/objdump能不能代替ldd找依賴庫-需求單與實際載入.html)

### objdump 3. 一支程式連結時用了 `-L./lib -lfoo`，編譯成功；執行時卻出現 `error while loading shared libraries: libfoo.so`。用「連結時」與「執行時」分別是誰在找檔案，解釋為什麼連結找得到、執行卻找不到。這時 `objdump -p` 與 `ldd` 各會顯示什麼？

> 盒子 3｜過去的作答：[s1-01 quiz.md 第 3 題](../small-project/s1-01-hello-compile/quiz.md#3-一支程式連結時用了--llib--lfoo編譯成功執行時卻出現-error-while-loading-shared-libraries-libfooso用連結時與執行時分別是誰在找檔案解釋為什麼連結找得到執行卻找不到這時-objdump--p-與-ldd-各會顯示什麼)、[10-04 複習](./2026-10-04-字元-C資料-objdump-hello-ELF.md)、[10-08 複習](./2026-10-08-字元-C資料-objdump-hello-ELF-陣列.md)、[10-11 複習](./2026-10-11-字元-C資料-objdump-GNU-陣列.md)、[10-20 複習](./2026-10-20-C資料-objdump-字元-陣列-gcc.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-29 複習](./2026-10-29-hello-GNU-objdump-陣列-組語.md#objdump-3-一支程式連結時用了--llib--lfoo編譯成功執行時卻出現-error-while-loading-shared-libraries-libfooso用連結時與執行時分別是誰在找檔案解釋為什麼連結找得到執行卻找不到這時-objdump--p-與-ldd-各會顯示什麼)

```
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
```

#### 陣列 2-2. 這件事對「怎麼設計接收陣列的函式」有什麼影響？

> 題目：這件事（陣列傳進函式之後 `sizeof a` 從 40 變成 8）對「怎麼設計接收陣列的函式」有什麼影響？

```
```

#### 陣列 2-3. `a` 和 `&a` 印出來的位址一樣，它們差在哪裡？

> 題目：`a` 和 `&a` 印出來的位址一樣，它們差在哪裡？（`int a[10];`）

```
```

---

## 組語：[x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍](../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html)

### 組語 6. 組合語言的大小名稱 vs C 的型別

> 盒子 3｜過去的作答：[s1-04 quiz.md 第 6 題](../small-project/s1-04-asm-basics/quiz.md#6-組合語言的大小名稱-vs-c-的型別2026-10-06-課後提問新增)、[10-12 複習](./2026-10-12-objdump-hello-ELF-陣列-組語.md)、[10-21 複習](./2026-10-21-字元-objdump-陣列-組語-interp-ELF-GNU.md)、[10-23 複習](./2026-10-23-大複習-hello-gcc-GNU-interp-ELF-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-26 複習](./2026-10-26-hello-GNU-interp-objdump-C資料-字元-陣列-組語-組語二-多檔.md)、[10-29 複習](./2026-10-29-hello-GNU-objdump-陣列-組語.md#組語-6-組合語言的大小名稱-vs-c-的型別)

#### 組語 6-1. `movsbl` 的 `l` 叫 long，為什麼是 4 byte，而本機 C 的 `long` 是 8 byte？

```
```

#### 組語 6-2. 本機 C 的 `long` 在組合語言裡對到哪個字尾？

```
```

#### 組語 6-3. 這樣對嗎？為什麼？

> 題目：情境題：第 5 課你在 `gdb` 裡想看一個 `int` 變數的 4 個 byte，同學說「`int` 在組合語言是 `l`，所以用 `x/1l`」。這樣對嗎？為什麼？

```
```

#### 組語 6-4. 應該用哪個大小字母，為什麼？

> 題目：（承上）在 gdb 裡用 `x` 看 `int` 變數的 4 個 byte，應該用哪個大小字母，為什麼？

```
```

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
```

#### 組語 12-2. 如果程式是用 `-no-pie` 編的，`break *` 加 `objdump` 查到的位址會不會成功？為什麼？

```
```

---
