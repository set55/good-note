# ELF 的 interpreter 是什麼？—— 動態載入器，以及它跟腳本 `#!` 是同一個機制

> 日期：2026-09-23
> 情境：第 ① 站第 1 課的延伸提問。筆記裡 `file` 的輸出一直出現 `interpreter /lib64/ld-linux-x86-64.so.2`，追問那是什麼
> 適用範圍：本機 Ubuntu 24.04、x86-64、glibc 2.39；所有輸出都是本機實測。本篇**不需要組合語言**
> 相關：[hello.c怎麼變成能跑的程式-編譯的四個階段.md](./hello.c怎麼變成能跑的程式-編譯的四個階段.md)（第五、六節）、
> [readelf完整選項參考.md](./readelf完整選項參考.md)、
> [root執行腳本也Permission-denied-缺執行位元.md](../linux/root執行腳本也Permission-denied-缺執行位元.md)（`#!` 與 `not found`）

**一句話結論：interpreter（直譯器，這裡指「程式直譯器」）是**執行檔自己指定的「請先幫我跑這個程式」**。
動態連結的 ELF 執行檔裡寫著一行 `/lib64/ld-linux-x86-64.so.2`，核心在 `execve` 時看到它，就**先把動態載入器
載進來、讓它先跑**，由它去載入 `libc.so.6`、把 `puts` 的位址填好，最後才跳進你的程式。這跟腳本第一行的
`#!/bin/sh` 是**同一個概念**：核心讀檔案開頭，決定真正要啟動的是誰。靜態連結的程式沒有這一行，核心就直接跑它。**

---

## 一、它寫在執行檔的哪裡

ELF 執行檔的開頭有一張**程式標頭表（program header table）**，告訴核心「載入這個檔案時要做哪些事」。
用 `readelf -l` 看（`-l` 是 program headers，完整選項見 [readelf完整選項參考](./readelf完整選項參考.md)）：

```
$ readelf -lW hello
Elf file type is DYN (Position-Independent Executable file)
...
  Type           Offset   ...
  PHDR           0x000040 ...
  INTERP         0x000318 ...
      [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]   ← 就是這一項
  LOAD           0x000000 ...
  LOAD           0x001000 ...
  ...
```

`INTERP` 這一項指向檔案裡一個叫 `.interp` 的區段，內容只是一串路徑字串：

```
$ readelf -p .interp hello
String dump of section '.interp':
  [     0]  /lib64/ld-linux-x86-64.so.2
```

第 1 課作業第 1 題解說裡，`.interp` 佔 28 bytes——就是這串路徑（27 個字元加上結尾的 `\0`）。

靜態連結的版本**沒有** `INTERP` 這一項：

```
$ readelf -lW hello_static | grep -i interp
（沒有輸出）
```

---

## 二、執行時發生什麼事

`./hello` 按下 Enter 之後：

```
① shell 呼叫 execve("./hello")
② 核心讀 hello 的程式標頭，看到 INTERP：/lib64/ld-linux-x86-64.so.2
③ 核心把 hello「和」ld-linux-x86-64.so.2 都載入記憶體，
   但先執行的是 ld-linux-x86-64.so.2，不是 hello
④ ld-linux-x86-64.so.2（動態載入器）開始工作：
     - 讀 hello 需要哪些函式庫 → libc.so.6
     - 查 /etc/ld.so.cache 找到它在哪 → 打開、映射進記憶體
     - 檢查版本需求（GLIBC_2.34 等）
     - 把 puts 等符號的真實位址填好
⑤ 動態載入器把控制權交給 hello 的啟動碼 → main → puts → write
```

這就是第 1 課 `strace ./hello` 看到的那一串：`execve` 之後的 `openat("/etc/ld.so.cache")`、
`openat("libc.so.6")`、`mmap`，都是**動態載入器在做事**，還沒輪到你的程式。

### 「跳進程式」是什麼意思：進入點（entry point）

> 2026-09-24 補教：複習時使用者問「跳進程式本身代表什麼？」——第 ⑤ 步之前只說了「交給」，沒說交到哪裡。

CPU（Central Processing Unit，中央處理器）隨時都記著「下一條要執行的指令在哪個位址」。
「跳進程式」就是載入器把這個位址改成**你的程式的第一條指令**，從那一刻起 CPU 執行的就是 `hello` 的程式碼，
載入器的工作結束。這個「第一條指令的位址」寫在 ELF 檔頭裡，叫**進入點（entry point）**：

```
$ readelf -h hello | grep Entry
  Entry point address:               0x1060

$ readelf -sW hello | grep -E ' _start| main$|__libc_start_main'
     1: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@GLIBC_2.34 (2)
    29: 0000000000001060    38 FUNC    GLOBAL DEFAULT   16 _start
    31: 0000000000001149    30 FUNC    GLOBAL DEFAULT   16 main
```

注意進入點 `0x1060` 是 **`_start`**，**不是 `main`**（`main` 在 `0x1149`）。`_start` 是 `gcc` 連結時自動加進來的啟動碼，
它做的事是呼叫 libc 的 `__libc_start_main`（表裡 `UND` 代表這個符號不在 `hello` 裡，要執行時由載入器接到 `libc.so.6`；
就是第 1 課第九節那個 `GLIBC_2.34`），再由 `__libc_start_main` 呼叫你的 `main`。所以完整的順序是：

```
核心 → 載入器（載入 libc、填位址）→ 跳到進入點 _start → __libc_start_main（在 libc 裡）→ main → puts
```

（這裡的 `0x1060` 是檔案裡的相對位址。本機的執行檔是 PIE，執行時整支程式會被放到一個隨機的基底位址，
真正跳去的是「基底＋`0x1060`」，見 [ELF 是什麼](./ELF是什麼-可執行與可連結格式的兩種視角.md) 的 `Type: DYN`。）

載入器自己也會告訴你它在什麼時候交出控制權。`LD_DEBUG=files` 的最後幾行（`LD_DEBUG` 的全部選項見
[第 1 課筆記第十節](./hello.c怎麼變成能跑的程式-編譯的四個階段.md#十三個名字很像的檔案ldld-linux-x86-64so2libcso6)）：

```
$ LD_DEBUG=files ./hello
     55130:	calling init: /lib64/ld-linux-x86-64.so.2
     55130:	calling init: /lib/x86_64-linux-gnu/libc.so.6
     55130:	transferring control: ./hello        ← 就是這一步：跳到 hello 的進入點
hello
```

釐清一個常見誤會：跳進去之後，程式的指令是 **CPU 執行的**，不是核心在執行。核心只在程式發出系統呼叫
（例如 `puts` 最後呼叫的 `write`）時才介入。「跳」在指令層級長什麼樣子，第 4、5 課讀組合語言時會實際看到。
`readelf` 的 `-h`、`-s` 等全部選項見 [readelf完整選項參考](./readelf完整選項參考.md)。

### 名字很多，都是同一個東西

| 稱呼 | 出處 |
|---|---|
| program interpreter（程式直譯器） | ELF 規格與 `readelf` 的用詞 |
| interpreter | `file` 的輸出 |
| dynamic linker／dynamic loader（動態連結器／動態載入器） | 一般文件；`gcc` 的選項叫 `--dynamic-linker` |
| `ld.so` | 手冊頁名稱（`man ld.so`）與它自己的 `--help` |
| `ld-linux-x86-64.so.2` | 本機實際的檔名 |

注意跟**連結器 `ld`** 區分：`ld` 在**編譯時**產生執行檔；`ld.so` 在**執行時**載入函式庫。名字像，工作完全不同。

這裡的「interpreter」跟 Python「直譯器」不是同一個意思：它不逐行翻譯程式，只負責**把程式準備好再交出去**。

---

## 三、它自己也是一支程式：可以直接執行

`ld-linux-x86-64.so.2` 本身可以當命令用，它的 `--help` 第一段就說明了自己的角色：

```
$ /lib64/ld-linux-x86-64.so.2 --help
Usage: /lib64/ld-linux-x86-64.so.2 [OPTION]... EXECUTABLE-FILE [ARGS-FOR-PROGRAM...]
You have invoked 'ld.so', the program interpreter for dynamically-linked
ELF programs.  Usually, the program interpreter is invoked automatically
when a dynamically-linked executable is started.
```

所以可以手動做核心平常做的事：

```
$ /lib64/ld-linux-x86-64.so.2 ./hello
hello, world
```

**`ldd` 其實就是在叫它**：本機的 `ldd` 是一支 bash 腳本（第一行 `#!/bin/bash`），裡面列出候選的載入器
（`RTLDLIST="/lib/ld-linux.so.2 /lib64/ld-linux-x86-64.so.2 ..."`），再用 `--list` 請它列出相依的函式庫：

```
$ /lib64/ld-linux-x86-64.so.2 --list ./hello
	linux-vdso.so.1 (0x...)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x...)
	/lib64/ld-linux-x86-64.so.2 (0x...)
```

跟 `ldd ./hello` 的輸出一模一樣。

它自己怎麼啟動？`file` 顯示它是 `static-pie linked`——**它自己不需要 interpreter**，否則就會無限循環
（誰來載入「載入器」？）。

### `ld.so` 的全部選項（依本機 `--help`，glibc 2.39）

| 選項 | 作用 |
|---|---|
| `--list` | 列出所有相依的函式庫與解析結果（`ldd` 用的就是這個） |
| `--verify` | 檢查給定的檔案是不是它能處理的動態連結物件 |
| `--inhibit-cache` | 不使用 `/etc/ld.so.cache` |
| `--library-path PATH` | 用 PATH 取代環境變數 `LD_LIBRARY_PATH` 的內容 |
| `--glibc-hwcaps-prepend LIST` | 搜尋 LIST 裡的 glibc-hwcaps 子目錄 |
| `--glibc-hwcaps-mask LIST` | 只搜尋 LIST 裡的內建子目錄 |
| `--inhibit-rpath LIST` | 對 LIST 裡的物件忽略 RUNPATH 與 RPATH 資訊 |
| `--audit LIST` | 以 LIST 裡的物件做為稽核器（auditor） |
| `--preload LIST` | 預先載入 LIST 裡的物件 |
| `--argv0 STRING` | 執行前把 `argv[0]` 設成 STRING |
| `--list-tunables` | 列出所有可調參數（tunables）與其最小、最大值 |
| `--list-diagnostics` | 列出診斷資訊 |
| `--help` | 顯示說明後結束 |
| `--version` | 顯示版本後結束 |

`--help` 最後還印出本機的函式庫搜尋路徑：先查 `/etc/ld.so.cache`，再依序找
`/lib/x86_64-linux-gnu`、`/usr/lib/x86_64-linux-gnu`、`/lib`、`/usr/lib`。

---

## 四、跟腳本的 `#!` 是同一個機制

腳本第一行的 `#!`（shebang）也是在告訴核心「請先啟動這個程式」。用一個故意選的直譯器看得最清楚：

```
$ cat catscript
#!/bin/cat
這一行會被 cat 印出來

$ chmod +x catscript && ./catscript
#!/bin/cat
這一行會被 cat 印出來
```

核心看到 `#!/bin/cat`，實際執行的是 `/bin/cat ./catscript`——**把腳本的路徑當成參數交給直譯器**（`#!` 後面寫的那支程式，這裡就是 `cat`），
所以 `cat` 自己打開這個路徑，把整個檔案（連第一行）印出來。

> 2026-09-27 使用者問：「這個參數不是交給 `cat` 了嗎？」——是的，**直譯器就是 `cat`**。「直譯器」是一個**角色**，不是某一類程式：
> `#!` 後面寫哪支程式，那支程式就是這個腳本的直譯器，核心不管它懂不懂語法，只負責執行它並把腳本路徑當參數交過去。
> ELF 那邊的「interpreter」同理，指的是 `INTERP` 寫的那支 `ld-linux-x86-64.so.2`。

`#!/bin/sh` 也是一樣：真正跑起來的是 `/bin/sh ./腳本`。

| | 腳本 | 動態連結的 ELF |
|---|---|---|
| 指定直譯器的地方 | 第一行 `#!路徑` | 程式標頭的 `INTERP`（`.interp` 區段） |
| 誰讀這個資訊 | 核心（`execve` 時） | 核心（`execve` 時） |
| 實際先跑的程式 | `/bin/sh`、`/bin/bash`… | `/lib64/ld-linux-x86-64.so.2` |
| 直譯器拿到檔案之後 | 逐行讀文字、執行命令 | 載入函式庫、填好位址，再跳進程式本身 |
| 沒有直譯器時 | 沒寫 `#!` 的文字檔無法用 `./` 直接執行 | 靜態連結：核心直接跑程式 |


### 交給直譯器的是「路徑」，不是內容：三個實驗

> 2026-09-27 補教：「核心把腳本交給直譯器」這句話，交的**形式**是什麼，連續幾次複習都沒答出來。下面用實驗直接看。

**實驗一：讓直譯器把自己拿到的參數印出來。** `echo` 不讀任何檔案，只把自己拿到的參數印出來；拿它當直譯器，印出來的就是核心交給直譯器的東西。建檔與執行：

```
$ printf '#!/usr/bin/echo\n這一行 echo 不會讀\n' > echoscript.sh
$ chmod +x echoscript.sh
$ ./echoscript.sh
./echoscript.sh
$ ./echoscript.sh aaa bbb
./echoscript.sh aaa bbb
```

印出來的是 `./echoscript.sh`——**你在命令列打的那個路徑**；你在命令列多給的參數 `aaa bbb` 接在後面。第二行的內容完全沒出現。
所以核心實際執行的命令列是：

```
/usr/bin/echo  ./echoscript.sh  aaa bbb
└ #! 後面的路徑  └ 腳本路徑        └ 你原本給的參數
```

**實驗二：看是誰打開了腳本檔。**

```
$ printf '#!/usr/bin/cat\necho "12345"\n' > catscript.sh
$ chmod +x catscript.sh
$ strace -e trace=execve,openat ./catscript.sh
execve("./catscript.sh", ["./catscript.sh"], 0x7ffc023285f0 /* 94 vars */) = 0
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3              ← cat 自己的動態載入
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/usr/lib/locale/locale-archive", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "./catscript.sh", O_RDONLY) = 3                            ← cat 自己打開腳本
#!/usr/bin/cat
echo "12345"
+++ exited with 0 +++
```

- 只有**一個** `execve`：`#!` 是核心在同一次 `execve` 裡處理掉的，不會多一次系統呼叫，所以用 `trace=execve` 看不到「第二次執行」。
- `execve` 之後的 `openat` 全是 `cat` 這個行程做的，其中 `openat(..., "./catscript.sh", ...)` 就是 `cat` 用拿到的路徑**自己打開**腳本。內容是 `cat` 讀出來的，不是核心交給它的。

**實驗三：只給執行權限，不給讀取權限。** 接著實驗二的 `catscript.sh`：

```
$ chmod 111 catscript.sh
$ ls -l catscript.sh
---x--x--x 1 set set 28 Sep 27 15:39 catscript.sh
$ ./catscript.sh
/usr/bin/cat: ./catscript.sh: Permission denied
```

錯誤訊息是 **`cat` 印的**（開頭是 `/usr/bin/cat:`）。核心讀得到 `#!` 那一行（核心自己讀檔，不受這個權限位元限制），也成功啟動了 `cat`；
但 `cat` 是普通行程，用你的身分去打開 `./catscript.sh`，沒有讀取權限就被擋下。**如果核心交的是內容，`cat` 根本不必打開檔案，這個錯誤就不會發生。**

對照：只有執行權限的 ELF 執行檔可以正常跑——程式本身是**核心**直接映射進記憶體的，不需要哪個使用者行程去打開它：

```
$ cp /usr/bin/true mytrue
$ chmod 111 mytrue
$ ./mytrue; echo "exit=$?"
exit=0
```

所以**腳本要同時有 `r` 和 `x` 才跑得起來**：`x` 給核心看（允許執行），`r` 給直譯器用（它要自己打開、讀取腳本）。

---

## 五、直譯器不存在時：明明有檔案，卻說找不到

故意把 interpreter 路徑改成不存在的檔案（`-Wl,` 把後面的選項轉交給連結器 `ld`，
`--dynamic-linker` 就是設定 `INTERP` 的選項）：

```
$ gcc hello.c -o hello_badinterp -Wl,--dynamic-linker=/lib64/nope.so.2
$ ls -l hello_badinterp
-rwxrwxr-x 1 set set 15960 Sep 23 00:32 hello_badinterp        ← 檔案明明在，也有 x
$ ./hello_badinterp
zsh: no such file or directory: ./hello_badinterp               ← zsh 的訊息
$ dash -c ./hello_badinterp
dash: 1: ./hello_badinterp: not found                           ← dash 的訊息
```

**「找不到」的不是你的程式，是它指定的 interpreter。** 核心回報的錯誤碼是「檔案不存在」，shell 只知道
「執行 `./hello_badinterp` 失敗、原因是檔案不存在」，就把它當成你的程式不存在。

這跟 [root執行腳本也Permission-denied](../linux/root執行腳本也Permission-denied-缺執行位元.md) 第六節的
「腳本 `#!/bin/bash` 但機器上沒有 bash」、「CRLF 讓直譯器變成 `/bin/sh\r`」是**同一個錯誤、同一個原因**。

實務上會遇到的情況：

- 把在 Ubuntu（glibc）編好的程式複製到 **Alpine 或 OpenWrt**（musl，見
  [GNU是什麼](../linux/GNU是什麼-Linux核心與GNU工具的分工.md)）：那邊沒有 `/lib64/ld-linux-x86-64.so.2`，
  就會出現這種「檔案明明在卻 not found」。
- 32 位元的程式放到沒裝 32 位元函式庫的 64 位元系統：它的 interpreter 是 `/lib/ld-linux.so.2`，不存在。

排查方法：`file 程式` 或 `readelf -l 程式 | grep interpreter` 看它要的是哪一個，再 `ls -l` 那個路徑確認存不存在。

---

## 自我測驗

本篇的題目都**不需要組合語言**，答案都在上面的文字裡。

1. 動態連結的執行檔裡，interpreter 的路徑寫在哪裡？`execve` 之後，核心先執行的是你的程式還是它？為什麼要這樣安排？
2. `ld` 和 `ld.so`（`ld-linux-x86-64.so.2`）各在什麼時間點、做什麼事？
3. 腳本的 `#!/bin/sh` 和 ELF 的 interpreter，哪裡相同、哪裡不同？用 `#!/bin/cat` 的實驗說明「核心把什麼交給直譯器」。
4. 為什麼靜態連結的程式沒有 interpreter 也能跑？`ld-linux-x86-64.so.2` 自己又為什麼不需要 interpreter？
5. 情境題：你把 Ubuntu 上編好的程式複製到一台 Alpine Linux，`ls -l` 看得到檔案、也有 x 權限，執行卻說 `not found`。最可能的原因是什麼？你會用哪兩個步驟確認？用第 1 課學過的哪一種編譯方式可以避開這個問題？
6. 情境題：一支腳本權限是 `---x--x--x`（只有執行權限），第一行是 `#!/bin/sh`，執行時出現 `/bin/sh: ./x.sh: Permission denied`。
   這個錯誤是誰印的？為什麼核心讀得到 `#!` 那一行，直譯器卻讀不到腳本？同樣只有執行權限的 ELF 執行檔為什麼就能跑？
