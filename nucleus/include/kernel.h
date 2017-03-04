
#ifndef __KERNEL__
#define __KERNEL__

#include "stdarg.h"



#define TRUE			1
#define FALSE			0

#define BYTE			0xff
#define WORD			0xffff
#define LONG			0xffffffff

#define PRIVATE 		static
#define PUBLIC

#ifndef NULL
#define NULL			((void *) 0)
#endif


extern unsigned my_node_id;


#define DISABLE_INTR( save )	{ volatile int save_eax; \
				asm( "movl %%eax,%0" : "=m" (save_eax) ); \
				asm( "pushfl;popl %eax" ); \
                                asm( "movl %%eax,%0" : "=m" (save) : ); \
				asm( "movl %0,%%eax" : : "m" (save_eax)); \
				asm( "cli" ); }

#define ENABLE_INTR( save ) 	asm( "pushl %0" : : "m" (save) ); \
				asm( "popfl" );


typedef int ERROR;
typedef int BOOL;


/*
 * Max. number of processes
 */
#define MAX_PROCS		20



/*
 * Max. number of ready queues
 */
#define MAX_READY_QUEUES	32


#define STATE_READY 		0
#define STATE_SEMA_BLOCKED	1
#define STATE_SEND_BLOCKED	2
#define STATE_REPLY_BLOCKED 	3
#define STATE_RECEIVE_BLOCKED	4
#define STATE_MESSAGE_BLOCKED	5
#define STATE_EVENT_BLOCKED 	6
#define STATE_SUSPENDED		7


#define OK			0
#define ERR_NO_RESOURCES	-1
#define ERR_BAD_PORT		-2
#define ERR_BAD_PID 		-3
#define ERR_BAD_PRIO		-4
#define ERR_NOT_REPLY_BLOCKED	-5
#define ERR_ISR_BUSY		-6
#define ERR_BAD_INTR		-7
#define ERR_EVENT_QUEUE_BUSY	-8
#define ERR_EVENT_QUEUE_NOT_INIT -9
#define ERR_NOT_SUSPENDED	-10


/*=====>>> 386.s <<<========================================================*/


/* Selector used for interrupts */
#define IDT_SELECTOR            8

/* Kernel code selector */
#define CODE_SELECTOR		8

/* Kernel data selector */
#define DATA_SELECTOR		0x10

/* Global data selector */
#define GLOBAL_DATA_SELECTOR	0x18

/* TSS selector */
#define TSS_SELECTOR		0x30


/* Max. of 256 different interrupts */
#define MAX_IDT_ENTRIES         256

/* One IDT entry occupies 8 Bytes */
#define IDT_ENTRY_SIZE          8

/* Max. number of GDT entries */
#define MAX_GDT_ENTRIES		7

/* One GDT entry occupies 8 Bytes */
#define GDT_ENTRY_SIZE		8


/* Crawford & Gelsinger p.470 */
#define PRESENT_BIT 		1
#define READ_WRITE_BIT		2
#define USER_BIT		4

/*
 * See Crawford & Gelsinger p.456 for details.
 */

typedef struct {
        unsigned short limit_0_15;
        unsigned short base_0_15;
        unsigned short base_16_23  : 8;
        unsigned short type        : 4;
        unsigned short dt          : 1;
        unsigned short dpl         : 2;
        unsigned short p           : 1;
        unsigned short limit_16_19 : 4;
        unsigned short avl         : 1;
        unsigned short             : 1;
        unsigned short d           : 1;
        unsigned short g           : 1;
        unsigned short base_24_31  : 8;                                         	} MEM_SEGMENT_DESCR;


typedef struct {
	unsigned short offset_0_15;
	unsigned short selector;
	unsigned short count        : 4;
	unsigned short              : 3;
	unsigned short type         : 4;
	unsigned short dt           : 1;
	unsigned short dpl          : 2;
	unsigned short p            : 1;
	unsigned short offset_16_31;
	} GATE_SEGMENT_DESCR;



/*
 * Prototypes for assembler functions declared in 386.s
 */

void poke_mem_b( unsigned addr, unsigned char content );
void poke_mem_w( unsigned addr, unsigned short content );
void poke_mem_l( unsigned addr, unsigned content );
unsigned char peek_mem_b( unsigned addr );
unsigned short peek_mem_w( unsigned addr );
unsigned peek_mem_l( unsigned addr );



/*=====>>> gdt.c <<<========================================================*/

void init_gdt( void );


/*=====>>> tss.c <<<========================================================*/

typedef struct {
	unsigned	link;
	unsigned	esp0;
	unsigned	ss0;
	unsigned	regs[ 22 ];
	unsigned	iomap;
} TSS_SEGMENT;

extern TSS_SEGMENT tss;


void init_tss( void );

/*=====>>> idt.c <<<========================================================*/


#define TIMER_INTR_NO			0x60


#define IDT_ENTRY( offset )     (unsigned)(offset) & BYTE,              \
                                ( (unsigned)(offset) >> 8 ) & BYTE,     \
                                IDT_SELECTOR & BYTE,                    \
                                ( IDT_SELECTOR >> 8 ) & BYTE,           \
                                0,                                      \
                                0x0e | 0x80,                            \
                                ( (unsigned)(offset) >> 16 ) & BYTE,    \
                                ( (unsigned)(offset) >> 24 ) & BYTE



void init_idt_entry( int x, unsigned char b1,
                            unsigned char b2,
                            unsigned char b3,
                            unsigned char b4,
                            unsigned char b5,
                            unsigned char b6,
                            unsigned char b7,
                            unsigned char b8 );
void init_idt( void );



/*=====>>> ktrap.c <<<======================================================*/

#define KERNEL_TRAP_INTR	111

void service_kernel_trap( int op, int p1, int p2, int p3, int p4,
			  volatile int r1, volatile int r2,
			  volatile int r3, volatile int r4 );



/*=====>>> inout.c <<<======================================================*/


unsigned char inportb (unsigned short port);
void outportb( unsigned short port, unsigned char value );



/*=====>>> paging.c <<<=====================================================*/



#define INVALIDATE_PAGE_CACHE	asm( "pushl %eax;movl %cr3,%eax; \
				      movl %eax,%cr3;popl %eax" )

#define ONE_MB				((unsigned) 0x100000)

/* Size of a 80386 memory page in bytes */
#define PAGE_SIZE			4096

/* Number of page references within a 4kB page */
#define NUM_PAGE_ENTRIES		1024


/* Base of system stack in linear address space */
#define SYSTEM_STACK_BASE		( 128 * ONE_MB )

/*
 * The following define denotes a page that will be used for
 * the stack of the first process. It is just placed below the
 * second MB. This means that the system requires at least
 * 2MB of RAM.
 */
#define FIRST_SYS_STACK_PAGE	( 2 * ONE_MB - 4096 )

/* The base of the HERMES text segment is at 1 MB */
#define HERMES_TEXT_BASE		ONE_MB

#define VM_BEGIN			0
#define VM_END				0xffffffff

#define RECEIVE_BUFFER_SIZE		( 12 * 1024 )

#define VM_OS_BEGIN			VM_BEGIN
#define VM_USER_BEGIN			( VM_BEGIN + 132 * ONE_MB )
#define VM_USER_TEXT_BEGIN		( VM_USER_BEGIN + RECEIVE_BUFFER_SIZE )
#define VM_SMALL_USER_END		( VM_USER_BEGIN + 4 * ONE_MB )
#define VM_LARGE_USER_END		VM_END

#define RECEIVE_BUFFER_SIZE		( 12 * 1024 )

#define HERMES_DATA_PTR( i )	(void *) ( (i) - HERMES_TEXT_BASE )
#define LIN_ADDR_DATA_PTR( i )	( (PHY_ADDR) (i) + HERMES_TEXT_BASE )


typedef unsigned PHY_ADDR;
typedef unsigned VIR_ADDR;


PHY_ADDR get_pdt( VIR_ADDR stack_lin_addr );
PHY_ADDR get_memory_page( void );
PHY_ADDR vir_to_phy_addr( PHY_ADDR pdt, VIR_ADDR v_addr );
void map_phy_addr( PHY_ADDR pdt, PHY_ADDR p_addr, VIR_ADDR v_addr );
void mark_not_present( PHY_ADDR pdt, VIR_ADDR v_addr );
int page_present( PHY_ADDR pdt, VIR_ADDR v_addr );
void free_memory_page( PHY_ADDR addr );
void release_virtual_memory( PHY_ADDR pdt, VIR_ADDR kernel_stack );
void init_paging( void );


/*=====>>> process.c <<<====================================================*/

#define NO_HANDLE	0


#define VALIDATE_PID( p )						\
			( ( ( p >> 16 ) & WORD ) == my_node_id &&	\
			( p & BYTE ) < MAX_PROCS && 			\
			pcb[ p & BYTE ].used && 			\
			( pcb[ p & BYTE ].seq == ( ( p >> 8 ) & BYTE ) ) )


typedef int		PID;
typedef int		PORT_ID;
typedef unsigned	HANDLE;


typedef struct _PCB {
	unsigned		used : 1;
	unsigned		panic : 1;
	unsigned		system : 1;
	unsigned		to_be_debugged : 1;
	unsigned		single_step : 1;
	unsigned		debugger : 1;
	unsigned short		seq;
	unsigned short		priority;
	unsigned short		state;
	unsigned short		context_ss;
	VIR_ADDR		context_esp;
	PHY_ADDR		context_cr3;
	VIR_ADDR		sys_stack_base;
	VIR_ADDR		sbrk_ptr;
	PID 			param_pid;
	VIR_ADDR		param_copy_buffer;
	int 			param_data_len;
	int 			param_reply_data_len;
	struct _PORT		*port_list;
	struct _PORT		*port_scan;
	struct _PCB 		*next_blocked;
	struct _PCB 		*last_blocked;
	struct _PCB 		*next;
	struct _PCB 		*last;
	char			*name;
	/*
	 * The following entries are needed for the pager.
	 */
	int			fs_handle;
	int			text_size;
	int			data_size;
	int			bss_size;
	int			debug_size;
	} PCB;



extern PCB *active_pcb;
extern PCB *next_free_pcb;
extern PCB pcb[];


PID conv_pcb_to_pid( PCB *p );
PCB *conv_pid_to_pcb( PID pid );
ERROR setup_new_process( void (*new_proc) (PORT_ID,HANDLE),
						 int prio,
						 char *proc_name,
						 PID *new_pid,
						 PORT_ID *first_port,
						 HANDLE handle );
ERROR create( void (*new_proc) (PORT_ID,HANDLE),
			  int prio,
			  char *proc_name,
			  PID *new_pid,
			  PORT_ID *first_port,
			  HANDLE handle );
void init_kernel( void );


/*=====>>> dispatch.c <<<===================================================*/

PCB *dispatcher( void );
void add_ready_queue( PCB *p );
void remove_ready_queue( PCB *p );
void resign( void );
void init_dispatcher( PCB *p );


/*=====>>> sema.c <<<=======================================================*/

typedef struct {
	int 	count;
	PCB 	*first;
	PCB 	*last;
	} SEMA;

void init_sema( SEMA *sema, int count );
void P( SEMA *sema );
void V( SEMA *sema );



/*=====>>> event.c <<<======================================================*/


#define MAGIC_EVENT_COOKIE	0x34ca5802


typedef struct {
	int cookie;
	int queue_empty;
	int *end_of_event_queue;
	int num_ints_per_slot;
	int *head;
	int *tail;
	int *queue;
	} EVENT_BUFFER;



void init_event_queue( EVENT_BUFFER *e, int num_event_slots,
					int num_ints_per_slot,
					int *queue );
void queue_event( EVENT_BUFFER *e, ... );
void queue_event_with_overwrite( EVENT_BUFFER *e, ... );
void dequeue_event( EVENT_BUFFER *e, ... );
void raise_event( PORT_ID port_id );


/*=====>>> port.c <<<=======================================================*/

#define MAX_PORTS		( MAX_PROCS * 2 )

#define NO_PORT 		-1

#define VALIDATE_PORT_ID( i )						\
			( ( ( i >> 16 ) & WORD ) == my_node_id &&	\
			( i & BYTE ) < MAX_PORTS && 			\
			port[ i & BYTE ].used &&			\
			( port[ i & BYTE ].seq == ( ( i >> 8 ) & BYTE ) ) )

typedef struct _PORT {
	unsigned char	seq;		/* Sequence counter for this slot */
	unsigned	used : 1;	/* Port slot used? */
	unsigned	open : 1;	/* Port open? */
	PCB 		*pcb_index; 	/* Owner of this slot */
	PCB 		*blocked_first; /* First local blocked process */
	PCB 		*blocked_last;	/* Last local blocked process */
	EVENT_BUFFER	*event_queue;	/* Event queue */
	struct _PORT	*next;		/* Next port */
	} PORT;


extern PORT port[];

ERROR create_port( PORT_ID *_p );
ERROR create_new_port( PCB *_pcb, PORT_ID *_p );
ERROR destroy_port( PORT_ID port_id );
ERROR release_port( PORT *_port );
ERROR open_port( PORT_ID port_id );
ERROR close_port( PORT_ID port_id );
ERROR connect_port_to_event_queue( PORT_ID port_id, EVENT_BUFFER *e );
void init_ports( void );


/*=====>>> suspend.c <<<====================================================*/

void k_suspend( void );
ERROR k_wakeup( PID pid );


/*=====>>> termin.c <<<=====================================================*/

ERROR k_terminate( PID pid );
void k_suicide( void );


/*=====>>> send.c <<<=======================================================*/

ERROR k_send( PORT_ID port_id, VIR_ADDR a, int data_len, int *reply_data_len );


/*=====>>> receive.c <<<====================================================*/

/* KERNEL_PID is used for event messages received on that port */
#define KERNEL_PID	0xff00


ERROR k_receive( VIR_ADDR a, PID *pid, int *data_len );


/*=====>>> testrcv.c <<<====================================================*/

BOOL k_message_pending( void );


/*=====>>> message.c <<<====================================================*/


ERROR k_message( PORT_ID port_id, VIR_ADDR a, int data_len );


/*=====>>> reply.c <<<======================================================*/


ERROR k_reply( PID pid, VIR_ADDR a, int reply_data_len );


/*=====>>> kmemcpy.c <<<====================================================*/


ERROR k_memcpy( PHY_ADDR to_pdt, VIR_ADDR to_adr,
				PHY_ADDR from_pdt, VIR_ADDR from_adr, int nbytes );


/*=====>>> copy.c <<<=======================================================*/


void move_copy_buffer( PHY_ADDR from_pdt, VIR_ADDR from_addr,
		       PHY_ADDR to_pdt, VIR_ADDR to_addr, int nbytes );


/*=====>>> intr.c <<<=======================================================*/

#define MAX_INTERRUPTS	256

ERROR register_isr( unsigned char intr_no );
void await_event( void );
void init_interrupts( void );



/*=====>>> pfault.c <<<=====================================================*/

void init_page_fault( void );



/*=====>>> params.c <<<=====================================================*/


#define NUM_GLOBAL_PARAMS	11
#define GLOBAL_PARAM_BASE	0x100



/*
 * The following constants define indices into the global_param[]
 * array. Each index stores exactly one integer.
 */

/* Defines the physical base address of the screen */
#define GLOBAL_PARAM_SCREEN_BASE	0

/* Size of main memory in kBytes */
#define GLOBAL_PARAM_LO_MEM 		1

/* Size of "above" memory in kBytes */
#define GLOBAL_PARAM_HI_MEM 		2

/* Size of the HERMES-code segment in Bytes */
#define GLOBAL_PARAM_OS_TEXT		3

/* Size of the HERMES-data segment in Bytes */
#define GLOBAL_PARAM_OS_DATA		4

/* Size of the HERMES-bss segment in Bytes */
#define GLOBAL_PARAM_OS_BSS 		5

/* Size of the HERMES-debug segment in Bytes */
#define GLOBAL_PARAM_OS_DEBUG		6

/* Number of pages reserved for FS cache */
#define GLOBAL_PARAM_FS_CACHE		7

/* Screen mode */
#define GLOBAL_PARAM_SCREEN_MODE	8

/* Video type */
#define GLOBAL_PARAM_VIDEO_TYPE		9

/* Number of seconds since 1970 */
#define GLOBAL_PARAM_NUM_SECS		10


unsigned get_global_param( int param_nr );
void print_global_params( void );
void init_global_params( void );


/*=====>>> debug.c <<<======================================================*/

void k_hook_debugger( PORT_ID debugger_port );
void k_debugger_exited( void );
void k_define_prg_to_debug( PID pid );
void k_start_single_step( void );
void k_single_step( VIR_ADDR addr );
VIR_ADDR k_wait_for_single_step( void );
VIR_ADDR k_next_single_step( void );
void k_resume_single_step( void );


/*=====>>> version.c <<<====================================================*/

void print_version( void );



/*=====>>> panic.c <<<======================================================*/

void shutdown( char *message, ... );
int panic( int err_no, char *message );



/*=====>>> boot.c <<<=======================================================*/

#define PRIO_WINDOW_MANAGER 	20
#define PRIO_TIMER_MANAGER	20
#define PRIO_EVENT_MANAGER	20
#define PRIO_TIMER_NOTIFY	31
#define PRIO_KEYB_NOTIFY	31
/* #define PRIO_MOUSE_NOTIFY	30 */
#define PRIO_FILE_SERVER	3
#define PRIO_FLOPPY_SERVER	20
#define PRIO_DEBUG_PROCESS	10
#define PRIO_VULTURE		30


extern PORT_ID window_port;
extern PORT_ID timer_port;
extern PORT_ID event_port;
extern PORT_ID file_server_port;
extern PORT_ID floppy_port;


void boot( void );


/*=====>>> sbrk.c <<<=======================================================*/

extern VIR_ADDR k_sbrk( int nbytes );


/*=====>>> printf.c <<<=====================================================*/

#ifdef __HERMES__
extern void vsprintf( char *buf, const char *fmt, va_list argp );
#endif


/*=====>>> basicio.c <<<=====================================================*/

extern void output_string( const char *str );


#endif
