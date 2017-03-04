#ifndef __WINLIB__

#define __WINLIB__

#ifdef __GNUC__

#ifndef NULL
#define NULL			( ( void* ) 0 )
#endif

#endif

#define BORDER			3
#define HEADER          192
#define THIN_BORDER		1		/* The format of the window options */
#define THICK_BORDER	2		/* 	n N r F A V H S T T Z I R M B B					*/
#define USER_BORDER		3       /*  | | | | | | | | | | | | | | | |					*/
#define ALLOW_MOVE		4       /*	| | | | | | | | | | | | | | ----- Bordertype  	*/
#define ALLOW_RESIZE	8       /*	| | | | | | | | | | | | | ------- Movable	  	*/
#define ALLOW_ICON		16      /*	| | | | | | | | | | | | --------- Resizable		*/
#define ALLOW_ZOOM		32		/*	| | | | | | | | | | | ----------- Iconable		*/
#define LEFT_HEADER		64      /*	| | | | | | | | | | ------------- Zoomable		*/
#define RIGHT_HEADER	128     /*	| | | | | | | | ----------------- Headertype  	*/
#define MID_HEADER		192     /*	| | | | | | | ------------------- Shadow	  	*/
#define	PRINT_SHADOW	256     /*	| | | | | | --------------------- H-Scrollbar 	*/
#define HOR_SBAR		512     /*	| | | | | ----------------------- V-Scrollbar 	*/
#define VER_SBAR		1024    /*  | | | | ------------------------- Aux-Button  	*/
#define PRINT_AUX		2048	/*  | | | --------------------------- Father clip	*/
#define FATHER_CLIP		4096	/*	| | ----------------------------- Allow raise 	*/
#define ALLOW_RAISE		8192	/*  | ------------------------------- No def icon 	*/
#define NO_DEF_ICON		16384	/*	--------------------------------- No active win	*/

#define MAX_HEADER_LEN	30

#define DEFAULT_SIZE	160
#define DEFAULT_POS		160
#define DEFAULT_COL		0

#define ROOT_WINDOW		0

#define NO_EVENT		0
#define NEW_EVENT		1

#define LEFT_PRESSED	1
#define RIGHT_PRESSED	2
#define MIDDLE_PRESSED	4
#define LEFT_RELEASED	8
#define RIGHT_RELEASED	16
#define MIDDLE_RELEASED	32
#define DOUBLE_CLICK	64
#define MOUSE_PRESSED	7
#define MOUSE_RELEASED	56


#define WL_LINE_START	81
#define WL_LINE_STOP	87
#define WL_BLOCK_START	0
#define WL_BLOCK_STOP	87


#define WL_TIMER0			0x00000001
#define WL_TIMER1			0x00000002
#define WL_TIMER2			0x00000004
#define WL_TIMER3			0x00000008
#define WL_TIMER4			0x00000010
#define WL_TIMER5			0x00000020
#define WL_TIMER6			0x00000040
#define WL_TIMER7			0x00000080
#define WL_TIMER8			0x00000100
#define WL_TIMER9			0x00000200
#define WL_TIMER10			0x00000400
#define WL_TIMER11			0x00000800
#define WL_TIMER12			0x00001000
#define WL_TIMER13			0x00002000
#define WL_TIMER14			0x00004000
#define WL_TIMER15			0x00008000
#define WL_TIMER16			0x00010000
#define WL_TIMER17			0x00020000
#define WL_TIMER18			0x00040000
#define WL_TIMER19			0x00080000
#define WL_TIMER20			0x00100000
#define WL_TIMER21			0x00200000
#define WL_TIMER22			0x00400000
#define WL_TIMER23			0x00800000
#define WL_TIMER24			0x01000000
#define WL_TIMER25			0x02000000
#define WL_TIMER26			0x04000000
#define WL_TIMER27			0x08000000
#define WL_TIMER28			0x10000000
#define WL_TIMER29			0x20000000
#define WL_TIMER30			0x40000000
#define WL_TIMER31			0x80000000


typedef enum {
    WL_BLACK, WL_BLUE, WL_GREEN, WL_CYAN, WL_RED, WL_MAGENTA,
    WL_BROWN, WL_LIGHTGRAY, WL_DARKGRAY, WL_LIGHTBLUE, WL_LIGHTGREEN,
    WL_LIGHTCYAN, WL_LIGHTRED, WL_LIGHTMAGENTA, WL_YELLOW, WL_WHITE,
    WL_SHADOW_COL = 7
} WL_COLOR;

typedef enum {
    WL_LASTMODE=-1, WL_BW40=0, WL_C40, WL_BW80, WL_C80, WL_MONO=7,
    WL_C4350=64
} WL_TEXT_MODE;

typedef enum {
    EV_NO_EVENT, EV_MOUSE_EVENT, EV_WINDOW_RESIZED, EV_SCROLL_EVENT,
    EV_KEY_PRESSED, EV_WAKEUP, EV_SET_FOCUS, EV_KILL_FOCUS,
    EV_WINDOW_MOVED, EV_WINDOW_ICONED
} EV_TYPE;

typedef enum {
    WIN_RESIZE, WIN_ZOOM, WIN_UNZOOM, WIN_ICON, WIN_DEICON
} RESIZE_CAUSE;


typedef enum {
    SCROLL_UP, SCROLL_DOWN, SCROLL_LEFT, SCROLL_RIGHT, H_SCROLL_BAR,
    V_SCROLL_BAR, H_SCROLL_BLOCK, V_SCROLL_BLOCK, AUX_BUTTON,
    WINDOW_AREA, ICON_AREA, ICON_BORDER, UNZOOM_BUTTON, TOP_LEFT,
    TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT, LEFT_BORDER, RIGHT_BORDER,
    UPPER_BORDER, LOWER_BORDER, ZOOM_BUTTON, ICON_BUTTON
} LOCATION;

typedef enum {
    WL_NO_MONITOR, WL_MGA, WL_CGA, WL_R1, WL_EGA_COLOR,
    WL_EGA_MONO, WL_PGS_COLOR, WL_VGA_MONO, WL_VGA_COLOR,
    WL_R2, WL_R3, WL_MCGA_MONO, WL_MCGA_COLOR, WL_UNKNOWN = 0xff,
    WL_NOT_SUPPORTED = 0xfe
} WL_VIDEO_TYPE;


#ifdef __TURBOC__

typedef enum {
    ST_NORMAL, ST_SET_FOCUS
} EV_STATE;

#endif

typedef u_int  				WID;			/* ( Seq << 24 ) + Index */

typedef struct {
    WID						father_window;
    WID						super_father;
    u_char					tab_step;
    u_char					min_dx, min_dy;
    s_short					x, y, dx, dy;
    u_char					iconic : 1;
    u_char					full_screen : 1;
    u_short					window_options;
    WL_COLOR				border_col, title_col;
    WL_COLOR				screen_col, scroll_col;
    WL_COLOR				icon_col, button_col;
    u_char					icon_x, icon_y;
    s_short					abs_x, abs_y;
    u_char					title[ MAX_HEADER_LEN + 1 ];
    u_char					user_border[ 7 ];	/* Ð©®­Í¦ */
    u_char					icon_title[ 4 ];

#ifdef __GNUC__
    PID						pid;
#endif

} WIN_PARAM_TYPE;

typedef struct {
    EV_TYPE		event_type;
    WID			window_id;
    LOCATION	loc;
    u_short		button;
    s_short		x_pos;
    s_short		y_pos;
} MOUSE_EVENT;

typedef struct {
    EV_TYPE			event_type;
    WID				window_id;
    RESIZE_CAUSE    cause;
    s_short			new_dx;
    s_short			new_dy;
} RESIZE_EVENT;

typedef struct {
    EV_TYPE			event_type;
    WID				window_id;
    s_short			new_x;
    s_short			new_y;
} MOVE_EVENT;

typedef struct {
    EV_TYPE		event_type;
    WID			window_id;
    WID			old_window_id;
} FOCUS_EVENT;

typedef struct {
    EV_TYPE			event_type;
    WID				window_id;
    RESIZE_CAUSE	cause;
} ICON_EVENT;

typedef struct {
    EV_TYPE		event_type;
    WID			window_id;
    LOCATION	loc;
    s_short		button;
    u_char		percent;
} SCROLL_EVENT;

typedef struct {
    EV_TYPE		event_type;
    WID			window_id;
    u_short		key;
} KEY_EVENT;

typedef struct {
    EV_TYPE			event_type;
    u_int			timer_mask;
} TIMER_EVENT;

typedef union {
    EV_TYPE			event_type;
    MOUSE_EVENT		me;
    RESIZE_EVENT	re;
    MOVE_EVENT		pe;
    FOCUS_EVENT		fe;
    ICON_EVENT		ie;
    SCROLL_EVENT	se;
    KEY_EVENT		ke;
    TIMER_EVENT		te;
} EVENT_INFO_TYPE;

typedef struct {
    u_short	mouse_and_mask;
    u_short	mouse_xor_mask;
    WL_COLOR	def_screen_col;
    WL_COLOR	def_border_col;
    WL_COLOR	def_title_col;
    WL_COLOR	def_button_col;
    WL_COLOR	def_scroll_col;
    WL_COLOR	def_icon_col;
    u_char	act_border;
    u_char	pas_border;
    u_char	act_uborder[ 7 ];
    u_char	pas_uborder[ 7 ];
} DEFAULTS_TYPE;

typedef enum {
    NO_ERR, WS_INIT, WS_RANGE, WS_WINID, WS_ROOT, WS_NOMORE_WIN, WS_FATHER,
    WS_NOT_ALLOWED, WS_MEMORY, WS_NO_MOUSE, WS_COORDINATE, WS_GRAP, WS_UNGRAP
} ERR_CODE;

typedef ERR_CODE ( *WL_ERR_HANDLER ) ( WID wid, ERR_CODE ec );



extern u_short		up_arrow_and[ 16 ];
extern u_short		up_arrow_or[ 16 ];
extern u_short		hour_glass_and[ 16 ];
extern u_short		hour_glass_or[ 16 ];



ERR_CODE wl_def_err_handler( WID wid, ERR_CODE ec );
ERR_CODE wl_return_err_handler( WID wid, ERR_CODE ec );
WL_ERR_HANDLER wl_set_error_handler( WL_ERR_HANDLER handler );
s_char *wl_get_error_text( ERR_CODE ec );
u_char wl_test_for_event( EVENT_INFO_TYPE *ev );
void wl_wait_for_event( EVENT_INFO_TYPE *ev );
void wl_wait_for_specific_event( EVENT_INFO_TYPE *ev, EV_TYPE e_type );
ERR_CODE wl_init( u_char max_window );
ERR_CODE wl_close_win( WID wid );
ERR_CODE wl_scroll_up( WID wid, u_char x, u_char y, u_char dx, u_char dy );
ERR_CODE wl_scroll_down( WID wid, u_char x, u_char y, u_char dx, u_char dy );
ERR_CODE wl_scroll_left( WID wid, u_char x, u_char y, u_char dx, u_char dy );
ERR_CODE wl_scroll_right( WID wid, u_char x, u_char y, u_char dx, u_char dy );
ERR_CODE wl_open_win( WIN_PARAM_TYPE *p, WID *wid,
		      s_short *x, s_short *y, s_short *dx, s_short *dy );
ERR_CODE wl_define_border( WID wid, u_char *border );
ERR_CODE wl_modify_options( WID wid, u_short mask, u_short options );
ERR_CODE wl_printf( WID wid, s_char *format, ... );
ERR_CODE wl_raise_win( WID wid );
ERR_CODE wl_drop_win( WID wid );
void wl_set_background( u_short *img, u_char dx, u_char dy );
ERR_CODE wl_move_win( WID wid, s_short x, s_short y );
ERR_CODE wl_resize_win( WID wid, s_short dx, s_short dy );
ERR_CODE wl_zoom_win( WID wid );
ERR_CODE wl_unzoom_win( WID wid );
ERR_CODE wl_iconify_win( WID wid );
ERR_CODE wl_deiconify_win( WID wid );
ERR_CODE wl_define_icon( WID wid, s_short x, s_short y, WL_COLOR col,
			 u_char *title, u_char no_def );
ERR_CODE wl_move_icon( WID wid, s_short x, s_short y );
ERR_CODE wl_set_vscroll( WID wid, s_char percent );
ERR_CODE wl_set_hscroll( WID wid, s_char percent );
void wl_rebuild_screen( void );
ERR_CODE wl_activate_wid( WID wid );
ERR_CODE wl_get_win_info( WID wid, WIN_PARAM_TYPE *p );
ERR_CODE wl_clrscr( WID wid );
ERR_CODE wl_clreol( WID wid );
ERR_CODE wl_filleol( WID wid, u_char character );
ERR_CODE wl_gotoxy( WID wid, u_char x, u_char y );
ERR_CODE wl_get_textattr( WID wid, u_char *ta );
ERR_CODE wl_set_textattr( WID wid, u_char attrib );
ERR_CODE wl_get_csize( WID wid, u_short *size );
ERR_CODE wl_set_csize( WID wid, u_short size );
ERR_CODE wl_set_csizepc( WID wid, u_short start, u_short stop );
ERR_CODE wl_cursor_on( WID wid );
ERR_CODE wl_cursor_off( WID wid );
ERR_CODE wl_wherex( WID wid, u_char *x );
ERR_CODE wl_wherey( WID wid, u_char *y );
ERR_CODE wl_textmode( WL_TEXT_MODE mode );
ERR_CODE wl_grap_mouse( WID wid );
ERR_CODE wl_ungrap_mouse( void );
ERR_CODE wl_copy_to_window( WID wid, u_char *content );
ERR_CODE wl_save_dos( void );
ERR_CODE wl_restore_dos( void );
void wl_get_def( DEFAULTS_TYPE *def );
void wl_set_def( DEFAULTS_TYPE *def );
void wl_wake_me_up( u_int timer_ticks, u_int timer_mask );
u_char wl_screen_max_x( void );
u_char wl_screen_max_y( void );
void wl_change_grmouse( u_short *new_and, u_short *new_or );
void wl_exit( void );

u_char wl_screen_max_x( void );
u_char wl_screen_max_y( void );
u_char wl_screen_mode( void );
s_char wl_ascii( s_char a );

#endif
