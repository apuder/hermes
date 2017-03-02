
#include <cinclude.h>
#include <assert.h>
#include <ci_types.h>
#include "winlib.h"
#include "win_serv.h"
#include "ws_intrn.h"
#include "specials.h"


/* Up-Arrow */
static u_short		and_mask[ 16 ] = {
    0xc000, 0xe000, 0xf000, 0xf800, 0xfc00, 0xfe00, 0xff00,
    0xff80, 0xfe00, 0x1e00, 0x1f00, 0x0f00, 0x0f00, 0x0000,
    0x0000, 0x0000
};

static u_short		or_mask[ 16 ] = {
    0x0000, 0x4000, 0x6000, 0x7000, 0x7800, 0x7c00, 0x7e00,
    0x6800, 0x0c00, 0x0c00, 0x0600, 0x0600, 0x0000, 0x0000,
    0x0000, 0x0000
};


static u_char	mchars[ 4 ] = { MOUSE_1, MOUSE_2, MOUSE_3, MOUSE_4 };
static u_char	dm[ 128 ];
static u_char	ofnt[ 128 ];
static u_char	ofnt1[ 128 ];
static u_short	wrkar[ 32 ];


/* Pointer to VGA-font area */
static u_char *cg;



static void open_cg( void )
{
    asm( "cli" );
    outportb( 0x03c4, 0x02 );
    outportb( 0x03c5, 0x04 );
    outportb( 0x03c4, 0x04 );
    outportb( 0x03c5, 0x07 );

    outportb( 0x03ce, 0x05 );
    outportb( 0x03cf, 0x00 );
    outportb( 0x03ce, 0x06 );
    outportb( 0x03cf, 0x04 );
    outportb( 0x03ce, 0x04 );
    outportb( 0x03cf, 0x02 );
    asm( "sti" );
}



static void close_cg( void )
{
    asm( "cli" );
    outportb( 0x03c4, 0x02 );
    outportb( 0x03c5, 0x03 );
    outportb( 0x03c4, 0x04 );
    outportb( 0x03c5, 0x03 );

    outportb( 0x03ce, 0x05 );
    outportb( 0x03cf, 0x10 );
    outportb( 0x03ce, 0x06 );
    outportb( 0x03cf, 0x0e );
    outportb( 0x03ce, 0x04 );
    outportb( 0x03cf, 0x00 );
    asm( "sti" );
}



static void get_chars( MOUSE_CHAR m )
{
    u_short		i, j, k;
    u_short		mx, my;

    if( m == MOUSE_ALL ) {
	i = 0;
	j = 4;
    }
    else {
	i = ( u_short ) m - 1;
	j = ( u_short ) m;
    }
    for( k = i; k < j; k++ ) {
	mx = mouse_x + ( k % 2 );
	my = mouse_y + ( k / 2 );
	if( mx >= 1 && mx <= screen_max_x && my >= 1 && my <= screen_max_y ) {
	    mouse_save[ k ] = GET_SCREEN_VAL( mx, my );
	}
    }

    open_cg();
    for( k = i; k < j; k++ ) {
	move_from_font( &cg[ ( mouse_save[ k ] & 0xff ) * 0x20 ], &dm[ k * 32 ], 32 );
    }
    close_cg();
}



static void put_dummy_chars( MOUSE_CHAR m )
{
    u_short		i, j, k;
    u_short		mx, my;

    if( m == MOUSE_ALL ) {
	i = 0;
	j = 4;
    }
    else {
	i = ( u_short ) m - 1;
	j = ( u_short ) m;
    }

    for( k = i; k < j; k++ ) {
	mx = mouse_x + ( k % 2 );
	my = mouse_y + ( k / 2 );
	if( mx >= 1 && mx <= screen_max_x && my >= 1 && my <= screen_max_y ) {
	    WRITE_TO_SCREEN( mx, my, ( mouse_save[ k ] & 0xff00 ) + mchars[ k ] );
	}
    }
    open_cg();
    for( i = 0; i < 4; i++ ) {
	move_to_font( &dm[ i * 32 ], &cg[ mchars[ i ] * 0x20 ], 32 );
    }                                       /* Store redefined characters */
    close_cg();                             /* into the character */
}                                           /* generator */



void gr_mouse_init( void )
{
    u_char	buf[ 32 ];
    u_char	i;

    cg = get_font_addr();
    open_cg();								/* Save original font */

    for( i = 0; i < 4; i++ ) {
	move_from_font( &cg[ mchars[ i ] * 0x20 ], &ofnt[ i * 32 ], 32 );
    }
    move_from_font( &cg[ 0x01 * 0x20 ], ofnt1, 128 );
    for( i = 0; i < 4; i++ ) {
	move_from_font( &cg[ mchars[ i ] * 0x20 ], buf, 32 );
	move_to_font( buf, &cg[ ( i + 1 ) * 0x20 ], 32 );
    }
    close_cg();

    memset( ( u_char* ) wrkar, 0, sizeof( wrkar ) );
    memset( dm, 0, sizeof( dm ) );
}



void draw_gr_mouse( MOUSE_CHAR m )
{
    u_short	xo, yo;
    u_short	i;

    xo = mouse_px % 8;
    yo = mouse_py % 16;

    get_chars( m );

    for( i = 0; i <= 15; i++ ) {
	wrkar[ i ] = ( dm[ i ] << 8 ) + dm[ i + 32 ];
	wrkar[ i + 16 ] = ( dm[ i + 64 ] << 8 ) + dm[ i + 96 ];
    }
    for( i = 0; i <= 15; i++ ) {
	wrkar[ i + yo ] = ( wrkar[ i + yo ] &
			    ( ( and_mask[ i ] >> xo ) ^ 0xffff ) ) |
	    ( or_mask[ i ] >> xo );
    }
    for( i = 0; i <= 15; i++ ) {
	dm[ i ] = ( wrkar[ i ] & 0xff00 ) >> 8;
	dm[ i + 32 ] = wrkar[ i ] & 0xff;
	dm[ i + 64 ] = ( wrkar[ i + 16 ] & 0xff00 ) >> 8;
	dm[ i + 96 ] = wrkar[ i + 16 ] & 0xff;
    }

    put_dummy_chars( m );
}



void ws_reload_font( void )
{
    u_char	i;
    if( ws_gr_mouse() ) {
	open_cg();
	for( i = 0; i < 4; i++ ) {
	    move_to_font( &ofnt[ i * 0x20 ], &cg[ mchars[ i ] * 0x20 ], 32 );
	}
	move_to_font( ofnt1, &cg[ 0x01 * 0x20 ], 128 );
	close_cg();
    }
}



void ws_change_grmouse( u_short *new_and_mask, u_short *new_or_mask )
{
    if( mouse_size != 2 ) {
	return;
    }

    ws_restore_mouse();
    bufcpy( and_mask, new_and_mask, 32 );
    bufcpy( or_mask, new_or_mask, 32 );
    draw_gr_mouse( MOUSE_ALL );
}


/*
  ws_redraw_mouse
  -------------------------------------------------------
  Prints the given character of the mouse to the screen.
  The characters are numbered as following: 1 2
  3 4
  A value of MOUSE_ALL means to redraw the whole
  cursor
*/

void ws_redraw_mouse( MOUSE_CHAR m )
{
    u_short		x, i, j, k;
    u_short		mx, my;

    if( !mouse_installed ) {
	return;
    }

    if( m == MOUSE_ALL ) {
	i = 0;
	j = ( mouse_size > 1 ) ? 4 : 1;
    }
    else {
	i = ( u_short ) m - 1;
	j = ( u_short ) m;
    }

    for( k = i; k < j; k++ ) {
	mx = mouse_x + ( k % 2 );
	my = mouse_y + ( k / 2 );
	if( mx >= 1 && mx <= screen_max_x && my >= 1 && my <= screen_max_y ) {
	    x = GET_MAP_VAL( mx, my );
	    x |= MOUSE_MASK;
	    WRITE_TO_MAP( mx, my, x );
	    if( mouse_size == 1 ) {
		mouse_save[ k ] = x = GET_SCREEN_VAL( mx, my );
		x &= mouse_and;
		x ^= mouse_xor;
		WRITE_TO_SCREEN( mx, my, x );
	    }
	}
    }
    if( mouse_size == 2 ) {
	draw_gr_mouse( m );
    }
}



/*
  ws_restore_mouse
  -------------------------------------------------------
  Restores the old characters, that were overwritten by
  the mouse cursor.
*/

void ws_restore_mouse( void )
{
    u_short	x, i, j;
    u_short	mx, my;

    if( !mouse_installed ) {
	return;
    }

    j = ( mouse_size > 1 ) ? 4 : 1;
    for( i = 0; i < j; i++ ) {
	mx = mouse_x + ( i % 2 );
	my = mouse_y + ( i / 2 );

	if( mx >= 1 && mx <= screen_max_x && my >= 1 && my <= screen_max_y ) {
	    WRITE_TO_SCREEN( mx, my, mouse_save[ i ] );
	    x = GET_MAP_VAL( mx, my );
	    x &= ~MOUSE_MASK;
	    WRITE_TO_MAP( mx, my, x );
	}
    }
}



/*
  ws_mouse_goto_dxdy
  -------------------------------------------------------
  Moves the mouse to the given position and returns the
  window_id and the event_id of the window at the new
  location.
*/

void ws_mouse_goto_dxdy( s_short dx, s_short dy )
{
    s_short	mx, my;

    if( !mouse_installed ) {
	return;
    }

    mouse_px += dx;
    mouse_py += dy;
    if( mouse_px > screen_max_x * 8 - 2 ) mouse_px = screen_max_x * 8 - 2;
    if( mouse_px < 0 ) mouse_px = 0;
    if( mouse_py > screen_max_y * 16 - 2 ) mouse_py = screen_max_y * 16 - 2;
    if( mouse_py < 0 ) mouse_py = 0;
    mx = mouse_px / 8 + 1;
    my = mouse_py / 16 + 1;
    if( mx != old_mx || my != old_my || mouse_size == 2 ) {
	ws_restore_mouse();
	mouse_x = mx; mouse_y = my;
	old_mx = mouse_x; old_my = mouse_y;
	ws_redraw_mouse( MOUSE_ALL );
    }
}



/*
  ws_whereis_mouse
  -------------------------------------------------------
  Returns the location and other infos of the mouse.
*/

LOCATION ws_whereis_mouse( WID *wid, s_char *x, s_char *y, u_char *percent )
{
    WIN_LIST_TYPE	*win;
    u_short			v, l;
    s_short			tp = 0;

    if( mouse_graped ) {
	win = win_array[ grap_wid & 0xff ].window;
	l = grap_loc;
    }
    else {
	v = GET_MAP_VAL( mouse_x, mouse_y );
	win = win_array[ ( v & 0xff00 ) >> 8 ].window;
	l = v & INFO_MASK;
    }
    /*
      assert( validate_wid( win->window_id ) );
    */
    *wid = win->window_id;
    *percent = *x = *y = 0;
    switch( l ) {

    case V_SCROLL_BAR:
    case V_SCROLL_BLOCK:
	tp = ( s_short ) ( mouse_y - win->y - 1 ) * 100 / ( win->dy - 3 );
	break;

    case SCROLL_UP:
    case SCROLL_DOWN:
	tp = win->scroll_y;
	break;

    case H_SCROLL_BAR:
    case H_SCROLL_BLOCK:
	tp = ( s_short ) ( ( mouse_x - win->x - 2 ) * 100 / ( win->dx - 5 ) );
	break;

    case SCROLL_LEFT:
    case SCROLL_RIGHT:
	tp = win->scroll_x;
	break;

    case WINDOW_AREA:
	*x = mouse_x - win->x + 1;
	*y = mouse_y - win->y + 1;
	break;

    }
    if( tp < 0 ) {
	*percent = 0;
    }
    else if ( tp > 100 ) {
	*percent = 100;
    }
    else {
	*percent = tp;
    }
    return( l );
}



u_char ws_gr_mouse( void )
{
    return( ( mouse_size == 2 ) );
}



s_char ws_ascii( s_char a )
{
    if( ws_gr_mouse() ) {
	switch( ( u_char ) a ) {
	case MOUSE_1:
	    return( 1 );

	case MOUSE_2:
	    return( 2 );

	case MOUSE_3:
	    return( 3 );

	case MOUSE_4:
	    return( 4 );
	}
    }
    return( a );
}



