
#include <kernel.h>
#include <assert.h>
#include <kstdarg.h>



void init_event_queue( EVENT_BUFFER *e, int num_event_slots,
										int num_ints_per_slot,
										int *queue )
{
	assert( num_event_slots > 0 );
	assert( num_ints_per_slot > 0 );
	e->cookie = MAGIC_EVENT_COOKIE;
	e->queue_empty = TRUE;
	e->end_of_event_queue = queue + num_event_slots * num_ints_per_slot;
	e->num_ints_per_slot = num_ints_per_slot;
	e->head = queue;
	e->tail = queue;
	e->queue = queue;
}



void queue_event( EVENT_BUFFER *e, ... )
{
	va_list			p;
	int				*ap;
	int				i;
	volatile int	i_flag;

	DISABLE_INTR( i_flag );
	va_start( p, e );
	if( e->queue_empty ) {
		e->queue_empty = FALSE;
	} else {
		if( ( e->head += e->num_ints_per_slot ) == e->end_of_event_queue )
			e->head = e->queue;
		assert( e->head != e->tail );
	}
	ap = e->head;
	for( i = 0; i < e->num_ints_per_slot; i++ )
		*ap++ = va_arg( p, int );
	va_end( p );
	ENABLE_INTR( i_flag );
}



void queue_event_with_overwrite( EVENT_BUFFER *e, ... )
{
	va_list			p;
	int				*ap;
	int				i;
	volatile int	i_flag;

	DISABLE_INTR( i_flag );
	va_start( p, e );
	ap = e->head;
	for( i = 0; i < e->num_ints_per_slot; i++ )
		*ap++ = va_arg( p, int );
	e->queue_empty = FALSE;
	va_end( p );
	ENABLE_INTR( i_flag );
}



void dequeue_event( EVENT_BUFFER *e, ... )
{
	va_list			p;
	int				*ap;
	int				*arg;
	int				i;
	volatile int	i_flag;

	DISABLE_INTR( i_flag );
	va_start( p, e );
	assert( !e->queue_empty );
	ap = e->tail;
	for( i = 0; i < e->num_ints_per_slot; i++ ) {
		arg = va_arg( p, int * );
		*arg = *ap++;
	}
	if( e->head == e->tail ) {
		e->queue_empty = TRUE;
	} else {
		if( ( e->tail += e->num_ints_per_slot ) == e->end_of_event_queue )
			e->tail = e->queue;
	}
	va_end( p );
	ENABLE_INTR( i_flag );
}



void raise_event( int port_id )
{
	PCB				*_pcb;
	PORT			*_port;
	volatile int	i_flag;

	DISABLE_INTR( i_flag );
	if( !VALIDATE_PORT_ID( port_id ) ) {
		ENABLE_INTR( i_flag );
		shutdown( "raise_event(): Bad port" );
	}
	_port = &port[ port_id & BYTE ];
	if( _port->event_queue == NULL ) {
		ENABLE_INTR( i_flag );
		shutdown( "raise_event(): Port not connected to event queue" );
	}
	_pcb = _port->pcb_index;
	if( _pcb->state == STATE_RECEIVE_BLOCKED && _port->open ) {

		_pcb->param_pid		 = KERNEL_PID;
		_pcb->param_data_len = 0;
		add_ready_queue( _pcb );
	}
	ENABLE_INTR( i_flag );
}
