# nft 完整用法 —— 從命令文法到規則語法

> 日期：2026-09-21
> 情境：要能自己寫 nft 規則——既要知道 `add`／`delete`／`list` 這些命令怎麼下，也要看懂
> `type filter hook forward priority filter; policy drop;` 裡每個字與標點的角色
> 對應檔案：路由器上的 `inet fw4` 規則集（fw4 從 `/etc/config/firewall` 編譯產生）
> 適用範圍：nftables 語法本身適用於任何 Linux；文中範例取自 OpenWrt 24.10.2 的 fw4 規則集（nftables 1.1.1）
> 相關：[iptables與nftables怎麼用.md](./iptables與nftables怎麼用.md)（兩代工具的對照與 iptables 用法）、
> [讀懂fw4的nft規則.md](../openwrt/讀懂fw4的nft規則.md)（範例規則集的整體骨架在那篇）

**一句話結論：nft 分成兩層——**命令層**（`nft <動詞> <物件> <識別> [規格]`，所有操作都是這個
形狀）與**規則層**（一串「比對」後面接一串「動作」）。把這兩層的文法各記一次，就不必再靠背例子。
規則層只有兩種東西——**宣告**（table／chain，用 `{}` 包住內容、用 `;` 結束每個
子句）和**規則**（由左到右：一串「比對」後面接一串「動作」，沒有 `-j` 這種旗標）。看不懂的時候，
先問「這個 token 是在描述封包長什麼樣，還是在描述要對它做什麼」，答案只有這兩類。**

---

## 一、命令的統一文法

**所有 nft 操作都是同一個形狀**，記住這個骨架就不必背個別指令：

```
nft [選項] <動詞> <物件型別> <識別> [規格]
```

先用一個完整例子把四個位置標出來：

```
nft delete rule inet myfilter input handle 7
    └──┬─┘ └┬─┘ └───────┬──────┘ └───┬───┘
     動詞  物件型別       識別         規格
                     ┌────┴─────┐
                  inet myfilter input
                    ①     ②       ③
```

| 位置 | 這個例子裡是 | 說明 |
|---|---|---|
| 動詞 | `delete` | 要做什麼 |
| 物件型別 | `rule` | 對什麼東西做 |
| 識別 | `inet myfilter input` | **指到哪一個**——見下 |
| 規格 | `handle 7` | 細節（哪一條規則／規則內容／鏈的屬性） |

### `<識別>`：一串由外而內的位置參數

識別**不是一個值，是一串**，從最外層一路指到目標：

```
①  inet       ← family（協定家族）：固定關鍵字，只能是 ip / ip6 / inet / arp / bridge / netdev
②  myfilter   ← table 的名字：你自己取的
③  input      ← chain 的名字：你自己取的（不是 hook，見〈鏈名 vs hook〉）
```

**要寫幾段，取決於物件型別：**

| 物件型別 | 識別的組成 | 例子 |
|---|---|---|
| `table` | family + 表名（**2 段**） | `nft list table inet myfilter` |
| `chain` | family + 表名 + 鏈名（**3 段**） | `nft list chain inet myfilter input` |
| `rule` | family + 表名 + 鏈名（**3 段**）＋規格 | `nft delete rule inet myfilter input handle 7` |
| `set` / `map` | family + 表名 + 集合名（**3 段**） | `nft list set inet myfilter blacklist` |
| `element` | 同 set，再加 `{ 元素 }` | `nft add element inet myfilter blacklist '{ 1.2.3.4 }'` |
| `ruleset` | **不需要識別** | `nft list ruleset` |

**規律：識別就是「這個物件的完整路徑」**——rule 住在 chain 裡、chain 住在 table 裡、table 屬於某個
family，所以要從 family 開始一路寫下來。就像檔案路徑 `/家族/表/鏈`。

### 為什麼 family 一定要寫

**表是用「family + 名字」共同識別的**，不是只有名字。man page 寫得很直接：

> Tables are ... identified by their address family and their name.

所以同一個名字可以在不同 family 各存在一張表，互不衝突。

**省略 family 不是「自動幫你找」，而是預設成 `ip`：**

> When no address family is specified, ip is used by default.

實測（你的 fw4 表在 `inet` family）：

```bash
$ nft list table fw4
Error: No such file or directory          # ← 被當成 ip fw4 去找

$ nft list table ip fw4
Error: No such file or directory          # ← 一模一樣的錯誤，證明上一行就是這個意思

$ nft list table inet fw4
table inet fw4 { ... }                    # ✅
```

### 兩種錯誤訊息代表不同的問題

```bash
$ nft list table ip fw4
Error: No such file or directory          # ← 語意錯誤：段數對，但指到不存在的東西

$ nft list chain inet input
Error: syntax error, unexpected newline, expecting string
                                          # ← 語法錯誤：少寫一段（漏了 table 名）
```

**看到 `No such file or directory` → 檢查 family／名字有沒有打錯；
看到 `syntax error` → 檢查段數夠不夠。**

### 識別怎麼查

```bash
$ nft list tables
table inet fw4          # ← 直接給你「family + 表名」兩段，照抄即可

$ nft list table inet fw4 | grep 'chain '
        chain input {   # ← 第三段（鏈名）在這裡
        chain forward {
```

### 規格：每種物件不一樣

| 情況 | 規格是什麼 |
|---|---|
| 建 base chain | `'{ type filter hook input priority 0; policy drop; }'`（**要單引號**，因為含 `{};`） |
| 建 regular chain | 不給規格 |
| 加 rule | 規則本身：`tcp dport 22 accept` |
| 刪 rule | `handle <N>` |
| 插 rule 到指定位置 | `position <handle>` 或 `index <N>` + 規則內容 |
| 建 set | `'{ type ipv4_addr; flags interval; }'` |

### 動詞：做什麼（完整 15 個）

| 動詞 | 意思 | 適用物件 |
|---|---|---|
| `add` | 新增；**已存在也不報錯** | 幾乎全部 |
| `create` | 新增；**已存在就報錯** | table／chain／set／map／element／flowtable |
| `insert` | 加 rule 到**最前面**或指定位置 | **只有 rule** |
| `replace` | **取代**指定 handle 的 rule | **只有 rule** |
| `delete` | 刪除 | 幾乎全部 |
| `destroy` | 刪除；**不存在也不報錯** | 幾乎全部（較新版本才有） |
| `get` | **取出集合裡的元素** | **只有 element** |
| `list` | 列出 | 幾乎全部 |
| `flush` | **清空內容但保留物件** | ruleset／table／chain／set／map |
| `reset` | **把狀態值歸零** | counter／quota／rule／rules／set／map |
| `rename` | 改名 | **只有 chain** |
| `import` | 從 JSON／二進位匯入 | ruleset |
| `export` | 匯出成 JSON／二進位 | ruleset |
| `monitor` | 即時顯示規則變動／封包追蹤 | — |
| `describe` | **查詢某欄位有哪些可用值** | — |

前一版我只列了 12 個，漏掉 `get`／`import`／`export`，而且把 `reset` 講得太窄——
**`reset rule`／`reset rules` 可以把規則的 counter 歸零而不用重建規則**，排查時很實用。

**`add` / `create` / `insert` 最常被搞混：**

```bash
nft add    table inet t            # 已存在 → 安靜成功（冪等，腳本愛用）
nft create table inet t            # 已存在 → 報錯 File exists
nft add    rule inet t c accept    # 接到鏈的最後
nft insert rule inet t c accept    # 插到鏈的最前
```

**`delete` / `destroy` / `flush` 的差別也要分清楚：**

```bash
nft delete  chain inet t c    # 刪掉這條鏈；不存在 → 報錯
nft destroy chain inet t c    # 刪掉這條鏈；不存在 → 安靜成功（腳本用）
nft flush   chain inet t c    # 鏈留著，只把裡面的規則清空
```

### 物件型別：對什麼做（完整）

| 分類 | 物件型別 |
|---|---|
| **三層結構** | `table`、`chain`、`rule` |
| **集合類** | `set`、`map`、`element`（集合裡的成員） |
| **具名狀態物件** | `counter`、`quota`、`limit`、`secmark`、`synproxy` |
| **conntrack 物件** | `ct helper`、`ct timeout`、`ct expectation` |
| **其他** | `flowtable`（硬體／軟體加速）、`meter`、`ruleset`（整份） |

**複數形只能配 `list`**，用來「列出這一類的全部」：

```bash
nft list tables          # 所有表（只列名字，不列內容）
nft list chains
nft list sets
nft list maps
nft list counters
nft list flowtables
nft list hooks           # 目前有哪些 hook 被註冊（含其他子系統的）
nft list ct helpers
```

`nft list tables` 特別常用——**它直接給你「family + 表名」，就是下一個指令要填的前兩段識別**。

### 選項：完整清單

`nft --help` 把選項分成四組：

**輸入處理**

| 選項 | 作用 |
|---|---|
| **`-f <檔>`** | 從檔案載入，**整份原子套用** |
| **`-c`** | **只檢查語法，不套用**（dry run），遠端改規則的保命符 |
| `-i` | 互動式 CLI（有自動補齊） |
| `-D name=value` | 定義變數，例如 `--define wan=eth0`，檔案裡用 `$wan` |
| `-I <目錄>` | `include` 的搜尋路徑 |
| `-o` | 自動最佳化規則集 |

**列出時的格式**

| 選項 | 作用 |
|---|---|
| **`-a`** | **顯示 handle**——刪 rule 前一定要先用它 |
| `-s` | 不印 counter 等狀態值（**適合存成設定檔**） |
| `-t` | 不展開集合內容（輸出較短） |
| `-n` | 全部用數字，不做名稱轉換 |
| `-y` | priority 印成數字而非名字（`0` 而非 `filter`） |
| `-S` | 埠號翻成 `/etc/services` 的服務名 |
| `-N` | IP 做反向 DNS 查詢（**會很慢，理由同 netstat**） |
| `-u` | UID／GID 翻成使用者名稱 |
| `-p` | 第四層協定印成數字 |
| `-T` | 時間值印成數字 |

**輸出格式**

| 選項 | 作用 |
|---|---|
| `-e` | 執行後把剛加的東西印出來（確認用） |
| `-j` | JSON 輸出（給腳本用） |
| `-d <level>` | 除錯輸出（`scanner`／`parser`／`netlink`／`all`…） |

**一般**：`-h` 說明、`-v` 版本、`-V` 詳細版本。

### family：只有這六個

`ip`、`ip6`、**`inet`**、`arp`、`bridge`、`netdev`。

---

---

## 二、規則的增刪改查

實務上最常做、也最容易卡住的地方——**因為刪 rule 不能用行號，只能用 handle**。

```bash
# 查：先拿到 handle（沒有 -a 就看不到）
nft -a list chain inet myfilter input
#   chain input { # handle 2
#       tcp dport 22 accept # handle 5     ← 這個 5 就是 handle
#   }

# 增：接到最後
nft add rule inet myfilter input tcp dport 80 accept

# 增：插到最前面
nft insert rule inet myfilter input iif lo accept

# 增：插在某條規則之前（position = 那條的 handle）
nft insert rule inet myfilter input position 5 tcp dport 443 accept

# 改：取代 handle 5 那條
nft replace rule inet myfilter input handle 5 tcp dport 22 counter accept

# 刪：只能用 handle
nft delete rule inet myfilter input handle 5

# 清空整條鏈的規則（鏈還在）
nft flush chain inet myfilter input
```

**handle 是核心配發的、不會重排**——刪掉中間某條，其他規則的 handle 不變。
這比 iptables 的行號可靠：iptables 刪掉第 3 條之後，原本第 4 條就變成第 3 條了。

> `position <handle>` 是「插在**這個 handle 的規則**之前」；`index <N>` 是「插在**第 N 條**之前」
> （從 0 算）。前者穩定，後者直覺。

---

## 三、表與鏈的操作

```bash
nft add table inet myfilter                    # 建表
nft list tables                                # 有哪些表

nft add chain inet myfilter input \
    '{ type filter hook input priority 0; policy drop; }'   # base chain（規格要用單引號）
nft add chain inet myfilter mysub               # regular chain（不給規格）
nft rename chain inet myfilter mysub newname
nft list chain inet myfilter input              # ★ 只看一條鏈，不用捲整份

nft flush chain inet myfilter input             # 清規則，留鏈
nft delete chain inet myfilter mysub            # 刪鏈（要先沒規則、也沒人 jump 它）
nft delete table inet myfilter                  # 刪整張表（連同裡面全部）
nft flush ruleset                               # ☠ 清空整台機器的所有規則
```

> `flush ruleset` 會把**別的程式建立的規則一起清掉**（Docker、fw4、libvirt）。
> 在有 Docker 的機器上執行 = 容器網路立刻壞掉。

---

## 四、查詢：`list`、`describe`、`monitor`

```bash
nft list ruleset                   # 全部
nft list tables                    # 只列表名
nft list table inet fw4            # 單一表
nft list chain inet fw4 input      # ★ 單一鏈，最常用
nft -a list ruleset                # 帶 handle
nft -s list ruleset > backup.nft   # 不含 counter，適合存檔
```

### `describe`：查「這個欄位能填什麼」

**nft 唯一的線上查詢工具，而且不需要 root**，背不起來時最快的救援：

```bash
$ nft describe ct state
ct expression, datatype ct_state (conntrack state) (basetype bitmask, integer), 32 bits
pre-defined symbolic constants (in hexadecimal):
        invalid        0x00000001
        new            0x00000008
        established    0x00000002
        ...

$ nft describe tcp flags
payload expression, datatype tcp_flag (TCP flag) (basetype bitmask, integer), 8 bits
        fin            0x01
        ...
```

不確定 `ct state` 有哪些值、`icmp type` 能寫什麼時，問它比翻文件快。

### `monitor`：即時看發生什麼事

```bash
nft monitor                        # 有人改規則就印出來（看誰在動你的規則）
nft monitor trace                  # 封包追蹤（要先加一條 meta nftrace set 1 的規則）
```

---

## 五、集合、對應與判決表

```bash
# 匿名集合：寫在規則裡，不能事後修改
nft add rule inet t c tcp dport { 22, 80, 443 } accept

# 具名集合：可動態增刪，規則不用改
nft add set inet t blacklist '{ type ipv4_addr; flags timeout; }'
nft add element inet t blacklist '{ 1.2.3.4 timeout 1h }'
nft add rule inet t c ip saddr @blacklist drop
nft delete element inet t blacklist '{ 1.2.3.4 }'
nft list set inet t blacklist

# 要放網段就加 flags interval
nft add set inet t nets '{ type ipv4_addr; flags interval; }'
nft add element inet t nets '{ 10.0.0.0/8, 192.168.0.0/16 }'

# vmap：查表決定動作
nft add rule inet t c ct state vmap { established : accept, invalid : drop }
```

**具名集合的價值在於「改內容不用改規則」**——動態封鎖 IP、白名單管理都靠它，
而且比對是雜湊／區間查找，比一堆規則線性比對快得多。

---

## 六、檔案模式：`nft -f`（強烈建議）

```
#!/usr/sbin/nft -f
flush ruleset                      # 先清空，確保結果可重現

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
nft -c -f rules.nft                # ★ 先檢查語法
nft -f rules.nft                   # 套用（原子操作）
```

**`nft -f` 是原子的**：整份檔案要嘛全部生效、要嘛全部不變，不會出現「改到一半」的半套狀態。
遠端改防火牆時這是保命符——逐條下指令的話，中間出錯就可能把自己鎖在門外。

**建議工作流程**：命令列用 `nft add` 試 → 通了寫進 `.nft` 檔 → `nft -c -f` 檢查 →
`nft -f` 套用 → 存進版本控制。

---

## 七、整份檔案的三層結構

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

## 八、`table inet fw4 {` 逐字

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

## 九、chain 宣告那一行逐字

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

### 同一個陷阱的另一半：鏈名 vs hook

```bash
nft add chain inet myfilter input '{ type filter hook input priority 0; policy drop; }'
                                ↑                      ↑
                          ①鏈的「名字」            ②hook 的名稱
```

**這兩個 `input` 也是完全不同的東西：**

| | 是什麼 | 可以自己取嗎 |
|---|---|---|
| ① `nft add chain <family> <table> **input**` | **你要建立的鏈叫什麼名字** | ✅ **完全自由**，叫 `foo` 也行 |
| ② `hook **input**` | **掛在哪個 netfilter 攔截點** | ❌ 只能是那五個固定關鍵字 |

它們長一樣，純粹是因為**大家習慣拿 hook 的名字來當鏈名**——看到鏈名就知道它掛在哪。
但這只是慣例，不是語法要求。

**你路由器上就有現成的反例**：13 條 base chain 裡有 9 條的名字和 hook 不一樣。

```
鏈名 input                hook input           ← 同名（慣例）
鏈名 forward              hook forward         ← 同名
鏈名 dstnat               hook prerouting      ← 不一樣！
鏈名 srcnat               hook postrouting     ← 不一樣！
鏈名 mangle_input         hook input           ← 不一樣
鏈名 mangle_forward       hook forward         ← 不一樣
鏈名 raw_prerouting       hook prerouting      ← 不一樣
```

`dstnat` 是最好的例子：**它叫 `dstnat`，但掛的是 `prerouting` 這個 hook**。fw4 選擇用「用途」
而不是「hook」來命名，因為同一個 hook 上掛了好幾條鏈（`raw_prerouting`、`mangle_prerouting`、
`dstnat`、`prerouting`），全叫 `prerouting` 會撞名。

**判斷方法：位置。** `add chain <family> <table> <名字>` 的第四個位置永遠是名字；
`hook` 這個關鍵字後面跟的永遠是 hook。**名字是你取的，hook 是核心定義的。**

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

## 十、一條規則的組成

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

## 十一、比對（expression）逐個拆

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

## 十二、動作（statement）逐個拆

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

## 十三、標點符號總表

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

## 十四、實戰：讀懂最複雜的三條

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
