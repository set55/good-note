# tmux 用 `Ctrl+b c` 開了好幾個 window，除了 `Ctrl+b s` 還能怎麼切？

> 日期：2026-10-12
> 情境：用 `Ctrl+b c` 建了多個 window，只會用 `Ctrl+b s` 叫出清單來切，想知道其他方法。
> 適用範圍：tmux 3.4（Ubuntu 24.04）；按鍵以預設設定為準（`~/.tmux.conf` 只有 `set -g mouse on`，沒有改按鍵）。

**一句話結論：快捷鍵只是 `select-window`／`next-window`／`previous-window`／`last-window`／`choose-tree`／`find-window` 這幾個命令的別名——知道編號用 `Ctrl+b 數字`，來回兩個用 `Ctrl+b l`，依序走用 `n`／`p`，忘了在哪用 `w`（看清單）或 `f`（搜尋）。**

---

## 一、先釐清：`Ctrl+b s` 其實是「選 session」的清單

tmux 的層級是 **session（工作階段）→ window（視窗，狀態列上一格一格的那個）→ pane（窗格，一個 window 切出來的區塊）**。

`Ctrl+b s` 綁的是 `choose-tree -Zs`：`-s` 的意思是**一開始把 session 收合起來**，所以你看到的是 session 清單，要展開才看到 window。
它能切 window 只是因為這棵樹裡也有 window。專門給 window 用的是 `Ctrl+b w`（`choose-tree -Zw`，`-w` 是把 window 收合，所以畫面會直接列出每個 session 底下的 window）。

用 `tmux list-keys -T prefix` 可以查到預設綁定（這是 prefix 表全部 85 個綁定裡跟切 window 有關的部分）：

```text
bind-key    -T prefix 0       select-window -t :=0
...（1～9 同理）
bind-key    -T prefix \'      command-prompt -T window-target -p index { select-window -t ":%%" }
bind-key    -T prefix f       command-prompt { find-window -Z "%%" }
bind-key    -T prefix l       last-window
bind-key    -T prefix n       next-window
bind-key    -T prefix p       previous-window
bind-key    -T prefix s       choose-tree -Zs
bind-key    -T prefix w       choose-tree -Zw
bind-key    -T prefix M-n     next-window -a
bind-key    -T prefix M-p     previous-window -a
```

## 二、所有切 window 的方法

| 按法（先按 `Ctrl+b` 放開再按） | 實際執行的命令 | 什麼時候用 |
|---|---|---|
| `0`～`9` | `select-window -t :=N` | 知道編號（狀態列上 `0:bash 1:vim` 前面的數字），最快 |
| `'` 再輸入編號 Enter | `select-window -t :N` | 編號 ≥ 10，按一個數字鍵到不了 |
| `n` / `p` | `next-window` / `previous-window` | 依編號往後／往前一個，走到底會繞回頭 |
| `l`（小寫 L） | `last-window` | **在最近兩個 window 之間來回**，像遙控器的「上一台」 |
| `w` | `choose-tree -Zw` | 看全部 window 清單（含預覽）再選 |
| `f` 再輸入字串 | `find-window -Z` | 忘了編號，只記得名字或畫面上出現過的字 |
| `Alt+n` / `Alt+p` | `next-window -a` / `previous-window -a` | 只跳到「有警示（alert）」的 window，例如有 bell 的 |
| 滑鼠點狀態列 | （mouse 綁定） | 你的設定有 `set -g mouse on`，直接點狀態列上的 window 名稱就切過去 |

在 `w`／`s` 的樹狀清單裡：上下鍵移動、Enter 選定；每行左邊括號裡的字（`(0)`、`(1)`…）可以**直接按那個鍵**立刻跳過去，不用移游標；`q` 離開。
清單模式全部的按鍵見 `man tmux` 的 `choose-tree` 一節（共 25 個，例如 `x` 關掉選中的項目、`C-s` 搜尋名稱）。

## 三、背後的命令與完整旗標

> tmux 3.4 共有 90 個子命令（`tmux list-commands` 列全部），這裡只列切 window 的 6 個；完整清單見 `man tmux`。
> 文法骨架：`tmux <命令> [旗標] [-t 目標]`。在 tmux 裡也可以按 `Ctrl+b :` 直接打命令（不用加前面的 `tmux`）。

| 命令（別名） | 文法 | 旗標逐一 |
|---|---|---|
| `select-window` (`selectw`) | `[-lnpT] [-t target-window]` | `-l`＝`last-window`；`-n`＝`next-window`；`-p`＝`previous-window`；`-T`：目標已經是目前的 window 時改成 `last-window`（拿來做「按同一鍵來回切」） |
| `next-window` (`next`) | `[-a] [-t target-session]` | `-a`：只找有警示的 window |
| `previous-window` (`prev`) | `[-a] [-t target-session]` | `-a`：同上，往前 |
| `last-window` (`last`) | `[-t target-session]` | 只有 `-t`；省略時是目前的 session |
| `find-window` (`findw`) | `[-iCNrTZ] [-t target-pane] match-string` | `-C` 只比對畫面內容；`-N` 只比對 window 名稱；`-T` 只比對標題；**都不給時預設 `-CNT` 三種都比**；`-i` 不分大小寫；`-r` 把字串當正規表示式（預設是 `fnmatch(3)` 萬用字元）；`-Z` 放大 pane。注意只搜**目前畫面**，不搜捲動歷史 |
| `choose-tree` | `[-GNrswZ] [-F 格式] [-f 過濾] [-K 鍵格式] [-O 排序] [-t target-pane] [template]` | `-s` session 收合；`-w` window 收合；`-Z` 放大；`-N` 不顯示預覽；`-G` 顯示 session group 裡所有 session；`-r` 反向排序；`-O` 起始排序欄位（`index`／`name`／`time`）；`-f` 過濾條件；`-F`／`-K` 每行內容與快捷鍵的格式；`template` 選定後要執行的命令（預設 `switch-client -t '%%'`） |

### `-t target-window` 怎麼寫

格式是 `session:window`，`session` 省略就是目前的 session。`window` 這段 tmux 依序這樣找（`man tmux` 的 target-window 一節）：

1. 特殊記號：`{start}`／`^` 編號最小、`{end}`／`$` 編號最大、`{last}`／`!` 上一個、`{next}`／`+` 下一個、`{previous}`／`-` 前一個
2. window 編號，例如 `demo:1`
3. window ID，例如 `@1`
4. 完整名稱，例如 `demo:build`
5. **名稱開頭**，例如 `demo:bu` 也會找到 `build`
6. `fnmatch` 萬用字元比對

前面加 `=` 表示**只接受完全相符**——這就是 `Ctrl+b 1` 綁 `:=1` 而不是 `:1` 的原因：按 `1` 就要編號 1，不能「沒有編號 1 就去找名字開頭是 1 的 window」。

### 實測

在獨立的 tmux server（`-L probe`，不影響自己正在用的 tmux；`-f /dev/null` 不讀設定檔）建一個 session、三個 window，逐一試切換命令。
`lsw -F` 印出每個 window 的「編號 名稱 標記」，`*` 是目前的 window，`-` 是 `last-window` 會回去的那個：

```bash
t(){ tmux -L probe -f /dev/null "$@"; }
ls_(){ t lsw -t demo -F '#I #W #{?window_active,*,}#{?window_last_flag,-,}'; }
t new -d -s demo -n editor; t neww -t demo -n build; t neww -t demo -n logs; ls_
echo "--- selectw -t demo:editor"; t selectw -t demo:editor; ls_
echo "--- last";                   t last -t demo; ls_
echo "--- next";                   t next -t demo; ls_
echo "--- selectw -t demo:bu";     t selectw -t demo:bu; ls_
echo "--- selectw -t 'demo:{end}'"; t selectw -t 'demo:{end}'; ls_
echo "--- selectw -t demo:9";      t selectw -t demo:9
echo "--- selectw -t demo:=bu";    t selectw -t demo:=bu
t kill-server
```

輸出：

```text
0 editor 
1 build -
2 logs *
--- selectw -t demo:editor
0 editor *
1 build 
2 logs -
--- last
0 editor -
1 build 
2 logs *
--- next
0 editor *
1 build 
2 logs -
--- selectw -t demo:bu
0 editor -
1 build *
2 logs 
--- selectw -t 'demo:{end}'
0 editor 
1 build -
2 logs *
--- selectw -t demo:9
can't find window: 9
--- selectw -t demo:=bu
can't find window: bu
```

逐段看：

- `last`：從 `editor` 回到上一個 `logs`，而且 `editor` 變成新的 `-`——所以連按 `Ctrl+b l` 就是在兩個之間來回。
- `next`：在 `logs`（編號最大）按下一個，**繞回** `editor`（編號 0）。
- `demo:bu`：只打名稱開頭也找得到 `build`（規則第 5 條）。
- `demo:=bu`：加了 `=` 只接受完全相符，`bu` 不是完整名稱，所以找不到。
- `demo:9`：不存在的編號直接報錯 `can't find window: 9`。

## 四、實用建議

- 給 window 取名字：`Ctrl+b ,` 改名（`rename-window`），之後 `Ctrl+b w` 清單、`Ctrl+b f` 搜尋、`-t 名稱開頭` 都好用很多。
- 最常用的組合：常駐的幾個 window 記編號用 `Ctrl+b 數字`；兩個之間反覆切用 `Ctrl+b l`。
- 改綁定（例如不想按 prefix）屬於設定檔範圍，例如 `bind -n M-1 select-window -t :=1` 讓 `Alt+1` 直接切；`-n` 表示不用先按 prefix。

---

## 自我測驗

> 參考答案寫在每題下面，作答時先遮住答案。

1. `Ctrl+b s` 跟 `Ctrl+b w` 都能切到某個 window，兩者差在哪？為什麼說 `s` 不是「切 window」專用的？
   > **參考答案**【[第一節](#一先釐清ctrlb-s-其實是選-session的清單)】核心關鍵：(1) 兩個都是同一個命令 `choose-tree`，差別只在一開始收合哪一層：`s` 是 `-s`（session 收合），`w` 是 `-w`（window 收合）；(2) 所以 `s` 先看到 session，要展開才看到 window，`w` 直接列出每個 session 底下的 window。
   > 詳細說明：`s` 不是「只能選 session」——同一棵樹裡展開後一樣能選 window 甚至 pane；說它不是切 window 專用，是因為它的起始畫面是為選 session 設計的。
2. 你在 window 3 寫程式、window 7 跑測試，要一直在這兩個之間來回。用 `n`／`p` 跟用 `l` 各要按幾次、哪個合適？為什麼 `l` 能做到「來回」？
   > **參考答案**【[第二節](#二所有切-window-的方法)、[第三節〈實測〉](#實測)】核心關鍵：(1) `n`／`p` 每趟要按 4 次（3→7 按 `n` 4 次，回來按 `p` 4 次）；`l` 每趟 1 次，所以用 `l`；(2) `last-window` 切回「上一個」window，而切過去之後原本的 window 變成新的「上一個」，所以連按就在兩者之間來回。
   > 詳細說明：實測裡 `last` 前後 `-` 標記會互換，就是這個原因。
3. 預設綁定 `Ctrl+b 1` 是 `select-window -t :=1` 而不是 `select-window -t :1`。如果拿掉 `=`，在什麼情況下會出現跟預期不同的結果？
   > **參考答案**【[第三節〈`-t target-window` 怎麼寫〉](#-t-target-window-怎麼寫)】核心關鍵：(1) 沒有 `=` 時，**編號 1 不存在**，tmux 會繼續往下用名稱找（完整名稱 → 名稱開頭 → 萬用字元）；(2) 所以會跳到一個名稱以 `1` 開頭的 window（例如叫 `1abc` 的 window 2），而不是報錯；加了 `=` 就只接受完全相符，找不到就報錯。
   > 詳細說明：跟「二位數」無關——`:1` 的編號比對只看編號 1，不會比對到編號 11。2026-10-12 實測：window 0 `editor`、2 `1abc`、3 `logs`，`selectw -t demo:1` 跳到 `2 1abc`；`selectw -t demo:=1` 報 `can't find window: 1`。
4. 情境題：你開了 12 個 window，想切到 window 11，但按 `Ctrl+b 1` 再按 `1` 只會到 window 1。該怎麼切？另外，如果你忘了編號、只記得那個 window 裡曾經印出 `segmentation fault`，有什麼辦法找到它？這個辦法什麼情況下會找不到？
   > **參考答案**【[第二節](#二所有切-window-的方法)、[第三節](#三背後的命令與完整旗標)】核心關鍵：(1) 按 `Ctrl+b '` 再輸入 `11` Enter（或 `Ctrl+b w` 從清單選）；(2) 按 `Ctrl+b f` 輸入 `segmentation`，`find-window` 預設同時比對名稱、標題與畫面內容；(3) 它只搜**目前可見的畫面**，那行字已經捲到歷史裡（被後面的輸出推上去）就找不到。
