
#include <kernel.h>
#include <fs.h>
#include <assert.h>
#include "io.h"

PRIVATE SEMA pfault_mutex;

typedef struct _ELFHeader {
    TOS_Octet	magic [4];
    TOS_Octet	class;
    TOS_Octet	byteorder;
    TOS_Octet	hversion;
    TOS_Octet	pad [9];
    TOS_UInt16	filetype;
    TOS_UInt16	archtype;
    TOS_UInt32	fversion;
    TOS_UInt32	entry;
    TOS_UInt32	phdrpos;
    TOS_UInt32	shdrpos;
    TOS_UInt32	flags;
    TOS_UInt16	hdrsize;
    TOS_UInt16	phdrent;
    TOS_UInt16	phdrcnt;
    TOS_UInt16	shdrent;
    TOS_UInt16	shdrcnt;
    TOS_UInt16	strsec;
} ELFHeader;

typedef struct _ELFPrgHeader {
    TOS_UInt32	type;
    TOS_UInt32	offset;
    TOS_UInt32	virtaddr;
    TOS_UInt32	physaddr;
    TOS_UInt32	filesize;
    TOS_UInt32	memsize;
    TOS_UInt32	flags;
    TOS_UInt32	align;
} ELFPrgHeader;


void check_elf_header (ELFHeader* hdr)
{
    assert (hdr->magic[0] == 0x7f);
    assert (hdr->magic[1] == 'E');
    assert (hdr->magic[2] == 'L');
    assert (hdr->magic[3] == 'F');
    
    /* Check address size == 32 bit */
    assert (hdr->class == 1);
    
    /* Check byteorder == little endian */
    assert (hdr->byteorder == 1);
    
    /* Check header version == 1 */
    assert (hdr->hversion == 1);
    
    /* Check filetype == executable */
    assert (hdr->filetype == 2);
    
    /* Check architecture type == x86 */
    assert (hdr->archtype == 3);
    
    /* Check file version == 1 */
    assert (hdr->fversion == 1);

    /* Check entry point = 0 */
    assert (hdr->entry == 0);
    
    /* Check entries in program header == 1 */
    assert (hdr->phdrcnt == 1);
}


void check_elf_program_header (ELFPrgHeader* hdr)
{
    // Make sure that program segment starts at file offset 0x80
    assert (hdr->offset == 0x80);

    // Make sure binary was linked with -Ttext 0
    assert (hdr->virtaddr == 0);
}


void load_first_page( VIR_ADDR a )
{
    VIR_ADDR		cb = VM_USER_BEGIN;
    PHY_ADDR		page;
    FileReadMsg		*msg;
    FileReadReply	*r;
    FileSeekMsg         *seek_msg;
    ELFHeader           *h;
    ELFPrgHeader        *ph;
    int			total_size;
    int			nbytes;
    int			i;

    page = get_memory_page();
    map_phy_addr( active_pcb->context_cr3, page, a & ~0xfff );

    msg = HERMES_DATA_PTR( cb );
    seek_msg = HERMES_DATA_PTR( cb );
    r   = HERMES_DATA_PTR( cb );
    h   = HERMES_DATA_PTR( VM_USER_TEXT_BEGIN );
    ph  = HERMES_DATA_PTR( VM_USER_TEXT_BEGIN );

    // Read ELF Header
    msg->Size = sizeof( FileReadMsg );
    msg->Type = FILE_READ;
    msg->Handle = active_pcb->fs_handle;
    msg->NumberOfBytes = sizeof (ELFHeader);
    msg->Destination = page;
    k_send( file_server_port, cb, sizeof( FileReadMsg ), &nbytes );
    assert( r->Result == TOS_NO_ERROR );
    assert( r->NumOfBytes == sizeof (ELFHeader) );
    check_elf_header(h);

    // Read the one ELF program header
    seek_msg->Size = sizeof (FileSeekMsg);
    seek_msg->Type = FILE_SEEK;
    seek_msg->Handle = active_pcb->fs_handle;
    seek_msg->NumOfBytes = h->phdrpos;
    k_send (file_server_port, cb, sizeof (FileSeekMsg), &nbytes);
    assert (r->Result == TOS_NO_ERROR);
    
    msg->Size = sizeof( FileReadMsg );
    msg->Type = FILE_READ;
    msg->Handle = active_pcb->fs_handle;
    msg->NumberOfBytes = sizeof (ELFPrgHeader);
    msg->Destination = page;
    k_send( file_server_port, cb, sizeof( FileReadMsg ), &nbytes );
    assert( r->Result == TOS_NO_ERROR );
    assert( r->NumOfBytes == sizeof (ELFPrgHeader) );
    check_elf_program_header (ph);

    active_pcb->text_size = ph->filesize;
    active_pcb->data_size = 0;  // .data is part of .text
    active_pcb->bss_size = ph->memsize - ph->filesize;
    active_pcb->debug_size = 0;
    total_size = ph->memsize;
    active_pcb->sbrk_ptr = total_size + VM_USER_TEXT_BEGIN + PAGE_SIZE;
    total_size = ph->filesize;

    // Read first program page
    // Read the one ELF program header
    seek_msg->Size = sizeof (FileSeekMsg);
    seek_msg->Type = FILE_SEEK;
    seek_msg->Handle = active_pcb->fs_handle;
    seek_msg->NumOfBytes = 0x80;
    k_send (file_server_port, cb, sizeof (FileSeekMsg), &nbytes);
    assert (r->Result == TOS_NO_ERROR);
    
    msg->Size = sizeof( FileReadMsg );
    msg->Type = FILE_READ;
    msg->Handle = active_pcb->fs_handle;
    msg->NumberOfBytes = PAGE_SIZE;
    msg->Destination = page;
    k_send( file_server_port, cb, sizeof( FileReadMsg ), &nbytes );
    assert( r->Result == TOS_NO_ERROR );

    /*
     * Now we have to check if text + data is less than one page
     * (ie. we already loaded the complete program). In this case
     * the bss segment has to be zeroed!
     */
    if( total_size < PAGE_SIZE ) {
	page = vir_to_phy_addr( active_pcb->context_cr3, VM_USER_TEXT_BEGIN );
	for( i = total_size; i < PAGE_SIZE; i++ )
	    poke_mem_b( page + i, 0 );
    }
}



void load_program( VIR_ADDR a )
{
    VIR_ADDR		heap_area;
    int			total_size;
    VIR_ADDR		page;
    VIR_ADDR		cb = VM_USER_BEGIN;
    FileSeekMsg		*seek_msg;
    FileSeekReply	*seek_reply;
    FileReadMsg		*read_msg;
    FileReadReply	*r;
    int			nbytes;
    int			i;

    if( active_pcb->text_size == 0 ) {
	/*
	 * No page of this process has been loaded yet.
	 */
	load_first_page(a);
	return;
    }
    heap_area = active_pcb->text_size +
	active_pcb->data_size +
	VM_USER_TEXT_BEGIN;
    if( heap_area % PAGE_SIZE != 0 ) {
	heap_area /= PAGE_SIZE;
	heap_area = ( heap_area + 1 ) * PAGE_SIZE;
    }

    page = get_memory_page();
    map_phy_addr( active_pcb->context_cr3, page, a & ~0xfff );

    if( a >= heap_area ) {
	/*
	 * Page fault occured above the programs text and data segment
	 * (ie. stack or heap). We are done by simply allocating a 
	 * new page
	 */
	return;
    }
    seek_msg = HERMES_DATA_PTR( cb );
    seek_msg->Size = sizeof( FileSeekMsg );
    seek_msg->Type = FILE_SEEK;
    seek_msg->Handle = active_pcb->fs_handle;
    seek_msg->NumOfBytes = (( a - VM_USER_TEXT_BEGIN ) & ~0xfff) + 0x80;
    k_send( file_server_port, cb, sizeof( FileSeekMsg ), &nbytes );
    assert( nbytes == sizeof( FileSeekReply ) );
    seek_reply = HERMES_DATA_PTR( cb );
    if( seek_reply->Result != TOS_NO_ERROR ) {
	printf( "seek reply: %d\n", seek_reply->Result );
	printf( "Handle = %d\n", active_pcb->fs_handle );
    }
    assert( seek_reply->Result == TOS_NO_ERROR );
    read_msg = HERMES_DATA_PTR( cb );
    r   = HERMES_DATA_PTR( cb );
    read_msg->Size = sizeof( FileReadMsg );
    read_msg->Type = FILE_READ;
    read_msg->Handle = active_pcb->fs_handle;
    read_msg->NumberOfBytes = PAGE_SIZE; /*!!!*/
    read_msg->Destination = page;//(void *) ( ( a & ~0xfff ) - HERMES_TEXT_BASE);
    k_send( file_server_port, cb, sizeof( FileReadMsg ), &nbytes );
    assert( r->Result == TOS_NO_ERROR || r->Result == TOS_ERR_BEYOND_EOF );
    /*
     * Check if we loaded the last code/data page. If so, we might
     * have to clear the bss segment.
     */
    return;
    //XXX
    if( a < heap_area - PAGE_SIZE ) return;
    total_size = active_pcb->text_size + active_pcb->data_size;
    total_size %= PAGE_SIZE;
    if( total_size == 0 ) return;
    page = vir_to_phy_addr( active_pcb->context_cr3, a & ~0xfff );
    for( i = total_size; i < PAGE_SIZE; i++ )
	poke_mem_b( page + i, 0 );
}



void service_page_fault( VIR_ADDR a, int err )
{
    PHY_ADDR page;

    /*
      P( &pfault_mutex );
    */
    assert( ( err & 1 ) == 0 );
    if( a < VM_USER_BEGIN ) printf( "%s\n", active_pcb->name );
    assert( a >= VM_USER_BEGIN );
    if( active_pcb->system ) {
	/*
	 * System process generated a page fault. Always allocate
	 * a new empty page.
	 */
	page = get_memory_page();
	map_phy_addr( active_pcb->context_cr3, page, a & ~0xfff );
    } else if( a < VM_USER_TEXT_BEGIN ) {
	/*
	 * Page fault within the system receive buffer.
	 */
	page = get_memory_page();
	map_phy_addr( active_pcb->context_cr3, page, a & ~0xfff );
    } else {
	/*
	  printf( "\nPage fault at 0x%x for PCB %x (err=0x%x)\n",
	  a,
	  conv_pcb_to_pid( active_pcb ),
	  err );
	*/
	load_program( a );
    }
    /*
      V( &pfault_mutex );
    */
}



void init_page_fault( void )
{
    init_sema( &pfault_mutex, 1 );
}
