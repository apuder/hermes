
#include <ci_types.h>
#include <cinclude.h>
#include <winlib.h>
#include "db.h"


void single_step( void )
{
	int addr;

	while( TRUE ) {
		kernel_trap( K_NEXT_SINGLE_STEP );
		addr = k_r1;
		if( display_source( addr ) ) break;
	}
}
