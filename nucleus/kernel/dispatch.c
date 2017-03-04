
#include <kernel.h>



/*
 * Ready queues for all eight priorities.
 */
PRIVATE PCB *ready_queue[ MAX_READY_QUEUES ];

/*
 * The bits in ready_procs tell which ready queue is empty.
 * The MSB of ready_procs corresponds to ready_queue[ 31 ].
 */
PRIVATE unsigned ready_procs;



/*
 * add_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by p is put the ready queue.
 * The appropiate ready queue is determined by p->priority.
 */

void add_ready_queue( PCB* p )
{
	int 			prio;
	volatile int	i_flag;

	DISABLE_INTR( i_flag );
	prio = p->priority;
	if( ready_queue[ prio ] == NULL ) {
		/* The only process on this priority level */
		ready_queue[ prio ] = p;
		p->next 			= p;
		p->last 			= p;
		ready_procs |= 1 << prio;
	} else {
		/* Some other processes on this priority level */
		p->next  = ready_queue[ prio ];
		p->last  = ready_queue[ prio ]->last;
		ready_queue[ prio ]->last->next = p;
		ready_queue[ prio ]->last		= p;
	}
	p->state = STATE_READY;
	ENABLE_INTR( i_flag );
}



/*
 * remove_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by p is dequeued from the ready
 * queue.
 */

void remove_ready_queue( PCB *p )
{
	int 			prio;
	volatile int	i_flag;

	DISABLE_INTR( i_flag );
	prio = p->priority;
	if( p->next == p ) {
		/* No further processes on this priority level */
		ready_queue[ prio ] = NULL;
		ready_procs &= ~( 1 << prio );
	} else {
		ready_queue[ prio ] = p->next;
		p->next->last		= p->last;
		p->last->next		= p->next;
	}
	ENABLE_INTR( i_flag );
}



/*
 * dispatcher
 *----------------------------------------------------------------------------
 * Determines a new process to be dispatched. The process
 * with the highest priority is taken. Within one priority
 * level round robin is used.
 */

PCB *dispatcher( void )
{
	unsigned		i;
	volatile int	i_flag;

	DISABLE_INTR( i_flag );
	/* Find queue with highest priority that is not empty */
	for( i = MAX_READY_QUEUES - 1; i >= 0; i-- )
		if( ready_procs & ( 1 << i ) )
			break;
	ENABLE_INTR( i_flag );
	if( i == active_pcb->priority )
		/* Round robin within the same priority level */
		return( active_pcb->next );
	else
		/* Dispatch a process at a different priority level */
		return( ready_queue[ i ] );
}



/*
 * resign
 *----------------------------------------------------------------------------
 * The current process gives up the CPU voluntarily. The
 * next running process is determined via dispatcher().
 * The stack of the calling process is setup such that it
 * looks like an interrupt.
 */

void resign( void )
{
	/*
	 *		PUSHFL
	 *		CLI
	 *		POPL	%EAX		; EAX = Flags
	 *		XCHGL	(%ESP),%EAX ; Swap return adr with flags
	 *		PUSH	%CS 		; Push CS
	 *		PUSHL	%EAX		; Push return address
	 *		PUSHAL				; Save process' context
	 *		PUSH	%DS
	 *		PUSH	%ES
	 *		PUSH	%FS
	 *		PUSH	%GS
	 */
	asm( "pushfl;cli;popl %eax;xchgl (%esp),%eax" );
	asm( "push %cs;pushl %eax;pushal" );
	asm( "push %ds;push %es;push %fs;push %gs" );

	/* Save the context pointer SS:ESP to the PCB */

	asm( "movw %%ss,%0" : "=r" (active_pcb->context_ss) : );
	asm( "movl %%esp,%0" : "=r" (active_pcb->context_esp) : );
	asm( "movl %%cr3,%0" : "=r" (active_pcb->context_cr3) : );

	/* Dispatch new process */
	active_pcb = dispatcher();

	/* Save %esp in TSS */
	tss.esp0 = active_pcb->sys_stack_base + PAGE_SIZE - HERMES_TEXT_BASE;

	/* Restore context pointer SS:ESP */
	asm( "movl %0,%%cr3" : : "r" (active_pcb->context_cr3) );
	asm( "movw %0,%%ss" : : "r" (active_pcb->context_ss) );
	asm( "movl %0,%%esp" : : "r" (active_pcb->context_esp) );

	/*
	 *		POP 	%GS 	; Restore previously saved context
	 *		POP 	%FS
	 *		POP 	%ES
	 *		POP 	%DS
	 *		POPAL
	 *		IRET			; Return to new process
	 */
	asm( "pop %gs;pop %fs;pop %es;pop %ds" );
	asm( "popal;iret" );
}



/*
 * init_dispatcher
 *----------------------------------------------------------------------------
 * Initializes the necessary data structures. The parameter
 * "p" points to the first process in the system.
 */

void init_dispatcher( PCB *p )
{
	int i;

	for( i = 0; i < MAX_READY_QUEUES; i++ )
		ready_queue[ i ] = NULL;

	ready_procs = 0;

	/* Setup first process */
	add_ready_queue( p );
#if 0
	printf( "init_dispatcher(): Done\n" );
#endif
}
