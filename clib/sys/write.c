#include <cinclude.h>
#include <ci_types.h>
#include <winlib.h>
#include <sysmsg.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <fs.h>

extern FILE_HANDLE_TYPE	*handle_buf;
extern void		*winlib_copy_buffer;



int write( int handle, const void *source, unsigned n )
{
    assert (0);
#if 0
    FileWriteMsg	*fsmsg = lib_copy_buffer;
    FileWriteReply	*fsreply = lib_copy_buffer;
    PRINT_WINDOW_MSG	*msg;
    int			nbytes;
    char		*str;

    if( handle_buf[ handle ].handle_type == WINDOW_FILE ) {
	msg = winlib_copy_buffer;
	msg->req = PRINT_WINDOW;
	msg->wid = (WID) handle_buf[ handle ].handle_num;
	bcopy( source, &msg->string, n );
	str = &msg->string;
	str[ n ] = 0;
	WindowServerSend( winlib_copy_buffer,
			  sizeof( PRINT_WINDOW_MSG ) + n + 1, &nbytes );
	/*
	  if( nbytes != 0 ) {
	  return( -1 );
	  }
	  else {
	  return( nbytes );
	  }
	*/
	return( n );
    }

    fsmsg->Type = FILE_WRITE;
    fsmsg->Handle = (unsigned) handle_buf[ handle ].handle_num;
    fsmsg->NumberOfBytes = n;
    fsmsg->Source = USER_ADDR( source );
    fsmsg->Size = sizeof( FileWriteMsg );
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
#endif
    return( 0 );
}

