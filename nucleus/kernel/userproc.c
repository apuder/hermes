
#include <kernel.h>
#include <fs.h>
#include <assert.h>


FileOpenMsg file_open_msg;



void start_new_user_process( PORT_ID first_port, HANDLE handle )
{
    active_pcb->system = FALSE;
    active_pcb->fs_handle = handle;
    if( active_pcb->single_step )
	asm( "pushl $512 | 256" );	/* IF = TF = 1 */
    else
	asm( "pushl $512");		/* IF = 1 */

    /* Pop of EFLAGS from stack and save them in %ecx */
    asm( "popl %ecx" );

    /* Push first port */
    asm( "movl %0,%%ebx" : : "m" (first_port) );
    asm( "movw $35,%ax" );
    asm( "movw %ax,%ds" );
    asm( "movw %ax,%es" );
    asm( "movw %ax,%fs" );
    asm( "pushl $35" );		/* ss3 */
    asm( "pushl $1000000" );	/* esp3 */
    asm( "pushl %ecx" );	/* EFLAGS */
    asm( "pushl $43" );		/* CS */
    asm( "pushl $0" );	        /* IP */
    asm( "iret" );
    /* We never get back to this location! */
}


void create_user_process( HANDLE fs_handle, PID *pid, PORT_ID *child_port )
{
    if( setup_new_process( start_new_user_process, 15, "User process",
			   pid, child_port, fs_handle ) )
	shutdown( "create_user_process(): Can't create user process" );
}


void load_boot_process( void )
{
    VIR_ADDR		cb = VM_USER_BEGIN;
    char		*p;
    char		*s = (char *) &file_open_msg;
    FileOpenReply	*r;
    ChownHandleMsg	*fsmsg;
    ChownHandleReply	*fsreply;
    int			handle;
    int			nbytes;
    PID			dummy_pid;
    PORT_ID		dummy_port;
    int			i;

    p = HERMES_DATA_PTR( cb );
    r = HERMES_DATA_PTR( cb );
    fsmsg = HERMES_DATA_PTR( cb );
    fsreply = HERMES_DATA_PTR( cb );
    file_open_msg.Size = sizeof( FileOpenMsg );
    file_open_msg.Type = FILE_OPEN;
    file_open_msg.AccessMode = TOS_FS_OPEN_MODE_READ;
    file_open_msg.Path[ 0 ] = 'B';
    file_open_msg.Path[ 1 ] = 'O';
    file_open_msg.Path[ 2 ] = 'O';
    file_open_msg.Path[ 3 ] = 'T';
    file_open_msg.Path[ 4 ] = 0;

    for( i = 0; i < sizeof( file_open_msg ); i++ ) {
	*p++ = *s++;
    }
    k_send( file_server_port, cb, sizeof( file_open_msg ), &nbytes );
    assert( r->Result == TOS_NO_ERROR );
    handle = r->Handle;
    create_user_process( r->Handle, &dummy_pid, &dummy_port );
    fsmsg->Type = CHOWN_HANDLE;
    fsmsg->Handle = handle;
    fsmsg->newOwnerPid = dummy_pid;
    fsmsg->Size = sizeof( ChownHandleMsg );
    k_send( file_server_port, cb, sizeof( ChownHandleMsg ), &nbytes );
    assert( fsreply->Result == TOS_NO_ERROR );
    k_wakeup( dummy_pid );
}
