#include <cinclude.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <fs.h>

extern FILE_HANDLE_TYPE	*handle_buf;


long lseek( int handle, long offset, int flag )
{
    FileSeekMsg		*fsmsg = lib_copy_buffer;
    FileSeekReply	*fsreply = lib_copy_buffer;
    int			nbytes, fsflags;

    if( handle_buf[ handle ].handle_type == WINDOW_FILE ) {
	return( 0 );
    }

    fsmsg->Type = FILE_SEEK;
    fsmsg->Handle = (unsigned) handle_buf[ handle ].handle_num;
    //XXX fsmsg->SeekMode = flag;
    fsmsg->NumOfBytes = offset;
    fsmsg->Size = sizeof( FileSeekMsg );
    FileServerSend( lib_copy_buffer, fsmsg->Size, &nbytes );
    errno = fsreply->Result;
    if( fsreply->Result != 0 && fsreply->Result != 5 ) {
	errno = ( fsreply->Result != TOS_NO_ERROR ) ? EINVAL : EBADF;
	return( -1 );
    }

    return( fsreply->FilePosition );
}

