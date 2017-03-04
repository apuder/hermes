
#include <ci_types.h>
#include <cinclude.h>
#include <fs.h>

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <winlib.h>
#include <sysmsg.h>


extern FILE_HANDLE_TYPE	*handle_buf;
extern void		*winlib_copy_buffer;


/* Is new process to be debugged? */
int hermes_exec_debug_flag = FALSE;


/*
  change_fs_handle_owner
  -------------------------------------------------------
  Changes the owner of a file server handle.
*/

int change_fs_handle_owner( int handle, int new_owner )
{
    ChownHandleMsg	*fsmsg = lib_copy_buffer;
    ChownHandleReply	*fsreply = lib_copy_buffer;
    int			data_len, nbytes;

    fsmsg->Type = CHOWN_HANDLE;
    fsmsg->Handle = handle;
    fsmsg->newOwnerPid = new_owner;
    fsmsg->Size = sizeof( ChownHandleMsg );
    FileServerSend( lib_copy_buffer, fsmsg->Size, &nbytes );
    return( fsreply->Result );
}



/*
  change_handle_owner
  -------------------------------------------------------
  Changes the owner of a handle.
*/

int change_handle_owner( int handle, int new_owner )
{
    COMMON_WINDOW_MSG	*msg = winlib_copy_buffer;
    int			err, data_len, nbytes;

    if( handle_buf[ handle ].handle_type == WINDOW_FILE ) {
	msg->req = WS_CHOWN_HANDLE;
	msg->wid = handle_buf[ handle ].handle_num;
	msg->param1 = new_owner;
	WindowServerSend( winlib_copy_buffer, sizeof( COMMON_WINDOW_MSG ),
			  &data_len );
	handle_buf[ handle ].handle_num = handle_buf[ handle ].handle_type = -1;
	return( msg->rc );
    }

    err = change_fs_handle_owner( handle_buf[ handle ].handle_num, new_owner );
    handle_buf[ handle ].handle_num = handle_buf[ handle ].handle_type = -1;
    return( err );
}



/*
  pack_args
  -------------------------------------------------------
  Packs the arguments given in argc and argv to continous
  block.
*/

int pack_args( char **args, void *dest, int len )
{
    int		used_len = 0;
    int		arg_cnt = 0;
    int		i = 0;
    int		base;
    char	*dest_ptr;

    dest_ptr = ( char * ) dest;
    base = 2 * sizeof( int );
    while( 1 ) {
	
	if( args[ i ] == NULL ) {
	    break;
	}

	if( used_len + strlen( args[ i ] ) + 1 > len ) {
	    ( ( int* ) dest )[ 0 ] = arg_cnt;
	    ( ( int* ) dest )[ 1 ] = used_len;
	    return( TOO_MANY_ARGS );
	}

	strcpy( &dest_ptr[ base + used_len ], args[ i ] );
	used_len += strlen( args[ i ] ) + 1;
	arg_cnt++;
	i++;
    }
    ( ( int* ) dest )[ 0 ] = arg_cnt;
    ( ( int* ) dest )[ 1 ] = used_len;
    return( used_len + 2 * sizeof( int ) );
}



/*
  unpack_args
  -------------------------------------------------------
  Unpacks arguments given in a continous block and builds
  a argc, argv structure. The buffer is copied into an
  other block.
*/

void unpack_args( void *source, int *arg_num, void **arg_block )
{
    char	*buf;
    char	*source_ptr;
    char	**ptr_vec;
    int		buf_len, i;
    int		arg_cnt = 0;

    *arg_num = ( ( int* ) source )[ 0 ];
    buf_len = ( ( int* ) source )[ 1 ];
    buf = malloc( buf_len );
    ptr_vec = malloc( ( *arg_num + 1 ) * sizeof( char* ) );
    source_ptr = &( ( char* ) source )[ 2 * sizeof( int ) ];
    i = 0;

    while( i < buf_len ) {
	ptr_vec[ arg_cnt++ ] = &buf[ i ];
	do {
	    buf[ i ] = source_ptr[ i++ ];
	} while( source_ptr[ i - 1 ] != 0 );
    }
    ptr_vec[ *arg_num ] = NULL;
    *arg_block = ( void* ) ptr_vec;
}


/*
  hermes_exec
  ------------------------------------------------------------
  Execute a new process. After creating the new process, the
  old functions continues. args and environment are vectors to
  strings. Both vectors are terminated by a NULL pointer. The
  file handles are: stdin, stdout, stderr.
*/

int hermes_exec( const char *prg,
		 const char **args,
		 const char **environment,
		 int *handles )
{
    FILE_HANDLE_TYPE	*handle_msg = lib_copy_buffer;
    FileOpenMsg		*fsmsg = lib_copy_buffer;
    FileOpenReply	*fsreply = lib_copy_buffer;
    PID			new_pid;
    PORT_ID		child_port;
    int			fs_handle;
    char		*cp;
    char		**new_args;
    int			nbytes, nRecv;
    int			i, j, h0, h1, h2;

    cp = &fsmsg->Path;
    fsmsg->Type = FILE_OPEN;
    fsmsg->AccessMode = TOS_FS_OPEN_MODE_READ;
    for( i = 0; prg[ i ] != '\0'; i++ )
	*cp++ = prg[ i ];
    *cp = '\0';
    fsmsg->Size = sizeof( FileOpenMsg ) + i;
    FileServerSend( lib_copy_buffer, fsmsg->Size, &nbytes );
    if( fsreply->Result != 0 ) {
	return( fsreply->Result != 0 );
    }
    fs_handle = fsreply->Handle;
    k_p1 = fs_handle;
    kernel_trap( K_CREATE_USER_PROC );
    new_pid = k_r1;
    child_port = k_r2;
    change_fs_handle_owner( fs_handle, new_pid );
    /*
     * Set the process' debug flag if necessary.
     */
    if( hermes_exec_debug_flag ) {
	k_p1 = new_pid;
	kernel_trap( K_SET_DEBUG_FLAG );
    }

    k_p1 = new_pid;
    kernel_trap( K_WAKEUP_PROCESS );

    i = 0;
    while( args != NULL && args[ i ] != NULL ) i++;
    new_args = malloc( ( i + 2 ) * sizeof( char* ) );
    new_args[ 0 ] = prg;
    new_args[ i + 1 ] = NULL;
    for( j = 0; j < i; j++ ) {
	new_args[ j + 1 ] = args[ j ];
    }

    nbytes = pack_args( new_args, lib_copy_buffer, COPY_BUFFER_SIZE );
    free( new_args );
    Message( child_port, lib_copy_buffer, nbytes );

    nbytes = pack_args( environment, lib_copy_buffer, COPY_BUFFER_SIZE );
    Message( child_port, lib_copy_buffer, nbytes );

    /* Duplicate stdin, stdout, stderr and send it to the child. Erase
       duplicated handles from handle_buf */

#if 1
    //XXX
    handle_msg[ 0 ].handle_type = handle_msg[ 0 ].handle_num = DEFAULT_FILE;
    handle_msg[ 1 ].handle_type = handle_msg[ 1 ].handle_num = DEFAULT_FILE;
    handle_msg[ 2 ].handle_type = handle_msg[ 2 ].handle_num = DEFAULT_FILE;
#else
    if( handles[ 0 ] == DEFAULT_FILE ) {
	handle_msg[ 0 ].handle_type = handle_msg[ 0 ].handle_num = DEFAULT_FILE;
    }
    else {
	h0 = dup( handles[ 0 ] );
	handle_msg[ 0 ] = handle_buf[ h0 ];
	change_handle_owner( h0, new_pid );
	/*
	  handle_buf[ h0 ].handle_num = handle_buf[ h0 ].handle_type = -1;
	*/
    }


    if( handles[ 1 ] == DEFAULT_FILE ) {
	handle_msg[ 1 ].handle_type = handle_msg[ 1 ].handle_num = DEFAULT_FILE;
    }
    else {
	h1 = dup( handles[ 1 ] );
	handle_msg[ 1 ] = handle_buf[ h1 ];
	change_handle_owner( h1, new_pid );
	/*
	  handle_buf[ h1 ].handle_num = handle_buf[ h1 ].handle_type = -1;
	*/
    }

    if( handles[ 2 ] == DEFAULT_FILE ) {
	handle_msg[ 2 ].handle_type = handle_msg[ 2 ].handle_num = DEFAULT_FILE;
    }
    else {
	h2 = dup( handles[ 2 ] );
	handle_msg[ 2 ] = handle_buf[ h2 ];
	change_handle_owner( h2, new_pid );
	/*
	  handle_buf[ h2 ].handle_num = handle_buf[ h2 ].handle_type = -1;
	*/
    }
#endif
    Message( child_port, lib_copy_buffer, 3 * sizeof( FILE_HANDLE_TYPE ) );

    return( OK );
}
