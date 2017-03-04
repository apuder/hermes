
#include <ci_types.h>
#include <cinclude.h>
#include <winlib.h>
#include <sysmsg.h>
#include "win_serv.h"
#include "ws_intrn.h"


void *win_copy_buffer;



void send_dealloc_to_event_manager( WID wid )
{
    DEALLOC_EVENT_MSG	*dealloc_msg;
    int					data_len;

    dealloc_msg = win_copy_buffer;
    dealloc_msg->req			= DEALLOC_EVENT_QUEUE;
    dealloc_msg->wid			= wid;
    dealloc_msg->input_focus	= ws_actwid();
    EventManagerSend( win_copy_buffer, sizeof( DEALLOC_EVENT_MSG ), &data_len );
}



void open_wnd( PID pid, WINDOW_MSG *msg )
{
    ALLOC_EVENT_MSG	*alloc_msg;
    WID       		wid;
    WID				parent_wid;
    int				data_len;
    int				err;
    s_short			x, y, dx, dy;

    parent_wid = msg->new_window.father_window;
    msg->new_window.pid = pid;
    err = ws_open_window( &msg->new_window, &wid, &x, &y, &dx, &dy );
    if( err == 0 ) {
	alloc_msg = win_copy_buffer;
	alloc_msg->req			= ALLOC_EVENT_QUEUE;
	alloc_msg->pid			= pid;
	alloc_msg->wid			= wid;
	alloc_msg->input_focus	= ws_actwid();
	alloc_msg->parent_wid	= parent_wid;
	EventManagerSend( win_copy_buffer,
			  sizeof( ALLOC_EVENT_MSG ), &data_len );
    }
    msg->wid = wid;
    msg->err = err;
    msg->new_window.x = x;
    msg->new_window.y = y;
    msg->new_window.dx = dx;
    msg->new_window.dy = dy;
    Reply( pid, win_copy_buffer, sizeof( WINDOW_MSG ) );
}



void print_wnd( PID pid, PRINT_WINDOW_MSG *msg )
{
    COMMON_WINDOW_MSG   *cmsg;
    ERR_CODE            rc;

    cmsg = win_copy_buffer;
    rc = ws_print( msg->wid, (u_char *) &msg->string );
    cmsg->rc = rc;
    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
}



void main( void )
{
    WINDOW_MSG		*msg;
    ALLOC_EVENT_MSG	*alloc_msg;
    COMMON_WINDOW_MSG	*cmsg;
    PRINT_WINDOW_MSG	*pmsg;
    DEF_ICON_MSG	*imsg;
    WINDOW_DEF_MSG	*dmsg;
    ENQUEUE_EVENT_MSG	*e;
    CLIPS_TYPE		cp;
    int			data_len;
    int			mouse_px;
    int			mouse_py;
    int			mouse_button;
    PID			pid;
    PORT_ID		mouse_port;
    u_char		dummy;
    MOUSE_INFO_TYPE	mi;
    ERR_CODE		ret;
    u_short		background = 32 + ( WL_WHITE << 8 );

    RegisterWindowServer( FirstPortId );

    ws_init( MAX_WINDOW );
    ws_set_background( &background, 1, 1 );

    win_copy_buffer	= GetCopyBuffer();
    msg			= win_copy_buffer;
    dmsg		= win_copy_buffer;
    imsg		= win_copy_buffer;
    alloc_msg		= win_copy_buffer;
    cmsg		= win_copy_buffer;
    e			= win_copy_buffer;
    pmsg		= win_copy_buffer;

    CreatePort( &mouse_port );
    k_p1 = mouse_port;
    k_p2 = 0; /* Flag for MS-Mouse */
    kernel_trap( K_CREATE_MOUSE_PROCESS );

    alloc_msg->req		= ALLOC_EVENT_QUEUE;
    alloc_msg->pid		= 0;
    alloc_msg->wid		= ROOT_WINDOW;
    alloc_msg->input_focus	= ws_actwid();
    alloc_msg->parent_wid	= ROOT_WINDOW;
    EventManagerSend( win_copy_buffer, sizeof( ALLOC_EVENT_MSG ), &data_len );
    while( TRUE ) {
	Receive( win_copy_buffer, &pid, &data_len );
	if( pid == KERNEL_PID ) {
	    kernel_trap( K_GET_MOUSE_EVENT );
	    mouse_px = k_r1;
	    mouse_py = k_r2;
	    mouse_button = k_r3;
	    mi.mouse_dx = mouse_px; mi.mouse_dy = mouse_py;
	    mi.button = mouse_button;
	    wm_process_mouse_event( &mi, &e->event, &dummy );
	    if( e->event.event_type != EV_NO_EVENT ) {
		e->req = ENQUEUE_WINDOW_EVENT;
		e->new_input_focus = ws_actwid();
		EventManagerMessage( win_copy_buffer,
				     sizeof( ENQUEUE_EVENT_MSG ) );
	    }
	    continue;
	}
	switch( msg->req ) {
	case OPEN_WINDOW:
	    open_wnd( pid, msg );
	    break;
	case PRINT_WINDOW:
	    print_wnd( pid, (PRINT_WINDOW_MSG *) msg );
	    break;
	case CLS_WINDOW:
	    cmsg->rc = ws_clrscr( cmsg->wid );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case GOTOXY_WINDOW:
	    cmsg->rc = ws_gotoxy( cmsg->wid, cmsg->param1, cmsg->param2 );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_GET_TEXTATTR:
	    cmsg->rc = ws_get_textattr( cmsg->wid, &cmsg->param1 );
	    Reply( pid, win_copy_buffer, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_SET_TEXTATTR:
	    cmsg->rc = ws_set_textattr( cmsg->wid, cmsg->param1 );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_CURSOR_ON:
	    cmsg->rc = ws_cursor_on( cmsg->wid );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_CURSOR_OFF:
	    cmsg->rc = ws_cursor_off( cmsg->wid );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case CLOSE_WINDOW:
	    cmsg->rc = ws_close_window( cmsg->wid, DISPLAY );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_DUP_WINDOW:
	    cmsg->rc = ws_dup_window( (WID*) &cmsg->wid );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_DEFINE_BORDER:
	    cmsg->rc = ws_define_border( pmsg->wid, &pmsg->string );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_MODIFY_OPTIONS:
	    cmsg->rc = ws_modify_options( cmsg->wid, cmsg->param1,
					  cmsg->param2 );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_RAISE_WINDOW:
	    cmsg->rc = ws_raise_window( cmsg->wid, DISPLAY );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_DROP_WINDOW:
	    cmsg->rc = ws_drop_window( cmsg->wid );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_SET_BACKGROUND:
	    ws_set_background( (u_short*) &cmsg->param3, cmsg->param1,
			       cmsg->param2 );
	    Reply( pid, 0, 0 );
	    break;
	case WS_MOVE_WINDOW:
	    cmsg->rc = ws_move_window( cmsg->wid, cmsg->param1,
				       cmsg->param2, DISPLAY );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_RESIZE_WINDOW:
	    cmsg->rc = ws_resize_window( cmsg->wid, cmsg->param1, cmsg->param2 );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_ZOOM_WINDOW:
	    cmsg->rc = ws_zoom_window( cmsg->wid );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_UNZOOM_WINDOW:
	    cmsg->rc = ws_unzoom_window( cmsg->wid );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_ICON_WINDOW:
	    cmsg->rc = ws_iconify_window( cmsg->wid );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_DEICON_WINDOW:
	    cmsg->rc = ws_deiconify_window( cmsg->wid );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_DEF_ICON:
	    cmsg->rc = ws_define_icon( imsg->wid, imsg->x, imsg->y,
				       imsg->col, &imsg->title,
				       imsg->no_def );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_DEF_ICON_POS:
	    cmsg->rc = ws_def_icon_pos( cmsg->wid, cmsg->param1, cmsg->param2 );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_SET_VSCROLL:
	    cmsg->rc = ws_set_vscroll( cmsg->wid, cmsg->param1 );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_SET_HSCROLL:
	    cmsg->rc = ws_set_hscroll( cmsg->wid, cmsg->param1 );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_REBUILD_SCREEN:
	    ws_rebuild_screen();
	    Reply( pid, 0, 0 );
	    break;
	case WS_WINDOW_INFO:
	    msg->err = ws_get_window_info( msg->wid, &msg->new_window, &cp );
	    Reply( pid, msg, sizeof( WINDOW_MSG ) );
	    break;
	case WS_CLREOL:
	    cmsg->rc = ws_filleol( cmsg->wid, ' ' );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_GET_CSIZE:
	    cmsg->rc = ws_get_crs_size( cmsg->wid, &cmsg->param1 );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_SET_CSIZE:
	    cmsg->rc = ws_set_crs_size( cmsg->wid, cmsg->param1 );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_SET_CSIZEPC:
	    cmsg->param3 = compute_cursorsize( cmsg->param1, cmsg->param2 );
	    cmsg->rc = ws_set_crs_size( cmsg->wid, cmsg->param3 );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_WHEREX:
	    cmsg->rc = ws_wherex( cmsg->wid, &cmsg->param1 );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_WHEREY:
	    cmsg->rc = ws_wherey( cmsg->wid, &cmsg->param1 );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_SET_DEFAULT:
	    ws_set_defaults( &dmsg->def );
	    Reply( pid, 0, 0 );
	    break;
	case WS_GET_DEFAULT:
	    ws_get_defaults( &dmsg->def );
	    Reply( pid, dmsg, sizeof( WINDOW_DEF_MSG ) );
	    break;
	case WS_SCREEN_INFO:
	    cmsg->param1 = ws_screen_max_x();
	    cmsg->param2 = ws_screen_max_y();
	    cmsg->param3 = ws_screen_mode();
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_ASCII:
	    cmsg->param1 = ws_ascii( cmsg->param1 );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	case WS_REMOVE_WINDOWS:
	    ws_remove_windows( (PID) cmsg->param1 );
	    break;
	case WS_CHOWN_HANDLE:
	    cmsg->rc = ws_chown_handle( cmsg->wid, cmsg->param1 );
	    Reply( pid, cmsg, sizeof( COMMON_WINDOW_MSG ) );
	    break;
	default: shutdown( "window_manager(): Bad req" );
	}
    }
}
