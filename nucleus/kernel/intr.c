
#include <kernel.h>
#include <ps2mouse.h>
#include <keyb.h>


PRIVATE unsigned char interrupt_table[ MAX_INTERRUPTS ];
PRIVATE unsigned char p;



/*
 * Timer interrupt
 */

PRIVATE void service_intr_0x60( void )
{
    /*
     *		PUSHAL			; Push context
     *		PUSH	%DS
     *		PUSH	%ES
     *		PUSH	%FS
     *		PUSH	%GS
     *		MOVW	$DATA_SELECTOR,%AX
     *		MOVW	%AX,%DS
     */
    asm( "pushal;push %ds;push %es;push %fs;push %gs" );
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );

    if( ( p = interrupt_table[ 0x60 ] ) == 0xff )
	shutdown( "service_intr_0x60: Spurious interrupt\n" );

    if( pcb[ p ].state != STATE_EVENT_BLOCKED )
	shutdown( "service_intr_0x60: No proc waiting" );

    /* Save the context pointer SS:ESP to the PCB */
    asm( "movw %%ss,%0" : "=r" (active_pcb->context_ss) : );
    asm( "movl %%esp,%0" : "=m" (active_pcb->context_esp) : );
    asm( "movl %%cr3,%0" : "=r" (active_pcb->context_cr3) : );

    /* Add event handler to ready queue */
    add_ready_queue( &pcb[ p ] );

    /* Dispatch new process */
    active_pcb = dispatcher();

    /* Restore context pointer SS:ESP */
    asm( "movl %0,%%cr3" : : "r" (active_pcb->context_cr3) );
    asm( "movw %0,%%ss" : : "r" (active_pcb->context_ss) );
    asm( "movl %0,%%esp" : : "m" (active_pcb->context_esp) );

    /*
     *		MOVB	$0x20,%AL	; Reset interrupt controller
     *		OUTB	%AL,$0x20
     *		POP 	%GS 		; Restore previously saved context
     *		POP 	%FS
     *		POP 	%ES
     *		POP 	%DS
     *		POPAL
     *		IRET			; Return to new process
     */
    asm( "movb $0x20,%al;outb %al,$0x20" );
    asm( "pop %gs;pop %fs;pop %es;pop %ds" );
    asm( "popal;iret" );
}



/*
 * Mouse interrupt
 */
#if 0
PRIVATE void service_intr_0x64( void )
{
    /*
     *		PUSHAL			; Push context
     *		PUSH	%DS
     *		PUSH	%ES
     *		PUSH	%FS
     *		PUSH	%GS
     *		MOVW	$DATA_SELECTOR,%AX
     *		MOVW	%AX,%DS
     */
    asm( "pushal;push %ds;push %es;push %fs;push %gs" );
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );

    if( ( p = interrupt_table[ 0x64 ] ) == 0xff )
	shutdown( "service_intr_0x64: Spurious interrupt\n" );

#if 0
    // VMWare is too slow
    if( pcb[ p ].state != STATE_EVENT_BLOCKED )
	shutdown( "service_intr_0x64: No proc waiting" );
#endif

    /* Save the context pointer SS:ESP to the PCB */
    asm( "movw %%ss,%0" : "=r" (active_pcb->context_ss) : );
    asm( "movl %%esp,%0" : "=m" (active_pcb->context_esp) : );
    asm( "movl %%cr3,%0" : "=r" (active_pcb->context_cr3) : );

#if 1
    // VMWare is too slow
    if( pcb[ p ].state == STATE_EVENT_BLOCKED )
	add_ready_queue( &pcb[ p ] );
#else
    /* Add event handler to ready queue */
    add_ready_queue( &pcb[ p ] );
#endif

    /* Dispatch new process */
    active_pcb = dispatcher();

    /* Restore context pointer SS:ESP */
    asm( "movl %0,%%cr3" : : "r" (active_pcb->context_cr3) );
    asm( "movw %0,%%ss" : : "r" (active_pcb->context_ss) );
    asm( "movl %0,%%esp" : : "m" (active_pcb->context_esp) );

    /*
     *		MOVB	$0x20,%AL	; Reset interrupt controller
     *		OUTB	%AL,$0x20
     *		POP 	%GS 		; Restore previously saved context
     *		POP 	%FS
     *		POP 	%ES
     *		POP 	%DS
     *		POPAL
     *		IRET			; Return to new process
     */
    asm( "movb $0x20,%al;outb %al,$0x20" );
    asm( "pop %gs;pop %fs;pop %es;pop %ds" );
    asm( "popal;iret" );
}
#endif


/*
 * Floppy interrupt
 */

PRIVATE void service_intr_0x66( void )
{
    /*
     *		PUSHAL			; Push context
     *		PUSH	%DS
     *		PUSH	%ES
     *		PUSH	%FS
     *		PUSH	%GS
     *		MOVW	$DATA_SELECTOR,%AX
     *		MOVW	%AX,%DS
     */
    asm( "pushal;push %ds;push %es;push %fs;push %gs" );
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );

    if( ( p = interrupt_table[ 0x66 ] ) == 0xff )
	shutdown( "service_intr_0x66: Spurious interrupt\n" );

    if( pcb[ p ].state == STATE_EVENT_BLOCKED ) {

	/* Save the context pointer SS:ESP to the PCB */
	asm( "movw %%ss,%0" : "=r" (active_pcb->context_ss) : );
	asm( "movl %%esp,%0" : "=m" (active_pcb->context_esp) : );
	asm( "movl %%cr3,%0" : "=r" (active_pcb->context_cr3) : );

	/* Add event handler to ready queue */
	add_ready_queue( &pcb[ p ] );

	/* Dispatch new process */
	active_pcb = dispatcher();

	/* Restore context pointer SS:ESP */
	asm( "movl %0,%%cr3" : : "r" (active_pcb->context_cr3) );
	asm( "movw %0,%%ss" : : "r" (active_pcb->context_ss) );
	asm( "movl %0,%%esp" : : "m" (active_pcb->context_esp) );
    }

    /*
     *		MOVB	$0x20,%AL	; Reset interrupt controller
     *		OUTB	%AL,$0x20
     *		POP 	%GS 		; Restore previously saved context
     *		POP 	%FS
     *		POP 	%ES
     *		POP 	%DS
     *		POPAL
     *		IRET			; Return to new process
     */
    asm( "movb $0x20,%al;outb %al,$0x20" );
    asm( "pop %gs;pop %fs;pop %es;pop %ds" );
    asm( "popal;iret" );
}



ERROR register_isr( unsigned char intr_no )
{
    volatile int i_flag;

    DISABLE_INTR( i_flag );
    if( interrupt_table[ intr_no ] != 0xff ) {
	ENABLE_INTR( i_flag );
	return( panic( ERR_ISR_BUSY, "register_isr(): ISR busy" ) );
    }
    switch( intr_no ) {
    case 0x60: /* timer */
	init_idt_entry( 0x60, IDT_ENTRY( service_intr_0x60 ) );
	break;
    case 0x66: /* floppy */
	init_idt_entry( 0x66, IDT_ENTRY( service_intr_0x66 ) );
	break;
    default:
	ENABLE_INTR( i_flag );
	return( panic( ERR_BAD_INTR, "register_isr(): Bad intr" ) );
    }
    interrupt_table[ intr_no ] = active_pcb - pcb;
    ENABLE_INTR( i_flag );
    return( OK );
}



void await_event( void )
{
    volatile int i_flag;

    DISABLE_INTR( i_flag );
    remove_ready_queue( active_pcb );
    active_pcb->state = STATE_EVENT_BLOCKED;
    resign();
    ENABLE_INTR( i_flag );
}


void init_interrupts( void )
{
    int i;

    for( i = 0; i < MAX_INTERRUPTS; i++ )
	interrupt_table[ i ] = 0xff;
}
