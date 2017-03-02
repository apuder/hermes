
#include <ci_types.h>
#include <kernel.h>
#include <vulture.h>
#include <fs.h>
#include "../../clib/include/winlib.h"
#include "../../clib/include/sysmsg.h"


PID process_to_kill;

SEMA vulture_request_sema;
SEMA vulture_wakeup_sema;

PRIVATE VIR_ADDR vulture_copy_buffer;


void send_remove_windows_to_ws( PID pid )
{
    COMMON_WINDOW_MSG	*msg;

    /* If no window server is running, just forget about it! */
    if( window_port == NO_PORT ) return;
    msg = HERMES_DATA_PTR( vulture_copy_buffer );
    msg->req = WS_REMOVE_WINDOWS;
    msg->param1 = (int) pid;
    k_message( window_port, vulture_copy_buffer, sizeof( COMMON_WINDOW_MSG ) );
}



void send_kill_to_fs( PID pid )
{
    ProcessExitedMsg	*fsmsg;
    int			nbytes;

    /* If no file server is running, just forget about it! */
    if( file_server_port == NO_PORT ) return;
    fsmsg = HERMES_DATA_PTR( vulture_copy_buffer );
    fsmsg->Size = sizeof( ProcessExitedMsg );
    fsmsg->Type = PROCESS_EXITED;
    fsmsg->pid = pid;
    k_send( file_server_port, vulture_copy_buffer,
	    sizeof( ProcessExitedMsg ), &nbytes );
}



void vulture_process( PORT_ID first_port, HANDLE handle )
{
    PID		 pid;
    PCB		 *p;
    volatile int i_flag;

    destroy_port( first_port );
    vulture_copy_buffer = k_sbrk( RECEIVE_BUFFER_SIZE );
    while( TRUE ) {
	P( &vulture_wakeup_sema );
	pid = process_to_kill;
	if( VALIDATE_PID( pid ) ) {
	    DISABLE_INTR( i_flag );
	    p = conv_pid_to_pcb( pid );
	    if( p->state == STATE_READY )

		/* Process is still running; force it to stop */
		remove_ready_queue( p );

	    /* Invalidate PCB slot by setting used-flag to false */
	    p->used = FALSE;
	    ENABLE_INTR( i_flag );

	    /* Now start cleaning up. First destroy all ports. */
	    while( p->port_list != NULL ) release_port( p->port_list );

	    /* Next, return all memory pages to the free list */
	    release_virtual_memory( p->context_cr3, p->sys_stack_base );

	    /* Finally send messages to the window- and file-server.
	     * They might have to do some cleaning up */
	    send_remove_windows_to_ws( pid );
	    send_kill_to_fs( pid );

	    /* Now the PCB-slot can be returned to the free list. */
	    DISABLE_INTR( i_flag );
	    p->next = next_free_pcb;
	    next_free_pcb = p;
	    ENABLE_INTR( i_flag );
	}
	V( &vulture_request_sema );
    }
}
