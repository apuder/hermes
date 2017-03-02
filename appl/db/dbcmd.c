
#include <ci_types.h>
#include <cinclude.h>
#include <winlib.h>
#include <sysmsg.h>
#include "db.h"



char *help_text[] = {
	"HERMES Debugger V0.4",
	"====================",
	"",
	"h|?           - This help screen",
	"c|cls         - Clear screen",
	"r             - Display registers",
	"w <pid>       - Wakeup suspended process",
	"ps            - Show processes",
	"gdt           - Show GDT",
	"v|var         - Show variables",
	"d <e1>[,<e2>] - Displays the physical memory starting at address <e1>",
	"                Optional parameter <e2> specifies length of output in bytes",
	"d             - Displays 256 bytes starting from the previous position",
	"t <e1>,<e2>   - Translates and displays virtual address <e2> according",
	"                to page directory <e1> ( = physical base of PD )",
	"!",
	"Expression syntax",
	"=================",
	"",
	"<expr>      : <term>",
	"            : ~ <term>",
	"            : <expr> +|-|&|'|'|>>|<< <term>",
	"",
	"<term>      : <factor>",
	"            : <term> *|/|% <factor>",
	"",
	"<factor>    : <number>",
	"            : - <number>",
	"            : <last_expr>",
	"            : ( <expr> )",
	"",
	"<number>    - Either a decimal or a binary (prefix 0b) or a octal (prefix 0)",
	"              or a hexadecimal (prefix 0x).",
	"<last_expr> - The last computed expression is denoted by \"",
	"",
	"",
	"Kludged by AP 8/91"
	};


char *sys_segment[] = {
	/*	0 */ "Undef",
	/*	1 */ "Avail 286 TSS",
	/*	2 */ "LDT",
	/*	3 */ "Busy 286 TSS",
	/*	4 */ "286 Call Gate",
	/*	5 */ "Task Gate",
	/*	6 */ "286 Intr Gate",
	/*	7 */ "286 Trap Gate",
	/*	8 */ "Undef",
	/*	9 */ "Avail 386 TSS",
	/* 10 */ "Undef",
	/* 11 */ "Busy 386 TSS",
	/* 12 */ "386 Call Gate",
	/* 13 */ "Undef",
	/* 14 */ "386 Intr Gate",
	/* 15 */ "386 Trap Gate" };


char *mem_segment[] = {
	/*	0 */ "---R--",
	/*	1 */ "A--R--",
	/*	2 */ "---RW-",
	/*	3 */ "A--RW-",
	/*	4 */ "-D-R--",
	/*	5 */ "AD-R--",
	/*	6 */ "-D-RW-",
	/*	7 */ "AD-RW-",
	/*	8 */ "-----E",
	/*	9 */ "A----E",
	/* 10 */ "---R-E",
	/* 11 */ "A--R-E",
	/* 12 */ "--C--E",
	/* 13 */ "A-C--E",
	/* 14 */ "--CR-E",
	/* 15 */ "A-CR-E" };



char *state_table[] = {
	"Ready",
	"Sema blocked",
	"Send blocked",
	"Reply blocked",
	"Receive blocked",
	"Message blocked",
	"Event blocked",
	"Suspended"
	};


unsigned last_hex_dump_addr = 0;


unsigned eflags, cs, eip, eax, ecx, edx, ebx, esp, ebp, esi, edi;



/*
 * db_display_regs()
 *----------------------------------------------------------------------------
 * Displays the 80386 registers.
 */

void db_display_regs( void )
{
	wl_printf( debug_wid, "cs  =     %04x  eip = %08x\n\r", cs & 0xffff, eip );
	wl_printf( debug_wid, "eax = %08x  ebx = %08x  ecx = %08x  edx = %08x\n\r",
				 eax, ebx, ecx, edx );
	wl_printf( debug_wid, "esi = %08x  edi = %08x  ebp = %08x  esp = %08x\n\r",
				 esi, edi, ebp, esp );
	wl_printf( debug_wid, "efl = %08x\n\r", eflags );
	wl_printf( debug_wid, "      " );
	if( eflags & FLAG_CF ) wl_printf( debug_wid, "CF " );
	if( eflags & FLAG_PF ) wl_printf( debug_wid, "PF " );
	if( eflags & FLAG_AF ) wl_printf( debug_wid, "AF " );
	if( eflags & FLAG_ZF ) wl_printf( debug_wid, "ZF " );
	if( eflags & FLAG_SF ) wl_printf( debug_wid, "SF " );
	if( eflags & FLAG_OF ) wl_printf( debug_wid, "OF " );
	if( eflags & FLAG_TF ) wl_printf( debug_wid, "TF " );
	if( eflags & FLAG_IF ) wl_printf( debug_wid, "IF " );
	if( eflags & FLAG_DF ) wl_printf( debug_wid, "DF " );
	if( eflags & FLAG_NT ) wl_printf( debug_wid, "NT " );
	if( eflags & FLAG_RF ) wl_printf( debug_wid, "RF " );
	if( eflags & FLAG_VM ) wl_printf( debug_wid, "VM " );
	wl_printf( debug_wid, "IOPL = %d\n\r\n\r", ( eflags >> 12 ) & 3 );
}



/*
 * db_show_gdt()
 *----------------------------------------------------------------------------
 * Displays the global descriptor table.
 */

void db_show_gdt( void )
{
#if 0
	unsigned char		saved_gdt[ 6 ];
	MEM_SEGMENT_DESCR	gdt;
	unsigned char		*gdt_ptr;
	unsigned			*base_ptr;
	unsigned short		*limit_ptr;
	unsigned			limit;
	int 				gdt_entries;
	int 				x, y;

	wl_printf( debug_wid, "GLOBAL DESCRIPTOR TABLE:\n\r" );
	wl_printf( debug_wid, "========================\n\r\n\r" );
	asm( "sgdt %0" : "=m" (saved_gdt) : );
	base_ptr = (unsigned *) &saved_gdt[ 2 ];
	limit_ptr = (unsigned short *) &saved_gdt[ 0 ];
	wl_printf( debug_wid, "Base    = 0x%x\n\r", *base_ptr );
	gdt_entries = (unsigned) ( (*limit_ptr + 1) / GDT_ENTRY_SIZE );
	wl_printf( debug_wid, "Entries = %d\n\r\n\r", gdt_entries );
	for( x = 0; x < gdt_entries; x++ ) {
		if( x % 10 == 0 ) {
/*
  #  Type		  Access  DPL	   Base  Slct  Limit/Ofs  Count  P	G  D  AVL
...  ....  .............   ..  ........  ....	........	 ..  .	.  .	.
*/
			wl_printf( debug_wid, "  #  Type         Access  DPL      Base" );
			wl_printf( debug_wid, "  Slct  Limit/Ofs  Count  P  G  D  AVL\n\r" );
			wl_printf( debug_wid, "---------------------------------------" );
			wl_printf( debug_wid, "--------------------------------------\n\r" );
		}
		gdt_ptr = (unsigned char *) &gdt;
		for( y = 0; y < GDT_ENTRY_SIZE; y++ )
			*gdt_ptr++ = peek_mem_b( *base_ptr + x * GDT_ENTRY_SIZE + y );
		wl_printf( debug_wid, "%3x  ", x ); /* # */
		if( gdt.dt ) {

			/* Memory segment */
			wl_printf( debug_wid, " MEM  " );
			wl_printf( debug_wid, "%13s   %2u  %8x     -  ", mem_segment[ gdt.type ],
				gdt.dpl, gdt.base_0_23 | ( gdt.base_24_31 << 24 ) );
			limit = gdt.limit_0_15 | ( gdt.limit_16_19 << 16);
			if( gdt.g )
				limit = ( limit << 12 ) + 0xfff;
			wl_printf( debug_wid, " %8x      -  ", limit );
			wl_printf( debug_wid, "%d  %d  %d    %d", gdt.p, gdt.g, gdt.d, gdt.avl );
		}
		if( gdt.dt == 0 && ( gdt.type == 0 || gdt.type == 1 ||
			gdt.type == 2 || gdt.type == 3 || gdt.type == 8 ||
			gdt.type == 9 || gdt.type == 10 || gdt.type == 11 ||
			gdt.type == 13 ) ) {

			/* System segment */
			wl_printf( debug_wid, " SYS  " );
			wl_printf( debug_wid, "%13s   %2u  %8x     -  ", sys_segment[ gdt.type ],
				gdt.dpl, gdt.base_0_23 | ( gdt.base_24_31 << 24 ) );
			limit = gdt.limit_0_15 | ( gdt.limit_16_19 << 16 );
			if( gdt.g )
				limit = ( limit << 12 ) + 0xfff;
			wl_printf( debug_wid, " %8x      -  ", limit );
			wl_printf( debug_wid, "%d  %d  -    %d", gdt.p, gdt.g, gdt.avl );
		} else {

			/* Gate */
		}
		wl_printf( debug_wid, "\n\r" );
	}
	wl_printf( debug_wid, "\n\r" );
#endif
wl_printf( debug_wid, "Not implemented yet!\n\r" );
}



/*
 * db_wakeup()
 *----------------------------------------------------------------------------
 * Wakeup a suspended process.
 */

void db_wakeup( PID pid )
{
#if 0
	if( !VALIDATE_PID( pid ) ) {
		wl_printf( debug_wid, "Bad PID!\n\r" );
		return;
	}
	if( pcb[ pid & BYTE ].state != STATE_SUSPENDED ) {
		wl_printf( debug_wid, "Process not suspended!\n\r" );
		return;
	}
	k_wakeup( pid );
	wl_printf( debug_wid, "OK.\n\r" );
#endif
wl_printf( debug_wid, "Not implemented yet!\n\r" );
}



/*
 * db_show_var()
 *----------------------------------------------------------------------------
 * Displays the variables.
 */

void db_show_var( void )
{
	int i;

	wl_printf( debug_wid, "\n\r" );
	for( i = 0; i < 9; i++ )
		wl_printf( debug_wid, "!%d = %-8d (0x%x)\n\r", i + 1,
								 db_value[ i ], db_value[ i ] );
	wl_printf( debug_wid, "\n\r" );
}



/*
 * db_show_pcb()
 *----------------------------------------------------------------------------
 * Displays the process control block.
 */

void db_show_pcb( void )
{
#if 0
	PCB *p;
	int i;

/*
     PID   State            Prio        CR3   Name
........   ...............    ..   ........   .....................
*/
	wl_printf( debug_wid, "     PID   State            Prio        CR3   Name\n\r" );
	wl_printf( debug_wid, "---------------------------------------------------------------\n\r" );
	for( i = 0; i < MAX_PROCS; i++ ) {
		p = &pcb[ i ];
		if( p->used ) {
			wl_printf( debug_wid, "%08x   ", conv_pcb_to_pid( p ) );
			wl_printf( debug_wid, "%-15s    ", state_table[ p->state ] );
			wl_printf( debug_wid, "%2d   ", p->priority );
			wl_printf( debug_wid, "%8x   ", p->context_cr3 );
			wl_printf( debug_wid, "%-21s\n\r", p->name );
		}
	}
#endif
wl_printf( debug_wid, "Not implemented yet!\n\r" );
}



/*
 * db_translate()
 *----------------------------------------------------------------------------
 * Translates a virtual into a physical address.
 */

void db_translate( unsigned pdt, unsigned v_addr )
{
#if 0
	if( pdt % 4096 != 0 ) {
		wl_printf( debug_wid, "First expression must have lower 12 bits cleared to zero!\n\r" );
		return;
	}
	if( !page_present( pdt, v_addr ) ) {
		wl_printf( debug_wid, "Physical address does not exist!\n\r" );
		return;
	}
	wl_printf( debug_wid, "Virtual address 0x%x maps to 0x%x\n\r",
				v_addr, vir_to_phy_addr( pdt, v_addr ) );
#endif
wl_printf( debug_wid, "Not implemented yet!\n\r" );
}



void gen_hex_dump( unsigned addr )
{
#if 0
	int x;
	int a;

	a = addr;
	wl_printf( debug_wid, "%08x: ", addr );
	for( x = 0; x < 16; x++ )
		wl_printf( debug_wid, "%02x ", peek_mem_b( a++ ) );
	wl_printf( debug_wid, " " );
	a = addr;
	for( x = 0; x < 16; x++ ) {
		if( peek_mem_b( a ) < ' ' )
			wl_printf( debug_wid, "." );
		else
			wl_printf( debug_wid, "%c", peek_mem_b( a ) );
		a++;
	}
	wl_printf( debug_wid, "\n\r" );
#endif
wl_printf( debug_wid, "Not implemented yet!\n\r" );
}



/*
 * db_hex_dump()
 *----------------------------------------------------------------------------
 * Generates a hex dump listing of the physical memory.
 */

void db_hex_dump( unsigned addr, int len )
{
	int x;

	if( addr == 0xffffffff ) addr = last_hex_dump_addr;
	for( x = 0; x < len / 16; x++ )
		gen_hex_dump( addr + x * 16 );
	last_hex_dump_addr = addr + len;
}



/*
 * db_help_debug()
 *----------------------------------------------------------------------------
 * Displays help texts for the debugger.
 */

void db_help_debug( void )
{
	EVENT_INFO_TYPE	event;
	int				x;
	int				c;

	wl_clrscr( debug_wid );
	for( x = 0; x < sizeof( help_text ) / sizeof( char * ); x++ ) {
		if( *help_text[ x ] == '!' ) {
			wl_printf( debug_wid, "\n\r\n\r>>>>> Hit any key to continue " );
			wl_cursor_on( debug_wid );
			do {
				wl_wait_for_event( &event );
			} while( event.event_type != EV_KEY_PRESSED );
			wl_cursor_off( debug_wid );
			c = event.ke.key;
			if( c == 3 || c == 27 ) {
				wl_printf( debug_wid, "\n\r" );
				return;
			}
			wl_clrscr( debug_wid );
			continue;
		}
		wl_printf( debug_wid, " %s\n\r", help_text[ x ] );
	}
}
