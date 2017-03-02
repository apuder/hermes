#ifdef __GNUC__

#include <cinclude.h>


#endif

#include <ci_types.h>
#include <winlib.h>
#include "win_serv.h"
#include "mevent.h"
#include "specials.h"


extern s_short		mouse_x, mouse_y;
extern u_char		screen_max_x, screen_max_y;
extern u_short		*screen_buf;
extern u_short		*video_base;
extern u_short		mouse_button;
extern u_short		grap_button;
extern u_char		mouse_graped;


static WIN_MANAGER_STATE	win_manager_state = NORMAL;
static s_short			df_x, df_y;
static s_short			df_dx, df_dy;
static s_short			orig_x, orig_y, orig_dx, orig_dy;
static s_short			lh_clip, hh_clip;
static s_short			lv_clip, hv_clip;
static u_char			min_dx = 0;
static u_char			min_dy = 0;
static u_short			last_bstate;
static u_short			delta_double = DELTA_DOUBLE;
static u_int			last_tick;



WIN_MANAGER_STATE manager_state()
{
    return( win_manager_state );
}



/*
  void activate_window( WID wid )
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
  ws_raise_window( wid, DISPLAY );
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
  ws_rebuild_screen();
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
*/


void clip_frame( s_short old_x, s_short old_y, s_short old_dx,
		 s_short old_dy, u_char move_what )
{
    if( move_what == WINDOW ) {
	if( df_dx < min_dx || df_dx > screen_max_x - 2 ||
	    df_x + df_dx - 1 < lh_clip || df_x > hh_clip ) {
	    df_dx = old_dx;
	    df_x = old_x;
	}

	if( df_dy < min_dy || df_dy > screen_max_y - 2 ||
	    df_y - 1 < lv_clip || df_y - 1 > hv_clip ) {
	    df_dy = old_dy;
	    df_y = old_y;
	}
    }
    else {
	if( df_x < 2 ) df_x = 2;
	if( df_y < 2 ) df_y = 2;
	if( df_x + df_dx > screen_max_x ) df_x = screen_max_x - 3;
	if( df_y + df_dy > screen_max_y ) df_y = screen_max_y - 1;
    }
}



void wm_process_mouse_event( MOUSE_INFO_TYPE *mi, EVENT_INFO_TYPE *ev,
			     u_char *new_act )
{
    CLIPS_TYPE			cp;
    LOCATION			loc;
    WIN_PARAM_TYPE		params;
    s_short			new_x, new_y;
    s_char			x, y;
    u_char			p;
    s_short			old_x, old_y, old_dx, old_dy;
    u_int			tick_diff;
    static WID			win_id;
    static u_char		move_what, click_pos;
    static s_short		abs_rel_dx, abs_rel_dy;
    static s_short		win_mouse_dx, win_mouse_dy;
    static u_char		old_mx, old_my;
    static WIN_PARAM_TYPE	param_save;

    if( mi->button ) {
	ws_set_button( mi->button );
    }
    ws_mouse_goto_dxdy( mi->mouse_dx, mi->mouse_dy );
    loc = ws_whereis_mouse( &ev->me.window_id, &x, &y, &p );
    if( ( mi->button & MOUSE_RELEASED ) && !( mouse_button & grap_button ) &&
	mouse_graped ) {
	ws_ungrap_mouse();
    }
    ws_get_window_info( ev->me.window_id, &params, &cp );
    *new_act = 0;

    switch( win_manager_state ) {

    case IGNORE_LEFT_REL:
	if( mi->button & LEFT_RELEASED ) {
	    win_manager_state = NORMAL;
	}
	ev->event_type = EV_NO_EVENT;
	break;

    case IGNORE_RIGHT_REL:
	if( mi->button & RIGHT_RELEASED ) {
	    win_manager_state = NORMAL;
	}
	ev->event_type = EV_NO_EVENT;
	break;

    case MOVING_WINDOW:
	if( mouse_x != old_mx || mouse_y != old_my ||
	    mi->button & LEFT_RELEASED ) {
	    ws_erase_dummy_frame( df_x, df_y, df_dx, df_dy, INTO_BUFFER );
	}
	if( mi->button & LEFT_RELEASED ) {
	    win_manager_state = NORMAL;
	    if( move_what == WINDOW ) {
		new_x = abs_rel_dx + df_x;
		new_y = abs_rel_dy + df_y;
		hw_puttext( 1, 1, screen_max_x, screen_max_y, screen_buf );
		ws_redraw_mouse( MOUSE_ALL );
		/*
		  restore_frame( df_x, df_y, df_dx, df_dy, screen_buf );
		*/
		if( new_x != orig_x || new_y != orig_y ) {
		    if( param_save.super_father == ws_actwid() ) {
			if( param_save.window_options & ALLOW_RAISE ) {
			    ws_raise_window( win_id, NO_DISPLAY );
			}
			ws_move_window( win_id, new_x, new_y, DISPLAY );
		    }
		    else {
			ws_move_window( win_id, new_x, new_y, DISPLAY );
			activate_window( win_id, DISPLAY );
			*new_act = 1;
		    }
		    ev->event_type = EV_WINDOW_MOVED;
		    ev->pe.window_id = win_id;
		    ev->pe.new_x = new_x;
		    ev->pe.new_y = new_y;
		}
		else {
		    if( param_save.super_father != ws_actwid() ) {
			activate_window( win_id, DISPLAY );
			ev->event_type = EV_SET_FOCUS;
			ev->fe.window_id = params.super_father;
			ev->fe.old_window_id = ws_last_actwid();
		    }
		    else {
			if( param_save.window_options & ALLOW_RAISE ) {
			    ws_raise_window( win_id, DISPLAY );
			}
			ev->event_type = EV_NO_EVENT;
		    }
		}
	    }
	    else {
		ws_def_icon_pos( win_id, df_x - 1, df_y - 1 );
		ev->event_type = EV_NO_EVENT;
	    }

#ifdef __GNUC__
	    OpenWindowServerPort();
#endif

	    return;
	}
	if( mouse_x != old_mx || mouse_y != old_my ) {
	    old_x = df_x; old_y = df_y;
	    old_dx = df_dx; old_dy = df_dy;
	    df_x = mouse_x - win_mouse_dx;
	    df_y = mouse_y - win_mouse_dy;
	    clip_frame( old_x, old_y, old_dx, old_dy, move_what );
	    ws_draw_dummy_frame( win_id, df_x, df_y, df_dx, df_dy, INTO_BUFFER );
	    /*
	      restore_frame( old_x, old_y, old_dx, old_dy, screen_buf );
	      restore_frame( df_x, df_y, df_dx, df_dy, screen_buf );
	    */
	    old_mx = mouse_x; old_my = mouse_y;
	    hw_puttext( 1, 1, screen_max_x, screen_max_y, screen_buf );
	    ws_redraw_mouse( MOUSE_ALL );
	}
	ev->event_type = EV_NO_EVENT;
	break;

    case RESIZING_WINDOW:
	if( mouse_x != old_mx || mouse_y != old_my ||
	    mi->button & LEFT_RELEASED ) {
	    ws_erase_dummy_frame( df_x, df_y, df_dx, df_dy, INTO_BUFFER );
	}
	if( mi->button & LEFT_RELEASED ) {
	    hw_puttext( 1, 1, screen_max_x, screen_max_y, screen_buf );
	    ws_redraw_mouse( MOUSE_ALL );
	    win_manager_state = NORMAL;
	    min_dx = min_dy = 0;
	    new_x = abs_rel_dx + df_x;
	    new_y = abs_rel_dy + df_y;
	    if( df_dx != orig_dx || df_dy != orig_dy ) {
		if( click_pos != BOTTOM_RIGHT ) {
		    ws_move_window( win_id, new_x, new_y, NO_DISPLAY );
		}
		if( param_save.super_father != ws_actwid() ) {
		    *new_act = 1;
		    ws_resize_window( win_id, df_dx, df_dy );
		    activate_window( win_id, DISPLAY );
		}
		else {
		    if( param_save.window_options & ALLOW_RAISE ) {
			ws_raise_window( win_id, NO_DISPLAY );
		    }
		    ws_resize_window( win_id, df_dx, df_dy );
		}
		ev->event_type = EV_WINDOW_RESIZED;
		ev->re.window_id = win_id;
		ev->re.cause = WIN_RESIZE;
		ev->re.new_dx = df_dx;
		ev->re.new_dy = df_dy;
	    }
	    else {
		hw_puttext( 1, 1, screen_max_x, screen_max_y, screen_buf );
		ws_redraw_mouse( MOUSE_ALL );
		if( ws_actwid() != param_save.super_father ) {
		    activate_window( win_id, DISPLAY );
		    ev->event_type = EV_SET_FOCUS;
		    ev->fe.window_id = params.super_father;
		    ev->fe.old_window_id = ws_last_actwid();
		}
		else {
		    if( param_save.window_options & ALLOW_RAISE ) {
			ws_raise_window( win_id, DISPLAY );
		    }
		    ev->event_type = EV_NO_EVENT;
		}
	    }

#ifdef __GNUC__
	    OpenWindowServerPort();
#endif

	    return;
	}

	if( old_mx != mouse_x || old_my != mouse_y ) {
	    old_x = df_x; old_y = df_y;
	    old_dx = df_dx; old_dy = df_dy;

	    switch( click_pos ) {
	    case BOTTOM_RIGHT:
		df_dx = mouse_x - df_x;
		df_dy = mouse_y - df_y;
		break;

	    case TOP_RIGHT:
		df_dx = mouse_x - df_x;
		df_dy = df_dy + ( df_y - mouse_y - 1 );
		df_y = mouse_y + 1;
		break;

	    case BOTTOM_LEFT:
		df_dy = mouse_y - df_y;
		df_dx = df_dx + ( df_x - mouse_x - 1 );
		df_x = mouse_x + 1;
		break;

	    case TOP_LEFT:
		df_dx = df_dx + ( df_x - mouse_x - 1 );
		df_x = mouse_x + 1;
		df_dy = df_dy + ( df_y - mouse_y - 1 );
		df_y = mouse_y + 1;
		break;

	    case RIGHT_BORDER:
		if( mouse_y < orig_y )
		    click_pos = TOP_RIGHT;
		if( mouse_y >= orig_y + orig_dy )
		    click_pos = BOTTOM_RIGHT;
		df_dx = mouse_x - df_x;
		break;

	    case LEFT_BORDER:
		if( mouse_y < orig_y )
		    click_pos = TOP_LEFT;
		if( mouse_y >= orig_y + orig_dy )
		    click_pos = BOTTOM_LEFT;
		df_dx = df_dx + ( df_x - mouse_x - 1 );
		df_x = mouse_x + 1;
		break;

	    case LOWER_BORDER:
		if( mouse_x < orig_x )
		    click_pos = BOTTOM_LEFT;
		if( mouse_x >= orig_x + orig_dx )
		    click_pos = BOTTOM_RIGHT;
		df_dy = mouse_y - df_y;
		break;
	    }

	    clip_frame( old_x, old_y, old_dx, old_dy, WINDOW );
	    ws_draw_dummy_frame( win_id, df_x, df_y, df_dx, df_dy, INTO_BUFFER );
	    hw_puttext( 1, 1, screen_max_x, screen_max_y, screen_buf );
	    ws_redraw_mouse( MOUSE_ALL );
	    old_mx = mouse_x; old_my = mouse_y;
	    ev->event_type = EV_NO_EVENT;
	}
	break;

    case NORMAL:

	/* The window the mouse points to, will be the active window by a
	   click of the left mouse button into the window area */

	if( ( loc == WINDOW_AREA || loc == AUX_BUTTON ||
	      ( loc >= SCROLL_UP && loc <= V_SCROLL_BLOCK ) )
	    && mi->button & LEFT_PRESSED &&
	    params.super_father != ws_actwid() &&
	    ev->me.window_id != ROOT_WINDOW ) {
	    activate_window( ev->me.window_id, DISPLAY );
	    ev->event_type = EV_SET_FOCUS;
	    ev->fe.window_id = params.super_father;
	    ev->fe.old_window_id = ws_last_actwid();
	    win_manager_state = IGNORE_LEFT_REL;
	    return;
	}

	/* A click to the icon button with the left mouse button iconifies
	   the window. If the window is the active one, the ROOT_WINDOW
	   will be activated.  */

	if( loc == ICON_BUTTON && mi->button & LEFT_PRESSED ) {
	    ev->event_type = EV_NO_EVENT;
	    win_manager_state = IGNORE_LEFT_REL;
	    ws_iconify_window( ev->me.window_id );
	    if( ev->me.window_id == ws_actwid() ) {
		*new_act = 1;
		activate_window( ws_get_top(), DISPLAY );
	    }
	    ev->event_type = EV_WINDOW_ICONED;
	    ev->ie.window_id = ev->me.window_id;
	    ev->ie.cause = WIN_ICON;
	    return;
	}

	/* A click to the icon area with the left mouse button deiconifies
	   the window. */

	if( loc == ICON_AREA && mi->button & LEFT_PRESSED ) {
	    ev->event_type = EV_WINDOW_ICONED;
	    ev->ie.window_id = ev->me.window_id;
	    ev->ie.cause = WIN_DEICON;
	    win_manager_state = IGNORE_LEFT_REL;
	    activate_window( ev->me.window_id, DISPLAY );
	    *new_act = 1;
	    ws_deiconify_window( ev->me.window_id );
	    return;
	}

	/* A click to the zoom button with the left mouse button zooms
	   and activates the window */

	if( loc == ZOOM_BUTTON && mi->button & LEFT_PRESSED &&
	    params.window_options & ALLOW_ZOOM ) {
	    win_manager_state = IGNORE_LEFT_REL;
	    ws_zoom_window( ev->me.window_id );
	    if( params.super_father != ws_actwid() ) {
		activate_window( ev->me.window_id, DISPLAY );
		*new_act = 1;
	    }
	    ws_get_window_info( ev->me.window_id, &params, &cp );
	    ev->event_type = EV_WINDOW_RESIZED;
	    ev->re.cause = WIN_ZOOM;
	    ev->re.new_dx = params.dx;
	    ev->re.new_dy = params.dy;
	    return;
	}

	/* A click to the unzoom button with the left mouse button unzooms
	   the window */

	if( loc == UNZOOM_BUTTON && mi->button & LEFT_PRESSED &&
	    params.window_options & ALLOW_ZOOM ) {
	    win_manager_state = IGNORE_LEFT_REL;
	    ws_unzoom_window( ev->me.window_id );
	    ws_get_window_info( ev->me.window_id, &params, &cp );
	    ev->event_type = EV_WINDOW_RESIZED;
	    ev->re.cause = WIN_UNZOOM;
	    ev->re.new_dx = params.dx;
	    ev->re.new_dy = params.dy;
	    return;
	}

	/* A click to the upper frame with the right mouse button drops the
	   window */

	if( ( loc == TOP_LEFT || loc == TOP_RIGHT ||
	      loc == UPPER_BORDER ) && mi->button & RIGHT_PRESSED ) {
	    ev->event_type = EV_NO_EVENT;
	    win_manager_state = IGNORE_RIGHT_REL;
	    ws_drop_window( ev->me.window_id );
	    return;
	}

	/* A click to the upper frame with the left mouse button raises and
	   moves and activates the window */

	if( ( loc == UPPER_BORDER || loc == ICON_BORDER ||
	      ( ( loc == TOP_LEFT || loc == TOP_RIGHT ||
		  loc == BOTTOM_LEFT || loc == BOTTOM_RIGHT ||
		  loc == LEFT_BORDER || loc == RIGHT_BORDER ||
		  loc == LOWER_BORDER ) &&
		!( params.window_options & ALLOW_RESIZE ) ) ) &&
	    mi->button & LEFT_PRESSED ) {

	    old_mx = mouse_x; old_my = mouse_y;
	    win_id = ev->me.window_id;
	    param_save = params;
	    if( loc == ICON_BORDER ) {
		move_what = ICON;
		df_x = params.icon_x + 1;
		df_y = params.icon_y + 1;
		df_dx = 3; df_dy = 1;
		lh_clip = 2; hh_clip = screen_max_x - 3;
		lv_clip = 2; hv_clip = screen_max_y - 1;
		win_mouse_dx = mouse_x - ( params.icon_x + 1 );
		win_mouse_dy = mouse_y - ( params.icon_y + 1 );
	    }
	    else {
		if( !( params.window_options & ALLOW_MOVE ) ) {
		    if( params.super_father != ws_actwid() ) {
			ev->event_type = EV_SET_FOCUS;
			ev->fe.window_id = params.super_father;
			ev->fe.old_window_id = ws_last_actwid();
			activate_window( ev->fe.window_id, DISPLAY );
		    }
		    else {
			if( params.window_options & ALLOW_RAISE ) {
			    ws_raise_window( ev->me.window_id, DISPLAY );
			}
			ev->event_type = EV_NO_EVENT;
		    }
		    win_manager_state = IGNORE_LEFT_REL;
		    return;
		}
		move_what = WINDOW;
		df_x = old_x = params.abs_x;
		df_y = old_y = params.abs_y;
		orig_x = params.x;
		orig_y = params.y;
		abs_rel_dx = params.x - params.abs_x;
		abs_rel_dy = params.y - params.abs_y;
		df_dx = old_dx = params.dx;
		df_dy = old_dy = params.dy;
		lh_clip = cp.lh; hh_clip = cp.hh;
		lv_clip = cp.lv; hv_clip = cp.hv;
		win_mouse_dx = mouse_x - params.abs_x;
		win_mouse_dy = mouse_y - params.abs_y;
	    }

	    ev->event_type = EV_NO_EVENT;
	    win_manager_state = MOVING_WINDOW;

#ifdef __GNUC__
	    CloseWindowServerPort();
#endif

	    ws_restore_mouse();
	    hw_gettext( 1, 1, screen_max_x, screen_max_y, screen_buf );
	    ws_draw_dummy_frame( ev->me.window_id, df_x, df_y, df_dx, df_dy, INTO_BUFFER );
	    hw_puttext( 1, 1, screen_max_x, screen_max_y, screen_buf );
	    /*
	      restore_frame( df_x, df_y, df_dx, df_dy, screen_buf );
	    */
	    ws_redraw_mouse( MOUSE_ALL );
	    return;
	}

	/* A click with the left mouse button to one of the edges of the
	   window will resize and activate the window */

	if( ( loc == TOP_LEFT || loc == TOP_RIGHT ||
	      loc == BOTTOM_LEFT || loc == BOTTOM_RIGHT ||
	      loc == LEFT_BORDER || loc == RIGHT_BORDER ||
	      loc == LOWER_BORDER ) &&
	    mi->button & LEFT_PRESSED ) {

	    if( !( params.window_options & ALLOW_RESIZE ) ) {
		if( params.super_father != ws_actwid() ) {
		    activate_window( ev->me.window_id, DISPLAY );
		    ev->event_type = EV_SET_FOCUS;
		    ev->fe.window_id = params.super_father;
		    ev->fe.old_window_id = ws_last_actwid();
		}
		else {
		    if( params.window_options & ALLOW_RAISE ) {
			ws_raise_window( ev->me.window_id, DISPLAY );
		    }
		    ev->event_type = EV_NO_EVENT;
		}
		win_manager_state = IGNORE_LEFT_REL;
		return;
	    }

	    click_pos = loc;
	    if( !( params.window_options & ALLOW_RESIZE ) ) {
		ev->event_type = EV_NO_EVENT;
		win_manager_state = IGNORE_LEFT_REL;
		return;
	    }

	    old_mx = mouse_x; old_my = mouse_y;
	    win_id = ev->me.window_id;
	    param_save = params;
	    min_dx = params.min_dx;
	    min_dy = params.min_dy;
	    df_x = orig_x = params.abs_x;
	    df_y = orig_y = params.abs_y;
	    abs_rel_dx = params.x - params.abs_x;
	    abs_rel_dy = params.y - params.abs_y;
	    df_dx = orig_dx = params.dx;
	    df_dy = orig_dy = params.dy;
	    lh_clip = cp.lh; hh_clip = cp.hh;
	    lv_clip = cp.lv; hv_clip = cp.hv;

	    ev->event_type = EV_NO_EVENT;
	    win_manager_state = RESIZING_WINDOW;

#ifdef __GNUC__
	    CloseWindowServerPort();
#endif

	    ws_restore_mouse();
	    hw_gettext( 1, 1, screen_max_x, screen_max_y, screen_buf );
	    ws_draw_dummy_frame( ev->me.window_id, df_x, df_y, df_dx, df_dy, INTO_BUFFER );
	    hw_puttext( 1, 1, screen_max_x, screen_max_y, screen_buf );
	    ws_redraw_mouse( MOUSE_ALL );
	    return;
	}

	/* The mouse event is not processed by the manager. All mouse infos
	   will be passed to the application */

	if( ( mouse_x != old_mx || mouse_y != old_my || mi->button ) &&
	    ( params.super_father == ws_actwid() ||
	      ev->me.window_id == ROOT_WINDOW ) && !params.iconic ) {

	    ev->me.button = 0;
	    if( mi->button & MOUSE_PRESSED ) {
		tick_diff = get_ticker_diff( last_tick );
		if( tick_diff <= delta_double ) {
		    ev->me.button = DOUBLE_CLICK;
		}
		last_bstate = mouse_button;
		last_tick = get_ticks();
	    }

	    old_mx = mouse_x;
	    old_my = mouse_y;
	    if( loc == SCROLL_LEFT || loc == SCROLL_RIGHT ||
		loc == SCROLL_UP   || loc == SCROLL_DOWN  ||
		loc == H_SCROLL_BLOCK || loc == V_SCROLL_BLOCK ||
		loc == H_SCROLL_BAR || loc == V_SCROLL_BAR ) {
		ev->event_type = EV_SCROLL_EVENT;
		ev->se.percent = p;
		ev->se.button |= mi->button;
		ev->se.loc = loc;
	    }
	    else {
		ev->event_type = EV_MOUSE_EVENT;
		ev->me.button |= mi->button;
		ev->me.x_pos = x;
		ev->me.y_pos = y;
		ev->me.loc = loc;
	    }
	}
	else {
	    ev->event_type = EV_NO_EVENT;
	}

	break;
    }
}
