# s1-07：靜態與動態函式庫——`ar` 與 `.a`、`-fPIC` 與 `.so`、`-L`／`-l`、執行時怎麼找到 `.so`

> 對應課程：第 ① 站第 7 課
> 課程筆記：[靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html)
> 自我測驗：寫在同資料夾的 [quiz.md](./quiz.md)

## 說明

這一課要**自己動手做**函式庫：用 `ar` 包 `.a`、用 `-fPIC -shared` 做 `.so`，然後讓程式在連結時、執行時都找得到它們。

- 每一題一個資料夾（`hw1/`～`hw5/`），檔案自己建。題目給的程式碼照打；題目給的指令照下，**指令的輸出要貼進作答框**。
- 編譯一律加 `-Wall`，**有警告就要處理**，不能忽略。
- 編譯產物（`.o`、`.a`、`.so`、執行檔）不會進 git（白名單），要留的輸出**貼進作答框**。
- 筆記裡的範例（`calc_add`、`counter`）可以參考，但每一題的檔案、情境都跟範例不同，照抄範例答不出來。

每題「解釋原因」的地方都標了答案的來源：
- 【筆記第 X 節】：本課教過，回筆記那一節找。
- 【新觀念，先猜】：還沒教過，寫下你的猜測就好，猜錯不算錯。

**「先預測」的題目，一定要先把預測寫進作答框，再執行。** 預測錯了沒關係，把錯在哪裡寫下來，這才是練習的重點。

**第 6 課反覆卡在同一件事：實驗沒跑到最後一步、沒貼證據就下結論。** 這一課每個小題都寫明「要貼什麼」，缺了就算不完整：
- 「沒有錯誤訊息」不代表結果對，一定要**執行**並貼上輸出；
- 改了檔案之後，確認你執行的是**這次重新做出來的**檔案（必要時先 `rm` 舊的產物）。

**怎麼作答**：每個小題下面有作答框，直接寫在裡面（把 `YYYY-MM-DD` 換成作答日期）。批改與參考答案寫在作答框正下方，不重答。

---

## 筆記的範例程式

課程筆記裡每個實驗的原始碼都放在 [`examples/`](./examples/)，一個實驗一個子資料夾，可以直接跟著筆記重做。**它們是範例，不是作業**；作業寫在 `hw1/`～`hw5/`，檔名跟範例不重複。

| 資料夾 | 檔案 | 筆記裡的位置 | 示範什麼 |
|---|---|---|---|
| [`examples/calc/`](./examples/calc/) | [`calc.h`](./examples/calc/calc.h)、[`add.c`](./examples/calc/add.c)、[`mul.c`](./examples/calc/mul.c)、[`main.c`](./examples/calc/main.c)、[`both.c`](./examples/calc/both.c) | [第一節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#一本課在做什麼函式庫就是先編好的-o)～[第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#四動態函式庫-shared-與--fpic) | `ar rcs` 做 `libcalc.a`、只拉需要的成員、`both.c` 整個 `.o` 被拉進來、`-lcalc` 寫在前面的 `undefined reference`、`-L`／`-l`／`-l:`／`-static`、做 `libcalc.so` 與執行時找不到 |
| [`examples/pic/`](./examples/pic/) | [`count.c`](./examples/pic/count.c)、[`count_static.c`](./examples/pic/count_static.c)、[`usecount.c`](./examples/pic/usecount.c)、[`callbump.c`](./examples/pic/callbump.c)、[`tally.c`](./examples/pic/tally.c)、[`conf.c`](./examples/pic/conf.c)、[`app.c`](./examples/pic/app.c)、[`greet.c`](./examples/pic/greet.c)、[`greet_main.c`](./examples/pic/greet_main.c)、[`logger.c`](./examples/pic/logger.c)、[`runner.c`](./examples/pic/runner.c)、[`usecount2.c`](./examples/pic/usecount2.c)、[`count_v2.c`](./examples/pic/count_v2.c)、[`cfg.c`](./examples/pic/cfg.c)、[`setlevel.c`](./examples/pic/setlevel.c) | [第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#四動態函式庫-shared-與--fpic)〈為什麼一定要 `-fPIC`〉；課後討論在[第九節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#九課後討論)第 1～5 章 | 不加 `-fPIC` 時 `recompile with -fPIC`、GOT 間接存取、符號插入（主程式的 `counter` 贏）、`static` 變數不用 `-fPIC`、`callbump.c` 用 gdb 看 GOT 那格載入後裝 `&counter`（10-25 追問）、`tally.c` 是自我測驗第 6 題、`conf.c`／`app.c` 是第 7 題（10-25 追問「接 `.so` 為什麼不算重複定義」）、`greet.c`／`greet_main.c` 看 PLT 跳板與 `.got.plt` 的延遲綁定（10-26 追問）、`logger.c`／`runner.c` 是第 8 題；`usecount2.c` 只寫 `extern int counter;` 看複製重定位、`count_v2.c` 把 `counter` 改成 `long` 看大小不符的警告（第九節第 3 章，10-26 追問）、`cfg.c`／`setlevel.c` 是第 11 題 |
| [`examples/share/`](./examples/share/) | [`a.c`](./examples/share/a.c)、[`b.c`](./examples/share/b.c)、[`hits.c`](./examples/share/hits.c)、[`x.c`](./examples/share/x.c)、[`y.c`](./examples/share/y.c)、[`slot.h`](./examples/share/slot.h)、[`q.c`](./examples/share/q.c)、[`r.c`](./examples/share/r.c)、[`pfn.c`](./examples/share/pfn.c)、[`shape.c`](./examples/share/shape.c)、[`m.c`](./examples/share/m.c)、[`n.c`](./examples/share/n.c) | [第九節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#九課後討論)第 6～9 章 | `a`、`b` 同時跑、連 `../pic/count.c` 做的 `libcount.so`，用 `/proc/<pid>/smaps` 看 `.text` 是 `Shared_Clean`、GOT 與 `counter` 是 `Private_Dirty`（寫入時複製，10-26 追問）；`hits.c`／`x.c`／`y.c` 是自我測驗第 9 題；`q.c`／`r.c`（引入 `slot.h`，連 `../pic/greet.c` 做的 `libgreet.so`）讀出各自 `.got.plt` 的 `say` 那格、`Pss` 證明跳板共用；`pfn.c` 讀 `/proc/self/pagemap` 看到 PFN 被遮成 0；`shape.c`／`m.c`／`n.c` 是第 10 題 |
| [`examples/cowpage/`](./examples/cowpage/) | [`box.c`](./examples/cowpage/box.c)、[`cow.c`](./examples/cowpage/cow.c) | [第九節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#九課後討論)第 16 章 | `box.c` 做成 `libbox.so`（`v` 對齊到獨佔一頁），`cow.c` 用 `/proc/self/pagemap` 第 61、63 位元看 `.text`、`.got`、`v` 三頁：GOT 進 `main` 前就被複製、讀 `v` 不複製、寫 `v` 才複製且虛擬位址不變（10-30 追問） |
| [`examples/fflush/`](./examples/fflush/) | [`nf.c`](./examples/fflush/nf.c)、[`wf.c`](./examples/fflush/wf.c) | [第九節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#九課後討論)第 14 章 | `printf` 先寫進緩衝區：導向檔案時沒 `fflush` 要到 `exit()` 才 `write`、被 `kill` 就消失；`strace` 看 `write` 與 `clock_nanosleep` 的先後（10-29 追問）|
| [`examples/asneeded/`](./examples/asneeded/) | [`case.c`](./examples/asneeded/case.c)、[`space.c`](./examples/asneeded/space.c)、[`trim.c`](./examples/asneeded/trim.c)、[`main.c`](./examples/asneeded/main.c)、[`none.c`](./examples/asneeded/none.c) | [第九節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#九課後討論)第 15 章 | 讀到 `-ltxt` 時清單裡只有啟動檔帶進來的符號；`--as-needed` 丟掉此刻沒人要的 `.so`、`--no-as-needed` 一律留下（`none.c` 也記進 `NEEDED`）；`.a` 加 `--no-as-needed` 沒用（10-29 追問）|
| [`examples/headers/`](./examples/headers/) | [`mydecl.c`](./examples/headers/mydecl.c)、[`noinc.c`](./examples/headers/noinc.c)、[`needfile.c`](./examples/headers/needfile.c)、[`sq.c`](./examples/headers/sq.c)、[`temp/include/temp.h`](./examples/headers/temp/include/temp.h)、[`temp/temp.c`](./examples/headers/temp/temp.c)、[`temp/main.c`](./examples/headers/temp/main.c) | [第九節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#九課後討論)第 11 章 | 自己寫 `printf` 宣告就能用 `libc.so.6`、不寫宣告只是警告、`FILE`／`NULL` 一定要 `stdio.h`、`#include <math.h>` 沒有 `-lm` 照樣 `undefined reference`（10-26 追問）；`temp/` 是自我測驗第 12 題 |
| [`examples/search/`](./examples/search/) | [`calc.h`](./examples/search/calc.h)、[`mul.c`](./examples/search/mul.c)、[`main.c`](./examples/search/main.c)、[`v1/add.c`](./examples/search/v1/add.c)、[`v2/add.c`](./examples/search/v2/add.c) | [第五節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#五執行時怎麼找到-so)〈`RPATH` 和 `RUNPATH` 差在哪〉、[第六節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#六換掉-so-不用重新連結以及-soname)〈換掉 `.so`〉 | `RUNPATH` 讓 `LD_LIBRARY_PATH` 優先、`RPATH` 不讓；換掉 `.so` 不用重新連結 |
| [`examples/order/`](./examples/order/) | [`temp.h`](./examples/order/temp.h)、[`ctof.c`](./examples/order/ctof.c)、[`ftoc.c`](./examples/order/ftoc.c)、[`limit.c`](./examples/order/limit.c)、[`round.c`](./examples/order/round.c)、[`main.c`](./examples/order/main.c) | [第九節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#九課後討論)第 12 章 | 作業第 2 題批改的補教：`.so` 整份符號一次進來、`--as-needed` 丟掉 `libround.so`、`--no-as-needed` 與 `--allow-shlib-undefined` 的對照 |
| [`examples/soname/`](./examples/soname/) | [`calc.h`](./examples/soname/calc.h)、[`add.c`](./examples/soname/add.c)、[`mul.c`](./examples/soname/mul.c)、[`main.c`](./examples/soname/main.c) | 第六節〈SONAME〉 | `-Wl,-soname`、三個名字與符號連結、`NEEDED` 記的是 SONAME |
| [`examples/rpathlink/`](./examples/rpathlink/) | [`temp.h`](./examples/rpathlink/temp.h)、[`round.c`](./examples/rpathlink/round.c)、[`ctof.c`](./examples/rpathlink/ctof.c)、[`ftoc.c`](./examples/rpathlink/ftoc.c)、[`main.c`](./examples/rpathlink/main.c) | [第九節第 13 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#13-連結時-ld-為什麼要找依賴的依賴-rpath-link-與--rpath作業-4-3) | 連結時 `ld` 找「依賴的依賴」：`-rpath-link` vs `-rpath`、`--allow-shlib-undefined`、`.so` 自己的 `RUNPATH` |

---

## 批改結果（2026-10-27）

第 1、2 題 2026-10-27 批改；第 1～5 題全部批改完（第 1、2 題 2026-10-27，其餘 2026-10-28）。

| 題目 | 結果 | 錯在哪／缺什麼 |
|---|---|---|
| 1-1 | 對 | 指令、輸出全部正確（我用你的檔案重做一次，結果一樣）；抓到索引是「符號 → 在哪個成員」的表、`ld` 靠它決定拉哪個成員；沒有索引時 `ld` 拒絕、`ranlib` 補上後就能連結 |
| 1-2 | 對 | 預測對；抓到 `ftoc.o` 被拉進來後帶來新的缺口 `temp_round`，所以 `round.o` 也被拉進來；沒有人要 `c_to_f`，`ctof.o` 就不拉。用詞小提醒：「`f_to_c` 與 `temp_round` 連結」要說成「`ftoc.o` 裡有未定義的 `temp_round`（`nm` 的 `U`）」 |
| 2-1 | 對 | 三個指令的輸出都對，(b) 預測錯但執行後自己找出漏了 `temp_clamp`；三個都用「還缺什麼、往右找」解釋到位。用詞提醒同 1-2：要講拉的是哪個**成員**（`ftoc.o`、`round.o`、`limit.o`），以及 (b) 第一次掃 `libconv.a` 時 `limit.o` 沒被拉，是因為那時還沒人缺 `temp_clamp` |
| 2-2 | 對 | 群組指令正確，(a)(b) 兩種順序都能連結、`./hw2` 印 `f_to_c(100) = 38` |
| 2-3 | 大致對 | 結果都對（(a) 失敗、(b) 成功並執行）。(a) 抓到「掃到 `libround` 時沒人要它，就被丟掉」，但沒講出原因是 Ubuntu 預設的 `--as-needed`（筆記第四節），而且丟掉的是 `NEEDED` 名單、不是「記憶體」。(b) 的「`libconv` 整份留著，後面還找得到」是新觀念的猜測，方向對，已補教：`.so` 整份符號一次進來，不是拉成員 |
| 3-1 | 不完整 | 輸出對、`-fPIE` 直接用 `%rip` 加距離讀 `temp_calls`、要經 GOT 的理由（變數最後可能是別人那份）都抓到了；「為什麼 `ctof.o`、`ftoc.o` 沒被點名」（最後一段）只說「出問題的是 `temp_round`」，沒解釋它們同樣參照別的檔案的 `temp_round` 為什麼沒事：它們是**呼叫函式**，重定位是 `R_X86_64_PLT32`，`ld` 讓 `call` 改指向自己做的 PLT 跳板，不必改程式碼。另有兩處錯：`.so` 載入位址不固定（ASLR），問題不在這；警告說的是「要填的洞在唯讀的 `.text`」，不是「找到了 `.text`」。（10-28 使用者說明最後一段就是在答這一問，批改描述已更正，結果維持不完整） |
| 3-2 | 不完整 | 讀的那一半對（`b` 從 GOT 讀位址、`12` 讀值、`14` 加 1）；漏了寫回的 `17`、`1e`（再讀一次 GOT、照位址寫回），說成「下面都是計算」；`readelf -r` 那列只描述 GOT 的結果，沒講出是留給 `ld.so`、載入時把 `temp_calls` 真正的位址寫進 `0x3fd8` |
| 3-3 | 對 | 抓到：`ld.so` 印的錯、`-L` 只給 `ld`、(a)(b) 兩種方法與 `RUNPATH [$ORIGIN]`、`temp_calls = 2` 是兩個函式各呼叫一次（可補：`NEEDED` 只記名字不記路徑） |
| 3-4 | 大致對 | 動態 1002、`LD_DEBUG` 綁到 `./hw3`、執行檔先找；靜態 `round.o` 被拉進來撞名都對；缺「`.so` 不合併進執行檔，`ld` 不把它的定義算重複、留給 `ld.so` 選」（自己也寫了不確定，第九節第 2 章教過） |
| 4-1 | 大致對 | 抓到：找不到的是 `libround.so`、它是 `libtemp.so` 的依賴、`libtemp.so` 沒有 `RUNPATH`；缺「`RUNPATH` 只管自己的 `NEEDED`」的規則與「同資料夾不會自動找」，`LD_DEBUG` 寫成沒有輸出（實際有，正是證據） |
| 4-2 | 大致對 | 兩種方法的指令都對（含 `--disable-new-dtags`）、選 (a) 理由對；缺 (b) 為什麼能修好：`RPATH` 套用到整棵依賴樹、`RUNPATH` 只管自己的 `NEEDED`（自己寫了不清楚，第五節第 3 步教過）；沒貼 `readelf -d` |
| 4-3 | 大致對 | 輸出對、注意到變成連結時就出錯；「`ld` 替 `libtemp.so` 找它的依賴」是新觀念，猜對方向（補教寫進筆記第九節第 13 章）；`-rpath` 只說「執行檔的依賴位置」，缺核心：`-rpath-link` 只給連結時用、不寫進輸出檔，`-rpath` 寫進輸出檔給執行時的 `ld.so` |
| 5-1 | 對 | 抓到：兩支都 37、`NEEDED` 寫 SONAME `libtemp.so.1`（可補：`ld` 打開的是 `libtemp.so`，寫進去的是檔案裡的 SONAME） |
| 5-2 | 不完整 | 預測與輸出都對、`hw5_static` 要重新連結；缺「為什麼不同」：靜態是連結時把 `round.o` 複製進執行檔、動態是執行時才載入 `libtemp.so.1`（現在指向 `1.0.1`）；缺 `ar t` 仍是 3 個的理由：`r` 遇到同名就取代 |
| 5-3 | 對 | 抓到：38／377、兩支 `NEEDED` 不同所以各載各的、偷懶時印 377（可補：換 SONAME 才是不同檔名可並存；`ld.so` 只比名字，偷懶會靜靜地算錯） |

## 1. 做一個靜態函式庫，看連結器拉了哪些成員

在 `hw1/` 裡建下面五個檔案。這是一個溫度換算函式庫：溫度先算成「十分之一度」的整數，再由 `temp_round` 四捨五入成整數度。

`temp.h`：

```c
#ifndef TEMP_H
#define TEMP_H

int c_to_f(int c);
int f_to_c(int f);
int temp_round(int x10);

#endif
```

`ctof.c`：

```c
#include "temp.h"

int c_to_f(int c)
{
    return temp_round(c * 18 + 320);
}
```

`ftoc.c`：

```c
#include "temp.h"

int f_to_c(int f)
{
    return temp_round((f - 32) * 100 / 18);
}
```

`round.c`：

```c
#include "temp.h"

int temp_round(int x10)
{
    return (x10 + 5) / 10;
}
```

`hw1_main.c`（**只呼叫 `f_to_c`**）：

```c
#include <stdio.h>
#include "temp.h"

int main(void)
{
    printf("f_to_c(212) = %d\n", f_to_c(212));
    return 0;
}
```

把 `ctof.c`、`ftoc.c`、`round.c` 各自 `gcc -Wall -c` 成 `.o`，用 `ar rcs` 包成 `libtemp.a`，再用 `-L. -ltemp` 連結成 `hw1`。

### 1-1. 貼上你建立 `libtemp.a` 與連結 `hw1` 的指令、`./hw1` 的輸出、`ar t libtemp.a` 與 `nm -s libtemp.a` 的輸出。`nm -s` 印出的 `Archive index` 是什麼、誰會用到它？再用 `ar rcS libnoidx.a ctof.o ftoc.o round.o` 做一個沒有索引的版本、用 `-L. -lnoidx` 連結，貼上結果，並用 `ranlib` 修好它（貼上修好後的連結與執行結果）。【筆記[第二節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#二靜態函式庫用-ar-把-o-包成-a)〈實驗：做出 `libcalc.a`〉】

**第 1 次（2026-10-27）**
```
gcc -Wall -Wextra -c ctof.c ftoc.c round.c
ar rcs libtemp.a ctof.o ftoc.o round.o
gcc -Wall -Wextra hw1_main.c -L. -ltemp -o hw1

f_to_c(212) = 100

ar t libtemp.a
ctof.o
ftoc.o
round.o

nm -s libtemp.a
Archive index:
c_to_f in ctof.o
f_to_c in ftoc.o
temp_round in round.o

ctof.o:
0000000000000000 T c_to_f
                 U temp_round

ftoc.o:
0000000000000000 T f_to_c
                 U temp_round

round.o:
0000000000000000 T temp_round

archive index就是一個符號的索引,連接的時候ld會根據這個索引決定要把命中索引的object file加入到執行檔

gcc -Wall -Wextra hw1_main.c -L. -lnoidx -o hw1noidx
/usr/bin/ld: ./libnoidx.a: error adding symbols: archive has no index; run ranlib to add one
collect2: error: ld returned 1 exit status

ranlib libnoidx.a

nm -s libnoidx.a
Archive index:
c_to_f in ctof.o
f_to_c in ftoc.o
temp_round in round.o

ctof.o:
0000000000000000 T c_to_f
                 U temp_round

ftoc.o:
0000000000000000 T f_to_c
                 U temp_round

round.o:
0000000000000000 T temp_round

gcc -Wall -Wextra hw1_main.c -L. -lnoidx -o hw1noidx

./hw1noidx
f_to_c(212) = 100
```

> **批改**：**對**。我在 scratchpad 用你的五個檔案照樣重做：`gcc -Wall -c` 沒有警告，`./hw1` 印 `f_to_c(212) = 100`；`ar t`、`nm -s` 的輸出，`libnoidx.a` 的 `archive has no index; run ranlib to add one`，以及 `ranlib` 之後能連結、能執行，全都跟你貼的一樣。
> 「索引就是符號的索引，`ld` 根據它決定要把哪個 object file 加進執行檔」：索引是什麼、誰用它，兩點都答到了。可以再精確一點：索引的每一列是「符號 → 定義它的成員」（`f_to_c in ftoc.o`），`ld` 拿手上「還缺的符號」去查這張表，就知道要拉哪個成員，不必把每個成員的符號表讀一遍。
> 小建議：`ranlib` 前也可以先跑一次 `nm -s libnoidx.a`，看到開頭**沒有** `Archive index:`，前後對照會更清楚（題目沒要求，不扣分）。
>
> **參考答案**【筆記[第二節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#二靜態函式庫用-ar-把-o-包成-a)〈[實驗：做出 `libcalc.a`](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#實驗做出-libcalca)〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 指令與輸出：`gcc -Wall -c` 三個 `.c` → `ar rcs libtemp.a ctof.o ftoc.o round.o` → `gcc -Wall hw1_main.c -L. -ltemp -o hw1`，`./hw1` 印 `f_to_c(212) = 100`；`ar t` 列出三個成員。
> 2. `Archive index` 是 `ar` 的 `s` 建的一張表：「哪個符號定義在哪個成員裡」。
> 3. 用它的是連結器 `ld`：查這張表就知道缺的符號要從哪個成員拉，不用逐一讀每個成員的符號表。
> 4. 用 `S` 做出沒有索引的 `.a`，GNU `ld` 直接拒絕（`archive has no index; run ranlib to add one`）；`ranlib libnoidx.a`（等於 `ar s`）補上索引後就能連結、執行。
>
> **詳細說明**：`.a` 只是把幾個 `.o` 接在一起的封存檔，跟 `tar` 是同一類東西，`ar` 不會重新編譯、也不會連結。索引放在封存檔開頭，所以 `nm -s` 先印 `Archive index:`，再逐個成員印各自的符號表；`U temp_round` 那兩列表示 `ctof.o`、`ftoc.o` 都**需要** `temp_round`，但這個符號不是它們定義的，所以索引裡只有 `temp_round in round.o`。索引只收「有定義」的符號（`T`），沒收 `U`，因為它要回答的是「這個符號去哪裡拿」。

### 1-2. 先預測：`nm hw1 | grep -E 'c_to_f|f_to_c|temp_round'` 會列出這三個函式裡的哪幾個？寫下預測後再執行，貼上結果，並貼上 `gcc -Wall hw1_main.c -L. -ltemp -Wl,--verbose -o hw1 2>&1 | grep libtemp` 的輸出。`hw1_main.c` 沒有直接呼叫 `temp_round`，它為什麼（或為什麼沒有）在執行檔裡？`c_to_f` 呢？【筆記第二節〈連結：只拉需要的成員〉】

**第 1 次（2026-10-27）**
```
我預測有:f_to_c, temp_round

nm hw1 | grep -E 'c_to_f|f_to_c|temp_round'
0000000000001178 T f_to_c
00000000000011b1 T temp_round

gcc -Wall hw1_main.c -L. -ltemp -Wl,--verbose -o hw1 2>&1 | grep libtemp
attempt to open ./libtemp.so failed
attempt to open ./libtemp.a succeeded
./libtemp.a
(./libtemp.a)ftoc.o
(./libtemp.a)round.o

他雖然沒有直接與temp_round連結,但是有與f_to_c連接,而f_to_c又與temp_round連結,所以最後還是是出現在了執行檔裏

main一開始的符號就沒有他了,所以也不會去把ctof.o拉入執行檔,自然不會有這個符號
```

> **批改**：**對**。預測 `f_to_c`、`temp_round` 正確，`nm` 和 `--verbose` 的輸出我重做後也一樣：只拉了 `ftoc.o`、`round.o`。
> 兩個理由的主線都對：`temp_round` 是被 `f_to_c` 間接帶進來的；`main` 沒用到 `c_to_f`，所以 `ctof.o` 不拉。
> 用詞要改準一點：「`f_to_c` 與 `temp_round` 連結」講得不清楚，`ld` 看的不是「誰跟誰連結」，而是**還缺哪些符號**。精確的說法是：`ftoc.o` 被拉進來以後，它自己的符號表裡有 `U temp_round`（你在 1-1 的 `nm -s` 看過），於是 `temp_round` 被加進「還缺」的清單，`ld` 查索引找到 `round.o`，也把它拉進來。
>
> **參考答案**【筆記[第二節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#二靜態函式庫用-ar-把-o-包成-a)〈[連結：只拉需要的成員](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#連結只拉需要的成員)〉、〈[順序：`ld` 由左往右只掃一次](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#順序ld-由左往右只掃一次)〉的「還沒找到定義的符號」清單】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 執行檔裡有 `f_to_c`、`temp_round`，沒有 `c_to_f`；`--verbose` 證明 `libtemp.a` 裡只拉了 `ftoc.o`、`round.o`。
> 2. `temp_round` 在執行檔裡：`main` 缺 `f_to_c` → 拉進 `ftoc.o`；`ftoc.o` 自己有未定義的 `temp_round`，成了新的缺口 → 再拉進 `round.o`。
> 3. `c_to_f` 不在：沒有任何已拉進來的檔案需要它，`ctof.o` 就不拉（封存檔的成員是有人需要才拉，拉的單位是整個 `.o`）。
>
> **詳細說明**：
> - 逐步看 `ld` 手上的「還缺」清單：讀 `hw1_main.c` 編出的 `.o` → 清單 `{f_to_c, printf}`；掃到 `libtemp.a`，查索引得到 `f_to_c in ftoc.o` → 拉 `ftoc.o`，`f_to_c` 解決，但多了 `temp_round`；查索引得到 `temp_round in round.o` → 拉 `round.o`。`c_to_f` 從頭到尾都沒進過清單。`printf` 最後由 libc 解決。
> - `ctof.o` 也有 `U temp_round`，但那不重要：成員自己「需要什麼」只有在它被拉進來之後才算數。`ctof.o` 沒被拉進來，它的需求就不存在。
> - 補充（筆記沒寫，我實測過）：**同一個 `.a` 裡面**，`ld` 會反覆查索引，直到再也拉不出新成員才離開，所以成員在 `.a` 裡的順序沒有影響。我用 `ar rcs librev.a round.o ctof.o ftoc.o` 把 `round.o` 放在最前面，連結照樣成功，`--verbose` 一樣印出 `(./librev.a)ftoc.o`、`(./librev.a)round.o`。筆記講的「由左往右只掃一次、不回頭」，指的是命令列上**不同的檔案**之間；第 2 題會碰到兩個 `.a` 互相需要的情況。

---

## 2. 兩個互相需要的靜態函式庫：順序怎麼排

在 `hw2/` 裡，把函式庫拆成兩個：`libconv.a` 放換算函式，`libround.a` 放四捨五入。這次 `temp_round` 會先呼叫 `temp_clamp`，把低於絕對零度（−273.2 度，也就是 −2732 個十分之一度）的值夾住，而 `temp_clamp` 放在 **`libconv.a`** 裡。

`temp.h`：

```c
#ifndef TEMP_H
#define TEMP_H

int c_to_f(int c);
int f_to_c(int f);
int temp_round(int x10);
int temp_clamp(int x10);

#endif
```

`ctof.c`、`ftoc.c`：跟第 1 題一樣（照第 1 題的內容再建一次）。

`limit.c`：

```c
#include "temp.h"

int temp_clamp(int x10)
{
    return x10 < -2732 ? -2732 : x10;
}
```

`round.c`（跟第 1 題不同：多了一行呼叫 `temp_clamp`）：

```c
#include "temp.h"

int temp_round(int x10)
{
    x10 = temp_clamp(x10);
    return (x10 + 5) / 10;
}
```

`hw2_main.c`：

```c
#include <stdio.h>
#include "temp.h"

int main(void)
{
    printf("f_to_c(100) = %d\n", f_to_c(100));
    return 0;
}
```

建立兩個函式庫：

```sh
gcc -Wall -c ctof.c ftoc.c limit.c round.c
ar rcs libconv.a ctof.o ftoc.o limit.o
ar rcs libround.a round.o
```

### 2-1. 先預測下面三個連結指令，哪些會成功、失敗的會說哪個符號找不到（是在哪個檔案裡需要它）？寫下預測後再逐一執行，貼上每一個的完整輸出（成功的要執行 `./hw2` 並貼上結果）。用「`ld` 由左往右掃，手上有一張還缺哪些符號的清單」的方式，**逐步**說明每個指令掃到每個檔案時拉了哪些成員、清單怎麼變。【筆記第二節〈順序：`ld` 由左往右只掃一次〉】

```sh
gcc -Wall hw2_main.c -L. -lround -lconv -o hw2           # (a)
gcc -Wall hw2_main.c -L. -lconv -lround -o hw2           # (b)
gcc -Wall hw2_main.c -L. -lconv -lround -lconv -o hw2    # (c)
```

**第 1 次（2026-10-27）**
```
(a) 預測:找不到
gcc -Wall hw2_main.c -L. -lround -lconv -o hw2
/usr/bin/ld: ./libconv.a(ftoc.o): in function `f_to_c':
ftoc.c:(.text+0x33): undefined reference to `temp_round'
collect2: error: ld returned 1 exit status

hw2 想要連 f_to_c 但是在libround找不到這個符號,接着去找libconv,找到了,可是f_to_c要temp_round, 但是又找不到了

(b) 預測: 可以成功
gcc -Wall hw2_main.c -L. -lconv -lround -o hw2
/usr/bin/ld: ./libround.a(round.o): in function `temp_round':
round.c:(.text+0x15): undefined reference to `temp_clamp'
collect2: error: ld returned 1 exit status

我的預測錯誤,我漏考慮了temp_clamp
hw2 在libconv連到f_to_c, f_to_c 在 libround 連到了temp_round, 但temp_round需要temp_clamp 往右沒庫了連不到

(c) 預測:可以成功
gcc -Wall hw2_main.c -L. -lconv -lround -lconv -o hw2
./hw2
f_to_c(100) = 38
hw2 在libconv連到f_to_c, f_to_c 在 libround 連到了temp_round, temp_round需要temp_clamp, 在libconv 找到了temp_clamp
成功
```

> **批改**：**對**。我在 scratchpad 用你的檔案重做，三個指令的輸出跟你貼的一模一樣；(c) 加 `-Wl,--verbose` 看到依序拉了 `(./libconv.a)ftoc.o`、`(./libround.a)round.o`、`(./libconv.a)limit.o`，正好是你描述的三步。
> (b) 預測錯了，但你執行後自己找出「漏考慮了 `temp_clamp`」，這正是「先預測」要練的東西。三個指令的推理主線都對：每一步都講出「現在缺什麼 → 往右找 → 找到後又多缺什麼」，也講出右邊沒東西就失敗（不回頭）。
> 用詞提醒（跟 1-2 一樣）：你寫「在 libconv 連到 f_to_c」，精確的說法是「`ld` 查 `libconv.a` 的索引，拉進成員 `ftoc.o`」。拉的單位是**整個 `.o`**，這點在 (b) 很關鍵：第一次掃 `libconv.a` 時，`limit.o` 就在同一個 `.a` 裡，但那時清單上還沒有 `temp_clamp`，所以 `limit.o` 不拉；等 `round.o` 帶來 `temp_clamp` 的缺口時，`libconv.a` 已經掃過了。錯誤訊息 `./libconv.a(ftoc.o): in function 'f_to_c'` 括號裡的就是成員名，可以直接拿來對照。
>
> **參考答案**【筆記[第二節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#二靜態函式庫用-ar-把-o-包成-a)〈[順序：`ld` 由左往右只掃一次](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#順序ld-由左往右只掃一次)〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. (a) 失敗：掃 `libround.a` 時清單只有 `f_to_c`（和 `printf`），`round.o` 沒人要、不拉；掃 `libconv.a` 拉 `ftoc.o`，清單多了 `temp_round`；右邊沒有檔案了 → `ftoc.o` 裡的 `temp_round` 是 `undefined reference`。
> 2. (b) 失敗：掃 `libconv.a` 只拉 `ftoc.o`（`limit.o` 這時沒人要）；掃 `libround.a` 拉 `round.o`，清單多了 `temp_clamp`；`libconv.a` 已經掃過、不回頭 → `round.o` 裡的 `temp_clamp` 是 `undefined reference`。
> 3. (c) 成功：前兩步同 (b)，第三個 `-lconv` 再掃一次 `libconv.a`，這時清單上有 `temp_clamp` → 拉 `limit.o`，清單清空；`./hw2` 印 `f_to_c(100) = 38`。
>
> **詳細說明**：
> - 清單的變化寫成表（`printf` 最後由 libc 解決，省略）：
>
>   | 指令 | 讀 `hw2_main.o` | 第 1 個 `-l` | 第 2 個 `-l` | 第 3 個 `-l` | 結果 |
>   |---|---|---|---|---|---|
>   | (a) `-lround -lconv` | `{f_to_c}` | `libround.a`：不拉，`{f_to_c}` | `libconv.a`：拉 `ftoc.o`，`{temp_round}` | — | 缺 `temp_round` |
>   | (b) `-lconv -lround` | `{f_to_c}` | `libconv.a`：拉 `ftoc.o`，`{temp_round}` | `libround.a`：拉 `round.o`，`{temp_clamp}` | — | 缺 `temp_clamp` |
>   | (c) `-lconv -lround -lconv` | `{f_to_c}` | 同 (b) | 同 (b) | `libconv.a`：拉 `limit.o`，`{}` | 成功 |
>
> - 這就是筆記講的「互相需要」：`libconv.a` 的 `ftoc.o` 需要 `libround.a`，`libround.a` 的 `round.o` 又需要 `libconv.a` 裡**另一個還沒拉進來的成員** `limit.o`，所以只掃一次怎麼排都不夠。
> - 錯誤訊息報的是「誰需要它」：(a) 是 `./libconv.a(ftoc.o): in function 'f_to_c'`，(b) 是 `./libround.a(round.o): in function 'temp_round'`。看括號裡的成員名，就知道缺口是哪一步產生的。

### 2-2. 不重複寫 `-lconv`，改用 `-Wl,--start-group … -Wl,--end-group` 讓 (b) 的順序也能連結成功。貼上指令與執行結果。【筆記第二節〈順序〉最後一段】

**第 1 次（2026-10-27）**
```
gcc -Wall hw2_main.c -L. -Wl,--start-group -lround -lconv -Wl,--end-group -o hw2
gcc -Wall hw2_main.c -L. -Wl,--start-group -lconv -lround -Wl,--end-group -o hw2
不管(a),(b)都可以成功
./hw2
f_to_c(100) = 38  
```

> **批改**：**對**。群組指令寫法正確，我重做也一樣能連結、`./hw2` 印 `f_to_c(100) = 38`；你另外試了 (a) 的順序也成功，比題目多做了一步。
> 加 `-Wl,--verbose` 可以看到群組在做什麼：(b) 的順序依序拉了 `(./libconv.a)ftoc.o`、`(./libround.a)round.o`，接著**回頭**再掃 `./libconv.a` 拉了 `limit.o`，最後再掃一次 `./libround.a`，什麼都沒拉，確認沒有新的缺口才結束。
>
> **參考答案**【筆記[第二節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#二靜態函式庫用-ar-把-o-包成-a)〈[順序：`ld` 由左往右只掃一次](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#順序ld-由左往右只掃一次)〉最後一段】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 指令：`gcc -Wall hw2_main.c -L. -Wl,--start-group -lconv -lround -Wl,--end-group -o hw2`，`./hw2` 印 `f_to_c(100) = 38`。
>
> **詳細說明**：`--start-group`／`--end-group` 是 `ld` 的選項，透過 gcc 要用 `-Wl,` 傳。群組裡的封存檔會**反覆掃**，直到一整輪都沒有新的符號被解決，所以群組內的順序不重要。代價是比較慢（要多掃幾輪），所以只在真的互相依賴時才用；大部分情況把 `-l` 排對順序就好。

### 2-3. 先猜再實驗：把兩個函式庫改做成 `.so`（指令如下），再用 (a)、(b) 兩種順序連結。各會成功還是失敗？貼上結果（成功的要執行）。為什麼 `.so` 版的 (b) 不必像 `.a` 那樣把 `-lconv` 寫兩次？(a) 為什麼還是失敗（或為什麼成功）？【新觀念，先猜；(a) 的部分可以用筆記[第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#四動態函式庫-shared-與--fpic)〈`.so` 在 Ubuntu 上也有順序問題〉推】

```sh
mkdir -p so
gcc -Wall -fPIC -shared ctof.c ftoc.c limit.c -o so/libconv.so
gcc -Wall -fPIC -shared round.c -o so/libround.so
gcc -Wall hw2_main.c -Lso -lround -lconv -Wl,-rpath,'$ORIGIN/so' -o hw2so   # (a)
gcc -Wall hw2_main.c -Lso -lconv -lround -Wl,-rpath,'$ORIGIN/so' -o hw2so   # (b)
```

**第 1 次（2026-10-27）**
```
我預測都會成功
(a)
gcc -Wall hw2_main.c -L. -lround -lconv -Wl,--verbose,-rpath,'$ORIGIN' -o hw2so | grep -E 'libconv|libround'
attempt to open ./libround.so succeeded
./libround.so
attempt to open ./libconv.so succeeded
./libconv.so
/usr/bin/ld: ./libconv.so: undefined reference to `temp_round'
/usr/bin/ld: link errors found, deleting executable `hw2so'
collect2: error: ld returned 1 exit status

我十分意外a會失敗,看來即使是動態庫也是按順序找

(b)
gcc -Wall hw2_main.c -L. -lconv -lround -Wl,--verbose,-rpath,'$ORIGIN' -o hw2so | grep -E 'libconv|libround'
attempt to open ./libconv.so succeeded
./libconv.so
attempt to open ./libround.so succeeded
./libround.so

./hw2so
f_to_c(100) = 38

b的順序成功了

我猜測因爲a 一開始找了libround, main沒有用到round,接下來他去找libconv, 找到了加到記憶體, 又要回去找libround,但他不再記憶體,所以就沒了

可是a 一開始就找了libconv,存到了記憶體,在找round ,然後再從記憶體在去找libconv,我們不用重寫兩次
```

> **批改**：**大致對**。結果都對：(a) 失敗在 `libconv.so: undefined reference to 'temp_round'`，(b) 成功且執行印 `f_to_c(100) = 38`。我照題目的指令（放在 `so/`、`-Lso`）重做，結果一樣。你改成放在同一個資料夾用 `-L.` 也可以：同一個資料夾有 `libconv.so` 和 `libconv.a` 時，`ld` 先找 `.so`（筆記第三節），你的 `--verbose` 也印了 `attempt to open ./libround.so succeeded`。最後一句的「可是 a 一開始就找了 libconv」應該是 **b**，筆誤。
> - **(a) 為什麼失敗**（這部分筆記第四節教過）：你抓到了主線「掃到 `libround` 時 `main` 沒用到它，它就沒了」，但**沒講出原因是 `--as-needed`**——Ubuntu 的 gcc 預設會傳給 `ld`，意思是「`.so` 只有在當下有人需要它的符號時才寫進 `NEEDED`」。另外「不在記憶體」用詞不對：這是**連結時** `ld` 在決定執行檔要記哪些 `NEEDED`，程式還沒跑，跟記憶體無關。證據：加 `-Wl,--no-as-needed` 後 (a) 的順序就能連結，`NEEDED` 依序是 `libround.so`、`libconv.so`，`./hw2so_a` 照樣印 38。
> - **(b) 為什麼不必寫兩次**（新觀念，猜錯不算錯）：你猜「`libconv` 整份留著，後面還從它找得到」，方向是對的。精確的說法：`.so` 不是「拉成員」，`ld` 讀到一個 `.so` 時，把它**整份動態符號表**（`nm -D` 看到的全部 `T`）一次記下來；之後 `libround.so` 帶來的 `U temp_clamp`，`ld` 拿去跟所有已經讀過的 `.so` 比，`libconv.so` 早就提供了。真正執行時，`ld.so` 也是在所有載入的 `.so` 裡找（`LD_DEBUG=bindings` 印出 `libround.so ... to .../libconv.so ...: normal symbol 'temp_clamp'`）。
> - 補教：(a) 那行錯誤是 `ld` 在檢查「`.so` 裡未定義的符號找不找得到」，做執行檔時預設會檢查（`--no-allow-shlib-undefined`）。完整的實驗與說明寫進筆記第九節〈課後討論〉[第 12 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#12-so-之間為什麼順序幾乎不重要a-為什麼還是失敗作業-2-3)。
>
> **參考答案**【(a)：筆記[第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#四動態函式庫-shared-與--fpic)〈[`.so` 在 Ubuntu 上也有順序問題](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#so-在-ubuntu-上也有順序問題)〉；(b) 與補教：筆記第九節〈課後討論〉[第 12 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#12-so-之間為什麼順序幾乎不重要a-為什麼還是失敗作業-2-3)】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 結果：(a) 失敗（`so/libconv.so: undefined reference to 'temp_round'`），(b) 成功，`./hw2so` 印 `f_to_c(100) = 38`。
> 2. (a) 失敗的原因是 Ubuntu gcc 預設的 `--as-needed`：掃到 `libround.so` 時還沒有人需要 `temp_round`，它就不寫進 `NEEDED`（被丟掉）；之後 `libconv.so` 需要 `temp_round` 時已經沒有人提供。
> 3. (b) 不必寫兩次：`.so` 是整份進來，不是只拉需要的成員；`libconv.so` 一讀進來，它所有的符號（包括 `temp_clamp`）就都記下了，`libround.so` 後來需要 `temp_clamp` 時照樣找得到。
>
> **詳細說明**：
> - 為什麼 `.a` 要寫兩次、`.so` 不用：`.a` 是「當下缺什麼才拉哪個 `.o`」，沒拉的成員（`limit.o`）之後就看不到了；`.so` 本身是一個連結好的整體，沒有「成員」可以只拉一部分，讀到它時它的所有符號都進了 `ld` 的符號表。
> - 所以 `.so` 之間的順序，只剩 `--as-needed` 一個影響：被依賴的 `.so` 要寫在需要它的檔案（`.o` 或另一個 `.so`）後面，否則讀到它時沒人需要、會被丟掉。這跟 `.a` 的規則（被依賴的寫後面）方向一樣，但原因不同。
> - 實驗佐證（筆記第 12 章有完整指令）：`-Wl,--no-as-needed` 讓 (a) 成功；`-Wl,--allow-shlib-undefined` 讓 (a) **連結**成功，但 `NEEDED` 只有 `libconv.so`，執行時 `ld.so` 報 `symbol lookup error: .../libconv.so: undefined symbol: temp_round`——連結時那道檢查，就是在防這種「到執行才爆」。
> - 真正的專案通常會在做 `libconv.so` 時就加 `-L... -lround`，讓 `libconv.so` 自己的 `NEEDED` 記著 `libround.so`；這題故意沒加，才看得到這些現象（`readelf -d so/libconv.so` 沒有 `libround.so`）。

---

## 3. 做一個 `.so`：`-fPIC`、GOT、讓執行檔找得到它

在 `hw3/` 裡。這次函式庫多了一個全域變數 `temp_calls`，記錄 `temp_round` 被呼叫了幾次。

`temp.h`：

```c
#ifndef TEMP_H
#define TEMP_H

extern int temp_calls;

int c_to_f(int c);
int f_to_c(int f);
int temp_round(int x10);

#endif
```

`ctof.c`、`ftoc.c`：跟第 1 題一樣。

`round.c`：

```c
#include "temp.h"

int temp_calls = 0;

int temp_round(int x10)
{
    temp_calls = temp_calls + 1;
    return (x10 + 5) / 10;
}
```

`hw3_main.c`：

```c
#include <stdio.h>
#include "temp.h"

int main(void)
{
    printf("%d %d\n", f_to_c(100), c_to_f(100));
    printf("temp_calls = %d\n", temp_calls);
    return 0;
}
```

### 3-1. 先預測：三個檔案都**不加** `-fPIC`，執行 `gcc -Wall -c ctof.c ftoc.c round.c && gcc -shared ctof.o ftoc.o round.o -o libtemp.so`，會成功嗎？如果失敗，錯誤訊息會指向哪個 `.o`、哪個符號？寫下預測後執行，貼上輸出。為什麼 `ctof.o`、`ftoc.o` 沒有被點名（它們也呼叫了別的檔案的 `temp_round`）？【筆記第四節〈為什麼一定要 `-fPIC`〉、〈為什麼不能直接用固定距離〉】

**第 1 次（2026-10-28）**
```
預測會成功,我沒有預測失敗
gcc -shared ctof.o ftoc.o round.o -o libtemp.so
/usr/bin/ld: round.o: warning: relocation against `temp_calls' in read-only section `.text'
/usr/bin/ld: round.o: relocation R_X86_64_PC32 against symbol `temp_calls' can not be used when making a shared object; recompile with -fPIC
/usr/bin/ld: final link failed: bad value
collect2: error: ld returned 1 exit status

應該是因爲沒有加-fPIC就走默認的-fPIE了,我看了他的組合語言如下
<temp_round>:
   0:   f3 0f 1e fa             endbr64
   4:   55                      push   %rbp
   5:   48 89 e5                mov    %rsp,%rbp
   8:   89 7d fc                mov    %edi,-0x4(%rbp)
   b:   8b 05 00 00 00 00       mov    0x0(%rip),%eax        # 11 <temp_round+0x11>
                        d: R_X86_64_PC32        temp_calls-0x4
  11:   83 c0 01                add    $0x1,%eax
  14:   89 05 00 00 00 00       mov    %eax,0x0(%rip)        # 1a <temp_round+0x1a>
                        16: R_X86_64_PC32       temp_calls-0x4
  1a:   8b 45 fc                mov    -0x4(%rbp),%eax
  1d:   83 c0 05                add    $0x5,%eax
  20:   48 63 d0                movslq %eax,%rdx
  23:   48 69 d2 67 66 66 66    imul   $0x66666667,%rdx,%rdx
  2a:   48 c1 ea 20             shr    $0x20,%rdx
  2e:   c1 fa 02                sar    $0x2,%edx
  31:   c1 f8 1f                sar    $0x1f,%eax
  34:   29 c2                   sub    %eax,%edx
  36:   89 d0                   mov    %edx,%eax
  38:   5d                      pop    %rbp
  39:   c3                      ret

看起來是直接透過rip與相對距離去存取temp_calls 然後下面就直接加1了,但是筆記有說過so 有可能全域變數不一定是在so, 可能在別的檔案,so的程式碼位址是固定的,因此應該用fPIC,這樣從rip找的就是got的位址,再從got(so的)讀真正的temp_calls在哪裏然後就可以得到正確的值

因爲出問題的是temp_round, 按照fPIE的方式寫出來的組語,最後temp_round再找temp_calls找到了.text的區段,顯然程式碼距離的寫法有問題
```

> **批改**：**不完整**。我用你 `hw3/` 的檔案重做，錯誤訊息跟你貼的一模一樣。預測錯（以為會成功）不扣分。
> - **答到的**：錯誤指向 `round.o`、`temp_calls`；沒加 `-fPIC` 時 Ubuntu 預設 `-fPIE`；組語裡 `mov 0x0(%rip),%eax` 配 `R_X86_64_PC32 temp_calls-0x4`，是「`%rip` 加固定距離、直接讀變數本身」；`.so` 的全域變數最後用的可能不是自己那一份，所以要先從 GOT 讀出位址再間接讀。這是本題的主線，講得對。
> - **最後一問沒答到核心**（10-28 你說明最後一段就是在答這一問，原本批改寫成「沒有回答」是我看錯，已更正；結果維持不完整）：你答「因為出問題的是 `temp_round`……它找 `temp_calls` 的程式碼距離寫法有問題」，這只說了**哪裡**出錯，沒解釋題目括號裡那個疑問：`ctof.o`、`ftoc.o` **也參照了別的檔案的符號 `temp_round`**，參照的位置同樣在唯讀的 `.text`，為什麼它們就沒事？要答出「函式和變數的處理方式不同」才算答到。關鍵在它們用到的是**函式**，不是變數：`objdump -dr ftoc.o` 是 `call 37 <f_to_c+0x37>`，下面的重定位是 `R_X86_64_PLT32 temp_round-0x4`。`PLT32` 的意思是「這個 `call` 可以接到 PLT 跳板」。`ld` 做 `.so` 時自己產生一個 `temp_round@plt`，把 `call` 的距離填成到跳板的距離（同一個 `.so` 裡，距離固定），真正的 `temp_round` 在哪，交給跳板去讀 `.got.plt`。所以**程式碼不用改**，不加 `-fPIC` 也行。我實測只用 `ctof.o ftoc.o` 做 `.so` 是成功的，反組譯看到兩個 `call 1050 <temp_round@plt>`。變數就沒有這種跳板：`mov 0x0(%rip),%eax` 讀的就是那 4 byte 本身，沒辦法「先跳到別處再讀」。所以只能在編譯時就改成經過 GOT 的寫法，也就是 `-fPIC`。
> - **錯的兩處**：
>   1. 「so 的程式碼位址是固定的」不對：`.so` 每次載入的位址都不同（ASLR）。但這也不是問題所在，因為 `%rip` 相對定址不管載入位址，`round.o` 跟自己的 `temp_calls` 在同一個 `.so` 裡，距離是固定的。問題是 `temp_calls` **最後可能不是這一份**（符號插入）。
>   2. 「`temp_round` 找 `temp_calls` 找到了 `.text` 的區段」把警告讀錯了。`relocation against 'temp_calls' in read-only section '.text'` 的意思是：**要填 `temp_calls` 距離的那個洞，位在唯讀的 `.text` 裡**。要讓它指到正確的那一份，只能在載入時改寫 `.text`；但 `.text` 是唯讀、而且要給所有行程共用，不能改。`temp_calls` 本身在 `.bss`，不在 `.text`。
>
> **參考答案**【筆記[第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#四動態函式庫-shared-與--fpic)〈[為什麼一定要 `-fPIC`：先看不加會怎樣](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#為什麼一定要--fpic先看不加會怎樣)〉、〈[為什麼不能直接用固定距離：`counter` 最後不一定是這一份](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#為什麼不能直接用固定距離counter-最後不一定是這一份)〉（「`add.c`、`mul.c` 不加 `-fPIC` 也做得成」那段）】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 失敗：`round.o: relocation R_X86_64_PC32 against symbol 'temp_calls' can not be used when making a shared object; recompile with -fPIC`，點名的是 `round.o` 的 `temp_calls`。
> 2. 原因：沒加 `-fPIC`（預設 `-fPIE`）時，`temp_round` 用「`%rip` 加固定距離」直接讀寫 `temp_calls`；但 `.so` 的全域變數最後可能被別人（例如執行檔）那一份取代，做 `.so` 時不知道是哪一份，要經 GOT 間接存取。程式碼在唯讀、共用的 `.text`，載入時也不能改。
> 3. `ctof.o`、`ftoc.o` 沒被點名：它們只是**呼叫函式** `temp_round`，`call` 的重定位是 `R_X86_64_PLT32`，`ld` 可以讓它改接到自己產生的 PLT 跳板，再由 `.got.plt` 決定真正的位址，程式碼不用改；存取變數沒有跳板可用。
>
> **詳細說明**：
> - 實測對照（你 `hw3/` 的檔案，沒加 `-fPIC`）：
>
>   ```text
>   $ objdump -dr --no-show-raw-insn ftoc.o | grep -A1 'call'
>     32:	call   37 <f_to_c+0x37>
>   			33: R_X86_64_PLT32	temp_round-0x4      ← 函式呼叫：可以接到 PLT
>   $ gcc -shared ctof.o ftoc.o -o libcf.so && objdump -d --no-show-raw-insn libcf.so | grep temp_round
>   0000000000001050 <temp_round@plt>:
>       1054:	jmp    *0x2fa6(%rip)        # 4000 <temp_round>
>       113b:	call   1050 <temp_round@plt>           ← c_to_f 的 call 被接到跳板
>       1174:	call   1050 <temp_round@plt>           ← f_to_c 的 call 被接到跳板
>   ```
>
>   `round.o` 的 `temp_calls` 是 `R_X86_64_PC32`（「直接到那個變數的距離」），`ld` 沒有辦法把它改成間接讀，只能叫你重編。
> - 同一個道理：筆記裡的 `add.c`、`mul.c` 沒存取全域變數，不加 `-fPIC` 也做得成 `.so`；`count_static.c` 的 `counter` 是 `static`，別人插入不了，也做得成。會失敗的只有「存取會被插入的全域變數」這一種。
> - 錯誤訊息怎麼讀：第一行 `warning: relocation against 'X' in read-only section '.text'` 說「要填 `X` 的洞在唯讀的 `.text`」；第二行點名是哪個 `.o`、哪一種重定位、哪個符號，並直接給解法 `recompile with -fPIC`。

### 3-2. 改用 `-fPIC` 重編三個檔案、做出 `libtemp.so`。貼上 `objdump -dr --no-show-raw-insn round.o` 裡 `temp_round` 的內容，標出**哪幾行在存取 `temp_calls`**，逐行說明那幾行在做什麼（中間 `imul $0x66666667`、`shr`、`sar` 那幾行是 gcc 用乘法代替「除以 10」的寫法，這題不用解釋）。再貼上 `readelf -r libtemp.so | grep temp_calls`，說明那一列是留給誰、要它做什麼。【筆記第四節〈為什麼一定要 `-fPIC`〉】

**第 1 次（2026-10-28）**
```
0:   endbr64
   4:   push   %rbp
   5:   mov    %rsp,%rbp
   8:   mov    %edi,-0x4(%rbp)
   b:   mov    0x0(%rip),%rax        # 12 <temp_round+0x12>
                        e: R_X86_64_REX_GOTPCRELX       temp_calls-0x4
  12:   mov    (%rax),%eax
  14:   lea    0x1(%rax),%edx
  17:   mov    0x0(%rip),%rax        # 1e <temp_round+0x1e>
                        1a: R_X86_64_REX_GOTPCRELX      temp_calls-0x4
  1e:   mov    %edx,(%rax)
  20:   mov    -0x4(%rbp),%eax
  23:   add    $0x5,%eax
  26:   movslq %eax,%rdx
  29:   imul   $0x66666667,%rdx,%rdx
  30:   shr    $0x20,%rdx
  34:   sar    $0x2,%edx
  37:   sar    $0x1f,%eax
  3a:   sub    %eax,%edx
  3c:   mov    %edx,%eax
  3e:   pop    %rbp
  3f:   ret

mov    0x0(%rip),%rax        # 12 <temp_round+0x12>
                 e: R_X86_64_REX_GOTPCRELX       temp_calls-0x4
這行是把got位址那格存的位址 複製到rax


mov    (%rax),%eax 取位址那格的值 也就是temp_calls 覆寫到eax
lea    0x1(%rax),%edx temp_calls + 1
大概是這些,下面都是一些計算



000000003fd8  000500000006 R_X86_64_GLOB_DAT 0000000000004014 temp_calls + 0
應該是got裏面存着指向實際temp_calls的位址
```

> **批改**：**不完整**。我用你 `hw3/` 的 `round.o`、`libtemp.so` 重跑，輸出跟你貼的一模一樣。
> - **答到的**：`b` 那行從 GOT 那一格讀出 `temp_calls` 的**位址**放進 `%rax`；`12` 照這個位址讀出值；`14` 算 +1；`readelf -r` 那一列跟 GOT 有關，那一格最後放的是 `temp_calls` 的位址。讀的那一半是對的。
> - **漏的 1：寫回的兩行沒標出來**。`temp_calls = temp_calls + 1` 除了讀還要寫：`17: mov 0x0(%rip),%rax` 再從 GOT 拿一次 `temp_calls` 的位址，`1e: mov %edx,(%rax)` 照位址把 +1 的結果寫回去。你寫「下面都是一些計算」把這兩行跳過了；真正的計算（`(x10 + 5) / 10`）是從 `20` 開始。
> - **漏的 2：那一列是「留給誰、要它做什麼」沒答到**。你寫的是 GOT 那格「最後的樣子」，題目問的是這一列是**給誰的指示**：它在 `.rela.dyn`（動態重定位表）裡，是留給**動態載入器 `ld.so`** 的，意思是「**載入時**，查出 `temp_calls` 最後用的是哪一份，把它的真正位址寫進 GOT 的 `0x3fd8` 那一格」。做 `.so` 時 `ld` 還不知道這格該填什麼（`temp_calls` 可能被別人那一份取代），所以才留下這筆給執行時處理。
> - 小修（不扣分）：`b` 那行可以講得更準：洞的型別是 `R_X86_64_REX_GOTPCRELX`，`ld` 填進去的是「到 GOT 那一格的距離」，不是到 `temp_calls` 的距離，所以讀到的 8 byte 是位址、不是值（`libtemp.so` 裡這行變成 `mov 0x2e4b(%rip),%rax  # 3fd8`）。另外 `mov (%rax),%eax` 寫的是 32 位元的 `%eax`，會把 `%rax` 的高 32 位元清成 0，所以下一行 `lea 0x1(%rax),%edx` 拿 `%rax` 來算也沒問題（第 4 課〈寫低位的名字時，高位會怎樣？〉）。
>
> **參考答案**【筆記[第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#四動態函式庫-shared-與--fpic)〈[為什麼一定要 `-fPIC`：先看不加會怎樣](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#為什麼一定要--fpic先看不加會怎樣)〉（`count.pic.o` 的組語對照與 `readelf -r libcount.so` 那段）】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 存取 `temp_calls` 的是 `b`、`12`、`17`、`1e`（加上中間算 +1 的 `14`）：`b` 從 GOT 讀出 `temp_calls` 的位址 → `12` 照位址讀出值 → `14` 加 1 → `17` 再從 GOT 讀一次位址 → `1e` 照位址寫回。
> 2. 兩次都是**先讀 GOT 那一格、再照位址間接存取**：`R_X86_64_REX_GOTPCRELX` 的洞填的是「到 GOT 那一格的距離」，程式碼裡沒有任何「到 `temp_calls` 本身的距離」。
> 3. `readelf -r` 的 `R_X86_64_GLOB_DAT ... temp_calls` 是留給 **`ld.so`** 的：**載入時**把 `temp_calls` 最後選定那一份的真正位址寫進 GOT 的 `0x3fd8`。
>
> **詳細說明**：
> - 實測（你 `hw3/` 的檔案）：做成 `libtemp.so` 之後，洞被 `ld` 填好了：
>
>   ```text
>   $ objdump -d --no-show-raw-insn libtemp.so | sed -n '/<temp_round>:/,/ret/p'
>       1186:	mov    0x2e4b(%rip),%rax        # 3fd8 <temp_calls-0x3c>
>       118d:	mov    (%rax),%eax
>       118f:	lea    0x1(%rax),%edx
>       1192:	mov    0x2e3f(%rip),%rax        # 3fd8 <temp_calls-0x3c>
>       1199:	mov    %edx,(%rax)
>   $ readelf -S libtemp.so | grep -E '\.got |\.bss'
>     [19] .got              PROGBITS         0000000000003fc0  00002fc0
>     [22] .bss              NOBITS           0000000000004010  00003010
>   $ readelf -s libtemp.so | grep ' temp_calls'
>      26: 0000000000004014     4 OBJECT  GLOBAL DEFAULT   22 temp_calls
>   ```
>
>   `%rip` 是下一條指令的位址：`0x118d + 0x2e4b = 0x3fd8`、`0x1199 + 0x2e3f = 0x3fd8`，兩次都指到 `.got`（從 `0x3fc0` 開始）裡同一格。`temp_calls` 自己在 `0x4014`，第 22 個區段 `.bss`。程式碼只知道「GOT 在離我固定距離的地方」，`temp_calls` 實際在哪由 GOT 那格決定。
> - **`GLOB_DAT` 那一列每一欄是什麼、`ld.so` 怎麼用**（2026-10-28 使用者指出原本的說明沒講 `0x4014` 跟 `ld.so` 的關係，補上；原本寫「別的地方也有 `temp_calls` 時，3-4 會實際做」也不對——你的 `hw3` 本身就是那種情況，見下面實測）：
>
>   | 欄 | 值 | 意思 |
>   |---|---|---|
>   | `Offset` | `0x3fd8` | **要寫到哪裡**：GOT 那一格（相對 `.so` 載入基址） |
>   | `Info` | `0x000500000006` | 高 32 位元 `5` ＝ `.dynsym` 第 5 個符號（`temp_calls`）；低 32 位元 `6` ＝ 型別 `GLOB_DAT`：「把這個符號的位址整個寫進去」 |
>   | `Sym. Value` | `0x4014` | `readelf` 順手印出第 5 個符號在 `.dynsym` 裡的值：**這個 `.so` 自己那一份** `temp_calls` 的位置（相對基址，在 `.bss`）。它不是「答案」，只是候選之一 |
>   | `Addend` | `+ 0` | 位址再加多少 |
>
>   `ld.so` 處理這列時，拿的是**符號名字** `temp_calls`，照搜尋順序（執行檔先、再依載入順序的各個 `.so`）找第一個定義：找到的是 `libtemp.so` 自己那一份，才寫「`libtemp.so` 的基址 ＋ `0x4014`」；找到的是別人的，就寫別人那一份的位址，`0x4014` 用不到。
> - **實測：`0x3fd8` 最後被寫成什麼**（`libtemp.so` 照 3-2 用 `-fPIC` 做；在 `temp_round` 讀完 GOT 那一行之後停下來，`%rax` 就是 GOT `0x3fd8` 裡的值）。兩支程式：`hw3`（你的 `hw3_main.c`，有用到 `temp_calls`）和 `nouse`（沒用到 `temp_calls`）：
>
>   ```c
>   /* nouse.c：不碰 temp_calls */
>   #include <stdio.h>
>   #include "temp.h"
>
>   int main(void)
>   {
>       printf("%d\n", f_to_c(100));
>       return 0;
>   }
>   ```
>
>   ```text
>   $ gcc -Wall -g hw3_main.c -L. -ltemp -Wl,-rpath,'$ORIGIN' -o hw3
>   $ gcc -Wall -g nouse.c    -L. -ltemp -Wl,-rpath,'$ORIGIN' -o nouse
>   $ readelf -r hw3 | grep temp_calls
>   000000004010  000900000005 R_X86_64_COPY     0000000000004010 temp_calls + 0
>   $ readelf -r nouse | grep temp_calls          ← 沒有輸出
>   $ gdb -q -batch -ex start -ex 'break *temp_round+18' -ex continue -ex 'p/x $rax' -ex 'p &temp_calls' ./hw3
>   $1 = 0x555555558010                          ← GOT 0x3fd8 裡的值
>   $2 = (int *) 0x555555558010 <temp_calls>     ← 執行檔 .bss 那一份
>   $ gdb -q -batch -ex start -ex 'break *temp_round+18' -ex continue -ex 'p/x $rax' -ex 'p &temp_calls' ./nouse
>   $1 = 0x7ffff7fba014                          ← GOT 0x3fd8 裡的值
>   $2 = (...) 0x7ffff7fba014 <temp_calls>       ← libtemp.so 那一份
>   ```
>
>   （`gdb` 關掉了位址隨機化：`info sharedlibrary` 顯示 `libtemp.so` 的 `.text`（檔案裡從 `0x1060` 開始）在 `0x7ffff7fb7060`，所以基址是 `0x7ffff7fb6000`。`temp_round+18` 是 `mov (%rax),%eax` 那一行，停在它執行之前。）
>   - `nouse`：`0x7ffff7fb6000 + 0x4014 = 0x7ffff7fba014`，**這時才用到 `0x4014`**。
>   - `hw3`：`hw3_main.c` 直接讀了 `temp_calls`，`ld` 替它做了**複製重定位**（`R_X86_64_COPY`，執行檔 `.bss` 的 `0x4010` 放一份），執行檔先被搜尋，所以 `ld.so` 把**執行檔那一份**的位址 `0x555555558010` 寫進 `libtemp.so` 的 GOT，`libtemp.so` 自己 `0x4014` 那一份從此沒人用。這就是 `-fPIC` 經過 GOT 的理由：`temp_round` 的程式碼不用改，就能改去讀寫執行檔那一份。複製重定位見筆記[第九節第 3 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#3-主程式只寫-extern-int-counter變數放進執行檔複製重定位)。
> - 為什麼 `17` 要再讀一次 GOT：`-O0` 不把值留在暫存器裡重用，每次用到 `temp_calls` 都重新照規矩取一次位址；`-O2` 會只讀一次。
> - 不加 `-fPIC` 時（3-1）這兩處是 `mov 0x0(%rip),%eax` 配 `R_X86_64_PC32`，直接讀寫 `temp_calls` 本身，所以做不成 `.so`。

### 3-3. 用 `gcc -Wall hw3_main.c -L. -ltemp -o hw3` 連結，執行 `./hw3`，貼上輸出與 `ldd ./hw3` 的輸出。這個錯誤是誰印的？為什麼連結成功了，執行卻找不到？接著用兩種方法讓它跑起來，各貼上指令與輸出：(a) `LD_LIBRARY_PATH`；(b) 重新連結，用 `-Wl,-rpath,'$ORIGIN'` 把路徑寫進執行檔（另外貼上 `readelf -d hw3 | grep -E 'NEEDED|RUNPATH'`）。輸出裡 `temp_calls` 是幾？為什麼是這個數字？【筆記[第五節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#五執行時怎麼找到-so)〈方法一〉、〈方法二〉】

**第 1 次（2026-10-28）**
```
./hw3: error while loading shared libraries: libtemp.so: cannot open shared object file: No such file or directory
 linux-vdso.so.1 (0x000072faf7ac1000)
        libtemp.so => not found
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x000072faf7800000)
        /lib64/ld-linux-x86-64.so.2 (0x000072faf7ac3000)
這個錯誤是ld.so印的

因爲ld.so的搜尋路徑沒有-L.,這個參數是給ld看的,ld.so不知道

(a)
LD_LIBRARY_PATH=. ./hw3
38 212
temp_calls = 2

(b)
 gcc -Wall hw3_main.c -L. -ltemp -Wl,-rpath,'$ORIGIN' -o hw3
38 212
temp_calls = 2
0x0000000000000001 (NEEDED)             Shared library: [libtemp.so]
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
 0x000000000000001d (RUNPATH)            Library runpath: [$ORIGIN]

temp_calls 是2
因爲c_to_f, f_to_c都各自用temp_round+1一次
```

> **批改**：**對**。我用你 `hw3/` 的檔案重做，沒加 `-rpath` 的版本同樣是 `cannot open shared object file`、`ldd` 顯示 `libtemp.so => not found`；(b) 的 `RUNPATH [$ORIGIN]` 與兩次輸出都一樣。
> - 抓到：錯誤是 `ld.so` 印的；`-L.` 只給連結時的 `ld` 看，`ld.so` 不知道；(a)、(b) 兩種方法的指令與輸出都對；`temp_calls = 2` 是 `f_to_c`、`c_to_f` 各呼叫 `temp_round` 一次。
> - 小修（不扣分）：
>   1. 「為什麼找不到」可以再補一句 `ld.so` 手上有什麼：執行檔的 `NEEDED` 只記了名字 `libtemp.so`、**沒有記路徑**，`ld.so` 只能照自己的順序找（`LD_LIBRARY_PATH` → `RUNPATH` → `/etc/ld.so.cache` → 預設資料夾），裡面沒有目前這個資料夾。
>   2. `temp_calls` 是 2，前提是它從 0 開始（`round.c` 的 `int temp_calls = 0;`）。另外 3-2 補的實測說過：`hw3_main.c` 讀 `temp_calls`，執行檔裡有一份複製重定位來的 `temp_calls`，`libtemp.so` 的 GOT 也指向它，所以 `temp_round` 加的和 `main` 印的是**同一份**，才會剛好是 2。
>
> **參考答案**【筆記[第五節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#五執行時怎麼找到-so)〈[`ld.so` 去了哪些地方找](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#ldso-去了哪些地方找)〉、〈[方法一：`LD_LIBRARY_PATH`（臨時用）](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#方法一ld_library_path臨時用)〉、〈[方法二：`-Wl,-rpath`——把路徑寫進執行檔](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#方法二-wl-rpath把路徑寫進執行檔)〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 錯誤是動態載入器 `ld.so` 在執行時印的（`ldd` 也顯示 `libtemp.so => not found`）。
> 2. `-L.` 只告訴連結時的 `ld` 去哪找；執行檔的 `NEEDED` 只記名字 `libtemp.so`，`ld.so` 照自己的搜尋順序找，裡面沒有 `.`，所以找不到。
> 3. (a) `LD_LIBRARY_PATH=. ./hw3`；(b) 用 `-Wl,-rpath,'$ORIGIN'` 重新連結，`readelf -d` 多出 `RUNPATH [$ORIGIN]`，`$ORIGIN` 在執行時代表執行檔所在的資料夾。
> 4. `temp_calls = 2`：從 0 開始，`f_to_c(100)`、`c_to_f(100)` 各呼叫 `temp_round` 一次。
>
> **詳細說明**：
> - `LD_DEBUG=libs` 可以看到 `ld.so` 實際找了哪裡（沒加 `-rpath` 的版本）：
>
>   ```text
>   $ LD_DEBUG=libs ./hw3 2>&1 | grep -E 'find library|search'
>         find library=libtemp.so [0]; searching
>          search cache=/etc/ld.so.cache
>          search path=/lib/x86_64-linux-gnu/glibc-hwcaps/x86-64-v3:...:/lib:/usr/lib		(system search path)
>   ```
>
>   沒有 `LD_LIBRARY_PATH`、也沒有 `RUNPATH`，所以直接查快取再查系統資料夾，都沒有 `libtemp.so`。
> - (a) 只對這一次執行有效；(b) 寫進執行檔，而且用 `$ORIGIN` 而不是絕對路徑，整個資料夾搬到別處也還找得到。
### 3-4. 先預測：在 `hw3_main.c` 的 `#include "temp.h"` 下面加一行 `int temp_calls = 1000;`（`temp.h` 不改），照 3-3 (b) 的方式重新連結、執行，`temp_calls = ` 會印出多少？寫下預測後執行，貼上輸出與 `LD_DEBUG=bindings ./hw3 2>&1 | grep temp_calls`。為什麼？接著把 `round.o` 等三個 `-fPIC` 的 `.o` 用 `ar rcs libtemp.a ctof.o ftoc.o round.o` 包成 `.a`，用 `gcc -Wall hw3_main.c -L. -l:libtemp.a -o hw3s` 靜態連結，先預測會怎樣再執行、貼上結果；為什麼同樣兩份 `temp_calls`，動態和靜態的結果不一樣？做完把加的那一行刪掉。【筆記第四節〈為什麼不能直接用固定距離〉】

**第 1 次（2026-10-28）**
```
1002

38 212
temp_calls = 1002
15125:     binding file /home/set/work/Projects/github/good-note/small-project/s1-07-libraries/hw3/libtemp.so [0] to ./hw3 [0]: normal symbol `temp_calls'
temp_calls = 1002
因爲動態載入器選擇temp_calls在got的位址是從執行檔,然後才在so選,所以會選到執行檔的temp_calls

從hw3_main可以看到ctof,ftoc, round全都會用到,所以三個.o都會被加入執行檔,然後選擇執行檔比較近的int temp_calls=1000
結果會是1002

/usr/bin/ld: ./libtemp.a(round.o):(.bss+0x0): multiple definition of `temp_calls'; /tmp/cco7GIjT.o:(.data+0x0): first defined here
collect2: error: ld returned 1 exit status

結果是連結錯誤了,temp_calls重複定義

動態是執行時,動態載入器選擇符號的時候,按照執行檔.so的順序重定位temp calls,但我不確定這是否就是hw3_main.c不跟so做符號連接?
靜態是連結是,ld在連結符號的時候發現兩個符號重複定義
```

> **批改**：**大致對**。我用你 `hw3/` 的原始碼（`hw3_main.c.bak` 裡加了那一行的版本）重做，動態 `temp_calls = 1002`、`LD_DEBUG` 那一行、靜態的 `multiple definition` 都跟你貼的一樣。動態的預測 1002 對；靜態預測成 1002（實際是連結錯誤），預測錯不扣分。
> - **抓到的**：動態印 1002；`LD_DEBUG` 顯示 `libtemp.so` 的 `temp_calls` 被綁到 `./hw3`；原因是 `ld.so` 先找執行檔、再找 `.so`，所以 `libtemp.so` 經 GOT 存取的是執行檔那一份（初值 1000），`temp_round` 被呼叫兩次 → 1002。靜態：`f_to_c`、`c_to_f` 需要 `temp_round`，所以 `round.o` 被整個拉進來，連同它的 `temp_calls` 定義，跟 `hw3_main.o` 的定義撞在一起 → `multiple definition`。
> - **缺的（最後一問的動態那一半）**：你自己也寫了不確定——為什麼動態時 `ld` **不報**重複定義？關鍵是 **`.so` 不會被合併進執行檔**：`.a` 的成員 `round.o` 是被**複製進**執行檔，兩個 `GLOBAL` 定義放進同一個檔案，`ld` 只能報錯；`.so` 只是被「記下名字」（`NEEDED`），它裡面的 `temp_calls` 留在 `.so` 自己的檔案裡，`ld` 不把它算成第二個定義，選哪一份延到執行時由 `ld.so` 照搜尋順序挑第一個（符號插入）。這在筆記第九節第 2 章〈[兩份 `counter`：接 `.so` 為什麼不算重複定義](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#2-兩份-counter接-so-為什麼不算重複定義)〉教過。至於你問的「是不是 `hw3_main.c` 不跟 `.so` 做符號連結」：有做——`hw3_main.c` 要用的 `f_to_c`、`c_to_f` 就是從 `libtemp.so` 解析來的；只是 `temp_calls` 執行檔自己有定義，用自己的，反過來是 `libtemp.so` 的參照被綁到執行檔。
> - 小修（不扣分）：
>   1. 錯誤訊息裡兩份的位置不一樣：`hw3_main.o` 的在 `.data`（初值 1000 不是 0），`round.o` 的在 `.bss`（初值 0）——這兩份都是有初值的「定義」，所以就算 `-fcommon` 也救不了（第 6 課多檔 4-4 的 `-fcommon` 只對沒有初值的暫定定義有用）。
>   2. 「選擇執行檔比較近的」：`ld.so` 選的依據是**搜尋順序**（執行檔排第一），不是距離遠近。
>   3. 3-3 是複製重定位（`R_X86_64_COPY`）把 `temp_calls` 放進執行檔；這次執行檔自己定義了它，`readelf -r hw3 | grep temp_calls` 什麼都沒有，因為不需要複製，`libtemp.so` 的 GOT 一樣指向執行檔那一份。
>
> **參考答案**【筆記第四節〈[為什麼不能直接用固定距離：`counter` 最後不一定是這一份](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#為什麼不能直接用固定距離counter-最後不一定是這一份)〉、第二節〈[連結：只拉需要的成員](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#連結只拉需要的成員)〉、第九節第 2 章〈[兩份 `counter`：接 `.so` 為什麼不算重複定義](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#2-兩份-counter接-so-為什麼不算重複定義)〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 動態印 `temp_calls = 1002`；`LD_DEBUG=bindings` 顯示 `libtemp.so` 的 `temp_calls` 被綁到 `./hw3`。
> 2. 原因：`ld.so` 照搜尋順序先找執行檔，`libtemp.so` 的 GOT 填的是執行檔那一份（初值 1000）的位址，`temp_round` 被呼叫兩次都加在它上面（符號插入）。
> 3. 靜態：`round.o` 因為 `temp_round` 被拉進執行檔，連同它的 `temp_calls` 定義，跟 `hw3_main.o` 的定義在同一次連結裡撞名 → `multiple definition`，連結失敗。
> 4. 差別：`.a` 的成員被**合併**進同一個執行檔，兩個定義只能二選一而報錯；`.so` **不合併**，`ld` 不把它的定義算成重複，留到執行時由 `ld.so` 選第一個找到的。
>
> **詳細說明**：
> - 實測（`gcc -Wall -fPIC -c ctof.c ftoc.c round.c`、`gcc -shared … -o libtemp.so`、`gcc -Wall hw3_main.c -L. -ltemp -Wl,-rpath,'$ORIGIN' -o hw3`）：
>
>   ```text
>   $ ./hw3
>   38 212
>   temp_calls = 1002
>   $ LD_DEBUG=bindings ./hw3 2>&1 | grep temp_calls
>   binding file …/libtemp.so [0] to ./hw3 [0]: normal symbol `temp_calls'
>   $ gcc -Wall hw3_main.c -L. -l:libtemp.a -Wl,--verbose -o hw3s 2>&1 | grep 'libtemp.a)'
>   (./libtemp.a)ctof.o
>   (./libtemp.a)ftoc.o
>   (./libtemp.a)round.o
>   $ readelf -s hw3_main.o | grep temp_calls
>        4: 0000000000000000     4 OBJECT  GLOBAL DEFAULT    3 temp_calls      ← 區段 3 是 .data
>   $ readelf -s round.o | grep temp_calls
>        3: 0000000000000000     4 OBJECT  GLOBAL DEFAULT    4 temp_calls      ← 區段 4 是 .bss
>   ```
>
>   `binding file A to B` 讀作「A（`libtemp.so`）裡對 `temp_calls` 的參照，綁到 B（`./hw3`）的定義」。
> - 同一件事的兩面：動態時「多一份同名定義」不是錯，而是**刻意可用的功能**（`LD_PRELOAD` 換掉 `malloc` 就是靠它）；靜態時兩份定義被放進同一個檔案，沒有「先找誰」可言，只能報錯。
> - 如果 `round.o` 沒被拉進來（例如 `hw3_main.c` 完全不呼叫 `f_to_c`、`c_to_f`），靜態連結就不會撞名——`ld` 只拉需要的成員（第二節）。
---

## 4. `.so` 依賴另一個 `.so`：`RUNPATH` 只管直接依賴

在 `hw4/` 裡。這次 `libtemp.so` 自己也需要另一個函式庫 `libround.so`，兩個都放在 `hw4/lib/`，執行檔 `hw4` 放在 `hw4/`。

`temp.h`、`ctof.c`、`ftoc.c`、`round.c`：跟**第 1 題**一樣（沒有 `temp_calls`、沒有 `temp_clamp`）。

`hw4_main.c`：

```c
#include <stdio.h>
#include "temp.h"

int main(void)
{
    printf("c_to_f(37) = %d\n", c_to_f(37));
    return 0;
}
```

建立與連結（照打；`-Wl,-rpath-link,lib` 是什麼見 4-3）：

```sh
mkdir -p lib
gcc -Wall -fPIC -shared round.c -o lib/libround.so
gcc -Wall -fPIC -shared ctof.c ftoc.c -Llib -lround -o lib/libtemp.so
gcc -Wall hw4_main.c -Llib -ltemp -Wl,-rpath-link,lib -Wl,-rpath,'$ORIGIN/lib' -o hw4
```

### 4-1. 先預測 `./hw4` 會成功嗎？寫下預測後執行，貼上輸出；再貼上 `readelf -d hw4 | grep -E 'NEEDED|RUNPATH'`、`readelf -d lib/libtemp.so | grep -E 'NEEDED|RUNPATH'`，以及 `LD_DEBUG=libs ./hw4 2>&1 | grep -E 'find library|search path'`。`hw4` 明明有 `RUNPATH`、`libround.so` 也跟 `libtemp.so` 放在同一個資料夾，為什麼找不到（或為什麼找得到）？【筆記第五節〈`ld.so` 去了哪些地方找〉的第 3 步、〈`RPATH` 和 `RUNPATH` 差在哪〉最後一段】

**第 1 次（2026-10-28）**
```
可以成功

./hw4: error while loading shared libraries: libround.so: cannot open shared object file: No such file or directory

0x0000000000000001 (NEEDED)             Shared library: [libtemp.so]
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
 0x000000000000001d (RUNPATH)            Library runpath: [$ORIGIN/lib]

0x0000000000000001 (NEEDED)             Shared library: [libround.so]

沒有輸出

hw4可以找得到libtemp.so, 但問題處在libtemp.so找不到libround.so,他沒有runpath
```

> **批改**：**大致對**。我在你的 `hw4/` 直接執行，錯誤訊息、兩個 `readelf -d` 的輸出都跟你貼的一樣。預測「可以成功」錯了，不扣分。
> - **抓到的**：失敗的是 `libround.so`；`hw4` 靠自己的 `RUNPATH`（`$ORIGIN/lib`）找到了 `libtemp.so`，但 `libround.so` 是 **`libtemp.so` 的** `NEEDED`，要看 `libtemp.so` 自己的 `RUNPATH`，而它沒有。主線對。
> - **缺的 1：`LD_DEBUG` 那一行不是「沒有輸出」**。我在你的 `hw4/` 跑同一行，有 6 行輸出（見下面詳細說明）；你那邊沒輸出，可能是不在 `hw4/` 裡執行（`./hw4` 找不到，錯誤訊息不含 `find library`），或指令打錯——我沒辦法從作答看出是哪一個。這段輸出正是這題的**證據**：找 `libtemp.so` 時括號寫 `(RUNPATH from file ./hw4)`，找 `libround.so` 時只剩 `search cache` 與 `(system search path)`，`hw4` 的 `RUNPATH` 完全沒被拿來用。
> - **缺的 2：沒有講明規則本身**。你寫「它（`libtemp.so`）沒有 `RUNPATH`」，等於默認了規則，但沒說出來：**`DT_RUNPATH` 只用來找「這個檔案自己」的 `NEEDED`，不會套用到依賴的依賴**（`RPATH` 才會套用到整棵依賴樹）。題目括號裡「`libround.so` 也跟 `libtemp.so` 放在同一個資料夾」的疑問也要回答：`ld.so` **不會**自動去「某個 `.so` 所在的資料夾」找它的依賴，除非那個 `.so` 的 `RUNPATH` 寫了 `$ORIGIN`（4-2 (a) 就是這樣修）。
>
> **參考答案**【筆記[第五節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#五執行時怎麼找到-so)〈[`ld.so` 去了哪些地方找](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#ldso-去了哪些地方找)〉的第 3 步、〈[`RPATH` 和 `RUNPATH` 差在哪：跟 `LD_LIBRARY_PATH` 誰優先](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#rpath-和-runpath-差在哪跟-ld_library_path-誰優先)〉最後一段「`RUNPATH` 只管直接依賴」】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 失敗：`error while loading shared libraries: libround.so: cannot open shared object file`；找不到的是 `libround.so`，不是 `libtemp.so`。
> 2. `DT_RUNPATH` 只用來找**這個檔案自己的** `NEEDED`：`hw4` 的 `RUNPATH` 只負責 `libtemp.so`；`libround.so` 是 `libtemp.so` 的 `NEEDED`，要用 `libtemp.so` 的 `RUNPATH`，它沒有，所以只查快取與系統資料夾。放在同一個資料夾不會自動被找到。
> 3. 證據：`LD_DEBUG=libs` 找 `libtemp.so` 的那行標 `(RUNPATH from file ./hw4)`，找 `libround.so` 只有 `search cache` 和 `(system search path)`。
>
> **詳細說明**：
> - 實測（在你的 `hw4/` 執行，路徑縮寫成 `…/hw4`）：
>
>   ```text
>   $ LD_DEBUG=libs ./hw4 2>&1 | grep -E 'find library|search path'
>   find library=libtemp.so [0]; searching
>    search path=…/hw4/lib/glibc-hwcaps/x86-64-v3:…/hw4/lib/glibc-hwcaps/x86-64-v2:…/hw4/lib		(RUNPATH from file ./hw4)
>   find library=libc.so.6 [0]; searching
>    search path=…/hw4/lib		(RUNPATH from file ./hw4)
>   find library=libround.so [0]; searching
>    search path=/lib/x86_64-linux-gnu/glibc-hwcaps/x86-64-v3:…:/usr/lib		(system search path)
>   ```
>
>   `libc.so.6` 也是 `hw4` 自己的 `NEEDED`，所以同樣先試 `hw4` 的 `RUNPATH`（`…/hw4/lib` 裡沒有，再去快取找到）。`libround.so` 的 `find library` 是在載入 `libtemp.so` 之後、處理**它的** `NEEDED` 時發生的，括號裡完全沒有 `RUNPATH`。
> - 為什麼這樣設計：`RUNPATH` 是每個檔案替**自己**負責——函式庫作者知道自己的依賴放哪，執行檔作者只管自己直接用的。舊的 `RPATH` 會往下傳到整棵樹，執行檔的路徑會影響到它根本不知道的間接依賴，所以被新的 `RUNPATH` 取代（`ld` 現在預設寫 `RUNPATH`）。
> - 修法就是 4-2 的兩種：讓 `libtemp.so` 自己帶 `RUNPATH`，或讓 `hw4` 改用會往下傳的 `RPATH`。
### 4-2. 用兩種方法修好，各貼上你改的指令、`readelf -d` 相關的列與 `./hw4` 的輸出：(a) 只重做 `lib/libtemp.so`、讓它自己帶 `RUNPATH`（路徑要用 `$ORIGIN` 寫，想一想 `libtemp.so` 的 `$ORIGIN` 是哪個資料夾）；(b) 不改 `libtemp.so`（先把它做回原本的樣子），改成重新連結 `hw4`、用 `RPATH`。兩種方法你會選哪一種？為什麼？【筆記第五節〈`RPATH` 和 `RUNPATH` 差在哪〉】

**第 1 次（2026-10-28）**
```
我先做(b)
gcc -Wall hw4_main.c -Llib -ltemp -Wl,-rpath-link,lib -Wl,--disable-new-dtags,-rpath,'$ORIGIN/lib' -o hw4 
./hw4
c_to_f(37) = 99

再做(a)
gcc -Wall -shared -fPIC ctof.c ftoc.c -Llib -lround -Wl,-rpath,'$ORIGIN' -o lib/libtemp.so
gcc -Wall hw4_main.c -Llib -ltemp -Wl,-rpath-link,lib -Wl,-rpath,'$ORIGIN/lib' -o hw4
./hw4
c_to_f(37) = 99

libtemp 的 ORIGIN要用lib, origin是編譯出來的so或執行檔的路徑

我會選擇a
a.不論是執行檔,還是so都是用runpath, 在so標記runpath解決了剛剛執行時libtemp找不到libround的問題
b. 用了rpath,但我不清楚爲什麼rpath可以不管so沒有rpath的問題

我選a純粹是覺得libtemp既然有用到libround他理應把round在哪裏的路徑記住
```

> **批改**：**大致對**。我看了你 `hw4/` 現在的檔案（(a) 做完的狀態）：`lib/libtemp.so` 有 `RUNPATH [$ORIGIN]`、`./hw4` 印 `c_to_f(37) = 99`；(b) 我用你的指令在 scratchpad 重做，`hw4` 是 `RPATH [$ORIGIN/lib]`，也能跑。兩個方法的指令都對。
> - **抓到的**：(a) 在 `libtemp.so` 加 `-Wl,-rpath,'$ORIGIN'`；(b) 用 `--disable-new-dtags` 讓 `-rpath` 寫成舊式 `RPATH`（這個選項筆記第五節〈方法二〉的 `app_rpath` 例子和第七節 `ld` 表有）；選 (a)，理由「`libtemp.so` 用到 `libround.so`，理應自己記住它在哪」就是重點——每個檔案替自己的依賴負責。
> - **缺的：(b) 為什麼能修好**。你寫「不清楚為什麼 `RPATH` 可以不管 `.so` 沒有 `RPATH`」——這是本題要考的差別，筆記教過：`DT_RUNPATH` **只用來找這個檔案自己的 `NEEDED`**，`DT_RPATH` 則**套用到整棵依賴樹**（第五節〈`ld.so` 去了哪些地方找〉第 3 步）。所以 `hw4` 的 `RPATH` 在載入 `libtemp.so` 之後、找**它的** `NEEDED` `libround.so` 時也會被拿來用。`LD_DEBUG` 看得到：找 `libround.so` 那一行標的是 `(RPATH from file ./hw4)`，跟 4-1 的 `(system search path)` 對照。
> - **缺的：沒貼 `readelf -d`**。題目要兩種方法各貼 `readelf -d` 相關的列，這是證明「真的寫進去了、寫的是 `RUNPATH` 還是 `RPATH`」的證據（下面詳細說明有我重做的輸出）。
> - 小修（不扣分）：「`$ORIGIN` 是編譯出來的 `.so` 或執行檔的路徑」——是**執行時**那個檔案**所在的資料夾**，不是編譯時的位置；`$ORIGIN` 原封不動寫進檔案（`readelf` 印出來就是字面上的 `$ORIGIN`），`ld.so` 載入時才換掉。`libtemp.so` 在 `hw4/lib/`，所以它的 `$ORIGIN` 就是 `lib/`，寫 `'$ORIGIN'` 就夠，你的指令是對的；「要用 lib」那句意思對但說得含糊。
>
> **參考答案**【筆記第五節〈[`RPATH` 和 `RUNPATH` 差在哪：跟 `LD_LIBRARY_PATH` 誰優先](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#rpath-和-runpath-差在哪跟-ld_library_path-誰優先)〉、〈[`ld.so` 去了哪些地方找](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#ldso-去了哪些地方找)〉第 1、3 步】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. (a)：重做 `libtemp.so` 時加 `-Wl,-rpath,'$ORIGIN'`；`$ORIGIN` 是 `libtemp.so` 執行時所在的資料夾（`hw4/lib/`），`readelf -d lib/libtemp.so` 多一列 `RUNPATH [$ORIGIN]`，`./hw4` 成功。
> 2. (b)：重新連結 `hw4` 時加 `-Wl,--disable-new-dtags,-rpath,'$ORIGIN/lib'`，`readelf -d hw4` 是 `RPATH [$ORIGIN/lib]`；能修好是因為 **`RPATH` 會套用到整棵依賴樹**（`RUNPATH` 只管自己的 `NEEDED`），找 `libround.so` 時也用得到。
> 3. 選 (a)：函式庫自己記住自己的依賴，誰用這個 `.so` 都不必知道它的間接依賴；`RPATH` 已不建議使用（而且優先於 `LD_LIBRARY_PATH`，臨時換函式庫測試時蓋不過它）。
>
> **詳細說明**：
> - (b) 重做（scratchpad，用原本沒有 `RUNPATH` 的 `libtemp.so`）：
>
>   ```text
>   $ gcc -Wall hw4_main.c -Llib -ltemp -Wl,-rpath-link,lib -Wl,--disable-new-dtags,-rpath,'$ORIGIN/lib' -o hw4
>   $ ./hw4
>   c_to_f(37) = 99
>   $ readelf -d hw4 | grep -E 'NEEDED|RPATH|RUNPATH'
>    0x0000000000000001 (NEEDED)             Shared library: [libtemp.so]
>    0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
>    0x000000000000000f (RPATH)              Library rpath: [$ORIGIN/lib]
>   $ readelf -d lib/libtemp.so | grep -E 'NEEDED|RPATH|RUNPATH'
>    0x0000000000000001 (NEEDED)             Shared library: [libround.so]
>   $ LD_DEBUG=libs ./hw4 2>&1 | grep -A1 'find library=libround'
>   find library=libround.so [0]; searching
>    search path=…/lib		(RPATH from file ./hw4)
>   ```
>
>   型別編號也不同：`RPATH` 是 `0xf`（`DT_RPATH`），`RUNPATH` 是 `0x1d`（`DT_RUNPATH`）。
> - (a) 你 `hw4/` 現在的狀態：
>
>   ```text
>   $ readelf -d lib/libtemp.so | grep -E 'NEEDED|RUNPATH|RPATH'
>    0x0000000000000001 (NEEDED)             Shared library: [libround.so]
>    0x000000000000001d (RUNPATH)            Library runpath: [$ORIGIN]
>   $ readelf -d hw4 | grep -E 'NEEDED|RUNPATH|RPATH'
>    0x0000000000000001 (NEEDED)             Shared library: [libtemp.so]
>    0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
>    0x000000000000001d (RUNPATH)            Library runpath: [$ORIGIN/lib]
>   ```
>
> - `RPATH` 的另一個缺點，(b) 實測：設 `LD_LIBRARY_PATH=elsewhere`，`LD_DEBUG` 顯示找 `libtemp.so` 時**第一個**查的還是 `(RPATH from file ./hw4)`——`RPATH` 排在 `LD_LIBRARY_PATH` 前面（第五節的搜尋順序第 1 步），使用者想臨時換一版函式庫測試也蓋不過它；`RUNPATH` 排在 `LD_LIBRARY_PATH` 後面，就沒這個問題。
### 4-3. 先猜再實驗：把連結 `hw4` 的指令拿掉 `-Wl,-rpath-link,lib`，重新連結，貼上輸出。`hw4_main.c` 根本沒呼叫 `temp_round`，為什麼 `ld` 在**連結時**就要去找 `libround.so`？`-rpath-link` 跟 `-rpath` 差在哪？【`-rpath-link` 的作用見筆記[第七節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#七本課用到的工具選項與完整清單在哪)〈`ld`〉的表；`ld` 為什麼要在連結時檢查是新觀念，先猜】

**第 1 次（2026-10-28）**
```
 應該跟沒有幫libtemp.so做runpath的情況一樣,找不到libround.so
gcc -Wall -shared ctof.c ftoc.c -Llib -lround -o ./lib/libtemp.so 
gcc -Wall hw4_main.c -Llib -ltemp -Wl,-rpath,'$ORIGIN/lib' -o hw4 
/usr/bin/ld: warning: libround.so, needed by lib/libtemp.so, not found (try using -rpath or -rpath-link)
/usr/bin/ld: lib/libtemp.so: undefined reference to `temp_round'
collect2: error: ld returned 1 exit status
似乎變成了在連接的時候就出錯了,而不是執行時發現找不到libround.so

我猜ld不是幫hw4_main.c找,而是幫libtemp.so找,感覺ld在連結的時候不會只連結hw4_main.c與libtemp.so,而是也會去看libtemp.so的依賴libround.so
所以這個rpath-link應該就是告訴ld libround.so的資料夾位置
rpath 是執行檔的依賴位置
```

> **批改**：**大致對**。我照你的兩行指令重做，輸出跟你貼的一模一樣（`warning: libround.so, needed by lib/libtemp.so, not found` ＋ `lib/libtemp.so: undefined reference to 'temp_round'`）。預測「跟沒有 `RUNPATH` 一樣找不到」方向對，而且你注意到這次是**連結時**就出錯，不是執行時。
> - **「為什麼連結時要找」（新觀念，先猜，不計對錯）**：你猜「`ld` 不是幫 `hw4_main.c` 找，而是幫 `libtemp.so` 找，也會去看 `libtemp.so` 的依賴」——方向完全對。補教：做執行檔時 `ld` 預設要確認每個 `.so` **自己缺的符號**都有人提供（`--no-allow-shlib-undefined`）；`libtemp.so` 有 `U temp_round`，所以 `ld` 要把它的 `NEEDED` `libround.so` 找來看。第二行錯誤的開頭是 `lib/libtemp.so:`，就是在說「`libtemp.so` 缺的」。現在不查，這個洞要到執行時才爆。另外 `-Llib` 明明寫了也沒用：**`ld` 找間接依賴時不看 `-L`**。這段補教連同實驗寫進了筆記第九節第 13 章〈[連結時 `ld` 為什麼要找「依賴的依賴」：`-rpath-link` 與 `-rpath`（作業 4-3）](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#13-連結時-ld-為什麼要找依賴的依賴-rpath-link-與--rpath作業-4-3)〉。
> - **`-rpath-link` 跟 `-rpath` 的差別（第七節 `ld` 表教過）缺一半**：你寫「`-rpath-link` 告訴 `ld` `libround.so` 在哪」對；「`-rpath` 是執行檔的依賴位置」太含糊，沒講出差別的核心——**`-rpath-link` 只在連結時給 `ld` 用、不寫進輸出檔；`-rpath` 寫進輸出檔（`RUNPATH`／`RPATH`），給執行時的 `ld.so` 用**。實測只加 `-rpath-link` 的執行檔，`readelf -d` 看不到任何路徑，執行時連 `libtemp.so` 都找不到。
>
> **參考答案**【`-rpath-link`：筆記[第七節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#七本課用到的工具選項與完整清單在哪)〈[`ld`：本課透過 `-Wl,` 用到的選項](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#ld本課透過--wl-用到的選項)〉的表；為什麼連結時要找（補教）：第九節第 13 章〈[連結時 `ld` 為什麼要找「依賴的依賴」](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#13-連結時-ld-為什麼要找依賴的依賴-rpath-link-與--rpath作業-4-3)〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 拿掉後**連結時**就失敗：`warning: libround.so, needed by lib/libtemp.so, not found` 和 `lib/libtemp.so: undefined reference to 'temp_round'`。
> 2. （新觀念，先猜）`ld` 做執行檔時要確認每個 `.so` 自己未定義的符號都找得到（`libtemp.so` 的 `U temp_round`），所以要找 `libtemp.so` 的 `NEEDED` `libround.so`；找間接依賴時不看 `-L`。
> 3. `-rpath-link`：只給連結時的 `ld` 找間接依賴，**不寫進輸出檔**；`-rpath`：**寫進輸出檔**成為 `RUNPATH`／`RPATH`，給執行時的 `ld.so` 用。
>
> **詳細說明**：
> - 實測（筆記第 13 章，`examples/rpathlink/`）：
>
>   ```text
>   $ gcc -Wall main.c -Llib -ltemp -Wl,-rpath-link,lib -o app && echo 連結成功
>   連結成功
>   $ readelf -d app | grep -E 'NEEDED|RUNPATH|RPATH'
>    0x0000000000000001 (NEEDED)             Shared library: [libtemp.so]
>    0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
>   $ ./app
>   ./app: error while loading shared libraries: libtemp.so: cannot open shared object file: No such file or directory
>   ```
>
> - `-rpath` 在連結時也會被 `ld` 拿來找間接依賴（`man ld` 的搜尋順序第 2 步）：`-Wl,-rpath,lib` 不加 `-rpath-link` 也連結成功。但第 4 題用的 `-rpath,'$ORIGIN/lib'` 在連結時沒被用上（本機實測，原因沒查證），所以才要另外寫 `-rpath-link`。
> - 用 4-2 (a) 的修法（`libtemp.so` 自己帶 `RUNPATH [$ORIGIN]`）時，`ld` 會看那個 `.so` 自己的 `RUNPATH`（搜尋順序第 6 步），連結時也不必再加 `-rpath-link`——實測連結成功、執行也成功。
---

## 5. 函式庫更新：靜態 vs 動態，以及 SONAME

在 `hw5/` 裡。`temp.h`、`ctof.c`、`ftoc.c` 跟第 1 題一樣，但**`round.c` 先故意寫錯**（無條件捨去，不是四捨五入）：

```c
#include "temp.h"

int temp_round(int x10)
{
    return x10 / 10;
}
```

`hw5_main.c`：

```c
#include <stdio.h>
#include "temp.h"

int main(void)
{
    printf("f_to_c(100) = %d\n", f_to_c(100));
    return 0;
}
```

做出靜態版與帶 SONAME 的動態版，各連結一支程式：

```sh
gcc -Wall -fPIC -c ctof.c ftoc.c round.c
ar rcs libtemp.a ctof.o ftoc.o round.o
gcc -shared -Wl,-soname,libtemp.so.1 ctof.o ftoc.o round.o -o libtemp.so.1.0.0
ln -sf libtemp.so.1.0.0 libtemp.so.1
ln -sf libtemp.so.1 libtemp.so
gcc -Wall hw5_main.c -L. -l:libtemp.a -o hw5_static
gcc -Wall hw5_main.c -L. -ltemp -Wl,-rpath,'$ORIGIN' -o hw5_dyn
```

### 5-1. 執行兩支程式，貼上輸出（100°F 正確是 37.8°C，四捨五入應該是 38）。貼上 `ls -l libtemp*`、`readelf -d hw5_dyn | grep -E 'NEEDED|RUNPATH'`。`ld` 照 `-ltemp` 打開的是 `libtemp.so`，`NEEDED` 卻寫 `libtemp.so.1`，為什麼？【筆記[第六節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#六換掉-so-不用重新連結以及-soname)〈SONAME〉】

**第 1 次（2026-10-28）**
```
./hw5_static
f_to_c(100) = 37

./hw5_dyn
f_to_c(100) = 37

-rw-rw-r-- 1 set set  4330 Oct 28 17:17 libtemp.a
lrwxrwxrwx 1 set set    12 Oct 28 17:18 libtemp.so -> libtemp.so.1
lrwxrwxrwx 1 set set    16 Oct 28 17:18 libtemp.so.1 -> libtemp.so.1.0.0
-rwxrwxr-x 1 set set 15392 Oct 28 17:17 libtemp.so.1.0.0

0x0000000000000001 (NEEDED)             Shared library: [libtemp.so.1]
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
 0x000000000000001d (RUNPATH)            Library runpath: [$ORIGIN]

因爲solname是libtemp.so.1
```

> **批改**：**對**。我在你的 `hw5/` 看過：兩個輸出、`ls -l` 的三個名字、`NEEDED [libtemp.so.1]` 都對。
> - 抓到：`NEEDED` 寫 `libtemp.so.1` 是因為那個 `.so` 的 SONAME 是 `libtemp.so.1`。
> - 小修（不扣分）：把因果講完整會更好——SONAME 是做 `.so` 時用 `-Wl,-soname,libtemp.so.1` **寫進 `.so` 檔案裡**的（`readelf -d libtemp.so.1.0.0 | grep SONAME` 看得到）；`ld` 照 `-ltemp` 打開 `libtemp.so`、一路跟著符號連結讀到 `libtemp.so.1.0.0`，寫進 `NEEDED` 的是**檔案裡的 SONAME**，不是它打開的檔名。所以執行時只需要 `libtemp.so.1` 這個名字，`libtemp.so` 只在連結時用（5-2 換成 `1.0.1` 不用重新連結，靠的就是這一點）。另外是 SONAME，不是 solname。
>
> **參考答案**【筆記第六節〈[SONAME：`NEEDED` 記的是哪個名字](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#sonameneeded-記的是哪個名字)〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 兩支都印 `f_to_c(100) = 37`（`round.c` 故意無條件捨去）。
> 2. `ld` 照 `-ltemp` 打開的是連結名稱 `libtemp.so`（→ `libtemp.so.1` → `libtemp.so.1.0.0`），但寫進 `NEEDED` 的是那個檔案裡記的 **SONAME** `libtemp.so.1`（`-Wl,-soname` 寫進去的），不是打開的檔名。
>
> **詳細說明**：三個名字各有用途——`libtemp.so.1.0.0` 是真實檔案；`libtemp.so.1` 是 SONAME，執行時 `ld.so` 找它；`libtemp.so` 是連結名稱，只給 `-ltemp` 用。沒有 SONAME 的 `.so`（例如第四節的 `libcalc.so`），`ld` 才退而把檔名寫進 `NEEDED`。
### 5-2. 先預測：把 `round.c` 改成正確的四捨五入（`return (x10 + 5) / 10;`），然後**只更新函式庫、不重新連結兩支程式**：

```sh
gcc -Wall -fPIC -c round.c
ar rcs libtemp.a round.o
gcc -shared -Wl,-soname,libtemp.so.1 ctof.o ftoc.o round.o -o libtemp.so.1.0.1
ln -sf libtemp.so.1.0.1 libtemp.so.1
```

`./hw5_static`、`./hw5_dyn` 各會印出多少？寫下預測後執行，貼上輸出。為什麼兩支結果不同？`hw5_static` 要怎樣才能拿到修正？`ar rcs libtemp.a round.o` 之後 `ar t libtemp.a` 有幾個成員，為什麼？【筆記第六節〈換掉 `.so`：程式不用重新連結〉、第二節的 `ar` 文法（`r`）】

**第 1 次（2026-10-28）**
```
預測 .a 會是37, .so是38

./hw5_static
f_to_c(100) = 37

./hw5_dyn
f_to_c(100) = 38

hw5_static需要重編, 否則就一直是舊的

3個成員,本來就只有ctof, ftoc round
```

> **批改**：**不完整**。我在你的 `hw5/` 執行，`hw5_static` 37、`hw5_dyn` 38，跟你貼的一樣；預測兩個都對。
> - **抓到的**：輸出與預測；`hw5_static` 要重新產生才拿得到修正；`ar t` 有 3 個成員。
> - **缺的 1：「為什麼兩支結果不同」沒回答**。要講出機制：`hw5_static` 連結時 `ld` 把 `libtemp.a` 裡的 `round.o` 等成員**複製進執行檔**，之後 `libtemp.a` 怎麼改都跟它無關；`hw5_dyn` 的執行檔裡**沒有** `temp_round` 的程式碼，只記了 `NEEDED libtemp.so.1`，每次執行時 `ld.so` 去載入 `libtemp.so.1`——它現在指向 `libtemp.so.1.0.1`，所以拿到新版。新版的 SONAME 還是 `libtemp.so.1`，`NEEDED` 對得上，所以不用重新連結。
> - **缺的 2：`ar t` 為什麼是 3 個**。「本來就只有 ctof、ftoc、round」沒解釋「再放一次 `round.o` 為什麼沒變成 4 個」：`ar` 的 **`r`（replace）遇到同名成員就取代**，不是加一份（第二節 `ar rcs` 的文法逐字母說明）。
> - 小修（不扣分）：`hw5_static` 是要「**重新連結**」（再跑一次 `gcc -Wall hw5_main.c -L. -l:libtemp.a -o hw5_static`），不必重新編譯 `hw5_main.c`；它的機器碼沒變，變的是要被複製進來的 `round.o`。
>
> **參考答案**【筆記第六節〈[換掉 `.so`：程式不用重新連結](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#換掉-so程式不用重新連結)〉；`ar` 的 `r`：第二節〈[實驗：做出 `libcalc.a`](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#實驗做出-libcalca)〉的 `ar rcs` 文法說明、第七節〈[`ar`：全部選項](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#ar全部選項ar---help-實測gnu-ar-242)〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `hw5_static` 還是 37、`hw5_dyn` 變成 38。
> 2. 靜態：連結時 `round.o` 的程式碼已被**複製進**執行檔，換掉 `libtemp.a` 影響不到它；動態：執行檔裡沒有那段程式碼，每次執行由 `ld.so` 載入 `libtemp.so.1`，它現在指向 `1.0.1`（SONAME 沒變，所以不用重新連結）。
> 3. `hw5_static` 要用新的 `libtemp.a` **重新連結**才拿得到修正。
> 4. `ar t` 還是 3 個：`r` 遇到同名成員 `round.o` 就**取代**，不會多加一份。
>
> **詳細說明**：
> - 實測（scratchpad 重做，先用舊的 `round.c`）：
>
>   ```text
>   $ ar tv libtemp.a                 # 舊版
>   rw-r--r-- 0/0   1384 Jan  1 08:00 1970 ctof.o
>   rw-r--r-- 0/0   1400 Jan  1 08:00 1970 ftoc.o
>   rw-r--r-- 0/0   1256 Jan  1 08:00 1970 round.o
>   $ gcc -Wall -fPIC -c round.c; ar rcs libtemp.a round.o     # 改成四捨五入後
>   $ ar tv libtemp.a
>   rw-r--r-- 0/0   1384 Jan  1 08:00 1970 ctof.o
>   rw-r--r-- 0/0   1400 Jan  1 08:00 1970 ftoc.o
>   rw-r--r-- 0/0   1264 Jan  1 08:00 1970 round.o          ← 同一個位置，大小 1256 → 1264，被取代了
>   $ cp libtemp.a dup.a; ar q dup.a round.o; ar t dup.a     # 對照：q（quick append）不檢查同名，直接加在最後
>   ctof.o
>   ftoc.o
>   round.o
>   round.o
>   $ ./hw5_static                   # 沒重新連結
>   f_to_c(100) = 37
>   $ gcc -Wall hw5_main.c -L. -l:libtemp.a -o hw5_static && ./hw5_static
>   f_to_c(100) = 38
>   ```
>
>   （`ar tv` 的時間都是 1970，是 Ubuntu 的 `ar` 預設 `D`（deterministic，決定性）模式，把時間、擁有者都寫成 0，讓同樣的輸入做出一模一樣的 `.a`。）
> - `nm hw5_static | grep temp_round` 看得到 `T temp_round`：函式本身就在執行檔的 `.text` 裡；`nm -D hw5_dyn` 只會看到 `U f_to_c`，程式碼在 `.so` 裡。
> - 這就是第六節說的「最大的好處，也是代價」：動態的修正一換就全部生效，但不相容的改變也會一起生效——5-3 的 SONAME 換號就是為了處理後者。
### 5-3. 先預測：下一版把 `f_to_c` 改成回傳「十分之一度」（不相容的改變），新增 `ftoc2.c`，做成 SONAME 是 `libtemp.so.2` 的新版，並把連結名稱指到新版：

```c
#include "temp.h"

int f_to_c(int f)
{
    return (f - 32) * 100 / 18;
}
```

```sh
gcc -Wall -fPIC -c ftoc2.c
gcc -shared -Wl,-soname,libtemp.so.2 ctof.o ftoc2.o round.o -o libtemp.so.2.0.0
ln -sf libtemp.so.2.0.0 libtemp.so.2
ln -sf libtemp.so.2 libtemp.so
gcc -Wall hw5_main.c -L. -ltemp -Wl,-rpath,'$ORIGIN' -o hw5_dyn2
```

**不重新連結**舊的 `hw5_dyn`，`./hw5_dyn` 和 `./hw5_dyn2` 各印出多少？寫下預測後執行，貼上輸出、`ls -l libtemp*` 與兩支程式的 `readelf -d … | grep NEEDED`。為什麼兩個版本可以同時存在、舊程式沒有被新版弄壞？如果 5-3 偷懶、新版沿用 SONAME `libtemp.so.1` 並把 `libtemp.so.1` 指過去，`hw5_dyn` 會印出什麼？（這一問只要推理，不用做。）【筆記第六節〈SONAME〉最後一段】

**第 1 次（2026-10-28）**
```
預測 38, 377
./hw5_dyn
f_to_c(100) = 38

./hw5_dyn2
f_to_c(100) = 377

-rw-rw-r-- 1 set set  4338 Oct 28 22:48 libtemp.a
lrwxrwxrwx 1 set set    12 Oct 28 23:13 libtemp.so -> libtemp.so.2
lrwxrwxrwx 1 set set    16 Oct 28 22:34 libtemp.so.1 -> libtemp.so.1.0.1
-rwxrwxr-x 1 set set 15392 Oct 28 17:17 libtemp.so.1.0.0
-rwxrwxr-x 1 set set 15392 Oct 28 22:36 libtemp.so.1.0.1
lrwxrwxrwx 1 set set    16 Oct 28 23:12 libtemp.so.2 -> libtemp.so.2.0.0
-rwxrwxr-x 1 set set 15392 Oct 28 23:12 libtemp.so.2.0.0

readelf -dW hw5_dyn
0x0000000000000001 (NEEDED)             Shared library: [libtemp.so.1]
0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]

readelf -dW hw5_dyn2
0x0000000000000001 (NEEDED)             Shared library: [libtemp.so.2]
0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]

因爲兩個程式NEEDED的庫不同,不會被新版弄壞的

377
```

> **批改**：**對**。我在你的 `hw5/` 執行，`hw5_dyn` 38、`hw5_dyn2` 377，`ls -l` 與兩個 `NEEDED` 都跟你貼的一樣；預測兩個都對。「偷懶」那一問我也在 scratchpad 實際做了（新版沿用 SONAME `libtemp.so.1`、把 `libtemp.so.1` 指過去），`hw5_dyn` 印 `f_to_c(100) = 377`，結束狀態 0，跟你推的一樣。
> - 抓到：兩支程式的 `NEEDED` 不同（`libtemp.so.1` vs `libtemp.so.2`），所以各自載入各自的版本；偷懶時舊程式會印 377。
> - 小修（不扣分）：
>   1. 「為什麼能同時存在」可以多講一層：**SONAME 換了號，兩版就是不同的檔名**（`libtemp.so.1` → `1.0.1`、`libtemp.so.2` → `2.0.0`），可以並排放在同一個資料夾；改掉的只有連結名稱 `libtemp.so`，它只影響**之後的連結**（`hw5_dyn2` 連到 `.so.2`），已經連結好的 `hw5_dyn` 的 `NEEDED` 寫死 `libtemp.so.1`，執行時照樣找到舊版。
>   2. 「377」最好附上理由並點出危險：`ld.so` 只比對**名字** `libtemp.so.1`，對得上就載入，不會檢查函式的意義變了；舊程式照「回傳攝氏度」的理解去用，把 377 當成 377°C——**沒有任何錯誤訊息、結束狀態也是 0**，靜靜地算錯。這就是「不相容就要換 SONAME」的理由。
>
> **參考答案**【筆記第六節〈[SONAME：`NEEDED` 記的是哪個名字](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#sonameneeded-記的是哪個名字)〉最後一段】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `hw5_dyn` 印 38、`hw5_dyn2` 印 377；`NEEDED` 分別是 `libtemp.so.1`、`libtemp.so.2`。
> 2. 不相容的新版換了 SONAME，兩版是不同的檔名可以並存；舊程式的 `NEEDED` 寫死 `libtemp.so.1`，執行時仍載入 `1.0.1`；連結名稱 `libtemp.so` 改指新版只影響之後新連結的程式。
> 3. 偷懶沿用 `libtemp.so.1`：`hw5_dyn` 會載入新版、印 377，而且不會有任何錯誤（`ld.so` 只比對名字）。
>
> **詳細說明**：
> - 偷懶版實測（scratchpad，複製你的 `hw5_dyn` 與 `.o`）：
>
>   ```text
>   $ gcc -shared -Wl,-soname,libtemp.so.1 ctof.o ftoc2.o round.o -o libtemp.so.1.9.9
>   $ ln -sf libtemp.so.1.9.9 libtemp.so.1
>   $ ./hw5_dyn; echo "exit=$?"
>   f_to_c(100) = 377
>   exit=0
>   ```
>
> - 5-2 和 5-3 合起來就是 SONAME 的規則：**相容的修正**（5-2 的四捨五入）沿用 SONAME、只換真實檔名（`1.0.0` → `1.0.1`），舊程式不用重新連結就拿到修正；**不相容的改變**（5-3 的單位改了）換 SONAME（`.so.1` → `.so.2`），舊程式繼續用舊版，新程式連結到新版。系統上常看到同一個函式庫的兩個大版本並存（例如兩個不同大版本號的 `.so`），就是這個原因。