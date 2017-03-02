
#ifndef __EVENT__
#define __EVENT__


#define MAX_CLIENT_HASH		( 2 * MAX_WINDOW )
#define MAX_CLIENTS			MAX_PROCS
#define MAX_WINDOW_EVENTS	200
#define MAX_SAVE_EVENTS		20


typedef struct _WIN_EVENT {
	EVENT_INFO_TYPE		window_event;
	struct _WIN_EVENT	*next;
	} WIN_EVENT;


typedef struct _WIN_QUEUE {
	PID					owner_pid;
	int					num_events;
	WIN_EVENT			*event_queue;
	WIN_EVENT			*last_event;
	struct _WIN_QUEUE	*parent_win;
	struct _WIN_QUEUE	*next_win;
	} WIN_QUEUE;


typedef enum {
	WORKING,
	WAITING
	} CLIENT_STATE;


typedef struct _CLIENT_DATA {
	PID					pid;
	CLIENT_STATE		state;
	unsigned			timer_mask;
	WIN_QUEUE			*first_win;
	struct _CLIENT_DATA	*next;
	} CLIENT_DATA;


#endif
