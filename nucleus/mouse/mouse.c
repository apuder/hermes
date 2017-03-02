
#include <ci_types.h>
#include <kernel.h>
#include <mouse.h>

static HANDLE   mouse_port;
static char	mouse_dat[ 3 ];
static unsigned	button_stat;
static unsigned	button;
static char	dx, dy;
static char	left_button_pressed;
static char	right_button_pressed;
static int	tag;
static int	char_count;
static char	new_char;


void initcom( void )
{
    int divisor;

    divisor = 115200 / 1200;
    outportb( ComPort + 3, 0x80 );
    outportb( ComPort, divisor & 255 );
    outportb( ComPort + 1, (divisor >> 8) & 255 );
    outportb( ComPort + 3, 2 );
    outportb( ComPort + 1, 1 );
    outportb( ComPort + 4, 0x0b );
    inportb( ComPort );
}



void int_ein()
{
    outportb( IRQ_Mask, inportb( IRQ_Mask ) & 0xe7 );
}



void int_aus()
{
    outportb( IRQ_Mask, inportb( IRQ_Mask ) | 0x18 );
}


void mouse_init( HANDLE father_port )
{
    mouse_port = father_port;
    initcom();
    int_ein();
    char_count = 0;
    right_button_pressed = FALSE;
    left_button_pressed  = FALSE;

    /* install interrupt service routine. */
    init_idt_entry( 0x64, IDT_ENTRY( mouse_isr ) );
}


void mouse_isr( void )
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

    new_char = inportb( ComPort );
    if( (new_char & 64) != 0 ) {
        char_count = 1;
        mouse_dat[ 0 ] = new_char;
    }
    if( (new_char & 64) == 0 && char_count < 3 )
        mouse_dat[ char_count++ ] = new_char;
    if( char_count == 3 ) {
        dx = (( mouse_dat[0] << 6 ) & 192) | mouse_dat[1];
        dy = (( mouse_dat[0] << 4 ) & 192) | mouse_dat[2];
        button = ( mouse_dat[ 0 ] >> 4 ) & 3;
        tag = 1;
        button_stat = 0;
        if( button & 1 ) {
            tag = 0;
            button_stat |= RIGHT_PRESSED;
            right_button_pressed = TRUE;
        } else if( right_button_pressed ) {
            tag = 0;
            button_stat |= RIGHT_RELEASED;
            right_button_pressed = FALSE;
        }
        if( button & 2 ) {
            tag = 0;
            button_stat |= LEFT_PRESSED;
            left_button_pressed = TRUE;
        } else if( left_button_pressed ) {
            tag = 0;
            button_stat |= LEFT_RELEASED;
            left_button_pressed = FALSE;
        }
        /* noch Unterscheidung tag == 1 oder nicht */
        queue_event( &mouse_event, dx, dy,
                     button_stat );
        raise_event( (PORT_ID) mouse_port );
    }

    /*
     * INTERRUPT SERVICE ROUTINE EPILOG
     * 
     *		MOVB	$0x20,%AL	; Reset interrupt controller
     *		OUTB	%AL,$0x20       ; send to master
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
