# 用 `journalctl` 看 systemd 服務的日誌

> 日期：2026-09-21
> 情境：`systemctl` 啟動的服務怎麼看 log？服務又沒有自己的 log 檔
> 適用範圍：實測 Ubuntu 24.04、systemd 255、journal 已持久化（`/var/log/journal` 存在）
> 相關：[OpenWrt的服務管理-service與procd.md](../openwrt/OpenWrt的服務管理-service與procd.md)
> （OpenWrt 用 `logread`，機制完全不同）

**一句話結論：systemd 服務的 stdout／stderr 預設**直接被 journald 收走**，所以不用去找 log 檔——
`journalctl -u <服務名>` 就是。它存的不是純文字而是**帶結構化欄位的二進位資料庫**，
所以可以用 `-u`／`-p`／`--since`／`-g` 這些條件精確過濾，這是 `tail -f /var/log/xxx` 做不到的。**

---

## 一、為什麼服務沒有自己的 log 檔

systemd unit 的 `StandardOutput=` 預設是 `journal`，**服務印到 stdout／stderr 的東西會被
journald 直接收走**，不落地成檔案。所以：

```bash
systemctl cat docker | grep -i standard      # 多數 unit 根本沒寫這行，因為預設就是 journal
```

journald 收三種來源：**服務的 stdout／stderr、syslog(3) API、核心的 kmsg**，
全部放進同一個 journal，再用欄位區分。

---

## 二、最常用的幾招

```bash
journalctl -u v2ray                  # 某個服務的全部日誌
journalctl -u v2ray -f               # ★ 持續跟隨（等同 tail -f）
journalctl -u v2ray -n 50            # 最後 50 行
journalctl -u v2ray -e               # 直接跳到最後（用分頁器）
journalctl -u v2ray -r               # 最新的排最前面
journalctl -u v2ray --since '10 min ago'
journalctl -u v2ray -p err           # 只看 error 以上
journalctl -u v2ray -g 'timeout'     # 訊息內容用正規式過濾
journalctl -u v2ray -b               # 只看這次開機以來
journalctl -u v2ray --since today --until '1 hour ago'
```

**`-f` 和 `-e` 是日常最常用的兩個**：前者盯即時輸出，後者看「剛剛發生什麼」。

`--since`／`--until` 吃得懂人話：`"2026-09-21 13:00"`、`yesterday`、`today`、
`"10 min ago"`、`"-1h"`。

---

## 三、完整選項（依本機 `journalctl --help`，systemd 255）

### 來源選項

| 選項 | 作用 |
|---|---|
| `--system` / `--user` | **系統層／當前使用者層**的 journal |
| `-M` `--machine=<容器>` | 讀本機某個容器的 journal |
| `-m` `--merge` | 合併所有可用的 journal |
| `-D` `--directory=<路徑>` | 讀指定目錄的 journal 檔 |
| `--file=<路徑>` | 讀單一 journal 檔 |
| `--root=<路徑>` | 以另一個檔案系統根目錄操作（救援時用） |
| `--image=<路徑>` / `--image-policy=<政策>` | 直接讀磁碟映像 |
| `--namespace=<名稱>` | 讀指定的 journal namespace |

### 過濾選項

| 選項 | 作用 |
|---|---|
| `-u` `--unit=<單元>` | **指定服務** |
| `--user-unit=<單元>` | 指定使用者層服務 |
| `-S` `--since=<時間>` / `-U` `--until=<時間>` | 時間範圍 |
| `-b` `--boot[=ID]` | **這次開機**（`-b -1` 是上一次） |
| `-p` `--priority=<範圍>` | 優先度：`emerg`(0)／`alert`／`crit`／`err`(3)／`warning`／`notice`／`info`／`debug`(7)，可寫範圍 `-p 0..3` |
| `--facility=<設施>` | syslog facility |
| `-g` `--grep=<正規式>` | **訊息內容比對** |
| `--case-sensitive[=BOOL]` | 強制（不）區分大小寫 |
| `-t` `--identifier=<字串>` | 依 syslog identifier |
| `-k` `--dmesg` | **只看核心訊息**（等同 `dmesg`） |
| `-c` `--cursor` / `--after-cursor` / `--cursor-file` | 從游標位置繼續（**寫監控腳本用**） |

### 輸出控制

| 選項 | 作用 |
|---|---|
| `-o` `--output=<模式>` | `short`（預設）／`short-iso`／`short-precise`／`short-monotonic`／`short-unix`／`verbose`／`cat`／`with-unit`／`json`／`json-pretty`／`json-sse`／`json-seq`／`export` |
| `--output-fields=<清單>` | verbose／export／json 模式要印哪些欄位 |
| `-n` `--lines[=N]` | 顯示幾行 |
| `-r` `--reverse` | 最新在前 |
| `-f` `--follow` | **持續跟隨** |
| `--no-tail` | follow 模式也顯示全部歷史 |
| `-e` `--pager-end` | **直接跳到結尾** |
| `--show-cursor` | 結尾印出游標 |
| `--utc` | 時間用 UTC |
| `-x` `--catalog` | **附上訊息的說明文字**（有的話） |
| `-a` `--all` | 顯示所有欄位，含過長與不可印字元 |
| `--no-full` | 過長欄位用省略號 |
| `--no-hostname` | 不印主機名 |
| `--truncate-newline` | 遇到第一個換行就截斷 |
| `-q` `--quiet` | 不印提示與權限警告 |
| `--no-pager` | **不要用分頁器**（腳本必加） |

### 命令型（不是查詢，是操作）

| 選項 | 作用 |
|---|---|
| `-N` `--fields` | **列出目前用到的所有欄位名** |
| `-F` `--field=<欄位>` | **列出某欄位出現過的所有值** ← 探索用 |
| `--list-boots` | 列出記錄過的開機 |
| `--disk-usage` | **journal 佔多少磁碟** |
| `--vacuum-size=<位元組>` / `--vacuum-files=<n>` / `--vacuum-time=<時間>` | **清理舊日誌** |
| `--verify` | 驗證檔案一致性 |
| `--sync` / `--flush` / `--rotate` | 同步到磁碟／把 `/run` 的資料刷進 `/var`／立即輪替 |
| `--relinquish-var` / `--smart-relinquish-var` | 停止寫磁碟，改寫暫存檔 |
| `--header` | 顯示 journal 檔頭資訊 |
| `--list-catalog` / `--dump-catalog` / `--update-catalog` | 訊息目錄相關 |
| `--setup-keys` / `--interval` / `--verify-key` / `--force` | FSS（前向安全封印） |
| `-h` `--help` / `--version` | 說明／版本 |

---

## 四、結構化查詢：journal 真正的價值

journal 每筆訊息都帶一堆欄位，不只是一行字：

```bash
$ journalctl -u docker -n 1 -o json-pretty
{
    "PRIORITY" : "6",
    "_CMDLINE" : "/sbin/init splash",
    "_EXE" : "/usr/lib/systemd/systemd",
    "_BOOT_ID" : "54c126bc...",
    "UNIT" : "docker.service",
    "CODE_FILE" : "src/core/job.c",
    "CODE_LINE" : "796",
    ...
}
```

**可以直接用 `欄位=值` 當查詢條件**（就是 usage 裡的 `[MATCHES...]`）：

```bash
journalctl _PID=1234                      # 某個 PID 的所有訊息
journalctl _COMM=sshd                     # 某個執行檔名
journalctl _UID=1000                      # 某個使用者
journalctl _SYSTEMD_UNIT=docker.service   # 等同 -u docker
journalctl _TRANSPORT=kernel              # 只看核心來源
journalctl -u docker _PID=1234            # 條件可以疊（AND）
journalctl -u docker + -u containerd      # 用 + 表示 OR
```

**不知道有哪些欄位可用時：**

```bash
journalctl -N                 # 列出所有欄位名
journalctl -F _COMM           # _COMM 這個欄位出現過哪些值
```

**這是 `logread`／`tail` 完全做不到的**——它們只有一行純文字，沒有結構。

---

## 五、`--user` 與 `--system`

```bash
journalctl -u nginx              # 系統層服務
journalctl --user -u myapp       # 使用者層服務（systemctl --user 啟動的）
```

**兩者的 journal 是分開的**。用 `systemctl --user` 跑的服務（例如常駐的個人程式），
一定要加 `--user`，否則 `-u` 找不到。

---

## 六、磁碟空間管理

本機實測：

```bash
$ journalctl --disk-usage
Archived and active journals take up 550.1M in the file system.

$ ls -d /var/log/journal        # 存在 → 持久化，重開機後仍查得到
/var/log/journal
```

**`/var/log/journal` 存不存在，決定 journal 會不會留過重開機**：

| 情況 | 行為 |
|---|---|
| `/var/log/journal` 存在 | 寫磁碟，**重開機後仍查得到**（本機是這種） |
| 不存在 | 只寫 `/run/log/journal`（記憶體），**重開機就沒了** |

由 `/etc/systemd/journald.conf` 的 `Storage=` 控制（預設 `auto`＝目錄在就寫磁碟）。

清理：

```bash
sudo journalctl --vacuum-size=200M      # 只留 200MB
sudo journalctl --vacuum-time=7d        # 只留 7 天
sudo journalctl --vacuum-files=5        # 只留 5 個檔
```

查歷史開機：

```bash
$ journalctl --list-boots
-2 c922706e...  Sat 2026-09-19 08:04 ...
-1 b263162b...  Sun 2026-09-20 08:29 ...
 0 54c126bc...  Mon 2026-09-21 09:31 ...

journalctl -b -1 -u docker      # 上一次開機時 docker 的日誌
```

---

## 七、與 OpenWrt `logread` 的對照

| | systemd（journalctl） | OpenWrt（logread） |
|---|---|---|
| 收集者 | `systemd-journald` | `logd`（procd 生態） |
| 儲存 | **二進位、結構化欄位**的 journal | **純文字、記憶體環狀緩衝區** |
| 預設持久化 | 是（`/var/log/journal` 存在時） | **否**，重開機就沒 |
| 容量 | 數百 MB（本機 550M） | 很小（本機 `log_size=128` KB） |
| 依服務過濾 | `-u <unit>` | `-e <正規式>`（**只能比對文字**） |
| 依時間過濾 | `--since` / `--until` | ❌ 沒有 |
| 依等級過濾 | `-p err` | `-z`／`-Z`（只能依 facility） |
| 跟隨 | `-f` | `-f` |
| 結構化查詢 | ✅ `_PID=`、`-N`、`-F` | ❌ |

**最大的差異是「有沒有結構」**：journal 可以問「docker 這個 unit 在昨天下午出現的 error」，
logread 只能 `grep`。代價是 journal 吃磁碟、格式不透明（不能直接 `cat`）。

---

## 自我測驗

1. 用 `systemctl` 啟動的服務為什麼通常沒有自己的 log 檔？它的 stdout 去哪裡了？
2. `journalctl -u X -f` 和 `journalctl -u X -e` 分別適合什麼情境？
3. journal 存的不是純文字，這帶來哪兩個 `tail -f /var/log/xxx` 做不到的能力？
   舉出兩個具體的查詢例子。
4. 你用 `systemctl --user` 啟動了一個服務，但 `journalctl -u <名稱>` 找不到日誌。為什麼？
5. 情境題：某台機器重開機之後就查不到前一天的日誌了。可能是什麼原因？
   要怎麼確認、怎麼改？
