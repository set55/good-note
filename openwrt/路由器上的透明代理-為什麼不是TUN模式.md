# 路由器上的 v2ray 要怎麼改成 TUN 模式？—— 你要的其實是透明代理

> 日期：2026-09-20
> 情境：桌機用 clash-verge 的 TUN 模式做到了 L4（第四層）全系統代理，想把同一招套到
> OpenWrt 上的 v2ray
> 對應檔案：`/root/v2ray/v2ray`、`/root/v2ray/config_openwrt.json`、`/etc/init.d/v2ray`
> （皆為路由器上的正式設定，不在本 repo 內）
> 適用範圍：OpenWrt 24.10.2（mediatek/filogic、aarch64、kernel 6.6.93）、v2ray 5.40.0、nftables 1.1.1
> 相關：[讓整個系統走SOCKS5代理.md](../linux/讓整個系統走SOCKS5代理.md)、
> [clash-verge怎麼用-流量要先進得去核心.md](../linux/clash-verge怎麼用-流量要先進得去核心.md)

**一句話結論：TUN 模式解決的是「本機自己產生的流量」，而路由器上絕大多數流量是「幫別人轉發的」——
轉發流量根本不經過本機的 socket 層，改本機預設路由對它毫無作用。攔截轉發流量的標準機制是透明代理
（transparent proxy）的 TPROXY／REDIRECT。兩者同樣是 L4、程式同樣無感，差別只在流量是誰的。**

---

## 目錄

- [一、為什麼路由器不用 TUN](#一為什麼路由器不用-tun)
- [二、三條路的對照](#二三條路的對照)
- [三、本機實況檢查](#三本機實況檢查)
- [四、TPROXY 是怎麼運作的](#四tproxy-是怎麼運作的)
- [五、真的要在路由器上用 TUN 的話](#五真的要在路由器上用-tun-的話)
- [六、DNS 沒處理等於白做](#六dns-沒處理等於白做)
- [七、動手前的風險與救援](#七動手前的風險與救援)
- [自我測驗](#自我測驗)

---

## 一、為什麼路由器不用 TUN

關鍵在**封包在核心裡走的路不同**：

```
本機自己發起的連線（桌機的情境）
  程式 socket → 路由表查詢 → 出口網卡
                    ↑
              TUN 在這裡攔：把預設路由改指向虛擬網卡

幫別的裝置轉發的連線（路由器的情境）
  LAN 網卡 → PREROUTING → 路由判定「這不是給我的」→ FORWARD → NAT → WAN 網卡
                  ↑
          透明代理在這裡攔：TPROXY／REDIRECT
```

**轉發流量從頭到尾不會經過路由器的 socket 層**，也不會去查「本機該走哪個預設路由」——它只是被
核心從一張網卡搬到另一張網卡。所以：

- TUN 那套「建虛擬網卡 + 改預設路由」對轉發流量**沒有任何作用**。
- 你在路由器上開 TUN，只會攔到**路由器自己**發起的連線（opkg 更新、ntp、ping 等）。

**心法：問「這個流量是誰產生的」——自己產生的用 TUN，幫別人轉的用 TPROXY。**

---

## 二、三條路的對照

| 方案 | 機制 | UDP | 需要的核心模組 | 攔得到誰 |
|---|---|---|---|---|
| **A. TPROXY** | nftables `tproxy` 把封包交給 dokodemo-door，配合 fwmark 策略路由 | ✅ | `nft_tproxy`、`nft_socket` | **整個 LAN 所有裝置** |
| **B. REDIRECT** | nftables nat `redirect` 改寫目的埠 | ❌ 僅 TCP | `nft_redir` | 整個 LAN，但只有 TCP |
| **C. v2ray TUN** | `app/tun` 建虛擬網卡 | ✅ | 無（binary 內建） | **只有路由器自己**，不含 LAN 裝置 |

A 是 OpenWrt 上的標準作法；B 只適合快速驗證（UDP、QUIC、DNS 全部漏掉）；C 解決的是另一個問題。

**REDIRECT 為什麼不支援 UDP**：`REDIRECT` 屬於 NAT，它把目的位址改寫成本機後，原始目的地是靠
conntrack 記錄還原的。TCP 有連線狀態可以對應回去，UDP 是無連線的，代理程式拿不回「這個封包原本
要去哪」。TPROXY 則不改寫封包，而是在保留原始目的位址的情況下把封包**交付**給本機 socket——
這就是它能支援 UDP 的原因，也是它需要額外核心模組的原因。

---

## 三、本機實況檢查

路由器（192.168.1.1）唯讀檢查結果：

```bash
$ nft -v
nftables v1.1.1                          # ✅ 24.10 用 fw4 + nftables

$ ls /lib/modules/6.6.93/ | grep -iE 'tproxy|socket|redir'
nft_redir.ko                             # ⚠️ 只有 redir，沒有 nft_tproxy.ko

$ opkg list-installed | grep kmod-nft
kmod-nft-core / kmod-nft-fib / kmod-nft-nat / kmod-nft-offload
                                         # ❌ kmod-nft-tproxy 沒裝

$ netstat -lntup | grep v2ray
:::1080  tcp+udp     :::3080  tcp+udp    # SOCKS5
:::8880  tcp         :::8890  tcp        # HTTP proxy
:::12345 tcp+udp                         # ← TCP 與 UDP 都聽，dokodemo-door 的典型特徵

$ nft list ruleset | grep -iE 'tproxy|12345|meta mark'
(空)                                      # ❌ 沒有任何把流量導進 12345 的規則

$ ip rule show
0: from all lookup local / 32766: main / 32767: default
                                         # ❌ 沒有 fwmark 策略路由
```

**結論：透明代理的 inbound（12345）早就備好了，但核心模組沒裝、防火牆規則沒寫、策略路由沒設。
現在這台路由器純粹是個「顯式代理伺服器」——LAN 裝置必須自己把 proxy 指過來才會用到它。**

---

## 四、TPROXY 是怎麼運作的

三個零件缺一不可，這也是它比 REDIRECT 難設的原因：

### 1. 代理程式端：`IP_TRANSPARENT`

dokodemo-door 監聽時要設 `IP_TRANSPARENT` socket option，核心才允許它接收「目的地不是自己」
的封包。v2ray 的實作在 `transport/internet/sockopt_linux.go`（`IP_TRANSPARENT`、`SO_MARK`、
`IP_RECVORIGDSTADDR` 都在裡面），設定上對應 `streamSettings.sockopt.tproxy: "tproxy"`。

### 2. 策略路由：把打了標記的封包留在本機

```bash
ip rule add fwmark 1 lookup 100
ip route add local 0.0.0.0/0 dev lo table 100
```

`local 0.0.0.0/0 dev lo` 的意思是「這張表裡所有位址都算本機位址」。沒有這兩行，核心在
PREROUTING 之後會判定「這封包不是給我的」而直接轉發出去，TPROXY 規則根本輪不到。
**這是最常被漏掉的一步，也是「規則寫了卻沒作用」的頭號原因。**

### 3. nftables：打標記並交付

```
chain prerouting {
    type filter hook prerouting priority mangle;
    # 先排除不該被代理的目的地
    ip daddr { 127.0.0.0/8, 192.168.0.0/16, 10.0.0.0/8, 172.16.0.0/12, 224.0.0.0/4 } return
    # 其餘交給 12345，並打上 mark 1
    meta l4proto { tcp, udp } tproxy to 127.0.0.1:12345 meta mark set 1 accept
}
```

還要有一條 **output 鏈的排除規則**：v2ray 自己連往上游伺服器的封包若再被自己攔一次，就是無窮
迴圈。標準解法是 v2ray 出站帶 `SO_MARK`（`sockopt.mark`），nftables 看到那個 mark 就 return。

**三個零件的關係：nftables 負責「攔下來並打標記」，ip rule 負責「讓打了標記的封包留在本機」，
IP_TRANSPARENT 負責「讓程式收得到不是寄給自己的信」。**

---

## 五、真的要在路由器上用 TUN 的話

如果你的目的是讓**路由器自己**（不是 LAN 裝置）的流量走代理——例如 `opkg update`、路由器上的
其他服務——那 TUN 才是對的工具。v2ray v5 內建 `app/tun`（gVisor netstack），而且
`app/tun/device/gvisor/gvisor_linux.go` 的 build constraint 是 `linux && (amd64 || arm64)`，
**aarch64 的這台路由器編得出來**。

v5 JSON 設定長這樣：

```json
{
  "services": {
    "tun": { "name": "tun0", "mtu": 1500, "tag": "tun-in",
             "ips": [{"ip": "198.18.0.1", "prefix": 30}] }
  }
}
```

**但有個大坑**：`ips` 與 `routes` 是設給 gVisor 的**使用者空間協定堆疊**的，不是核心介面。
v2ray 只做 `open("/dev/net/tun")` + `TUNSETIFF`（需要 `CAP_NET_ADMIN`），整個 `app/tun/`
裡**沒有任何 netlink 或 `ip route` 呼叫**——核心側的 `ip addr add`、`ip link set up`、
路由改寫全部要你自己補，而且要記得排除上游伺服器位址，否則封包繞回 tun 形成迴圈、路由器直接失聯。

這也是為什麼 clash-verge 有個 `auto-route` 選項而 v2ray 沒有：clash 幫你做了核心側的路由設定，
v2ray 把這件事留給使用者。

---

## 六、DNS 沒處理等於白做

透明代理只攔 TCP／UDP 的連線，但 LAN 裝置**查 DNS 這一步**若還是走原本的路：

- 你的分流規則是寫域名的，卻拿不到正確解析結果；
- 被污染的 DNS 會讓你連到錯的 IP，代理再怎麼設都沒用。

OpenWrt 上常見的兩種處理：把 dnsmasq 的上游改指向 v2ray 的 DNS inbound，或用 nftables 把
53 埠的查詢一併 TPROXY 進去。無論哪種，**都要和代理規則一起規劃**，不能只做一半。

---

## 七、動手前的風險與救援

改路由器的防火牆與路由表，**失敗的後果是全家斷網**，而且你很可能正透過這台路由器連線。

| 原則 | 做法 |
|---|---|
| **先用非持久化規則測試** | 直接下 `nft add` / `ip rule add`，**不要**先寫進 `/etc/nftables.d/` 或 uci。搞砸了重開機就全部消失 |
| 保留一條後路 | 確認有實體網路孔或 Wi-Fi 能連上，或設一個排除規則讓自己的 IP 不走代理 |
| 分段驗證 | 先只對**單一測試裝置的 IP** 套規則，確認沒問題再擴大到整個網段 |
| 確認能救回來 | `/etc/init.d/firewall restart` 會重載 fw4 規則；`reboot` 清掉所有手動規則 |

**心法：在路由器上做實驗，第一件事不是「怎麼設定」，而是「設壞了怎麼回到現在這個狀態」。**

---

## OpenWrt 的服務管理：`/etc/init.d/<服務>` 的完整動詞

OpenWrt 用 procd 而不是 systemd，服務操作是**直接執行 init script 加上動詞**：

```
/etc/init.d/<服務> <動詞>
```

| 動詞 | 作用 |
|---|---|
| `start` / `stop` / `restart` | 啟動／停止／重啟 |
| `reload` | **重新載入設定**（服務沒實作 reload 就退回 restart） |
| `enable` / `disable` | **開機自動啟動的開關**（建立／移除 `/etc/rc.d/` 的符號連結） |
| `enabled` | **檢查是否設為開機啟動**（用離開碼判斷，適合腳本） |
| `running` | 檢查是否正在執行 |
| `status` | 顯示服務狀態 |
| `trace` | **用 syscall trace 啟動**（除錯服務起不來時很有用） |
| `info` | 倒出 procd 的服務資訊（JSON） |

> **`enable`／`enabled` 只差一個字母但意思完全不同**：前者是「設定成開機啟動」，
> 後者是「查詢是不是開機啟動」。寫腳本時很容易打錯。

也可以用 `service <服務> <動詞>`，效果相同。

## `fw4` 的完整子命令

| 子命令 | 作用 |
|---|---|
| `start` / `stop` / `restart` / `reload` | 啟動／停止／重啟／重載防火牆 |
| `flush` | **清空 fw4 的規則** |
| `reload-sets` | **只重載具名集合的內容**，不動規則本身 |
| `print` | **印出算繪後的完整 ruleset**（看 uci 會編譯成什麼，不套用） |
| `check` | **用 nft 的檢查模式測試 ruleset，不套用** ← 改設定後先跑這個 |
| `network <網段>` | 印出涵蓋該網段的 zone 名稱（找不到則離開碼 1） |

選項：`-v`（囉嗦）、`-q`（安靜）。

**`fw4 print` 與 `fw4 check` 是改防火牆前的兩道保險**——前者讓你先看產生物，
後者讓你在不影響現行規則的前提下驗證語法。

## 自我測驗

1. 為什麼「改本機預設路由」這招對路由器上的轉發流量沒有作用？從封包在核心裡經過的路徑說明。
2. REDIRECT 為什麼不能代理 UDP，而 TPROXY 可以？從「原始目的位址怎麼被還原」解釋兩者的差別。
3. TPROXY 三個零件中，`ip rule add fwmark 1 lookup 100` + `ip route add local 0.0.0.0/0 dev lo table 100`
   在做什麼？少了它，nftables 的 tproxy 規則會發生什麼事？
4. 代理程式自己連往上游伺服器的流量，為什麼一定要排除？通常用什麼機制排除？
5. 情境題：你想讓路由器**自己**的 `opkg update` 走代理，但 LAN 裝置維持直連。這該用本文的哪一種
   方案？為什麼另外兩種不適合？實作時最容易讓路由器失聯的是哪一步？
