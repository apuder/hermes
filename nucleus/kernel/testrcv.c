
#include <kernel.h>



BOOL k_message_pending( void )
{
	PORT			*scan;
	PORT			*stop_scan;
	volatile int	i_flag;

	DISABLE_INTR( i_flag );
	scan = stop_scan = active_pcb->port_scan;
	do {
		if( scan->event_queue != NULL &&
				!scan->event_queue->queue_empty ) {

			/* There are events pending */
			ENABLE_INTR( i_flag );
			return( TRUE );
		}
		if( scan->blocked_first != NULL && scan->open ) {
			ENABLE_INTR( i_flag );
			return( TRUE );
		}
		if( ( scan = scan->next ) == NULL )
			scan = active_pcb->port_list;
	} while( scan != stop_scan );

	/* No messages pending */
	ENABLE_INTR( i_flag );
	return( FALSE );
}
