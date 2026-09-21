# nftables 與 uci 分別是什麼？—— 核心的封包規則 vs OpenWrt 的設定抽象層

> 日期：2026-09-20
> 情境：研究 OpenWrt 透明代理時一直碰到這兩個詞，想搞清楚它們各自是什麼、又是什麼關係
> 對應檔案：`/etc/config/*`（uci 設定，路由器上的正式設定）、`/etc/nftables.d/*.nft`
> 適用範圍：OpenWrt 24.10.2、nftables 1.1.1、firewall4；nftables 的部分適用於任何 Linux
> 相關：[LuCI能設定TPROXY嗎-fw4的uci抽象邊界.md](./LuCI能設定TPROXY嗎-fw4的uci抽象邊界.md)、
> [路由器上的透明代理-為什麼不是TUN模式.md](./路由器上的透明代理-為什麼不是TUN模式.md)

**一句話結論：兩者根本不是同類東西。nftables 是 Linux **核心**的封包過濾框架，任何 Linux 都有，
跟 OpenWrt 無關；uci 是 **OpenWrt 自己**發明的設定抽象層，管的是全系統所有服務，跟封包無關。
它們在 OpenWrt 上是上下游關係：你編輯 uci，fw4 把它**編譯**成 nftables 規則餵給核心——
uci 是「來源碼」，nft 規則是「產生物」。**

---

## 一、nftables：核心怎麼處理封包

### 它在哪一層

```
使用者空間    nft 指令 / fw4 / 你寫的 .nft 檔
                 │ netlink
核心          nf_tables 子系統 ── 規則被編譯成 bytecode，在核心的小型虛擬機（VM）裡執行
                 │
              netfilter hooks ── 封包經過網路堆疊時的五個攔截點
```

**nftables 是 iptables 的後繼者**。以前要用四套工具（`iptables`、`ip6tables`、`arptables`、
`ebtables`）分別處理 IPv4／IPv6／ARP／bridge，現在統一成一個 `nft`。

### 三層結構

```
table  <family> <name>          家族：ip / ip6 / inet（雙棧）/ arp / bridge / netdev
  └─ chain <name>               掛在哪個 hook、什麼 type、什麼 priority
       └─ rule                  比對條件（expression）+ 動作（statement）
```

本機路由器只有一個 table：

```bash
$ nft list tables
table inet fw4              # inet = IPv4 與 IPv6 共用一套規則
```

chain 的宣告決定了它**什麼時候被呼叫**：

```
chain input {
    type filter hook input priority filter; policy drop;
    ...
}
```

- `hook`：五個攔截點之一——`prerouting`、`input`、`forward`、`output`、`postrouting`。
  這正好對應[上一篇](./路由器上的透明代理-為什麼不是TUN模式.md)講的封包路徑：
  轉發流量走 `prerouting → forward → postrouting`，本機流量走 `input` 或 `output`。
- `type`：`filter`（過濾）、`nat`（位址轉換）、`route`（改了封包後重新查路由）。
- `priority`：同一個 hook 上多條 chain 的執行順序，數字小的先跑。`mangle`(-150)、
  `dstnat`(-100)、`filter`(0)、`srcnat`(100) 都是有名字的常數。

**TPROXY 之所以要寫 `priority mangle`，就是因為它必須在路由判定「這封包不是給我的」之前動手。**

### 比 iptables 好在哪

| | iptables | nftables |
|---|---|---|
| 工具數量 | 4 套（v4/v6/arp/bridge） | 1 套 `nft` |
| 更新方式 | 一條一條加，中途可能是半套狀態 | **整份規則集原子更新**（要嘛全生效、要嘛全不變） |
| 集合 | 需要 ipset 這個外掛 | 內建具名 set／map／vmap |
| 鏈與優先權 | 表與鏈固定（filter/nat/mangle…） | **自己宣告**要哪個 hook、什麼優先權 |

本機規則裡就能看到 vmap（verdict map，判決對應表）這種 iptables 做不到的寫法：

```
ct state vmap { established : accept, related : accept }
```

`tproxy`、`redirect`、`masquerade` 這些都是 **statement**（動作），各自需要對應的核心模組
——這就是為什麼[前一篇](./路由器上的透明代理-為什麼不是TUN模式.md)查到「沒有 `nft_tproxy.ko`
就不能用 tproxy」。

---

## 二、uci：OpenWrt 怎麼描述「設定」

**UCI（Unified Configuration Interface，統一設定介面）** 是 OpenWrt 特有的東西，其他 Linux 發行版
沒有。它解決的問題是：一台路由器上有 dnsmasq、防火牆、網路介面、無線、DHCP……**每個服務的設定檔
格式都不一樣**，沒辦法用同一套 GUI 或腳本管理。

uci 的作法是：**所有設定都先寫成同一種格式，再由各服務的啟動腳本翻譯成該服務看得懂的格式。**

### 統一的檔案格式

`/etc/config/firewall` 實際內容：

```
config defaults
	option input 'REJECT'
	option output 'ACCEPT'
	option forward 'REJECT'

config zone
	option name 'lan'
	option input 'ACCEPT'
	list network 'lan'
```

- `config <type> ['<name>']`：一個區段（section）。沒給名字的叫**匿名區段**。
- `option`：單一值；`list`：可重複的多值。

### 統一的存取介面

同一份設定用 `uci` 指令看到的樣子：

```bash
$ uci show firewall
firewall.@defaults[0].input='REJECT'
firewall.@zone[0]=zone
firewall.@zone[0].name='lan'
firewall.@zone[0].input='ACCEPT'
```

`@zone[0]` 就是「firewall 這個設定檔裡第 0 個匿名的 zone 區段」——匿名區段靠**索引**定位，
所以**插入或刪除區段會讓後面的索引全部位移**，寫腳本時要小心。

常用操作：

```bash
uci show firewall                      # 看全部
uci get firewall.@zone[0].input        # 取單一值
uci set firewall.@zone[0].input='DROP' # 改（只寫進暫存，尚未生效）
uci add_list firewall.@zone[0].network='vpn'
uci commit firewall                    # 寫回 /etc/config/firewall
/etc/init.d/firewall restart           # 讓服務重新讀取
```

**`uci set` 之後不 `commit`，改動只存在 `/tmp/.uci/` 的暫存檔裡。** 這是 uci 的交易機制：
可以改一堆再一次套用，但也常見「改了沒生效」其實是忘了 commit。

### 誰在讀 uci

LuCI（Lua Configuration Interface，就是那個 Web GUI）、`uci` 命令列、以及各服務的 init script，
三者讀的是同一份資料——**這就是為什麼 GUI 改的東西命令列看得到、反之亦然**。

---

## 三、兩者怎麼串起來

本機實測，同一份設定的兩個樣貌：

```bash
$ wc -l /etc/config/firewall          # uci 來源
（約 30 行人看的設定）

$ nft list ruleset | wc -l            # fw4 產生的結果
180
```

對照著看，uci 的 `zone` 概念在 nft 裡被展開成**一整組 chain**：

```
uci:  config zone / option name 'lan' / option input 'ACCEPT'
        ↓ fw4 編譯
nft:  chain input {
          iifname "br-lan" jump input_lan   comment "!fw4: Handle lan IPv4/IPv6 input traffic"
      }
      chain input_lan { ... }
      chain forward_lan { ... }
      chain output_lan { ... }
      chain accept_from_lan { ... }
```

那句 `comment "!fw4: ..."` 就是產生物的標記——**看到 `!fw4:` 就知道這條規則是 fw4 從 uci 生出來的，
不是人手寫的。**

完整的資料流：

```
LuCI (Web GUI) ─┐
uci 命令        ─┼──→ /etc/config/firewall ──fw4──→ nft 規則 ──netlink──→ 核心 nf_tables
init script     ─┘        （uci 格式）              （nft 語法）
```

---

## 四、最重要的推論：不要手改產生物

`nft add rule ...` 當場下的規則，或直接改 fw4 生出來的東西，**下一次 `/etc/init.d/firewall restart`
或任何人在 LuCI 上按儲存，就會被整份覆蓋**——因為 fw4 是從 uci 重新編譯整個 ruleset。

這和[桌機那邊的教訓](../linux/clash-verge怎麼用-流量要先進得去核心.md)是同一回事：
clash-verge 的 `clash-verge.yaml` 也是產生物，要改得去改 Merge 設定。

**通用心法：拿到一個設定檔，先問「這是人寫的還是程式生的」。是產生物就往上游找來源，
在產生物上動手的修改都活不過下一次重新產生。**

所以在 OpenWrt 上要持久化自訂 nft 規則，正確位置是 `/etc/nftables.d/*.nft`——fw4 每次產生
ruleset 時都會把它 include 進去，等於「參與編譯」而不是「事後修改」。

---

## 五、什麼時候用哪一層

| 你要做的事 | 該用 |
|---|---|
| 開放某個埠、設 port forward、改 zone 策略 | **uci / LuCI**（schema 涵蓋的範圍） |
| TPROXY、自訂 hook 與 priority、fwmark | **直接寫 nft**，放 `/etc/nftables.d/*.nft` |
| 改 DHCP、無線、網路介面 | **uci**（這些沒有「另一層」可繞） |
| 臨時測試規則（不想持久化） | `nft add ...`，重開機或 firewall restart 就消失 |

---

## 六、速查

```bash
# --- nftables ---
nft list ruleset                 # 全部規則（fw4 產生的會帶 "!fw4:" 註解）
nft list tables                  # 有哪些 table
nft list table inet fw4          # 單一 table
nft -a list ruleset              # 加上 handle 編號（刪除規則時要用）
nft add rule inet fw4 <chain> <rule>       # 臨時加（不持久）
nft delete rule inet fw4 <chain> handle N

# --- uci ---
uci show                         # 全部設定
uci show firewall                # 單一設定檔
uci get firewall.@zone[0].name
uci set ... ; uci commit firewall ; /etc/init.d/firewall restart
uci changes                      # 看有哪些改動還沒 commit
uci revert firewall              # 丟棄未 commit 的改動
```

---

## uci 完整命令與選項

```
uci [<選項>] <命令> [<參數>]
```

### 命令（全部 15 個）

**讀**

| 命令 | 作用 |
|---|---|
| `show [<config>[.<section>[.<option>]]]` | **顯示設定**（可逐層縮小範圍） |
| `get <config>.<section>[.<option>]` | 取單一值 |
| `export [<config>]` | 匯出成 uci 檔案格式 |
| `changes [<config>]` | **顯示尚未 commit 的改動** ← 最該記住的一個 |

**寫（都只寫進暫存，要 commit 才生效）**

| 命令 | 作用 |
|---|---|
| `set <config>.<section>[.<option>]=<值>` | 設定值 |
| `add <config> <section-type>` | **新增一個匿名 section**（回傳它的名字） |
| `add_list <config>.<section>.<option>=<字串>` | 往 list 加一項 |
| `del_list <config>.<section>.<option>=<字串>` | 從 list 移除一項 |
| `delete <config>[.<section>[[.<option>][=<id>]]]` | 刪除 |
| `rename <config>.<section>[.<option>]=<名稱>` | **把匿名 section 命名**（之後就能用名字取代索引） |
| `reorder <config>.<section>=<位置>` | 改變 section 的順序 |

**交易**

| 命令 | 作用 |
|---|---|
| `commit [<config>]` | **寫回 `/etc/config/`** |
| `revert <config>[.<section>[.<option>]]` | **丟棄未 commit 的改動** |
| `batch` | 從 stdin 讀多個命令一次執行 |
| `import [<config>]` | 從 uci 格式匯入 |

`rename` 特別值得知道：**把匿名 section 命名之後就能寫 `firewall.myzone.input='ACCEPT'`，
不必再用會位移的 `@zone[1]`**——腳本可靠度差很多。

### 選項（全部 13 個）

| 選項 | 作用 |
|---|---|
| `-c <路徑>` | 設定檔搜尋路徑（預設 `/etc/config`） |
| `-C <路徑>` | 覆寫檔搜尋路徑（預設 `/var/run/uci`） |
| `-d <字串>` | `uci show` 時 list 值的分隔符 |
| `-f <檔案>` | 用檔案取代 stdin |
| `-m` | import 時**合併**進既有套件 |
| `-n` | export 時替匿名 section 命名（預設） |
| `-N` | export 時不替匿名 section 命名 |
| `-p <路徑>` / `-P <路徑>` | 追加改動檔搜尋路徑（`-P` 並設為預設） |
| `-t <路徑>` | 改動檔的儲存路徑 |
| `-q` | 安靜模式（不印錯誤） |
| `-s` / `-S` | 強制／關閉嚴格模式（遇到解析錯誤是否停止；預設 `-s`） |
| `-X` | `show` 時不使用延伸語法 |

## 自我測驗

1. nftables 和 uci 為什麼不能拿來比較「哪個比較好」？各自屬於哪一層、管的是什麼？
2. 你在 LuCI 上按了儲存，結果先前用 `nft add rule` 加的規則不見了。為什麼？要怎麼加才不會被洗掉？
3. chain 宣告裡的 `hook` 和 `priority` 各自決定什麼？為什麼 TPROXY 的 chain 一定要用
   `priority mangle` 而不能用 `filter`？
4. `uci set` 改完之後為什麼可能「看起來沒生效」？uci 的交易機制是怎麼運作的？
5. 情境題：你在別人的 OpenWrt 上看到一條你沒印象的 nft 規則，想知道它從哪來、該去哪裡改。
   有哪兩個線索可以判斷它是人手寫的還是某個程式產生的？
