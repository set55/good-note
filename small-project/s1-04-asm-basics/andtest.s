# andtest.s：andl 與 testl 做一樣的 AND，差在結果有沒有寫回 %edi
	.text
	.globl	and_val
and_val:                         # unsigned and_val(unsigned n)：andl 之後的 %edi
	andl	$1, %edi             # %edi = n & 1，寫回 %edi，設旗標
	movl	%edi, %eax           # 把做完後的 %edi 當回傳值
	ret

	.globl	test_val
test_val:                        # unsigned test_val(unsigned n)：testl 之後的 %edi
	testl	$1, %edi             # 算 n & 1，結果丟掉，只設旗標
	movl	%edi, %eax
	ret

	.globl	and_flags
and_flags:                       # unsigned long and_flags(unsigned n)：andl 之後的旗標
	andl	$1, %edi
	pushfq                       # 把 rflags 複製到 %rax（見 flags.s 的說明）
	popq	%rax
	ret

	.globl	test_flags
test_flags:                      # unsigned long test_flags(unsigned n)：testl 之後的旗標
	testl	$1, %edi
	pushfq
	popq	%rax
	ret

	.section	.note.GNU-stack,"",@progbits
