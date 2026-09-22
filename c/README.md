# C 筆記索引

這個目錄收錄 C 語言、編譯與連結工具鏈（gcc、binutils）、ELF 與程式執行前後的機制——
也就是「一支 C 程式怎麼變成行程」這一層。系統呼叫本身（行程、記憶體、I/O、網路）的筆記仍放在 `linux/`。

## 筆記列表

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [hello.c怎麼變成能跑的程式-編譯的四個階段.md](./hello.c怎麼變成能跑的程式-編譯的四個階段.md) | `gcc` 是總指揮，依序呼叫 cc1（前處理＋編譯）、as、collect2／ld；`.i` 仍是 C、`.s` 是組合語言、`.o` 是機器碼但外部呼叫留下全 0 的洞與重定位項目；連結後 `puts` 仍是 `U`，因為動態連結要到執行時由 `ld-linux-x86-64.so.2` 填上；gcc 在 `-O0` 也會把 `printf("%s\n")` 換成 `puts`；`strace` 顯示印字最後只是一次 `write(1, ...)`，34 次系統呼叫大多是動態載入器 | `gcc -E/-S/-c/-v/-save-temps` `-fno-builtin` `-static` `cc1` `as` `collect2` `ELF` `relocatable` `R_X86_64_PLT32` `nm` `objdump -dr` `ldd` `file` `strace -c` `System V ABI` `%rdi` `%eax` `PLT` `execve` `write` `exit_group` |

---
> 新增筆記時，記得把它加進上面的「筆記列表」表格。
