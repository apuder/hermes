
#include <kernel.h>
#include <assert.h>


int debugger_hooked = FALSE;
int program_loaded = FALSE;
PORT_ID debugger_port;
PID program_pid;
VIR_ADDR debug_ip_address;



void k_hook_debugger( PORT_ID db_port )
{
	volatile int i_flag;

	DISABLE_INTR( i_flag );
	if( debugger_hooked )
		shutdown( "Debugger already hooked" );
	debugger_hooked = TRUE;
	program_loaded = FALSE;
	active_pcb->debugger = TRUE;
	debugger_port = db_port;
	ENABLE_INTR( i_flag );
}



void k_define_prg_to_debug( PID pid )
{
	PCB *p;

	assert( VALIDATE_PID( pid ) );
	p = conv_pid_to_pcb( pid );
	p->to_be_debugged = TRUE;
	program_pid = pid;
	program_loaded = TRUE;
}



/*
 * k_start_single_step()
 *----------------------------------------------------------------------------
 * The program to be debugged calls this function to enable single step.
 */

void k_start_single_step( void )
{
	assert( active_pcb->to_be_debugged );
	active_pcb->single_step = TRUE;
}



/*
 * k_single_step()
 *----------------------------------------------------------------------------
 * This function gets called when an INT 3 occurs. The program to be
 * debugged has to provide the IP-address. All that is to be done is
 * to save this address and to send a dummy message to the debug-port.
 * The program can process with the next instruction upon receiving
 * the reply.
 */

void k_single_step( VIR_ADDR addr )
{
	int				nbytes;
	volatile int	i_flag;

	debug_ip_address = addr;
	DISABLE_INTR( i_flag );
	if( debugger_hooked )
		k_send( debugger_port, 0, 0, &nbytes );
	ENABLE_INTR( i_flag );
}



/*
 * k_wait_for_single_step()
 *----------------------------------------------------------------------------
 * The debugger calls this function to block until the program to be
 * debugged traps into an INT 3 (single step). The result is the current
 * IP value.
 */

VIR_ADDR k_wait_for_single_step( void )
{
	PID	pid;
	int nbytes;

	k_receive( 0, &pid, &nbytes );
	assert( pid == program_pid );
	return( debug_ip_address );
}



/*
 * k_resume_single_step()
 *----------------------------------------------------------------------------
 * The debugger calls this function to resume the program to be debugged.
 * Since the program is reply-blocked, a k_reply() is sufficient.
 */

void k_resume_single_step( void )
{
	k_reply( program_pid, 0, 0 );
}


VIR_ADDR k_next_single_step( void )
{
	assert( active_pcb->debugger );
	if( k_message_pending() ) {
		/*
		 * There is a message pending; must be a pending debug trap.
		 */
		return( k_wait_for_single_step() );
	}
	k_resume_single_step();
	return( k_wait_for_single_step() );
}



/*
 * k_debugger_exited()
 *----------------------------------------------------------------------------
 * This function gets called when the debug process terminates. It is
 * necessary to call this function within the debug process context!
 */

void k_debugger_exited( void )
{
	PCB				*p;
	volatile int	i_flag;

	DISABLE_INTR( i_flag );
	assert( active_pcb->debugger );
	if( program_loaded ) {
		p = conv_pid_to_pcb( program_pid );
		p->to_be_debugged = FALSE;
		p->single_step = FALSE;
		if( k_message_pending() )
			k_wait_for_single_step();
		k_resume_single_step();
		program_loaded = FALSE;
	}
	debugger_hooked = FALSE;
	ENABLE_INTR( i_flag );
}
