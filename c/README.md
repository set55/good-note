# C 筆記索引

這個目錄收錄 C 語言、編譯與連結工具鏈（gcc、binutils）、ELF 與程式執行前後的機制——
也就是「一支 C 程式怎麼變成行程」這一層。系統呼叫本身（行程、記憶體、I/O、網路）的筆記仍放在 `linux/`。

## 筆記列表

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [hello.c怎麼變成能跑的程式-編譯的四個階段.md](./hello.c怎麼變成能跑的程式-編譯的四個階段.md) | `gcc` 是總指揮，依序呼叫 cc1（前處理＋編譯）、as、collect2／ld；`.i` 仍是 C、`.s` 是組合語言、`.o` 是機器碼但外部呼叫留下全 0 的洞與重定位項目；連結後 `puts` 仍是 `U`，因為動態連結要到執行時由 `ld-linux-x86-64.so.2` 填上；gcc 在 `-O0` 也會把 `printf("%s\n")` 換成 `puts`；`strace` 顯示印字最後只是一次 `write(1, ...)`，34 次系統呼叫大多是動態載入器 | `gcc -E/-S/-c/-v/-save-temps` `-fno-builtin` `-static` `cc1` `as` `collect2` `ELF` `relocatable` `R_X86_64_PLT32` `nm` `objdump -dr` `ldd` `file` `strace -c` `System V ABI` `%rdi` `%eax` `PLT` `execve` `write` `exit_group` |
| [gcc最佳化等級-O0到O3差在哪.md](./gcc最佳化等級-O0到O3差在哪.md) | `-O0` 是大寫 O 加數字 0＝不最佳化，且是預設值；全部等級（`-O0/1/2/3/s/z/g/fast`）的意義、「最後一個 `-O` 為準」、`-Q --help=optimizers` 自查；實測 `-O0` 每圈讀寫記憶體、`-O2` 把 `sum_to(100)` 內嵌並在編譯時算成 5050；最佳化不等於變小（`-O3` 234 bytes 最大）；除錯用 `-O0`／`-Og`；kernel 預設 `-O2`、不能用 `-O0`，所以要習慣 `<optimized out>` | `-O0` `-O1` `-O2` `-O3` `-Os` `-Oz` `-Og` `-Ofast` `-Q --help=optimizers` `-g` `inline` `constant folding` `size -A` `gdb` `<optimized out>` |
| [gdb完整命令參考.md](./gdb完整命令參考.md) | 本機 gdb 15.1 的完整參考：`gdb --help` 全部啟動選項（依其 7 組）、命令共通文法（縮寫與別名、Enter 重複、LOCATION 三種寫法、C 表達式、`$N` 值歷史、`/FMT` 格式與大小字母）、依 `help all` 的 13 類**全部 173 個頂層命令**與別名、`info` 全部 65 個子命令；`set`／`show`／`maintenance`（456／463／197 個）明講不逐項列的原因與查法；`next`/`step`/`finish`/`until`/`advance`、`break`/`tbreak`/`watch`/`catch`、`delete`/`clear`、`info` vs `show` 的差別與錯誤訊息對照 | `gdb` `-g` `break` `tbreak` `watch` `catch` `run` `start` `starti` `next` `step` `finish` `until` `print/FMT` `x/FMT` `display` `ptype` `backtrace` `frame` `info locals` `info registers` `disassemble` `help all` `apropos` `-batch -ex` |
| [ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md](./ELF的interpreter是什麼-動態載入器與井字號驚嘆號.md) | interpreter 是執行檔自己指定的「請先幫我跑這個程式」：程式標頭的 `INTERP`（`.interp` 區段）寫著 `/lib64/ld-linux-x86-64.so.2`，核心 `execve` 時先跑它，由它載入 libc、檢查符號版本、填好位址，再交給程式；跟腳本 `#!` 同一機制（`#!/bin/cat` 實驗：核心把腳本路徑交給直譯器）；`ldd` 其實是呼叫 `ld.so --list` 的 bash 腳本、`ld.so` 自己是 static-pie；interpreter 不存在時「檔案明明在卻 not found」（glibc 程式搬到 Alpine／OpenWrt）；含 `ld.so` 全部 14 個選項 | `INTERP` `.interp` `program interpreter` `ld.so` `ld-linux-x86-64.so.2` `dynamic linker` `execve` `#!` `shebang` `ldd` `--list` `static-pie` `-Wl,--dynamic-linker` `not found` `musl` |
| [readelf完整選項參考.md](./readelf完整選項參考.md) | `readelf` 只讀 ELF 結構、不反組譯（反組譯用 `objdump`）；`--help` 全部 52 行選項依用途分 8 組列出（綜合、標頭與區段、符號、動態連結／版本／重定位、傾印、DWARF、CTF／SFrame、輸出）；課程常用 `-h`／`-l`／`-S`／`-s`／`-d`／`-V`／`-r`／`-p`／`-W` 與對應用途；錯誤訊息實測對照 | `readelf` `-h` `-l` `-S` `-s` `-d` `-V` `-r` `-p` `-W` `--dyn-syms` `-w` `DWARF` `program header` `section header` `NEEDED` `objdump` |

---
> 新增筆記時，記得把它加進上面的「筆記列表」表格。
