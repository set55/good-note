# TPROXY 與 REDIRECT 差在哪？—— 兩種透明代理的原理與取捨

> 日期：2026-09-21
> 情境：要在 OpenWrt 上做透明代理，發現 v2ray 的 dokodemo-door 設定對兩者的要求不同，
> 想搞清楚它們到底差在哪、為什麼 REDIRECT 不能代理 UDP
> 適用範圍：Linux netfilter 通用；語法以 nftables 1.0.9／1.1.1 為準（man nft 為依據）
> 相關：[用dokodemo-door做TPROXY透明代理.md](../openwrt/用dokodemo-door做TPROXY透明代理.md)（實作）、
> [路由器上的透明代理-為什麼不是TUN模式.md](../openwrt/路由器上的透明代理-為什麼不是TUN模式.md)、
> [iptables與nftables怎麼用.md](./iptables與nftables怎麼用.md)

**一句話結論：REDIRECT 是「**改寫**封包的目的地，事後再去 conntrack 問原本要去哪」；
TPROXY 是「**完全不改**封包，直接把它交付給一個宣告過 `IP_TRANSPARENT` 的本機 socket」。
一句話的差別，衍生出所有取捨——改寫過的封包只有 TCP 拿得回原始目的地，所以 REDIRECT 只能代理 TCP；
而 TPROXY 為了不改寫，就得多付出策略路由那一整套設定，而且只能用在 prerouting。**

---

## 目錄

- [一、REDIRECT 的原理](#一redirect-的原理)
- [二、TPROXY 的原理](#二tproxy-的原理)
- [三、為什麼 REDIRECT 不能代理 UDP](#三為什麼-redirect-不能代理-udp)
- [四、兩者的 hook 限制不同（很多人不知道）](#四兩者的-hook-限制不同很多人不知道)
- [五、完整對照表](#五完整對照表)
- [六、優劣與怎麼選](#六優劣與怎麼選)
- [七、規則與程式端的寫法對照](#七規則與程式端的寫法對照)
- [自我測驗](#自我測驗)

---

## 一、REDIRECT 的原理

**REDIRECT 是 DNAT 的特例**——`man nft` 說得很白：

> The redirect statement is a special form of dnat which always [translates to the local host].

所以它本質上就是位址轉換：

```
LAN 電腦送出：  src=192.168.1.50:54321   dst=1.1.1.1:443
                          │
                  [nat prerouting] redirect to :12345
                          │  ← 封包的目的地被改寫
                          ▼
核心看到的變成：src=192.168.1.50:54321   dst=192.168.1.1:12345
                          │
                  路由判定：「這是給我的」→ input → 本機 12345 socket
```

**原始目的地 `1.1.1.1:443` 已經不在封包裡了**，那代理程式怎麼知道要連去哪？

答案是 **conntrack**。做 NAT 時，conntrack 會記錄這條連線的原始五元組。
代理程式接受連線後，用一個 socket 選項去問核心：

```c
getsockopt(fd, SOL_IP, SO_ORIGINAL_DST, &addr, &len);   // → 1.1.1.1:443
```

**回程也靠 conntrack**：代理程式回覆時，核心會自動把來源位址反向改寫回 `1.1.1.1:443`，
LAN 電腦完全不知道中間發生過什麼。

**三個零件：**

| 零件 | 做什麼 |
|---|---|
| `nat` 型別的鏈 + `redirect` | 改寫目的地 |
| conntrack | 記住原始目的地、處理回程 |
| `SO_ORIGINAL_DST` | 程式向核心問回原始目的地 |

**程式端不需要任何特殊權限**——只要會呼叫 `getsockopt` 就行。這是 REDIRECT 最大的優點。

---

## 二、TPROXY 的原理

`man nft` 對 tproxy 的描述是關鍵：

> Tproxy redirects the packet to a local socket **without changing the packet header in any way**.

```
LAN 電腦送出：  src=192.168.1.50:54321   dst=1.1.1.1:443
                          │
              [filter prerouting priority mangle] tproxy to :12345
                          │  ← 封包完全沒變，只是被「交付」給本機 socket
                          ▼
封包仍然是：    src=192.168.1.50:54321   dst=1.1.1.1:443
                          │
                  但核心把它交給了本機的 12345 socket
```

**因為封包沒被改寫，原始目的地就大剌剌寫在封包上**，程式不需要問 conntrack，
直接從封包的輔助資料（control message）讀：

```c
setsockopt(fd, SOL_IP, IP_RECVORIGDSTADDR, &on, sizeof(on));
recvmsg(fd, &msg, 0);          // 從 msg 的 cmsg 取出原始目的地
```

### `tproxy to :12345` 在 prerouting 到底做了什麼（2026-09-22 補充）

常見的疑問：「prerouting 之後核心反正會重新查路由表，那在 prerouting 寫 `tproxy to :12345`
有什麼用？」答案是：**`tproxy` 和路由判定做的是兩件不同的事，兩件都要有**。

| 誰 | 回答的問題 | 決定的東西 |
|---|---|---|
| **路由判定**（`ip rule` → `ip route`） | 這個封包要**收下**（給本機），還是**轉發**？ | 走 input 還是 forward |
| **`tproxy`** | 收下之後，要交給**哪一個 socket**？ | 封包身上綁定的 socket |

`tproxy` 不改封包的目的位址，也不決定路線。它做的是**預先指定 socket**：在封包身上
（核心裡代表封包的 `sk_buff` 結構的 `sk` 欄位）先綁好「等一下交給 12345 那個 socket」。
（socket 在核心裡是什麼、平常核心怎麼替封包找 socket，見 [socket是什麼-從核心看連線的端點.md](./socket是什麼-從核心看連線的端點.md)。）

#### 文法

```
tproxy to [位址][:埠]            ← ip／ip6 表；位址和埠至少給一個
tproxy {ip | ip6} to 位址[:埠]    ← inet 表要指定 family 時
tproxy to :埠                    ← inet 表只給埠時，IPv4／IPv6 都適用
```

| 位置 | 意思 | 省略時 |
|---|---|---|
| `位址` | 要找的 socket 綁在哪個本機位址上 | 用**封包進來那張網卡的主位址**（例如 `br-lan` 的 `192.168.1.1`）——依 `man iptables-extensions` TPROXY 的 `--on-ip`：*By default the address is the IP address of the incoming interface* |
| `:埠` | 要找的 socket 聽在哪個埠 | 沿用封包原本的目的埠 |

所以 `tproxy to :12345` 的意思是「找綁在 `br-lan 主位址:12345` 的透明 socket」。
dokodemo-door 聽在 `0.0.0.0:12345`，`0.0.0.0` 涵蓋所有本機位址，所以找得到。

`man nft` 另外寫明 *Tproxy matching requires another rule that ensures the presence of transport
protocol header*——這就是規則裡一定要有 `meta l4proto { tcp, udp }` 的原因，
`tproxy` 得先確定有 TCP／UDP 標頭，才能讀出埠號。

#### 一個封包的完整時間軸

```
LAN 送出  src=192.168.1.50:54321  dst=1.1.1.1:443
   │
   ├─ [prerouting, priority mangle]
   │     ① tproxy to :12345
   │        先找：有沒有已建立、四元組完全相同的連線 socket？（同一條連線的後續封包走這裡）
   │        沒有：找 192.168.1.1:12345 上、設了 IP_TRANSPARENT 的監聽 socket
   │        找到 → 把它綁在封包身上（封包內容一個位元都沒改）
   │     ② meta mark set 1
   │     ③ accept
   │
   ├─ 路由判定：fwmark 1 → table 100 → local 0.0.0.0/0 → 「給本機的」
   │
   ├─ [input]
   │
   └─ 傳輸層（TCP／UDP）要找 socket 時：
         封包身上已經綁好 socket → 直接用它
         （若沒綁，會照封包標頭找「1.1.1.1:443」的 socket——本機根本沒有）
```

#### 拿掉其中一半會怎樣

| 情況 | 結果 | 原因 |
|---|---|---|
| 只有 `tproxy`，沒有 mark＋`ip rule`＋`local` 路由 | 封包被**轉發**到 `1.1.1.1`，代理完全沒收到 | 路由判定說「不是給我的」，綁好的 socket 沒機會用上。`man iptables-extensions` 對 `--tproxy-mark` 的說明正是這句：*otherwise these packets will get forwarded* |
| 只有 mark＋`ip rule`＋`local` 路由，沒有 `tproxy` | 封包被收下，但 TCP 回 RST、UDP 被丟棄 | 路由說「給本機」，傳輸層就照標頭找 `1.1.1.1:443` 的 socket，找不到 |
| 兩者都有 | 交給 12345 | 路由負責「收下」，`tproxy` 負責「交給誰」 |

**為什麼一定要在 prerouting**：socket 必須在傳輸層查找之前就綁好，而且要趁封包還沒被判定成
轉發之前。所以 `tproxy` 只能用在 prerouting（iptables 版更明確：只能在 mangle 表的 PREROUTING）。

#### 代理沒在聽時的副作用（已對照核心原始碼）

核心 `net/netfilter/nft_tproxy.c` 的 `nft_tproxy_eval_v4()` 最後兩行（torvalds/linux master，2026-09-22 查閱）：

```c
	if (sk && nf_tproxy_sk_is_transparent(sk))
		nf_tproxy_assign_sock(skb, sk);          // 找到透明 socket：綁到封包上
	else
		regs->verdict.code = NFT_BREAK;          // 找不到：這條規則到此中斷
```

`NFT_BREAK` 的意思是「這條規則不成立，換下一條」。所以**找不到**透明 socket 時（v2ray 沒啟動，
或 inbound 沒開 `sockopt.tproxy`），同一條規則後面的 `meta mark set 1` 和 `accept` 都不會執行。
同一個函式裡還看得到本節其他說法的出處：非 TCP／UDP 或分片封包一開始就 `NFT_BREAK`
（所以需要 `meta l4proto`）；先用 `NF_TPROXY_LOOKUP_ESTABLISHED` 找已建立的 socket、找不到才用
`NF_TPROXY_LOOKUP_LISTENER` 找監聽 socket；`nft_tproxy_validate()` 限制只能掛在
`NF_INET_PRE_ROUTING`。省略位址時用的 `nf_tproxy_laddr4()`（`net/ipv4/netfilter/nf_tproxy_ipv4.c`）
取的是入口網卡第一個非 secondary 的位址，沒有才退回封包的目的位址。

結果是：**封包沒打 mark → 走正常路由 → 直接轉發出去，不經過代理。** 不會斷網，
但會「悄悄直連」。排查時如果發現 counter 在跳、流量卻沒經過代理，先確認 12345 真的有在聽，
而且是以 tproxy 模式聽的。

但「不改寫」也帶來兩個必須解決的問題：

### 問題一：核心憑什麼把「不是給我的」封包交給本機 socket？

預設不會。所以 socket 必須先宣告 `IP_TRANSPARENT`：

```c
setsockopt(fd, SOL_IP, IP_TRANSPARENT, &on, sizeof(on));   // 需要 CAP_NET_ADMIN
```

這就是 v2ray dokodemo-door 那個 `"sockopt": {"tproxy": "tproxy"}` 背後做的事。

### 問題二：路由判定會把封包轉發走

封包的目的地還是 `1.1.1.1`，核心在 prerouting 之後做路由判定，
當然判定「不是給我的」→ **直接轉發出去**，`tproxy` 的交付白做了。

所以要用策略路由騙核心：

```bash
ip rule add fwmark 1 lookup 100
ip route add local 0.0.0.0/0 dev lo table 100     # 這張表裡「所有位址都是本機」
```

（完整推導見 [ip-rule與ip-route完整指南.md](./ip-rule與ip-route完整指南.md)。）

### 問題三：回程怎麼辦

代理程式要回覆給 LAN 電腦時，**來源位址必須偽裝成 `1.1.1.1:443`**，
否則 LAN 電腦會拒收（它從沒連過路由器的 12345）。
這同樣靠 `IP_TRANSPARENT`——它允許 socket 用不屬於本機的位址當來源。

**四個零件：**

| 零件 | 做什麼 |
|---|---|
| `filter` 型別、`prerouting`、`mangle` 優先權 + `tproxy` | 交付封包 |
| `IP_TRANSPARENT` | 讓 socket 收得到、也送得出非本機位址 |
| fwmark + `ip rule` + `local` 路由 | 阻止核心把封包轉發走 |
| `IP_RECVORIGDSTADDR` | 程式讀出原始目的地 |

---

## 三、為什麼 REDIRECT 不能代理 UDP

這是最常被問、也最少被解釋清楚的一點。**不是 netfilter 不能 DNAT UDP**——
DNAT UDP 完全合法。問題出在**應用層拿不回原始目的地**。

| | TCP | UDP |
|---|---|---|
| 連線模型 | 每條連線一個獨立的 accepted socket | **一個 socket 收所有來源的 datagram** |
| conntrack 對應 | socket ↔ 連線一對一，可用五元組查到 conntrack 條目 | 沒有「連線」概念，無法把某個 datagram 對應到特定 socket |
| `SO_ORIGINAL_DST` | ✅ 查得到 | ❌ **沒有可查的對象** |
| `IP_RECVORIGDSTADDR` | — | 拿得到「封包上的目的地」，但**REDIRECT 已經把它改寫了**，讀到的是 `192.168.1.1:12345`，不是原始目的地 |

**所以 UDP + REDIRECT 的結果是：你知道有一個封包來了，但不知道它原本要去哪。** 無法轉發。

這也解釋了為什麼各家代理軟體的文件都寫「redir 模式僅支援 TCP」——
v2ray、clash、shadowsocks 都一樣，**不是它們偷懶，是機制上做不到**。

**連帶的後果**：用 REDIRECT 的話，DNS（UDP 53）、QUIC（UDP 443）、遊戲、WireGuard
全部不會走代理。現代流量裡 QUIC 佔比越來越高，這個限制比以前嚴重。

---

## 四、兩者的 hook 限制不同（很多人不知道）

`man nft` 的兩句話決定了一個重要差異：

> The **nat** statements are only valid from **nat chain types**.
> The dnat and **redirect** statements are only valid in the **prerouting and output** chains.

而 tproxy 的官方範例用的是：

```
type filter hook prerouting priority mangle;
```

**`tproxy` 只能用在 prerouting。**

| | REDIRECT | TPROXY |
|---|---|---|
| 鏈型別 | 必須 `type nat` | `type filter` |
| 可用 hook | **`prerouting` 與 `output`** | **只有 `prerouting`** |
| 代理**轉發**流量（LAN 裝置） | ✅ | ✅ |
| 代理**本機自己**發起的流量 | ✅ 直接在 `output` 鏈寫 | ⚠️ **不能直接在 output 寫**，要繞一圈（見下） |

**這是一個實務上很關鍵的差異**：要讓路由器**自己**的流量（`opkg update`、ntp、容器）走代理，
`tproxy` 不能直接寫在 output 鏈。可行的做法有三種：

1. **繞回 prerouting**：在 `type route hook output` 的鏈裡替本機流量打 mark → mark 改變觸發重新路由 →
   `ip rule fwmark` 命中 table 100 的 `local` 路由 → 封包經 `lo` 重新進入 **prerouting** →
   在那裡照常做 `tproxy`（TCP＋UDP 都可以）。這時代理程式自己連往上游的流量一定要用 `SO_MARK`
   排除，否則會被自己攔回來形成迴圈（推導見
   [多張nft表的執行順序與ip-rule的位置.md](./多張nft表的執行順序與ip-rule的位置.md) 第六節第 4 點）。
2. 用 `output` 鏈的 REDIRECT（TCP 限定）。
3. 改用 [TUN 模式](../openwrt/路由器上的透明代理-為什麼不是TUN模式.md)。

> 更正（2026-09-22）：本節原本寫「TPROXY 代理本機流量：做不到」，這不精確——做不到的是
> 「直接在 output 用 tproxy」，繞回 prerouting 的做法是可行的，也是 v2ray 透明代理教學常見的寫法。

常見的組合是：`prerouting` 用 TPROXY 處理 LAN 的 TCP+UDP；本機流量則看需求，選「繞回
prerouting」（要 UDP 時）或 `output` 的 REDIRECT（只要 TCP、想簡單時）。

---

## 五、完整對照表

| 面向 | REDIRECT | TPROXY |
|---|---|---|
| **本質** | DNAT 的特例 | 封包交付（不做 NAT） |
| **封包被改寫** | ✅ 目的地被改寫 | ❌ 完全不動 |
| **取回原始目的地** | `SO_ORIGINAL_DST`（問 conntrack） | `IP_RECVORIGDSTADDR`（讀封包） |
| **TCP** | ✅ | ✅ |
| **UDP** | ❌ **機制上做不到** | ✅ |
| **鏈型別／hook** | `type nat`；prerouting + **output** | `type filter`；**僅 prerouting** |
| **優先權** | `dstnat`(-100) | `mangle`(-150) |
| **程式端需求** | 只要會 `getsockopt` | **`IP_TRANSPARENT`（需 CAP_NET_ADMIN）** |
| **額外路由設定** | 不需要 | **需要 fwmark + ip rule + local 路由** |
| **核心模組** | `nft_redir`／`ipt_REDIRECT`（幾乎都有） | **`nft_tproxy`／`xt_TPROXY`（常要另裝）** |
| **conntrack 負擔** | 每條連線都佔 NAT 條目 | 較輕（不做 NAT） |
| **回程處理** | conntrack 自動反向改寫 | 程式要用 `IP_TRANSPARENT` 偽裝來源 |
| **IPv6** | 支援但歷史上較弱 | 支援完整 |
| **設定複雜度** | 低（一條規則） | 高（四個零件） |
| **除錯難度** | 低 | 高（斷點多，見實作篇的驗證流程） |

---

## 六、優劣與怎麼選

### REDIRECT 的優勢在「簡單」

- **一條 nft 規則就完成**，不用碰路由表。
- 不需要特殊核心模組、不需要特殊權限。
- conntrack 幫你處理回程，錯的機會少。
- **可以代理本機自己的流量**（`output` 鏈）。

**代價只有一個，但很致命：只有 TCP。**

### TPROXY 的優勢在「完整」

- **TCP + UDP 全覆蓋**——DNS、QUIC、遊戲都能代理。
- 封包不被改寫，原始資訊完整，代理程式看到的跟真實一模一樣。
- 不做 NAT，conntrack 壓力較小（高連線數時有差）。

**代價是四個零件全部要對，而且只能處理轉發流量。**

### 決策

| 你的情況 | 選 |
|---|---|
| 只要網頁、只想快速驗證整條鏈通不通 | **REDIRECT** |
| 代理程式沒有 `IP_TRANSPARENT` 支援 | **REDIRECT** |
| 核心沒有 `nft_tproxy` 又不方便裝 | **REDIRECT** |
| 要代理 DNS／QUIC／遊戲／任何 UDP | **TPROXY** |
| 要代理**本機自己**的流量 | **REDIRECT（output 鏈）** 或 TUN |
| 正式長期使用、要求完整 | **TPROXY（LAN）+ REDIRECT（本機）** |

**實務建議：先用 REDIRECT 把整條鏈驗證通**——防火牆規則、代理程式、出口伺服器都確認沒問題之後，
再升級成 TPROXY。這樣出問題時你知道是新加的那一半壞掉，而不是從頭猜。

---

## 七、規則與程式端的寫法對照

### nftables 規則

```bash
# ── REDIRECT ──（注意是 type nat）
nft add table ip proxy_redir
nft add chain ip proxy_redir prerouting '{ type nat hook prerouting priority dstnat; policy accept; }'
nft add rule  ip proxy_redir prerouting ip daddr @bypass return
nft add rule  ip proxy_redir prerouting iifname "br-lan" tcp dport { 80, 443 } counter redirect to :12345
#                                                        ^^^ 只能 tcp

# ── TPROXY ──（注意是 type filter + mangle 優先權）
nft add table ip proxy_tproxy
nft add chain ip proxy_tproxy prerouting '{ type filter hook prerouting priority mangle; policy accept; }'
nft add rule  ip proxy_tproxy prerouting ip daddr @bypass return
nft add rule  ip proxy_tproxy prerouting iifname "br-lan" meta l4proto { tcp, udp } \
    counter tproxy to :12345 meta mark set 1 accept
#                                     ^^^ tcp udp 都可以

# TPROXY 還需要這兩行，REDIRECT 不用
ip rule  add fwmark 1 lookup 100
ip route add local 0.0.0.0/0 dev lo table 100
```

### v2ray dokodemo-door 設定

```jsonc
// REDIRECT：只要 followRedirect
{
  "port": 12345, "protocol": "dokodemo-door",
  "settings": { "network": "tcp", "followRedirect": true }
}

// TPROXY：多一個 sockopt
{
  "port": 12345, "protocol": "dokodemo-door",
  "settings": { "network": "tcp,udp", "followRedirect": true },
  "streamSettings": { "sockopt": { "tproxy": "tproxy" } }
}
```

**判斷別人的設定用哪一種，看有沒有 `sockopt.tproxy` 就知道。**

---

## 自我測驗

1. REDIRECT 與 TPROXY 對封包本身做了什麼不同的事？這一個差別如何連帶決定了
   「代理程式怎麼取回原始目的地」？
2. 為什麼 REDIRECT 不能代理 UDP？請從「TCP 與 UDP 的 socket 模型差異」回答，
   而不是「因為軟體不支援」。
3. 你要讓路由器**自己**發起的連線走代理。為什麼不能直接在 output 鏈寫 `tproxy`？
   「繞回 prerouting」的做法靠什麼讓封包回到 prerouting？為什麼這時一定要把代理程式自己的流量排除？
4. TPROXY 需要 `ip rule` + `ip route add local`，REDIRECT 完全不需要。為什麼？
   從「封包的目的地有沒有被改寫」解釋。反過來，既然路由判定在 prerouting 之後才發生，
   `tproxy to :12345` 寫在 prerouting 又負責什麼？只留它、拿掉路由設定，會怎樣？
5. 情境題：你把 nft 規則從 `redirect to :12345` 改成 `tproxy to :12345`，
   其餘都沒動（鏈型別、優先權、代理程式設定、路由）。會有哪三件事出錯？
