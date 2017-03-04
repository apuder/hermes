
#include <ci_types.h>
#include <kernel.h>
#include <winlib.h>
#include <sysmsg.h>
#include <timer.h>
#include "tqueue.h"


int seconds_since_1970;


PRIVATE EVENT_BUFFER timer_event;
PRIVATE int timer_event_queue[ 1 ];
PRIVATE int timer_ticks_since_boot = 0;


PRIVATE VIR_ADDR timer_copy_buffer;


PRIVATE void timer_notify( PORT_ID first_port, HANDLE parent_port )
{
	int num_timer_ticks = 0;

	destroy_port( first_port );
	register_isr( TIMER_INTR_NO );
	while( TRUE ) {
		await_event();
		num_timer_ticks++;
		if( num_timer_ticks % 18 == 0 ) seconds_since_1970++;
		/*
		 * Timer process only needs to be woken up, if someone
		 * is on the blocked list.
		 */
		if( blocked_list != NULL ) {
			queue_event_with_overwrite( &timer_event, num_timer_ticks );
			raise_event( (PORT_ID) parent_port );
		}
	}
}


#if 0
PRIVATE void enqueue_client( PID pid, TIMER_MSG *msg )
{
	TIMER	*t;
	TIMER	*p1, *p2;
	int 	count;

	t = next_free_timer_slot;
	if( t == NULL ) {
		shutdown( "enqueue_client(): No more timers available" );
	}
	next_free_timer_slot = t->next;
	t->client_pid	= pid;
	t->wakeup_mode	= msg->wakeup_mode;
	t->func_to_call = msg->func_to_call;
	t->timer_mask	= msg->timer_mask;
	count			= msg->count;
	if( blocked_list == NULL ) {

		/* No-one blocked so far */
		t->next 	 = NULL;
		t->last 	 = NULL;
		blocked_list = t;
	} else {
		p1 = NULL;
		p2 = blocked_list;
		while( p2 != NULL && count >= p2->count ) {
			count -= p2->count;
			p1 = p2;
			p2 = p2->next;
		}
		if( p1 == NULL ) {
			t->next = blocked_list;
			t->last = NULL;
			blocked_list->last = t;
			blocked_list = t;
		} else {
			t->next = p2;
			t->last = p1;
			p1->next = t;
			if( p2 != NULL ) p2->last = t;
		}
		if( p2 != NULL ) p2->count -= count;
	}
	t->count = count;
}
#endif



PRIVATE void wake_up_client( TIMER *t )
{
	ENQUEUE_TIMER_MSG	*msg;

	switch( t->wakeup_mode ) {
		case WAKEUP_BY_REPLY:
			/* Wakeup client by sending the reply */
			k_reply( t->client_pid, 0, 0 );
			break;
		case WAKEUP_BY_CALL:
			(*t->func_to_call)();
			break;
		case WAKEUP_BY_EVENT:
			msg = HERMES_DATA_PTR( timer_copy_buffer );
			msg->req = ENQUEUE_TIMER_EVENT;
			msg->timer_mask = t->timer_mask;
			msg->client_pid = t->client_pid;
			k_message( event_port, timer_copy_buffer,
							sizeof( ENQUEUE_TIMER_MSG ) );
			break;
	}
}



PRIVATE void timer_tick( void )
{
	int		new_timer_ticks;
	int		delta_ticks;
	TIMER	*t;

	dequeue_event( &timer_event, &new_timer_ticks );
	delta_ticks = new_timer_ticks - timer_ticks_since_boot;
	timer_ticks_since_boot = new_timer_ticks;

	/* Return if nobody is blocked */
	if( blocked_list == NULL ) return;

	while( blocked_list != NULL && blocked_list->count - delta_ticks <= 0 ) {

		/* Timer expired, wakeup client */
		t = blocked_list;
		blocked_list = t->next;
		wake_up_client( t );
		delta_ticks -= t->count;
		t->next = next_free_timer_slot;
		next_free_timer_slot = t;
		if( blocked_list != NULL ) blocked_list->last = NULL;
	}
	if( blocked_list != NULL ) blocked_list->count -= delta_ticks;
}



PRIVATE void init_timer( void )
{
	unsigned		count;
	unsigned char	low_byte, high_byte;
	volatile int	i_flag;

	timer_copy_buffer = k_sbrk( RECEIVE_BUFFER_SIZE );

	seconds_since_1970 = get_global_param( GLOBAL_PARAM_NUM_SECS );

/*
	DISABLE_INTR( i_flag );
	count = IBM_FREQ / HZ;
	low_byte = count & BYTE;
	high_byte = ( count >> 8 ) & BYTE;
	outportb( TIMER_MODE, SQUARE_WAVE );
	outportb( TIMER0, low_byte );
	outportb( TIMER0, high_byte );
	ENABLE_INTR( i_flag );
*/
}



void timer_manager( PORT_ID first_port, HANDLE parent_port )
{
	TIMER_MSG	*msg;
	PID 		pid;
	PORT_ID 	dummy_port;
	int 		data_len;

	init_timer();
	init_timer_queue();
	init_event_queue( &timer_event, 1, 1, timer_event_queue );
	connect_port_to_event_queue( timer_port, &timer_event );
	msg = HERMES_DATA_PTR( timer_copy_buffer );
	if( create( timer_notify, PRIO_TIMER_NOTIFY, "Timer notifier",
				&pid, &dummy_port, timer_port ) )
		shutdown( "timer_manager(): Couldn't create notifier" );
	while( TRUE ) {
		k_receive( timer_copy_buffer, &pid, &data_len );
		if( pid == KERNEL_PID ) {
			timer_tick();
			continue;
		}
		enqueue_timer_client( pid, msg );
	}
}
