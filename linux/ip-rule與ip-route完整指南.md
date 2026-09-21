# `ip rule` 與 `ip route` 完整指南 —— 路由決策的兩層結構

> 日期：2026-09-21
> 情境：要在 OpenWrt 上做 TPROXY，那兩行 `ip rule add fwmark 1 lookup 100` 與
> `ip route add local 0.0.0.0/0 dev lo table 100` 看不懂，決定先把路由整套搞清楚
> 適用範圍：實測 Ubuntu 24.04（kernel 7.0）與 OpenWrt 24.10.2；iproute2 通用
> 相關：[查詢轉發表與路由表.md](./查詢轉發表與路由表.md)（`ip route show` 的欄位在那篇）、
> [用dokodemo-door做TPROXY透明代理.md](../openwrt/用dokodemo-door做TPROXY透明代理.md)

**一句話結論：Linux 的路由決策是**兩層**，不是一層。第一層 `ip rule` 決定「這個封包要查**哪一張**
路由表」，第二層才在那張表裡做最長前綴匹配。平常感覺不到第一層，是因為預設規則永遠把你送去
`main` 表。一旦你要做「同一個目的地、不同封包走不同路」（fwmark、來源位址、不同介面），
就一定得動第一層。**

---

## 目錄

- [一、心智模型：兩層決策](#一心智模型兩層決策)
- [二、命令的統一文法](#二命令的統一文法)
- [三、`ip rule`：決定查哪張表](#三ip-rule決定查哪張表)
- [四、路由表不只有一張](#四路由表不只有一張)
- [五、`ip route`：表裡的每一條路](#五ip-route表裡的每一條路)
- [六、`ip route get`：最重要的排查工具](#六ip-route-get最重要的排查工具)
- [七、回頭看 TPROXY 的那兩行](#七回頭看-tproxy-的那兩行)
- [八、常用情境例子](#八常用情境例子)
- [九、七個陷阱](#九七個陷阱)
- [十、速查](#十速查)
- [自我測驗](#自我測驗)

---

## 一、心智模型：兩層決策

```
封包要送出去
     │
     ▼
┌─────────────────────────────────────────────┐
│ 第一層：RPDB（Routing Policy DataBase）      │  ← ip rule
│  由上而下比對規則，看封包符合哪一條           │
│  命中 → 得到「要查哪一張表」                  │
└─────────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────────┐
│ 第二層：在那張表裡做最長前綴匹配              │  ← ip route
│  多條都符合時，前綴最長的贏；同長比 metric     │
└─────────────────────────────────────────────┘
     │
     ▼
決定出口網卡、下一跳、來源位址
```

**為什麼平常感覺不到第一層**：預設只有三條規則，而且沒有任何條件限制
（`from all`），所以任何封包都會走到 `main` 表。**第一層等於不存在**——直到你自己加規則。

---

## 二、命令的統一文法

`ip` 底下所有子命令都是同一個形狀：

```
ip [ 全域選項 ] <物件> { <動詞> [ 參數... ] | help }
        -4        route      add    default via 192.168.1.1
        -br       rule       del    pref 100
        -j        addr       show
```

**最該記住的一件事：文法本身可以問工具。**

```bash
ip route help        # 印出 ROUTE 的完整文法
ip rule help         # 印出 SELECTOR 與 ACTION 的完整清單
ip help              # 有哪些物件、有哪些全域選項
```

背不起來時問它，比翻網頁快——下面幾張表就是從這三個指令的輸出整理出來的。

### `ip route` 的動詞

```
ip route { add | del | change | append | replace } ROUTE
ip route { list | flush } SELECTOR
ip route get [FLAGS] ADDRESS
ip route { save | restore }
```

| 動詞 | 行為 | 什麼時候用 |
|---|---|---|
| `add` | 新增；**已存在會報 `File exists` 而失敗** | 確定是新的 |
| `change` | 修改**已存在**的；不存在會失敗 | 確定要改既有的 |
| **`replace`** | **有就改、沒有就加** | **寫腳本用這個**，重跑不會失敗 |
| `append` | 加在同 prefix 的既有路由**後面**（做多路徑） | 負載平衡 |
| `del` | 刪除 | |
| `list`／`show` | 列出 | |
| `flush` | 依條件批次刪除 | |
| `get` | **問核心實際會怎麼走**（見第六節〈`ip route get`〉） | 排查 |
| `save`／`restore` | 二進位備份／還原 | 大改之前先備份 |

### `ip route` 的參數怎麼組

```
ip route add [型別] <前綴> [via <下一跳>] [dev <介面>] [src <來源位址>]
             [table <表>] [metric <數字>] [scope <範圍>] [proto <來源>]
             [mtu <數字>] ...
```

| 參數 | 意思 |
|---|---|
| 型別 | `unicast`(預設)／`local`／`blackhole`／`unreachable`／`prohibit`／`throw` |
| 前綴 | `default`、`10.0.0.0/8`、單一 IP |
| `via` | 下一跳的 IP（**不同網段**時必須） |
| `dev` | 從哪張網卡送出（**同網段**時只要這個） |
| `src` | 本機發起時**用哪個位址當來源**（不影響轉發的封包） |
| `table` | 加到哪張表，不寫就是 `main` |
| `metric` | 同前綴時的優先順序，**數字小的贏** |
| `scope` | `global`（要經過閘道）／`link`（同網段可直達）／`host`（本機） |
| `proto` | 誰加的（`static`／`kernel`／`dhcp`／`boot`），只是標記，不影響行為 |

### `ip rule` 的動詞與組成

```
ip rule { add | del } SELECTOR ACTION
ip rule { flush | save | restore }
ip rule [ list [ SELECTOR ] ]
```

**一條規則 = 選擇器（挑哪些封包）+ 動作（挑到了要做什麼）**：

```
SELECTOR := [not] [from 前綴] [to 前綴] [tos TOS] [fwmark 值[/遮罩]]
            [iif 介面] [oif 介面] [pref 數字] [uidrange A-B]
            [ipproto 協定] [sport 埠[-埠]] [dport 埠[-埠]]

ACTION   := [table 表] | [goto 數字] | [nat 位址]
            | blackhole | unreachable | prohibit
            | [suppress_prefixlength 數字]
```

幾個少見但好用的：

| 寫法 | 用途 |
|---|---|
| `not from 192.168.1.0/24` | **反向**條件（`not` 可以套在任何選擇器前） |
| `fwmark 1/0xff` | 只比對 mark 的**某幾個位元**（多個功能共用 mark 時必備） |
| `uidrange 1001-1001` | 依**執行使用者**分流 |
| `goto 200` | 跳到 priority 200 的規則繼續比對 |
| `suppress_prefixlength 0` | 「**忽略這張表裡的預設路由**」，常用來讓 VPN 表不吃掉 default |

### 全域選項

| 選項 | 作用 |
|---|---|
| `-4` / `-6` | 只看 IPv4／IPv6 |
| **`-br`** | **每行一筆的精簡輸出**（`ip -br addr` 特別好用） |
| `-j` / `-p` | JSON／格式化 JSON（**給腳本用**） |
| `-s` | 附統計數字 |
| `-d` | 更詳細 |
| `-n <netns>` | 在指定的 network namespace 裡執行 |
| `-c` | 彩色輸出 |

實測：

```bash
$ ip -br -4 addr show
lo          UNKNOWN  127.0.0.1/8
wlp59s0f0   UP       192.168.1.171/24
docker0     DOWN     172.17.0.1/16

$ ip -j route show | head -c 120
[{"dst":"default","gateway":"192.168.1.1","dev":"wlp59s0f0","protocol":"dhcp","prefsrc":"192.168.1.171",...
```

---

## 三、`ip rule`：決定查哪張表

### 預設的三條

```bash
$ ip rule show
0:      from all lookup local        ← 最先，且不該動
32766:  from all lookup main         ← 你平常看的那張表
32767:  from all lookup default      ← 幾乎永遠是空的
```

| 優先權 | 表 | 作用 |
|---|---|---|
| `0` | `local` | 「**這個位址是不是我自己**」——核心自動維護，見第四節〈路由表不只有一張〉 |
| `32766` | `main` | 一般路由，`ip route show` 預設顯示的就是它 |
| `32767` | `default` | 歷史遺留，通常空的 |

**數字小的先比對**，所以自訂規則放在 0 到 32765 之間就會**早於 main** 被評估——
這正是「讓特定封包走特別的路」的實作方式。

### 一條規則的三個部分

```
ip rule add  fwmark 1   lookup 100
             └─選擇器─┘ └──動作──┘
         （priority 不寫的話自動指派）
```

**選擇器（selector）**——用什麼條件挑封包：

| 選擇器 | 意思 |
|---|---|
| `from <網段>` | 來源位址 |
| `to <網段>` | 目的位址 |
| `fwmark <值>` | **防火牆打的標記**（nftables 的 `meta mark set`） |
| `iif <介面>` | 從哪張網卡進來 |
| `oif <介面>` | 要從哪張網卡出去 |
| `uidrange <a>-<b>` | 發送它的行程 uid（可做「某使用者走 VPN」） |
| `ipproto` / `sport` / `dport` | 協定與埠（較新的核心支援） |

**動作（action）**：

| 動作 | 效果 |
|---|---|
| `lookup <表>` / `table <表>` | **查這張表**（最常用） |
| `blackhole` | 直接丟棄，不回應 |
| `unreachable` | 回 ICMP「目的地不可達」 |
| `prohibit` | 回 ICMP「管理上禁止」 |

### 一個很多人不知道的細節

**規則命中、但那張表裡查不到符合的路由時，核心會繼續評估下一條規則**，而不是宣告失敗。

```
規則 A (pref 100, fwmark 1 → table 100)   ← 命中，但 table 100 裡沒有對應路由
規則 B (pref 32766, from all → main)      ← 繼續，在 main 裡找到了
```

所以策略路由「沒生效」時，常常不是規則沒命中，而是**命中了但那張表是空的**，於是悄悄掉回 main。
**驗證方式是 `ip route get`（第六節〈`ip route get`〉），不是看規則有沒有列出來。**

### 操作

```bash
ip rule show                                  # 列出
ip rule add fwmark 1 lookup 100               # 新增（自動配 priority）
ip rule add from 192.168.1.50 lookup 100 pref 100   # 指定 priority
ip rule del fwmark 1 lookup 100               # 刪除（條件要寫得跟當初一樣）
ip rule del pref 100                          # 或用 priority 刪，較保險
```

> ⚠️ **`ip rule flush` 會連那三條預設規則一起刪掉**，機器立刻失去所有路由能力。
> 遠端操作時這是一鍵斷線。見第九節〈七個陷阱〉。

---

## 四、路由表不只有一張

### 表的編號與名字

```bash
$ cat /etc/iproute2/rt_tables
255  local
254  main
253  default
0    unspec
```

**這個檔案只是「編號 ↔ 名字」的對照表**，方便你寫 `lookup mytable` 而不是 `lookup 100`。
**表本身存在核心裡，不在這個檔案**——你 `ip route add ... table 100` 的當下，核心就有了 table 100，
不需要先在這裡登記。登記只是為了可讀性。

表的編號範圍是 1–252（加上保留的 253/254/255），所以自訂表隨便挑一個沒用過的即可。

### `local` 表：最特別的一張

```bash
$ ip route show table local
local     127.0.0.0/8     dev lo        proto kernel scope host src 127.0.0.1
local     127.0.0.1       dev lo        proto kernel scope host src 127.0.0.1
broadcast 127.255.255.255 dev lo        proto kernel scope link src 127.0.0.1
local     192.168.1.171   dev wlp59s0f0 proto kernel scope host src 192.168.1.171
local     172.17.0.1      dev docker0   proto kernel scope host src 172.17.0.1
broadcast 172.17.255.255  dev docker0   ...
```

**它回答的問題是「哪些位址算是我自己的」。** 每次你給介面設位址，核心**自動**在這張表加一條
`local`；`proto kernel` 就是「核心自己加的」的標記。

**實測證據**——查自己的位址會得到 `local ... dev lo`：

```bash
$ ip route get 192.168.1.171
local 192.168.1.171 dev lo src 192.168.1.171
```

送往自己位址的封包，核心直接從 `lo` 繞回來，不會真的送上網卡。
**記住這個行為，第七節〈回頭看 TPROXY 的那兩行〉 就是靠它。**

> **不要手動改 local 表**。改壞了會讓核心搞不清楚哪些位址是自己的，症狀非常詭異
> （服務綁得起來卻收不到封包）。

---

## 五、`ip route`：表裡的每一條路

（`via`／`dev`／`src`／`metric` 這些欄位的完整解讀見
[查詢轉發表與路由表.md](./查詢轉發表與路由表.md)，這裡只補那篇沒講的**型別**與**操作**。）

### 路由型別

每條路由都有型別，**不寫就是 `unicast`**：

| 型別 | 意思 | 典型用途 |
|---|---|---|
| `unicast`（預設） | 正常轉送到某個下一跳／介面 | 一般路由 |
| **`local`** | **這個位址是本機的**，交給本機處理 | local 表；**TPROXY** |
| `broadcast` | 廣播位址 | local 表 |
| `blackhole` | 丟棄，不回應 | 擋掉某網段 |
| `unreachable` | 丟棄並回 ICMP 不可達 | 明確告知 |
| `prohibit` | 丟棄並回 ICMP 管理禁止 | |
| `throw` | **在這張表裡當作查不到**，繼續評估後面的規則 | 策略路由的「例外」 |

`throw` 很實用：在自訂表裡放一條 `throw 192.168.0.0/16`，就等於「這個網段不要用這張表的規則，
回去走正常流程」。

### 操作

```bash
ip route show                       # 顯示 main
ip route show table 100             # 顯示指定表
ip route show table all             # 所有表（會很長）

ip route add 10.0.0.0/8 via 192.168.1.254           # 新增
ip route add default via 192.168.1.1 table 100      # 在自訂表裡加預設路由
ip route add local 0.0.0.0/0 dev lo table 100       # 加一條 local 型別的路由
ip route del 10.0.0.0/8                             # 刪除
ip route replace default via 192.168.1.2            # 有就換、沒有就加（比 del+add 安全）
ip route flush table 100                            # 清空某張表
```

**`add` 與 `replace` 的差別**：`add` 遇到已存在的路由會報 `File exists` 而失敗；
`replace` 直接覆蓋。**寫腳本時用 `replace` 比較不會因為重跑而失敗。**

---

## 六、`ip route get`：最重要的排查工具

**它不是「查表」，而是叫核心真的跑一次完整的路由決策**（兩層都跑），然後告訴你結果。
這是唯一能回答「這個封包到底會怎麼走」的方法。

本機實測：

```bash
$ ip route get 8.8.8.8
8.8.8.8 via 192.168.1.1 dev wlp59s0f0 src 192.168.1.171      ← 走預設閘道

$ ip route get 192.168.1.50
192.168.1.50 dev wlp59s0f0 src 192.168.1.171                  ← 同網段，直送

$ ip route get 192.168.1.171
local 192.168.1.171 dev lo src 192.168.1.171                  ← 自己的位址 → lo

$ ip route get 172.17.0.5
172.17.0.5 dev docker0 src 172.17.0.1                         ← 走 docker 橋接
```

**它可以模擬各種條件**，這才是它真正強大的地方：

```bash
ip route get 8.8.8.8 mark 1              # 假裝這個封包帶著 fwmark 1
ip route get 8.8.8.8 from 172.17.0.1     # 假裝來源是這個位址
ip route get 8.8.8.8 oif eth1            # 假裝要從 eth1 出去
ip route get 8.8.8.8 iif br-lan          # 假裝從 br-lan 進來（測轉發）
```

**加了 `mark` 就能在寫規則前後做對照**：

```bash
$ ip route get 8.8.8.8 mark 1
8.8.8.8 via 192.168.1.1 dev wlp59s0f0 src 192.168.1.171 mark 1
                                          ↑ 目前沒有 fwmark 規則，所以跟沒加一樣
```

**心法：改完 `ip rule`／`ip route` 之後，不要用「看起來有列出來」當作驗證，
要用 `ip route get` 問核心。** 這正是
[排查方法論.md](./排查方法論.md) 說的「不要相信自己以為的狀態」。

---

## 七、回頭看 TPROXY 的那兩行

```bash
ip route add local 0.0.0.0/0 dev lo table 100
ip rule add fwmark 1 lookup 100
```

現在可以逐字讀懂了：

### 第一行

| 片段 | 意思 |
|---|---|
| `ip route add` | 新增一條路由 |
| **`local`** | **型別是 local**——「這個位址屬於本機」 |
| `0.0.0.0/0` | 範圍是**所有位址** |
| `dev lo` | 從迴路介面處理 |
| `table 100` | 加在編號 100 的表裡（不是 main） |

合起來：**「在 table 100 裡，宣告全世界所有位址都是我自己的位址」**。

這為什麼有用？回想第四節〈路由表不只有一張〉的實測——查自己的位址會得到 `local ... dev lo`，核心會把封包交給本機
處理而不是轉送出去。現在整個 `0.0.0.0/0` 都是「自己的位址」，**任何目的地的封包只要查到這張表，
都會被當成給本機的。**

### 第二行

「**帶有 fwmark 1 的封包，改查 table 100**」。priority 沒指定，會自動配一個小於 32766 的值，
所以**早於 main 被評估**。

### 兩行合起來的效果

```
LAN 電腦 → 1.1.1.1:443 的封包
   ↓
nftables 在 prerouting 打上 mark 1
   ↓
第一層：ip rule 比對 → fwmark 1 命中 → 查 table 100
   ↓
第二層：table 100 說「0.0.0.0/0 都是 local」→ 這封包是給本機的
   ↓
核心不轉發，交給本機的 socket（IP_TRANSPARENT 的那個）
```

**沒有這兩行，第一層會走到 main 表，main 說「1.1.1.1 要從 eth1 轉出去」，
封包就直接出去了，你的 12345 socket 永遠等不到。**

### 驗證方式

```bash
# 設定之後，模擬一個帶 mark 的封包
ip route get 1.1.1.1 mark 1
# 期待看到：local 1.1.1.1 dev lo ...   ← 有 local 就對了
```

---

## 八、常用情境例子

### 最基本的增刪改

```bash
# 加一條靜態路由：10.0.0.0/8 走 192.168.1.254
ip route add 10.0.0.0/8 via 192.168.1.254

# 同網段、不需要閘道時只要指定網卡
ip route add 192.168.5.0/24 dev eth1

# 換掉預設閘道（replace：有就改、沒有就加）
ip route replace default via 192.168.1.2

# 刪除
ip route del 10.0.0.0/8

# 批次刪除：清掉所有由 dhcp 加的路由
ip route flush proto dhcp
```

### `add` 重跑會失敗，腳本要用 `replace`

```bash
$ ip route add 10.0.0.0/8 via 192.168.1.254
$ ip route add 10.0.0.0/8 via 192.168.1.254
RTNETLINK answers: File exists          # ← 腳本重跑就在這裡死掉

$ ip route replace 10.0.0.0/8 via 192.168.1.254   # 冪等，重跑幾次都一樣
```

`ip rule add` 更麻煩——**它不會擋重複，會真的加第二條一模一樣的規則**：

```bash
ip rule add fwmark 1 lookup 100      # 跑兩次就有兩條
ip rule show | grep 'fwmark 0x1'     # 確認
# 安全寫法：先刪再加，或用固定 pref 方便刪
ip rule del pref 100 2>/dev/null
ip rule add fwmark 1 lookup 100 pref 100
```

### 指定來源位址

```bash
# 本機連 10.x 時，用 192.168.1.171 當來源（多網卡機器常需要）
ip route add 10.0.0.0/8 via 192.168.1.254 src 192.168.1.171
ip route get 10.0.0.1                      # 驗證 src 是否如預期
```

### 多路徑（負載平衡）

```bash
ip route add default \
    nexthop via 10.0.0.1 dev eth0 weight 1 \
    nexthop via 10.0.1.1 dev eth1 weight 1
```

### 擋掉某個網段

```bash
ip route add blackhole 203.0.113.0/24       # 安靜丟棄
ip route add unreachable 198.51.100.0/24    # 丟棄並回 ICMP 不可達
```

### 雙 ISP／多出口

```bash
# 每個 ISP 一張表
ip route add default via 10.0.0.1 dev eth0 table 101
ip route add default via 10.0.1.1 dev eth1 table 102

# 從哪個位址出去就走對應的表（否則回程走錯網卡會被丟掉）
ip rule add from 10.0.0.2 lookup 101 pref 101
ip rule add from 10.0.1.2 lookup 102 pref 102
```

### 特定來源／使用者走 VPN

```bash
ip route add default dev tun0 table 200
ip rule add from 192.168.1.50 lookup 200 pref 200     # 只有這台
ip rule add uidrange 1001-1001 lookup 200 pref 201    # 只有這個 uid
```

### 讓某個網段不要走那張表（`throw`）

```bash
ip route add throw 192.168.0.0/16 table 200
# table 200 遇到內網位址 → 當作查不到 → 繼續評估後面的規則 → 回到 main
```

### 備份與還原

```bash
ip route save > routes.bin        # 二進位備份
ip route restore < routes.bin     # 還原
ip rule save  > rules.bin
ip rule restore < rules.bin
```

**大改之前先備份**，比事後回想自己改了什麼可靠。

### 在容器／netns 裡操作

```bash
ip netns list
ip -n <netns名> route show        # 不用 nsenter，直接指定命名空間
ip -n <netns名> route get 8.8.8.8
```

### 給腳本用的輸出

```bash
ip -j route show | jq -r '.[] | select(.dst=="default") | .gateway'   # 取出預設閘道
ip -br -4 addr show                                                    # 一行一介面
```

## 九、七個陷阱

| # | 陷阱 | 說明 |
|---|---|---|
| 1 | **`ip rule flush` 會刪掉預設三條** | 機器立刻失去所有路由能力，遠端操作等於自殺。要清自訂規則請用 `ip rule del pref <N>` 逐條刪 |
| 2 | **改了 `ip route` 但已建立的連線沒變** | conntrack 已記住那條連線的路徑。要讓新路由對現有連線生效得清 conntrack |
| 3 | **規則命中但表是空的** | 會悄悄掉回下一條規則，看起來像「規則沒作用」。用 `ip route get` 驗證 |
| 4 | **全部都是非持久化的** | `ip rule`／`ip route` 的改動重開機就消失。這在測試時是優點，正式用要寫進 init script 或 hotplug |
| 5 | **手改 `local` 表** | 核心會搞不清楚哪些位址是自己的，症狀極詭異。除非你很清楚在做什麼（例如 TPROXY 那條加在**自訂表**裡，不是 local 表） |
| 6 | **priority 撞號** | 同一個 priority 可以塞多條規則，比對順序變得不確定。自訂時明確指定 `pref` |
| 7 | **`ip route add` 重跑會失敗** | 腳本裡用 `replace`，或加 `2>/dev/null` 容錯 |

---

## 十、速查

```bash
# === 看 ===
ip rule show                       # 第一層：規則
ip route show                      # 第二層：main 表
ip route show table local          # 本機位址（核心維護）
ip route show table all            # 全部表
ip route get <IP>                  # ★ 問核心「實際會怎麼走」
ip route get <IP> mark 1           #   模擬帶標記的封包
ip route get <IP> from <IP>        #   模擬指定來源

# === 改（都是暫時的，重開機消失）===
ip rule add fwmark 1 lookup 100 pref 100
ip rule del pref 100
ip route add default via 192.168.1.1 table 100
ip route add local 0.0.0.0/0 dev lo table 100
ip route replace default via 192.168.1.1        # 冪等，腳本用這個
ip route flush table 100

# === 文法忘了就問工具 ===
ip route help ; ip rule help ; ip help

# === 備份 ===
ip route save > routes.bin ; ip route restore < routes.bin
ip rule  save > rules.bin  ; ip rule  restore < rules.bin

# === 救援 ===
ip rule show                       # 先確認預設三條還在
ip route show table local | head   # 確認本機位址還在
#（最壞情況：reboot 會還原所有非持久化的改動）
```

---

## 自我測驗

1. Linux 的路由決策分成哪兩層？平常為什麼幾乎感覺不到第一層的存在？
2. 你加了 `ip rule add fwmark 1 lookup 100`，`ip rule show` 也確實列出來了，但封包行為完全沒變。
   除了「規則沒命中」之外，還有一個很常見的原因是什麼？要用哪個指令才能分辨這兩種情況？
3. `ip route add local 0.0.0.0/0 dev lo table 100` 這條路由，`local` 這個字讓核心做了什麼判斷？
   為什麼這正好是 TPROXY 需要的？
4. 你要寫一個開機腳本來建立策略路由。`ip route add` 與 `ip route replace` 該選哪個、為什麼？
   另外 `ip rule add` 重複執行會發生什麼事（和 `ip route add` 不一樣），該怎麼寫才安全？
5. 情境題：你在遠端主機上想清掉自己剛加的策略路由規則，打了 `ip rule flush`。會發生什麼事？
   正確的做法應該是什麼？
