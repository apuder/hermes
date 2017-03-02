
#include <kernel.h>
#include <assert.h>
#include "io.h"



PRIVATE unsigned global_param[ NUM_GLOBAL_PARAMS ];



unsigned get_global_param( int param_nr )
{
	assert( param_nr < NUM_GLOBAL_PARAMS );
	return( global_param[ param_nr ] );
}



void print_global_params( void )
{
	printf( "Screen base = 0x%x\n", global_param[ GLOBAL_PARAM_SCREEN_BASE ] );
	printf( "Video type  = %d\n", global_param[ GLOBAL_PARAM_VIDEO_TYPE ] );
	printf( "Lo-mem      = %d kB\n", global_param[ GLOBAL_PARAM_LO_MEM ] );
	printf( "Hi-mem      = %d kB\n", global_param[ GLOBAL_PARAM_HI_MEM ] );
	printf( "Text size   = %d Bytes\n", global_param[ GLOBAL_PARAM_OS_TEXT ] );
	printf( "Data size   = %d Bytes\n", global_param[ GLOBAL_PARAM_OS_DATA ] );
	printf( "BSS size    = %d Bytes\n", global_param[ GLOBAL_PARAM_OS_BSS ] );
	printf( "DEBUG size  = %d Bytes\n", global_param[ GLOBAL_PARAM_OS_DEBUG ] );
	printf( "FS-cache    = %d pages\n", global_param[ GLOBAL_PARAM_FS_CACHE ] );
}



void init_global_params( void )
{
	int i;

	for( i = 0; i < NUM_GLOBAL_PARAMS; i++ )
		global_param[ i ] = peek_mem_l( GLOBAL_PARAM_BASE + i * sizeof( int ) );
/*!!!!! Sollte durch run.exe vorgegeben werden!!!*/
global_param[ GLOBAL_PARAM_FS_CACHE ] = 64 / 4;
}

