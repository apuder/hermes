.text

	.align 4
.globl _start

_start:
	xorw %ax,%ax
	movw %ax,%es
	movw %ax,%fs
	movw $0x10,%ax
	movw %ax,%ds
	movw %ax,%ss
	movw $0x18,%ax
	movw %ax,%gs
/ Place stack pointer just below 3MB
/	movl $1024*1024,%esp
	movl $1024*1024-1024,%esp
	call kernel_main
L1:
	jmp L1
