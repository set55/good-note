# `objdump` 能不能代替 `ldd` 找出 ELF 的依賴庫？—— 讀需求單 vs 請載入器實際跑一次

> 日期：2026-09-23
> 情境：第 ① 站第 1 課的延伸提問。剛學完〈三個名字很像的檔案〉（`ld`／`ld-linux-x86-64.so.2`／`libc.so.6`）後，
> 問 `objdump` 能不能取代 `ldd`
> 適用範圍：實測本機 Ubuntu 24.04、x86-64、glibc 2.39、GNU binutils 2.42（`objdump`）；所有輸出都是本機實際執行結果
> 對應檔案：跨架構實驗用的 `busybox`、`libc.so` 從 OpenWrt 路由器（24.10.2，aarch64）複製到 scratchpad，不入 git
> 相關：[hello.c怎麼變成能跑的程式](./hello.c怎麼變成能跑的程式-編譯的四個階段.md) 第十節、
> [ELF的interpreter是什麼](./ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md)、
> [readelf完整選項參考](./readelf完整選項參考.md)

**一句話結論：只能取代一半。`objdump -p` 讀的是執行檔裡 `ld` 寫下的「需求單」（`NEEDED`），只有**直接**依賴、
只有名字、不知道檔案在哪、也不知道找不找得到；`ldd` 則是**請動態載入器實際模擬載入一次**，
得到完整的依賴樹（含間接依賴）、實際路徑、找不到的會標 `not found`。
反過來，`objdump`／`readelf` 不執行任何東西，所以檢查來路不明或別的架構的檔案時，只有它們能用；
其中 `readelf -d` 更可靠——路由器上那種剝掉區段標頭的檔案，`objdump -p` 會看不到 `NEEDED`。**

---

## 一、先看差多少：`curl` 的例子

```
$ objdump -p /usr/bin/curl | grep NEEDED
  NEEDED               libcurl.so.4
  NEEDED               libz.so.1
  NEEDED               libc.so.6

$ ldd /usr/bin/curl | wc -l
33
$ ldd /usr/bin/curl
	linux-vdso.so.1 (0x00007737cc745000)
	libcurl.so.4 => /lib/x86_64-linux-gnu/libcurl.so.4 (0x00007737cc619000)
	libz.so.1 => /lib/x86_64-linux-gnu/libz.so.1 (0x00007737cc5fd000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007737cc200000)
	libnghttp2.so.14 => /lib/x86_64-linux-gnu/libnghttp2.so.14 (0x00007737cc5d2000)
	libssl.so.3 => /lib/x86_64-linux-gnu/libssl.so.3 (0x00007737cc460000)
	...（中間省略，共 33 行）
	/lib64/ld-linux-x86-64.so.2 (0x00007737cc747000)
	...
```

3 個 vs 33 行。差在哪裡：

| | `objdump -p`（或 `readelf -d`） | `ldd` |
|---|---|---|
| 列出什麼 | 只有**這個檔案自己**的 `NEEDED`（直接依賴） | 整棵依賴樹：直接依賴 **＋依賴的依賴**（間接依賴） |
| 名字 → 路徑 | 只有名字 `libcurl.so.4` | 解析成實際路徑 `/lib/x86_64-linux-gnu/libcurl.so.4` |
| 找不到的函式庫 | 看不出來，照樣列名字 | 標成 `not found` |
| 其他項目 | 沒有 | 還會列出 `linux-vdso.so.1`（核心直接塞進行程的虛擬函式庫，磁碟上沒有這個檔）與載入器本身 |
| 位址 | 沒有 | 括號裡是這次模擬載入的位址，每次都不一樣，沒有意義 |
| 會不會執行東西 | **不會**，只讀檔案 | **會**請動態載入器跑起來（見第三節） |

那 30 個多出來的是哪來的？例如 `libssl.so.3`：`curl` 自己沒有要它，是 `libcurl.so.4` 要的：

```
$ objdump -p /lib/x86_64-linux-gnu/libcurl.so.4 | grep NEEDED | wc -l
14
```

每個 `.so` 也有自己的需求單。`objdump` 只讀你給它的那一個檔案，**不會自己往下追**；
要用 `objdump` 得到完整清單，就得自己對每一個依賴再跑一次，還得自己知道每個名字對應到哪個路徑。

---

## 二、為什麼會差這麼多：兩者看的是不同時間點

套用〈三個名字很像的檔案〉那個比喻：

- **`objdump -p` 是在讀施工單。** `NEEDED libc.so.6` 這一行是 `ld` 在**連結時**寫進檔案的。施工單上只寫
  「要 libc.so.6」，沒寫倉庫在哪條街，也沒寫 libc 自己又要什麼。
- **`ldd` 是請工頭實際走一趟。** 誰去找檔案、決定路徑、再讀每個 `.so` 的需求單往下追？就是**執行時**的動態載入器
  `ld-linux-x86-64.so.2`。`ldd` 本身不會做這件事，它把工作交給載入器。

證據：`ldd` 根本不是一支編譯過的程式，而是 shell 腳本：

```
$ file /usr/bin/ldd
/usr/bin/ldd: Bourne-Again shell script, ASCII text executable

$ grep -n LD_TRACE /usr/bin/ldd
103:add_env="LD_TRACE_LOADED_OBJECTS=1 LD_WARN=$warn LD_BIND_NOW=$bind_now"
```

它做的事是：設一個環境變數 `LD_TRACE_LOADED_OBJECTS=1`，再啟動載入器處理你的程式。
載入器看到這個變數，就照常把依賴一個個找出來、載入，**但在跳進 `main` 之前停下來**，改成把清單印出來。
所以下面三個寫法的輸出一模一樣：

```
$ ldd /usr/bin/curl
$ LD_TRACE_LOADED_OBJECTS=1 /usr/bin/curl                 ← 直接設變數跑程式
$ /lib64/ld-linux-x86-64.so.2 --list /usr/bin/curl        ← 直接叫載入器
	linux-vdso.so.1 (0x...)
	libcurl.so.4 => /lib/x86_64-linux-gnu/libcurl.so.4 (0x...)
	...
```

這也解釋了表格裡每一個差異：路徑、`not found`、間接依賴，全都是**搜尋**的結果，
而搜尋是載入器在執行時才做的事；需求單上本來就沒有這些資訊。

---

## 三、`ldd` 會執行東西：這是它的代價

因為 `ldd` 要靠載入器跑起來，`man ldd` 的 Security 一節特別警告：

> in some circumstances (e.g., where the program specifies an ELF interpreter other than ld-linux.so),
> some versions of ldd may attempt to obtain the dependency information by attempting to directly execute the program

意思是：如果一個執行檔在 interpreter 欄位寫的**不是**正常的 `ld-linux`，而是攻擊者自己寫的程式，
那麼對它跑 `ldd`，就可能執行到攻擊者的程式碼。本機的 `ldd` 腳本會先用 `--verify` 檢查載入器
（`/usr/bin/ldd` 第 159 行），但 man page 的建議仍然是：**對不信任的檔案，不要用 `ldd`**，改用 `objdump -p` 或 `readelf -d`。
（這個攻擊本機未實測。）

另外還有兩種情況，`ldd` 根本不能用、只能靠 `objdump`／`readelf`：

- **別的架構的檔案**：本機的載入器沒辦法處理別的架構，`ldd` 給不出結果，而且**錯誤訊息會誤導人**。
  實測：從 OpenWrt 路由器（aarch64，musl）複製 `/bin/busybox` 回本機（2026-09-23）：

```
$ file busybox
busybox: ELF 64-bit LSB executable, ARM aarch64, ..., dynamically linked, interpreter /lib/ld-musl-aarch64.so.1, no section header

$ ldd ./busybox
	not a dynamic executable            ← 錯！file 明明說 dynamically linked，只是本機載入器處理不了 ARM
$ ./busybox
exec format error: ./busybox            ← 直接執行：核心拒絕（見 ELF 筆記的 ENOEXEC）

$ readelf -d busybox | grep NEEDED      ← 只讀檔案，照樣看得到
 0x0000000000000001 (NEEDED)             Shared library: [libgcc_s.so.1]
 0x0000000000000001 (NEEDED)             Shared library: [libc.so]
```

  注意這裡用的是 `readelf -d`，不是 `objdump -p`——原因見下一段。

- **靜態連結的檔案**：兩者都會告訴你「沒有依賴」，只是說法不同：

```
$ ldd hs                          （hs 是用 gcc -static 編出來的）
	not a dynamic executable
$ objdump -p hs | grep -c NEEDED
0
```

**陷阱：`objdump -p` 在「剝掉區段標頭」的檔案上看不到 `NEEDED`。** 同一支 `busybox`：

```
$ objdump -p busybox | grep NEEDED
（沒有輸出——只印出了 Program Header）

$ readelf -S busybox
There are no sections in this file.
```

OpenWrt 為了省空間，連區段標頭（section header）都剝掉了（`file` 顯示 `no section header`）。
`objdump -p` 是透過**區段標頭**找到 `.dynamic` 區段的，區段標頭沒了就找不到；
`readelf -d` 則是透過**程式標頭**的 `DYNAMIC` 那一項找到同一份資料，所以照樣讀得到。
這正是 [ELF 是什麼](./ELF是什麼-可執行與可連結格式的兩種視角.md) 講的兩種視角：區段是給連結器看的、可以丟；
程式標頭是給核心與載入器看的，**丟了程式就跑不起來，所以一定還在**。
**要檢查的檔案可能被剝過（嵌入式裝置、路由器韌體）時，用 `readelf -d`。**

---

## 四、找不到函式庫時，兩者各看到什麼

自己做一個函式庫，連結完再讓它找不到（兩個檔案當初沒有保存，這是重建版，2026-09-24 重新實測，輸出與下面一致）。

`foo.c`：函式庫的內容，只有一個函式。

```c
int foo(void)
{
    return 42;
}
```

`main.c`：呼叫 `foo()` 的程式。第 3 行是宣告，告訴編譯器 `foo` 在別處、回傳 `int`。

```c
#include <stdio.h>

int foo(void);

int main(void)
{
    printf("%d\n", foo());
    return 0;
}
```

`-shared` 是「做成共享函式庫（`.so`）」，`-fPIC` 是「產生不管被載入到哪個位址都能執行的程式碼」（共享函式庫每次載入的位址不同，所以需要）；
`-lfoo` 是「連結名叫 `libfoo.so` 的函式庫」（`-l` 後面的名字會自動補上 `lib` 前綴與 `.so` 副檔名）。
這幾個 gcc 選項的完整說明見 `man gcc`（Link Options、Code Gen Options）。


```
$ gcc -shared -fPIC foo.c -o libfoo.so      # 做一個共享函式庫
$ gcc main.c -L. -lfoo -o app               # 連結時用 -L. 告訴 ld 去目前目錄找
$ LD_LIBRARY_PATH=. ./app                   # 執行時也告訴載入器去目前目錄找
42

$ objdump -p app | grep NEEDED              # 需求單：看不出有問題
  NEEDED               libfoo.so
  NEEDED               libc.so.6

$ ldd ./app                                 # 請載入器實際找：找不到
	linux-vdso.so.1 (0x00007c775b07b000)
	libfoo.so => not found
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007c775ae00000)
	/lib64/ld-linux-x86-64.so.2 (0x00007c775b07d000)

$ ./app                                     # 實際執行：跟 ldd 說的一樣
./app: error while loading shared libraries: libfoo.so: cannot open shared object file: No such file or directory
```

這個實驗再一次把兩個時間點分開：

- 連結時 `ld` 找得到 `libfoo.so`（因為有 `-L.`），所以連結成功，需求單上寫了 `libfoo.so`。
- 執行時載入器**不知道** `-L.` 這回事，那個選項只給 `ld` 看，沒有寫進執行檔。載入器只去它自己的預設路徑找，所以找不到。
- 錯誤訊息開頭的 `error while loading shared libraries` 就是載入器發出的，跟 `ldd` 的 `not found` 是同一件事。

**所以要回答「這支程式在這台機器上能不能跑」，要用 `ldd`；`objdump` 答不了這個問題。**

---

## 五、怎麼選

| 你想知道的 | 用什麼 |
|---|---|
| 這支程式在**這台機器**上跑不跑得起來、缺哪個 `.so` | `ldd`（要找 `not found`） |
| 這支程式**自己**宣告要哪些函式庫 | `objdump -p \| grep NEEDED` 或 `readelf -d \| grep NEEDED` |
| 檔案來路不明，不想冒執行的風險 | `readelf -d`（或 `objdump -p`） |
| 別的架構的檔案、路由器／嵌入式韌體裡的檔案 | `readelf -d`（區段標頭常被剝掉，`objdump -p` 可能看不到） |
| 需要的 glibc 符號版本（例如 `GLIBC_2.34`） | `objdump -p` 的 `Version References` 段、`readelf -V`（見第 1 課筆記第九節） |

`readelf -d` 跟 `objdump -p` 讀的是同一份資料（動態區段），但找到它的路徑不同：`objdump -p` 靠區段標頭，
`readelf -d` 靠程式標頭。一般檔案兩者結果一樣；**區段標頭被剝掉的檔案只有 `readelf -d` 讀得到**（第三節實測）。
`readelf` 的全部選項見 [readelf完整選項參考](./readelf完整選項參考.md)。

---

## 六、本篇用到的工具選項

### `ldd`：全部選項（`ldd --help` 實測，共 6 個，全列）

```
ldd [選項]... 檔案...
```

| 選項 | 作用 |
|---|---|
| `--help` | 印出說明後結束 |
| `--version` | 印出版本（本機 `ldd (Ubuntu GLIBC 2.39-0ubuntu8.9) 2.39`，版本就是 glibc 的版本） |
| `-d`, `--data-relocs` | 額外做資料的重定位，並回報缺少的符號（只對執行檔有效） |
| `-r`, `--function-relocs` | 資料與函式的重定位都做，回報缺少的符號與函式 |
| `-u`, `--unused` | 列出「寫在需求單上卻沒用到」的直接依賴（`ldd -u /usr/bin/curl` 沒有輸出，代表每個都有用到） |
| `-v`, `--verbose` | 印出全部資訊，包括每個函式庫需要的符號版本 |

相關的環境變數：`LD_TRACE_LOADED_OBJECTS=1`（`ldd` 背後用的機制，見第二節）；
`LD_DEBUG` 的全部選項見 [第 1 課筆記第十節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#十三個名字很像的檔案ldld-linux-x86-64so2libcso6)。

### `objdump`：「顯示什麼」這一組全部列出

```
objdump <選項> <檔案>...
```

`objdump --help` 把選項分成兩組：
- **第一組「至少要給一個」**：決定**顯示什麼**，每一個都是一種功能。下表**全部列出（31 項）**。
- **第二組「選用」**：調整顯示的方式（例如 `-M intel` 改用 Intel 語法、`-C` 還原 C++ 名稱、`--no-show-raw-insn` 不印機器碼），
  約 40 行，有些是同一個選項的不同值，這裡**不列**，完整清單見 `objdump --help` 或 `man objdump`。

| 選項 | 顯示什麼 |
|---|---|
| `-a`, `--archive-headers` | 靜態函式庫（`.a` 封存檔）裡每個成員的檔頭 |
| `-f`, `--file-headers` | 整個檔案的檔頭摘要（架構、類型、進入點） |
| `-p`, `--private-headers` | **該格式特有的檔頭內容**：對 ELF 是程式標頭、`.dynamic`（`NEEDED` 在這裡）、版本需求。本篇的主角 |
| `-P`, `--private=OPT,...` | 該格式特有的其他內容（依格式而定，ELF 很少用） |
| `-h`, `--[section-]headers` | 區段標頭（每個 section 的名字、大小、位址） |
| `-x`, `--all-headers` | 全部檔頭，等於 `-a -f -h -p -r -t` 合在一起 |
| `-d`, `--disassemble` | 反組譯可執行的區段（第 1 課用過） |
| `-D`, `--disassemble-all` | 反組譯**所有**區段，連資料也當成指令解讀 |
| `--disassemble=<sym>` | 只反組譯某個符號（例如 `--disassemble=main`） |
| `-S`, `--source` | 反組譯時穿插原始碼（要用 `-g` 編譯） |
| `--source-comment[=<txt>]` | 同 `-S`，但原始碼那幾行前面加上註解前綴 |
| `-s`, `--full-contents` | 以十六進位傾印區段的完整內容 |
| `-Z`, `--decompress` | 顯示前先把壓縮過的區段解壓縮 |
| `-g`, `--debugging` | 除錯資訊 |
| `-e`, `--debugging-tags` | 除錯資訊，用 ctags 格式輸出 |
| `-G`, `--stabs` | 舊式的 STABS 除錯資訊（原始格式） |
| `-W`, `--dwarf[=...]` | DWARF（現代的除錯資訊格式）各區段的內容，可用字母或名稱只挑一種 |
| `-Wk`, `--dwarf=links` | 指向「獨立除錯資訊檔」的區段 |
| `-WK`, `--dwarf=follow-links` | 追進獨立的除錯資訊檔（預設） |
| `-WN`, `--dwarf=no-follow-links` | 不追進獨立的除錯資訊檔 |
| `-L`, `--process-links` | 顯示獨立除錯資訊檔裡的非除錯區段（隱含 `-WK`） |
| `--ctf[=SECTION]` | CTF（Compact Type Format，精簡型別格式）型別資訊 |
| `--sframe[=SECTION]` | SFrame（簡單堆疊框格式）資訊 |
| `-t`, `--syms` | 符號表（類似 `nm`） |
| `-T`, `--dynamic-syms` | 動態符號表（類似 `nm -D`；[第 1 課第九節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#自我測驗第-5-題延伸找不到版本符號是怎麼回事)〈自我測驗第 5 題延伸〉用來看 `GLIBC_2.34`） |
| `-r`, `--reloc` | 重定位項目（第 1 課看 `R_X86_64_PLT32 puts-0x4` 用的） |
| `-R`, `--dynamic-reloc` | 動態重定位項目（留給執行時載入器處理的那些） |
| `@<file>` | 從檔案讀選項 |
| `-v`, `--version` | 印出版本 |
| `-i`, `--info` | 列出支援的檔案格式與架構 |
| `-H`, `--help` | 印出說明 |

**文法可以問工具**：`objdump --help`、`ldd --help`、`man objdump`、`man ldd`、`man ld.so`（`LD_TRACE_LOADED_OBJECTS` 等環境變數寫在這裡）。

---

## 自我測驗

1. `objdump -p /usr/bin/curl` 只列出 3 個 `NEEDED`，`ldd` 卻列出 33 行。多出來的是哪幾類東西？為什麼 `objdump` 看不到？
2. `ldd` 是怎麼得到依賴清單的？從這個機制解釋：為什麼對來路不明的執行檔用 `ldd` 有風險，`objdump -p` 卻沒有？
3. 一支程式連結時用了 `-L./lib -lfoo`，編譯成功；執行時卻出現 `error while loading shared libraries: libfoo.so`。
   用「連結時」與「執行時」分別是誰在找檔案，解釋為什麼連結找得到、執行卻找不到。這時 `objdump -p` 與 `ldd` 各會顯示什麼？
4. 情境題：同事傳來一支 ARM 開發板用的執行檔，問你「它需要哪些函式庫」。你在自己的 x86-64 電腦上該用哪個工具？
   用 `ldd` 會發生什麼事，為什麼？
