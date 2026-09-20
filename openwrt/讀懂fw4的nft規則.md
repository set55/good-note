# 怎麼讀懂 OpenWrt 的 `nft list ruleset`？

> 日期：2026-09-20
> 情境：OpenWrt 24.10 沒有 iptables，只能用 `nft list ruleset`，但 180 行輸出完全看不懂
> 對應檔案：路由器上的 `inet fw4` 規則集（由 `/etc/config/firewall` 編譯產生，不是人手寫的）
> 適用範圍：OpenWrt 24.10.2、nftables 1.1.1、firewall4
> 相關：[nftables與uci是什麼-兩層設定的分工.md](./nftables與uci是什麼-兩層設定的分工.md)、
> [iptables與nftables怎麼用.md](../linux/iptables與nftables怎麼用.md)、
> [nft規則語法逐字拆解.md](../linux/nft規則語法逐字拆解.md)（每個 token 的意思在那篇）

**一句話結論：不要從第一行開始讀。先把 chain 分成兩種——有 `hook` 的是**核心的入口**（只有 13 條），
沒有 `hook` 的只是**被 jump 呼叫的子函式**（22 條）。然後挑一條你在乎的封包路徑，從入口鏈一路
`jump` 追下去。fw4 的鏈名有嚴格規律，看名字就知道它在處理誰。**

---

## 一、先分兩種 chain

```bash
nft list ruleset | grep -A1 'chain '     # 看每條鏈的下一行有沒有 type/hook
```

| | 有 `type ... hook ...` | 沒有 |
|---|---|---|
| 叫什麼 | **base chain**（基礎鏈） | **regular chain**（一般鏈） |
| 誰呼叫它 | **核心**，封包經過該 hook 時自動執行 | 只有被 `jump`／`goto` 才會執行 |
| 本機有幾條 | 13 | 22 |

本機的 13 條 base chain（**這就是整份規則的全部入口**）：

```
input               filter hook input       priority filter   policy drop
forward             filter hook forward     priority filter   policy drop
output              filter hook output      priority filter   policy accept
prerouting          filter hook prerouting  priority filter   policy accept
dstnat              nat    hook prerouting  priority dstnat   policy accept
srcnat              nat    hook postrouting priority srcnat   policy accept
raw_prerouting      filter hook prerouting  priority raw      policy accept
raw_output          filter hook output      priority raw      policy accept
mangle_prerouting   filter hook prerouting  priority mangle   policy accept
mangle_postrouting  filter hook postrouting priority mangle   policy accept
mangle_input        filter hook input       priority mangle   policy accept
mangle_output       route  hook output      priority mangle   policy accept
mangle_forward      filter hook forward     priority mangle   policy accept
```

**同一個 hook 上可以掛好幾條鏈，靠 `priority` 決定先後**（數字小的先跑）：
`raw`(-300) → `mangle`(-150) → `dstnat`(-100) → `filter`(0) → `srcnat`(100)。
這正好對應 iptables 的 raw／mangle／nat／filter 表——**fw4 只是把 iptables 的「表」換成了
「不同優先權的鏈」**。

剩下 22 條 regular chain 全是子函式，名字有嚴格規律：

| 名稱樣式 | 意思 |
|---|---|
| `input_<zone>` / `forward_<zone>` / `output_<zone>` | 該 zone 的對應方向規則 |
| `accept_to_<zone>` / `accept_from_<zone>` | 放行「往該 zone」／「從該 zone 來」 |
| `reject_to_<zone>` / `reject_from_<zone>` | 拒絕同上 |
| `srcnat_<zone>` | 該 zone 的來源 NAT |
| `helper_<zone>` | conntrack helper |

本機有三個 zone：**`lan`、`wan`、`ovpnsetwall`**（OpenVPN）。
**看到 `forward_lan` 就知道：這是「從 lan 進來、要被轉發」的封包會走的地方**——
名字是從 `/etc/config/firewall` 的 zone 名字編出來的。

---

## 二、怎麼讀單獨一條規則

nft 的規則沒有 `-j`，語法是**由左到右：一串條件，最後一個動作**。

```
iifname "br-lan" jump forward_lan comment "!fw4: Handle lan IPv4/IPv6 forward traffic"
└────條件────┘ └──────動作──────┘ └────────────註解────────────┘
```

常見條件：

| 寫法 | 意思 |
|---|---|
| `iifname "br-lan"` | 從這張網卡**進來**（`iif` 是介面索引版，比較快但介面重建會失效） |
| `oifname { "eth1", "phy1-sta0" }` | 從這幾張網卡**出去**（大括號是集合，等於「或」） |
| `meta nfproto ipv4` | 只比對 IPv4（因為 `inet` 家族同時處理 v4／v6） |
| `ct state established,related` | conntrack 狀態 |
| `tcp dport 22` | TCP 目的埠 |

常見動作：`accept`、`drop`、`reject`、`jump <鏈>`、`masquerade`、`counter`。

**`counter` 不是動作而是「順便計數」**，後面還會接真正的動作：

```
counter packets 274619 bytes 16200896 accept
        └─────── 已經命中 27 萬個封包 ───────┘ └動作┘
```

**這個數字是排查的黃金線索**：規則沒作用時，先看它的 counter 是不是 0——是 0 就代表封包
根本沒走到這裡，問題在更前面。

---

## 三、走一遍：LAN 電腦開網頁

這是路由器最主要的工作。封包是**轉發**的，走的是第三條路線（見
[iptables與nftables怎麼用.md](../linux/iptables與nftables怎麼用.md) 的三條路線）。

**它實際經過的每一站**——`prerouting` 一個都沒少，只是多數是空的：

| 順序 | hook | priority | 鏈 | 本機狀況 |
|---:|---|---|---|---|
| 1 | prerouting | raw (-300) | `raw_prerouting` | **空** |
| 2 | prerouting | mangle (-150) | `mangle_prerouting` | **空** |
| 3 | prerouting | dstnat (-100) | `dstnat` | **空**（沒設任何埠轉發） |
| 4 | prerouting | filter (0) | `prerouting` | 1 條：`jump helper_lan`（helper_lan 本身也是空的） |
| — | **路由判定** | | | 「不是給我的」→ 走 forward |
| 5 | forward | mangle (-150) | `mangle_forward` | **2 條：MSS clamping** |
| 6 | forward | filter (0) | `forward` | 5 條 ← **下面細講** |
| 7 | postrouting | mangle (-150) | `mangle_postrouting` | **2 條：MSS clamping** |
| 8 | postrouting | srcnat (100) | `srcnat` | 2 條 ← **下面細講** |

**所以「為什麼走查從 `forward` 開始」的答案是：前四站在你的路由器上幾乎都是空的**，
封包穿過去什麼事都沒發生。但**鏈是存在的、封包是有經過的**——之後你要加 TPROXY 規則，
就是要掛在第 2 站那個優先權上。空的鏈在做什麼、mangle 那四條規則是什麼，見
[第四節](#四那些空的鏈與-mangle-在做什麼)。

### 第 1 站：`forward`（base chain，核心呼叫）

```
chain forward {
    type filter hook forward priority filter; policy drop;      ← 預設丟棄
    ct state vmap { established : accept, related : accept }    ← ①
    iifname "br-lan"  jump forward_lan                          ← ②
    iifname { "eth1", "phy1-sta0" } jump forward_wan
    iifname "tun0"    jump forward_ovpnsetwall
    jump handle_reject                                          ← ③
}
```

- **①** 「已建立／相關」的連線直接放行。`vmap`（verdict map，判決對應表）是 nft 才有的寫法，
  等於一張查表：狀態是 `established` 就 `accept`。**放第一行是刻意的**——絕大多數封包是
  回程流量，一條規則就處理掉，不必往下比對幾十條。
- **②** 第一次的封包才會走到這裡。從 `br-lan`（LAN 橋接）進來 → 跳到 `forward_lan`。
- **③** 前面都沒命中才會到這行（例如從一個沒定義 zone 的介面進來）。

> `policy drop` 是「這條鏈跑完都沒有人 accept 就丟棄」。這裡永遠輪不到，因為第 ③ 行的
> `handle_reject` 會先處理掉。**兩者差別：drop 是不回應（對方等到逾時），reject 是明確回絕。**

### 第 2 站：`forward_lan`（regular chain）

```
chain forward_lan {
    jump accept_to_wan       ← 允許 lan → wan
    jump accept_to_lan
}
```

就是把 `/etc/config/firewall` 裡「lan → wan 允許轉發」那條設定展開成兩個跳轉。

### 第 3 站：`accept_to_wan`

```
chain accept_to_wan {
    meta nfproto ipv4 oifname { "eth1", "phy1-sta0" } ct state invalid counter packets 212 drop
                                                     « Prevent NAT leakage »
    oifname { "eth1", "phy1-sta0" } counter packets 274619 bytes 16200896 accept
}
```

- 第一條擋掉 `invalid` 狀態的封包，註解寫著「防止 NAT 洩漏」——conntrack 認不得的封包若放出去，
  可能帶著內網來源位址（沒被 NAT），會洩漏內網結構。**實測已經擋掉 212 個。**
- 第二條：要從 wan 口出去的，放行。**counter 顯示 27 萬個封包——這就是你上網的流量。**

到這裡 filter 階段結束，封包被放行。

### 第 4 站：`srcnat` —— 注意，這裡**不是** jump 過來的

看完 `accept_to_wan` 你可能會問：它最後一行是 `accept`，**那怎麼跳到 `srcnat` 的？**

**答案是：沒有跳。整份規則裡沒有任何一行 `jump srcnat`。**

```bash
$ nft list ruleset | grep 'jump srcnat$'
（沒有結果）
```

`forward`／`forward_lan`／`accept_to_wan` 是 forward hook 底下的一棵樹；`srcnat` 是
**postrouting hook 上的另一條獨立 base chain**，由核心在封包走到那裡時另外呼叫。
**串起它們的是核心的封包路徑，不是規則。**

```
chain srcnat {
    type nat hook postrouting priority srcnat; policy accept;
    oifname { "eth1", "phy1-sta0" } jump srcnat_wan
}
chain srcnat_wan {
    meta nfproto ipv4 masquerade      « Masquerade IPv4 wan traffic »
}
```

`masquerade` 就是把來源位址改成出口網卡的位址——`/etc/config/firewall` 裡 wan zone 那個
`option masq '1'` 的產物。這也是為什麼它沒有 counter 也無所謂：`type nat` 的鏈只處理
**每條連線的第一個封包**，不像 `accept_to_wan` 是每個封包都算（所以那裡是 27 萬）。

> `accept` 的作用範圍、hook 與 jump 為何是兩回事、`nat` 鏈只處理首包的完整說明，見
> [iptables與nftables怎麼用.md](../linux/iptables與nftables怎麼用.md#二兩者共用的心智模型)。

---

**完整路徑（標示清楚哪裡是 jump、哪裡是核心換 hook）：**

```
LAN 電腦 → br-lan
   │
   │  ── 核心進入 forward hook ──
   ├─► chain forward ──jump──► forward_lan ──jump──► accept_to_wan → accept
   │                                                    （離開 forward hook）
   │  ── 核心繼續走網路堆疊，抵達 postrouting hook ──
   ├─► chain srcnat ──jump──► srcnat_wan → masquerade（改寫來源位址）
   │
   └─► eth1 → 網際網路
```

---

## 四、那些空的鏈，與 mangle 在做什麼

### 為什麼 13 條 base chain 有大半是空的

實測本機每條 base chain 的規則數：

```
input               7 條        dstnat              （空）
forward             5 條        raw_prerouting      （空）
output              5 條        raw_output          （空）
prerouting          1 條        mangle_prerouting   （空）
srcnat              2 條        mangle_input        （空）
mangle_forward      2 條        mangle_output       （空）
mangle_postrouting  2 條        helper_lan          （空）
```

**fw4 一律把整套骨架建好，不管你有沒有設定。** 這樣之後你在 uci 裡加一條埠轉發，
它只要往既有的 `dstnat` 鏈插一條規則就好，不必重新規劃鏈結構。

**空的鏈等於一個明確的訊息**：那個 hook／優先權的組合，你目前什麼都沒設定。
例如 `dstnat` 是空的 → **你沒有設定任何通訊埠轉發**（LuCI 的「通訊埠轉發」頁是空的）。
這比「找不到規則」有用得多——你可以直接排除「是不是 DNAT 把封包改走了」這個可能。

### mangle 在你的路由器上只做一件事：MSS clamping

四條 mangle 規則，全部是同一件事，分成進與出兩個方向：

```
chain mangle_forward {          # 從 wan／tun0 進來的
    iifname { "eth1", "phy1-sta0" } tcp flags & (fin|syn|rst) == syn \
        tcp option maxseg size set rt mtu   « Zone wan ... ingress MTU fixing »
    iifname "tun0" ...                      « Zone ovpnsetwall ... ingress MTU fixing »
}

chain mangle_postrouting {      # 往 wan／tun0 出去的
    oifname { "eth1", "phy1-sta0" } ...     « Zone wan ... egress MTU fixing »
    oifname "tun0" ...                      « Zone ovpnsetwall ... egress MTU fixing »
}
```

**它從哪來？一個 uci 開關：**

```bash
$ uci show firewall | grep mtu_fix
firewall.@zone[1].mtu_fix='1'        # ← wan zone 的「MSS clamping」勾選
```

**一個 uci 選項 → 兩條鏈、四條 nft 規則**，這是 [uci 與 nftables 上下游關係](./nftables與uci是什麼-兩層設定的分工.md)
最好的例子。（同一個 zone 的 `masq='1'` 則產生了 `srcnat_wan` 的 `masquerade`。）

**它在解決什麼**：PPPoE、VPN 這類封裝會讓實際 MTU 小於 1500。雙方若照 1500 協商 MSS、
中間又有人擋掉 ICMP「需要分片」訊息，就會出現**「網頁開得開、但大檔案或某些網站卡死」**
的經典症狀。改寫 SYN 封包裡的 MSS 值是最直接的解法。
（`tcp option maxseg size set rt mtu` 的逐字拆解見
[nft規則語法逐字拆解.md](../linux/nft規則語法逐字拆解.md#六動作statement逐個拆)。）

**為什麼要做兩次（forward 進來 + postrouting 出去）**：連線的兩個方向都會各自發一個 SYN
（你發 SYN、對方回 SYN+ACK），兩邊宣告的 MSS 都要修正，所以進出兩個方向都要攔。

### mangle 優先權一般還能做什麼

| 用途 | 典型寫法 |
|---|---|
| 打封包標記（policy routing 用） | `meta mark set 1` |
| **TPROXY 透明代理** | `tproxy to :12345`——**必須掛在 mangle 優先權**，因為要早於路由判定 |
| 改 DSCP／TOS（QoS 分類） | `ip dscp set cs1` |
| 改 TTL | `ip ttl set 64` |

**`mangle_prerouting` 現在是空的，但它正是你之後要加 TPROXY 規則的地方**——
或者更乾淨的做法是另開一張自己的表掛在同一個優先權上（見
[用dokodemo-door做TPROXY透明代理.md](./用dokodemo-door做TPROXY透明代理.md)）。

### `raw` 優先權與 `helper_*` 鏈

- **`raw`（-300）**：比 conntrack 更早執行，典型用途是 `notrack`——對不需要追蹤狀態的流量
  （例如大量的 DNS）跳過 conntrack 以節省資源。你這台是空的。
- **`helper_lan`**：conntrack helper 的指派（FTP、SIP 這類需要核心幫忙解析控制通道的協定）。
  也是空的，代表沒啟用任何 helper——現代作法多半改用其他方式，預設關閉是合理的。

---

## 五、走一遍：外網的人連路由器（本文的「第二條路徑」）

> **先澄清一個命名混淆**：這是本文的第二次走查，但在
> [三條路線的分類](../linux/iptables與nftables怎麼用.md#二兩者共用的心智模型)裡，
> 它其實是**路線①「給本機的」**。前一節那個轉發的才是路線③。兩邊的編號沒有對應關係。

情境：外面某台機器想連你路由器的某個埠（例如有人掃到你的 IP，試著連 22）。

### 第 1 站：prerouting 的四條鏈

和轉發的封包**一模一樣**——任何從網卡進來的封包都先過這裡，此時核心**還不知道**它是給本機的
還是要轉走。

```
raw_prerouting     （空）
mangle_prerouting  （空）
dstnat             （空）   ← 這一站很關鍵，見下面
prerouting/filter  iifname "br-lan" jump helper_lan  → 從 wan 進來，不符合，略過
```

**為什麼 `dstnat` 這一站特別關鍵**：如果你設過通訊埠轉發，規則就在這條鏈裡，
它會**改寫封包的目的位址**。而下一步的路由判定是**拿改寫後的位址**去判斷的——

```
沒有埠轉發：目的地 192.168.0.103（路由器自己）→ 路由判定「是給我的」→ input
有埠轉發  ：dstnat 把目的地改成 192.168.1.50 → 路由判定「不是給我的」→ forward
```

**這就是為什麼埠轉發的放行規則要寫在 `forward` 而不是 `input`**——經過 DNAT 之後，
那個封包在核心眼裡已經不是給路由器的了。很多人在 input 開了埠卻發現埠轉發不通，原因就在這裡。

### 第 2 站：路由判定——核心怎麼知道「這是給我的」

它查的是 **`local` 路由表**。本機實測：

```bash
$ ip route show table local
local     127.0.0.0/8     dev lo          ← 迴路
local     192.168.0.103   dev phy1-sta0   ← WAN 側位址（這台是用 Wi-Fi 上聯）
local     192.168.1.1     dev br-lan      ← LAN 側位址
broadcast 192.168.1.255   dev br-lan
broadcast 192.168.0.255   dev phy1-sta0
```

**目的位址只要命中這張表裡任何一條 `local`（或 `broadcast`），就判定「是給我的」→ 走 `input`；
否則 → 走 `forward`。** 這張表是你每次設定介面位址時，核心自動維護的，不需要手動加。

所以「外面的人連 192.168.0.103:22」→ 命中 `local 192.168.0.103` → 進入 input hook。

### 第 3 站：input hook 的兩條鏈

```
mangle_input   priority mangle (-150)   （空）
input          priority filter (0)      ← 主角
```

### 第 4 站：逐行走 `chain input`

```
chain input {
    type filter hook input priority filter; policy drop;
①  iif "lo" accept
②  ct state vmap { established : accept, related : accept }
③  tcp flags & (fin | syn | rst | ack) == syn jump syn_flood
④  iifname "br-lan" jump input_lan
⑤  iifname { "eth1", "phy1-sta0" } jump input_wan
⑥  iifname "tun0" jump input_ovpnsetwall
⑦  jump handle_reject
}
```

以「外網來的新 SSH 連線」逐條比對：

| 行 | 比對結果 | 發生什麼 |
|---|---|---|
| ① | 不是從 `lo` 來 | 不命中，往下 |
| ② | 是新連線（`new`），vmap 查不到 | 不命中，往下 |
| ③ | **是 SYN** | **跳進 `syn_flood`** |
| ④ | 不是從 br-lan | 不命中，往下 |
| ⑤ | **是從 eth1／phy1-sta0** | **跳進 `input_wan`** |
| ⑥⑦ | — | `input_wan` 若沒放行就在裡面被拒了，走不到這裡 |

**第 ③ 行的 `jump` 很值得看清楚**：

```
chain syn_flood {
    limit rate 25/second burst 50 packets return    ← 沒超速 → return 回到 input 的第 ④ 行
    drop                                             ← 超速的封包在這裡就死了
}
```

`jump` 進去、`return` 出來，**回到原本那條鏈的下一行繼續**——所以速率正常的 SYN 還是會往下走
到第 ⑤ 行。這是 `jump`／`return` 最典型的用法：**把一段共用邏輯抽成子鏈**。

### 第 5 站：`input_wan` 的白名單

```
chain input_wan {
    udp dport 68 accept                    « Allow-DHCP-Renew »   ← 向上游續租 IP
    icmp type echo-request accept          « Allow-Ping »
    meta l4proto igmp counter packets 355 accept   « Allow-IGMP »
    udp dport 546 accept                   « Allow-DHCPv6 »
    ip6 saddr fe80::/10 icmpv6 type . icmpv6 code { ... } accept    « Allow-MLD »
    icmpv6 type { ... } limit rate 1000/second burst 5 packets accept
    jump reject_from_wan                   ← 白名單之外，統統到這裡
}
```

**這是「預設拒絕、明列例外」的白名單模型。** SSH（tcp 22）不在名單上 → 逐條都不命中 →
走到最後的 `jump reject_from_wan`。

對照 `input_lan`：

```
chain input_lan { jump accept_from_lan }    ← 內網無條件放行
```

**兩條鏈的對比就是整個防火牆的政策**：內網全信、外網只開必要的幾項。

### 第 6 站：被拒絕的路徑

```
chain reject_from_wan {
    iifname { "eth1", "phy1-sta0" } counter packets 2209 bytes 406600 jump handle_reject
}

chain handle_reject {
    meta l4proto tcp reject with tcp reset   ← TCP 回 RST
    reject                                    ← 其他回 ICMP port-unreachable
}
```

**那個 `counter packets 2209` 是真實數字**——從開機到現在，已經有 2209 個來自外網的封包被擋下來。
這就是你的路由器暴露在公網上的日常。

### 完整路徑圖

```
外網主機
   │  目的地 192.168.0.103:22
   ▼
phy1-sta0（wan）
   │
   │ ── prerouting hook ──
   ├─► raw_prerouting（空）→ mangle_prerouting（空）→ dstnat（空）→ prerouting（不符）
   │
   │ ── 路由判定：查 local 表，命中 local 192.168.0.103 → 「是給我的」──
   │
   │ ── input hook ──
   ├─► mangle_input（空）
   ├─► chain input
   │     ③ SYN → jump syn_flood → 未超速 → return
   │     ⑤ iifname phy1-sta0 → jump input_wan
   │           └ 白名單都不命中 → jump reject_from_wan
   │                 └ counter +1 → jump handle_reject
   │                       └ TCP → reject with tcp reset
   ▼
回一個 RST 給對方，連線被拒
```

**如果是被放行的情況**（例如 ping）：在 `input_wan` 的 `icmp type echo-request accept` 命中 →
`accept` 結束整條 input base chain → 封包交給路由器本機的網路堆疊 → 由核心回應 ICMP echo-reply。

## 六、自己查的指令

```bash
# 只看骨架（哪些是入口）
nft list ruleset | grep -E 'chain |hook'

# 只看某一條鏈，不用捲 180 行
nft list chain inet fw4 forward
nft list chain inet fw4 input_wan

# 看 handle 編號（要刪規則時需要）
nft -a list chain inet fw4 forward

# 排查：這條規則到底有沒有被命中
nft list chain inet fw4 accept_to_wan      # 看 counter 數字會不會增加

# 即時追蹤一個封包走過哪些規則（需先加一條 meta nftrace set 1 的規則）
nft monitor trace
```

**判斷規則是不是人寫的**：看註解。帶 `comment "!fw4: ..."` 的全部是 fw4 從
`/etc/config/firewall` 編譯出來的產生物——**改它沒有用，下次 `/etc/init.d/firewall restart`
就會被重新生成覆蓋**（見 [nftables與uci是什麼](./nftables與uci是什麼-兩層設定的分工.md)）。

---

## 自我測驗

1. 面對 180 行的 `nft list ruleset`，為什麼不該從第一行往下讀？正確的切入點是什麼、怎麼一眼找出來？
2. `chain forward` 的第一條規則是 `ct state vmap { established : accept, related : accept }`。
   為什麼它要放在最前面？放到最後會有什麼後果（功能上與效能上）？
3. `policy drop` 和鏈最後那行 `jump handle_reject`，在「封包被擋下來」這件事上有什麼不同？
   哪一個對連線方比較友善？
4. 你看到 `accept_to_wan` 裡有條規則的 counter 是 `packets 0`。這代表規則寫錯了嗎？
   還可能是什麼原因？你會往哪個方向查？
5. 情境題：你想讓外網能連到路由器的 22 埠（SSH）。根據本文的鏈結構，這條規則應該出現在哪一條
   鏈裡、為什麼不是 `forward`？而且照 fw4 的運作方式，你應該去改哪個檔案、而不是直接 `nft add rule`？
