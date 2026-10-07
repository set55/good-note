# flags.s：做一次 8 位元的 cmp 或 add，把做完後的旗標整包回傳
# 參數：%dil = D（第 1 個參數的最低 1 byte），%sil = S（第 2 個參數的最低 1 byte）
	.text
	.globl	flags_cmp
flags_cmp:                       # unsigned long flags_cmp(unsigned char d, unsigned char s)
	cmpb	%sil, %dil           # 算 D − S（8 位元），結果丟掉，只設旗標
	pushfq                       # 把整個 rflags 複製一份放到堆疊上
	popq	%rax                 # 再從堆疊取到 %rax ＝ 回傳值
	ret

	.globl	flags_add
flags_add:                       # unsigned long flags_add(unsigned char d, unsigned char s)
	addb	%sil, %dil           # 算 D + S（8 位元），結果寫回 %dil，同時設旗標
	pushfq
	popq	%rax
	ret

	.section	.note.GNU-stack,"",@progbits
