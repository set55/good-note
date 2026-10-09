set disassemble-next-line off
break *main+27
run
echo \n=== 1. call 之前\n
x/i $pc
info registers rdi rsi rdx rsp rbp
x/2gx $rsp
stepi
echo \n=== 2. call 之後（進到 add3 第一條）\n
x/i $pc
info registers rsp rip
x/gx $rsp
stepi
stepi
echo \n=== 3. push %rbp 之後\n
x/i $pc
info registers rsp rbp
x/2gx $rsp
stepi
echo \n=== 4. mov %rsp,%rbp 之後\n
info registers rsp rbp
break *add3+47
continue
echo \n=== 5. 停在 ret（pop %rbp 已執行）\n
x/i $pc
info registers rax rsp rbp
x/gx $rsp
stepi
echo \n=== 6. ret 之後，回到 main\n
x/i $pc
info registers rsp rip
continue
