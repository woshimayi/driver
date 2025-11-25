	.file	"test.c"
	.text
	.globl	infinite_loop
	.type	infinite_loop, @function
infinite_loop:
.LFB6:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$0, %eax
	call	infinite_loop
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	infinite_loop, .-infinite_loop
	.globl	g_jsontype
	.data
	.align 32
	.type	g_jsontype, @object
	.size	g_jsontype, 544
g_jsontype:
	.long	0
	.string	"JSON_NULL"
	.zero	54
	.long	1
	.string	"JSON_OBJ"
	.zero	55
	.long	2
	.string	"JSON_ARRAY"
	.zero	53
	.long	3
	.string	"JSON_STRING"
	.zero	52
	.long	4
	.string	"JSON_INT"
	.zero	55
	.long	5
	.string	"JSON_DOUBLE"
	.zero	52
	.long	6
	.string	"JSON_BOOL"
	.zero	54
	.long	7
	.string	"JSON_MAX"
	.zero	55
	.section	.rodata
.LC0:
	.string	"UploadLossPackets"
	.text
	.globl	__mf_key_UpLoadLossPackets
	.type	__mf_key_UpLoadLossPackets, @function
__mf_key_UpLoadLossPackets:
.LFB7:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	.LC0(%rip), %rax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	__mf_key_UpLoadLossPackets, .-__mf_key_UpLoadLossPackets
	.globl	igd_tab_list
	.section	.rodata
.LC1:
	.string	"IGD_TAB_ID_BEGIN"
.LC2:
	.string	"IGD_DEV_CAPABILITY_TAB"
.LC3:
	.string	"IGD_DEVINFO_TAB"
.LC4:
	.string	"IGD_GLOBAL_ATTR_TAB"
.LC5:
	.string	"IGD_DEV_STATUS_INFO_TAB"
.LC6:
	.string	"IGD_SYSCMD_CONFIG_TAB"
.LC7:
	.string	"IGD_PON_LINK_STATUS_TAB"
.LC8:
	.string	"IGD_PON_STATISTICS_TAB"
.LC9:
	.string	"IGD_LAN_IP_ADDR_TAB"
	.section	.data.rel.local,"aw"
	.align 32
	.type	igd_tab_list, @object
	.size	igd_tab_list, 144
igd_tab_list:
	.long	0
	.zero	4
	.quad	.LC1
	.long	1
	.zero	4
	.quad	.LC2
	.long	2
	.zero	4
	.quad	.LC3
	.long	3
	.zero	4
	.quad	.LC4
	.long	4
	.zero	4
	.quad	.LC5
	.long	5
	.zero	4
	.quad	.LC6
	.long	6
	.zero	4
	.quad	.LC7
	.long	7
	.zero	4
	.quad	.LC8
	.long	8
	.zero	4
	.quad	.LC9
	.section	.rodata
.LC10:
	.string	"tab = [%d|%s]\n"
	.text
	.globl	test_tabToname
	.type	test_tabToname, @function
test_tabToname:
.LFB8:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	movl	-4(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	8+igd_tab_list(%rip), %rax
	movq	(%rdx,%rax), %rdx
	movl	-4(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	igd_tab_list(%rip), %rax
	movl	(%rcx,%rax), %eax
	movl	%eax, %esi
	leaq	.LC10(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	test_tabToname, .-test_tabToname
	.section	.rodata
.LC11:
	.string	"Not found tab for name: %s\n"
	.text
	.globl	test_nameTotab
	.type	test_nameTotab, @function
test_nameTotab:
.LFB9:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L6
.L9:
	movl	-4(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	8+igd_tab_list(%rip), %rax
	movq	(%rdx,%rax), %rax
	movq	-24(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	strcmp@PLT
	testl	%eax, %eax
	jne	.L7
	movl	-4(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	8+igd_tab_list(%rip), %rax
	movq	(%rdx,%rax), %rdx
	movl	-4(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	igd_tab_list(%rip), %rax
	movl	(%rcx,%rax), %eax
	movl	%eax, %esi
	leaq	.LC10(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
	jmp	.L5
.L7:
	addl	$1, -4(%rbp)
.L6:
	movl	-4(%rbp), %eax
	cmpl	$8, %eax
	jbe	.L9
	movq	-24(%rbp), %rax
	movq	%rax, %rsi
	leaq	.LC11(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
.L5:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	test_nameTotab, .-test_nameTotab
	.section	.rodata
.LC12:
	.string	"IGD_XXXX_TAB"
	.text
	.globl	main
	.type	main, @function
main:
.LFB10:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	movq	%rsi, -16(%rbp)
	movl	$6, %edi
	call	test_tabToname
	leaq	.LC7(%rip), %rax
	movq	%rax, %rdi
	call	test_nameTotab
	leaq	.LC12(%rip), %rax
	movq	%rax, %rdi
	call	test_nameTotab
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE10:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
