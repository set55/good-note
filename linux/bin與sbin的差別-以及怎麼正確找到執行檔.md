# `/bin`、`/sbin`、`/usr/bin`、`/usr/sbin` 差在哪？—— 以及怎麼正確找到一個執行檔

> 日期：2026-09-20
> 情境：追問「為什麼查 iptables 直接去 `/usr/sbin` 找，而不是 `/usr/bin`」——
> 答案是「依慣例猜的」，而猜對不等於方法對
> 適用範圍：實測 Ubuntu 24.04（已 usrmerge）；FHS 規範適用於所有 Linux 發行版
> 相關：[排查方法論.md](./排查方法論.md)、[dpkg完整使用指南.md](./dpkg完整使用指南.md)、
> [從dpkg反推套件的啟動命令.md](./從dpkg反推套件的啟動命令.md)

**一句話結論：四個目錄的分法是兩個維度的交叉——`bin` vs `sbin` 看「一般使用者執行它有沒有意義」，
`/` vs `/usr` 看「開機早期就需不需要」。但在已 usrmerge 的現代系統上，第二個維度已經消失（`/bin`
只是指向 `usr/bin` 的符號連結）。而最重要的一點是：這些都只是**慣例**——要知道某個指令在哪，
應該問系統（`command -v`／`type -a`／`dpkg -S`），不是憑慣例猜路徑。**

---

## 一、先講方法：猜對不等於方法對

查 iptables 的執行檔時直接打了 `ls -l /usr/sbin/iptables*`——這是根據 FHS 慣例推測的路徑。
**它猜對了，但如果猜錯，回來的會是 `No such file or directory`，然後就要重猜第二次、第三次。**

正確的順序是先問系統：

```bash
$ command -v iptables
/usr/sbin/iptables

$ type -a iptables
iptables is /usr/sbin/iptables
iptables is /sbin/iptables          # ← 同一個檔案，見第三節
```

這正是 [排查方法論.md](./排查方法論.md) 講的「先推翻前提」：**「它應該在 `/usr/sbin`」是一個假設，
不是事實。** 用 `command -v` 只要一步就能把假設換成事實，沒有理由省這一步。

三個查法的差別：

| 指令 | 特性 |
|---|---|
| `command -v <cmd>` | **POSIX 標準、shell 內建**，最可靠；會考慮 alias／function／builtin |
| `type -a <cmd>` | bash／zsh 內建，**列出所有**符合的（包含被 PATH 順序蓋掉的） |
| `which <cmd>` | 外部程式，不知道 alias／function，行為在各發行版間不一致——**最不建議** |
| `whereis <cmd>` | 連 man page、原始碼一起找，但只搜固定目錄清單 |

---

## 二、四個目錄的分法

FHS（Filesystem Hierarchy Standard，檔案系統階層標準）的原始設計是**兩個維度**：

|  | 一般使用者會用的 | 只有系統管理才有意義的 |
|---|---|---|
| **開機／單人模式就必須有** | `/bin` | `/sbin` |
| **可以晚點才掛載** | `/usr/bin` | `/usr/sbin` |

- **`bin` vs `sbin`**：判準是「**一般使用者執行它有沒有意義**」，不完全等於「需不需要 root」——
  雖然兩者高度相關。像 `ip`、`fdisk` 放在 sbin，一般使用者其實也能跑 `ip addr` 來看資訊，
  但它們的主要用途是管理系統。
- **`/` vs `/usr`**：歷史原因。早期 `/usr` 可能掛在另一顆磁碟甚至網路上，開機初期還沒掛載，
  所以「開機救援必備」的東西必須放在根目錄底下。

### 實例：iptables 套件同時裝到兩邊

這是 FHS 判準最好的示範：

```bash
$ dpkg -L iptables | grep -E '^/usr/s?bin/'
/usr/sbin/iptables-apply
/usr/sbin/xtables-nft-multi        # iptables/ip6tables/arptables/ebtables 都是它的連結
/usr/sbin/xtables-legacy-multi
/usr/sbin/arptables-nft
/usr/sbin/ebtables-nft
...
/usr/bin/iptables-xml              # ← 只有這一個在 /usr/bin
```

**為什麼只有 `iptables-xml` 不一樣？** 因為它只是把 `iptables-save` 的輸出轉成 XML 格式——
純粹的文字處理，不碰核心、不需要權限，一般使用者拿它來分析規則檔完全合理。
其他每一個都會去讀寫核心的規則集，非 root 執行沒有意義。

**同一個套件、同一組工具，僅僅因為「一般使用者跑它有沒有意義」就被分到不同目錄。**

---

## 三、usrmerge：第二個維度已經消失

現代發行版（Ubuntu 20.04+、Debian 12+、Fedora 更早）都做了 **usrmerge**：

```bash
$ ls -ld /bin /sbin /lib /usr/bin /usr/sbin
lrwxrwxrwx  /bin  -> usr/bin        # ← 符號連結
lrwxrwxrwx  /sbin -> usr/sbin       # ← 符號連結
lrwxrwxrwx  /lib  -> usr/lib        # ← 符號連結
drwxr-xr-x  /usr/bin                # ← 真正的目錄
drwxr-xr-x  /usr/sbin               # ← 真正的目錄
```

所以：

- `/sbin/iptables` 和 `/usr/sbin/iptables` **是同一個檔案**，不是兩份。
- `type -a iptables` 列出兩行，不是因為有兩個 iptables，而是因為 PATH 裡同時有 `/usr/sbin`
  和 `/sbin` 這兩個路徑名，而它們指向同一處。
- 「開機早期 `/usr` 還沒掛載」的前提早就不成立了——現在 initramfs 會先把 `/usr` 準備好。

**維度剩下一個：只剩 `bin` vs `sbin` 還有意義。** 而 Debian 也正在推進把 `/usr/sbin` 併進
`/usr/bin`（DEP17），未來連這個區分都可能消失。本機目前仍是兩個獨立目錄。

---

## 四、PATH：誰先被找到

```bash
$ echo $PATH | tr ':' '\n' | nl
  ...
   7  /usr/local/sbin
   8  /usr/local/bin
   9  /usr/sbin        ← sbin 在前
  10  /usr/bin
  11  /sbin
  12  /bin
```

兩個觀察：

1. **一般使用者的 PATH 現在也包含 sbin。** 早期只有 root 的 PATH 有 sbin，所以一般使用者打
   `ifconfig` 會得到 `command not found`——那是 PATH 的問題，不是沒裝。現代 Ubuntu 在
   `/etc/environment` 裡就把 sbin 放進所有人的 PATH。
2. **`/usr/local/*` 排在系統目錄前面**，這是刻意的：讓你自己編譯安裝的版本蓋過套件版本。
   這也是「明明 `apt install` 了新版，跑起來還是舊版」的常見原因——用 `type -a` 一看就知道
   有兩個同名執行檔。

> 順帶一提，本機 PATH 有 30 個項目，其中 `/home/set/bin`、`/usr/local/go/bin`、`/home/set/go/bin`
> 各重複出現 4 次——通常是 shell 設定檔裡的 `export PATH=...:$PATH` 被重複執行（例如 `.zshrc`
> 與 `.zprofile` 都加了一次）。不影響正確性，但會讓每次查指令多掃幾個目錄。

---

## 五、完整的「這個執行檔在哪／是什麼」查法

```bash
# 1. 現在會執行哪一個
command -v <cmd>
type -a <cmd>                       # 列出全部，看有沒有被別的版本蓋掉

# 2. 它是不是連結、真身在哪
readlink -f "$(command -v <cmd>)"
ls -l "$(command -v <cmd>)"

# 3. 它屬於哪個套件
dpkg -S "$(readlink -f "$(command -v <cmd>)")"

# 4. 那個套件還裝了哪些執行檔（找出同系列工具）
dpkg -L <pkg> | grep -E '^/usr/s?bin/'

# 5. 如果根本不在 PATH 上（例如裝在 /opt）
dpkg -L <pkg> | grep -E '\.desktop$'   # GUI 程式的入口在這裡
```

**心法：問「這個指令在哪」永遠不要用 `ls` 去猜目錄——`command -v` 一步到位，而且會告訴你
shell 實際會執行哪一個，這跟「檔案存在於某處」是兩回事。**

---

## 自我測驗

1. `bin` 與 `sbin` 的分野是「需不需要 root」嗎？用 iptables 套件裡 `iptables-xml` 被放在
   `/usr/bin` 的例子說明真正的判準。
2. `type -a iptables` 顯示 `/usr/sbin/iptables` 和 `/sbin/iptables` 兩行，代表系統上有兩個
   iptables 嗎？為什麼？
3. `/` 與 `/usr` 的區分原本是為了解決什麼問題？為什麼這個理由在現代系統上已經不成立？
4. 你 `apt install` 升級了某個工具，但執行起來還是舊版本。用 PATH 的順序解釋，並說出哪一個
   指令能一眼看出問題。
5. 情境題：同事說「這台機器沒裝 `ip` 指令，因為我打 `ip addr` 說 command not found」。
   你要怎麼在兩步之內判斷他是真的沒裝、還是 PATH 的問題？
