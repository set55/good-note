# iptables 與 nftables 怎麼用？—— 同一個核心框架的兩代前端

> 日期：2026-09-20
> 情境：研究透明代理時一直碰到這兩個工具，想從頭搞懂用法與差別
> 適用範圍：實測 Ubuntu 24.04（iptables 1.8.10、nftables 1.0.9）與 OpenWrt 24.10.2（nftables 1.1.1）
> 相關：[nftables與uci是什麼-兩層設定的分工.md](../openwrt/nftables與uci是什麼-兩層設定的分工.md)、
> [查詢轉發表與路由表.md](./查詢轉發表與路由表.md)

**一句話結論：它們不是兩套防火牆，而是同一個核心框架（netfilter）的兩代使用者空間前端。更關鍵的是——
現代發行版上你打的 `iptables` 指令，底層寫進去的其實已經是 nftables 規則了。所以新系統只需要學 nft，
iptables 的語法「看得懂」就夠。**

---

## 目錄

- [一、先破除誤解：你的 iptables 可能根本不是 iptables](#一先破除誤解你的-iptables-可能根本不是-iptables)
- [二、兩者共用的心智模型](#二兩者共用的心智模型)
- [三、iptables 怎麼用](#三iptables-怎麼用)
- [四、nftables 怎麼用](#四nftables-怎麼用)
- [五、語法對照（官方工具實測產生）](#五語法對照官方工具實測產生)
- [六、關鍵差異](#六關鍵差異)
- [七、為什麼我的 iptables 看起來是空的？](#七為什麼我的-iptables-看起來是空的)
- [八、排查：規則寫了卻沒作用](#八排查規則寫了卻沒作用)
- [九、速查](#九速查)
- [自我測驗](#自我測驗)

---

## 一、先破除誤解：你的 iptables 可能根本不是 iptables

```bash
$ iptables -V
iptables v1.8.10 (nf_tables)        # ← 括號裡寫著 nf_tables
```

**這個括號是整件事的關鍵。** 現在的 `iptables` 套件提供兩個不同的實作：

```bash
$ ls -l /usr/sbin/iptables*
iptables         -> /etc/alternatives/iptables
iptables-legacy  -> xtables-legacy-multi     # 真正的舊版：寫進核心的 x_tables
iptables-nft     -> xtables-nft-multi        # 相容層：語法是 iptables，寫進去的是 nftables

$ update-alternatives --display iptables
link currently points to /usr/sbin/iptables-nft
```

也就是說，在 Ubuntu 24.04 上你打 `iptables -A INPUT ...`，**核心裡建立的是 nftables 規則**，
只是用 iptables 的語法輸入而已。核心裡的舊 `x_tables` 子系統仍在，但預設沒人用它。

另一頭，OpenWrt 24.10 更直接：

```bash
$ ssh root@192.168.1.1 'which iptables nft'
/usr/sbin/nft
ash: iptables: not found              # ← iptables 連裝都沒裝
```

**所以結論很清楚：新系統一律用 nft；iptables 只需要具備「讀得懂別人的腳本、看得懂網路上的教學」
的程度。** 兩者混用要小心——用 `iptables-legacy` 寫的規則，`nft list ruleset` 看不到，
這是「明明設了規則卻查不到」的經典原因。

---

## 二、兩者共用的心智模型

不管哪一代，底下都是 **netfilter**：核心在封包經過網路堆疊時開的五個攔截點（hook）。

**先記三條路線就好**——任何封包只會走其中一條：

```
① 給本機的      入網卡 → [prerouting] → 路由判定 →「給我的」→ [input] → 本機程式
② 本機發出的    本機程式 → 路由判定 → [output] → [postrouting] → 出網卡
③ 幫別人轉的    入網卡 → [prerouting] → 路由判定 →「不是給我的」→ [forward] → [postrouting] → 出網卡
```

把三條疊起來看，就是完整的流程圖：

```
  入網卡
    │
    ▼
[prerouting]
    │
    ▼
  路由判定 ──── 不是給我的 ───► [forward] ──┐
    │                                       │
  給我的                                    │
    │                                       │
    ▼                                       │
 [input]                                    │
    │                                       │
    ▼                                       │
  本機程式                                  │
    │                                       │
    ▼                                       │
  路由判定                                  │
    │                                       │
    ▼                                       │
 [output]                                   │
    │                                       │
    └─────────────────┬─────────────────────┘
                      ▼
                [postrouting]
                      │
                      ▼
                   出網卡
```

**兩個容易看錯的地方：**

1. **`input` 和 `output` 不在同一條路上**。它們是「進來給本機」與「本機發出去」兩個方向的端點，
   中間隔著本機程式。只有轉發的封包（③）才會走 `forward`，而且**轉發的封包永遠不會經過
   `input`／`output`**——這正是[路由器上的透明代理](../openwrt/路由器上的透明代理-為什麼不是TUN模式.md)
   那篇的核心。
2. **本機發出的封包，路由判定在 `output` 之前**。所以在 `output` 鏈裡改了目的位址，
   路由已經選好了、不會自動重選——需要重新查路由時得用 nftables 的 `type route` 鏈
   （iptables 的 `mangle OUTPUT` 也有同樣的問題）。

| hook | 什麼時候 | 典型用途 |
|---|---|---|
| `prerouting` | 剛進來、**路由判定之前** | DNAT（Destination NAT，目的位址轉換）、TPROXY |
| `input` | 確定是給本機的 | 保護本機服務 |
| `forward` | 確定要轉給別人 | 路由器的過濾規則 |
| `output` | 本機程式剛發出（**路由判定之後**） | 限制本機對外 |
| `postrouting` | 要出網卡前 | SNAT（Source NAT）／MASQUERADE |

### `hook` 與 `jump` 是兩回事

一條 base chain 加上它 `jump` 進去的所有子鏈，是**一棵樹**；`jump` 只能在**這棵樹內部**流動。
封包從樹裡出來之後，**不會再跳去別的 hook**——是核心繼續把它往網路堆疊的下一步送，
到了下一個 hook，核心才**重新**呼叫掛在那裡的 base chain。

**把不同 hook 串起來的是核心的封包路徑，不是規則。**

所以「filter 的 forward 鏈 `accept` 之後，怎麼跑到 nat 的 postrouting 鏈去做 NAT？」
這個問題的答案是：**沒有任何規則做這件事，是封包自己走到了那裡。**

```
   ┌── 核心呼叫 ──┐              ┌── 核心呼叫 ──┐
  forward hook                 postrouting hook
    └ chain forward              └ chain srcnat
        └ jump <子鏈>                └ jump <子鏈>
            └ jump <子鏈>                └ masquerade
                └ accept ─┐
                          │（離開 forward hook，
                          │  封包繼續走網路堆疊）
                          └──────────────► 抵達 postrouting hook
```

### `accept`／`drop`／`jump` 的作用範圍

**`accept` 不是「整個防火牆放行」，而是「結束當前這條 base chain 的處理」。**
封包接著照常往下走，後面的 hook 一個都不會少。

| 動作 | 效果 |
|---|---|
| `accept` | 結束**當前 base chain**（含它的 jump 樹），封包繼續走後續 hook |
| `drop` | **立刻丟棄**，整趟旅程到此為止，後面所有 hook 都不執行 |
| `jump <鏈>` | 進入子鏈；子鏈跑完沒有結論就**回到原處繼續** |

還有一個容易忽略的細節：**同一個 hook 上可以掛多條 base chain**（靠 priority 排序），
而 `accept` 只結束自己那一條，**priority 排在後面的照樣會執行**。例如 OpenWrt 的 forward hook：

```
mangle_forward   priority mangle (-150)   ← 先跑
forward          priority filter (0)      ← 後跑
```

只有 `drop` 是跨 hook 的終結——一旦 drop，後面什麼都不會再跑。

### `nat` 型別的鏈只處理每條連線的第一個封包

`type nat`（iptables 的 nat 表）的鏈**不是每個封包都會進去**。第一個封包決定了怎麼轉換之後，
conntrack 會記住這條連線的對應關係，**後續封包與回程封包由 conntrack 直接套用**，不再進 nat 鏈。

兩個實際後果：

- **在 nat 鏈上加 `counter` 量到的是「連線數」，不是封包數**。同一條連線的 filter 鏈 counter
  可能是幾萬，nat 鏈只會是 1。
- **改了 nat 規則，不影響已經建立的連線**——它們的轉換方式早就記在 conntrack 裡了。
  要讓新規則對現有連線生效，得清掉 conntrack 表（`conntrack -F`）。

### `iifname` 與 `oifname` 怎麼記

**`i` = in（進來的介面），`o` = out（出去的介面）**，方向都是相對於**這台機器**，不是相對於網路。

但更好的記法不是背字母，而是問一句：**「封包走到這個 hook 時，核心知不知道答案？」**

| hook | 封包當下的處境 | `iifname` | `oifname` |
|---|---|:---:|:---:|
| `prerouting` | 剛進門，**還沒查路由** | ✅ | ❌ 還不知道要從哪出去 |
| `input` | 已確定是給本機的 | ✅ | ❌ 沒有「出去」這回事 |
| `forward` | 進來了，也決定好要從哪出去 | ✅ | ✅ **兩個都有** |
| `output` | **本機自己產生的** | ❌ 沒有進來的介面 | ✅ |
| `postrouting` | 準備出門 | （轉發的還有） | ✅ |

**這不是規定，是資訊在那個時間點存不存在。** 想不起來時就重走一次
[上面那三條路線](#二兩者共用的心智模型)：封包還沒被路由，核心當然答不出「要從哪張網卡出去」。

上面那張表可以用本機路由器的規則直接驗證——fw4 產生的規則**沒有一條例外**：

```
chain prerouting  (hook prerouting)   iifname × 1     oifname × 0
chain input       (hook input)        iifname × 4     oifname × 0
chain forward     (hook forward)      iifname × 3     oifname × 0
  └ accept_to_wan （forward 的子鏈）   iifname × 0     oifname × 1   ← forward 樹裡兩者都用得到
chain output      (hook output)       iifname × 0     oifname × 4
chain srcnat      (hook postrouting)  iifname × 0     oifname × 2
```

**一個可靠的自我檢查：寫規則時如果你想在 `output` 鏈裡比對 `iifname`，那一定是想錯了**——
本機自己發出的封包沒有「進來的介面」。

### 順帶：`iif` 與 `iifname` 差在哪

| | 比對什麼 | 特性 |
|---|---|---|
| `iif` / `oif` | 介面的**索引編號**（integer） | 比對快；但**介面刪除重建後編號會變**，規則就失效 |
| `iifname` / `oifname` | 介面**名稱**（字串） | 稍慢；**介面還不存在時也能先寫規則** |

所以動態建立的介面（`ppp0`、`tun0`、容器的 veth）一定要用 `iifname`。

本機路由器的規則正好示範了這個取捨：

```
iif "lo" accept                     ← loopback 永遠存在、編號固定，用 iif
iifname "br-lan" jump input_lan     ← 其他介面用 iifname
iifname "tun0" jump input_ovpnsetwall   ← OpenVPN 的 tun0 是動態建立的，更非用 iifname 不可
```

iptables 的對應寫法是 `-i`（in）與 `-o`（out），規則完全一樣。

三個共通的觀念：

1. **規則由上而下比對，第一個命中就決定**（除非動作是 `return`／`jump`）。
2. **每條規則都有 counter**（封包數／位元組數）——這是判斷「規則有沒有被命中」的唯一可靠依據。
3. **conntrack（connection tracking，連線追蹤）** 記住每條連線的狀態，所以只要放行
   「新連線」，回程封包用 `established,related` 一條規則就全放行——這條通常放在最前面，
   既正確又省效能。

---

## 三、iptables 怎麼用

### 四表五鏈

| 表（table） | 用途 | 常用鏈 |
|---|---|---|
| `filter`（預設） | 過濾 | INPUT / FORWARD / OUTPUT |
| `nat` | 位址轉換 | PREROUTING / POSTROUTING / OUTPUT |
| `mangle` | 改封包欄位、打 mark | 五條都有 |
| `raw` | 在 conntrack 之前處理 | PREROUTING / OUTPUT |

### 語法骨架

```
iptables [-t 表] <指令> <鏈> [比對條件...] -j <目標>
```

| 指令 | 意思 |
|---|---|
| `-A <鏈>` | append，加到**最後面** |
| `-I <鏈> [位置]` | insert，插到**最前面**（或指定第幾條） |
| `-D <鏈> <編號或規則>` | 刪除 |
| `-R <鏈> <編號>` | 取代 |
| `-L` / `-S` | 列出（`-S` 印成可複製的指令形式） |
| `-F <鏈>` | 清空整條鏈 |
| `-P <鏈> <目標>` | 設鏈的預設政策 |

常用比對：`-p tcp|udp|icmp`、`-s 來源`、`-d 目的`、`--dport`／`--sport`、`-i 進入網卡`、
`-o 送出網卡`、`-m conntrack --ctstate`、`-m multiport --dports`、`-m limit`。

常用目標：`ACCEPT`、`DROP`（默默丟棄）、`REJECT`（回一個錯誤）、`LOG`、`MASQUERADE`、
`DNAT`、`SNAT`、`RETURN`。

### 常用配方

```bash
# 基本三件套（順序很重要）
iptables -P INPUT DROP                                          # 預設拒絕
iptables -A INPUT -i lo -j ACCEPT                               # 本機迴路一定要放行
iptables -A INPUT -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT   # 回程流量

# 開放服務
iptables -A INPUT -p tcp --dport 22 -j ACCEPT
iptables -A INPUT -p tcp -m multiport --dports 80,443 -j ACCEPT

# 封鎖來源
iptables -I INPUT 1 -s 1.2.3.4 -j DROP                          # -I 插到最前面才會生效

# 分享網路（NAT）
iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
echo 1 > /proc/sys/net/ipv4/ip_forward

# 通訊埠轉發
iptables -t nat -A PREROUTING -p tcp --dport 80 -j DNAT --to-destination 192.168.1.10:8080

# 查看（排查必用）
iptables -L -n -v --line-numbers       # -n 不做 DNS 反查（快很多）、-v 看 counter
iptables -t nat -L -n -v
iptables -S                            # 印成指令形式，好複製

# 刪除
iptables -D INPUT 3                    # 用 --line-numbers 看到的編號
```

**`-A` 與 `-I` 的陷阱**：鏈裡若已經有一條 `-j DROP` 或預設政策是 DROP，你用 `-A` 加的放行規則
排在後面，永遠輪不到。**加放行規則用 `-I`，加封鎖規則用 `-A`**——記不住就先 `-L --line-numbers`
看清楚順序再決定。

### 持久化

iptables 規則**重開機就消失**。

```bash
iptables-save > /etc/iptables/rules.v4      # 匯出
iptables-restore < /etc/iptables/rules.v4   # 匯入
apt install iptables-persistent             # Debian/Ubuntu：開機自動還原
```

---

## 四、nftables 怎麼用

### 三步：建表 → 建鏈 → 加規則

```bash
# 1. 建表（要指定 family）
nft add table inet myfilter
#          ^^^^ ip=IPv4 / ip6=IPv6 / inet=兩者共用 / arp / bridge / netdev

# 2. 建鏈（宣告 hook、type、priority、預設政策）
nft add chain inet myfilter input '{ type filter hook input priority 0; policy drop; }'

# 3. 加規則
nft add rule inet myfilter input iif lo accept
nft add rule inet myfilter input ct state established,related accept
nft add rule inet myfilter input tcp dport 22 accept
nft add rule inet myfilter input tcp dport { 80, 443 } accept      # 集合，一行搞定
```

**和 iptables 最大的不同：表與鏈不是固定的四表五鏈，而是你自己宣告。** 想要什麼 hook、
什麼優先權，自己決定——這就是 TPROXY 能寫 `priority mangle` 的原因。

`inet` family 特別實用：一套規則同時管 IPv4 與 IPv6，不必像 iptables 那樣 `iptables` 與
`ip6tables` 各寫一遍。

### 查看與刪除

```bash
nft list ruleset                       # 全部
nft list table inet myfilter           # 單一表
nft -a list ruleset                    # ← 加 -a 才會顯示 handle 編號
nft delete rule inet myfilter input handle 7      # 刪除要用 handle，不是行號
nft flush chain inet myfilter input    # 清空鏈
nft delete table inet myfilter         # 整張表砍掉
```

**刪規則一定要先 `-a` 查 handle**，這是 nft 和 iptables 用法差最多的地方之一。

### 集合、對應與判決表

```bash
# 具名集合（可動態增刪，不用改規則）
nft add set inet myfilter blacklist '{ type ipv4_addr; flags timeout; }'
nft add element inet myfilter blacklist '{ 1.2.3.4 timeout 1h }'
nft add rule inet myfilter input ip saddr @blacklist drop

# vmap（verdict map，判決對應表）——iptables 做不到
nft add rule inet myfilter input ct state vmap { established : accept, invalid : drop }
```

### 一次載入整份規則（強烈建議）

```bash
# /etc/nftables.conf
#!/usr/sbin/nft -f
flush ruleset                          # ← 先清空，確保結果可重現

table inet myfilter {
    chain input {
        type filter hook input priority 0; policy drop;
        iif lo accept
        ct state established,related accept
        tcp dport { 22, 80, 443 } accept
    }
}
```

```bash
nft -f /etc/nftables.conf              # 套用
systemctl enable --now nftables        # 開機自動載入
```

**`nft -f` 是原子操作**：整份檔案要嘛全部生效、要嘛全部不變，不會出現「改到一半」的半套狀態。
這是它比 iptables 逐條下指令安全的主要原因，也是遠端改防火牆時的保命符。

---

## 五、語法對照（官方工具實測產生）

`iptables-translate` 會把 iptables 指令翻成 nft 語法，**不需要 root、也不會真的套用**，
是最好的學習與遷移工具。以下是在本機跑出來的實際輸出：

| iptables | nft |
|---|---|
| `-A INPUT -p tcp --dport 22 -j ACCEPT` | `add rule ip filter INPUT tcp dport 22 counter accept` |
| `-A INPUT -s 10.0.0.0/8 -j DROP` | `add rule ip filter INPUT ip saddr 10.0.0.0/8 counter drop` |
| `-A INPUT -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT` | `add rule ip filter INPUT ct state related,established counter accept` |
| `-t nat -A POSTROUTING -o eth0 -j MASQUERADE` | `add rule ip nat POSTROUTING oifname "eth0" counter masquerade` |
| `-t nat -A PREROUTING -p tcp --dport 80 -j DNAT --to-destination 192.168.1.10:8080` | `add rule ip nat PREROUTING tcp dport 80 counter dnat to 192.168.1.10:8080` |
| `-A INPUT -p tcp -m multiport --dports 80,443 -j ACCEPT` | `add rule ip filter INPUT ip protocol tcp tcp dport { 80, 443 } counter accept` |
| `-A INPUT -p icmp -j REJECT --reject-with icmp-port-unreachable` | `add rule ip filter INPUT ip protocol icmp counter reject` |
| `-I INPUT 1 -i lo -j ACCEPT` | `insert rule ip filter INPUT iifname "lo" counter accept` |

觀察幾個規律：**`-A` → `add`／`-I` → `insert`、`-j ACCEPT` → `accept`（小寫）、
`-m multiport --dports 80,443` → `{ 80, 443 }`（集合取代了 match 模組）、
`-m conntrack --ctstate` → `ct state`。**

整份腳本可以用 `iptables-restore-translate -f rules.v4` 一次翻譯。

---

## 六、關鍵差異

| | iptables | nftables |
|---|---|---|
| 工具數量 | 4 套（`iptables`／`ip6tables`／`arptables`／`ebtables`） | 1 套 `nft` |
| IPv4／IPv6 | 各寫一遍 | `inet` family 一套搞定 |
| 表與鏈 | 固定的四表五鏈 | **自己宣告** family／hook／priority |
| 更新 | 逐條下指令，中途可能半套 | `nft -f` **原子更新** |
| 集合 | 要另外裝 ipset | 內建 set／map／vmap，支援 timeout |
| 比對擴充 | `-m xxx` 一堆 match 模組 | 統一的運算式語法 |
| 刪除規則 | 用行號 | 用 handle（要先 `-a`） |
| 效能 | 規則多時線性比對 | 集合用雜湊／區間查找，規則多時明顯較快 |

---

## 七、為什麼我的 iptables 看起來是空的？

**因為 Ubuntu 預設就沒有任何防火牆規則——這是正常的，不是壞了。**

桌面版 Ubuntu 出廠時三條鏈的預設政策都是 `ACCEPT`、一條規則都沒有。它的安全性不是靠防火牆，
而是靠「預設沒有服務對外監聽」。所以你會看到的規則，**全部來自某個自己去寫規則的程式**。

### 誰會自己寫規則

| 來源 | 什麼時候會出現 |
|---|---|
| **Docker** | 只要 daemon 在跑就一定有——它**必須**寫規則才能運作 |
| `ufw` | 只有 `ufw enable` 之後（會出現一堆 `ufw-*` 鏈） |
| `firewalld` | RHEL 系的預設，Ubuntu 通常沒裝 |
| libvirt／VirtualBox | 開了虛擬機的 NAT 網路時 |
| Kubernetes（kube-proxy） | 節點上會有大量 `KUBE-*` 鏈 |
| `fail2ban`、tailscale 等 | 各自插自己的鏈 |

本機實測，為什麼只剩 Docker：

```bash
$ systemctl is-active ufw
active                              # ← 看起來開著？
$ grep ^ENABLED /etc/ufw/ufw.conf
ENABLED=no                          # ← 實際上是關的
```

**這是個很容易誤判的陷阱**：`ufw` 的 systemd unit 是 `active`，但它開機時讀設定檔發現
`ENABLED=no`，於是什麼都不做就結束了。**要看 `ufw status`（需 root），不是看 `systemctl`。**

其餘的：`nftables.service` 是 `disabled`、`/etc/iptables/` 不存在（沒裝 `iptables-persistent`）、
`firewalld` 沒裝。所以整台機器只剩 Docker 在寫規則。

### Docker 為什麼一定會有規則

因為它的兩個核心功能都靠 netfilter 實作：

- **容器要連得到外網** → 需要 `MASQUERADE`（在 `nat` 表的 `POSTROUTING`）
- **`-p 8080:80` 的埠對映** → 需要 `DNAT`（在 `nat` 表的 `PREROUTING`）

本機有 `docker0`（172.17.0.0/16）與另一個自訂 bridge，所以這些規則一定存在。
想確認的話，`iptables -t nat -L -n` 會看到 `DOCKER` 鏈。

### 三個「明明有規則卻看不到」的技術原因

1. **只看了 `filter` 表**。`iptables -L` 不加 `-t` 預設只列 `filter`，而 Docker 最主要的規則
   在 `nat`。要看全部：`iptables -L -n -v; iptables -t nat -L -n -v`。
2. **`iptables` 看不到原生 nft 規則**。`iptables-nft` 只認得它自己建立的那幾張相容表
   （`ip filter`、`ip nat`…）。如果有工具直接用 nft 建了自己的 table（像 OpenWrt 的
   `inet fw4`），`iptables -L` **完全看不到**。**`nft list ruleset` 才是唯一的全貌。**
3. **沒有 root**。非 root 執行會直接報權限錯誤，不是「空的」。

### 順帶：沒有防火牆時，暴露面就等於監聽清單

既然沒有規則擋，**任何綁在 `0.0.0.0` 的服務，區網內都連得到**。本機實測綁在 `0.0.0.0`／`::`
的埠包括：

```
80 (http)   111 (sunrpc)   2049 (nfs)   5432 (postgresql)   2080   6030   6060  …
```

`rpcbind`(111)、NFS(2049)、PostgreSQL(5432) 都在對整個區網開著。這不一定有問題（看你信不信任
這個網路），但**值得知道現況**——判斷暴露面要看 `ss -ltn` 的綁定位址，而不是「我沒設防火牆
應該沒差」。（綁定位址的意義見
[netstat 那篇](./netstat怎麼用-以及該改用ss的理由.md)。）

---

## 八、排查：規則寫了卻沒作用

照這個順序查：

```bash
# 1. 規則真的在嗎？（注意 legacy/nft 兩套後端可能各存各的）
nft list ruleset
iptables -S ; iptables-legacy -S 2>/dev/null

# 2. 有沒有被命中？看 counter
iptables -L -n -v --line-numbers        # pkts/bytes 一直是 0 → 根本沒命中
nft list ruleset                        # 規則裡的 counter 同理

# 3. 是不是順序問題？前面有沒有更早命中的規則
iptables -L INPUT -n --line-numbers

# 4. 是不是根本沒走到這個 hook？（轉發流量不會經過 INPUT）
#    先確認封包路徑，再確認 hook 選對

# 5. 追封包實際經過哪些規則
nft monitor trace                       # 搭配 `meta nftrace set 1` 的規則
```

### 五個常見陷阱

1. **Docker 會自己插規則**：Docker daemon 啟動時會建立自己的鏈並改 NAT 表，你手動加的規則
   可能被它的規則搶先命中，或被它重啟時覆蓋。
2. **`ufw`／`firewalld` 只是前端**：用了它們就不要再手動下 iptables／nft，兩邊會互相覆蓋。
3. **重開機消失**：nft 與 iptables 的規則都在記憶體裡，沒存檔就沒了。臨時測試時這其實是**優點**
   ——改壞了重開機就恢復。
4. **`DROP` 與 `REJECT` 的差別**：`DROP` 不回應，對方會一直等到逾時（掃描者看起來像沒這台機器）；
   `REJECT` 立刻回 ICMP（Internet Control Message Protocol，網際網路控制訊息協定）錯誤，
   對方馬上知道被拒。對內用 `REJECT` 體驗較好，對外常用 `DROP`。
5. **預設政策設成 DROP 之前，先確保 SSH 放行規則已經生效**——否則遠端操作時會把自己鎖在門外。
   保險作法：先設一個 `sleep 60 && nft flush ruleset` 的背景任務當後路。

---

## 九、速查

```bash
# === nftables ===
nft list ruleset                        # 看全部
nft -a list ruleset                     # 帶 handle（刪除時需要）
nft add table inet t
nft add chain inet t c '{ type filter hook input priority 0; policy drop; }'
nft add rule inet t c tcp dport { 22, 80 } accept
nft insert rule inet t c iif lo accept  # 插到最前面
nft delete rule inet t c handle 7
nft flush ruleset                       # 清空全部（危險）
nft -f /etc/nftables.conf               # 原子套用整份
nft -c -f file.nft                      # 只檢查語法不套用

# === iptables ===
iptables -L -n -v --line-numbers        # 看規則與 counter
iptables -S                             # 指令形式
iptables -A INPUT -p tcp --dport 22 -j ACCEPT
iptables -I INPUT 1 -i lo -j ACCEPT
iptables -D INPUT 3
iptables-save > rules.v4 ; iptables-restore < rules.v4

# === 兩者之間 ===
iptables -V                             # 看括號：(nf_tables) 還是 (legacy)
iptables-translate -A INPUT -p tcp --dport 22 -j ACCEPT    # 語法翻譯
iptables-restore-translate -f rules.v4                      # 整份腳本翻譯
```

---

## 自我測驗

1. `iptables -V` 顯示 `(nf_tables)` 代表什麼？在這種系統上用 `iptables` 和用 `nft` 下規則，
   核心裡發生的事有什麼不同？
2. 為什麼 `-A` 加的放行規則常常「沒有作用」，改成 `-I` 就好了？背後是什麼機制？
3. 一條 filter 的 forward 鏈 `accept` 之後，封包怎麼會跑到 nat 的 postrouting 鏈去做 NAT？
   是哪條規則把它送過去的？順便說明 `accept` 和 `drop` 的作用範圍差在哪。
4. nftables 的 `nft -f` 相較於逐條下 iptables 指令，最大的安全優勢是什麼？為什麼遠端改防火牆時
   這點特別重要？
5. 情境題：你在伺服器上加了一條 `iptables -A INPUT -p tcp --dport 8080 -j ACCEPT`，但外面還是連不上。
   請列出你的排查順序（至少三個檢查點），並說明每一步分別能排除什麼可能。
