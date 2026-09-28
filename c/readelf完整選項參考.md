# readelf 怎麼用？—— 看懂 ELF 檔案內部的全部選項

> 日期：2026-09-23
> 情境：[ELF的interpreter是什麼](./ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md) 用到 `readelf -l`、`-p`；
> 依課程規則，介紹工具時要附完整選項參考。之後讀 ELF、連結、動態載入的課都會用到它
> 適用範圍：本機 GNU readelf 2.42（binutils，Ubuntu 24.04）；選項清單取自本機 `readelf --help`，逐項對照
> 相關：[hello.c怎麼變成能跑的程式-編譯的四個階段.md](./hello.c怎麼變成能跑的程式-編譯的四個階段.md)（ELF、區段、符號表）

> **這是「參考型」筆記，不是拿來背的。** 用法是：需要時打開來查，或在 `readelf` 裡打 `--help` 問它自己。
> 要記住的只有一件事：**遇到什麼問題，該找哪一類命令**——這也是文末自我測驗唯一會考的東西
> （測驗不考選項字面，可以翻著這份筆記作答）。

**一句話結論：`readelf` 專門「讀」ELF 檔案的結構，不執行它、也不反組譯。最常用的是四個：`-h` 看檔頭、
`-l` 看程式標頭（載入時怎麼放進記憶體、有沒有 interpreter）、`-S` 看區段、`-s` 看符號表；
`-d`、`-V`、`-r` 看動態連結的需求、符號版本與重定位。要看機器碼改用 `objdump -d`。**

---

## 一、文法

```
readelf <選項...> <ELF 檔案...>
```

| 位置 | 意思 | 省略時 |
|---|---|---|
| `<選項...>` | 要顯示哪些部分；**至少要一個**，可以多個一起用（`-h -l` 或合寫 `-hl`） | 不給任何選項會印出用法並結束 |
| `<ELF 檔案...>` | 一或多個 ELF 檔：可執行檔、`.o`、`.so`、core 檔 | 必填 |

`readelf` 跟 `objdump` 都在 binutils 裡，差別是：`readelf` **只懂 ELF**、直接照 ELF 規格列出結構；
`objdump` 透過 BFD 函式庫支援多種格式，並且能**反組譯**（第 1 課用的 `objdump -dr`）。

---

## 二、全部選項

`readelf --help` 本身**沒有分組**，是一整串清單；以下的分組是本筆記依用途整理的，選項一個不漏
（`--help` 共 52 行選項，其中 `--segments`、`--sections`、`--symbols` 是別名行，本篇併入主選項同一列）。★ 為本課程會常用的。

### 2.1 綜合

| 選項 | 作用 |
|---|---|
| `-a`、`--all` | 等於 `-h -l -S -s -r -d -V -A -I` 全部一起 |
| `-e`、`--headers` | 等於 `-h -l -S`：三種標頭一起 |

### 2.2 標頭與區段

| 選項 | 作用 |
|---|---|
| ★ `-h`、`--file-header` | ELF 檔頭：檔案類型（EXEC／DYN／REL）、CPU 架構、進入點位址 |
| ★ `-l`、`--program-headers`、`--segments` | 程式標頭：載入時分成哪幾段放進記憶體、權限、`INTERP`（interpreter） |
| ★ `-S`、`--section-headers`、`--sections` | 區段標頭：`.text`、`.data`、`.rodata`、`.bss` 等所有區段的位置與大小 |
| `-g`、`--section-groups` | 區段群組 |
| `-t`、`--section-details` | 區段的詳細資訊（比 `-S` 多印旗標說明） |

### 2.3 符號

| 選項 | 作用 |
|---|---|
| ★ `-s`、`--syms`、`--symbols` | 符號表（跟 `nm` 看的是同一份資料，格式較詳細） |
| `--dyn-syms` | 只看動態符號表（執行時要跟共享函式庫對接的那些符號） |
| `--lto-syms` | 顯示 LTO（Link Time Optimization，連結時最佳化）符號表 |
| `--sym-base=[0\|8\|10\|16]` | 符號大小用什麼進位印：混合（預設）、八、十、十六進位 |
| `-C`、`--demangle[=STYLE]` | 還原被編碼的符號名稱（C++、Rust 等）；STYLE 可選 `none`、`auto`、`gnu-v3`、`java`、`gnat`、`dlang`、`rust` |
| `--no-demangle` | 不還原符號名稱（預設） |
| `--recurse-limit` | 還原名稱時限制遞迴深度（預設） |
| `--no-recurse-limit` | 不限制還原名稱的遞迴深度 |
| `-U[dlexhi]`、`--unicode=[default\|locale\|escape\|hex\|highlight\|invalid]` | 符號名稱裡的 Unicode 字元怎麼顯示：依 locale（預設）、跳脫序列、十六進位、反白、或當成無效字元 |
| `-X`、`--extra-sym-info` | 顯示符號時多印額外資訊 |
| `--no-extra-sym-info` | 不印額外資訊（預設） |
| `-D`、`--use-dynamic` | 顯示符號時改用動態區段裡的資訊 |
| `-I`、`--histogram` | 符號雜湊表各桶子長度的直方圖 |
| `-T`、`--silent-truncation` | 符號名稱被截斷時不加 `[...]` 標記 |

### 2.4 動態連結、版本、重定位

| 選項 | 作用 |
|---|---|
| ★ `-d`、`--dynamic` | 動態區段：需要哪些共享函式庫（`NEEDED`）、搜尋路徑等——`ld.so` 讀的就是這裡 |
| ★ `-V`、`--version-info` | 符號版本：需要 libc 的哪些版本（第 1 課的 `GLIBC_2.34`） |
| ★ `-r`、`--relocs` | 重定位項目：將來要填的「洞」（第 1 課的 `R_X86_64_PLT32 puts`） |
| `-n`、`--notes` | 註記區段：Build ID、ABI 標籤、GNU 屬性 |
| `-u`、`--unwind` | 堆疊展開資訊（例外處理與 `backtrace` 用） |
| `-A`、`--arch-specific` | CPU 架構專屬資訊 |
| `-c`、`--archive-index` | 靜態函式庫（`.a`）的符號／檔案索引 |

### 2.5 傾印區段內容

| 選項 | 作用 |
|---|---|
| `-x`、`--hex-dump=<編號\|名稱>` | 以十六進位位元組傾印某個區段 |
| ★ `-p`、`--string-dump=<編號\|名稱>` | 以字串傾印某個區段（例如 `-p .interp`、`-p .rodata`、`-p .comment`） |
| `-R`、`--relocated-dump=<編號\|名稱>` | 套用重定位之後再傾印某個區段 |
| `-z`、`--decompress` | 區段有壓縮時先解壓再傾印 |

### 2.6 除錯資訊（DWARF）

`-g` 產生的除錯資訊用的格式叫 DWARF。這組是看它的內部內容，`gdb` 平常幫你讀；需要知道「除錯資訊裡到底存了什麼」時才用。

| 選項 | 作用 |
|---|---|
| `-w`、`--debug-dump[=子項]` | 傾印 DWARF 除錯區段；子項可指定只看某一種：`a`＝abbrev、`A`＝addr、`r`＝aranges、`c`＝cu_index、`L`＝decodedline（行號表，解碼後）、`f`＝frames、`F`＝frames-interp、`g`＝gdb_index、`i`＝info（變數、型別、函式）、`o`＝loc、`m`＝macro、`p`＝pubnames、`t`＝pubtypes、`R`＝Ranges、`l`＝rawline（行號表原始形式）、`s`＝str、`O`＝str-offsets、`u`＝trace_abbrev、`T`＝trace_aranges、`U`＝trace_info |
| `-wk`、`--debug-dump=links` | 顯示「連到另外的除錯資訊檔」的區段內容 |
| `-P`、`--process-links` | 顯示另外的除錯資訊檔裡的非除錯區段（隱含 `-wK`） |
| `-wK`、`--debug-dump=follow-links` | 跟著連結去讀另外的除錯資訊檔（預設） |
| `-wN`、`--debug-dump=no-follow-links` | 不跟著連結去讀 |
| `--dwarf-depth=N` | 不顯示深度 N 以上的 DIE（除錯資訊的樹狀節點） |
| `--dwarf-start=N` | 從偏移量 N 的 DIE 開始顯示 |

### 2.7 其他格式的型別與堆疊資訊

| 選項 | 作用 |
|---|---|
| `--ctf=<編號\|名稱>` | 顯示 CTF（Compact Type Format，精簡型別格式）資訊 |
| `--ctf-parent=<名稱>` | 指定 CTF 封存檔的父成員 |
| `--ctf-symbols=<編號\|名稱>` | 指定某區段當 CTF 的外部符號表 |
| `--ctf-strings=<編號\|名稱>` | 指定某區段當 CTF 的外部字串表 |
| `--sframe[=名稱]` | 顯示 SFrame（簡易堆疊框格式）資訊，預設區段 `.sframe` |

### 2.8 輸出與一般

| 選項 | 作用 |
|---|---|
| ★ `-W`、`--wide` | 允許一行超過 80 字元（不加的話 64 位元位址會被折成兩行，很難讀） |
| `-L`、`--lint`、`--enable-checks` | 顯示可能有問題的警告 |
| `@<檔案>` | 從檔案讀入選項 |
| `-H`、`--help` | 顯示說明 |
| `-v`、`--version` | 顯示版本 |

---

## 三、本課程會怎麼用它

| 想知道 | 命令 | 出現在哪篇 |
|---|---|---|
| 這是可執行檔、`.o` 還是 `.so`？進入點在哪？ | `readelf -h 檔案` | — |
| 有沒有 interpreter、是哪一個？ | `readelf -lW 檔案`（找 `INTERP`） | [ELF的interpreter](./ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md) |
| `.interp`、`.rodata`、`.comment` 裡的字串 | `readelf -p .interp 檔案` | 同上 |
| 各區段多大？ | `readelf -SW 檔案`（或 `size -A`） | [第 1 課](./hello.c怎麼變成能跑的程式-編譯的四個階段.md) 第九節 |
| 需要 libc 的哪些版本？ | `readelf -V 檔案` | [第 1 課第九節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#自我測驗第-5-題延伸找不到版本符號是怎麼回事)〈自我測驗第 5 題延伸〉 |
| 還有哪些洞要填？ | `readelf -r 檔案.o` | [第 1 課第四節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#四組譯機器碼裡的洞)（當時用 `objdump -dr`） |
| 執行時需要哪些共享函式庫？ | `readelf -d 檔案`（找 `NEEDED`） | — |

**錯誤訊息對照**：

| 情況 | 訊息 |
|---|---|
| 不是 ELF 檔（例如對 `.c` 或腳本下命令） | `readelf: Error: Not an ELF file - it has the wrong magic bytes at the start` |
| 要傾印的區段不存在（例如對靜態版下 `-p .interp`） | `readelf: Warning: Section '.interp' was not dumped because it does not exist` |

（兩則都是本機實測：對 `hello.c` 下 `readelf -h`、對靜態版下 `readelf -p .interp`。）

---

## 自我測驗

（**可以翻筆記作答**：本篇是參考型筆記。題目考的是「差別」與「該用哪個」，不考選項字面。）

1. `readelf` 和 `objdump` 都能看 ELF，差在哪？什麼情況一定要用 `objdump`？
2. `-l`（程式標頭）和 `-S`（區段標頭）描述的是同一個檔案的兩種切法。從 [ELF的interpreter](./ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md) 的例子說明：`INTERP` 和 `.interp` 各屬於哪一種、關係是什麼？
3. 想知道一支程式在別台機器上會不會因為 glibc 版本太舊而跑不起來，要用哪個選項看、看什麼？
4. 情境題：你拿到一個來路不明的執行檔，想在**不執行它**的前提下知道它是動態還是靜態連結、會去載入哪些函式庫。為什麼 `readelf` 比 `ldd` 更適合？（提示：[第 1 課](./hello.c怎麼變成能跑的程式-編譯的四個階段.md) 第七節對 `ldd` 的警告。）
