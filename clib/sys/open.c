#include <cinclude.h>
#include <ci_types.h>
#include <winlib.h>
#include <sysmsg.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include <fs.h>
#include <errno.h>

extern void		*winlib_copy_buffer;

int			_fmode = 0x4000;
FILE_HANDLE_TYPE	*handle_buf = NULL;



void open_init( void )
{
    int	i, n;

    n = getdtablesize();
    handle_buf = (FILE_HANDLE_TYPE*) malloc( n * sizeof( FILE_HANDLE_TYPE ) );
    for( i = 0; i < n; i++ ) {
	handle_buf[ i ].handle_num = handle_buf[ i ].handle_type = -1;
    }
}



int find_free_slot( void )
{
    int	i;

    for( i = 0; i < getdtablesize(); i++ ) {
	if( handle_buf[ i ].handle_num == -1 ) {
	    return( i );
	}
    }

    return( -1 );
}



unsigned open_defwindow( char *name )
{
    WINDOW_MSG	*msg;
    int		data_len, rc;

    msg = winlib_copy_buffer;
    msg->req = OPEN_WINDOW;
    msg->new_window.min_dx = 10;
    msg->new_window.min_dy = 2;
    msg->new_window.x = DEFAULT_POS; msg->new_window.y = DEFAULT_POS;
    msg->new_window.dx = DEFAULT_SIZE; msg->new_window.dy = DEFAULT_SIZE;
    msg->new_window.window_options = USER_BORDER | ALLOW_ICON | ALLOW_ZOOM |
	MID_HEADER | PRINT_SHADOW |
	ALLOW_RAISE | ALLOW_MOVE | ALLOW_RESIZE;
    msg->new_window.tab_step = 8;
    msg->new_window.border_col = msg->new_window.screen_col =
	msg->new_window.title_col = msg->new_window.scroll_col =
	msg->new_window.icon_col = msg->new_window.button_col = DEFAULT_COL;
    strncpy( (char*) msg->new_window.title, name, MAX_HEADER_LEN );
    strcpy( (char*) msg->new_window.user_border, "Ø¹Õ¾Íþ" );
    msg->new_window.father_window = ROOT_WINDOW;
    msg->new_window.iconic = 0;
    msg->new_window.full_screen = 0;
    WindowServerSend( winlib_copy_buffer, sizeof( WINDOW_MSG ), &data_len );

    rc = 0;
    if( rc ) {
	return( -1 );
    }
    else {
	return( (int) msg->wid );
    }
}



int open( const char *path, int flags, ... )
{
    FileOpenMsg		*fsmsg = lib_copy_buffer;
    FileOpenReply	*fsreply = lib_copy_buffer;
    int			nbytes, fsflags, i, tmp;

    if( !handle_buf ) {
	open_init();
    }

    i = find_free_slot();
    if( i < 0 ) {								/* No more slots available */
	return( -1 );
    }

    if( strcmp( path, "WINFILE" ) == 0 ) {
	tmp = (int) open_defwindow( path );
	if( tmp == -1 ) {
	    return( -1 );
	}

	handle_buf[ i ].handle_num = tmp;
	handle_buf[ i ].handle_type = WINDOW_FILE;
	return( i );
    }

    fsflags = 0;
    if( ( flags & O_RDONLY ) || ( flags & O_RDWR ) ) {
	fsflags |= TOS_FS_OPEN_MODE_READ;
    }
    if( ( flags & O_WRONLY || flags & O_RDWR ) && ( flags & O_TRUNC ) ) {
	fsflags |= TOS_FS_OPEN_MODE_WRITE;
    }
    if( ( flags & O_WRONLY || flags & O_RDWR ) && !( flags & O_TRUNC ) ) {
	fsflags |= TOS_FS_OPEN_MODE_APPEND;
    }

    fsmsg->Type = FILE_OPEN;
    fsmsg->AccessMode = fsflags;
    strcpy( &fsmsg->Path, path );
    fsmsg->Size = sizeof( FileOpenMsg ) + strlen( path ) + 1;
    FileServerSend( lib_copy_buffer, fsmsg->Size, &nbytes );
    errno = fsreply->Result;
    if( fsreply->Result != 0 ) {
	/*
	  hier muß errno noch gesetzt werden...
	  switch( fsreply->Result )
	  case CREATE_ERROR
	*/
	return( -1 );
    }
    handle_buf[ i ].handle_type = PHYS_FILE;
    handle_buf[ i ].handle_num = (int) fsreply->Handle;
    return( i );
}

