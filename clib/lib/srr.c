
#include <cinclude.h>



void *GetCopyBuffer( void )
{
	kernel_trap( K_GET_COPY_BUFFER );
	return( (void *) k_r1 );
}



void Send( PORT_ID hPort, void *pBuffer, int nSend, int *pnRecv )
{
	k_p1 = hPort;
	k_p2 = (int) pBuffer;
	k_p3 = nSend;
	kernel_trap( K_SEND );
	*pnRecv = k_r2;
}


void Message( PORT_ID hPort, void *pBuffer, int nSend )
{
	k_p1 = hPort;
	k_p2 = (int) pBuffer;
	k_p3 = nSend;
	kernel_trap( K_MESSAGE );
}



void Receive( void *pBuffer, PID *phPid, int *pnSend )
{
	k_p1 = (int) pBuffer;
	kernel_trap( K_RECEIVE );
	*phPid = k_r2;
	*pnSend = k_r3;
}


void Reply( PID hPid, void *pBuffer, int nSend )
{
	k_p1 = hPid;
	k_p2 = (int) pBuffer;
	k_p3 = nSend;
	kernel_trap( K_REPLY );
}



void CreatePort( PORT_ID *port )
{
	kernel_trap( K_CREATE_PORT );
	*port = k_r2;
}


void ClosePort( PORT_ID port )
{
	k_p1 = port;
	kernel_trap( K_CLOSE_PORT );
}



void OpenPort( PORT_ID port )
{
	k_p1 = port;
	kernel_trap( K_OPEN_PORT );
}


void RegisterWindowServer( PORT_ID port )
{
	k_p1 = port;
	kernel_trap( K_REGISTER_WINDOW_SERVER );
}


void CloseWindowServerPort( void )
{
	kernel_trap( K_CLOSE_WINDOW_SERVER_PORT );
}



void OpenWindowServerPort( void )
{
	kernel_trap( K_OPEN_WINDOW_SERVER_PORT );
}



void FileServerSend( void *pBuffer, int nSend, int *pnRecv )
{
	k_p1 = (int) pBuffer;
	k_p2 = nSend;
	kernel_trap( K_FS_SEND );
	*pnRecv = k_r2;
}


void WindowServerSend( void *pBuffer, int nSend, int *pnRecv )
{
	k_p1 = (int) pBuffer;
	k_p2 = nSend;
	kernel_trap( K_WINDOW_SERVER_SEND );
	*pnRecv = k_r2;
}



void EventManagerSend( void *pBuffer, int nSend, int *pnRecv )
{
	k_p1 = (int) pBuffer;
	k_p2 = nSend;
	kernel_trap( K_EVENT_SEND );
	*pnRecv = k_r2;
}



void EventManagerMessage( void *pBuffer, int nSend )
{
	k_p1 = (int) pBuffer;
	k_p2 = nSend;
	kernel_trap( K_EVENT_MESSAGE );
}



void TimerManagerMessage( void *pBuffer, int nSend )
{
	k_p1 = (int) pBuffer;
	k_p2 = nSend;
	kernel_trap( K_TIMER_MANAGER_MESSAGE );
}

