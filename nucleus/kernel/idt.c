
#include <kernel.h>
#include "io.h"



extern void spurious_int( void );



PRIVATE unsigned char idt_table[ MAX_IDT_ENTRIES * IDT_ENTRY_SIZE ];



PRIVATE void load_idt( unsigned base, short unsigned limit )
{
    volatile unsigned char mem48[ 6 ];
    volatile unsigned *base_ptr;
    volatile short unsigned *limit_ptr;

    base_ptr = (unsigned *) &mem48[ 2 ];
    limit_ptr = (short unsigned *) &mem48[ 0 ];
    *base_ptr = base + HERMES_TEXT_BASE;
    *limit_ptr = limit;
    asm( "lidt %0" : "=m" (mem48) );
}



PRIVATE void exception0( void )
{
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    printf( "Fatal exception 0 (%s)... ", active_pcb->name );
    for(;;);
}



PRIVATE void exception1( void )
{
    /*
     * Save the registers.
     */
    asm( "pushal" );
    asm( "push %ds;push %es;push %fs;push %gs" );
    /*
     * Enable interrupts and initialize %ds and %gs
     */
    asm( "sti" );
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    /*
     * Push the %eip-address onto the stack.
     */
    asm( "pushl 12*4(%esp)" );
    /*
     * Call the debugger.
     */
    asm( "call k_single_step" );
    /*
     * Remove parameters.
     */
    asm( "addl $4,%esp" );
    /*
     * Test if the TF should be cleared.
     */
    if( !active_pcb->single_step )
	/*
	 * Clear TF bit in EFLAGS.
	 */
	asm( "andl $~256,14*4(%esp)" );
    /*
     * Restore registers, iret.
     */
    asm( "pop %gs;pop %fs;pop %es;pop %ds;popal" );
    asm( "iret" );
}



PRIVATE void exception2( void )
{
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    printf( "Fatal exception 2 (%s)... ", active_pcb->name );
    for(;;);
}


PRIVATE void exception3( void )
{
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    printf( "Fatal exception 3 (%s)... ", active_pcb->name );
    for(;;);
}


PRIVATE void exception4( void )
{
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    printf( "Fatal exception 4 (%s)... ", active_pcb->name );
    for(;;);
}


PRIVATE void exception5( void )
{
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    printf( "Fatal exception 5 (%s)... ", active_pcb->name );
    for(;;);
}


PRIVATE void exception6( void )
{
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    printf( "Fatal exception 6 (%s)... ", active_pcb->name );
    for(;;);
}


PRIVATE void exception7( void )
{
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    printf( "Fatal exception 7 (%s)... ", active_pcb->name );
    for(;;);
}


PRIVATE void exception8( void )
{
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    printf( "Fatal exception 8 (%s)... ", active_pcb->name );
    for(;;);
}


PRIVATE void exception9( void )
{
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    printf( "Fatal exception 9 (%s)... ", active_pcb->name );
    for(;;);
}


PRIVATE void exception10( void )
{
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    printf( "Fatal exception 10 (%s)... ", active_pcb->name );
    for(;;);
}


PRIVATE void exception11( void )
{
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    printf( "Fatal exception 11 (%s)... ", active_pcb->name );
    for(;;);
}



PRIVATE void exception12( void )
{
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    printf( "Fatal exception 12 (%s)... ", active_pcb->name );
    for(;;);
}


PRIVATE void exception13( void )
{
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    printf( "*****General protection error (%s) ErrGP=0x???... ",
	    active_pcb->name );
    for(;;);
}


/*
 * exception14()
 *----------------------------------------------------------------------------
 * This fault is raised by the 386 whenever a page fault occurs.
 * An error code is pushed onto the stack just before this function
 * is called.
 * Care must be taken to save all registers and to initialize the
 * data segment for the kernel data space.
 */

PRIVATE void exception14( void )
{
    /*
     * The next instruction swaps %eax with the top of the stack.
     * This will save %eax and simultaneusly load %eax with the
     * error code provided by the 386.
     */
    asm( "xchgl (%esp),%eax" );
    /*
     * Save the remaining registers.
     */
    asm( "pushl %ecx;pushl %edx;pushl %ebx" );
    asm( "pushl %esp;pushl %ebp;pushl %esi;pushl %edi" );
    asm( "push %ds;push %es;push %fs;push %gs" );
    /*
     * Push the error code onto the stack.
     */
    asm( "pushl %eax" );
    /*
     * Push the linear address that caused the fault onto the stack.
     */
    asm( "movl %cr2,%eax;pushl %eax" );
    /*
     * Enable interrupts and initialize %ds and %gs
     */
    asm( "sti" );
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    /*
     * Call the page fault handler.
     */
    asm( "call service_page_fault" );
    /*
     * Remove parameters, restore registers, iret.
     */
    asm( "addl $8,%esp" );
    asm( "pop %gs;pop %fs;pop %es;pop %ds;popal" );
    asm( "iret" );
}



PRIVATE void exception15( void )
{
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    printf( "Fatal exception 15 (%s)... ", active_pcb->name );
    for(;;);
}


PRIVATE void exception16( void )
{
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );
    printf( "Fatal exception 16 (%s)... ", active_pcb->name );
    for(;;);
}



PRIVATE void kernel_trap( void )
{
    /*
     * Save %ds and leave space for 4 return parameters.
     */
    asm( "push %ds" );
    asm( "subl $4*4,%esp" );
    /*
     * Save parameters on stack.
     */
    asm( "pushl %edx;pushl %ecx;pushl %ebx;pushl %eax;pushl %esi" );
    /*
     * Enable interrupts and initialize %ds and %gs
     */
    asm( "sti" );
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );
    asm( "movw %0,%%ax" : : "i" (GLOBAL_DATA_SELECTOR) );
    asm( "movw %ax,%gs" );

    /*
     * Call the kernel trap handler.
     */
    asm( "call service_kernel_trap" );
    /*
     * Check if the trap bit has to be set.
     */
    if( active_pcb->single_step )
	asm( "orl $256,12*4(%esp)" );
    /*
     * Remove input-parameters and load the return parameters into %eax, ...
     */
    asm( "addl $5*4,%esp" );
    asm( "popl %eax;popl %ebx;popl %ecx;popl %edx" );
    asm( "pop %ds" );
    asm( "iret" );
}



void init_idt_entry( int x, unsigned char b1,
		     unsigned char b2,
		     unsigned char b3,
		     unsigned char b4,
		     unsigned char b5,
		     unsigned char b6,
		     unsigned char b7,
		     unsigned char b8 )
{
    x *= IDT_ENTRY_SIZE;
    idt_table[ x++ ] = b1;
    idt_table[ x++ ] = b2;
    idt_table[ x++ ] = b3;
    idt_table[ x++ ] = b4;
    idt_table[ x++ ] = b5;
    idt_table[ x++ ] = b6;
    idt_table[ x++ ] = b7;
    idt_table[ x   ] = b8;
}



void spurious_int( void )
{
    asm( "pusha;movb $0x20,%al;outb %al,$0x20;popa;iret" );
}


void delay ()
{
    asm ("nop;nop;nop");
}


void init_idt( void )
{
    int x;

    load_idt( (unsigned) idt_table, MAX_IDT_ENTRIES * IDT_ENTRY_SIZE - 1 );
    for( x = 0; x < MAX_IDT_ENTRIES; x++ )
	init_idt_entry( x, IDT_ENTRY( spurious_int ) );
    init_idt_entry( 0, IDT_ENTRY( exception0 ) );
    init_idt_entry( 1, IDT_ENTRY( exception1 ) );
    init_idt_entry( 2, IDT_ENTRY( exception2 ) );
    init_idt_entry( 3, IDT_ENTRY( exception3 ) );
    init_idt_entry( 4, IDT_ENTRY( exception4 ) );
    init_idt_entry( 5, IDT_ENTRY( exception5 ) );
    init_idt_entry( 6, IDT_ENTRY( exception6 ) );
    init_idt_entry( 7, IDT_ENTRY( exception7 ) );
    init_idt_entry( 8, IDT_ENTRY( exception8 ) );
    init_idt_entry( 9, IDT_ENTRY( exception9 ) );
    init_idt_entry( 10, IDT_ENTRY( exception10 ) );
    init_idt_entry( 11, IDT_ENTRY( exception11 ) );
    init_idt_entry( 12, IDT_ENTRY( exception12 ) );
    init_idt_entry( 13, IDT_ENTRY( exception13 ) );
    init_idt_entry( 14, IDT_ENTRY( exception14 ) );
    init_idt_entry( 15, IDT_ENTRY( exception15 ) );
    init_idt_entry( 16, IDT_ENTRY( exception16 ) );

    /* Set kernel trap entry point and make it accessible for level 3 */
    init_idt_entry( KERNEL_TRAP_INTR, IDT_ENTRY( kernel_trap ) );
    idt_table[ KERNEL_TRAP_INTR * IDT_ENTRY_SIZE + 5 ] |= 3 << 5;

    /* Shift IRQ Vectors so they don't collide with the
       386 generated IRQs */

    // Send initialization sequence to 8259A-1
    asm( "movb $0x11,%al;outb %al,$0x20;call delay" );
    // Send initialization sequence to 8259A-2
    asm( "movb $0x11,%al;outb %al,$0xA0;call delay" );
    // IRQ base for 8259A-1 is 0x60
    asm( "movb $0x60,%al;outb %al,$0x21;call delay" );
    // IRQ base for 8259A-2 is 0x68
    asm( "movb $0x68,%al;outb %al,$0xA1;call delay" );
    // 8259A-1 is the master
    asm( "movb $0x04,%al;outb %al,$0x21;call delay" );
    // 8259A-2 is the slave
    asm( "movb $0x02,%al;outb %al,$0xA1;call delay" );
    // 8086 mode for 8259A-1
    asm( "movb $0x01,%al;outb %al,$0x21;call delay" );
    // 8086 mode for 8259A-2
    asm( "movb $0x01,%al;outb %al,$0xA1;call delay" );
    // Don't mask IRQ for 8259A-1
    asm( "movb $0x00,%al;outb %al,$0x21;call delay" );
    // Don't mask IRQ for 8259A-2
    asm( "movb $0x00,%al;outb %al,$0xA1;call delay" );


#if 0
    printf( "init_idt(): IDT initialized\n" );
#endif
}

