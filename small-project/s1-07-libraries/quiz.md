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
| [第 7 課：靜態與動態函式庫](#已答第-7-課靜態與動態函式庫) | [靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html) | 已答、已批改（2026-10-29） |
| [第 7 課追加題：第 13 題（10-29 課後討論第 15 章）](#未答第-7-課追加題第-13-題10-29-課後討論第-15-章) | [靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#自我測驗) | 未答 |
---

## 已答：第 7 課：靜態與動態函式庫

> 題目來自 [靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html)。
> 每題拆成小題（`N-M`），每個小題各有一個作答框；題目用到的程式碼附在每個大題的開頭。批改寫在該小題的作答框正下方。

### 批改結果（2026-10-29）

> 41 個小題：對 22、大致對 5、不完整 8、錯 6。不重答，每題直接附參考答案（核心關鍵＋詳細說明）。以大題計：第 1、5、7 題對；第 3、4 題大致對，第 2、9 題不完整，第 6、8、10、11、12 題錯。**還沒進複習排程**（2026-10-29 新規則）：不是「對」的 19 個小題要在各自的自我練習框練到 Claude 判定算對，連同追加的第 13 題都過關後，整份才進排程（第 1、5、7 題 → 盒子 2，其餘 → 盒子 1）。

| 題目 | 結果 | 錯在哪／缺什麼 | 過關 |
|---|---|---|---|
| 1-1 | 對 | 抓到重點：`main` 要 `trim` → 拉 `trim.o`，`trim.o` 要 `is_space` → 拉 `space.o`，`case.o` 沒人要 | — |
| 1-2 | 對 | 抓到重點：命令列直接寫的 `.o` 一律整個放進去，`case.o` 的兩個函式也進來 | — |
| 2-1 | 對 | 抓到重點：遇到 `libtxt.a` 時清單是空的、一個成員都不拉，之後不回頭 | — |
| 2-2 | 對 | 抓到重點：一個是檔案本身找不到，一個是檔案找到了但符號沒被解決 | — |
| 2-3 | 不完整 | 沒講出原因：Ubuntu 的 gcc 預設傳 `--as-needed`，遇到 `-ltxt` 時還沒人需要它，`.so` 被丟掉 | ✅ 過關（10-30） |
| 3-1 | 對 | 抓到重點：失敗，錯誤提到 `hits`（沒 `-fPIC` 時用 `%rip` 加固定距離直接讀） | — |
| 3-2 | 對 | 抓到重點：`misses` 是 `static`，只有本檔用得到，距離固定 | — |
| 3-3 | 大致對 | 理由一（符號插入，最後用的可能是執行檔那份）對；理由二「規則簡單」不是，缺「不能改 `.text`：唯讀、改了就不能共用」 | 未過關 |
| 4-1 | 對 | 抓到重點：`ld.so` 印的；連結時命令列給了位置，執行時只拿到名字、只找預設路徑 | — |
| 4-2 | 大致對 | 指令對；單引號的理由沒答出：雙引號裡 shell 還是會展開 `$ORIGIN`（變成空字串） | 未過關 |
| 4-3 | 大致對 | 結論對（`RUNPATH` 換得掉、`RPATH` 換不掉）；原因沒答出：搜尋順序 `RPATH` → `LD_LIBRARY_PATH` → `RUNPATH` | 未過關 |
| 5-1 | 對 | 抓到重點：`NEEDED` 記 SONAME `libcalc.so.1`，`libcalc.so` 只在連結時用 | — |
| 5-2 | 對 | 抓到重點：把 `libcalc.so.1` 指到 `.1.0.1`；靜態的要重新連結 | — |
| 5-3 | 對 | 抓到重點：SONAME 改成 `libcalc.so.2`，舊程式的 `NEEDED` 還是 `.so.1` | — |
| 6-1 | 對 | 抓到重點：下一條指令 `0x1121` 到 GOT 裡 `total` 那格，`0x1121 + 0x2eaf = 0x3fd0` | — |
| 6-2 | 對 | 抓到重點：目標格子固定，`%rip` 是下一條指令、位置不同，距離就不同 | — |
| 6-3 | 大致對 | `0x3fd0` 是 GOT 那格、裝 `total` 的位址、`total` 在 `.bss` 都對；`-0x48` 不知道：它只是用附近符號標位置（`0x4018 − 0x48`） | 未過關 |
| 6-4 | 錯 | 把 `%rax` 說成 `0x3fd0`（格子的位址），差了一層：`mov` 是**讀** `0x3fd0` 那格，`%rax` ＝ `total` 的位址；`1121` 讀到 `total` 的值 | 未過關 |
| 6-5 | 對 | 抓到重點：那格要等執行時由 `ld.so` 填，而且 `total` 最後可能是執行檔那份 | — |
| 7-1 | 對 | 抓到重點：`ld` 只把 `.so` 記成需求單，不複製它的定義 | — |
| 7-2 | 對 | 抓到重點：`.o` 整個放進輸出檔，兩份定義在同一個檔裡 | — |
| 7-3 | 對 | 抓到重點：印 3，`ld.so` 先找執行檔，把執行檔的 `&verbose` 填進 `libconf.so` 的 GOT | — |
| 7-4 | 對 | 抓到重點：印 0，`static` 不經 GOT、直接用 `%rip` 加固定距離讀自己那份 | — |
| 8-1 | 對 | 抓到重點：PLT 是跳板程式碼；位址存在 `.got.plt` 的 `0x4008`，`0x107a + 0x2f8e` | — |
| 8-2 | 對 | 抓到重點：第一次經過叫 `ld.so` 的程式碼、`ld.so` 把位址寫回格子；第二次直接跳 | — |
| 8-3 | 錯 | 理由不對：延遲綁定一樣照「先執行檔」找；真正原因是讀變數只是一條 `mov`，中間沒有跳板可以插入「先去查」 | 未過關 |
| 9-1 | 不完整 | 「不是兩份、`.text` 共用、`.got`／`.bss` 各一份」對；錯：`hits` 沒有被複製進 `x`、`y` 的 `.bss`；缺：`&hit` 不同是因為載入基底不同（ASLR） | 未過關 |
| 9-2 | 大致對 | 5、各自一份都對；但 `hits` 在 `libhits.so` 的 `.bss`（各自一份是因為寫入時複製），不是在執行檔的 `.bss`（跟 9-1 同一處） | 未過關 |
| 9-3 | 不完整 | 方向對（各行程要的內容不同，`.text` 就不能同一份），但沒用寫入時複製說明：`ld.so` 一寫，核心就替該行程複製那頁 | 未過關 |
| 10-1 | 錯 | 說成一樣；實際不一樣：那格是「這個行程的載入基底 ＋ `0x1119`」，兩個行程基底不同。`objdump -s` 看到的 `0x1030` 是檔案裡的初值 | 未過關 |
| 10-2 | 不完整 | 「是」對；證據沒答出：`smaps` 裡 `r-xp` 那段兩個行程都是 `Shared_Clean:4`、`Pss:2` | 未過關 |
| 10-3 | 不完整 | 只答「kernel」：頁表是核心準備的對，但每次翻譯的是 CPU 的 MMU | 未過關 |
| 10-4 | 不完整 | 「gdb 下基底一樣」對到一半；缺：兩個行程各有位址空間和頁表，同一個虛擬位址不代表同一個東西。「同時啟動」不是解法 | 未過關 |
| 11-1 | 對 | 抓到重點：3 與 7 | — |
| 11-2 | 不完整 | 會看到對；區段錯：你貼的 `nm` 是 `B` ＝ `.bss`，不是 `.data`；原因只猜對一半：`ld` 只留位置、寫 `R_X86_64_COPY`，初值是載入時才複製 | 未過關 |
| 11-3 | 不完整 | 「`get_level` 讀主程式那份」對；誰在何時填 GOT 說錯：是 `ld.so` 載入時、不是 `ld` 連結時；主程式讀 `level` 不經 GOT；`COPY` 沒講 | 未過關 |
| 11-4 | 錯 | 猜成「改用 GOT」；實際：執行檔已經照舊大小留了 4 byte，`ld.so` 警告 `Symbol 'level' has different size in shared object, consider re-linking` | 未過關 |
| 12-1 | 錯 | 說成執行階段；其實是前處理：`fatal error: temp.h: No such file or directory`，要補 `-Iinclude` | 未過關 |
| 12-2 | 錯 | 說成執行階段；其實是連結：`undefined reference to 'c_to_f'`，要補 `-ltemp` | 未過關 |
| 12-3 | 對 | 抓到重點：`temp.h` 的內容在編 `main.c` 時就進了執行檔；精確說巨集是前處理換掉的 | — |
| 12-4 | 對 | 抓到重點：`.h` 只在編譯時用，`.so` 執行時要載入 | — |

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

**第 1 次（2026-10-29）**
```
trim, is_space
因爲main需要用trim, 而trim要is_space
```

> **批改**：**對**。抓到重點：只拉「能解決目前未定義符號」的成員，`trim.o` 又帶出 `space.o`，`case.o` 沒人要就不拉。
>
> **參考答案**【[第二節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#連結只拉需要的成員)〈連結：只拉需要的成員〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 執行檔裡只有 `trim` 和 `is_space`。
> 2. `ld` 從 `.a` 只拉能解決「目前還沒定義的符號」的成員：`main` 要 `trim` → 拉 `trim.o`；`trim.o` 要 `is_space` → 再拉 `space.o`；`case.o` 沒人要，整個不拉。
>
> **詳細說明**：拉的單位是**整個 `.o`**：如果有人用到 `to_upper_str`，同在 `case.o` 的 `to_lower_str` 也會跟著進來（筆記的 `both.c`）。成員在封存檔裡的順序不影響：本機實測 `ar t libtxt.a` 是 `case.o space.o trim.o`，`space.o` 排在 `trim.o` 前面，照樣拉得到——`ld` 在**同一個** `.a` 裡會反覆看成員，直到沒有新的符號被解決；「只掃一次、不回頭」是對命令列上前後不同的輸入檔而言（第 2 題）。驗證：`nm app | grep -E 'trim|is_space|to_'` 只列出 `trim`、`is_space`。

#### 1-2. 如果把連結指令改成直接寫三個 `.o`（`gcc main.c case.o trim.o space.o`），結果有什麼不同、為什麼？

**第 1 次（2026-10-29）**
```
執行結果沒有不同,但是看到的符號數量會不同,執行檔大小也不同

原本的方式,連結器會根據你實際用到的符號,從.a裏面挑trim.o, space.o,置於case.o沒有用到就不會被併入執行檔

現在這個指令是要把這4個當值直接打包成執行檔,所以case.o也會進去
```

> **批改**：**對**。抓到重點：執行結果一樣，但 `case.o` 也被放進去，執行檔多了兩個函式、變大。
>
> **參考答案**【[第二節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#連結只拉需要的成員)〈連結：只拉需要的成員〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 執行檔多了 `to_upper_str`、`to_lower_str`（執行結果一樣，檔案變大）。
> 2. 命令列上直接寫的 `.o` 一律整個放進輸出檔，不管有沒有人用；只有封存檔裡的成員才是「有人需要才拉」。
>
> **詳細說明**：本機實測：`nm app2` 四個函式都在；`app` 16136 byte、`app2` 16240 byte。

### 2. 順序：`-ltxt` 寫在 `main.c` 前面

承上題的檔案（`case.c`、`space.c`、`trim.c`、`main.c` 見第 1 題），`gcc -Wall -L. -ltxt main.c -o app` 會失敗，錯誤訊息是 `undefined reference to 'trim'`。用 `ld` 處理輸入檔的方式解釋為什麼。這個錯誤跟 `cannot find -ltxt` 差在哪（各是什麼東西找不到）？在 Ubuntu 上把 `libtxt.a` 換成 `libtxt.so`、命令一樣寫成 `-L. -ltxt main.c`，也會失敗，原因是什麼？

#### 2-1. `gcc -Wall -L. -ltxt main.c -o app` 會失敗，錯誤訊息是 `undefined reference to 'trim'`。用 `ld` 處理輸入檔的方式解釋為什麼。

**第 1 次（2026-10-29）**
```
因爲ld 連接是從命令列的左邊看到右邊, 他先看了要引入libtxt.a,但是這個時候沒有檔案讓他做連結,所以就跳過到main.c,然後main的符號表有trim ,他想去找有trim的庫,但是這個時候已經沒有-L -ltxt了,所以他只能報trim我找不到他還是未定義
```

> **批改**：**對**。抓到重點：由左往右、遇到 `libtxt.a` 時還沒人需要 `trim`，讀到 `main` 之後 `ld` 不會回頭。
>
> **參考答案**【[第二節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#順序ld-由左往右只掃一次)〈順序：`ld` 由左往右只掃一次〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `ld` 照命令列順序處理輸入檔，手上維持一張「還沒找到定義的符號」清單。
> 2. 遇到 `libtxt.a` 時 `main` 的 `.o` 還沒讀，清單裡沒有 `trim`（只有啟動檔帶進來的 `main` 等，筆記第九節第 15 章），一個成員都不拉就略過。
> 3. 讀到 `main` 才多了 `trim`，命令列結束時還在清單上，`ld` 不會回頭重掃 `libtxt.a` → `undefined reference`。
>
> **詳細說明**：規則：函式庫寫在用到它的檔案後面（`main.c -L. -ltxt`）。函式庫之間互相需要時，寫兩次或用 `-Wl,--start-group … -Wl,--end-group`。

#### 2-2. 這個錯誤跟 `cannot find -ltxt` 差在哪（各是什麼東西找不到）？

**第 1 次（2026-10-29）**
```
cannot find -ltxt 是ld在搜尋libtxt.a這個庫的時候,在搜尋資料夾找不到與上一題的情況不同
上一題是有找到,但順序錯了,導致main後來找不到trim的定義
```

> **批改**：**對**。抓到重點：`cannot find` 是函式庫檔案本身找不到；這題是找到了，但順序讓 `trim` 沒被解決。
>
> **參考答案**【[第三節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#三-l-與--l連結時去哪裡找找什麼名字)、[第二節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#順序ld-由左往右只掃一次)〈順序：`ld` 由左往右只掃一次〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `cannot find -ltxt`：連結時在搜尋目錄裡找不到 `libtxt.so`／`libtxt.a` **這個檔案**（例如沒給 `-L.`、名字拼錯）。
> 2. `undefined reference to 'trim'`：檔案找到了，但 `trim` 這個**符號**到最後都沒有被解決（這題是因為順序，一個成員都沒拉）。
>
> **詳細說明**：兩者都是連結時 `ld` 的錯。對照第五節的 `cannot open shared object file`：那是**執行時** `ld.so` 找不到 `.so` 檔，第 4 題會用到。

#### 2-3. 在 Ubuntu 上把 `libtxt.a` 換成 `libtxt.so`、命令一樣寫成 `-L. -ltxt main.c`，也會失敗，原因是什麼？

**第 1 次（2026-10-29）**
```
轉換成shared object,不代表ld找符號連結的順序有改變,.a與.so的差別只在 前者是把要用到的.o合併到執行檔,後者不合並,只是找到對應符號的定義,並且寫到dynamic section,給未來執行是interpreter用
```

> **批改**：**不完整**。「還是順序問題」的方向對，但原因沒講出來：`ld` 讀到 `.so` 時其實會記住它全部的符號，單純由左往右並不會讓 `.so` 失敗（實測加 `-Wl,--no-as-needed` 就成功）；失敗是因為 Ubuntu 的 gcc 預設多傳了 `--as-needed`。
>
> **參考答案**【[第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#so-在-ubuntu-上也有順序問題)〈`.so` 在 Ubuntu 上也有順序問題〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. Ubuntu 的 gcc 預設傳 `--as-needed` 給 `ld`：`.so` 只有在**目前有人需要它的符號**時才寫進 `NEEDED`、才拿來解決符號。
> 2. 遇到 `-ltxt` 時 `main` 還沒讀、沒人需要 `trim`，`libtxt.so` 就被丟掉 → 之後 `main` 的 `trim` 沒人提供 → `undefined reference`。
>
> **詳細說明**：本機實測：`gcc -Wall -fPIC -shared case.c space.c trim.c -o libtxt.so && gcc -Wall -L. -ltxt main.c -o app3` 得到 `undefined reference to 'trim'`；把 `-Wl,--no-as-needed` 放在 `-ltxt` 前面（`gcc -Wall -L. -Wl,--no-as-needed -ltxt main.c -o app4`）就成功，`readelf -d app4` 有 `NEEDED libtxt.so`。`gcc -v` 印出的 `collect2` 那一行看得到 `--as-needed`。所以 `.a` 和 `.so` 一律把 `-l` 寫在後面。你寫的「`.a` 把要用的 `.o` 合併進執行檔、`.so` 只寫進動態區段給執行時用」是對的，只是不是這題的原因。

**自我練習**（不批改）

```
gcc 預設是 --as-needed 意思是,ld掃到-ltxt的時候會看自己手上的清單是否與libtxt的符號是否相符,有的話就把libtxt.so記成needed, 但是沒有所以txt就被丟掉了,然後到main.o發現找不到trim這個符號

但是用--no-as-needed的話,ld督導-ltxt就直接把.so全部的符號加載進來,所以就可以正常發現trim,就可以成功

重點在於命令列是下--as-needed還是--no-as-needed 
順序只與--as-needed有關
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

**第 1 次（2026-10-29）**
```
不會成功

hits, share object file全域變數的存取需要透過got取得他的位址,沒加-fPIC 組合語言的寫法是直接把他當作固定位址,算出與rip的距離
而不是用got去那hits的變數
```

> **批改**：**對**。抓到重點：失敗、提到 `hits`，原因是沒加 `-fPIC` 時直接用 `%rip` 加固定距離讀它。
>
> **參考答案**【[第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#為什麼一定要--fpic先看不加會怎樣)〈為什麼一定要 `-fPIC`：先看不加會怎樣〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 不會成功。
> 2. 錯誤提到 `hits`：`relocation R_X86_64_PC32 against symbol 'hits' can not be used when making a shared object; recompile with -fPIC`。
>
> **詳細說明**：本機實測輸出：`warning: relocation against 'hits' in read-only section '.text'`、`relocation R_X86_64_PC32 against symbol 'hits' can not be used when making a shared object; recompile with -fPIC`、`final link failed: bad value`。沒加 `-fPIC`（Ubuntu 預設 `-fPIE`）時，編譯器假設 `hits` 就在附近、用 `R_X86_64_PC32` 直接讀；做 `.so` 時 `ld` 不接受對全域符號用這種重定位。

#### 3-2. 為什麼另一個變數不會有問題？

**第 1 次（2026-10-29）**
```
misses他是靜態變數,只有stats能用,他與rip的距離是固定的
```

> **批改**：**對**。抓到重點：`static` → 只有本檔用得到，一定是 `.so` 自己那份，距離固定。
>
> **參考答案**【[第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#為什麼不能直接用固定距離counter-最後不一定是這一份)〈為什麼不能直接用固定距離：`counter` 最後不一定是這一份〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `misses` 是 `static`（`LOCAL`），別的檔案不可能用到它、也不可能用同名定義插入。
> 2. 所以它一定是這個 `.so` 自己那一份，跟程式碼在同一個 `.so` 裡、距離固定，`%rip` 加固定距離就讀得到，不需要 GOT。
>
> **詳細說明**：筆記的 `count_static.c`（`static int counter`）不加 `-fPIC` 也做得成 `.so`，是同一個道理。

#### 3-3. 為什麼 `.so` 存取全域變數一定要經過 GOT，而不能像執行檔那樣用 `%rip` 加固定距離直接讀（講出兩個理由）？

**第 1 次（2026-10-29）**
```
因爲全域變數今天不一定是在.so的 .bss/.data裏, 固定相對距離可能不適用
1. 假如執行檔也有定義全域變數,interpreter在載入的時候,定義選擇順序會是 執行檔 > .so,所以這個時候全域變數會存在執行檔的.bss/.data,本來執行檔分配的記憶體位址根據ASLR本就不固定,那.so到全域變數的距離將不會是固定(每次),所以interpreter會在.got存入當前全域變數的位址,.so只要算出他距離這個符號在.got的距離就好

2. 規則簡單,組合語言好寫,我們不要管全域變數在哪了,只要是shared object file的全域變數,不管是so本身還是其他,統一用got的邏輯,這樣不需要擔心相對距離的問題,因爲每個檔案自己到自己got的距離一定是固定的, 只需要在動態加載的時候決定全域到底要存在誰的.bss/.data
```

> **批改**：**大致對**。第 1 點對：全域變數最後用的可能是執行檔那份（`ld.so` 先找執行檔），做 `.so` 時不知道距離——這是主因。第 2 點「規則簡單、組語好寫」不是筆記的第二個理由；缺的是：為什麼不乾脆讓 `ld.so` 載入時把正確距離寫進 `.text`？因為 `.text` 唯讀，而且一寫就要每個行程各複製一份，`.so` 的「共享」就沒了。另外，ASLR 本身不是原因：`counter` 跟程式碼在同一個 `.so` 裡時，載入到哪裡距離都不變。
>
> **參考答案**【[第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#為什麼不能直接用固定距離counter-最後不一定是這一份)〈為什麼不能直接用固定距離：`counter` 最後不一定是這一份〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 符號插入：`.so` 的全域變數最後被使用的可能不是它自己那份（執行檔或先載入的 `.so` 有同名定義，`ld.so` 先找執行檔），做 `.so` 時不知道它在哪 → 只能等載入時由 `ld.so` 把真正位址填進 GOT。
> 2. 不能讓 `ld.so` 載入時直接改 `.text` 裡的距離：`.text` 是唯讀的，而且每個行程寫進自己的位址就得各複製一份，同一份 `.text` 實體頁給所有行程共用的好處就沒了；所以把要因行程而異的位址集中在可寫的 GOT。
>
> **詳細說明**：正文〈為什麼不能直接用固定距離〉的 `usecount` 實驗：主程式自己定義 `counter = 100`，`libcount.so` 的 `bump()` 加的是主程式那份，印出 `counter = 101`。第二個理由在同一節〈那為什麼不讓 `ld.so` 載入時直接改 `.text` 裡的距離就好？〉那段，第 9 題會再考到。

### 4. 情境：連結成功、執行找不到 `libfoo.so`

情境題：你在 `/home/me/proj/` 下做了 `lib/libfoo.so` 和用它的 `app`，連結指令是 `gcc main.c -Llib -lfoo -o app`。連結成功，`./app` 卻印出 `error while loading shared libraries: libfoo.so: cannot open shared object file`。(1) 這個訊息是誰印的？為什麼連結時找得到、執行時找不到？(2) 你想讓 `app` 和 `lib/` 一起打包給同事，同事解壓到任何地方都能直接執行，連結指令要怎麼改？為什麼要用單引號？(3) 同事想臨時用 `LD_LIBRARY_PATH` 換成他自己編的 `libfoo.so` 來除錯，在你的做法（`RUNPATH`）下換得掉嗎？如果是 `RPATH` 呢？為什麼？

#### 4-1. 這個訊息是誰印的？為什麼連結時找得到、執行時找不到？

**第 1 次（2026-10-29）**
```
這是interpreter報的
連接時命令列直接之名庫的位置,執行時interpreter從執行檔只拿到庫名稱,他只能從預設的搜尋路徑找,找不到
```

> **批改**：**對**。抓到重點：`ld.so`（interpreter）印的；`-Llib` 只在連結時有用，執行時 `ld.so` 只照自己的搜尋順序找。
>
> **參考答案**【[第五節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#ldso-去了哪些地方找)〈`ld.so` 去了哪些地方找〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 是動態連結器 `ld.so`（執行檔的 interpreter）在執行時印的。
> 2. `-Llib` 只告訴**連結時**的 `ld` 去哪找，不會寫進執行檔；執行檔的 `NEEDED` 只記名字 `libfoo.so`。
> 3. 執行時 `ld.so` 只找 `RPATH`、`LD_LIBRARY_PATH`、`RUNPATH`、`/etc/ld.so.cache`、預設目錄，`lib/`（和當前目錄）都不在裡面。
>
> **詳細說明**：`LD_DEBUG=libs ./app` 會印出它試過的每個路徑，最後放棄。

#### 4-2. 你想讓 `app` 和 `lib/` 一起打包給同事，同事解壓到任何地方都能直接執行，連結指令要怎麼改？為什麼要用單引號？

**第 1 次（2026-10-29）**
```
gcc main.c -Llib -lfoo -Wl,-rpath,'$ORIGIN/lib' -o app

我不清楚,雙引號不行嗎?
```

> **批改**：**大致對**。指令完全正確。「雙引號不行嗎」——不行：雙引號裡 shell 照樣展開 `$` 變數，`$ORIGIN` 沒有這個 shell 變數，就變成空字串。本機實測 `gcc main.c -L. -ltxt -Wl,-rpath,"$ORIGIN/lib" -o app5`，`readelf -d app5` 的 `RUNPATH` 是 `[/lib]`。
>
> **參考答案**【[第五節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#方法二-wl-rpath把路徑寫進執行檔)〈方法二：`-Wl,-rpath`——把路徑寫進執行檔〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `gcc main.c -Llib -lfoo -Wl,-rpath,'$ORIGIN/lib' -o app`：把 `$ORIGIN/lib` 寫進執行檔的 `RUNPATH`，`ld.so` 執行時把 `$ORIGIN` 換成執行檔所在的目錄。
> 2. `$ORIGIN` 不是 shell 變數，要**原封不動**寫進執行檔給 `ld.so` 換；不用單引號（包括用雙引號）時 shell 會先把 `$ORIGIN` 換成空字串，寫進去的就變成 `/lib`。
>
> **詳細說明**：單引號裡 shell 什麼都不展開；一定要用雙引號時寫成 `"\$ORIGIN/lib"`。用 `readelf -d app | grep RUNPATH` 檢查實際寫進去的是什麼。只要 `app` 和 `lib/` 的相對位置不變，解壓到哪裡都找得到。

#### 4-3. 同事想臨時用 `LD_LIBRARY_PATH` 換成他自己編的 `libfoo.so` 來除錯，在你的做法（`RUNPATH`）下換得掉嗎？如果是 `RPATH` 呢？為什麼？

**第 1 次（2026-10-29）**
```
可以,我的做法就是runpath
rpath不行

不確定原因
```

> **批改**：**大致對**。兩個結論都對，原因寫「不確定」：關鍵是 `ld.so` 的搜尋順序。
>
> **參考答案**【[第五節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#rpath-和-runpath-差在哪跟-ld_library_path-誰優先)〈`RPATH` 和 `RUNPATH` 差在哪：跟 `LD_LIBRARY_PATH` 誰優先〉、[第五節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#ldso-去了哪些地方找)〈`ld.so` 去了哪些地方找〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `RUNPATH` 換得掉：`ld.so` 的搜尋順序裡 `LD_LIBRARY_PATH`（第 2 步）排在 `RUNPATH`（第 3 步）前面，先找到同事自己的版本。
> 2. `RPATH` 換不掉：`RPATH`（第 1 步，只在沒有 `RUNPATH` 時用）排在 `LD_LIBRARY_PATH` 前面，先找到原本 `lib/` 裡的版本。
>
> **詳細說明**：筆記的 `v1`／`v2` 實驗：`LD_LIBRARY_PATH=v2 ./app_runpath` 印 1005（用了 v2），`./app_rpath` 印 5（還是 v1）。`RUNPATH` 是故意排在後面的：讓使用者能臨時換版本來除錯；所以 `ld` 預設寫 `RUNPATH`，加 `--disable-new-dtags` 才寫舊式的 `RPATH`，`man ld.so` 也標明 `DT_RPATH` 不建議使用。`LD_DEBUG=libs` 每段 `search path=` 後面的括號會寫來源（`(LD_LIBRARY_PATH)`、`(RPATH from file …)`）。

### 5. SONAME 與函式庫更新

你維護 `libcalc`，SONAME 是 `libcalc.so.1`。系統上有 `libcalc.so -> libcalc.so.1`、`libcalc.so.1 -> libcalc.so.1.0.0` 兩個符號連結。(1) 一支程式動態連結了 `libcalc`，它的 `NEEDED` 寫的是哪個名字？為什麼刪掉 `libcalc.so` 它照樣能跑，刪掉 `libcalc.so.1` 就不行？(2) 你修了一個 bug（介面不變），做成 `libcalc.so.1.0.1`。已經動態連結的程式要怎樣才能用到修正？另一支用 `libcalc.a` 靜態連結的程式呢？(3) 下一版要把 `calc_add` 改成三個參數（不相容），SONAME 該怎麼辦？為什麼這樣舊程式不會壞？

#### 5-1. 一支程式動態連結了 `libcalc`，它的 `NEEDED` 寫的是哪個名字？為什麼刪掉 `libcalc.so` 它照樣能跑，刪掉 `libcalc.so.1` 就不行？

**第 1 次（2026-10-29）**
```
libcalc.so.1
因爲執行檔的需求單不是寫libcalc.so,刪了也沒關係

但是刪了libcalc.so.1,interpreter就找不到這個庫的連接了
```

> **批改**：**對**。抓到重點：`NEEDED` 是 `libcalc.so.1`；執行時找的是它，`libcalc.so` 刪了無所謂。
>
> **參考答案**【[第六節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#sonameneeded-記的是哪個名字)〈SONAME：`NEEDED` 記的是哪個名字〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `NEEDED` 寫的是 SONAME `libcalc.so.1`：`ld` 照 `-lcalc` 打開 `libcalc.so`，但記下的是那個檔案裡寫的 SONAME，不是檔名。
> 2. `libcalc.so`（連結名稱）只在連結時給 `-lcalc` 用；執行時 `ld.so` 找的是 `libcalc.so.1`，所以刪掉它就找不到。
>
> **詳細說明**：這就是開發套件（`zlib1g-dev`，有 `libz.so`）和執行套件（`zlib1g`，只有 `libz.so.1`）分開的原因：只裝執行套件，程式能跑，但不能 `-lz` 連結。

#### 5-2. 你修了一個 bug（介面不變），做成 `libcalc.so.1.0.1`。已經動態連結的程式要怎樣才能用到修正？另一支用 `libcalc.a` 靜態連結的程式呢？

**第 1 次（2026-10-29）**
```
把libcalc.so.1的軟連接改連到libcalc.so.1.0.1

靜態連接只能重新編譯,因爲原本的.o都已經包到執行檔了
```

> **批改**：**對**。抓到重點：換 SONAME 連結指向的真實檔，舊程式不用動；靜態的程式碼已經在執行檔裡。小修：靜態那支精確說是「重新連結」，`main.c` 不必重編。
>
> **參考答案**【[第六節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#sonameneeded-記的是哪個名字)〈SONAME：`NEEDED` 記的是哪個名字〉、[第六節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#換掉-so程式不用重新連結)〈換掉 `.so`：程式不用重新連結〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 動態：SONAME 不變（`libcalc.so.1`），把 `libcalc.so.1` 改指向 `libcalc.so.1.0.1`；程式不必重新連結，重新執行就用到修正。
> 2. 靜態：`libcalc.a` 的 `.o` 已經複製進執行檔，換函式庫影響不到它，要用新的 `libcalc.a` 重新連結。
>
> **詳細說明**：`ldconfig` 會照各 `.so` 的 SONAME 自動建立 SONAME 符號連結。已經在跑的行程要重開才會載入新檔。

#### 5-3. 下一版要把 `calc_add` 改成三個參數（不相容），SONAME 該怎麼辦？為什麼這樣舊程式不會壞？

**第 1 次（2026-10-29）**
```
建一個新的so檔,如libcalc.so.2.0.0 soname用libcalc.so.2, 一樣建立軟連接
ln -sf libcalc.so.2.0.0 libcalc.so.2

然後爲了讓ld找到依賴庫
ln -sf libcalc.so.2 libcalc.so

然後在編譯新的執行檔 這樣相同的編譯命令一樣可以連到libcalc.so.2.0.0

而舊的執行檔只看dynamic section needed還是libcalc.so.1所以他還是可以找到舊的
```

> **批改**：**對**。抓到重點：新 SONAME `libcalc.so.2`、三個名字照樣建，舊程式照 `NEEDED` 找 `.so.1`。
>
> **參考答案**【[第六節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#sonameneeded-記的是哪個名字)〈SONAME：`NEEDED` 記的是哪個名字〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 新版 SONAME 改成 `libcalc.so.2`（`-Wl,-soname,libcalc.so.2`，真實檔例如 `libcalc.so.2.0.0`，`libcalc.so.2` 指向它，`libcalc.so` 改指向新版給新程式連結）。
> 2. 舊程式的 `NEEDED` 寫的是 `libcalc.so.1`，執行時照樣找到舊檔（舊檔留著、兩版並存），不會拿到參數不相容的新版；重新連結的新程式才記下 `libcalc.so.2`。
>
> **詳細說明**：反過來，如果不相容卻沒換 SONAME，舊程式會載入新版、用兩個參數呼叫三個參數的 `calc_add`，第三個參數是暫存器裡的垃圾值。

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

**第 1 次（2026-10-29）**
```
是從下一行rip的位址到total在got的位址

0x1121 + 0x2eaf = 0x3fd0
```

> **批改**：**對**。抓到重點：起點是下一條指令 `0x1121`，終點是 GOT 那格，算式正確。
>
> **參考答案**【[第九節第 1 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#1-0x2ec8-和-0x3fd0-各是什麼mov-0x2ec8riprax-的兩個數字)〈`0x2ec8` 和 `0x3fd0` 各是什麼：`mov 0x2ec8(%rip),%rax` 的兩個數字〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 從**下一條指令**（`0x1121`）到 GOT 裡 `total` 那一格的距離。
> 2. `0x1121 ＋ 0x2eaf ＝ 0x3fd0`。
>
> **詳細說明**：公式是第 4 課的 `D(%rip)`：位址 ＝ 下一條指令的位址 ＋ D。

#### 6-2. `112b` 那一行的 `D` 是 `0x2e9e`，跟 `0x2eaf` 不一樣，為什麼算出來還是 `3fd0`？

**第 1 次（2026-10-29）**
```
total 在 got的位址是固定的,也就是3fd0, 但是$rip是會變得,所以相對距離自然會隨着$rip變動而變,rip是指下一行命令的位址
```

> **批改**：**對**。抓到重點：目標都是 `0x3fd0`，`%rip`（下一條指令）不同所以 `D` 不同。可以再補算式 `0x1132 ＋ 0x2e9e ＝ 0x3fd0`。
>
> **參考答案**【[第九節第 1 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#1-0x2ec8-和-0x3fd0-各是什麼mov-0x2ec8riprax-的兩個數字)〈`0x2ec8` 和 `0x3fd0` 各是什麼：`mov 0x2ec8(%rip),%rax` 的兩個數字〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 兩行都是去 `0x3fd0` 那一格，但 `%rip` 是各自的下一條指令：`112b` 的下一條是 `0x1132`。
> 2. 起點不同，到同一格的距離自然不同：`0x1132 ＋ 0x2e9e ＝ 0x3fd0`。
>
> **詳細說明**：同一個函式裡每次拿 `total` 的位址，`D` 都不一樣、算出來都是同一格，這正好證明 `0x3fd0` 是位址、`D` 是距離。

#### 6-3. 註解 `<total-0x48>` 是說 `total` 放在 `0x3fd0` 嗎？`0x3fd0` 是什麼、`total` 本身在哪？

**第 1 次（2026-10-29）**
```
我不確定0x48的意思

total不是放在0x3fdo, 0x3fdo是在.got的區段裏,0x3fd0存的那格是total本體的位址
total本身在.bss這個區段裏,當然也有它自己的位址,而這個位址的值就存在0x3fd0
```

> **批改**：**大致對**。「不是放在 `0x3fd0`、`0x3fd0` 是 GOT 裡裝 `total` 位址的那格、`total` 本身在 `.bss`」都對。缺的是註解本身的意思：GOT 的格子沒有自己的名字，`objdump` 拿附近的符號來標位置。
>
> **參考答案**【[第九節第 1 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#1-0x2ec8-和-0x3fd0-各是什麼mov-0x2ec8riprax-的兩個數字)〈`0x2ec8` 和 `0x3fd0` 各是什麼：`mov 0x2ec8(%rip),%rax` 的兩個數字〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 不是：`<total-0x48>` 只是位置標籤，意思是「`total` 往前 `0x48`」：`0x4018 − 0x48 ＝ 0x3fd0`。GOT 的格子沒有自己的符號名，`objdump` 只好找附近的符號來標。
> 2. `0x3fd0` 是 `.got` 裡的一格，裡面裝的是 `total` 的位址（`readelf -r` 那行 `R_X86_64_GLOB_DAT … total`）。
> 3. `total` 本身在 `.bss` 的 `0x4018`（`readelf -r` 的 `Sym. Value`）。
>
> **詳細說明**：標籤跟「這格裝什麼」沒關係，只是剛好這個 `.so` 很小、最近的符號就是 `total`。真正說明那格裝什麼的是 `readelf -r`：第一欄 `3fd0` 是要被填的格子，`GLOB_DAT` 是「填一個符號的位址」，`total` 是填誰。

#### 6-4. 執行完 `111a` 那一行，`%rax` 裡裝的是什麼？`1121` 讀到的又是什麼？

**第 1 次（2026-10-29）**
```
就是0x3fd0(當然實際執行要加上基準點的位址)也就是total的got位址

(rax) 是去讀0x3fd0這格存的值,也就是total真正的位址
```

> **批改**：**錯**。你把兩條指令都往前推了一層。`mov 0x2eaf(%rip),%rax` 的來源 `0x2eaf(%rip)` 有括號、是**記憶體**：它去 `0x3fd0` 那格**讀 8 byte**，不是把 `0x3fd0` 這個數字放進 `%rax`（只算位址不讀記憶體的是 `lea`，第 4 課）。
>
> **參考答案**【[第九節第 1 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#1-0x2ec8-和-0x3fd0-各是什麼mov-0x2ec8riprax-的兩個數字)〈`0x2ec8` 和 `0x3fd0` 各是什麼：`mov 0x2ec8(%rip),%rax` 的兩個數字〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `111a` 去 `0x3fd0` 那格讀 8 byte，所以 `%rax` 裝的是格子的**內容**——`total` 的位址（載入時 `ld.so` 填的）。
> 2. `1121` 的 `mov (%rax),%rdx` 照這個位址再讀 8 byte，讀到的是 **`total` 的值**。
>
> **詳細說明**：兩條指令＝兩次讀記憶體：先讀 GOT 拿到位址，再照位址讀變數。對照第 4 課〈[`lea`：只算位址，不碰記憶體](../../html/c/x86-64組合語言入門-暫存器、mov與lea、cmp與條件跳躍.html#lea只算位址不碰記憶體)〉：如果寫成 `lea 0x2eaf(%rip),%rax`，`%rax` 才會是 `0x3fd0`（加上載入基底）這個格子的位址。筆記第 1 章用 gdb 實測：`libcount.so` 的 GOT 那格（`0x7ffff7fb6000 + 0x3fd0`）裡裝的是 `0x7ffff7fba00c`，正是 `&counter`。

#### 6-5. 為什麼用 `objdump -s -j .got libtally.so` 看，`0x3fd0` 那格在檔案裡是 0？

**第 1 次（2026-10-29）**
```
tally.so:     file format elf64-x86-64

Contents of section .got:
 3fb8 00000000 00000000 00000000 00000000  ................
 3fc8 00000000 00000000 00000000 00000000  ................
 3fd8 00000000 00000000 00000000 00000000  ...............

因爲決定.got到底要存 total記憶體位址的決定是在執行時,interpreter決定的,全域變數也有可能存在執行檔的main檔
```

> **批改**：**對**。抓到重點：要等載入時 `ld.so` 決定、填進去；最後用的可能不是 `.so` 自己那份。
>
> **參考答案**【[第九節第 1 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#1-0x2ec8-和-0x3fd0-各是什麼mov-0x2ec8riprax-的兩個數字)〈`0x2ec8` 和 `0x3fd0` 各是什麼：`mov 0x2ec8(%rip),%rax` 的兩個數字〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 檔案裡那格還沒填：要等**載入時** `ld.so` 照 `R_X86_64_GLOB_DAT total` 把 `total` 的真正位址寫進去。
> 2. 做 `.so` 時填不了：載入基底每次不同，而且最後用的 `total` 可能是別人那份（符號插入）。
>
> **詳細說明**：你貼的 `.got` 全是 0 就是證據；用 gdb 停在函式裡看同一格，就會看到填好的位址。

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

**第 1 次（2026-10-29）**
```
不會,會出現這個錯誤是因爲ld連結時,某符號重複定義
而.so裏的定義是到執行的時候由interpreter動態載入,並且會以執行檔優先,然後纔是.so
ld只是把.so先寫成需求單
```

> **批改**：**對**。抓到重點：`multiple definition` 是 `ld` 檢查放進同一個輸出檔的定義；`.so` 只被記成需求單。
>
> **參考答案**【[第九節第 2 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#2-兩份-counter接-so-為什麼不算重複定義)〈兩份 `counter`：接 `.so` 為什麼不算重複定義〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 不會。
> 2. `ld` 不把 `.so` 的內容複製進輸出檔，只讀它的動態符號表（解決未定義符號）並記進 `NEEDED`；輸出檔裡只有 `app.c` 那份 `verbose`，`multiple definition` 只檢查放進同一個輸出檔的定義。
>
> **詳細說明**：連結成功後兩份 `verbose` 都還在，各在各的檔案裡，要到執行時才由 `ld.so` 挑一份。

#### 7-2. 改成 `gcc -Wall app.c conf.pic.o -o app2` 呢？差別在 `ld` 怎麼處理 `.so` 和 `.o`。

**第 1 次（2026-10-29）**
```
那就不行了,ld 會看到兩個定義同時存在
```

> **批改**：**對**。抓到重點：會出錯；配合 7-1 的「`.so` 只是需求單」，差別講到了。
>
> **參考答案**【[第九節第 2 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#2-兩份-counter接-so-為什麼不算重複定義)〈兩份 `counter`：接 `.so` 為什麼不算重複定義〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 會出現 `multiple definition of 'verbose'`。
> 2. `.o` 的內容會被整個**複製進**輸出檔，`app.c` 和 `conf.pic.o` 的 `verbose` 都進了 `app2`，同一個輸出檔裡兩份定義。
>
> **詳細說明**：同樣兩份定義，接 `.o` 是連結錯誤、接 `.so` 是執行時的符號插入。

#### 7-3. 能連結成功的那一個，`./app` 會印出 `get_verbose() = ` 多少？用執行時 `ld.so` 怎麼找符號解釋。

**第 1 次（2026-10-29）**
```
get_verbose() = 3

ld.so 依據需求載入ldconf.so,並依據符號表做符號連結, 然後他會按照先執行檔 後 ldconf.so的順序,決定verbose要用誰的定義,然後決定用執行檔的定義,就會把執行檔定義的verbose位址寫入到ld.so的.got裏
```

> **批改**：**對**。抓到重點：印 3；先找執行檔、再找 `.so`，`libconf.so` 的 GOT 填執行檔那份的位址。小筆誤：「寫入到 `ld.so` 的 `.got`」應是 `libconf.so` 的 GOT；`ldconf.so` 應是 `libconf.so`。
>
> **參考答案**【[第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#為什麼不能直接用固定距離counter-最後不一定是這一份)〈為什麼不能直接用固定距離：`counter` 最後不一定是這一份〉、[第九節第 2 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#2-兩份-counter接-so-為什麼不算重複定義)〈兩份 `counter`：接 `.so` 為什麼不算重複定義〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 印 `get_verbose() = 3`。
> 2. `ld.so` 找符號**先找執行檔、再依 `NEEDED` 順序找各個 `.so`**，第一個找到的贏（符號插入）；所以 `libconf.so` 的 GOT 裡 `verbose` 那格填的是執行檔那份的位址，`get_verbose` 經過 GOT 讀到 3。
>
> **詳細說明**：`LD_DEBUG=bindings ./app` 會看到 `binding file …/libconf.so to ./app: normal symbol 'verbose'`。`libconf.so` 自己那份（初值 0）一直在記憶體裡，只是沒人用。

#### 7-4. 把 `conf.c` 第一行改成 `static int verbose = 0;`，重做 `libconf.so`，不重新連結就執行 `./app`，印出多少？為什麼跟 7-3 不同？

**第 1 次（2026-10-29）**
```
印出 0
因爲這次ldconf.so並不會通過got去找verbose
由於他已經宣告了verbose是靜態變數了,他會直接算出rip與.bss裏verbose的距離直接拿來讀取
```

> **批改**：**對**。抓到重點：`static` → 不經 GOT、直接讀 `.so` 自己的 `verbose`。
>
> **參考答案**【[第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#為什麼不能直接用固定距離counter-最後不一定是這一份)〈為什麼不能直接用固定距離：`counter` 最後不一定是這一份〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 印 `get_verbose() = 0`。
> 2. `static` 讓 `verbose` 變成 `LOCAL`，別人不可能插入：編譯器直接用 `%rip` 加固定距離讀 `.so` 自己那份，不經過 GOT，`ld.so` 也不會把它接到執行檔的 `verbose`。
>
> **詳細說明**：執行檔不用重新連結也會變：`get_verbose` 的程式碼在 `.so` 裡，換掉 `.so` 就換了讀法。

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

**第 1 次（2026-10-29）**
```
1. 不是.plt是一個程式碼,讓代碼往log_msg的.got.plt跳
2. 就存在.got.plt的區段,0x4008存的那格
0x2f8e + 0x107a= 0x4008
```

> **批改**：**對**。抓到重點：跳板是程式碼、位址在 `.got.plt` 的 `0x4008`，算式正確。
>
> **參考答案**【[第九節第 4 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#4-plt-存函式位址got-存變數位址嗎plt-是跳板以及延遲綁定)〈PLT 存函式位址、GOT 存變數位址嗎？PLT 是跳板，以及延遲綁定〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 不是：`log_msg@plt` 是**程式碼**（`.plt.sec` 裡的跳板，權限 `AX`），不存位址。
> 2. `log_msg` 的位址存在 `.got.plt`（`0x3fe8` 起）的 `0x4008` 那格（`R_X86_64_JUMP_SLOT`）。
> 3. `jmp *0x2f8e(%rip)` 的下一條是 `0x107a`：`0x107a ＋ 0x2f8e ＝ 0x4008`。
>
> **詳細說明**：`*` 表示「跳到記憶體裡存的那個位址」。函式和變數的位址都在 GOT 裡，只是函式那些放在 `.got.plt`。

#### 8-2. 第一次和第二次 `call 1070 <log_msg@plt>`，從 `call` 到進入 `log_msg` 各經過哪些地方？`ld.so` 在哪一次介入、做了什麼？

**第 1 次（2026-10-29）**
```
第一次: call -> log_msg@plt -> .got.plt -> 呼叫ld.so的程式碼-> ld.so把log_msg的位址存到.got.plt -> 進入log_msg
第二次: call -> log_msg@plt -> .got.plt -> log_msg

在第一次介入,本來.got.plt那格存的是call ld.so載入的程式碼位址,ld.so載入log_msg後把log_msg的位址存到.got.plt
```

> **批改**：**對**。抓到重點：兩條路徑都對，`ld.so` 只在第一次介入、把位址寫回 `.got.plt`。小用詞：`ld.so` 不是「載入 `log_msg`」——`liblogger.so` 早就載入了，它做的是**查**出 `log_msg` 的位址（基底 ＋ `0x1139`）。
>
> **參考答案**【[第九節第 4 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#4-plt-存函式位址got-存變數位址嗎plt-是跳板以及延遲綁定)〈PLT 存函式位址、GOT 存變數位址嗎？PLT 是跳板，以及延遲綁定〉、[第九節第 8 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#8-ldso-填進格子的是什麼這個行程裡的虛擬位址)〈`ld.so` 填進格子的是什麼：這個行程裡的虛擬位址〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 第一次：`call` → `log_msg@plt` 跳板 → `jmp *(0x4008 那格)`，那格載入時填的是 `.plt` 裡「叫 `ld.so` 去查」的程式碼 → `ld.so` 查出 `log_msg` 的位址、寫回 `0x4008`、跳進 `log_msg`。
> 2. 第二次：`call` → 跳板 → 照 `0x4008`（已經是 `log_msg` 的位址）直接跳進去，`ld.so` 不再介入。
>
> **詳細說明**：這叫延遲綁定（lazy binding）：好處是啟動時不必查完所有函式。跳板只是照格子跳，不判斷格子「空不空」。

#### 8-3. 全域變數為什麼不能用這種「第一次用到才查」的方式，一定要載入時就把 `.got` 那格填好？

**第 1 次（2026-10-29）**
```
因爲如果用plt的方式,那就代表全域變數一定是用.so的全域變數,否則就沒必要用plt的方式
這與原本C的設計,全域變數的選擇順序是執行檔 > so違背
```

> **批改**：**錯**。延遲綁定並不代表「一定用 `.so` 那份」：第一次呼叫時 `ld.so` 一樣照「先執行檔、再 `.so`」找（筆記第 4 章的 `greet_app`，`libgreet.so` 呼叫的 `say` 就接到主程式那份）。問題在**機制**：函式能延遲，是因為呼叫前一定先經過跳板。
>
> **參考答案**【[第九節第 4 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#4-plt-存函式位址got-存變數位址嗎plt-是跳板以及延遲綁定)〈PLT 存函式位址、GOT 存變數位址嗎？PLT 是跳板，以及延遲綁定〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 延遲綁定靠跳板：呼叫函式時一定先經過 PLT 這段程式碼，第一次才有機會轉去叫 `ld.so` 查。
> 2. 讀變數就是一條 `mov`（從 GOT 拿位址、再照位址讀），中間沒有程式碼可以攔下來「先去查」，所以 `.got` 裡變數那格一定要在載入時就填好。
>
> **詳細說明**：如果 `.got` 那格沒填，`mov (%rax),…` 會直接拿檔案裡的 0 當位址去讀，當場 segfault。

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

**第 1 次（2026-10-29）**
```
1. 不是,程式碼是同一份.但是根據實驗,interpreter在做載入的時候會copy一份hits在執行檔的.bss, 所以這個hits 各自存在 x與y各自的.bss裏面
2. .text是共用的,但是.bss, .got是分開的 各自有各自的
```

> **批改**：**不完整**。對的部分：不是兩份程式碼、`.text` 共用、`.got` 和 `.bss` 各一份。兩個問題：(1) 「interpreter 把 `hits` 複製到執行檔的 `.bss`」是錯的——複製重定位（第 3 章）只發生在**主程式自己用到那個變數**時（`extern int level;` 而且讀寫它），`x.c`、`y.c` 只呼叫 `hit()`，沒碰 `hits`。本機實測 `nm x` 沒有 `hits`；`readelf -r x` 只有 `R_X86_64_GLOB_DAT … hit`，那是 `(void *)hit` 取函式位址用的。`hits` 一直在 `libhits.so` 自己的 `.bss`，每個行程一份是因為寫入時複製。(2) 題目第一問的關鍵沒講：`&hit` 為什麼不一樣。
>
> **參考答案**【[第九節第 6 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#6-兩支程式都用-libcountso記憶體裡是同一份嗎)〈兩支程式都用 `libcount.so`，記憶體裡是同一份嗎？〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 不是兩份程式碼：`&hit` 不同是因為兩個行程把 `libhits.so` 載入在不同的基底（ASLR），虛擬位址不同，但對到同一個實體頁（尾巴都是檔案裡的 `0x10f9`）。
> 2. 共用同一份實體記憶體的是沒被寫過的頁：`.text`（`r-xp`，`smaps` 是 `Shared_Clean`）、唯讀資料。
> 3. 各自一份的是被寫過的頁：`.got`（`ld.so` 載入時寫）和 `.data`／`.bss`（`hits` 被寫）——私有映射一寫就寫入時複製（`Private_Dirty`）。
>
> **詳細說明**：本機實測：`x: &hit=0x7ace937850f9  hit() 回傳 2`、`y: &hit=0x713b714f10f9  hit() 回傳 5`。`x`、`y` 是兩個獨立的行程，各有自己的位址空間；`libhits.so` 映射進來時是私有映射（`smaps` 權限欄的 `p`），沒人寫的頁大家共用，誰寫了核心就替誰複製一份。

#### 9-2. `y` 印出的 `hit()` 回傳值是多少？為什麼不是 `x`、`y` 加起來的 7？

**第 1 次（2026-10-29）**
```
是5

因爲兩個行程 各自儲存hits在各自的.bss裏面,互不影響
```

> **批改**：**大致對**。5 對、「兩個行程各自一份、互不影響」對；位置說錯了（跟 9-1 同一處）：那份 `hits` 在 `libhits.so` 的 `.bss`，不在 `x`、`y` 執行檔的 `.bss`。
>
> **參考答案**【[第九節第 6 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#6-兩支程式都用-libcountso記憶體裡是同一份嗎)〈兩支程式都用 `libcount.so`，記憶體裡是同一份嗎？〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 印 5。
> 2. `hits` 那一頁（`libhits.so` 的 `.bss`）每個行程各一份：`hit()` 一寫就觸發寫入時複製，`x` 改的是 `x` 的那頁、`y` 改的是 `y` 的那頁，所以 `x` 算到 2、`y` 算到 5，不會加成 7。
>
> **詳細說明**：共用的只有沒被寫過的頁。要讓兩個行程真的共用一個計數器，得用共享記憶體（例如 `mmap` 的 `MAP_SHARED`）這類明確的機制，函式庫本身做不到。

#### 9-3. 假設沒有 GOT，改成讓 `ld.so` 載入時把位址直接寫進 `.text`，用「寫入時複製」說明 `.text` 還能不能共用、為什麼。

**第 1 次（2026-10-29）**
```
不行, 假如.text還是共用的, 不用got, 兩個行程在.so的rip到各自.bss的距離不可能相同,那就意味着.text 不可能同一份
```

> **批改**：**不完整**。「每個行程要的距離不同，所以不能同一份」方向對，但題目要你用**寫入時複製**說明機制，這部分沒寫：不是「不可能」共用，而是一寫就被核心複製、自動變成各一份。
>
> **參考答案**【[第九節第 6 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#6-兩支程式都用-libcountso記憶體裡是同一份嗎)〈兩支程式都用 `libcount.so`，記憶體裡是同一份嗎？〉、[第四節](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#為什麼不能直接用固定距離counter-最後不一定是這一份)〈為什麼不能直接用固定距離：`counter` 最後不一定是這一份〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `ld.so` 載入時要把**這個行程自己的位址**寫進 `.text` 那一頁。
> 2. `.so` 是私有映射，一寫就觸發寫入時複製：核心替這個行程複製一份那頁 → 每個行程各一份，`.text` 不再共用（何況 `.text` 是唯讀的，還得先改權限）。
>
> **詳細說明**：PIC 的設計正是為了避免這件事：程式碼完全不改、大家共用；要因行程而異的位址集中在可寫的 GOT，只讓 GOT 那一頁被複製。

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

**第 1 次（2026-10-29）**
```
一樣,
objdump -s -j .got.plt libshape.so                                                                           ok | base py | at 17:15:27

libshape.so:     file format elf64-x86-64

Contents of section .got.plt:
 3fe8 483e0000 00000000 00000000 00000000  H>..............
 3ff8 00000000 00000000 30100000 00000000  ........0.......

都是 30100000, 但是我用objdump -s -j .got.plt libshape.so找出來的結果與你的不太相同

理論上我理解.got.plt應該是直接放area的位址,因爲都在同一個檔案,不用像其他檔案需要先呼叫ld.so, 找libshape.so裏area的位址

用那兩樣數字可以算出0x1119?我不確定感覺不用算也知道.建立.so就定了?
```

> **批改**：**錯**。你讀檔案的方法是對的：`0x4000` 那 8 byte 是 `30100000 00000000`，小端序就是 `0x1030`（你說「跟我的不同」，是因為題目沒給檔案內容，`0x1119` 是 `readelf -r` 的符號值）。但 `0x1030` 是**檔案裡的初值**，指向 `.plt` 裡叫 `ld.so` 的那段（本機實測 `1030: endbr64`、`push $0x0`、`jmp 1020`），不是 `area`，也不是執行時的值。題目問的是第一次呼叫**之後**，行程記憶體裡那格的值：每個行程不一樣。另外「同一個檔案所以直接放 `area` 的位址」也不對：`area` 是 `GLOBAL`、可能被插入，`calc` 照樣經過 `area@plt`、延遲綁定（就像第 4 章 `libgreet.so` 的 `say`）。
>
> **參考答案**【[第九節第 7 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#7-函式那邊plt-跳板共用gotplt-每個行程各一份)〈函式那邊：PLT 跳板共用、`.got.plt` 每個行程各一份〉、[第九節第 8 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#8-ldso-填進格子的是什麼這個行程裡的虛擬位址)〈`ld.so` 填進格子的是什麼：這個行程裡的虛擬位址〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 不一樣。
> 2. 那格裝的是**這個行程裡的虛擬位址**：`libshape.so` 在這個行程的載入基底 ＋ `area` 在檔案裡的位置 `0x1119`（`.dynsym`，也就是 `R_X86_64_JUMP_SLOT` 那行的 `0x1119`）。
> 3. 兩個行程的載入基底不同（ASLR），所以數字不同。
>
> **詳細說明**：`0x1119` 確實在做 `.so` 時就定了，但它只是「檔案內的位置」，還要加上基底；基底要到 `ld.so` 把檔案映射進來時才知道，每個行程不同。那格一共被寫兩次：載入時寫「基底 ＋ `0x1030`」，第一次呼叫時改成「基底 ＋ `0x1119`」。筆記第 7 章的 `q`、`r` 實測兩個行程的格子分別是 `0x7187194b9139`、`0x7125ada21139`。

#### 10-2. 兩個行程執行的 `area` 程式碼是同一份實體記憶體嗎？`/proc/<pid>/smaps` 裡哪個欄位、出現什麼數值可以證明？

**第 1 次（2026-10-29）**
```
是

不知道
```

> **批改**：**不完整**。「是同一份」對，題目第二問（`smaps` 的證據）答不知道。
>
> **參考答案**【[第九節第 7 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#7-函式那邊plt-跳板共用gotplt-每個行程各一份)〈函式那邊：PLT 跳板共用、`.got.plt` 每個行程各一份〉、[第九節第 6 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#6-兩支程式都用-libcountso記憶體裡是同一份嗎)〈兩支程式都用 `libcount.so`，記憶體裡是同一份嗎？〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 是同一份實體記憶體。
> 2. `/proc/<pid>/smaps` 裡 `libshape.so` 的 `r-xp`（程式碼）那段，兩個行程都是 `Shared_Clean:4`（共用、沒被寫過），`Pss:2`（4 kB 由兩個行程分攤）。
>
> **詳細說明**：本機實測（`./m & ./n` 同時跑，用筆記第 6 章的 `awk` 印 `smaps`）：兩個行程的 `r-xp 00001000` 都是 `Pss:2 Shared_Clean:4 Private_Clean:0 Private_Dirty:0`；而 `.got` 那段和 `rw-p` 那段是 `Pss:4 … Private_Dirty:4`，各自一份。`Shared` ＝ 還有別的行程映射著這頁、`Private` ＝ 只有自己；`Clean` ＝ 沒被寫過；`Pss` 把共用的頁平均分給共用它的行程，只有一個行程在跑時同一段會是 `Pss:4`、`Private_Clean:4`。

#### 10-3. 跳到那格裡的位址時，虛擬位址由誰翻成實體位址？翻譯用的表又是誰準備的？

**第 1 次（2026-10-29）**
```
kernel
```

> **批改**：**不完整**。核心只對了一半：核心**準備**頁表；**翻譯**是 CPU 的 MMU 在每次存取記憶體、取指令時自動做的，核心不必介入。
>
> **參考答案**【[第九節第 9 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#9-虛擬位址怎麼變成實體位址核心準備頁表cpu-的-mmu-翻譯)〈虛擬位址怎麼變成實體位址：核心準備頁表，CPU 的 MMU 翻譯〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 翻譯由 **CPU 的 MMU** 做：每一次讀寫記憶體、取指令、跳躍，都照目前行程的頁表把虛擬位址翻成實體位址（翻過的結果暫存在 TLB）。
> 2. 頁表由**核心**替每個行程建立、維護（哪個虛擬頁對到哪個實體頁、權限）。
>
> **詳細說明**：核心只在 MMU 翻不下去時介入：頁表裡查不到（頁還沒載入）或權限不符（寫到要做寫入時複製的頁），CPU 觸發分頁錯誤（page fault），交給核心處理。`ld.so`、gdb 都只碰虛擬位址。

#### 10-4. 同學分別用 gdb 跑 `m` 和 `n`，`p area` 兩邊都印 `0x7ffff7fb7119`，他說「看吧，兩個行程的 `area` 在同一個位址」。這個推論哪裡有問題？

**第 1 次（2026-10-29）**
```
可能會有問題,gdb啓動的話用的都是自己的位置baseline都一樣,所以即使相同,也只能說他們分開啓動看起來相同,最好是同時啓動比較能證明
```

> **批改**：**不完整**。「gdb 啟動時 baseline 都一樣」對到一半（gdb 預設關掉 ASLR）。但「同時啟動比較能證明」不對：同時用 gdb 跑也會印一樣的數字，而且不管數字一不一樣，比虛擬位址都證明不了是不是同一份。
>
> **參考答案**【[第九節第 9 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#9-虛擬位址怎麼變成實體位址核心準備頁表cpu-的-mmu-翻譯)〈虛擬位址怎麼變成實體位址：核心準備頁表，CPU 的 MMU 翻譯〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. gdb 預設關掉 ASLR，所以兩次的載入基底一樣，印出同一個數字只是巧合。
> 2. 兩個行程各有自己的位址空間和頁表，虛擬位址只在自己的行程裡有意義；數字一樣不代表是同一個東西或同一個實體頁（反過來，數字不同也可能是同一個實體頁，像 9-1）。
>
> **詳細說明**：要證明兩個行程用的是同一份實體記憶體，看 `smaps` 的 `Shared_Clean`、`Pss`（10-2）。一般程式拿不到實體位址：`/proc/self/pagemap` 沒有 `CAP_SYS_ADMIN` 時 PFN 會被遮成 0（筆記第 9 章 `pfn.c`）。

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

**第 1 次（2026-10-29）**
```
3與7
```

> **批改**：**對**。抓到重點：印 3 和 7。
>
> **參考答案**【[第九節第 3 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#3-主程式只寫-extern-int-counter變數放進執行檔複製重定位)〈主程式只寫 `extern int counter;`：變數放進執行檔（複製重定位）〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 第一行 `一開始 level = 3`，第二行 `get_level() = 7`。
>
> **詳細說明**：主程式和 `get_level()` 用的是同一份 `level`（11-3），所以主程式改成 7，`get_level()` 讀到 7。

#### 11-2. `nm setlevel` 會不會看到 `level`？在哪一種區段？為什麼原始碼只有 `extern` 卻會有？

**第 1 次（2026-10-29）**
```
應該會看到,畢竟在setlevel.c有宣告,我測試一下
0000000000004010 B level 測試的結果有

在.data,因爲 default是3

猜測應該是ld在連接的時候,把libcfg.so裏面的level定義copy到執行檔裏了,放在執行檔自己的.data
```

> **批改**：**不完整**。會看到 `level` 對（你也實測了）。但你自己貼的 `0000000000004010 B level` 是 **`B`**，代表 `.bss`，不是 `.data`；「default 是 3 所以在 `.data`」的推論錯在：執行檔裡這個位置連結時**沒有初值**，3 是執行時才複製進來的。原因猜「`ld` 把定義 copy 到執行檔」對一半：`ld` 只在執行檔留一個同樣大小的位置、寫一個 `R_X86_64_COPY` 重定位，真正複製值的是載入時的 `ld.so`（11-3）。
>
> **參考答案**【[第九節第 3 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#3-主程式只寫-extern-int-counter變數放進執行檔複製重定位)〈主程式只寫 `extern int counter;`：變數放進執行檔（複製重定位）〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 會看到，類型 `B`，在執行檔的 `.bss`。
> 2. 主程式是 `-fPIE`：編譯器假設全域變數都在執行檔自己裡面，用 `%rip` 加固定距離直接讀，不經 GOT。
> 3. 但 `level` 定義在 `.so`，距離不可能固定；`ld` 只好在執行檔的 `.bss` 替它留一個同樣大小的位置，並寫一個 `R_X86_64_COPY` 重定位（複製重定位）。
>
> **詳細說明**：本機實測：`nm setlevel` → `0000000000004010 B level`；`readelf -SW setlevel` 的 `.bss` 從 `0x4010` 開始；`readelf -r setlevel` → `000000004010 … R_X86_64_COPY … level + 0`。

#### 11-3. 第一行印出的值是怎麼放進那裡的？`get_level()` 讀的是哪一份 `level`？用重定位的類型和 `ld.so` 找符號的順序說明。

**第 1 次（2026-10-29）**
```
連接的時候ld把level在執行檔的位址(.data)寫入到liblcfg.so自己的.got,執行檔可能是自己從自己的got找到在.data的位置,然後在跳到那邊把值讀出來

同樣是main copy出來的,
```

> **批改**：**不完整**。結論「`get_level()` 讀的是主程式那份」對。錯的地方：(1) 把 `libcfg.so` 的 GOT 填好的是**執行時的 `ld.so`**，不是連結時的 `ld`——`ld` 連結 `setlevel` 時不會改 `libcfg.so`，而且那時也不知道載入位址；(2) 主程式讀 `level` 不經 GOT，是 `%rip` 加固定距離直接讀自己 `.bss` 那格；(3) 第一行的 3 怎麼來的（`R_X86_64_COPY`）沒講。
>
> **參考答案**【[第九節第 3 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#3-主程式只寫-extern-int-counter變數放進執行檔複製重定位)〈主程式只寫 `extern int counter;`：變數放進執行檔（複製重定位）〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 第一行的 3：載入時 `ld.so` 處理執行檔的 `R_X86_64_COPY`，去 `libcfg.so` 找到 `level`，把初值 3 複製進執行檔 `.bss` 的 `level`；`main` 直接讀它。
> 2. `get_level()` 經過 `libcfg.so` 的 GOT 讀 `level`，那格是 `R_X86_64_GLOB_DAT level`；`ld.so` 照「先找執行檔、再找 `.so`」找到**執行檔那份**，把執行檔的 `&level` 填進去。
> 3. 所以兩邊用的都是執行檔那份；`main` 改成 7，`get_level()` 讀到 7。`libcfg.so` 自己那份 `level` 只在載入時被讀一次初值。
>
> **詳細說明**：本機實測 `LD_DEBUG=bindings ./setlevel` 的兩行就是這兩件事：`binding file ./setlevel to …/libcfg.so: normal symbol 'level'`（處理 `COPY`：去 `.so` 拿初值）、`binding file …/libcfg.so to ./setlevel: normal symbol 'level'`（處理 `GLOB_DAT`：`.so` 的 GOT 接到執行檔）。

#### 11-4. 之後把 `cfg.c` 改成 `long level = 3;` 重做 `libcfg.so`，`setlevel` 不重新連結就執行，會發生什麼事、為什麼？

**第 1 次（2026-10-29）**
```
我不清楚,
我猜測類別換了,變成setlevel的got要去記住level的位址,執行的時候我猜執行檔的got會改放libcfg.so的.data的level
```

> **批改**：**錯**（你寫了不清楚、用猜的）。執行檔沒有重新連結，裡面的程式碼和 `R_X86_64_COPY` 都不會變，不會「改成經過 GOT」。
>
> **參考答案**【[第九節第 3 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#3-主程式只寫-extern-int-counter變數放進執行檔複製重定位)〈主程式只寫 `extern int counter;`：變數放進執行檔（複製重定位）〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 執行時 `ld.so` 印警告 `Symbol 'level' has different size in shared object, consider re-linking`，程式照樣跑。
> 2. 原因：執行檔 `.bss` 留給 `level` 的大小（4 byte）是**連結時**照當時的 `.so` 決定的；現在 `.so` 裡的 `level` 是 8 byte，大小對不上，要重新連結。
>
> **詳細說明**：本機實測（`cfg.c` 改成 `long level = 3;` 重做 `libcfg.so`，`setlevel` 不動）：先印那行警告，接著照樣印 `一開始 level = 3`、`get_level() = 7`。印對是運氣：`get_level()` 經過 GOT 到執行檔那格讀 8 byte，多讀的 4 byte 是執行檔 `.bss` 裡 `level` 後面的空間，剛好是 0；值一大、或後面放了別的變數，就會讀寫到別人。這是函式庫通常**不直接給外面用全域變數、而是提供函式**（`get_level()`）的原因之一：函式走 PLT，執行檔不必知道它的大小。

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

**第 1 次（2026-10-29）**
```
前處理階段
找不到.h

命令行補上 -Iinclude
```

> **批改**：**錯**。這個指令根本走不到執行：`main.c` 第 2 行 `#include "temp.h"`，`temp.h` 在 `include/` 底下，沒給 `-Iinclude`，前處理器就找不到。你補的 `-rpath` 是**之後**才會遇到的問題（實測補了 `-Iinclude` 之後能編、能連結，執行時才出 `cannot open shared object file`），想得到這一步很好，但不是這題問的。
>
> **參考答案**【[第九節第 11 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#11-做-so-需要-h-嗎include-管編譯-l-管連結)〈做 `.so` 需要 `.h` 嗎？`#include` 管編譯、`-l` 管連結〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 失敗在**前處理**：`fatal error: temp.h: No such file or directory`。
> 2. `#include "temp.h"` 先找 `main.c` 所在的目錄、再找 `-I` 指定的目錄；`temp.h` 在 `include/`，要補 `-Iinclude`。
>
> **詳細說明**：本機實測輸出：`main.c:2:10: fatal error: temp.h: No such file or directory`、`compilation terminated.`。四個階段的錯誤長相不同：前處理是 `fatal error: xxx.h`、連結是 `undefined reference`／`cannot find -l`、執行是 `error while loading shared libraries`。

#### 12-2. `gcc -Wall -Iinclude main.c -Llib -o app` 呢？

**第 1 次（2026-10-29）**
```
1. 執行階段,

2. ld.so找不到libtemp.so

3. 改下 gcc -Wall -Iinclude main.c -Llib -ltemp -Wl,-rpath,'$ORIGIN/lib' -o app
```

> **批改**：**錯**。少的是 `-ltemp`（你的修正指令有補上，但階段和錯誤說錯）：有 `-Iinclude`，前處理、編譯都會過（`.h` 只給宣告）；沒有 `-l`，`ld` 根本不會去找 `libtemp.so`，`c_to_f` 沒人提供。
>
> **參考答案**【[第九節第 11 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#11-做-so-需要-h-嗎include-管編譯-l-管連結)〈做 `.so` 需要 `.h` 嗎？`#include` 管編譯、`-l` 管連結〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 失敗在**連結**：`undefined reference to 'c_to_f'`。
> 2. `.h` 只讓編譯器知道 `c_to_f` 的型別；要 `-ltemp`（寫在 `main.c` 後面）`ld` 才會去 `-Llib` 找 `libtemp.so`、解決 `c_to_f`。
>
> **詳細說明**：本機實測輸出：`main.c:(.text+0x15): undefined reference to 'c_to_f'`。對照筆記第 11 章的 `sq.c`：有 `#include <math.h>`、沒 `-lm`，一樣是 `undefined reference to 'sqrt'`。

#### 12-3. 同事說「`FREEZING_C` 的值存在 `libtemp.so` 裡，執行時才讀進來」，對嗎？為什麼？

**第 1 次（2026-10-29）**
```
不對

因爲在編譯階段temp.h已經被執行檔include進來並且編譯了
```

> **批改**：**對**。抓到重點：值在編譯呼叫端時就寫進 `app` 了，跟 `.so` 無關。小修：巨集是在**前處理**階段被換掉的（`gcc -E` 看得到 `c_to_f(0.0)`）。
>
> **參考答案**【[第九節第 11 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#11-做-so-需要-h-嗎include-管編譯-l-管連結)〈做 `.so` 需要 `.h` 嗎？`#include` 管編譯、`-l` 管連結〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. 不對。
> 2. `FREEZING_C` 是巨集，前處理時就被換成 `0.0`，寫進 `main.c` 編出來的程式碼；`libtemp.so` 裡根本沒有這個東西。
>
> **詳細說明**：本機實測：`gcc -E -Iinclude main.c` 的最後幾行是 `printf("%.1f\n", c_to_f(0.0));`。所以改了 `temp.h` 的 `FREEZING_C`，只換 `.so` 沒用，要重新編譯 `main.c`。

#### 12-4. 編好的 `app`（連結時加了 `-Wl,-rpath,'$ORIGIN/lib'`）要拿給別人執行，`include/temp.h` 要一起給嗎？`lib/libtemp.so` 呢？

**第 1 次（2026-10-29）**
```
include/temp.h不用,前處理的時候已經寫了
lib/libtemp.so要,因爲這個要在執行動態載入,call func的時候也需要ld.so補位址在.got.plt
```

> **批改**：**對**。抓到重點：`.h` 不用（編譯時已經用完），`.so` 要（執行時載入、接上函式）。
>
> **參考答案**【[第九節第 11 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#11-做-so-需要-h-嗎include-管編譯-l-管連結)〈做 `.so` 需要 `.h` 嗎？`#include` 管編譯、`-l` 管連結〉】
>
> **核心關鍵**（答到這幾點就算對）：
>
> 1. `include/temp.h` 不用給：它只在編譯呼叫端時用，內容已經編進 `app`。
> 2. `lib/libtemp.so` 要給：`c_to_f` 的程式碼在裡面，執行時 `ld.so` 照 `NEEDED` 和 `RUNPATH $ORIGIN/lib` 載入它，所以要放在 `app` 旁邊的 `lib/`。
>
> **詳細說明**：這就是 `zlib1g`（執行用，只有 `.so.1`）和 `zlib1g-dev`（開發用，`.h`、`.a`、`.so`）分成兩個套件的原因。

---

## 未答：第 7 課追加題：第 13 題（10-29 課後討論第 15 章）

> 題目來自 [靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#自我測驗) 第 13 題，是 10-29 追問「還沒讀到 `main.o` 怎麼看清單」「`--no-as-needed` 是把 `trim` 加進清單嗎」整理成筆記第九節〈課後討論〉[第 15 章](../../html/c/靜態與動態函式庫-ar與.a、-fPIC與.so、執行時怎麼找到.so.html#15-ld-還沒讀到-maino怎麼知道誰需要什麼未定義符號清單與---as-needed) 時新增的。
> 每個小題各有一個作答框，批改寫在作答框正下方。

### 13. `--as-needed`：讀到 `.so` 那一刻的清單

`libtxt.so` 由下面的 `case.c`、`space.c`、`trim.c` 做成；`main.c` 只呼叫 `trim`，`none.c` 什麼都不呼叫。(1) `gcc -v` 顯示 gcc 交給 `ld` 的順序是 `Scrt1.o crti.o crtbeginS.o … -ltxt <main.c 的 .o> … -lc …`。讀到 `-ltxt` 那一刻，「還沒找到定義的符號」清單是空的嗎？裡面有 `trim` 嗎？為什麼？(2) 預設下 `gcc -Wall -L. -ltxt main.c -o app_bad` 失敗；把 `-Wl,--no-as-needed` 放在 `-ltxt` 前面（`gcc -Wall -L. -Wl,--no-as-needed -ltxt main.c -o app_no`）就成功。讀到 `libtxt.so` 時兩種模式各做了什麼？`main.o` 要的 `trim` 最後是怎麼被解決的——是 `libtxt.so` 把 `trim` 加進了清單嗎？(3) `none.c` 用 `gcc -Wall none.c -L. -Wl,--no-as-needed -ltxt -o none_no` 和 `gcc -Wall none.c -L. -ltxt -o none_as` 各連結一次，`readelf -d` 的 `NEEDED` 會差在哪？Ubuntu 為什麼預設開 `--as-needed`？(4) 把 `libtxt.so` 換成同樣三個 `.o` 做成的 `libtxt.a`（並刪掉 `libtxt.so`），`gcc -Wall -L. -Wl,--no-as-needed -ltxt main.c -o app_a` 會成功嗎？為什麼？

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
做 `.so`：`gcc -Wall -fPIC -shared case.c space.c trim.c -o libtxt.so`；做 `.a`：`gcc -Wall -c case.c space.c trim.c && ar rcs libtxt.a case.o space.o trim.o && rm libtxt.so`（檔案都在 [`examples/asneeded/`](./examples/asneeded/)）。啟動檔 `Scrt1.o` 未定義的符號可以用 `nm -u $(gcc -print-file-name=Scrt1.o)` 看。

#### 13-1. `gcc -v` 顯示 gcc 交給 `ld` 的順序是 `Scrt1.o crti.o crtbeginS.o … -ltxt <main.c 的 .o> … -lc …`。讀到 `-ltxt` 那一刻，「還沒找到定義的符號」清單是空的嗎？裡面有 `trim` 嗎？為什麼？

**第 1 次（YYYY-MM-DD）**
```
```

#### 13-2. 預設下 `gcc -Wall -L. -ltxt main.c -o app_bad` 失敗；把 `-Wl,--no-as-needed` 放在 `-ltxt` 前面（`gcc -Wall -L. -Wl,--no-as-needed -ltxt main.c -o app_no`）就成功。讀到 `libtxt.so` 時兩種模式各做了什麼？`main.o` 要的 `trim` 最後是怎麼被解決的——是 `libtxt.so` 把 `trim` 加進了清單嗎？

**第 1 次（YYYY-MM-DD）**
```
```

#### 13-3. `none.c` 用 `gcc -Wall none.c -L. -Wl,--no-as-needed -ltxt -o none_no` 和 `gcc -Wall none.c -L. -ltxt -o none_as` 各連結一次，`readelf -d` 的 `NEEDED` 會差在哪？Ubuntu 為什麼預設開 `--as-needed`？

**第 1 次（YYYY-MM-DD）**
```
```

#### 13-4. 把 `libtxt.so` 換成同樣三個 `.o` 做成的 `libtxt.a`（並刪掉 `libtxt.so`），`gcc -Wall -L. -Wl,--no-as-needed -ltxt main.c -o app_a` 會成功嗎？為什麼？

**第 1 次（YYYY-MM-DD）**
```
```
