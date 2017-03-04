
#ifndef __KEYB__
#define __KEYB__

#define KEYB_INTR_NO				0x61

#define NUM_KEYB_EVENT_SLOTS		30
#define NUM_INTS_PER_KEYB_SLOT		1

extern EVENT_BUFFER keyb_event;

void keyb_init( HANDLE handle );

void keyb_isr( void );

#endif
