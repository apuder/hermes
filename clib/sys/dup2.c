#include <cinclude.h>
#include <ci_types.h>
#include <winlib.h>
#include <sysmsg.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <fs.h>

extern FILE_HANDLE_TYPE	*handle_buf;
extern void		*winlib_copy_buffer;



int dup2( int old_handle, int new_handle )
{
    assert (0);
#if 0
    DupHandleMsg	*fsmsg = lib_copy_buffer;
    DupHandleReply	*fsreply = lib_copy_buffer;
    COMMON_WINDOW_MSG	*msg = winlib_copy_buffer;
    int			nbytes, rc;

    if( handle_buf[ new_handle ].handle_num != -1 ) {
	rc = close( new_handle );
	if( rc == -1 ) return( -1 );
    }

    if( handle_buf[ old_handle ].handle_type == WINDOW_FILE ) {
	msg->req = WS_DUP_WINDOW;
	msg->wid = (WID) handle_buf[ old_handle ].handle_num;
	WindowServerSend( winlib_copy_buffer,
			  sizeof( COMMON_WINDOW_MSG ), &nbytes );
	handle_buf[ new_handle ].handle_type = WINDOW_FILE;
	handle_buf[ new_handle ].handle_num = msg->wid;
	return( 0 );
    }

    fsmsg->Type = DUP_HANDLE;
    fsmsg->Handle = (unsigned) handle_buf[ old_handle ].handle_num;
    fsmsg->Size = sizeof( DupHandleMsg );
    FileServerSend( lib_copy_buffer, fsmsg->Size, &nbytes );
    errno = fsreply->Result;
    handle_buf[ new_handle ].handle_type = PHYS_FILE;
    handle_buf[ new_handle ].handle_num = fsreply->Handle;
    if( fsreply->Result != 0 ) {
	/*
	  hier muß errno noch gesetzt werden...
	  switch( fsreply->Result )
	  case CREATE_ERROR
	*/
	return( -1 );
    }
#endif
    return( 0 );
}

