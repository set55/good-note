# s1-01：hello.c 的四個編譯階段

> 對應課程：第 ① 站第 1 課
> 課程筆記：[hello.c怎麼變成能跑的程式-編譯的四個階段.md](../../c/hello.c怎麼變成能跑的程式-編譯的四個階段.md)
> 自我測驗：寫在同資料夾的 [quiz.md](./quiz.md)
> 2026-09-27 重排版面：照 `CLAUDE.md` 的新規則改成「題目 → 作答 → 批改 → 重答 → 批改」一路往下。
> 作答內容逐字搬過來（只去掉行尾空白），沒有改動文字。

## 說明

在這個資料夾裡自己寫 `hello.c`（不要複製筆記裡的），然後做下面四題。每題下面是作答框，批改寫在作答框正下方。

## 批改總表

> 四題的實驗操作與輸出**全部正確**。第一次批改時要求解釋本課沒教過的原因，是出題錯誤（沒有標明答案來源），
> 已改成補教；完整解說寫在課程筆記[第九節〈作業解說〉](../../c/hello.c怎麼變成能跑的程式-編譯的四個階段.md#九作業解說四題背後的原因)。
> 各題的批改與參考答案摘要在該題作答框下方（批改日期 2026-09-22）。

| 題 | 第 1 次（2026-09-22） |
|---|---|
| 1 | 輸出**對**；大小差異的原因是補教的新觀念 |
| 2 | 輸出**對**；為什麼能跑是補教的新觀念 |
| 3 | 輸出**對**；為什麼不換成 `puts` 是補教的新觀念 |
| 4 | 輸出**對** |

---

## 1. 用 `gcc -E`、`-S`、`-c` 各停一次，再完整編譯一次；用 `ls -l` 比較四個檔案的大小。

**第 1 次（2026-09-22）**
```
-rwxrwxr-x 1 set set  16K Sep 22 18:26 hello
-rw-rw-r-- 1 set set  110 Sep 22 18:02 hello.c
-rw-rw-r-- 1 set set  21K Sep 22 18:03 hello.i
-rw-rw-r-- 1 set set 1.5K Sep 22 18:22 hello.o
-rw-rw-r-- 1 set set  667 Sep 22 18:17 hello.s
```

> **批改**：**對**。
>
> **參考答案**：`.c` 110 → `.i` 21K 是因為 `#include <stdio.h>` 把整份標頭檔（及它再 include 的檔案）貼進來（筆記第二節）。
> `.o` 1.5K → `hello` 16K：連結器加入啟動碼與動態連結要用的區段（`.plt`、`.got`、`.dynamic`、`.interp` 等，合計約 2K），
> 但主因是**分頁對齊**——4 個 `LOAD` 區塊各自從 4096 bytes 的分頁邊界開始，中間補空白（新觀念，第九節）。

---

## 2. 刪掉 `#include <stdio.h>` 再編譯。是在**哪個階段**出錯？錯誤（或警告）訊息是什麼？

**第 1 次（2026-09-22）**
```
gcc -S -O0 ./hello.c -o hello.s                                                       ok | base py | at 18:42:57
./hello.c: In function ‘main’:
./hello.c:5:9: warning: implicit declaration of function ‘printf’ [-Wimplicit-function-declaration]
    5 |         printf("%s\n", GREETING);
      |         ^~~~~~
./hello.c:1:1: note: include ‘<stdio.h>’ or provide a declaration of ‘printf’
  +++ |+#include <stdio.h>
    1 | #define GREETING "hello, world"
./hello.c:5:9: warning: incompatible implicit declaration of built-in function ‘printf’ [-Wbuiltin-declaration-mismatch]
    5 |         printf("%s\n", GREETING);
      |         ^~~~~~
./hello.c:5:9: note: include ‘<stdio.h>’ or provide a declaration of ‘printf’
在編譯出現警告沒有報錯,而且hello還是能執行
```

> **批改**：**對**（輸出與「只有警告、還能執行」的觀察都正確）。
>
> **參考答案**：出錯的是**編譯階段（`cc1`）**，不是前處理；少了 `#include` 只是少貼一段文字。
> 沒有宣告時編譯器用 C89 舊規則「猜」函式型別，只發警告；**連結器只比對名字、不比對型別**，
> 所以照名字 `printf` 仍然接得上 libc，程式能跑。型別猜錯時（例如回傳 `double` 的函式）會在執行時得到錯誤結果，
> 沒有任何階段會擋下來（新觀念，第九節有 `half(10) = 0.000000` 的實測）。

---

## 3. 把輸出那行改成 `printf("hi %d\n", 42);`，再產生 `.s`。這次還會變成 `puts` 嗎？為什麼？

**第 1 次（2026-09-22）**
```
call	printf@PLT
變成printf 但我不知道爲什麼
```

> **批改**：**對**（觀察到的 `call printf@PLT` 正確；「為什麼」本課沒教過，不算錯，已補教）。
>
> **參考答案**：不會換成 `puts`。`puts(s)` 只能「原封不動印字串再補換行」，不懂任何 `%` 格式；
> `%d` 要把 42 轉成字元 `'4'`、`'2'`，只有 `printf` 做得到。gcc 只在效果剛好等於「印現成字串加換行」時才換（新觀念，第九節）。

---

## 4. `gcc -static hello.c -o hello_static`，比較動態版與靜態版的 `ls -l`、`file`、`ldd`、`strace -c`。

**第 1 次（2026-09-22）**
```
-rwxrwxr-x 1 set set  16K Sep 22 20:42 hello
-rw-rw-r-- 1 set set  110 Sep 22 20:42 hello.c
-rwxrwxr-x 1 set set 767K Sep 22 20:43 hello_static
-rw-rw-r-- 1 set set 2.0K Sep 22 19:21 README.md

hello: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=1fc44f39a13be07fa1f3f3ccc798f52df8d70b90, for GNU/Linux 3.2.0, not stripped

hello_static: ELF 64-bit LSB executable, x86-64, version 1 (GNU/Linux), statically linked, BuildID[sha1]=879fecdf788e0144f71dd23969a463dceea46563, for GNU/Linux 3.2.0, not stripped

~/w/Proj/g/good-note/s/s1-01-hello-compile | on main >10 !5 ?2  ldd hello                                                                             ok | base py | at 20:44:03
        linux-vdso.so.1 (0x0000716061b0d000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x0000716061800000)
        /lib64/ld-linux-x86-64.so.2 (0x0000716061b0f000)

 ~/w/Proj/g/good-note/s/s1-01-hello-compile | on main >10 !5 ?2  ldd hello_static                                                                      ok | base py | at 20:45:22
        not a dynamic executable


~/w/Proj/g/good-note/s/s1-01-hello-compile | on main >10 !5 ?2  strace -c ./hello                                                                  1 err | base py | at 20:46:14
hello, world
% time     seconds  usecs/call     calls    errors syscall
------ ----------- ----------- --------- --------- ----------------
 35.59    0.000200          25         8           mmap
 11.74    0.000066          22         3           mprotect
  9.96    0.000056          18         3           fstat
  8.01    0.000045          45         1           munmap
  6.76    0.000038          19         2           openat
  5.16    0.000029          29         1           write
  4.27    0.000024          12         2           pread64
  3.91    0.000022           7         3           brk
  3.02    0.000017           8         2           close
  2.31    0.000013          13         1           read
  1.96    0.000011          11         1           getrandom
  1.78    0.000010          10         1           prlimit64
  1.42    0.000008           8         1           arch_prctl
  1.42    0.000008           8         1           set_robust_list
  1.42    0.000008           8         1           rseq
  1.25    0.000007           7         1           set_tid_address
  0.00    0.000000           0         1         1 access
  0.00    0.000000           0         1           execve
------ ----------- ----------- --------- --------- ----------------
100.00    0.000562          16        34         1 total

~/w/Proj/g/good-note/s/s1-01-hello-compile | on main >10 !5 ?2  strace -c ./hello_static                                                              ok | base py | at 20:46:23
hello, world
% time     seconds  usecs/call     calls    errors syscall
------ ----------- ----------- --------- --------- ----------------
  0.00    0.000000           0         1           write
  0.00    0.000000           0         1           fstat
  0.00    0.000000           0         1           mprotect
  0.00    0.000000           0         5           brk
  0.00    0.000000           0         1           execve
  0.00    0.000000           0         1           arch_prctl
  0.00    0.000000           0         1           set_tid_address
  0.00    0.000000           0         1           readlinkat
  0.00    0.000000           0         1           set_robust_list
  0.00    0.000000           0         1           prlimit64
  0.00    0.000000           0         1           getrandom
  0.00    0.000000           0         1           rseq
------ ----------- ----------- --------- --------- ----------------
100.00    0.000000           0        16           total
```

> **批改**：**對**。
>
> **參考答案**：靜態版把 libc 用到的程式碼複製進執行檔，所以大約 50 倍大（程式碼區 1375 → 668305 bytes）。
> 動態版的 `openat` 是動態載入器打開 `/etc/ld.so.cache` 與 `libc.so.6`；靜態版沒有 interpreter、libc 已在自己身上，
> 沒有東西要打開，系統呼叫 34 → 16。兩者都只有 1 次 `write`：連結方式只影響程式碼從哪來，不影響最後請核心做的事（筆記第五、六節）。
