/* Window-Tool für C
   --------------------------------------------------
   9/91 by WS
*/

#ifdef __TURBOC__

#include <stdlib.h>
#include <dos.h>

#endif

#ifdef __HERMES__

#include <cinclude.h>
#endif

#include <malloc.h>
#include <ci_types.h>
#include <winlib.h>
#include "win_serv.h"
#include "ws_intrn.h"
#include "specials.h"

#ifdef __HERMES__
#include "mevent.h"
#endif



/*
  ws_set_cursor
  -------------------------------------------------------
  Sets the cursor to the given window. If the current
  position is invisible, the cursor will be switched off.
*/

void ws_set_cursor( WID wid )
{
    WIN_LIST_TYPE	*win, *swin;
    MOUSE_CHAR		ms;
    u_char			visi;

    if( active_wid == ROOT_WINDOW || wid == ROOT_WINDOW ) {
	hw_set_cursor_size( 0x2021 );
	return;
    }

    swin = win = WIN_INFO( wid );
    while( swin->father_ptr != ROOT_PTR ) {
	swin = swin->father_ptr;
    }
    swin->cursor_wid = wid;
    cursor_wid = wid;
    visi = coordinate_visible( wid, win->x + win->x_pos - 1,
			       win->y + win->y_pos - 1, &ms );
    if( visi != INVISIBLE && !win->iconic && win->cursor_on ) {
	hw_set_cursor_size( win->crs_size );
	hw_gotoxy( win->x + win->x_pos - 1, win->y + win->y_pos - 1 );
    }
    else {
	hw_set_cursor_size( 0x2021 );
    }
}




/*
  ws_init
  -------------------------------------------------------
  Initializes the window server.
*/

ERR_CODE ws_init( u_char max_window )
{
    u_int		i;
    u_char		x_pos, y_pos;
    WIN_LIST_TYPE	*win;

    scr_image = malloc( 2 );
    if( scr_image == NULL ) {
	return( WS_MEMORY );
    }

    *scr_image = ( ( DEFAULT_ATTRIB ) << 8 ) + DEFAULT_CHAR;
    hw_get_screen_info( &x_pos, &y_pos, &screen_max_x, &screen_max_y, &screen_mode );
    video_type = hw_get_video_type();
    video_base = get_video_adr( screen_mode );
    hw_set_cursor_size( 0x2021 );
    def = ( screen_mode == WL_MONO ) ? def_mono : def_color;
    mouse_and = def.mouse_and_mask;
    mouse_xor = def.mouse_xor_mask;

	#ifdef ALLOW_GR_MOUSE
	allow_grmouse = 1;
	#endif
	
    gr_mouse_possible = ( ( video_type == WL_VGA_MONO ||
			    video_type == WL_VGA_COLOR ) && allow_grmouse ) ?
	1 : 0;
    mouse_size = ( gr_mouse_possible && screen_mode == WL_C80 ) ? 2 : 1;
    screen_buf = ( u_short* ) malloc( sizeof( u_short ) * screen_max_x * screen_max_y );
    screen_map = ( u_short* ) malloc( sizeof( u_short ) * screen_max_x * screen_max_y );
    upper_buf = ( u_short* ) malloc( sizeof( u_short ) * screen_max_x );
    lower_buf = ( u_short* ) malloc( sizeof( u_short ) * screen_max_x );
    left_buf = ( u_short* ) malloc( sizeof( u_short ) * screen_max_y );
    right_buf = ( u_short* ) malloc( sizeof( u_short ) * screen_max_y );
    max_window_num = max_window + 1;
    win_array = ( WIN_ARRAY_TYPE* ) malloc( max_window_num *
					    sizeof( WIN_ARRAY_TYPE ) );

#ifdef __HERMES__
    max_dup_num = max_window_num;
    dup_array = ( WIN_ARRAY_TYPE* ) malloc( max_dup_num *
					    sizeof( WIN_ARRAY_TYPE ) );
    for( i = 0; i < max_dup_num; i++ ) {
	dup_array[ i ].window = NULL;
	dup_array[ i ].seq_cnt = 0;
    }
#endif

    if( screen_buf == NULL || screen_map == NULL || win_array == NULL ) {
	return( WS_MEMORY );
    }

    for( i = 0; i < screen_max_x * screen_max_y; i++ ) {
	screen_map[ i ] = WINDOW_AREA;
    }
    for( i = 0; i < max_window_num; i++ ) {
	win_array[ i ].seq_cnt = 0;
	win_array[ i ].window = NULL;
    }
    win = ( WIN_LIST_TYPE* ) malloc( sizeof( WIN_LIST_TYPE ) );
    if( win == NULL ) {
	return( WS_MEMORY );
    }

    win->window_id = 0;			/* set values for the root */
    win->win_level = 0;			/* window */
    win->lh_clip = win->lv_clip = 1;
    win->hv_clip = screen_max_y;
    win->hh_clip = screen_max_x;
    win->x = win->orig_x = win->rel_x =
	win->y = win->orig_y = win->rel_y = 1;
    win->dx = win->orig_dx = screen_max_x;
    win->dy = win->orig_dy = screen_max_y;
    win->father_ptr = win;
    win->cursor_wid = ROOT_WINDOW;
    win->tab_step = TAB_STEP;
    win->scroll_x = win->scroll_y = win->icon_x = win->icon_y =
	win->iconic = win->window_options = win->border_col = win->title_col =
	win->screen_col = win->scroll_col = win->attrib = WL_BLACK;
    win->window_options = 0;
    win->cursor_on = 0;
    win->crs_size = hw_get_cursor_size();
    win->x_pos = x_pos; win->y_pos = y_pos;
    win->current_content = ( u_short* ) NULL;
    win->title = ( u_char* ) malloc( 1 );
    win->title[ 0 ] = 0x00;
    win->prev_window = win->next_window = NULL;
    win_list = win_last = win;
    win_array[ 0 ].window = win;
    init_ok = TRUE;
    mouse_x = mouse_y = 1;
    mouse_px = mouse_py = 0;
    put_background( VIDEO_TESTED );
    if( mouse_size == 2 ) {
	gr_mouse_init();
    }
    ws_redraw_mouse( MOUSE_ALL );
    return( NO_ERR );
}



/*
  ws_scroll_up
  -------------------------------------------------------
  Scrolls the window on line up.
*/

ERR_CODE ws_scroll_up( WID wid, u_char x, u_char y, u_char dx, u_char dy )
{
    u_short			i, j;
    u_short     	*buf;
    u_char			m;
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) ) {
	return( WS_WINID );
    }
    win = WIN_INFO( wid );
    if( !check_coordinates( win, x, y, dx, dy ) ) {
	return( WS_COORDINATE );
    }

    buf = win->current_content;
    for( j = y - 1; j < y + dy - 1; j++ ) {
	for( i = x - 1; i < x + dx - 1; i++ ) {
	    buf[ win->dx * j + i ] = buf[ win->dx * ( j + 1 ) + i ];
	}
    }

    for( i = x - 1; i < x + dx - 1; i++ ) {
	buf[ win->dx * ( y + dy - 2 ) + i ] = ( win->attrib << 8 ) + 32;
    }

    if( !win->iconic ) {
	if( win->prev_window != NULL ) {
	    restore_content( win, VIDEO_TESTED );
	}
	else {
	    m = mouse_in_window( wid );
	    if( m ) {
		ws_restore_mouse();
	    }
	    restore_content( win, VIDEO_DIRECT );
	    if( m ) {
		ws_redraw_mouse( MOUSE_ALL );
	    }
	}
    }
    return( NO_ERR );
}



/*
  ws_scroll_down
  -------------------------------------------------------
  Scrolls the window on line down.
*/

ERR_CODE ws_scroll_down( WID wid, u_char x, u_char y, u_char dx, u_char dy )
{
    u_short			i, j;
    u_short     	*buf;
    u_char			m;
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) ) {
	return( WS_WINID );
    }
    win = WIN_INFO( wid );
    if( !check_coordinates( win, x, y, dx, dy ) ) {
	return( WS_COORDINATE );
    }

    buf = win->current_content;
    for( j = y + dy - 2; j > y - 1; j-- ) {
	for( i = x - 1; i < x + dx - 1; i++ ) {
	    buf[ win->dx * j + i ] = buf[ win->dx * ( j - 1 ) + i ];
	}
    }

    for( i = x - 1; i < x + dx - 1; i++ ) {
	buf[ win->dx * ( y - 1 ) + i ] = ( win->attrib << 8 ) + 32;
    }

    if( !win->iconic ) {
	if( win->prev_window != NULL ) {
	    restore_content( win, VIDEO_TESTED );
	}
	else {
	    m = mouse_in_window( wid );
	    if( m ) {
		ws_restore_mouse();
	    }
	    restore_content( win, VIDEO_DIRECT );
	    if( m ) {
		ws_redraw_mouse( MOUSE_ALL );
	    }
	}
    }
    return( NO_ERR );
}



/*
  ws_scroll_left
  -------------------------------------------------------
  Scrolls the window on line left.
*/

ERR_CODE ws_scroll_left( WID wid, u_char x, u_char y, u_char dx, u_char dy )
{
    u_short			i, j;
    u_short     	*buf;
    u_char			m;
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) ) {
	return( WS_WINID );
    }
    win = WIN_INFO( wid );
    if( !check_coordinates( win, x, y, dx, dy ) ) {
	return( WS_COORDINATE );
    }

    buf = win->current_content;
    for( j = x - 1; j < x + dx - 1; j++ ) {
	for( i = y - 1; i < y + dy - 1; i++ ) {
	    buf[ win->dx * i + j ] = buf[ win->dx * i + j + 1 ];
	}
    }

    for( i = y - 1; i < y + dy - 1; i++ ) {
	buf[ win->dx * i + dx ] = ( win->attrib << 8 ) + 32;
    }

    if( !win->iconic ) {
	if( win->prev_window != NULL ) {
	    restore_content( win, VIDEO_TESTED );
	}
	else {
	    m = mouse_in_window( wid );
	    if( m ) {
		ws_restore_mouse();
	    }
	    restore_content( win, VIDEO_DIRECT );
	    if( m ) {
		ws_redraw_mouse( MOUSE_ALL );
	    }
	}
    }
    return( NO_ERR );
}





/*
  ws_scroll_right
  -------------------------------------------------------
  Scrolls the window on line right.
*/

ERR_CODE ws_scroll_right( WID wid, u_char x, u_char y, u_char dx, u_char dy )
{
    u_short			i, j;
    u_short     	*buf;
    u_char			m;
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) ) {
	return( WS_WINID );
    }
    win = WIN_INFO( wid );
    if( !check_coordinates( win, x, y, dx, dy ) ) {
	return( WS_COORDINATE );
    }

    buf = win->current_content;
    for( j = x + dx - 2; j > x - 1; j-- ) {
	for( i = y - 1; i < y + dy - 1; i++ ) {
	    buf[ win->dx * i + j ] = buf[ win->dx * i + j - 1 ];
	}
    }

    for( i = y - 1; i < y + dy - 1; i++ ) {
	buf[ win->dx * i + x - 1 ] = ( win->attrib << 8 ) + 32;
    }

    if( !win->iconic ) {
	if( win->prev_window != NULL ) {
	    restore_content( win, VIDEO_TESTED );
	}
	else {
	    m = mouse_in_window( wid );
	    if( m ) {
		ws_restore_mouse();
	    }
	    restore_content( win, VIDEO_DIRECT );
	    if( m ) {
		ws_redraw_mouse( MOUSE_ALL );
	    }
	}
    }
    return( NO_ERR );
}



/*
  ws_insline
  -------------------------------------------------------
  Inserts a line at the current cursor position. All
  following lines scroll one line down. The least line is
  lost.
*/

ERR_CODE ws_insline( WID wid )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    ws_scroll_down( wid, 1, win->y_pos, win->dx, win->dy - win->y_pos + 1 );
    return( NO_ERR );
}



/*
  ws_delline
  -------------------------------------------------------
  Deletes a line at the current cursor position. All
  following lines scroll one line up. At the bottom of the
  window a blank line is inserted.
*/

ERR_CODE ws_delline( WID wid )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    ws_scroll_up( wid, 1, win->y_pos, win->dx, win->dy - win->y_pos + 1 );
    return( NO_ERR );
}



/*
  ws_inschar
  -------------------------------------------------------
  Inserts a character at the current cursor position. All
  following characters move one position towards the lower
  left corner of the window.
*/

ERR_CODE ws_inschar( WID wid )
{
    WIN_LIST_TYPE	*win;
    u_short			*content_ptr;
    u_short			first, last, i;

    if( !validate_wid( wid ) ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    first = ( win->y_pos - 1 ) * win->dx + win->x_pos - 1;
    last = win->dx * win->dy - 2;

    for( i = last; i >= first ; i-- ) {
	win->current_content[ i + 1 ] = win->current_content[ i ];
    }
    win->current_content[ first ] = ( win->current_content[ first ] & 0xff00 )
	| 32;
    restore_content( win, VIDEO_TESTED );
    return( NO_ERR );
}



/*
  ws_delchar
  -------------------------------------------------------
  Deletes a character at the current cursor position. All
  following characters move one position towards the
  current cursor possition. At the lower left corner a
  blank character is inserted.
*/

ERR_CODE ws_delchar( WID wid )
{
    WIN_LIST_TYPE	*win;
    u_short			*content_ptr;
    u_short			first, last, i;

    if( !validate_wid( wid ) ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    first = ( win->y_pos - 1 ) * win->dx + win->x_pos - 1;
    last = win->dx * win->dy - 2;

    for( i = first; i <= last ; i++ ) {
	win->current_content[ i ] = win->current_content[ i + 1 ];
    }
    win->current_content[ last + 1 ] = ( win->current_content[ last + 1 ] &
					 0xff00 ) | 32;
    restore_content( win, VIDEO_TESTED );
    return( NO_ERR );
}



/*
  ws_move_cursor
  -------------------------------------------------------
  Moves the cursor one position to the spcified direction.
*/

ERR_CODE ws_move_cursor( WID wid, DIR_TYPE dir )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );

    switch( dir ) {
    case UP_DIR:
	if( win->y_pos > 1 ) {
	    win->y_pos--;
	}
	break;
    case DOWN_DIR:
	if( win->y_pos < win->dy ) {
	    win->y_pos++;
	}
	break;
    case LEFT_DIR:
	if( win->x_pos > 1 ) {
	    win->scroll_flag = 0;
	    win->x_pos--;
	}
	break;
    case RIGHT_DIR:
	if( win->x_pos < win->dx ) {
	    win->x_pos++;
	}
	break;
    }

    ws_set_cursor( cursor_wid );
    return( NO_ERR );
}



/*
  ws_open_window
  -------------------------------------------------------
  Opens a window with the given parameters. Child windows
  are inserted into the window list at the corresponding
  position.
*/

ERR_CODE ws_open_window( WIN_PARAM_TYPE *param, WID *wid,
			 s_short *x, s_short *y, s_short *dx, s_short *dy )
{
    WIN_LIST_TYPE	*new_window, *ptr;
    WIN_PARAM_TYPE	p;
    ERR_CODE		ec;
    s_int			i, j, w_index;

    if( !init_ok ) {
	return( WS_INIT );
    }

    w_index = -1;							/* Find a free window slot */
    for( i = 0; i < max_window_num; i++ ) {
	if( win_array[ i ].window == NULL ) {
	    w_index = i;
	    break;
	}
    }

    if( w_index == -1 ) {
	return( WS_NOMORE_WIN );
    }

    bufcpy( &p, param, sizeof( WIN_PARAM_TYPE ) );
    ec = check_input( &p );
    if( ec != NO_ERR ) {
	return( ec );
    }

    new_window = ( WIN_LIST_TYPE* ) malloc( sizeof( WIN_LIST_TYPE ) );
    if( new_window == NULL ) {
	return( WS_MEMORY );
    }
    win_array[ i ].window = new_window;

    new_window->window_id = ( (u_int) ++win_array[ i ].seq_cnt << 24 ) + w_index;
    new_window->cursor_wid = new_window->window_id;
    if( win_list == win_last ) {
	cursor_wid = active_wid = last_active_wid = new_window->window_id;
    }

    new_window->full_screen = p.full_screen;
    new_window->scroll_flag = 0;
    new_window->father_ptr = win_array[ p.father_window & 0xff ].window;
    new_window->win_level = new_window->father_ptr->win_level + 1;
    new_window->orig_x = p.x; new_window->orig_y = p.y;
    new_window->orig_dx = p.dx; new_window->orig_dy = p.dy;
    new_window->min_dx = p.min_dx; new_window->min_dy = p.min_dy;

#ifdef __HERMES__
    win_array[ w_index ].pid = p.pid;
    new_window->ref_cnt = 1;
#endif

    if( p.full_screen ) {
	if( !( p.window_options & FATHER_CLIP ) ||
	    new_window->father_ptr == ROOT_PTR  ) {
	    new_window->dx = screen_max_x - 2;
	    new_window->dy = screen_max_y - 2;
	    new_window->rel_x = 3 - new_window->father_ptr->x;
	    new_window->rel_y = 3 - new_window->father_ptr->y;
	}
	else {
	    new_window->rel_x = 2;
	    new_window->rel_y = 2;
	    new_window->dx = new_window->father_ptr->dx - 2;
	    new_window->dy = new_window->father_ptr->dy - 2;
	}
    }
    else {
	new_window->rel_x = p.x; new_window->rel_y = p.y;
	new_window->dx = p.dx; new_window->dy = p.dy;
    }

    if( new_window->dx < new_window->min_dx ) {
	new_window->dx = new_window->min_dx;
    }
    if( new_window->dy < new_window->min_dy ) {
	new_window->dy = new_window->min_dy;
    }

    /*
      new_window->x = new_window->father_ptr->orig_x + new_window->rel_x - 1;
      new_window->y = new_window->father_ptr->orig_y + new_window->rel_y - 1;
    */
    new_window->x = new_window->father_ptr->x + new_window->rel_x - 1;
    new_window->y = new_window->father_ptr->y + new_window->rel_y - 1;

    new_window->icon_x = new_window->x;
    new_window->icon_y = new_window->y;
    new_window->icon_moved = 0;
    new_window->iconic = ( param->father_window == ROOT_WINDOW ) ? p.iconic :
	new_window->father_ptr->iconic;
    new_window->tab_step = param->tab_step;
    new_window->scroll_x = new_window->scroll_y = 0;
    new_window->x_pos = new_window->y_pos = 1;
    /*
      new_window->crs_size = 0x0b0c;
    */
    new_window->crs_size = compute_cursorsize( 81, 87 );	/* Underline */
    new_window->cursor_on = 1;
    new_window->window_options = p.window_options;
    new_window->border_col = p.border_col;
    new_window->title_col = p.title_col;
    new_window->screen_col = new_window->attrib = p.screen_col;
    new_window->scroll_col = p.scroll_col;
    new_window->icon_col = p.icon_col;
    new_window->button_col = p.button_col;
    if( new_window->window_options & HEADER ) {
	i = strlen( p.title );
	new_window->title = ( u_char* ) malloc( i + 1 );
	j = ( i < MAX_HEADER_LEN ) ? i : MAX_HEADER_LEN;
	strncpy( new_window->title, p.title, j );
	new_window->title[ j ] = '\0';
    }
    else {
	new_window->title = ( u_char* ) malloc( 1 );
	new_window->title[ 0 ] = 0x00;
    }
    for( i = 0; i < 4; new_window->icon_title[ i ] = 0, i++ );
    if( new_window->window_options & HEADER ) {
	strncpy( new_window->icon_title, param->title, 4 );
    }
    else {
	new_window->icon_title[ 0 ] = 0;
    }
    if( ( new_window->window_options & BORDER ) == USER_BORDER ) {
	strncpy( new_window->user_border, param->user_border, 7 );
    }
    else {
	new_window->user_border[ 0 ] = 0;
    }
    new_window->current_content = ( u_short* ) malloc( new_window->dx *
						       new_window->dy *
						       sizeof( u_short ) );

    if( new_window->current_content == NULL ) {
	return( WS_MEMORY );
    }
    clear_content( new_window );
    if( param->father_window == ROOT_WINDOW ) {
	win_list->prev_window = new_window;
	new_window->next_window = win_list;
	win_list = new_window;
	new_window->prev_window = NULL;

	if( p.iconic ) {
	    redraw_screen( win_last->prev_window );
	}
	else {
	    get_clips( new_window );
	    ws_restore_mouse();
	    print_header( new_window, VIDEO_DIRECT );
	    print_left( new_window, VIDEO_DIRECT );
	    print_right( new_window, VIDEO_DIRECT );
	    print_bottom( new_window, VIDEO_DIRECT );
	    restore_content( new_window, VIDEO_DIRECT );
	    print_shadow( new_window, VIDEO_DIRECT );
	    ws_set_cursor( cursor_wid );
	    ws_redraw_mouse( MOUSE_ALL );
	}
    }
    else {
	ptr = new_window->father_ptr;
	while( ptr->prev_window != NULL &&
	       ptr->prev_window->win_level >= new_window->win_level ) {
	    ptr = ptr->prev_window;
	}
	if( ptr->prev_window == NULL ) {
	    new_window->prev_window = NULL;
	    new_window->next_window = ptr;
	    ptr->prev_window = new_window;
	    win_list = new_window;
	    if( !new_window->iconic ) {
		ws_restore_mouse();
		get_clips( new_window );
		print_header( new_window, VIDEO_DIRECT );
		print_left( new_window, VIDEO_DIRECT );
		print_right( new_window, VIDEO_DIRECT );
		print_bottom( new_window, VIDEO_DIRECT );
		restore_content( new_window, VIDEO_DIRECT );
		print_shadow( new_window, VIDEO_DIRECT );
		ws_set_cursor( cursor_wid );
		ws_redraw_mouse( MOUSE_ALL );
	    }
	}
	else {
	    ptr->prev_window->next_window = new_window;
	    new_window->prev_window = ptr->prev_window;
	    new_window->next_window = ptr;
	    ptr->prev_window = new_window;
	    if( !new_window->iconic ) {
		redraw_screen( win_last->prev_window );
	    }
	}
    }
    *wid = new_window->window_id;
    *x = new_window->rel_x; *y = new_window->rel_y;
    *dx = new_window->dx; *dy = new_window->dy;
    return( NO_ERR );
}



/*
  ws_close_window
  -------------------------------------------------------
  Closes the specified window.
*/

ERR_CODE ws_close_window( WID wid, u_char mode )
{
    WIN_LIST_TYPE	*win, *uppest, *lowest, *swin;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = uppest = lowest = WIN_INFO( wid );
    if( ( wid & 0xffffff ) < max_window_num ) {
	win_array[ wid & 0xffffff ].window = NULL;
    }
    else {
	dup_array[ ( wid & 0xffffff ) - max_window_num ].window = NULL;
    }

#ifdef __HERMES__				/* Decrement the window's reference counter */
    uppest->ref_cnt--;
#endif

    swin = win;
    while( swin->father_ptr != ROOT_PTR ) {
	swin = swin->father_ptr;
    }

#ifdef __HERMES__
    if( swin->cursor_wid == uppest->window_id && swin->ref_cnt <= 0 ) {
	swin->cursor_wid = lowest->father_ptr->window_id;
    }
    if( uppest->window_id == cursor_wid && swin->ref_cnt <= 0 ) {
	cursor_wid = lowest->father_ptr->window_id;
    }
#endif

#ifdef __TURBOC__
    if( swin->cursor_wid == uppest->window_id ) {
	swin->cursor_wid = lowest->father_ptr->window_id;
    }
    if( uppest->window_id == cursor_wid ) {
	cursor_wid = lowest->father_ptr->window_id;
    }
#endif

    while( uppest->prev_window != NULL &&
	   uppest->prev_window->win_level > lowest->win_level ) {

	uppest = uppest->prev_window;

#ifdef __HERMES__
	/* Decrement the reference counter of all child windows of wid */
	uppest->ref_cnt--;
#endif

	if( swin->cursor_wid == uppest->window_id ) {
	    swin->cursor_wid = lowest->father_ptr->window_id;
	}
	if( uppest->window_id == cursor_wid ) {
	    cursor_wid = lowest->father_ptr->window_id;
	}
    }

#ifdef __HERMES__
    /* Close all father windows with a reference counter less or equal than
       zero also.
    */

    if( lowest->ref_cnt > 0 ) {
	return( NO_ERR );
    }

    while( lowest->father_ptr->ref_cnt <= 0 &&
	   lowest->father_ptr != ROOT_PTR ) {
	lowest = lowest->father_ptr;
    }
#endif

    if( uppest->prev_window == NULL ) {
	win_list = lowest->next_window;
    }
    else {
	uppest->prev_window->next_window = lowest->next_window;
    }
    lowest->next_window->prev_window = uppest->prev_window;

    if( wid == active_wid ) {
	activate_window( ws_get_top(), mode );
	/*
	  active_wid = cursor_wid = ROOT_WINDOW;
	*/
    }

    lowest = lowest->next_window;
    win = uppest;
    while( win != lowest ) {

#ifdef __HERMES__
	send_dealloc_to_event_manager( uppest->window_id );
#endif

	free( uppest->current_content );
	free( uppest->title );
	win_array[ uppest->window_id & 0xff ].window = NULL;
	win = win->next_window;
	free( uppest );
	uppest = win;
    };

    if( mode == DISPLAY ) {
	redraw_screen( win_last->prev_window );
    }
    return( NO_ERR );
}



/*
  ws_define_border
  -------------------------------------------------------
  Allows to create a user defined border.
*/

ERR_CODE ws_define_border( WID wid, u_char *border )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    strncpy( win->user_border, border, 7 );
    return( NO_ERR );
}



/*
  ws_modify_options
  -------------------------------------------------------
  Allows to change the border of a window. The new window
  will be immediately printed out to the screen.
*/

ERR_CODE ws_modify_options( WID wid, u_short mask, u_short options )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    mask &= 0xfeff;
    if( !( options & BORDER ) ) {
	options = 0;
    }

    win = WIN_INFO( wid );
    if( ( mask & BORDER ) && ( win->window_options & PRINT_SHADOW ) &&
	!( options & BORDER ) ) {
	mask &= 0xfffc;
    }

    win->window_options &= ~mask;
    win->window_options |= ( options & mask );
    if( !win->iconic ) {
	get_clips( win );
	print_header( win, VIDEO_TESTED );
	print_left( win, VIDEO_TESTED );
	print_right( win, VIDEO_TESTED );
	print_bottom( win, VIDEO_TESTED );
    }
    return( NO_ERR );
}



/*
  ws_print
  -------------------------------------------------------
  Prints out a string into the given window.
*/

ERR_CODE ws_print( WID wid, u_char *text )
{
    u_short			i, max;
    u_short			*wbuf;
    u_char			visi, new_pos;
    MOUSE_CHAR		ms;
    WIN_LIST_TYPE   *win;
    u_short			attrib;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    wbuf = win->current_content;
    attrib = win->attrib << 8;

    for( i = 0; text[ i ] != '\0'; i++ ) {
	switch( text[ i ] ) {
	case '\a':
	    beep();
	    continue;

	case '\n':
	    if( win->y_pos == win->dy ) {
		ws_scroll_up( wid, 1, 1, win->dx, win->dy );
	    }
	    else {
		win->y_pos++;
	    }

#ifdef __TURBOC__						/* LF -> CR/LF under TURBO-C only */
	    win->x_pos = 1;
	    win->scroll_flag = 0;
#endif
	    continue;

	case '\r':
	    win->x_pos = 1;
	    win->scroll_flag = 0;
	    continue;

	case '\t':
	    if( win->scroll_flag ) {
		ws_scroll_up( wid, 1, 1, win->dx, win->dy );
		win->scroll_flag = 0;
	    }
	    new_pos = ( win->x_pos - 1 ) / win->tab_step;
	    new_pos = ( new_pos + 1 ) * win->tab_step + 1;
	    max = ( new_pos > win->dx ) ? win->dx + 1 : new_pos;
	    while( win->x_pos < max ) {
		win->current_content[ ( win->y_pos - 1 ) * win->dx +
				      ( win->x_pos - 1 ) ] =
		    win->attrib << 8 + 32;
		if( !( win->iconic ) ) {
		    visi = coordinate_visible( wid, win->x + win->x_pos - 1,
					       win->y + win->y_pos - 1, &ms );
		    if( visi == VISIBLE ) {
			WRITE_TO_SCREEN( win->x + win->x_pos - 1,
					 win->y + win->y_pos - 1,
					 attrib + 32 );
		    } else if( visi == SHADOWED ) {
			WRITE_TO_SCREEN( win->x + win->x_pos - 1,
					 win->y + win->y_pos - 1,
					 ( WL_SHADOW_COL << 8 ) + 32 );
		    }
		    if( visi != INVISIBLE && ms ) {
			ws_redraw_mouse( ms );
		    }
		}
		win->x_pos++;
	    }
	    win->x_pos = ( new_pos > win->dx ) ? 1 : new_pos;
	    continue;

	case '\b':
	    if( !win->scroll_flag ) {
		if( win->x_pos == 1 ) {
		    if( win->y_pos > 1 ) {
			win->y_pos--;
			win->x_pos = win->dx;
		    }
		}
		else {
		    win->x_pos--;
		}
	    }
	    else {
		win->scroll_flag = 0;
	    }
	    break;

#if 0
	    visi = coordinate_visible( wid, win->x + win->x_pos - 1,
				       win->y + win->y_pos - 1, &ms );
	    if( !( win->iconic ) ) {
		visi = coordinate_visible( wid, win->x + win->x_pos - 1,
					   win->y + win->y_pos - 1, &ms );
		if( visi == VISIBLE ) {
		    WRITE_TO_SCREEN( win->x + win->x_pos - 1,
				     win->y + win->y_pos - 1,
				     attrib + 32 );
		} else if( visi == SHADOWED ) {
		    WRITE_TO_SCREEN( win->x + win->x_pos - 1,
				     win->y + win->y_pos - 1,
				     ( WL_SHADOW_COL << 8 ) + 32 );
		}
		if( ms && wid == active_wid && visi != INVISIBLE ) {
		    ws_redraw_mouse( ms );
		}
	    }
	    wbuf[ win->dx * ( win->y_pos - 1 ) + ( win->x_pos - 1 ) ] =
		attrib + 32;
	    break;
#endif

	default:
	    if( win->x_pos == win->dx &&
		win->y_pos == win->dy && win->scroll_flag ) {
		ws_scroll_up( wid, 1, 1, win->dx, win->dy );
		win->x_pos = 1;
		win->scroll_flag = 0;
	    }
	    if( !( win->iconic ) ) {
		visi = coordinate_visible( wid, win->x + win->x_pos - 1,
					   win->y + win->y_pos - 1, &ms );
		if( visi == VISIBLE ) {
		    WRITE_TO_SCREEN( win->x + win->x_pos - 1,
				     win->y + win->y_pos - 1,
				     attrib +
				     ( u_char ) ws_ascii( text[ i ] ) );
		} else if( visi == SHADOWED ) {
		    WRITE_TO_SCREEN( win->x + win->x_pos - 1,
				     win->y + win->y_pos - 1,
				     ( WL_SHADOW_COL << 8 ) +
				     ( u_char ) ws_ascii( text[ i ] ) );
		}
		if( visi != INVISIBLE && ms ) {
		    ws_redraw_mouse( ms );
		}
	    }
	    wbuf[ win->dx * ( win->y_pos - 1 ) +
		  ( win->x_pos - 1 ) ] = attrib +
		( u_char ) ws_ascii( text[ i ] );
	    if( win->x_pos == win->dx && win->y_pos == win->dy ) {
		win->scroll_flag = 1;
	    } else if( win->x_pos == win->dx ) {
		win->x_pos = 1;
		win->y_pos++;
	    } else {
		win->x_pos++;
	    }
	    break;
	}
    }

    ws_set_cursor( cursor_wid );
    return( NO_ERR );
}



/*
  ws_raise_window
  -------------------------------------------------------
  The given window will be the top window. Child windows
  are inserted at the corresponding position.
*/

ERR_CODE ws_raise_window( WID wid, u_char mode )
{
    WIN_LIST_TYPE	*uppest, *lowest, *ptr;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    ptr = uppest = lowest = WIN_INFO( wid );
    while( uppest->prev_window != NULL &&
	   uppest->prev_window->win_level > lowest->win_level ) {
	uppest = uppest->prev_window;
    }

    if( uppest->prev_window == NULL ) {		/* window is already top window */
	return( NO_ERR );
    }

    if( lowest->father_ptr == ROOT_PTR ) {
	ptr = win_list;
    }
    else {
	while( ptr->prev_window != NULL &&
	       ptr->prev_window->win_level >= lowest->win_level ) {
	    ptr = ptr->prev_window;
	}
    }
    if( ptr == uppest ) {					/* window is the only window */
	return( NO_ERR );                       /* at this level, -> no raise */
    }                                       /* necesarry */

    lowest->next_window->prev_window = uppest->prev_window;
    uppest->prev_window->next_window = lowest->next_window;
    if( ptr == win_list ) {
	uppest->prev_window = NULL;
	lowest->next_window = win_list;
	win_list->prev_window = lowest;
	win_list = uppest;
    }
    else {
	uppest->prev_window = ptr->prev_window;
	lowest->next_window = ptr;
	ptr->prev_window->next_window = uppest;
	ptr->prev_window = lowest;
    }

    if( mode == NO_DISPLAY || lowest->iconic ) {
	return( NO_ERR );
    }

    if( uppest == lowest && uppest->father_ptr == ROOT_PTR ) {
	ws_restore_mouse();
	print_header( lowest, VIDEO_DIRECT );	/* Only one window has to */
	print_left( lowest, VIDEO_DIRECT );     /* be raised to be the */
	print_right( lowest, VIDEO_DIRECT );    /* "absolute top window". */
	print_bottom( lowest, VIDEO_DIRECT );   /* So write it direct to */
	restore_content( lowest, VIDEO_DIRECT );/* screen and avoid  */
	print_shadow( lowest, VIDEO_DIRECT );   /* the redraw */
	ws_set_cursor( cursor_wid );
	ws_redraw_mouse( MOUSE_ALL );
	return( NO_ERR );
    }
    redraw_screen( win_last->prev_window );
    return( NO_ERR );
}



/*
  ws_drop_window
  -------------------------------------------------------
  The given window will be the lowest window. Child
  windows are inserted at the corresponding position of
  the window list.
*/

ERR_CODE ws_drop_window( WID wid )
{
    WIN_LIST_TYPE	*uppest, *lowest, *ptr;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    lowest = uppest = WIN_INFO( wid );
    ptr = lowest->father_ptr;
    if( lowest->next_window == lowest->father_ptr ) {
	return( NO_ERR );						/* window is already dropped */
    }

    while( uppest->prev_window != NULL &&
	   uppest->prev_window->win_level > lowest->win_level ) {
	uppest = uppest->prev_window;
    }

    if( uppest == win_list ) {
	win_list = lowest->next_window;
    }
    else {
	uppest->prev_window->next_window = lowest->next_window;
    }

    lowest->next_window->prev_window = uppest->prev_window;
    lowest->next_window = ptr;
    uppest->prev_window = ptr->prev_window;
    ptr->prev_window->next_window = uppest;
    ptr->prev_window = lowest;
    if( !lowest->iconic ) {
	redraw_screen( win_last->prev_window );
    }
    return( NO_ERR );
}



/*
  ws_set_background
  -------------------------------------------------------
  Defines a new screen background and prints it out to
  the screen.
*/

void ws_set_background( u_short *image, u_char dx, u_char dy )
{
    scr_image = image;
    img_dx = dx;
    img_dy = dy;
    put_background( VIDEO_TESTED );
}



/*
  ws_move_window
  -------------------------------------------------------
  Moves the window to the given position.
*/

ERR_CODE ws_move_window( WID wid, s_short x, s_short y, u_char mode )
{
    WIN_LIST_TYPE	*win;
    s_short			abs_x, abs_y;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );

    /* Check window position. So a window can be moved directly by an
       application by calling a library function.
    */

    get_clips( win );
    abs_x = win->father_ptr->x + x - 1;
    abs_y = win->father_ptr->y + y - 1;
    if( abs_x > win->hh_clip ) abs_x = win->hh_clip;
    if( abs_x + win->dx - 1 < win->lh_clip ) abs_x = win->lh_clip - win->dx + 1;
    if( abs_y - 1 > win->hv_clip ) abs_y = win->hv_clip;
    if( abs_y + win->dy - 1 < win->lv_clip ) abs_y = win->lv_clip - win->dy + 1;
    x = abs_x + 1 - win->father_ptr->x;
    y = abs_y + 1 - win->father_ptr->y;
    /*
      win->rel_x = win->orig_x = x;
      win->rel_y = win->orig_y = y;
    */
    win->rel_x = win->x = x;
    win->rel_y = win->y = y;
    if( !win->iconic && mode == DISPLAY ) {
	redraw_screen( win_last->prev_window );
    }
    return( NO_ERR );
}



/*
  ws_resize_window
  -------------------------------------------------------
  Resizes the window to the given size. A size less than
  min_dx/ (dy) is set to min_dx/ (dy).
*/

ERR_CODE ws_resize_window( WID wid, s_short dx, s_short dy )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );

    /* Check window size. So a window can be resized directly by an
       application by calling a library function.
    */

    if( dx < win->min_dx ) dx = win->min_dx;
    if( dx > screen_max_x - 2 ) dx = screen_max_x - 2;
    if( dy < win->min_dy ) dy = win->min_dy;
    if( dy > screen_max_y - 2 ) dy = screen_max_y - 2;

    win->dx = dx;
    win->dy = dy;
    win->orig_dx = dx;
    win->orig_dy = dy;
    win->x_pos = win->y_pos = 1;
    win->full_screen = 0;
    win->orig_x = win->rel_x;
    win->orig_y = win->rel_y;
    free( win->current_content );
    win->current_content = malloc( sizeof( u_short ) * dx * dy );
    if( win->current_content == NULL ) {
	return( WS_MEMORY );
    }

    clear_content( win );
    if( !win->iconic ) {
	redraw_screen( win_last->prev_window );
    }
    return( NO_ERR );
}



/*
  ws_zoom_window
  -------------------------------------------------------
  Raises the window and resizes it to the maximum size.
  If the father_clip option is set, the size of child
  windows depends on the size of their father window.
*/

ERR_CODE ws_zoom_window( WID wid )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );

    if( win->full_screen ) {
	return( NO_ERR );
    }

    win->full_screen = 1;
    if( win->window_options & FATHER_CLIP || win->father_ptr == ROOT_PTR ) {
	win->dx = win->father_ptr->dx - 2;
	win->dy = win->father_ptr->dy - 2;
	win->rel_x = 2;
	win->rel_y = 2;
	win->x = win->father_ptr->x + win->rel_x - 1;
	win->y = win->father_ptr->y + win->rel_y - 1;
    }
    else {
	win->dx = ROOT_PTR->dx - 2;
	win->dy = ROOT_PTR->dy - 2;

	win->rel_x = 3 - win->father_ptr->x;
	win->rel_y = 3 - win->father_ptr->y;
    }
    win->x_pos = win->y_pos = 1;
    free( win->current_content );
    win->current_content = malloc( sizeof( u_short ) * win->dx * win->dy );
    if( win->current_content == NULL ) {
	return( WS_MEMORY );
    }

    clear_content( win );
    ws_raise_window( wid, NO_DISPLAY );
    if( !win->iconic ) {
	if( win->father_ptr == ROOT_PTR ||
	    !( win->window_options & FATHER_CLIP ) ) {
	    redraw_screen( win );
	}
	else {
	    redraw_screen( win_last->prev_window );
	}
    }
    return( NO_ERR );
}



/*
  ws_unzoom_window
  -------------------------------------------------------
  Sets a window to it original size.
*/

ERR_CODE ws_unzoom_window( WID wid )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    if( !win->full_screen ) {
	return( NO_ERR );
    }

    win->full_screen = 0;
    win->dx = win->orig_dx;
    win->dy = win->orig_dy;
    win->x_pos = win->y_pos = 1;
    win->rel_x = win->orig_x;
    win->rel_y = win->orig_y;
    free( win->current_content );
    win->current_content = malloc( sizeof( u_short ) * win->dx * win->dy );
    if( win->current_content == NULL ) {
	return( WS_MEMORY );
    }

    clear_content( win );
    if( !win->iconic ) {
	redraw_screen( win_last->prev_window );
    }
    return( NO_ERR );
}



/*
  ws_iconify_window
  -------------------------------------------------------
  Prints out an icon box an removes the window including
  all child windows.
*/

ERR_CODE ws_iconify_window( WID wid )
{
    WIN_LIST_TYPE	*win;
    u_char			level;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    if( win->iconic ) {
	return( NO_ERR );
    }
    win->iconic = 1;
    win->icon_moved = 0;
    level = win->win_level;
    while( win->prev_window != NULL &&
	   win->prev_window->win_level > level ) {
	win = win->prev_window;
	win->iconic = 1;
    }
    redraw_screen( win_last->prev_window );
    return( NO_ERR );
}



/*
  ws_deiconify_window
  -------------------------------------------------------
  Removes the icon box and redisplays the window and it's
  child windows.
*/

ERR_CODE ws_deiconify_window( WID wid )
{
    WIN_LIST_TYPE	*win;
    u_char			level;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    if( !win->iconic ) {
	return( NO_ERR );
    }

    win->iconic = 0;
    level = win->win_level;
    while( win->prev_window != NULL &&
	   win->prev_window->win_level > level ) {
	win = win->prev_window;
	win->iconic = 0;
    }
    redraw_screen( win_last->prev_window );
    return( NO_ERR );
}



/*
  ws_define_icon
  -------------------------------------------------------
  Defines the parameters of an icon.
*/

ERR_CODE ws_define_icon( WID wid, s_short x, s_short y, WL_COLOR col,
			 u_char *title, u_char no_def )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    win->icon_x = ( x >= 1 && x <= screen_max_x - 4 ) ? x : 1;
    win->icon_y = ( y >= 1 && y <= screen_max_y - 2 ) ? y : 1;
    win->icon_col = col;
    strncpy( win->icon_title, title, 4 );
    if( no_def ) {
	win->window_options |= NO_DEF_ICON;
    }
    else {
	win->window_options &= ~NO_DEF_ICON;
    }
    return( NO_ERR );
}



/*
  ws_def_icon_pos
  -------------------------------------------------------
  Defines the position of an icon.
*/

ERR_CODE ws_def_icon_pos( WID wid, s_short x, s_short y )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    win->icon_x = ( x >= 1 && x <= screen_max_x - 4 ) ? x : 1;
    win->icon_y = ( y >= 1 && y <= screen_max_y - 2 ) ? y : 1;
    win->icon_moved = 1;
    redraw_screen( win_last->prev_window );
    return( NO_ERR );
}



/*
  ws_set_vscroll
  -------------------------------------------------------
  Sets the block of the vertical scroll bar to the given
  percentage.
*/

ERR_CODE ws_set_vscroll( WID wid, s_char percent )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    if( !win->window_options & VER_SBAR ) {
	return( WS_NOT_ALLOWED );
    }

    if( percent < 0 ) percent = 0;
    if(	percent > 100 ) percent = 100;
    win->scroll_y = percent;
    print_right( win, VIDEO_TESTED );
    return( NO_ERR );
}



/*
  ws_set_hscroll
  -------------------------------------------------------
  Sets the block of the horizontal scroll bar to the given
  percentage.
*/

ERR_CODE ws_set_hscroll( WID wid, s_char percent )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    if( !win->window_options & HOR_SBAR ) {
	return( WS_NOT_ALLOWED );
    }

    if( percent < 0 ) percent = 0;
    if(	percent > 100 ) percent = 100;
    win->scroll_x = percent;
    print_bottom( win, VIDEO_TESTED );
    return( NO_ERR );
}



/*
  ws_draw_dummy_frame
  -------------------------------------------------------
  Draws a dummy frame for the window. The part of the
  window that will be clipped is displayed in an other
  color. A size less than min_dx/dy is set to min_dx/dy.
*/

ERR_CODE ws_draw_dummy_frame( WID wid, s_short x, s_short y,
			      s_short dx, s_short dy, u_char mode )
{
    WIN_LIST_TYPE	*win;
    s_short			i;
    u_short			n_sign, o_sign, n_sign1, o_sign1;
    u_short			val;
    u_short			b_col = 0;

    if( !validate_wid( wid ) ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    o_sign = ( ( b_col + WL_WHITE ) << 8 ) + 220;
    n_sign = ( ( b_col + WL_LIGHTGREEN ) << 8 ) + 220;
    o_sign1 = ( ( b_col + WL_WHITE ) << 8 ) + 223;
    n_sign1 = ( ( b_col + WL_LIGHTGREEN ) << 8 ) + 223;

    ws_restore_mouse();
    get_clips( win );
    for( i = x - 1; i < x + dx + 1; i++ ) {
	if( i < 1 || i > screen_max_x ) {
	    continue;
	}

	if( y - 1 >= 1 ) {
	    if( mode == VIDEO_DIRECT ) {
		val = GET_SCREEN_VAL( i, y - 1 );
	    }
	    else {
		val = GET_BUFFER_VAL( i, y - 1 );
	    }
	    upper_buf[ i - 1 ] = val;
	    b_col = val & 0x7000;
	    if( i >= win->lh_clip && i <= win->hh_clip &&
		y - 1 >= win->lv_clip && y - 1 <= win->hv_clip ) {
		if( mode == VIDEO_DIRECT ) {
		    WRITE_TO_SCREEN( i, y - 1,
				     ( n_sign & 0x0fff ) | b_col);
		}
		else {
		    WRITE_TO_BUFFER( i, y - 1,
				     ( n_sign & 0x0fff ) | b_col);
		}
	    }
	    else {
		if( mode == VIDEO_DIRECT ) {
		    WRITE_TO_SCREEN( i, y - 1,
				     ( o_sign & 0x0fff ) | b_col );
		}
		else {
		    WRITE_TO_BUFFER( i, y - 1,
				     ( o_sign & 0x0fff ) | b_col );
		}
	    }
	}

	if( y + dy <= screen_max_y ) {
	    if( mode == VIDEO_DIRECT ) {
		val = GET_SCREEN_VAL( i, y + dy );
	    }
	    else {
		val = GET_BUFFER_VAL( i, y + dy );
	    }
	    lower_buf[ i - 1 ] = val;
	    b_col = val & 0x7000;
	    if( i >= win->lh_clip && i <= win->hh_clip &&
		y + dy >= win->lv_clip && y + dy <= win->hv_clip ) {
		if( mode == VIDEO_DIRECT ) {
		    WRITE_TO_SCREEN( i, y + dy,
				     ( n_sign1 & 0x0fff ) | b_col );
		}
		else {
		    WRITE_TO_BUFFER( i, y + dy,
				     ( n_sign1 & 0x0fff ) | b_col );
		}
	    }
	    else {
		if( mode == VIDEO_DIRECT ) {
		    WRITE_TO_SCREEN( i, y + dy,
				     ( o_sign1 & 0x0fff ) | b_col );
		}
		else {
		    WRITE_TO_BUFFER( i, y + dy,
				     ( o_sign1 & 0x0fff ) | b_col );
		}
	    }
	}
    }

    o_sign = ( ( b_col + WL_WHITE ) << 8 ) + 219;
    n_sign = ( ( b_col + WL_LIGHTGREEN ) << 8 ) + 219;
    for( i = y; i < y + dy; i++ ) {
	if( i < 1 || i > screen_max_y ) {
	    continue;
	}

	if( x - 1 >= 1 ) {
	    if( mode == VIDEO_DIRECT ) {
		val = GET_SCREEN_VAL( x - 1, i );
	    }
	    else {
		val = GET_BUFFER_VAL( x - 1, i );
	    }
	    left_buf[ i - 1 ] = val;
	    b_col = val & 0x7000;
	    if( i >= win->lv_clip && i <= win->hv_clip &&
		x - 1 >= win->lh_clip && x - 1 <= win->hh_clip ) {
		if( mode == VIDEO_DIRECT ) {
		    WRITE_TO_SCREEN( x - 1, i,
				     ( n_sign & 0x0fff ) | b_col );
		}
		else {
		    WRITE_TO_BUFFER( x - 1, i,
				     ( n_sign & 0x0fff ) | b_col );
		}
	    }
	    else {
		if( mode == VIDEO_DIRECT ) {
		    WRITE_TO_SCREEN( x - 1, i,
				     ( o_sign & 0x0fff ) | b_col );
		}
		else {
		    WRITE_TO_BUFFER( x - 1, i,
				     ( o_sign & 0x0fff ) | b_col );
		}
	    }
	}
	if( x + dx <= screen_max_x ) {
	    if( mode == VIDEO_DIRECT ) {
		val = GET_SCREEN_VAL( x + dx, i );
	    }
	    else {
		val = GET_BUFFER_VAL( x + dx, i );
	    }
	    right_buf[ i - 1 ] = val;
	    b_col = val & 0x7000;
	    if( i >= win->lv_clip && i <= win->hv_clip &&
		x + dx >= win->lh_clip && x + dx <= win->hh_clip ) {
		if( mode == VIDEO_DIRECT ) {
		    WRITE_TO_SCREEN( x + dx, i,
				     ( n_sign & 0x0fff ) | b_col );
		}
		else {
		    WRITE_TO_BUFFER( x + dx, i,
				     ( n_sign & 0x0fff ) | b_col );
		}
	    }
	    else {
		if( mode == VIDEO_DIRECT ) {
		    WRITE_TO_SCREEN( x + dx, i,
				     ( o_sign & 0x0fff ) | b_col );
		}
		else {
		    WRITE_TO_BUFFER( x + dx, i,
				     ( o_sign & 0x0fff ) | b_col );
		}
	    }
	}
    }
    ws_redraw_mouse( MOUSE_ALL );
    return( NO_ERR );
}



/*
  ws_erase_dummy_frame
  -------------------------------------------------------
  Erases the dummy frame.
*/

ERR_CODE ws_erase_dummy_frame( s_short x, s_short y, s_short dx, s_short dy,
			       u_char mode )
{
    s_short			i;
    u_short			val;

    ws_restore_mouse();
    for( i = x - 1; i < x + dx + 1; i++ ) {
	if( i < 1 || i > screen_max_x ) {
	    continue;
	}

	if( y - 1 >= 1 ) {
	    val = upper_buf[ i - 1 ];
	    if( mode == VIDEO_DIRECT ) {
		WRITE_TO_SCREEN( i, y - 1, val );
	    }
	    else {
		WRITE_TO_BUFFER( i, y - 1, val );
	    }
	}

	if( y + dy <= screen_max_y ) {
	    val = lower_buf[ i - 1 ];
	    if( mode == VIDEO_DIRECT ) {
		WRITE_TO_SCREEN( i, y + dy, val );
	    }
	    else {
		WRITE_TO_BUFFER( i, y + dy, val );
	    }
	}
    }

    for( i = y; i < y + dy; i++ ) {
	if( i < 1 || i > screen_max_y ) {
	    continue;
	}

	if( x - 1 >= 1 ) {
	    val = left_buf[ i - 1 ];
	    if( mode == VIDEO_DIRECT ) {
		WRITE_TO_SCREEN( x - 1, i, val );
	    }
	    else {
		WRITE_TO_BUFFER( x - 1, i, val );
	    }
	}
	if( x + dx <= screen_max_x ) {
	    val = right_buf[ i - 1 ];
	    if( mode == VIDEO_DIRECT ) {
		WRITE_TO_SCREEN( x + dx, i, val );
	    }
	    else {
		WRITE_TO_BUFFER( x + dx, i, val );
	    }
	}
    }
    ws_redraw_mouse( MOUSE_ALL );
    return( NO_ERR );
}



/*
  ws_rebuild_screen
  -------------------------------------------------------
  Rebuilds the screen.
*/

void ws_rebuild_screen( void )
{
    redraw_screen( win_last->prev_window );
}



/*
  ws_get_window_info
  -------------------------------------------------------
  Returns all parameter of a window.
*/

ERR_CODE ws_get_window_info( WID wid, WIN_PARAM_TYPE *param, CLIPS_TYPE *cp )
{
    WIN_LIST_TYPE	*win, *tmp;
    if( !validate_wid( wid ) ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    param->tab_step = win->tab_step;
    param->x = win->rel_x; param->y = win->rel_y;
    param->dx = win->dx; param->dy = win->dy;
    param->iconic = win->iconic;
    param->window_options = win->window_options;
    param->border_col = win->border_col;
    param->title_col = win->title_col;
    param->screen_col = win->screen_col;
    param->scroll_col = win->scroll_col;
    param->icon_col = win->icon_col;
    param->icon_x = win->icon_x;
    param->icon_y = win->icon_y;
    param->abs_x = win->x;
    param->abs_y = win->y;
    param->min_dx = win->min_dx;
    param->min_dy = win->min_dy;
    param->father_window = win->father_ptr->window_id;
    tmp = win;
    while( tmp->father_ptr != ROOT_PTR ) {
	tmp = tmp->father_ptr;
    }
    param->super_father = tmp->window_id;
    if( win->window_options & HEADER ) {
	strncpy( param->title, win->title, MAX_HEADER_LEN );
	param->title[ MAX_HEADER_LEN ] = '\0';
	strncpy( param->icon_title, win->icon_title, 4 );
    }
    else {
	param->title[ 0 ] = 0;
	param->icon_title[ 0 ] = 0;
    }
    if( ( win->window_options & BORDER ) == USER_BORDER ) {
	strncpy( param->user_border, win->user_border, 7 );
    }

    get_clips( win );
    cp->lh = win->lh_clip;
    cp->hh = win->hh_clip;
    cp->lv = win->lv_clip;
    cp->hv = win->hv_clip;

    return( NO_ERR );
}



/*
  ws_get_window_clips
  -------------------------------------------------------
  Returns clip parameter of a window.
*/

ERR_CODE ws_get_window_clips( WID wid, CLIPS_TYPE *cp )
{
    WIN_LIST_TYPE	*win;
    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    get_clips( win );
    cp->lh = win->lh_clip;
    cp->hh = win->hh_clip;
    cp->lv = win->lv_clip;
    cp->hv = win->hv_clip;
    cp->window_options = win->window_options;

    return( NO_ERR );
}



/*
  ws_clrscr
  -------------------------------------------------------
  Clears the given window.
*/

ERR_CODE ws_clrscr( WID wid )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    clear_content( win );
    restore_content( win, VIDEO_TESTED );
    ws_gotoxy( wid, 1, 1 );
    return( NO_ERR );
}



/*
  ws_filleol
  -------------------------------------------------------
  Fills to the end of the line in the given window.
*/

ERR_CODE ws_filleol( WID wid, u_char character )
{
    WIN_LIST_TYPE	*win;
    u_short			*ptr;
    u_char			i;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    ptr = &( win->current_content[ ( win->y_pos - 1 ) * win->dx +
				   ( win->x_pos - 1 ) ] );

    for( i = win->x_pos; i <= win->dx; i++ ) {
	*ptr = ( win->attrib << 8 ) + character;
	ptr++;
    }
    restore_content( win, VIDEO_TESTED );
    win->scroll_flag = 0;
    return( NO_ERR );
}



/*
  ws_gotoxy
  -------------------------------------------------------
  Positions the cursor to the given position.
*/

ERR_CODE ws_gotoxy( WID wid, u_char x, u_char y )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    if( x < 1 || y < 1 || x > win->dx || y > win->dy ) {
	return( WS_COORDINATE );
    }

    win->x_pos = x;
    win->y_pos = y;
    ws_set_cursor( cursor_wid );
    win->scroll_flag = 0;
    return( NO_ERR );
}



/*
  ws_get_textattr
  -------------------------------------------------------
  Returns the current textattribute of the window
*/

ERR_CODE ws_get_textattr( WID wid, u_char *attrib )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    *attrib = win->attrib;
    return( NO_ERR );
}



/*
  ws_set_text_attr
  -------------------------------------------------------
  Sets the current textattribute of a window.
*/

ERR_CODE ws_set_textattr( WID wid, u_char attrib )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    win->attrib = attrib;
    return( NO_ERR );
}



/*
  ws_get_crs_size
  -------------------------------------------------------
  Returns the cursor size of the given window.
*/

ERR_CODE ws_get_crs_size( WID wid, u_short *size )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    *size = win->crs_size;
    return( NO_ERR );
}



/*
  ws_set_cursor_size
  -------------------------------------------------------
  Sets the cursor size of the given window.
*/

ERR_CODE ws_set_crs_size( WID wid, u_short size )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    win->crs_size = size;
    ws_set_cursor( cursor_wid );
    return( NO_ERR );
}



/*
  ws_cursor_off
  -------------------------------------------------------
  Switches off the cursor of the given window.
*/

ERR_CODE ws_cursor_off( WID wid )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    win->cursor_on = 0;
    ws_set_cursor( cursor_wid );
    return( NO_ERR );
}



/*
  ws_cursor_on
  -------------------------------------------------------
  Switches on the cursor of the given window.
*/

ERR_CODE ws_cursor_on( WID wid )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    /*
      win = WIN_INFO( wid );
      if( video_type != WL_NOT_SUPPORTED ) {
      switch( video_type ) {
      case WL_CGA:
      win->crs_size = 0x0607;
      break;

      case WL_MGA:
      win->crs_size = 0x0b0c;
      break;

      case WL_EGA_MONO:
      case WL_EGA_COLOR:
      h = 350 / screen_max_y;
      if( h < 14 ) {
      win->crs_size = ( ( h - 1 ) << 8 ) + h;
      }
      else {
      win->crs_size = ( ( h - 3 ) << 8 ) + h - 2;
      }
      break;

      case WL_VGA_MONO:
      case WL_VGA_COLOR:
      h = 400 / screen_max_y;
      if( h < 16 ) {
      win->crs_size = ( ( h - 1 ) << 8 ) + h;
      }
      else {
      win->crs_size = ( ( h - 5 ) << 8 ) + h - 4;
      }
      break;

      case WL_MCGA_MONO:
      case WL_MCGA_COLOR:
      win->crs_size = ( screen_mode == WL_MONO ) ? 0x0b0c : 0x0607;
      break;

      default:
      win->crs_size = 0x0e0f;
      break;
      }
      }
      else {
      if( screen_mode == WL_MONO ) {
      win->crs_size = 0x0b0c;
      }
      else {
      win->crs_size = 0x0607;
      }
      }
      ws_set_cursor( cursor_wid );
      return( NO_ERR );
    */

    win = WIN_INFO( wid );
    win->cursor_on = 1;
    ws_set_cursor( cursor_wid );
    return( NO_ERR );
}



/*
  ws_wherex
  -------------------------------------------------------
  Returns the x-position of the give window.
*/

ERR_CODE ws_wherex( WID wid, u_char *x )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    *x = win->x_pos;
    return( NO_ERR );
}



/*
  ws_wherey
  -------------------------------------------------------
  Returns the y-position of the give window.
*/

ERR_CODE ws_wherey( WID wid, u_char *y )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    *y = win->y_pos;
    return( NO_ERR );
}



/*
  ws_text_mode
  -------------------------------------------------------
  Switches the screen to a new mode.
*/

ERR_CODE ws_text_mode( WL_TEXT_MODE mode )
{
    u_char			x, y, old_dx, old_dy;
    WL_TEXT_MODE	m;
    WIN_LIST_TYPE	*win;

    ws_restore_mouse();
    old_dx = screen_max_x;
    old_dy = screen_max_y;
    if( mode == WL_C4350 && mouse_size == 2 ) {
	ws_reload_font();
	mouse_size = 1;
    }
    hw_text_mode( mode );
    if( mode == WL_C80 && gr_mouse_possible ) {
	mouse_size = 2;
	gr_mouse_init();
    }
    hw_get_screen_info( &x, &y, &screen_max_x, &screen_max_y, &m );
    win = win_array[ ROOT_WINDOW ].window;
    win->dx = screen_max_x; win->dy = screen_max_y;
    free( screen_buf );
    free( screen_map );
    free( upper_buf ); free( lower_buf );
    free( left_buf ); free( right_buf );
    screen_buf = ( u_short* ) malloc( sizeof( u_short ) * screen_max_x *
				      screen_max_y );
    screen_map = ( u_short* ) malloc( screen_max_x * screen_max_y *
				      sizeof( u_short ) );
    upper_buf = ( u_short* ) malloc( sizeof( u_short ) * screen_max_x );
    lower_buf = ( u_short* ) malloc( sizeof( u_short ) * screen_max_x );
    left_buf = ( u_short* ) malloc( sizeof( u_short ) * screen_max_y );
    right_buf = ( u_short* ) malloc( sizeof( u_short ) * screen_max_y );
    if( screen_buf == NULL || screen_map == NULL ) {
	return( WS_MEMORY );
    }

    if( mouse_x > screen_max_x ) mouse_x = screen_max_x;
    if( mouse_y > screen_max_y ) mouse_y = screen_max_y;
    while( win != NULL ) {
	win->icon_x = win->icon_y = 0;
	win = win->prev_window;
    }
    win = win_last->prev_window;

    while( win != NULL ) {
	if( win->dx > screen_max_x ) {
	    win->dx = screen_max_x - 2;
	}
	if( win->dy > screen_max_y ) {
	    win->dy = screen_max_y - 2;
	}
	if( win->orig_dx > screen_max_x ) {
	    win->orig_dx = screen_max_x - 2;
	}
	if( win->orig_dy > screen_max_y ) {
	    win->orig_dy = screen_max_y - 2;
	}
	if( win->x > screen_max_x ) {
	    win->x = screen_max_x - ( old_dx - win->x );
	}
	if( win->y > screen_max_y ) {
	    win->y = screen_max_y - ( old_dy - win->y );
	}
	if( win->rel_x > screen_max_x ) {
	    win->rel_x = screen_max_x - ( old_dx - win->rel_x );
	}
	if( win->rel_y > screen_max_y ) {
	    win->rel_y = screen_max_y - ( old_dy - win->rel_y );
	}
	if( win->orig_x > screen_max_x ) {
	    win->orig_x = screen_max_x - ( old_dx - win->orig_x );
	}
	if( win->orig_y > screen_max_y ) {
	    win->orig_y = screen_max_y - ( old_dy - win->orig_y );
	}
	win = win->prev_window;
    }
    redraw_screen( win_last->prev_window );
    return( NO_ERR );
}



/*
  ws_get_defaults
  -------------------------------------------------------
  Returns the default parameters of the server.
*/

void ws_get_defaults( DEFAULTS_TYPE *d )
{
    bufcpy( d, &def, sizeof( DEFAULTS_TYPE ) );
}



/*
  ws_set_defaults
  -------------------------------------------------------
  Sets the default parameters of the server.
*/

void ws_set_defaults( DEFAULTS_TYPE *d )
{
    bufcpy( &def, d, sizeof( DEFAULTS_TYPE ) );
}



WID ws_last_actwid( void )
{
    return( last_active_wid );
}



WID ws_actwid( void )
{
    return( active_wid );
}



WID ws_cursorwid( void )
{
    return( cursor_wid );
}



ERR_CODE ws_set_actwid( WID wid, WID cwid )
{
    if( !validate_wid( wid ) ) {
	return( WS_WINID );
    }
    if( !validate_wid( cwid ) ) {
	return( WS_WINID );
    }

    last_active_wid = active_wid;
    active_wid = wid;
    cursor_wid = cwid;
    return( NO_ERR );
}



u_char ws_screen_max_x( void )
{
    return( screen_max_x );
}



u_char ws_screen_max_y( void )
{
    return( screen_max_y );
}



u_char ws_screen_mode( void )
{
    return( screen_mode );
}



WID ws_get_cursor_wid( WID wid )
{
    WIN_LIST_TYPE	*win;

    win = WIN_INFO( wid );
    return( win->cursor_wid );
}



WID ws_get_top( void )
{
    WIN_LIST_TYPE	*win;

    win = win_list;
    while( ( win->iconic || win->father_ptr != ROOT_PTR ) &&
	   win->window_id != ROOT_WINDOW ) {
	win = win->next_window;
    }
    return( win->window_id );
}



void ws_set_button( u_short button )
{
    u_short tmp = mouse_button;

    tmp |= ( button & 7 );
    tmp &= ( ( ~button >> 3 ) & 7 );
    mouse_button = ( tmp & 7 );
}



ERR_CODE ws_grap_mouse( WID wid )
{
    s_char			x, y;
    u_char			percent;
    WID				mwid;
    LOCATION		loc;
    WIN_LIST_TYPE   *win;
    WIN_LIST_TYPE	*tmp;

    if( !validate_wid( wid ) ) {
	return( WS_WINID );
    }

    tmp = win = WIN_INFO( wid );
    while( tmp->father_ptr != ROOT_PTR ) {
	tmp = tmp->father_ptr;
    }

    loc = ws_whereis_mouse( &mwid, &x, &y, &percent );
    if( win->window_id != mwid || !mouse_button ||
	tmp->window_id != active_wid ) {
	return( WS_GRAP );
    }

    mouse_graped = 1;
    grap_loc = loc;
    grap_button = mouse_button;
    grap_wid = wid;
    return( NO_ERR );
}



ERR_CODE ws_ungrap_mouse( void )
{
    if( !mouse_graped ) {
	return( WS_UNGRAP );
    }

    mouse_graped = 0;
    return( NO_ERR );
}



ERR_CODE ws_copy_to_window( WID wid, u_char *content )
{
    WIN_LIST_TYPE	*win;

    if( !validate_wid( wid ) || wid == ROOT_WINDOW ) {
	return( WS_WINID );
    }

    win = WIN_INFO( wid );
    bufcpy( win->current_content, content, win->dx * win->dy * 2 );
    restore_content( win, VIDEO_TESTED );
    return( NO_ERR );
}



void activate_window( WID wid, u_char mode )
{
    WID				cwid;
    DEFAULTS_TYPE	def;
    WIN_PARAM_TYPE	params;
    CLIPS_TYPE		cp;

    ws_get_window_info( wid, &params, &cp );
    cwid = ws_get_cursor_wid( params.super_father );

    ws_get_defaults( &def );
    if( def.pas_border != NO_CHANGE ) {
	if( def.pas_border == USER_BORDER ) {
	    ws_define_border( ws_actwid(), def.pas_uborder );
	}
	ws_modify_options( ws_actwid(), BORDER, def.pas_border );
    }

    if( params.father_window == ROOT_WINDOW ) {
	if( wid != ROOT_WINDOW ) {
	    ws_raise_window( wid, mode );
	}
	ws_set_actwid( wid, cwid );
    }
    else {
	do {
	    ws_get_window_info( wid, &params, &cp );
	    if( params.window_options & ALLOW_RAISE ) {
		ws_raise_window( wid, NO_DISPLAY );
	    }
	    if( params.father_window != ROOT_WINDOW ) {
		wid = params.father_window;
	    }
	} while( params.father_window != ROOT_WINDOW );
	if( mode == DISPLAY ) {
	    ws_rebuild_screen();
	}
	ws_set_actwid( wid, cwid );
    }

    if( ws_actwid() != ROOT_WINDOW && def.act_border != NO_CHANGE ) {
	if( def.act_border == USER_BORDER ) {
	    ws_define_border( ws_actwid(), def.act_uborder );
	}
	ws_modify_options( ws_actwid(), BORDER, def.act_border );
    }
    ws_set_cursor( cwid );
}



#ifdef __TURBOC__

ERR_CODE ws_activate_wid( WID wid )
{
    WIN_LIST_TYPE	*win;
    WIN_PARAM_TYPE	params;
    DEFAULTS_TYPE	def;
    CLIPS_TYPE		cp;
    WID				cwid;

    if( !validate_wid( wid ) ) {
	return( WS_WINID );
    }

    ws_get_window_info( wid, &params, &cp );
    if( params.super_father == ws_actwid() ) {
	return( NO_ERR );
    }

    ws_get_defaults( &def );
    cwid = ws_get_cursor_wid( params.super_father );
    if( def.pas_border != NO_CHANGE ) {
	if( def.pas_border == USER_BORDER ) {
	    ws_define_border( ws_actwid(), def.pas_uborder );
	}
	ws_modify_options( ws_actwid(), BORDER, def.pas_border );
    }
    ws_set_actwid( wid, cwid );
    if( ws_actwid() != ROOT_WINDOW && def.act_border != NO_CHANGE ) {
	if( def.act_border == USER_BORDER ) {
	    ws_define_border( ws_actwid(), def.act_uborder );
	}
	ws_modify_options( ws_actwid(), BORDER, def.act_border );
    }
    ws_set_cursor( cwid );
    return( NO_ERR );
}



ERR_CODE ws_save_dos( void )
{
    WIN_LIST_TYPE	*win;
    u_char			x_pos, y_pos;
    u_char			screen_max_x, screen_max_y;
    WL_TEXT_MODE	screen_mode;

    hw_get_screen_info( &x_pos, &y_pos, &screen_max_x, &screen_max_y,
			&screen_mode );
    dos_screen = ( u_short* ) malloc( screen_max_x * screen_max_y * 2 );
    if( !dos_screen ) {
	return( WS_MEMORY );
    }

    hw_gettext( 1, 1, screen_max_x, screen_max_y, dos_screen );
    return( NO_ERR );
}



ERR_CODE ws_restore_dos( void )
{
    WIN_LIST_TYPE	*win;

    win = win_array[ 0 ].window;
    hw_puttext( 1, 1, screen_max_x, screen_max_y, dos_screen );
    hw_gotoxy( win->x_pos, win->y_pos );
    hw_set_cursor_size( win->crs_size );
    free( dos_screen );
    return( NO_ERR );
}

#endif



#ifdef __HERMES__

/*
  ws_dup_window
  -------------------------------------------------------
  Duplicates a given window handle by incrementing the
  reference counter of the window itself and all of its
  father windows.
*/

ERR_CODE ws_dup_window( WID *wid )
{
    WIN_LIST_TYPE	*win;
    u_int			w_index, i;
    PID				pid;

    if( !validate_wid( *wid ) || ( *wid == ROOT_WINDOW ) ) {
	return( WS_WINID );
    }

    win = WIN_INFO( *wid );
    while( win->window_id != ROOT_WINDOW ) {
	win->ref_cnt++;
	win = win->father_ptr;
    }

    w_index = -1;
    for( i = 0; i < max_dup_num; i++ ) {
	if( dup_array[ i ].window == NULL ) {
	    w_index = i;
	    break;
	}
    }
    if( w_index == -1 ) {
	dup_array = (WIN_ARRAY_TYPE*) realloc( (void*) dup_array,
					       ( max_dup_num + 10 ) *
					       sizeof( WIN_ARRAY_TYPE ) );
	for( i = 0; i < 10; i++ ) {
	    dup_array[ max_dup_num + i ].window = NULL;
	    dup_array[ max_dup_num + i ].seq_cnt = 0;
	}
	w_index = max_dup_num;
	max_dup_num += 10;
    }

    if( ( *wid & 0xffffff ) < max_window_num ) {
	pid = win_array[ *wid & 0xffffff ].pid;
    }
    else {
	pid = dup_array[ ( *wid & 0xffffff ) - max_window_num ].pid;
    }
    dup_array[ w_index ].seq_cnt++;
    dup_array[ w_index ].window = WIN_INFO( *wid );
    dup_array[ w_index ].pid = pid;
    *wid = ( (u_int) dup_array[ w_index ].seq_cnt << 24 ) +
	w_index + max_window_num;
    return( NO_ERR );
}



/*
  ws_remove_windows
  -------------------------------------------------------
  Removes all windows corresponding to a given pid.
*/

void ws_remove_windows( PID pid )
{
    u_int		i;

    for( i = 1; i < max_window_num; i++ ) {
	if( win_array[ i ].pid == pid && win_array[ i ].window != NULL ) {
	    ws_close_window( win_array[ i ].window->window_id, NO_DISPLAY );
	}
    }
    for( i = 0; i < max_dup_num; i++ ) {
	if( dup_array[ i ].pid == pid && dup_array[ i ].window != NULL ) {
	    ws_close_window( dup_array[ i ].window->window_id, NO_DISPLAY );
	}
    }

    redraw_screen( win_last->prev_window );
}



ERR_CODE ws_chown_handle( WID wid, PID pid )
{
    if( !validate_wid( wid ) ) {
	return( WS_WINID );
    }

    if( ( wid & 0xffffff ) < max_window_num ) {
	win_array[ wid & 0xffffff ].pid = pid;
	return( NO_ERR );
    }
    else {
	dup_array[ ( wid & 0xffffff ) - max_window_num ].pid = pid;
	return( NO_ERR );
    }
}

#endif
