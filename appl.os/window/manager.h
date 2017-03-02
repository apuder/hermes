#ifndef __MANAGER__
#define __MANAGER__

enum win_manager_states {
		NORMAL, MOVING_WINDOW, RESIZING_WINDOW, IGNORE_LEFT_REL,
		IGNORE_RIGHT_REL
};


void send_dealloc_to_event_manager( WID wid );
void clip_frame( s_short old_x, s_short old_y, s_short old_dx,
				 s_short old_dy, u_char move_what );
void wm_process_mouse_event( mouse_info_type *mi, event_info_type *ev );


#endif
