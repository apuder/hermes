/* This is file _MEMCPY.S */
/*
** Copyright (C) 1991 DJ Delorie, 24 Kirsten Ave, Rochester NH 03867-2954
**
** This file is distributed under the terms listed in the document
** "copying.dj", available from DJ Delorie at the address above.
** A copy of "copying.dj" should accompany this file: if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY: without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

	.globl	__memcpy
__memcpy:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	pushl	%edi
	movl	8(%ebp),%edi
	movl	12(%ebp),%esi
	movl	16(%ebp),%ecx
	jecxz	L2
	cld
	cmp	%esi,%edi
	jb	L1
L3:
	std
	addl	%ecx,%esi
	addl	%ecx,%edi
	decl	%esi
	decl	%edi
L1:
	lodsb
	stosb
	loop	L1
L2:
	cld
	popl	%edi
	popl	%esi
	leave
	ret

