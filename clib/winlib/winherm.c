

void *winlib_copy_buffer;


#if 0
u_char wl_test_for_event( event_info_type *ev )
{
	MOUSE_INFO_TYPE	*mi;
	u_short			ch;

	if( new_mouse_info ) {
		ms_get_mouse_info( &mi );
		wm_process_mouse_event( mi, ev );
		if( ev->event_type != EV_NO_EVENT ) {
			return( NEW_EVENT );
		}
	}

	if( kbhit() ) {
		ch = getch();
		if( !ch ) {
		   ch = ( getch() << 8 );
		}
		ev->key = ch;
		ev->event_type = EV_KEY_PRESSED;
		return( NEW_EVENT );
	}

	ev->event_type = EV_NO_EVENT;
	return( NO_EVENT );
}

#endif


void wl_wait_for_event( EVENT_INFO_TYPE *ev )
{
	DEQUEUE_EVENT_MSG	*msg;
	EVENT_INFO_TYPE		*e;
	int					data_len;

	msg = winlib_copy_buffer;
	e   = winlib_copy_buffer;
	msg->req = DEQUEUE_WINDOW_EVENT;
	EventManagerSend( winlib_copy_buffer,
					  sizeof( DEQUEUE_EVENT_MSG ), &data_len );
	*ev = *e;
}


#if 0

void wl_wait_for_specific_event( event_info_type *ev, EV_TYPE e_type )
{
	u_char	x;

	do {
		x = wl_test_for_event( ev );
	} while( x == NO_EVENT || ev->event_type != e_type );
}
#endif



ERR_CODE wl_init( u_char max_windows )
{
	winlib_copy_buffer = GetCopyBuffer();
}



ERR_CODE wl_close_win( WID wid )
{
	COMMON_WINDOW_MSG	*msg;
	int 				data_len;

	msg = winlib_copy_buffer;
	msg->req = CLOSE_WINDOW;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_open_win( WIN_PARAM_TYPE *p, WID *wid,
				  s_short *x, s_short *y, s_short *dx, s_short *dy )
{
	WINDOW_MSG	*msg;
	int			data_len;

	msg = winlib_copy_buffer;
	msg->req = OPEN_WINDOW;
	msg->new_window = *p;
	WindowServerSend( winlib_copy_buffer, sizeof( WINDOW_MSG ), &data_len );
	*wid = msg->wid;
	*x = msg->new_window.x;
	*y = msg->new_window.y;
	*dx = msg->new_window.dx;
	*dy = msg->new_window.dy;
}



ERR_CODE wl_define_border( WID wid, u_char *border )
{
	PRINT_WINDOW_MSG	*msg;
	COMMON_WINDOW_MSG	*cmsg;
	int 				data_len;

	cmsg = msg = winlib_copy_buffer;
	msg->req = WS_DEFINE_BORDER;
	msg->wid = wid;
	strcpy( &msg->string, border );
	WindowServerSend( winlib_copy_buffer, sizeof( PRINT_WINDOW_MSG ) + 7, &data_len );
    return( error_handler( wid, cmsg->rc ) );
}



ERR_CODE wl_modify_options( WID wid, u_short mask, u_short options )
{
	COMMON_WINDOW_MSG	*msg;
	int 				data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_MODIFY_OPTIONS;
	msg->wid = wid;
	msg->param1 = (int) mask;
	msg->param2 = (int) options;
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_printf( WID wid, s_char *format, ... )
{
	PRINT_WINDOW_MSG	*msg;
    COMMON_WINDOW_MSG   *cmsg;
	char				*p;
	int					i;
	int					data_len;
	va_list				arg_ptr;

	va_start( arg_ptr, format );
	cmsg = msg = winlib_copy_buffer;
	vsprintf( &msg->string, format, arg_ptr );
	va_end( arg_ptr );


	i = 0;
	for( p = &msg->string; *p != '\0'; p++ ) i++;
	msg->req = PRINT_WINDOW;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer,
				sizeof( PRINT_WINDOW_MSG ) + i, &data_len );
    return( error_handler( wid, cmsg->rc ) );
}



ERR_CODE wl_raise_win( WID wid )
{
	COMMON_WINDOW_MSG	*msg;
	int 				data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_RAISE_WINDOW;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_drop_win( WID wid )
{
	COMMON_WINDOW_MSG	*msg;
	int 				data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_DROP_WINDOW;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



void wl_set_background( u_short *img, u_char dx, u_char dy )
{
	COMMON_WINDOW_MSG	*msg;
	int 				data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_SET_BACKGROUND;
	msg->param1 = dx;
	msg->param2 = dy;
	bcopy( (void*) img, (void*) &msg->param3, sizeof( u_short ) * dx * dy );
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ) +
					  sizeof( u_short ) * dx * dy, &data_len );
}



ERR_CODE wl_move_win( WID wid, s_short x, s_short y )
{
	COMMON_WINDOW_MSG	*msg;
	int 				data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_MOVE_WINDOW;
	msg->wid = wid;
	msg->param1 = x;
	msg->param2 = y;
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_resize_win( WID wid, s_short dx, s_short dy )
{
	COMMON_WINDOW_MSG	*msg;
	int 				data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_RESIZE_WINDOW;
	msg->wid = wid;
	msg->param1 = dx;
	msg->param2 = dy;
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_zoom_win( WID wid )
{
	COMMON_WINDOW_MSG	*msg;
	int 				data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_ZOOM_WINDOW;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_unzoom_win( WID wid )
{
	COMMON_WINDOW_MSG	*msg;
	int 				data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_UNZOOM_WINDOW;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_iconify_win( WID wid )
{
	COMMON_WINDOW_MSG	*msg;
	int 				data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_ICON_WINDOW;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_deiconify_win( WID wid )
{
	COMMON_WINDOW_MSG	*msg;
	int 				data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_DEICON_WINDOW;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_define_icon( WID wid, s_short x, s_short y, WL_COLOR col,
					 u_char *title, u_char no_def )
{
	DEF_ICON_MSG		*msg;
	COMMON_WINDOW_MSG	*cmsg;
	int					data_len;

	msg = cmsg = winlib_copy_buffer;
	msg->req = WS_DEF_ICON;
	msg->wid = wid;
	msg->x = (int) x;
	msg->y = (int) y;
	msg->col = (int) col;
	msg->no_def = (int) no_def;
	strcpy( &msg->title, title );
	WindowServerSend( winlib_copy_buffer, sizeof( DEF_ICON_MSG ) +
				      strlen( &msg->title ), &data_len );
    return( error_handler( wid, cmsg->rc ) );
}



ERR_CODE wl_def_ipos( WID wid, s_short x, s_short y )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_DEF_ICON_POS;
	msg->wid = wid;
	msg->param1 = (int) x;
	msg->param2 = (int) y;
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_set_vscroll( WID wid, s_char percent )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_SET_VSCROLL;
	msg->wid = wid;
	msg->param1 = (int) percent;
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_set_hscroll( WID wid, s_char percent )
{
	COMMON_WINDOW_MSG   *msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_SET_HSCROLL;
	msg->wid = wid;
	msg->param1 = (int) percent;
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



void wl_rebuild_screen( void )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_REBUILD_SCREEN;
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ), &data_len );
}



ERR_CODE wl_get_win_info( WID wid, WIN_PARAM_TYPE *p )
{
	WINDOW_MSG	*msg;
	int			data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_WINDOW_INFO;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer, sizeof( WINDOW_MSG ), &data_len );
	*p = msg->new_window;
    return( error_handler( wid, msg->err ) );
}



ERR_CODE wl_clrscr( WID wid )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = CLS_WINDOW;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer,
					  sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_clreol( WID wid )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_CLREOL;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer,
					  sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_gotoxy( WID wid, u_char x, u_char y )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = GOTOXY_WINDOW;
	msg->wid = wid;
	msg->param1 = (int) x;
	msg->param2 = (int) y;
	WindowServerSend( winlib_copy_buffer,
					sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_set_textattr( WID wid, u_char attrib )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_SET_TEXTATTR;
	msg->wid = wid;
	msg->param1 = (int) attrib;
	WindowServerSend( winlib_copy_buffer,
				sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_get_csize( WID wid, u_short *size )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_GET_CSIZE;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer,
					  sizeof( COMMON_WINDOW_MSG ), &data_len );
	*size = msg->param1;
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_set_csize( WID wid, u_short size )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_SET_CSIZE;
	msg->wid = wid;
	msg->param1 = size;
	WindowServerSend( winlib_copy_buffer,
					  sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_set_csizepc( WID wid, u_short start, u_short stop )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_SET_CSIZEPC;
	msg->wid = wid;
	msg->param1 = start;
	msg->param2 = stop;
	WindowServerSend( winlib_copy_buffer,
					  sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_cursor_on( WID wid )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_CURSOR_ON;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer,
				sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_cursor_off( WID wid )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_CURSOR_OFF;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer,
				sizeof( COMMON_WINDOW_MSG ), &data_len );
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_wherex( WID wid, u_char *x )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_WHEREX;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer,
				sizeof( COMMON_WINDOW_MSG ), &data_len );
	*x = msg->param1;
    return( error_handler( wid, msg->rc ) );
}



ERR_CODE wl_wherey( WID wid, u_char *y )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_WHEREX;
	msg->wid = wid;
	WindowServerSend( winlib_copy_buffer,
				sizeof( COMMON_WINDOW_MSG ), &data_len );
	*y = msg->param1;
    return( error_handler( wid, msg->rc ) );
}


#if 0
void wl_textmode( WL_TEXT_MODE mode )
{
	panic( ws_text_mode( mode ) );
}
#endif



void wl_get_def( DEFAULTS_TYPE *def )
{
	WINDOW_DEF_MSG		*dmsg;
	int					data_len;

	dmsg = winlib_copy_buffer;
	dmsg->req = WS_GET_DEFAULT;
	WindowServerSend( winlib_copy_buffer,
					  sizeof( WINDOW_DEF_MSG ), &data_len );
	*def = dmsg->def;
}



void wl_set_def( DEFAULTS_TYPE *def )
{
	WINDOW_DEF_MSG		*dmsg;
	int					data_len;

	dmsg = winlib_copy_buffer;
	dmsg->req = WS_SET_DEFAULT;
	dmsg->def = *def;
	WindowServerSend( winlib_copy_buffer,
					  sizeof( WINDOW_DEF_MSG ), &data_len );
}



void wl_wake_me_up( u_int timer_ticks, u_int timer_mask )
{
	TIMER_MSG	*msg;

	msg = winlib_copy_buffer;
	msg->wakeup_mode	= WAKEUP_BY_EVENT;
	msg->count			= timer_ticks;
	msg->timer_mask		= timer_mask;
	TimerManagerMessage( winlib_copy_buffer, sizeof( TIMER_MSG ) );
}


u_char wl_screen_max_x( void )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_SCREEN_INFO;
	WindowServerSend( winlib_copy_buffer,
				sizeof( COMMON_WINDOW_MSG ), &data_len );
	return( msg->param1 );
}



u_char wl_screen_max_y( void )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_SCREEN_INFO;
	WindowServerSend( winlib_copy_buffer,
				sizeof( COMMON_WINDOW_MSG ), &data_len );
	return( msg->param2 );
}


u_char wl_screen_mode( void )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_SCREEN_INFO;
	WindowServerSend( winlib_copy_buffer,
				sizeof( COMMON_WINDOW_MSG ), &data_len );
	return( msg->param3 );
}


s_char wl_ascii( s_char a )
{
	COMMON_WINDOW_MSG	*msg;
	int					data_len;

	msg = winlib_copy_buffer;
	msg->req = WS_ASCII;
	msg->param1 = a;
	WindowServerSend( winlib_copy_buffer,
				sizeof( COMMON_WINDOW_MSG ), &data_len );
	return( msg->param1 );
}


