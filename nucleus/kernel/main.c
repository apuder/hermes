
#include <kernel.h>
#include "io.h"



void kernel_main( void )
{
	init_global_params();
	init_basicio();
	cls();
	printf( "Booting HERMES... %d\n", 42 );
	while(1);
	print_global_params();
	init_gdt();
	init_tss();
	init_idt();
	init_paging();
}


void main_after_enable_paging( void )
{
	printf( "Paging enabled.\n" );
	init_kernel();
	init_page_fault();
	print_version();
	boot();
#if 1
	load_boot_process();
	asm( "sti" );
	k_suicide();
#if 0
	await_event();
	resign();
	for( ; ; ) ;
#endif
#endif
resign();
printf ("After resign()\n");
	asm( "sti" );
k_suicide();
printf ("After k_suicide\n");
	asm( "sti" );
	while (1) ;
}
