#ifdef __HERMES__
#include <cinclude.h>
#endif

#include <stdarg.h>
#include <ci_types.h>
#include <winlib.h>

#ifdef __TURBOC__

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#endif

#ifdef __GNUC__

#include <sysmsg.h>
#include "../../nucleus/include/timer.h"

#endif


#ifdef __TURBOC__

#include "win_serv.h"
#include "mevent.h"
#include "mouse.h"
#include "specials.h"
#include "ticker.h"

#endif


#define MAX_PBUF	1024

extern u_char		new_mouse_info;
extern u_char		screen_max_x, screen_max_y;
extern WL_ERR_HANDLER	error_handler;

/* Up-Arrow */
u_short	up_arrow_and[ 16 ] = {
    0xc000, 0xe000, 0xf000, 0xf800, 0xfc00, 0xfe00, 0xff00,
    0xff80, 0xfe00, 0x1e00, 0x1f00, 0x0f00, 0x0f00, 0x0000,
    0x0000, 0x0000
};

u_short	up_arrow_or[ 16 ] = {
    0x0000, 0x4000, 0x6000, 0x7000, 0x7800, 0x7c00, 0x7e00,
    0x6800, 0x0c00, 0x0c00, 0x0600, 0x0600, 0x0000, 0x0000,
    0x0000, 0x0000
};

/* hour-glass */
u_short	hour_glass_and[ 16 ] = {
    0xffc0, 0xffc0, 0xc0c0, 0xd4c0, 0x6980, 0x6180, 0x3f00,
    0x1e00, 0x1e00, 0x3f00, 0x6180, 0x6580, 0xcac0, 0xc0c0,
    0xffc0, 0xffc0
};

u_short	hour_glass_or[ 16 ] = {
    0x0000, 0x7f80, 0x4080, 0x5480, 0x2900, 0x2100, 0x1e00,
    0x0c00, 0x0c00, 0x1e00, 0x2100, 0x2500, 0x4a80, 0x4080,
    0x7f80, 0x0000
};


#ifdef __TURBOC__

static EV_STATE		ev_state;



int c_break(void)
{
    return ( 1 );
}



u_char wl_test_for_event( EVENT_INFO_TYPE *ev )
{
    static u_int	last_tick = 0;
    MOUSE_INFO_TYPE	*mi;
    u_short		ch;
    u_char		new_act;
    u_int		delta_ticks;
    u_int		timer_mask;
    WIN_MANAGER_STATE	state;
    
    switch( ev_state ) {

    case ST_NORMAL:
	if( new_mouse_info ) {
	    do {
		while( !new_mouse_info );
		ms_get_mouse_info( &mi );
		wm_process_mouse_event( mi, ev, &new_act );
		state = manager_state();
	    } while( state == MOVING_WINDOW || state == RESIZING_WINDOW );
	    if( new_act ) {
		ev_state = ST_SET_FOCUS;
	    }

	    if( ev->event_type != EV_NO_EVENT ) {
		return( NEW_EVENT );
	    }
	}

	if( hw_kbhit() ) {
	    ch = hw_getch();
	    ev->ke.key = ( ch & 0xff ) ? ch & 0xff : ch;
	    ev->event_type = EV_KEY_PRESSED;
	    return( NEW_EVENT );
	}

	if( get_ticks() > last_tick ) {
	    last_tick = get_ticks();
	    timer_mask = hardware_timer_tick();
	    if( timer_mask ) {
		ev->event_type = EV_WAKEUP;
		ev->te.timer_mask = timer_mask;
		return( NEW_EVENT );
	    }
	}

	ev->event_type = EV_NO_EVENT;
	return( NO_EVENT );

    case ST_SET_FOCUS:
	ev->event_type = EV_SET_FOCUS;
	ev->fe.window_id = ws_actwid();
	ev->fe.old_window_id = ws_last_actwid();
	ev_state = ST_NORMAL;
	return( NEW_EVENT );
    }

    return( NO_EVENT );
}



void wl_wait_for_event( EVENT_INFO_TYPE *ev )
{
    while( !wl_test_for_event( ev ) );
}



void wl_wait_for_specific_event( EVENT_INFO_TYPE *ev, EV_TYPE e_type )
{
    EV_TYPE	x;

    do {
	x = wl_test_for_event( ev );
    } while( x == NO_EVENT || ev->event_type != e_type );
}



ERR_CODE wl_init( u_char max_window )
{
    ERR_CODE	ec;

    setcbrk( 0 );
    init_ticker();
    ctrlbrk(c_break);
    ec = ws_init( max_window );
    if( ec == NO_ERR ) {
	ec = ms_init();
    }
    return(	error_handler( 0, ec ) );
}



ERR_CODE wl_close_win( WID wid )
{
    ERR_CODE	ec;

    ec = ws_close_window( wid, DISPLAY );
    return( error_handler( wid, ec ) );
}



ERR_CODE wl_scroll_up( WID wid, u_char x, u_char y, u_char dx, u_char dy )
{
    ERR_CODE	ec;

    ec = ws_scroll_up( wid, x, y, dx, dy );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_scroll_down( WID wid, u_char x, u_char y, u_char dx, u_char dy )
{
    ERR_CODE	ec;

    ec = ws_scroll_down( wid, x, y, dx, dy );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_scroll_left( WID wid, u_char x, u_char y, u_char dx, u_char dy )
{
    ERR_CODE	ec;

    ec = ws_scroll_left( wid, x, y, dx, dy );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_scroll_right( WID wid, u_char x, u_char y, u_char dx, u_char dy )
{
    ERR_CODE	ec;

    ec = ws_scroll_right( wid, x, y, dx, dy );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_open_win( WIN_PARAM_TYPE *p, WID *wid,
		      s_short *x, s_short *y, s_short *dx, s_short *dy )
{
    ERR_CODE	ec;

    ec = ws_open_window( p, wid, x, y, dx, dy );
    return(	error_handler( *wid, ec ) );
}



ERR_CODE wl_define_border( WID wid, u_char *border )
{
    ERR_CODE	ec;

    ec = ws_define_border( wid, border );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_modify_options( WID wid, u_short mask, u_short options )
{
    ERR_CODE	ec;

    ec = ws_modify_options( wid, mask, options );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_printf( WID wid, s_char *format, ... )
{
    u_char		buf[ MAX_PBUF ];
    va_list		arg_ptr;
    ERR_CODE	ec;

    va_start( arg_ptr, format );
    vsprintf( buf, format, arg_ptr );
    va_end( arg_ptr );

    ec = ws_print( wid, buf );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_raise_win( WID wid )
{
    ERR_CODE	ec;

    ec = ws_raise_window( wid, DISPLAY );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_drop_win( WID wid )
{
    ERR_CODE	ec;

    ec = ws_drop_window( wid );
    return(	error_handler( wid, ec ) );
}



void wl_set_background( u_short *img, u_char dx, u_char dy )
{
    ws_set_background( img, dx, dy );
}



ERR_CODE wl_move_win( WID wid, s_short x, s_short y )
{
    ERR_CODE	ec;

    ec = ws_move_window( wid, x, y, DISPLAY );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_resize_win( WID wid, s_short dx, s_short dy )
{
    ERR_CODE	ec;

    ec = ws_resize_window( wid, dx, dy );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_zoom_win( WID wid )
{
    ERR_CODE	ec;

    ec = ws_zoom_window( wid );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_unzoom_win( WID wid )
{
    ERR_CODE	ec;

    ec = ws_unzoom_window( wid );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_iconify_win( WID wid )
{
    ERR_CODE	ec;

    ec = ws_iconify_window( wid );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_deiconify_win( WID wid )
{
    ERR_CODE	ec;

    ec = ws_deiconify_window( wid );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_define_icon( WID wid, s_short x, s_short y, WL_COLOR col,
			 u_char *title, u_char no_def )
{
    ERR_CODE	ec;

    ec = ws_define_icon( wid, x, y, col, title, no_def );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_move_icon( WID wid, s_short x, s_short y )
{
    ERR_CODE	ec;

    ec = ws_def_icon_pos ( wid, x, y );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_set_vscroll( WID wid, s_char percent )
{
    ERR_CODE	ec;

    ec = ws_set_vscroll( wid, percent );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_set_hscroll( WID wid, s_char percent )
{
    ERR_CODE	ec;

    ec = ws_set_hscroll( wid, percent );
    return(	error_handler( wid, ec ) );
}



void wl_rebuild_screen( void )
{
    ws_rebuild_screen();
}



ERR_CODE wl_activate_wid( WID wid )
{
    ERR_CODE	ec;

    ec = ws_activate_wid( wid );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_get_win_info( WID wid, WIN_PARAM_TYPE *p )
{
    CLIPS_TYPE	cp;
    ERR_CODE	ec;

    ec = ws_get_window_info( wid, p, &cp );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_clrscr( WID wid )
{
    ERR_CODE	ec;

    ec = ws_clrscr( wid );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_clreol( WID wid )
{
    ERR_CODE	ec;

    ec = ws_filleol( wid, 32 );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_filleol( WID wid, u_char character )
{
    ERR_CODE	ec;

    ec = ws_filleol( wid, character );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_gotoxy( WID wid, u_char x, u_char y )
{
    ERR_CODE	ec;

    ec = ws_gotoxy( wid, x, y );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_get_textattr( WID wid, u_char *ta )
{
    ERR_CODE	ec;

    ec = ws_get_textattr( wid, ta );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_set_textattr( WID wid, u_char attrib )
{
    ERR_CODE	ec;

    ec = ws_set_textattr( wid, attrib );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_get_csize( WID wid, u_short *size )
{
    ERR_CODE	ec;

    ec = ws_get_crs_size( wid, size );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_set_csize( WID wid, u_short size )
{
    ERR_CODE	ec;

    ec = ws_set_crs_size( wid, size );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_set_csizepc( WID wid, u_short start, u_short stop )
{
    ERR_CODE	ec;
    u_short		size;

    size = compute_cursorsize( start, stop );
    ec = ws_set_crs_size( wid, size );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_cursor_off( WID wid )
{
    ERR_CODE	ec;

    ec = ws_cursor_off( wid );
    return(	error_handler( wid, ec ) );
}




ERR_CODE wl_cursor_on( WID wid )
{
    ERR_CODE	ec;

    ec = ws_cursor_on( wid );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_wherex( WID wid, u_char *x )
{
    ERR_CODE	ec;

    ec = ws_wherex( wid, x );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_wherey( WID wid, u_char *y )
{
    ERR_CODE	ec;

    ec = ws_wherey( wid, y );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_textmode( WL_TEXT_MODE mode )
{
    ERR_CODE	ec;

    ec = ws_text_mode( mode );
    return(	error_handler( 0, ec ) );
}



ERR_CODE wl_grap_mouse( WID wid )
{
    ERR_CODE	ec;

    ec = ws_grap_mouse( wid );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_ungrap_mouse( void )
{
    ERR_CODE	ec;

    ec = ws_ungrap_mouse();
    return(	error_handler( 0, ec ) );
}



ERR_CODE wl_copy_to_window( WID wid, u_char *content )
{
    ERR_CODE	ec;

    ec = ws_copy_to_window( wid, content );
    return(	error_handler( wid, ec ) );
}



ERR_CODE wl_save_dos( void )
{
    ERR_CODE	ec;
    ec = ws_save_dos();
    return( error_handler( 0, ec ) );
}



ERR_CODE wl_restore_dos( void )
{
    ERR_CODE	ec;
    ec = ws_restore_dos();
    return( error_handler( 0, ec ) );
}



void wl_get_def( DEFAULTS_TYPE *def )
{
    ws_get_defaults( def );
}



void wl_set_def( DEFAULTS_TYPE *def )
{
    ws_set_defaults( def );
}



void wl_wake_me_up( u_int timer_ticks, u_int timer_mask )
{
    SET_TIMER	timer;

    timer.count = timer_ticks;
    timer.timer_mask = timer_mask;
    enqueue_timer_client( &timer );
}



void wl_change_grmouse( u_short *new_and, u_short *new_or )
{
    ws_change_grmouse( new_and, new_or);
}



s_char wl_ascii( s_char x )
{
    return( ws_ascii( x ) );
}



u_char wl_screen_max_x( void )
{
    return( ws_screen_max_x() );
}



u_char wl_screen_max_y( void )
{
    return( ws_screen_max_y() );
}



u_char wl_screen_mode( void )
{
    return( ws_screen_mode() );
}



void wl_exit( void )
{
    ms_exit();
    exit_ticker();
    ws_reload_font();
}

#endif



#ifdef __GNUC__

#include "winherm.c"

#endif
