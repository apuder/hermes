
#ifndef __TIMER__
#define __TIMER__


#define MAX_TIMERS			( MAX_PROCS * 2 )

#define HZ					18
#define TIMER0				0x40
#define TIMER_MODE			0x43
#define IBM_FREQ			1193182
#define SQUARE_WAVE 		0x36



#define WAKEUP_BY_REPLY 	0
#define WAKEUP_BY_EVENT 	1
#define WAKEUP_BY_CALL		2


typedef void (*TIMER_FUNC) (void);


typedef struct {
	int			wakeup_mode;
	int			count;
	unsigned	timer_mask;
	TIMER_FUNC	func_to_call;
} TIMER_MSG;


typedef struct _TIMER {
	int 			wakeup_mode;
	TIMER_FUNC		func_to_call;
	unsigned		timer_mask;
	PID 			client_pid;
	int 			count;
	struct _TIMER	*next;
	struct _TIMER	*last;
	} TIMER;


extern int seconds_since_1970;

void timer_manager( PORT_ID first_port, HANDLE handle );


#endif
