/* Window-Tool für C
   --------------------------------------------------
   9/91 by WS
*/

#ifndef __WIN_SERV__

#define __WIN_SERV__

#define NO_DISPLAY		0
#define DISPLAY			1

#define NO_CHANGE		0

#define WINDOW			1
#define ICON			2

#define TRUE			1
#define FALSE			0

#define INTO_BUFFER		0
#define VIDEO_DIRECT	1
#define VIDEO_TESTED	2

#define MOUSE_MASK		64

#define GET_MAP_VAL( x, y )		screen_map[ screen_max_x * ( (y) - 1 ) \
											  + ( (x) - 1 ) ]
#define GET_BUFFER_VAL( x, y )	screen_buf[ screen_max_x * ( (y) - 1 ) \
											  + ( (x) - 1 ) ]

/*
#ifdef __TURBOC__
*/

#define GET_SCREEN_VAL( x, y )	video_base[ screen_max_x * ( (y) - 1 ) \
											  + ( (x) - 1 ) ]

#define WRITE_TO_SCREEN( x, y, v )	video_base[ screen_max_x * ( (y) - 1 ) \
												+ ( (x) - 1 ) ] = (v)
/*
#endif
*/

#ifdef __GNUC__

#define abs( x )	( (x) < 0 ? -(x) : (x) )

/*
extern u_int screen_base;
extern u_int screen_port;
extern u_char screen_max_x;

#define GET_SCREEN_VAL( x, y )	peek_mem_w( screen_base + \
											screen_max_x * ( (y) - 1 ) * 2 + \
											( (x) - 1 ) * 2 )

#define WRITE_TO_SCREEN( x, y, v )	poke_mem_w( screen_base + \
												screen_max_x * ( (y) - 1 ) * 2 + \
												( (x) - 1 ) * 2, (v) )
*/
#endif

#define WRITE_TO_MAP( x, y, v )	screen_map[ screen_max_x * ( (y) - 1 ) \
											  + ( (x) - 1 ) ] = (v)

#define WRITE_TO_BUFFER( x, y, v )	screen_buf[ screen_max_x * ( (y) - 1 ) \
												+ ( (x) - 1 ) ] = (v)

typedef enum {
			NO_MOUSE, MOUSE_LO, MOUSE_RO, MOUSE_LU, MOUSE_RU,
			MOUSE_ALL = 0xff
} MOUSE_CHAR;

typedef struct {
			s_short	lh;
			s_short	hh;
			s_short	lv;
			s_short	hv;
			u_short window_options;
} CLIPS_TYPE;

typedef struct {
			u_short		mouse_dx;
			u_short		mouse_dy;
			u_short		button;
} MOUSE_INFO_TYPE;

typedef enum { UP_DIR, DOWN_DIR, LEFT_DIR, RIGHT_DIR }	DIR_TYPE;


ERR_CODE ws_init( u_char max_window );
ERR_CODE ws_scroll_up( WID wid, u_char x, u_char y, u_char dx, u_char dy );
ERR_CODE ws_scroll_down( WID wid, u_char x, u_char y, u_char dx, u_char dy );
ERR_CODE ws_scroll_left( WID wid, u_char x, u_char y, u_char dx, u_char dy );
ERR_CODE ws_scroll_right( WID wid, u_char x, u_char y, u_char dx, u_char dy );
void ws_rebuild_screen( void );
void ws_redraw_mouse( MOUSE_CHAR m );
void ws_restore_mouse( void );
void ws_set_cursor( WID wid );
void ws_mouse_goto_dxdy( s_short dx, s_short dy );
ERR_CODE ws_close_window( WID wid, u_char mode );
ERR_CODE ws_open_window( WIN_PARAM_TYPE *param, WID *wid,
						 s_short *x, s_short *y, s_short *dx, s_short *dy );
ERR_CODE ws_define_border( WID wid, u_char *border );
ERR_CODE ws_modify_options( WID wid, u_short mask, u_short options );
ERR_CODE ws_print( WID wid, u_char *text );
ERR_CODE ws_raise_window( WID wid, u_char mode );
ERR_CODE ws_drop_window( WID wid );
void ws_set_background( u_short *image, u_char dx, u_char dy );
ERR_CODE ws_move_window( WID wid, s_short x, s_short y, u_char mode );
ERR_CODE ws_resize_window( WID wid, s_short dx, s_short dy );
ERR_CODE ws_zoom_window( WID wid );
ERR_CODE ws_unzoom_window( WID wid );
ERR_CODE ws_iconify_window( WID wid );
ERR_CODE ws_deiconify_window( WID wid );
ERR_CODE ws_define_icon( WID wid, s_short x, s_short y, WL_COLOR col,
						 u_char *title, u_char no_def );
ERR_CODE ws_def_icon_pos( WID wid, s_short x, s_short y );
ERR_CODE ws_set_vscroll( WID wid, s_char percent );
ERR_CODE ws_set_hscroll( WID wid, s_char percent );
ERR_CODE ws_draw_dummy_frame( WID wid, s_short x, s_short y,
							  s_short dx, s_short dy, u_char mode );
ERR_CODE ws_erase_dummy_frame( s_short x, s_short y, s_short dx, s_short dy,
							  u_char mode );
ERR_CODE ws_get_window_info( WID wid, WIN_PARAM_TYPE *param, CLIPS_TYPE *cp );
ERR_CODE ws_get_window_clips( WID wid, CLIPS_TYPE *cp );
ERR_CODE ws_clrscr( WID wid );
ERR_CODE ws_filleol( WID wid, u_char character );
ERR_CODE ws_gotoxy( WID wid, u_char x, u_char y );
ERR_CODE ws_get_textattr( WID wid, u_char *attrib );
ERR_CODE ws_set_textattr( WID wid, u_char attrib );
ERR_CODE ws_get_crs_size( WID wid, u_short *size );
ERR_CODE ws_set_crs_size( WID wid, u_short size );
ERR_CODE ws_cursor_on( WID wid );
ERR_CODE ws_cursor_off( WID wid );
ERR_CODE ws_wherex( WID wid, u_char *x );
ERR_CODE ws_wherey( WID wid, u_char *y );
LOCATION ws_whereis_mouse( WID *wid, s_char *x, s_char *y, u_char *percent );
ERR_CODE ws_text_mode( WL_TEXT_MODE mode );
void ws_get_defaults( DEFAULTS_TYPE *d );
void ws_set_defaults( DEFAULTS_TYPE *d );
WID ws_last_actwid( void );
WID ws_actwid( void );
WID ws_cursorwid( void );
ERR_CODE ws_set_actwid( WID wid, WID cwid );
u_char ws_screen_max_x( void );
u_char ws_screen_max_y( void );
u_char ws_screen_mode( void );
WID ws_get_cursor_wid( WID wid );
WID ws_get_top( void );
u_char ws_gr_mouse( void );
void ws_change_grmouse( u_short *new_and_mask, u_short *new_or_mask );
void ws_reload_font( void );
s_char ws_ascii( s_char a );
void ws_set_button( u_short button );
ERR_CODE ws_grap_mouse( WID wid );
ERR_CODE ws_ungrap_mouse( void );
ERR_CODE ws_copy_to_window( WID wid, u_char *content );
void activate_window( WID wid, u_char mode );

#ifdef __TURBOC__
ERR_CODE ws_activate_wid( WID wid );
ERR_CODE ws_save_dos( void );
ERR_CODE ws_restore_dos( void );
#endif

#ifdef __GNUC__
ERR_CODE ws_dup_window( WID *wid );
void ws_remove_windows( PID pid );
ERR_CODE ws_chown_handle( WID wid, PID pid );
#endif

#endif
