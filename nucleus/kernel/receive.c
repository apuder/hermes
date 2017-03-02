
#include <kernel.h>



ERROR k_receive( VIR_ADDR a, PID *pid, int *data_len )
{
	PCB 			*deliver_pcb;
	PORT			*scan;
	PORT			*stop_scan;
	volatile int	i_flag;

	DISABLE_INTR( i_flag );
	if( ( active_pcb->port_scan = active_pcb->port_scan->next ) == NULL )
		active_pcb->port_scan = active_pcb->port_list;
	scan = stop_scan = active_pcb->port_scan;
	do {
		if( scan->event_queue != NULL &&
				!scan->event_queue->queue_empty ) {

			/* There are events pending */
			*pid = KERNEL_PID;
			ENABLE_INTR( i_flag );
			return( OK );
		}
		if( scan->blocked_first != NULL && scan->open ) {
			deliver_pcb = scan->blocked_first;
			*pid = conv_pcb_to_pid( deliver_pcb );
			if( ( scan->blocked_first = scan->blocked_first->next_blocked )
						== NULL ) scan->blocked_last = NULL;

			/* Re-map the copy buffer */
			move_copy_buffer( deliver_pcb->context_cr3,
							  deliver_pcb->param_copy_buffer,
							  active_pcb->context_cr3,
							  a,
							  deliver_pcb->param_data_len );

			*data_len = deliver_pcb->param_data_len;
			if( deliver_pcb->state == STATE_MESSAGE_BLOCKED ) {
				add_ready_queue( deliver_pcb );
				ENABLE_INTR( i_flag );
				return( OK );
			} else if( deliver_pcb->state == STATE_SEND_BLOCKED ) {
				deliver_pcb->state = STATE_REPLY_BLOCKED;
				ENABLE_INTR( i_flag );
				return( OK );
			}
		}
		if( ( scan = scan->next ) == NULL )
			scan = active_pcb->port_list;
	} while( scan != stop_scan );

	/* No messages pending */
	remove_ready_queue( active_pcb );
	active_pcb->param_copy_buffer = a;
	active_pcb->state = STATE_RECEIVE_BLOCKED;
	resign();
	*pid = active_pcb->param_pid;
	*data_len = active_pcb->param_data_len;
	ENABLE_INTR( i_flag );
	return( OK );
}
