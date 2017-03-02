
#ifndef __MOUSE__
#define __MOUSE__


#define LEFT_PRESSED	1
#define RIGHT_PRESSED	2
#define MIDDLE_PRESSED	4
#define LEFT_RELEASED	8
#define RIGHT_RELEASED	16
#define MIDDLE_RELEASED	32

#define MOUSE_INTR_NO	0x64
#define ComPort		0x3f8	/* Com1: 0x3f8, Com2: 0x2f8 */
#define Int_Contr	0x20
#define IRQ_Mask	0x21

#define MOUSE_SCALE_X	2
#define MOUSE_SCALE_Y	2

#define NUM_MOUSE_EVENT_SLOTS		40
#define NUM_INTS_PER_MOUSE_SLOT		3

extern EVENT_BUFFER mouse_event;

void mouse_init( HANDLE father_port );

void mouse_isr( void );


#endif
