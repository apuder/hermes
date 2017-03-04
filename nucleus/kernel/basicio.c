/*
 * basicio.c
 *---------------------------------------------------------
 * Basic I/O package for direct screen access.	  AP  11/90
 */

#include <kernel.h>
#include "io.h"


#define MAX_SCREEN_X			80
#define MAX_SCREEN_Y			25
#define TAB_SIZE				4


#define POKE_SCREEN( x, ch )	poke_mem_w( screen_base + ( (x) << 1 ), (ch) )
#define PEEK_SCREEN( x )		peek_mem_w( screen_base + ( (x) << 1 ) )

PRIVATE int screen_base;
PRIVATE int screen_pos			= 0;



void set_cursor( void )
{
	POKE_SCREEN( screen_pos, 0x0fb1 );
}



void scroll_screen( void )
{
	int i;
	int pos1, pos2;
	volatile int i_flag;

	DISABLE_INTR( i_flag );
	pos1 = 0;
	pos2 = MAX_SCREEN_X;
	for( i = 0; i < MAX_SCREEN_X * ( MAX_SCREEN_Y - 1 ); i++ )
		POKE_SCREEN( pos1++, PEEK_SCREEN( pos2++ ) );
	screen_pos = pos1;
	for( i = 0; i < MAX_SCREEN_X; i++ )
		POKE_SCREEN( pos1++, 0 );
	set_cursor();
	ENABLE_INTR( i_flag );
}



void cls( void )
{
	int i;
	volatile int i_flag;

	DISABLE_INTR( i_flag );
	for( i = 0; i < MAX_SCREEN_X * MAX_SCREEN_Y; i++ )
		POKE_SCREEN( i, 0 );
	screen_pos = 0;
	set_cursor();
	ENABLE_INTR( i_flag );
}



void output_char( unsigned char c )
{
	int x_pos;
	volatile int i_flag;

	DISABLE_INTR( i_flag );
	switch( c ) {
		case '\n':
		case 13:
			POKE_SCREEN( screen_pos, 0 );
			screen_pos += MAX_SCREEN_X - screen_pos % MAX_SCREEN_X;
			break;
		case 9:
			x_pos = screen_pos % MAX_SCREEN_X;
			if( x_pos % TAB_SIZE == 0 ) x_pos = TAB_SIZE;
			while( x_pos-- ) POKE_SCREEN( screen_pos++, 0 );
			break;
		case '\b':
			POKE_SCREEN( screen_pos, 0 );
			if( screen_pos != 0 ) screen_pos--;
			break;
		default:
			POKE_SCREEN( screen_pos, (short unsigned int) c | 0x0f00 );
			screen_pos++;
			break;
	}
	if( screen_pos >= MAX_SCREEN_X * MAX_SCREEN_Y ) scroll_screen();
	set_cursor();
	ENABLE_INTR( i_flag );
}



void output_string( const char *str )
{
	while( *str != '\0' ) output_char( *str++ );
}



void init_basicio( void )
{
	screen_base = get_global_param( GLOBAL_PARAM_SCREEN_BASE );
}
