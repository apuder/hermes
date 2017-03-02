
#include <kernel.h>



ERROR k_send( PORT_ID port_id, VIR_ADDR a, int data_len, int *reply_data_len )
{
    PCB			*_pcb;
    PORT		*_port;
    volatile int	i_flag;

    DISABLE_INTR( i_flag );
    if( !VALIDATE_PORT_ID( port_id ) ) {
	ENABLE_INTR( i_flag );
	return( panic( ERR_BAD_PORT, "k_send(): Bad port" ) );
    }
    _port = &port[ port_id & BYTE ];
    _pcb = _port->pcb_index;
    if( _pcb->state == STATE_RECEIVE_BLOCKED && _port->open ) {
	/*
	 * Receiver is receive blocked. We can deliver our message
	 * immediately.
	 */
	_pcb->param_pid	     = conv_pcb_to_pid( active_pcb );
	_pcb->param_data_len = data_len;
	active_pcb->state    = STATE_REPLY_BLOCKED;

	/* Re-map the copy buffer */
	move_copy_buffer( active_pcb->context_cr3, a, /* From */
			  _pcb->context_cr3, _pcb->param_copy_buffer, /* To */
			  data_len );

	add_ready_queue( _pcb );
    } else {
	/*
	 * Receiver is busy. Get on the send blocked queue of the port.
	 */
	if( _port->blocked_first == NULL )
	    _port->blocked_first = active_pcb;
	else
	    _port->blocked_last->next_blocked = active_pcb;
	_port->blocked_last = active_pcb;
	active_pcb->next_blocked = NULL;
	active_pcb->state = STATE_SEND_BLOCKED;
	active_pcb->param_data_len = data_len;
    }
    active_pcb->param_copy_buffer = a;
    remove_ready_queue( active_pcb );
    resign();
    *reply_data_len = active_pcb->param_reply_data_len;
    ENABLE_INTR( i_flag );
    return( OK );
}
