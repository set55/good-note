# s1-08 自我測驗作答

> 第 ① 站第 8 課的自我測驗寫在這裡。題目從筆記複製過來；批改完後，最新版的參考答案也寫在筆記每題下面。
>
> **怎麼寫**：直接在每題下面的空白區塊裡作答，**闔上筆記、用自己的話**（把 `YYYY-MM-DD` 換成作答日期）。寫完跟 Claude 說「幫我批改」。
> 批改與參考答案會寫在你的答案下方，**不重答**：不是「對」的小題要在批改下方的自我練習框重寫，練到 Claude 判定算對才過關；全部過關後整份才進複習排程。
>
> **複習**：間隔複習的作答寫在 repo 根目錄的 [`review/`](../../review/README.md)（每天一個檔），排程見 [複習排程](../../learning/複習排程.md)。本檔只放第一次作答。
>
> **順序**：讀完筆記 → 寫[作業](./README.md) → 作業不對的小題練到過關 → 再寫這份測驗。

## 目錄

| 測驗 | 對應筆記 | 狀態 |
|---|---|---|
| [第 8 課：寫自己的 `wc` 與 `cat`](#未答第-8-課寫自己的-wc-與-cat) | [寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html#自我測驗) | 未答 |

---

## 未答：第 8 課：寫自己的 `wc` 與 `cat`

> 題目來自 [寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html](../../html/c/寫自己的wc與cat-命令列參數、讀到EOF與errno錯誤處理.html)。
> 每題拆成小題（`N-M`），每個小題各有一個作答框；題目用到的程式碼附在每個大題的開頭。批改寫在該小題的作答框正下方。
> 2026-11-04 追加第 6、7 題（筆記第十節〈課後討論〉第 1 章：`strace` 輸出逐行讀），跟第 1～5 題一起作答。

### 1. `EOF` 與 `int`：用 `char` 接 `getchar()`

下面的 `charbug.c` 用 `char` 接 `getchar()` 的回傳值。用 `printf 'AB\xffCD' > ff.bin` 做一個 5 byte 的檔案（第 3 個 byte 是 `0xff`），在 x86-64 Ubuntu 上 `gcc -Wall charbug.c -o charbug && ./charbug < ff.bin`，只印出「讀到 2 個 byte」；改用 `gcc -Wall -funsigned-char charbug.c -o charbug_u` 編，`./charbug_u < ff.bin` 卻永遠停不下來。(1) 為什麼預設版只數到 2？(2) 為什麼 `-funsigned-char` 版停不下來？(3) `getchar` 為什麼要回傳 `int`，不能直接回傳 `char`？

```c
/* charbug.c */
#include <stdio.h>

int main(void)
{
    char c;                /* 錯誤示範：用 char 裝 getchar() 的回傳值 */
    long n = 0;

    while ((c = getchar()) != EOF)
        n++;
    printf("讀到 %ld 個 byte\n", n);
    return 0;
}
```

#### 1-1. 為什麼預設版只數到 2？

**第 1 次（YYYY-MM-DD）**
```
```

#### 1-2. 為什麼 `-funsigned-char` 版停不下來？

**第 1 次（YYYY-MM-DD）**
```
```

#### 1-3. `getchar` 為什麼要回傳 `int`，不能直接回傳 `char`？

**第 1 次（YYYY-MM-DD）**
```
```

### 2. `errno`：成功的呼叫不清 `errno`

下面的 `stale.c` 在同一個資料夾執行（`ok.txt` 存在、`nofile.txt` 不存在：`echo hi > ok.txt`），編譯 `gcc -Wall stale.c -o stale && ./stale`，第二行印出「成功之後：good = 0x612d78eab2a0, errno = 2 (No such file or directory)」。(1) `good` 開成功了，`errno` 為什麼還是 2？(2) 所以要怎麼判斷一個函式有沒有失敗？什麼時候讀 `errno` 才有意義？(3) 同學把錯誤處理寫成下面第二段那樣：`fopen` 失敗後先 `printf("開檔失敗\n");` 再 `perror(argv[1]);`。這樣寫有什麼風險？

```c
/* stale.c */
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    FILE *bad = fopen("nofile.txt", "r");      /* 失敗：設 errno */
    printf("失敗之後：bad = %p, errno = %d\n", (void *)bad, errno);

    FILE *good = fopen("ok.txt", "r");         /* 成功：不會把 errno 清成 0 */
    printf("成功之後：good = %p, errno = %d (%s)\n", (void *)good, errno, strerror(errno));

    errno = 0;                                 /* 要拿 errno 判斷，得自己先清 */
    FILE *good2 = fopen("ok.txt", "r");
    printf("先清 0 再成功：errno = %d\n", errno);

    fclose(good);
    fclose(good2);
    return 0;
}
```

```c
/* 同學的寫法（片段） */
FILE *fp = fopen(argv[1], "r");
if (fp == NULL) {
    printf("開檔失敗\n");
    perror(argv[1]);
    return 1;
}
```

#### 2-1. `good` 開成功了，`errno` 為什麼還是 2？

**第 1 次（YYYY-MM-DD）**
```
```

#### 2-2. 所以要怎麼判斷一個函式有沒有失敗？什麼時候讀 `errno` 才有意義？

**第 1 次（YYYY-MM-DD）**
```
```

#### 2-3. 同學把錯誤處理寫成下面第二段那樣：`fopen` 失敗後先 `printf("開檔失敗\n");` 再 `perror(argv[1]);`。這樣寫有什麼風險？

**第 1 次（YYYY-MM-DD）**
```
```

### 3. `EOF` 的兩種原因：開目錄、`ferror`

下面的 `countch.c` 數一個檔案裡某個字元有幾個。`mkdir adir && ./countch adir a` 印出「`adir: Is a directory`」、結束狀態 1。(1) 錯誤是在 `fopen` 那一步還是之後才發生的？為什麼？(2) 如果把 `while` 迴圈後面的 `if (ferror(fp)) { ... }` 整段刪掉，`./countch adir a` 會印出什麼、結束狀態是多少？為什麼這樣是錯的？

```c
/* countch.c */
#include <stdio.h>

/* 用法：./countch 檔名 字元 —— 數這個檔案裡有幾個該字元 */
int main(int argc, char *argv[])
{
    if (argc != 3 || argv[2][0] == '\0' || argv[2][1] != '\0') {
        fprintf(stderr, "用法：%s 檔名 單一字元\n", argv[0]);
        return 2;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror(argv[1]);
        return 1;
    }

    int target = (unsigned char)argv[2][0];
    long hits = 0;
    int c;
    while ((c = getc(fp)) != EOF)
        if (c == target)
            hits++;

    if (ferror(fp)) {                 /* 迴圈停下來，是讀完了還是出錯？ */
        perror(argv[1]);
        fclose(fp);
        return 1;
    }
    fclose(fp);

    printf("%ld %s\n", hits, argv[1]);
    return 0;
}
```

編譯：`gcc -Wall countch.c -o countch`

#### 3-1. 錯誤是在 `fopen` 那一步還是之後才發生的？為什麼？

**第 1 次（YYYY-MM-DD）**
```
```

#### 3-2. 如果把 `while` 迴圈後面的 `if (ferror(fp)) { ... }` 整段刪掉，`./countch adir a` 會印出什麼、結束狀態是多少？為什麼這樣是錯的？

**第 1 次（YYYY-MM-DD）**
```
```

### 4. 情境：`stdout`／`stderr` 與結束狀態

情境題：用第 3 題的 `countch`（`./countch 檔名 字元` 把「次數 檔名」用 `printf` 印到 `stdout`、回傳 0；開檔失敗時用 `perror(檔名)` 印到 `stderr`、回傳 1；完整原始碼在第 3 題），`printf 'banana\n' > fruit.txt` 之後執行 `./countch fruit.txt a > result.txt; ./countch nofile.txt a >> result.txt; echo "exit=$?"`。(1) 終端機上會看到哪幾行？`result.txt` 裡有什麼？為什麼？(2) 用 `printf 'hello world\n' > one.txt; printf 'no newline at end' > nonl.txt` 建兩個檔案（`nofile.txt` 不存在），GNU `wc one.txt nofile.txt nonl.txt` 印了兩個檔案的結果、一行錯誤和 `total`，結束狀態卻是 1。為什麼這類工具遇到壞檔案不馬上停，但最後要回傳 1？

#### 4-1. 終端機上會看到哪幾行？`result.txt` 裡有什麼？為什麼？

**第 1 次（YYYY-MM-DD）**
```
```

#### 4-2. GNU `wc one.txt nofile.txt nonl.txt` 印了兩個檔案的結果和一行錯誤，結束狀態卻是 1。為什麼這類工具遇到壞檔案不馬上停，但最後要回傳 1？

**第 1 次（YYYY-MM-DD）**
```
```

### 5. 情境：`argv` 與萬用字元

情境題：資料夾裡有 `a.txt`、`b.txt` 兩個檔案，用下面的 `args.c`。(1) `./args '*.txt' *.txt` 的 `argc` 是多少？`argv[1]`～`argv[argc]` 各是什麼？為什麼 `'*.txt'` 和 `*.txt` 的結果不一樣？(2) 誰把 `*.txt` 換成檔名、在什麼時候？程式有沒有辦法知道使用者原本打的是 `*.txt`？為什麼？(3) 換到一個**沒有任何 `.txt`** 的資料夾執行 `../args *.txt`，在 bash 和 zsh 下各會怎樣？

```c
/* args.c */
#include <stdio.h>

int main(int argc, char *argv[])
{
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++)
        printf("argv[%d] = \"%s\"\n", i, argv[i]);
    printf("argv[%d] = %p\n", argc, (void *)argv[argc]);   /* 最後一格固定是 NULL */
    return 0;
}
```

編譯：`gcc -Wall args.c -o args`

#### 5-1. `./args '*.txt' *.txt` 的 `argc` 是多少？`argv[1]`～`argv[argc]` 各是什麼？為什麼 `'*.txt'` 和 `*.txt` 的結果不一樣？

**第 1 次（YYYY-MM-DD）**
```
```

#### 5-2. 誰把 `*.txt` 換成檔名、在什麼時候？程式有沒有辦法知道使用者原本打的是 `*.txt`？為什麼？

**第 1 次（YYYY-MM-DD）**
```
```

#### 5-3. 換到一個**沒有任何 `.txt`** 的資料夾執行 `../args *.txt`，在 bash 和 zsh 下各會怎樣？

**第 1 次（YYYY-MM-DD）**
```
```

### 6. `strace` 輸出怎麼讀（2026-11-04 追加，課後討論第 1 章）

用第 3 題的 `countch.c` 跑 `strace -o tr.txt ./countch fruit.txt n`（`fruit.txt` 用 `printf 'banana\nbandana\n' > fruit.txt` 建、`gcc -Wall countch.c -o countch` 編），`cat -n tr.txt` 裡有下面幾行（行號是 `cat -n` 加的）：

```text
     4	access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
    33	openat(AT_FDCWD, "fruit.txt", O_RDONLY) = 3
    35	read(3, "banana\nbandana\n", 4096)      = 15
    36	read(3, "", 4096)                       = 0
    40	exit_group(0)                           = ?
    41	+++ exited with 0 +++
```

（`countch.c` 的完整原始碼見本檔第 3 題。）

#### 6-1. 第 4 行和第 36 行各代表什麼結果？為什麼第 36 行不算錯誤？

**第 1 次（YYYY-MM-DD）**
```
```

#### 6-2. 第 35 行的 `4096` 和 `15` 各是什麼？為什麼兩個數字不一樣？引號裡的字串是傳進去的，還是讀出來的？

**第 1 次（YYYY-MM-DD）**
```
```

#### 6-3. 第 33 行的 `AT_FDCWD` 是什麼意思？回傳的 `3` 是什麼、為什麼剛好是 3？

**第 1 次（YYYY-MM-DD）**
```
```

#### 6-4. 第 40 行的回傳值為什麼是 `?`？第 41 行是不是系統呼叫？

**第 1 次（YYYY-MM-DD）**
```
```

### 7. 情境：`fopen` 用 `"w"` 寫檔的系統呼叫（2026-11-04 追加，課後討論第 1 章）

情境題：下面的 `savemsg.c` 用 `fopen(…, "w")` 把一行訊息寫進檔案（檔案：[`savemsg.c`](./examples/savemsg/savemsg.c)，編譯：`gcc -Wall savemsg.c -o savemsg`）。執行前 `note.txt` 不存在，`strace -o tr.txt ./savemsg note.txt hello` 裡跟這個檔案有關的是：

```c
/* savemsg.c：把第二個參數寫進第一個參數指定的檔案（檔案不存在就建立、存在就清空） */
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "用法：%s 檔名 訊息\n", argv[0]);
        return 2;
    }
    FILE *fp = fopen(argv[1], "w");
    if (fp == NULL) {
        perror(argv[1]);
        return 1;
    }
    fprintf(fp, "%s\n", argv[2]);
    fclose(fp);
    return 0;
}
```

```text
    33	openat(AT_FDCWD, "note.txt", O_WRONLY|O_CREAT|O_TRUNC, 0666) = 3
    34	fstat(3, {st_mode=S_IFREG|0664, st_size=0, ...}) = 0
    35	write(3, "hello\n", 6)                  = 6
    36	close(3)                                = 0
```

一般使用者換成執行 `strace -o tr.txt ./savemsg /etc/passwd hi`，則是：

```text
    33	openat(AT_FDCWD, "/etc/passwd", O_WRONLY|O_CREAT|O_TRUNC, 0666) = -1 EACCES (Permission denied)
    37	write(2, "/etc/passwd: Permission denied\n", 31) = 31
```

#### 7-1. 第一次的 `openat` 多了第 4 個參數 `0666`，`countch` 開 `fruit.txt` 那次（`openat(AT_FDCWD, "fruit.txt", O_RDONLY) = 3`）沒有，為什麼？

**第 1 次（YYYY-MM-DD）**
```
```

#### 7-2. 第一次的 `write` 第一個參數是 `3`，`countch` 最後印結果的 `write(1, "4 fruit.txt\n", 12)` 是 `1`，為什麼不一樣？各寫到哪裡？

**第 1 次（YYYY-MM-DD）**
```
```

#### 7-3. 第二次從 `openat` 失敗到第 37 行的 `write`，中間發生了什麼（誰把錯誤碼交給誰、誰印出訊息、為什麼寫到 fd 2）？為什麼程式不必自己寫 `"Permission denied"` 這段字？

**第 1 次（YYYY-MM-DD）**
```
```
