# `/dev/null` 是什麼？`>/dev/null 2>&1` 到底在做什麼？

> 日期：2026-09-19
> 情境：學到 `setsid -f prog >/dev/null 2>&1` 之後，追問 `/dev/null` 和 `2>&1` 的意思（當時誤寫成 `$2>1`）
> 適用範圍：bash / zsh / POSIX sh 都通用（`&>` 簡寫除外）
> 延伸自：[從終端機啟動GUI程式並脫離終端機.md](./從終端機啟動GUI程式並脫離終端機.md)

**一句話結論：每個行程有編號 0／1／2 的三個輸入輸出口；`>` 是把某個編號改接到檔案，`2>&1` 是「讓 2 號接到 1 號**現在**接的地方」。`/dev/null` 是核心提供的黑洞裝置，寫進去的東西直接丟掉。重新導向由左到右套用，所以順序會改變結果。**

---

## 一、三個標準檔案描述符

檔案描述符（file descriptor，fd）是行程手上的一張「已開啟檔案」編號表。每個行程一開始就有三個：

| fd | 名稱 | 預設接到 | 用途 |
|----|------|---------|------|
| 0 | stdin（standard input，標準輸入） | 終端機 | 程式讀取輸入 |
| 1 | stdout（standard output，標準輸出） | 終端機 | 正常結果 |
| 2 | stderr（standard error，標準錯誤） | 終端機 | 錯誤訊息、警告、log |

stdout 和 stderr 在畫面上看起來一樣，但其實是**兩條獨立的線**，只是預設都接到同一個終端機。分成兩條，是為了讓你能把「結果」和「錯誤」分開處理（例如結果存檔，錯誤照樣顯示在畫面上）。

可以直接看：`ls -l /proc/self/fd/` 會顯示 0、1、2 各自指向哪裡（在終端機裡通常是 `/dev/pts/N`）。

---

## 二、`/dev/null`：寫入就消失的黑洞

```
$ ls -l /dev/null
crw-rw-rw- 1 root root 1, 3 ... /dev/null
```

- 開頭的 `c` 代表字元裝置（character device），不是一般檔案——它是核心提供的虛擬裝置，主／次裝置號（major/minor number）是 `1, 3`。
- **寫入**：永遠成功，資料直接丟棄，不佔磁碟空間。
- **讀取**：立刻回傳 EOF（End Of File，檔案結尾），讀到 0 個位元組（`wc -c </dev/null` 輸出 `0`）。
- 權限 `rw-rw-rw-`：所有使用者都能用。

所以 `>/dev/null` 的意思就是「把輸出丟掉」。

---

## 三、重新導向語法逐字拆解

| 寫法 | 意思 |
|------|------|
| `> file` | 等同 `1> file`：把 fd 1（stdout）改接到 `file`（覆寫） |
| `>> file` | 同上，但改成附加（append） |
| `2> file` | 把 fd 2（stderr）改接到 `file` |
| `2>&1` | 把 fd 2 改接到 **fd 1 目前接的地方**（複製 fd 1 的連接） |
| `< file` | 等同 `0< file`：從 `file` 讀取 stdin |
| `&> file` | bash／zsh 簡寫，等同 `> file 2>&1`（POSIX sh 不支援） |

**`&` 為什麼不能省：** `2>1` 裡的 `1` 會被當成**檔名**——實測 `ls /nonexist 2>1` 會在當前目錄建立一個叫 `1` 的檔案，錯誤訊息被寫進去。加了 `&`，shell 才知道 `1` 指的是 fd 1 而不是檔案。

**`$2` 是完全不同的東西：** `$2` 是位置參數（positional parameter），代表傳給腳本或函式的第二個引數（`set -- a b; echo $2` → `b`），跟 fd 無關。重新導向的數字前面不加 `$`。

---

## 四、順序很重要：由左到右套用

`2>&1` 是在**那一刻**複製 fd 1 的連接，不是永久綁在一起。所以：

```bash
cmd >/dev/null 2>&1
# 1. fd1 → /dev/null
# 2. fd2 → fd1 現在接的地方 = /dev/null
# 結果：兩個都丟掉 ✅

cmd 2>&1 >/dev/null
# 1. fd2 → fd1 現在接的地方 = 終端機
# 2. fd1 → /dev/null
# 結果：stdout 丟掉，stderr 照樣印在終端機上 ❌（如果你原本想全部丟掉）
```

實測：`ls /nonexist 2>&1 >/dev/null | sed 's/^/[piped] /'` 輸出 `[piped] ls: cannot access ...`——錯誤訊息沒被丟掉，還跑進了管線（pipe）。這個「錯誤的順序」其實有正當用途：**只把 stderr 送進管線、丟掉 stdout**。

---

## 五、常見組合

```bash
cmd >/dev/null 2>&1        # 全部靜音（最常見，也是背景啟動 GUI 程式的寫法）
cmd 2>/dev/null            # 只丟錯誤，例如 find / -name x 2>/dev/null 藏掉 Permission denied
cmd >out.log 2>&1          # 結果和錯誤都寫進同一個 log
cmd >out.log 2>err.log     # 分開存
cmd 2>&1 | less            # 錯誤也送進管線一起翻頁（管線只接 stdout，所以要先把 2 併進 1）
cmd </dev/null             # 給程式一個空的 stdin，避免它卡著等輸入
```

---

## 自我測驗

1. stdout 和 stderr 在畫面上看起來一樣，為什麼還要分成兩個 fd？
2. `2>&1` 裡的 `&` 如果漏掉寫成 `2>1`，會發生什麼事？為什麼？
3. `cmd >/dev/null 2>&1` 和 `cmd 2>&1 >/dev/null` 的結果差在哪？用「`2>&1` 複製的是什麼」解釋。
4. 情境題：你執行 `find / -name '*.conf' | grep nginx`，畫面還是被一大堆 `Permission denied` 洗版。為什麼管線沒把它們濾掉？要怎麼改？
5. 情境題：某個背景程式啟動時會讀 stdin 等使用者輸入，結果一放到背景就卡住。可以用 `/dev/null` 的哪個特性解決？
