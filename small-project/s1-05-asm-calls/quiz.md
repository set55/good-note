# s1-05 自我測驗作答

> 第 ① 站第 5 課的自我測驗寫在這裡。題目從筆記複製過來；批改完後，最新版的參考答案也寫在筆記每題下面。
>
> **怎麼寫**：直接在每題下面的空白區塊裡作答，**闔上筆記、用自己的話**（把 `YYYY-MM-DD` 換成作答日期）。寫完跟 Claude 說「幫我批改」。
> 批改與參考答案會寫在你的答案下方，**不重答**：答錯或不完整也直接附參考答案，沒弄懂的交給間隔複習再考。
>
> **複習**：間隔複習的作答寫在 repo 根目錄的 [`review/`](../../review/README.md)（每天一個檔），排程見 [複習排程](../../learning/複習排程.md)。本檔只放第一次作答。

## 目錄

| 測驗 | 對應筆記 | 狀態 |
|---|---|---|
| [第 5 課：讀懂 x86-64 組合語言（二）](#未答第-5-課讀懂-x86-64-組合語言二) | [x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md) | 未答 |

---

## 未答：第 5 課：讀懂 x86-64 組合語言（二）

> 題目來自 [x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md](../../c/x86-64組合語言入門二-call與ret、堆疊框、參數與呼叫慣例.md)。
> 每題拆成小題（`N-M`），每個小題各有一個作答框；題目用到的程式碼附在每個大題的開頭。

### 1. `call` 與 `ret`

`frame.c` 的 `main` 裡有 `call 1149 <add3>`（位址 `0x1194`），下一條指令在 `0x1199`；`add3` 最後一條是 `ret`。

```asm
# main
1194:	call   1149 <add3>
1199:	mov    %rax,-0x8(%rbp)
# add3 的最後
1177:	pop    %rbp
1178:	ret
```

#### 1-1. `call` 做了哪兩件事？

**第 1 次（YYYY-MM-DD）**
```

```

#### 1-2. `ret` 做了什麼？

**第 1 次（YYYY-MM-DD）**
```

```

#### 1-3. `ret` 怎麼知道要回到 `0x1199`？

**第 1 次（YYYY-MM-DD）**
```

```

#### 1-4. 如果 `add3` 裡有個陣列越界寫入，剛好蓋掉了堆疊上的返回位址，`ret` 會發生什麼事？

**第 1 次（YYYY-MM-DD）**
```

```

---

### 2. 堆疊框

```asm
add3:                                 main:
  push   %rbp                           push   %rbp
  mov    %rsp,%rbp                      mov    %rsp,%rbp
  mov    %rdi,-0x18(%rbp)               sub    $0x10,%rsp
  ...（沒有 sub、沒有 call）              ...（有 call add3、call printf）
  pop    %rbp                           leave
  ret                                   ret
```

#### 2-1. 函式開頭的 `push %rbp` 和 `mov %rsp,%rbp` 各在做什麼？

**第 1 次（YYYY-MM-DD）**
```

```

#### 2-2. `main` 結尾的 `leave` 等於哪兩條指令？

**第 1 次（YYYY-MM-DD）**
```

```

#### 2-3. `add3` 把區域變數放在 `%rsp` 下面、沒有 `sub %rsp`，`main` 卻要 `sub $0x10,%rsp`，為什麼？

**第 1 次（YYYY-MM-DD）**
```

```

---

### 3. 8 個參數

`args8.c` 的 `sum8` 有 8 個 `long` 參數。

```c
long sum8(long a, long b, long c, long d, long e, long f, long g, long h)
{
    return a + b + c + d + e + f + g + h;
}

long call8(void)
{
    return sum8(1, 2, 3, 4, 5, 6, 7, 8);
}
```

```asm
call8:                                sum8 裡讀第 7、8 個參數：
  push   %rbp                           mov    0x10(%rbp),%rax
  mov    %rsp,%rbp                      ...
  push   $0x8                           mov    0x18(%rbp),%rax
  push   $0x7
  mov    $0x6,%r9d
  ...（5～1 分別放進 %r8d、%ecx、%edx、%esi、%edi）
  call   sum8
  add    $0x10,%rsp
  leave
  ret
```

#### 3-1. 前 6 個參數放在哪裡？

**第 1 次（YYYY-MM-DD）**
```

```

#### 3-2. 第 7、8 個放在哪裡、`call8` 用什麼順序放？

**第 1 次（YYYY-MM-DD）**
```

```

#### 3-3. 在 `sum8` 裡第 7 個參數為什麼是 `0x10(%rbp)`，而不是 `0(%rbp)`？

**第 1 次（YYYY-MM-DD）**
```

```

#### 3-4. `call8` 在 `call` 之後為什麼要 `add $0x10,%rsp`？

**第 1 次（YYYY-MM-DD）**
```

```

---

### 4. callee-saved 與對齊

下面兩個函式用 `-O2` 編譯（`g` 只有宣告）。

```c
/* saved.c */                          /* noret.c */
long g(long x);                         long g(long x);

long f(long x)                          long f(long x)
{                                       {
    long y = g(x);                          long y = g(x);
    return x + y;                           return y * 2;
}                                       }
```

```asm
# saved.c 的 f                          # noret.c 的 f
push   %rbx                             sub    $0x8,%rsp
mov    %rdi,%rbx                        call   g
call   g                                add    $0x8,%rsp
add    %rbx,%rax                        add    %rax,%rax
pop    %rbx                             ret
ret
```

#### 4-1. 為什麼 `saved.c` 的 `f` 要 `push %rbx`，而不是讓 `x` 留在 `%rdi`？

**第 1 次（YYYY-MM-DD）**
```

```

#### 4-2. `noret.c` 的 `f` 不需要保存任何值，為什麼還要 `sub $0x8,%rsp`？

**第 1 次（YYYY-MM-DD）**
```

```

---

### 5. 情境題：停在函式的第一條指令

你用 `break *add3` 停在 `add3` 的第一條指令（還沒執行 `push %rbp`），gdb 顯示 `add3 (a=0, b=0, c=0)`，可是 `main` 明明傳的是 1、2、3。

```c
long add3(long a, long b, long c)
{
    long s = a + b;
    return s + c;
}
```

```asm
add3:
  endbr64                 ← 停在這裡
  push   %rbp
  mov    %rsp,%rbp
  mov    %rdi,-0x18(%rbp)
  mov    %rsi,-0x20(%rbp)
  mov    %rdx,-0x28(%rbp)
  ...
```

#### 5-1. 為什麼 gdb 印錯？這時候要看真正的參數，該看哪裡？

**第 1 次（YYYY-MM-DD）**
```

```

#### 5-2. 同一個時間點，你想知道「是誰呼叫了 `add3`」，可是不能用 `bt`。根據本課，你會看哪個暫存器指的位址？看到的值代表什麼？

**第 1 次（YYYY-MM-DD）**
```

```
