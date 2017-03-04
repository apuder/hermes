#include <cinclude.h>
#include <ci_types.h>
#include <winlib.h>
#include <sysmsg.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <fs.h>

extern void				*winlib_copy_buffer;
extern FILE_HANDLE_TYPE	*handle_buf;

int close( int handle )
{
	FileCloseMsg		*fsmsg = lib_copy_buffer;
	FileCloseReply		*fsreply = lib_copy_buffer;
	COMMON_WINDOW_MSG	*msg;
	int					nbytes, rc;

	if( handle_buf[ handle ].handle_type == DEFAULT_FILE ) {
		return( 0 );
	}

	if( handle_buf[ handle ].handle_type == WINDOW_FILE ) {
		msg = winlib_copy_buffer;
		msg->req = CLOSE_WINDOW;
		msg->wid = (WID) handle_buf[ handle ].handle_num;
		WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ), &nbytes );
		handle_buf[ handle ].handle_num = handle_buf[ handle ].handle_type = -1;
rc = 0;
		if( rc != 0 ) {
			return( -1 );
		}
		else {
			return( rc );
		}
	}

	fsmsg->Type = FILE_CLOSE;
	fsmsg->Handle = (unsigned) handle_buf[ handle ].handle_num;
	fsmsg->Size = sizeof( FileCloseMsg );
	FileServerSend( lib_copy_buffer, fsmsg->Size, &nbytes );
	handle_buf[ handle ].handle_num = handle_buf[ handle ].handle_type = -1;
	errno = fsreply->Result;
	if( fsreply->Result != 0 ) {
/*
hier muß errno noch gesetzt werden...
		switch( fsreply->Result )
			case CREATE_ERROR
*/
		return( -1 );
	}
	return( 0 );
}

