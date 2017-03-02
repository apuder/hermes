#include <cinclude.h>
#include <ci_types.h>
#include <winlib.h>
#include <sysmsg.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <fs.h>

extern void		*winlib_copy_buffer;
extern FILE_HANDLE_TYPE	*handle_buf;



int read( int handle, void *dest, unsigned n )
{
    FileReadMsg		*fsmsg = lib_copy_buffer;
    FileReadReply	*fsreply = lib_copy_buffer;
    DEQUEUE_EVENT_MSG	*msg;
    EVENT_INFO_TYPE	*e;
    int			nbytes;
    unsigned short	*dest_buf;

    if( handle_buf[ handle ].handle_type == WINDOW_FILE ) {
	dest_buf = (unsigned short*) dest;
	msg = winlib_copy_buffer;
	e = winlib_copy_buffer;
	do {
	    msg->req = DEQUEUE_WINDOW_EVENT;
	    EventManagerSend( winlib_copy_buffer,
			      sizeof( DEQUEUE_EVENT_MSG ), &nbytes );
	} while( e->event_type != EV_KEY_PRESSED );

	if( e->ke.key == 13 ) {			/* Translate CR to LF */
	    e->ke.key = 10;
	}

	if( e->ke.key == 26 ) {
	    return( 0 );
	}

	*dest_buf = e->ke.key;
	if( e->ke.key < 256 ) {
	    return( 1 );
	}
	else {
	    return( 2 );
	}
    }

    fsmsg->Type = FILE_READ;
    fsmsg->Handle = (unsigned) handle_buf[ handle ].handle_num;
    fsmsg->NumberOfBytes = n;
    fsmsg->Destination = USER_ADDR( dest );
    fsmsg->Size = sizeof( FileReadMsg );
    FileServerSend( lib_copy_buffer, fsmsg->Size, &nbytes );
    errno = fsreply->Result;
    if( fsreply->Result == TOS_NO_ERROR || fsreply->Result == TOS_ERR_BEYOND_EOF ) {
	return( (int) fsreply->NumOfBytes );
    }
    else {
	return( -1 );
    }
}

