# procfs（process filesystem，行程檔案系統，即 `/proc`）是什麼，以及為什麼 `/proc/net/nf_conntrack` 不存在

> 日期：2026-07-25
> 起因：`lsmod` 明明看得到 `nf_conntrack` 已載入，但 `/proc/net/nf_conntrack` 卻不存在
> 結論先講：**這是核心編譯期的決定（`CONFIG_NF_CONNTRACK_PROCFS` 沒開），不是模組沒裝，也沒有任何 runtime 開關可以打開它**
> 關聯：[排查方法論.md](./排查方法論.md)（「東西不在該在的地方」的通用找法）、[核心模組檢查.md](./核心模組檢查.md)（模組三層安裝概念）

---

## 目錄

1. [問題現場](#一問題現場)
2. [procfs 是什麼](#二procfs-是什麼)
3. [`/proc` 的兩大類內容](#三proc-的兩大類內容)
4. [`/proc/sys` 特例：可以寫的旋鈕](#四procsys-特例可以寫的旋鈕)
5. [回頭解 nf_conntrack](#五回頭解-nf_conntrack)
6. [替代方案：走 netlink](#六替代方案走-netlink)
7. [procfs / sysfs / netlink 的分工](#七procfs--sysfs--netlink-的分工)
8. [可帶走的通則](#八可帶走的通則)

---

## 一、問題現場

模組明明在跑，而且有三個使用者：

```bash
lsmod | grep -i conntrack
```
```
xt_conntrack           12288  2
nf_conntrack          192512  3 xt_conntrack,nf_nat,xt_MASQUERADE
nf_defrag_ipv6         24576  1 nf_conntrack
nf_defrag_ipv4         12288  1 nf_conntrack
```

功能也確實正常運作（連線追蹤表裡有 73 筆）：

```bash
cat /proc/sys/net/netfilter/nf_conntrack_count   # 73
cat /proc/sys/net/netfilter/nf_conntrack_max     # 262144
```

但檔案就是不在：

```bash
ls -l /proc/net/nf_conntrack
# ls: cannot access '/proc/net/nf_conntrack': No such file or directory
```

**注意這裡的矛盾**：`/proc/sys/net/netfilter/nf_conntrack_*` 存在，`/proc/net/nf_conntrack` 不存在。同樣掛在 `/proc` 底下，一個有一個沒有——這代表它們背後是**兩套不同的註冊機制**，不能因為前者在就推論後者也該在。

---

## 二、procfs 是什麼

`/proc` 是一個**虛擬檔案系統**（virtual filesystem）。確實有掛載：

```bash
mount | grep '^proc '
# proc on /proc type proc (rw,nosuid,nodev,noexec,relatime)
```

但它**不佔任何磁碟空間**。底下每個「檔案」都不是真的檔案，而是核心在你 `read()` 的那一瞬間**現場產生的一段文字**——你 `cat` 一次，核心就跑一次對應的函式，把內部資料格式化成字串回給你。

驗證它是即時產生的：

```bash
cat /proc/uptime; sleep 2; cat /proc/uptime
```
```
4823.19 38102.44
4825.23 38118.51     ← 數字變了，不是靜態檔案
```

### 為什麼要做成檔案

Unix 哲學是 "everything is a file"。與其為「查行程列表」「查記憶體用量」各發明一套 syscall，不如全部長成檔案——這樣 `cat`、`grep`、`awk`、任何語言的 `open()` 都能直接用，不需要學特殊 API。

**這就是理解 procfs 的入口問題**：

> `ps` 指令怎麼知道系統上有哪些行程在跑？

它不可能自己知道，資料一定得從核心來。答案是它去讀 `/proc/<PID>/status` 這類檔案。`ps`、`top`、`htop`、`free`、`lsof`、`netstat` 全部都只是在 parse `/proc` 底下的文字——沒有 procfs 它們就瞎了。

---

## 三、`/proc` 的兩大類內容

### 3-1 數字目錄 = 每個行程一個

`ls /proc` 看到的 `1`、`10`、`1023`… 那些數字就是 PID（Process ID，行程識別碼）：

```bash
cat /proc/self/cmdline    # 命令列（用 \0 分隔，可搭配 tr '\0' ' '）
cat /proc/self/maps       # 記憶體映射
ls -l /proc/self/fd/      # 開啟的所有 fd（symlink 指向真正的檔案/pipe/socket）
cat /proc/self/limits     # ulimit 各項上限
cat /proc/self/status     # 狀態、UID、記憶體摘要
cat /proc/self/environ    # 環境變數
```

`/proc/self` 是**魔術 symlink**，永遠指向「正在讀它的那個行程」。用 `/proc/<PID>/` 則可以看別人的（權限允許的話）。

### 3-2 系統全域資訊

| 路徑 | 內容 | 誰在讀它 |
|------|------|----------|
| `/proc/cpuinfo` | CPU 型號、核心數、旗標 | `lscpu` |
| `/proc/meminfo` | 記憶體各項統計 | `free` |
| `/proc/mounts` | 掛載表 | `mount`（無參數時） |
| `/proc/modules` | 已載入模組 | `lsmod` |
| `/proc/net/tcp` | TCP（Transmission Control Protocol，傳輸控制協定）連線表 | `netstat` / `ss`（`ss` 主要走 netlink） |
| `/proc/net/stat/nf_conntrack` | conntrack 的 per-CPU 統計 | `conntrack -S` |

> 注意最後一列：`/proc/net/stat/nf_conntrack`（統計）**存在**，`/proc/net/nf_conntrack`（連線列表）**不存在**。兩個不同的東西，別看到名字像就以為是同一個。

---

## 四、`/proc/sys` 特例：可以寫的旋鈕

`/proc` 絕大部分是唯讀的，但 `/proc/sys` 底下是**可寫的核心參數**，寫進去即時生效——這就是 `sysctl` 的本體：

```bash
cat  /proc/sys/net/ipv4/ip_forward          # 讀
echo 1 > /proc/sys/net/ipv4/ip_forward      # 寫（需 root）
sudo sysctl -w net.ipv4.ip_forward=1        # 等價寫法
```

**路徑對應規則**：`sysctl` 的點換成斜線，前面加 `/proc/sys/`

```
net.ipv4.ip_forward              ↔  /proc/sys/net/ipv4/ip_forward
net.netfilter.nf_conntrack_max   ↔  /proc/sys/net/netfilter/nf_conntrack_max
```

`echo` 寫進去的是**暫時的**，重開機消失；要永久生效寫進 `/etc/sysctl.conf` 或 `/etc/sysctl.d/*.conf`。

想知道有哪些旋鈕可調，直接列：

```bash
sysctl -a | grep conntrack        # 或
ls /proc/sys/net/netfilter/
```

---

## 五、回頭解 nf_conntrack

查核心編譯設定（Ubuntu 會把設定檔放在 `/boot/config-$(uname -r)`）：

```bash
grep -E "CONFIG_NF_CONNTRACK(_PROCFS)?=" /boot/config-$(uname -r)
grep NF_CONNTRACK_PROCFS /boot/config-$(uname -r)
```
```
CONFIG_NF_CONNTRACK=m
# CONFIG_NF_CONNTRACK_PROCFS is not set     ← 兇手
```

### 為什麼「模組載入了」還是沒有檔案

`/proc/net/nf_conntrack` 這個節點，是 `nf_conntrack` 模組載入時**主動去 procfs 註冊**才會出現的。而那段註冊程式碼在原始碼裡被 `#ifdef CONFIG_NF_CONNTRACK_PROCFS` 包住——選項沒開，那段程式碼**根本沒被編進模組**，載入時自然不會註冊。

它不是「檔案遺失」或「權限問題」，是**核心從來沒打算生它**。

### 三個層次要分開看

| 層次 | 決定時機 | 本例狀態 | 能不能事後改 |
|------|----------|----------|--------------|
| 核心編譯選項 `CONFIG_NF_CONNTRACK_PROCFS` | 編譯 kernel 時 | **未開啟** | ❌ 只能重編核心 |
| 模組是否載入 `nf_conntrack` | `modprobe` / 自動載入 | 已載入 | ✅ |
| procfs 節點是否註冊 | 模組載入時，受編譯選項控制 | 沒註冊 | ❌ 沒有 sysctl／模組參數可開 |

所以 `modprobe nf_conntrack` 再跑一百次也不會生出這個檔案。

### 為什麼 Ubuntu 要關掉

`CONFIG_NF_CONNTRACK_PROCFS` 在核心 3.x 之後就被標為 **deprecated**，預設 `n`。Debian/Ubuntu 很久以前就跟進關閉。官方立場是：這個介面應該被 netlink 取代（見下節）。

---

## 六、替代方案：走 netlink

官方取代方案是 `NFNETLINK_CONNTRACK`，使用者端工具是 `conntrack`：

```bash
sudo apt install conntrack
```

```bash
sudo conntrack -L            # 列出所有連線（等同舊的 cat /proc/net/nf_conntrack）
sudo conntrack -L -p tcp     # 只看 TCP
sudo conntrack -C            # 只印當前連線數
sudo conntrack -E            # 即時 event 串流（tail -f 的效果）
sudo conntrack -S            # per-CPU 統計：drop / invalid / insert_failed
sudo conntrack -F            # 清空整張表（謹慎）
```

沒裝工具時的替代觀察點（這些不受該編譯選項影響）：

```bash
cat /proc/sys/net/netfilter/nf_conntrack_count    # 當前筆數
cat /proc/sys/net/netfilter/nf_conntrack_max      # 上限
cat /proc/net/stat/nf_conntrack                   # per-CPU 統計原始數字
```

---

## 七、procfs / sysfs / netlink 的分工

`/proc` 因為歷史因素塞了太多雜物（`/proc/meminfo` 一個檔案裡幾十個數值，要自己 parse），後來的設計把東西往外分流：

| 介面 | 設計原則 | 現在該放什麼 |
|------|----------|--------------|
| **procfs** `/proc` | 早期，格式自由、一檔多值 | 行程資訊（它的本分）、`/proc/sys` 參數 |
| **sysfs（system filesystem，系統檔案系統）** `/sys` | 一個檔案一個值，嚴格對應裝置模型 | 裝置 / 驅動 / bus 屬性 |
| **netlink** | socket 式 API，二進位、可訂閱事件 | 網路狀態與統計（`ss`、`ip`、`conntrack` 都走這條） |

`CONFIG_NF_CONNTRACK_PROCFS` 被 deprecated，就是這股「把東西推離 procfs」潮流的一部分。

**副作用**：舊教學／StackOverflow 答案大量使用 `cat /proc/net/nf_conntrack`，在現代發行版上會直接失敗。看到舊文章叫你 cat 某個 `/proc` 檔卻找不到時，先想「這介面是不是被淘汰了」。

---

## 八、可帶走的通則

1. **`/proc` 底下的東西是「有人註冊才存在」的，不是天生就有。** 不同節點由不同模組／子系統各自註冊，彼此獨立。`/proc/sys/net/netfilter/*` 在，不代表 `/proc/net/nf_conntrack` 該在。

2. **分清楚「編譯期」與「執行期」。** 執行期的手段（`modprobe`、`sysctl`、改權限）對編譯期就被 `#ifdef` 排除的功能完全無效。查 `/boot/config-$(uname -r)` 是最快的判定方式。

3. **模組「載入了」不等於「功能全開」。** 同一個模組的功能可以被編譯選項切掉一部分——這是 [核心模組檢查.md](./核心模組檢查.md) 裡「三層安裝」概念的延伸：還有第四層是「編進去的功能子集」。

4. **找不到東西時先推翻前提**（見 [排查方法論.md](./排查方法論.md)）。本例中「前提」就是「conntrack 表一定在 /proc/net」——這個前提在 2026 年的 Ubuntu 上根本不成立。

---

## 總結

**`/proc` 是核心把內部狀態偽裝成檔案攤出來給你看的地方，內容即時產生、不佔磁碟；`/proc/sys` 是其中可寫的參數旋鈕。**

**`/proc/net/nf_conntrack` 不存在，是因為 Ubuntu 編譯核心時關閉了 `CONFIG_NF_CONNTRACK_PROCFS`（該介面已 deprecated）。conntrack 功能本身完全正常，改用 `conntrack` 指令走 netlink 查詢即可。**

```bash
# 一行判定這類問題
grep -i "<某功能>" /boot/config-$(uname -r)
# 出現 "# CONFIG_XXX is not set" → 編譯期就沒了，別再找 runtime 開關
```
