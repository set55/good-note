# 字元是什麼？—— `char` 就是 1 byte 的整數，字元靠編碼表對應成數字

> 日期：2026-09-24
> 情境：第 ① 站第 2 課的延伸提問。[第 2 課筆記](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md) 裡出現了 `char`、`%c`、
> 「印幾個字元」，使用者問「字元是什麼？」
> 適用範圍：實測本機 Ubuntu 24.04、x86-64、gcc 13.3.0、gdb 15.1，locale 是 `en_US.UTF-8`；所有輸出都是本機實際執行結果
> 對應檔案：實驗程式 `chars.c`、`utf.c`、`count.c` 的完整原始碼都寫在正文裡，可以直接複製編譯
> 相關：[C的資料在記憶體裡長怎樣](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md)（第一節 `char` 有號、第六節符號擴展）

**一句話結論：字元（character）是一個符號：字母、數字、標點、換行。電腦記不住符號，只記得住數字，所以用一張
編碼表（encoding）把每個符號對應成一個數字：`A` 是 65、`0` 是 48、換行是 10。C 的 `char` 其實就是 **1 byte 的整數**，
`%c` 和 `%d` 只是同一個數字的兩種印法。英文字母一個 byte 就裝得下（ASCII）；中文字要用 UTF-8 編成 **3 個 byte**，
所以 C 的「一個 `char`」不等於人眼中的「一個字」：「字」是 3 個 `char`（byte），但只算 1 個 Unicode 字元。**

---

## 一、字元就是一個數字

`chars.c` 完整原始碼：

```c
#include <stdio.h>

int main(void)
{
    char c = 'A';
    char d = 65;
    char nl = '\n';
    char zero = '0';

    printf("c    : %%c=%c  %%d=%d  hex=0x%x\n", c, c, c);    /* 第 10 行 */
    printf("d    : %%c=%c  %%d=%d\n", d, d);
    printf("c + 1: %%c=%c  %%d=%d\n", c + 1, c + 1);
    printf("'0'  : %%c=%c  %%d=%d\n", zero, zero);
    printf("'\\n' : %%d=%d\n", nl);
    printf("sizeof(c) = %zu, sizeof('A') = %zu\n", sizeof(c), sizeof('A'));
    printf("sizeof(\"A\") = %zu, sizeof(\"字\") = %zu\n", sizeof("A"), sizeof("字"));
    return 0;
}
```

```
$ gcc -Wall -Wextra -g -O0 chars.c -o chars && ./chars
c    : %c=A  %d=65  hex=0x41
d    : %c=A  %d=65          ← 寫 'A' 和寫 65，存進去的是同一個東西
c + 1: %c=B  %d=66          ← 字元可以做加法：'A' + 1 就是 'B'
'0'  : %c=0  %d=48          ← 字元 '0' 不是數字 0，它的編號是 48
'\n' : %d=10                ← 換行也是一個字元，編號 10
sizeof(c) = 1, sizeof('A') = 4               ← 第三節
sizeof("A") = 2, sizeof("字") = 4            ← 第三、四節
```

用 `gdb` 看記憶體，`c` 那一格存的就是一個 byte `0x41`：

```
$ gdb -q ./chars
(gdb) break 10          ← chars.c 標註的第 10 行
(gdb) run
(gdb) x/1xb &c
0x7fffffffd26c:	0x41
(gdb) print c
$1 = 65 'A'                 ← gdb 把兩種讀法都印出來
```

這跟 [第 2 課](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md) 的主線是同一件事：**記憶體裡只有數字，怎麼解讀是別人決定的**。
這裡決定解讀方式的是 `printf` 的格式：`%c` 拿數字去查編碼表、印出對應的符號；`%d` 直接印數字。

**`'0'` 和 `0` 的差別**是最常見的錯誤來源：`'0'` 是「數字 0 這個符號」，編號 48；`0` 是數值零。
因為 `'0'`～`'9'` 在編碼表裡是連號的（48～57），要把數字字元轉成數值，減掉 `'0'` 就好：

```
'7' - '0' = 7
```

---

## 二、編碼表：ASCII

C 程式裡 `'A'` 是 65，是因為 **ASCII**（American Standard Code for Information Interchange，美國資訊交換標準碼）這張表這樣規定。
ASCII 只有 128 個（0～127），7 個 bit 就裝得下，一個 byte 綽綽有餘。**完整的表見 `man ascii`**（本機有安裝），開頭長這樣：

```
$ man ascii
       Oct   Dec   Hex   Char                        Oct   Dec   Hex   Char
       ────────────────────────────────────────────────────────────────────────
       000   0     00    NUL '\0' (null character)   100   64    40    @
       001   1     01    SOH (start of heading)      101   65    41    A
       002   2     02    STX (start of text)         102   66    42    B
       ...
```

欄位是八進位、十進位、十六進位、符號。表的結構，記住這幾段就夠了（這裡只是分段摘要，逐個字元的完整清單請查 `man ascii`）：

| 範圍（十進位） | 內容 |
|---|---|
| 0～31 | **控制字元**：不是拿來顯示的，而是控制動作，例如 0 是 NUL（空字元）、9 是 Tab、10 是換行、13 是歸位（carriage return） |
| 32 | 空白 |
| 33～47、58～64、91～96、123～126 | 標點與符號（`!`、`#`、`@`、`[`、`{` 等） |
| 48～57 | `0`～`9` |
| 65～90 | `A`～`Z` |
| 97～122 | `a`～`z`（小寫比大寫多 32） |
| 127 | DEL（刪除），也是控制字元 |

---

## 三、C 的字元常數：單引號，而且型別是 `int`

`'A'` 這種單引號的寫法叫**字元常數（character constant）**。兩件事要注意：

- **單引號是一個字元，雙引號是字串**：`'A'` 是數字 65；`"A"` 是字串，第 3 課會講，這裡只看大小：
  `sizeof("A")` 是 **2**，因為字串後面還藏了一個結尾的 `'\0'`（值是 0 的 byte）。
- **在 C 裡，`'A'` 的型別是 `int`，不是 `char`**：

```
sizeof(c) = 1, sizeof('A') = 4          ← chars.c 輸出的第 6 行
```

  存進 `char` 變數時才會變窄成 1 byte。（C++ 的 `'A'` 是 `char`，這是兩個語言的差異之一。）

### 跳脫序列（escape sequence）：全部列出

控制字元打不出來、單引號本身又被語法佔用，所以 C 用反斜線開頭的寫法表示它們。
以下是 C 標準規定的**全部**。「值」那一欄是第四節 `utf.c` 最後一行實測印出來的：

| 寫法 | 值 | 意思 |
|---|---|---|
| `\a` | 7 | 響鈴（alert） |
| `\b` | 8 | 退格（backspace） |
| `\f` | 12 | 換頁（form feed） |
| `\n` | 10 | 換行（newline） |
| `\r` | 13 | 歸位（carriage return），游標回到行首 |
| `\t` | 9 | 水平 Tab |
| `\v` | 11 | 垂直 Tab |
| `\\` | 92 | 反斜線本身 |
| `\'` | 39 | 單引號 |
| `\"` | 34 | 雙引號 |
| `\?` | 63 | 問號（歷史原因，避免被當成三字元序列） |
| `\` 加 1～3 位八進位 | — | 直接寫編號：`\101` 是 65（`A`），`\0` 是 0 |
| `\x` 加十六進位 | — | 直接寫編號：`\x41` 是 65（`A`） |
| `\u` 加 4 位、`\U` 加 8 位十六進位 | — | Unicode 編號（universal character name），例如 `\u5b57` 是「字」 |

`\0` 就是 ASCII 表的第 0 號 NUL，它是 C 字串的結尾記號（第 3 課）。

---

## 四、中文字：一個 byte 裝不下，UTF-8 用 3 個 byte

ASCII 只有 128 個位置，放不下中文。現在通用的做法是 **Unicode**：給全世界每個符號一個編號（「字」是 U+5B57），
再用 **UTF-8**（Unicode Transformation Format – 8-bit）這種**編碼方式**，把編號轉成 1～4 個 byte 存起來。
英文字母在 UTF-8 裡跟 ASCII 完全一樣，還是 1 個 byte；中文字通常是 3 個。

```
sizeof("A") = 2, sizeof("字") = 4        ← chars.c 輸出的最後一行：「字」3 個 byte，加上結尾的 '\0'

（同一個 gdb 工作階段，停在第 10 行）
(gdb) x/2xb "A"
0x5555555592a0:	0x41	0x00
(gdb) x/4xb "字"
0x5555555592c0:	0xe5	0xad	0x97	0x00
```

（這裡的位址是 `gdb` 為了印這個字串，臨時放進程式記憶體的位置，不用在意。）

本機的原始碼、終端機都是 UTF-8（`locale` 顯示 `LANG=en_US.UTF-8`），所以在 `chars.c` 裡打的「字」，存進檔案就是這 3 個 byte。
UTF-8 怎麼把編號拆成 byte 的規則見 `man utf-8`（本機有安裝）。

**所以 C 的 `char` 不等於「一個字」**：`char` 是 1 byte，「字」要 3 個 `char` 才裝得下。
將來 `strlen` 算字串長度時，算的是 **byte 數**，不是人眼看到的字數。

### 跟第 2 課的連結：`char` 有號，讀到中文會變負數

[第 2 課第一節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#一位元組與型別大小)說本機的 `char` 是有號的。UTF-8 的中文 byte 都大於 127（最高位是 1），用 `char` 讀就變成負數：

`"字"[0]` 是取字串的第一個 byte `0xe5`（`[0]` 的語法第 3 課教）。`utf.c` 完整原始碼：

```c
#include <stdio.h>

int main(void)
{
    char s = "字"[0];
    unsigned char u = "字"[0];
    int from_s = s, from_u = u;
    char digit = '7';

    printf("char: %d, unsigned char: %d\n", from_s, from_u);
    printf("'7' - '0' = %d\n", digit - '0');
    printf("escapes: \\a=%d \\b=%d \\f=%d \\n=%d \\r=%d \\t=%d \\v=%d \\\\=%d \\'=%d \\\"=%d \\?=%d \\0=%d \\101=%d \\x41=%d\n",
           '\a', '\b', '\f', '\n', '\r', '\t', '\v', '\\', '\'', '\"', '\?', '\0', '\101', '\x41');
    return 0;
}
```

```
$ gcc -Wall -Wextra utf.c -o utf && ./utf
char: -27, unsigned char: 229
'7' - '0' = 7                                    ← 第一節的 '0' 轉換
escapes: \a=7 \b=8 \f=12 \n=10 \r=13 \t=9 \v=11 \\=92 \'=39 \"=34 \?=63 \0=0 \101=65 \x41=65   ← 第三節的跳脫序列表
```

同一個 byte `0xe5`：當 `signed char` 讀，最高位是符號位，二補數是 −27，擴展成 `int` 時做符號擴展，還是 −27；
當 `unsigned char` 讀是 229，零擴展後還是 229。這正是[第 2 課第一節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#一位元組與型別大小)說「要處理「原始位元組」時，一律寫 `unsigned char`」的實際例子（理由在[第 2 課第六節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#六有號與無號之間的轉換位元不變換個讀法)最後一段）：
用 `char` 處理 UTF-8 文字，一拿去做比較或當索引就會出錯。

### 判斷「是不是非 ASCII 的 byte」：用哪個型別、為什麼是 127

> 2026-09-27 自我測驗第 4 題追問：「改成 `unsigned int` 好嗎？具體 `> 127` 就好嗎？為什麼是 127？」

**為什麼是 127**：ASCII 只用 7 個 bit，值是 0～127（`0x00`～`0x7f`）。UTF-8 刻意這樣設計：ASCII 字元照舊是 1 個 byte、值不變；
非 ASCII 字元（例如中文）的**每一個** byte 最高位都是 1，也就是 ≥ 128（`0x80`～`0xff`）。所以「byte 的值 > 127」就等於「這個 byte 不是 ASCII」。
前提是**用 0～255 的讀法**去讀這個 byte——這正是 `char` 做不到的地方。

**用哪個型別**：同一個 byte `0xe5`（「字」的第一個 byte）用三種型別讀。`bytes.c` 完整原始碼：

```c
#include <stdio.h>

int main(void)
{
    const char *s = "字";
    char c = s[0];
    unsigned char uc = s[0];
    unsigned int ui = s[0];

    printf("char          : %d  (c > 127 -> %d, c < 0 -> %d)\n", c, c > 127, c < 0);    /* 第 10 行 */
    printf("unsigned char : %d  (uc > 127 -> %d)\n", uc, uc > 127);
    printf("unsigned int  : %u  (ui > 127 -> %d)\n", ui, ui > 127);
    printf("'A' as unsigned char > 127 -> %d\n", (unsigned char)'A' > 127);
    return 0;
}
```

（`s[0]` 是取字串第一個 byte，語法第 3 課教。）

```
$ gcc -Wall -Wextra bytes.c -o bytes && ./bytes
bytes.c:10:71: warning: comparison is always false due to limited range of data type [-Wtype-limits]
char          : -27  (c > 127 -> 0, c < 0 -> 1)
unsigned char : 229  (uc > 127 -> 1)
unsigned int  : 4294967269  (ui > 127 -> 1)
'A' as unsigned char > 127 -> 0
```

| 型別 | `0xe5` 讀成 | `> 127` | 評語 |
|---|---|---|---|
| `char`（本機有號，−128～127） | −27 | **永遠假** | 就是題目的 bug。`-Wextra` 的 `-Wtype-limits` 直接警告「因為型別範圍有限，永遠為假」（[第 2 課第七節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#七陷阱有號和無號混在一起比較)〈`-Wtype-limits` 只抓「永遠成立／永遠不成立」的比較〉的同一個警告） |
| `unsigned char`（0～255） | **229** | 真 | **正解**：byte 本來就是 0～255 的數，值正確，`> 127` 的意思也正確 |
| `unsigned int` | 4294967269 | 真 | 判斷**碰巧**對了，但值是錯的：`s[0]` 是有號的 `char`，轉成 `unsigned int` 時先照原本的型別做**符號擴展**（[第 2 課第六節](./C的資料在記憶體裡長怎樣-位元組、指標、位元組順序與有號無號.md#六有號與無號之間的轉換位元不變換個讀法)），變成 `0xffffffe5`。拿去當陣列索引、查表、印出來都會出錯 |

所以正確的改法是**把變數改成 `unsigned char`**（或比較時寫 `(unsigned char)c > 127`），而不是換成更大的無號型別：
問題出在 `char` 把 byte 讀成負數，要修的是「讀法」，換成 `unsigned int` 只是把 −27 變成另一個錯的數。
（用 `char` 硬要判斷也可以寫 `c < 0`，但這依賴 `char` 在這台機器上有號——ARM 上的 `char` 通常是無號的，同一行程式會變成永遠假。）

---

## 五、那「字」是 3 個字元嗎？——看你說的是哪一種「字元」

> 2026-09-24 追問：「那這樣中文字就是 3 個字元？」

「字元」這個詞有兩個意思，要先分清楚：

| 說法 | 指的是 | 「字」算幾個 | 「字A」算幾個 |
|---|---|---|---|
| C 的 `char`（位元組） | 1 byte | **3** | 4 |
| Unicode 的字元（碼位，code point） | 編碼表裡的一個編號，例如 U+5B57 | **1** | 2 |

C 的 `char` 取名的年代，英文一個字元剛好一個 byte，兩種意思沒有差別；到了 UTF-8 才分開。
所以**「字」是 3 個 `char`（3 個 byte），但只是 1 個字元**。說「字元」時，要看上下文指的是哪一種；
嚴謹的文件會直接說 byte 或 code point。

實測兩種數法。`setlocale(LC_ALL, "")` 照環境變數設定 locale，程式才知道文字是 UTF-8；
`mbstowcs(NULL, s, 0)` 第一個參數給 `NULL` 代表只數不存；`L"字A"` 是寬字元字串，每個字元一個 `wchar_t`。`count.c` 完整原始碼：

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>

int main(void)
{
    const char *s = "字A";

    setlocale(LC_ALL, "");
    printf("strlen(\"字A\")   = %zu   (bytes)\n", strlen(s));
    printf("mbstowcs count  = %zu   (characters)\n", mbstowcs(NULL, s, 0));
    printf("wcslen(L\"字A\") = %zu\n", wcslen(L"字A"));
    printf("sizeof(wchar_t) = %zu\n", sizeof(wchar_t));
    printf("L'字' = 0x%x\n", (unsigned)L'字');
    return 0;
}
```

```
$ gcc -Wall -Wextra count.c -o count && ./count
strlen("字A")   = 4   (bytes)            ← 3 + 1
mbstowcs count  = 2   (characters)       ← 字、A
wcslen(L"字A") = 2
sizeof(wchar_t) = 4                       ← Linux 上一個寬字元 4 byte，裝得下任何 Unicode 編號
L'字' = 0x5b57                            ← 寬字元存的就是 Unicode 編號 U+5B57
```

用到的東西（C 的多位元組與寬字元函式很多，本機 `man -k '^wcs'` 就有 24 個，這裡**只列本節用到的 5 個**，各自見 `man 3 <名稱>`）：

| 名稱 | 標頭檔 | 作用 |
|---|---|---|
| `strlen` | `<string.h>` | 數到 `'\0'` 為止有幾個 **byte** |
| `setlocale(LC_ALL, "")` | `<locale.h>` | 依環境變數（`LANG` 等）設定程式的語系；不呼叫的話，C 程式預設是 `"C"` locale，只認 ASCII |
| `mbstowcs` | `<stdlib.h>` | multibyte string to wide-character string：把 UTF-8 這種多位元組文字轉成寬字元；第一個參數給 `NULL` 時只回傳字元數 |
| `wchar_t`、`L"..."`、`L'...'` | `<wchar.h>` | 寬字元型別，與寬字元字串／字元常數的寫法 |
| `wcslen` | `<wchar.h>` | 數寬字元字串有幾個**字元** |

**`setlocale` 一定要呼叫**：在 `C` locale 下，`mbstowcs` 看不懂 UTF-8，回傳錯誤：

```
$ LC_ALL=C ./count
mbstowcs count  = 18446744073709551615   (characters)
```

這個怪數字就是第 2 課的內容：`mbstowcs` 用 `(size_t)-1` 表示錯誤，`-1` 轉成 8 byte 的無號 `size_t` 就是 2⁶⁴ − 1。

（其他語言多半直接用「字元」的意思：Python 的 `len("字A")` 是 2，`len("字A".encode())` 才是 4 byte。）

---

## 自我測驗

1. `char c = 'A';` 和 `char c = 65;` 存進記憶體的東西一樣嗎？為什麼 `printf` 用 `%c` 印出 `A`、用 `%d` 印出 65？
2. 使用者輸入的字元是 `'7'`，你想得到數值 7。直接把它當數字用會得到多少？該怎麼轉，這個方法為什麼成立？
3. `sizeof('A')`、`sizeof("A")`、`sizeof("字")` 在本機各是多少？分別解釋為什麼。
4. 情境題：你寫了一個函式，用 `char c` 一個一個 byte 讀一段 UTF-8 文字，並用 `if (c > 127)` 判斷「這是不是非 ASCII 的 byte」。
   在本機這個判斷會成立嗎？為什麼？該怎麼改？
5. 有人說「『字』是 3 個字元」，另一個人說「是 1 個字元」，誰對？`strlen("字")` 和 `mbstowcs(NULL, "字", 0)` 各會得到多少？
   如果程式忘了呼叫 `setlocale`，後者會得到什麼，那個數字是怎麼來的？
