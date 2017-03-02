#ifndef __SPECIAL__
#define __SPECIAL__

#ifdef __TURBOC__

#define move_to_font	movmem
#define move_from_font	movmem

#endif

#ifdef __GNUC__

#define inportb( p )				InportB( p )
#define outportb( p, b )			OutportB( p, b )

#define move_to_font( s, d, n )		memcpy( d, s, n )
#define move_from_font( s, d, n )	memcpy( d, s, n )

#endif

#ifdef __TURBOC__

u_int get_ticker_diff( u_int last_tick );

#endif


void bufcpy( void *dest, void *source, u_int size );
u_char hw_kbhit( void );
u_short hw_getch( void );
u_short hw_get_cursor_size( void );
void hw_set_cursor_size( u_short size );
s_short hw_gettext( s_short left, s_short top, s_short right,
					s_short bottom, void *destin );
s_short hw_puttext( s_short left, s_short top, s_short right,
					s_short bottom, void *source );
void hw_gotoxy( s_short x, s_short y );
WL_VIDEO_TYPE hw_get_video_type( void );
void hw_get_screen_info( u_char *x, u_char *y, u_char *dx, u_char *dy,
						 WL_TEXT_MODE *mode );
void hw_text_mode( WL_TEXT_MODE mode );
u_short *get_video_adr( WL_TEXT_MODE mode );
u_char *get_font_addr( void );
u_int get_ticks( void );


#endif
