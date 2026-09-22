# 把 TPROXY 設定寫成帶參數的腳本 —— 位置參數、nft 集合與「重跑不會壞」

> 日期：2026-09-22
> 情境：把 [用dokodemo-door做TPROXY透明代理.md](./用dokodemo-door做TPROXY透明代理.md) 的
> `ip rule`／`ip route`／`nft` 指令收成一支 shell 腳本，並希望要代理的來源 IP（原本寫死的
> `192.168.1.50`）改成執行時用參數帶入
> 對應檔案：路由器上的個人腳本（例如 `/root/tproxy-up.sh`），不入 git；本文收錄完整內容
> 適用範圍：OpenWrt 的 `/bin/sh`（BusyBox ash）；腳本只用 POSIX sh 語法，已在 dash 下用假的
> `ip`／`nft` 乾跑（dry run）驗證輸出，產生的 nft 規則集因本機無 root 權限**未用 `nft -c` 實測**
> 相關：[多張nft表的執行順序與ip-rule的位置.md](../linux/多張nft表的執行順序與ip-rule的位置.md)、
> [shell的for迴圈語法.md](../linux/shell的for迴圈語法.md)、
> [錢字號展開的四種形式-bad-substitution的根因.md](../linux/錢字號展開的四種形式-bad-substitution的根因.md)

**一句話結論：參數化用 `"$@"` 收下所有 IP、組成一個 nft 具名集合（named set），規則只寫
`ip saddr @proxy_src`；而一支會被反覆執行的設定腳本，最重要的不是參數，是**重跑不會壞**
（冪等，idempotent）——所以 `ip rule` 要先刪再加、`ip route` 用 `replace`、nft 整張表砍掉用
`nft -f` 重建，不能沿用互動時的 `add`。**

---

## 目錄

- [一、原腳本的三個問題](#一原腳本的三個問題)
- [二、完整腳本](#二完整腳本)
- [三、shell 怎麼把參數交給腳本](#三shell-怎麼把參數交給腳本)
- [四、為什麼把 IP 放進 nft 集合，而不是直接拼進規則](#四為什麼把-ip-放進-nft-集合而不是直接拼進規則)
- [五、重跑不會壞：每一行的冪等寫法](#五重跑不會壞每一行的冪等寫法)
- [六、here-document 裡的變數展開](#六here-document-裡的變數展開)
- [七、對應的清理腳本](#七對應的清理腳本)
- [自我測驗](#自我測驗)

---

## 一、原腳本的三個問題

原本的版本：

```sh
ip rule add fwmark 1 lookup 100 pref 2000
ip route add local 0.0.0.0/0 dev lo table 100
ip route add local 0.0.0.0/0 dev lo table 100      # ← 重複了
nft add table ip v2ray_tproxy
nft add set ip v2ray_tproxy bypass '{ ... }'
nft add chain ip v2ray_tproxy prerouting '{ ... }'
nft add rule ip v2ray_tproxy prerouting ip daddr @bypass counter return
nft add rule ip v2ray_tproxy prerouting iifname "br-lan" ip saddr 192.168.1.50 ...
```

| 問題 | 會發生什麼 | 根因 |
|---|---|---|
| `ip route add` 寫了兩次 | 第二次回 `RTNETLINK answers: File exists` | 同一張表裡已經有一模一樣的路由，`add` 不允許重複 |
| **整支腳本跑第二次** | `ip rule` 疊出**兩條** pref 2000 的相同規則；`nft add rule` 讓鏈裡的規則**變兩份**；`ip route add` 報 `File exists` | `ip rule add` 與 `nft add rule` 都不檢查「是否已存在」，每跑一次就多一條（`nft add table/chain/set` 已存在時倒是不報錯，所以前幾行看起來沒事，更容易誤判） |
| IP 寫死在規則中間 | 換一台機器要改腳本內文；要代理兩台就得複製一整行規則 | 可變的值沒有抽出來 |

第二個問題最危險：測試時一定會反覆執行，規則越疊越多，`counter` 分散到好幾條，排查時很難看懂。
這也是為什麼下面的寫法把「重跑」當成第一個要解決的事。

---

## 二、完整腳本

```sh
#!/bin/sh
# 用法：tproxy-up.sh <來源IP或網段> [來源IP或網段 ...]
# 例：  tproxy-up.sh 192.168.1.50
#       tproxy-up.sh 192.168.1.50 192.168.1.60 192.168.1.128/28
# 可用環境變數覆寫：PORT（預設 12345）、LAN_IF（預設 br-lan）
set -eu

TABLE=v2ray_tproxy
PORT=${PORT:-12345}
LAN_IF=${LAN_IF:-br-lan}
MARK=1
RT_TABLE=100      # 路由表編號
PREF=2000         # ip rule 的優先權（跟上面的表編號是兩回事）

if [ $# -eq 0 ]; then
    echo "用法: $0 <來源IP或網段> [...]" >&2
    echo "例:   $0 192.168.1.50 192.168.1.60" >&2
    exit 1
fi

# 把所有參數組成 nft 集合元素：「192.168.1.50, 192.168.1.60」
SRC=""
for ip in "$@"; do
    case $ip in
        ''|*[!0-9./]*) echo "不像 IPv4 位址或網段: '$ip'" >&2; exit 1 ;;
    esac
    SRC="${SRC:+$SRC, }$ip"
done

# ── 策略路由：先刪再加，重跑不會疊出第二條 ──
ip rule del pref "$PREF" 2>/dev/null || true
ip rule add fwmark "$MARK" lookup "$RT_TABLE" pref "$PREF"
ip route replace local 0.0.0.0/0 dev lo table "$RT_TABLE"

# ── nft：刪表與重建放在同一份輸入裡，nft -f 一次原子套用 ──
nft -f - <<EOF
table ip $TABLE
delete table ip $TABLE
table ip $TABLE {
    set bypass {
        type ipv4_addr
        flags interval
        elements = { 0.0.0.0/8, 10.0.0.0/8, 127.0.0.0/8, 169.254.0.0/16,
                     172.16.0.0/12, 192.168.0.0/16, 224.0.0.0/4, 240.0.0.0/4 }
    }
    set proxy_src {
        type ipv4_addr
        flags interval
        elements = { $SRC }
    }
    chain prerouting {
        type filter hook prerouting priority mangle; policy accept;
        ip daddr @bypass counter return
        iifname "$LAN_IF" ip saddr @proxy_src meta l4proto { tcp, udp } counter tproxy to :$PORT meta mark set $MARK accept
    }
}
EOF

echo "TPROXY 已套用：來源 { $SRC } → :$PORT"
```

執行方式：

```sh
chmod +x /root/tproxy-up.sh
/root/tproxy-up.sh 192.168.1.50                       # 一台
/root/tproxy-up.sh 192.168.1.50 192.168.1.60          # 兩台
/root/tproxy-up.sh 192.168.1.128/28                   # 一整段（flags interval 才能放網段）
PORT=10808 /root/tproxy-up.sh 192.168.1.50            # 臨時換 dokodemo-door 的埠
```

乾跑時實際產生的 nft 內容（參數 `192.168.1.50 192.168.1.128/28`）：

```
    set proxy_src {
        type ipv4_addr
        flags interval
        elements = { 192.168.1.50, 192.168.1.128/28 }
    }
```

---

## 三、shell 怎麼把參數交給腳本

執行 `/root/tproxy-up.sh 192.168.1.50 192.168.1.60` 時，shell 先依空白把整行切成幾個字（word），
第一個是腳本本身，其餘依序變成**位置參數（positional parameters）**：

```
/root/tproxy-up.sh   192.168.1.50   192.168.1.60
└──── $0 ────────┘   └── $1 ────┘   └── $2 ────┘        $# = 2
```

POSIX sh 跟參數有關的**特殊參數（special parameters）完整只有這些**（依 `man dash` 的
Special Parameters 一節，ash／dash／bash 都通用）：

| 寫法 | 展開成什麼 | 本腳本用在哪 |
|---|---|---|
| `$0` | 腳本（或 shell）本身的名字，照你呼叫時打的路徑 | 用法說明 `用法: $0 ...` |
| `$1`…`$9` | 第 1～9 個參數；第 10 個起要寫 `${10}`（`$10` 會被當成 `$1` 後面接字元 `0`） | 沒直接用——參數數量不固定，改用 `"$@"` |
| `$#` | 參數**個數**（不含 `$0`） | `[ $# -eq 0 ]` 檢查有沒有給參數（逐字拆解見 [shell條件判斷測試](../linux/shell條件判斷測試.md#九實例拆解---eq-0-有沒有給參數)） |
| `"$@"` | **每個參數各自成為一個字**，原本含空白的參數也不會被拆開；沒有參數時展開成零個字 | `for ip in "$@"` |
| `"$*"` | 所有參數**黏成一個字串**，中間用 `IFS` 的第一個字元（預設空白）隔開 | 不用——迴圈要的是逐一取出 |
| `$@`／`$*`（不加引號） | 兩者相同：先展開、再依 `IFS` 重新切字並做路徑展開（globbing） | 不用——含空白或 `*` 的參數會被破壞 |
| `$?` | 上一個指令（管線）的結束狀態（exit status），0 表示成功 | 由 `set -e` 間接使用 |
| `$$` | 目前 shell 的行程 ID（PID） | 不用 |
| `$!` | 最近一個丟到背景（`&`）的指令的 PID | 不用 |
| `$-` | 目前開啟的 shell 選項字母，例如開了 `set -eu` 後含有 `e`、`u` | 不用 |

`"$@"` 與 `"$*"` 的差別在迴圈裡最明顯：`for ip in "$@"` 每圈拿到一個 IP；
`for ip in "$*"` 只會跑**一圈**，拿到的是整串 `192.168.1.50 192.168.1.60`。
（`for` 迴圈本身的各種形式見 [shell的for迴圈語法.md](../linux/shell的for迴圈語法.md)。）

另外兩個常見工具，本腳本沒用到但要知道：

- `shift [n]`：把參數往左移 n 格（預設 1），`$2` 變成 `$1`、`$#` 減 n。適合「第一個參數是子命令、
  其餘是 IP」的設計：先 `cmd=$1; shift`，剩下的 `"$@"` 就全是 IP。
- `getopts`：解析 `-p 12345` 這類短選項。選項一多才值得用，本腳本用環境變數覆寫就夠了。

### 本腳本用到的參數展開（parameter expansion）

| 寫法 | 意思 | 本腳本 |
|---|---|---|
| `${PORT:-12345}` | `PORT` 未設定**或是空字串**時，展開成 `12345`；否則用它自己的值 | 讓 `PORT=10808 ./tproxy-up.sh ...` 能覆寫 |
| `${SRC:+$SRC, }` | `SRC` **有值**時展開成 `$SRC, `；空的時候展開成空字串 | 組逗號分隔清單，讓第一個元素前面不會多一個逗號 |

這裡只列本腳本用到的兩種；`${x:=}`、`${x:?}`、`${x#pattern}`、`${x%pattern}` 等其餘形式見
`man dash` 的 Parameter Expansion 一節，以及
[錢字號展開的四種形式](../linux/錢字號展開的四種形式-bad-substitution的根因.md)。

`SRC` 迴圈的逐圈變化：

```
開始        SRC=""
第 1 圈     SRC 是空的 → ${SRC:+...} 展開為空 → SRC="192.168.1.50"
第 2 圈     SRC 有值   → 展開為 "192.168.1.50, " → SRC="192.168.1.50, 192.168.1.60"
```

### 為什麼要驗證參數

`case $ip in ''|*[!0-9./]*)` 的意思是：**空字串，或含有任何「數字、點、斜線」以外的字元**，就拒絕。
`[!...]` 是 shell 樣式（pattern）裡的「不在這組字元裡」（`case` 文法與樣式規則的完整說明見
[shell的case語法-樣式比對與分支.md](../linux/shell的case語法-樣式比對與分支.md)）。

這不是完整的 IPv4 驗證（`999.1.1.1` 會通過，交給 nft 報錯即可），它的目的是**擋掉會改變
nft 語法的字元**。參數會被原封不動插進 nft 規則文字裡，如果不檢查，
`'1.2.3.4 } ; flush ruleset ; #'` 這種參數就能在你的規則集裡多塞一段指令。
**把外部輸入拼進另一種語言的文字時，一定要先限制字元**——這跟 SQL 注入（SQL injection）是同一類問題。

### `set -eu`

| 選項 | 效果 | 為什麼這支腳本需要 |
|---|---|---|
| `-e` | 任何指令失敗（結束狀態非 0）就立刻結束腳本 | `ip rule add` 失敗了還繼續裝 nft 規則，只會留下半套狀態 |
| `-u` | 使用未設定的變數時報錯結束，而不是當成空字串 | 變數名打錯（`$PROT`）會立刻被抓到，而不是產生 `tproxy to :` 這種壞規則 |

開了 `-e` 之後，「允許失敗」的指令要明講：`ip rule del ... 2>/dev/null || true`——
`|| true` 讓整行的結束狀態永遠是 0，第一次執行（還沒有東西可刪）才不會把腳本中止。

---

## 四、為什麼把 IP 放進 nft 集合，而不是直接拼進規則

也可以寫成 `ip saddr { $SRC }`（匿名集合，anonymous set），效果一樣。改用具名集合 `@proxy_src` 的理由：

1. **一條規則涵蓋任意多個來源**，不必每個 IP 複製一整行。比對是查表，多幾個 IP 也不會變慢。
2. **執行期可以增刪，不用重跑腳本、也不會中斷其他連線**：
   ```sh
   nft add element    ip v2ray_tproxy proxy_src '{ 192.168.1.70 }'
   nft delete element ip v2ray_tproxy proxy_src '{ 192.168.1.50 }'
   nft list set       ip v2ray_tproxy proxy_src
   ```
   匿名集合寫死在規則裡，事後不能修改（見
   [nft完整用法](../linux/nft完整用法-從命令到規則語法.md) 的〈集合、對應與判決表〉）。
3. 跟既有的 `@bypass` 是同一個模式，讀起來一致。

`flags interval` 讓集合可以放 `192.168.1.128/28` 這種網段；只放單一位址的話可以不加，但加了沒壞處。

---

## 五、重跑不會壞：每一行的冪等寫法

| 零件 | 互動時常用 | 腳本裡改成 | 為什麼 |
|---|---|---|---|
| `ip rule` | `ip rule add` | `ip rule del pref 2000 \|\| true` 再 `add` | `ip rule` **沒有 `replace`**，而 `add` 重複執行會疊出多條；指定了固定的 `pref` 才能精準刪掉自己那條 |
| `ip route` | `ip route add` | `ip route replace` | `replace`：不存在就新增、存在就覆蓋，天生冪等 |
| nft | 一行行 `nft add` | 在同一份 `nft -f` 輸入裡「建空表 → 刪表 → 完整定義」 | 刪表會連同 set、chain、rule 一起清掉，沒有相依順序問題；`nft -f` 是**原子（atomic）**套用——整份語法都對才一次生效，有錯就整份不套用，不會留下半套 |

### nft 那段開頭的三行

```
table ip v2ray_tproxy            ← ① 建一張空表（已存在時 add 不報錯，什麼都不做）
delete table ip v2ray_tproxy     ← ② 刪掉它（因為 ① 保證它存在，這行永遠不會失敗）
table ip v2ray_tproxy { ... }    ← ③ 重新建立完整內容
```

乍看 ① 很多餘，但少了它，第一次執行（表還不存在）時 ② 會報錯，整份交易就不套用了。
為什麼不在 shell 裡先跑一行 `nft delete table ... || true` 就好？因為那樣「刪」和「建」是
**兩次**交易，中間有一小段時間**完全沒有 TPROXY 規則**：正在代理中的連線，這段期間的封包
會被當成一般流量直接轉發出去。三行放在同一份 `nft -f` 裡，核心看到的是**一次切換**——
舊規則直接換成新規則，中間沒有空窗；如果新內容有語法錯誤，舊規則也原封不動。

`nft -f -` 的 `-` 表示「從標準輸入（stdin）讀」（`man nft`：*If filename is -, read from stdin*），
所以可以直接接下面的 here-document，不必先寫出一個暫存檔。

`pref 2000` 與 `lookup 100` 的兩個數字要分清楚：`pref` 是**規則的優先權**（決定這條規則在 RPDB
裡第幾個被比對），`lookup` 後面是**路由表編號**，兩者無關（見
[ip-rule與ip-route完整指南](../linux/ip-rule與ip-route完整指南.md)）。

---

## 六、here-document 裡的變數展開

```sh
nft -f - <<EOF
table ip $TABLE { ... elements = { $SRC } ... tproxy to :$PORT ... }
EOF
```

`<<EOF` 叫 here-document：把從下一行到單獨一行 `EOF` 之間的文字當成指令的標準輸入。
**結束標記有沒有加引號，決定裡面的 `$` 會不會展開**：

| 寫法 | `$SRC` 會怎樣 | 用途 |
|---|---|---|
| `<<EOF` | 被 shell 換成變數的值 | **本腳本要的**：把參數塞進 nft 規則 |
| `<<'EOF'` | 原樣保留 `$SRC` 這四個字元 | 內容本身就含 `$`、不想被 shell 動到時 |

這裡用不加引號的版本才能把 IP 帶進去。**`@bypass`、`{ }`、`;` 對 shell 在 here-document 裡
沒有特殊意義**，不需要跳脫；需要小心的只有 `$`、`` ` ``、`\`。
nft 自己也有 `$變數`（`define`）語法，如果你之後在這份內容裡用到 nft 的變數，要寫成 `\$var`，
否則會先被 shell 展開掉。

---

## 七、對應的清理腳本

三個零件要一起清（原因見 [用dokodemo-door做TPROXY透明代理](./用dokodemo-door做TPROXY透明代理.md)
的〈回退與救援〉），數字要跟 `tproxy-up.sh` 一致：

```sh
#!/bin/sh
# tproxy-down.sh：移除 tproxy-up.sh 建立的所有東西；重跑也不會報錯
nft delete table ip v2ray_tproxy 2>/dev/null || true
ip rule del pref 2000 2>/dev/null || true
ip route flush table 100 2>/dev/null || true
echo "TPROXY 已移除"
```

測試階段的標準節奏：`tproxy-up.sh 192.168.1.50` → 驗證 → 有問題就 `tproxy-down.sh`，
改完再 up。**因為 up 本身冪等，其實不 down 直接再跑一次 up 也可以。**

### 合併成一支：`del` 子命令

也可以不另寫 down 腳本，讓同一支腳本認得 `del`：`tproxy.sh 192.168.1.50` 套用、
`tproxy.sh del` 移除。第一版寫成這樣：

```sh
set -eu
...
if [ "$1" = "del" ]; then          # ✗ 沒帶任何參數時，腳本會在這行直接中止
```

問題出在 `set -u`：沒帶參數時 `$1` 是**未設定**，`-u` 讓 shell 一讀到它就中止，使用者看不到用法說明：

```
$ dash -c 'set -eu; if [ "$1" = "del" ]; then echo del; fi; echo 後面' x
x: 1: 1: parameter not set
```

改成 `${1:-}`——`$1` 未設定時展開成空字串，`-u` 就不會觸發。

注意這一行**每次執行都會經過**，不是只有打 `del` 時才跑：

| 執行 | `$1` | `[ "$1" = del ]` | `[ "${1:-}" = del ]` |
|---|---|---|---|
| `tproxy.sh del` | `del` | 成立 | 成立 |
| `tproxy.sh 192.168.1.50` | `192.168.1.50` | 不成立，往下走 | 不成立，往下走 |
| `tproxy.sh`（忘了帶參數） | **未設定** | **`set -u` 當場中止** | 空字串，不成立 → 走到用法說明 |

「未設定」與「空字串」是兩回事，`set -u` 只對前者中止。

完整的合併版（已在 dash 用假的 `ip`／`nft` 乾跑五種呼叫方式：無參數、`del`、`del 1.2.3.4`、`abc`、兩個 IP，行為都符合預期；同樣未在真的 nft 上實測）：

```sh
#!/bin/sh
# tproxy.sh：OpenWrt 上 TPROXY 透明代理的套用與移除
#
# 用法：tproxy.sh <來源IP或網段> [來源IP或網段 ...]   套用（重跑會整份替換）
#       tproxy.sh del                                  移除
# 例：  tproxy.sh 192.168.1.50 192.168.1.60 192.168.1.128/28
# 可用環境變數覆寫：PORT（預設 12345）、LAN_IF（預設 br-lan）
set -eu

TABLE=v2ray_tproxy
PORT=${PORT:-12345}
LAN_IF=${LAN_IF:-br-lan}
MARK=1
RT_TABLE=100      # 路由表編號
PREF=2000         # ip rule 的優先權（跟上面的表編號是兩回事）

usage() {
    echo "用法: $0 <來源IP或網段> [...]   套用" >&2
    echo "      $0 del                    移除" >&2
    exit 1
}

# ── 移除：先停止攔截（nft），再拆策略路由 ──
if [ "${1:-}" = "del" ]; then
    [ $# -eq 1 ] || usage
    nft delete table ip "$TABLE" 2>/dev/null || true
    ip rule del pref "$PREF" 2>/dev/null || true
    ip route flush table "$RT_TABLE" 2>/dev/null || true
    echo "TPROXY 已移除"
    exit 0
fi

[ $# -gt 0 ] || usage

# ── 檢查參數，組成 nft 集合元素：「192.168.1.50, 192.168.1.60」 ──
SRC=""
for ip in "$@"; do
    case $ip in
        ''|*[!0-9./]*) echo "不像 IPv4 位址或網段: '$ip'" >&2; exit 1 ;;
    esac
    SRC="${SRC:+$SRC, }$ip"
done

# ── 策略路由：先刪再加，重跑不會疊出第二條 ──
ip rule del pref "$PREF" 2>/dev/null || true
ip rule add fwmark "$MARK" lookup "$RT_TABLE" pref "$PREF"
ip route replace local 0.0.0.0/0 dev lo table "$RT_TABLE"

# ── nft：刪表與重建放在同一份輸入裡，nft -f 一次原子套用 ──
nft -f - <<EOF
table ip $TABLE
delete table ip $TABLE
table ip $TABLE {
    set bypass {
        type ipv4_addr
        flags interval
        elements = { 0.0.0.0/8, 10.0.0.0/8, 127.0.0.0/8, 169.254.0.0/16,
                     172.16.0.0/12, 192.168.0.0/16, 224.0.0.0/4, 240.0.0.0/4 }
    }
    set proxy_src {
        type ipv4_addr
        flags interval
        elements = { $SRC }
    }
    chain prerouting {
        type filter hook prerouting priority mangle; policy accept;
        ip daddr @bypass counter return
        iifname "$LAN_IF" ip saddr @proxy_src meta l4proto { tcp, udp } counter tproxy to :$PORT meta mark set $MARK accept
    }
}
EOF

echo "TPROXY 已套用：來源 { $SRC } → :$PORT"
```

跟上面分開的版本相比多了兩處：用法說明抽成 `usage` 函式（三個地方要用）；`del` 後面若還帶了東西（`tproxy.sh del 1.2.3.4`）就印用法，而不是默默忽略多餘的參數——使用者可能以為只會刪掉那個 IP。

幾個位置上的考量：

- **`del` 的判斷必須放在 IP 檢查迴圈之前**：`del` 含字母，走到 `case` 會被當成不合格的 IP 拒絕。
- **路由那行用 `ip route flush table`，不用 `ip route del table 100`**：`ip route del` 是刪
  「一條指定的路由」，沒寫目的網段時刪的是哪一條並不明確（本機無 root 權限，沒有實測它的行為），
  要刪就寫完整的 `ip route del local 0.0.0.0/0 dev lo table 100`；`flush table` 則明確是
  「這張表全部清空」，表 100 本來就是這支腳本專用的。
- **`nft` 先刪**：先停止攔截與打 mark，再拆路由，拆的過程中不會有「打了 mark 卻找不到路由」的狀態
  （其實反過來也不會出事——規則命中但表是空的會掉回 `main`，見
  [ip-rule與ip-route完整指南](../linux/ip-rule與ip-route完整指南.md)——只是這個順序比較好推理）。
- `2>/dev/null || true` 讓「本來就沒有」不算錯誤，但也會吞掉真正的錯誤（例如沒有權限），
  所以 `TPROXY 已刪除` 只代表「跑完了」。要確認就 `nft list tables`、`ip rule show` 看一眼。
- 腳本同時負責套用與移除，檔名就不該再叫 `tproxy-up.sh`，改成 `tproxy.sh` 之類。

---

## 自我測驗

1. 原腳本（重複的那行 `ip route add` 已刪掉）執行第二次，只看到一行 `File exists`，
   你以為無害就忽略了，但之後排查時發現 counter 很奇怪。實際上系統裡多了什麼？
   為什麼 `nft add table`／`nft add set` 那幾行沒有報錯，更容易讓人誤以為一切正常？
2. `for ip in "$@"` 改成 `for ip in "$*"` 或 `for ip in $@`，各會發生什麼事？
3. 為什麼 `ip rule` 要用「先 `del pref` 再 `add`」，而 `ip route` 可以直接用 `replace`？
   如果 `ip rule add` 沒有指定 `pref`，「先刪再加」這招還能用嗎？
4. 拿掉 `case` 那段驗證，會有什麼風險？為什麼說它跟 SQL 注入是同一類問題？
5. 情境題：TPROXY 已經在跑，現在要把 `192.168.1.70` 也加入代理。你有兩種做法：只加集合元素，
   或帶著兩個 IP 重跑整支腳本。兩者各會怎樣？如果有人把腳本改成先在 shell 裡
   `nft delete table ... || true`、再 `nft -f` 建表，重跑時正在代理中的 `192.168.1.50` 會受什麼影響？
