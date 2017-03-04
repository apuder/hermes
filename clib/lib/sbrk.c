
#include <cinclude.h>


void *sbrk( int nbytes )
{
	k_p1 = nbytes;
	kernel_trap( K_SBRK );
	/*!!! Noch errno setzen */
	return( (void *) k_r1 );
}
