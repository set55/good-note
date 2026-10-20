# s1-06 自我測驗作答

> 第 ① 站第 6 課的自我測驗寫在這裡。題目從筆記複製過來；批改完後，最新版的參考答案也寫在筆記每題下面。
>
> **怎麼寫**：直接在每題下面的空白區塊裡作答，**闔上筆記、用自己的話**（把 `YYYY-MM-DD` 換成作答日期）。寫完跟 Claude 說「幫我批改」。
> 批改與參考答案會寫在你的答案下方，**不重答**：答錯或不完整也直接附參考答案，沒弄懂的交給間隔複習再考。
>
> **複習**：間隔複習的作答寫在 repo 根目錄的 [`review/`](../../review/README.md)（每天一個檔），排程見 [複習排程](../../learning/複習排程.md)。本檔只放第一次作答。

## 目錄

| 測驗 | 對應筆記 | 狀態 |
|---|---|---|
| [第 6 課：多檔案的 C 程式](#未答第-6-課多檔案的-c-程式) | [多檔案的C程式-標頭檔、static與extern、連結錯誤與make](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html) | 未答 |

---

## 未答：第 6 課：多檔案的 C 程式

> 題目來自 [多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html)。
> 每題拆成小題（`N-M`），每個小題各有一個作答框；題目用到的程式碼附在每個大題的開頭。批改寫在該小題的作答框正下方。

### 1. 忘了宣告：`square(100000)` 印出 1410065408

`main.c` 沒有 `square` 的宣告，`gcc -c main.c` 只有一個警告，連結也成功，但 `square(100000)` 印出 1410065408 而不是 10000000000。為什麼編譯器會產生錯的程式？為什麼連結器沒有發現兩邊型別不一樣？`main.o` 裡的 `cltq` 在這個錯誤裡扮演什麼角色？

```c
/* mathx.c */
long square(long x)
{
    return x * x;
}
```
```c
/* main.c */
#include <stdio.h>

int main(void)
{
    long r = square(100000);
    printf("%ld\n", r);
    return 0;
}
```
```asm
# objdump -dr --no-show-raw-insn main.o（main 的一部分）
  c:	mov    $0x186a0,%edi         # 第 1 個參數 = 100000
 11:	mov    $0x0,%eax
 16:	call   1b <main+0x1b>        # 呼叫 square（位址連結時才填）
			17: R_X86_64_PLT32	square-0x4
 1b:	cltq                         # 把 %eax 符號擴展成 %rax（= movslq %eax,%rax）
 1d:	mov    %rax,-0x8(%rbp)       # 存進 r
```
編譯與執行：`gcc -c main.c && gcc -c mathx.c && gcc -o calc main.o mathx.o && ./calc`

#### 1-1. 為什麼編譯器會產生錯的程式？

**第 1 次（YYYY-MM-DD）**
```
```

#### 1-2. 為什麼連結器沒有發現兩邊型別不一樣？

**第 1 次（YYYY-MM-DD）**
```
```

#### 1-3. `main.o` 裡的 `cltq` 在這個錯誤裡扮演什麼角色？

**第 1 次（YYYY-MM-DD）**
```
```

### 2. 定義的 `.c` 也要引入自己的標頭檔

為什麼「定義函式的那個 `.c` 也要 `#include` 自己的標頭檔」？如果 `mathx.c` 沒有 `#include "mathx.h"`，有人把 `mathx.c` 的定義改成 `int square(int x)` 卻忘了改 `mathx.h`，會發生什麼事、在哪個階段（編譯、連結、執行）才發現？

```c
/* mathx.h */
#ifndef MATHX_H
#define MATHX_H

long square(long x);

#endif
```

#### 2-1. 為什麼「定義函式的那個 `.c` 也要 `#include` 自己的標頭檔」？

**第 1 次（YYYY-MM-DD）**
```
```

#### 2-2. 如果 `mathx.c` 沒有 `#include "mathx.h"`，有人把 `mathx.c` 的定義改成 `int square(int x)` 卻忘了改 `mathx.h`，會發生什麼事、在哪個階段（編譯、連結、執行）才發現？

**第 1 次（YYYY-MM-DD）**
```
```

### 3. `static` 的兩種意思

`static` 寫在函式外面和寫在函式裡面，各改變了什麼？用下面兩個例子說明：(a) `a.c`、`b.c` 各有一個 `static int helper(void)`，為什麼能一起連結，拿掉 `static` 為什麼就 `multiple definition`？(b) `next_id()` 呼叫三次回傳 1、2、3，`plain_id()` 每次都是 1，兩個 `n` 各放在記憶體的哪裡？

```c
/* a.c（b.c 一樣，只是 helper 回傳 2、函式叫 b_value） */
static int helper(void)
{
    return 1;
}

int a_value(void)
{
    return helper();
}
```
```c
/* nextid.c 的兩個函式 */
int next_id(void)
{
    static int n = 0;
    n = n + 1;
    return n;
}

int plain_id(void)
{
    int n = 0;
    n = n + 1;
    return n;
}
```

#### 3-1. `static` 寫在函式外面和寫在函式裡面，各改變了什麼？

**第 1 次（YYYY-MM-DD）**
```
```

#### 3-2. (a) `a.c`、`b.c` 各有一個 `static int helper(void)`，為什麼能一起連結，拿掉 `static` 為什麼就 `multiple definition`？

**第 1 次（YYYY-MM-DD）**
```
```

#### 3-3. (b) `next_id()` 呼叫三次回傳 1、2、3，`plain_id()` 每次都是 1，兩個 `n` 各放在記憶體的哪裡？

**第 1 次（YYYY-MM-DD）**
```
```

### 4. 標頭檔裡的 `long hits;`

下面的 `hits.h` 被 `hits.c` 和 `hits_main.c` 都 `#include`，兩個 `.c` 都沒有再寫 `hits` 的定義。本機 gcc 13 連結時出現 `multiple definition of 'hits'`。為什麼？已經有 include guard 了，為什麼沒擋住？該怎麼改（標頭檔寫什麼、哪裡寫什麼）？為什麼同樣的程式碼在舊版 gcc 上可以連結？

```c
/* hits.h */
#ifndef HITS_H
#define HITS_H

long hits;
void hit(void);

#endif
```
```c
/* hits.c */
#include "hits.h"

void hit(void)
{
    hits = hits + 1;
}
```
```c
/* hits_main.c */
#include <stdio.h>
#include "hits.h"

int main(void)
{
    hit();
    hit();
    hit();
    printf("hits = %ld\n", hits);
    return 0;
}
```
編譯與連結：`gcc -c hits.c hits_main.c && gcc -o hits hits_main.o hits.o`

#### 4-1. 為什麼？

**第 1 次（YYYY-MM-DD）**
```
```

#### 4-2. 已經有 include guard 了，為什麼沒擋住？

**第 1 次（YYYY-MM-DD）**
```
```

#### 4-3. 該怎麼改（標頭檔寫什麼、哪裡寫什麼）？

**第 1 次（YYYY-MM-DD）**
```
```

#### 4-4. 為什麼同樣的程式碼在舊版 gcc 上可以連結？

**第 1 次（YYYY-MM-DD）**
```
```

### 5. 情境題：改了標頭檔，`make` 說不用重編

情境題：同事的專案用下面的 `Makefile`。他把 `config.h` 裡的 `#define BUF_SIZE 64` 改成 `128`，存檔後執行 `make`，`make` 只印了 `make: 'server' is up to date.`，跑起來的緩衝區還是 64。`make` 是怎麼判斷不用重編的？為什麼這個判斷錯了？你會怎麼改這個 `Makefile`，讓以後改任何標頭檔都會重編該重編的檔案？（不必寫出完整的 `Makefile`，說明要改哪些地方、各自的作用即可。）

```make
server: server.o net.o
	gcc -o server server.o net.o

server.o: server.c
	gcc -Wall -c server.c

net.o: net.c
	gcc -Wall -c net.c
```
（`server.c` 和 `net.c` 都 `#include "config.h"`；命令前面是 TAB。）

#### 5-1. `make` 是怎麼判斷不用重編的？

**第 1 次（YYYY-MM-DD）**
```
```

#### 5-2. 為什麼這個判斷錯了？

**第 1 次（YYYY-MM-DD）**
```
```

#### 5-3. 你會怎麼改這個 `Makefile`，讓以後改任何標頭檔都會重編該重編的檔案？（不必寫出完整的 `Makefile`，說明要改哪些地方、各自的作用即可。）

**第 1 次（YYYY-MM-DD）**
```
```

### 6. 符號是什麼：`t.o` 的符號表裡有誰

（2026-10-20 提問「符號代表什麼」追加，出處：[第一節](../../html/c/多檔案的C程式-標頭檔、static與extern、連結錯誤與make.html#一本課在做什麼編譯器一次只看一個-c)〈上面說的「符號」到底是什麼〉）下面的 `t.c` 用 `gcc -c t.c` 編成 `t.o`。哪些名字會出現在 `t.o` 的符號表裡？其中哪些是「這個檔案提供」、哪些是「需要別人提供」？`n`、`sum`、`i` 為什麼不在符號表裡？對連結器來說，符號表裡的一個符號代表什麼、拿來做什麼用？

```c
#include <stdio.h>

long total = 0;
extern long limit;

void add(long n)
{
    long sum = total + n;
    total = sum;
}

int main(void)
{
    for (int i = 0; i < 3; i++)
        add(i);
    if (total > limit)
        puts("big");
    return 0;
}
```

編譯：`gcc -c t.c`（只編不連結；`limit` 沒有定義，連結會失敗，這題不用連結）

#### 6-1. 哪些名字會出現在 `t.o` 的符號表裡？其中哪些是「這個檔案提供」、哪些是「需要別人提供」？

**第 1 次（YYYY-MM-DD）**
```
```

#### 6-2. `n`、`sum`、`i` 為什麼不在符號表裡？

**第 1 次（YYYY-MM-DD）**
```
```

#### 6-3. 對連結器來說，符號表裡的一個符號代表什麼、拿來做什麼用？

**第 1 次（YYYY-MM-DD）**
```
```
