
#include <kernel.h>



ERROR k_message( PORT_ID port_id, VIR_ADDR a, int data_len )
{
    PCB 		*_pcb;
    PORT		*_port;
    volatile int	i_flag;

    DISABLE_INTR( i_flag );
    if( !VALIDATE_PORT_ID( port_id ) ) {
	ENABLE_INTR( i_flag );
	return( panic( ERR_BAD_PORT, "k_message(): Bad port" ) );
    }
    _port = &port[ port_id & BYTE ];
    _pcb = _port->pcb_index;
    if( _pcb->state == STATE_RECEIVE_BLOCKED && _port->open ) {

	/* Re-map the copy buffer */
	move_copy_buffer( active_pcb->context_cr3,
			  a,
			  _pcb->context_cr3,
			  _pcb->param_copy_buffer,
			  data_len );

	_pcb->param_pid 	 = conv_pcb_to_pid( active_pcb );
	_pcb->param_data_len = data_len;
	add_ready_queue( _pcb );
    } else {
	remove_ready_queue( active_pcb );
	if( _port->blocked_first == NULL )
	    _port->blocked_first = active_pcb;
	else
	    _port->blocked_last->next_blocked = active_pcb;
	_port->blocked_last = active_pcb;
	active_pcb->next_blocked = NULL;
	active_pcb->state = STATE_MESSAGE_BLOCKED;
	active_pcb->param_copy_buffer = a;
	active_pcb->param_data_len = data_len;
	resign();
    }
    ENABLE_INTR( i_flag );
    return( OK );
}
