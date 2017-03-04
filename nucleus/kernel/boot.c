
#include <kernel.h>
#include <vulture.h>


PORT_ID window_port 		= NO_PORT;
PORT_ID timer_port			= NO_PORT;
PORT_ID event_port			= NO_PORT;
PORT_ID file_server_port	= NO_PORT;


extern void event_manager( PORT_ID, HANDLE );
extern void timer_manager( PORT_ID, HANDLE );
extern void file_server( PORT_ID, HANDLE );
extern void debug_process( PORT_ID, HANDLE );
extern void vulture_process( PORT_ID, HANDLE );



void null_process( PORT_ID first_port, HANDLE handle )
{
	destroy_port( first_port );
	while( TRUE ) ;
}



void boot( void )
{
	PID 	dummy_pid;
	PORT_ID dummy_port;

	if( create( null_process, 0, "Null process",
				&dummy_pid, &dummy_port, NO_HANDLE ) )
		shutdown( "boot(): Can't create null process" );
#if 0
	if( create( debug_process, PRIO_DEBUG_PROCESS, "Debugger",
				&dummy_pid, &dummy_port, NO_HANDLE ) )
		shutdown( "boot(): Can't create debug process" );
#endif
	if( create( event_manager, PRIO_EVENT_MANAGER, "Event manager",
				&dummy_pid, &event_port, NO_HANDLE ) )
		shutdown( "boot(): Can't create event manager" );
	if( create( timer_manager, PRIO_TIMER_MANAGER, "Timer manager",
				&dummy_pid, &timer_port, NO_HANDLE ) )
		shutdown( "boot(): Can't create timer manager" );
	if( create( file_server, PRIO_FILE_SERVER, "File server",
				&dummy_pid, &file_server_port, NO_HANDLE ) )
		shutdown( "boot(): Can't create file server" );

	/* Init semaphores for vulture */
	init_sema( &vulture_request_sema, 1 );
	init_sema( &vulture_wakeup_sema, 0 );
	if( create( vulture_process, PRIO_VULTURE, "Vulture",
				&dummy_pid, &dummy_port, NO_HANDLE ) )
		shutdown( "boot(): Can't create vulture" );
}
