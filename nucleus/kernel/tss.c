
#include <kernel.h>


TSS_SEGMENT tss;


void init_tss( void )
{
	int i;

	for( i = 0; i < 22; i++ ) tss.regs[ i ] = 0;
	tss.link  = 0;
	tss.iomap = 0;
	tss.ss0   = DATA_SELECTOR;
	asm( "ltr %0" : : "m" (TSS_SELECTOR) );
#if 0
	printf( "init_tss(): Init done.\n" );
#endif
}
