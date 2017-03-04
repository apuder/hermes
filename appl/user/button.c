
#include <ci_types.h>

#ifdef __HERMES__
#include <cinclude.h>
#endif

#include <winlib.h>
#include "button.h"



WID  define_button( WID father_win, u_int x, u_int y,
		    u_int dx, u_int dy, s_char *text )
{
    WID 		wid;
    WIN_PARAM_TYPE	param;
    s_short		i, x_pos, y_pos;
    s_short		wx, wy, wdx, wdy;

    param.min_dx = param.min_dy = 0;
    param.x = x; param.y = y;
    param.dx = dx; param.dy = dy;
    param.window_options = FATHER_CLIP;
    /*
      USER_BORDER | ALLOW_ICON | ALLOW_ZOOM | PRINT_AUX |
      MID_HEADER | VER_SBAR | HOR_SBAR | PRINT_SHADOW |
      ALLOW_RAISE | ALLOW_MOVE | ALLOW_RESIZE;
    */
    param.tab_step = 0;
    param.border_col = param.screen_col = WL_WHITE + 16 * WL_RED;
    /*
      param.title_col = WL_LIGHTGREEN + 16 * WL_BLUE;
      param.scroll_col = WL_BLUE + 16 * WL_CYAN;
      param.icon_col = WL_WHITE + 16 * WL_LIGHTGRAY;
      param.button_col = WL_LIGHTGREEN + 16 * WL_BLUE;
      strncpy( param.title, ( u_char* ) "TestHeader 1", MAX_HEADER_LEN );
    */
    strncpy( param.user_border, "Ø¹Õ¾Íþ", 6 );
    param.father_window = father_win;
    param.iconic = 0;
    param.full_screen = 0;
    wl_open_win( &param, &wid, &wx, &wy, &wdx, &wdy );
    for( i = 0; text[ i ] != '\0'; i++ ) ;
    i /= 2;
    x_pos = dx / 2 - i + 1;
    y_pos = dy / 2 + 1;
    wl_gotoxy( wid, x_pos, y_pos );
    wl_printf( wid, "%s", text );
    return wid;
}

