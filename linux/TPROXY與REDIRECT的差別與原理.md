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
| 代理**本機自己**發起的流量 | ✅ 用 `output` 鏈 | ❌ **做不到** |

**這是一個實務上很關鍵的差異**：要讓路由器**自己**的流量（`opkg update`、ntp、容器）走代理，
TPROXY 幫不上忙——只能用 `output` 鏈的 REDIRECT（TCP 限定），或改用
[TUN 模式](../openwrt/路由器上的透明代理-為什麼不是TUN模式.md)。

所以常見的成熟方案是**兩者並用**：
`prerouting` 用 TPROXY 處理 LAN 的 TCP+UDP，`output` 用 REDIRECT 處理本機的 TCP。

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
3. 你要讓路由器**自己**發起的連線走代理。TPROXY 為什麼幫不上忙？還有哪兩個選擇？
4. TPROXY 需要 `ip rule` + `ip route add local`，REDIRECT 完全不需要。為什麼？
   從「封包的目的地有沒有被改寫」解釋。
5. 情境題：你把 nft 規則從 `redirect to :12345` 改成 `tproxy to :12345`，
   其餘都沒動（鏈型別、優先權、代理程式設定、路由）。會有哪三件事出錯？
