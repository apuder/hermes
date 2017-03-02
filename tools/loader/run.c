/*
 * run.c
 *---------------------------------------------------------
 * Run ELF    executable.                          AP  3/01
 */

#pragma inline


#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <time.h>

#include "elf.h"
#include "run.h"

#define WL_NOT_SUPPORTED	0xfe
#define ENABLE_A20		0xdf
#define DISABLE_A20		0xdd


unsigned long global_param[ NUM_GLOBAL_PARAMS ];



void get_system_params( void )
{
	struct REGPACK		regs;
	struct text_info	ti;
	int			i = 0;

	/* Absolute base of screen (0) */
	regs.r_ax = 15 << 8;
	intr( 0x10, &regs );
	if( ( regs.r_ax & 0xff ) != 7 )
		global_param[ i ] = 0xb8000;
	else
		global_param[ i ] = 0xb0000;
	i++;

	/* Size of main memory in kB (1) */
	global_param[ i++ ] = 640;

	/* Size of above memory in kB (2) */
	regs.r_ax = 136 * 256;
	intr( 0x15, &regs );
	global_param[ i++ ] = regs.r_ax;

	/* Size of code segment (3) */
	global_param[ i++ ] = phdr_text.memsize;

	/* Size of data segment (4) */
	global_param[ i++ ] = phdr_data.filesize;

	/* Size of bss segment (5) */
	global_param[ i++ ] = phdr_data.memsize - phdr_data.filesize;

	/* Size of debug section (6) */
	global_param[ i++ ] = 0;

	/* Size of FS-cache (7) */
	global_param[ i++ ] = 0;

	/* Screen mode (8) */
	gettextinfo( &ti );
	global_param[ i++ ] = ti.currmode;

	/* Video type (9) */
	regs.r_ax = 0x1a00;
	intr( 0x10, &regs );
	if( ( regs.r_ax & 0x00ff ) != 0x1a ) {
		global_param[ i ] = WL_NOT_SUPPORTED;
	} else {
		global_param[ i ] = regs.r_bx & 0x00ff;
	}
	i++;

	/* Get seconds since 1970. Add one sec. because of delay during boot */
	global_param[ i++ ] = time( NULL ) + 1;
}



void copy_system_params( void )
{
	unsigned long	*p;
	int				i;

	p = MK_FP( 0, GLOBAL_PARAM_BASE );
	for( i = 0; i < NUM_GLOBAL_PARAMS; i++ )
		*p++ = global_param[ i ];
}



void clear_segment_descr( int i )
{
	char	*ptr;
	int		x;

	ptr = (char *) GDT_BASE;
	ptr += i * SIZE_OF_GDT_ENTRY;
	for( x = 0; x < SIZE_OF_GDT_ENTRY; x++ ) *ptr++ = '\0';
}



void define_segment_base( int i, unsigned long base )
{
	char *ptr;

	ptr = (char *) GDT_BASE;
	ptr += i * SIZE_OF_GDT_ENTRY;
	*( ptr + 2 ) = base & BYTE;
	*( ptr + 3 ) = ( base >> 8 ) & BYTE;
	*( ptr + 4 ) = ( base >> 16 ) & BYTE;
	*( ptr + 7 ) = ( base >> 24 ) & BYTE;
}



void define_segment_limit( int i, unsigned long limit )
{
	char *ptr;

	ptr = (char *) GDT_BASE;
	ptr += i * SIZE_OF_GDT_ENTRY;
	*( ptr + 0 ) = limit & BYTE;
	*( ptr + 1 ) = ( limit >> 8 ) & BYTE;
	*( ptr + 6 ) = ( limit >> 16 ) & 0x0f;
}



void define_segment_attr( int i, unsigned int type )
{
	char *ptr;
	unsigned int	t;

	t = 0;
	t |= 0x8000;		/* Granularity = 1 */
	t |= 0x4000;		/* D = 1 */
	t |= 0x0080;		/* Present bit = 1 */
	t |= 0x0010;		/* Memory segment */
	t |= type & 0x0f;
	ptr = (char *) GDT_BASE;
	ptr += i * SIZE_OF_GDT_ENTRY;
	*( ptr + 5 ) = t & BYTE;
	*( ptr + 6 ) |= ( t >> 8 ) & BYTE;
}



void is_8042_empty( void )
{
	asm		xor	cx,cx

	empty_loop:
	asm		in	al,64h
	asm		and	al,2
	asm		loopnz	empty_loop
}


void test_for_ps2( void )
{
	asm		push	ax
	asm		push	bx
	asm		push	es
	asm		mov	ah,0c0h
	asm		int	15h
	asm		jc	ps2_end
	asm		cmp	byte ptr es:[bx+2],0f8h
	asm		stc
	asm		jnz	ps2_end
	asm		clc

	ps2_end:
	asm		pop	es
	asm		pop	bx
	asm		pop	ax
}



void enable_a20( char enable_flag )
{
	asm		call	is_8042_empty
	asm		jnz	enable_a20_end
	asm		mov	ah,byte ptr enable_flag
	asm		mov	al,0d1h
	asm		out	64h,al
	asm		call	test_for_ps2
	asm		jc	no_mask
	asm		and	ah,4fh

	no_mask:
	asm		call	is_8042_empty
	asm		jnz	enable_a20_end
	asm		mov	al,ah
	asm		out	60h,al
	asm		call	is_8042_empty

	enable_a20_end:
	;
}



void special_bcopy( long dest, char *source_ptr, int source_size )
{
	char	gdt[ 8 * 6 ];
	char	*gdt_ptr = gdt;
	long	source;
	int	x, y;

	for( x = 0; x < 8 * 2; x++ ) gdt[ x ] = 0;

	/* Source GDT descriptor */
	gdt[ x++ ] = 0xff;
	gdt[ x++ ] = 0xff;
	source = (long) FP_SEG( source_ptr ) * 16 + (long) FP_OFF( source_ptr );
	gdt[ x++ ] = source & 0xff;
	gdt[ x++ ] = ( source >> 8 ) & 0xff;
	gdt[ x++ ] = ( source >> 16 ) & 0xff;
	gdt[ x++ ] = 0x92;
	gdt[ x++ ] = 0;
	gdt[ x++ ] = 0;

	/* Destination GDT descriptor */
	gdt[ x++ ] = 0xff;
	gdt[ x++ ] = 0xff;
	gdt[ x++ ] = dest & 0xff;
	gdt[ x++ ] = ( dest >> 8 ) & 0xff;
	gdt[ x++ ] = ( dest >> 16 ) & 0xff;
	gdt[ x++ ] = 0x92;
	gdt[ x++ ] = 0;
	gdt[ x++ ] = 0;

	for( y = x; y < 6 * 8; y++ ) gdt[ y ] = 0;

	source_size >>= 1;
	source_size++;

	asm	push	cx
	asm	push	si
	asm	push	es
	asm	mov	ah,135
	asm	mov	cx,word ptr [source_size]
	asm	les	si,dword ptr [gdt_ptr]
	asm	int	15h
	asm	pop	es
	asm	pop	si
	asm	pop	cx
}



void run_elf_executable( void )
{
	long	dest;
	char	gdt_ptr[ 6 ];
	int	*gdt_length;
	long	*gdt_pos;
	long	size;
	long	x;
	long	i;

	get_system_params();
	dest = TEXT_SEGMENT_BASE + phdr_text.virtaddr;

	x = phdr_text.filesize;
	i = 0;
	while( x != 0 ) {
		size = ( x > MAX_MALLOC ) ? MAX_MALLOC : x;
		special_bcopy( dest, text_seg[ i++ ], size );
		dest += size;
		x -= size;
	}

	dest = TEXT_SEGMENT_BASE + phdr_data.virtaddr;
	x = phdr_data.filesize;
	i = 0;
	while( x != 0 ) {
		size = ( x > MAX_MALLOC ) ? MAX_MALLOC : x;
		special_bcopy( dest, data_seg[ i++ ], size );
		dest += size;
		x -= size;
	}

	enable_a20( ENABLE_A20 );

	asm	.386p
	asm	cli

	copy_system_params();

	/* Null Segment Descriptor */
	clear_segment_descr( 0 );

	/* OS text descriptor */
	clear_segment_descr( 1 );
	define_segment_base( 1, TEXT_SEGMENT_BASE );
	define_segment_limit( 1, 0x00ffffff );
	define_segment_attr( 1, ATTR_CODE );

	/* OS data descriptor */
	clear_segment_descr( 2 );
	define_segment_base( 2, /* text_size + */ TEXT_SEGMENT_BASE );
	define_segment_limit( 2, 0x00ffffff );
	define_segment_attr( 2, ATTR_DATA );

	/* Global data descriptor (spans whole memory) */
	clear_segment_descr( 3 );
	define_segment_base( 3, 0 );
	define_segment_limit( 3, 0x00ffffff );
	define_segment_attr( 3, ATTR_DATA );

	gdt_length = (int *) &gdt_ptr[ 0 ];
	gdt_pos    = (long *) &gdt_ptr[ 2 ];

	*gdt_length = 4 * 8 - 1;
	*gdt_pos    = GDT_BASE;

	asm	lgdt	pword ptr gdt_ptr

	/* Enable protection */
	asm	smsw	ax
	asm	or	ax,1
	asm	lmsw	ax

	/* Now we're in 16 bit p-mode. Do jump to clear pre-fetch queue */
	asm	jmp	flush
	flush:

	/* Opcode for JMP FAR */
	asm	db	066h,0eah
	/* Offset to HERMES-bootstrap */
	asm	dw 0,0
	/* Code selector in GDT */
	asm	dw	8

	for( ; ; ) ;
}



