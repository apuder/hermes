.text

.globl _start

_start:
	call c0_boot
L1:	jmp L1

