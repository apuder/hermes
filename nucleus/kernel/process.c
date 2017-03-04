
#include <kernel.h>
#include <assert.h>
#include "io.h"


unsigned my_node_id = 0;


PCB pcb[ MAX_PROCS ];
PCB *next_free_pcb;
PCB *active_pcb;



PID conv_pcb_to_pid( PCB *p )
{
    PID i;

    i = p - pcb;
    i |= p->seq << 8;
    i |= my_node_id << 16;
    return( i );
}


PCB *conv_pid_to_pcb( PID pid )
{
    return( &pcb[ pid & BYTE ] );
}



ERROR setup_new_process( void (*new_proc) (PORT_ID, HANDLE),
			 int prio,
			 char *descr,
			 PID *new_pid,
			 PORT_ID *first_port,
			 HANDLE handle )
{
    unsigned		esp;
    unsigned		phy_esp;
    PCB 		*p;
    int 		err;
    volatile int	i_flag;

    DISABLE_INTR( i_flag );
    if( prio >= MAX_READY_QUEUES ) {
	ENABLE_INTR( i_flag );
	return( panic( ERR_BAD_PRIO, "create(): Bad priority" ) );
    }
    if( next_free_pcb == NULL ) {
	ENABLE_INTR( i_flag );
	return( panic( ERR_NO_RESOURCES, "create(): PCB full" ) );
    }
    p = next_free_pcb;
    next_free_pcb = p->next;
    p->used 		= TRUE;
    p->state		= STATE_SUSPENDED;
    p->panic		= TRUE;
    p->system		= TRUE;
    p->to_be_debugged	= FALSE;
    p->single_step	= FALSE;
    p->debugger		= FALSE;
    p->priority 	= prio;
    p->port_list	= NULL;
    p->port_scan	= NULL;
    p->name 		= descr;
    p->fs_handle	= 0;
    p->text_size	= 0;
    p->data_size	= 0;
    p->bss_size		= 0;
    p->debug_size	= 0;
    if( ++p->seq == 0x80 ) p->seq = 0;
    ENABLE_INTR( i_flag );

    /*
     * So far we've found a free PCB slot which is marked used.
     * Now we try to allocate a new port. If this fails, the
     * new PCB is freed again and an error is returned.
     */
    err = create_new_port( p, first_port );
    if( err ) {

	/* Release PCB slot and return error */
	p->used = FALSE;
	DISABLE_INTR( i_flag );
	p->next = next_free_pcb;
	next_free_pcb = p;
	ENABLE_INTR( i_flag );
	return( err );
    }
    p->sbrk_ptr = VM_USER_BEGIN;
    p->context_cr3	= get_pdt( p->sys_stack_base );

    /* Compute linear address of new process' system stack */
    esp = p->sys_stack_base + PAGE_SIZE;
    phy_esp = vir_to_phy_addr( p->context_cr3, esp - 4 );
    assert( phy_esp != -1 );

#define PUSH( x )	poke_mem_l( phy_esp, (unsigned) (x) ); \
					phy_esp -= 4; esp -= 4;

    /* Initialize the stack for the new process */
    PUSH( handle );			/* Parameter of new process */
    PUSH( *first_port );		/* Parameter of new process */
    PUSH( 0 );			/* Dummy return address */
    PUSH( 512 );			/* Flags with enabled Interrupts */
    PUSH( CODE_SELECTOR );		/* Kernel code selector */
    PUSH( new_proc );		/* Entry point of new process */
    PUSH( 0 );			/* EAX */
    PUSH( 0 );			/* ECX */
    PUSH( 0 );			/* EDX */
    PUSH( 0 );			/* EBX */
    PUSH( 0 );			/* ESP */
    PUSH( 0 );			/* EBP */
    PUSH( 0 );			/* ESI */
    PUSH( 0 );			/* EDI */
    PUSH( DATA_SELECTOR );		/* DS */
    PUSH( DATA_SELECTOR );		/* ES */
    PUSH( DATA_SELECTOR );		/* FS */
    PUSH( GLOBAL_DATA_SELECTOR );	/* GS */

#undef PUSH

    /* Save context ptr (actually current stack pointer) */
    p->context_ss = DATA_SELECTOR;	/* Kernel stack selector */
    p->context_esp = esp - HERMES_TEXT_BASE;

    /*
     * Return the PID of new process. The PID is coded as an index
     * into the PCB table (LSB) and a sequence counter (MSB) for
     * fast process lookup. The upper word contains the network ID.
     */
    *new_pid = conv_pcb_to_pid( p );
    return( OK );
}



ERROR create( void (*new_proc) (PORT_ID, HANDLE),
	      int prio,
	      char *descr,
	      PID *new_pid,
	      PORT_ID *first_port,
	      HANDLE handle )
{
    ERROR	err;

    err = setup_new_process( new_proc, prio, descr, new_pid,
			     first_port, handle );
    if( err == OK ) k_wakeup( *new_pid );
    return( err );
}


void print_pcb()
{
    char *state[] = 
	{ "READY          ",
	  "SEMA_BLOCKED   ",
	  "SEND_BLOCKED   ",
	  "REPLY_BLOCKED  ",
	  "RECEIVE_BLOCKED",
	  "MESSAGE_BLOCKED",
	  "EVENT_BLOCKED  ",
	  "SUSPENDED      "
	};
    int i;
    volatile int i_flag;
    PCB* p = pcb;
    
    DISABLE_INTR (i_flag);
    printf ("PID  State           Prio Name\n");
    for (i = 0; i < MAX_PROCS; i++, p++) {
	if (!p->used)
	    continue;
	printf ("%04x ", conv_pcb_to_pid (p));
	printf ("%s ", state[p->state]);
	printf ("  %2d ", p->priority);
	printf ("%s\n", p->name);
    }
    ENABLE_INTR (i_flag);
}


void init_kernel( void )
{
    int 	sys_stack_adr;
    int 	i;

    sys_stack_adr = SYSTEM_STACK_BASE + PAGE_SIZE;

    /* Clear all PCB's */
    for( i = 0; i < MAX_PROCS; i++ ) {
	pcb[ i ].used			= FALSE;
	pcb[ i ].seq			= 0;
	pcb[ i ].sys_stack_base = sys_stack_adr;
	sys_stack_adr += 2 * PAGE_SIZE;
    }

    /* Create free list; don't bother about the first entry,
     * it'll be used for the boot process. */
    for( i = 1; i < MAX_PROCS - 1; i++ )
	pcb[ i ].next = &pcb[ i + 1 ];
    pcb[ MAX_PROCS - 1 ].next = NULL;
    next_free_pcb = &pcb[ 1 ];

    /* Define pcb[0] for this process */
    active_pcb = &pcb[0];
    active_pcb->used		= TRUE;
    active_pcb->panic		= TRUE;
    active_pcb->system		= TRUE;
    active_pcb->to_be_debugged	= FALSE;
    active_pcb->single_step	= FALSE;
    active_pcb->debugger	= FALSE;
    active_pcb->priority	= 1;
    active_pcb->port_list	= NULL;
    active_pcb->port_scan	= NULL;
    active_pcb->name		= "Boot process";

    asm( "movl %%cr3,%0" : "=r" (active_pcb->context_cr3) );

    /* Initialize the dispatcher */
    init_dispatcher( active_pcb );

    /* Initialize ports */
    init_ports();

    /* Initialize interrupt service routines */
    init_interrupts();

    /* Initialize network stuff */
    /*
      init_network();
    */
    printf( "Max. number of processes: %d\n", MAX_PROCS );
}
