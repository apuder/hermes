|*===========================================================================*
|*		interrupt handlers for 386 32-bit protected mode	     *
|*===========================================================================*

#include <minix/config.h>
#include <minix/const.h>
#include <minix/com.h>
#include "const.h"
#include "protect.h"
#include "sconst.h"

#if !INTEL_32BITS
#error /* this file uses 32-bit instructions */
#endif

/* Selected 386 tss offsets. */
#define TSS3_S_SP0	4

| exported functions

	.text
.define		_idle_task
.define		_mpx_1hook
.define		_mpx_2hook
.define		_restart
.define		save

.define		_divide_error		
.define		_single_step_exception
.define		_nmi
.define		_breakpoint_exception
.define		_overflow
.define		_bounds_check
.define		_inval_opcode
.define		_copr_not_available
.define		_double_fault
.define		_copr_seg_overrun
.define		_inval_tss
.define		_segment_not_present
.define		_stack_exception
.define		_general_protection
.define		_page_fault
.define		_copr_error

.define		_clock_int		
.define		_tty_int
.define		_wini_int
.define		_disk_int
.define		_lpr_int
.define		_s_call			
.define		_p_s_call

#if C_RS232_INT_HANDLERS
.define		_prs232_int
.define		_psecondary_int
#endif

| imported functions

.extern		_clock_handler
.extern		_dp8390_int
.extern		_exception
.extern		_interrupt
.extern		_keyboard
.extern		_pr_char
.extern		_sys_call
.extern		_unhold

#if C_RS232_INT_HANDLERS
.extern		_rs232_1handler
.extern		_rs232_2handler
#endif

| imported variables

	.bss
.extern		_break_vector
.extern		_db_enabled
.extern		_held_head
.extern		_k_reenter
.extern		k_stktop
.extern		_pc_at
.extern		_proc_ptr
.extern		_ps_mca
.extern		_sstep_vector
.extern		_tss

	.text
|*===========================================================================*
|*				exception handlers			     *
|*===========================================================================*
_divide_error:
	push	dword #DIVIDE_VECTOR
	j	exception

_single_step_exception:
	seg	ss
	cmpb	_db_enabled,#0
	jz	over_sstep
	seg	ss
	jmp	far [_sstep_vector]

over_sstep:
	push	dword #DEBUG_VECTOR
	j	exception

_nmi:
	push	dword #NMI_VECTOR
	j	exception

_breakpoint_exception:
	seg	ss
	cmpb	_db_enabled,#0
	jz	over_breakpoint
	seg	ss
	jmp	far [_break_vector]

over_breakpoint:
	push	dword #BREAKPOINT_VECTOR
	j	exception

_overflow:
	push	dword #OVERFLOW_VECTOR
	j	exception

_bounds_check:
	push	dword #BOUNDS_VECTOR
	j	exception

_inval_opcode:
	push	dword #INVAL_OP_VECTOR
	j	exception

_copr_not_available:
	push	dword #COPROC_NOT_VECTOR
	j	exception

_double_fault:
	push	dword #DOUBLE_FAULT_VECTOR
	j	errexception

_copr_seg_overrun:
	push	dword #COPROC_SEG_VECTOR
	j	exception

_inval_tss:
	push	dword #INVAL_TSS_VECTOR
	j	errexception

_segment_not_present:
	push	dword #SEG_NOT_VECTOR
	j	errexception

_stack_exception:
	push	dword #STACK_FAULT_VECTOR
	j	errexception

_general_protection:
	push	dword #PROTECTION_VECTOR
	j	errexception

_page_fault:
	push	dword #PAGE_FAULT_VECTOR
	j	errexception

_copr_error:
	push	dword #COPROC_ERR_VECTOR
	j	exception


|*===========================================================================*
|*				exception				     *
|*===========================================================================*
| This is called for all exceptions which don't push an error code.

exception:
	seg	ss
	pop	dword ex_number
	call	save
	j	exception1


|*===========================================================================*
|*				errexception				     *
|*===========================================================================*
| This is called for all exceptions which push an error code.

errexception:
	seg	ss
	pop	dword ex_number
	seg	ss
	pop	dword trap_errno
	call	save
exception1:			| Common for all exceptions.
	push	dword ex_number
	call	_exception
	add	esp,#4
	cli
	ret


|*===========================================================================*
|*				interrupt handlers			     *
|*===========================================================================*


|*===========================================================================*
|*				tty_int					     *
|*===========================================================================*
_tty_int:			| Interrupt routine for terminal input.
	call	save		| save the machine state
	in	INT_CTLMASK	| mask out further keyboard interrupts
	orb	al,#KEYBOARD_MASK
	out	INT_CTLMASK
	sti			| allow unmasked, non-keyboard, interrupts
	call	_keyboard	| process a keyboard interrupt
	cli
	in	INT_CTLMASK	| unmask keyboard interrupt
	andb	al,#notop(KEYBOARD_MASK)
	out	INT_CTLMASK
	ret			| return to appropiate restart built by save()


#if C_RS232_INT_HANDLERS
|*===========================================================================*
|*				prs232_int				     *
|*===========================================================================*
_prs232_int:			| Interrupt routine for rs232 I/O.
	call	save
	in	INT_CTLMASK
	orb	al,#RS232_MASK
	out	INT_CTLMASK

| Don't enable interrupts, the handlers are not designed for it.
| The mask was set as usual so the handler can reenable interrupts as desired.

	call	_rs232_1handler	| process a serial line 1 interrupt
	in	INT_CTLMASK
	andb	al,#notop(RS232_MASK)
	out	INT_CTLMASK
	ret


|*===========================================================================*
|*				psecondary_int				     *
|*===========================================================================*
_psecondary_int:		| Interrupt routine for rs232 port 2
	call	save
	in	INT_CTLMASK
	orb	al,#SECONDARY_MASK
	out	INT_CTLMASK
	call	_rs232_2handler	| process a serial line 2 interrupt
	in	INT_CTLMASK
	andb	al,#notop(SECONDARY_MASK)
	out	INT_CTLMASK
	ret
#endif /* C_RS232_INT_HANDLERS */


|*===========================================================================*
|*				lpr_int					     *
|*===========================================================================*
_lpr_int:			| Interrupt routine for printer output.
	call	save
	in	INT_CTLMASK
	orb	al,#PRINTER_MASK
	out	INT_CTLMASK
	sti
	call	_pr_char	| process a line printer interrupt
	cli
	in	INT_CTLMASK
	andb	al,#notop(PRINTER_MASK) & 0xFF
	out	INT_CTLMASK
	ret


|*===========================================================================*
|*				disk_int				     *
|*===========================================================================*
_disk_int:			| Interrupt routine for the floppy disk.
	call	save
	in	INT_CTLMASK
	orb	al,#FLOPPY_MASK
	out	INT_CTLMASK
	sti
	mov	eax,#FLOPPY
	push	eax
	call	_interrupt	| interrupt(FLOPPY)
	add	esp,#4
	cli
	ret


|*===========================================================================*
|*				wini_int				     *
|*===========================================================================*
_wini_int:			| Interrupt routine for the winchester disk.
	call	save
	cmp	dword _pc_at,#0	| check for 2nd int controller on AT
	jnz	at_wini_int

xt_wini_int:
	in	INT_CTLMASK
	orb	al,#XT_WINI_MASK
	out	INT_CTLMASK
	sti
	mov	eax,#WINCHESTER
	push	eax
	call	_interrupt	| interrupt(WINCHESTER)
	add	esp,#4
	cli
	ret

at_wini_int:
	in	INT2_MASK
	orb	al,#AT_WINI_MASK
	out	INT2_MASK
	sti
	movb	al,#ENABLE	| reenable slave 8259
	out	INT2_CTL	| the master one was done in save() as usual
	mov	eax,#WINCHESTER
	push	eax
	call	_interrupt	| interrupt(WINCHESTER)
	add	esp,#4
	cli
	ret


|*===========================================================================*
|*				clock_int				     *
|*===========================================================================*
_clock_int:			| Interrupt routine for the clock.
	call	save
	cmp	_ps_mca, #0	| if not a PS/2, don't twiddle bits
	je	skip_mca_clock
	in	CLOCK_ACK_PORT	| PS/2 clock needs to be told to stop 
	orb	al,#CLOCK_ACK_BIT	| interrupting CPU, or we'll hang
	out	CLOCK_ACK_PORT
skip_mca_clock:
	in	INT_CTLMASK
	orb	al,#CLOCK_MASK
	out	INT_CTLMASK
	sti
	call	_clock_handler	| process a clock interrupt
				| This calls interrupt() only if necessary.
				| Very rarely.
	cli
	in	INT_CTLMASK
	andb	al,#notop(CLOCK_MASK)
	out	INT_CTLMASK
	ret


|*===========================================================================*
|*				eth_int					     *
|*===========================================================================*
_eth_int:			| Interrupt routine for ethernet input
	call	save
	in	INT_CTLMASK
	orb	al,#ETHER_MASK
	out	INT_CTLMASK
	sti
	call	_dp8390_int	| call the handler
	cli
	ret


|*===========================================================================*
|*				save					     *
|*===========================================================================*
| Save for protected mode.
| This is much simpler than for 8086 mode, because the stack already points
| into the process table, or has already been switched to the kernel stack.

save:
	cld			| set direction flag to a known value
	pushad			| save "general" registers
	push	ds		| save ds
	push	es		| save es
	push	fs		| save fs
	push	gs		| save gs
	mov	dx,ss		| ss is kernel data segment
	mov	ds,dx		| load rest of kernel segments
	mov	es,dx		| kernel doesn't use fs, gs
	movb	al,#ENABLE	| reenable int controller
	out	INT_CTL
	mov	ebp,esp		| prepare to return
	incb	_k_reenter	| from -1 if not reentering
	jnz	set_restart1	| stack is already kernel's
	mov	esp,#k_stktop
#if SPLIMITS
	mov	dword splimit,#k_stack+8
#endif
	push	dword #_restart	| build return address for int handler
	jmp	ERETADR-P_STACKBASE(ebp)

set_restart1:
	push	#restart1
	jmp	ERETADR-P_STACKBASE(ebp)


|*===========================================================================*
|*				_s_call					     *
|*===========================================================================*
_s_call:
_p_s_call:
	cld			| set direction flag to a known value
	sub	esp,#6*4	| skip RETADR, eax, ecx, edx, ebx, est
	push	ebp		| stack already points into proc table
	push	esi
	push	edi
	push	ds
	push	es
	push	fs
	push	gs
	mov	dx,ss
	mov	ds,dx
	mov	es,dx
	incb	_k_reenter
	mov	esi,esp		| assumes P_STACKBASE == 0
	mov	esp,#k_stktop
#if SPLIMITS
	mov	dword splimit,#k_stack+8
#endif
				| end of inline save
	sti			| allow SWITCHER to be interrupted
				| now set up parameters for sys_call()
	push	ebx		| pointer to user message
	push	eax		| src/dest
	push	ecx		| SEND/RECEIVE/BOTH
	call	_sys_call	| sys_call(function, src_dest, m_ptr)
				| caller is now explicitly in proc_ptr
	mov	EAXREG(esi),eax	| sys_call MUST PRESERVE si
	cli

| Fall into code to restart proc/task running.

_restart:

| Flush any held-up interrupts.
| This reenables interrupts, so the current interrupt handler may reenter.
| This doesn't matter, because the current handler is about to exit and no
| other handlers can reenter since flushing is only done when k_reenter == 0.

	cmp	dword _held_head,#0	| do fast test to usually avoid call
	jz	over_call_unhold
	call	_unhold		| this is rare so overhead acceptable
over_call_unhold:
	mov	esp,_proc_ptr	| will assume P_STACKBASE == 0
#if SPLIMITS
	mov	eax,P_SPLIMIT(esp)	| splimit = p_splimit
	mov	splimit,eax
#endif
	lldt	P_LDT_SEL(esp)		| enable task's segment descriptors
	lea	eax,P3_STACKTOP(esp)	| arrange for next interrupt
	mov	_tss+TSS3_S_SP0,eax	| to save state in process table
restart1:
	decb	_k_reenter
	pop	gs
	pop	fs
	pop	es
	pop	ds
	popad
	add	esp,#4		| skip RETADR
	iretd			| continue process


|*===========================================================================*
|*				idle					     *
|*===========================================================================*
_idle_task:			| executed when there is no work 
	j	_idle_task	| a "hlt" before this fails in protected mode


|*===========================================================================*
|*				mpx_1hook				     *
|*				mpx_2hook				     *
|*===========================================================================*
| PUBLIC void mpx_1hook();
| Initialize mpx from real mode for protected mode (no real mode).
| Nothing to do.

| PUBLIC void mpx_2hook();
| Initialize mpx from protected mode for protected mode (no real mode).
| Nothing to do.

_mpx_1hook:
_mpx_2hook:
	ret				| cheat: opcode same for 16/32-bits


|*===========================================================================*
|*				data					     *
|*===========================================================================*
	.bss
ex_number:
	.space	4
splimit:
	.space	4		| stack limit (kernel only)
trap_errno:
	.space	4
