
#include <ci_types.h>
#include <cinclude.h>
#include <winlib.h>
#include "dbyacc.h"
#include "db.h"


#define EOF 		0

#define UPCASE( x ) ( ( (x) >= 'a' ) ? ( (x) - 32 ) : (x) )


extern char *db_buffer_ptr;


typedef struct {
	char	*cmd_name;
	int 	token_nr;
	} DB_COMMAND;


DB_COMMAND db_commands[] = {
	{ "",			TOKEN_UNDEF },
	{ "h",			COMMAND_HELP },
	{ "?",			COMMAND_HELP },
	{ "help",		COMMAND_HELP },
	{ "c",			COMMAND_CLS },
	{ "cls",		COMMAND_CLS },
	{ "d",			COMMAND_DUMP },
	{ "t",			COMMAND_TRANSLATE },
	{ "r",			COMMAND_DISP_REGS },
	{ "v",			COMMAND_VAR },
	{ "var",		COMMAND_VAR },
	{ "gdt",		COMMAND_GDT },
	{ "ps",			COMMAND_PS },
	{ "w",			COMMAND_WAKEUP },
	{ "exit",		COMMAND_EXIT },
	{ "load",		COMMAND_LOAD_EXE },
	{ "syms",		COMMAND_DISP_SYMS },
	{ "show",		COMMAND_SHOW },
	{ "s",			COMMAND_STEP },
	{ "+",			OP_PLUS },
	{ "-",			OP_MINUS },
	{ "*",			OP_TIMES },
	{ "/",			OP_DIV },
	{ "%",			OP_MOD },
	{ ">>", 		OP_RIGHT },
	{ "<<", 		OP_LEFT },
	{ "~",			OP_NOT },
	{ "(",			BRACKET_OPEN },
	{ ")",			BRACKET_CLOSE },
	{ ",",			COMMA },
	{ "=",			ASSIGN },
	{ "\"", 		LAST_VALUE },
	{ "!1",			VALUE1 },
	{ "!2",			VALUE2 },
	{ "!3",			VALUE3 },
	{ "!4",			VALUE4 },
	{ "!5",			VALUE5 },
	{ "!6",			VALUE6 },
	{ "!7",			VALUE7 },
	{ "!8",			VALUE8 },
	{ "!9",			VALUE9 }
	};


char *token_lookup( char *s, int *token )
{
	int 	x;
	int 	act_token_nr;
	int 	act_token_len;
	int 	token_len;
	char	*curr_token;
	char	*old_s;

	old_s = s;
	act_token_len = -1;   /* No token found yet */
	for( x = 0; x < sizeof( db_commands ) / sizeof( DB_COMMAND ); x++ ) {
		s = old_s;
		curr_token = db_commands[ x ].cmd_name;
		token_len = 0;

		/* Scan current token */
		while( *curr_token != '\0' && *s == *curr_token ) {
			curr_token++;
			s++;
			token_len++;
		}
		if( *curr_token == '\0' ) {

			/* Token match */
			if( act_token_len < token_len ) {

				/* New token is longer than last one -> take it */
				act_token_nr = x;
				act_token_len = token_len;
			}
		}
	}
	*token = db_commands[ act_token_nr ].token_nr;
	return( old_s + act_token_len );
}



int yylex( void )
{
	int new_token;

	while( *db_buffer_ptr == ' ' ) db_buffer_ptr++;
	if( *db_buffer_ptr == '\0' ) return( EOF );
	db_buffer_ptr = token_lookup( db_buffer_ptr, &new_token );
	if( new_token == TOKEN_UNDEF )
		switch( *db_buffer_ptr ) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				yylval.num = get_number();
				return( NUMBER );
			default:
				yylval.str = get_str();
				return( STRING );
		}
	return( new_token );
}



int get_number( void )
{
	int 	val = 0;
	int 	base = 10;
	int 	finished;
	char	c;

	if( *db_buffer_ptr == '0' ) {
		c = *( db_buffer_ptr + 1 );
		switch( UPCASE( c ) ) {
			case 'X':
				base = 16;
				db_buffer_ptr += 2;
				break;
			case 'O':
				base = 8;
				db_buffer_ptr += 2;
				break;
			case 'B':
				base = 2;
				db_buffer_ptr += 2;
				break;
			default:
				base = 8;
				break;
		}
	}
	finished = 0;
	while( !finished ) {
		c = UPCASE( *db_buffer_ptr );
		if( c >= '0' && c <= '9' )
			c -= '0';
		else if( c >= 'A' && c <= 'F' )
			c -= 'A' - 10;
		else
			finished = 1;
		if( !finished ) {
			if( c < base ) {
				val *= base;
				val += c;
				db_buffer_ptr++;
			} else
				finished = 1;
		}
	}
	return( val );
}


#define MAX_STR_LEN	50

char *get_str( void )
{
	static char	buf[ MAX_STR_LEN + 1 ];
	int			i;

	i = 0;
	while( TRUE ) {
		if( i == MAX_STR_LEN || *db_buffer_ptr == '\0' ) {
			buf[ i ] = '\0';
			return( buf );
		}
		if( *db_buffer_ptr == ' ' ) {
			buf[ i ] = '\0';
			db_buffer_ptr++;
			return( buf );
		}
		buf[ i++ ] = *db_buffer_ptr++;
	}
}
