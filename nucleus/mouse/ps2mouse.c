#include <stdio.h>
#include <ci_types.h>
#include <kernel.h>
#include <ps2mouse.h>
#include "io.h"


static HANDLE      mouse_port;            /* the internal hermes port */
static char        left_button_pressed;   /* left button flag */
static char        right_button_pressed;  /* right button flag */
static int         char_count;            /* 3 chars is a complete sequence */
static char        mouse_data[ 3 ];       /* holds the mouse data */
static unsigned    button_stat;           /* holds the button status */
static unsigned    button;                /* used to determine status */
static char        dx, dy;                /* relative mouse position */


/*
 *   Inits the PS/2 mouse:
 *   sends init commands to the device,
 *   sets the variables used by the interrupt service routine,
 *   installs interrupt service routine.
 */
void ps2mouse_init( HANDLE father_port )
{
    char command_code;

#if 0
    printf("init ps2mouse\n");
#endif

    outportb( 0x64, 0xa8 );                /* init auxd */
    outportb( 0x64, 0x20 );                /* request keyboard command code */
    command_code = inportb ( 0x60 );       /* read keyboard command code */
    outportb( 0x64, 0x60 );                /* announce command code */
    outportb( 0x60, command_code | 0x02 ); /* write command code */
    outportb( 0x64, 0xd4 );                /* direct command to auxd */
    outportb( 0x60, 0xf4 );                /* init auxd */
    inportb ( 0x60 );                      /* read ACK */
    outportb( 0xA0, 0x20 );                /* reset slave PIC */

    /* init globals */
    char_count = 0;
    left_button_pressed  = FALSE;
    right_button_pressed = FALSE;
    mouse_port = father_port;

    /* install interrupt service routine. */
    init_idt_entry( 0x6C, IDT_ENTRY( ps2mouse_isr ) );
}


/*
 * This function should be pasted into ps2mouse_isr(). But when we
 * do that, gcc 3.2 will save %ebx in the function prologue which
 * miss-aligns the calling stack.
 */
void ps2mouse_isr_aux (void)
{
#if 0
    printf("m ");
#endif
    
    /* PS/2 MOUSE HANDLER */

    outportb( 0x64, 0xad );                          /* disable keyboard */
    /* read data */
    mouse_data [ char_count++ ] = inportb ( 0x60 );  /* read byte */

    /* this is to synchronize the mouse byte packets.
     * bit 3 of the first packet is always 1.
     */
    if ( char_count == 1 )
        if (!(mouse_data[0] & 0x08))
            char_count--;
    
    /* mouse data is complete if 3 bytes are read */
    if ( char_count == 3 )
    {
        char_count = 0;

        /* mouse movement */
        if (mouse_data[1] != 0)
            dx=   (mouse_data[0] & 0x10)
                ? mouse_data[1]-256 : mouse_data[1];
        else
            dx = 0;
        if (mouse_data[2] != 0)
            dy = -((mouse_data[0] & 0x20)
                   ? mouse_data[2]-256 : mouse_data[2]);
        else
            dy = 0;

        /* mouse buttons */
        button_stat = 0;
        button = mouse_data[0] & 0x03;

        /* left button */
        if (button & 0x01)
        {
            button_stat |= LEFT_PRESSED;
            left_button_pressed = TRUE;
        }
        else if (left_button_pressed)
        {
            button_stat |= LEFT_RELEASED;
            left_button_pressed = FALSE;
        }

        /* right button */
        if (button & 0x02)
        {
            button_stat |= RIGHT_PRESSED;
            right_button_pressed = TRUE;
        }
        else if (right_button_pressed)
        {
            button_stat |= RIGHT_RELEASED;
            right_button_pressed = FALSE;
        }

        /* put event in hermes event queue */
        queue_event( &mouse_event, dx, dy, button_stat );
        raise_event( (PORT_ID) mouse_port );
    }

    outportb( 0x64, 0xae );                          /* enable keyboard */
}


/*
 *   PS/2 mouse interrupt service routine.
 */
void ps2mouse_isr( void )
{

    /*
     * INTERRUPT SERVICE ROUTINE PROLOG
     * 
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

    ps2mouse_isr_aux();
    
    /*
     * INTERRUPT SERVICE ROUTINE EPILOG
     * 
     *		MOVB	$0x20,%AL	; Reset interrupt controller
     *		OUTB	%AL,$0xA0       ; send to slave
     *		POP 	%GS 		; Restore previously saved context
     *		POP 	%FS
     *		POP 	%ES
     *		POP 	%DS
     *		POPAL
     *		IRET			; Return to new process
     *
     *		
     * Since this interrupt is raised from the slave 8259,
     * the reset command gets sent to the slave on port A0h
     * and the master on port 20h.
     * -mc
     */
    asm( "movb $0x20,%al;outb %al,$0xA0" );
    asm( "movb $0x20,%al;outb %al,$0x20" );
    asm( "pop %gs;pop %fs;pop %es;pop %ds" );
    asm( "popal;iret" );
}
