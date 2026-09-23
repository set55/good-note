# ELF 是什麼？—— 「可執行」與「可連結」：同一個檔案格式的兩種視角

> 日期：2026-09-23
> 情境：第 ① 站第 1 課的延伸提問。第 1 課、interpreter、readelf 幾篇一直出現 ELF，追問它到底是什麼意思
> 適用範圍：本機 Ubuntu 24.04、x86-64；所有輸出都是本機實測。本篇**不需要組合語言**
> 相關：[hello.c怎麼變成能跑的程式-編譯的四個階段.md](./hello.c怎麼變成能跑的程式-編譯的四個階段.md)、
> [ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md](./ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md)、
> [readelf完整選項參考.md](./readelf完整選項參考.md)

**一句話結論：ELF（Executable and Linkable Format，可執行與可連結格式）是 Linux 上「放機器碼的檔案」的
統一格式——`.o`、可執行檔、`.so` 函式庫、崩潰時的 core 檔全都是 ELF。名字裡的兩個詞對應兩種讀法：
**連結器**把檔案看成一個個**區段（section）**來拼接，**核心載入時**把它看成一塊塊**載入段（segment）**放進記憶體。
`.o` 只有區段、沒有載入段（還不能執行）；可執行檔兩種都有。檔案開頭固定是 `7f 45 4c 46`，也就是 `\177ELF`。**

---

## 一、名字的意思

**ELF = Executable and Linkable Format**，逐字就是：

| 字 | 意思 | 對應的使用者 |
|---|---|---|
| Executable | 可**執行**的 | 核心（`execve` 時把它載入記憶體） |
| Linkable | 可**連結**的 | 連結器 `ld`（把多個 `.o` 拼成一個執行檔） |
| Format | 檔案**格式** | 一份規格：檔案的每個位置放什麼 |

它是一份**規格**，不是一個程式。就像 PNG 規定圖片檔怎麼排列，ELF 規定「機器碼、資料、符號、重定位資訊」
在檔案裡怎麼排列，讓編譯器、連結器、核心、`gdb`、`readelf` 都照同一份規格讀寫。

來歷：ELF 出自 1989 年前後的 Unix System V Release 4（SVR4），後來被 Linux 在 1990 年代中期採用、
取代更早的 `a.out` 格式（`gcc` 沒給 `-o` 時的預設檔名 `a.out` 就是從那個年代留下來的名字）。
本機的輸出也還看得到這段歷史：`readelf -h` 印 `OS/ABI: UNIX - System V`，core 檔的 `file` 輸出寫 `SVR4-style`。

---

## 二、怎麼認出一個 ELF 檔：開頭 4 個 byte

每個 ELF 檔的前 4 個 byte 都一樣，這叫**魔術數字（magic number）**：

```
$ xxd -l 16 hello
00000000: 7f45 4c46 0201 0100 0000 0000 0000 0000  .ELF............
```

`7f` 之後的 `45 4c 46` 就是字母 `E`、`L`、`F` 的編碼。第 1 課 `strace` 看到動態載入器讀 libc 時，
讀到的開頭 `"\177ELF..."` 就是這 4 個 byte（`\177` 是八進位的 `0x7f`）。

`file`、`readelf`、核心都是先看這 4 個 byte 判斷「這是不是 ELF」。對 `.c` 檔下 `readelf` 會得到：

```
readelf: Error: Not an ELF file - it has the wrong magic bytes at the start
```

其他作業系統用不同的格式：Windows 是 PE（檔案開頭是 `MZ`）、macOS 是 Mach-O。這是**同一顆 CPU 上，
Linux 的程式也不能直接拿到 Windows 或 macOS 跑**的原因之一——連檔案格式都看不懂。

---

## 三、一種格式，四種檔案

ELF 檔頭裡有一個 `Type` 欄位。本機的四種實例：

```
hello.o                        REL  (Relocatable file)                       ← 目的檔，還有洞
hello                          DYN  (Position-Independent Executable file)   ← 一般的執行檔
hello_static                   EXEC (Executable file)                        ← 靜態連結的執行檔
/usr/lib/x86_64-linux-gnu/libc.so.6   DYN (Shared object file)               ← 共享函式庫
core.hello                     CORE (Core file)                              ← 用 gdb 的 gcore 存下來的行程快照
```

| Type | 是什麼 | 出現在哪 |
|---|---|---|
| `REL` | 可重定位目的檔：機器碼已經有了，但位址還沒決定，還有洞要填 | 第 1 課 `gcc -c` 的 `.o` |
| `EXEC` | 位址固定的執行檔 | `-static` 編出的版本 |
| `DYN` | 可以被放在任意位址的檔案：共享函式庫、以及 PIE 執行檔 | 一般 `gcc` 編出的執行檔、`libc.so.6` |
| `CORE` | 行程某一刻的記憶體與暫存器快照，給除錯器事後檢查 | 程式崩潰時產生，或 `gdb` 的 `gcore` |

**為什麼一般的執行檔跟函式庫同一類（DYN）？** 本機 gcc 預設產生 **PIE**（Position-Independent Executable，
位置無關的執行檔）：每次執行時載入到**不同的隨機位址**，讓攻擊者猜不到程式碼在哪（這個安全機制叫 ASLR，
Address Space Layout Randomization，位址空間配置隨機化，第 ③ 站會講）。能放在任意位址這點跟共享函式庫一樣，
所以同屬 DYN。靜態版是舊式的固定位址，所以是 EXEC。

---

## 四、名字的關鍵：兩種視角

ELF 的「Executable」和「Linkable」不只是好聽，它們是**兩張不同的目錄**，給兩種不同的讀者：

```
               ┌──────────────────────────────┐
               │ ELF 檔頭（ELF header）        │  固定在最前面：魔術數字、型別、CPU、兩張表在哪
               ├──────────────────────────────┤
  核心看這張 → │ 程式標頭表（program headers）  │  「載入時把哪幾塊放進記憶體、權限是什麼」
               ├──────────────────────────────┤
               │ .interp                        │
               │ .text      （程式碼）          │
               │ .rodata    （唯讀資料）        │  實際內容
               │ .data／.bss（資料）            │
               │ .symtab、.rela.* …            │
               ├──────────────────────────────┤
  連結器看這張→│ 區段標頭表（section headers）  │  「每個區段叫什麼、在哪、多大、做什麼用」
               └──────────────────────────────┘
```

| 視角 | 單位 | 誰在用 | `readelf` 選項 |
|---|---|---|---|
| **連結視角**（Linkable） | **區段（section）**：`.text`、`.data`、`.symtab`、`.rela.text`… 切得很細，本機 `hello` 有 31 個 | 連結器 `ld`、`nm`、`objdump` | `-S` |
| **執行視角**（Executable） | **載入段（segment）**：把權限相同的區段打包成一塊，本機 `hello` 有 13 個程式標頭，其中 4 個 `LOAD` | 核心、動態載入器 | `-l` |

`readelf -l` 最後一段會直接列出「哪些區段被打包進哪個載入段」（本機實測，節錄）：

```
 Section to Segment mapping:
  Segment Sections...
   01     .interp                                      ← INTERP：interpreter 的路徑
   03     .init .plt .plt.got .plt.sec .text .fini      ← 可執行的程式碼，打包成一塊
   04     .rodata .eh_frame_hdr .eh_frame               ← 唯讀資料
   05     .init_array .fini_array .dynamic .got .data .bss   ← 可寫資料
```

核心不在乎 `.text` 和 `.plt` 是不同的區段，它只在乎「這一塊要可執行、那一塊要可寫」——
這正是第 1 課作業第 1 題解說裡「權限不同的內容要放在不同分頁」的由來。

**最有力的證據：`.o` 沒有執行視角。**

```
$ readelf -l hello.o
There are no program headers in this file.
```

`.o` 只給連結器用，還沒準備好被載入執行，所以根本沒有程式標頭表。連結器的工作之一，就是**把一堆 `.o` 的區段
合併、再產生程式標頭表**，把「可連結的」變成「可執行的」。

---

## 五、檔頭裡的其他欄位

`readelf -h hello` 本機輸出，每一欄的意思：

```
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 ...     ← 魔術數字，後面幾個 byte 是下面三欄的編碼
  Class:                             ELF64 ← 64 位元格式（02）；32 位元是 ELF32（01）
  Data:                              2's complement, little endian   ← 位元組順序：小端序（01）
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  Type:                              DYN (Position-Independent Executable file)
  Machine:                           Advanced Micro Devices X86-64   ← 給哪種 CPU 用
  Entry point address:               0x1060                           ← 程式從哪裡開始執行
  Start of program headers:          64 (bytes into file)             ← 執行視角的目錄在哪
  Start of section headers:          13976 (bytes into file)          ← 連結視角的目錄在哪
  Number of program headers:         13
  Number of section headers:         31
```

三個跟之後課程直接相關的：

- **`Machine`**：ELF 是跨 CPU 的格式，但檔案**內容**是給某一種 CPU 的。你的路由器是 MIPS 或 ARM，
  x86-64 的 ELF 放上去，格式看得懂、機器碼卻跑不了。本機模擬：把 `hello` 複製一份，只改檔頭裡
  `Machine` 那 2 個 byte（檔案位移 18，改成 AArch64 的代碼 `0xb7`）：
  ```
  $ readelf -h hello_arm | grep Machine
    Machine:                           AArch64
  $ ./hello_arm
  zsh: exec format error: ./hello_arm
  $ strace ./hello_arm
  execve("./hello_arm", ...) = -1 ENOEXEC (Exec format error)
  ```
  核心在 `execve` 時先檢查 `Machine`，不符就回 **`ENOEXEC`（Exec format error）**——跟 interpreter
  不存在時的「No such file or directory／not found」是**不同的錯誤**，看到哪一個就知道該查哪裡。
- **`Data: little endian`**：多位元組的數字在記憶體裡「低位的 byte 放前面」——**這就是第 2 課的主題之一**。
- **`Entry point address`**：程式開始執行的位址。它指向啟動碼 `_start`（本機 `nm hello`：`_start` 在 `0x1060`、`main` 在 `0x1149`），**不是 `main`**
  （第 1 課「程式不是從 `main` 開始的」）。動態連結的程式，核心會先跳到 interpreter 的進入點，
  interpreter 做完事才跳到這個位址。

---

## 自我測驗

本篇的題目都**不需要組合語言**，答案都在上面的文字裡。

1. ELF 三個字母各代表什麼？為什麼說「Executable」和「Linkable」對應兩種不同的讀者？
2. `readelf -l hello.o` 說 `There are no program headers in this file`，這代表什麼？連結器做了什麼才讓執行檔有程式標頭？
3. 本機一般 `gcc` 編出的執行檔，`Type` 為什麼是 DYN，跟 `libc.so.6` 同一類，而靜態版是 EXEC？
4. 區段（section）和載入段（segment）差在哪？為什麼核心載入時要把很多區段打包成少數幾個載入段？
5. 情境題：你把 Ubuntu（x86-64）上編好的程式複製到路由器（ARM），`file` 顯示它是 ELF，執行卻失敗。這跟 [interpreter 那篇](./ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md) 第五節「interpreter 不存在」是不是同一種問題？從檔頭的哪一欄可以看出來？
