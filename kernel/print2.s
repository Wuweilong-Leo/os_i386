	.file	"print.c"
	.globl	video_base_addr
	.section	.rodata
	.align 8
	.type	video_base_addr, @object
	.size	video_base_addr, 8
video_base_addr:
	.quad	3221979136
	.globl	roll_video_src_addr
	.data
	.align 8
	.type	roll_video_src_addr, @object
	.size	roll_video_src_addr, 8
roll_video_src_addr:
	.quad	3221979296
	.globl	roll_video_dst_addr
	.align 8
	.type	roll_video_dst_addr, @object
	.size	roll_video_dst_addr, 8
roll_video_dst_addr:
	.quad	3221979136
	.text
	.type	set_cursor, @function
set_cursor:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, %eax
	movw	%ax, -20(%rbp)
	movzwl	-20(%rbp), %eax
	shrw	$8, %ax
	movb	%al, -1(%rbp)
	movzwl	-20(%rbp), %eax
	movb	%al, -2(%rbp)
	movw	$980, -4(%rbp)
	movw	$981, -6(%rbp)
	movb	$14, -7(%rbp)
	movb	$15, -8(%rbp)
	movzwl	-4(%rbp), %edx
	movzbl	-7(%rbp), %eax
#APP
# 20 "print.c" 1
	outb %al, %dx
# 0 "" 2
#NO_APP
	movzwl	-6(%rbp), %edx
	movzbl	-1(%rbp), %eax
#APP
# 21 "print.c" 1
	outb %al, %dx
# 0 "" 2
#NO_APP
	movzwl	-4(%rbp), %edx
	movzbl	-8(%rbp), %eax
#APP
# 22 "print.c" 1
	outb %al, %dx
# 0 "" 2
#NO_APP
	movzwl	-6(%rbp), %edx
	movzbl	-2(%rbp), %eax
#APP
# 23 "print.c" 1
	outb %al, %dx
# 0 "" 2
#NO_APP
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	set_cursor, .-set_cursor
	.type	get_cursor, @function
get_cursor:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movb	$0, -1(%rbp)
	movw	$980, -4(%rbp)
	movw	$981, -6(%rbp)
	movb	$14, -7(%rbp)
	movb	$15, -8(%rbp)
	movzwl	-4(%rbp), %edx
	movzbl	-7(%rbp), %eax
#APP
# 33 "print.c" 1
	outb %al, %dx
# 0 "" 2
#NO_APP
	movzwl	-6(%rbp), %eax
	movl	%eax, %edx
#APP
# 34 "print.c" 1
	inb %dx, %al
# 0 "" 2
#NO_APP
	movb	%al, -1(%rbp)
	movzwl	-4(%rbp), %edx
	movzbl	-8(%rbp), %eax
#APP
# 37 "print.c" 1
	outb %al, %dx
# 0 "" 2
#NO_APP
	movzwl	-6(%rbp), %eax
	movl	%eax, %edx
#APP
# 38 "print.c" 1
	inb %dx, %al
# 0 "" 2
#NO_APP
	movb	%al, -9(%rbp)
	movzbl	-1(%rbp), %eax
	sall	$8, %eax
	movl	%eax, %edx
	movzbl	-9(%rbp), %eax
	subw	$256, %ax
	andl	%edx, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	get_cursor, .-get_cursor
	.type	memcpy, @function
memcpy:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -40(%rbp)
	movq	%rsi, -48(%rbp)
	movl	%edx, -52(%rbp)
	movq	-40(%rbp), %rax
	movq	%rax, -16(%rbp)
	movq	-48(%rbp), %rax
	movq	%rax, -24(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L5
.L6:
	movl	-4(%rbp), %edx
	movq	-16(%rbp), %rax
	addq	%rax, %rdx
	movl	-4(%rbp), %ecx
	movq	-24(%rbp), %rax
	addq	%rcx, %rax
	movzbl	(%rax), %eax
	movb	%al, (%rdx)
	addl	$1, -4(%rbp)
.L5:
	movl	-4(%rbp), %eax
	cmpl	-52(%rbp), %eax
	jb	.L6
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	memcpy, .-memcpy
	.type	clean_last_line, @function
clean_last_line:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$3840, -4(%rbp)
	movl	$0, -8(%rbp)
	jmp	.L8
.L9:
	movl	$3221979136, %ecx
	movl	-4(%rbp), %eax
	leal	1(%rax), %edx
	movl	%edx, -4(%rbp)
	movl	%eax, %eax
	addq	%rcx, %rax
	movb	$32, (%rax)
	movl	$3221979136, %ecx
	movl	-4(%rbp), %eax
	leal	1(%rax), %edx
	movl	%edx, -4(%rbp)
	movl	%eax, %eax
	addq	%rcx, %rax
	movb	$7, (%rax)
	addl	$1, -8(%rbp)
.L8:
	cmpl	$79, -8(%rbp)
	jbe	.L9
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	clean_last_line, .-clean_last_line
	.globl	put_char
	.type	put_char, @function
put_char:
.LFB4:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$24, %rsp
	movl	%edi, %eax
	movb	%al, -20(%rbp)
	movl	$0, %eax
	call	get_cursor
	movw	%ax, -4(%rbp)
	cmpb	$13, -20(%rbp)
	je	.L11
	cmpb	$10, -20(%rbp)
	jne	.L12
.L11:
	movzwl	-4(%rbp), %ecx
	movzwl	%cx, %eax
	imull	$52429, %eax, %eax
	shrl	$16, %eax
	movl	%eax, %edx
	shrw	$6, %dx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	sall	$4, %eax
	subl	%eax, %ecx
	movl	%ecx, %edx
	movzwl	-4(%rbp), %eax
	subl	%edx, %eax
	addl	$80, %eax
	movw	%ax, -2(%rbp)
	cmpw	$1999, -2(%rbp)
	jbe	.L13
	movq	roll_video_src_addr(%rip), %rcx
	movq	roll_video_dst_addr(%rip), %rax
	movl	$3840, %edx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	memcpy
	movl	$0, %eax
	call	clean_last_line
	movw	$1920, -2(%rbp)
	jmp	.L14
.L13:
	jmp	.L14
.L12:
	cmpb	$8, -20(%rbp)
	jne	.L15
	movzwl	-4(%rbp), %eax
	subl	$1, %eax
	addl	%eax, %eax
	movl	%eax, -8(%rbp)
	movl	$3221979136, %edx
	movl	-8(%rbp), %eax
	addq	%rdx, %rax
	movb	$32, (%rax)
	movl	$3221979136, %edx
	movl	-8(%rbp), %eax
	addl	$1, %eax
	movl	%eax, %eax
	addq	%rdx, %rax
	movb	$7, (%rax)
	movzwl	-4(%rbp), %eax
	movw	%ax, -2(%rbp)
	jmp	.L14
.L15:
	movzwl	-4(%rbp), %eax
	addl	%eax, %eax
	movl	%eax, -12(%rbp)
	movl	$3221979136, %edx
	movl	-12(%rbp), %eax
	addq	%rax, %rdx
	movzbl	-20(%rbp), %eax
	movb	%al, (%rdx)
	movl	$3221979136, %edx
	movl	-12(%rbp), %eax
	addl	$1, %eax
	movl	%eax, %eax
	addq	%rdx, %rax
	movb	$7, (%rax)
	movzwl	-4(%rbp), %eax
	addl	$1, %eax
	movw	%ax, -2(%rbp)
	cmpw	$1999, -2(%rbp)
	jbe	.L14
	movq	roll_video_src_addr(%rip), %rcx
	movq	roll_video_dst_addr(%rip), %rax
	movl	$3840, %edx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	memcpy
	movl	$0, %eax
	call	clean_last_line
	movw	$1920, -2(%rbp)
.L14:
	movzwl	-2(%rbp), %eax
	movl	%eax, %edi
	call	set_cursor
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	put_char, .-put_char
	.globl	put_str
	.type	put_str, @function
put_str:
.LFB5:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$24, %rsp
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	%rax, -8(%rbp)
	jmp	.L17
.L18:
	movq	-8(%rbp), %rax
	movzbl	(%rax), %eax
	movzbl	%al, %eax
	movl	%eax, %edi
	call	put_char
	addq	$1, -8(%rbp)
.L17:
	movq	-8(%rbp), %rax
	movzbl	(%rax), %eax
	testb	%al, %al
	jne	.L18
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	put_str, .-put_str
	.ident	"GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-44)"
	.section	.note.GNU-stack,"",@progbits
