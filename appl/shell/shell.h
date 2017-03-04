#ifndef __HSHELL__

#define __HSHELL__


#define FALSE				0
#define TRUE				1

#define SH_OK				0
#define SH_NO_EVENT_FOUND	-1

#define MAX_CMD_LINE		128
#define MAX_PARAM			32

#define ELDEST_CMD( x )		( x == first_cmd )
#define YOUNGEST_CMD( x )	( x == last_cmd )
#define NEXT_HIST( x )		( ( (x) + 1 ) % max_history )
#define PREV_HIST( x )		( ( (x) + max_history - 1 ) % max_history )

#ifdef __HERMES__
#define FHANDLE( x )		(x)._file
#endif

#ifdef __TURBOC__
#define FHANDLE( x )		(x).fd
#endif


typedef void (*PROC_PTR) (char**);


typedef struct {
			char		*cmd;
			PROC_PTR	proc;
			char		*help_txt;
} DISPATCH_TABLE_TYPE;



#endif
