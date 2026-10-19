/* nosub.S：main 把變數 x 放在 -8(%rbp)，比較「有 sub」和「沒 sub」時，
   中間 call 一次 noop 之後 x 還在不在。
   編譯：gcc -g -o nosub nosub.S            （沒 sub）
         gcc -g -DSUB -o withsub nosub.S    （有 sub） */

        .section .rodata
fmt:    .string "x = %#lx\n"

        .text
noop:                           /* 什麼都不做的函式 */
        ret

        .globl main
main:
        push    %rbp
        mov     %rsp, %rbp
#ifdef SUB
        sub     $0x10, %rsp     /* 把 x 那格收進堆疊：%rsp 移到 x 的下面 */
#endif
        movq    $42, -8(%rbp)   /* x = 42，放在 %rbp - 8 */
        call    noop            /* 推返回位址到 %rsp - 8，再跳去 noop */
        mov     -8(%rbp), %rsi  /* printf 的第 2 個參數 = x */
        lea     fmt(%rip), %rdi /* printf 的第 1 個參數 = 字串位址 */
        mov     $0, %eax        /* 沒有 double 參數（第七節） */
        call    printf@PLT
        mov     $0, %eax        /* return 0 */
        leave
        ret

        .section .note.GNU-stack,"",@progbits
