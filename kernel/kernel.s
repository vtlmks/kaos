	.file	"kernel.cpp"
	.globl	loaderWelcome
	.section	.rodata
.LC0:
	.string	"Second stage loader started"
	.data
	.align 8
	.type	loaderWelcome, @object
	.size	loaderWelcome, 8
loaderWelcome:
	.quad	.LC0
	.globl	loaderInitializing
	.section	.rodata
	.align 8
.LC1:
	.string	" [  ] Initializing memory, setting up structures"
	.data
	.align 8
	.type	loaderInitializing, @object
	.size	loaderInitializing, 8
loaderInitializing:
	.quad	.LC1
	.globl	loaderMemoryprobe
	.section	.rodata
	.align 8
.LC2:
	.string	" [  ] Trying to find out how much memory this machine have available"
	.data
	.align 8
	.type	loaderMemoryprobe, @object
	.size	loaderMemoryprobe, 8
loaderMemoryprobe:
	.quad	.LC2
	.globl	loaderSwitchingvideomode
	.section	.rodata
	.align 8
.LC3:
	.string	" [  ] Switching to something better than 80x25"
	.data
	.align 8
	.type	loaderSwitchingvideomode, @object
	.size	loaderSwitchingvideomode, 8
loaderSwitchingvideomode:
	.quad	.LC3
	.globl	loaderMessage
	.section	.rodata
.LC4:
	.string	" [  ] Loading kernel"
	.data
	.align 8
	.type	loaderMessage, @object
	.size	loaderMessage, 8
loaderMessage:
	.quad	.LC4
	.globl	loaderOK
	.section	.rodata
.LC5:
	.string	" [OK]"
	.data
	.align 8
	.type	loaderOK, @object
	.size	loaderOK, 8
loaderOK:
	.quad	.LC5
	.text
	.globl	_Z10kernelmainv
	.type	_Z10kernelmainv, @function
_Z10kernelmainv:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
/APP
# 41 "kernel.cpp" 1
	mov	$deadbeef,%rax;jmp	.;cli;mov	%ax,0x10;mov	%ax,%ds;mov	%ax,%es;mov	%ax,%fs;mov	%ax,%gs;mov	%ax,%ss;mov	$80000,%esp;mov	%esp,%ebp;sti;
# 0 "" 2
/NO_APP
	call	_Z8apicInitv
	movq	loaderWelcome(%rip), %rax
	movq	%rax, -16(%rbp)
	movl	$0, -20(%rbp)
	movl	$0, -4(%rbp)
.L5:
	cmpl	$9, -4(%rbp)
	ja	.L2
	movl	-4(%rbp), %edx
	movq	-16(%rbp), %rax
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	testb	%al, %al
	jne	.L3
	movl	$1, -20(%rbp)
	jmp	.L4
.L3:
	movl	$0, -20(%rbp)
.L4:
	addl	$1, -4(%rbp)
	jmp	.L5
.L2:
	movl	-20(%rbp), %eax
	testl	%eax, %eax
	setne	%al
	testb	%al, %al
	je	.L7
	movq	-16(%rbp), %rax
	addq	$2, %rax
	movb	$0, (%rax)
.L7:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	_Z10kernelmainv, .-_Z10kernelmainv
	.globl	_Z8apicInitv
	.type	_Z8apicInitv, @function
_Z8apicInitv:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	_Z8apicInitv, .-_Z8apicInitv
	.ident	"GCC: (GNU) 5.4.0"
