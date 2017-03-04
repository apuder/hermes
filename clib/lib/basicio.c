/*
 * basicio.c
 *---------------------------------------------------------
 * Basic I/O package for direct screen access.	  AP  11/90
 */

#include <cinclude.h>
#include "io.h"


#define MAX_SCREEN_X			80
#define MAX_SCREEN_Y			25
#define TAB_SIZE				4


#define POKE_SCREEN( x, ch )	*( screen_base + (x) ) = ch
#define PEEK_SCREEN( x )		*( screen_base + (x) )

static unsigned short *screen_base;
static int screen_pos = 0;



void set_cursor( void )
{
	POKE_SCREEN( screen_pos, 0x0fb1 );
}



void scroll_screen( void )
{
	int i;
	int pos1, pos2;

	pos1 = 0;
	pos2 = MAX_SCREEN_X;
	for( i = 0; i < MAX_SCREEN_X * ( MAX_SCREEN_Y - 1 ); i++ )
		POKE_SCREEN( pos1++, PEEK_SCREEN( pos2++ ) );
	screen_pos = pos1;
	for( i = 0; i < MAX_SCREEN_X; i++ )
		POKE_SCREEN( pos1++, 0 );
	set_cursor();
}



void cls( void )
{
	int i;

	for( i = 0; i < MAX_SCREEN_X * MAX_SCREEN_Y; i++ )
		POKE_SCREEN( i, 0 );
	screen_pos = 0;
	set_cursor();
}



void output_char( unsigned char c )
{
	int x_pos;

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
}



void output_string( const char *str )
{
	while( *str != '\0' ) output_char( *str++ );
}



void init_basicio( void )
{
	kernel_trap( K_MAP_SCREEN );
	screen_base = (unsigned short *) k_r1;
}
