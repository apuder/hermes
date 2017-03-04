#include <signal.h>
#include <setjmp.h>
#include <time.h>

#if 0
struct tm *localtime( const time_t *time )
{
}



unsigned long time( unsigned long *time )
{
}
#endif


SignalHandler signal( int sig, SignalHandler action )
{
}


int setjmp( jmp_buf env )
{
}


void longjmp( jmp_buf env , int val )
{
	_exit( 1 );
}
