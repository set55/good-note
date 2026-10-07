# cmptest.s：32 位元的 cmpl 與 testl，把做完後的旗標整包回傳（寫法同 flags.s）
	.text
	.globl	flags_cmpl
flags_cmpl:                      # unsigned long flags_cmpl(int i, int n)
	cmpl	%esi, %edi           # 算 i − n（%edi 是 i、%esi 是 n），結果丟掉，只設旗標
	pushfq                       # 把 rflags 複製到 %rax（見 flags.s 的說明）
	popq	%rax
	ret

	.globl	flags_testl
flags_testl:                     # unsigned long flags_testl(int n)
	testl	%edi, %edi           # 算 n & n，結果丟掉，只設旗標
	pushfq
	popq	%rax
	ret

	.section	.note.GNU-stack,"",@progbits
