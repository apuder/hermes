
#include <cinclude.h>
#include <varargs.h>


void shutdown( const char *fmt, ... )
{
	va_list		argp;
	char		buf[ 160 ];

	hprintf( "SHUTDOWN: " );
	va_start( argp, fmt );
	hvsprintf( buf, fmt, argp );
	output_string( buf );
	va_end( argp );
	asm( "cli" );
}


void _exit( int code )
{
	kernel_trap( K_EXIT );
}

