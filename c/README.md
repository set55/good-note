# C 筆記索引

這個目錄收錄 C 語言、編譯與連結工具鏈（gcc、binutils）、ELF 與程式執行前後的機制——
也就是「一支 C 程式怎麼變成行程」這一層。系統呼叫本身（行程、記憶體、I/O、網路）的筆記仍放在 `linux/`。

## 筆記列表

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [hello.c怎麼變成能跑的程式-編譯的四個階段.md](./hello.c怎麼變成能跑的程式-編譯的四個階段.md) | `gcc` 是總指揮，依序呼叫 cc1（前處理＋編譯）、as、collect2／ld；`.i` 仍是 C、`.s` 是組合語言、`.o` 是機器碼但外部呼叫留下全 0 的洞與重定位項目；連結後 `puts` 仍是 `U`，因為動態連結要到執行時由 `ld-linux-x86-64.so.2` 填上；gcc 在 `-O0` 也會把 `printf("%s\n")` 換成 `puts`；`strace` 顯示印字最後只是一次 `write(1, ...)`，34 次系統呼叫大多是動態載入器 | `gcc -E/-S/-c/-v/-save-temps` `-fno-builtin` `-static` `cc1` `as` `collect2` `ELF` `relocatable` `R_X86_64_PLT32` `nm` `objdump -dr` `ldd` `file` `strace -c` `System V ABI` `%rdi` `%eax` `PLT` `execve` `write` `exit_group` |
| [gcc最佳化等級-O0到O3差在哪.md](./gcc最佳化等級-O0到O3差在哪.md) | `-O0` 是大寫 O 加數字 0＝不最佳化，且是預設值；全部等級（`-O0/1/2/3/s/z/g/fast`）的意義、「最後一個 `-O` 為準」、`-Q --help=optimizers` 自查；實測 `-O0` 每圈讀寫記憶體、`-O2` 把 `sum_to(100)` 內嵌並在編譯時算成 5050；最佳化不等於變小（`-O3` 234 bytes 最大）；除錯用 `-O0`／`-Og`；kernel 預設 `-O2`、不能用 `-O0`，所以要習慣 `<optimized out>` | `-O0` `-O1` `-O2` `-O3` `-Os` `-Oz` `-Og` `-Ofast` `-Q --help=optimizers` `-g` `inline` `constant folding` `size -A` `gdb` `<optimized out>` |
| [gdb完整命令參考.md](./gdb完整命令參考.md) | 本機 gdb 15.1 的完整參考：`gdb --help` 全部啟動選項（依其 7 組）、命令共通文法（縮寫與別名、Enter 重複、LOCATION 三種寫法、C 表達式、`$N` 值歷史、`/FMT` 格式與大小字母）、依 `help all` 的 13 類**全部 173 個頂層命令**與別名、`info` 全部 65 個子命令；`set`／`show`／`maintenance`（456／463／197 個）明講不逐項列的原因與查法；`next`/`step`/`finish`/`until`/`advance`、`break`/`tbreak`/`watch`/`catch`、`delete`/`clear`、`info` vs `show` 的差別與錯誤訊息對照 | `gdb` `-g` `break` `tbreak` `watch` `catch` `run` `start` `starti` `next` `step` `finish` `until` `print/FMT` `x/FMT` `display` `ptype` `backtrace` `frame` `info locals` `info registers` `disassemble` `help all` `apropos` `-batch -ex` |

---
> 新增筆記時，記得把它加進上面的「筆記列表」表格。
