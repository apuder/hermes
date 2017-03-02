
#ifndef __PS2MOUSE__
#define __PS2MOUSE__


#define LEFT_PRESSED	1
#define RIGHT_PRESSED	2
#define MIDDLE_PRESSED	4
#define LEFT_RELEASED	8
#define RIGHT_RELEASED	16
#define MIDDLE_RELEASED	32

#define PS2MOUSE_INTR_NO	0x6C

#define MOUSE_SCALE_X	2
#define MOUSE_SCALE_Y	2

#define NUM_MOUSE_EVENT_SLOTS		40
#define NUM_INTS_PER_MOUSE_SLOT		3

extern EVENT_BUFFER mouse_event;


/* PS/2 Mouse initialization & installation */
void ps2mouse_init( HANDLE father_port );

/* PS/2 Mouse interrupt service routine */
void ps2mouse_isr( void );

#endif
