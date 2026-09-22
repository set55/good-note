# `hello.c` 怎麼變成能跑的程式？—— 前處理、編譯、組譯、連結四個階段

> 日期：2026-09-22
> 情境：系統化學習路線的第 ① 站第 1 課（見 [Linux與系統程式的系統化學習路線.md](../learning/Linux與系統程式的系統化學習路線.md)），
> 對應 CS:APP 第 1 章 1.1～1.4 節
> 適用範圍：實測本機 Ubuntu 24.04、x86-64、gcc 13.3.0、GNU binutils 2.42；所有輸出都是本機實際執行結果
> 相關：[fd是什麼-行程的檔案描述符表.md](../linux/fd是什麼-行程的檔案描述符表.md)、
> [root執行腳本也Permission-denied-缺執行位元.md](../linux/root執行腳本也Permission-denied-缺執行位元.md)（`execve`）、
> [bin與sbin的差別-以及怎麼正確找到執行檔.md](../linux/bin與sbin的差別-以及怎麼正確找到執行檔.md)

**一句話結論：`gcc hello.c` 不是一個動作，而是四個程式接力：前處理器把 `#include` 貼進來（`.i`，
還是 C）、編譯器翻成組合語言（`.s`）、組譯器翻成機器碼但留下「洞」（`.o`，呼叫外部函式的位址是 0）、
連結器把洞接上函式庫。而 `puts` 這種函式庫函式，直到**程式執行時**才由動態載入器填上真正的位址。
`gcc` 本身只是總指揮（driver）。最後程式印出文字，靠的是一個 `write(1, ...)` 系統呼叫——
C 語言與作業系統的交界就在這裡。**

---

## 目錄

- [一、四個階段總覽](#一四個階段總覽)
- [二、前處理：`#include` 只是複製貼上](#二前處理include-只是複製貼上)
- [三、編譯：C 變成組合語言（以及一個意外）](#三編譯c-變成組合語言以及一個意外)
- [四、組譯：機器碼裡的「洞」](#四組譯機器碼裡的洞)
- [五、連結：把洞接上，但 `puts` 還是沒接完](#五連結把洞接上但-puts-還是沒接完)
- [六、執行：從 `execve` 到 `write`](#六執行從-execve-到-write)
- [七、本課用到的工具與選項](#七本課用到的工具與選項)
- [八、動手作業](#八動手作業)
- [自我測驗](#自我測驗)

---

## 一、四個階段總覽

實驗用的程式：

```c
#include <stdio.h>

#define GREETING "hello, world"

int main(void)
{
    printf("%s\n", GREETING);
    return 0;
}
```

```
hello.c ──前處理（cpp）──► hello.i ──編譯（cc1）──► hello.s ──組譯（as）──► hello.o ──連結（ld）──► hello
  C 原始碼               還是 C，但展開了          x86-64 組合語言         可重定位目的檔         可執行檔
  116 bytes              822 行、21323 bytes       667 bytes              1504 bytes            15960 bytes
```

`gcc` 自己不做這些事，它是**總指揮（compiler driver）**，依序呼叫別的程式。`gcc -v` 會把它實際
執行的命令印出來（本機輸出，節錄）：

```
/usr/libexec/gcc/x86_64-linux-gnu/13/cc1 ... hello.c ...     ← 前處理＋編譯（cc1 內含前處理器）
as -v --64 -o /tmp/ccDQyhD0.o /tmp/ccS7uQYu.s                ← 組譯
/usr/libexec/gcc/x86_64-linux-gnu/13/collect2 ... -dynamic-linker /lib64/ld-linux-x86-64.so.2 ...
                                                              ← 連結（collect2 是 gcc 包在 ld 外面的一層）
```

平常中間檔寫到 `/tmp` 用完就刪。`gcc -save-temps hello.c -o hello` 會把 `.i`、`.s`、`.o` 都留下來，
或用 `-E`／`-S`／`-c` 讓它停在某一階段（第七節）。

---

## 二、前處理：`#include` 只是複製貼上

```sh
gcc -E hello.c -o hello.i
```

前處理器（preprocessor）處理所有 `#` 開頭的指令（directive），輸出**還是 C**：

- `#include <stdio.h>`：把 `/usr/include/stdio.h`（以及它再 include 的檔案）**整個貼進來**。
  116 bytes 的檔案變成 822 行，裡面有這一行：
  ```c
  extern int printf (const char *__restrict __format, ...);
  ```
  這只是**宣告（declaration）**——告訴編譯器「有一個叫 `printf` 的函式，參數長這樣」，
  **沒有**它的程式碼。程式碼在 libc 裡，要到連結時才會碰到。
- `#define GREETING "hello, world"`：純文字取代。`hello.i` 的結尾變成：
  ```c
  printf("%s\n", "hello, world");
  ```

**重點：真正的編譯器從來沒看過 `#include` 與 `#define`**，它看到的是已經展開好的一大份 C。
所以巨集寫錯時，錯誤訊息常常很奇怪——編譯器在罵的是展開後的樣子。

---

## 三、編譯：C 變成組合語言（以及一個意外）

```sh
gcc -S -O0 hello.c -o hello.s
```

`hello.s` 的核心部分（去掉除錯用的 `.cfi_*` 指令）：

```asm
    .section .rodata
.LC0:
    .string "hello, world"          ← 字串常數放在唯讀資料區（read-only data）

    .text                           ← 以下是程式碼區
    .globl main                     ← main 是全域符號，連結器看得到
main:
    endbr64                         ← CPU 的控制流程防護標記，先略過
    pushq %rbp                      ← 建立這個函式的堆疊框（stack frame）
    movq  %rsp, %rbp
    leaq  .LC0(%rip), %rax          ← 取字串的位址
    movq  %rax, %rdi                ← 放進 %rdi：第 1 個參數
    call  puts@PLT                  ← 呼叫 puts（！）
    movl  $0, %eax                  ← 回傳值 0 放進 %eax
    popq  %rbp
    ret
```

兩個觀念：

1. **參數怎麼傳、回傳值放哪，是約定好的。** x86-64 Linux 遵循 System V AMD64 ABI
   （Application Binary Interface，應用程式二進位介面）：第 1 個整數／指標參數放 `%rdi`，
   回傳值放 `%eax`／`%rax`。所以 `return 0;` 變成 `movl $0, %eax`。第 ③ 站會細講堆疊框。
2. **意外：程式寫的是 `printf`，組合語言卻是 `puts`。** `printf("%s\n", 字串)` 的效果
   跟 `puts(字串)` 完全一樣（`puts` 本來就會補換行），而 `puts` 不用解析格式字串、比較快，
   gcc 就**自動換掉了**——即使是 `-O0`（不最佳化）。這是 gcc 對標準函式庫函式的內建知識
   （builtin）。加 `-fno-builtin` 就會老實地保留：
   ```
   $ gcc -O0 -fno-builtin -S hello.c -o nb.s && grep call nb.s
       call printf@PLT
   ```

**教訓：編譯器只保證「行為相同」，不保證「照你寫的字面去做」。** 之後讀 kernel、除錯效能問題時，
要看的是編譯器實際產生的東西，而不是原始碼的字面。

---

## 四、組譯：機器碼裡的「洞」

```sh
gcc -c hello.c -o hello.o
file hello.o
# hello.o: ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), not stripped
```

`.o` 是 **ELF**（Executable and Linkable Format，可執行與可連結格式）的**可重定位目的檔
（relocatable object file）**——已經是機器碼，但還不能執行。用 `objdump` 反組譯，並加 `-r` 顯示重定位資訊：

```
$ objdump -dr hello.o
  12:	e8 00 00 00 00       	call   17 <main+0x17>
			13: R_X86_64_PLT32	puts-0x4
```

`e8` 是 call 指令，後面四個 byte 應該是「要跳去哪」——**全是 0**。組譯器不知道 `puts` 在哪裡
（它根本不在這個檔案裡），所以留了一個洞，並記下一筆**重定位項目（relocation entry）**：
「位移 `0x13` 的地方，將來要填上 `puts` 的位址」。字串 `.LC0` 的位址（`lea 0x0(%rip)`）也是同樣的洞。

`nm` 列出目的檔的**符號表（symbol table）**，看得更直接：

```
$ nm hello.o
0000000000000000 T main      ← T：定義在程式碼區（text），這個檔案提供它
                 U puts      ← U：未定義（undefined），這個檔案需要別人提供
```

---

## 五、連結：把洞接上，但 `puts` 還是沒接完

```sh
gcc hello.o -o hello
file hello
# hello: ELF 64-bit LSB pie executable, x86-64, ..., dynamically linked,
#        interpreter /lib64/ld-linux-x86-64.so.2, ...
```

連結器（linker）做兩件事：

1. **符號解析（symbol resolution）**：每個 `U` 都要找到某個地方的定義。
2. **重定位（relocation）**：決定每段程式與資料最後放在哪個位址，把第四節的洞填上。

它還會加入你沒寫的東西：C 執行期的啟動碼（startup code）。**程式不是從 `main` 開始執行的**——
是啟動碼先設定好環境，再呼叫 `main`，`main` 回傳後再用它的回傳值結束行程。

但看連結後的符號表：

```
$ nm hello | grep -E " main| puts"
0000000000001149 T main
                 U puts@GLIBC_2.2.5     ← 還是 U！
```

**`puts` 在連結之後仍然未定義。** 因為這是**動態連結（dynamic linking）**：`puts` 的程式碼
在共享函式庫 `libc.so.6` 裡，連結器只記下「需要 glibc 2.2.5 版介面的 `puts`」，
真正的位址要等**執行時**才填。`file` 輸出裡的 `interpreter /lib64/ld-linux-x86-64.so.2`
就是負責這件事的**動態載入器（dynamic loader）**。`ldd` 列出執行時需要的函式庫：

```
$ ldd ./hello
	linux-vdso.so.1 (0x00007b51555aa000)                   ← 核心直接映射進來的特殊區塊
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (...)
	/lib64/ld-linux-x86-64.so.2 (...)                      ← 動態載入器本身
```

動態連結的好處是：每支程式不必各塞一份 libc，而且 libc 修了安全漏洞，所有程式重開就生效。
代價是執行時要多做一次載入與解析。（`gcc -static` 可以做靜態連結，把 libc 整份放進去——第 ① 站
之後的作業會比較兩者。）

---

## 六、執行：從 `execve` 到 `write`

`strace` 記錄程式做的每一個**系統呼叫（system call）**——程式請核心幫忙做事的唯一入口。
本機 `strace ./hello` 的輸出（節錄，去掉記憶體保護等細節）：

```
execve("./hello", ["./hello"], ...) = 0                         ① 核心載入 ELF，看到 interpreter，先把 ld.so 跑起來
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3   ② ld.so 查函式庫在哪
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", ...) = 3   ③ 打開 libc
read(3, "\177ELF\2\1\1\3...", 832) = 832                       ④ 讀它的 ELF 標頭（開頭就是 \177ELF）
mmap(..., PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|..., 3, 0x28000) = ...
                                                                ⑤ 把 libc 的程式碼映射進記憶體
close(3) = 0
write(1, "hello, world\n", 13) = 13                             ⑥ puts 最後做的事：寫 13 bytes 到 fd 1
exit_group(0) = ?                                               ⑦ main 回傳 0 → 行程以 0 結束
```

串起三件你已經知道的事：

- ① `execve` 就是 [root 執行腳本那篇](../linux/root執行腳本也Permission-denied-缺執行位元.md) 裡
  「需要 x 位元」的那個系統呼叫。腳本的 `#!/bin/sh` 和 ELF 的 interpreter 是同一個概念：
  核心讀開頭，決定要先啟動誰。
- ⑥ **`printf`／`puts` 不是魔法，最後是 `write(1, ...)`**——對 fd 1（標準輸出）寫入。
  fd 1 接到終端機、檔案還是管線，`hello` 不知道也不在乎（見 [fd是什麼](../linux/fd是什麼-行程的檔案描述符表.md)）。
- ⑦ `return 0` → `exit_group(0)` → shell 裡的 `$?` 是 0。

`strace -c` 統計了整個過程：**34 次系統呼叫**，其中真正「印字」的只有 1 次 `write`，
其餘幾乎都是動態載入器在準備環境。這個數字第 ② 站會再用到。

---

## 七、本課用到的工具與選項

### `gcc`：控制停在哪個階段

`gcc` 的選項有上千個，**這裡只列「控制編譯階段與觀察過程」這一類**；完整清單見 `man gcc`（Overall Options 一節是這一類的來源）。

| 選項 | 作用 | 產出 |
|---|---|---|
| `-E` | 只做前處理，停下來 | C（預設印到 stdout，用 `-o` 存檔） |
| `-S` | 做到編譯，停下來 | 組合語言 `.s` |
| `-c` | 做到組譯，停下來，不連結 | 目的檔 `.o` |
| （都不加） | 四個階段全做 | 可執行檔（沒有 `-o` 時叫 `a.out`） |
| `-o <檔名>` | 指定輸出檔名 | — |
| `-v` | 印出實際呼叫的每個子程式與參數 | — |
| `-save-temps` | 全部做完，但保留 `.i`、`.s`、`.o` | — |
| `-O0`／`-O1`／`-O2`／`-O3`／`-Os` | 最佳化等級，`-O0` 是不最佳化（預設） | — |
| `-fno-builtin` | 不把標準函式庫函式當成內建知識處理（本課用來阻止 `printf`→`puts`） | — |
| `-static` | 靜態連結，把函式庫程式碼放進可執行檔 | — |

### 觀察工具

| 工具 | 本課的用法 | 看什麼 | 完整說明 |
|---|---|---|---|
| `file` | `file hello.o` | 檔案類型：relocatable／executable、動態或靜態、interpreter | `man file` |
| `nm` | `nm hello.o` | 符號表 | `man nm` |
| `objdump` | `-d` 反組譯、`-r` 顯示重定位、`--no-show-raw-insn` 不印機器碼位元組 | 機器碼與洞 | `man objdump` |
| `ldd` | `ldd ./hello` | 執行時需要哪些共享函式庫 | `man ldd`（注意：`ldd` 可能真的執行程式，不要對不信任的檔案用） |
| `strace` | `strace ./hello`；`-f` 連子行程一起追；`-c` 只印統計 | 系統呼叫 | `man strace` |

`nm` 的符號類型字母，**這裡只列本課與下一課會遇到的**（完整清單見 `man nm`）。大寫是全域、小寫是只在本檔可見（`static`），
本機用一個小檔案實測：

| 字母 | 意思 | 對應的 C |
|---|---|---|
| `T`／`t` | 程式碼區（text） | 函式 |
| `D`／`d` | 已初始化的資料區（data） | `int counter = 5;`（`static int hidden = 1;` 是 `d`） |
| `B`／`b` | 未初始化的資料區（BSS） | `int zeroed;` |
| `R`／`r` | 唯讀資料區（read-only） | `const int answer = 42;` |
| `U` | 未定義，需要別人提供 | `extern int elsewhere;`、呼叫的外部函式 |

---

## 八、動手作業

在自己的機器上做一遍（不要只看本文的輸出）：

1. 寫出 `hello.c`，用 `-E`、`-S`、`-c` 各停一次，`ls -l` 比較四個檔案的大小。
2. 在 `hello.i` 裡找到 `printf` 的宣告；想一想：如果刪掉 `#include <stdio.h>`，會在哪個階段出錯？實際試試看，看錯誤訊息是什麼。
3. 把 `printf("%s\n", GREETING);` 改成 `printf("hi %d\n", 42);`，再看 `.s`——這次還會變成 `puts` 嗎？為什麼？
4. `gcc -static hello.c -o hello_static`，比較 `ls -l`、`file`、`ldd`、`strace -c` 的結果跟動態版有什麼不同。
5. （有 CS:APP 的話）讀第 1 章 1.1～1.4 節。

做完把觀察到的結果（尤其是第 2、3、4 題）貼回來，一起對答案。

---

## 自我測驗

1. `gcc hello.c` 背後實際是哪幾個程式在接力？為什麼說 `gcc` 是「總指揮」？
2. `hello.i` 裡有 `printf` 的宣告，卻沒有它的程式碼。那它的程式碼在哪？是在哪個時間點、由誰把「呼叫 `printf`」和「`printf` 的程式碼」接起來的？
3. `hello.o` 裡 `call` 指令後面的四個 byte 為什麼是 `00 00 00 00`？什麼資訊讓連結器知道要把它填成什麼？
4. 連結完成後 `nm hello` 仍然顯示 `U puts@GLIBC_2.2.5`，這是連結失敗嗎？為什麼？
5. 情境題：你把一支動態連結的程式複製到另一台機器，執行時出現找不到 `libc.so.6` 某個版本符號的錯誤。從本課的哪個階段可以解釋這件事？如果改用 `-static` 編譯，這個問題還會發生嗎？
