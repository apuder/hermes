
#include <kernel.h>


VIR_ADDR k_sbrk( int nbytes )
{
	VIR_ADDR	p;

	/*!!! noch testen, ob ueberhaupt weiterer Speicher da ist */
	p = active_pcb->sbrk_ptr;
	active_pcb->sbrk_ptr += nbytes;
	return p;
}

