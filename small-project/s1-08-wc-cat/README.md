# s1-08：寫自己的 `wc` 與 `cat`——命令列參數、讀到 `EOF`、`errno` 錯誤處理

> 對應課程：第 ① 站第 8 課（本站專案）
> 課程筆記：[寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html)
> 自我測驗：寫在同資料夾的 [quiz.md](./quiz.md)

## 說明

這一課是第 ① 站的畢業專案：**自己從零寫**兩支命令列工具，`mycat` 和 `mywc`，行為要跟 GNU `cat`、`wc` 一致（範圍見每題的規格）。

- 每一題一個資料夾（`hw1/`～`hw3/`），程式檔自己建。題目給的測試指令照下，**輸出要貼進作答框**。
- 編譯一律用 `gcc -Wall -Wextra`，**有警告就要處理**，不能忽略。`-Wextra` 是在 `-Wall` 之外再多開一批警告（例如有號和無號混在一起比較、函式參數沒用到），寫命令列工具時很容易踩到。
- 測試用的檔案都用題目裡的 `printf` 指令建（`.txt`、`.bin` 不會進 git，白名單只收原始碼與 `.md`）。
- 筆記裡的範例（`args.c`、`getbytes.c`、`openfail.c`、`countch.c`）可以參考，但它們都只處理**一個**檔案或只讀 `stdin`；作業要處理多個檔案、`-`、總計、錯誤後繼續，要自己設計。
- **只能用筆記教過的函式**（`fopen`、`fclose`、`getc`、`putc`、`printf`、`fprintf`、`perror`、`strerror`、`ferror`、`isspace`、`isprint`、`strcmp`），不要用 `fread`／`read` 之類一次讀一塊的寫法——這一課要練的是一個 byte 一個 byte 處理。`strcmp`（`<string.h>`）比較兩個字串，相等回傳 0，例如 `strcmp(argv[i], "-") == 0` 判斷參數是不是 `-`。

每題「解釋原因」的地方都標了答案的來源：
- 【筆記第 X 節】：本課教過，回筆記那一節找。
- 【新觀念，先猜】：還沒教過，寫下你的猜測就好，猜錯不算錯。

**「先預測」的題目，一定要先把預測寫進作答框，再執行。** 預測錯了沒關係，把錯在哪裡寫下來，這才是練習的重點。

**實驗要跑到最後一步、貼上證據**：「沒有錯誤訊息」不代表結果對，每一題都要貼上執行的輸出與 `exit=` 的值。

**怎麼作答**：每個小題下面有作答框，直接寫在裡面（把 `YYYY-MM-DD` 換成作答日期）。程式碼本身寫在 `hw1/` 等資料夾的 `.c` 檔裡，作答框貼**測試的輸出**與**解釋**。批改與參考答案寫在作答框正下方，不重答。

---

## 筆記的範例程式

課程筆記裡每個實驗的原始碼都放在 [`examples/`](./examples/)，一個實驗一個子資料夾，可以直接跟著筆記重做。**它們是範例，不是作業**；作業寫在 `hw1/`～`hw3/`，檔名跟範例不重複。

| 資料夾 | 檔案 | 筆記裡的位置 | 示範什麼 |
|---|---|---|---|
| [`examples/args/`](./examples/args/) | [`args.c`](./examples/args/args.c) | [第二節](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#二命令列參數argc-與-argv) | 印出 `argc` 與每一格 `argv`；引號、萬用字元、`-`、空字串 |
| [`examples/eof/`](./examples/eof/) | [`getbytes.c`](./examples/eof/getbytes.c)、[`charbug.c`](./examples/eof/charbug.c) | [第三節](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#三讀到-eofgetc-為什麼回傳-int) | 用 `int` 接 `getchar()` 數 byte；用 `char` 接時遇到 `0xff` 提早停、`-funsigned-char` 時停不下來 |
| [`examples/errno/`](./examples/errno/) | [`openfail.c`](./examples/errno/openfail.c)、[`stale.c`](./examples/errno/stale.c)、[`errlist.c`](./examples/errno/errlist.c) | [第四節](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#四開檔與錯誤fopenerrnoperrorstrerror) | 四種開檔結果與 `perror`；成功的呼叫不清 `errno`；印出全部 131 個錯誤碼 |
| [`examples/countch/`](./examples/countch/) | [`countch.c`](./examples/countch/countch.c) | [第六節](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#六接起來countch-與用-strace-看它做了哪些系統呼叫) | 一個檔案、一個字元的計數工具；`ferror`；用 `strace` 看 `openat`／`read`／`write` |
| [`examples/ctype/`](./examples/ctype/) | [`spaces.c`](./examples/ctype/spaces.c)、[`classes.c`](./examples/ctype/classes.c) | [第七節](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#七wc-與-cat-的精確行為作業的規格)〈`wc` 數的三個數字〉 | `isspace` 認定的 6 個空白 byte；0～255 分成空白、可印、其他三類 |

---

## 1. `mycat`：依序原樣輸出

在 `hw1/` 寫 `mycat.c`，編譯：`gcc -Wall -Wextra mycat.c -o mycat`。

**規格**（跟筆記[第七節](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#七wc-與-cat-的精確行為作業的規格)〈`cat`：依序把每個檔案的 byte 原樣寫到 `stdout`〉一致）：

1. 依序處理每個參數；每個檔案的每個 byte **原樣**寫到 `stdout`，不加、不改。
2. 沒有參數時讀 `stdin`；參數是 `-` 時讀 `stdin`（`-` 可以出現在任何位置）。
3. 打不開或讀的時候出錯的檔案：印 `mycat: <檔名>: <錯誤說明>` 到 **`stderr`**（錯誤說明就是 `strerror(errno)` 的字串），**繼續處理下一個**。
4. 全部成功回傳 0；有任何一個失敗回傳 1。
5. 打開的檔案都要 `fclose`（`stdin` 不用關）。

建測試檔：

```sh
cd hw1
printf 'hello world\n' > one.txt
printf 'no newline at end' > nonl.txt
printf 'AB\xffCD\n' > ff.bin
mkdir -p adir
```

### 1-1. 寫好 `mycat.c`，執行下面每一行，貼上**完整輸出**（含 `exit=`）。再貼上同樣的指令把 `./mycat` 換成 `cat` 的輸出，逐行比對有沒有不一樣的地方（錯誤訊息開頭的 `mycat`／`cat` 不同是正常的）。最後一行用 `cmp` 確認 byte 完全一樣，貼上結果。【筆記第七節〈`cat`：依序把每個檔案的 byte 原樣寫到 `stdout`〉】

```sh
./mycat one.txt nonl.txt one.txt; echo "exit=$?"
./mycat one.txt nofile.txt one.txt; echo "exit=$?"
printf 'from stdin\n' | ./mycat one.txt - one.txt; echo "exit=$?"
printf 'only stdin\n' | ./mycat; echo "exit=$?"
./mycat adir; echo "exit=$?"
./mycat ff.bin | od -c
for f in one.txt nonl.txt ff.bin; do ./mycat "$f" | cmp - "$f" && echo "$f 一樣"; done
```

**第 1 次（YYYY-MM-DD）**
```
```

### 1-2. 執行 `./mycat one.txt nofile.txt > out.txt; echo "exit=$?"`，貼上終端機上看到的內容，再貼上 `cat out.txt`。錯誤訊息出現在哪裡、沒出現在哪裡？如果規格第 3 點改成「錯誤訊息印到 `stdout`」，使用者打 `./mycat *.txt > all.txt` 會遇到什麼麻煩？【筆記第五節〈結果寫 `stdout`，錯誤寫 `stderr`〉】

**第 1 次（YYYY-MM-DD）**
```
```

### 1-3. 先預測：把你程式裡接 `getc` 回傳值的變數**暫時改成 `char`**，重新編譯，執行 `./mycat ff.bin | od -c; echo "exit=$?"`。輸出會有幾個 byte、是哪幾個？寫下預測後再執行，貼上結果與編譯有沒有警告。為什麼？做完改回 `int`。【筆記第三節〈錯誤示範：用 `char` 接 `getchar()`〉】

**第 1 次（YYYY-MM-DD）**
```
```

### 1-4. 先預測：`./mycat one.txt nofile.txt one.txt 2>&1 | cat` 印出的三行，順序會跟在終端機直接執行 `./mycat one.txt nofile.txt one.txt` 一樣嗎？寫下預測後兩個都執行，貼上結果。為什麼一樣（或不一樣）？【筆記第五節〈輸出順序為什麼會變〉】

**第 1 次（YYYY-MM-DD）**
```
```

---

## 2. `mywc`：數行、字、byte

在 `hw2/` 寫 `mywc.c`，編譯：`gcc -Wall -Wextra mywc.c -o mywc`。

**規格**（跟筆記[第七節](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#七wc-與-cat-的精確行為作業的規格)〈`wc` 數的三個數字〉〈多個檔案、標準輸入、錯誤〉一致；數字要跟 **`LC_ALL=C wc`** 完全相同）：

1. 每個檔案印一行：`printf("%7ld %7ld %7ld %s\n", 行數, 字數, byte 數, 檔名)`。
2. **行數**＝ `\n` 的個數；**字數**＝照筆記第七節〈第三種 byte：不是空白、也不可印〉的規則：被空白（`isspace`）隔開、裡面至少有一個可印字元（`isprint`）的一段算一個字，其他 byte 不算字也不切字；**byte 數**＝總共幾個 byte。
3. 沒有參數時讀 `stdin`，**不印檔名**（`printf("%7ld %7ld %7ld\n", ...)`）；參數是 `-` 時讀 `stdin`，檔名印 `-`。
4. 兩個以上的參數時，最後多印一行總計，檔名欄印 `total`；總計不含失敗的檔案。
5. 打不開或讀的時候出錯的檔案：印 `mywc: <檔名>: <錯誤說明>` 到 `stderr`，**不印**那個檔案的數字，繼續處理下一個；有任何失敗就回傳 1，否則 0。

建測試檔：

```sh
cd hw2
printf 'hello world\n' > one.txt
printf 'no newline at end' > nonl.txt
printf '  lots   of\t\tspace  \n\n\nend\n' > spaces.txt
: > empty.txt
printf 'a\r\nb c\t\n' > crlf.txt
printf '\xe4\xb8\xad\xe6\x96\x87 \xff\xfe\n' > high.bin
mkdir -p adir
```

### 2-1. 寫好 `mywc.c`，執行下面每一行，貼上**完整輸出**（含 `exit=`），再貼上把 `./mywc` 換成 `LC_ALL=C wc` 的輸出，逐行比對三個數字（欄寬不同是正常的）。有不一樣的，找出原因、修好再貼最後一次的結果。【筆記第七節〈`wc` 數的三個數字〉〈多個檔案、標準輸入、錯誤〉】

```sh
for f in one.txt nonl.txt spaces.txt empty.txt crlf.txt high.bin; do ./mywc "$f"; done
./mywc one.txt nonl.txt spaces.txt; echo "exit=$?"
./mywc one.txt nofile.txt nonl.txt; echo "exit=$?"
printf 'a b\nc\n' | ./mywc; echo "exit=$?"
printf 'a b\nc\n' | ./mywc - one.txt; echo "exit=$?"
./mywc adir one.txt; echo "exit=$?"
```

**第 1 次（YYYY-MM-DD）**
```
```

### 2-2. `nonl.txt` 明明有一行字，`mywc` 和 `wc` 的行數都是 0；`spaces.txt` 只有兩行有字，行數卻是 4。這兩個數字為什麼是對的？如果你想讓 `nonl.txt` 算成 1 行，要多判斷什麼情況（用文字說明就好，不用改程式）？【筆記第七節〈`wc` 數的三個數字〉；最後一問是新觀念，先猜】

**第 1 次（YYYY-MM-DD）**
```
```

### 2-3. 說明你的程式怎麼數字數：用 `printf ' a  bc\t\nd\n'` 這 10 個 byte，逐個 byte 寫出你的程式裡跟數字有關的變數怎麼變（例如「現在在不在一個字裡面」的變數、字數），最後字數為什麼是 3。再用 `printf 'a\x01b \x02\n'` 這 6 個 byte 同樣逐個走一遍，字數為什麼是 1？`high.bin` 裡有 `0xff`、`0xfe` 和中文的 byte，你傳給 `isspace`／`isprint` 的值是什麼型別、範圍是多少？為什麼不能直接傳 `char`？【筆記第七節〈`wc` 數的三個數字〉〈第三種 byte〉、第三節】

**第 1 次（YYYY-MM-DD）**
```
```

### 2-4. 先預測：`printf 'a\xc2\xa0b\n'` 這 5 個 byte 分別給 `./mywc`、`wc`、`LC_ALL=C wc`，字數各是多少？寫下預測後執行，貼上三個結果。你的 `mywc` 跟哪一個一樣？為什麼？【筆記第七節〈語系（locale）的例外〉】

**第 1 次（YYYY-MM-DD）**
```
```

---

## 3. 替 `mywc` 加上 `-l`、`-w`、`-c`

把 `hw2/mywc.c` 複製到 `hw3/mywc.c` 再改，編譯：`gcc -Wall -Wextra mywc.c -o mywc`。

**規格**（加在第 2 題的規格上；跟筆記[第七節](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#七wc-與-cat-的精確行為作業的規格)〈`wc` 的全部選項〉一致）：

1. 選項寫在檔名前面：從 `argv[1]` 開始，開頭是 `-`、而且不只一個字元的參數是選項；遇到第一個不是選項的參數（或單獨的 `-`）就開始是檔名。
2. 認得 `-l`、`-w`、`-c`，可以分開寫（`-l -w`）也可以合起來（`-lw`、`-wcl`）。
3. 有給選項時只印選了的欄，**順序固定是行、字、byte**，跟選項寫的順序無關；沒給任何選項時三欄都印。每欄都用 `%7ld`，欄與欄之間一個空格，最後是檔名（讀 `stdin` 沒有檔名時不印）。
4. 不認得的選項（例如 `-x`）：印 `mywc: invalid option -- 'x'` 到 `stderr`，**什麼檔案都不處理**，回傳 1（跟 GNU `wc` 一樣）。

### 3-1. 寫好之後執行下面每一行，貼上完整輸出，並跟 `LC_ALL=C wc` 的同一行比對數字與欄的順序。【筆記第七節〈`wc` 的全部選項〉；選項怎麼從 `argv` 拆出來是你自己的設計，用到第二節的 `argv[i][j]`】

```sh
cp ../hw2/*.txt .
./mywc -l one.txt spaces.txt; echo "exit=$?"
./mywc -w -l one.txt; echo "exit=$?"
./mywc -cl one.txt; echo "exit=$?"
./mywc -wcl spaces.txt; echo "exit=$?"
printf 'a b\nc\n' | ./mywc -c; echo "exit=$?"
printf 'a b\nc\n' | ./mywc -w -; echo "exit=$?"
./mywc -x one.txt; echo "exit=$?"
./mywc -l nofile.txt one.txt; echo "exit=$?"
```

**第 1 次（YYYY-MM-DD）**
```
```

### 3-2. 先猜再實驗：有一個檔案的名字就叫 `-l`（建法：`printf 'x\n' > ./-l`）。`./mywc -l` 會把它當檔名還是選項？要怎麼讓 `mywc` 數這個檔案（想兩種寫法，不改程式）？GNU `wc` 另外支援 `wc -- -l`，`--` 是什麼意思、為什麼需要它？貼上 `./mywc -l < /dev/null`、`./mywc ./-l`、`wc -- -l` 的輸出。做完 `rm ./-l`。【`./-l` 的寫法可以用筆記第二節〈誰把 `*.txt` 換成檔名〉推（程式只看到字串）；`--` 是新觀念，先猜】

**第 1 次（YYYY-MM-DD）**
```
```
