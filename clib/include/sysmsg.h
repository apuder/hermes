

#ifndef __SYSMSG__
#define __SYSMSG__



/*=========================================================*/

#define OPEN_WINDOW			1
#define PRINT_WINDOW		2
#define CLS_WINDOW			3
#define GOTOXY_WINDOW		4
#define WS_GET_TEXTATTR		5
#define WS_SET_TEXTATTR		6
#define WS_CURSOR_ON		7
#define WS_CURSOR_OFF		8
#define CLOSE_WINDOW		9
#define WS_DUP_WINDOW		10
#define WS_DEFINE_BORDER	11
#define WS_MODIFY_OPTIONS	12
#define WS_RAISE_WINDOW		13
#define WS_DROP_WINDOW		14
#define WS_SET_BACKGROUND	15
#define WS_MOVE_WINDOW		16
#define WS_RESIZE_WINDOW	17
#define WS_ZOOM_WINDOW		18
#define WS_UNZOOM_WINDOW	19
#define WS_ICON_WINDOW		20
#define WS_DEICON_WINDOW	21
#define WS_DEF_ICON			22
#define WS_DEF_ICON_POS		23
#define WS_SET_VSCROLL		24
#define WS_SET_HSCROLL		25
#define WS_REBUILD_SCREEN	26
#define WS_WINDOW_INFO		27
#define WS_CLREOL			28
#define WS_GET_CSIZE		29
#define WS_SET_CSIZE		30
#define WS_SET_CSIZEPC		31
#define WS_WHEREX			32
#define WS_WHEREY			33
#define WS_SET_DEFAULT		34
#define WS_GET_DEFAULT		35
#define WS_SCREEN_INFO		36
#define WS_ASCII			37
#define WS_REMOVE_WINDOWS	38
#define WS_CHOWN_HANDLE		39


typedef struct {
	int				req;
	int				err;
	WID				wid;
	WIN_PARAM_TYPE	new_window;
} WINDOW_MSG;


typedef struct {
	int		req;
	WID		wid;
	char	string;
} PRINT_WINDOW_MSG;


typedef struct {
	int		req;
	int		wid;
    int     rc;
	int		param1;
	int		param2;
	int		param3;
} COMMON_WINDOW_MSG;


typedef struct {
	int		req;
	int		wid;
	int		x;
	int		y;
	int		col;
	int		no_def;
	char	title;
} DEF_ICON_MSG;

typedef struct {
	int				req;
	DEFAULTS_TYPE	def;
} WINDOW_DEF_MSG;


/*=========================================================*/

#define ALLOC_EVENT_QUEUE		1
#define ENQUEUE_WINDOW_EVENT	2
#define DEQUEUE_WINDOW_EVENT	3
#define ENQUEUE_TIMER_EVENT		4
#define DEALLOC_EVENT_QUEUE		5


typedef struct {
	int				req;
	PID				pid;
	WID				wid;
	WID				input_focus;
	WID				parent_wid;
} ALLOC_EVENT_MSG;


typedef struct {
	int				req;
	WID				wid;
	WID				input_focus;
} DEALLOC_EVENT_MSG;


typedef struct {
	int				req;
	WID				new_input_focus;
	EVENT_INFO_TYPE	event;
	} ENQUEUE_EVENT_MSG;

typedef struct {
	int				req;
	} DEQUEUE_EVENT_MSG;


typedef struct {
	int				req;
	unsigned		timer_mask;
	PID				client_pid;
	} ENQUEUE_TIMER_MSG;

#endif
