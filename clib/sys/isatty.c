#include <stdio.h>


extern FILE_HANDLE_TYPE	*handle_buf;


int isatty( int handle )
{
	if( handle_buf[ handle ].handle_type == WINDOW_FILE ) {
		return( 1 );
	}
	else {
		return( 0 );
	}
}

