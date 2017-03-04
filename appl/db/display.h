#ifndef __DISPLAY__

#define __DISPLAY__

#define MAX_LINE_LEN	255
#define NORMAL_COL		16 * WL_BLUE + WL_YELLOW
#define STATUS_COL		16 * WL_MAGENTA + WL_WHITE
#define BREAK_COL		16 * WL_RED + WL_WHITE
#define CURRENT_COL		16 * WL_CYAN + WL_BLACK


typedef struct line_type {
			char				*src_line;
			struct line_type	*prev_line;
			struct line_type	*next_line;
} LINE_TYPE;


typedef struct section_type {
			int					first_line_num;
			int					last_line_num;
			struct line_type	*first_line;
			struct line_type	*last_line;
} SECTION_TYPE;


void manage_event( EVENT_INFO_TYPE *ev );
void set_window_size( int dx, int dy, int tab );
void db_scroll_up( WID wid );
void db_scroll_down( WID wid );
void print_status( WID wid, char *name, int line );
int load_new_src( char *name );
void free_old_lines( void );
int line_visible( int line );
void print_clipped( WID wid, char *text );
void redisplay( WID wid, int line_num );
int display_file( WID wid, int line, char *name );

#endif
