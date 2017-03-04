#ifndef __MOUSE__

#define __MOUSE__

#define MAX_MOUSE_BUFFER	25

#define OK				0

#define NEXT_BUF_POS	( next_write_idx + 1 > MAX_MOUSE_BUFFER - 1 ) ? \
						0 : next_write_idx + 1


ERR_CODE ms_init( void );
void ms_exit( void );
void ms_get_mouse_info( MOUSE_INFO_TYPE	**mi );

#endif
