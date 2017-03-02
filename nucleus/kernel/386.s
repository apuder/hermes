	.text
	.align 4
	.globl poke_mem_b

poke_mem_b:
	pushl %ebp
	movl %esp,%ebp
	movb 12(%ebp),%al
	movl 8(%ebp),%edx

/ The following .byte 0x65 is a %GS segment override. Unfortunately
/ GAS doesn't generate it!
	.byte 0x65
	movb %al,(%edx)
	leave
	ret


	
	.align 4
	.globl peek_mem_b

peek_mem_b:
	pushl %ebp
	movl %esp,%ebp
	movl 8(%ebp),%edx
	.byte 0x65
	movzbl (%edx),%eax
	leave
	ret


	.align 4
	.globl poke_mem_w
	
poke_mem_w:
	pushl %ebp
	movl %esp,%ebp
	movw 12(%ebp),%dx
	movl 8(%ebp),%eax
	.byte 0x65
	movw %dx,(%eax)
	leave
	ret



	.align 4
	.globl peek_mem_w
	
peek_mem_w:
	pushl %ebp
	movl %esp,%ebp
	movl 8(%ebp),%edx
	.byte 0x65
	movzwl (%edx),%eax
	leave
	ret


	
	.align 4
	.globl poke_mem_l
	
poke_mem_l:
	pushl %ebp
	movl %esp,%ebp
	movl 12(%ebp),%edx
	movl 8(%ebp),%eax
	.byte 0x65
	movl %edx,(%eax)
	leave
	ret


	
	.align 4
	.globl peek_mem_l
	
peek_mem_l:
	pushl %ebp
	movl %esp,%ebp
	movl 8(%ebp),%edx
	.byte 0x65
	movl (%edx),%eax
	leave
	ret
