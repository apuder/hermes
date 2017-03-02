
#ifndef __TQUEUE__
#define __TQUEUE__


extern TIMER *next_free_timer_slot;
extern TIMER *blocked_list;

void enqueue_timer_client( PID pid, TIMER_MSG *msg );
void init_timer_queue( void );

#endif
