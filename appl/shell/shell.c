#ifdef __HERMES__
#include <cinclude.h>
#endif

#include <ci_types.h>
#include <stdio.h>
#include <winlib.h>
#include <string.h>
#include <alloc.h>
#include <ctype.h>
#include <fs.h>
#include <assert.h>
#include <time.h>
#include "shell.h"


//extern int				errno;
extern char				**environ;
extern FILE_HANDLE_TYPE	*handle_buf;

WID					shell_wid;
int					cmd_num = 0;
int					first_cmd = 0;
int					cur_cmd;
int					max_history = 40;
int					last_cmd;
char				**history_list;
char				cmd_line[ MAX_CMD_LINE ];
char				*arg_ptr[ MAX_PARAM ];
int					arg_cnt;



int test_whitespace( char *buf )
{
    while( *buf ) {
	if( *buf != '\t' && *buf != ' ' ) {
	    return( FALSE );
	}
    }
    return( TRUE );
}



int last_history_event( char *buf )
{
    int		i, j, k;

    if( cmd_num < 1 ) {
	return( SH_NO_EVENT_FOUND );
    }

    if( buf[ 1 ] =='!' && buf[ 2 ] == '\0' ) {
	strcpy( buf, history_list[ last_cmd ] );
	return( SH_OK );
    }

    j = 1;
    for( i = 0; i < strlen( &buf[ 1 ] ); i++ ) {
	if( !isdigit( buf[ i + 1 ] ) ) {
	    j = 0;
	    break;
	}
    }

    if( j ) {
	i = atoi( &buf[ 1 ] );
	if( i > cmd_num || i < cmd_num - max_history ) {
	    return( SH_NO_EVENT_FOUND );
	}
	strcpy( buf, history_list[ i % max_history ] );
	return( SH_OK );
    }
    else {
	k = strlen( &buf[ 1 ] );
	j = ( cmd_num - max_history > 0 ) ? cmd_num - max_history : 0;
	for( i = cmd_num - 1; i >= j; i-- ) {
	    if( !strncmp( &buf[ 1 ], history_list[ i % max_history ], k ) ) {
		strcpy( buf, history_list[ i % max_history ] );
		return( SH_OK );
	    }
	}
    }

    return( SH_NO_EVENT_FOUND );
}



void exit_shell( char **args )
{
    _exit( 0 );
}



void set_env( char **args )
{
    char	*help_arg[] = { "setenv", NULL };

    if( arg_cnt < 3 ) {
	show_help( help_arg );
	return;
    }

#ifdef __HERMES__
    setenv( args[ 0 ], args[ 1 ] );
#endif

#ifdef __TURBOC__
    putenv( strcat( args[ 0 ], strcat( "=", args[ 1 ] ) ) );
#endif

}



void unset_env( char **args )
{
    char	*help_arg[] = { "unsetenv", NULL };

    if( arg_cnt < 2 ) {
	show_help( help_arg );
	return;
    }

#ifdef __HERMES__
    unsetenv( args[ 0 ] );
#endif

}



void disp_env( char **args )
{
    int		i;

    i = 0;
    while( environ != NULL && environ[ i ] != 0 ) {
	wl_printf( shell_wid, "%s\n\r", environ[ i++ ] );
    }
}



void cat( char **args )
{
    FILE	*fp;
    char	buf[ 1024 ];
    int		i;
    char	*help_arg[] = { "cat", NULL };

    if( arg_cnt < 2 ) {
	show_help( help_arg );
	return;
    }
    i = 0;
    while( args[ i ] ) {
	fp = fopen( args[ i ], "r" );
	if( !fp ) {
	    wl_printf( shell_wid, "CAT: File not found %s, rc = %d \n\r",
		       args[ i ], errno );
	    i++;
	    continue;
	}

	while( fgets( buf, 1024, fp ) ) {
	    wl_printf( shell_wid, "%s", buf );
	}
	fclose( fp );
	i++;
    }
}



void dir_list( char **args )
{
#if 0
    FILE			*fp;
    int				nbytes, i;
    int				byteCounter = 0;
    int 			blockByteCounter = 0;
    int				blockCounter = 0;
    char			buf[ 1024 ];
    char			buf1[ 50 ];
    DirectoryEntry	*dirEntryPtr;

    fp = fopen( ".", "r" );
    if( !fp ) {
	wl_printf( shell_wid, "Error opening directory: rc %d\n\r", errno );
	return;
    }

    nbytes = read( FHANDLE( *fp ), buf, 1024 );
    dirEntryPtr = (DirectoryEntry*) buf;
    while ( byteCounter < nbytes )
	{
	    if( dirEntryPtr->Filename[ 0 ] ) {
		i = strlen( dirEntryPtr->Filename );
		memset( buf1, 0, 50 );
		memset( buf1, 32, 30 - i );
		wl_printf( shell_wid, "%s%s%d\n\r", dirEntryPtr->Filename, buf1,
			   dirEntryPtr->IndexNodeNumber );
	    }
	    byteCounter += dirEntryPtr->SizeOfEntry;
	    blockByteCounter += dirEntryPtr->SizeOfEntry;
	    dirEntryPtr =
		(DirectoryEntry*) ((s_char*) dirEntryPtr + dirEntryPtr->SizeOfEntry);
	    if (blockByteCounter == BYTES_PER_BLOCK)
		{
		    blockByteCounter = 0;
		    blockCounter ++;
		}
	    else if (blockByteCounter > BYTES_PER_BLOCK)
		/* !!! Daten inkonsistent !!! */
		/* !!! Noch mehr Inkonsistenzitäts-Tests !!! */
		assert (0);
	}
    fclose( fp );
#endif
}



void copy_file( char **args)
{
    FILE	*source_fp, *dest_fp;
    char	buf[ 1024 ];
    int		nbytes;
    char	*help_arg[] = { "cp", NULL };

    if( arg_cnt < 3 ) {
	show_help( help_arg );
	return;
    }

    source_fp = fopen( args[ 0 ], "r" );
    if( !source_fp ) {
	wl_printf( shell_wid, "Source file not found %s, rc = %d\n\r",
		   args[ 0 ], errno );
	return;
    }

    dest_fp = fopen( args[ 1 ], "w" );
    if( !dest_fp ) {
	wl_printf( shell_wid, "Unable to create destination file %s, rc = %d\n\r",
		   args[ 1 ], errno );
	fclose( source_fp );
	return;
    }

    while( nbytes = read( FHANDLE( *source_fp ), buf, 1024 ) ) {
	if( nbytes == -1 ) {
	    wl_printf( shell_wid, "Error while reading source file, rc = %d\n\r",
		       errno );
	    break;
	}
	if( write( FHANDLE( *dest_fp ), buf, nbytes ) == -1 ) {
	    wl_printf( shell_wid, "Error while writing destination file, rc = %d\n\r",
		       errno );
	    break;
	}
    }

    fclose( source_fp );
    fclose( dest_fp );
}



void show_history( char **args )
{
    int		i, start;

    start = ( cmd_num - max_history > 0 ) ? cmd_num - max_history : 0;
    for( i = start; i < cmd_num; i++ ) {
	wl_printf( shell_wid, "%d:\t%s\n\r", i,
		   history_list[ i % max_history ] );
    }
}



void clear_disp( char **args )
{
    wl_clrscr( shell_wid );
}



void insert_history( char *s, int num )
{
    if( history_list[ num ] != NULL ) {
	free( history_list[ num ] );
    }
    history_list[ num ] = malloc( strlen( s ) + 1 );
    strcpy( history_list[ num ], s );
}



void clear_cmdline( char *buf, int *len )
{
    *( buf + *len ) = '\0';
    memset( (void*) buf, '\b', *len );
    wl_printf( shell_wid, "%s", buf );
    memset( (void*) buf, ' ', *len );
    wl_printf( shell_wid, "%s", buf );
    memset( (void*) buf, '\b', *len );
    wl_printf( shell_wid, "%s", buf );
    *len = 0;
}



void input( char *buf, int max_input_len )
{
    EVENT_INFO_TYPE	event;
    int				x = 0;
    unsigned		c;

    cur_cmd = ( last_cmd + 1 ) % max_history;
    wl_printf( shell_wid, "HSH[%d]>", cmd_num );
    wl_cursor_on( shell_wid );
    while( TRUE ) {
	do {
	    wl_wait_for_event( &event );
	    if( event.event_type == EV_WINDOW_RESIZED ) {
		*( buf + x ) = '\0';
		wl_printf( shell_wid, "HSH[%d]>%s", cmd_num, buf );
	    }
	} while( event.event_type != EV_KEY_PRESSED );
	c = event.ke.key;

	switch( c ) {
	case '\t':	/* Tab */
	    continue;

	case 13:	/* Carriage Return */
	    *( buf + x ) = '\0';
	    wl_cursor_off( shell_wid );
	    if( buf[ 0 ] != '!' && !test_whitespace( buf ) ) {
		last_cmd = NEXT_HIST( last_cmd );
		if( NEXT_HIST( last_cmd ) == first_cmd ) {
		    first_cmd = NEXT_HIST( first_cmd );
		}
		insert_history( buf, last_cmd );
	    }
	    return;

	case 0x4800:	/* Cursor up */
	    if( ELDEST_CMD( cur_cmd ) ) {
		wl_printf( shell_wid, "\a" );
		continue;
	    }

	    *( buf + x ) = '\0';

	    if( cur_cmd == NEXT_HIST( last_cmd ) ) {
		insert_history( buf, cur_cmd );
	    }
	    clear_cmdline( buf, &x );

	    cur_cmd = PREV_HIST( cur_cmd );
	    strcpy( buf, history_list[ cur_cmd ] );
	    wl_printf( shell_wid, "%s", history_list[ cur_cmd ] );
	    x = strlen( history_list[ cur_cmd ] );
	    continue;

	case 0x5000:					/* Cursor down */
	    if( cur_cmd == NEXT_HIST( last_cmd ) ) {
		wl_printf( shell_wid, "\a" );
		continue;
	    }

	    clear_cmdline( buf, &x );
	    cur_cmd = NEXT_HIST( cur_cmd );
	    strcpy( buf, history_list[ cur_cmd ] );
	    wl_printf( shell_wid, "%s", history_list[ cur_cmd ] );
	    x = strlen( history_list[ cur_cmd ] );
	    continue;

	case 27:						/* Escape */
	    cur_cmd = NEXT_HIST( last_cmd );
	    clear_cmdline( buf, &x );
	    continue;

	case '\b':						/* Backspace */
	    if( x != 0 ) {
		wl_printf( shell_wid, "\b \b" );
		x--;
	    }
	    continue;

	default:
	    if( x == max_input_len ) continue;
	    wl_printf( shell_wid, "%c", (char) c );
	    *( buf + x++ ) = (char) c;
	}
    }
}



void skip_whitespace( char **cmd_line, int *eoln )
{
    while( **cmd_line == ' ' || **cmd_line == '\t' ) {
	( *cmd_line )++;
    }
    *eoln = ( **cmd_line == 0 ) ? TRUE : FALSE;
}



void close_arg( char **cmd_line, int *eoln )
{
    while( **cmd_line != ' ' && **cmd_line != '\t' && **cmd_line != 0 ) {
	( *cmd_line )++;
    }
    *eoln = ( **cmd_line == 0 ) ? TRUE : FALSE;
    **cmd_line = 0;
    ( *cmd_line )++;
}



void parse_input( char **args, char *cmd_line )
{
    int		eoln = FALSE;
    int		arg_num = 0;
    int		i;

    for( i = 0; i < MAX_PARAM; i++ ) {
	args[ i ] = NULL;
    }

    do {
	skip_whitespace( &cmd_line, &eoln );
	if( !eoln ) {
	    args[ arg_num++ ] = cmd_line;
	    close_arg( &cmd_line, &eoln );
	}
    } while( !eoln );
    args[ arg_num ] = NULL;
    arg_cnt = arg_num;
}



void get_time( char **args )
{
    time_t	t;

    tzset();
    t = time( NULL );
    wl_printf( shell_wid, "%s\r", ctime( &t ) );
}



void disp_help( char **args )
{
    char	*help_arg[] = { NULL };

    show_help( help_arg );
}



DISPATCH_TABLE_TYPE	dispatch_table[] = {
    "ls", dir_list,          "                    | Displays the directory",
    "cp", copy_file,         "<source> <dest>     | Copies a file",
    "cat", cat,              "<file> ... <file>  | Displays specified files",
    "env", disp_env,         "                   | Displays the environment",
    "setenv", set_env,       "<name> <value>  | Sets/changes the environment",
    "unsetenv", unset_env,   "<name>        | Undefines an environment entry",
    "history", show_history, "               | Diplays the history list",
    "clear", clear_disp,     "                 | Clears the shell screen",
    "help", disp_help,       "                  | Displays this text",
    "exit", exit_shell,      "                  | Leaves the shell",
    "time", get_time,		 "                | Gets seconds since 1970"
};

#define NUM_OF_DTABLE_ENTRIES ( sizeof( dispatch_table ) / sizeof( DISPATCH_TABLE_TYPE ) )



int find_cmd( char *cmd )
{
    int		i;

    for( i = 0; i < NUM_OF_DTABLE_ENTRIES; i++ ) {
	if( strcmp( cmd, dispatch_table[ i ].cmd ) == 0 ) {
	    return( i );
	}
    }
    return( -1 );
}



int show_help( char **args )
{
    int		i;

    if( !args[ 0 ] ) {
	wl_printf( shell_wid, "Hermes Shell help:\n\r" );
	for( i = 0; i < NUM_OF_DTABLE_ENTRIES; i++ ) {
	    wl_printf( shell_wid, "%s %s\n\r", dispatch_table[ i ].cmd,
		       dispatch_table[ i ].help_txt );
	}
    }
    else {
	i = find_cmd( args[ 0 ] );
	wl_printf( shell_wid, "%s %s\n\r", dispatch_table[ i ].cmd,
		   dispatch_table[ i ].help_txt );
    }
    return( 0 );
}



void main( void )
{
    WIN_PARAM_TYPE	param;
    short		x, y, dx, dy;
    int			rc, i, ok;
    int			handles[] = { 0, 1, 2 };

    wl_init( 10 );
    history_list = malloc( max_history * sizeof( char* ) );
    for( i = 0; i < max_history; i++ ) {
	history_list[ i ] = NULL;
    }

    last_cmd = max_history - 1;
    param.min_dx = 40;
    param.min_dy = MAX_CMD_LINE / 40 + 1;
    param.x = DEFAULT_POS; param.y = DEFAULT_POS;
    param.dx = DEFAULT_SIZE; param.dy = DEFAULT_SIZE;
    param.window_options = USER_BORDER | ALLOW_ICON | ALLOW_ZOOM |
	MID_HEADER | PRINT_SHADOW |
	ALLOW_RAISE | ALLOW_MOVE | ALLOW_RESIZE;
    param.tab_step = 8;
    param.border_col = param.screen_col = WL_WHITE + 16 * WL_BLUE;
    param.title_col = WL_LIGHTGREEN + 16 * WL_BLUE;
    param.scroll_col = WL_BLUE + 16 * WL_CYAN;
    param.icon_col = WL_WHITE + 16 * WL_LIGHTGRAY;
    param.button_col = WL_LIGHTGREEN + 16 * WL_BLUE;
    strncpy( (char*) param.title, "Hermes-Shell", MAX_HEADER_LEN );
    strcpy( (char*) param.user_border, "Ø¹Õ¾Íþ" );
    param.father_window = ROOT_WINDOW;
    param.iconic = 0;
    param.full_screen = 0;
    wl_open_win( &param, &shell_wid, &x, &y, &dx, &dy );
    wl_printf( shell_wid, "Welcome to Hermes Shell V0.5\n\r" );

    if( handle_buf[ 0 ].handle_type != DEFAULT_FILE ) {
	close( 0 );
    }
    if( handle_buf[ 1 ].handle_type != DEFAULT_FILE ) {
	close( 1 );
    }
    if( handle_buf[ 2 ].handle_type != DEFAULT_FILE ) {
	close( 2 );
    }

    handle_buf[ 0 ].handle_num = handle_buf[ 1 ].handle_num =
	handle_buf[ 2 ].handle_num = (int) shell_wid;
    handle_buf[ 0 ].handle_type = handle_buf[ 1 ].handle_type =
	handle_buf[ 2 ].handle_type = WINDOW_FILE;

    while( TRUE ) {
	input( cmd_line, MAX_CMD_LINE );
	wl_printf( shell_wid, "\n\r" );
	parse_input( arg_ptr, cmd_line );

	if( *arg_ptr[ 0 ] == '!' ) {
	    ok = last_history_event( cmd_line );
	    if( ok != SH_OK ) {
		wl_printf( shell_wid, "No matching event found\n\r" );
		continue;
	    }
	    last_cmd = NEXT_HIST( last_cmd );
	    insert_history( cmd_line, last_cmd );
	    wl_printf( shell_wid, "%s\n\r", cmd_line );
	    parse_input( arg_ptr, cmd_line );
	}


	if( arg_ptr[ 0 ] != NULL ) {
	    i = find_cmd( arg_ptr[ 0 ] );
	    if( i >= 0 ) {
		dispatch_table[ i ].proc( &arg_ptr[ 1 ] );
	    }
	    else {
		rc = hermes_exec( arg_ptr[ 0 ], &arg_ptr[ 1 ], environ,
				  handles );
		switch( rc ) {
		case 0:
		    break;
		default:
		    wl_printf( shell_wid, "Unknown command or filename\n\r" );
		    break;
		}
	    }
	    cmd_num++;
	}
    }
    _exit( 1 );
}

