
#include <kernel.h>



ERROR k_reply( PID pid, VIR_ADDR a, int reply_data_len )
{
	PCB				*p;
	volatile int	i_flag;

	DISABLE_INTR( i_flag );
	if( !VALIDATE_PID( pid ) ) {
		ENABLE_INTR( i_flag );
		return( panic( ERR_BAD_PID, "k_reply(): Bad PID" ) );
	}
	p = conv_pid_to_pcb( pid );
	if( p->state != STATE_REPLY_BLOCKED ) {
		ENABLE_INTR( i_flag );
		return( panic( ERR_NOT_REPLY_BLOCKED, "k_reply(): Not reply blocked" ) );
	}
	move_copy_buffer( active_pcb->context_cr3,
					  a,
					  p->context_cr3,
					  p->param_copy_buffer,
					  reply_data_len );
	p->param_reply_data_len = reply_data_len;
	add_ready_queue( p );
	ENABLE_INTR( i_flag );
	return( OK );
}
