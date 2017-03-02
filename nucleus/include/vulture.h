
#ifndef __VULTURE__
#define __VULTURE__


extern PID process_to_kill;

extern SEMA vulture_request_sema;
extern SEMA vulture_wakeup_sema;


void vulture_process( PORT_ID first_port, HANDLE handle );


#endif

