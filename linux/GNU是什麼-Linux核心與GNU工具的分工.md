# GNU 是什麼？—— 為什麼 gcc、gdb、glibc 都叫 GNU，而 Linux 只是其中的「核心」

> 日期：2026-09-22
> 情境：第 ① 站第 1 課一路用了 gcc、GNU ld、`gdb`（GNU Debugger）、glibc，追問「GNU 是什麼」
> 適用範圍：本機 Ubuntu 24.04 的版本輸出為實測；歷史年份與授權條款依 GNU／FSF 公開資料
> 相關：[hello.c怎麼變成能跑的程式-編譯的四個階段.md](../c/hello.c怎麼變成能跑的程式-編譯的四個階段.md)、
> [Linux與系統程式的系統化學習路線.md](../learning/Linux與系統程式的系統化學習路線.md)（kernel 維護者路線）

**一句話結論：GNU 是一個 1983 年開始的「自由軟體作業系統」計畫，名字是遞迴縮寫 *GNU's Not Unix*。
它做出了幾乎所有使用者空間的基本工具——gcc、binutils、glibc、bash、coreutils、gdb、make——卻一直缺一個
可用的核心；1991 年的 Linux 核心補上了這一塊。所以你平常用的「Linux 系統」其實是 **Linux 核心 ＋ GNU 工具**，
這也是 `uname -o` 會印出 `GNU/Linux` 的原因。反過來，OpenWrt、Android 用的是 Linux 核心，但**不是** GNU
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
`uname -o` 在本機印出的就是 `GNU/Linux`。**嚴格說，「Linux」這個詞只指核心。** 對你的 kernel 目標來說，
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

（本機也裝了 BusyBox：`busybox` 印出 `BusyBox v1.36.1 (Ubuntu ...) multi-call binary`，可以用它在本機模擬路由器上的指令行為。）

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
  不必開放自己的原始碼。這就是為什麼任何程式——包括商業軟體——都能動態連結 `libc.so.6`（第 1 課第五節）。

本機的授權全文放在 `/usr/share/common-licenses/`（`GPL-2`、`GPL-3`、`LGPL-2.1` 等都在）。

---

## 自我測驗

1. 「Linux」這個詞嚴格說指的是什麼？那 `gcc`、`bash`、glibc 屬於誰？為什麼 FSF 主張叫 GNU/Linux？
2. 目標三元組 `x86_64-linux-gnu` 的三段各代表什麼？為什麼說它剛好說明了「核心」和「C 函式庫」是兩件事？
3. 情境題：一支 shell 腳本在你的 Ubuntu 上用 `bash` 測試完全正常，放到 OpenWrt 上卻出錯。從本篇的角度，最可能的原因是什麼？寫腳本時該怎麼預防？
4. GPL 的 copyleft 要求什麼？glibc 用 LGPL 而不是 GPL，對「商業軟體能不能連結 libc」有什麼差別？
5. 你將來送 kernel patch 時要加的 `Signed-off-by:` 跟 GPL 有什麼關係？
