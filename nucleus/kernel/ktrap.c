
#include <kernel.h>
#include <mouse.h>
#include <ps2mouse.h>
#include <timer.h>
#include <assert.h>
#include "../../clib/include/cinclude.h"



VIR_ADDR k_user_map_screen( void )
{
    VIR_ADDR	base;
    VIR_ADDR	screen_adr;
    int			npages;
    int			screen_size;
    PHY_ADDR	scr_adr;
    int			i;

    base = k_sbrk( 0 );
    /* Make sbrk pointer page aligned */
    if( base % PAGE_SIZE )
	k_sbrk( PAGE_SIZE - ( base % PAGE_SIZE ) );
    screen_size = ( 80 * 25 * 2 ) / PAGE_SIZE + 1;
    screen_size *= PAGE_SIZE;
    base = k_sbrk( screen_size );
    screen_adr = base - VM_USER_TEXT_BEGIN;
    npages = 80 * 25 * 2 / PAGE_SIZE + 1;
    scr_adr = get_global_param( GLOBAL_PARAM_SCREEN_BASE );
    for( i = 0; i < npages; i++ ) {
	map_phy_addr( active_pcb->context_cr3, scr_adr, base );
	scr_adr += PAGE_SIZE;
	base += PAGE_SIZE;
    }
    return( screen_adr );
}



VIR_ADDR k_map_font_area( void )
{
    VIR_ADDR	base;

    base = k_sbrk( 0 );
    /* Make sbrk pointer page aligned */
    if( base % PAGE_SIZE )
	k_sbrk( PAGE_SIZE - ( base % PAGE_SIZE ) );
    base = k_sbrk( 2 * PAGE_SIZE );
    map_phy_addr( active_pcb->context_cr3, 0xa0000, base );
    map_phy_addr( active_pcb->context_cr3, 0xa0000 + PAGE_SIZE, base + PAGE_SIZE );
    return( base - VM_USER_TEXT_BEGIN );
}



int k_user_sbrk( int nbytes )
{
    int p;

    p = k_sbrk( nbytes );
    if( p == -1 ) return( p );
    return( p - VM_USER_TEXT_BEGIN );
}


int k_get_copy_buffer( void )
{
    VIR_ADDR	base;

    base = k_sbrk( 0 );
    /* Make sbrk pointer page aligned */
    if( base % PAGE_SIZE )
	k_sbrk( PAGE_SIZE - ( base % PAGE_SIZE ) );
    return( k_sbrk( RECEIVE_BUFFER_SIZE ) - VM_USER_TEXT_BEGIN );
}



EVENT_BUFFER mouse_event;
int mouse_queue[ NUM_MOUSE_EVENT_SLOTS * NUM_INTS_PER_MOUSE_SLOT ];



void k_create_mouse_process( PORT_ID father_port, HANDLE param )
{
    PID		dummy_pid;
    PORT_ID	dummy_port;

    init_event_queue( &mouse_event, NUM_MOUSE_EVENT_SLOTS,
		      NUM_INTS_PER_MOUSE_SLOT,
		      mouse_queue );
    connect_port_to_event_queue( father_port, &mouse_event );
#ifdef USE_PS2_MOUSE
    ps2mouse_init( father_port );
#endif
#ifdef USE_SERIAL_MOUSE
    mouse_init( father_port );
#endif
}



void k_get_mouse_event( int *mouse_px, int *mouse_py, int *mouse_button )
{
    dequeue_event( &mouse_event, mouse_px, mouse_py, mouse_button );
}



/*
 * service_kernel_trap()
 *----------------------------------------------------------------------------
 * This function gets called by kernel_trap() within idt.c which
 * itself gets called by a user function. The user provides
 * five parameters: op for operation and p1, p2, p3, p4 for
 * user parameters.
 * service_kernel_trap() itself responds with four return
 * parameters r1, r2, r3, r4. Beware that those return parameters
 * are *NOT* call-by-reference. kernel_trap() simply pops those
 * parameters off the stack. For this reason r1, ..., r4 need
 * to be tagged with volatile. Otherwise GCC would ignore these
 * variables (in an ordinary C-program this would not work).
 */

void service_kernel_trap( int op, int p1, int p2, int p3, int p4,
			  volatile int r1, volatile int r2,
			  volatile int r3, volatile int r4 )
{
    switch( op ) {
    case K_MAP_SCREEN:
	r1 = k_user_map_screen();
	break;
    case K_SBRK:
	r1 = k_user_sbrk( (int) p1 );
	break;
    case K_GET_SCREEN_MODE:
	r1 = get_global_param( GLOBAL_PARAM_SCREEN_MODE );
	break;
    case K_GET_VIDEO_TYPE:
	r1 = get_global_param( GLOBAL_PARAM_VIDEO_TYPE );
	break;
    case K_INPORTB:
	r1 = (int) inportb( p1 );
	break;
    case K_OUTPORTB:
	outportb( p1, (char) p2 );
	break;
    case K_GET_COPY_BUFFER:
	r1 = k_get_copy_buffer();
	break;
    case K_SEND:
	r1 = k_send( p1, p2 + VM_USER_TEXT_BEGIN, p3, (int *) &r2 );
	break;
    case K_MESSAGE:
	r1 = k_message( p1, p2 + VM_USER_TEXT_BEGIN, p3 );
	break;
    case K_RECEIVE:
	r1 = k_receive( p1 + VM_USER_TEXT_BEGIN, (int *) &r2, (int *) &r3 );
	break;
    case K_REPLY:
	r1 = k_reply( p1, p2 + VM_USER_TEXT_BEGIN, p3 );
	break;
    case K_FS_SEND:
	r1 = k_send( file_server_port, p1 + VM_USER_TEXT_BEGIN,
		     p2, (int *) &r2 );
	break;
    case K_WINDOW_SERVER_SEND:
	r1 = k_send( window_port, p1 + VM_USER_TEXT_BEGIN,
		     p2, (int *) &r2 );
	break;
    case K_EVENT_SEND:
	r1 = k_send( event_port, p1 + VM_USER_TEXT_BEGIN,
		     p2, (int *) &r2 );
	break;
    case K_EVENT_MESSAGE:
	r1 = k_message( event_port, p1 + VM_USER_TEXT_BEGIN, p2 );
	break;
    case K_TIMER_MANAGER_MESSAGE:
	r1 = k_message( timer_port, p1 + VM_USER_TEXT_BEGIN, p2 );
	break;
    case K_REGISTER_WINDOW_SERVER:
	window_port = p1;
	break;
    case K_OPEN_WINDOW_SERVER_PORT:
	open_port( window_port );
	break;
    case K_CLOSE_WINDOW_SERVER_PORT:
	close_port( window_port );
	break;
    case K_CREATE_USER_PROC:
	create_user_process( p1, (int *) &r1, (int *) &r2 );
	break;
    case K_EXIT:
	if( active_pcb->debugger )
	    k_debugger_exited();
	k_suicide();
	break;
    case K_CREATE_MOUSE_PROCESS:
	k_create_mouse_process( p1, p2 );
	break;
    case K_GET_MOUSE_EVENT:
	k_get_mouse_event( (int *) &r1, (int *) &r2, (int *) &r3 );
	break;
    case K_CREATE_PORT:
	r1 = create_port( (PORT_ID *) &r2 );
	break;
    case K_CLOSE_PORT:
	r1 = close_port( p1 );
	break;
    case K_OPEN_PORT:
	r1 = open_port( p1 );
	break;
    case K_MAP_FONT_AREA:
	r1 = k_map_font_area();
	break;
    case K_WAKEUP_PROCESS:
	r1 = k_wakeup( (PID) p1 );
	break;
    case K_SECS_SINCE_1970:
	r1 = seconds_since_1970;
	break;
    case K_START_SINGLE_STEP:
	if( active_pcb->to_be_debugged )
	    k_start_single_step();
	break;
    case K_HOOK_DEBUGGER:
	k_hook_debugger( (PORT_ID) p1 );
	break;
    case K_SET_DEBUG_FLAG:
	k_define_prg_to_debug( (PID) p1 );
	break;
    case K_NEXT_SINGLE_STEP:
	r1 = k_next_single_step();
	break;
    default:
	shutdown( "service_kernel_trap(): Bad op (%d)", op );
	break;
    }
}

