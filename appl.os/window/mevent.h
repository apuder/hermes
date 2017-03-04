#ifndef __MANAGER__

#define __MANAGER__

#define DELTA_DOUBLE	4

typedef enum {
			NORMAL, MOVING_WINDOW, RESIZING_WINDOW, IGNORE_LEFT_REL,
			IGNORE_RIGHT_REL
} WIN_MANAGER_STATE;

/*
void restore_frame( s_short x, s_short y, s_short dx, s_short dy, u_short *buf );
*/

#ifdef __HERMES__
void send_dealloc_to_event_manager( WID wid );
#endif

WIN_MANAGER_STATE manager_state( void );
void clip_frame( s_short old_x, s_short old_y, s_short old_dx,
				 s_short old_dy, u_char move_what );
void wm_process_mouse_event( MOUSE_INFO_TYPE *mi, EVENT_INFO_TYPE *ev,
							 u_char *new_act );


#endif
