
/*
 * History:
 *	0.1		June '91	: Initial kernel version. S/R/R. Primitive create().
 *	0.2		July '91	: Added kernel event handling. Ported MS-DOS version
 *						  of Hermes window server (without event manager).
 *	0.3		01.12.91	: Major re-write of header file structure. Erased
 *						  kernel/include. Made copy buffer more flexible.
 *						  Added new window server.
 *  0.4		25.12.91	: Renamed KERNEL to NUCLEUS. Added test_receive.
 *	0.5		Jan. '92	: Named NUCLEUS back to KERNEL!!!  :-)
 *	0.6		21.02.92	: Implemented user processes. Window server is now
 *						  loaded via demand paging. Pager enhanced to load
 *						  processes via demand paging. Added ktrap.c to
 *						  satisfy kernel traps.
 *  0.7		30.03.92	: Added first version of Vulture process.
 *	0.8		25.04.92	: Added debug facilities.
 */

#include <kernel.h>
#include "io.h"


#define MAJOR_VERSION	0
#define MINOR_VERSION	8



void print_version( void )
{
	printf( "Kernel version %d.%d, compiled %s, %s.\n",
				MAJOR_VERSION,
				MINOR_VERSION,
				__TIME__,
				__DATE__ );
}
