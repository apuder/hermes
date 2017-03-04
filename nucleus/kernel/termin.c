
#include <kernel.h>
#include <vulture.h>



void k_suicide( void )
{
	k_terminate( conv_pcb_to_pid( active_pcb ) );
	k_suspend();
}



ERROR k_terminate( PID pid )
{
	if( !VALIDATE_PID( pid ) )
		return( panic( ERR_BAD_PID, "k_terminate(): Bad PID" ) );
	P( &vulture_request_sema );
	process_to_kill = pid;
	V( &vulture_wakeup_sema );
	return( OK );
}
