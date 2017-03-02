| This file contains a number of assembly code utility routines needed by the
| kernel.  They are:

.define		_build_sig
.define		_check_mem
.define		_cim_at_wini
.define		_cim_floppy
.define		_cim_printer
.define		_cim_xt_wini
.define		_cp_mess
.define		_get_byte
.define		_in_byte
.define		_klib_1hook
.define		_klib_2hook
.define		_lock
.define		_out_byte
.define		_phys_copy
.define		_port_read
.define		_port_write
.define		_reset
.define		_scr_down
.define		_scr_up
.define		_sim_printer
.define		_tasim_printer
.define		_test_and_set
.define		_unlock
.define		_vid_copy
.define		_wait_retrace

| The routines only guarantee to preserve the registers the 'C' compiler
| expects to be preserved (ebx, esi, edi, ebp, esp, segment registers, and
| direction bit in the flags).

#include <minix/config.h>
#include <minix/const.h>
#include "const.h"
#include "sconst.h"
#include "protect.h"

#if !INTEL_32BITS
#error /* this file uses 32-bit instructions */
#endif

/* Paging constants. */
#define CR0_PG			0x80000000
#define EXTENDED_MEM_BASE	0x100000
#define LOG2_MEM_UNIT		10
#define LOG2_PAGE_DATA_SIZE	2
#define LOG2_PAGE_SIZE		12
#define MEM_UNIT		1024
#define NR_PAGE_DIRS		1
#define NR_PAGE_TABLES		4
#define PAGE_DATA_SIZE		4
#define PAGE_SIZE		0x1000
#define PAGE_DIR_SIZE		0x1000
#define PAGE_TABLE_SIZE		0x1000

| imported variables

	.bss
.extern		_blank_color
.extern		_code_base
.extern		_ext_memsize
.extern		_gdt
.extern		_low_memsize
.extern		_sizes
.extern		_snow
.extern		_vid_mask
.extern		_vid_port

	.text
|*===========================================================================*
|*				build_sig				     *
|*===========================================================================*
| PUBLIC void build_sig(char *sig_stuff, struct proc *rp, int sig)
| Build a structure that is pushed onto the stack for signals.  It contains
| pc, psw, etc., and is machine dependent. The format is the same as generated
| by hardware interrupts, except that after the "interrupt", the signal number
| is also pushed.  The signal processing routine within the user space first
| pops the signal number, to see which function to call.  Then it calls the
| function.  Finally, when the function returns to the low-level signal
| handling routine, control is passed back to where it was prior to the signal
| by executing a return-from-interrupt instruction, hence the need for using
| the hardware generated interrupt format on the stack.

_build_sig:
	mov	ecx,4(esp)		| sig_stuff
	mov	edx,4+4(esp)		| rp
	mov	eax,4+4+4(esp)		| sig
	mov	(ecx),eax		| put signal number in sig_stuff
	mov	eax,EPCREG(edx)		| signalled process' PC
	mov	4(ecx),eax		| put pc in sig_stuff
	mov	eax,ECSREG(edx)		| signalled process' cs
	mov	4+4(ecx),eax		| put cs in sig_stuff
	mov	eax,EPSWREG(edx)	| signalled process' PSW
	mov	4+4+4(ecx),eax		| put psw in sig_stuff
	ret


|*===========================================================================*
|*				check_mem				     *
|*===========================================================================*
| PUBLIC phys_bytes check_mem(phys_bytes base, phys_bytes size);
| Check a block of memory, return the amount valid.
| Only every 16th byte is checked.
| An initial size of 0 means everything.
| This really should do some alias checks.

CM_DENSITY	=	16
CM_LOG_DENSITY	=	4
CHKM_ARGS	=	2 + 4 + 4	| 4 + 4
|			ds ebx eip	base size

_check_mem:
	push	ebx
	push	ds
	mov	ax,#FLAT_DS_SELECTOR
	mov	ds,ax
	mov	eax,CHKM_ARGS(esp)
	mov	ebx,eax
	mov	ecx,CHKM_ARGS+4(esp)
	shr	ecx,#CM_LOG_DENSITY
cm_loop:
	movb	dl,#TEST1PATTERN
	xchgb	dl,(eax)		| write test pattern, remember original
	xchgb	dl,(eax)		| restore original, read test pattern
	cmpb	dl,#TEST1PATTERN	| must agree if good real memory
	jnz	cm_exit			| if different, memory is unusable
	movb	dl,#TEST2PATTERN
	xchgb	dl,(eax)
	xchgb	dl,(eax)
	add	eax,#CM_DENSITY
	cmpb	dl,#TEST2PATTERN
	loopz	cm_loop
cm_exit:
	sub	eax,ebx
#if UNPORTABLE_PAGING
	sub	eax,eax
#endif
	pop	ds
	pop	ebx
	ret


|*===========================================================================*
|*				cim_at_wini				     *
|*				cim_floppy				     *
|*				cim_printer				     *
|*				cim_xt_wini				     *
|*===========================================================================*
| All these routines are meant to be called from the task level where
| interrupts should not be globally disabled, so they return with interrupts
| enabled.

| PUBLIC void cim_at_wini();
| Clear the AT winchester interrupt mask.

_cim_at_wini:
	cli
	in	INT2_MASK
	andb	al,#notop(AT_WINI_MASK)
	out	INT2_MASK
	sti
	ret

| PUBLIC void cim_floppy();
| Clear the AT winchester interrupt mask.

_cim_floppy:
	cli
	in	INT_CTLMASK
	andb	al,#notop(FLOPPY_MASK)
	out	INT_CTLMASK
	sti
	ret

| PUBLIC void cim_printer();
| Clear the printer interrupt mask.

_cim_printer:
	cli
	in	INT_CTLMASK
	andb	al,#notop(PRINTER_MASK) & 0xFF
	out	INT_CTLMASK
	sti
	ret

| PUBLIC void cim_xt_wini();
| Clear the xt_wini interrupt mask.

_cim_xt_wini:
	cli
	in	INT_CTLMASK
	andb	al,#notop(XT_WINI_MASK)
	out	INT_CTLMASK
	sti
	ret


|*===========================================================================*
|*				cp_mess					     *
|*===========================================================================*
| PUBLIC void cp_mess(int src, phys_clicks src_clicks, vir_bytes src_offset,
|		      phys_clicks dst_clicks, vir_bytes dst_offset);
| This routine makes a fast copy of a message from anywhere in the address
| space to anywhere else.  It also copies the source address provided as a
| parameter to the call into the first word of the destination message.
|
| Note that the message size, 'Msize' is in WORDS (not bytes) and must be set
| correctly.  Changing the definition of message in the type file and not
| changing it here will lead to total disaster.

CM_ARGS	=	2 + 2 + 4 + 4 + 4	| 4 + 4 + 4 + 4 + 4
|		es  ds edi esi eip 	proc scl sof dcl dof

_cp_mess:
	push	esi
	push	edi
	push	ds
	push	es

	mov	ax,#FLAT_DS_SELECTOR
	mov	ds,ax
	mov	es,ax

	movzx	esi,word CM_ARGS+4(esp)		| src clicks
	shl	esi,#CLICK_SHIFT
	add	esi,CM_ARGS+4+4(esp)		| src offset
	movzx	edi,word CM_ARGS+4+4+4(esp)	| dst clicks
	shl	edi,#CLICK_SHIFT
	add	edi,CM_ARGS+4+4+4+4(esp)	| dst offset

	mov	eax,CM_ARGS(esp)	| sender's proc no. from arg, not msg
	stosd
	add	esi,#4			| don't copy first word
	mov	ecx,#Msize/2 - 1	| Msize assumed even, dword count now
	rep
	movsd

	pop	es
	pop	ds
	pop	edi
	pop	esi
	ret


|*===========================================================================*
|*				get_byte				     *
|*===========================================================================*
| PUBLIC u32_t get_byte(u32_t segment, u32_t *offset);
| Load and return the byte at the far pointer  segment:offset.

_get_byte:
	mov	cx,ds
	mov	ds,4(esp)		| segment
	mov	eax,4+4(esp)		| offset
	movzx	eax,byte (eax)		| byte to return
	mov	ds,cx
	ret


|*===========================================================================*
|*				in_byte					     *
|*===========================================================================*
| PUBLIC unsigned in_byte(port_t port);
| Read an (unsigned) byte from the i/o port  port  and return it.

_in_byte:
	mov	edx,4(esp)		| port
	sub	eax,eax
	in				| read 1 byte
	ret


|*===========================================================================*
|*				klib_1hook				     *
|*===========================================================================*
| PUBLIC void klib_1hook();
| Initialize klib from real mode for protected mode (no real mode).
| Nothing to do.

_klib_1hook:
	ret				| cheat: opcode same for 16/32-bits


|*===========================================================================*
|*				klib_2hook				     *
|*===========================================================================*
| PUBLIC void klib_2hook();
| Initialize klib from protected mode for protected mode (no real mode).
| Load idt, task reg, flags and enable shadow RAM.
| Kludge a page table to make RAM more contiguous.
| Enable paging.

_klib_2hook:
	push	esi
	push	edi
	lidt	_gdt+BIOS_IDT_SELECTOR	| loaded by BIOS, but in wrong mode!
	mov	ax,#TSS_SELECTOR	| no other TSS is used
	ltr	ax
	sub	eax,eax			| zero
	push	eax			| set flags to known good state
	popfd				| esp, clear nested task and int enable

#if UNPORTABLE_SHADOW_MEMORY_CONTROL
| Enable all banks of RAM on BT386.
| Do it before enabling paging because the magic location is not mapped.
| The BIOS call to switch to protected mode ought to have done this.

	push	ds
	mov	ax,#FLAT_DS_SELECTOR
	mov	ds,ax
	movb	UNPORTABLE_SHADOW_MEMORY_CONTROL,#0xFF
	pop	ds
#endif

| Steal low memory above kernel+mm+fs+init to map to extended memory.
| Don't bother to unmap it from  low memory.

	mov	edi,_code_base
	mov	esi,#_sizes
	mov	ecx,#8
find_end_of_system:
	movzx	eax,word -2(esi+ecx*2)
	shl	eax,#CLICK_SHIFT
	add	edi,eax
	loop	find_end_of_system
	add	edi,#PAGE_SIZE-1	| round up to page boundary
	and	edi,#!(PAGE_SIZE-1)
	mov	cr3,edi			| use for page directory base

	push	es
	mov	ax,#FLAT_DS_SELECTOR
	mov	es,ax

	mov	eax,_low_memsize
	mov	ecx,edi
#if MEM_UNIT > PAGE_SIZE
#error /* several calculations depend on MEM_UNIT <= PAGE_SIZE */
#endif
	shr	ecx,#LOG2_MEM_UNIT
	mov	_low_memsize,ecx	| now it ends at the page directory
	add	ecx,#(NR_PAGE_DIRS*PAGE_DIR_SIZE+NR_PAGE_TABLES*PAGE_TABLE_SIZE)/MEM_UNIT
	sub	eax,ecx
	shr	eax,#LOG2_PAGE_SIZE-LOG2_MEM_UNIT
	push	eax			| pages in low memory to remap
	shl	eax,#LOG2_PAGE_SIZE-LOG2_MEM_UNIT
	mov	ecx,_ext_memsize
	and	ecx,#!(PAGE_SIZE/MEM_UNIT-1)	| round down so top of extended
					| memory is at a page boundary
	push	ecx
	add	ecx,eax
	mov	_ext_memsize,ecx

	lea	eax,PAGE_DIR_SIZE+0x07(edi)	| 0x07 is some access bits
	mov	ecx,#NR_PAGE_TABLES
init_page_dir:
	stosd
	add	eax,#PAGE_TABLE_SIZE
	loop	init_page_dir

	sub	eax,eax
	mov	ecx,#PAGE_TABLE_SIZE/PAGE_DATA_SIZE-NR_PAGE_TABLES
	rep
	stosd

	mov	eax,#0x07
	mov	ecx,#NR_PAGE_TABLES*PAGE_TABLE_SIZE/PAGE_DATA_SIZE
init_page_table:
	stosd
	add	eax,#PAGE_SIZE
	loop	init_page_table

	mov	eax,edi			| entry for base of low memory pages
	or	eax,#0x07
	add	edi,#-NR_PAGE_TABLES*PAGE_TABLE_SIZE+EXTENDED_MEM_BASE/PAGE_SIZE*PAGE_DATA_SIZE
					| page table for beginning of ext mem
	pop	ecx			| size of extended memory
#if LOG2_MEM_UNIT-LOG2_PAGE_SIZE+LOG2_PAGE_DATA_SIZE > 0
	shl	ecx,#LOG2_MEM_UNIT-LOG2_PAGE_SIZE+LOG2_PAGE_DATA_SIZE
#endif
#if LOG2_MEM_UNIT-LOG2_PAGE_SIZE+LOG2_PAGE_DATA_SIZE < 0
	shr	ecx,#LOG2_PAGE_SIZE-LOG2_PAGE_DATA_SIZE-LOG2_MEM_UNIT
#endif
	add	edi,ecx			| page table for orig end of ext mem
	pop	ecx			| number of pages stolen from low mem
	jcxz	over_init_stolen_page_table
init_stolen_page_table:
	stosd
	add	eax,#PAGE_SIZE
	loop	init_stolen_page_table
over_init_stolen_page_table:

#if UNPORTABLE_PAGING

	mov	eax,#0xFA0007		| shadow memory at 0xFA0000
	mov	ecx,#0x60000/PAGE_SIZE	| size 0x60000
init_shadow_page_table:
	stosd
	add	eax,#PAGE_SIZE
	loop	init_shadow_page_table
	add	_ext_memsize,#0x60000/MEM_UNIT

	mov	eax,#0xB0007		| Hercules memory at 0xB0000
	mov	ecx,#0x8000/PAGE_SIZE	| size 0x8000
init_hercules_page_table:
	stosd
	add	eax,#PAGE_SIZE
	loop	init_hercules_page_table
	add	_ext_memsize,#0x8000/MEM_UNIT

#endif /* UNPORTABLE_PAGING */

	pop	es

| start paging

	mov	eax,cr0
	or	eax,#CR0_PG
	mov	cr0,eax

	pop	edi
	pop	esi
	ret


|*===========================================================================*
|*				lock					     *
|*===========================================================================*
| PUBLIC void lock();
| Disable CPU interrupts.

_lock:
	cli
	ret


|*===========================================================================*
|*				phys_copy				     *
|*===========================================================================*
| PUBLIC void phys_copy(phys_bytes source, phys_bytes destination,
|			phys_bytes bytecount);
| Copy a block of physical memory.

PC_ARGS	=	2 + 2 + 4 + 4 + 4	| 4 + 4 + 4
|		es  ds edi esi eip 	src dst len

_phys_copy:
	push	esi
	push	edi
	push	ds
	push	es

	mov	ax,#FLAT_DS_SELECTOR
	mov	ds,ax
	mov	es,ax

	mov	esi,PC_ARGS(esp)
	mov	edi,PC_ARGS+4(esp)
	mov	eax,PC_ARGS+4+4(esp)

	cmp	eax,#10			| avoid align overhead for small counts
	jb	pc_small
	mov	ecx,esi			| align source, hope target is too
	neg	ecx
	and	ecx,#3			| count for alignment
	sub	eax,ecx
	rep
	movb
	mov	ecx,eax
	shr	ecx,#2			| count of dwords
	rep
	movsd
	and	eax,#3
pc_small:
	xchg	ecx,eax			| remainder
	rep
	movb

	pop	es
	pop	ds
	pop	edi
	pop	esi
	ret


|*===========================================================================*
|*				out_byte				     *
|*===========================================================================*
| PUBLIC void out_byte(port_t port, int value);
| Write  value  (cast to a byte)  to the I/O port  port.

_out_byte:
	mov	edx,4(esp)		| port
	mov	al,4+4(esp)		| truncated value
	out				| output 1 byte
	ret


|*===========================================================================*
|*				port_read				     *
|*===========================================================================*
| PUBLIC void port_read(port_t port, phys_bytes destination,unsigned bytcount);
| Transfer data from (hard disk controller) port to memory.

PR_ARGS	=	2 + 4 + 4		| 4 + 4 + 4
|		es edi eip		port dst len

_port_read:
	push	edi
	push	es
	mov	cx,#FLAT_DS_SELECTOR
	mov	es,cx
	mov	edx,PR_ARGS(esp)
	mov	edi,PR_ARGS+4(esp)
	mov	ecx,PR_ARGS+4+4(esp)
	shr	ecx,#1			| word count
	rep				| hardware can't handle dwords
	insw				| read everything
	pop	es
	pop	edi
	ret


|*===========================================================================*
|*				port_write				     *
|*===========================================================================*
| PUBLIC void port_write(port_t port, phys_bytes source, unsigned bytcount);
| Transfer data from memory to (hard disk controller) port.

PW_ARGS	=	2 + 4 + 4		| 4 + 4 + 4
|		es edi eip		port src len

_port_write:
	push	esi
	push	ds
	mov	cx,#FLAT_DS_SELECTOR
	mov	ds,cx
	mov	edx,PW_ARGS(esp)
	mov	esi,PW_ARGS+4(esp)
	mov	ecx,PW_ARGS+4+4(esp)
	shr	ecx,#1			| word count
	rep				| hardware can't handle dwords
	outsw				| write everything
	pop	ds
	pop	esi
	ret


|*===========================================================================*
|*				reset					     *
|*===========================================================================*
| PUBLIC void reset();
| Reset the system.
| This only works in real mode.
| For protected mode, it would be necessary to trap to privilege 0, then do
| something fatal like loading an IDT with offset 0 and interrupting.

_reset:
	j	_reset			| dummy, real mode is not used


|*===========================================================================*
|*				scr_down & scr_up			     *
|*===========================================================================*
| PUBLIC void scr_down(unsigned videoseg, int source, int dest, int count);
| Scroll the screen down one line.
|
| PUBLIC void scr_up(unsigned videoseg, int source, int dest, int count);
| Scroll the screen up one line.
|
| These are identical except scr_down() must reverse the direction flag
| during the copy to avoid problems with overlap.

SDU_ARGS	=	2 + 2 + 4 + 4 + 4	| 4 + 4 + 4 + 4
|			es  ds edi esi eip 	 seg src dst ct

_scr_down:
	std
_scr_up:
	push	esi
	push	edi
	push	ds
	push	es
	mov	ax,SDU_ARGS(esp)	| videoseg (selector for video ram)
	mov	esi,SDU_ARGS+4(esp)	| source (offset within video ram)
	mov	edi,SDU_ARGS+4+4(esp)	| dest (offset within video ram)
	mov	ecx,SDU_ARGS+4+4+4(esp)	| count (in words)
	mov	ds,ax			| set source and dest segs to videoseg
	mov	es,ax
	rep				| do the copy
	movw
	pop	es
	pop	ds
	pop	edi
	pop	esi
	cld				| restore (unnecessarily for scr_up)
	ret


|*===========================================================================*
|*				sim_printer				     *
|*===========================================================================*
| PUBLIC void sim_printer();
| Set the printer interrupt mask.
| This is meant to be called from the task level, so it returns with
| interrupts enabled.

_sim_printer:
	cli
	in	INT_CTLMASK
	orb	al,#PRINTER_MASK
	out	INT_CTLMASK
	sti
	ret


|*===========================================================================*
|*				tasim_printer				     *
|*===========================================================================*
| PUBLIC unsigned tasim_printer();
| Set the printer interrupt mask, indivisibly with getting its old value.
| Return old value.
| This is meant to be called from the task level, so it returns with
| interrupts enabled.
| This might not work for multiple processors, unlike test_and_set().

_tasim_printer:
	cli
	in	INT_CTLMASK
	movb	ah,al
	orb	al,#PRINTER_MASK
	out	INT_CTLMASK
	sti
	movb	al,ah
	and	eax,#PRINTER_MASK
	ret


|*===========================================================================*
|*				test_and_set				     *
|*===========================================================================*
| PUBLIC int test_and_set(int *flag);
| Set the flag to TRUE, indivisibly with getting its old value.
| Return old flag.

_test_and_set:
	mov	ecx,4(esp)
	mov	eax,#1
	xchg	eax,(ecx)
	ret


|*===========================================================================*
|*				unlock					     *
|*===========================================================================*
| PUBLIC void unlock();
| Enable CPU interrupts.

_unlock:
	sti
	ret


|*===========================================================================*
|*				vid_copy				     *
|*===========================================================================*
| PUBLIC void vid_copy(char *buffer, unsigned videobase, int offset,
|		       int words);
| where
|     'buffer'    is a pointer to the (character, attribute) pairs
|     'videobase' is 0xB800 for color and 0xB000 for monochrome displays
|     'offset'    tells where within video ram to copy the data
|     'words'     tells how many words to copy
| if buffer is zero, the fill char (blank_color) is used
|
| This routine takes a string of (character, attribute) pairs and writes them
| onto the screen.  For a snowy display, the writing only takes places during
| the vertical retrace interval, to avoid displaying garbage on the screen.

VC_ARGS	=	2 + 4 + 4 + 4 + 4	| 4 + 4 + 4 + 4
|		es edi esi ebx eip 	 buf bas off words

_vid_copy:
	push	ebx
	push	esi
	push	edi
	push	es
vid.0:
	mov	esi,VC_ARGS(esp)	| buffer
	mov	es,VC_ARGS+4(esp)	| video_base
	mov	edi,VC_ARGS+4+4(esp)	| offset
	and	edi,_vid_mask		| only 4K or 16K counts
	mov	ecx,VC_ARGS+4+4+4(esp)	| word count for copy loop

	lea	ebx,-1(edi+ecx*2)	| point at last char in target
	sub	ebx,_vid_mask		| # characters beyond end of video ram
	jle	vid.1			| copy does not run off end of vram
	sar	ebx,#1			| # words that don't fit
	sub	ecx,ebx			| reduce count by overrun

vid.1:
	push	ecx			| save actual count used for later

| With a snowy (color, most CGA's) display, you can avoid snow by only copying
| to video ram during vertical retrace.

	cmp	_snow,#0
	jz	over_wait_for_retrace
	call	_wait_retrace

over_wait_for_retrace:
	test	esi,esi			| buffer == 0 means blank the screen
	je	vid.7			| jump for blanking
	rep				| this is the copy loop
	movw				| ditto

vid.5:
	pop	esi			| count of words copied
	test	ebx,ebx			| if no overrun, we are done
	jle	vc_exit			| everything fit

	mov	VC_ARGS+4+4+4(esp),ebx	| set up residual count
	mov	VC_ARGS+4+4(esp),#0	| start copying at base of video ram
	cmp	VC_ARGS(esp),#0		| NIL_PTR means store blanks
	je	vid.0			| go do it
	add	esi,esi			| count of bytes copied
	add	VC_ARGS(esp),esi	| increment buffer pointer
	j	vid.0			| go copy some more

vc_exit:
	pop	es
	pop	edi
	pop	esi
	pop	ebx
	ret

vid.7:	mov	ax,_blank_color		| ax = blanking character
	rep				| copy loop
	stow				| blank screen
	j	vid.5			| done


|*===========================================================================*
|*			      wait_retrace				     *
|*===========================================================================*
| PUBLIC void wait_retrace();
| Wait for the *start* of retrace period.
| The VERTICAL_RETRACE_MASK of the color vid_port is set during retrace.
| First wait until it is off (no retrace).
| Then wait until it comes on (start of retrace).
| We can't afford to worry about interrupts.

_wait_retrace:
	mov	dx,_vid_port
	orb	dl,#COLOR_STATUS_PORT & 0xFF
wait_no_retrace:
	in
	testb	al,#VERTICAL_RETRACE_MASK
	jnz	wait_no_retrace
wait_retrace:
	in
	testb	al,#VERTICAL_RETRACE_MASK
	jz	wait_retrace
	ret
