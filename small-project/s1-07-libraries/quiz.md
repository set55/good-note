# s1-07 自我測驗作答

> 第 ① 站第 7 課的自我測驗寫在這裡。題目從筆記複製過來；批改完後，最新版的參考答案也寫在筆記每題下面。
>
> **怎麼寫**：直接在每題下面的空白區塊裡作答，**闔上筆記、用自己的話**（把 `YYYY-MM-DD` 換成作答日期）。寫完跟 Claude 說「幫我批改」。
> 批改與參考答案會寫在你的答案下方，**不重答**：答錯或不完整也直接附參考答案，沒弄懂的交給間隔複習再考。
>
> **複習**：間隔複習的作答寫在 repo 根目錄的 [`review/`](../../review/README.md)（每天一個檔），排程見 [複習排程](../../learning/複習排程.md)。本檔只放第一次作答。

## 目錄

| 測驗 | 對應筆記 | 狀態 |
|---|---|---|
| [第 7 課：靜態與動態函式庫](#未答第-7-課靜態與動態函式庫) | [靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html) | 未答 |

---

## 未答：第 7 課：靜態與動態函式庫

> 題目來自 [靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html)。
> 每題拆成小題（`N-M`），每個小題各有一個作答框；題目用到的程式碼附在每個大題的開頭。批改寫在該小題的作答框正下方。

### 1. 靜態函式庫：哪些函式會進執行檔

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

#### 1-1. 最後的執行檔裡會有哪些函式（`to_upper_str`、`to_lower_str`、`trim`、`is_space`）？為什麼？

**第 1 次（YYYY-MM-DD）**
```
```

#### 1-2. 如果把連結指令改成直接寫三個 `.o`（`gcc main.c case.o trim.o space.o`），結果有什麼不同、為什麼？

**第 1 次（YYYY-MM-DD）**
```
```

### 2. 順序：`-ltxt` 寫在 `main.c` 前面

承上題的檔案（`case.c`、`space.c`、`trim.c`、`main.c` 見第 1 題），`gcc -Wall -L. -ltxt main.c -o app` 會失敗，錯誤訊息是 `undefined reference to 'trim'`。用 `ld` 處理輸入檔的方式解釋為什麼。這個錯誤跟 `cannot find -ltxt` 差在哪（各是什麼東西找不到）？在 Ubuntu 上把 `libtxt.a` 換成 `libtxt.so`、命令一樣寫成 `-L. -ltxt main.c`，也會失敗，原因是什麼？

#### 2-1. `gcc -Wall -L. -ltxt main.c -o app` 會失敗，錯誤訊息是 `undefined reference to 'trim'`。用 `ld` 處理輸入檔的方式解釋為什麼。

**第 1 次（YYYY-MM-DD）**
```
```

#### 2-2. 這個錯誤跟 `cannot find -ltxt` 差在哪（各是什麼東西找不到）？

**第 1 次（YYYY-MM-DD）**
```
```

#### 2-3. 在 Ubuntu 上把 `libtxt.a` 換成 `libtxt.so`、命令一樣寫成 `-L. -ltxt main.c`，也會失敗，原因是什麼？

**第 1 次（YYYY-MM-DD）**
```
```

### 3. `-fPIC`：哪個變數出問題、為什麼要 GOT

下面的 `stats.c` 不加 `-fPIC` 編成 `stats.o`，再用 `gcc -shared stats.o -o libstats.so`。會成功嗎？錯誤訊息會提到哪個變數？為什麼另一個變數不會有問題？為什麼 `.so` 存取全域變數一定要經過 GOT，而不能像執行檔那樣用 `%rip` 加固定距離直接讀（講出兩個理由）？

```c
/* stats.c */
int hits = 0;
static int misses = 0;

void hit(void)  { hits = hits + 1; }
void miss(void) { misses = misses + 1; }
```
編譯：`gcc -Wall -c stats.c -o stats.o && gcc -shared stats.o -o libstats.so`

#### 3-1. 會成功嗎？錯誤訊息會提到哪個變數？

**第 1 次（YYYY-MM-DD）**
```
```

#### 3-2. 為什麼另一個變數不會有問題？

**第 1 次（YYYY-MM-DD）**
```
```

#### 3-3. 為什麼 `.so` 存取全域變數一定要經過 GOT，而不能像執行檔那樣用 `%rip` 加固定距離直接讀（講出兩個理由）？

**第 1 次（YYYY-MM-DD）**
```
```

### 4. 情境：連結成功、執行找不到 `libfoo.so`

情境題：你在 `/home/me/proj/` 下做了 `lib/libfoo.so` 和用它的 `app`，連結指令是 `gcc main.c -Llib -lfoo -o app`。連結成功，`./app` 卻印出 `error while loading shared libraries: libfoo.so: cannot open shared object file`。(1) 這個訊息是誰印的？為什麼連結時找得到、執行時找不到？(2) 你想讓 `app` 和 `lib/` 一起打包給同事，同事解壓到任何地方都能直接執行，連結指令要怎麼改？為什麼要用單引號？(3) 同事想臨時用 `LD_LIBRARY_PATH` 換成他自己編的 `libfoo.so` 來除錯，在你的做法（`RUNPATH`）下換得掉嗎？如果是 `RPATH` 呢？為什麼？

#### 4-1. 這個訊息是誰印的？為什麼連結時找得到、執行時找不到？

**第 1 次（YYYY-MM-DD）**
```
```

#### 4-2. 你想讓 `app` 和 `lib/` 一起打包給同事，同事解壓到任何地方都能直接執行，連結指令要怎麼改？為什麼要用單引號？

**第 1 次（YYYY-MM-DD）**
```
```

#### 4-3. 同事想臨時用 `LD_LIBRARY_PATH` 換成他自己編的 `libfoo.so` 來除錯，在你的做法（`RUNPATH`）下換得掉嗎？如果是 `RPATH` 呢？為什麼？

**第 1 次（YYYY-MM-DD）**
```
```

### 5. SONAME 與函式庫更新

你維護 `libcalc`，SONAME 是 `libcalc.so.1`。系統上有 `libcalc.so -> libcalc.so.1`、`libcalc.so.1 -> libcalc.so.1.0.0` 兩個符號連結。(1) 一支程式動態連結了 `libcalc`，它的 `NEEDED` 寫的是哪個名字？為什麼刪掉 `libcalc.so` 它照樣能跑，刪掉 `libcalc.so.1` 就不行？(2) 你修了一個 bug（介面不變），做成 `libcalc.so.1.0.1`。已經動態連結的程式要怎樣才能用到修正？另一支用 `libcalc.a` 靜態連結的程式呢？(3) 下一版要把 `calc_add` 改成三個參數（不相容），SONAME 該怎麼辦？為什麼這樣舊程式不會壞？

#### 5-1. 一支程式動態連結了 `libcalc`，它的 `NEEDED` 寫的是哪個名字？為什麼刪掉 `libcalc.so` 它照樣能跑，刪掉 `libcalc.so.1` 就不行？

**第 1 次（YYYY-MM-DD）**
```
```

#### 5-2. 你修了一個 bug（介面不變），做成 `libcalc.so.1.0.1`。已經動態連結的程式要怎樣才能用到修正？另一支用 `libcalc.a` 靜態連結的程式呢？

**第 1 次（YYYY-MM-DD）**
```
```

#### 5-3. 下一版要把 `calc_add` 改成三個參數（不相容），SONAME 該怎麼辦？為什麼這樣舊程式不會壞？

**第 1 次（YYYY-MM-DD）**
```
```

### 6. GOT：`D(%rip)` 的距離、GOT 那一格與變數本身（10-25 追問新增）

看下面 `libtally.so` 的反組譯（`total = total + x;` 那一段）回答：(1) `111a` 那一行的 `0x2eaf` 是從哪裡到哪裡的距離？用這段的數字算出右邊註解的 `3fd0`。`112b` 那一行的 `D` 是 `0x2e9e`，跟 `0x2eaf` 不一樣，為什麼算出來還是 `3fd0`？(2) 註解 `<total-0x48>` 是說 `total` 放在 `0x3fd0` 嗎？`0x3fd0` 是什麼、`total` 本身在哪？(3) 執行完 `111a` 那一行，`%rax` 裡裝的是什麼？`1121` 讀到的又是什麼？為什麼用 `objdump -s -j .got libtally.so` 看，`0x3fd0` 那格在檔案裡是 0？

```c
/* tally.c */
int seen = 0;
long total = 0;

void add_sample(long x)
{
    seen = seen + 1;
    total = total + x;
}
```
編譯與觀察：`gcc -Wall -fPIC -c tally.c -o tally.pic.o && gcc -shared tally.pic.o -o libtally.so`，再 `objdump -d --no-show-raw-insn libtally.so` 與 `readelf -r libtally.so`，`total = total + x;` 那一段是：
```asm
    111a:	mov    0x2eaf(%rip),%rax        # 3fd0 <total-0x48>   ← 從 %rip 加 0x2eaf 的地方讀 8 byte 放進 %rax
    1121:	mov    (%rax),%rdx                                  ← 照 %rax 裡的位址讀 8 byte 放進 %rdx
    1124:	mov    -0x8(%rbp),%rax                              ← 從堆疊取出參數 x 放進 %rax
    1128:	add    %rax,%rdx                                    ← %rdx = %rdx + x
    112b:	mov    0x2e9e(%rip),%rax        # 3fd0 <total-0x48>   ← 同第一行，D 不同
    1132:	mov    %rdx,(%rax)                                  ← 把 %rdx 寫到 %rax 裡的位址
```
```text
000000003fd0  000700000006 R_X86_64_GLOB_DAT 0000000000004018 total + 0
000000003fd8  000600000006 R_X86_64_GLOB_DAT 0000000000004010 seen + 0
```
（檔案：[`tally.c`](./examples/pic/tally.c)）

#### 6-1. `111a` 那一行的 `0x2eaf` 是從哪裡到哪裡的距離？用這段的數字算出右邊註解的 `3fd0`。

**第 1 次（YYYY-MM-DD）**
```
```

#### 6-2. `112b` 那一行的 `D` 是 `0x2e9e`，跟 `0x2eaf` 不一樣，為什麼算出來還是 `3fd0`？

**第 1 次（YYYY-MM-DD）**
```
```

#### 6-3. 註解 `<total-0x48>` 是說 `total` 放在 `0x3fd0` 嗎？`0x3fd0` 是什麼、`total` 本身在哪？

**第 1 次（YYYY-MM-DD）**
```
```

#### 6-4. 執行完 `111a` 那一行，`%rax` 裡裝的是什麼？`1121` 讀到的又是什麼？

**第 1 次（YYYY-MM-DD）**
```
```

#### 6-5. 為什麼用 `objdump -s -j .got libtally.so` 看，`0x3fd0` 那格在檔案裡是 0？

**第 1 次（YYYY-MM-DD）**
```
```

### 7. 兩份同名全域變數：接 `.so` 為什麼不算重複定義（10-25 追問新增）

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
做函式庫：`gcc -Wall -fPIC -c conf.c -o conf.pic.o && gcc -shared conf.pic.o -o libconf.so`（檔案：[`conf.c`](./examples/pic/conf.c)、[`app.c`](./examples/pic/app.c)）

#### 7-1. `gcc -Wall app.c -L. -lconf -Wl,-rpath,'$ORIGIN' -o app` 會不會出現 `multiple definition`？為什麼？

**第 1 次（YYYY-MM-DD）**
```
```

#### 7-2. 改成 `gcc -Wall app.c conf.pic.o -o app2` 呢？差別在 `ld` 怎麼處理 `.so` 和 `.o`。

**第 1 次（YYYY-MM-DD）**
```
```

#### 7-3. 能連結成功的那一個，`./app` 會印出 `get_verbose() = ` 多少？用執行時 `ld.so` 怎麼找符號解釋。

**第 1 次（YYYY-MM-DD）**
```
```

#### 7-4. 把 `conf.c` 第一行改成 `static int verbose = 0;`，重做 `libconf.so`，不重新連結就執行 `./app`，印出多少？為什麼跟 7-3 不同？

**第 1 次（YYYY-MM-DD）**
```
```

### 8. PLT 與 GOT：函式的位址存在哪、延遲綁定（10-26 追問新增）

`liblogger.so` 的 `run()` 呼叫了 `log_msg()` 兩次，主程式 `runner.c` 只呼叫 `run()`。(1) 反組譯裡的 `log_msg@plt` 是一張存位址的表嗎？`log_msg` 的位址存在哪個區段的哪一格？用 `jmp` 那行的數字算出來。(2) 第一次和第二次 `call 1070 <log_msg@plt>`，從 `call` 到進入 `log_msg` 各經過哪些地方？`ld.so` 在哪一次介入、做了什麼？(3) 全域變數為什麼不能用這種「第一次用到才查」的方式，一定要載入時就把 `.got` 那格填好？

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
編譯：`gcc -Wall -fPIC -c logger.c -o logger.pic.o && gcc -shared logger.pic.o -o liblogger.so && gcc -Wall runner.c -L. -llogger -Wl,-rpath,'$ORIGIN' -o runner`（檔案：[`logger.c`](./examples/pic/logger.c)、[`runner.c`](./examples/pic/runner.c)）。觀察到的反組譯與區段（`objdump -d --no-show-raw-insn liblogger.so`、`readelf -SW liblogger.so`、`readelf -r liblogger.so`）：
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

#### 8-1. 反組譯裡的 `log_msg@plt` 是一張存位址的表嗎？`log_msg` 的位址存在哪個區段的哪一格？用 `jmp` 那行的數字算出來。

**第 1 次（YYYY-MM-DD）**
```
```

#### 8-2. 第一次和第二次 `call 1070 <log_msg@plt>`，從 `call` 到進入 `log_msg` 各經過哪些地方？`ld.so` 在哪一次介入、做了什麼？

**第 1 次（YYYY-MM-DD）**
```
```

#### 8-3. 全域變數為什麼不能用這種「第一次用到才查」的方式，一定要載入時就把 `.got` 那格填好？

**第 1 次（YYYY-MM-DD）**
```
```

### 9. 兩支程式共用一個 `.so`：哪些是同一份、哪些各一份（10-26 追問新增）

兩支程式 `x`、`y` 都連 `libhits.so`，同時在跑（各自停在 `sleep(5)`）。(1) `x` 印出的 `&hit` 和 `y` 印出的不一樣，這代表記憶體裡有兩份 `hit` 的程式碼嗎？`libhits.so` 的哪些部分兩個行程共用同一份實體記憶體、哪些是各自一份？(2) `y` 印出的 `hit()` 回傳值是多少？為什麼不是 `x`、`y` 加起來的 7？(3) 假設沒有 GOT，改成讓 `ld.so` 載入時把位址直接寫進 `.text`，用「寫入時複製」說明 `.text` 還能不能共用、為什麼。

```c
/* hits.c */
int hits = 0;

int hit(void)
{
    hits = hits + 1;
    return hits;
}
```
```c
/* x.c */
#include <stdio.h>
#include <unistd.h>

int hit(void);

int main(void)
{
    int r = 0;
    for (int i = 0; i < 2; i++)               /* x 呼叫 2 次 */
        r = hit();
    printf("x: &hit=%p  hit() 回傳 %d\n", (void *)hit, r);
    fflush(stdout);
    sleep(5);
    return 0;
}
```
```c
/* y.c */
#include <stdio.h>
#include <unistd.h>

int hit(void);

int main(void)
{
    int r = 0;
    for (int i = 0; i < 5; i++)               /* y 呼叫 5 次 */
        r = hit();
    printf("y: &hit=%p  hit() 回傳 %d\n", (void *)hit, r);
    fflush(stdout);
    sleep(5);
    return 0;
}
```
編譯：`gcc -Wall -fPIC -c hits.c -o hits.pic.o && gcc -shared hits.pic.o -o libhits.so && gcc -Wall x.c -L. -lhits -Wl,-rpath,'$ORIGIN' -o x && gcc -Wall y.c -L. -lhits -Wl,-rpath,'$ORIGIN' -o y`，執行：`./x & ./y & wait`（檔案：[`hits.c`](./examples/share/hits.c)、[`x.c`](./examples/share/x.c)、[`y.c`](./examples/share/y.c)）。

#### 9-1. `x` 印出的 `&hit` 和 `y` 印出的不一樣，這代表記憶體裡有兩份 `hit` 的程式碼嗎？`libhits.so` 的哪些部分兩個行程共用同一份實體記憶體、哪些是各自一份？

**第 1 次（YYYY-MM-DD）**
```
```

#### 9-2. `y` 印出的 `hit()` 回傳值是多少？為什麼不是 `x`、`y` 加起來的 7？

**第 1 次（YYYY-MM-DD）**
```
```

#### 9-3. 假設沒有 GOT，改成讓 `ld.so` 載入時把位址直接寫進 `.text`，用「寫入時複製」說明 `.text` 還能不能共用、為什麼。

**第 1 次（YYYY-MM-DD）**
```
```

### 10. 共用的 `.so`：格子裡的位址與實體頁（10-26 追問新增）

兩支程式 `m`、`n` 都連 `libshape.so`（兩支都沒有自己的 `area`），同時在跑。`libshape.so` 的 `calc()` 透過 `area@plt` 呼叫 `area()`。(1) 兩個行程第一次呼叫 `calc` 之後，各自 `.got.plt` 裡 `area` 那格裝的數字一樣嗎？那個數字是誰、用哪兩樣東西算出來的？(2) 兩個行程執行的 `area` 程式碼是同一份實體記憶體嗎？`/proc/<pid>/smaps` 裡哪個欄位、出現什麼數值可以證明？(3) 跳到那格裡的位址時，虛擬位址由誰翻成實體位址？翻譯用的表又是誰準備的？(4) 同學分別用 gdb 跑 `m` 和 `n`，`p area` 兩邊都印 `0x7ffff7fb7119`，他說「看吧，兩個行程的 `area` 在同一個位址」。這個推論哪裡有問題？

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
編譯與執行：`gcc -Wall -fPIC -c shape.c -o shape.pic.o && gcc -shared shape.pic.o -o libshape.so && gcc -Wall m.c -L. -lshape -Wl,-rpath,'$ORIGIN' -o m && gcc -Wall n.c -L. -lshape -Wl,-rpath,'$ORIGIN' -o n && ./m & ./n & wait`（檔案：[`shape.c`](./examples/share/shape.c)、[`m.c`](./examples/share/m.c)、[`n.c`](./examples/share/n.c)）。`libshape.so` 的反組譯與重定位（`objdump -d --no-show-raw-insn libshape.so`、`readelf -r libshape.so`）：
```asm
    1156:	call   1050 <area@plt>                  ← calc 裡呼叫 area
0000000000001050 <area@plt>:
    1050:	endbr64                             ← 間接跳躍的保護標記，可以忽略
    1054:	jmp    *0x2fa6(%rip)        # 4000 <area+0x2ee7>   ← 照 0x4000 那格裡存的位址跳
```
```text
000000004000  000600000007 R_X86_64_JUMP_SLO 0000000000001119 area + 0
```

#### 10-1. 兩個行程第一次呼叫 `calc` 之後，各自 `.got.plt` 裡 `area` 那格裝的數字一樣嗎？那個數字是誰、用哪兩樣東西算出來的？

**第 1 次（YYYY-MM-DD）**
```
```

#### 10-2. 兩個行程執行的 `area` 程式碼是同一份實體記憶體嗎？`/proc/<pid>/smaps` 裡哪個欄位、出現什麼數值可以證明？

**第 1 次（YYYY-MM-DD）**
```
```

#### 10-3. 跳到那格裡的位址時，虛擬位址由誰翻成實體位址？翻譯用的表又是誰準備的？

**第 1 次（YYYY-MM-DD）**
```
```

#### 10-4. 同學分別用 gdb 跑 `m` 和 `n`，`p area` 兩邊都印 `0x7ffff7fb7119`，他說「看吧，兩個行程的 `area` 在同一個位址」。這個推論哪裡有問題？

**第 1 次（YYYY-MM-DD）**
```
```

### 11. 主程式只寫 `extern`：變數放在哪裡（10-26 追問新增）

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
編譯：`gcc -Wall -fPIC -c cfg.c -o cfg.pic.o && gcc -shared cfg.pic.o -o libcfg.so && gcc -Wall setlevel.c -L. -lcfg -Wl,-rpath,'$ORIGIN' -o setlevel`（檔案：[`cfg.c`](./examples/pic/cfg.c)、[`setlevel.c`](./examples/pic/setlevel.c)）。主程式是 Ubuntu 預設的 `-fPIE`。

#### 11-1. 執行後兩行各印多少？

**第 1 次（YYYY-MM-DD）**
```
```

#### 11-2. `nm setlevel` 會不會看到 `level`？在哪一種區段？為什麼原始碼只有 `extern` 卻會有？

**第 1 次（YYYY-MM-DD）**
```
```

#### 11-3. 第一行印出的值是怎麼放進那裡的？`get_level()` 讀的是哪一份 `level`？用重定位的類型和 `ld.so` 找符號的順序說明。

**第 1 次（YYYY-MM-DD）**
```
```

#### 11-4. 之後把 `cfg.c` 改成 `long level = 3;` 重做 `libcfg.so`，`setlevel` 不重新連結就執行，會發生什麼事、為什麼？

**第 1 次（YYYY-MM-DD）**
```
```

### 12. `.h` 與 `-l`：各在哪個階段用到（10-26 追問新增）

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
做 `.so`：`gcc -Wall -fPIC -Iinclude -c temp.c -o temp.pic.o && gcc -shared temp.pic.o -o lib/libtemp.so`（檔案：[`include/temp.h`](./examples/headers/temp/include/temp.h)、[`temp.c`](./examples/headers/temp/temp.c)、[`main.c`](./examples/headers/temp/main.c)）。

#### 12-1. `gcc -Wall main.c -Llib -ltemp -o app` 會失敗在哪個階段（前處理、編譯、連結、執行）？錯誤大概長怎樣、要補什麼？

**第 1 次（YYYY-MM-DD）**
```
```

#### 12-2. `gcc -Wall -Iinclude main.c -Llib -o app` 呢？

**第 1 次（YYYY-MM-DD）**
```
```

#### 12-3. 同事說「`FREEZING_C` 的值存在 `libtemp.so` 裡，執行時才讀進來」，對嗎？為什麼？

**第 1 次（YYYY-MM-DD）**
```
```

#### 12-4. 編好的 `app`（連結時加了 `-Wl,-rpath,'$ORIGIN/lib'`）要拿給別人執行，`include/temp.h` 要一起給嗎？`lib/libtemp.so` 呢？

**第 1 次（YYYY-MM-DD）**
```
```
