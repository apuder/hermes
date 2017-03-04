
#include <cinclude.h>


unsigned char InportB( short unsigned port )
{
	k_p1 = port;
	kernel_trap( K_INPORTB );
	return( (char) k_r1 );
}



void OutportB( short unsigned port, unsigned char byte )
{
	k_p1 = port;
	k_p2 = byte;
	kernel_trap( K_OUTPORTB );
}
