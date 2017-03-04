
#include <cinclude.h>
#include <stdio.h>
#include "io.h"



/* We need to copy the following declarations from clib/lib/startup.c
   so we don't need to link this file */
volatile void           *lib_copy_buffer;
volatile PORT_ID        FirstPortId;
volatile char           **environ; 


void c0_boot( void )
{
	int handle[] = { DEFAULT_FILE, DEFAULT_FILE, DEFAULT_FILE };

	asm( "movl %%ebx,%0" : "=m" (FirstPortId) : );
	init_basicio();
	lib_copy_buffer = GetCopyBuffer();

	environ = malloc( sizeof( char* ) );
	environ[ 0 ] = NULL;

	hermes_exec( "WINDOW", NULL, environ, handle );
	hermes_exec( "SHELL", NULL, environ, handle );
	//hermes_exec( "USER", NULL, environ, handle );
	//hermes_exec( "TETRIS", NULL, environ, handle );
	_exit( 0 );
}

