
#include <kernel.h>
#include <assert.h>


void move_copy_buffer( PHY_ADDR from_pdt, VIR_ADDR from_addr,
					   PHY_ADDR to_pdt, VIR_ADDR to_addr,
					   int nbytes )
{
	PHY_ADDR	copy_buffer;
	PHY_ADDR	old_copy_buffer;
	int			i, n;

	if( nbytes == 0 ) return;

	/* Re-map the copy buffer */
	n = nbytes / PAGE_SIZE;
	if( nbytes % PAGE_SIZE != 0 ) n++;
	for( i = 0; i < n; i++ ) {
		if( page_present( from_pdt, from_addr ) ) {
			copy_buffer = vir_to_phy_addr( from_pdt, from_addr );
			assert( copy_buffer != 0 );
			mark_not_present( from_pdt, from_addr );
			if( page_present( to_pdt, to_addr ) ) {

				/* We first have to erase the old copy buffer of the recv */
				old_copy_buffer = vir_to_phy_addr( to_pdt, to_addr );
				assert( old_copy_buffer != 0 );
				mark_not_present( to_pdt, to_addr );
				free_memory_page( old_copy_buffer );
			}
			map_phy_addr( to_pdt, copy_buffer, to_addr );
		}
		from_addr += PAGE_SIZE;
		to_addr += PAGE_SIZE;
	}
}
