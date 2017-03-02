
#include <kernel.h>
#include <assert.h>
#include "io.h"


MEM_SEGMENT_DESCR gdt[ MAX_GDT_ENTRIES ];



void init_gdt( void )
{
    unsigned char	mem48[ 6 ];
    unsigned	*base_ptr;
    unsigned short	*limit_ptr;
    unsigned char	*p;
    unsigned	base;
    unsigned	limit;
    int		i;

    /* Sanity check */
    assert (sizeof (MEM_SEGMENT_DESCR) == GDT_ENTRY_SIZE);
    assert (sizeof (gdt) == GDT_ENTRY_SIZE * MAX_GDT_ENTRIES);

    /* Clear all GDT entries to 0 */
    p = (unsigned char *) gdt;
    for( i = 0; i < MAX_GDT_ENTRIES * GDT_ENTRY_SIZE; i++ )
	*p++ = 0;

    /* Initialize OS-code */
    gdt[ 1 ].d		= 1;
    gdt[ 1 ].dt		= 1;
    gdt[ 1 ].p		= 1;
    gdt[ 1 ].base_0_15	= HERMES_TEXT_BASE & 0xffff;
    gdt[ 1 ].base_16_23	= HERMES_TEXT_BASE >> 16;
    gdt[ 1 ].base_24_31	= HERMES_TEXT_BASE >> 24;
    gdt[ 1 ].limit_0_15	= get_global_param( GLOBAL_PARAM_OS_TEXT );
    gdt[ 1 ].limit_16_19	= get_global_param( GLOBAL_PARAM_OS_TEXT ) >> 16;
    gdt[ 1 ].type		= 10; /* Execute & Read */

    /* Initialize OS-data & stack selector */
    gdt[ 2 ].d		= 1;
    gdt[ 2 ].g		= 1;
    gdt[ 2 ].dt		= 1;
    gdt[ 2 ].p		= 1;
    gdt[ 2 ].base_0_15	= HERMES_TEXT_BASE & 0xffff;
    gdt[ 2 ].base_16_23	= HERMES_TEXT_BASE >> 16;
    gdt[ 2 ].base_24_31	= HERMES_TEXT_BASE >> 24;
    /* Limit is 4GB - HERMES_TEXT_BASE */
    limit = ( ((unsigned) 0xffffffff) - HERMES_TEXT_BASE ) / 4096;
    gdt[ 2 ].limit_0_15	= limit;
    gdt[ 2 ].limit_16_19	= limit >> 16;
    gdt[ 2 ].type		= 2; /* Read & Write */

    /* Initialize global data selector */
    gdt[ 3 ].d		= 1;
    gdt[ 3 ].g		= 1;
    gdt[ 3 ].dt		= 1;
    gdt[ 3 ].p		= 1;
    gdt[ 3 ].base_0_15	= 0;
    gdt[ 3 ].base_16_23	= 0;
    gdt[ 3 ].base_24_31	= 0;
    limit = 0xffffffff / 4096;
    gdt[ 3 ].limit_0_15	= limit;
    gdt[ 3 ].limit_16_19	= limit >> 16;
    gdt[ 3 ].type		= 2; /* Read & Write */

    /* Data & stack selector for user address space */
    gdt[ 4 ].d		= 1;
    gdt[ 4 ].g		= 1;
    gdt[ 4 ].dt		= 1;
    gdt[ 4 ].dpl		= 3; /* Should be 3 */
    gdt[ 4 ].p		= 1;
    gdt[ 4 ].base_0_15	= VM_USER_TEXT_BEGIN & 0xffff;
    gdt[ 4 ].base_16_23	= (VM_USER_TEXT_BEGIN >> 16) & 0xff;
    gdt[ 4 ].base_24_31	= VM_USER_TEXT_BEGIN >> 24;
    limit = ( VM_LARGE_USER_END - VM_USER_TEXT_BEGIN ) / 4096;
    gdt[ 4 ].limit_0_15	= limit;
    gdt[ 4 ].limit_16_19	= limit >> 16;
    gdt[ 4 ].type		= 2; /* Read & Write */

    /* Code selector for user address space */
    gdt[ 5 ].d		= 1;
    gdt[ 5 ].g		= 1;
    gdt[ 5 ].dt		= 1;
    gdt[ 5 ].dpl		= 3; /* Should be 3 */
    gdt[ 5 ].p		= 1;
    gdt[ 5 ].base_0_15	= VM_USER_TEXT_BEGIN & 0xffff;
    gdt[ 5 ].base_16_23	= (VM_USER_TEXT_BEGIN >> 16) & 0xff;
    gdt[ 5 ].base_24_31	= VM_USER_TEXT_BEGIN >> 24;
    limit = ( VM_LARGE_USER_END - VM_USER_TEXT_BEGIN ) / 4096;
    gdt[ 5 ].limit_0_15	= limit;
    gdt[ 5 ].limit_16_19	= limit >> 16;
    gdt[ 5 ].type		= 10; /* Execute & Read */

    /* TSS selector */
    gdt[ 6 ].d		= 1;
    gdt[ 6 ].g		= 0;
    gdt[ 6 ].dt		= 0;
    gdt[ 6 ].dpl		= 0;
    gdt[ 6 ].p		= 1;
    base = HERMES_TEXT_BASE + (unsigned) &tss;
    gdt[ 6 ].base_0_15	= base;
    gdt[ 6 ].base_16_23	= base >> 16;
    gdt[ 6 ].base_24_31	= base >> 24;
    gdt[ 6 ].limit_0_15	= sizeof( TSS_SEGMENT );
    gdt[ 6 ].limit_16_19	= sizeof( TSS_SEGMENT ) >> 16;
    gdt[ 6 ].type		= 9; /* Available 386 TSS */

    base_ptr = (unsigned *) &mem48[ 2 ];
    limit_ptr = (unsigned short *) &mem48[ 0 ];
    *base_ptr = (unsigned) gdt + HERMES_TEXT_BASE;
    *limit_ptr = MAX_GDT_ENTRIES * GDT_ENTRY_SIZE - 1;

    /* This is a bit of a kludge. mem48 is actually an input
       parameter of the lgdt operation. But this seems to be
       the only way to force an lvalue without further dereference */
    asm( "lgdt %0" : "=m" (mem48) );
    asm( "movw $0x10,%ax;movw %ax,%ds;movw %ax,%ss" );
    asm( "movw $0x18,%ax;movw %ax,%gs" );

    /* Reset NT bit in EFLAGS */
    asm( "pushfl;popl %eax;andl $0xffffbfff,%eax" );
    asm( "pushl %eax;popfl" );

#if 0
    printf( "init_gdt(): GDT initialized\n" );
#endif
}
