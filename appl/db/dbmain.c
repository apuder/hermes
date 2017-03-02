
#include <ci_types.h>
#include <cinclude.h>
#include <winlib.h>
#include <sysmsg.h>
#include "db.h"
#include "display.h"



WID debug_wid;


extern int yyparse( void );


char db_buffer[ MAX_DB_BUFFER + 1 ];
char *db_buffer_ptr;

int db_value[ 9 ];


void input( char *buf, int max_input_len )
{
	EVENT_INFO_TYPE	event;
	int				x = 0;
	unsigned		c;

	wl_printf( debug_wid, ">" );
	wl_cursor_on( debug_wid );
	while( TRUE ) {
		do {
			wl_wait_for_event( &event );
			if( event.me.window_id != debug_wid ) {
				manage_event( &event );
				continue;
			}
			if( event.event_type == EV_WINDOW_RESIZED ) {
				*( buf + x ) = '\0';
				wl_printf( debug_wid, ">%s", buf );
			}
		} while( event.event_type != EV_KEY_PRESSED );
		c = event.ke.key;
		if( c > 0xff ) continue;  /* Ignore function keys */
		if( c == '\t' ) continue;
		if( c == 13 ) {
			*( buf + x ) = '\0';
			wl_cursor_off( debug_wid );
			return;
		}
		if( c == 27 ) {		/* Escape */
			while( x != 0 ) {
				wl_printf( debug_wid, "\b" );
				x--;
			}
			continue;
		}
		if( c == '\b' ) {
			if( x != 0 ) {
				wl_printf( debug_wid, "\b" );
				x--;
			}
			continue;
		}
		if( x == max_input_len ) continue;
		wl_printf( debug_wid, "%c", (char) c );
		*( buf + x++ ) = (char) c;
	}
}



void main( int argc, char **argv )
{
	WIN_PARAM_TYPE	param;
	EVENT_INFO_TYPE	event;
	s_short			x, y, dx, dy;
	int				i;

	wl_init( 0 );

	/* Register as debugger. Use first port as debugger port. */
	k_p1 = (int) FirstPortId;
	kernel_trap( K_HOOK_DEBUGGER );

	param.min_dx = param.min_dy = 0;
	param.x = 2; param.y = 13;
	param.dx = 78; param.dy = 12;
	param.window_options = USER_BORDER | ALLOW_ICON | ALLOW_ZOOM |
						   MID_HEADER | PRINT_SHADOW |
						   ALLOW_RAISE | ALLOW_MOVE | ALLOW_RESIZE;
	param.tab_step = 8;
	param.border_col = param.screen_col = WL_WHITE + 16 * WL_BLUE;
	param.title_col = WL_LIGHTGREEN + 16 * WL_BLUE;
	param.scroll_col = WL_BLUE + 16 * WL_CYAN;
	param.icon_col = WL_WHITE + 16 * WL_LIGHTGRAY;
	param.button_col = WL_LIGHTGREEN + 16 * WL_BLUE;
	strncpy( param.title, ( u_char* ) "Debug", MAX_HEADER_LEN );
	strncpy( param.user_border, "Ø¹Õ¾Íþ", 6 );
	param.father_window = ROOT_WINDOW;
	param.iconic = 0;
	param.full_screen = 0;
	wl_open_win( &param, &debug_wid, &x, &y, &dx, &dy );

	for( i = 0; i < 9; i++ ) db_value[ i ] = 0;

	wl_printf( debug_wid, "Welcome to the HERMES-Debugger\n\r\n\r" );

    while( TRUE ) {
		input( db_buffer, MAX_DB_BUFFER );
		wl_printf( debug_wid, "\n\r" );
		db_buffer_ptr = db_buffer;
		yyparse();
		wl_printf( debug_wid, "\n\r" );
	}
}
