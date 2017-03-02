/*
 * tetris.c
 *---------------------------------------------------------
 * A Mini-Tetris!                                  AP  9/90
 */

#include <ci_types.h>
#include <cinclude.h>
#include <winlib.h>
#include <sysmsg.h>
#include <stdlib.h>
#include "button.h"


#define TRUE	1
#define FALSE	0


typedef struct {
    s_int  dx, dy;
    s_char object_def[ 5 ][ 5 ];
} OBJECT;


typedef enum {
    SCAN_LEFT,
    SCAN_RIGHT,
    SCAN_UP,
    SCAN_DOWN
} SCAN;


u_char *field = NULL;


OBJECT obj1 = { 3, 2, { { 1, 1, 1 }, { 0, 1, 0 } } };
OBJECT obj2 = { 3, 2, { { 1, 0, 0 }, { 1, 1, 1 } } };
OBJECT obj3 = { 3, 2, { { 0, 0, 1 }, { 1, 1, 1 } } };
OBJECT obj4 = { 2, 2, { { 1, 1 }, { 1, 1 } } };
OBJECT obj5 = { 4, 1, { { 1, 1, 1, 1 }, { 0, 0, 0, 0 } } };
OBJECT obj6 = { 2, 3, { { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 } } };
OBJECT obj7 = { 2, 3, { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 0, 0 } } };


OBJECT *objects[] = {
    &obj1,
    &obj2,
    &obj3,
    &obj4,
    &obj5,
    &obj6,
    &obj7 };

s_int num_objects = sizeof( objects ) / sizeof( OBJECT * );


s_short window_dx;
s_short window_dy;

s_int field_size_dx;
s_int field_size_dy;

s_int act_x_pos;
s_int act_y_pos;
s_int act_dx;
s_int act_dy;

WID game_wid;
WID father_wid;
WID button_wid = 0;



s_int test_point( s_int x, s_int y )
{
    s_int i;
    s_int c;
    s_int r;

    i = x + y * field_size_dx;
    c = i / 8;
    r = 1 << ( i % 8 );
    return( ( *( field + c ) & r ) == 0 ? 0 : 1 );
}



s_int set_point( s_int x, s_int y )
{
    s_int i;
    s_int c;
    s_int r;

    i = x + y * field_size_dx;
    c = i / 8;
    r = 1 << ( i % 8 );
    *( field + c ) |= r ;
    return( 0 );
}



s_int reset_point( s_int x, s_int y )
{
    s_int i;
    s_int c;
    s_int r;

    i = x + y * field_size_dx;
    c = i / 8;
    r = 1 << ( i % 8 );
    *( field + c ) &= ~r ;
    return( 0 );
}



void display_field( s_int x, s_int y, s_int dx, s_int dy )
{
    s_int	c, i, j;
    u_char	str[ 200 ];
    u_char	*s;

    dy += y % 2;
    y  -= y % 2;
    dy += dy % 2;

    for( i = 0; i < dy / 2; i++ ) {
	wl_gotoxy( game_wid, x, y / 2 + i );
	s = str;
	for( j = 0; j < dx; j++ ) {
	    c = ( test_point( x + j, y + i * 2 ) << 1 ) |
		test_point( x + j, y + i * 2 + 1 );
	    switch( c ) {
	    case 0: *s++ = ' '; break;
	    case 1: *s++ = '\xdc'; break;
	    case 2: *s++ = '\xdf'; break;
	    case 3: *s++ = '\xdb'; break;
	    }
	}
	*s = '\0';
	wl_printf( game_wid, "%s", str );
    }
}



s_int draw_object( s_int (* point_func) ( s_int, s_int ),
		   OBJECT *obj, SCAN dir, s_int x, s_int y )
{
    s_int i, j;
    s_int ii, jj;
    s_int dx, dy;

    dx = obj->dx / 2;
    dy = obj->dy / 2;
    switch( dir ) {
    case SCAN_LEFT:
	act_x_pos = x - dx;
	act_y_pos = y - dy;
	act_dx    = obj->dx;
	act_dy    = obj->dy;
	for( i = 0; i < obj->dy; i++ )
	    for( j = 0; j < obj->dx; j++ )
		if( obj->object_def[ i ][ j ] )
		    if( (* point_func) ( x + j - dx, y + i - dy ) )
			return( 1 );
	break;
    case SCAN_RIGHT:
	act_x_pos = x - dx;
	act_y_pos = y - dy;
	act_dx    = obj->dx;
	act_dy    = obj->dy;
	for( i = obj->dy - 1, ii = 0; i >= 0; i--, ii++ )
	    for( j = obj->dx - 1, jj = 0; j >= 0; j--, jj++ )
		if( obj->object_def[ i ][ j ] )
		    if( (* point_func) ( x + jj - dx, y + ii - dy ) )
			return( 1 );
	break;
    case SCAN_UP:
	act_x_pos = x - dy;
	act_y_pos = y - dx;
	act_dx    = obj->dy;
	act_dy    = obj->dx;
	for( i = obj->dy - 1, ii = 0; i >= 0; i--, ii++ )
	    for( j = 0; j < obj->dx; j++ )
		if( obj->object_def[ i ][ j ] )
		    if( (* point_func) ( x + ii - dy, y + j - dx ) )
			return( 1 );
	break;
    case SCAN_DOWN:
	act_x_pos = x - dy;
	act_y_pos = y - dx;
	act_dx    = obj->dy;
	act_dy    = obj->dx;
	for( i = 0; i < obj->dy; i++ )
	    for( j = obj->dx - 1, jj = 0; j >= 0; j--, jj++ )
		if( obj->object_def[ i ][ j ] )
		    if( (* point_func) ( x + i - dy, y + jj - dx ) )
			return( 1 );
	break;
    }
    return( 0 );
}



void init_game( void )
{
    s_int	i, c;
    s_int	x, y;

    /*
      while( window_dx < 7 || window_dy < 4 ) {
      do {
      wait_for_event( &event );
      } while( event.type != RESIZE_WINDOW );
      window_dx = event.dx;
      window_dy = event.dy;
      }
    */

    x = window_dx;
    y = window_dy;
    x += 2;
    y *= 2;
    y += 4;

    if( field != NULL ) free( field );
    c = x * y / 8 + 1;
    field = malloc( c );
    for( i = 0; i < c; i++ )
	*( field + i ) = 0;
    field_size_dx = x;
    field_size_dy = y;

    for( i = 0; i < y; i++ ) {
	set_point( 0, i );
	set_point( x - 1, i );
    }
    for( i = 0; i < x; i++ )
	set_point( i, y - 2 );
    wl_clrscr( game_wid );
}



OBJECT *choose_object( void )
{
    static s_int	next_obj = 0;
    OBJECT		*o;
    u_char		str[ 6 ];
    u_char		*s;
    u_char		c;
    u_short		x, y;
    s_int		i;

    i = next_obj;
    next_obj = rand() % num_objects;
    o = objects[ next_obj ];
    wl_gotoxy( father_wid, window_dx + 10, 3 );
    wl_printf( father_wid, "Next object:     " );
    wl_gotoxy( father_wid, window_dx + 23, 4 );
    wl_printf( father_wid, "     " );
    for( y = 0; y < o->dy / 2 + 1; y++ ) {
	s = str;
	for( x = 0; x < o->dx; x++ ) {
	    c = ( o->object_def[ y * 2 ][ x ] << 1 ) |
		o->object_def[ y * 2 + 1 ][ x ];
	    switch( c ) {
	    case 0: *s++ = ' '; break;
	    case 1: *s++ = '\xdc'; break;
	    case 2: *s++ = '\xdf'; break;
	    case 3: *s++ = '\xdb'; break;
	    }
	}
	*s = '\0';
	wl_gotoxy( father_wid, window_dx + 23, 3 + y );
	wl_printf( father_wid, "%s", str );
    }
    return( objects[ i ] );
}



void delete_line( u_int y )
{
    s_int i, j;

    for( j = 0; j < 20; j++ ) {
	for( i = 1; i < field_size_dx - 2; i++ )
	    reset_point( i, y );
	display_field( 1, y, field_size_dx - 2, 1 );
	for( i = 1; i < field_size_dx - 2; i++ )
	    set_point( i, y );
	display_field( 1, y, field_size_dx - 2, 1 );
    }
    for( i = y; i > 0; i-- )
	for( j = 1; j < field_size_dx - 1; j++ )
	    if( test_point( j, i - 1 ) )
		set_point( j, i );
	    else
		reset_point( j, i );
    display_field( 1, 2, field_size_dx - 2, field_size_dy - 4 );
}



void test_full_lines( void )
{
    s_int i, j;

    for( i = 2; i < field_size_dy - 2; i++ ) {
	for( j = 1; j < field_size_dx - 1; j++ )
	    if( !test_point( j, i ) ) break;
	if( j == field_size_dx - 1 ) delete_line( i );
    }
}



void clip_act_coordinates( void )
{
    if( act_x_pos < 1 ) act_x_pos = 1;
    if( act_y_pos < 3 ) act_y_pos = 3;
}



void play_tetris( void )
{
    EVENT_INFO_TYPE	event;
    OBJECT		*obj;
    SCAN		direction;
    SCAN		old_direction;
    s_int		obj_pos_x;
    s_int		obj_pos_y;
    s_int		new_game;
    s_int		i;
    u_char		ch;
    int ccc=0;

    init_game();
    while( 1 ) {
	obj = choose_object();
	direction = SCAN_LEFT;
	obj_pos_x = window_dx / 2 + 1;
	obj_pos_y = 2;
	if( draw_object( test_point, obj, direction, obj_pos_x, obj_pos_y ) )
	    return;
	new_game = FALSE;
	while( !new_game ) {
	    draw_object( set_point, obj, direction, obj_pos_x, obj_pos_y );
	    clip_act_coordinates();
	    display_field( act_x_pos, act_y_pos, act_dx, act_dy );
	    do {
		wl_wait_for_event( &event );
		if( event.event_type == EV_MOUSE_EVENT &&
		    event.me.loc == AUX_BUTTON &&
		    ( event.me.button & LEFT_PRESSED ) ) {
		    _exit( 0 );
		}
		if( event.me.window_id == button_wid && event.me.button & LEFT_RELEASED ) {
		    init_game();
		    new_game = TRUE;
		    break;
		}
		if( event.event_type == EV_WAKEUP ) {
		    event.event_type = EV_KEY_PRESSED;
		    event.ke.key = 0;
		    wl_wake_me_up( 10, 1 );
		}
		/*
		  if( event.type == RESIZE_WINDOW ) {
		  window_dx = event.dx;
		  window_dy = event.dy;
		  init_game();
		  new_game = TRUE;
		  break;
		  }
		*/
	    } while( event.event_type != EV_KEY_PRESSED && !new_game );
	    if( new_game ) continue;
	    ch = event.ke.key;
	    draw_object( reset_point, obj, direction, obj_pos_x, obj_pos_y );
	    clip_act_coordinates();
	    display_field( act_x_pos, act_y_pos, act_dx, act_dy );
	    switch( ch ) {
	    case '\0':
		if( draw_object( test_point, obj, direction,
				 obj_pos_x, ++obj_pos_y ) ) {
		    draw_object( set_point, obj, direction,
				 obj_pos_x, --obj_pos_y );
		    display_field( act_x_pos, act_y_pos, act_dx, act_dy );
		    test_full_lines();
		    new_game = TRUE;
		}
		break;
	    case ' ':
		while( !draw_object( test_point, obj, direction,
				     obj_pos_x, ++obj_pos_y ) ) ;
		draw_object( set_point, obj, direction,
			     obj_pos_x, --obj_pos_y );
		display_field( act_x_pos, act_y_pos, act_dx, act_dy );
		test_full_lines();
		new_game = TRUE;
		break;
	    case 'y':
		if( draw_object( test_point, obj, direction,
				 --obj_pos_x, obj_pos_y ) )
		    obj_pos_x++;
		break;
	    case 'x':
		if( draw_object( test_point, obj, direction,
				 ++obj_pos_x, obj_pos_y ) )
		    obj_pos_x--;
		break;
	    case '-':
		switch( old_direction = direction ) {
		case SCAN_LEFT:
		    direction = SCAN_DOWN;
		    break;
		case SCAN_UP:
		    direction = SCAN_LEFT;
		    break;
		case SCAN_RIGHT:
		    direction = SCAN_UP;
		    break;
		case SCAN_DOWN:
		    direction = SCAN_RIGHT;
		    break;
		}
		if( draw_object( test_point, obj, direction,
				 obj_pos_x, obj_pos_y ) )
		    direction = old_direction;
		break;
	    }
	}
    }
}



void main( void )
{
    WIN_PARAM_TYPE	param;
    PID			pid;
    int			data_len;
    s_short		x, y;


    wl_init( 0 );

    /* Open father window */
    param.dx = param.min_dx = 40;
    param.dy = param.min_dy = 12;
    param.x = 30; param.y = 3;
    param.window_options = USER_BORDER | ALLOW_ICON | /*ALLOW_ZOOM |*/
	MID_HEADER | PRINT_SHADOW | PRINT_AUX |
	ALLOW_RAISE | ALLOW_MOVE /*| ALLOW_RESIZE*/;
    param.tab_step = 8;
    param.border_col = param.screen_col = WL_WHITE + 16 * WL_BLUE;
    param.title_col = WL_LIGHTGREEN + 16 * WL_BLUE;
    param.scroll_col = WL_BLUE + 16 * WL_CYAN;
    param.icon_col = WL_WHITE + 16 * WL_LIGHTGRAY;
    param.button_col = WL_LIGHTGREEN + 16 * WL_BLUE;
    strncpy( param.title, ( u_char* ) "Tetris", MAX_HEADER_LEN );
    strncpy( param.user_border, "Ø¹Õ¾Íþ", 6 );
    param.father_window = ROOT_WINDOW;
    param.iconic = 0;
    param.full_screen = 0;
    wl_open_win( &param, &father_wid, &x, &y, &window_dx, &window_dy );
    wl_cursor_off( father_wid );

    window_dx = 12;
    window_dy = 10;
    param.dx = param.min_dx = window_dx;
    param.dy = param.min_dy = window_dy;
    param.x = 4; param.y = 2;
    param.window_options = THIN_BORDER | FATHER_CLIP;
    param.tab_step = 8;
    param.border_col = param.screen_col = WL_WHITE + 16 * WL_BLUE;
    param.title_col = WL_LIGHTGREEN + 16 * WL_BLUE;
    param.scroll_col = WL_BLUE + 16 * WL_CYAN;
    param.icon_col = WL_WHITE + 16 * WL_LIGHTGRAY;
    param.button_col = WL_LIGHTGREEN + 16 * WL_BLUE;
    strncpy( param.title, ( u_char* ) "", MAX_HEADER_LEN );
    param.father_window = father_wid;
    param.iconic = 0;
    param.full_screen = 0;
    wl_open_win( &param, &game_wid, &x, &y, &window_dx, &window_dy );
    button_wid = define_button( father_wid, window_dx + 10, window_dy,
				12, 1, "New Game" );
    /*
      k_suspend();
    */
    wl_wake_me_up( 10, 1 );
    while( 1 ) {
	play_tetris();
    }
}
