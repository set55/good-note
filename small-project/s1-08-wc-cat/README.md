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
| [`examples/savemsg/`](./examples/savemsg/) | [`savemsg.c`](./examples/savemsg/savemsg.c) | [自我測驗](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#自我測驗)第 7 題（[第十節第 1 章](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#1-strace-輸出逐行讀格式7-個系統呼叫的參數ldso-那-30-行)延伸） | `fopen(…, "w")` 底下是 `openat(…, O_WRONLY\|O_CREAT\|O_TRUNC, 0666)`；寫不進 `/etc/passwd` 時 `EACCES` 經 `errno` 由 `perror` 寫到 fd 2 |
| [`examples/syscall/`](./examples/syscall/) | [`rawio.c`](./examples/syscall/rawio.c)、[`io.py`](./examples/syscall/io.py)、[`io.go`](./examples/syscall/io.go)、[`io.rs`](./examples/syscall/io.rs) | [第十節第 3 章](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#3-系統呼叫是-kernel-的介面c-的包裝函式syscall-指令其他語言也一樣) | C 的 `open()` 底下是 `openat`、`syscall(SYS_write, …)` 跟 `write()` 一樣；Python／Go／Rust 讀檔寫檔是同一組系統呼叫（Go 不用 libc）（11-04 追問） |
| [`examples/od/`](./examples/od/) | [`hexesc.c`](./examples/od/hexesc.c) | [第十節第 2 章](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#2-od把檔案的每個-byte-印出來)〈`printf` 怎麼造出指定的 byte〉 | C 字串的 `\x` 吃掉所有十六進位數字（`"\x421"` 只剩 `0x21`、警告 `out of range`），shell 的 `printf` 只吃 2 位；拆成 `"\x42" "1"`（11-04 追問） |
| [`examples/ctype/`](./examples/ctype/) | [`spaces.c`](./examples/ctype/spaces.c)、[`classes.c`](./examples/ctype/classes.c) | [第七節](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#七wc-與-cat-的精確行為作業的規格)〈`wc` 數的三個數字〉 | `isspace` 認定的 6 個空白 byte；0～255 分成空白、可印、其他三類 |

---

## 批改結果

> 第 1 題 2026-11-05 批改；第 2～4 題待作答。程式碼另依 Linus 的「good taste」給品味意見（寫在 1-1 批改裡），品味不影響對錯判定。

| 題目 | 結果 | 錯在哪／缺什麼 | 過關 |
|---|---|---|---|
| 1-1 | 大致對 | 輸出內容、結束狀態、`cmp` 都對；但錯誤訊息把規格的佔位符 `<…>` 照字面印出（`mycat: <nofile.txt>: <No such…>`），逐行比對時沒抓到跟 `cat` 不同；`-` 那行漏貼 `mycat` 的輸出；程式對 `stdin` 也 `fclose`，違反規格第 5 點（`./mycat - -` 第二個 `-` 會 `Bad file descriptor`） | 未過關 |
| 1-2 | 對 | 抓到 `>` 只導 `stdout`、錯誤走 `stderr` 所以留在終端機；改成印到 `stdout` 會混進 `all.txt` | — |
| 1-3 | 不完整 | 預測與結果（只有 `A`、`B`）對；沒解釋**為什麼**停在 `0xff`（存進有號 `char` 變 −1 ＝ `EOF`）；還誤以為「應該要有警告」（`-Wall -Wextra` 都抓不到）；沒貼 `od -c` 與 `exit=` | 未過關 |
| 1-4 | 錯 | 只貼了終端機那次；接管線時實際是錯誤訊息跑到最前面（你的預測其實對了）。原因寫反：`2>&1` 不會讓 `stderr` 變成有緩衝，是 `stdout` 接管線變**全緩衝**、留到程式結束才寫，`stderr` 不緩衝馬上寫 | 未過關 |

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

**第 1 次（2026-11-05）**
```
./mycat one.txt nonl.txt one.txt; echo "exit=$?"
hello world
no newline at endhello world
exit=0

cat one.txt nonl.txt one.txt; echo "exit=$?"
hello world
no newline at endhello world
exit=0

./mycat one.txt nofile.txt one.txt; echo "exit=$?"
hello world
mycat: <nofile.txt>: <No such file or directory>
hello world
exit=1

cat one.txt nofile.txt one.txt; echo "exit=$?"
hello world
cat: nofile.txt: No such file or directory
hello world
exit=1

printf 'from stdin\n' | cat one.txt - one.txt; echo "exit=$?"
hello world
from stdin
hello world
exit=0

printf 'only stdin\n' | ./mycat; echo "exit=$?"
only stdin
exit=0

printf 'only stdin\n' | cat; echo "exit=$?"
only stdin
exit=0

./mycat adir; echo "exit=$?"
mycat: <adir>: <Is a directory>
exit=1

cat adir; echo "exit=$?"
cat: adir: Is a directory
exit=1

./mycat ff.bin | od -c
0000000   A   B 377   C   D  \n
0000006

cat ff.bin | od -c
0000000   A   B 377   C   D  \n
0000006

for f in one.txt nonl.txt ff.bin; do ./mycat "$f" | cmp - "$f" && echo "$f 一樣"; done
one.txt 一樣
nonl.txt 一樣
ff.bin 一樣


for f in one.txt nonl.txt ff.bin; do cat "$f" | cmp - "$f" && echo "$f 一樣"; done
one.txt 一樣
nonl.txt 一樣
ff.bin 一樣
```

> **批改（2026-11-05）：大致對。** 你的 `mycat.c` 我在 scratchpad 照抄編譯（`gcc -Wall -Wextra`，沒有警告）重跑過，貼上的輸出都對得上。
>
> - 對的地方：檔案內容原樣輸出（`nonl.txt` 接著 `hello world`）、壞檔案印錯誤後繼續、目錄印 `Is a directory`、結束狀態 0／1、`ff.bin` 的 `377` 有印出來、三個 `cmp` 都一樣。
> - **錯誤訊息的格式跟 `cat` 不一樣**：規格的 `mycat: <檔名>: <錯誤說明>` 裡，`<…>` 是「這裡填檔名」的佔位符，不是要印的字元。你印出 `mycat: <nofile.txt>: <No such file or directory>`，`cat` 是 `cat: nofile.txt: No such file or directory`，題目要你逐行比對，這個差別就擺在兩行之間，卻沒抓到。（佔位符的寫法題目沒說清楚，這點我有責任；但比對是這題的要求。）
> - **漏貼一行**：`printf 'from stdin\n' | ./mycat one.txt - one.txt` 只貼了 `cat` 的結果，沒貼 `mycat` 的。我替你跑了，輸出是對的：`hello world`／`from stdin`／`hello world`／`exit=0`。
> - **違反規格第 5 點（「`stdin` 不用關」）**：參數是 `-` 時 `fp = stdin`，迴圈最後照樣 `fclose(fp)`，把 `stdin` 關掉了。題目的測試只用一次 `-`，所以沒露出來；`-` 出現兩次就壞了：
>
> ```text
> $ printf 'abc\n' | ./mycat - -; echo "exit=$?"
> mycat: <->: <Bad file descriptor>
> abc
> exit=1
> $ printf 'abc\n' | cat - -; echo "exit=$?"
> abc
> exit=0
> ```
>
>   第一個 `-` 讀完 `abc` 後把 `stdin` 關了，第二個 `-` 對已關的 `stdin` 讀，`read` 失敗、`errno` 是 `EBADF`（Bad file descriptor）。錯誤訊息排在 `abc` 前面，是 1-4 講的緩衝。（GNU `cat` 遇到第二個 `-` 時 `stdin` 已經讀到結尾，所以什麼都不印。）
>
> **參考答案**【[筆記第七節〈`cat`：依序把每個檔案的 byte 原樣寫到 `stdout`〉](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#cat依序把每個檔案的-byte-原樣寫到-stdout)】
>
> 核心關鍵：
> 1. 每一行指令的 `mycat` 輸出（含 `exit=`）都貼出來，而且跟 `cat` 除了開頭的程式名之外**逐字相同**，包括錯誤訊息的格式 `mycat: nofile.txt: No such file or directory`。
> 2. 三個檔案的 `cmp` 都沒有差異（印出「一樣」），證明 byte 完全相同，包括 `0xff`。
> 3. 程式符合規格全部 5 點，包括 `-` 讀 `stdin` 時不 `fclose`。
>
> 詳細說明：比對要拿兩邊的同一行一個字一個字看，看「大概一樣」就會漏掉 `<`、`>` 這種差別。`cmp` 沒輸出、只印「一樣」，代表兩邊 byte 完全相同；有差時它會印 `differ: byte N, line M`。
>
> **品味（good taste）**：照 Linus 的標準，這份程式最大的問題是**同一件事寫了兩次，而且兩份不一樣**。
>
> 1. **「沒有參數」被寫成特殊情況**（第 47～53 行）：它其實就是「只有一個參數 `-`」。你另外抄了一份讀 `stdin` 的迴圈，抄過去的那份**沒有 `ferror` 檢查**。這就是特殊情況的代價：兩份程式碼會慢慢長得不一樣，bug 只修到其中一份。改成 `if (argc == 1) return cat_one("-");`，走同一條路。
> 2. **錯誤處理也寫了兩次**（第 25～28 行與第 38～41 行，一字不差）。開檔失敗和讀檔失敗的差別只在「`errno` 從哪來」，印訊息、設回傳值是同一件事。先把 `errno` 記進一個 `err`，最後只印一次。
> 3. **先 `fopen` 再判斷是不是 `-`**（第 18～22 行）：順序反了。參數是 `-` 時你還是去開一個叫 `-` 的檔案；萬一真的有這個檔案，開出來的 `FILE *` 被 `fp = stdin` 蓋掉，永遠不會關（資源洩漏），而且 `fp` 已經變成 `stdin`，後面的 `fclose` 關到的是 `stdin`，也就是上面那個 bug。先決定 `fp` 是什麼，再開：`fp = strcmp(name, "-") == 0 ? stdin : fopen(name, "r");`，一行就不會出錯。
> 4. **`errno = saved;` 是沒作用的一行**（第 26、39 行）：筆記的 `openfail.c` 需要它，是因為存起來之後中間呼叫了 `printf`（可能改掉 `errno`），呼叫 `perror` 之前要放回去，`perror` 是讀 `errno` 的。你的程式直接把 `saved` 傳給 `strerror`，中間什麼都沒呼叫，把 `errno` 設回它本來的值等於什麼都沒做。不知道為什麼要寫的一行就不要抄。
> 5. **整個程式塞在 `main` 裡**：處理一個檔案是一個完整的動作，抽成函式 `cat_one()`，`main` 只剩迴圈，一眼看得出「每個參數做一次、有失敗就回 1」。
> 6. **小地方**：註解掉的除錯碼（第 7～11 行）交作業前刪掉，版本控制會記得它們；`c` 宣告在 `main` 的最外層，其實只有讀檔時用到，可以縮到用它的地方；kernel coding style 是 tab 縮排，`if`／`while` 的 `{` 放在同一行（只有函式的 `{` 另起一行），單行的 `while` 本體不加大括號。
>
> 改寫後（`gcc -Wall -Wextra` 沒有警告，上面 7 行測試的結果跟 `cat` 一樣，`- -` 也對了）：
>
> ```c
> #include <errno.h>
> #include <stdio.h>
> #include <string.h>
>
> /* 把一個檔案（"-" 是 stdin）原樣寫到 stdout；成功回傳 0，失敗印錯誤、回傳 1 */
> static int cat_one(const char *name)
> {
> 	FILE *fp = strcmp(name, "-") == 0 ? stdin : fopen(name, "r");
> 	int c, err = 0;
>
> 	if (!fp) {
> 		err = errno;
> 	} else {
> 		while ((c = getc(fp)) != EOF)
> 			putc(c, stdout);
> 		if (ferror(fp))
> 			err = errno;
> 		if (fp != stdin)
> 			fclose(fp);
> 	}
>
> 	if (err) {
> 		fprintf(stderr, "mycat: %s: %s\n", name, strerror(err));
> 		return 1;
> 	}
> 	return 0;
> }
>
> int main(int argc, char *argv[])
> {
> 	int ret = 0;
>
> 	if (argc == 1)
> 		return cat_one("-");
>
> 	for (int i = 1; i < argc; i++)
> 		ret |= cat_one(argv[i]);
> 	return ret;
> }
> ```
>
> - `err` 在 `fclose` **之前**就記下來：`fclose` 也可能改 `errno`。
> - `ret |= cat_one(...)`：只要有一次回傳 1，`ret` 就一直是 1，不必寫 `if`。
> - 留給你想（不算這題的分）：`./mycat one.txt > /dev/full` 寫不進去（`/dev/full` 是永遠「磁碟已滿」的裝置），你的程式和上面的改寫都回傳 0；`cat` 會印 `cat: write error: No space left on device`、回傳 1。規格沒要求檢查寫入，但真正的工具要檢查：結束前 `fflush(stdout)`，失敗或 `ferror(stdout)` 就報錯。

### 1-2. 執行 `./mycat one.txt nofile.txt > out.txt; echo "exit=$?"`，貼上終端機上看到的內容，再貼上 `cat out.txt`。錯誤訊息出現在哪裡、沒出現在哪裡？為什麼？如果規格第 3 點改成「錯誤訊息印到 `stdout`」，使用者打 `./mycat *.txt > all.txt` 會遇到什麼麻煩？【筆記第五節〈結果寫 `stdout`，錯誤寫 `stderr`〉】

**第 1 次（2026-11-05）**
```
mycat: <nofile.txt>: <No such file or directory>
exit=1

hello world

因爲我的代碼明確的寫出
fprintf(stderr, "mycat: <%s>: <%s>\n", argv[i], strerror(saved));
錯誤信息出現在stderr, > 是stdout 所以沒有出現在stdout,所以沒有進入out.txt


那這樣錯誤信息也會跟着進all.txt, 無法區分哪裏有錯誤
```

> **批改（2026-11-05）：對。** 現象、原因、後果都講到了。只差一點不扣分：輸出的格式照 1-1，應該是 `mycat: nofile.txt: No such file or directory`，沒有 `<>`。
>
> **參考答案**【[筆記第五節〈結果寫 `stdout`，錯誤寫 `stderr`〉](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#結果寫-stdout錯誤寫-stderr)】
>
> 核心關鍵：
> 1. 錯誤訊息出現在終端機，沒有進 `out.txt`；`out.txt` 只有 `hello world`。
> 2. 原因：`> out.txt` 只把 fd 1（`stdout`）導到檔案；錯誤是 `fprintf(stderr, …)` 寫到 fd 2，fd 2 沒被導，還是終端機。
> 3. 改成印到 `stdout` 時，錯誤訊息會混進 `all.txt`，變成資料的一部分。
>
> 詳細說明：混進去之後，下一個讀 `all.txt` 的程式（或人）會把「`mycat: nofile.txt: No such file or directory`」當成某個檔案的內容；另一個麻煩是使用者在終端機上**看不到**出錯，以為全部成功。分開寫，報告乾淨，錯誤也照樣看得到。想把錯誤另存，用 `2> err.txt`。

### 1-3. 先預測：把你程式裡接 `getc` 回傳值的變數**暫時改成 `char`**，重新編譯，執行 `./mycat ff.bin | od -c; echo "exit=$?"`。輸出會有幾個 byte、是哪幾個？寫下預測後再執行，貼上結果與編譯有沒有警告。為什麼？做完改回 `int`。【筆記第三節〈錯誤示範：用 `char` 接 `getchar()`〉】

**第 1 次（2026-11-05）**
```
預測: 2個byte, A,B

改成char後沒有警告
AB%
應該要有警告,畢竟是int轉char有號
```

> **批改（2026-11-05）：不完整。** 預測（2 個 byte：`A`、`B`）對，結果也對；但題目問的「為什麼」沒有回答，關於警告的判斷也錯了。
>
> - **沒有解釋為什麼停在 `0xff`**：這是這題的重點，見下方核心關鍵 3。
> - **「應該要有警告」是錯的**：`-Wall -Wextra` 都不管 `int` 存進 `char` 這種縮小轉換，筆記第三節實測過「兩個都沒有警告：`-Wall` 抓不到這個錯」。要多加 `-Wconversion` 才抓得到。我用改成 `char` 的版本實測：
>
> ```text
> $ gcc -Wall -Wextra -Wconversion charbug.c -o cb2
> charbug.c:14:29: warning: conversion from ‘int’ to ‘char’ may change value [-Wconversion]
>    14 |                 while ((c = getc(fp)) != EOF)
> ```
>
>   `-Wconversion` 不在 `-Wall`／`-Wextra` 裡，是因為它在一般程式碼裡會噴出大量「其實沒問題」的警告。所以這個 bug 主要還是要靠「`getc` 的回傳值一律用 `int` 接」這個習慣防。
> - **沒有照題目的指令跑**：題目是 `./mycat ff.bin | od -c; echo "exit=$?"`，你貼的 `AB%` 是直接印到終端機的結果（`%` 是 zsh 提示「最後沒有換行」的記號，不是程式印的）。照題目跑是：
>
> ```text
> 0000000   A   B
> 0000002
> exit=0
> ```
>
>   `exit=0`：程式以為檔案讀完了，`ferror` 是假，不會報錯。這個 bug 很難發現，就是因為它**安靜地少輸出**。
>
> **參考答案**【[筆記第三節〈錯誤示範：用 `char` 接 `getchar()`〉](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#錯誤示範用-char-接-getchar)】
>
> 核心關鍵：
> 1. 輸出只有 2 個 byte：`A`、`B`（`od` 顯示 `0000002`），`exit=0`。
> 2. 沒有警告：`-Wall -Wextra` 抓不到這個錯。
> 3. 為什麼：`getc` 讀到 `0xff` 回傳 255，存進 x86-64 上**有號**的 `char` 只留 8 位元 `11111111`，讀成 −1；跟 `EOF`（−1）比較時相等，迴圈以為讀完了，提早結束。
>
> 詳細說明：`int` 接時，255 是 `0x000000ff`、`EOF` 是 `0xffffffff`，兩者分得開；存進 `char` 後兩者都是 `11111111`，再也分不出來。在 `char` 是無號的平台（例如 ARM 上的 Linux，或 `-funsigned-char`），反過來 `EOF` 存進去變 255，永遠不等於 `EOF`，變成無窮迴圈。

### 1-4. 先預測：`./mycat one.txt nofile.txt one.txt 2>&1 | cat` 印出的三行，順序會跟在終端機直接執行 `./mycat one.txt nofile.txt one.txt` 一樣嗎？寫下預測後兩個都執行，貼上結果。為什麼一樣（或不一樣）？【筆記第五節〈輸出順序為什麼會變〉】

**第 1 次（2026-11-05）**
```
第一個是nofile的錯誤
第二個是 one.txt
第三個是 one.txt

hello world
mycat: <nofile.txt>: <No such file or directory>
hello world

我的預測錯了,因爲現在stderr轉到stdout, 都是到緩存裏了,而且hello world\n 標準輸出是行緩存,hello world就先輩打出去了
```

> **批改（2026-11-05）：錯。** 你的**預測其實是對的**，接管線時錯誤訊息確實排在最前面；錯的是後面的實驗和解釋。
>
> - **只貼了一次的結果**：貼的 `hello world`／錯誤／`hello world` 是在終端機直接執行的那次。題目要兩個都貼，你可能把直接執行的結果當成管線的結果了，才會以為預測錯了。我用你的程式跑管線那一行：
>
> ```text
> $ ./mycat one.txt nofile.txt one.txt 2>&1 | cat
> mycat: <nofile.txt>: <No such file or directory>
> hello world
> hello world
> ```
>
> - **解釋錯了兩處**：
>   - 「`stderr` 轉到 `stdout`，都到緩存裡了」：`2>&1` 是 **shell 在 fd 層**做的事，讓 fd 2 指向 fd 1 指的那個管線；程式裡 `stderr` 這個 `FILE` 的緩衝方式沒有改變，還是**不緩衝**，`fprintf(stderr, …)` 一呼叫就寫出去。
>   - 「標準輸出是行緩存」：只有接**終端機**時才是行緩衝。這裡 `stdout` 接的是管線（`| cat`），是**全緩衝**：兩個 `hello world` 都留在緩衝區，等到 `return`（或緩衝區滿）才一次寫出去，所以排在錯誤訊息後面。
>
> **參考答案**【[筆記第五節〈輸出順序為什麼會變：`stdout` 有緩衝、`stderr` 沒有〉](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#輸出順序為什麼會變stdout-有緩衝stderr-沒有)】
>
> 核心關鍵：
> 1. 不一樣：直接執行是 `hello world`／錯誤／`hello world`；接管線時錯誤訊息跑到最前面，接著兩行 `hello world`。
> 2. 接管線時 `stdout` 是**全緩衝**，兩行 `hello world` 留在緩衝區，程式結束時才寫出去。
> 3. `stderr` **不緩衝**，錯誤訊息當場就寫出去；`2>&1` 只是讓兩者寫到同一個管線，不會改變誰有緩衝。
> 4. 直接執行時 `stdout` 接終端機，是**行緩衝**，每個 `\n` 就寫，所以順序跟程式執行的順序一樣。
>
> 詳細說明：兩種情況下，各自資料流的內容都一樣，變的只有交錯的順序。所以比對輸出時 `stdout`、`stderr` 要分開比（例如 `2>err.txt`），不要靠它們的相對順序。想讓順序固定，可以每次寫錯誤前 `fflush(stdout)`；GNU `cat` 根本不用 stdio 的緩衝，直接 `write`，所以順序永遠照程式執行的順序。

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

### 3-2. 先猜再實驗：有一個檔案的名字就叫 `-l`（建法：`printf 'x\n' > ./-l`）。`./mywc -l` 會把它當檔名還是選項？為什麼？要怎麼讓 `mywc` 數這個檔案（想兩種寫法，不改程式）？GNU `wc` 另外支援 `wc -- -l`，`--` 是什麼意思、為什麼需要它？貼上 `./mywc -l < /dev/null`、`./mywc ./-l`、`wc -- -l` 的輸出。做完 `rm ./-l`。【`./-l` 的寫法可以用筆記第二節〈誰把 `*.txt` 換成檔名〉推（程式只看到字串）；`--` 是新觀念，先猜】

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

### 4-5. 先猜再實驗：執行 `./tohex int ' 7' '7 ' +7 010; echo "exit=$?"`，哪幾個成功、哪幾個失敗？各為什麼？`010` 變成多少？為什麼？再執行 `./tohex int 9; ./tohex float 9; ./tohex double 9`，三個都是「9」，byte 卻完全不像——不只是長度不同，`float 9` 的 byte 裡根本沒有 `09`。你猜 `float`／`double` 是怎麼存數字的？【前兩問用上方 `strtol` 的說明推；最後一問是新觀念，先猜（浮點數的表示法 IEEE 754 之後補課）】

**第 1 次（YYYY-MM-DD）**
```
```
