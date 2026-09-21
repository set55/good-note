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
- [二、實際檢查一份設定檔的結果（本機案例）](#二實際檢查一份設定檔的結果本機案例)
- [三、步驟 1：安裝核心模組](#三步驟-1安裝核心模組)
- [四、步驟 2：v2ray 的 dokodemo-door inbound](#四步驟-2v2ray-的-dokodemo-door-inbound)
- [五、步驟 3：策略路由](#五步驟-3策略路由)
- [六、步驟 4：nftables 規則](#六步驟-4nftables-規則)
- [七、步驟 5：驗證](#七步驟-5驗證)
- [八、DNS 怎麼辦](#八dns-怎麼辦)
- [九、持久化](#九持久化)
- [十、回退與救援](#十回退與救援)
- [十一、常見陷阱](#十一常見陷阱)
- [十二、opkg 完整命令與選項（安裝核心模組會用到）](#十二opkg-完整命令與選項安裝核心模組會用到)
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

## 二、實際檢查一份設定檔的結果（本機案例）

把第零節的檢查實際跑過一次，結果值得記錄——**它剛好示範了「看起來有做、其實沒做」**。

該設定的 12345 inbound 長這樣（省略無關欄位）：

```json
{
  "port": 12345,
  "protocol": "dokodemo-door",
  "tag": "tproxy-in",
  "settings": { "network": "tcp,udp", "followRedirect": true },
  "sniffing": { "enabled": false }
}
```

### 發現一：**沒有 `streamSettings.sockopt.tproxy`**

整個 `streamSettings` 區塊根本不存在 → socket **沒有 `IP_TRANSPARENT`** →
收不到「目的地不是自己」的封包 → **TPROXY 規則寫了也是白寫**。

### 但這個設定並非沒用——它是為 REDIRECT 準備的

關鍵在於**兩種透明代理對 inbound 的要求不同**：

| 機制 | inbound 需要什麼 | 怎麼取回原始目的地 |
|---|---|---|
| **REDIRECT** | **只要 `followRedirect: true`** | `SO_ORIGINAL_DST`（conntrack 記著） |
| **TPROXY** | `followRedirect: true` **＋ `sockopt.tproxy: "tproxy"`** | `IP_RECVORIGDSTADDR`（封包沒被改寫） |

**所以「有 `followRedirect` 但沒有 `sockopt.tproxy`」＝ 這份設定是 REDIRECT-ready，不是 TPROXY-ready。**
這和該機器「有 `nft_redir.ko`、沒有 `nft_tproxy.ko`」的狀態完全吻合——
看得出當初是照 REDIRECT 的路線準備的，只是防火牆那一半從沒接上。

**由此得到一個可以帶走的判斷法：看到別人的 dokodemo-door 設定，
先看有沒有 `sockopt.tproxy` 就知道對方打算用哪一種機制。**

### 發現二：`sniffing.enabled: false`

分流靠 `inboundTag` 時不影響「走不走代理」，但會影響**域名**：
TPROXY 攔到的封包只有 IP，關掉 sniffing 就**永遠拿不回域名**。
後果是 DNS 一旦被污染，v2ray 只會忠實地把你送到那個錯的 IP。

### 發現三：沒有 `freedom` outbound

整份設定只有兩個 vmess outbound、沒有直連出口。代表**v2ray 沒有能力把任何流量送直連**——
私有網段的排除只能靠防火牆那一層。防火牆規則若有疏漏，內網流量會被送進隧道繞一圈。

---

## 三、步驟 1：安裝核心模組

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

## 四、步驟 2：v2ray 的 dokodemo-door inbound

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

## 五、步驟 3：策略路由

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

## 六、步驟 4：nftables 規則

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

### 加完之後怎麼查、怎麼刪

**查（由粗到細）**

```sh
nft list tables                              # 有哪些表 → 確認 v2ray_tproxy 建起來了
nft list table ip v2ray_tproxy               # ★ 只看這張表：set、chain、rule 一次全出來
nft list chain ip v2ray_tproxy prerouting    # 只看這條鏈
nft list set   ip v2ray_tproxy bypass        # 只看這個集合的成員
nft -a list table ip v2ray_tproxy            # 帶 handle ← 要刪單條規則時必須先跑這個
nft -s list table ip v2ray_tproxy            # 不印 counter，適合存檔備份
```

**只看自己的表是關鍵**——`nft list ruleset` 會連 fw4 那 180 行一起倒出來，
你的規則會淹沒在裡面。**這也是當初刻意建獨立表、不寫進 `inet fw4` 的好處之一。**

**刪（由細到粗）**

```sh
# ① 刪單條規則：必須用 handle
nft -a list chain ip v2ray_tproxy prerouting      # 先查
#   ... counter packets 0 bytes 0 tproxy to :12345 # handle 4
nft delete rule ip v2ray_tproxy prerouting handle 4

# ② 清空鏈裡所有規則（鏈留著）
nft flush chain ip v2ray_tproxy prerouting

# ③ 刪集合的單一成員 / 整個集合
nft delete element ip v2ray_tproxy bypass '{ 10.0.0.0/8 }'
nft delete set     ip v2ray_tproxy bypass

# ④ ★ 一次清光整張表（最實用）
nft delete table ip v2ray_tproxy
```

**建議用 ④。** 刪表會連同裡面的 chain、set、rule 一起消失，
**不必煩惱「set 還被規則引用著能不能刪」「chain 裡還有規則能不能刪」這類相依順序問題**。
測試階段規則本來就是反覆重建的，整表砍掉重來最乾淨：

```sh
nft delete table ip v2ray_tproxy 2>/dev/null    # 先清（不存在也不報錯）
nft -f /etc/v2ray-tproxy.nft                    # 再整份套用
```

**刪之前先備份**：

```sh
nft -s list table ip v2ray_tproxy > /tmp/tproxy-backup.nft
```

> **`ip rule` 與 `ip route` 要另外清**，刪 nft 表不會動到它們：
> ```sh
> ip rule del pref 100
> ip route flush table 100
> ```

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

## 七、步驟 5：驗證

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

## 八、DNS 怎麼辦

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

## 九、持久化：三樣東西、三種做法

**重開機之後，`nft` 規則、`ip rule`、`ip route` 全部都會消失**——它們都只是核心的執行期狀態，
不是設定檔。會「自己回來」的東西，都是因為有某個開機服務重新套用了一次。

本機實測開機順序：

```
S19firewall   → fw4 從 /etc/config/firewall 重新產生整套 nft 規則
S20network    → netifd 從 /etc/config/network 重新套用位址、路由、規則
```

**所以 fw4 的規則會回來、你手動 `nft add` 的不會；
介面自帶的路由會回來、你手動 `ip route add` 的不會。**

| 要持久化的東西 | 做法 | 為什麼 |
|---|---|---|
| **`ip rule`** | **`/etc/config/network` 的 `config rule`** | **OpenWrt 原生支援**，見下 |
| **`ip route`**（自訂表） | **`config route` 的 `option table`** | 同上 |
| **nft 規則** | `/etc/nftables.d/*.nft` 或自己的 init script | fw4 每次產生 ruleset 時會 include |

### `ip rule` 可以用 uci 持久化（不必寫腳本）

這是 OpenWrt 的原生功能，很多教學沒提。netifd 的二進位裡就有這些欄位的解析錯誤訊息，
證明它支援：

```
[INTERFACE] Failed to parse rule fwmark: %s
[INTERFACE] Failed to parse rule lookup table: %s
[INTERFACE] Failed to parse rule source / destination: %s
```

寫法：

```
# /etc/config/network
config rule
    option mark     '1'
    option lookup   '100'
    option priority '100'
```

`config rule` 支援的屬性（從 netifd 二進位裡抽出來的完整清單）：

| 屬性 | 對應 `ip rule` 的 |
|---|---|
| `src` / `dest` | `from` / `to` |
| `mark` | `fwmark` |
| `lookup` | `table` |
| `priority` | `pref` |
| `goto` | `goto` |
| `action` | `blackhole`／`unreachable`／`prohibit` |
| `invert` | `not` |
| `ipproto` / `sport` / `dport` | 同名 |
| `uidrange` | `uidrange` |
| `suppress_prefixlength` | 同名 |
| `disabled` | 暫時停用這條（不用刪掉） |

IPv6 用 `config rule6`。

### `ip route` 也可以

```
# /etc/config/network
config route
    option interface 'lan'
    option target    '0.0.0.0/0'
    option type      'local'
    option table     '100'
```

支援 `target`／`netmask`／`gateway`／`source`／`metric`／`table`／`type`／`mtu`／`onlink`。

> **注意**：`config route` 綁在某個 `interface` 上——**該介面 down 的時候這條路由也會被移除**。
> 對 TPROXY 那條 `local 0.0.0.0/0 dev lo table 100` 來說，綁 `loopback` 介面最合適。

### nft 規則：兩個位置

| 做法 | 適用 |
|---|---|
| `/etc/nftables.d/*.nft` | 內容會被放進 `inet fw4` **表裡面**，所以只能寫 chain，不能寫 `table`；要用 inet family 語法 |
| 自己的 init script + `nft -f` | 獨立的 `table ip xxx`，與 fw4 解耦，`fw4 restart` 不影響 |

（兩者的取捨見本篇稍早的說明。）

### 測試階段請故意不要持久化

**「重開機就消失」在測試時是優點，不是缺點**——它是你最可靠的救援手段。
確認整條鏈都通、也拿掉了測試用的 `ip saddr` 限制之後，才做持久化。
持久化之後**一定要重開機驗一次**，因為「開機自動生效」正是它唯一的目的。

## 十、回退與救援

### 清理一定要三樣一起

TPROXY 是三個零件，**清理也要三個一起**——只刪其中一個會留下「半套狀態」：

```bash
nft delete table ip v2ray_tproxy 2>/dev/null
ip rule del pref 100 2>/dev/null          # 用 pref 刪比重打條件安全
ip route flush table 100 2>/dev/null
```

建議寫成一個 `tproxy-down.sh`，測試時反覆用。

**只刪 nft 表、留下 `ip rule` 與 `ip route` 會怎樣？** 實測過一次：

```
ip rule:   32765: from all fwmark 0x1 lookup 100      ← 還在
table 100: local default dev lo scope host            ← 還在
nft:       整份 ruleset 裡沒有任何 meta mark set       ← 沒人設 mark 了
```

**當下無害**——規則的條件是 `fwmark 0x1`，沒有封包帶這個標記，所以永遠不命中。

**但它是一顆未爆彈。** 那條規則配上 `local default` 的語意是
「**任何帶 mark 1 的封包一律視為目的地是本機，不轉發、不送出**」——非常粗暴的攔截。
只要哪天有東西開始設 mark 1，那些流量就會被整個吞掉，**而且封包只是消失、沒有任何錯誤訊息**。

`1` 這個數字太小太常見：VPN 分流、mwan3、SQM 都是 fwmark 的重度使用者
（本機就裝了 OpenVPN）。**撞號的機率不低，清乾淨比較省事。**

清完確認：

```bash
ip rule show                 # 應該只剩 0 / 32766 / 32767 三條
ip route show table 100      # 應該是空的
```

**或直接 `reboot`**——非持久化的東西全部消失。**這就是為什麼第一階段不要持久化。**

### ⚠️ 驗證陷阱：BusyBox 的 `ip` 不支援 `route get ... mark`

想用 `ip route get <IP> mark 1` 直接驗證「帶 mark 的封包會被導去 lo」時，
**在 OpenWrt 上這招行不通，而且它不報錯**：

```
桌機（完整 iproute2）：
  ip route get 8.8.8.8 mark 1  → 8.8.8.8 via 192.168.1.1 ... mark 1     ✅

路由器（BusyBox ip）：
  ip route get 8.8.8.8 mark 1  → 1.0.0.0 via 192.168.0.1 ...            ❌
                                  ↑ 它把 "1" 當成目的位址了
```

**這是最危險的失敗模式：不是錯誤訊息，而是一個看起來合理、實際上完全錯誤的答案。**
在 OpenWrt 上驗證策略路由，**不能依賴 `ip route get` 的 mark 模擬**，
只能靠實際流量的 nft counter 與 v2ray 的 log（見第七節的驗證流程）。

這也是 [排查方法論.md](../linux/排查方法論.md) 那條「先確認你的工具真的在回答你問的問題」
的具體案例。

**已持久化之後**：

```bash
/etc/init.d/tproxy-rules stop
/etc/init.d/tproxy-rules disable      # 開機不再套用
```

**完全連不上路由器時**：OpenWrt 的 failsafe 模式（開機時按住 reset／按鍵，
從 `192.168.1.1` 以唯讀模式進入）可以救回來。**動手前先確認你知道你的機型怎麼進 failsafe。**

---

## 十一、常見陷阱

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

### 陷阱之外：與「顯式代理」並存會怎樣

透明代理設好之後，如果客戶端**同時**還開著 `http_proxy`／`all_proxy`
指向 v2ray 的顯式 inbound（8880／1080），**不會壞，但會產生四個問題**。

**實測**（桌機環境變數指向 8880，送一個請求後看路由器的 log）：

```
proxy/http: request to Method [CONNECT] Host [example.com:443]
app/dispatcher: taking detour [vmess-out-s02] for [tcp:example.com:443]
192.168.1.171:37522 accepted //example.com:443 [vmess-out-s02]
```

#### 1. 同一台機器分裂成兩條出口路徑

本機的 routing 規則是：

```
inboundTag: [socks-in-1080, http-in-8880] → vmess-out-s02
inboundTag: [socks-in-3080, tproxy-in]    → vmess-out-s03
```

**顯式代理走 s02、透明代理走 s03——同一台機器的流量從兩台不同伺服器出去。**
出口 IP 不同、延遲不同、被封鎖的狀況也可能不同。

#### 2. 哪個程式走哪條，取決於它讀不讀環境變數

| 程式 | 走哪條 |
|---|---|
| `curl`／`git`／`pip`（讀環境變數） | 顯式代理 |
| `ssh`／遊戲／不讀環境變數的 | 透明代理 |
| 從選單啟動的 GUI（拿不到環境變數） | 透明代理 |

**排查時會很痛苦**——同一個域名在兩個程式裡行為不同，得先確認它走了哪條路。

#### 3. 前提：bypass 清單必須涵蓋路由器自己的 IP

這是**會真的壞掉**的那個。顯式代理時封包的目的地是**路由器自己**：

```
dst=192.168.1.1:8880 → br-lan → prerouting
  → ip daddr @bypass return
     192.168.1.1 ∈ 192.168.0.0/16 → ✅ return，不代理
  → 正常路由 → input → v2ray 的 8880
```

**若 bypass 沒涵蓋它**：這個封包會被 `tproxy to :12345`，dokodemo 取到的原始目的地是
`192.168.1.1:8880`，塞進 vmess 送給遠端——**遠端伺服器去連 `192.168.1.1:8880`，
那是它自己的內網**，直接失敗。顯式代理會整個不能用，而且錯誤訊息非常難懂。

#### 4. DNS 與域名的處理方式相反（顯式代理反而較好）

| | 顯式代理 | 透明代理 |
|---|---|---|
| 誰解析 DNS | **遠端伺服器**（客戶端只送域名） | **客戶端本地** |
| v2ray 怎麼得到域名 | `CONNECT example.com:443` **直接帶著** | 要靠 **sniffing** 從 TLS SNI 挖 |
| 需要 sniffing | ❌ | ✅ |
| DNS 污染的影響 | **免疫** | 會連到錯的 IP |

上面 log 裡的 `Host [example.com:443]` 就是客戶端直接告訴它的，**沒有經過任何嗅探**。
**「顯式代理比較原始」這個直覺是錯的**——在域名處理上它反而更乾淨。
（原理見 [代理與VPN的根本差別](../linux/代理與VPN的根本差別-為什麼需要sniffing.md)。）

#### 建議

**二選一，不要並存：**

- 走透明代理 → 把客戶端的 proxy 環境變數拿掉，並開 `sniffing`
- 走顯式代理 → 就不用弄 TPROXY，但不讀環境變數的程式沒得代理

**真要並存**，至少讓兩條路走**同一個 outbound**，消除出口不一致：

```jsonc
{ "inboundTag": ["socks-in-1080","http-in-8880","tproxy-in"], "outboundTag": "vmess-out-s02" }
```


---

## 十二、opkg 完整命令與選項（安裝核心模組會用到）

### 命令

**套件操作**

| 命令 | 作用 |
|---|---|
| `update` | **更新可用套件清單**（本機 `/var/opkg-lists/` 是空的，一定要先跑） |
| `install <pkgs>` | 安裝 |
| `upgrade <pkgs>` | 升級 |
| `configure <pkgs>` | 設定已解開的套件 |
| `remove <pkgs\|regexp>` | 移除 |
| `flag <flag> <pkgs>` | 標記套件；flag 可為 `hold`／`noprune`／`user`／`ok`／`installed`／`unpacked` |

**查詢**

| 命令 | 作用 |
|---|---|
| `list` / `list-installed` / `list-upgradable` | 列出可用／已安裝／可升級 |
| `list-changed-conffiles` | **列出被使用者改過的設定檔** |
| `files <pkg>` | 該套件裝了哪些檔案（等同 `dpkg -L`） |
| `search <file\|regexp>` | **哪個套件提供這個檔案**（等同 `dpkg -S`） |
| `find <regexp>` | 名稱或描述符合的套件 |
| `info` / `status [pkg]` | 完整資訊／狀態 |
| `download <pkg>` | 只下載不安裝 |
| `compare-versions <v1> <op> <v2>` | 比較版本（`<=` `<` `>` `>=` `=` `<<` `>>`） |
| `print-architecture` | 可安裝的架構 |
| `depends` / `whatdepends` / `whatdependsrec` | 相依／被誰相依／遞迴 |
| `whatrecommends` / `whatsuggests` / `whatprovides` / `whatconflicts` / `whatreplaces` | 各種關係查詢（都可加 `-A`） |

### 選項

| 選項 | 作用 |
|---|---|
| `-A` | 查詢**所有**套件，不只已安裝的 |
| `-V<等級>` / `--verbosity[=<等級>]` | 0 只有錯誤／1 一般（預設）／2 資訊／3 除錯／4 除錯 2 |
| `-f <檔>` / `--conf <檔>` | 指定設定檔 |
| `--cache <目錄>` | 使用套件快取 |
| `-d <名稱>` / `--dest <名稱>` | **安裝到指定的 dest**（外接儲存裝置常用） |
| `-o <目錄>` / `--offline-root <目錄>` | 離線安裝的根目錄 |
| `--verify-program <路徑>` | 指定驗簽程式 |
| `--add-arch <arch>:<優先度>` / `--add-dest <名稱>:<路徑>` | 註冊架構／目的地 |
| **`--noaction`** | **只測試不執行** ← 裝東西前先跑這個 |
| `--download-only` | 只下載 |
| `--no-check-certificate` | 不驗證 SSL 憑證 |

**Force 選項**

`--force-depends`（無視相依）、`--force-maintainer`（覆寫既有設定檔）、
`--force-reinstall`、`--force-overwrite`（覆寫其他套件的檔案）、`--force-downgrade`、
`--force-space`（跳過空間檢查）、`--force-postinstall`（離線模式也跑 postinstall）、
`--force-remove`（prerm 失敗也移除）、`--force-checksum`（checksum 不符也繼續）。

> **路由器的 flash 空間通常很小**，`--force-space` 尤其危險——空間不足硬裝可能讓系統無法開機。
> 本機 overlay 還有 6.8G，這點倒是不用擔心。

## 自我測驗

1. TPROXY 的三個零件各自解決什麼問題？如果只做了 nftables 規則、漏掉 `ip rule` 與
   `ip route add local`，封包會走到哪裡、為什麼 v2ray 完全收不到？
2. `followRedirect: true` 沒開會發生什麼事？從「dokodemo-door 怎麼知道封包原本要去哪」回答。
3. 為什麼本文的設定不會造成「v2ray 連往上游伺服器的封包被自己攔回來」的迴圈？
   什麼情況下你才**必須**開始用 `SO_MARK` 排除自己的流量？
4. 透明代理設好之後，客戶端若同時還開著指向 8880 的 `http_proxy`，會發生什麼？
   請說出「出口會不一致」以外的另一個問題，以及為什麼 bypass 清單沒涵蓋路由器 IP 時
   顯式代理會整個失效。
5. 情境題：你照著做完，`nft list table ip v2ray_tproxy` 的 counter 一直在增加，但測試機完全連不上
   網路、v2ray 的 log 也沒有任何新連線。請說出最可能的斷點在哪一個零件，以及你會用哪兩個指令確認。
