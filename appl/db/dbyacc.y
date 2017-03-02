%{

#include <ci_types.h>
#include <cinclude.h>
#include <winlib.h>
#include <sysmsg.h>
#include "db.h"


#define YYDEBUG 0

unsigned last_value = 0;


yyerror( char *s )
{
	wl_printf( debug_wid, "%s\n\r", s );
}


%}


%union {
	int 	num;		/* Found a numeric constant */
	char	*str;		/* Found an identifier */
}


%token			TOKEN_UNDEF
%token			COMMAND_HELP
%token			COMMAND_CLS
%token			COMMAND_DISP_REGS
%token			COMMAND_DUMP
%token			COMMAND_TRANSLATE
%token			COMMAND_GDT
%token			COMMAND_PS
%token			COMMAND_WAKEUP
%token			COMMAND_VAR
%token			COMMAND_EXIT
%token			COMMAND_LOAD_EXE
%token			COMMAND_DISP_SYMS
%token			COMMAND_SHOW
%token			COMMAND_STEP

%token <num>	NUMBER
%token <str>	STRING
%token			LAST_VALUE
%token			VALUE1
%token			VALUE2
%token			VALUE3
%token			VALUE4
%token			VALUE5
%token			VALUE6
%token			VALUE7
%token			VALUE8
%token			VALUE9
%token			COLON
%token			DOLLAR
%token			COMMA
%token			ASSIGN
%token			OP_PLUS
%token			OP_MINUS
%token			OP_TIMES
%token			OP_DIV
%token			OP_AND
%token			OP_OR
%token			OP_RIGHT
%token			OP_LEFT
%token			OP_MOD
%token			OP_NOT
%token			BRACKET_OPEN
%token			BRACKET_CLOSE
%token			SQR_BRACKET_OPEN
%token			SQR_BRACKET_CLOSE

%type <num> 	expr
%type <num> 	expr1
%type <num> 	term
%type <num> 	factor
%type <num>		db_value


%%

prg
	:
	| debug_command
	;

debug_command
	: disp_regs
	| dump_memory
	| expr
		{
			wl_printf( debug_wid, "Hex = %32x\n\rDec = %32u\n\r",
									$1, $1, $1 );
		}
	| COMMAND_GDT
		{
			db_show_gdt();
		}
	| COMMAND_PS
		{
			db_show_pcb();
		}
	| COMMAND_WAKEUP expr
		{
			db_wakeup( (PID) $2 );
		}
	| COMMAND_CLS
		{
			wl_clrscr( debug_wid );
		}
	| COMMAND_TRANSLATE expr COMMA expr
		{
			db_translate( $2, $4 );
		}
	| COMMAND_HELP
		{
			db_help_debug();
		}
	| COMMAND_VAR
		{
			db_show_var();
		}
	| db_value ASSIGN expr
		{
			db_value[ $1 ] = $3;
		}
	| COMMAND_EXIT
		{
			exit( 0 );
		}
	| COMMAND_LOAD_EXE STRING
		{
			syms_init( $2 );
		}
	| COMMAND_DISP_SYMS
		{
			syms_list( 0 );
		}
	| COMMAND_STEP
		{
			single_step();
		}
	| COMMAND_SHOW expr
		{
			display_source( $2 );
		}
	;

disp_regs
	: COMMAND_DISP_REGS
		{
			db_display_regs();
		}
	;

dump_memory
	: COMMAND_DUMP
		{
			db_hex_dump( 0xffffffff, 0x100 );
		}
	| COMMAND_DUMP expr
		{
			db_hex_dump( $2, 0x100 );
		}
	| COMMAND_DUMP expr COMMA expr
		{
			db_hex_dump( $2, $4 );
		}
	;


expr
	: expr1 						{ $$ = $1; last_value = $1; }
	;

expr1
	: term							{ $$ = $1; }
	| OP_NOT term					{ $$ = ~$2; }
	| expr1 OP_PLUS term			{ $$ = $1 + $3; }
	| expr1 OP_MINUS term			{ $$ = $1 - $3; }
	| expr1 OP_AND term 			{ $$ = $1 & $3; }
	| expr1 OP_OR term				{ $$ = $1 | $3; }
	| expr1 OP_RIGHT term			{ $$ = $1 >> $3; }
	| expr1 OP_LEFT term			{ $$ = $1 << $3; }
	;

term
	: factor						{ $$ = $1; }
	| term OP_TIMES factor			{ $$ = $1 * $3; }
	| term OP_DIV factor			{ $$ = $1 / $3; }
	| term OP_MOD factor			{ $$ = $1 % $3; }
	;

factor
	: NUMBER						{ $$ = $1; }
	| OP_MINUS NUMBER				{ $$ = -$2; }
	| LAST_VALUE					{ $$ = last_value; }
	| db_value						{ $$ = db_value[ $1 ]; }
	| BRACKET_OPEN expr1 BRACKET_CLOSE
									{ $$ = $2; }
	;

db_value
	: VALUE1						{ $$ = 0; }
	| VALUE2						{ $$ = 1; }
	| VALUE3						{ $$ = 2; }
	| VALUE4						{ $$ = 3; }
	| VALUE5						{ $$ = 4; }
	| VALUE6						{ $$ = 5; }
	| VALUE7						{ $$ = 6; }
	| VALUE8						{ $$ = 7; }
	| VALUE9						{ $$ = 8; }
	;
