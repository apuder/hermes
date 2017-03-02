
#ifndef __CINCLUDE__
#define __CINCLUDE__


#define USER_ADDR( x )  ( (x) + 132 * 0x100000 + 12 * 1024 - 0x100000 )

#ifndef __KERNEL__
#define KERNEL_PID 0xff00


typedef unsigned PHY_ADDR;
typedef unsigned PID;
typedef unsigned PORT_ID;
typedef unsigned HANDLE;
#endif

#define OK	0

/*=====>>> ostrap.c <<<=====================================================*/


#define K_MAP_SCREEN				1
#define K_SBRK						2
#define K_GET_SCREEN_MODE			3
#define K_GET_VIDEO_TYPE			4
#define K_INPORTB					5
#define K_OUTPORTB					6
#define K_GET_COPY_BUFFER			7
#define K_SEND						8
#define K_RECEIVE					9
#define K_REPLY						10
#define K_FS_SEND					11
#define K_CREATE_USER_PROC			12
#define K_EXIT						13
#define K_MESSAGE					14
#define K_CREATE_MOUSE_PROCESS		15
#define K_GET_MOUSE_EVENT			16
#define K_CREATE_PORT				17
#define K_CLOSE_PORT				18
#define K_OPEN_PORT					19
#define K_CLOSE_WINDOW_SERVER_PORT	20
#define K_OPEN_WINDOW_SERVER_PORT	21
#define K_EVENT_SEND				22
#define K_EVENT_MESSAGE				23
#define K_WINDOW_SERVER_SEND		24
#define K_TIMER_MANAGER_MESSAGE		25
#define K_REGISTER_WINDOW_SERVER	26
#define K_MAP_FONT_AREA				27
#define K_WAKEUP_PROCESS			28
#define K_SECS_SINCE_1970			29
#define K_START_SINGLE_STEP			30
#define K_HOOK_DEBUGGER				31
#define K_SET_DEBUG_FLAG			32
#define K_NEXT_SINGLE_STEP			33


extern volatile int k_p1;
extern volatile int k_p2;
extern volatile int k_p3;
extern volatile int k_p4;

extern volatile int k_r1;
extern volatile int k_r2;
extern volatile int k_r3;
extern volatile int k_r4;


void kernel_trap( int op );

/*=====>>> startup.c <<<====================================================*/


volatile extern void		*lib_copy_buffer;
volatile extern PORT_ID	FirstPortId;


void c0_startup( void );


/*=====>>> inout.c <<<======================================================*/

unsigned char InportB( short unsigned port );
void OutportB( short unsigned port, unsigned char byte );


/*=====>>> srr.c <<<========================================================*/

#define COPY_BUFFER_SIZE	( 12 * 1024 )


void *GetCopyBuffer( void );
void Send( PORT_ID hPort, void *pBuffer, int nSend, int *pnRecv );
void Message( PORT_ID hPort, void *pBuffer, int nSend );
void Receive( void *pBuffer, PID *phPid, int *pnSend );
void Reply( PID hPid, void *pBuffer, int nSend );

void CreatePort( PORT_ID *port );
void ClosePort( PORT_ID port );
void OpenPort( PORT_ID port );

void RegisterWindowServer( PORT_ID port );
void FileServerSend( void *pBuffer, int nSend, int *pnRecv );
void WindowServerSend( void *pBuffer, int nSend, int *pnRecv );
void EventManagerSend( void *pBuffer, int nSend, int *pnRecv );
void EventManagerMessage( void *pBuffer, int nSend );
void TimerManagerMessage( void *pBuffer, int nSend );



/*=====>>> exec.c <<<=======================================================*/

#define TOO_MANY_ARGS	-1

extern int hermes_exec_debug_flag;


int hermes_exec( const char *prg, const char **args, const char **environment,
			     int *handles );


/*=====>>> exit.c <<<=======================================================*/

#if 0
// Conflicts with exit() in std.h
void exit( int );
#endif


#endif
