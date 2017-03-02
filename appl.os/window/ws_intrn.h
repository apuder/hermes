#ifndef __WS_INTRN__

#define __WS_INTRN__

#define MAX_WINDOW		25					/* Max. 256 */

#define SHADOW			32
#define INFO_MASK		31

#define MAX_DX			120
#define MAX_DY			60
#define MOUSE_SIZE		2
#define TAB_STEP		8

#define INVISIBLE		0
#define SHADOWED		1
#define	VISIBLE			2

#define MOUSE_1			0xd7
#define MOUSE_2			0xd8
#define MOUSE_3			0xd9
#define MOUSE_4			0xdd

#define DEF_BORDER_COL	( WL_BLUE << 4 ) + WL_WHITE
#define DEF_SCREEN_COL	( WL_BLUE << 4 ) + WL_WHITE
#define DEF_TITLE_COL	( WL_BLUE << 4 ) + WL_LIGHTGREEN
#define DEF_SCROLL_COL	( WL_CYAN << 4 ) + WL_BLUE
#define DEF_ICON_COL	( WL_LIGHTGRAY << 4 ) + WL_WHITE
#define DEF_BUTTON_COL	( WL_BLUE << 4 ) + WL_LIGHTGREEN

#define DEFAULT_ATTRIB	( WL_BLACK << 4 ) + WL_WHITE
#define DEFAULT_CHAR	32

#define ROOT_PTR		win_array[ 0 ].window

#if 0
#define VALIDATE_WID( wid )		( ( (wid) & 0xff ) < max_window_num && \
								( (wid) >> 8 ) == \
									win_array[ (wid) & 0xff ].seq_cnt  &&\
								win_array[ (wid) & 0xff ].window != NULL )
#endif

#define WIN_INFO( wid )			( ( (wid) & 0xffffff ) < max_window_num ) ? \
								win_array[ (wid) & 0xffffff].window : \
								dup_array[ ( (wid) & 0xffffff ) - \
											  max_window_num ].window

typedef struct _WIN_LIST_TYPE {
			WID						window_id;
			WID						cursor_wid;
			u_char					win_level;
			u_char					tab_step;
			u_char					full_screen : 1;
			u_char					iconic : 1;
			u_char					icon_moved : 1;
			u_char					scroll_flag : 1;
			u_char					cursor_on : 1;
			u_char					lh_clip, hh_clip, lv_clip, hv_clip;
			u_char					min_dx, min_dy;
			s_short					orig_dx, orig_dy;
			s_short					orig_x, orig_y;
			s_short					rel_x, rel_y;
			s_short					x, y, dx, dy;
			s_short					x_pos, y_pos;
			u_char					scroll_x, scroll_y;
			u_char					icon_x, icon_y;
			u_short					crs_size;
			u_short					window_options;
			WL_COLOR				border_col, title_col;
			WL_COLOR				screen_col, scroll_col;
			WL_COLOR 				icon_col, button_col;
			u_char					attrib;
			u_char					*title;
			u_char					icon_title[ 4 ];
			u_char					user_border[ 7 ];	/* Ð©®­Í¦ */
			u_short					*current_content;
			struct _WIN_LIST_TYPE	*father_ptr;
			struct _WIN_LIST_TYPE	*next_window;
			struct _WIN_LIST_TYPE	*prev_window;

#ifdef __HERMES__
			s_short					ref_cnt;
#endif

} WIN_LIST_TYPE;


typedef struct {
			u_char					seq_cnt;
			WIN_LIST_TYPE			*window;

#ifdef __HERMES__
			PID						pid;
#endif

} WIN_ARRAY_TYPE;


extern u_int 				max_window_num;
extern u_int				max_dup_num;
extern u_short				*screen_map;			/* Map: WWWWWWWW RMSLLLLL */
extern u_short				*screen_buf;			/* Buffer for redraw */
extern u_short				*video_base;			/* Base address of the VideoRam */
extern WIN_ARRAY_TYPE		*win_array;          	/* window table */
extern WIN_ARRAY_TYPE		*dup_array;
extern WIN_LIST_TYPE		*win_list, *win_last;	/* first & last window */
extern u_short				*scr_image;     		/* Image for screenbackground */
extern u_char				img_dx;					/* size of the screen */
extern u_char				img_dy;					/* image */
extern u_char				init_ok;
extern WID					active_wid;
extern WID					last_active_wid;
extern WID					cursor_wid;
extern u_char				screen_max_x, screen_max_y;
extern WL_TEXT_MODE			screen_mode;
extern WL_VIDEO_TYPE		video_type;
extern u_short				*upper_buf, *lower_buf;
extern u_short				*left_buf, *right_buf;
extern s_short				mouse_x, mouse_y;
extern u_char				gr_mouse_possible;
extern u_char				mouse_size;				/* size of mouse cursor */
extern u_short				mouse_and;				/* possible sizes are 1 */
extern u_short				mouse_xor;				/* and 2, others don't work */
extern s_short				old_mx;
extern s_short 				old_my;
extern u_char				mouse_installed;
extern u_char				mouse_button;
extern u_short				mouse_save[ 4 ];
extern s_short				mouse_px, mouse_py;
extern u_char				mouse_graped;
extern u_char				allow_grmouse;
extern WID					grap_wid;
extern LOCATION				grap_loc;
extern u_char				grap_button;
extern u_char				server_blocked;
extern DEFAULTS_TYPE		def;
extern DEFAULTS_TYPE		def_color;
extern DEFAULTS_TYPE		def_mono;

#ifdef __TURBOC__

extern u_short				*dos_screen;

#endif


int validate_wid( WID wid );
u_int check_coordinates( WIN_LIST_TYPE *win, u_int x, u_int y,
						 u_int dx, u_int dy );
u_short compute_cursorsize( u_short start, u_short stop );
u_char mouse_in_window( WID wid );
void get_next_wpos( s_short *x, s_short *y, s_short *dx, s_short *dy );
void gettext_buffer( u_char x1, u_char y1, u_char x2, u_char y2,
							void *content );
void puttext_buffer( s_char x1, s_char y1, s_char x2, s_char y2,
							void *content );
void restore_content( WIN_LIST_TYPE *win, u_char mode );
u_char coordinate_visible( WID wid, s_short x, s_short y, MOUSE_CHAR *ms );
void print_shadow( WIN_LIST_TYPE *win, u_char mode ) ;
void clear_content( WIN_LIST_TYPE *win );
void get_clips( WIN_LIST_TYPE *win );
void print_header( WIN_LIST_TYPE *win, u_char mode );
void print_left( WIN_LIST_TYPE * win, u_char mode );
void print_right( WIN_LIST_TYPE *win, u_char mode );
void print_bottom( WIN_LIST_TYPE *win, u_char mode );
void print_icon( WIN_LIST_TYPE *win, u_char mode );
void put_background( u_char mode );
void redraw_screen( WIN_LIST_TYPE *win );
void beep( void );
void check_active_window( WID *wid );
ERR_CODE check_input( WIN_PARAM_TYPE *p );
void open_cg( void );
void close_cg( void );
void get_chars( MOUSE_CHAR m );
void put_dummy_chars( MOUSE_CHAR m );
void gr_mouse_init( void );
void draw_gr_mouse( MOUSE_CHAR m );

#endif
