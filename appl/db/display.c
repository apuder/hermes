#ifdef __HERMES__
#include <cinclude.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ci_types.h>
#include <winlib.h>
#include "display.h"


static u_short		win_ptr = NULL;
static char			*current_src = NULL;
static int			current_line;
static int			first_line, max_line;
static int			win_dx, win_dy;
static int			percentage;
static SECTION_TYPE	visible_section;
static LINE_TYPE	*src_file = NULL;
static LINE_TYPE	*current_ptr;
static int			win_dx, win_dy, win_tab;
static int			normal_col = NORMAL_COL;
static int			status_col = STATUS_COL;
static int			break_col = BREAK_COL;
static int			current_col = CURRENT_COL;



void manage_event( EVENT_INFO_TYPE *ev )
{
	if( ev->event_type == EV_WINDOW_RESIZED ) {
		win_dx = ev->re.new_dx;
		win_dy = ev->re.new_dy;
		print_status( ev->re.window_id, current_src, current_line );
		redisplay( ev->re.window_id, current_line );
		return;
	}
/*
	if( ev->event_type == EV_SCROLL_EVENT ) {
		switch( ev->se.loc ) {
			case SCROLL_DOWN:
				if( ev->se.button & LEFT_PRESSED ) {
					wl_grap_mouse( ev->se.window_id );
					do {
						db_scroll_down( ev->se.window_id );
						delay( 100 );
						if( wl_test_for_event( ev ) ) {
							if( ev->event_type == EV_SCROLL_EVENT &&
								ev->se.button == LEFT_RELEASED ) {
									break;
							}
						}
					} while( 1 );
				}
				break;
			case SCROLL_UP:
				if( ev->se.button & LEFT_PRESSED ) {
					wl_grap_mouse( ev->se.window_id );
					do {
						db_scroll_up( ev->se.window_id );
						delay( 100 );
						if( wl_test_for_event( ev ) ) {
							if( ev->event_type == EV_SCROLL_EVENT &&
								ev->se.button == LEFT_RELEASED ) {
									break;
							}
						}
					} while( 1 );
				}
				break;
		}
	}
*/
}



void set_window_size( int dx, int dy, int tab )
{
	win_dx = dx;
	win_dy = dy;
	win_tab = tab;
	if( win_ptr ) {
		free( win_ptr );
	}
	win_ptr = malloc( dx * dy *sizeof( u_short ) );
}


/*
void db_scroll_down( WID wid )
{
	if( visible_section.last_line->next_line ) {
		visible_section.first_line_num++;
		visible_section.last_line_num++;
		visible_section.first_line = visible_section.first_line->next_line;
		visible_section.last_line = visible_section.last_line->next_line;
		percentage = (long) visible_section.first_line_num * 100 /
					 ( max_line - win_dy + 2 );
		wl_scroll_up( wid, 1, 2, win_dx, win_dy - 1);
		wl_gotoxy( wid, 1, win_dy );
		if( visible_section.last_line_num == current_line ) {
			wl_set_textattr( wid, current_col );
			wl_clreol( wid );
			print_clipped( wid, visible_section.last_line->src_line );
			wl_set_textattr( wid, normal_col );
		}
		else {
			print_clipped( wid, visible_section.last_line->src_line );
		}
		wl_set_vscroll( wid, percentage );
	}
}



void db_scroll_up( WID wid )
{
	if( visible_section.first_line_num > 1 ) {
		visible_section.first_line_num--;
		visible_section.last_line_num--;
		visible_section.first_line = visible_section.first_line->prev_line;
		visible_section.last_line = visible_section.last_line->prev_line;
		percentage = (long) visible_section.first_line_num * 100 /
					 ( max_line - win_dy + 2 );
		wl_scroll_down( wid, 1, 2, win_dx, win_dy - 1 );
		wl_gotoxy( wid, 1, 2 );
		if( visible_section.first_line_num == current_line ) {
			wl_set_textattr( wid, current_col );
			wl_clreol( wid );
			print_clipped( wid, visible_section.first_line->src_line );
			wl_set_textattr( wid, normal_col );
		}
		else {
			print_clipped( wid, visible_section.first_line->src_line );
		}
		wl_set_vscroll( wid, percentage );
	}
}
*/


void db_page_down( WID wid )
{
	int		new_line;

	new_line = visible_section.first_line_num + win_dy - 1;
	if( new_line + win_dy - 2 > max_line ) {
		new_line = max_line - win_dy + 2;
	}
	if( new_line == visible_section.first_line_num ) {
		return;
	}

	new_line += ( win_dy - 1 ) / 2;
	wl_clrscr( wid );
	print_status( wid, current_src, current_line );
	redisplay( wid, new_line );
}



void print_status( WID wid, char *name, int line )
{
	char	buf[ 160 ];
	int		i;

	sprintf( buf, " File: %s   Line: %d\n\r", name, line );
	i = strlen( buf );
	wl_set_textattr( wid, status_col );
	wl_gotoxy( wid, 1, 1 );
	print_clipped( wid, buf );
	if( i < win_dx ) {
		wl_clreol( wid );
	}
	wl_set_textattr( wid, normal_col );
}



int load_new_src( char *name )
{
	FILE		*fp;
	char		buf[ MAX_LINE_LEN ];
	LINE_TYPE	*cur_line, *prev_line;

	if( ( fp = fopen( name, "r" ) ) == NULL ) {
		return( 0 );
	}

	max_line = 0;
	if( !src_file ) {
		src_file = (LINE_TYPE*) malloc( sizeof( LINE_TYPE ) );
		if( !src_file ) {
			return( 0 );
		}

		src_file->prev_line = src_file->next_line = NULL;
		src_file->src_line = NULL;
	}


	/* Load source file and allocate new memory if necessary */

	cur_line = prev_line = src_file;
	while( fgets( buf, MAX_LINE_LEN, fp ) ) {
		if( !cur_line ) {
			cur_line = (LINE_TYPE*) malloc( sizeof( LINE_TYPE ) );
			if( !cur_line ) {
				return( 0 );
			}

			cur_line->prev_line = prev_line;
			prev_line->next_line = cur_line;
			cur_line->next_line = NULL;
		}
		max_line++;
		cur_line->src_line = strdup( buf );
		prev_line = cur_line;
		cur_line = cur_line->next_line;
	}
	fclose( fp );


	/* Free space of all old unused lines */

	while( cur_line ) {
		free( cur_line->src_line );
		cur_line = cur_line->next_line;
	}

	return( 1 );
}



void free_old_lines( void )
{
	LINE_TYPE	*tmp;

	tmp = src_file;
	while( tmp ) {
		free( tmp->src_line );
		tmp->src_line = NULL;
		tmp = tmp->next_line;
	}
}



int line_visible( int line )
{
	return( ( line >= visible_section.first_line_num ) &&
			( line <= visible_section.last_line_num ) &&
			visible_section.first_line_num );
}



void print_clipped( WID wid, char *text )
{
	char	buf[ MAX_LINE_LEN ];
	int		i, j;

	for( i = j = 0; ( j < win_dx ) && ( text[ i ] != '\n' ); i++ ) {
		if( text[ i ] != '\t' ) {
			j++;
		}
		else {
			j++;
			while( j % win_tab ) j++;
		}
		buf[ i ] = text[ i ];
	}
	buf[ i ] = 0;
	wl_printf( wid, "%s", buf );
}



void redisplay( WID wid, int line_num )
{
	LINE_TYPE	*tmp_line;
	int			y, i;

	visible_section.first_line_num = line_num;
	tmp_line = current_ptr;

if( line_num > current_line ) {
	for( i = 0; i < line_num - current_line; i++ ) {
		tmp_line = tmp_line->next_line;
	}
}
else if( line_num < current_line ) {
	for( i = 0; i < current_line - line_num; i++ ) {
		tmp_line = tmp_line->prev_line;
	}
}

	for( i = 0; i < ( win_dy - 1 ) / 2; i++ ) {
		if( tmp_line->prev_line ) {
			tmp_line = tmp_line->prev_line;
			visible_section.first_line_num--;
		}
	}
	visible_section.first_line = tmp_line;
	visible_section.last_line_num = visible_section.first_line_num + win_dy - 2;

	y = 2;
	for( i = 0; i < win_dy - 1; i++ ) {
		if( tmp_line ) {
			wl_gotoxy( wid, 1, y );
			if( tmp_line == current_ptr ) {
				wl_set_textattr( wid, current_col );
				wl_clreol( wid );
				print_clipped( wid, tmp_line->src_line );
				wl_set_textattr( wid, normal_col );
			}
			else {
				print_clipped( wid, tmp_line->src_line );
			}
			tmp_line = tmp_line->next_line;
			y++;
		}
		else {
			print_clipped( wid, "" );
		}
	}

	visible_section.last_line = tmp_line->prev_line;
	percentage = (long) visible_section.first_line_num * 100 /
				 ( max_line - win_dy + 2 );
	wl_set_vscroll( wid, percentage );
}



int display_file( WID wid, int line, char *name )
{
	int			i;
	LINE_TYPE	*tmp_line;

	if( ( line < 1 ) || ( ( line > max_line ) && src_file ) ) {
		return( 0 );
	}


	/* A new source file is required, so free the space allocated by the
	   old file
	*/

	if( strcmp( name, current_src ) ) {
		if( current_src ) {
			free_old_lines();
		}
		free( current_src );
		load_new_src( name );
		current_src = strdup( name );
		current_line = visible_section.first_line_num =
		visible_section.last_line_num = 0;
		current_ptr = NULL;
	}

	if( !line_visible( line ) ) {
		wl_clrscr( wid );
	}

	print_status( wid, name, line );

	if( !line_visible( line ) ) {
		if( !current_line ) {		/* The file was just loaded, so start at */
			current_ptr = src_file; /* the first line */
			current_line = 1;
		}


		/* Find the source line */

		if( line > current_line ) {
			for( i = 0; i < line - current_line; i++ ) {
				current_ptr = current_ptr->next_line;
			}
		}
		else {
			for( i = 0; i < current_line - line; i++ ) {
				current_ptr = current_ptr->prev_line;
			}
		}
		current_line = line;
		redisplay( wid, current_line );
	}
	else {
		wl_gotoxy( wid, 1, current_line - visible_section.first_line_num + 2 );
		wl_clreol( wid );
		print_clipped( wid, current_ptr->src_line );
		if( line > current_line ) {
			for( i = 0; i < line - current_line; i++ ) {
				current_ptr = current_ptr->next_line;
			}
		}
		else {
			for( i = 0; i < current_line - line; i++ ) {
				current_ptr = current_ptr->prev_line;
			}
		}
		current_line = line;
		wl_gotoxy( wid, 1, current_line - visible_section.first_line_num + 2 );
		wl_set_textattr( wid, current_col );
		wl_clreol( wid );
		print_clipped( wid, current_ptr->src_line );
		wl_set_textattr( wid, normal_col );
	}

	return( 0 );
}
