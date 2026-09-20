# LuCI 能用 GUI 設定 TPROXY 嗎？—— fw4 的 uci 抽象邊界

> 日期：2026-09-20
> 情境：想在 OpenWrt 上做透明代理，問 LuCI 的防火牆頁面能不能直接設 TPROXY
> 對應檔案：`/etc/config/firewall`、`/etc/nftables.d/*.nft`、`/etc/firewall.user`
> （皆為路由器上的正式設定，不在本 repo 內）
> 適用範圍：OpenWrt 24.10.2、firewall4（fw4）+ nftables 1.1.1、luci-app-firewall 25.168
> 延伸自：[路由器上的透明代理-為什麼不是TUN模式.md](./路由器上的透明代理-為什麼不是TUN模式.md)

**一句話結論：不行。LuCI 不是「nftables 的圖形介面」，而是「uci 設定檔的圖形介面」——它只做得到
fw4 的 uci schema 定義過的事（zone、port forward、SNAT/DNAT、ipset）。TPROXY 需要的 mangle hook、
fwmark、`ip rule` 這三樣 schema 裡根本沒有，所以不是「沒人寫 UI」，是這條路走不通。要 GUI 就得裝
現成的代理套件，代價是交出整套防火牆與 DNS 的控制權。**

---

## 一、LuCI 的防火牆頁面實際有什麼

本機 `luci-app-firewall` 提供的分頁（`/www/luci-static/resources/view/firewall/`）：

```
zones.js      區域與轉發策略（lan/wan、forward/input/output）
forwards.js   通訊埠轉發（DNAT）
rules.js      流量規則（accept/drop/reject）
snats.js      來源 NAT
ipsets.js     具名集合
custom.js     自訂規則（見下一節）
```

這六個對應的就是 `/etc/config/firewall` 裡的 `config zone`／`redirect`／`rule`／`nat`／`ipset`。
**GUI 是 uci schema 的封裝，schema 沒有的東西，GUI 就不可能有。**

TPROXY 需要的三樣東西，沒有一樣在 schema 裡：

| 需要的東西 | uci firewall 有嗎 |
|---|---|
| `type filter hook prerouting priority mangle` 的自訂鏈 | ❌ fw4 的鏈都固定在 filter/nat，優先權寫死 |
| `tproxy to 127.0.0.1:12345` 語句 | ❌ |
| `meta mark set 1` + `ip rule add fwmark 1 lookup 100` | ❌ 而且 `ip rule` 屬於路由，根本不歸防火牆管 |

第三點特別值得記住：**`ip rule`／`ip route` 不是防火牆的東西**。就算 fw4 哪天支援了 tproxy 語句，
策略路由那一半仍然要另外處理——這也是為什麼透明代理沒辦法只靠一個設定頁搞定。

---

## 二、「自訂規則」分頁是個陷阱

你的 LuCI 有 `custom.js`，所以看得到「自訂規則（Custom Rules）」分頁。但實測它在 fw4 下已經是**死的**：

```bash
$ grep -o '/etc/[a-z0-9./_-]*' custom.js
/etc/firewall.user            # ← 它只是把文字框內容存到這個檔
/etc/init.d/firewall

$ grep -o '_(.\{20,160\}' custom.js
'Custom rules allow you to execute arbitrary iptables commands ...'
                              # ← 說明文字還停留在 iptables 時代

$ ls -l /etc/firewall.user
No such file or directory     # ← 檔案根本不存在

$ uci show firewall | grep include
(空)                          # ← 也沒有 include 設定
```

**fw4 不會自動執行 `/etc/firewall.user`。** 這個檔案是 iptables 時代 `firewall3` 的遺產；換成
nftables 後，要讓它復活必須自己補一段：

```
config include
    option type 'script'
    option path '/etc/firewall.user'
    option fw4_compatible '1'
```

沒補這段，你在 GUI 上打字、按儲存、看到「內容已儲存」——**然後什麼都不會發生**。這是很容易
浪費半小時的陷阱。

而且就算補了，你在那個文字框裡寫的還是 shell 指令，不是「用 GUI 設定」——只是換個地方寫指令。

---

## 三、fw4 時代正確的持久化入口：`/etc/nftables.d/`

你的系統已經有這個目錄，README 說得很清楚：

```
All *.nft files in this directory are included by the firewall4 ruleset
within the inet/fw4 table context which allows referencing named sets
declared and populated by the firewall configuration.
```

兩個重點：

1. 這些檔案被**包進 `inet fw4` table 的內容裡**，所以你可以在裡面宣告自己的鏈，包括 fw4 自己
   沒有的 hook 與優先權。
2. 因為在同一個 table 裡，可以直接引用 fw4 建立的具名 set（例如 LuCI 上設的 ipset），這是
   `/etc/firewall.user` 那種外掛 script 做不到的。

既有範例檔 `10-custom-filter-chains.nft` 示範了怎麼在預設鏈之前插隊：

```
# chain user_pre_input {
#     type filter hook input priority -1; policy accept;
# }
```

**所以持久化 TPROXY 規則的正確位置是這裡，不是 `/etc/firewall.user`。** 但 `ip rule`／`ip route`
那一半還是得靠 init script 或 hotplug，`/etc/nftables.d/` 只管 nftables。

---

## 四、真的想要 GUI：裝現成的代理套件

有完整透明代理 GUI 的是這些（都不在 OpenWrt 官方 feed，需要加第三方 feed 或下載 ipk）：

| 套件 | 核心 | 特點 |
|---|---|---|
| `luci-app-homeproxy` | sing-box | ImmortalWrt 主推，設定乾淨 |
| `luci-app-passwall` | 多核心（v2ray/xray/ss/…） | 功能最多，也最肥 |
| `luci-app-nikki` | mihomo | 和桌機的 clash 同核心，規則寫法通用 |
| `luci-app-openclash` | mihomo/clash | 老牌，資源吃得兇 |

它們才有「透明代理模式：TPROXY / REDIRECT / TUN」的下拉選單、分流規則編輯器、DNS 設定的完整 UI。

**代價要想清楚**：這類套件會**自己接管一整套 nft 規則、策略路由與 DNS 設定**（通常還會改
dnsmasq）。你現在那套手寫的 `/root/v2ray` + 自寫 `/etc/init.d/v2ray` 會和它打架——同一時間只能
有一套在管透明代理，所以實務上是二選一，不是並存。

本機狀況：

```bash
$ opkg list-installed | grep '^luci-app'
luci-app-firewall / luci-app-openvpn / luci-app-package-manager   # 沒有任何代理套件

$ df -h | grep overlay
overlayfs:/overlay   7.2G   491.1M   6.8G   7%   /      # ✅ 空間非常充裕

$ ls /var/opkg-lists/
(空)                                  # ⚠️ 套件清單快取是空的，要先 opkg update 才查得到
```

---

## 五、所以你的選擇是

| 路線 | 怎麼做 | 有 GUI 嗎 | 和現有架構 |
|---|---|---|---|
| **維持手寫** | 規則寫進 `/etc/nftables.d/*.nft`，`ip rule` 用 init script | ❌ | ✅ 沿用現有的 v2ray 與 12345 inbound |
| **改用套件** | 裝 `luci-app-homeproxy` 等，讓它全權管理 | ✅ 完整 | ❌ 要放棄手寫那套 |

沒有第三條「用 LuCI 原生 GUI 設 TPROXY」的路。

**心法：GUI 的能力邊界等於它背後設定檔 schema 的邊界。看到「這個 GUI 為什麼沒有某功能」，
先去看它到底在編輯什麼檔案——答案通常不是「作者沒做」，而是「那個檔案沒有這個欄位」。**

---

## 自我測驗

1. 為什麼 LuCI 的防火牆頁面做不到 TPROXY？從「LuCI 到底是什麼的圖形介面」回答，而不是「功能沒做」。
2. 你在 LuCI 的「自訂規則」分頁貼了一段規則、按儲存、看到「內容已儲存」，但規則完全沒生效。
   在 fw4 的系統上最可能的原因是什麼？要怎麼確認？
3. `/etc/nftables.d/*.nft` 和 `/etc/firewall.user` 都能放自訂規則，前者有哪兩個後者沒有的好處？
4. 情境題：某個 LuCI 套件宣稱能一鍵開啟透明代理。裝上去之後，你原本手寫的 v2ray 透明代理就失效了。
   最可能是哪三類設定被它接管了？如果想保留手寫那套，該檢查什麼？
