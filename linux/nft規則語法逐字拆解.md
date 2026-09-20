# nft 規則語法逐字拆解 —— 每個英文字、每個標點是什麼意思

> 日期：2026-09-21
> 情境：看得懂規則在做什麼了，但 `type filter hook forward priority filter; policy drop;`
> 這種句子裡每個字的角色、分號為什麼出現在那裡，還是說不清楚
> 對應檔案：路由器上的 `inet fw4` 規則集（fw4 從 `/etc/config/firewall` 編譯產生）
> 適用範圍：nftables 語法本身適用於任何 Linux；文中範例取自 OpenWrt 24.10.2 的 fw4 規則集（nftables 1.1.1）
> 相關：[iptables與nftables怎麼用.md](./iptables與nftables怎麼用.md)、
> [讀懂fw4的nft規則.md](../openwrt/讀懂fw4的nft規則.md)（範例規則集的整體骨架在那篇）

**一句話結論：nft 的語法只有兩種東西——**宣告**（table／chain，用 `{}` 包住內容、用 `;` 結束每個
子句）和**規則**（由左到右：一串「比對」後面接一串「動作」，沒有 `-j` 這種旗標）。看不懂的時候，
先問「這個 token 是在描述封包長什麼樣，還是在描述要對它做什麼」，答案只有這兩類。**

---

## 目錄

- [一、整份檔案的三層結構](#一整份檔案的三層結構)
- [二、`table inet fw4 {` 逐字](#二table-inet-fw4--逐字)
- [三、chain 宣告那一行逐字](#三chain-宣告那一行逐字)
- [四、一條規則的組成](#四一條規則的組成)
- [五、比對（expression）逐個拆](#五比對expression逐個拆)
- [六、動作（statement）逐個拆](#六動作statement逐個拆)
- [七、標點符號總表](#七標點符號總表)
- [八、實戰：讀懂最複雜的三條](#八實戰讀懂最複雜的三條)
- [自我測驗](#自我測驗)

---

## 一、整份檔案的三層結構

```
table inet fw4 {                ← 第 1 層：表
        chain input {           ← 第 2 層：鏈
                type filter hook input priority filter; policy drop;   ← 鏈的「規格」
                iif "lo" accept                                        ← 第 3 層：規則
                iifname "br-lan" jump input_lan
        }
        chain forward {
                ...
        }
}
```

**`{}` 在這裡是「內容的範圍」**，跟程式語言的大括號一樣。整份 180 行就是
「一個 table 包著 35 條 chain，每條 chain 包著若干 rule」。

---

## 二、`table inet fw4 {` 逐字

| token | 是什麼 | 說明 |
|---|---|---|
| `table` | **關鍵字** | 宣告一張表 |
| `inet` | **family（協定家族）** | 這張表處理哪種封包 |
| `fw4` | **名字** | 自己取的，fw4 用這個名字 |
| `{` | 開始 | 表的內容從這裡開始 |

**family 有五種**，決定這張表能看到什麼封包：

| family | 處理什麼 |
|---|---|
| `ip` | 只有 IPv4 |
| `ip6` | 只有 IPv6 |
| **`inet`** | **IPv4 與 IPv6 共用一套規則**（現代寫法，fw4 用這個） |
| `arp` | ARP 封包 |
| `bridge` | 橋接的封包 |
| `netdev` | 網卡驅動層，比 prerouting 更早 |

**因為 `inet` 同時處理 v4 和 v6**，所以規則裡才會常看到 `meta nfproto ipv4` 這種「先篩掉另一種」
的寫法——這在 `ip` family 的表裡是不需要的。

> 對照 iptables：iptables 的「表」是固定的四張（filter/nat/mangle/raw），**你不能自己建**；
> nft 的 table 是你自己建、自己命名、自己決定 family 的。

---

## 三、chain 宣告那一行逐字

```
type filter hook forward priority filter; policy drop;
└─┬┘ └─┬──┘ └┬─┘ └─┬───┘ └──┬───┘ └─┬──┘│ └─┬──┘ └┬─┘│
  1    2     3     4        5      6   7   8    9  10
```

| # | token | 是什麼 |
|---|---|---|
| 1 | `type` | 關鍵字：接下來要說這條鏈的**種類** |
| 2 | `filter` | **種類的值**（見下表） |
| 3 | `hook` | 關鍵字：接下來要說掛在哪個**攔截點** |
| 4 | `forward` | **攔截點的值**（prerouting／input／forward／output／postrouting） |
| 5 | `priority` | 關鍵字：接下來是**優先權** |
| 6 | `filter` | **優先權的值**——這裡的 `filter` 是一個**有名字的數字常數**，等於 `0` |
| 7 | `;` | **子句結束**——「鏈的規格」這句講完了 |
| 8 | `policy` | 關鍵字：接下來是**預設處置** |
| 9 | `drop` | 預設處置的值（`accept` 或 `drop`） |
| 10 | `;` | 子句結束 |

**注意第 2 個和第 6 個都是 `filter`，但意思完全不同**——一個是「種類」，一個是「優先權數字的
別名」。這是最容易看糊塗的地方。

### `type` 的三種值

| type | 能做什麼 | 你的規則裡誰用 |
|---|---|---|
| `filter` | 一般過濾：accept／drop／reject | `input`、`forward`、`output`、`mangle_*` |
| `nat` | **只有這種能做位址轉換**（`masquerade`／`dnat`／`snat`）。而且**只有每條連線的第一個封包會進來** | `dstnat`、`srcnat` |
| `route` | 封包被改動後**重新查路由**（改了 mark 或目的位址時需要） | `mangle_output` |

你機器上 `mangle_output` 是 `type route hook output` 而不是 `type filter`，就是因為在 output
改了封包之後必須重新做路由判定——否則路由已經選好了不會變（見
[iptables與nftables 的心智模型](./iptables與nftables怎麼用.md#二兩者共用的心智模型)）。

### `priority` 的名字與數字

同一個 hook 上可以掛多條鏈，**數字小的先執行**：

| 名字 | 數字 | 你的規則裡 |
|---|---|---|
| `raw` | -300 | `raw_prerouting`、`raw_output` |
| `mangle` | -150 | `mangle_prerouting`、`mangle_forward`… |
| `dstnat` | -100 | `dstnat` |
| `filter` | 0 | `input`、`forward`、`output` |
| `srcnat` | 100 | `srcnat` |

**可以直接寫數字**：`priority 0` 和 `priority filter`完全等價。這些名字正好對應 iptables 的
四張表——fw4 等於把 iptables 的表模擬成了不同優先權的鏈。

### `policy` 什麼時候才會用到

**只有「整條鏈從頭跑到尾，沒有任何規則做出結論」時才會套用。**

你的 `chain forward` 是 `policy drop`，但最後一行是 `jump handle_reject`——所以實務上
policy 永遠輪不到，因為 `handle_reject` 會先把封包 reject 掉。
**policy 是最後一道保險，不是主要機制。** 省略不寫的話預設是 `accept`。

> **實務提醒**：在命令列建鏈時，這串規格裡有 `{`、`}`、`;`，全都是 shell 的特殊字元，
> **必須用單引號包起來**：
> ```bash
> nft add chain inet fw4 mychain '{ type filter hook input priority 0; policy drop; }'
> ```

---

## 四、一條規則的組成

```
iifname "br-lan" jump input_lan comment "!fw4: Handle lan input traffic"
└──── 比對 ────┘ └── 動作 ───┘ └────────── 註解 ──────────┘
   expression       statement
```

規則的文法很單純：

```
[比對1] [比對2] ... [動作1] [動作2] ...
```

- **比對（expression）**：描述「什麼樣的封包」。多個比對之間是 **AND**（全部成立才算命中）。
- **動作（statement）**：描述「要做什麼」。可以有多個，**由左到右依序執行**。
- **沒有 `-j`**：iptables 用 `-j ACCEPT` 指定目標，nft 直接把動作寫在最後。

這就是為什麼 `counter packets 274619 bytes 16200896 accept` 讀起來像兩件事——它**真的**是兩個
動作：先計數，再放行。

**「AND」與「OR」怎麼表達**：

```
iifname "br-lan" tcp dport 22 accept          # AND：從 br-lan 進來 而且 目的埠是 22
tcp dport { 22, 80, 443 } accept              # OR：用集合表達「其中之一」
```

---

## 五、比對（expression）逐個拆

以下全部來自你的規則。

### `iifname "br-lan"` / `oifname { "eth1", "phy1-sta0" }`

進入／送出的介面名稱。**引號是因為它是字串**；大括號是「集合」（見第七節）。
`iif`／`oif` 是用介面索引編號的版本。

### `meta nfproto ipv4`

**`meta` 表示「封包的中繼資料」**——不是封包內容本身，而是核心附加的資訊
（哪張網卡進來、是 v4 還是 v6、mark 值、送出它的行程 uid 等）。

`nfproto` 是「netfilter 協定」，值為 `ipv4` 或 `ipv6`。在 `inet` family 裡用來分流。

### `meta l4proto igmp`

`l4proto` 是第四層協定（tcp／udp／icmp／igmp…）。
**為什麼不寫 `ip protocol igmp`？** 因為在 `inet` 表裡要同時支援 v4／v6，
`meta l4proto` 兩邊通用，而 `ip protocol` 只適用 IPv4。

### `ct state established,related`

`ct` = **conntrack（連線追蹤）**。`state` 的值有 `new`／`established`／`related`／`invalid`／`untracked`。
逗號分隔表示「其中之一」。

### `tcp flags & (fin | syn | rst | ack) == syn`

這是**位元遮罩運算**，整行的意思是「**只有 SYN 被設定、其他三個都沒設**」＝ 新連線的第一個封包。

```
tcp flags              取出 TCP 的旗標位元
& (fin|syn|rst|ack)    只保留這四個位元（AND 遮罩），其餘清掉
== syn                 遮罩後的結果必須剛好等於「只有 syn」
```

**為什麼不能直接寫 `tcp flags syn`？** 那樣只要 SYN 有設就算命中，連
`SYN+ACK`（第二次握手）也會被抓到。加上遮罩比對，才能精確抓出「第一個封包」。

### `icmpv6 type . icmpv6 code { packet-too-big . 0, parameter-problem . 1 }`

**點號 `.` 是「串接（concatenation）」**，把多個欄位綁成一個複合值來比對。

意思是：「**(type, code) 這一對組合**，必須落在右邊那個配對清單裡」——
`packet-too-big` 且 code 為 0、或 `parameter-problem` 且 code 為 1。

不用串接的話，你得寫成一堆 `(type==A and code==0) or (type==B and code==1)`，又長又慢。

### `ip6 saddr fe80::/10`

IPv6 來源位址落在 `fe80::/10`（link-local 連線本地位址）範圍內。
`ip saddr`／`ip daddr` 是 IPv4 的來源／目的位址。

### `udp dport 68` / `tcp dport 22`

目的埠；`sport` 是來源埠。`th dport` 則是「不管 tcp 還 udp 的傳輸層目的埠」。

---

## 六、動作（statement）逐個拆

### 終結類：`accept` / `drop` / `reject`

| 動作 | 效果 |
|---|---|
| `accept` | 結束**當前 base chain** 的處理，封包繼續走後面的 hook |
| `drop` | **立刻丟棄**，不回應對方，後面所有 hook 都不執行 |
| `reject` | 丟棄並**回一個錯誤**給對方 |

`reject` 可以指定回什麼，你的 `handle_reject` 示範了兩種：

```
meta l4proto tcp reject with tcp reset    ← TCP 回 RST，對方立刻知道被拒
reject                                     ← 其他協定回 ICMP port-unreachable（預設）
```

**`drop` vs `reject` 的取捨**：drop 讓掃描者等到逾時（對外通常用這個），
reject 讓正常使用者立刻得到明確錯誤（對內比較友善）。

### 流程類：`jump` / `goto` / `return`

| 動作 | 效果 |
|---|---|
| `jump <鏈>` | 進入子鏈；**子鏈跑完會回到原處繼續** |
| `goto <鏈>` | 進入子鏈；**跑完不回來**，直接結束 |
| `return` | 從子鏈**返回呼叫點**；在 base chain 裡等於套用 policy |

你的 `syn_flood` 用了 `return`：

```
chain syn_flood {
    limit rate 25/second burst 50 packets return    ← 沒超速 → 返回 input 鏈繼續往下
    drop                                             ← 超速的才會走到這行
}
```

### `counter`

```
counter packets 274619 bytes 16200896 accept
```

**重點：寫規則時只要打 `counter`**，後面的 `packets N bytes N` 是 nft **列印時**顯示的當前計數值，
不是你要填的參數。

```bash
nft add rule inet fw4 input tcp dport 22 counter accept    # 這樣寫就好
```

### `limit`

```
limit rate 25/second burst 50 packets
limit rate 1000/second burst 5 packets
```

**它的語意是「**通過**速率限制的封包才繼續執行後面的動作」**——所以它既像比對又像動作。

- `rate 25/second`：平均每秒 25 個
- `burst 50 packets`：允許瞬間爆量 50 個（令牌桶，token bucket）

所以 `limit rate 25/second burst 50 packets return` 讀作：「沒超速的就 return（放行），
超速的繼續往下走到 `drop`」。

### `masquerade`

```
chain srcnat_wan {
    meta nfproto ipv4 masquerade
}
```

把來源位址改寫成**出口網卡當下的位址**。和 `snat to 1.2.3.4` 的差別是：masquerade 不用寫死 IP，
適合浮動 IP（撥接、DHCP），代價是每個封包都要查一次出口位址，稍慢。

### `tcp option maxseg size set rt mtu`（你規則裡最進階的一條）

```
oifname { "eth1", "phy1-sta0" } tcp flags & (fin|syn|rst) == syn \
    tcp option maxseg size set rt mtu    « Zone wan IPv4/IPv6 egress MTU fixing »
```

這是 **MSS clamping（最大區段大小箝制）**：

- `tcp option maxseg size` = TCP SYN 封包裡的 MSS（Maximum Segment Size）選項
- `set rt mtu` = 把它**改成「這條路由的 MTU 推算出來的值」**
- 前面的 `tcp flags` 遮罩確保**只改 SYN 封包**（MSS 只在握手時協商）

**為什麼需要它**：PPPoE、VPN 這類封裝會讓實際可用的 MTU 小於 1500。對方若照 1500 發送、
中間又有人擋掉 ICMP「需要分片」訊息，就會出現「網頁開得開但大檔案卡住」的經典症狀。
改寫 MSS 是最直接的解法。

### `comment "..."`

純註解，不影響比對。fw4 產生的都帶 `!fw4:` 前綴——**看到它就知道這條規則是產生物，手改會被覆蓋**。

---

## 七、標點符號總表

| 符號 | 在哪裡 | 意思 |
|---|---|---|
| `{ }` | `table x {` / `chain y {` | **區塊**：內容的範圍 |
| `{ }` | `{ type filter hook input priority 0; policy drop; }` | **鏈的規格**（命令列建鏈時要用引號包） |
| `{ }` | `{ "eth1", "phy1-sta0" }`、`{ 80, 443 }` | **匿名集合**：其中之一（OR） |
| `{ }` | `vmap { established : accept, related : accept }` | **判決對應表**：查表決定動作 |
| `;` | 鏈規格裡 | **子句結束**（規格一句、policy 一句） |
| `,` | 集合裡、`ct state established,related` | 分隔多個值 |
| `.` | `icmpv6 type . icmpv6 code` | **串接**：把多欄位綁成複合值 |
| `:` | `established : accept` | vmap 的「鍵 : 值」 |
| `"` | `"br-lan"`、`comment "..."` | 字串（數字與關鍵字不用引號） |
| `&` | `tcp flags & (fin\|syn)` | 位元 AND（遮罩） |
| `\|` | `(fin \| syn \| rst)` | 位元 OR（組合旗標） |
| `==` | `== syn` | 相等比較（多數情況可省略） |
| `!=` | `iifname != "lo"` | 不等於 |
| `@` | `ip saddr @blacklist` | **引用具名集合**（你的規則裡沒有） |

**`{}` 有四種用途是最容易混淆的一點**——判斷方法：看它前面是什麼。
前面是 `table`／`chain` 就是區塊；前面是欄位名（`oifname`、`dport`）就是集合；
前面是 `vmap` 就是判決表。

---

## 八、實戰：讀懂最複雜的三條

### ①

```
ct state vmap { established : accept, related : accept }
```

> 取出這個封包的 conntrack 狀態（`ct state`），拿它去查右邊這張表（`vmap`）：
> 是 `established` 就執行 `accept`，是 `related` 也執行 `accept`，其他狀態查不到 → 不做任何事、
> 繼續往下一條規則。

比起寫兩條 `ct state established accept` / `ct state related accept`，vmap 是**一次查表**，
規則多時快得多。

### ②

```
meta nfproto ipv4 oifname { "eth1", "phy1-sta0" } ct state invalid counter packets 212 drop
```

> 三個比對（AND）：**是 IPv4**、**而且**要從 eth1 或 phy1-sta0 出去、**而且** conntrack 狀態是
> invalid。三個都成立 → 兩個動作：計數（已累積 212 個）、丟棄。

### ③

```
ip6 saddr fe80::/10 icmpv6 type . icmpv6 code { mld-listener-query . 0, mld-listener-report . 0 } \
    counter packets 0 bytes 0 accept
```

> 比對：來源是 IPv6 link-local 位址，**而且** (type, code) 這個組合是
> 「mld-listener-query 配 0」或「mld-listener-report 配 0」。
> 動作：計數後放行。（MLD 是 IPv6 的多播管理協定，等同 IPv4 的 IGMP。）

---

## 自我測驗

1. `type filter hook forward priority filter;` 這行裡出現了兩次 `filter`，各自是什麼意思？
   為什麼它們可以同名而不衝突？
2. 那行結尾和 `policy drop` 結尾各有一個 `;`。分號在這裡的作用是什麼？為什麼在命令列建鏈時
   整串規格要用單引號包起來？
3. `{ }` 在 nft 裡有四種用途，你要怎麼一眼判斷某個 `{ }` 是哪一種？
4. `tcp flags & (fin | syn | rst | ack) == syn` 為什麼要寫這麼長？直接寫 `tcp flags syn`
   會多抓到什麼封包、造成什麼後果？
5. 情境題：你想加一條規則——「從 br-lan 進來、目的埠是 22 或 443 的 TCP 封包，計數後放行」。
   請寫出完整的 `nft add rule` 指令，並指出你用到了哪幾種標點、各自的作用。
