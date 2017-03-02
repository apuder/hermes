
#include <kernel.h>



void init_sema( SEMA *sema, int count )
{
	sema->count = count;
	sema->first = NULL;
	sema->last  = NULL;
}


void P( SEMA *sema )
{
	volatile int i_flag;

	DISABLE_INTR( i_flag );
	if( --sema->count < 0 || sema->first != NULL ) {
		remove_ready_queue( active_pcb );
		active_pcb->state = STATE_SEMA_BLOCKED;
		active_pcb->next_blocked = NULL;
		if( sema->first == NULL )
			sema->first = active_pcb;
		else
			sema->last->next_blocked = active_pcb;
		sema->last = active_pcb;
		resign();
	}
	ENABLE_INTR( i_flag );
}



void V( SEMA *sema )
{
	PCB				*unblocked_proc;
	volatile int	i_flag;

	DISABLE_INTR( i_flag );
	if( ++sema->count <= 0 && sema->first != NULL ) {
		unblocked_proc = sema->first;
		sema->first = sema->first->next_blocked;
		if( sema->first == NULL )
			sema->last = NULL;
		add_ready_queue( unblocked_proc );
	}
	ENABLE_INTR( i_flag );
}
