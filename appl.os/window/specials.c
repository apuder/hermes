#include <ci_types.h>

#ifdef __TURBOC__

#include <dos.h>
#include <conio.h>
#include "ticker.h"

#endif

#ifdef __GNUC__

#include <cinclude.h>

#endif

#include <winlib.h>
#include "win_serv.h"
#include "ws_intrn.h"
#include "specials.h"


#define COLOR_6845		0x3d0
#define MONO_6845		0x3b0
#define INDEX			4		/* 6845 index register */
#define DATA			5		/* 6845 data register */
#define CURSOR_SIZE		10		/* 6845 cursor size register */
#define CURSOR			14		/* 6845 cursor register */
#define BYTE			0xff


u_int screen_port;

void bufcpy( void *dest, void *source, u_int size )
{
	u_int	i;
	u_char	*s, *d;

	s = ( u_char* ) source;
	d = ( u_char* ) dest;
	for( i = 0; i < size; *( d++ ) = *( s++ ), i++ );
}



/*
 * This little function dumps out an integer value
 * to the 6845 video controller.
 */

void set_6845( unsigned char reg, int val )
{
	outportb( screen_port + INDEX, reg );
	outportb( screen_port + DATA, (val >> 8) & BYTE );
	outportb( screen_port + INDEX, reg + 1 );
	outportb( screen_port + DATA, val & BYTE);
}



void hw_set_cursor_size( u_short size )
{
	set_6845( CURSOR_SIZE, size );
}



u_short hw_get_cursor_size( void )
{
	return( 0x0b0c );
}



#ifdef __TURBOC__


u_int get_ticker_diff( u_int last_tick )
{
	return( get_ticks() - last_tick );
}



u_char hw_kbhit( void )
{
	struct REGPACK	regs;

	regs.r_ax = 0x0100;
	intr( 0x16, &regs );
	if( regs.r_flags & 64 ) {
		return( 0 );
	}
	else {
		return( 1 );
	}
}



u_short hw_getch( void )
{
	struct REGPACK	regs;

	regs.r_ax = 0;
	intr( 0x16, &regs );
	return( regs.r_ax );
}



s_short hw_gettext( s_short left, s_short top, s_short right,
					s_short bottom, void *destin )
{
	return( gettext( left, top, right, bottom, destin ) );
}



s_short hw_puttext( s_short left, s_short top, s_short right,
					s_short bottom, void *source )
{
	return( puttext( left, top, right, bottom, source ) );
}



void hw_gotoxy( s_short x, s_short y )
{
	gotoxy( x, y );
}



void hw_get_screen_info( u_char *x, u_char *y, u_char *dx, u_char *dy,
						 WL_TEXT_MODE *mode )
{
	struct text_info	ti;

	gettextinfo( &ti );
	*dx = ti.screenwidth;
	*dy = ti.screenheight;
	*mode = ti.currmode;
	*x = ti.curx;
	*y = ti.cury;
}



WL_VIDEO_TYPE hw_get_video_type( void )
{
	struct REGPACK	regs;

	regs.r_ax = 0x1a00;
	intr( 0x10, &regs );
	if( ( regs.r_ax & 0x00ff ) != 0x1a ) {
		return( WL_NOT_SUPPORTED );
	}
	else {
		return( ( WL_VIDEO_TYPE ) ( regs.r_bx & 0x00ff ) );
	}
}



void hw_text_mode( WL_TEXT_MODE mode )
{
	textmode( mode );
}



u_short *get_video_adr( WL_TEXT_MODE mode )
{
	if( mode == WL_MONO ) {
		screen_port = MONO_6845;
		return( ( u_short* ) MK_FP( 0xb000, 0 ) );
	}
	else {
		screen_port = COLOR_6845;
		return( ( u_short* ) MK_FP( 0xb800, 0 ) );
	}
}


u_char *get_font_addr( void )
{
	return( (u_char *) MK_FP( 0xa000, 0 ) );
}


u_int get_ticks( void )
{
	return( ticker_count );
}


#endif



#ifdef __GNUC__



u_char *get_font_addr( void )
{
	u_char *font_addr = NULL;

	if( font_addr == NULL ) {
		kernel_trap( K_MAP_FONT_AREA );
		font_addr = (u_char *) k_r1;
	}
	return( font_addr );
}


/*
 * Moves the hardware cursor to the position (x, y).
 */

void hw_gotoxy( s_short x, s_short y )
{
	set_6845( CURSOR, ( y - 1 ) * 80 + x - 1 );
}




s_short hw_gettext( s_short left, s_short top, s_short right,
					s_short bottom, void *destin )
{
	u_int	x, y;
	u_short	*content = destin;

	for( y = top; y <= bottom; y++ )
		for( x = left; x <= right; x++ )
			*content++ = GET_SCREEN_VAL( x, y );
}



s_short hw_puttext( s_short left, s_short top, s_short right,
					s_short bottom, void *source )
{
	u_int	x, y;
	u_short	*content = source;

	for( y = top; y <= bottom; y++ )
		for( x = left; x <= right; x++ )
			WRITE_TO_SCREEN( x, y, *content++ );
}




void hw_get_screen_info( u_char *x, u_char *y, u_char *dx, u_char *dy,
						 WL_TEXT_MODE *mode )
{
	*dx = 80;
	*dy = 25;
	kernel_trap( K_GET_SCREEN_MODE );
	*mode = (WL_TEXT_MODE) k_r1;
	*x = 1;
	*y = 1;
}



WL_VIDEO_TYPE hw_get_video_type( void )
{
	kernel_trap( K_GET_VIDEO_TYPE );
	return( (WL_VIDEO_TYPE) k_r1 );
}


void hw_text_mode( WL_TEXT_MODE mode )
{
/*
	textmode( mode );
*/
	shutdown( "hw_text_mode(): Nasty!!!!" );
}


/*
u_int screen_base;
*/

u_short *get_video_adr( WL_TEXT_MODE mode )
{
/*
	screen_base = get_global_param( GLOBAL_PARAM_SCREEN_BASE );
*/
	if( mode == WL_MONO )
		screen_port = MONO_6845;
	else
		screen_port = COLOR_6845;

	kernel_trap( K_MAP_SCREEN );
	return( (u_short *) k_r1 );
}



u_int get_ticks( void )
{
	return( 0 );
}



u_int get_ticker_diff( u_int last_tick )
{
	return( 0 );
}

#endif
