
#include <ci_types.h>
#include <cinclude.h>
#include <winlib.h>
#include <sysmsg.h>
#include "db.h"
#include "types.h"
#include "syms.h"
#include "display.h"



static int			source_win_open = 0;

static WID			source_wid;
static s_short		x, y, dx, dy;



int display_source( unsigned addr )
{
	WIN_PARAM_TYPE	param;
	char			*fname;
	int				line;

	if( !source_win_open ) {
		param.min_dx = param.min_dy = 0;
		param.x = 10; param.y = 2;
		param.dx = 50; param.dy = 5;
		param.window_options = USER_BORDER | ALLOW_ICON | ALLOW_ZOOM |
							   MID_HEADER | PRINT_SHADOW | FATHER_CLIP |
							   ALLOW_RAISE | ALLOW_MOVE | ALLOW_RESIZE;
		param.tab_step = 4;
		param.border_col = param.screen_col = WL_WHITE + 16 * WL_BLUE;
		param.title_col = WL_LIGHTGREEN + 16 * WL_BLUE;
		param.scroll_col = WL_BLUE + 16 * WL_CYAN;
		param.icon_col = WL_WHITE + 16 * WL_LIGHTGRAY;
		param.button_col = WL_LIGHTGREEN + 16 * WL_BLUE;
		strncpy( param.title, ( u_char* ) "Source", MAX_HEADER_LEN );
		strncpy( param.user_border, "Ø¹Õ¾Íþ", 6 );
		param.father_window = debug_wid;
		param.iconic = 0;
		param.full_screen = 0;
		wl_open_win( &param, &source_wid, &x, &y, &dx, &dy );
		set_window_size( dx, dy, param.tab_step );
		source_win_open = TRUE;
	}
	if( ( fname = syms_val2line( addr, &line, 1 ) ) == NULL ) return( FALSE );
/*
	wl_printf( source_wid, "Addr = 0x%x\n\r", addr );
	wl_printf( source_wid, "File = %s, Line %d\n\r\n\r", fname, line );
*/
	display_file( source_wid, line, fname );
	return( TRUE );
}
