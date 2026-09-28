# GNU 是什麼？—— 為什麼 gcc、gdb、glibc 都叫 GNU，而 Linux 只是其中的「核心」

> 日期：2026-09-22
> 情境：第 ① 站第 1 課一路用了 gcc、GNU ld、`gdb`（GNU Debugger）、glibc，追問「GNU 是什麼」
> 適用範圍：本機 Ubuntu 24.04 的版本輸出為實測；歷史年份與授權條款依 GNU／FSF 公開資料
> 相關：[hello.c怎麼變成能跑的程式-編譯的四個階段.md](../c/hello.c怎麼變成能跑的程式-編譯的四個階段.md)、
> [Linux與系統程式的系統化學習路線.md](../learning/Linux與系統程式的系統化學習路線.md)（kernel 維護者路線）

**一句話結論：GNU 是一個 1983 年開始的「自由軟體作業系統」計畫，名字是遞迴縮寫 *GNU's Not Unix*。
它做出了幾乎所有使用者空間的基本工具——gcc、binutils、glibc、bash、coreutils、gdb、make——卻一直缺一個
可用的核心；1991 年的 Linux 核心補上了這一塊。所以你平常用的「Linux 系統」其實是 **Linux 核心 ＋ GNU 工具**，
（注意：`uname -o` 印出 `GNU/Linux` **不能**當成證據——它是編譯進 `uname` 程式裡的固定字串，連 OpenWrt 的 BusyBox 版也印 `GNU/Linux`，見第四節。）反過來，OpenWrt、Android 用的是 Linux 核心，但**不是** GNU
的工具——這正是你在路由器上遇到 BusyBox、`ash` 行為不同的根源。**

---

## 一、名字與起源

- **GNU** 是 *GNU's Not Unix* 的遞迴縮寫（recursive acronym）：縮寫裡又包含它自己。
  唸法是「g-noo」，G 要發音。
- 1983 年由 Richard Stallman 發起，目標是做出一套**完整、像 Unix、而且是自由軟體**的作業系統。
  當時的 Unix 是 AT&T 的商業軟體，使用者不能拿到原始碼、不能修改、不能分享。
- 1985 年成立 **FSF**（Free Software Foundation，自由軟體基金會）支持這個計畫。

這裡的「自由」（free）指的是**自由**，不是免費（*free as in freedom, not free beer*）。
GNU 對自由軟體的定義是四項自由：執行、研究並修改（所以要能拿到原始碼）、再散布、散布修改後的版本。

---

## 二、GNU 做了什麼：你這台機器上到處都是

本機實測，第 1 課用到的工具幾乎全出自 GNU 計畫：

| 工具 | 本機 `--version` 第一行 | 在第 1 課扮演的角色 |
|---|---|---|
| gcc（GNU Compiler Collection） | `gcc (Ubuntu 13.3.0-...) 13.3.0` | 編譯器總指揮 |
| binutils | `GNU ld (GNU Binutils for Ubuntu) 2.42` | `as`、`ld`、`nm`、`objdump`、`readelf` 都在這一包 |
| glibc（GNU C Library） | `ldd (Ubuntu GLIBC 2.39-...) 2.39` | `puts`、`printf` 的真正程式碼、動態載入器 |
| gdb（GNU Debugger） | `GNU gdb (Ubuntu 15.1-...) 15.1` | 除錯器 |
| coreutils | `ls (GNU coreutils) 9.4` | `ls`、`cp`、`chmod`、`cat` 等基本指令 |
| bash（Bourne Again SHell） | `GNU bash, version 5.2.21(1)-release (x86_64-pc-linux-gnu)` | 互動 shell（本機的 `/bin/sh` 也指向它） |
| make | `GNU Make 4.3` | 建置工具 |
| grep | `grep (GNU grep) 3.11` | 搜尋 |

連 **Linux 核心本身也是用 GNU 工具編出來的**：

```
$ cat /proc/version
Linux version 7.0.0-31-generic (...) (x86_64-linux-gnu-gcc-13 (...) 13.3.0, GNU ld (GNU Binutils for Ubuntu) 2.42) ...
```

### 那個到處出現的 `x86_64-linux-gnu` 是什麼

你已經在很多地方看過它：`/usr/lib/x86_64-linux-gnu/libc.so.6`、`x86_64-linux-gnu-gcc-13`、
bash 版本字串裡的 `x86_64-pc-linux-gnu`。這叫**目標三元組（target triple）**，描述「程式要跑在什麼環境」：

```
x86_64 - linux - gnu
  │       │      └─ ABI／C 函式庫：用 glibc（GNU 的 C 函式庫與它的呼叫慣例）
  │       └──────── 作業系統核心：Linux
  └──────────────── CPU 架構：64 位元 x86
```

同樣是 Linux 核心，C 函式庫換成別的，三元組就不同：OpenWrt、Alpine 用 musl，就是 `x86_64-linux-musl`
（或路由器的 `mips-openwrt-linux-musl` 這類）。**三元組把「核心」和「C 函式庫」分成兩欄，正好說明兩者是不同的東西。**

---

## 三、GNU 缺的那一塊：核心，以及「GNU/Linux」之爭

到 1990 年代初，GNU 已經有編譯器、shell、C 函式庫和大部分工具，但它自己的核心 **GNU Hurd** 一直沒有成熟到能實用。

1991 年 Linus Torvalds 發表了 **Linux 核心**，隔年改用 GNU 的 GPL（見下節）授權。把 Linux 核心和 GNU 工具
放在一起，就第一次得到一套完整、可用的自由作業系統。

```
┌─────────────────────────────────────────────┐
│ 使用者空間：bash、coreutils、gcc、gdb …      │  ← 大多來自 GNU
│ C 函式庫：glibc                               │  ← GNU
├──────────────── 系統呼叫 ────────────────────┤  ← 第 1 課 strace 看到的那一層
│ 核心：Linux                                   │  ← Linus Torvalds 與 kernel 社群
└─────────────────────────────────────────────┘
```

因此 FSF 主張應該叫「**GNU/Linux**」，認為只叫「Linux」抹去了 GNU 的貢獻；多數人日常還是簡稱 Linux。
（`uname -o` 在本機印出 `GNU/Linux`，但那只是 `uname` 這支程式自己寫死的字串，不是核心回報的，見第四節。）**嚴格說，「Linux」這個詞只指核心。** 對你的 kernel 目標來說，
這個區分很重要：你之後要貢獻的是橫線以下那一塊，`strace` 看到的系統呼叫就是它和上面那一層的邊界。

---

## 四、不是 GNU 的 Linux：你已經遇過了

「Linux 核心 ＋ 別家的使用者空間」完全可以成立，而且你在路由器上已經碰到：

| 系統 | 核心 | C 函式庫 | 基本工具與 shell |
|---|---|---|---|
| Ubuntu（本機） | Linux | glibc（GNU） | GNU coreutils、bash |
| OpenWrt（你的路由器） | Linux | musl | **BusyBox**（`ash`、`chmod`、`ls` 都是同一個程式） |
| Alpine（常見的 Docker 映像） | Linux | musl | BusyBox |
| Android | Linux | bionic | toybox |

這解釋了之前幾篇筆記裡的差異：

- [shell的case語法](./shell的case語法-樣式比對與分支.md)：`[^...]` bash 認、dash 不認——bash 是 GNU 的，POSIX 只規定 `[!...]`。
- [root執行腳本也Permission-denied](./root執行腳本也Permission-denied-缺執行位元.md)：OpenWrt 的 `chmod` 是 BusyBox 版，選項比 GNU coreutils 少。
- 很多 GNU 工具有 POSIX 沒規定的擴充選項（例如長選項 `--version`）。**寫要在路由器上跑的腳本時，只能假設 POSIX 有的功能。**

常見的誤解是「把開頭改成 `#!/bin/busybox` 就好」（2026-09-23 自我測驗中出現）。這樣行不通：

```
$ cat t1.sh
#!/usr/bin/busybox
echo hi
$ ./t1.sh
t1.sh: applet not found
```

核心照 `#!` 的規則執行 `busybox ./t1.sh`（見 [ELF的interpreter是什麼](../c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md) 第四節），
busybox 把第一個參數 `t1.sh` 當成要執行的工具名稱，找不到就報 `applet not found`。
另外，OpenWrt 沒有 bash，是因為**沒有安裝 bash 套件**，跟 C 函式庫用 musl 無關：bash 和 musl 是兩個不同的東西。
路由器實測：`/bin/bash` 不存在，`/bin/sh -> busybox`。

**本機要特別小心：`/bin/sh` 指向 bash。** Ubuntu 預設的 `/bin/sh` 是 dash，但這台機器被改成了 bash：

```
$ ls -l /bin/sh
lrwxrwxrwx 1 root root 4 May 24  2025 /bin/sh -> bash
```

所以就算腳本第一行寫 `#!/bin/sh`，在本機跑的還是 bash，bash 的擴充語法照樣能跑，**在本機完全測不出問題**。
要在本機模擬路由器，就要明確用精簡的 shell 來跑：

```
$ cat t4.sh
#!/bin/sh
arr=(a b); echo ${arr[1]}          ← bash 陣列
$ busybox sh t4.sh
t4.sh: line 2: syntax error: unexpected "("

$ cat t5.sh
#!/bin/sh
set -- a b; echo "$2"              ← POSIX 的寫法：用位置參數代替陣列
$ busybox sh t5.sh
b
$ dash t5.sh
b
```

（本機也裝了 BusyBox：`busybox` 印出 `BusyBox v1.36.1 (Ubuntu ...) multi-call binary`，可以用它在本機模擬路由器上的指令行為。）

### `uname -o` 印出 `GNU/Linux`，不代表它是 GNU

> 補充日期：2026-09-23。使用者在 OpenWrt 上打 `uname --all`，最後一欄是 `GNU/Linux`，跟本節「OpenWrt 不是 GNU」看起來矛盾。
> 原本的一句話結論與第三節把 `uname -o` 當成 GNU 的證據，是**筆記寫錯**，已更正。

**結論：`uname` 的輸出分兩種來源。前五欄是向核心問來的；`-o`（operating system）那一欄核心根本沒有，
是 `uname` 這支程式自己寫死的字串。BusyBox 的 `uname` 預設也寫死 `GNU/Linux`，所以 OpenWrt 上一樣印 `GNU/Linux`。**

**證據一：`-o` 根本沒有問核心。** 用 `strace` 只追 `uname` 這個系統呼叫：

```
$ strace -e trace=uname uname -s
uname({sysname="Linux", nodename="set-Vector-17-HX-A14VFG", ...}) = 0     ← 有問核心
Linux

$ strace -e trace=uname uname -o
GNU/Linux                                                                 ← 沒有任何系統呼叫，直接印出
```

核心回傳的結構 `struct utsname`（`man 2 uname`）只有這幾個欄位，**沒有「作業系統名稱」**：

```
struct utsname {
    char sysname[];    /* 核心名稱，例如 "Linux"  → uname -s */
    char nodename[];   /* 主機名稱               → uname -n */
    char release[];    /* 核心版本號              → uname -r */
    char version[];    /* 核心的建置資訊          → uname -v */
    char machine[];    /* 硬體架構               → uname -m */
    char domainname[]; /* NIS 網域名稱（GNU 擴充，uname 指令不印） */
};
```

所以 `-o` 的答案只能來自 `uname` 程式本身：GNU coreutils 在編譯時決定這個字串，glibc 系統上就是 `GNU/Linux`。

**證據二：BusyBox 的 `uname` 也印 `GNU/Linux`。** 本機剛好有 BusyBox（Ubuntu 套件，v1.36.1），它就是 OpenWrt 用的同一套工具：

```
$ busybox uname -o
GNU/Linux

$ strings /usr/bin/busybox | grep -x GNU/Linux
GNU/Linux                   ← 字串就放在 busybox 執行檔裡
```

BusyBox 的原始碼把這個字串做成編譯設定（`CONFIG_UNAME_OSNAME`），預設值就是 `"GNU/Linux"`，OpenWrt 沒有改它。

**證據三：在路由器上實測**（OpenWrt 24.10.2，`mediatek/filogic`，aarch64，2026-09-23 經 `ssh root@192.168.1.1` 執行）：

```
root@OpenWrt:~# uname -a
Linux OpenWrt 6.6.93 #0 SMP Mon Jun 23 20:40:36 2025 aarch64 GNU/Linux     ← 最後一欄一樣是 GNU/Linux

root@OpenWrt:~# ls -l $(which uname)
lrwxrwxrwx  1 root root  7 Jun 23  2025 /bin/uname -> busybox              ← uname 就是 BusyBox

root@OpenWrt:~# strings /bin/busybox | grep -x GNU/Linux
GNU/Linux                                                                  ← 字串寫在 busybox 裡

root@OpenWrt:~# ls -l /lib/ld-musl-*.so.1 /lib/libc.so
lrwxrwxrwx  1 root root       7 Jun 23  2025 /lib/ld-musl-aarch64.so.1 -> libc.so
-rwxr-xr-x  1 root root  590852 Jun 23  2025 /lib/libc.so

root@OpenWrt:~# /lib/ld-musl-aarch64.so.1
musl libc (aarch64)
Version 1.2.5
Dynamic Program Loader
```

`uname -o` 說自己是 `GNU/Linux`，但檔案顯示工具是 BusyBox、C 函式庫是 musl 1.2.5。**看檔案，不要看程式自我介紹的字串。**

判斷方法整理：

| 想確認的 | 執行 | OpenWrt 實測 |
|---|---|---|
| `uname` 是不是 BusyBox | `ls -l $(which uname)` | `/bin/uname -> busybox` |
| C 函式庫是不是 musl | `ls -l /lib/ld-musl-*.so.1` | 存在（musl 的動態載入器，名字裡直接寫 musl；glibc 系統是 `ld-linux-*.so.2`） |
| 同上，另一個角度 | 直接執行 `/lib/ld-musl-*.so.1`（不加參數） | 印出 `musl libc (aarch64)`、`Version 1.2.5` |

還有一個跟 glibc 很不一樣的地方：**musl 的載入器 `ld-musl-aarch64.so.1` 只是指向 `libc.so` 的符號連結——載入器和 C 函式庫是同一個檔案。**
glibc 則把兩者分成 `ld-linux-x86-64.so.2` 和 `libc.so.6` 兩個檔案（見[第 1 課筆記第十節](../c/hello.c怎麼變成能跑的程式-編譯的四個階段.md#十三個名字很像的檔案ldld-linux-x86-64so2libcso6)）。

這跟 [ELF 的 interpreter](../c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md) 是同一個思路：
**載入器的檔名就能說明這支程式是跟哪個 C 函式庫一起連結的。**

`uname` 的全部選項（本機 `uname --help`，GNU coreutils 版，共 11 個，全列）：

| 選項 | 印出什麼 | 來源 |
|---|---|---|
| `-a`, `--all` | 以下全部，依序；`-p`、`-i` 不知道時省略 | — |
| `-s`, `--kernel-name` | 核心名稱（`Linux`）；**不加任何選項時的預設** | 核心 `sysname` |
| `-n`, `--nodename` | 主機名稱 | 核心 `nodename` |
| `-r`, `--kernel-release` | 核心版本號（`7.0.0-31-generic`） | 核心 `release` |
| `-v`, `--kernel-version` | 核心建置資訊（`#31~24.04.1-Ubuntu SMP ...`） | 核心 `version` |
| `-m`, `--machine` | 硬體架構（`x86_64`） | 核心 `machine` |
| `-p`, `--processor` | 處理器類型（標明 non-portable，不同系統不一致；本機 `x86_64`） | 程式自己判斷 |
| `-i`, `--hardware-platform` | 硬體平台（non-portable；本機 `x86_64`） | 程式自己判斷 |
| `-o`, `--operating-system` | 作業系統名稱（`GNU/Linux`） | **程式寫死的字串** |
| `--help` | 印出說明後結束 | — |
| `--version` | 印出版本後結束 | — |

BusyBox 版（`busybox uname --help`）只有 `-a -m -n -r -s -p -v -i -o` 九個短選項，意思相同，沒有長選項。
查法：`uname --help`、`man uname`、`man 2 uname`（系統呼叫與 `struct utsname`）。

---

## 五、GPL：GNU 最深遠的影響

GNU 寫了 **GPL**（GNU General Public License，GNU 通用公共授權）來保護前面那四項自由。它的核心機制叫
**copyleft（著佐權）**：你可以自由使用、修改、散布，但**散布修改後的版本時，也必須用同樣的授權、提供原始碼**。
自由不能在傳遞途中被拿走。

跟你直接有關的兩點：

- **Linux 核心是 GPL 第 2 版**（GPL-2.0-only）。之後你送出的每個 kernel patch，都要加一行
  `Signed-off-by: 你的名字 <email>`，表示你確認自己有權以這個授權貢獻這段程式碼
  （這叫 DCO，Developer Certificate of Origin，開發者原創聲明；規則在 kernel 原始碼的
  `Documentation/process/submitting-patches.rst`，K1 會實際讀到）。
- **glibc 用的是 LGPL**（GNU Lesser General Public License，較寬鬆的版本）：允許非自由的程式**連結**它，
  不必開放自己的原始碼。這就是為什麼任何程式——包括商業軟體——都能動態連結 `libc.so.6`（[第 1 課第五節](../c/hello.c怎麼變成能跑的程式-編譯的四個階段.md#五連結把洞接上但-puts-還是沒接完)）。

本機的授權全文放在 `/usr/share/common-licenses/`（`GPL-2`、`GPL-3`、`LGPL-2.1` 等都在）。

---

## 自我測驗

1. 「Linux」這個詞嚴格說指的是什麼？那 `gcc`、`bash`、glibc 屬於誰？為什麼 FSF 主張叫 GNU/Linux？
2. 目標三元組 `x86_64-linux-gnu` 的三段各代表什麼？為什麼說它剛好說明了「核心」和「C 函式庫」是兩件事？
3. 情境題：一支 shell 腳本在你的 Ubuntu 上用 `bash` 測試完全正常，放到 OpenWrt 上卻出錯。從本篇的角度，最可能的原因是什麼？寫腳本時該怎麼預防？
4. GPL 的 copyleft 要求什麼？glibc 用 LGPL 而不是 GPL，對「商業軟體能不能連結 libc」有什麼差別？
5. 你將來送 kernel patch 時要加的 `Signed-off-by:` 跟 GPL 有什麼關係？
6. 情境題：同事說「我在這台機器上打 `uname -o` 看到 `GNU/Linux`，所以它一定用 glibc」。這個推論錯在哪？`uname` 的哪些欄位可以信、哪一欄不行，為什麼？你會改看什麼來判斷？
