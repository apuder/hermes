
#include <ci_types.h>
#include <kernel.h>
#include <winlib.h>
#include <sysmsg.h>
#include <keyb.h>
#include "event.h"
#include <assert.h>
#include "../../appl.os/window/win_serv.h"
#include "../../appl.os/window/ws_intrn.h"



PRIVATE int keyb_queue[ NUM_KEYB_EVENT_SLOTS * NUM_INTS_PER_KEYB_SLOT ];
EVENT_BUFFER keyb_event;


CLIENT_DATA	*client_hash[ MAX_CLIENT_HASH ];
CLIENT_DATA	client_data[ MAX_CLIENTS ];
CLIENT_DATA	*next_free_client_data;

WIN_QUEUE	win_queue[ MAX_WINDOW ];
WIN_EVENT	win_event[ MAX_WINDOW_EVENTS ];
WIN_EVENT	*next_free_win_event;

WID		input_focus;

PRIVATE VIR_ADDR event_copy_buffer;



PRIVATE CLIENT_DATA *compute_hash_entry( PID pid )
{
    CLIENT_DATA	*c;
    int		i;

    i = pid % MAX_CLIENT_HASH;
    /* Scan the overflow chain for this hash entry */
    for( c = client_hash[ i ]; c != NULL; c = c->next )
	if( c->pid == pid ) return( c );
    return( NULL );
}



PRIVATE void add_hash_table( CLIENT_DATA *c, PID pid )
{
    int i;

    i = pid % MAX_CLIENT_HASH;
    c->next = client_hash[ i ];
    client_hash[ i ] = c;
}



PRIVATE void remove_hash_table( PID pid )
{
    CLIENT_DATA	*c;
    int			i;

    i = pid % MAX_CLIENT_HASH;
    c = client_hash[ i ];
    if( c->pid == pid ) {
	client_hash[ i ] = c->next;
    } else {
	while( c->next != NULL ) {
	    if( c->next->pid == pid ) {
		c->next = c->next->next;
		return;
	    }
	    c = c->next;
	}
	assert( 0 );
    }
}



PRIVATE void alloc_event_queue( PID client_pid, WID wid, WID parent_wid )
{
    WIN_QUEUE	*win;
    CLIENT_DATA	*c;

    win = &win_queue[ wid & BYTE ];
    win->owner_pid	= client_pid;
    win->num_events	= 0;
    win->event_queue	= NULL;
    win->last_event	= NULL;
    win->parent_win	= &win_queue[ parent_wid & BYTE ];
    c = compute_hash_entry( client_pid );
    if( c != NULL ) {
	/* This client already owns (at least one) window */
	win->next_win				= c->first_win;
    } else {
	/* This client opens the first window */
	assert( next_free_client_data != NULL );
	c = next_free_client_data;
	next_free_client_data = next_free_client_data->next;
	add_hash_table( c, client_pid );
	win->next_win	= NULL;
	c->timer_mask	= 0;
	c->pid		= client_pid;
	c->state	= WORKING;
    }
    c->first_win = win;
}



PRIVATE void dealloc_event_queue( WID wid )
{
    WIN_QUEUE	*win;
    WIN_QUEUE	*w;
    CLIENT_DATA	*c;

    win = &win_queue[ wid & BYTE ];
    /* Clear all events that are still pending */
    if( win->event_queue != NULL ) {
	win->last_event->next = next_free_win_event;
	next_free_win_event = win->event_queue;
    }
    /* Unchain window in client's window list */
    c = compute_hash_entry( win->owner_pid );
    assert( c != NULL );
    assert( c->first_win != NULL );
    if( c->first_win == win ) {
	/* The first window has to be unchained */
	c->first_win = win->next_win;
	if( c->first_win == NULL ) {
	    /* Client owns no more windows; release client slot */
	    /* Client has to be unblocked if he is still waiting */
	    assert( c->state == WORKING );
	    remove_hash_table( win->owner_pid );
	    c->next = next_free_client_data;
	    next_free_client_data = c;
	}
    } else {
	w = c->first_win;
	while( w != NULL ) {
	    if( w->next_win == win ) {
		w->next_win = win->next_win;
		return;
	    }
	    w = w->next_win;
	}
	assert( 0 );
    }
}



PRIVATE void enqueue_new_window_event( EVENT_INFO_TYPE *e )
{
    WIN_EVENT		*event_slot;
    EVENT_INFO_TYPE	*event_msg;
    WID			wid;
    WIN_QUEUE		*win;
    CLIENT_DATA		*c;

    wid = ( e->event_type == EV_SET_FOCUS ) ?
	e->fe.old_window_id :
	e->fe.window_id;
    win = &win_queue[ wid & BYTE ];
    c = compute_hash_entry( win->owner_pid );
    assert( c != NULL );
    if( c->state == WAITING ) {
	event_msg = HERMES_DATA_PTR( event_copy_buffer );
	*event_msg = *e;
	k_reply( win->owner_pid, event_copy_buffer, sizeof( EVENT_INFO_TYPE ) );
	c->state = WORKING;
	return;
    }
    if( win->num_events == MAX_SAVE_EVENTS ) return;
    if( next_free_win_event == NULL ) return;
    event_slot = next_free_win_event;
    next_free_win_event = next_free_win_event->next;
    win->num_events++;
    event_slot->window_event = *e;
    event_slot->next = NULL;
    if( win->last_event == NULL ) {
	win->event_queue = event_slot;
	win->last_event  = event_slot;
    } else {
	win->last_event->next = event_slot;
    }
}



PRIVATE void enqueue_new_timer_event( ENQUEUE_TIMER_MSG *msg )
{
    EVENT_INFO_TYPE	*event_msg;
    CLIENT_DATA		*c;
    unsigned		timer_mask;

    c = compute_hash_entry( msg->client_pid );

    /* Discard timer event if client has died! */
    if( c == NULL ) return;
    timer_mask = msg->timer_mask;
    if( c->state == WAITING ) {
	event_msg = HERMES_DATA_PTR( event_copy_buffer );
	event_msg->te.event_type = EV_WAKEUP;
	event_msg->te.timer_mask = timer_mask;
	k_reply( msg->client_pid, event_copy_buffer, sizeof( EVENT_INFO_TYPE ) );
	c->state = WORKING;
	return;
    }
    c->timer_mask |= timer_mask;
}



PRIVATE void dequeue_window_event( PID pid )
{
    WIN_QUEUE		*win;
    EVENT_INFO_TYPE	*e;
    WIN_EVENT		*event_slot;
    CLIENT_DATA		*c;

    e = HERMES_DATA_PTR( event_copy_buffer );
    c = compute_hash_entry( pid );
    assert( c != NULL );
    if( c->timer_mask != 0 ) {
	e->te.event_type = EV_WAKEUP;
	e->te.timer_mask = c->timer_mask;
	k_reply( pid, event_copy_buffer, sizeof( EVENT_INFO_TYPE ) );
	c->timer_mask = 0;
	return;
    }
    win = c->first_win;
    /*
      assert( win != NULL );
    */
    while( win != NULL ) {
	if( win->event_queue != NULL ) {
	    *e = win->event_queue->window_event;
	    k_reply( pid, event_copy_buffer, sizeof( EVENT_INFO_TYPE ) );
	    event_slot = win->event_queue;
	    win->event_queue = event_slot->next;
	    if( event_slot->next == NULL )
		win->last_event = NULL;
	    event_slot->next = next_free_win_event;
	    next_free_win_event = event_slot;
	    win->num_events--;
	    return;
	}
	win = win->next_win;
    }
    c->state = WAITING;
}



PRIVATE void init_event_manager( void )
{
    int i;

    /*
      assert( MAX_CLIENT_HASH >= MAX_WINDOW );
    */
    for( i = 0; i < MAX_CLIENT_HASH; i++ )
	client_hash[ i ] = NULL;

    for( i = 0; i < MAX_CLIENTS - 1; i++ )
	client_data[ i ].next = &client_data[ i + 1 ];
    client_data[ MAX_CLIENTS - 1 ].next = NULL;
    next_free_client_data = client_data;

    for( i = 0; i < MAX_WINDOW_EVENTS - 1; i++ )
	win_event[ i ].next = &win_event[ i + 1 ];
    win_event[ MAX_WINDOW_EVENTS - 1 ].next = NULL;
    next_free_win_event = win_event;

    event_copy_buffer = k_sbrk( RECEIVE_BUFFER_SIZE );
}



void event_manager( PORT_ID first_port, PORT_ID parent_port )
{
    ALLOC_EVENT_MSG	*msg;
    DEALLOC_EVENT_MSG	*dealloc_msg;
    ENQUEUE_EVENT_MSG	*e;
    PID			pid;
    PORT_ID		dummy_port;
    int			data_len;
    EVENT_INFO_TYPE	key_event;
    unsigned		new_key;

    init_event_manager();
    dealloc_msg = HERMES_DATA_PTR( event_copy_buffer );
    msg = HERMES_DATA_PTR( event_copy_buffer );
    e = HERMES_DATA_PTR( event_copy_buffer );
    key_event.event_type = EV_KEY_PRESSED;
    input_focus = -1;

    init_event_queue( &keyb_event, NUM_KEYB_EVENT_SLOTS,
		      NUM_INTS_PER_KEYB_SLOT,
		      keyb_queue );
    connect_port_to_event_queue( event_port, &keyb_event );

    keyb_init( event_port );
    
    while( TRUE ) {
	k_receive( event_copy_buffer, &pid, &data_len );
	if( pid == KERNEL_PID ) {
	    dequeue_event( &keyb_event, &new_key );
	    if( input_focus == -1 ) continue;
	    key_event.ke.window_id = input_focus;
	    key_event.ke.key = (u_short) new_key;
	    enqueue_new_window_event( &key_event );
	    continue;
	}
	switch( msg->req ) {
	case ALLOC_EVENT_QUEUE:
	    input_focus = msg->input_focus;
	    alloc_event_queue( msg->pid, msg->wid, msg->parent_wid );
	    k_reply( pid, 0, 0 );
	    break;
	case DEALLOC_EVENT_QUEUE:
	    input_focus = dealloc_msg->input_focus;
	    dealloc_event_queue( dealloc_msg->wid );
	    k_reply( pid, 0, 0 );
	    break;
	case ENQUEUE_WINDOW_EVENT:
	    input_focus = e->new_input_focus;
	    enqueue_new_window_event( &e->event );
	    break;
	case DEQUEUE_WINDOW_EVENT:
	    dequeue_window_event( pid );
	    break;
	case ENQUEUE_TIMER_EVENT:
	    enqueue_new_timer_event( (ENQUEUE_TIMER_MSG*) msg );
	    break;
	default:
	    shutdown( "event_manager(): Bad req" );
	}
    }
}
