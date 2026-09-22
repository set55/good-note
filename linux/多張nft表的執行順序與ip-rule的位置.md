# 兩張 nft 表都有 prerouting，封包走哪張？—— 以及 nftables 與 `ip rule`／`ip route` 誰先誰後

> 日期：2026-09-22
> 情境：想知道兩張 nftables 表（table）各自都有 `prerouting`、`forward`、`input` 鏈時，
> 核心怎麼決定用哪張表的規則；以及 nftables 的 hook 與 `ip rule`／`ip route` 在封包旅程中的先後
> 適用範圍：Linux netfilter 通用；本機 kernel 7.0、nftables v1.0.9
> 相關：[iptables與nftables怎麼用.md](./iptables與nftables怎麼用.md)（五個 hook 與三條路線的基礎圖）、
> [nft完整用法-從命令到規則語法.md](./nft完整用法-從命令到規則語法.md)（`priority` 名字與數字對照）、
> [ip-rule與ip-route完整指南.md](./ip-rule與ip-route完整指南.md)（路由決策的兩層）

**一句話結論：核心從來不「選」哪張表——表（table）只是命名空間，核心只認 hook。每個 hook 上掛著
一串來自各張表的 base chain，全部依 `priority` 由小到大輪流執行；`accept` 只結束自己那一條，
`drop` 才是全部結束。而 `ip rule`／`ip route` 不是 hook，是夾在 hook 之間的一個「路由判定」站：
進來的封包先過 `prerouting`、才查路由；本機發出的封包則相反，先查路由、才過 `output`。**

---

## 目錄

- [一、核心不選表：表只是命名空間](#一核心不選表表只是命名空間)
- [二、同一個 hook 上多條鏈：priority 決定順序](#二同一個-hook-上多條鏈priority-決定順序)
- [三、判決（verdict）跨表的效果](#三判決verdict跨表的效果)
- [四、family 也會分流：ip／ip6／inet／netdev](#四family-也會分流ipip6inetnetdev)
- [五、路由判定在哪裡：完整時間軸](#五路由判定在哪裡完整時間軸)
- [六、由時間軸推出來的五個結論](#六由時間軸推出來的五個結論)
- [七、自己驗證的方法](#七自己驗證的方法)
- [自我測驗](#自我測驗)

---

## 一、核心不選表：表只是命名空間

直覺上會以為「封包進來 → 核心挑一張表 → 跑那張表的 prerouting」。**錯。** 核心的角度完全沒有「表」：

```
你寫的（人的視角）                       核心看到的（hook 的視角）
────────────────────                     ─────────────────────────
table inet A {                           ipv4 prerouting hook:
  chain prerouting {                       ├ priority -150  A/prerouting
    type filter hook prerouting            └ priority    0  B/pre_x
      priority mangle; ...
  }                                      ipv4 input hook:
  chain input { type filter                ├ priority    0  A/input
    hook input priority 0; ... }           └ priority   10  B/in_y
}
table ip B {
  chain pre_x { type filter
    hook prerouting priority 0; ... }
  chain in_y { type filter
    hook input priority 10; ... }
}
```

建立一條 **base chain**（有寫 `type … hook … priority …` 的鏈）時，nftables 做的事是**把這條鏈
註冊到核心的某個 hook 上**。從此以後，核心在那個 hook 只會看到「一串依 priority 排好的函式」，
它不知道、也不在乎這些函式來自哪張表。

所以三個常見誤解：

| 誤解 | 真相 |
|---|---|
| 鏈取名 `prerouting` 就會在 prerouting 執行 | **鏈名毫無意義**，決定位置的是 `hook prerouting`。沒寫 hook 的鏈是 regular chain，只能被 `jump`／`goto` 呼叫（見 [nft完整用法](./nft完整用法-從命令到規則語法.md) 的〈鏈名 vs hook〉） |
| 兩張表只會用其中一張 | **兩張都會跑**，只要封包走到那個 hook、family 也對得上 |
| 表 A 先建立所以先跑 | 建立順序不重要（除了 priority 完全相同的特例，見下節），**priority 才重要** |

表的真正用途是**管理上的分組**：`nft delete table inet A` 一次移除整組、不同程式各用自己的表
互不干擾（OpenWrt 的 fw4 用 `inet fw4`、Docker 用自己的表、你手寫的 TPROXY 規則放另一張）。

---

## 二、同一個 hook 上多條鏈：priority 決定順序

**數字小的先跑，跨表也一樣。** 常見名字與數字（完整對照在
[nft完整用法](./nft完整用法-從命令到規則語法.md#priority-的名字與數字)）：

```
prerouting hook 上的典型順序（ipv4）：

 -300  raw        ← 例：notrack
 -200  (conntrack) ← 不是你寫的鏈，是核心的連線追蹤（connection tracking）本身
 -150  mangle     ← 例：打 mark、TPROXY
 -100  dstnat     ← DNAT（Destination NAT，目的位址轉換）
    0  filter     ← 一般過濾
```

注意 **-200 那格不是任何表的鏈**，而是 conntrack 模組自己也掛在同一個 hook 上。這就是為什麼
`priority raw`（-300）的鏈可以做 `notrack`——它跑在 conntrack 之前；而 `ct state` 比對必須在
-200 之後的鏈才有意義。iptables（不論 legacy 還是 iptables-nft）的表也是用同一組數字掛上去的，
**所以 iptables 與 nftables 的規則會在同一個 hook 上交錯執行**，不是二選一。

**priority 完全相同時**：兩條鏈的先後是核心實作細節（大致依註冊順序），官方文件不保證。
**不要依賴它**——需要先後就明確給不同的數字，例如 `priority filter - 5`、`priority filter + 10`。

---

## 三、判決（verdict）跨表的效果

這是「兩張表」問題裡最容易出事的地方：

| 判決 | 在自己的鏈裡 | 對同一個 hook 上後面的鏈（不論哪張表） | 對後面的 hook |
|---|---|---|---|
| `accept` | 結束這條 base chain | **照樣執行** | 照樣執行 |
| `drop` | 結束 | **不執行**，封包消失 | 不會到 |
| `policy drop`（跑到鏈尾沒結論） | 等同 `drop` | 不執行 | 不會到 |
| `return`（在 base chain 裡） | 等同套用 policy | 依 policy 而定 | 依 policy 而定 |

由此得到兩條實務規則：

1. **放行要「每一張」都同意；拒絕只要「任何一張」說不。** 你在表 A `accept` 了 SSH，但表 B 的
   input 鏈是 `policy drop` 而沒放行 SSH，SSH 一樣進不來。這就是「我明明加了 accept 規則卻不通」
   最常見的原因之一——去找**別張表**（常是 fw4、firewalld、Docker 或殘留的 iptables 規則）。
2. **不能用「在前面的表 accept」來繞過後面的表。** 想讓某流量「跳過」另一張表的檢查，
   只能改那張表本身（或讓它的規則不命中），沒有跨表的「最終放行」。

**`type nat` 鏈的特例**：同一個 hook 上有多條 nat 鏈時，**第一條做出轉換（dnat／snat／masquerade）
的鏈就決定了這條連線的 NAT**，之後的 nat 鏈對這條連線不再評估；而且 nat 鏈本來就只看每條連線的
第一個封包（見 [iptables與nftables怎麼用](./iptables與nftables怎麼用.md) 的〈`nat` 型別的鏈只處理
每條連線的第一個封包〉）。所以兩張表都寫了 DNAT 時，是 priority 比較小的那張說了算。

---

## 四、family 也會分流：ip／ip6／inet／netdev

「兩張表都會跑」有個前提：**family 要對得上封包的種類**。

| 表的 family | 會看到什麼封包 |
|---|---|
| `ip` | 只有 IPv4 |
| `ip6` | 只有 IPv6 |
| `inet` | IPv4 與 IPv6 都會（它的鏈同時註冊到兩邊的 hook） |
| `arp` | ARP（Address Resolution Protocol，位址解析協定） |
| `bridge` | 經過 Linux bridge 的二層（L2）訊框，走的是 bridge 自己的 hook |
| `netdev` | 綁定某張網卡的 `ingress`／`egress`，**比 prerouting 更早**（封包剛從網卡驅動上來時） |

所以 `table ip B` 的 prerouting 永遠碰不到 IPv6 封包；`table inet A` 與 `table ip B` 則會在
IPv4 的 prerouting hook 上依 priority 交錯。

---

## 五、路由判定在哪裡：完整時間軸

`ip rule`／`ip route` 不是 netfilter 的一部分，它們是網路堆疊裡的**路由判定（routing decision）**，
發生在固定的兩個時間點之間。把 priority 與路由判定放在同一條時間軸上：

### ① 從外面進來的封包

```
網卡
 │
 ├─ [netdev ingress]                     ← 綁網卡的 netdev 表（若有）
 │
 ├─ [prerouting hook]  依 priority：
 │     -300 raw  →  -200 conntrack  →  -150 mangle  →  -100 dstnat  →  0 filter
 │     （這裡可以：打 meta mark、DNAT 改目的位址、TPROXY）
 │
 ├─ ★ 路由判定（輸入方向）
 │     1. ip rule 由 pref 小到大比對：from／to／iif／fwmark／tos …
 │        → 命中的規則說「查哪張表」
 │     2. 在那張表做最長前綴匹配（ip route）
 │        → 查到的是 local 路由（通常在 local 表） → 給本機
 │        → 查到一般路由                          → 轉發（需 ip_forward=1）
 │     3. 同時做 rp_filter 反向路徑檢查（Reverse Path filter）
 │
 ├─ 給本機 ──► [input hook]   -150 mangle → 0 filter → 100 srcnat → 本機 socket
 │
 └─ 轉發 ────► [forward hook] -150 mangle → 0 filter
                     │
                     ▼
               [postrouting hook]  -150 mangle → 100 srcnat（SNAT／masquerade）→ 網卡
```

### ② 本機程式發出的封包

```
本機程式 send()
 │
 ├─ ★ 路由判定（輸出方向）
 │     ip rule → ip route：決定出口網卡、下一跳、**來源位址**
 │     此時看得到的條件：目的位址、socket 的 SO_MARK、uid（uidrange）、綁定的網卡（oif）
 │
 ├─ [output hook]  依 priority：
 │     -300 raw → -200 conntrack → -150 mangle → -100 dstnat → 0 filter
 │     ★ 若 type route 鏈改了 mark／位址，或 dstnat 改了目的位址 → **重新做一次路由判定**
 │
 └─ [postrouting hook]  100 srcnat → 網卡
```

**兩條路線的關鍵差別：進來的封包是「先 hook、後路由」；發出的封包是「先路由、後 hook」。**
第二條之所以要「重新路由」機制，正是因為 output 的規則跑的時候，路由已經選好了。

---

## 六、由時間軸推出來的五個結論

1. **prerouting 打的 mark，`ip rule fwmark` 看得到。** 因為 prerouting 在路由判定之前。
   這就是 TPROXY 的整套設計：mangle 優先權的鏈 `meta mark set 1` → 路由判定時
   `ip rule add fwmark 1 lookup 100` 命中 → table 100 的 `local 0.0.0.0/0 dev lo` 讓封包被當成
   「給本機的」而走 input，而不是 forward（見
   [ip-rule與ip-route完整指南](./ip-rule與ip-route完整指南.md#七回頭看-tproxy-的那兩行)）。

2. **在 input／forward 打 mark 已經來不及影響這次路由。** 路由判定已經做完了，mark 只能給後面的
   hook 或回程封包（配合 `ct mark`）用。想用 mark 做策略路由，**進來的封包一定要在 prerouting 打**。

3. **DNAT 要在 prerouting 做，路由才會照新的目的地走。** `dstnat`（-100）排在路由判定之前，
   所以 DNAT 成 LAN 裡的某台機器後，路由判定看到的已經是新位址，自然走 forward。
   反過來，SNAT 放在 postrouting，是因為要等路由選好出口網卡才知道該換成哪個來源位址。

4. **本機流量要走策略路由，mark 得靠 `type route` 鏈觸發重新路由。** 在普通 `type filter` 的
   output 鏈改 mark，路由不會重選；改成 `type route hook output priority mangle` 的鏈，核心才會
   發現 mark 變了而重新查 `ip rule`。代理本機流量時常見的 `output` 打 mark → 封包經 `lo`
   繞回 prerouting → 在 prerouting 做 TPROXY，靠的就是這個。

5. **「規則沒作用」要先問：封包到底走到哪個 hook？** 是否走 input 還是 forward，是**路由判定**
   決定的，不是 nft 決定的。在 input 寫了規則卻沒命中，很可能路由判定把封包判成轉發了
   ——用 `ip route get` 模擬一次就知道。

---

## 七、自己驗證的方法

```bash
# 1. 看某個 hook 上，所有表的鏈實際怎麼排（需要 root）
sudo nft list hooks
#   輸出依 family → hook 分組，每行是「priority 數字 + 誰掛在這裡」，
#   形狀大致是：
#     family ip {
#         hook prerouting {
#             -0000000300 chain ip raw PREROUTING [nf_tables]
#             -0000000200 nf_conntrack_in [nf_conntrack]
#             -0000000150 chain inet A prerouting [nf_tables]
#             +0000000000 chain ip B pre_x [nf_tables]
#         }
#     }
#   這一份就是「核心的視角」：沒有表的分組，只有依 priority 排好的一串。

# 2. 追一個封包實際經過哪些表、哪些鏈、得到什麼判決
sudo nft add table inet trace
sudo nft add chain inet trace pre '{ type filter hook prerouting priority -350; }'
sudo nft add rule inet trace pre ip saddr 192.168.1.50 meta nftrace set 1
sudo nft monitor trace          # 會逐行印出 table/chain/rule 與 verdict
sudo nft delete table inet trace   # 用完記得刪

# 3. 模擬路由判定（帶上 mark、入口網卡，就等於模擬 prerouting 之後的那一刻）
ip route get 1.1.1.1 mark 1 from 192.168.1.50 iif br-lan
#   看到 "local 1.1.1.1 dev lo" → 會被當成給本機（走 input）
#   看到 "via ... dev wan"      → 會被轉發（走 forward）
```

trace 那張表故意用 `priority -350`：比 raw（-300）還早，確保在任何人 drop 之前就打上 trace 標記。
`ip route get` 的完整用法見 [ip-rule與ip-route完整指南](./ip-rule與ip-route完整指南.md#六ip-route-get最重要的排查工具)。

---

## 自我測驗

1. 表 A（`inet`）的 input 鏈 priority 0、對 tcp 22 `accept`；表 B（`ip`）的 input 鏈 priority 10、
   `policy drop` 且沒有任何規則。用 IPv4 SSH 進來會怎樣？改用 IPv6 呢？為什麼？
2. 為什麼說「核心不選表」？如果把表 B 的鏈從 `pre_x` 改名成 `prerouting`，行為會有任何改變嗎？
3. 你想讓來自 `192.168.1.50` 的流量走 table 100。有人把 `meta mark set 1` 寫在
   `type filter hook forward` 的鏈裡，`ip rule add fwmark 1 lookup 100` 也加了，但完全沒效果。
   從時間軸解釋為什麼，以及該改到哪裡。
4. 同一個 mark 設定，用在本機程式發出的流量上，放在一般的 `type filter hook output` 鏈裡為何不會
   改變路由？要怎麼改？這跟進來的封包差在哪？
5. 兩張表各有一條 `type nat hook prerouting` 的鏈，都把 tcp 80 DNAT 到不同機器，priority 分別是
   -100 與 -90。連線最後會被送到哪台？如果另外還有一張表在 prerouting `priority raw` 把這個封包
   `drop` 了，兩條 DNAT 還有機會執行嗎？
