
#include <cinclude.h>
#include <stdio.h>
#include "io.h"


extern int main( int argc, char **argv );

extern FILE_HANDLE_TYPE	*handle_buf;
extern FILE           	_iob[];

volatile void		*lib_copy_buffer;
volatile PORT_ID	FirstPortId;
volatile char		**environ;

void c0_startup( void )
{
	PID		pid;
	int		data_len, argc;
	int		ret;
	char	**argv;

	asm( "movl %%ebx,%0" : "=m" (FirstPortId) : );
	init_basicio();

	environ = malloc( sizeof( char* ) );
	environ[ 0 ] = NULL;

	lib_copy_buffer = GetCopyBuffer();

	Receive( lib_copy_buffer, &pid, &data_len );
	unpack_args( lib_copy_buffer, &argc, &argv );
	Receive( lib_copy_buffer, &pid, &data_len );
	unpack_args( lib_copy_buffer, &data_len, &environ );
	Receive( lib_copy_buffer, &pid, &data_len );
	open_init();
	bcopy( lib_copy_buffer, handle_buf, 3 * sizeof( FILE_HANDLE_TYPE ) );

	/*
	 * Start to single step if necessary.
	 */
	kernel_trap( K_START_SINGLE_STEP );
	ret = main( argc, argv );
	exit( ret );
}


