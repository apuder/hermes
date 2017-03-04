
#include <ci_types.h>
#include <kernel.h>
#include <winlib.h>
#include <sysmsg.h>
#include <timer.h>
#include "tqueue.h"


TIMER timer_tbl[ MAX_TIMERS ];
TIMER *next_free_timer_slot;

TIMER *blocked_list;



void remove_old_client( PID pid, TIMER_MSG *msg )
{
	TIMER	*t;
	TIMER	*p;

	for( t = blocked_list, p = NULL; t != NULL; p = t, t = t->next ) {
		if( t->client_pid == pid && t->wakeup_mode == msg->wakeup_mode ) {
			if( t->wakeup_mode == WAKEUP_BY_EVENT &&
				t->timer_mask != msg->timer_mask ) continue;

			/* Found an old timer to remove */
			if( t->next != NULL )
				t->next->count += t->count;
			if( p == NULL )
				blocked_list = t->next;
			else
				p->next = t->next;
			t->next = next_free_timer_slot;
			next_free_timer_slot = t;
			return;
		}
	}
}



void enqueue_timer_client( PID pid, TIMER_MSG *msg )
{
	TIMER	*t;
	TIMER	*p1, *p2;
	int 	count;

	remove_old_client( pid, msg );
	t = next_free_timer_slot;
	if( t == NULL ) {
		shutdown( "enqueue_timer_client(): No more timers available" );
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


void init_timer_queue( void )
{
	int i;

	for( i = 0; i < MAX_TIMERS - 1; i++ )
		timer_tbl[ i ].next = &timer_tbl[ i + 1 ];
	timer_tbl[ MAX_TIMERS - 1 ].next = NULL;
	next_free_timer_slot = timer_tbl;
	blocked_list = NULL;
}
