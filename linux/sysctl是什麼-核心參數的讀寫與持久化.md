# sysctl 是什麼？怎麼用？—— 核心參數的讀寫與持久化

> 日期：2026-09-20
> 情境：調 `net.core.somaxconn` 時順帶問的：sysctl 到底是什麼東西
> 適用範圍：實測 Ubuntu 24.04（kernel 7.0、1504 個可調參數）
> 相關：[procfs與conntrack遺失問題.md](./procfs與conntrack遺失問題.md)（`/proc/sys` 的機制在那篇）、
> [什麼是backlog-TCP交握背後的兩個佇列.md](./什麼是backlog-TCP交握背後的兩個佇列.md)

**一句話結論：sysctl 不是一個服務，也不是一個設定系統——它只是 `/proc/sys/` 底下那些檔案的
命令列包裝。「調核心參數」實際上就是「寫一個檔案」。理解這件事，所有「改了沒生效」的問題都能
歸到三類：寫錯地方、被覆蓋、或者你改的根本是另一個命名空間（namespace）的那一份。**

---

## 一、它其實就是在讀寫檔案

```bash
sysctl -n net.ipv4.ip_forward           # 等同 cat /proc/sys/net/ipv4/ip_forward
sudo sysctl -w net.ipv4.ip_forward=1    # 等同 echo 1 > /proc/sys/net/ipv4/ip_forward
```

**路徑對應規則：把點換成斜線，前面加 `/proc/sys/`。**

```
net.core.somaxconn    ↔  /proc/sys/net/core/somaxconn
vm.swappiness         ↔  /proc/sys/vm/swappiness
```

（`/proc/sys` 這個介面本身的機制——為什麼它可寫、和 `/proc` 其他部分有何不同——見
[procfs與conntrack遺失問題.md](./procfs與conntrack遺失問題.md) 第四節，這裡不重複。）

那為什麼還要 sysctl 這個工具？因為它多做了幾件 `cat`／`echo` 做不到的事：**批次列出、
從設定檔載入、開機自動套用、統一的錯誤處理**。

---

## 二、基本用法

```bash
sysctl -a                       # 列出全部（本機 1504 個）
sysctl -a | grep somaxconn      # 找參數
sysctl net.core.somaxconn       # 讀（連名字一起印）
sysctl -n net.core.somaxconn    # 只印值 ← 寫腳本用這個
sudo sysctl -w net.core.somaxconn=4096     # 寫（暫時）
sudo sysctl -p /etc/sysctl.d/99-x.conf     # 從指定檔案載入
sudo sysctl --system            # 依規則載入所有目錄的設定檔（開機時做的事）
```

參數的規模與分佈（本機實測）：

```
  1174  net.*         ← 佔了近八成
   149  kernel.*
    60  fs.*
    50  vm.*
```

**`net.*` 為什麼這麼多**：因為它是**每張網卡各一組**。本機 `net.ipv4.conf.*` 有 231 個項目，
分屬 7 個「介面」：

```
all  default  lo  enp58s0  wlp59s0f0  docker0  br-65aa0cb1b71e
```

其中 `all` 與 `default` 是特例：`all` 對所有介面生效，`default` 是**之後新建**介面的預設值——
所以改 `default` 不會影響已經存在的網卡，這是常見的困惑點。

---

## 三、唯讀還是可寫？看檔案權限就知道

既然是檔案，用 `ls -l` 一眼就能判斷，不必 root、也不必試：

```
  kernel/ostype          -r--r--r--     ← 唯讀，root 也改不了
  kernel/osrelease       -r--r--r--     ← 唯讀
  fs/file-nr             -r--r--r--     ← 唯讀（這是統計值，不是旋鈕）
  kernel/hostname        -rw-r--r--     ← 可寫
  vm/swappiness          -rw-r--r--     ← 可寫
  net/ipv4/ip_forward    -rw-r--r--     ← 可寫
```

**`/proc/sys` 底下同時有「旋鈕」和「儀表」**：可寫的是你能調的，唯讀的是核心報告給你看的
（像 `fs/file-nr` 是目前開啟的檔案描述符統計）。看到某個參數改不動，先確認它是不是本來就唯讀。

---

## 四、持久化：三個目錄與載入順序

`sysctl -w` 寫進去的東西**重開機就沒了**。要永久生效得寫成設定檔。

搜尋路徑（本機實測都存在）：

| 目錄 | 誰放的 | 優先權 |
|---|---|---|
| `/etc/sysctl.d/*.conf` | **系統管理員（你）** | 最高 |
| `/run/sysctl.d/*.conf` | 執行期產生的 | 中 |
| `/usr/lib/sysctl.d/*.conf` | **套件安裝時放的** | 最低 |
| `/etc/sysctl.conf` | 傳統單一檔案，仍然有效 | 通常最後套用 |

**載入規則有兩層，很容易搞混：**

1. **所有目錄的檔案先混在一起，按「檔名」排序**（不是按目錄排）——所以 `10-foo.conf`
   會比 `99-bar.conf` 先套用，**後套用的覆蓋先套用的**。
2. **同名檔案時，優先權高的目錄勝出**——`/etc/sysctl.d/10-x.conf` 會完全遮蔽
   `/usr/lib/sysctl.d/10-x.conf`（套件的預設值被你覆蓋，這是設計）。

所以自訂設定的慣例是用大數字開頭，例如 `/etc/sysctl.d/99-custom.conf`——確保它最後才套用。

開機時是誰做的：

```bash
$ systemctl cat systemd-sysctl.service
Description=Apply Kernel Variables
ExecStart=/usr/lib/systemd/systemd-sysctl
```

本機實際被調過的參數（節錄）：

```
fs.inotify.max_user_watches=1048576      # 檔案監看上限（IDE／打包工具常要調）
vm.max_map_count=1048576                 # 記憶體映射數上限（Elasticsearch 等要調）
net.ipv4.conf.default.rp_filter=2
-net.core.default_qdisc = fq_codel       # ← 開頭那個減號是什麼？
```

**開頭的 `-` 表示「這個參數不存在的話就安靜略過，不要報錯」。** 用在「需要某模組載入後才存在」
的參數上（例如 `fq_codel` 的排程器模組沒載入時）。沒有這個減號，開機時就會在日誌裡留下錯誤。

---

## 五、「我改了但沒生效」的五個原因

### 1. `-w` 只是暫時的

改完要同時寫進 `/etc/sysctl.d/99-xxx.conf`，否則重開機還原。

### 2. 非 root 執行時，輸出看起來像成功

```bash
$ sysctl -w vm.swappiness=60
sysctl: permission denied on key "vm.swappiness", ignoring
vm.swappiness = 60                # ← 這行照印！很容易誤以為成功
```

**錯誤只有第一行，後面照樣印出 `key = value`。** 一定要回頭 `sysctl -n` 驗證。

### 3. 參數根本不存在

模組沒載入時，對應的 `/proc/sys` 節點就不存在——這正是
[procfs與conntrack遺失問題.md](./procfs與conntrack遺失問題.md) 那篇的主題。先確認
`ls /proc/sys/<路徑>` 有東西，再談調值。

### 4. 被排序在後面的檔案覆蓋

多個檔案設了同一個參數時，檔名排序大的贏。用 `sysctl -n <參數>` 看最終結果，而不是相信自己
寫的那個檔案。

### 5. 命名空間隔離（最容易漏掉）

**`net.*` 參數是每個 network namespace 各一份。** 本機實測：

```bash
$ sysctl -n net.ipv4.ip_local_port_range
32768   60999                              # 主機

$ docker run --rm <image> cat /proc/sys/net/ipv4/ip_local_port_range
32768   60999                              # 容器預設繼承

$ docker run --rm --sysctl net.ipv4.ip_local_port_range="20000 30000" <image> \
      cat /proc/sys/net/ipv4/ip_local_port_range
20000   30000                              # 容器內改掉了

$ sysctl -n net.ipv4.ip_local_port_range
32768   60999                              # ← 主機完全不受影響
```

**所以在主機上調 `net.*` 不會影響容器，反之亦然。** 容器裡的服務要調 backlog、
port range 這類參數，得用 `--sysctl`（Docker）或 Pod 的 `securityContext.sysctls`（Kubernetes）。

相對地，`kernel.*`、`vm.*`、`fs.*` 大多是**全機共用**的（少數有 namespace 化），
在容器裡通常改不動——因為那會影響到主機。

---

## 六、常用參數速查

```bash
# 網路
net.ipv4.ip_forward=1                    # 開啟轉發（路由器／NAT／容器必備）
net.core.somaxconn=4096                  # accept queue 上限
net.ipv4.tcp_max_syn_backlog=4096        # SYN queue 上限
net.ipv4.tcp_syncookies=1                # 防 SYN flood
net.netfilter.nf_conntrack_max=262144    # 連線追蹤表上限

# 檔案與記憶體
fs.inotify.max_user_watches=1048576      # 監看檔案數（webpack/IDE 報 ENOSPC 時調這個）
fs.file-max                              # 全系統開檔上限
vm.swappiness=10                         # 越小越不愛用 swap
vm.max_map_count=1048576                 # mmap 區段數上限

# 查與驗
sysctl -a | grep <關鍵字>
sysctl -n <參數>                          # 驗證最終值 ← 改完一定要做
ls -l /proc/sys/<路徑>                    # 確認存在且可寫
```

---

## 自我測驗

1. sysctl 和 `echo 1 > /proc/sys/...` 的關係是什麼？既然等價，為什麼還需要 sysctl 這個工具？
2. 不需要 root、也不用實際嘗試，怎麼判斷一個核心參數是否可寫？為什麼 `/proc/sys` 底下會有
   改不了的項目？
3. `/etc/sysctl.d/10-a.conf` 和 `/etc/sysctl.d/99-b.conf` 設了同一個參數，最後哪個生效？
   如果是 `/etc/sysctl.d/10-a.conf` 和 `/usr/lib/sysctl.d/10-a.conf` 呢？兩題的規則不一樣，分別是什麼？
4. 你在主機上把 `net.core.somaxconn` 調到 4096，但容器裡的服務還是會 accept queue 溢位。
   為什麼？該怎麼改？
5. 情境題：同事說他執行 `sysctl -w fs.inotify.max_user_watches=1048576` 之後輸出顯示
   `fs.inotify.max_user_watches = 1048576`，但問題完全沒解決。請列出兩個最可能的原因，
   以及一行指令就能分辨是哪一個的驗證方法。
