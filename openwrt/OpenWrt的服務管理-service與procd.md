# OpenWrt 的服務管理 —— `service`、`/etc/init.d`、procd 與 systemd 的對照

> 日期：2026-09-21
> 情境：習慣 `systemctl start/stop/status` 之後，想知道 OpenWrt 上對應的指令是什麼
> 對應檔案：`/sbin/service`、`/etc/init.d/*`、`/etc/rc.d/*`（皆為路由器上的正式檔案）
> 適用範圍：OpenWrt 24.10.2、procd、BusyBox 1.36.1
> 相關：[nftables與uci是什麼-兩層設定的分工.md](./nftables與uci是什麼-兩層設定的分工.md)、
> [用dokodemo-door做TPROXY透明代理.md](./用dokodemo-door做TPROXY透明代理.md)

**一句話結論：OpenWrt 沒有 systemd，用的是 **procd**。對應指令是
`service <名稱> <動作>`（或直接 `/etc/init.d/<名稱> <動作>`），動作名稱幾乎和 systemctl 一樣。
最接近 `systemctl list-units` 的是**不給任何參數的 `service`**——它會列出所有服務與
enabled／running 狀態。想看 systemd 那種詳細資訊，得用 procd 原生的 `ubus`。**

---

## 一、最直接的答案

```sh
service <名稱> start|stop|restart|reload|enable|disable|status ...
```

實測：

```
# service v2ray status
running

# service v2ray running && echo 是
是
```

**而不給任何參數時，它會列出全部服務**——這就是你在找的「類似 systemctl 的總覽」：

```
# service
Usage: service <service> [command]
/etc/init.d/boot                 enabled    stopped
/etc/init.d/cron                 enabled    stopped
/etc/init.d/dnsmasq              enabled    running
/etc/init.d/dropbear             enabled    running
/etc/init.d/firewall             enabled    stopped
/etc/init.d/network              enabled    running
...
```

**兩欄分別是「開機會不會自動啟動」與「現在跑不跑」**，語意等同
`systemctl is-enabled` 與 `is-active` 合併成一張表。

---

## 二、`service` 其實只是一個 wrapper

看它的原始碼就懂了（`/sbin/service`，全檔 716 bytes）：

```sh
main() {
    local service="$1"
    shift
    if [ -f "/etc/init.d/${service}" ]; then
        /etc/init.d/"${service}" "$@"      # ← 就是轉呼叫
        exit "$?"
    fi
    ...
    # 沒給參數 → 走訪 /etc/init.d/* 印出狀態
}
```

**所以 `service v2ray restart` 和 `/etc/init.d/v2ray restart` 完全等價**，
沒有任何額外邏輯。知道這件事就不會糾結該用哪個——`service` 只是打起來順手，
而且多了「不給參數列出全部」這個功能。

---

## 三、完整動作清單

這些動作由 `/etc/rc.common` 提供，**所有 init script 都有**（即使腳本本身只實作了 start/stop）：

| 動作 | 作用 |
|---|---|
| `start` / `stop` / `restart` | 啟動／停止／重啟 |
| `reload` | **重新載入設定**；服務沒實作就退回 `restart` |
| `enable` / `disable` | **開機自動啟動的開關** |
| `enabled` | **查詢**是否設為開機啟動（用離開碼回答） |
| `running` | **查詢**是否正在執行（用離開碼回答） |
| `status` | 印出 `running` 或 `inactive` |
| `trace` | **用 syscall trace 啟動**（服務起不來時很有用） |
| `info` | 倒出 procd 的服務資訊（JSON） |

> **`enable` 與 `enabled` 只差一個字母，意思完全相反**：前者是「設定成開機啟動」（會改東西），
> 後者是「查詢是不是開機啟動」（唯讀）。寫腳本時很容易打錯。

`enabled` 與 `running` 是**用離開碼回答**的，所以適合寫在條件式裡：

```sh
service v2ray running || service v2ray start
```

---

## 四、`enable` 到底做了什麼：`/etc/rc.d` 的符號連結

OpenWrt 沒有 systemd 的 `wants/` 目錄，用的是傳統 SysV 風格的編號符號連結：

```
# ls -l /etc/rc.d/ | grep v2ray
K15v2ray -> ../init.d/v2ray
S99v2ray -> ../init.d/v2ray
```

| 前綴 | 意思 | 數字從哪來 |
|---|---|---|
| `S<NN>` | **開機時啟動**，數字小的先 | init script 裡的 `START=` |
| `K<NN>` | **關機時停止**，數字小的先 | init script 裡的 `STOP=` |

實測對照：

```
/etc/init.d/v2ray:START=99      → S99v2ray
/etc/init.d/v2ray:STOP=15       → K15v2ray
/etc/init.d/firewall:START=19   → 防火牆比 v2ray 早很多啟動
```

**所以 `enable`／`disable` 就是建立／刪除這兩個連結**，不神秘。
而**啟動順序完全由 `START=` 的數字決定**——這在寫依賴其他服務的 init script 時很關鍵
（例如 TPROXY 規則必須晚於 v2ray，就要把 `START` 設得比 99 大或相等）。

---

## 五、看日誌：`logread`（journalctl 的對應）

OpenWrt 的日誌在**記憶體的環狀緩衝區**（不是檔案），用 `logread` 讀：

```sh
logread                      # 全部
logread -f                   # 持續跟隨（等同 journalctl -f）
logread -e v2ray             # 用正規式過濾（等同 journalctl -u）
logread -l 50                # 只看最後 50 筆
```

**完整選項**（依 `logread` 自己的 usage）：

| 選項 | 作用 |
|---|---|
| `-l <筆數>` | 只取最後 N 筆 |
| `-e <正規式>` | **過濾訊息** |
| `-f` | 持續跟隨 |
| `-s <路徑>` | ubus socket 路徑 |
| `-r <伺服器> <埠>` | **把日誌串流到遠端** |
| `-F <檔案>` | 寫入檔案 |
| `-S <位元組>` | 日誌大小上限 |
| `-p <檔案>` | PID 檔 |
| `-h <主機名>` | 訊息附上主機名 |
| `-P <前綴>` | 串流時加前綴 |
| `-z <facility>` / `-Z <facility>` | 只處理／忽略指定 facility（0–23，可重複） |

### 服務的輸出怎麼進到 syslog

**不是自動的**——init script 要明確把 stdout／stderr 交給 procd。你的 v2ray 就有設：

```sh
# /etc/init.d/v2ray
procd_set_param stdout 1
procd_set_param stderr 1
```

**沒設這兩行的服務，它印的東西就不會出現在 `logread` 裡**（會直接被丟掉）。
這等同 systemd unit 的 `StandardOutput=journal`，差別是 systemd **預設就開**，procd 要自己寫。

實測效果：

```
# logread -e v2ray | tail -3
Mon Sep 21 13:32:19 2026 daemon.info v2ray[25283]: [Info] [2270844649] proxy/socks: TCP Connect request to tcp:mtalk.google.com:5228
Mon Sep 21 13:32:19 2026 daemon.info v2ray[25283]: [Info] app/dispatcher: taking detour [vmess-out-s02] for [tcp:mtalk.google.com:5228]
Mon Sep 21 13:32:19 2026 daemon.info v2ray[25283]: tcp:192.168.1.171:58496 accepted tcp:mtalk.google.com:5228 [vmess-out-s02]
```

### 實務配方

```sh
logread -e v2ray | tail -30       # 最近 30 筆
logread -f -e v2ray               # ★ 即時盯著（改設定後重啟服務，馬上看反應）
logread -e 'v2ray.*error'         # 只看錯誤（-e 是正規式）
logread | wc -l                   # 目前緩衝區有幾筆
uci show system | grep log_size   # 緩衝區大小（本機 128 KB）
```

**排查流程建議**：開一個終端機跑 `logread -f -e v2ray`，另一個終端機做操作，
這樣因果關係一目了然。

> **對照 systemd**：`logread` 只有純文字，**沒有 `--since`、沒有 `-u`、沒有等級過濾**。
> 差異見 [journalctl看服務日誌.md](../linux/journalctl看服務日誌.md) 的對照表。

> **重開機日誌就沒了**（在 RAM 裡）。要保留得用 `-F` 寫檔或 `-r` 送到遠端 syslog——
> 但寫進 flash 會耗損壽命，一般只在除錯時暫時開。

---

## 六、想看 systemd 那種詳細狀態：`ubus`

`service <名稱> status` **只會印 `running` 或 `inactive`**，沒有 systemd 那種
「主行程 PID、記憶體、最近幾行日誌」的摘要。要更多資訊得問 procd 本人：

```sh
# ubus call service list '{"name":"v2ray"}'
{
    "v2ray": {
        "instances": {
            "instance1": {
                "running": true,
                "pid": 4914,
                "command": [ "/root/v2ray/v2ray", "run", "-config", "/root/v2ray/config_openwrt.json" ],
                "term_timeout": 5,
                "respawn": { "threshold": 3600, "timeout": 5, "retry": 5 }
            }
        }
    }
}
```

**這比 `status` 有用得多**——PID、完整命令列、重啟策略全都在。

`ubus` 是 OpenWrt 的行程間通訊匯流排，服務管理只是它的一小部分：

```sh
ubus list                            # 有哪些物件可以問
ubus call service list               # 全部服務
ubus call service list '{"name":"X"}'  # 單一服務
ubus -v list service                 # service 物件支援哪些方法與參數
```

---

## 七、systemd ↔ OpenWrt 對照表

| systemd | OpenWrt |
|---|---|
| `systemctl start X` | `service X start` |
| `systemctl stop X` | `service X stop` |
| `systemctl restart X` | `service X restart` |
| `systemctl reload X` | `service X reload` |
| `systemctl enable X` | `service X enable` |
| `systemctl disable X` | `service X disable` |
| `systemctl is-enabled X` | `service X enabled` |
| `systemctl is-active X` | `service X running` |
| `systemctl status X` | `service X status`（**資訊少很多**）＋ `ubus call service list` |
| `systemctl list-units` | **`service`（不給參數）** |
| `systemctl cat X` | `cat /etc/init.d/X` |
| `systemctl daemon-reload` | **不需要**（init script 是 shell，每次執行才讀） |
| `journalctl -u X` | `logread -e X` |
| `journalctl -f` | `logread -f` |
| unit 檔 `/etc/systemd/system/` | init script `/etc/init.d/` |
| `Wants`／`After` 依賴 | **`START=`／`STOP=` 的數字順序** |
| `/etc/systemd/system/*.wants/` | `/etc/rc.d/S<NN>`、`K<NN>` 符號連結 |

**最大的觀念差異：systemd 是「宣告依賴關係」，procd 是「排數字順序」。**
沒有 `After=network-online.target` 這種東西，只有「我的 START 比它大」。

---

## 八、init script 的骨架

OpenWrt 的 init script 是 shell，開頭固定引入 `/etc/rc.common`：

```sh
#!/bin/sh /etc/rc.common

START=99          # 開機啟動順序
STOP=15           # 關機停止順序
USE_PROCD=1       # 使用 procd 管理（現代寫法）

start_service() {
    procd_open_instance
    procd_set_param command /path/to/binary --flag
    procd_set_param respawn          # 掛掉自動重啟
    procd_set_param stdout 1         # stdout 導進 syslog
    procd_set_param stderr 1
    procd_close_instance
}

# 可選
stop_service()   { ... }
reload_service() { ... }
service_triggers() { ... }           # 監看 uci 設定變化自動 reload
```

**`USE_PROCD=1` 的意義**：由 procd 負責監看行程、自動重啟、收集輸出——
等同 systemd unit 的 `Restart=`、`StandardOutput=journal`。
你的 `/etc/init.d/v2ray` 就是這個形式。

---

## 自我測驗

1. `service X restart` 和 `/etc/init.d/X restart` 有什麼差別？從 `/sbin/service` 的實作回答。
2. 要怎麼在 OpenWrt 上得到「所有服務＋是否開機啟動＋是否執行中」的總覽？
   這對應到 systemd 的哪個指令？
3. `service X enable` 實際上改了什麼？`START=99` 這個數字決定了什麼？
   systemd 用什麼機制取代它？
4. `service X status` 印出的資訊比 `systemctl status X` 少很多。要拿到 PID、完整命令列與
   重啟策略，該問誰、用什麼指令？
5. 情境題：你寫了一個 init script 來套用 nftables 規則，它必須在 v2ray（`START=99`）之後才執行。
   你會怎麼設定？另外，為什麼 OpenWrt 沒有 `systemctl daemon-reload` 這個步驟？
