# s1-08：寫自己的 `wc` 與 `cat`——命令列參數、讀到 `EOF`、`errno` 錯誤處理

> 對應課程：第 ① 站第 8 課（本站專案）
> 課程筆記：[寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html)
> 自我測驗：寫在同資料夾的 [quiz.md](./quiz.md)

## 說明

這一課是第 ① 站的畢業專案：**自己從零寫**兩支命令列工具，`mycat` 和 `mywc`，行為要跟 GNU `cat`、`wc` 一致（範圍見每題的規格）。

- 每一題一個資料夾（`hw1/`～`hw4/`），程式檔自己建。題目給的測試指令照下，**輸出要貼進作答框**。
- 編譯一律用 `gcc -Wall -Wextra`，**有警告就要處理**，不能忽略。`-Wextra` 是在 `-Wall` 之外再多開一批警告（例如有號和無號混在一起比較、函式參數沒用到），寫命令列工具時很容易踩到。
- 測試用的檔案都用題目裡的 `printf` 指令建（`.txt`、`.bin` 不會進 git，白名單只收原始碼與 `.md`）。
- 筆記裡的範例（`args.c`、`getbytes.c`、`openfail.c`、`countch.c`）可以參考，但它們都只處理**一個**檔案或只讀 `stdin`；作業要處理多個檔案、`-`、總計、錯誤後繼續，要自己設計。
- **只能用筆記教過的函式**（`fopen`、`fclose`、`getc`、`putc`、`printf`、`fprintf`、`perror`、`strerror`、`ferror`、`isspace`、`isprint`、`strcmp`），不要用 `fread`／`read` 之類一次讀一塊的寫法——這一課要練的是一個 byte 一個 byte 處理。`strcmp`（`<string.h>`）比較兩個字串，相等回傳 0，例如 `strcmp(argv[i], "-") == 0` 判斷參數是不是 `-`。第 4 題另外可以用 `strtol`／`strtof`／`strtod`（題目裡有說明），而且它處理的是命令列參數、不讀檔案。

每題「解釋原因」的地方都標了答案的來源：
- 【筆記第 X 節】：本課教過，回筆記那一節找。
- 【新觀念，先猜】：還沒教過，寫下你的猜測就好，猜錯不算錯。

**「先預測」的題目，一定要先把預測寫進作答框，再執行。** 預測錯了沒關係，把錯在哪裡寫下來，這才是練習的重點。

**實驗要跑到最後一步、貼上證據**：「沒有錯誤訊息」不代表結果對，每一題都要貼上執行的輸出與 `exit=` 的值。

**怎麼作答**：每個小題下面有作答框，直接寫在裡面（把 `YYYY-MM-DD` 換成作答日期）。程式碼本身寫在 `hw1/` 等資料夾的 `.c` 檔裡，作答框貼**測試的輸出**與**解釋**。批改與參考答案寫在作答框正下方，不重答。

---

## 筆記的範例程式

課程筆記裡每個實驗的原始碼都放在 [`examples/`](./examples/)，一個實驗一個子資料夾，可以直接跟著筆記重做。**它們是範例，不是作業**；作業寫在 `hw1/`～`hw4/`，檔名跟範例不重複。

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

---

## 4. `tohex`：把數值在記憶體裡的 byte 印成十六進位（2026-11-02 使用者要求追加）

在 `hw4/` 寫 `tohex.c`，編譯：`gcc -Wall -Wextra tohex.c -o tohex`。

這支工具把命令列給的數值，照指定的 C 型別存進一個變數，再把**那個變數在記憶體裡的每一個 byte** 印出來——就是你在 [第 2 課第二節](../../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#二指標就是位址) 用 `(unsigned char *)&x` 一個 byte 一個 byte 看 `int` 的做法，做成一支能處理六種型別、會檢查輸入的命令列工具。`float`、`double` 的 byte 為什麼長那樣（IEEE 754）還沒教，這題**不需要**懂它，只要把 byte 原樣印出來；對答案用 11-02 學的 `python3 -c "import struct; …"`。

**用法**：`./tohex <型別> <值>...`——第一個參數是型別，後面可以接一個以上的值，全部當成同一種型別。

**規格**：

1. **型別**只認 `char`、`short`、`int`、`long`、`float`、`double` 六個（用 `strcmp` 比）。少於兩個參數（沒給型別或沒給任何值），或型別不認得：印 `usage: tohex char|short|int|long|float|double VALUE...` 到 `stderr`，**什麼都不處理**，回傳 1（跟第 3 題不認得的選項一樣）。
2. **每個值**依序處理，成功的印三行到 `stdout`：
   ```text
   <型別> <使用者打的原字串> (<sizeof 的值> bytes)
     memory: <從低位址到高位址，每個 byte 兩位小寫十六進位，用一個空格隔開>
     value:  0x<從高位址到低位址，每個 byte 兩位，連在一起>
   ```
   例如 `./tohex short 258` 印（258 ＝ `0x0102`）：
   ```text
   short 258 (2 bytes)
     memory: 02 01
     value:  0x0102
   ```
3. **兩行十六進位都要從 byte 印出來**：把值存進該型別的變數 `x` 之後，用 `unsigned char *` 指向 `&x`，一個 byte 一個 byte 讀。`memory` 從第 0 個往後印，`value` 從最後一個往前印。**不可以**對 `x` 本身用 `%x`／`%lx`（`float`、`double` 也沒辦法這樣印）。把「印 `n` 個 byte」寫成**一個函式**，六種型別共用（參數怎麼設計自己想，提示：第 2 課的 `(unsigned char *)&x` 與 `sizeof`）。
4. **怎麼把字串轉成數值**：
   - `char`：參數必須**剛好是一個 byte**（例如 `A`），就存那個 byte；空字串或超過一個 byte 都是錯（判斷方式：用第 8 課第二節的 `argv[i][j]`，不要用 `strlen`）。
   - `short`、`int`、`long`：用 `strtol`（見下方說明）以十進位轉成 `long`；`short`、`int` 再檢查有沒有超出該型別的範圍（`<limits.h>` 的 `SHRT_MIN`／`SHRT_MAX`、`INT_MIN`／`INT_MAX`），沒超出才存進 `short`／`int` 變數。
   - `float` 用 `strtof`，`double` 用 `strtod`。
5. **錯誤**（印到 `stderr`，**繼續處理下一個值**，最後回傳 1；全部成功回傳 0）：
   - 不是合法的數字（一個數字都沒轉到，或數字後面還有別的字元）：`tohex: '<原字串>': not a valid <型別>`
   - 超出範圍：`tohex: '<原字串>': out of range for <型別>`
   - `char` 不是剛好一個 byte：`tohex: '<原字串>': char needs exactly one byte`

**本題可以用的新函式**（`<stdlib.h>`，課程還沒教，這裡先說明；完整說明見 `man 3 strtol`、`man 3 strtod`）：

| 函式 | 做什麼 |
|---|---|
| `long strtol(const char *s, char **end, int base)` | 把字串 `s` 開頭的整數轉成 `long`，`base` 是進位（這題固定給 `10`） |
| `float strtof(const char *s, char **end)` | 轉成 `float` |
| `double strtod(const char *s, char **end)` | 轉成 `double` |

三個函式的共同規則：

- **開頭的空白會先跳過**，接著盡量讀出一個數字，**讀到不能再讀的地方就停**，把停下來的位置存進 `*end`（所以要傳一個 `char *` 變數的位址進去）。例如 `s` 是 `"12abc"`，回傳 12，`end` 指向 `'a'`。
- **一個數字都沒讀到**時回傳 0，`end` 等於 `s`。所以「`end == s`」代表沒有數字，「`*end != '\0'`」代表數字後面還有東西。
- **超出範圍**時把 `errno` 設成 `ERANGE`（`<errno.h>`）並回傳一個極端值；**成功時不會把 `errno` 清成 0**。
- `strtod`／`strtof` 也認得 `1e3`（科學記號）、`inf`、`nan` 這種寫法。

**也可以用** `<limits.h>`、`<errno.h>`；其他沒列的函式（`atoi`、`sscanf`、`memcpy`、`strlen`…）不要用。

### 4-1. 寫好之後執行下面每一行，貼上**完整輸出**（含 `exit=`）。再用最後兩行的 Python 指令印出 `float`／`double` 的 9 和 0.1，跟你的 `memory` 行逐 byte 比對。【規格；`strtol` 等的用法見上方說明】

```sh
cd hw4
./tohex int 300 -1; echo "exit=$?"
./tohex short -1 32767 32768; echo "exit=$?"
./tohex long -1 9223372036854775808; echo "exit=$?"
./tohex int 12abc '' 2147483648 7; echo "exit=$?"
./tohex char A é ''; echo "exit=$?"
./tohex float 9 0.1 1e39; echo "exit=$?"
./tohex double 9 0.1 1e309; echo "exit=$?"
./tohex byte 1; echo "exit=$?"
./tohex int; echo "exit=$?"
python3 -c "import struct; print(struct.pack('<f', 9.0).hex(' '), '|', struct.pack('<f', 0.1).hex(' '))"
python3 -c "import struct; print(struct.pack('<d', 9.0).hex(' '), '|', struct.pack('<d', 0.1).hex(' '))"
```

（`struct.pack('<f', 數字)` 照 C 的 `float`、小端序轉成 byte，`'<d'` 是 `double`；`.hex(' ')` 印成用空格隔開的十六進位，跟 `memory` 行同一個順序。）

**第 1 次（YYYY-MM-DD）**
```
```

### 4-2. 用 4-1 的輸出回答：`int 300` 的 `memory` 和 `value` 兩行，為什麼 byte 的順序剛好相反？哪一行才是 `300` 寫成十六進位的樣子？`short -1`、`int -1`、`long -1` 的 byte 為什麼全是 `ff`，三者差在哪？【第 2 課[第三節〈位元組順序（endianness）〉](../../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#三位元組順序endianness)、[第五節〈有號整數：二補數〉](../../html/c/C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.html#五有號整數二補數)】

**第 1 次（YYYY-MM-DD）**
```
```

### 4-3. 先預測：把你程式裡「呼叫 `strtol` 之前把 `errno` 設成 0」那一行**暫時刪掉**（如果你沒寫這行，就是現在的樣子），執行 `./tohex long 99999999999999999999 5; echo "exit=$?"`。`5` 會被當成什麼？寫下預測再執行，貼上結果並解釋，做完改回來。【筆記[第四節〈實驗：成功的呼叫不會把 `errno` 清成 0〉](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#實驗成功的呼叫不會把-errno-清成-0)，加上方 `strtol` 的說明】

**第 1 次（YYYY-MM-DD）**
```
```

### 4-4. `./tohex char é` 為什麼是錯誤、不是印出一個 byte？`é` 實際上是幾個 byte、哪幾個（用 `printf 'é' | od -An -tx1` 查）？如果規格改成「`char` 只看參數的第一個 byte」，印出來的 `char` 用 `%d` 印會是正數還是負數？【[字元筆記第四節〈中文字：一個 byte 裝不下，UTF-8 用 3 個 byte〉](../../html/c/字元是什麼-char就是1byte的整數與ASCII和UTF-8.html#四中文字一個-byte-裝不下utf-8-用-3-個-byte)與〈跟第 2 課的連結：`char` 有號，讀到中文會變負數〉】

**第 1 次（YYYY-MM-DD）**
```
```

### 4-5. 先猜再實驗：執行 `./tohex int ' 7' '7 ' +7 010; echo "exit=$?"`，哪幾個成功、哪幾個失敗？`010` 變成多少？再執行 `./tohex int 9; ./tohex float 9; ./tohex double 9`，三個都是「9」，byte 卻完全不像——不只是長度不同，`float 9` 的 byte 裡根本沒有 `09`。你猜 `float`／`double` 是怎麼存數字的？【前兩問用上方 `strtol` 的說明推；最後一問是新觀念，先猜（浮點數的表示法 IEEE 754 之後補課）】

**第 1 次（YYYY-MM-DD）**
```
```
