# 在 OpenWrt 上用 v2ray 的 dokodemo-door 做 TPROXY 透明代理

> 日期：2026-09-21
> 情境：路由器上已有 v2ray（`/root/v2ray/v2ray`），12345 埠 TCP／UDP 都在監聽，但防火牆與策略
> 路由從來沒接上，整個 LAN 還在用「手動指 proxy」的方式；想改成透明代理
> 對應檔案：`/root/v2ray/config_openwrt.json`、`/etc/init.d/v2ray`（路由器上的正式設定）、
> 以及本文要新增的 `/etc/init.d/tproxy-rules`（正式設定）
> 適用範圍：OpenWrt 24.10.2、kernel 6.6.93、nftables 1.1.1／fw4、v2ray 5.40.0
> 原理篇：[路由器上的透明代理-為什麼不是TUN模式.md](./路由器上的透明代理-為什麼不是TUN模式.md)
> 語法篇：[nft完整用法-從命令到規則語法.md](../linux/nft完整用法-從命令到規則語法.md)

**一句話結論：TPROXY 要三個零件同時成立才會動——① v2ray 的 dokodemo-door 開 `tproxy` sockopt
（才收得到「目的地不是自己」的封包）② `ip rule` + `ip route local` 把打了 mark 的封包留在本機
（否則核心會直接把它轉發出去，規則等於沒寫）③ nftables 在 `prerouting` 的 `mangle` 優先權把封包
交付給 12345。缺任何一個都是「規則寫了卻沒作用」。**

---

## 目錄

- [零、動手前的三個確認](#零動手前的三個確認)
- [一、整體資料流](#一整體資料流)
- [二、步驟 1：安裝核心模組](#二步驟-1安裝核心模組)
- [三、步驟 2：v2ray 的 dokodemo-door inbound](#三步驟-2v2ray-的-dokodemo-door-inbound)
- [四、步驟 3：策略路由](#四步驟-3策略路由)
- [五、步驟 4：nftables 規則](#五步驟-4nftables-規則)
- [六、步驟 5：驗證](#六步驟-5驗證)
- [七、DNS 怎麼辦](#七dns-怎麼辦)
- [八、持久化](#八持久化)
- [九、回退與救援](#九回退與救援)
- [十、常見陷阱](#十常見陷阱)
- [自我測驗](#自我測驗)

---

## 零、動手前的三個確認

### 1. 確認 12345 真的是 dokodemo-door

`netstat` 只能看出「TCP 與 UDP 都在聽」，這是 dokodemo-door 的特徵但不是證據。
打開 `/root/v2ray/config_openwrt.json`，找 port 12345 的那個 inbound，確認它長得像這樣：

```json
{
  "tag": "tproxy-in",
  "port": 12345,
  "protocol": "dokodemo-door",
  "settings": { "network": "tcp,udp", "followRedirect": true },
  "streamSettings": { "sockopt": { "tproxy": "tproxy" } }
}
```

**如果 `sockopt.tproxy` 不是 `"tproxy"`（例如是 `"redirect"` 或根本沒有），後面全部白做**——
沒有 `IP_TRANSPARENT` 的 socket 收不到目的地不是自己的封包。

### 2. 確認你有一條「出事能救回來」的路

改的是**路由器的路由表與防火牆**，做錯就是全家斷網，而你很可能正透過它連線。

- 手邊要有實體網路孔或另一條 Wi-Fi；
- 第一階段**只用非持久化指令**（`nft add` / `ip rule add`），**重開機就全部消失**——
  這是你最可靠的救援手段。

### 3. 先只對一台測試機生效

不要一開始就對整個網段開。規則裡加一條 `ip saddr <測試機 IP>`，確認整條路走通再拿掉。

---

## 一、整體資料流

```
LAN 電腦 (192.168.1.50)
   │  想連 1.1.1.1:443，封包目的地就是 1.1.1.1:443
   ▼
br-lan ── 進入路由器
   │
   ▼
[prerouting / priority mangle]         ← 零件③ nftables 在這裡攔
   │  ├─ 目的地是私有網段？ → return（不代理）
   │  └─ 其餘：tproxy to :12345，並 meta mark set 1
   ▼
路由判定 ── 查到 fwmark 1 → 用 table 100 ← 零件② ip rule
   │        table 100 說「所有位址都算本機」 → 不轉發，交給本機
   ▼
本機的 12345 socket ← 零件① IP_TRANSPARENT 才收得到
   │  dokodemo-door 從 IP_RECVORIGDSTADDR 取回「原本要去 1.1.1.1:443」
   ▼
v2ray 路由規則 → outbound → 加密後送往你的伺服器
   │  （這條是路由器自己發起的連線，走 output → postrouting，
   │    不會再經過 prerouting，所以不會迴圈）
   ▼
eth1 → 網際網路
```

**三個零件各自負責一句話：**

| 零件 | 負責 |
|---|---|
| ① `IP_TRANSPARENT`（v2ray 側） | 讓程式**收得到**目的地不是自己的封包 |
| ② `ip rule` + `route local` | 讓打了 mark 的封包**留在本機**，不要被轉發出去 |
| ③ nftables `tproxy` | **把封包交付**給 12345，同時打上 mark |

---

## 二、步驟 1：安裝核心模組

你的路由器目前**沒有** `nft_tproxy`：

```bash
$ ls /lib/modules/6.6.93/ | grep -iE 'tproxy|socket'
（只有 nft_redir.ko）
$ opkg list-installed | grep kmod-nft
kmod-nft-core / kmod-nft-fib / kmod-nft-nat / kmod-nft-offload
```

```bash
opkg update
opkg install kmod-nft-tproxy
```

裝完確認模組存在（載入是用到時才會自動載）：

```bash
ls /lib/modules/$(uname -r)/ | grep -E 'tproxy|socket'
```

> 有些設定會用到 `socket transparent 1` 這個比對（讓已建立連線的後續封包走快路徑），
> 那需要 `nft_socket`。OpenWrt 上通常隨 `kmod-nft-tproxy` 一起提供，可用
> `opkg files kmod-nft-tproxy` 確認。**本文的最小可用版本不需要它**。

**為什麼一定要這個模組**：`tproxy` 是 nftables 的一個 statement（動作），它的實作在核心模組裡。
沒有模組，`nft` 下規則時會直接報錯（`Operation not supported`）——這是好事，總比安靜失敗好。

---

## 三、步驟 2：v2ray 的 dokodemo-door inbound

### 設定長什麼樣

```json
{
  "tag": "tproxy-in",
  "port": 12345,
  "protocol": "dokodemo-door",
  "settings": {
    "network": "tcp,udp",
    "followRedirect": true
  },
  "sniffing": {
    "enabled": true,
    "destOverride": ["http", "tls", "quic"]
  },
  "streamSettings": {
    "sockopt": { "tproxy": "tproxy" }
  }
}
```

### 先澄清：「dokodemo-door」不是協定

它**根本不是網路協定**，而是 v2ray 內部的 inbound handler 名稱。名字取自哆啦A夢的道具
**「どこでもドア」（Dokodemo Door，中文譯「任意門」）**——任意門通到你想去的任何地方，
正好描述它的行為：**把進來的連線原封不動送到「它原本要去的地方」**。

**v2ray 設定裡的 `protocol` 欄位混了兩類東西**，這是最容易誤會的地方：

| 類別 | 值 | 有線路格式／交握嗎 |
|---|---|---|
| **真的協定** | `socks`、`http`、`vmess`、`vless`、`trojan`、`shadowsocks`、`hysteria2`、`wireguard` | ✅ |
| **內部 handler** | **`dokodemo-door`**、`freedom`、`blackhole`、`loopback`、`dns` | ❌ 沒有線路協定 |

dokodemo-door 不做任何協定解析——它收到一條**裸的 TCP／UDP 連線**，只需要決定「這要送去哪」：

| `followRedirect` | 目的地哪裡來 | 用途 |
|---|---|---|
| `false` | 設定檔裡寫死的 `address`／`port` | 單純的埠轉發 |
| **`true`** | **去問核心**原本的目的地 | **透明代理**（本文要的） |

「去問核心」的實作方式，也正好對應兩種透明代理機制：REDIRECT 用 `SO_ORIGINAL_DST`，
TPROXY 用 `IP_RECVORIGDSTADDR`——所以 `followRedirect` 這個名字其實有點誤導，它兩種都管。

**同樣的功能在別的軟體叫什麼**：

| 軟體 | 名稱 |
|---|---|
| Xray | 一樣叫 `dokodemo-door`（它是 v2ray 的 fork） |
| sing-box | `tproxy` / `redirect` inbound |
| mihomo（clash） | `tproxy-port` / `redir-port` |

**功能是業界標準的，名字是 v2ray 的玩笑。** 同一批命名還有 `freedom`（直連出站）、
`blackhole`（丟棄）——查文件時要有心理準備：**在 `protocol` 欄位看到不認識的字，先確認它是
真協定還是這類內部名稱**，否則會白花時間去找「dokodemo-door 協定的 RFC」。

### 每個欄位為什麼要這樣寫

| 欄位 | 作用 |
|---|---|
| `protocol: dokodemo-door` | 「任意門」——**不解析應用層協定**，只負責把收到的連線原封不動轉給 outbound。透明代理的標準 inbound |
| `network: "tcp,udp"` | **兩個都要**。只寫 tcp 的話 DNS 查詢、QUIC、遊戲全部漏掉 |
| `followRedirect: true` | **關鍵**：告訴 dokodemo「目的地不要看我 `address` 欄位，去問核心原本要去哪」。沒有它，所有連線都會被送到同一個固定位址 |
| `sockopt.tproxy: "tproxy"` | 對 listen socket 設 `IP_TRANSPARENT`，**這是零件①** |
| `sniffing.destOverride` | 從 TLS 的 SNI／HTTP 的 Host 還原出**域名** |

### `sniffing` 為什麼重要（這決定你要不要改 DNS）

TPROXY 攔到的封包裡**只有 IP 沒有域名**，而你的分流規則多半是寫域名的
（`domain:google.com` 之類）。兩種解法：

- **開 sniffing**：v2ray 從第一個封包的 TLS SNI 或 HTTP Host 把域名挖出來 → **域名規則直接可用，
  不必動 DNS**。
- 改 DNS（見第七節）：工程較大。

**建議先開 sniffing 把路走通，DNS 之後再處理。**

### v2ray 必須有權限

`IP_TRANSPARENT` 需要 `CAP_NET_ADMIN`。你的 v2ray 是 procd 以 **root** 啟動的，所以沒問題。

### 別忘了路由規則

在 `routing.rules` 裡要有一條把 `inboundTag: ["tproxy-in"]` 的流量送到你的代理 outbound
（或依既有規則分流）。**改完設定一定要先測語法再重啟**：

```bash
/root/v2ray/v2ray test -config /root/v2ray/config_openwrt.json   # 你的 init script 也會做這件事
/etc/init.d/v2ray restart
netstat -lntup | grep 12345          # 確認還在聽
```

---

## 四、步驟 3：策略路由

```bash
ip rule add fwmark 1 lookup 100
ip route add local 0.0.0.0/0 dev lo table 100
```

### 這兩行到底在做什麼

**這是整套裡最反直覺、也最常被漏掉的一步。**

封包經過 `prerouting` 之後，核心要做路由判定：「這封包是給我的，還是要轉給別人？」
目的地是 `1.1.1.1`，核心當然判定「不是給我的」→ **直接轉發出去**，
你的 12345 socket 永遠等不到它。

所以要騙核心說「這個封包是給我的」：

- `ip route add local 0.0.0.0/0 dev lo table 100`
  ——在編號 100 的路由表裡宣告「**所有位址都是本機位址**」（`local` 是路由型別，
  不是關鍵字裝飾）。
- `ip rule add fwmark 1 lookup 100`
  ——「**帶有 mark 1 的封包，改用 table 100 查路由**」。

兩者合起來：打了 mark 的封包查 table 100 → 得到「這是本機位址」→ 核心把它交給本機處理 →
`IP_TRANSPARENT` 的 socket 收得到。

**沒有這兩行，nftables 的 tproxy 規則會安靜地沒有效果**——這是「規則寫了卻沒作用」的頭號原因。

> `100` 和 `1` 都是自己選的編號，沒有特殊意義，只要前後一致、不跟現有的衝突即可
> （`ip rule show`、`ip route show table all` 可以確認沒撞）。

---

## 五、步驟 4：nftables 規則

### 測試版（非持久化，重開機消失）

```bash
nft add table ip v2ray_tproxy

nft add set ip v2ray_tproxy bypass '{
    type ipv4_addr;
    flags interval;
    elements = { 0.0.0.0/8, 10.0.0.0/8, 127.0.0.0/8, 169.254.0.0/16,
                 172.16.0.0/12, 192.168.0.0/16, 224.0.0.0/4, 240.0.0.0/4 }
}'

nft add chain ip v2ray_tproxy prerouting '{
    type filter hook prerouting priority mangle; policy accept;
}'

# 第一階段：只對一台測試機生效（確認可行後再拿掉 ip saddr 那段）
nft add rule ip v2ray_tproxy prerouting ip daddr @bypass counter return
nft add rule ip v2ray_tproxy prerouting iifname "br-lan" ip saddr 192.168.1.50 \
    meta l4proto { tcp, udp } counter tproxy to :12345 meta mark set 1 accept
```

### 逐行解釋

| 片段 | 意思 |
|---|---|
| `table ip v2ray_tproxy` | 自己建一張獨立的表。**故意不放進 fw4 的表**，好處是出事時 `nft delete table ip v2ray_tproxy` 一行清乾淨，而且 `fw4 restart` 不會動到它 |
| `type filter hook prerouting priority mangle` | 掛在 prerouting、用 mangle 優先權（-150）——**必須早於路由判定** |
| `set bypass` + `flags interval` | 具名集合，`interval` 才能放網段。用集合而不是一堆規則，比對是查表、快得多 |
| `ip daddr @bypass counter return` | **目的地是私有網段就不代理**（內網互連、路由器管理頁、多播）。`return` 離開這條鏈，繼續原本的路 |
| `iifname "br-lan"` | 只處理從 LAN 進來的。不加的話從 WAN 進來的封包也會被攔 |
| `meta l4proto { tcp, udp }` | TCP 與 UDP 都代理 |
| `counter` | **一定要加**，這是後面驗證「規則有沒有被命中」的唯一依據 |
| `tproxy to :12345` | 把封包**交付**給本機 12345，**不改寫封包內容**（這是與 REDIRECT 的根本差別） |
| `meta mark set 1` | 打上 mark，配合步驟 3 的 `ip rule` |
| `accept` | 結束這條 base chain |

### 排除清單為什麼是這幾個網段

| 網段 | 為什麼要排除 |
|---|---|
| `10/8`、`172.16/12`、`192.168/16` | 內網互連、存取路由器本身，走代理沒有意義還會壞掉 |
| `127/8` | 迴路 |
| `169.254/16` | link-local |
| `224/4`、`240/4` | 多播與保留位址（mDNS、DHCP 探索等） |
| `0.0.0.0/8` | 未指定位址 |

**還要記得把你的代理伺服器位址也排除**（如果它在這些網段之外）——否則 v2ray 連往伺服器的封包
若剛好也經過 prerouting，就會被自己攔回來。

> **但本設定不會迴圈**，原因值得理解：v2ray 連往上游伺服器是**路由器自己發起**的連線，
> 走的是 `output → postrouting`，**根本不經過 `prerouting`**。只有當你之後想連「路由器自己的
> 流量」也代理、而去加 output 鏈的規則時，才必須用 `SO_MARK` 把 v2ray 自己的流量排除
> （outbound 的 `streamSettings.sockopt.mark`）。
> 網路上很多教學照抄 mark 卻沒解釋，就是因為它們同時做了 output 那一半。

---

## 六、步驟 5：驗證

**照順序查，每一步都確認再往下**：

```bash
# ① 模組在不在
lsmod | grep -E 'tproxy|socket'

# ② 策略路由有沒有生效
ip rule show | grep 'fwmark 0x1'
ip route show table 100                # 應該看到 local default dev lo

# ③ 規則有沒有被命中 ← 最關鍵
nft list table ip v2ray_tproxy         # 看 counter 會不會增加

# ④ v2ray 有沒有收到連線
logread -f | grep -i v2ray             # 或看 v2ray 的 access log
netstat -ntp | grep 12345

# ⑤ 端到端：在測試機上
curl -s https://ifconfig.me            # 出口 IP 變了沒
```

**判讀方式：**

| 現象 | 指向 |
|---|---|
| ③ 的 counter 一直是 0 | 封包沒走到規則——檢查 `iifname`、`ip saddr` 是否寫對，測試機是否真的從 br-lan 進來 |
| ③ 有計數但 ④ 沒連線 | **零件②沒生效**——`ip rule`／`route local` 沒設好，封包被轉發走了 |
| ④ 有連線但連不通 | 問題在 v2ray 的 routing／outbound，與 TPROXY 無關 |
| 測試機完全斷網 | 立刻執行第九節的回退 |

這個「逐段切開、用 counter 定位斷點」的方法，就是
[排查方法論.md](../linux/排查方法論.md) 的對照組原則。

---

## 七、DNS 怎麼辦

**開了 sniffing 之後，DNS 其實可以先不動**——域名從 TLS SNI／HTTP Host 就還原得到了。

但有兩種情況仍需要處理 DNS：

1. **DNS 被污染**：解析結果本身就是錯的 IP，sniffing 救不了（連都連不到對的地方）。
2. **不走 TLS／HTTP 的流量**：沒有 SNI 也沒有 Host 可嗅探。

兩種常見做法：

- **讓 dnsmasq 把上游指向 v2ray 的 DNS inbound**：改 `/etc/config/dhcp` 的
  `option server '127.0.0.1#<v2ray dns 埠>'`。乾淨、與 uci 整合。
- **把 53 埠也 TPROXY 進去**：規則裡已經含 UDP，所以 LAN 送往外部 DNS 的查詢本來就會被攔；
  但送往路由器自己（dnsmasq）的查詢不會——因為目的地是私有網段、被 bypass 掉了。

**建議順序**：先把 TCP／UDP 的代理跑通，再單獨處理 DNS。一次只動一個變數。

---

## 八、持久化

確認測試版可行、也拿掉了 `ip saddr` 限制之後，才做持久化。

### nftables 規則

**注意**：`/etc/nftables.d/*.nft` 的內容是被塞進 `table inet fw4 { ... }` **裡面**的，
所以**不能**把 `table ip v2ray_tproxy { ... }` 放進去（會變成巢狀 table，語法錯誤）。
兩個選擇：

| 做法 | 優點 | 代價 |
|---|---|---|
| **A. 獨立 table + 自己的 init script** | 與 fw4 完全解耦，`fw4 restart` 不影響；一行可整表刪除 | 要自己寫載入腳本 |
| **B. 寫成 chain 放進 `/etc/nftables.d/`** | fw4 自動載入、開機就有 | 要用 `inet` family 語法（`tproxy ip to :12345`），且每次 fw4 reload 會重建 |

**建議 A**，因為策略路由（`ip rule`）本來就得自己寫腳本管——乾脆放一起。

### 一份 init script 管兩件事

`/etc/init.d/tproxy-rules`（記得 `chmod +x` 並 `/etc/init.d/tproxy-rules enable`）：

```sh
#!/bin/sh /etc/rc.common
START=95          # 要晚於 firewall(19) 與 v2ray(99)？見下方說明
STOP=10

RULES=/etc/v2ray-tproxy.nft

start() {
    nft -f "$RULES"
    ip rule add fwmark 1 lookup 100 2>/dev/null
    ip route add local 0.0.0.0/0 dev lo table 100 2>/dev/null
}

stop() {
    nft delete table ip v2ray_tproxy 2>/dev/null
    ip rule del fwmark 1 lookup 100 2>/dev/null
    ip route flush table 100 2>/dev/null
}
```

把規則寫成 `/etc/v2ray-tproxy.nft`（用 `nft -f` 原子套用，語法見
[nft完整用法-從命令到規則語法.md](../linux/nft完整用法-從命令到規則語法.md)）：

```
table ip v2ray_tproxy {
    set bypass {
        type ipv4_addr
        flags interval
        elements = { 0.0.0.0/8, 10.0.0.0/8, 127.0.0.0/8, 169.254.0.0/16,
                     172.16.0.0/12, 192.168.0.0/16, 224.0.0.0/4, 240.0.0.0/4 }
    }
    chain prerouting {
        type filter hook prerouting priority mangle; policy accept;
        ip daddr @bypass counter return
        iifname "br-lan" meta l4proto { tcp, udp } counter tproxy to :12345 meta mark set 1 accept
    }
}
```

> **START 的取捨**：v2ray 的 `START=99`，所以 tproxy 規則若在它之前套用，會有一小段時間
> 「封包被交付給一個還沒開的 socket」——那些連線會失敗但不會壞掉。設成 `START=99` 之後
> （字母序在 v2ray 之後）比較保險。這也是為什麼**先手動測、確定順序沒問題再持久化**。

**持久化之後一定要重開機驗一次**——「開機自動生效」是它唯一的目的，沒驗過就不算完成。

---

## 九、回退與救援

**測試階段（非持久化）**：

```bash
nft delete table ip v2ray_tproxy
ip rule del fwmark 1 lookup 100
ip route flush table 100
```

或直接 `reboot`——非持久化的東西全部消失。**這就是為什麼第一階段不要持久化。**

**已持久化之後**：

```bash
/etc/init.d/tproxy-rules stop
/etc/init.d/tproxy-rules disable      # 開機不再套用
```

**完全連不上路由器時**：OpenWrt 的 failsafe 模式（開機時按住 reset／按鍵，
從 `192.168.1.1` 以唯讀模式進入）可以救回來。**動手前先確認你知道你的機型怎麼進 failsafe。**

---

## 十、常見陷阱

| 陷阱 | 症狀 | 原因 |
|---|---|---|
| 漏了 `ip route add local` | counter 有跳但 v2ray 收不到 | 核心判定「不是給我的」直接轉發走 |
| `sockopt.tproxy` 沒設 | v2ray 啟動正常但連線進不來 | socket 沒有 `IP_TRANSPARENT`，收不到非本機目的的封包 |
| `followRedirect` 沒開 | 所有連線都跑到同一個位址 | dokodemo 用了設定檔裡的固定 `address` |
| 只寫 `network: "tcp"` | 網頁能開但 DNS／QUIC／遊戲不通 | UDP 沒被代理 |
| 沒排除私有網段 | 內網互連、SSH 進路由器變慢或斷 | 內網流量被送進代理繞一圈 |
| 沒加 `iifname "br-lan"` | 奇怪的封包也被攔 | 從 WAN 進來的流量一併被 tproxy |
| 規則加在 `filter` 優先權 | 完全沒作用 | 路由判定已經做完了，太晚 |
| 直接持久化沒先測 | 重開機後斷網 | 見第九節 |

---

## 自我測驗

1. TPROXY 的三個零件各自解決什麼問題？如果只做了 nftables 規則、漏掉 `ip rule` 與
   `ip route add local`，封包會走到哪裡、為什麼 v2ray 完全收不到？
2. `followRedirect: true` 沒開會發生什麼事？從「dokodemo-door 怎麼知道封包原本要去哪」回答。
3. 為什麼本文的設定不會造成「v2ray 連往上游伺服器的封包被自己攔回來」的迴圈？
   什麼情況下你才**必須**開始用 `SO_MARK` 排除自己的流量？
4. 開了 `sniffing` 之後為什麼可以先不動 DNS？哪兩種情況 sniffing 救不了、非得處理 DNS 不可？
5. 情境題：你照著做完，`nft list table ip v2ray_tproxy` 的 counter 一直在增加，但測試機完全連不上
   網路、v2ray 的 log 也沒有任何新連線。請說出最可能的斷點在哪一個零件，以及你會用哪兩個指令確認。
