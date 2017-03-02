
#include <kernel.h>


/*
 * k_memcpy()
 *----------------------------------------------------------------------------
 * Copies memory from one virtual address space into another. It is
 * assumed that 'from_adr' is mapped in. This function doesn't not
 * re-map the pages but copies the data.
 */

ERROR k_memcpy( PHY_ADDR to_pdt, VIR_ADDR to_adr,
				PHY_ADDR from_pdt, VIR_ADDR from_adr, int nbytes )
{
	PHY_ADDR	source;
	PHY_ADDR	dest;
	PHY_ADDR	page;
	int i;

	if( nbytes == 0 ) return OK;
	if( !page_present( from_pdt, from_adr ) )
		shutdown( "k_memcpy(): No source addr" );
	source = vir_to_phy_addr( from_pdt, from_adr );
	if( !page_present( to_pdt, to_adr ) ) {
		page = get_memory_page();
		map_phy_addr( to_pdt, page, to_adr );
	}
	dest = vir_to_phy_addr( to_pdt, to_adr );
	for( i = 0; TRUE /* Test is done below! */; i++ ) {
		poke_mem_b( dest++, peek_mem_b( source++ ) );
		from_adr++;
		to_adr++;
		/*
		 * The test for exiting the loop has to be done before
		 * testing whether we cross a page boundary. We have
		 * to avoid mapping a new page to the destination
		 * process if no bytes are being copied.
		 */
		if( i == nbytes - 1 ) return OK;
		if( from_adr % PAGE_SIZE == 0 ) {
			if( !page_present( from_pdt, from_adr ) )
				shutdown( "k_memcpy(): No source addr" );
			source = vir_to_phy_addr( from_pdt, from_adr );
		}
		if( to_adr % PAGE_SIZE == 0 ) {
			if( !page_present( to_pdt, to_adr ) ) {
				page = get_memory_page();
				map_phy_addr( to_pdt, page, to_adr );
			}
			dest = vir_to_phy_addr( to_pdt, to_adr );
		}
	}
	return OK;
}
