# 如何讓 tmux 每次都用當前目錄名稱建立 session？——`${PWD##*/}` 參數展開

> 日期：2026-09-10
> 情境：習慣用 `tmux new -d -s <name>` 開 session，但每次都要手打名稱；想讓名稱自動等於當前目錄名
> 對應檔案：`~/.zshrc`（個人環境設定，不入 git）
> 適用範圍：tmux 3.4；`${PWD##*/}` 適用 bash / zsh / dash 等所有 POSIX shell

> **一句話結論：`${PWD##*/}` 是 shell 內建的字串切除（parameter expansion，參數展開），把 `$PWD` 從左邊砍掉「最長的、符合 `*/` 的前綴」，剩下的就是目錄名——不開子行程、不怕空白，比 `$(basename "$PWD")` 更該是預設選擇。**

---

## 目錄

1. [先給答案](#一先給答案)
2. [`${PWD##*/}` 逐字拆解](#二pwd-逐字拆解)
3. [四個切除運算子：`#` `##` `%` `%%`](#三四個切除運算子------)
4. [為什麼不用 `basename`](#四為什麼不用-basename)
5. [包成一個函式才真的「每次都能用」](#五包成一個函式才真的每次都能用)
6. [tmux 對 session 名稱的限制與陷阱](#六tmux-對-session-名稱的限制與陷阱)
7. [邊界情況](#七邊界情況)
8. [自我測驗](#八自我測驗)

---

## 一、先給答案

```bash
tmux new -d -s "${PWD##*/}"
```

在 `/home/set/work/Projects/github/good-note` 執行，就會得到名為 `good-note` 的 session。

雙引號不能省：目錄名可能含空白（例如 `my app`），沒引號的話 shell 會把它切成兩個參數，
`-s` 只吃到 `my`，`app` 變成要在 session 裡執行的 shell-command。

---

## 二、`${PWD##*/}` 逐字拆解

這不是 tmux 的語法，而是 shell 的**參數展開（parameter expansion）**。拆成四塊看：

| 片段 | 意思 |
|------|------|
| `PWD` | shell 自動維護的環境變數，值是當前工作目錄的絕對路徑 |
| `##` | 運算子：從**開頭**切掉符合樣式的部分，取**最長**匹配 |
| `*/` | 樣式（pattern）：任意字元若干個，最後是一個斜線 |
| `${...}` | 參數展開的外框，結果是切除後剩下的字串（原變數不會被改動） |

以 `/home/set/work/Projects/github/good-note` 為例，符合 `*/` 的前綴有很多個候選：

```
/
/home/
/home/set/
...
/home/set/work/Projects/github/      ← 最長的那個
```

`##` 取最長，所以切掉最後那一行，剩下 `good-note`。

關鍵理解：**`*/` 裡的 `*` 是 shell 的通用比對樣式（glob），不是正規表達式（regular expression）。**
glob 的 `*` 本身就能跨斜線（`/` 在 glob 樣式比對中不是特殊字元，只有在檔名展開時才有目錄邊界的概念），
所以它才吃得掉中間所有層級。

---

## 三、四個切除運算子：`#` `##` `%` `%%`

記法：鍵盤上 `#` 在 `$` 左邊（切前面），`%` 在 `$` 右邊（切後面）；單個=最短，雙個=最長。

以 `p=/home/set/work/Projects/github/good-note` 實測：

```bash
${p##*/}    # good-note                              ← 切前面、最長 = basename
${p#*/}     # home/set/work/Projects/github/good-note ← 切前面、最短（只切掉第一個 /）
${p%/*}     # /home/set/work/Projects/github          ← 切後面、最短 = dirname
${p%%/*}    # （空字串）                               ← 切後面、最長（整條都被吃掉）
```

實務上最常用的另一組是切副檔名：

```bash
f=archive.tar.gz
${f%.*}     # archive.tar   ← 切最後一個副檔名
${f%%.*}    # archive       ← 切掉所有副檔名
${f##*.}    # gz            ← 只留最後一個副檔名
```

> zsh 另外有更短的修飾子（modifier）寫法：`${PWD:t}`（tail）等於 `${PWD##*/}`，
> `${PWD:h}`（head）等於 `${PWD%/*}`。好用，但**只有 zsh 有**，寫進要給 bash/sh 跑的腳本會壞掉。

---

## 四、為什麼不用 `basename`

`$(basename "$PWD")` 結果通常一樣，但有三個實質差別：

1. **成本**：`basename` 是 `/usr/bin/basename`，一個外部程式。每次呼叫要 fork + exec 一個子行程；
   參數展開完全在 shell 內完成。單次跑看不出差異，寫在迴圈或 shell prompt 裡就會有感。
2. **引號**：`$(basename "$PWD")` 裡面那層引號漏掉，遇到含空白的路徑就錯；
   `${PWD##*/}` 只需要外面一層引號。
3. **根目錄的行為不同**：在 `/` 之下 `basename /` 回傳 `/`，而 `${PWD##*/}` 回傳**空字串**。
   這不是誰對誰錯，是兩種語意，要自己決定哪種對你有利（見下方邊界情況）。

反過來說，`basename` 可以處理「任意一段路徑字串」並支援 `-s .txt` 去副檔名，
在處理非 `$PWD` 的複雜路徑時仍有它的位置。這裡的結論只是：**取當前目錄名這件事，不值得開一個子行程。**

---

## 五、包成一個函式才真的「每次都能用」

原始需求是「每次都」自動命名，所以重點是寫進 `~/.zshrc`（或 `~/.bashrc`）：

```bash
# 用當前目錄名開一個 detached session；已存在就直接進去
tn() {
  local name="${PWD##*/}"
  name="${name//[.:]/_}"            # tmux 不接受 . 與 :（見下一節）
  if [ -z "$name" ]; then
    echo "tn: 目前目錄無法產生 session 名稱（在 / 底下？）" >&2
    return 1
  fi
  if tmux has-session -t="$name" 2>/dev/null; then
    tmux attach -t "$name"          # 已存在 → 附掛進去，不要再 new
  else
    tmux new -d -s "$name" && tmux attach -t "$name"
  fi
}
```

`-t=` 的等號寫法會做精確比對；`tmux has-session -t name` 是前綴比對，
`good` 會誤判成已存在 `good-note`，開發機上很容易踩到。

不想寫函式、只要一行別名也可以，但要知道它的行為：

```bash
alias tn='tmux new -d -s "${PWD##*/}"'
```

單引號是刻意的——要讓 `${PWD##*/}` 在**使用時**才展開；用雙引號會在定義 alias 的當下就被展開成固定的字串。

---

## 六、tmux 對 session 名稱的限制與陷阱

- **`.` 和 `:` 會被換成 `_`。** tmux 用 `session:window.pane` 的語法定位目標，所以名稱裡不能有這兩個字元。
  實測目錄 `my.app v1` 建出來的 session 名稱是 `my_app v1`——tmux 自動改名，不會報錯。
  這表示「目錄名」和「session 名」可能不相等，後續想用 `tmux attach -t "${PWD##*/}"` 就會找不到，
  所以上面的函式先自己做了替換，讓兩邊的規則一致。
- **空白可以存在**，但每次打名稱都要加引號，建議也一併換成 `_`。
- **名稱重複會直接失敗**：`duplicate session: dup`，而且離開狀態碼非 0。

  ```bash
  $ tmux new -d -s dup
  $ tmux new -d -s dup
  duplicate session: dup
  ```
- **`tmux new -A -s <name>`（存在就改為 attach）看起來是完美解，但和 `-d` 合不起來。**
  man page 寫明：給了 `-A`，在「已存在」那條路徑上它等同 `attach-session`，
  此時 `-d` 的意義變成 attach 的 `-d`（detach 其他 client），不再是「不要附掛」。
  所以 `tmux new -A -d -s x` 在 session 已存在、又沒有終端機（腳本／cron／hook）的情況下會失敗：

  ```
  open terminal failed: not a terminal
  ```

  互動式使用想要「有就進、沒有就開」，`tmux new -A -s "${PWD##*/}"`（不要 `-d`）是對的；
  要寫進腳本且堅持 detached，就用上面 `has-session` 的分支寫法。

---

## 七、邊界情況

| 情況 | `${PWD##*/}` 的結果 | 後果 |
|------|--------------------|------|
| `cd /` | 空字串 | `tmux new -d -s ""` → `invalid session: `，session 不會建立 |
| `cd ~` | `set`（家目錄名） | 能用，但所有使用者家目錄都長得像，辨識度低 |
| 兩個不同路徑、同一個目錄名（`a/build`、`b/build`） | 都是 `build` | 第二次必定 `duplicate session`——這是這個做法最常見的實際痛點 |
| 目錄名含 `.`（`my.app`） | `my.app` | tmux 自動改成 `my_app`，名稱與目錄名不再一致 |

第三列值得補一個解法：名稱改用「父目錄-目錄」兩層，碰撞機率大降，而且仍然一行搞定。

```bash
name="${PWD#"${PWD%/*/*}/"}"      # a/build → 取最後兩層
name="${name//\//-}"              # 斜線換成 - → a-build
```

---

## 參考

- 參數展開的 `*` 與引號規則，和 `[ ]` 測試命令同屬「shell 把東西當命令與參數處理」這套心智模型：
  [shell條件判斷測試.md](./shell條件判斷測試.md)
- session 建好之後要改名：[tmux重命名session.md](./tmux重命名session.md)

---

## 八、自我測驗

1. `${PWD##*/}` 和 `${PWD%/*}` 分別取到路徑的哪一段？從運算子的符號本身推導出答案，而不是背結果。
2. 為什麼 `*/` 這個樣式裡的 `*` 能跨越好幾層斜線？如果它的語意和正規表達式一樣，結果會變成什麼？
3. 既然 `$(basename "$PWD")` 的輸出通常相同，為什麼仍該優先選 `${PWD##*/}`？至少說出兩個理由，
   並指出一個「在 `/` 之下兩者行為不同」的具體差異。
4. 情境題：你把 `alias tn='tmux new -A -d -s "${PWD##*/}"'` 寫進 `~/.zshrc`，在專案目錄第一次用沒問題，
   第二次卻出現 `open terminal failed: not a terminal`。請說明這是 `-A` 與 `-d` 的哪一條交互作用造成的，
   以及你會怎麼改。
5. 情境題：你有 `~/work/api/build` 和 `~/work/web/build` 兩個目錄，都想用這套自動命名開 session。
   會發生什麼事？錯誤訊息會是什麼？提出一個仍然「只看當前目錄就能決定名稱」的修正方案。

## tmux 的相關命令與完整參數

> **tmux 有 90 個子命令**（`tmux list-commands` 可列出全部），這裡只列 session 操作這一組；
> 完整清單見 `man tmux`。查單一命令的文法用 `tmux list-commands | grep <命令>`。

| 命令（別名） | 完整文法 |
|---|---|
| `new-session` (`new`) | `[-AdDEPX] [-c 起始目錄] [-e 環境變數] [-F 格式] [-f 旗標] [-n 視窗名] [-s session名] [-t 目標session] [-x 寬] [-y 高] [shell 指令]` |
| `rename-session` (`rename`) | `[-t 目標session] <新名字>` |
| `has-session` (`has`) | `[-t 目標session]` |
| `attach-session` (`attach`) | `[-dErx] [-c 工作目錄] [-f 旗標] [-t 目標session]` |

**`new-session` 的旗標逐一**

| 旗標 | 作用 |
|---|---|
| `-A` | **已存在同名 session 就 attach 而不是建新的**（等同 `new-session` + `has-session` 的組合） |
| `-d` | **建立後不要 attach**（腳本必用） |
| `-D` | 配合 `-A` 時，把其他客戶端從該 session 踢掉 |
| `-E` | 不套用 `update-environment` |
| `-P` | 印出新 session 的資訊 |
| `-X` | 不使用 `destroy-unattached` |
| `-c <目錄>` | 起始工作目錄 |
| `-e <k=v>` | 設定環境變數 |
| `-F <格式>` | 配合 `-P` 的輸出格式 |
| `-n <名稱>` | 第一個視窗的名稱 |
| `-s <名稱>` | **session 名稱** |
| `-t <目標>` | 分組到既有 session |
| `-x` / `-y` | 指定寬高（detached 時有用） |

> **`-A` 與 `-d` 一起用會互相影響**：`-A` 時若 session 已存在，`-d` 的語意變成「attach 時
> detach 其他客戶端」。腳本要冪等地建 session，用 `has-session || new-session -d` 最不會出錯。

