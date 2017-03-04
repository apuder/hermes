
#include <kernel.h>


void k_suspend( void )
{
	volatile int	i_flag;

	DISABLE_INTR( i_flag );
	active_pcb->state = STATE_SUSPENDED;
	remove_ready_queue( active_pcb );
	resign();
	ENABLE_INTR( i_flag );
}



ERROR k_wakeup( PID pid )
{
	PCB				*p;
	volatile int	i_flag;

	DISABLE_INTR( i_flag );
	if( !VALIDATE_PID( pid ) ) {
		ENABLE_INTR( i_flag );
		return( panic( ERR_BAD_PID, "k_wakeup(): Bad PID" ) );
	}
	p = &pcb[ pid & BYTE ];
	if( p->state != STATE_SUSPENDED ) {
		ENABLE_INTR( i_flag );
		return( panic( ERR_NOT_SUSPENDED, "k_wakeup(): Not suspended" ) );
	}
	add_ready_queue( p );
	ENABLE_INTR( i_flag );
	return( OK );
}
