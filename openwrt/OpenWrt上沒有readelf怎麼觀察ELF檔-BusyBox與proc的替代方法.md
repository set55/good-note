# OpenWrt 上沒有 `readelf`、`file`、`nm`，怎麼觀察 ELF 檔？—— BusyBox 與 `/proc` 的替代方法

> 日期：2026-09-24
> 情境：學完第 ① 站第 1 課的 ELF 相關筆記後，想在 OpenWrt 路由器上用同樣的方法觀察檔案，卻發現 `readelf`、`file`、`nm` 都不存在
> 適用範圍：OpenWrt 24.10.2（`mediatek/filogic`，aarch64），BusyBox 1.36.1，musl 1.2.5；全部經 `ssh root@192.168.1.1` 實測，**沒有安裝任何套件**
> 相關：[ELF是什麼](../c/ELF是什麼-可執行與可連結格式的兩種視角.md)、[ELF的interpreter是什麼](../c/ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md)、
> [objdump能不能代替ldd](../c/objdump能不能代替ldd找依賴庫-需求單與實際載入.md)、[GNU是什麼](../linux/GNU是什麼-Linux核心與GNU工具的分工.md)

**一句話結論：路由器為了省空間，只放了 BusyBox 的精簡工具，沒有 binutils。但不裝任何東西也能觀察四件事：
`ldd` 其實存在（musl 的 C 函式庫兼任）、`hexdump -C` 可以直接讀 ELF 檔頭、`strings` 找得到 interpreter 路徑、
`/proc/<PID>/maps` 看得到**執行時實際載入**了什麼。要完整的 `readelf`／`file`／`strace`，可以用 `opkg` 安裝，
或把檔案複製回 Ubuntu 再看。**

---

## 一、先搞清楚路由器上有什麼

OpenWrt 的 BusyBox 編譯時關掉了說明與 `--list` 功能：

```
root@OpenWrt:~# busybox
busybox: applet not found
root@OpenWrt:~# busybox --list
-list: applet not found
```

所以要反過來查：BusyBox 的每個工具（applet）都是一個指向 `busybox` 的符號連結，把它們找出來就是完整清單：

```
root@OpenWrt:~# for d in /bin /sbin /usr/bin /usr/sbin; do ls -l $d | grep -- '-> .*busybox'; done
```

實測共 119 個（加 `| wc -l` 計數），跟「觀察檔案」有關的只有這些：`hexdump`、`strings`、`cmp`、`md5sum`、`sha256sum`、`dd`、`head`、`readlink`、`which`、`ls`。
**沒有** `od`、`xxd`、`file`、`readelf`、`objdump`、`nm`、`strace`、`gdb`。

跟 Ubuntu 的對照：

| Ubuntu 上用的 | 路由器上的替代 | 能看到什麼 | 本篇 |
|---|---|---|---|
| `ldd` | `ldd`（**存在**，是 musl 兼任的） | 依賴哪些函式庫、解析到哪個路徑 | 第二節 |
| `readelf -h`、`file` | `hexdump -C -n 64` | ELF 檔頭：位元數、位元組順序、型別、CPU、進入點 | 第三節 |
| `readelf -l \| grep interpreter` | `strings \| grep ld-` | interpreter 路徑（粗略） | 第四節 |
| `ldd`（看執行時） | `cat /proc/<PID>/maps` | **執行中的程式實際載入了哪些檔案、權限是什麼** | 第五節 |
| `readelf`、`nm`、`objdump`、`file`、`strace`、`gdb` | `opkg install ...`，或複製回 Ubuntu | 全部 | 第六節 |

---

## 二、`ldd` 其實存在：musl 的函式庫兼任

```
root@OpenWrt:~# ls -l /usr/bin/ldd
lrwxrwxrwx  1 root root  17 Jun 23  2025 /usr/bin/ldd -> ../../lib/libc.so

root@OpenWrt:~# ldd /bin/busybox
	/lib/ld-musl-aarch64.so.1 (0x7f8a980000)
	libgcc_s.so.1 => /lib/libgcc_s.so.1 (0x7f8a94f000)
	libc.so => /lib/ld-musl-aarch64.so.1 (0x7f8a980000)
```

`ldd` 是指向 `libc.so` 的符號連結，而 `ld-musl-aarch64.so.1` 也指向 `libc.so`（見 [GNU是什麼](../linux/GNU是什麼-Linux核心與GNU工具的分工.md) 第四節）。
**同一個檔案同時是 C 函式庫、動態載入器、`ldd`**。它看自己被用什麼名字叫起來（`argv[0]`）決定扮演哪個角色，
跟 BusyBox「一支程式、看名字決定當哪個工具」是同一招。

輸出也印證了這一點：`libc.so => /lib/ld-musl-aarch64.so.1`，函式庫和載入器在同一個位址 `0x7f8a980000`，因為它們就是同一個檔案。

跟 Ubuntu 的差別：glibc 的 `ldd` 是 bash 腳本，要設定 `LD_TRACE_LOADED_OBJECTS=1` 再請載入器處理
（見 [objdump能不能代替ldd](../c/objdump能不能代替ldd找依賴庫-需求單與實際載入.md) 第二節）；musl 則是載入器本身直接認得 `ldd` 這個名字。
兩者的共同點是：**都要讓載入器實際處理一次目標檔案**，所以對來路不明的檔案，一樣不建議用 `ldd`。（musl 版 `ldd` 的安全性本機未實測。）

musl 的 `ldd` 沒有任何選項，只接受檔案路徑；不加參數時會印出版本（`musl libc (aarch64) / Version 1.2.5`）。

---

## 三、`hexdump -C` 直接讀 ELF 檔頭

沒有 `readelf -h`，但 ELF 檔頭的格式是固定的，前 64 個 byte 直接讀就好：

```
root@OpenWrt:~# hexdump -C -n 64 /bin/busybox
00000000  7f 45 4c 46 02 01 01 00  00 00 00 00 00 00 00 00  |.ELF............|
00000010  02 00 b7 00 01 00 00 00  00 65 40 00 00 00 00 00  |.........e@.....|
00000020  40 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |@...............|
00000030  00 00 00 00 40 00 38 00  08 00 40 00 00 00 00 00  |....@.8...@.....|
```

`hexdump -C` 每行：最左邊是**位移**（從檔案開頭算第幾個 byte，十六進位），中間 16 個 byte，右邊是對應的 ASCII（不可印的顯示成 `.`）。
逐欄對照（位移都是十六進位）：

| 位移 | byte | 欄位 | 意思 | `readelf -h` 的寫法 |
|---|---|---|---|---|
| `0x00`～`0x03` | `7f 45 4c 46` | 魔術數字 | `\x7f` + `E` `L` `F`，右邊 ASCII 欄看得到 `.ELF` | `Magic: 7f 45 4c 46 ...` |
| `0x04` | `02` | Class | `1` = 32 位元，`2` = **64 位元** | `ELF64` |
| `0x05` | `01` | Data | `1` = **little endian**，`2` = big endian | `little endian` |
| `0x10`～`0x11` | `02 00` | Type | 數值 2 = **EXEC**（1 = REL、3 = DYN、4 = CORE） | `EXEC (Executable file)` |
| `0x12`～`0x13` | `b7 00` | Machine | `0xb7` = **AArch64**（x86-64 是 `0x3e`） | `AArch64` |
| `0x18`～`0x1f` | `00 65 40 00 00 00 00 00` | Entry point | 進入點位址 `0x406500` | `0x406500` |

**多 byte 的欄位要倒過來讀**：`Type` 的兩個 byte 是 `02 00`，數值是 `0x0002`；進入點的 8 個 byte 是 `00 65 40 00 …`，
數值是 `0x0000000000406500`。這就是位移 `0x05` 那個 `01` 說的 **little endian：數字的低位 byte 放在前面**。
這是第 2 課的主題，這裡先知道「要倒過來讀」就好。

驗證：把同一支 `busybox` 複製回 Ubuntu，用 `readelf -h` 讀，結果完全一致：

```
$ readelf -h busybox
  Class:                             ELF64
  Data:                              2's complement, little endian
  Type:                              EXEC (Executable file)
  Machine:                           AArch64
  Entry point address:               0x406500
```

順帶一個發現：路由器的 `busybox` 是 **EXEC**（固定位址），不是 Ubuntu 上一般程式的 DYN（PIE）。
這支程式沒有編成 PIE（OpenWrt 為什麼這樣編，本篇沒有查證），所以程式本身不享有 ASLR 的位址隨機化，第五節 `maps` 裡它固定在 `0x400000`
（PIE 與 ASLR 見 [ELF是什麼](../c/ELF是什麼-可執行與可連結格式的兩種視角.md) 第三節）。

BusyBox `hexdump` 的全部選項（`hexdump --help` 實測，共 11 個，全列）：

```
hexdump [-bcdoxCv] [-e FMT] [-f FMT_FILE] [-n LEN] [-s OFS] [FILE]...
```

| 選項 | 作用 |
|---|---|
| `-b` | 每個 byte 用八進位顯示 |
| `-c` | 每個 byte 用字元顯示 |
| `-d` | 每 2 個 byte 當一個數字，用十進位顯示 |
| `-o` | 每 2 個 byte 當一個數字，用八進位顯示 |
| `-x` | 每 2 個 byte 當一個數字，用十六進位顯示 |
| `-C` | 標準格式：十六進位＋ASCII，每行 16 個 byte（**本篇用的**，最直觀） |
| `-v` | 全部顯示；預設會把重複的行折成一個 `*` |
| `-e FORMAT_STR` | 自訂輸出格式，例如 `'16/1 "%02x|""\n"'` |
| `-f FORMAT_FILE` | 從檔案讀自訂格式 |
| `-n LENGTH` | 只顯示前 LENGTH 個 byte（本篇用 `-n 64` 只看檔頭） |
| `-s OFFSET` | 跳過前 OFFSET 個 byte 再開始，例如 `-s 18 -n 2` 只看 Machine 欄 |

注意：`-d`、`-o`、`-x` 把 2 個 byte 合成一個數字時，**會照本機的位元組順序組合**，所以看到的數字跟 `-C` 的 byte 順序是反的。讀檔頭建議用 `-C`。實測：

```
root@OpenWrt:~# hexdump -C -n 4 /bin/busybox
00000000  7f 45 4c 46                                       |.ELF|
root@OpenWrt:~# hexdump -x -n 4 /bin/busybox
0000000    457f    464c                  ← 7f 45 變成 457f：低位 byte 在前，組成數字時放到後面
```

---

## 四、`strings` 找 interpreter 路徑（粗略）

interpreter 的路徑以文字形式存在 `.interp` 區段裡，所以 `strings` 挖得出來：

```
root@OpenWrt:~# strings /bin/busybox | grep ld-musl
/lib/ld-musl-aarch64.so.1
```

**這只是粗略的方法**：`strings` 印出檔案裡所有「連續 4 個以上可印字元」的片段，不知道哪一段是 interpreter。
如果程式裡剛好有別的字串也含 `ld-musl`，就分不出來。要精確看，就用 `readelf -l`（第六節）。
另一個方法是 `ldd` 第一行，它就是 interpreter（第二節輸出的 `/lib/ld-musl-aarch64.so.1`）。

BusyBox `strings` 的全部選項（`strings --help` 實測，共 4 個，全列）：

| 選項 | 作用 |
|---|---|
| `-f` | 每行前面加上檔名（一次看多個檔案時用） |
| `-o` | 每行前面加上八進位的位移 |
| `-t o\|d\|x` | 每行前面加上位移，進位制可選八、十、十六進位 |
| `-n LEN` | 至少 LEN 個連續可印字元才算一個字串（預設 4） |

---

## 五、`/proc/<PID>/maps`：看執行時實際載入了什麼

前面三種都是讀**檔案**。想知道一支程式**正在執行時**實際載入了哪些檔案，不需要任何工具，核心就提供了：

```
root@OpenWrt:~# cat /proc/self/maps
00400000-00466000 r-xp 00000000 b3:07 4       /bin/busybox       ← 程式碼
0047f000-00480000 r--p 0006f000 b3:07 4       /bin/busybox       ← 唯讀資料
00480000-00481000 rw-p 00070000 b3:07 4       /bin/busybox       ← 可寫資料
35849000-3584a000 ---p 00000000 00:00 0       [heap]
3584a000-3584b000 rw-p 00000000 00:00 0       [heap]
7fb0b2d000-7fb0b5c000 r-xp 00000000 b3:07 353 /lib/libgcc_s.so.1
7fb0b5c000-7fb0b5d000 r--p 0001f000 b3:07 353 /lib/libgcc_s.so.1
7fb0b5d000-7fb0b5e000 rw-p 00020000 b3:07 353 /lib/libgcc_s.so.1
7fb0b5e000-7fb0be8000 r-xp 00000000 b3:07 351 /lib/libc.so       ← musl：函式庫＝載入器，只出現一個檔案
7fb0bf9000-7fb0bfb000 r--p 00000000 00:00 0   [vvar]
7fb0bfb000-7fb0bfc000 r-xp 00000000 00:00 0   [vdso]
7fb0bfc000-7fb0bff000 rw-p 0008e000 b3:07 351 /lib/libc.so
7fcbab7000-7fcbad8000 rw-p 00000000 00:00 0   [stack]
```

`self` 代表「讀這個檔案的行程自己」，這裡就是 `cat`（`cat` 是 BusyBox，所以第一個檔案是 `/bin/busybox`）。
要看別的程式，換成它的 PID，例如 `cat /proc/$(pidof dnsmasq)/maps`。

每一行的欄位：

| 欄位 | 例子 | 意思 |
|---|---|---|
| 位址範圍 | `00400000-00466000` | 這一塊在行程記憶體裡的起訖位址 |
| 權限 | `r-xp` | 讀（`r`）、寫（`w`）、執行（`x`），沒有的寫 `-`；最後一個字母 `p` 是 private（私有），`s` 是 shared（共享） |
| 檔案位移 | `0006f000` | 這一塊對應檔案裡從第幾個 byte 開始 |
| 裝置 | `b3:07` | 檔案所在裝置的主、次編號 |
| inode | `4` | 檔案在該檔案系統的 inode 編號 |
| 路徑 | `/bin/busybox` | 對應的檔案；方括號是核心自己產生的區域（`[heap]`、`[stack]`、`[vdso]`） |

這張表可以跟兩篇筆記對起來：

- **權限一塊一塊分開**：`busybox` 分成 `r-xp`、`r--p`、`rw-p` 三塊，正是 [ELF是什麼](../c/ELF是什麼-可執行與可連結格式的兩種視角.md) 第四節說的
  「連結器把權限相同的區段打包成載入段，核心照著一塊塊放進記憶體」。
- **`ldd` 說的是「會載入什麼」，`maps` 說的是「已經載入了什麼」**。`ldd` 請載入器模擬一次；`maps` 是核心記錄的實際結果，
  而且不需要另外執行目標程式（程式本來就在跑）。

---

## 六、需要完整工具時：`opkg` 安裝，或複製回 Ubuntu

路由器的套件庫裡有完整的工具，`opkg info` 查得到（**以下只查詢、沒有實際安裝**，裝完後的內容未實測）：

| 套件 | 提供 | 依賴 | 套件大小 |
|---|---|---|---|
| `binutils` | 連結器、組譯器等 binutils 工具（本機 Ubuntu 的 `readelf`、`nm` 都屬於 binutils；OpenWrt 版實際含哪些檔案未實測，裝完可用 `opkg files binutils` 確認） | `libc`、`objdump`、`ar` | 約 1.1 MB |
| `objdump` | `objdump` | `libc`、`libopcodes`、`libctf` | 約 149 KB |
| `file` | `file` | `libc`、`libmagic` | 約 8.7 KB（另加 `libmagic`） |
| `xxd` | `xxd` 十六進位傾印 | `libc` | 約 8.6 KB |
| `strace` | `strace` | `libc` | 約 395 KB |
| `gdb` | `gdb` | `libc`、`libreadline8`、`libncurses6`、`zlib`、`libgmp10`、`libmpfr6` | 約 3.1 MB |

本機路由器的 `/overlay` 還有 6.8 GB 可用，空間不是問題；一般的小型路由器（快閃記憶體只有 16～32 MB）就要斟酌。
安裝：`opkg update` 之後 `opkg install binutils file strace`（會改變路由器上的軟體，需要時再做）。

**不想在路由器上裝東西**：把檔案複製回 Ubuntu 用完整的工具看。本機實測，OpenWrt 沒有 `sftp-server`，新版 `scp` 預設走 SFTP 協定會失敗：

```
$ scp root@192.168.1.1:/bin/busybox .
ash: /usr/libexec/sftp-server: not found
scp: Connection closed

$ scp -O root@192.168.1.1:/bin/busybox .     ← -O 改用舊的 SCP 協定
```

（原因見 [scp用法與OpenSSH9的協定變動](../linux/scp用法與OpenSSH9的協定變動.md)。）複製回來後 `readelf`、`nm -D`、`objdump` 都能讀 aarch64 的檔案，
但要注意兩點（見 [objdump能不能代替ldd](../c/objdump能不能代替ldd找依賴庫-需求單與實際載入.md) 第三節）：
`ldd` 會誤報 `not a dynamic executable`；OpenWrt 的檔案剝掉了區段標頭，`objdump -p` 看不到 `NEEDED`，要用 `readelf -d`。

### `opkg` 的子命令（本篇用到 `list`、`info`、`search`，共 26 個中的 3 個）

`opkg` 不帶參數執行就會印出完整說明。子命令依它自己的分組全部列出：

**套件操作（6 個）**

| 子命令 | 作用 |
|---|---|
| `update` | 更新可安裝套件的清單（從網路下載） |
| `upgrade <pkgs>` | 升級套件 |
| `install <pkgs>` | 安裝套件 |
| `configure <pkgs>` | 設定已解開、尚未設定的套件 |
| `remove <pkgs\|regexp>` | 移除套件 |
| `flag <flag> <pkgs>` | 設定套件旗標（`hold`、`noprune`、`user`、`ok`、`installed`、`unpacked`，一次一個） |

**查詢（20 個）**

| 子命令 | 作用 |
|---|---|
| `list` | 列出可安裝的套件（本篇用來找 `binutils` 等） |
| `list-installed` | 列出已安裝的套件 |
| `list-upgradable` | 列出已安裝且可升級的套件 |
| `list-changed-conffiles` | 列出使用者改過的設定檔 |
| `files <pkg>` | 列出某個**已安裝**套件的所有檔案 |
| `search <file\|regexp>` | 查某個檔案屬於哪個套件（本篇用來確認 `strings`、`hexdump` 屬於 `busybox`） |
| `find <regexp>` | 用名稱或說明搜尋套件 |
| `info [pkg\|regexp]` | 顯示套件的完整資訊（本篇用來看依賴與大小） |
| `status [pkg\|regexp]` | 顯示套件的安裝狀態 |
| `download <pkg>` | 只下載套件到目前目錄，不安裝 |
| `compare-versions <v1> <op> <v2>` | 比較兩個版本號（`<=`、`<`、`>`、`>=`、`=`、`<<`、`>>`） |
| `print-architecture` | 列出可安裝的套件架構 |
| `depends` | 列出套件依賴哪些套件 |
| `whatdepends` | 列出哪些套件依賴它 |
| `whatdependsrec` | 同上，遞迴往上找 |
| `whatrecommends` | 列出哪些套件推薦它 |
| `whatsuggests` | 列出哪些套件建議它 |
| `whatprovides` | 列出哪些套件提供它 |
| `whatconflicts` | 列出哪些套件跟它衝突 |
| `whatreplaces` | 列出哪些套件取代它 |

`depends` 以下 8 個都可以加 `-A`：查**所有**套件，而不只已安裝的。

選項（`-V` 詳細程度、`-f` 設定檔、`-d` 安裝目的地、`--force-*` 一系列強制選項、`--noaction` 只模擬不執行等）約 40 個，這裡**不列**，
完整清單見路由器上直接執行 `opkg`（不加參數）的輸出。

---

## 自我測驗

1. 路由器上的 `ldd`、`ld-musl-aarch64.so.1`、`libc.so` 是什麼關係？同一個檔案怎麼知道自己該當哪個角色？這跟 BusyBox 有什麼相同之處？
2. `hexdump -C` 讀到 `Type` 欄是 `02 00`、進入點是 `00 65 40 00 00 00 00 00`，為什麼數值分別是 2 和 `0x406500`，而不是 `0x0200` 和 `0x00654000…`？檔頭的哪一個 byte 告訴你要這樣讀？
3. `ldd /bin/busybox` 和 `cat /proc/<PID>/maps` 都能看到函式庫，兩者看到的東西差在哪個時間點？哪一個不需要讓載入器另外處理一次目標檔案？
4. 情境題：你在路由器上發現一支來路不明的程式 `/tmp/x`，想知道它是哪種 CPU 的、需要哪個 interpreter，但不想執行它，也不想在路由器上安裝套件。你會用本篇的哪些方法？為什麼不用 `ldd`？
