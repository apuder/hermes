#ifdef __TURBOC__

#include <stdlib.h>
#include <dos.h>

#endif

#ifdef __GNUC__

#include <cinclude.h>

#endif


#include <ci_types.h>
#include <winlib.h>
#include "win_serv.h"
#include "ws_intrn.h"
#include "specials.h"

#ifdef __TURBOC__
extern unsigned		_stklen = 16384;
#endif

u_int 		max_window_num;
u_int		max_dup_num = 0;
u_short		*screen_map;		/* Map: WWWWWWWW RMSLLLLL */
u_short		*screen_buf;		/* Buffer for redraw */
u_short		*video_base;		/* Base address of the VideoRam */
WIN_ARRAY_TYPE	*win_array;          	/* window table */
WIN_ARRAY_TYPE	*dup_array = NULL;	/* dup handle table */
WIN_LIST_TYPE	*win_list, *win_last;	/* first & last window */
u_short		*scr_image;     	/* Image for screenbackground */
u_char		img_dx = 1;		/* size of the screen */
u_char		img_dy = 1;		/* image */
u_char		init_ok = FALSE;
WID		active_wid = ROOT_WINDOW;
WID		last_active_wid = ROOT_WINDOW;
WID		cursor_wid = ROOT_WINDOW;
u_char		screen_max_x, screen_max_y;
WL_TEXT_MODE	screen_mode;
WL_VIDEO_TYPE	video_type;
u_short		*upper_buf, *lower_buf;
u_short		*left_buf, *right_buf;
s_short		mouse_x, mouse_y;
u_char		gr_mouse_possible = 1;
u_char		allow_grmouse = 0;
u_char		mouse_size = 2;		/* size of mouse cursor */
u_short		mouse_and = 0xffff;	/* possible sizes are 1 */
u_short		mouse_xor = 0x4000;	/* and 2, others don't work */
s_short		old_mx = -1;
s_short 	old_my = -1;
u_char		mouse_button = 0;
u_char		mouse_installed = 1;
u_short		mouse_save[ 4 ];
s_short		mouse_px, mouse_py;
u_char		mouse_graped = 0;
WID		grap_wid;
LOCATION	grap_loc;
u_char		grap_button;
u_char		server_blocked = 0;
DEFAULTS_TYPE	def;

#ifdef __TURBOC__

u_short		*dos_screen;

#endif

DEFAULTS_TYPE		def_color = {
    0xffff, 0x4000,
    DEF_SCREEN_COL, DEF_BORDER_COL, DEF_TITLE_COL,
    DEF_BUTTON_COL, DEF_SCROLL_COL, DEF_ICON_COL,
    THICK_BORDER, THIN_BORDER,
    "      ", "      "
};

DEFAULTS_TYPE		def_mono = {
    0x0000, 0x022a,
    2, 2, 12, 12, 2, 2,
    THICK_BORDER, THIN_BORDER,
    "      ", "      "
};



/*
  validate_wid
  -------------------------------------------------------
  Validates a given window id.
*/

int validate_wid( WID wid )
{
    if( ( wid & 0xffffff ) < max_window_num ) {
	return( ( wid >> 24 ) == win_array[ wid & 0xffffff ].seq_cnt &&
		win_array[ wid & 0xffffff ].window != NULL );
    }
    else {
	if( ( wid & 0xffffff ) - max_window_num >= max_dup_num ) {
	    return( FALSE );
	}
	return( ( wid >> 24 ) ==
		dup_array[ ( wid & 0xffffff ) - max_window_num ].seq_cnt &&
		dup_array[ ( wid & 0xffffff ) - max_window_num ].window != NULL );
    }
}



/*
  check_coordinates
  -------------------------------------------------------
  Checks wether the given coordinates are within the
  given window.
*/

u_int check_coordinates( WIN_LIST_TYPE *win, u_int x, u_int y,
			 u_int dx, u_int dy )
{
    if( x < 1 || y < 1 || x + dx - 1 < 1 || y + dy - 1 < 1 ) {
	return( 0 );
    }
    if( x > win->dx || y > win->dy || x + dx - 1 > win->dx ||
	y + dy - 1 > win->dy ) {
	return( 0 );
    }

    return( 1 );
}



/*
  get_defcursor
  -------------------------------------------------------
  Computes the size of the text cursor independant of the
  video mode (underlining cursor)
*/

u_short compute_cursorsize( u_short start, u_short stop )
{
    u_short	scan_lines;

    if( start == 0 && stop == 0 ) {			/* Switch cursor off */
	return( 0x2021 );
    }

    switch( video_type ) {
    case WL_CGA:
	scan_lines = 8;
	break;

    case WL_MGA:
	scan_lines = 13;
	break;

    case WL_EGA_MONO:
    case WL_EGA_COLOR:
	scan_lines = 350 / screen_max_y;
	break;

    case WL_VGA_MONO:
    case WL_VGA_COLOR:
	scan_lines = 400 / screen_max_y;
	break;

    case WL_MCGA_MONO:
    case WL_MCGA_COLOR:
	scan_lines = ( screen_mode == WL_MONO ) ? 13 : 8;
	break;

    default:
	scan_lines = 16;
	break;
    }
    start = scan_lines * 10 * start / 100;
    start = ( start + 5 ) / 10;
    stop = scan_lines * 10 * stop / 100;
    stop = ( stop + 5 ) / 10;
    return( ( start << 8 ) + stop );
}



/*
  mouse_in_window
  -------------------------------------------------------
  Tests wether the mouse cursor is within the given win-
  dow.
*/

u_char mouse_in_window( WID wid )
{
    WIN_LIST_TYPE	*win;
    u_short			m1, m2, m3, m4;
    u_short			ref;

    win = WIN_INFO( wid );
    ref = ( win->window_id << 8 ) + WINDOW_AREA;

    m1 = GET_MAP_VAL( mouse_x, mouse_y ) & 0xff3f;
    if( mouse_size == 1 ) {
	return( m1 == ref );
    }

    m2 =  GET_MAP_VAL( mouse_x + 1, mouse_y ) & 0xff3f;
    m3 =  GET_MAP_VAL( mouse_x, mouse_y + 1 ) & 0xff3f;
    m4 =  GET_MAP_VAL( mouse_x + 1, mouse_y + 1 ) & 0xff3f;
    m1 = ( m1 == ref || m2 == ref || m3 == ref || m4 == ref );
    return( m1 );
}



/*	get_next_wpos
	-------------------------------------------------------
	calculates the next position for windows with default
	position. Windows are cascaded up to the middle of the
	screen.
*/

void get_next_wpos( s_short *x, s_short *y, s_short *dx, s_short *dy )
{
    WIN_LIST_TYPE	*win;
    s_short			next_x, next_y;
    s_int			found;

    next_x = 1; next_y = 1;
    found = 0;
    while( next_x < screen_max_x - 1 && !found ) {
	win = win_list;
	next_x = ( next_x < screen_max_x - 1 ) ? next_x + 1 : next_x;
	next_y = ( next_y < screen_max_y - 4 ) ? next_y + 1 : next_y;
	while( win ) {
	    found = 1;
	    if( win->orig_x == next_x && win->orig_y == next_y &&
		win->win_level == 1 ) {
		found = 0;
		break;
	    }
	    win = win->next_window;
	}
    }

    *x = next_x;
    *y = next_y;
    *dx = screen_max_x - next_x;
    *dy = screen_max_y - next_y - 3;
}



/*
  restore_content
  -------------------------------------------------------
  Restores the content of the window buffer.
*/

void restore_content( WIN_LIST_TYPE *win, u_char mode )
{
    s_short 	i, j, new_val;
    u_char		visi;
    MOUSE_CHAR	ms;

    /* Compute the visible part of the window */

    for( j = win->y; j < win->y + win->dy; j++ ) {
	for( i = win->x; i < win->x + win->dx; i++	) {
	    if( i < win->lh_clip || i > win->hh_clip ||
		j < win->lv_clip || j > win->hv_clip ) {
		continue;
	    }

	    switch( mode ) {
	    case INTO_BUFFER:
		WRITE_TO_BUFFER( i, j,
				 win->current_content[ win->dx * ( j - win->y ) +
						       ( i - win->x ) ] );
		WRITE_TO_MAP( i, j, ( win->window_id << 8 ) + WINDOW_AREA );
		break;

	    case VIDEO_DIRECT:
		WRITE_TO_SCREEN( i, j,
				 win->current_content[ win->dx * ( j - win->y ) +
						       ( i - win->x ) ] );
		WRITE_TO_MAP( i, j, ( win->window_id << 8 ) + WINDOW_AREA );
		break;

	    case VIDEO_TESTED:
		visi = coordinate_visible( win->window_id, i, j, &ms );
		if( visi == VISIBLE ) {
		    WRITE_TO_SCREEN( i, j,
				     win->current_content[ win->dx *
							   ( j - win->y ) + ( i - win->x ) ] );
		}
		else if( visi == SHADOWED ) {
		    new_val = win->current_content[ win->dx * ( j - win->y ) +
						    ( i - win->x ) ];
		    new_val = ( new_val & 0xff ) | ( WL_SHADOW_COL << 8 );
		    WRITE_TO_SCREEN( i, j, new_val );
		}
		if( ms && visi != INVISIBLE ) {
		    ws_redraw_mouse( ms );
		}
		break;
	    }
	}
    }
}



/*
  coordinate_visible
  -------------------------------------------------------
  Checks wether the given corrdinate of a given window is
  visible or not. Further it checks wether the mouse points
  to this position.
*/

u_char coordinate_visible( WID wid, s_short x, s_short y, MOUSE_CHAR *ms )
{
    u_short 		map_val;
    WIN_LIST_TYPE	*win;

    if( x < 1 || x > screen_max_x || y < 1 || y > screen_max_y ) {
	*ms = NO_MOUSE;
	return( INVISIBLE );
    }

    win = WIN_INFO( wid );
    map_val = GET_MAP_VAL( x, y );
    if( map_val & MOUSE_MASK ) {
	*ms = ( ( y > mouse_y ) << 1 ) + ( x > mouse_x ) + 1;
    }
    else {
	*ms = 0;
    }
    if( ( map_val & 0xff00 ) >> 8 != ( win->window_id & 0xff ) ) {
	return( INVISIBLE );
    }
    if( map_val & SHADOW ) {
	return( SHADOWED );
    }
    return( VISIBLE );
}



/*
  print_shadow
  -------------------------------------------------------
  Prints the shadow of the given window.
*/

void print_shadow( WIN_LIST_TYPE *win, u_char mode )
{
    s_char	shadow_height;
    s_char	shadow_width;
    u_short chr;
    u_int	i;

    if( !( win->window_options & PRINT_SHADOW ) ||
	!( win->window_options & BORDER ) ) {
	return;
    }

    shadow_width = win->hh_clip - ( win->x + win->dx );
    if( shadow_width > 0 ) {
	shadow_width = ( shadow_width >= 2 ) ? 2 : 1;
    }
    if( win->x + win->dx < win->lh_clip ) {
	shadow_width = 0;
    }

    if( win->y > win->hv_clip ) {
	shadow_height = 0;
    }
    else {
	shadow_height = win->hv_clip - win->y + 1;
	shadow_height = ( shadow_height > win->dy + 2 ) ?
	    win->dy + 2 :
	    shadow_height;
    }

    for( i = 0; i < shadow_height; i++ ) {
	if( win->y + i < win->lv_clip ) {
	    continue;
	}

	switch( mode ) {

	case VIDEO_DIRECT:
	    if( shadow_width > 0 ) {
		chr = GET_SCREEN_VAL( win->x + win->dx + 1, win->y + i );
		chr = ( chr & 0x80ff ) | 0x0700;
		WRITE_TO_SCREEN( win->x + win->dx + 1, win->y + i, chr );
	    }
	    if( shadow_width > 1 ) {
		chr = GET_SCREEN_VAL( win->x + win->dx + 2, win->y + i );
		chr = ( chr & 0x80ff ) | 0x0700;
		WRITE_TO_SCREEN( win->x + win->dx + 2, win->y + i, chr );
	    }
	    break;

	case INTO_BUFFER:
	    if( shadow_width > 0 ) {
		chr = GET_BUFFER_VAL( win->x + win->dx + 1, win->y + i );
		chr = ( chr & 0x80ff ) | 0x0700;
		WRITE_TO_BUFFER( win->x + win->dx + 1, win->y + i, chr );
	    }
	    if( shadow_width > 1 ) {
		chr = GET_BUFFER_VAL( win->x + win->dx + 2, win->y + i );
		chr = ( chr & 0x80ff ) | 0x0700;
		WRITE_TO_BUFFER( win->x + win->dx + 2, win->y + i, chr );
	    }
	    break;
	}
	if( shadow_width > 0 ) {
	    chr = GET_MAP_VAL( win->x + win->dx + 1, win->y + i );
	    chr |= SHADOW;
	    WRITE_TO_MAP( win->x + win->dx + 1, win->y + i, chr );
	}
	if( shadow_width > 1 ) {
	    chr = GET_MAP_VAL( win->x + win->dx + 2, win->y + i );
	    chr |= SHADOW;
	    WRITE_TO_MAP( win->x + win->dx + 2, win->y + i, chr );
	}
    }

    if( win->y + win->dy + 1 > win->hv_clip ||
	win->y + win->dy + 1 < win->lv_clip ) {
	return;
    }
    for( i = 0; i < win->dx; i++ ) {
	if( win->x + 1 + i < win->lh_clip || win->x + 1 + i > win->hh_clip ) {
	    continue;
	}
	switch( mode ) {

	case INTO_BUFFER:
	    chr = GET_BUFFER_VAL( win->x + 1 + i, win->y + win->dy + 1 );
	    chr = ( chr & 0x80ff ) | 0x0700;
	    WRITE_TO_BUFFER( win->x + 1 + i, win->y + win->dy + 1, chr );
	    break;

	case VIDEO_DIRECT:
	    chr = GET_SCREEN_VAL( win->x + 1 + i, win->y + win->dy + 1 );
	    chr = ( chr & 0x80ff ) | 0x0700;
	    WRITE_TO_SCREEN( win->x + 1 + i, win->y + win->dy + 1, chr );
	    break;
	}
	chr = GET_MAP_VAL( win->x + 1 + i, win->y + win->dy + 1 );
	chr |= SHADOW;
	WRITE_TO_MAP( win->x + 1 + i, win->y + win->dy + 1, chr );
    }
}



/*
  clear_content
  -------------------------------------------------------
  Clears the window buffer of a window.
*/

void clear_content( WIN_LIST_TYPE *win )
{
    u_short value;
    u_short elems, i;
    u_short *buf_ptr;

    elems = win->dx * win->dy;
    buf_ptr = ( u_short* ) win->current_content;
    value = 256 * win->attrib + 32;
    for( i = 0; i < elems; i++ ) {
	*( buf_ptr++ ) = value;
    }
}



/*
  get_clips
  -------------------------------------------------------
  Computes the absolute coordinate, where the given
  window has to be clipped.
*/

void get_clips( WIN_LIST_TYPE *win )
{
    u_char			lh_clip, hh_clip, lv_clip, hv_clip;


    if( win->window_options & FATHER_CLIP ) {
	lh_clip = ( win->father_ptr->x < win->father_ptr->lh_clip ) ?
	    win->father_ptr->lh_clip :
	    win->father_ptr->x;
	hh_clip = ( win->father_ptr->x + win->father_ptr->dx - 1 >
		    win->father_ptr->hh_clip ) ?
	    win->father_ptr->hh_clip :
	    win->father_ptr->x + win->father_ptr->dx - 1;
	lv_clip = ( win->father_ptr->y < win->father_ptr->lv_clip ) ?
	    win->father_ptr->lv_clip :
	    win->father_ptr->y;
	hv_clip = ( win->father_ptr->y + win->father_ptr->dy - 1 >
		    win->father_ptr->hv_clip ) ?
	    win->father_ptr->hv_clip :
	    win->father_ptr->y + win->father_ptr->dy - 1;
    }
    else {
	lh_clip = win_array[ ROOT_WINDOW ].window->x;
	hh_clip = win_array[ ROOT_WINDOW ].window->x +
	    win_array[ ROOT_WINDOW ].window->dx - 1;
	lv_clip = win_array[ ROOT_WINDOW ]. window->y;
	hv_clip = win_array[ ROOT_WINDOW ].window->y +
	    win_array[ ROOT_WINDOW ].window->dy - 1;
    }
    win->lh_clip = lh_clip;
    win->hh_clip = hh_clip;
    win->lv_clip = lv_clip;
    win->hv_clip = hv_clip;
}



/*
  print_header
  -------------------------------------------------------
  Prints out a window's header including all buttons.
*/

void print_header( WIN_LIST_TYPE *win, u_char mode )
{
    MOUSE_CHAR		ms;
    u_char			header_type;
    u_char			border_type;
    u_char			header_pos;
    u_short 		border_base, title_base, button_base;
    u_short 		map_base, new_val;
    u_char			*s, visi, len;
    s_short 		i, first_x, last_x;
    u_short 		tmp_map[ MAX_DX ];
    u_short 		tmp_buf[ MAX_DX ];
    u_short 		*buf_ptr;
    u_short 		*map_ptr;


    if( !( border_type = ( win->window_options & BORDER ) ) ||
	win->y <= win->lv_clip ||
	win->y - 1 > win->hv_clip ||
	win->x - 1 > win->hh_clip ) {
	return;
    }

    first_x = ( win->x <= win->lh_clip ) ? abs( win->x - win->lh_clip - 1 ) : 0;
    last_x = win->hh_clip - win->x + 1;
    last_x = ( last_x > win->dx + 1 ) ? win->dx + 1 : last_x;
    s = ( u_char* ) win->user_border;
    border_base = 256 * win->border_col;
    title_base = 256 * win->title_col;
    button_base = 256 * win->button_col;
    map_base = ( win->window_id & 0xff ) << 8;
    header_type = ( win->window_options & 192 ) >> 6;

    buf_ptr = tmp_buf;
    map_ptr = tmp_map;
    switch( border_type ) {
    case 1:
	*( buf_ptr++ ) = ( border_base + 218 );
	*( map_ptr++ ) = map_base + TOP_LEFT ;
	for( i = 0; i < win->dx; i++ ) {
	    *( buf_ptr++ ) = border_base + 196;
	    *( map_ptr++ ) = map_base + UPPER_BORDER;
	}
	*( buf_ptr++ ) = border_base + 191;
	*( map_ptr++ ) = map_base + TOP_RIGHT;
	break;

    case 2:
	*( buf_ptr++ ) = border_base + 201;
	*( map_ptr++ ) = map_base + TOP_LEFT ;
	for( i = 0; i < win->dx; i++ ) {
	    *( buf_ptr++ ) = border_base + 205;
	    *( map_ptr++ ) = map_base + UPPER_BORDER;
	}
	*( buf_ptr++ ) = border_base + 187;
	*( map_ptr++ ) = map_base + TOP_RIGHT;
	break;

    case 3:
	*( buf_ptr++ ) = border_base + s[ 0 ];
	*( map_ptr++ ) = map_base + TOP_LEFT ;
	for( i = 0; i < win->dx; i++ ) {
	    *( buf_ptr++ ) = border_base + s[ 4 ];
	    *( map_ptr++ ) = map_base + UPPER_BORDER;
	}
	*( buf_ptr++ ) = border_base + s[ 1 ];
	*( map_ptr++ ) = map_base + TOP_RIGHT;
	break;
    }


    if( win->window_options & PRINT_AUX ) {
	buf_ptr = &( tmp_buf[ 2 ] );
	map_ptr = &( tmp_map[ 2 ] );
	*( map_ptr ) = *( map_ptr + 1 ) = *( map_ptr + 2 ) = map_base + AUX_BUTTON;
	*( buf_ptr++ ) = border_base + '[';
	*( buf_ptr++ ) = button_base + 254;
	*( buf_ptr++ ) = border_base + ']';
    }

    if( win->window_options & ALLOW_ZOOM ) {
	buf_ptr = &( tmp_buf[ win->dx - 3 ] );
	map_ptr = &( tmp_map[ win->dx - 3 ] );
	if( win->full_screen ) {
	    *( map_ptr ) = *( map_ptr + 1 ) = *( map_ptr + 2 ) =
		map_base + UNZOOM_BUTTON;
	}
	else {
	    *( map_ptr ) = *( map_ptr + 1 ) = *( map_ptr + 2 ) =
		map_base + ZOOM_BUTTON;
        }
	*( buf_ptr++ ) = border_base + '[';
	if( win->full_screen ) {
	    *( buf_ptr++ ) = button_base + 18;
	}
	else {
	    *( buf_ptr++ ) = button_base + 24;
	}
	*( buf_ptr++ ) = border_base + ']';
    }

    header_pos = ( win->window_options & ALLOW_ZOOM ) ? 7 : 3;
    if( win->window_options & ALLOW_ICON ) {
	buf_ptr = &( tmp_buf[ win->dx - header_pos ] );
	map_ptr = &( tmp_map[ win->dx - header_pos ] );
	*( map_ptr ) = *( map_ptr + 1 ) = *( map_ptr + 2 ) = map_base + ICON_BUTTON;
	*( buf_ptr++ ) = border_base + '[';
	*( buf_ptr++ ) = button_base + 25;
	*( buf_ptr++ ) = border_base + ']';
    }

    len = win->dx - 2;
    if( win->window_options & PRINT_AUX ) len -= 4;
    if( win->window_options & ALLOW_ICON ) len -= 4;
    if( win->window_options & ALLOW_ZOOM ) len -= 4;
    len = ( len > strlen( win->title ) ) ? strlen( win->title ) : len;
    switch( header_type ) {
    case 1:
	if( win->window_options & AUX_BUTTON ) {
	    buf_ptr = &( tmp_buf[ 6 ] );
	}
	else {
	    buf_ptr = &( tmp_buf[ 2 ] );
	}
	break;

    case 2:
	if( win->window_options & ALLOW_ICON &&
	    win->window_options & ALLOW_ZOOM ) {
	    buf_ptr = &( tmp_buf[ win->dx - 9 - len ] );
	    break;
	}
	if( win->window_options & ALLOW_ZOOM ||
	    win->window_options & ALLOW_ICON ) {
	    buf_ptr = &( tmp_buf[ win->dx - 5 - len ] );
	    break;
	}
	buf_ptr = &( tmp_buf[ win->dx - 1 - len ] );
	break;

    case 0:
    case 3:
	header_pos = win->dx / 2 - len / 2 + 1;
	if( win->window_options & ALLOW_ZOOM &&
	    win->window_options & ALLOW_ICON ) {
	    header_pos -= 2;
	}
	if( !( win->window_options & PRINT_AUX ) ) {
	    header_pos -= 2;
	}
	buf_ptr = &( tmp_buf[ header_pos ] );
	break;
    }
    s = win->title;
    for( i = 0; i < len; i++ ) {
	*( buf_ptr++ ) = title_base + s[ i ];
    }
    for( i = first_x; i <= last_x; i++ ) {
	switch( mode ) {
	case INTO_BUFFER:
	    WRITE_TO_BUFFER( win->x - 1 + i, win->y - 1, tmp_buf[ i ] );
	    WRITE_TO_MAP( win->x - 1 + i, win->y - 1, tmp_map[ i ] );
	    break;

	case VIDEO_DIRECT:
	    WRITE_TO_SCREEN( win->x - 1 + i, win->y - 1, tmp_buf[ i ] );
	    WRITE_TO_MAP( win->x - 1 + i, win->y - 1, tmp_map[ i ] );
	    break;

	case VIDEO_TESTED:
	    visi = coordinate_visible( win->window_id,
				       win->x - 1 + i, win->y - 1, &ms );
	    if( visi == VISIBLE ) {
		WRITE_TO_SCREEN( win->x - 1 + i, win->y - 1,
				 tmp_buf[ i ] );
	    }
	    else if( visi == SHADOWED ) {
		new_val = tmp_buf[ i ];
		new_val = ( new_val & 0x80ff ) | ( WL_SHADOW_COL << 8 );
		WRITE_TO_SCREEN( win->x - 1 + i, win->y - 1, new_val );
	    }
	    if( ms && visi != INVISIBLE ) {
		ws_redraw_mouse( ms );
	    }
	    break;
	}
    }
    return;
}



/*
  print_left
  -------------------------------------------------------
  Prints out the left border of a window.
*/

void print_left( WIN_LIST_TYPE * win, u_char mode )
{
    MOUSE_CHAR		ms;
    s_short 		i;
    u_char			border_type;
    u_char			*s, visi;
    u_short 		border_base;
    u_short 		map_base, new_val;
    u_short 		*buf_ptr;
    u_short 		*map_ptr;
    u_short 		tmp_buf[ MAX_DY ];
    u_short 		tmp_map[ MAX_DY ];
    u_short 		first_y, last_y;

    if( !( border_type = ( win->window_options & BORDER ) ) ||
	win->x <= win->lh_clip ||
	win->x - 1 > win->hh_clip ||
	win->y > win->hv_clip ) {
	return;
    }

    first_y = ( win->y < win->lv_clip ) ? abs( win->y - win->lv_clip ) : 0;
    last_y = win->hv_clip - win->y;
    last_y = ( last_y > win->dy - 1 ) ? win->dy - 1 : last_y;
    map_base = ( win->window_id & 0xff ) << 8;
    border_base = 256 * win->border_col;
    buf_ptr = tmp_buf;
    map_ptr = tmp_map;
    s = ( u_char* ) win->user_border;
    for( i = 0; i < win->dy; i++ ) {
	*( map_ptr++ ) = map_base + LEFT_BORDER;
	switch( border_type ) {
	case 1:
	    *( buf_ptr++ ) = border_base + 179;
	    break;

	case 2:
	    *( buf_ptr++ ) = border_base + 186;
	    break;

	case 3:
	    *( buf_ptr++ ) = border_base + s[ 5 ];
	    break;
	}
    }
    for( i = first_y; i <= last_y; i++ ) {
	switch( mode ) {
	case INTO_BUFFER:
	    WRITE_TO_BUFFER( win->x - 1, win->y + i, tmp_buf[ i ] );
	    WRITE_TO_MAP( win->x - 1, win->y + i, tmp_map[ i ] );
	    break;

	case VIDEO_DIRECT:
	    WRITE_TO_SCREEN( win->x - 1, win->y + i, tmp_buf[ i ] );
	    WRITE_TO_MAP( win->x - 1, win->y + i, tmp_map[ i ] );
	    break;

	case VIDEO_TESTED:
	    visi = coordinate_visible( win->window_id,
				       win->x - 1, win->y + i, &ms );
	    if( visi == VISIBLE ) {
		WRITE_TO_SCREEN( win->x - 1, win->y + i, tmp_buf[ i ] );
	    }
	    else if( visi == SHADOWED ) {
		new_val = tmp_buf[ i ];
		new_val = ( new_val & 0x80ff ) | ( WL_SHADOW_COL << 8 );
		WRITE_TO_SCREEN( win->x - 1, win->y + i, new_val );
	    }
	    if( ms && visi != INVISIBLE ) {
		ws_redraw_mouse( ms );
	    }
	    break;
	}
    }
}



/*
  print_right
  -------------------------------------------------------
  Prints out the right border of a window and the verti-
  cal scroll bar. If the depth of the window is less than
  4 no scroll bar is printed.
*/

void print_right( WIN_LIST_TYPE *win, u_char mode )
{
    MOUSE_CHAR		ms;
    u_char			border_type, block_pos;
    u_char			*s, visi;
    u_short 		*buf_ptr;
    u_short 		*map_ptr;
    u_short 		tmp_buf[ MAX_DY ];
    u_short 		tmp_map[ MAX_DY ];
    u_short 		first_y, last_y;
    u_short 		map_base, new_val;
    u_short 		border_base;
    u_short 		scroll_base;
    s_int			i;

    if( !( border_type = ( win->window_options & BORDER ) ) ||
	win->x + win->dx > win->hh_clip ||
	win->x + win->dx < win->lh_clip ||
	win->y > win->hv_clip ) {
	return;
    }

    block_pos = ( u_char ) ( win->scroll_y + 1 ) * ( win->dy - 3 ) / 100 + 1;
    first_y = ( win->y < win->lv_clip ) ? abs( win->y - win->lv_clip ) : 0;
    last_y = win->hv_clip - win->y;
    last_y = ( last_y > win->dy - 1 ) ? win->dy - 1 : last_y;
    buf_ptr = tmp_buf;
    map_ptr = tmp_map;
    map_base = ( win->window_id & 0xff ) << 8;
    border_base = 256 * win->border_col;
    scroll_base = 256 * win->scroll_col;
    s = ( u_char* ) win->user_border;

    for( i = 0; i < win->dy; i++ ) {
	if( !( win->window_options & VER_SBAR ) || ( win->dy < 4 ) ) {
	    *map_ptr = map_base + RIGHT_BORDER;
	    switch( border_type ) {
	    case 1:
		*buf_ptr = border_base + 179;
		break;

	    case 2:
		*buf_ptr = border_base + 186;
		break;

	    case 3:
		*buf_ptr = border_base + s[ 5 ];
		break;
	    }
	}
	else {
	    if( i == 0 ) {
		*buf_ptr = scroll_base + 30;
		*map_ptr = map_base + SCROLL_UP;
	    } else
		if( i == block_pos ) {
		    *buf_ptr = scroll_base + 254;
		    *map_ptr = map_base + V_SCROLL_BLOCK;
		} else
		    if( i == win->dy - 1 ) {
			*buf_ptr = scroll_base + 31;
			*map_ptr = map_base + SCROLL_DOWN;
		    } else {
			*buf_ptr = scroll_base + 177;
			*map_ptr = map_base + V_SCROLL_BAR;
		    }
	}
	buf_ptr++;
	map_ptr++;
    }
    for( i = first_y; i <= last_y; i++ ) {
	switch( mode ) {
	case INTO_BUFFER:
	    WRITE_TO_BUFFER( win->x + win->dx, win->y + i, tmp_buf[ i ] );
	    WRITE_TO_MAP( win->x + win->dx, win->y + i, tmp_map[ i ] );
	    break;

	case VIDEO_DIRECT:
	    WRITE_TO_SCREEN( win->x + win->dx, win->y + i, tmp_buf[ i ] );
	    WRITE_TO_MAP( win->x + win->dx, win->y + i, tmp_map[ i ] );
	    break;

	case VIDEO_TESTED:
	    visi = coordinate_visible( win->window_id,
				       win->x + win->dx, win->y + i, &ms );
	    if( visi == VISIBLE ) {
		WRITE_TO_SCREEN( win->x + win->dx, win->y + i, tmp_buf[ i ] );
		WRITE_TO_MAP( win->x + win->dx, win->y + i, tmp_map[ i ] );
	    }
	    else if( visi == SHADOWED ) {
		new_val = tmp_buf[ i ];
		new_val = ( new_val & 0x80ff ) | ( WL_SHADOW_COL << 8 );
		WRITE_TO_SCREEN( win->x + win->dx, win->y + i, new_val );
		WRITE_TO_MAP( win->x + win->dx, win->y + i, tmp_map[ i ]
			      | SHADOW );
	    }
	    if( visi != INVISIBLE && ms ) {
		ws_redraw_mouse( ms );
	    }
	    break;
	}
    }
    return;
}



/*
  print_bottom
  -------------------------------------------------------
  Prints out the lower border of a window and the hori-
  zontal scroll bar. If the width of the window is less
  than 6, no scroll bar is printed.
*/

void print_bottom( WIN_LIST_TYPE * win, u_char mode )
{
    MOUSE_CHAR		ms;
    u_char			*s, visi, block_pos;
    u_short 		*buf_ptr;
    u_short 		*map_ptr;
    u_short 		tmp_buf[ MAX_DX ];
    u_short 		tmp_map[ MAX_DX ];
    u_short 		first_x, last_x;
    u_short 		map_base, new_val;
    u_short 		border_base;
    u_short 		scroll_base;
    u_char			border_type;
    s_int			i;

    if( !( border_type = ( win->window_options & BORDER ) ) ||
	win->y + win->dy > win->hv_clip ||
	win->y + win->dy < win->lv_clip ||
	win->x - 1 > win->hh_clip ) {
	return;
    }

    block_pos = ( u_char ) ( ( win->scroll_x + 1 ) * ( win->dx - 5 ) / 100 );
    first_x = ( win->x <= win->lh_clip ) ? abs( win->x - win->lh_clip - 1 ) : 0;
    last_x = win->hh_clip - win->x + 1;
    last_x = ( last_x > win->dx + 1 ) ? win->dx + 1 : last_x;
    border_base = 256 * win->border_col;
    scroll_base = 256 * win->scroll_col;
    map_base = ( win->window_id & 0xff ) << 8;
    buf_ptr = tmp_buf;
    map_ptr = tmp_map;
    s = ( u_char* ) win->user_border;

    switch( border_type ) {
    case 1:
	*( buf_ptr++ ) = border_base + 192;
	*( map_ptr++ ) = map_base + BOTTOM_LEFT;
	for( i = 0; i < win->dx; i++ ) {
	    *( buf_ptr++ ) = border_base + 196;
	    *( map_ptr++ ) = map_base + LOWER_BORDER;
	}
	*( buf_ptr++ ) = border_base + ( u_char ) ws_ascii( 217 );
	*( map_ptr++ ) = map_base + BOTTOM_RIGHT;
	break;

    case 2:
	*( buf_ptr++ ) = border_base + 200;
	*( map_ptr++ ) = map_base + BOTTOM_LEFT;
	for( i = 0; i < win->dx; i++ ) {
	    *( buf_ptr++ ) = border_base + 205;
	    *( map_ptr++ ) = map_base + LOWER_BORDER;
	}
	*( buf_ptr++ ) = border_base + 188;
	*( map_ptr++ ) = map_base + BOTTOM_RIGHT;
	break;

    case 3:
	*( buf_ptr++ ) = border_base + s[ 2 ];
	*( map_ptr++ ) = map_base + BOTTOM_LEFT;
	for( i = 0; i < win->dx; i++ ) {
	    *( buf_ptr++ ) = border_base + s[ 4 ];
	    *( map_ptr++ ) = map_base + LOWER_BORDER;
	}
	*( buf_ptr++ ) = border_base + s[ 3 ];
	*( map_ptr++ ) = map_base + BOTTOM_RIGHT;
	break;
    }

    if( win->window_options & HOR_SBAR && win->dx > 6 ) {
	buf_ptr = &( tmp_buf[ 2 ] );
	map_ptr = &( tmp_map[ 2 ] );
	*( buf_ptr++ ) = scroll_base + 17;
	*( map_ptr++ ) = map_base + SCROLL_LEFT;
	for( i = 0; i < win->dx - 4; i++ ) {
	    if( i == block_pos ) {
		*( buf_ptr++ ) = scroll_base + 254;
		*( map_ptr++ ) = map_base + H_SCROLL_BLOCK;
	    }
	    else {
		*( buf_ptr++ ) = scroll_base + 177;
		*( map_ptr++ ) = map_base + H_SCROLL_BAR;
	    }
	}
	*( buf_ptr ) = scroll_base + 16;
	*( map_ptr++ ) = map_base + SCROLL_RIGHT;
    }

    for( i = first_x; i <= last_x; i++ ) {
	switch( mode ) {
	case INTO_BUFFER:
	    WRITE_TO_BUFFER( win->x - 1 + i, win->y + win->dy, tmp_buf[ i ] );
	    WRITE_TO_MAP( win->x - 1 + i, win->y + win->dy, tmp_map[ i ] );
	    break;

	case VIDEO_DIRECT:
	    WRITE_TO_SCREEN( win->x - 1 + i, win->y + win->dy, tmp_buf[ i ] );
	    WRITE_TO_MAP( win->x - 1 + i, win->y + win->dy, tmp_map[ i ] );
	    break;

	case VIDEO_TESTED:
	    visi = coordinate_visible( win->window_id,
				       win->x - 1 + i, win->y + win->dy, &ms );
	    if( visi == VISIBLE ) {
		WRITE_TO_SCREEN( win->x - 1 + i, win->y + win->dy, tmp_buf[ i ] );
		WRITE_TO_MAP( win->x - 1 + i, win->y + win->dy, tmp_map[ i ] );
	    }
	    else if( visi == SHADOWED ) {
		new_val = tmp_buf[ i ];
		new_val = ( new_val & 0x80ff ) | ( WL_SHADOW_COL << 8 );
		WRITE_TO_SCREEN( win->x - 1 + i, win->y + win->dy, new_val );
		WRITE_TO_MAP( win->x - 1 + i, win->y + win->dy, tmp_map[ i ]
			      | SHADOW );
	    }
	    if( visi != INVISIBLE && ms ) {
		ws_redraw_mouse( ms );
	    }
	    break;
	}
    }
}



/*
  print_icon
  -------------------------------------------------------
  Prints out the icon box for a window.
*/

void print_icon( WIN_LIST_TYPE *win, u_char mode )
{
    u_char			i;
    u_char			x, y;
    u_short			c, v;

    x = win->icon_x; y = win->icon_y;
    c = win->icon_col << 8;
    v = ( win->window_id & 0xff ) << 8;
    if( mode == INTO_BUFFER ) {
	WRITE_TO_BUFFER( x, y, c + 218 );
	WRITE_TO_BUFFER( x, y + 1, c + 179 );
	WRITE_TO_BUFFER( x, y + 2, c + 192 );
	WRITE_TO_BUFFER( x + 4, y, c + 191 );
	WRITE_TO_BUFFER( x + 4, y + 1, c + 179 );
	WRITE_TO_BUFFER( x + 4, y + 2, c + ( u_char ) ws_ascii( 217 ) );
    }
    else {
	WRITE_TO_SCREEN( x, y, c + 218 );
	WRITE_TO_SCREEN( x, y + 1, c + 179 );
	WRITE_TO_SCREEN( x, y + 2, c + 192 );
	WRITE_TO_SCREEN( x + 4, y, c + 191 );
	WRITE_TO_SCREEN( x + 4, y + 1, c + 179 );
	WRITE_TO_SCREEN( x + 4, y + 2, c + ( u_char ) ws_ascii( 217 ) );
    }
    WRITE_TO_MAP( x, y, v + ICON_BORDER );
    WRITE_TO_MAP( x, y + 1, v + ICON_BORDER );
    WRITE_TO_MAP( x, y + 2, v + ICON_BORDER );
    WRITE_TO_MAP( x + 4, y, v + ICON_BORDER );
    WRITE_TO_MAP( x + 4, y + 1, v + ICON_BORDER );
    WRITE_TO_MAP( x + 4, y + 2, v + ICON_BORDER );
    for( i = 1; i < 4; i++ ) {
	if( mode == INTO_BUFFER ) {
	    WRITE_TO_BUFFER( x + i, y, c + 196 );
	    if( win->icon_title[ i - 1 ] ) {
		WRITE_TO_BUFFER( x + i, y + 1, c + win->icon_title[ i - 1 ] );
	    }
	    else {
		WRITE_TO_BUFFER( x + i, y + 1, c + 32 );
	    }
	    WRITE_TO_BUFFER( x + i, y + 2, c + 196 );
	}
	else {
	    WRITE_TO_SCREEN( x + i, y, c + 196 );
	    if( win->icon_title[ i - 1 ] ) {
		WRITE_TO_SCREEN( x + i, y + 1, c + win->icon_title[ i - 1 ] );
	    }
	    else {
		WRITE_TO_SCREEN( x + i, y + 1, c + 32 );
	    }
	    WRITE_TO_SCREEN( x + i, y + 2, c + 196 );
	}
	WRITE_TO_MAP( x + i, y, v + ICON_BORDER );
	WRITE_TO_MAP( x + i, y + 1, v + ICON_AREA );
	WRITE_TO_MAP( x + i, y + 2, v + ICON_BORDER );
    }
}



/*
  put_background
  -------------------------------------------------------
  Prints out the background of the screen
*/

void put_background( u_char mode )
{
    u_char		x, y, p, visi;
    MOUSE_CHAR	ms;
    u_short		v;

    for( y = 1; y <= screen_max_y; y++ ) {
	for( x = 1; x <= screen_max_x; x++ ) {
	    p = ( x - 1 ) % img_dx + ( ( y - 1 ) % img_dy ) * img_dx;
	    switch( mode ) {
	    case INTO_BUFFER:
		WRITE_TO_BUFFER( x, y, scr_image[ p ] );
		WRITE_TO_MAP( x, y, WINDOW_AREA );
		break;

	    case VIDEO_TESTED:
		visi = coordinate_visible( ROOT_WINDOW, x, y, &ms );
		if( visi == SHADOWED ) {
		    WRITE_TO_SCREEN( x, y, scr_image[ p ] & 0xff +
				     ( WL_SHADOW_COL << 8 ) );
		    WRITE_TO_MAP( x, y, WINDOW_AREA | SHADOW );
		}
		else {
		    WRITE_TO_SCREEN( x, y, scr_image[ p ] );
		    WRITE_TO_MAP( x, y, WINDOW_AREA );
		}
		if( ms && visi != INVISIBLE ) {
		    ws_redraw_mouse( ms );
		    v = GET_MAP_VAL( x, y );
		    WRITE_TO_MAP( x, y, v | ( ms << 6 ) );
		}
		break;
	    }
	}
    }
}



/*
  redraw_screen
  -------------------------------------------------------
  Redraws the entire screen
*/

void redraw_screen( WIN_LIST_TYPE *win )
{
    u_char		next_ix = 1;
    u_char		next_iy = screen_max_y - 2;

    put_background( INTO_BUFFER );
    while( win != NULL ) {
	win->x = win->father_ptr->x + win->rel_x - 1;
	win->y = win->father_ptr->y + win->rel_y - 1;
	if( !win->iconic ) {
	    get_clips( win );
	    print_header( win, INTO_BUFFER );
	    print_left( win, INTO_BUFFER );
	    print_right( win, INTO_BUFFER );
	    print_bottom( win, INTO_BUFFER );
	    restore_content( win, INTO_BUFFER );
	    print_shadow( win, INTO_BUFFER );
	}
	if( win->iconic && ( win->win_level == 1 ) ) {
	    if( !( win->window_options & NO_DEF_ICON ) &&
		!win->icon_moved ) {
		win->icon_x = next_ix;
		win->icon_y = next_iy;
		if( next_ix + 2 > screen_max_x - 4 ) {
		    next_ix = 1;
		    next_iy -= 3;
		}
		else {
		    next_ix += 5;
		}
		if( next_iy < 1 ) {
		    next_ix = 1;
		    next_iy = screen_max_y - 2;
		}
	    }
	    print_icon( win, INTO_BUFFER );
	}
	win = win->prev_window;
    }
    hw_puttext( 1, 1, screen_max_x, screen_max_y, screen_buf );
    ws_set_cursor( cursor_wid );
    ws_redraw_mouse( MOUSE_ALL );
}



/*
  beep
  -------------------------------------------------------
  Generates a short beep.
*/

void beep( void )
{
    u_short 	i, j;
    u_char		orgbits, bits;

    bits = orgbits = inportb( 0x61 );
    for( i = 0; i < 20; i++ ) {
	outportb( 0x61, bits & 0xfc );
	for( j = 0; j <= 1500; j++ );

	outportb( 0x61, bits | 2 );
	for( j = 0; j <= 1500; j++ );
    }
    outportb( 0x61, orgbits );
}



/*
  check_input
  -------------------------------------------------------
  Checks and corrects the given parameters of a window.
*/

ERR_CODE check_input( WIN_PARAM_TYPE *p )
{
    WIN_LIST_TYPE	*win;
    s_short			min_dx;
    s_short			def_dx, def_dy;

    if( !validate_wid( p->father_window ) ) {	/* Check father window */
	return( WS_FATHER );
    }

    if( p->father_window == ROOT_WINDOW &&
	def.act_border != NO_CHANGE && def.pas_border != NO_CHANGE ) {

	/* The window is the first one to be opened, so it will be
	   the active window */
	if( win_list == win_last ) {
	    p->window_options &= ~BORDER;
	    p->window_options |= def.act_border;
	    if( def.act_border == USER_BORDER ) {
		strncpy( p->user_border, def.act_uborder, 7 );
	    }
	}
	else {
	    p->window_options &= ~BORDER;
	    p->window_options |= def.pas_border;
	    if( def.pas_border == USER_BORDER ) {
		strncpy( p->user_border, def.pas_uborder, 7 );
	    }
	}
    }

    win = WIN_INFO( p->father_window );

    if( !p->window_options & BORDER ) {		/* Correct window options */
	p->window_options &= FATHER_CLIP;
    }
    if( p->father_window != ROOT_WINDOW ) {
	p->window_options &= ~ALLOW_ICON;
    }

    min_dx = 1;								/* Correct minimum size */
    if( p->window_options & ALLOW_ICON ) min_dx += 4;
    if( p->window_options & ALLOW_ZOOM ) min_dx += 4;
    if( p->window_options & PRINT_AUX ) min_dx += 4;
    if( strlen( p->title ) > 3 ) {
	min_dx += 4;
    }
    else {
	min_dx += ( strlen( p->title ) + 1 );
    }
    min_dx = ( min_dx > p->min_dx ) ? min_dx : p->min_dx;
    p->min_dx = ( min_dx <= screen_max_x - 2 ) ? min_dx :
	screen_max_x - 2;
    p->min_dy = ( p->min_dy < 1 ) ? 1 : p->min_dy;
    p->min_dy = ( p->min_dy <= screen_max_y - 2 ) ? p->min_dy :
	screen_max_y - 2;

    p->iconic = ( p->window_options & ALLOW_ICON ) ? p->iconic : 0;
    p->full_screen = ( p->window_options & ALLOW_ZOOM ) ? p->full_screen : 0;

    if( p->x == DEFAULT_POS || p->y == DEFAULT_POS ) {
	if( win == ROOT_PTR ) {
	    get_next_wpos( &p->x, &p->y, &def_dx, &def_dy );
	    if( p->dx == DEFAULT_SIZE ) {
		p->dx = def_dx;
	    }
	    if( p->dy == DEFAULT_SIZE ) {
		p->dy = def_dy;
	    }
	}
	else {
	    if( p->dx == DEFAULT_SIZE ) {
		p->dx = ( win->orig_dx - 4 ) / 2;
	    }
	    if( p->dy == DEFAULT_SIZE ) {
		p->dy = ( win->orig_dy - 2 ) / 2;
	    }
	    p->x = ( win->orig_dx - p->dx ) / 2;
	    p->y = ( win->orig_dy - p->dy ) / 2;
	    /*

	    if( p->dx == DEFAULT_SIZE ) {
	    p->dx = ( win->dx - 4 ) / 2;
	    }
	    if( p->dy == DEFAULT_SIZE ) {
	    p->dy = ( win->dy - 2 ) / 2;
	    }
	    p->x = ( win->dx - p->dx ) / 2;
	    p->y = ( win->dy - p->dy ) / 2;
	    */
	}
    }

    if( p->dx < p->min_dx ) p->dx = p->min_dx;
    if( p->dy < p->min_dy ) p->dy = p->min_dy;

    if( p->border_col == DEFAULT_COL ) p->border_col = def.def_border_col;
    if( p->title_col == DEFAULT_COL ) p->title_col = def.def_title_col;
    if( p->screen_col == DEFAULT_COL ) p->screen_col = def.def_screen_col;
    if( p->scroll_col == DEFAULT_COL ) p->scroll_col = def.def_scroll_col;
    if( p->icon_col == DEFAULT_COL ) p->icon_col = def.def_icon_col;
    if( p->button_col == DEFAULT_COL ) p->button_col = def.def_button_col;

    return( NO_ERR );
}



