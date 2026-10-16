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

### DYN／EXEC 跟「有沒有動態連結」無關：兩個反例

> 2026-09-29 複習時補：「因為是動態連結，所以是 DYN」是反覆出現的誤會。上面的例子剛好「動態連結＝PIE」「靜態連結＝非 PIE」，
> 兩個性質綁在一起，看不出是哪一個決定 `Type`。下面用 gcc 把它們拆開。

`hello.c`（跟第 1 課同一支）：

```c
#include <stdio.h>
int main(void){ puts("hi"); return 0; }
```

編譯與檢查（本機 gcc 13.3 實測）：

```
$ gcc hello.c -o hello                    # 預設：動態連結 + PIE
$ gcc -no-pie hello.c -o hello_nopie      # 動態連結，但不是 PIE
$ gcc -static-pie hello.c -o hello_spie   # 靜態連結，但是 PIE

$ readelf -h hello | grep Type
  Type:                              DYN (Position-Independent Executable file)
$ readelf -h hello_nopie | grep Type
  Type:                              EXEC (Executable file)                        ← 有 interpreter，卻是 EXEC
$ readelf -h hello_spie | grep Type
  Type:                              DYN (Position-Independent Executable file)    ← 沒有 interpreter，卻是 DYN

$ file hello_nopie hello_spie              （節錄）
hello_nopie: ELF 64-bit LSB executable, x86-64, ..., interpreter /lib64/ld-linux-x86-64.so.2
hello_spie:  ELF 64-bit LSB pie executable, x86-64, ...                            ← 沒有 interpreter 這一項
```

這裡用到的兩個 gcc 選項（gcc 的選項有上千個，這裡只用到其中 2 個，完整清單見 `man gcc` 的 Options for Linking 一節）：

- `-no-pie`：不要產生 PIE，連結成固定位址的執行檔。仍然是動態連結，照樣需要 `ld-linux-x86-64.so.2`。
- `-static-pie`：靜態連結（libc 的程式碼複製進執行檔、沒有 interpreter），但整支程式仍是位置無關的，可以被放在任意位址。
  動態載入器 `ld-linux-x86-64.so.2` 自己就是這種檔案（見 [interpreter 那篇第三節](./ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md#三它自己也是一支程式可以直接執行)）。

| | 動態連結（有 interpreter） | 靜態連結（沒有 interpreter） |
|---|---|---|
| **PIE**（可放在任意位址） | 一般 `gcc` → **DYN** | `-static-pie`、`ld-linux-x86-64.so.2` → **DYN** |
| **非 PIE**（位址固定） | `-no-pie` → **EXEC** | `-static` → **EXEC** |

`Type` 只跟**直的那一欄**（是不是 PIE）走，跟橫的那一欄（動態或靜態連結）無關。`libc.so.6` 也是「可放在任意位址」，
所以跟 PIE 執行檔同屬 DYN。

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
  `Machine` 那 2 個 byte（檔案位移 18，改成 AArch64 的代碼 `0xb7`）。`hello` 就是第 1 課的 `hello.c` 編出來的。
  改檔的指令（2026-09-24 補上，重新實測）：
  ```
  $ cp hello hello_arm
  $ printf '\xb7' | dd of=hello_arm bs=1 seek=18 conv=notrunc
  1+0 records in
  1+0 records out
  1 byte copied, 3.7753e-05 s, 26.5 kB/s
  ```
  - `printf '\xb7'`：輸出一個值是 `0xb7` 的 byte（shell 的 `printf` 也認 `\x` 跳脫）。
    `Machine` 欄位是 2 byte、小端序，x86-64 的代碼是 `0x3e`，存成 `3e 00`；AArch64 是 `0xb7`，存成 `b7 00`，所以只要改第一個 byte。
  - `dd`：從標準輸入讀、寫到 `of=` 指定的檔案。本機 coreutils 9.4 的 `dd --help` 共 13 個運算元，**全部列出**：

    | 運算元 | 作用 |
    |---|---|
    | `if=FILE` | 從 FILE 讀（預設標準輸入；這裡沒給，所以讀 `printf` 的輸出） |
    | `of=FILE` | 寫到 FILE（預設標準輸出）。**這裡用** |
    | `bs=BYTES` | 讀和寫都以 BYTES 為一塊（預設 512）。**這裡用 `bs=1`，讓 `seek` 以 byte 為單位** |
    | `ibs=BYTES` | 只設定讀的塊大小 |
    | `obs=BYTES` | 只設定寫的塊大小 |
    | `cbs=BYTES` | 做轉換（`conv=block` 等）時一次處理的大小 |
    | `count=N` | 只複製 N 塊 |
    | `skip=N`（或 `iseek=N`） | 讀之前跳過輸入的前 N 塊 |
    | `seek=N`（或 `oseek=N`） | 寫之前跳過輸出的前 N 塊。**這裡用 `seek=18`，跳到位移 18 再寫** |
    | `conv=CONVS` | 轉換選項，逗號分隔。**這裡用 `notrunc`**：不要把輸出檔截斷。沒加的話，`dd` 會把 `hello_arm` 在寫完的地方截斷，後面全部消失 |
    | `iflag=FLAGS` | 讀取時的旗標 |
    | `oflag=FLAGS` | 寫入時的旗標 |
    | `status=LEVEL` | 印到標準錯誤的資訊多寡（`none`、`noxfer`、`progress`） |

    `conv=` 可用的符號共 16 個（`ascii ebcdic ibm block unblock lcase ucase sparse swab sync excl nocreat notrunc noerror fdatasync fsync`），
    `iflag`／`oflag` 的旗標也有十幾個，這裡只用到 `notrunc`，完整說明見 `dd --help` 或 `man dd`。
  - 輸出的 `1+0 records in/out`：讀了 1 個完整的塊、0 個不完整的塊；寫也一樣。
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

> 參考答案寫在每題下面（2026-10-06 使用者指定）。作答時先遮住答案、自己想過再對照。

本篇的題目都**不需要組合語言**，答案都在上面的文字裡。

1. ELF 三個字母各代表什麼？為什麼說「Executable」和「Linkable」對應兩種不同的讀者？
   > **參考答案**【[第一節](#一名字的意思)、[第四節](#四名字的關鍵兩種視角)】Executable and Linkable Format（可執行與可連結格式）。ELF 有兩張目錄，各給一種讀者：**程式標頭表**（執行視角，單位是載入段 segment，給核心與動態載入器看：載入時把哪幾塊放進記憶體、權限是什麼）和**區段標頭表**（連結視角，單位是區段 section，給連結器 `ld`、`nm`、`objdump` 看：每個區段叫什麼、在哪、多大）。
2. `readelf -l hello.o` 說 `There are no program headers in this file`，這代表什麼？連結器做了什麼才讓執行檔有程式標頭？
   > **參考答案**【[第四節](#四名字的關鍵兩種視角)〈最有力的證據：`.o` 沒有執行視角〉】（2026-10-16 改寫成核心關鍵格式）
   >
   > 核心關鍵：
   > 1. 沒有程式標頭表 ＝ 沒有執行視角：`.o` 只給連結器用，核心還不能載入執行它。
   > 2. 連結器先把多個 `.o` 的同類區段**合併**，再把權限相同的區段打包成載入段 `LOAD`（依分頁對齊），最後寫出程式標頭表。
   >
   > 詳細說明：合併之後還要補上位址（重定位）。動態連結時，程式標頭表裡另外有 `INTERP`（interpreter 的路徑）、`DYNAMIC`（指向 `.dynamic`，裡面有 `NEEDED`）等項，「可連結的」才變成「可執行的」。
3. 本機一般 `gcc` 編出的執行檔，`Type` 為什麼是 DYN，跟 `libc.so.6` 同一類，而靜態版是 EXEC？
   > **參考答案**【[第三節](#三一種格式四種檔案)〈DYN／EXEC 跟「有沒有動態連結」無關：兩個反例〉】（2026-10-12 改寫成核心關鍵格式）
   >
   > 核心關鍵：
   >
   > 1. `Type` 看的是**是不是 PIE**（Position-Independent Executable，位置無關執行檔，每次執行由 ASLR 放在隨機位址）：本機 gcc 預設產生 PIE → DYN。
   > 2. 不是 PIE（`-static` 版，固定位址）→ EXEC。（2026-10-12 使用者爭論後修正：不要求解釋 PIE 的意思）
   >
   > 詳細說明：PIE 跟共享函式庫一樣「可以放在任意位址」，所以跟 `libc.so.6` 同屬 DYN。`Type` 只看「是不是 PIE」，跟有沒有動態連結無關：`-no-pie` 動態連結 → EXEC，`-static-pie` 靜態連結 → DYN。
4. 區段（section）和載入段（segment）差在哪？為什麼核心載入時要把很多區段打包成少數幾個載入段？
   > **參考答案**【[第四節](#四名字的關鍵兩種視角)】區段（section）是連結視角的單位，切得很細（`hello` 有 31 個），給 `ld`、`nm`、`objdump` 用；載入段（segment）是執行視角的單位，數量很少（4 個 `LOAD`），給核心與動態載入器用，描述「哪一塊放進記憶體、權限是什麼」。核心載入時只在乎權限（可讀／可寫／可執行），而記憶體權限**最小只能以分頁（本機 4096 byte）為單位**設定——同一個分頁不能一半可執行、一半可寫。所以 `ld` 把權限相同的區段排在一起、打包成一個載入段，核心用少數幾次映射、每塊設一種權限就能把程式放進記憶體。
5. 情境題：你把 Ubuntu（x86-64）上編好的程式複製到路由器（ARM），`file` 顯示它是 ELF，執行卻失敗。**讓它失敗的原因**，跟 [interpreter 那篇](./ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md) 第五節「interpreter 不存在」是不是同一種問題？（提示：一般 `gcc` 編出的是動態連結程式，路由器上也沒有它要的 interpreter，想想核心先檢查哪一個。）從檔頭的哪一欄可以看出來？（2026-10-09 改寫題目：原題沒講明問的是「失敗的原因」，動態程式在路由器上同時缺 interpreter，容易混淆）
   > **參考答案**【[第五節](#五檔頭裡的其他欄位)】（2026-10-09 改寫成核心關鍵格式；2026-10-16 把錯誤碼移到詳細說明，題目沒有問）
   >
   > 核心關鍵：
   > 1. 不是同一種問題：ELF 格式在哪台機器上都讀得懂，但裡面的**機器碼是給特定 CPU 的**。
   > 2. 看檔頭（`readelf -h`）的 **`Machine`** 欄位：它寫著 `Advanced Micro Devices X86-64`，跟 ARM 的 CPU 不符。
   >
   > 詳細說明：kernel 在 `execve` 讀檔頭時發現架構不符，就回 `ENOEXEC`（`Exec format error`）；interpreter 不存在則是 `ENOENT`（`not found`），兩種錯誤訊息不同。動態連結的程式在路由器上確實**也**缺 interpreter（`/lib64/ld-linux-x86-64.so.2`），但架構不符時，kernel 讀到檔頭就停了，還沒走到「找 interpreter」那一步；就算把 x86-64 的載入器放到那個路徑，它本身也是 x86-64 的機器碼，一樣跑不了。`INTERP` 在程式標頭（program header），不在檔頭；就算程式是靜態編譯、根本沒有 `INTERP`，x86-64 的機器碼一樣不能在 ARM 上跑。
