
#include <kernel.h>
#include <assert.h>
#include "io.h"


/* Pointer to the next free page in memory (free list) */
PRIVATE PHY_ADDR next_free_page;

/* Base of the global PD-entries */
PRIVATE PHY_ADDR base_global_pde;

/*
 * num_pde_entries contains the number of PD-entries that
 * have to be mapped into the page directory to cover the
 * complete physical memory (starting from base_global_pde).
 */

PRIVATE int num_pde_entries;


/*
 * All processes share the same system stack page table, since
 * every system stack is mapped to a different linear address.
 * system_stack_pt points to this global page.
 */

PRIVATE PHY_ADDR system_stack_pt;



/*
 * bzero_page
 *----------------------------------------------------------------------------
 * Zeros a 4 kB page in main memory.
 */

void bzero_page( PHY_ADDR addr )
{
    int i;

    assert( addr % PAGE_SIZE == 0 );
    for( i = 0; i < PAGE_SIZE / 4; i++ ) poke_mem_l( addr + i * 4, 0 );
}



/*
 * get_memory_page
 *----------------------------------------------------------------------------
 * Returns a physical address to a free page in memory.
 * -1 is returned if no more free page is available.
 */

PHY_ADDR get_memory_page( void )
{
    PHY_ADDR		addr;
    volatile int	i_flag;

    DISABLE_INTR( i_flag );
    assert( next_free_page != -1 );
    if( next_free_page == -1 ) {
	ENABLE_INTR( i_flag );
	return( -1 );
    }
    addr = next_free_page;
    next_free_page = peek_mem_l( addr );
    ENABLE_INTR( i_flag );
    bzero_page( addr );
    return( addr );
}



/*
 * free_memory_page
 *----------------------------------------------------------------------------
 * Returns a page to the free list.
 */

void free_memory_page( PHY_ADDR addr )
{
    volatile int i_flag;

    DISABLE_INTR( i_flag );
    assert( addr % PAGE_SIZE == 0 );
    poke_mem_l( addr, next_free_page );
    next_free_page = addr;
    ENABLE_INTR( i_flag );
}



BOOL page_belongs_to_free_list( PHY_ADDR a )
{
    /* Test if page is in ROM-BIOS area (640kB - 1 MB) */
    if( a >= 640 * 1024 || a < ONE_MB )
	return( FALSE );

    /* Test if page is within the File Server Cache */
    if( a < get_global_param( GLOBAL_PARAM_FS_CACHE ) * PAGE_SIZE )
	return( FALSE );
		
    return( TRUE );
}



void release_virtual_memory( PHY_ADDR pdt, VIR_ADDR kernel_stack )
{
    PHY_ADDR	pd_index;
    PHY_ADDR	pt_index;
    unsigned	pt;
    PHY_ADDR	phy_addr;
    int			i;

    /* First release the kernel-stack (one page) */
    assert( page_present( pdt, kernel_stack ) );
    phy_addr = vir_to_phy_addr( pdt, kernel_stack );
    free_memory_page( phy_addr );
    mark_not_present( pdt, kernel_stack );

    /* Now hunt down every page in the user part of the virtual memory */
    for( pd_index = ( VM_USER_BEGIN >> 22 ) << 2;
	 pd_index < PAGE_SIZE;
	 pd_index += 4 ) {

	pt = peek_mem_l( pdt + pd_index );
	if( pt & PRESENT_BIT ) {
		
	    /* Secondary page index exists, scan through all entries */
	    pt_index = pt & ~0xfff;
	    for( i = 0; i < NUM_PAGE_ENTRIES; i++ ) {
		phy_addr = peek_mem_l( pt_index + i * 4 );
		if( phy_addr & PRESENT_BIT ) {

		    /*
		     * Found a page that has to be released. Check if it
		     * belongs to the global free chain list or not.
		     * ( ie. page references pointing to the screen.
		     */
		    /*!!!Noch testen, ob ausgepagete Seite gelöscht werden muß!!*/				 
		    phy_addr &= ~0xfff;
		    if( page_belongs_to_free_list( phy_addr ) )
			free_memory_page( phy_addr );
		}
	    }

	    /* Finally, release secondary index page */
	    free_memory_page( pt_index );
	}
    }

    /* Last thing is to release the page directory */
    free_memory_page( pdt );
}



void init_global_pages( void )
{
    int 		i, j;
    int 		mb;
    PHY_ADDR	a;
    PHY_ADDR	addr;

    /* How much MB of RAM do we have? (Including ROM-BIOS, etc.) */
    mb = get_global_param( GLOBAL_PARAM_HI_MEM ) / 1024 + 1;

    /* One PDE-entry holds 4MB. How many PDE-entries do we need? */
    num_pde_entries = ( mb + 4 ) / 4;
    addr = 0;
    for( i = 0; i < num_pde_entries; i++ ) {
	a = get_memory_page();
	if( a >= ONE_MB )
	    shutdown( "init_global_pages(): FS cache too big" );
	bzero_page( a );
	if( i == 0 ) base_global_pde = a;
	for( j = 0; j < NUM_PAGE_ENTRIES; j++ ) {
	    poke_mem_l( a + j * 4, addr | PRESENT_BIT );
	    addr += 0x1000;  /* One entry covers 4 kB */
	}
    }
    system_stack_pt = get_memory_page();
    bzero_page( system_stack_pt );
}



/*
 * get_pdt_without_stack()
 *----------------------------------------------------------------------------
 * This function prepares a page directory for a new task. It allocates
 * a new PD and maps the global pages into it. Those global pages just
 * span the physical memory. The remaining PD-entries are marked as
 * not present.
 * NOTE: neither the system stack nor any user pages are allocated here!
 * Only the second level page table is mapped to linear address XXX
 * to the physical address stored in system_stack_pt.
 */

PHY_ADDR get_pdt_without_stack( void )
{
    int 		i;
    PHY_ADDR	pdt;
    int 		pt_base;

    pdt = get_memory_page();
    bzero_page( pdt );

    /* Global page tables start at phy. addr. base_global_pde */
    pt_base = base_global_pde;
    for( i = 0; i < num_pde_entries; i++ ) {
	poke_mem_l( pdt + i * 4, pt_base | PRESENT_BIT );
	pt_base += 0x1000;	/* Start of next global page table */
    }
    poke_mem_l( pdt + ( SYSTEM_STACK_BASE >> 20 ),
		system_stack_pt | PRESENT_BIT );
    return( pdt );
}



/*
 * page_present()
 *----------------------------------------------------------------------------
 * Tests if the page that v_addr maps to is present in memory.
 * Function returns TRUE if it is present, FALSE otherwise.
 */

int page_present( PHY_ADDR pdt, VIR_ADDR v_addr )
{
    PHY_ADDR	pd_index;
    PHY_ADDR	pt_index;

    pd_index = v_addr >> 22;
    pd_index <<= 2;
    pd_index += pdt;
    if( !( peek_mem_l( pd_index ) & PRESENT_BIT ) ) return( FALSE );
    pt_index = peek_mem_l( pd_index ) & ~0xfff;
    pt_index += ( ( v_addr >> 12 ) & 0x3ff ) << 2;
    if( !( peek_mem_l( pt_index ) & PRESENT_BIT ) ) return( FALSE );
    return( TRUE );
}



/*
 * mark_not_present()
 *----------------------------------------------------------------------------
 * The page mapped to by v_addr is marked as not present. This
 * function does not release the page!
 */

void mark_not_present( PHY_ADDR pdt, VIR_ADDR v_addr )
{
    PHY_ADDR	pd_index;
    PHY_ADDR	pt_index;

    pd_index = v_addr >> 22;
    pd_index <<= 2;
    pd_index += pdt;
    if( !( peek_mem_l( pd_index ) & PRESENT_BIT ) ) return;
    pt_index = peek_mem_l( pd_index ) & ~0xfff;
    pt_index += ( ( v_addr >> 12 ) & 0x3ff ) << 2;
    poke_mem_l( pt_index, peek_mem_l( pt_index ) & ~PRESENT_BIT );
    if( active_pcb->context_cr3 == pdt )
	INVALIDATE_PAGE_CACHE;
}



/*
 * map_phy_addr()
 *----------------------------------------------------------------------------
 * The page table pdt is modified in a way that access to the virtual
 * address v_addr will result in the physical address p_addr.
 * If the second level page table is not present (ie. doesn't
 * exist), it will be allocated.
 */

void map_phy_addr( PHY_ADDR pdt, PHY_ADDR p_addr, VIR_ADDR v_addr )
{
    PHY_ADDR	pd_index;
    PHY_ADDR	pt_index;

    pd_index = v_addr >> 22;
    pd_index <<= 2;
    pd_index += pdt;
    if( !( peek_mem_l( pd_index ) & PRESENT_BIT ) ) {
	pt_index = get_memory_page();
	bzero_page( pt_index );
	poke_mem_l( pd_index, pt_index |
		    PRESENT_BIT | USER_BIT | READ_WRITE_BIT );
    } else
	pt_index = peek_mem_l( pd_index ) & ~0xfff;
    pt_index += ( ( v_addr >> 12 ) & 0x3ff ) << 2;
    assert( ( peek_mem_l( pt_index ) & PRESENT_BIT ) == 0 );
    poke_mem_l( pt_index, ( p_addr & ~0xfff ) |
		PRESENT_BIT | USER_BIT | READ_WRITE_BIT );
    if( active_pcb->context_cr3 == pdt )
	INVALIDATE_PAGE_CACHE;
}



/*
 * vir_to_phy_addr
 *----------------------------------------------------------------------------
 * A virtual address is translated into a physical address. The base of
 * the PDT must be provided. If there exists no such virtual address
 * (ie. PRESENT-BIT == 0) a value of 0 is returned.
 */

PHY_ADDR vir_to_phy_addr( PHY_ADDR pdt, VIR_ADDR v_addr )
{
    PHY_ADDR pd_index;
    PHY_ADDR pt_index;

    pd_index = v_addr >> 22;
    pd_index <<= 2;
    pd_index += pdt;
    if( !( peek_mem_l( pd_index ) & PRESENT_BIT ) ) return( 0 );
    pt_index = peek_mem_l( pd_index ) & ~0xfff;
    pt_index += ( ( v_addr >> 12 ) & 0x3ff ) << 2;
    if( !( peek_mem_l( pt_index ) & PRESENT_BIT ) ) return( 0 );
    return( ( peek_mem_l( pt_index ) & ~0xfff ) | ( v_addr & 0xfff ) );
}



/*
 * get_pdt_with_stack()
 *----------------------------------------------------------------------------
 * A new page directory is allocated. The caller must provide a pointer
 * to a 4 kB system stack page and where it is to be hooked into the
 * linear address space (lin_addr).
 * The global OS-pages are automatically mapped into the PDT.
 */

PHY_ADDR get_pdt_with_stack( PHY_ADDR stack_addr, VIR_ADDR lin_addr )
{
    PHY_ADDR new_pdt;

    new_pdt = get_pdt_without_stack();
    map_phy_addr( new_pdt, stack_addr, lin_addr );
    return( new_pdt );
}



/*
 * get_pdt()
 *----------------------------------------------------------------------------
 * This function creates a the page directory. The global OS-pages are
 * mapped into the linear address space. Furthermore a 4 kB stack page
 * is allocated and mapped at linear address 'stack_lin_addr'.
 */

PHY_ADDR get_pdt( VIR_ADDR stack_lin_addr )
{
    PHY_ADDR sys_stack;

    sys_stack = get_memory_page();
    return( get_pdt_with_stack( sys_stack, stack_lin_addr ) );
}



PRIVATE void link_mem_pages( PHY_ADDR base, int num_pages, PHY_ADDR nil )
{
    PHY_ADDR	next_page;
    int 		i;

    for( i = 0; i < num_pages - 1; i++ ) {
	next_page = base + PAGE_SIZE;
	poke_mem_l( base, next_page );
	base = next_page;
    }
    poke_mem_l( base, nil );
}


/*
 * init_mem_pages
 *----------------------------------------------------------------------------
 * Links all available 4kB pages to a single
 * linked list. The free memory is split into three parts:
 *	   - Lo-mem (below 1MB)
 *	   - Free mem in the second MB (without TEXT, DATA, BSS
 *		 segment + one 4kB page for the system stack.
 *	   - Hi-mem above 2MB
 * The variable next_free_page points to the first free 4kB
 * page. The first int value of the page points to the next
 * free page. -1 denotes the end of the list.
 */


PRIVATE void init_mem_pages( void )
{
    PHY_ADDR	base1;
    PHY_ADDR	base2;
    PHY_ADDR	base3;
    PHY_ADDR	nil;
    int 		num_pages1;
    int 		num_pages2;
    int 		num_pages3;
    int 		os_size;

    base1 = 0;
    num_pages1 = get_global_param( GLOBAL_PARAM_LO_MEM ) / 4;

    os_size = get_global_param( GLOBAL_PARAM_OS_TEXT ) +
	get_global_param( GLOBAL_PARAM_OS_DATA ) +
	get_global_param( GLOBAL_PARAM_OS_BSS ) +
	get_global_param( GLOBAL_PARAM_OS_DEBUG );

    /* Align os_size to next page boundary */
    if( os_size % PAGE_SIZE )
	os_size = ( os_size / PAGE_SIZE + 1 ) * PAGE_SIZE;

    base2 = HERMES_TEXT_BASE + os_size;
    num_pages2 = ( 0x100000 - os_size - PAGE_SIZE ) / PAGE_SIZE;

    base3 = 0x200000;
    num_pages3 = ( get_global_param( GLOBAL_PARAM_HI_MEM ) - 0x100000 / 1024 ) / 4;

    nil = -1;
    if( num_pages3 != 0 ) {
	link_mem_pages( 0x200000, num_pages3, nil );
	nil = 0x200000;
    }
    link_mem_pages( base2, num_pages2, nil );
    link_mem_pages( base1, num_pages1, base2 );
    next_free_page = 0;
    printf( "Free pages  = %d\n", num_pages1 + num_pages2 + num_pages3 );
}



void alloc_fs_cache( void )
{
    PHY_ADDR	a;
    int		size_fs_cache;
    int 		i;

    a = 0;
    size_fs_cache = get_global_param( GLOBAL_PARAM_FS_CACHE );
    for( i = 0; i < size_fs_cache; i++ ) {
	a = get_memory_page();
	bzero_page( a );
    }
    if( a >= ONE_MB )
	shutdown( "alloc_fs_cache(): FS cache is too big" );
}



void enable_paging( PHY_ADDR pde )
{
    asm( "movl %0,%%eax" : : "m" (pde) );
    asm( "movl %eax,%cr3" );
    asm( "movl %cr0,%eax" );
    asm( "orl $0x80000000,%eax" );
    asm( "movl %eax,%cr0" );
    asm( "jmp flush_queue" );
    asm( "flush_queue: nop" );
    asm( "movl %0,%%eax" : : "i" (SYSTEM_STACK_BASE +
				  PAGE_SIZE * 2 - HERMES_TEXT_BASE) );
    asm( "movl %eax,%esp" );

    /*
     * Jump back to a function in main.c because all stack
     * references are now invalid (especially the EBP chain).
     */
    asm( "jmp main_after_enable_paging" );
}



void init_paging( void )
{
    PHY_ADDR	first_pdt;

    init_mem_pages();
    alloc_fs_cache();
    init_global_pages();
    first_pdt = get_pdt_with_stack( FIRST_SYS_STACK_PAGE,
				    SYSTEM_STACK_BASE + PAGE_SIZE );
    enable_paging( first_pdt );

    /* We'll never get back to here!!! */
    for( ; ; ) ;
}
