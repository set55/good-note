# shell 的 `for` 迴圈語法

> 日期：2026-09-21
> 情境：想完整搞懂 `for` 的所有寫法，而不是只會 `for i in 1 2 3`
> 適用範圍：實測 bash 5.2、zsh 5.9、dash（POSIX sh）on Ubuntu 24.04
> 相關：[shell條件判斷測試.md](./shell條件判斷測試.md)、
> [錢字號展開的四種形式-bad-substitution的根因.md](./錢字號展開的四種形式-bad-substitution的根因.md)、
> [輸出重新導向與dev-null.md](./輸出重新導向與dev-null.md)

**一句話結論：`for` 只有三種形式，而且第一種（`for VAR in 清單`）的本質是「**把一串已經展開好的
字詞逐一指派給變數**」——所以九成的 bug 不在 `for` 本身，而在「那串字詞是怎麼被切出來的」。
記住這件事，`for f in $(ls)` 為什麼會壞、為什麼該用 `for f in *` 就一目了然。**

---

## 目錄

- [一、三種形式](#一三種形式)
- [二、清單可以從哪裡來](#二清單可以從哪裡來)
- [三、`do` / `done` 與分號規則](#三do--done-與分號規則)
- [四、`break` 與 `continue`](#四break-與-continue)
- [五、迴圈的重新導向](#五迴圈的重新導向)
- [六、七個陷阱（全部實測）](#六七個陷阱全部實測)
- [七、bash 與 zsh 的差異](#七bash-與-zsh-的差異)
- [八、迴圈變數裝的是「名字」時：間接參照](#八迴圈變數裝的是名字時間接參照)
- [九、什麼時候不該用 `for`](#九什麼時候不該用-for)
- [十、速查](#十速查)
- [自我測驗](#自我測驗)

---

## 一、三種形式

### 形式 1：`for VAR in 清單`（POSIX，到處都能用）

```sh
for i in a b c; do
    echo "$i"
done
```

**語意：把 `in` 後面展開出來的每一個字詞，依序指派給 `i`，each 跑一次迴圈本體。**

### 形式 2：`for VAR; do`（省略 `in`，隱含 `"$@"`）

```sh
f() {
    for x; do          # 等同 for x in "$@"; do
        printf '[%s] ' "$x"
    done
}
f a "b c" d            # → [a] [b c] [d]
```

**實測確認它保留了 `"b c"` 的完整性**——因為隱含的是 `"$@"`（有引號），不是 `$*`。
寫函式處理參數時這是最短也最安全的寫法。

### 形式 3：C 風格 `for ((初始; 條件; 遞增))`

```bash
for ((i = 0; i < 3; i++)); do
    echo "$i"
done
```

**這不是 POSIX**，實測：

```
bash  → 0 1 2      ✅
zsh   → 0 1 2      ✅
dash  → Syntax error: Bad for loop variable    ❌
```

**所以腳本開頭寫 `#!/bin/sh` 就不能用它**（`/bin/sh` 在 Ubuntu 上是 dash）。
要用就明確寫 `#!/bin/bash`。

裡面是**算術環境**（等同 `(( ))`），所以變數不用加 `$`、可以用 `<`／`++`／`+=`。

---

## 二、清單可以從哪裡來

這是 `for` 真正的重點——**清單的來源決定了它會不會出錯**。

| 來源 | 寫法 | 注意 |
|---|---|---|
| 字面值 | `for i in a b c` | 最安全 |
| **萬用字元（glob）** | `for f in *.txt` | **處理檔名的正確做法** |
| 大括號展開 | `for i in {1..10}` | **不能用變數**，見陷阱 2 |
| 命令替換 | `for i in $(cmd)` | **會被分詞**，見陷阱 1 |
| 變數 | `for i in $list` | 同上，會被分詞 |
| 位置參數 | `for i in "$@"` | **引號不能省** |
| 陣列（bash/zsh） | `for i in "${arr[@]}"` | 引號不能省 |
| `seq` | `for i in $(seq 1 10)` | 外部程式，比大括號慢；但**可以用變數** |
| 算術 | `for ((i=1; i<=n; i++))` | 最適合數字範圍 |

---

## 三、`do` / `done` 與分號規則

```sh
for i in a b; do echo "$i"; done        # 單行：兩個分號
for i in a b
do
    echo "$i"
done                                     # 多行：不用分號
```

**為什麼單行時 `do` 前面要分號**：`do` 是一個**關鍵字**，shell 需要知道前面的清單已經結束。
換行本身就是分隔符，所以多行寫法不需要分號。

這和 `if ...; then` 是同一條規則——
`[` 其實是命令、`then`／`do` 是關鍵字，前面必須有命令結束符。
（完整推導見 [shell條件判斷測試.md](./shell條件判斷測試.md)。）

**迴圈的離開碼 = 最後一次執行的那個命令的離開碼**；清單是空的時候迴圈本體一次都不跑，離開碼是 0。

---

## 四、`break` 與 `continue`

| 寫法 | 作用 |
|---|---|
| `break` | 跳出**當前**迴圈 |
| `break N` | **跳出 N 層** |
| `continue` | 跳過本次，進入下一輪 |
| `continue N` | 跳到第 N 層外的下一輪 |

實測 `break 2`：

```bash
for i in 1 2; do
  for j in a b; do
    printf '%s%s ' "$i" "$j"
    [ "$j" = a ] && break 2
  done
done
# 輸出：1a        ← 兩層一起跳出，只印了一次
```

**`break N` 是很多人不知道的寫法**，比用旗標變數再逐層 break 乾淨得多。

---

## 五、迴圈的重新導向

`do ... done` 是一個複合命令，**可以整個接重新導向或管線**：

```sh
while read -r line; do echo "$line"; done < input.txt     # 整個迴圈讀檔
for i in a b; do echo "$i"; done > out.txt                # 整個迴圈的輸出
for i in a b; do echo "$i"; done | sort                   # 接管線
```

**把 `< file` 放在 `done` 後面，比在迴圈裡每次 `cat` 有效率得多**，
而且 `read` 才能正確地一行一行讀。

---

## 六、七個陷阱（全部實測）

### 陷阱 1：`for f in $(ls)` —— 檔名含空白就爆炸

```bash
$ touch "my file.txt" other.txt

$ for f in $(ls); do printf '[%s] ' "$f"; done
[my] [file.txt] [other.txt]        # ❌ 被空白切成三個

$ for f in *; do printf '[%s] ' "$f"; done
[my file.txt] [other.txt]          # ✅ glob 不會分詞
```

**根因**：命令替換的輸出會經過**分詞（word splitting）**，依 `IFS`（預設空白、tab、換行）切開。
**glob 展開的結果不會分詞**，所以檔名含空白也安全。

**規則：處理檔案一律用 glob，不要用 `$(ls)`。**

### 陷阱 2：`{1..$n}` 在 bash 不會展開

```bash
$ bash -c 'n=3; for i in {1..$n}; do printf "%s " "$i"; done'
{1..3}                             # ❌ 印出字面值

$ zsh -c 'n=3; for i in {1..$n}; do printf "%s " "$i"; done'
1 2 3                              # ✅ zsh 可以
```

**根因**：bash 的**大括號展開發生在變數展開之前**，所以看到 `{1..$n}` 時 `$n` 還沒被換掉，
不構成合法的數字範圍，於是原樣輸出。

**可攜的解法：**

```bash
for ((i = 1; i <= n; i++)); do ... done      # 最推薦
for i in $(seq 1 "$n"); do ... done          # 需要外部 seq
```

### 陷阱 3：glob 沒配對到檔案時，bash 與 zsh 行為完全不同

```bash
$ bash -c 'for f in *.nope; do echo "[$f]"; done'
[*.nope]                           # ❌ 把樣式本身當成檔名跑了一輪

$ zsh -c 'for f in *.nope; do echo "[$f]"; done'
zsh: no matches found: *.nope      # ❌ 整個指令直接失敗，迴圈一次都沒跑
```

**兩個都不是你要的**。解法：

```bash
shopt -s nullglob      # bash：沒配對到就展開成空清單（迴圈不跑）
setopt null_glob       # zsh 對應寫法
# 或在迴圈裡防禦
for f in *.nope; do [ -e "$f" ] || continue; ...; done
```

### 陷阱 4：迴圈變數沒加引號

```bash
for f in *; do
    cp $f /tmp/      # ❌ 檔名含空白就拆開了
    cp "$f" /tmp/    # ✅
done
```

`for f in *` 本身安全，但**用的時候忘記引號一樣會壞**。

### 陷阱 5：管線讓迴圈跑在子 shell（bash 才有）

```bash
$ bash -c 'n=0; printf "a\nb\n" | while read -r l; do n=$((n+1)); done; echo "n=$n"'
n=0                                # ❌ 變數改了卻沒帶回來

$ bash -c 'n=0; while read -r l; do n=$((n+1)); done < <(printf "a\nb\n"); echo "n=$n"'
n=2                                # ✅ 改用行程替換

$ zsh -c 'n=0; printf "a\nb\n" | while read -r l; do n=$((n+1)); done; echo "n=$n"'
n=2                                # ✅ zsh 不一樣！
```

**根因**：bash 預設讓管線的**每一段都在子 shell 執行**，子 shell 的變數改動不會傳回父行程。
**zsh 讓最後一段在當前 shell 執行**，所以沒這個問題。

bash 的解法：`< <(...)`（行程替換）、`shopt -s lastpipe`、或把結果用其他方式帶出來。

### 陷阱 6：在迴圈裡修改正在迭代的清單

```bash
for f in *.txt; do
    mv "$f" "$f.bak"      # 危險：glob 在迴圈開始前就展開完了，
done                      # 所以不會無限迴圈；但如果是 *.bak 就會出事
```

**glob 是在迴圈開始前一次展開完的**，所以上面這個其實安全。
但若樣式會配對到新產生的檔案（例如 `for f in *; do cp "$f" "$f.copy"; done`），
就要先把清單存進陣列再跑。

### 陷阱 7：用 `for` 讀檔案的每一行

```bash
for line in $(cat file); do ... done        # ❌ 依空白切，不是依行切
while read -r line; do ... done < file      # ✅
```

見第八節。

---

## 七、bash 與 zsh 的差異

**你的互動 shell 是 zsh、但腳本常以 bash 執行**，這幾個差異會咬人：

| 情況 | bash | zsh |
|---|---|---|
| `{1..$n}` | ❌ 字面值 | ✅ 展開 |
| glob 無配對 | 傳回樣式字面值 | **整個指令失敗** |
| `$var` 未加引號 | **分詞** | **不分詞** |
| `$(cmd)` 未加引號 | 分詞 | **一樣分詞** |
| 管線最後一段 | 子 shell | **當前 shell** |
| C 風格 `for (( ))` | ✅ | ✅ |

**所以在 zsh 互動下測通的迴圈，貼進 `#!/bin/bash` 腳本可能行為不同。**
測試腳本要用它實際會跑的那個 shell 測。

---

## 八、迴圈變數裝的是「名字」時：間接參照

這是 `for` 最常見的進階需求：**清單裡放的是一串變數名，你要對「那個名字所指的變數」動手。**

```sh
for i in http_proxy all_proxy; do
    #  $i 是 "http_proxy" 這個名字
    #  但你想要的往往是它的「值」或想「改」它
done
```

shell 把這件事叫做**間接參照（indirect expansion）**，**bash 與 zsh 的語法完全不同**。
以下四種操作全部在兩個 shell 實測過。

### 操作 1：`unset` —— 最簡單，因為它本來就吃名字

```sh
for i in A B; do unset "$i"; done      # bash ✅ zsh ✅
```

**`unset`／`export`／`readonly` 這類指令的參數本來就是「變數名」**，所以直接傳 `"$i"` 就對了，
不需要任何間接參照語法。**這也是為什麼 `unset "$i"` 能動，而 `unset $A` 不能**——
前者傳的是名字，後者傳的是值。

### 操作 2：讀出它的值

| shell | 寫法 |
|---|---|
| bash | `${!i}` |
| zsh | `${(P)i}` |
| POSIX sh | `eval "v=\$$i"` |

**兩邊互不相容**，實測：

```
bash  ${!i}   → hello          ✅
zsh   ${!i}   → bad substitution   ❌
zsh   ${(P)i} → hello          ✅
bash  ${(P)i} → bad substitution   ❌
```

（又是 `bad substitution`——理由見
[錢字號展開的四種形式](./錢字號展開的四種形式-bad-substitution的根因.md)：
`${...}` 裡只能放該 shell 認得的參數表達式。）

### 操作 3：設定它的值

| shell | 寫法 |
|---|---|
| bash | `printf -v "$i" '%s' "新值"` 或 `declare -g "$i=新值"` |
| zsh | `typeset -g "$i=新值"` 或 `: ${(P)i::=新值}` |
| 兩邊皆可 | `eval "$i=\"新值\""` |

`-g` 是「global」——在函式裡沒加它，設出來的變數只有函式內看得到。

**`eval` 能跨 shell，但它會把字串當程式碼執行**：值裡若含 `;`、`$(...)`、反引號就會被執行。
**只在值完全由你掌控時才用**，處理外部輸入一律改用 `printf -v`／`typeset -g`。

### 操作 4：判斷「這個名字的變數有沒有設定」

```sh
[[ -v $i ]]        # bash ✅ zsh ✅  ← 難得兩邊一樣
```

實測兩個 shell 都正確分辨出「有設」與「沒設」。（POSIX sh 沒有 `-v`，要用
`eval "[ -n \"\${$i+x}\" ]"`。）

### 完整實例：印出目前值再清掉

```sh
for i in http_proxy https_proxy all_proxy no_proxy; do
    if [[ -v $i ]]; then
        printf '清掉 %-12s（原值 %s）\n' "$i" "${(P)i}"    # zsh；bash 用 ${!i}
        unset "$i"
    else
        printf '%-12s 本來就沒設\n' "$i"
    fi
done
```

實測輸出：

```
清掉 http_proxy  （原值 http://192.168.1.1:8880）
清掉 all_proxy   （原值 socks5://192.168.1.1:1080）
結果：http_proxy=空 all_proxy=空
```

### 但多數情況你該用關聯陣列

**需要間接參照，通常代表資料結構選錯了。** 如果你本來就要管理「名字 → 值」的對應，
直接用關聯陣列（associative array）比一堆散裝變數清楚得多：

```bash
# bash
declare -A m=([http_proxy]=http://a:8880 [all_proxy]=socks5h://a:1080)
for k in "${!m[@]}"; do printf '%s = %s\n' "$k" "${m[$k]}"; done
```

```zsh
# zsh
typeset -A m=(http_proxy http://a:8880 all_proxy socks5h://a:1080)
for k in "${(@k)m}"; do printf '%s = %s\n' "$k" "${m[$k]}"; done
```

**注意取鍵的語法也不同**（bash `${!m[@]}`、zsh `${(@k)m}`），而且**兩者都不保證順序**——
實測 zsh 就把 `all_proxy` 排在 `http_proxy` 前面。需要固定順序就自己另外給一個名字清單。

**判斷原則：要「讀寫別人名下的變數」才用間接參照；只是要一組成對資料，用關聯陣列。**

---

## 九、什麼時候不該用 `for`

**要逐行處理文字時，用 `while read` 而不是 `for`：**

```bash
while IFS= read -r line; do
    printf '[%s]\n' "$line"
done < file
```

三個部分各有用途：

| 部分 | 為什麼 |
|---|---|
| `IFS=` | **不要去掉行首行尾的空白** |
| `-r` | **不要把反斜線當跳脫字元** |
| `< file` 放在 `done` 後 | 一次開檔，而且不會有子 shell 問題 |

**處理檔名清單、而且可能含換行時**，用 `find -print0` 配 `read -d ''`：

```bash
while IFS= read -r -d '' f; do
    printf '[%s]\n' "$f"
done < <(find . -name '*.txt' -print0)
```

---

## 十、速查

```bash
# 三種形式
for i in a b c;        do ...; done      # POSIX，清單
for i;                 do ...; done      # 隱含 "$@"
for ((i=0; i<n; i++)); do ...; done      # C 風格（bash/zsh，非 POSIX）

# 常見清單來源
for f in *.txt                  # glob ← 處理檔案用這個
for i in {1..10}                # 大括號（不能用變數）
for i in $(seq 1 "$n")          # 可以用變數
for x in "$@"                   # 位置參數（引號不能省）
for x in "${arr[@]}"            # 陣列（引號不能省）

# 流程控制
break / break N / continue / continue N

# 重新導向
for ...; do ...; done < in.txt > out.txt
while IFS= read -r line; do ...; done < file        # 逐行讀
while IFS= read -r -d '' f; do ...; done < <(find . -print0)   # 檔名含換行

# 防禦
shopt -s nullglob     # bash：glob 無配對時展開成空
setopt null_glob      # zsh
```

---

## 自我測驗

1. `for f in $(ls)` 和 `for f in *` 在檔名含空白時結果不同。用「分詞」解釋為什麼，
   並說出哪一個才是對的。
2. `n=3; for i in {1..$n}` 在 bash 裡印出 `{1..3}`。為什麼？從「展開的先後順序」回答，
   並給出兩個可攜的替代寫法。
3. `for f in *.nope`（沒有任何檔案符合）時，bash 與 zsh 的行為分別是什麼？
   兩者都不理想，該怎麼處理？
4. `printf 'a\nb\n' | while read -r l; do n=$((n+1)); done; echo "$n"` 在 bash 印 0、
   在 zsh 印 2。為什麼？bash 有哪兩種解法？
5. 情境題：你要對某個目錄下所有 `.log` 檔案逐一處理，檔名可能含空白甚至換行，
   而且要統計處理了幾個檔案、迴圈結束後印出來。請寫出這段程式碼，
   並說明你為什麼不用 `for f in $(find ...)`。
