
#include <ci_types.h>
#include <cinclude.h>
#include <winlib.h>
#include <stdio.h>
#include <string.h>


static int x = 5;
static int y = 5;

#define FALSE	0
#define TRUE	1


extern char		**environ;
extern FILE_HANDLE_TYPE	*handle_buf;

void main( int argc, char **argv )
{
    WID 		wid, child;
    WIN_PARAM_TYPE	param;
    EVENT_INFO_TYPE	event;
    s_short		wx, wy, wdx, wdy;
    int			i;

    wl_init( 0 );

    param.min_dx = param.min_dy = 0;
    param.x = x; param.y = y;
    x += 10;
    y += 5;
    param.dx = 40; param.dy = 10;
    param.window_options = USER_BORDER | ALLOW_ICON | ALLOW_ZOOM | PRINT_AUX |
	MID_HEADER | VER_SBAR | HOR_SBAR | PRINT_SHADOW |
	ALLOW_RAISE | ALLOW_MOVE | ALLOW_RESIZE;
    param.tab_step = 8;
    param.border_col = param.screen_col = WL_WHITE + 16 * WL_BLUE;
    param.title_col = WL_LIGHTGREEN + 16 * WL_BLUE;
    param.scroll_col = WL_BLUE + 16 * WL_CYAN;
    param.icon_col = WL_WHITE + 16 * WL_LIGHTGRAY;
    param.button_col = WL_LIGHTGREEN + 16 * WL_BLUE;
    strncpy( (char*) param.title, "TestHeader 1", MAX_HEADER_LEN );
    strncpy( (char*) param.user_border, "\0xd8\0xb9\0xd5\0xbe\0xcd\0xfe", 6 );
    param.father_window = ROOT_WINDOW;
    param.iconic = 0;
    param.full_screen = 0;
    wl_open_win( &param, &wid, &wx, &wy, &wdx, &wdy );

    param.x = 5; param.y = 2;
    param.dx = 20; param.dy = 5;
    param.window_options = ALLOW_ICON | ALLOW_ZOOM | PRINT_AUX | MID_HEADER |
	VER_SBAR | HOR_SBAR | PRINT_SHADOW | USER_BORDER |
	ALLOW_RAISE | ALLOW_MOVE | ALLOW_RESIZE;
    param.tab_step = 8;
    param.border_col = param.screen_col = WL_WHITE + 16 * WL_BLUE;
    param.title_col = WL_LIGHTGREEN + 16 * WL_BLUE;
    param.scroll_col = WL_BLUE + 16 * WL_CYAN;
    param.icon_col = WL_WHITE + 16 * WL_LIGHTGRAY;
    param.button_col = WL_LIGHTGREEN + 16 * WL_BLUE;
    strncpy( (char*) param.title, "TestHeader 2", MAX_HEADER_LEN );
    strncpy( (char*) param.user_border, "\0xd8\0xb9\0xd5\0xbe\0xcd\0xfe", 6 );
    param.father_window = wid;
    param.iconic = 0;
    param.full_screen = 0;
    wl_open_win( &param, &child, &wx, &wy, &wdx, &wdy );

    wl_printf( wid, "Hello world!\n\r" );

    wl_printf( wid, "Parameters...\n\r" );
    for( i = 0; i < argc; i++ ) {
	wl_printf( wid, "%d: %s\n\r", i, argv[ i ] );
    }
    wl_printf( wid, "\nEnvironment...\n\r" );

    for( i = 0; environ[ i ] != NULL; i++ ) {
	wl_printf( wid, "%d: %s\n\r", i, environ[ i ] );
    }
    wl_printf( wid, "\nStandard files\n\r" );
    wl_printf( wid, "stdin: %d %d\n\r", handle_buf[ 0 ]. handle_num,
	       handle_buf[ 0 ]. handle_type );
    wl_printf( wid, "stdout: %d %d\n\r", handle_buf[ 1 ]. handle_num,
	       handle_buf[ 1 ]. handle_type );
    wl_printf( wid, "stderr: %d %d\n\n\r", handle_buf[ 2 ]. handle_num,
	       handle_buf[ 2 ]. handle_type );


    /*
      wl_wake_me_up( 30, 1 );
    */
    while( TRUE ) {
	wl_wait_for_event( &event );
	switch( event.event_type ) {
	case EV_MOUSE_EVENT:
	    wl_printf( wid, "Mouse X=%d, Y=%d, Button=0x%X\n",
		       (int) event.me.x_pos,
		       (int) event.me.y_pos,
		       (int) event.me.button );
	    break;
	case EV_WINDOW_RESIZED:
	    wl_printf( wid, "Window resized (%d,%d)\n",
		       (int) event.re.new_dx,
		       (int) event.re.new_dy );
	    break;
	    /*
	      case EV_ZOOMED:
	      wl_printf( wid, "Window zoomed\n" );
	      break;
	      case EV_UNZOOMED:
	      wl_printf( wid, "Window unzoomed\n" );
	      break;
	    */
	case EV_SET_FOCUS:
	    wl_printf( wid, "Set Focus\n" );
	    break;
	case EV_SCROLL_EVENT:
	    wl_printf( wid, "Scroll event Per=%d, Button=0x%x\n",
		       (int) event.se.percent,
		       (int) event.se.button );
	    break;
	case EV_KEY_PRESSED:
	    wl_printf( wid, "Key %c\n", event.ke.key );
	    break;
	case EV_WAKEUP:
	    wl_printf( wid, "Wakeup (Mask=%d)\n", event.te.timer_mask );
	    wl_wake_me_up( 30, 1 );
	    break;
	default:
	    wl_printf( wid, "Unkown event %d\n", (int) event.event_type );
	    break;
	}
    }
}
