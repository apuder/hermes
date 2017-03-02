
#include <kernel.h>
#include <assert.h>
#include "io.h"


PORT port[ MAX_PORTS ];

PRIVATE PORT *next_free_port;



/*
 * create_port()
 *----------------------------------------------------------------------------
 * A new port is allocated for the calling process. The new port id is
 * returned in p.
 */

ERROR create_port( PORT_ID *p )
{
	return( create_new_port( active_pcb, p ) );
}



/*
 * create_new_port()
 *----------------------------------------------------------------------------
 * This function creates a new port for the process pointed to by _pcb.
 */

ERROR create_new_port( PCB *_pcb, PORT_ID *_p )
{
	PORT			*p;
	volatile int	i_flag;

	DISABLE_INTR( i_flag );
	if( next_free_port == NULL ) {
		ENABLE_INTR( i_flag );
		return( panic( ERR_NO_RESOURCES, "create_new_port(): PORT full" ) );
	}
	p = next_free_port;
	next_free_port = p->next;
	p->used			= TRUE;
	p->pcb_index	= _pcb;
	p->blocked_first= NULL;
	p->blocked_last	= NULL;
	p->event_queue	= NULL;
	p->open			= TRUE;
	if( _pcb->port_list == NULL ) {
		p->next			= NULL;
		_pcb->port_list	= p;
		_pcb->port_scan	= p;
	} else {
		p->next = _pcb->port_list;
		_pcb->port_list = p;
	}
	ENABLE_INTR( i_flag );
	if( ++p->seq == 0x80 )
		p->seq = 0;
	*_p = ( my_node_id << 16 ) | ( p->seq << 8 ) | ( p - port );
	return( OK );
}



/*
 * destroy_port()
 *----------------------------------------------------------------------------
 * This function destroys a port. The main work is done by release_port().
 */

ERROR destroy_port( PORT_ID port_id )
{
	if( !VALIDATE_PORT_ID( port_id ) )
		return( panic( ERR_BAD_PORT, "destroy_port(): Bad port" ) );
	return( release_port( &port[ port_id & BYTE ] ) );
}



/*
 * release_port()
 *----------------------------------------------------------------------------
 * This function releases a port slot and returns is to the free list.
 * Various things have to done. First, the used flag is set to FALSE. This
 * will invalidate the port. Any VALIDATE_PORT_ID will therefore fail
 * when referencing this port. Second, the send blocked list must be
 * scaned for pending messages. All those clients must be "awakened"
 * with an error. Finally the port slot is returned to the free list.
 */

ERROR release_port( PORT *_port )
{
	PORT			*_port1;
	PCB				*_pcb;
	volatile int	i_flag;

	DISABLE_INTR( i_flag );

	/* Invalidate this slot */
	assert( _port->used );
	_port->used = FALSE;

	/* Unchain this slot from the owner's port list */
	_pcb = _port->pcb_index;
	if( _pcb->port_list == _port ) {

		/* First port is the list is the one */
		_pcb->port_list = _port->next;
	} else {

		/* We have to scan the port list */
		_port1 = _pcb->port_list;
		while( _port1->next != _port ) {
			_port1 = _port1->next;
			assert( _port1 != NULL );
		}
		_port1->next = _port->next;
	}
	/* Reset port_scan (for k_receive) to a proper value */
	_pcb->port_scan = _pcb->port_list;
	ENABLE_INTR( i_flag );

	/* Look for send-blocked clients on this port */
	assert( _port->blocked_first == NULL );

	/* Return port slot to free list */
	DISABLE_INTR( i_flag );
	_port->next = next_free_port;
	next_free_port = _port;
	ENABLE_INTR( i_flag );
	return( OK );
}



/*
 * open_port()
 *----------------------------------------------------------------------------
 * A previously closed port is opened again. Messages pending at a closed
 * port will not get ignored by receive().
 */

ERROR open_port( PORT_ID port_id )
{
	volatile int i_flag;

	DISABLE_INTR( i_flag );
	if( !VALIDATE_PORT_ID( port_id ) ) {
		ENABLE_INTR( i_flag );
		return( panic( ERR_BAD_PORT, "open_port(): Bad port" ) );
	}
	port[ port_id & BYTE ].open = TRUE;
	ENABLE_INTR( i_flag );
	return( OK );
}



/*
 * close_port()
 *----------------------------------------------------------------------------
 * A port is closed.
 */

ERROR close_port( PORT_ID port_id )
{
	volatile int i_flag;

	DISABLE_INTR( i_flag );
	if( !VALIDATE_PORT_ID( port_id ) ) {
		ENABLE_INTR( i_flag );
		return( panic( ERR_BAD_PORT, "close_port(): Bad port" ) );
	}
	port[ port_id & BYTE ].open = FALSE;
	ENABLE_INTR( i_flag );
	return( OK );
}



/*
 * connect_port_to_event_queue()
 *----------------------------------------------------------------------------
 * A port is connected to an event queue. receive() automatically checks
 * if there are any events pending on the port before looking for other
 * messages. So events have higher priority than ordinary messages.
 * One port can only be connected to one event queue.
 */

ERROR connect_port_to_event_queue( PORT_ID port_id, EVENT_BUFFER *e )
{
	volatile int i_flag;

	DISABLE_INTR( i_flag );
	if( !VALIDATE_PORT_ID( port_id ) ) {
		ENABLE_INTR( i_flag );
		return( panic( ERR_BAD_PORT,
					 "connect_port_to_event_queue(): Bad port" ) );
	}
	if( port[ port_id & BYTE ].event_queue != NULL ) {
		ENABLE_INTR( i_flag );
		return( panic( ERR_EVENT_QUEUE_BUSY,
					"connect_port_to_event_queue(): Already connected" ) );
	}
	if( e->cookie != MAGIC_EVENT_COOKIE ) {
		ENABLE_INTR( i_flag );
		return( panic( ERR_EVENT_QUEUE_NOT_INIT,
					"connect_port_to_event_queue(): Queue not initialized" ) );
	}
	port[ port_id & BYTE ].event_queue = e;
	ENABLE_INTR( i_flag );
	return( OK );
}



void init_ports( void )
{
	int i;

	next_free_port = port;
	for( i = 0; i < MAX_PORTS - 1; i++ ) {
		port[ i ].used = FALSE;
		port[ i ].seq  = 0;
		port[ i ].next = &port[ i + 1 ];
	}
	port[ MAX_PORTS - 1 ].used = FALSE;
	port[ MAX_PORTS - 1 ].seq  = 0;
	port[ MAX_PORTS - 1 ].next = NULL;
	printf( "Allocated %d ports.\n", MAX_PORTS );
}
