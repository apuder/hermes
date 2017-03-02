
#include <kernel.h>
#include <kstdarg.h>
#include "io.h"


void shutdown( char *message, ... )
{
	char	buf[ 200 ];
	va_list	arg_ptr;

	asm( "cli" );
	va_start( arg_ptr, message );
	vsprintf( buf, message, arg_ptr );
	printf( "SHUTDOWN: %s\n", buf );
	va_end( arg_ptr );
	for( ; ; ) ;
}


int panic( int err_no, char *message )
{
	asm( "cli" );
	printf( "ERROR %d *****%s\n", err_no, message );
	for( ; ; );
}
