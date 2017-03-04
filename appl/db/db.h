

#ifndef __DB_H__
#define __DB_H__

#define TRUE	1
#define FALSE	0


#define MAX_DB_BUFFER   	70
#define HERMES_DEBUG( s )	printf( (s) ); asm( "int $3" )

#define FLAG_CF         	( 1 )
#define FLAG_PF         	( 1 << 2 )
#define FLAG_AF         	( 1 << 4 )
#define FLAG_ZF         	( 1 << 6 )
#define FLAG_SF         	( 1 << 7 )
#define FLAG_TF         	( 1 << 8 )
#define FLAG_IF         	( 1 << 9 )
#define FLAG_DF         	( 1 << 10 )
#define FLAG_OF         	( 1 << 11 )
#define FLAG_NT         	( 1 << 14 )
#define FLAG_RF         	( 1 << 16 )
#define FLAG_VM         	( 1 << 17 )



/*=====>>> dblex.c <<<======================================================*/

int get_number( void );
char *get_str( void );



/*=====>>> dbsource.c <<<===================================================*/

int display_source( unsigned addr );


/*=====>>> step.c <<<=======================================================*/

void single_step( void );


/*=====>>> dbcmd.c <<<======================================================*/

extern unsigned eflags, cs, eip, eax, ecx, edx, ebx, esp, ebp, esi, edi;

extern int db_value[];

void db_display_regs( void );
void db_wakeup( PID pid );
void db_show_gdt( void );
void db_show_pcb( void );
void db_show_var( void );
void db_translate( unsigned pdt, unsigned v_addr );
void db_hex_dump( unsigned addr, int len );
void db_floppy_load( int dev, int pos, int addr );
void db_help_debug( void );


/*=====>>> dbmain.c <<<=====================================================*/

extern WID debug_wid;

void init_db( void );


#endif
