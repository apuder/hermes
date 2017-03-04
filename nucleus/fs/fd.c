
#include <kernel.h>
#include <timer.h>
#include <assert.h>
#include "fd.h"
//#include "fsdefs.h"



int pc_at = TRUE;
int ps = TRUE;



/* Variables. */
PRIVATE struct floppy { 	/* main drive struct, one entry per drive */
    int fl_opcode;		/* FDC_READ, FDC_WRITE or FDC_FORMAT */
    int fl_curcyl;		/* current cylinder */
    int fl_drive; 		/* drive number addressed */
    int fl_cylinder;		/* cylinder number addressed */
    int fl_sector;		/* sector addressed */
    int fl_head;		/* head number addressed */
    int fl_count; 		/* byte count */
    PHY_ADDR fl_address;	/* user virtual address */
    char fl_results[MAX_RESULTS]; /* the controller can give lots of output */
    char fl_calibration;	/* CALIBRATED or UNCALIBRATED */
    char fl_density;		/* 0 = 360K/360K; 1 = 360K/1.2M; etc. */
    char fl_last_density;
    char fl_auto_type;		/* nonzero to allow search for working type */
    /*	struct disk_parameter_s fl_param;*/ /* parameters for format */
} floppy[NR_DRIVES];

#define UNCALIBRATED	   0	/* drive needs to be calibrated at next use */
#define CALIBRATED	   1	/* no calibration needed */

PRIVATE int motor_status;	/* current motor status is in 4 high bits */
PRIVATE int motor_goal; 	/* desired motor status is in 4 high bits */
PRIVATE int prev_motor; 	/* which motor was started last */
PRIVATE int need_reset; 	/* set to 1 when controller must be reset */
PRIVATE int d;			/* diskette/drive combination */
PRIVATE int current_spec1;	/* latest spec1 sent to the controller */

PRIVATE VIR_ADDR	floppy_copy_buffer;
PRIVATE ToDriverMsg	request;


PRIVATE char len[] = {-1,0,1,-1,2,-1,-1,3,-1,-1,-1,-1,-1,-1,-1,4};
PRIVATE char base_sector = 1;	/* physical sectors form range starting here */

/* Seven combinations of diskette/drive are supported.
 *
 * # Drive  disk Sectrs Trcks Rot       Data-rate  Comment
 * 0  360K  360K    9	  40  300 RPM	250 kbps   Standard PC DSDD
 * 1  1.2M  1.2M   15	  80  360 RPM	500 kbps   AT disk in AT drive
 * 2  720K  360K    9	  40  300 RPM	250 kbps   Quad density PC
 * 3  720K  720K    9	  80  300 RPM	250 kbps   Toshiba, et al.
 * 4  1.2M  360K    9	  40  360 RPM	300 kbps   PC disk in AT drive
 * 5  1.2M  720K    9	  80  360 RPM	300 kbps   Toshiba in AT drive
 * 6  1.44M 1.44M  18	  80  300 RPM	500 kbps   PS/2, et al.
 *
 * In addition, 720K diskettes can be read in 1.44MB drives, but that does
 * not need a different set of parameters.	This combination uses
 *
 * X  1.44M   720K	9	80	   300 RPM	250 kbps   PS/2, et al.
 */
PRIVATE int gap[NT] =
    {0x2A, 0x1B, 0x2A, 0x2A, 0x23, 0x23, 0x1B}; /* gap size */
PRIVATE int rate[NT] =
    {0x02, 0x00, 0x02, 0x02, 0x01, 0x01, 0x00}; /* 2=250,1=300,0=500 kbps*/
PRIVATE int nr_sectors[NT] =
    {9,    15,	 9,    9,	 9,    9,	 18};	/* sectors/track */
PRIVATE int nr_blocks[NT] =
    {720,  2400, 720,  1440, 720,  1440, 2880}; /* sectors/diskette*/
PRIVATE int steps_per_cyl[NT] =
    {1,    1,	 2,    1,	 2,    1,	  1};	/* 2 = dbl step */
PRIVATE int mtr_setup[NT] =
    {1*HZ/4,3*HZ/4,1*HZ/4,4*HZ/4,3*HZ/4,3*HZ/4,4*HZ/4}; /* in ticks */
PRIVATE char spec1[NT] =
    {0xDF, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF, 0xAF}; /* step rate, etc. */

PRIVATE char drive_class[NR_DRIVES] = {
    AUTOMATIC, AUTOMATIC	/* both drives can handle both types */
};

PRIVATE void clock_mess();
PRIVATE void dma_setup();
PRIVATE int do_rdwt();
PRIVATE void f_reset();
PRIVATE void fdc_out();
PRIVATE int fdc_results();
PRIVATE int recalibrate();
PRIVATE int seek();
PRIVATE void start_motor();
PRIVATE void stop_motor();
PRIVATE int transfer();

void cim_floppy()
{
    unsigned char x;

    asm( "cli" );
    x = inportb( 0xa1 );
    x &= ~0x40;
    outportb( 0xa1, x );
    asm( "sti" );
}



void wait_some_ticks( int count )
{
    TIMER_MSG	*msg;
    int 		data_len;

    msg = HERMES_DATA_PTR( floppy_copy_buffer );
    msg->wakeup_mode = WAKEUP_BY_REPLY;
    msg->count = count;
    k_send( timer_port, floppy_copy_buffer, sizeof( TIMER_MSG ), &data_len );
}


void floppy_task( PORT_ID first_port, PORT_ID parent_port )
{
    int 	ret = 0;
    PID 	pid;
    int 	data_len;
    ToDriverMsg	*msg;
    DriverMsg	*answer_msg;
    FLOPPY_REQ	req;
    u_int		Seq = 0;
    int			i;

    cim_floppy();			/* ready for floppy interrupts */
    register_isr( 0x66 );  /*!!!!!*/
    floppy_copy_buffer = k_sbrk( RECEIVE_BUFFER_SIZE );
    msg = HERMES_DATA_PTR( floppy_copy_buffer );
    answer_msg = HERMES_DATA_PTR( floppy_copy_buffer );

    for( i = 0; i < NR_DRIVES; i++ )
	floppy[ i ].fl_last_density = 0;

    while( TRUE ) {
	/* First wait for a request to read or write a disk block. */
	k_receive( floppy_copy_buffer, &pid, &data_len);	/* get a request to do some work */
	
	assert( data_len == sizeof( ToDriverMsg ) );
	request = *msg;

	/* Ignore any alarm to turn motor off, now there is work to do. */
	motor_goal = motor_status;

	/* Now carry out the work. */
	req.req = ( request.Type == DEVICE_READ ) ? DISK_READ : DISK_WRITE;
	req.device = request.MinorDevice;
	req.position = request.SectorAddr;
	req.address = request.MemoryAddr;
	req.count = SECTOR_SIZE;
	Seq = request.Seq;
	ret = do_rdwt( &req, FALSE );

	/* Start watch_dog timer to turn all motors off in a few seconds. */
	motor_goal = ENABLE_INT;
	clock_mess( MOTOR_OFF, stop_motor );

	/* Finally, prepare and send the reply message. */
	answer_msg->Size   = sizeof( DriverMsg );
	answer_msg->Type   = DRIVER_ACK;
	answer_msg->Seq    = Seq;
	answer_msg->Result = ret;
	k_message( parent_port, floppy_copy_buffer, sizeof( DriverMsg ) ); /* send reply to caller */
    }
}


PRIVATE int do_rdwt(m_ptr, dont_retry)
     FLOPPY_REQ *m_ptr;			/* pointer to read or write message */
     int dont_retry; 		/* nonzero to skip retry after error */
{
    struct floppy *fp;
    int r, sectors, drive, errors;
    unsigned block;
    unsigned dtype;

    /* Decode the message parameters. */
    r = 0;
    drive = m_ptr->device & ~(DEV_TYPE_BITS | FORMAT_DEV_BIT);
    if( drive < 0 || drive >= NR_DRIVES ) return( ERR_PARAM );
    fp = &floppy[ drive ];		/* 'fp' points to entry for this drive */
    fp->fl_drive = drive;		/* save drive number explicitly */
    fp->fl_opcode = ( m_ptr->req == DISK_READ ? FDC_READ : FDC_WRITE );
    assert( !( m_ptr->device & FORMAT_DEV_BIT ) );
    dtype = m_ptr->device & DEV_TYPE_BITS;
    if( dtype != 0 ) {
	dtype = ( dtype >> DEV_TYPE_SHIFT ) - 1;
	if( dtype >= NT ) return( ERR_PARAM );
	fp->fl_density = dtype;
	fp->fl_auto_type = FALSE;
    } else {
	fp->fl_auto_type = TRUE;
	fp->fl_density	 = fp->fl_last_density;
    }

    block = m_ptr->position;
    if( block >= HC_SIZE ) return( 0 ); /* sector is beyond end of all disks */

    d = fp->fl_density; 	/* diskette/drive combination */
    if( fp->fl_auto_type ) {
	/* Check bit map to skip illegal densities. */
	while((drive_class[ drive ] & ( 1 << d ) ) == 0 ) d = ( d + 1 ) % NT;
    }

    /* Store the message parameters in the fp->fl array. */
    d = 6; //XXX !!!  Hardcoded density
    fp->fl_density = d;
    
    /*
      sectors = ( m_ptr->device & FORMAT_DEV_BIT ?
      fp->fl_param.sectors_per_cylinder : nr_sectors[ d ] );
    */
    sectors = nr_sectors[ d ];	/* Erstmal keine Formatierung!!! */

    fp->fl_cylinder = block / ( NR_HEADS * sectors );
    fp->fl_sector = base_sector + block % sectors;
    fp->fl_head = ( block % ( NR_HEADS * sectors ) ) / sectors;
    fp->fl_count = m_ptr->count;
    fp->fl_address = m_ptr->address;
    //XXX if( fp->fl_count != BLOCK_SIZE ) return( ERR_PARAM );

    errors = 0;

    /* This loop allows a failed operation to be repeated.	It is really a
     * nested loop allowing MAX_ERRORS errors for each of NT drive types.
     */
    while( errors <= MAX_ERRORS * NT ) {
	if( ++errors % MAX_ERRORS == 0 ) {

#if 0
	    /* this works well except for programs like mkfs which seek to end 1st*/
	    /* A lot of errors probably means that we are trying the
	     * wrong drive type.  Try another one if the cylinder is <= 1.
	     * Otherwise, assume the disk is unchanged and give up.
	     */
	    if( fp->fl_cylinder > 1 )
		return( block >= nr_blocks[ d ] ? 0 : EIO );
#else
	    /* A lot of errors probably means that we are trying the
	     * wrong drive type.  Try another one.
	     */
#endif
	    if( !fp->fl_auto_type ) return( ERR_PARAM );
	    d++;

	    /* Check bit map to skip illegal densities. */
	    while( ( drive_class[ drive ] & ( 1 << d ) ) == 0 )
		d = ( d + 1 ) % NT;
	    fp->fl_density = d;

	    sectors = nr_sectors[ d ];
	    fp->fl_cylinder = block / ( NR_HEADS * sectors );
	    fp->fl_sector = base_sector + block % sectors;
	    fp->fl_head = ( block % ( NR_HEADS * sectors ) ) / sectors;
	    need_reset = 1;
	}
	if( block >= nr_blocks[ d ] ) continue;

	/* First check to see if a reset is needed. */
	if( need_reset ) f_reset();
	/* Set the stepping rate */
	if( current_spec1 != spec1[ d ] ) {
	    fdc_out( FDC_SPECIFY );
	    fdc_out( current_spec1 = spec1[ d ] );
	    fdc_out( SPEC2 );
	}

	/* Set the data rate */
	if( pc_at ) outportb( FDC_RATE, rate[ d ] );

	/* Now set up the DMA chip. */
	dma_setup( fp );

	/* See if motor is running; if not, turn it on and wait */
	start_motor( fp );

	/* If we are going to a new cylinder, perform a seek. */
	r = seek( fp );

	/* Perform the transfer. */
	if( r == OK ) r = transfer( fp );

	if( r == OK ) break;				/* if successful, exit loop */
	if( dont_retry ) break; 			/* retries not wanted */
	if( r == ERR_WR_PROTECT ) break;	/* retries won't help */
    }
    if( r == OK )
	fp->fl_last_density = fp->fl_density;
    return( r == OK ? OK : ERR_PARAM );
}



PRIVATE void dma_setup(fp)
     struct floppy *fp;		/* pointer to the drive struct */
{
    /* The IBM PC can perform DMA operations by using the DMA chip.  To use it,
     * the DMA (Direct Memory Access) chip is loaded with the 20-bit memory address
     * to be read from or written to, the byte count minus 1, and a read or write
     * opcode.	This routine sets up the DMA chip.	Note that the chip is not
     * capable of doing a DMA across a 64K boundary (e.g., you can't read a
     * 512-byte block starting at physical address 65520).
     */

    int mode, low_addr, high_addr, top_addr, low_ct, high_ct, top_end;
    int ct;
    PHY_ADDR user_phys;

    mode = ( fp->fl_opcode == FDC_READ ? DMA_READ : DMA_WRITE );
    user_phys = fp->fl_address;
    ct = fp->fl_count;
    low_addr  = ( user_phys >>	0 ) & BYTE;
    high_addr = ( user_phys >>	8 ) & BYTE;
    top_addr  = ( user_phys >> 16 ) & BYTE;
    low_ct	= ( ( ct - 1 ) >> 0 ) & BYTE;
    high_ct = ( ( ct - 1 ) >> 8 ) & BYTE;

    /* Check to see if the transfer will require the DMA address counter to
     * go from one 64K segment to another.	If so, do not even start it, since
     * the hardware does not carry from bit 15 to bit 16 of the DMA address.
     * Also check for bad buffer address.  These errors mean FS contains a bug.
     */
    top_end = ( ( user_phys + ct - 1 ) >> 16 ) & BYTE;
    if( top_end != top_addr )
	shutdown( "Trying to DMA across 64K boundary" );

    /* Now set up the DMA registers. */
    outportb( DMA_INIT, DMA_RESET_VAL );	/* reset the dma controller */
    outportb( DMA_M2, mode );				/* set the DMA mode */
    outportb( DMA_M1, mode );				/* set it again */
    outportb( DMA_ADDR, low_addr ); 		/* output low-order 8 bits */
    outportb( DMA_ADDR, high_addr );		/* output next 8 bits */
    outportb( DMA_TOP, top_addr );			/* output highest 4 bits */
    outportb( DMA_COUNT, low_ct );			/* output low 8 bits of count - 1 */
    outportb( DMA_COUNT, high_ct ); 		/* output high 8 bits of count - 1 */
    outportb( DMA_INIT, 2 );				/* initialize DMA */
}



PRIVATE void start_motor(fp)
     struct floppy *fp;		/* pointer to the drive struct */
{
    /* Control of the floppy disk motors is a big pain.  If a motor is off, you
     * have to turn it on first, which takes 1/2 second.  You can't leave it on
     * all the time, since that would wear out the diskette.  However, if you turn
     * the motor off after each operation, the system performance will be awful.
     * The compromise used here is to leave it on for a few seconds after each
     * operation.  If a new operation is started in that interval, it need not be
     * turned on again.  If no new operation is started, a timer goes off and the
     * motor is turned off.  I/O port DOR has bits to control each of 4 drives.
     * The timer cannot go off while we are changing with the bits, since the
     * clock task cannot run while another (this) task is active, so there is no
     * need to lock().
     */

    int motor_bit, running;

    motor_bit = 1 << ( fp->fl_drive + 4 );	/* bit mask for this drive */
    running = motor_status & motor_bit; /* nonzero if this motor is running */
    motor_goal = motor_bit | ENABLE_INT | fp->fl_drive;
    if( motor_status & prev_motor ) motor_goal |= prev_motor;
    outportb( DOR, motor_goal );
    motor_status = motor_goal;
    prev_motor = motor_bit; /* record motor started for next time */

    /* If the motor was already running, we don't have to wait for it. */
    if( running ) return;			/* motor was already running */
    wait_some_ticks( mtr_setup[ d ] );	/* motor was not running */
}



PRIVATE void stop_motor()
{
    /* This routine is called by the clock interrupt after several seconds have
     * elapsed with no floppy disk activity.  It checks to see if any drives are
     * supposed to be turned off, and if so, turns them off.
     */

    if ( ( motor_goal & MOTOR_MASK ) != ( motor_status & MOTOR_MASK ) ) {
	outportb( DOR, motor_goal );
	motor_status = motor_goal;
    }
}



PRIVATE int seek(fp)
     struct floppy *fp;		/* pointer to the drive struct */
{
    /* Issue a SEEK command on the indicated drive unless the arm is already
     * positioned on the correct cylinder.
     */

    int r;

    /* Are we already on the correct cylinder? */
    if( fp->fl_calibration == UNCALIBRATED )
	if( recalibrate( fp ) != OK ) return( ERR_SEEK );
    if( fp->fl_curcyl == fp->fl_cylinder ) return( OK );

    /* No.	Wrong cylinder.  Issue a SEEK and wait for interrupt. */
    fdc_out( FDC_SEEK );		/* start issuing the SEEK command */
    fdc_out( ( fp->fl_head << 2 ) | fp->fl_drive );
    fdc_out( fp->fl_cylinder * steps_per_cyl[ d ] );
    if( need_reset ) return( ERR_SEEK );	/* if controller is sick, abort seek */
    await_event();

    /* Interrupt has been received.  Check drive status. */
    fdc_out( FDC_SENSE );		/* probe FDC to make it return status */
    r = fdc_results( fp );		/* get controller status bytes */
    if( ( fp->fl_results[ ST0 ] & ST0_BITS ) != SEEK_ST0 ) r = ERR_SEEK;
    if( fp->fl_results[ ST1 ] != fp->fl_cylinder * steps_per_cyl[ d ] )
	r = ERR_SEEK;
    if( r != OK )
	if( recalibrate( fp ) != OK ) return( ERR_SEEK );
    fp->fl_curcyl = ( r == OK ? fp->fl_cylinder : -1 );
    if( r == OK && ( ( d == 6 ) || ( d == 3 ) ) ) {
	/* give head time to settle on 3.5 */
	wait_some_ticks( 2 );
    }
    return( r );
}



PRIVATE int transfer(fp)
     struct floppy *fp;	/* pointer to the drive struct */
{
    /* The drive is now on the proper cylinder.  Read, write or format 1 block. */

    int r, s;

    /* Never attempt a transfer if the drive is uncalibrated or motor is off. */
    if( fp->fl_calibration == UNCALIBRATED )
	return( ERR_TRANSFER );
    if( ( ( motor_status >> ( fp->fl_drive+4 ) ) & 1 ) == 0 )
	return( ERR_TRANSFER );

    /* The command is issued by outputting 9 bytes to the controller chip. */
    fdc_out( fp->fl_opcode );	/* issue the read, write or format command */
    fdc_out( ( fp->fl_head << 2 ) | fp->fl_drive );
    if( fp->fl_opcode == FDC_FORMAT ) {
	shutdown( "FS: No format allowed" );
	/*
	  fdc_out( fp->fl_param.sector_size_code );
	  fdc_out( fp->fl_param.sectors_per_cylinder );
	  fdc_out( fp->fl_param.gap_length_for_format );
	  fdc_out( fp->fl_param.fill_byte_for_format );
	*/
    } else {
	fdc_out( fp->fl_cylinder );
	fdc_out( fp->fl_head );
	fdc_out( fp->fl_sector );
	fdc_out( (int) len[ SECTOR_SIZE / DIVISOR ] );	/* sector size code */
	fdc_out( nr_sectors[ d ] );
	fdc_out( gap[ d ] );	/* sector gap */
	fdc_out( DTL ); 	/* data length */
    }

    /* Block, waiting for disk interrupt. */
    if( need_reset ) return( ERR_TRANSFER );
    /* if controller is sick, abort op */
    await_event();

    /* Get controller status and check for errors. */
    r = fdc_results( fp );
    if( r != OK ) return( r );
    if( ( fp->fl_results[ ST1 ] & BAD_SECTOR ) ||
	( fp->fl_results[ ST2 ] & BAD_CYL ) )
	fp->fl_calibration = UNCALIBRATED;
    if( fp->fl_results[ ST1 ] & WRITE_PROTECT ) {
	/*
	  printf( "Diskette in drive %d is write protected.\n", fp->fl_drive );
	*/
	return( ERR_WR_PROTECT );
    }
    if( ( fp->fl_results[ ST0 ] & ST0_BITS) != TRANS_ST0)
	return( ERR_TRANSFER );
    if( fp->fl_results[ ST1 ] | fp->fl_results[ ST2 ] )
	return( ERR_TRANSFER );

    if( fp->fl_opcode == FDC_FORMAT ) return( OK );

    /* Compare actual numbers of sectors transferred with expected number. */
    s = ( fp->fl_results[ ST_CYL ] - fp->fl_cylinder ) *
	NR_HEADS * nr_sectors[ d ];
    s += ( fp->fl_results[ ST_HEAD ] - fp->fl_head ) * nr_sectors[ d ];
    s += ( fp->fl_results[ ST_SEC ] - fp->fl_sector );
    if( s * SECTOR_SIZE != fp->fl_count ) return( ERR_TRANSFER );
    return( OK );
}



PRIVATE int fdc_results(fp)
     struct floppy *fp;		/* pointer to the drive struct */
{
    /* Extract results from the controller after an operation, then allow floppy
     * interrupts again.
     */

    int result_nr;
    int retries;
    int status;

    /* Extract bytes from FDC until it says it has no more.  The loop is
     * really an outer loop on result_nr and an inner loop on status.
     */
    result_nr = 0;
    retries = MAX_FDC_RETRY;
    while( TRUE ) {
	/* Reading one byte is almost a mirror of fdc_out() - the DIRECTION
	 * bit must be set instead of clear, but the CTL_BUSY bit destroys
	 * the perfection of the mirror.
	 */
	status = inportb( FDC_STATUS ) & ( MASTER | DIRECTION | CTL_BUSY );
	if( status == ( MASTER | DIRECTION | CTL_BUSY ) ) {
	    if( result_nr >= MAX_RESULTS ) break;	/* too many results */
	    fp->fl_results[ result_nr++ ] = inportb( FDC_DATA );
	    retries = MAX_FDC_RETRY;
	    continue;
	}
	if( status == MASTER ) {	/* all read */
	    cim_floppy();
	    return( OK );	/* only good exit */
	}
	if( --retries == 0 ) break; /* time out */
    }
    need_reset = TRUE;		/* controller chip must be reset */
    cim_floppy();
    return( ERR_STATUS );
}


PRIVATE void fdc_out( val )
     int val;		/* write this byte to floppy disk controller */
{
    /* Output a byte to the controller.  This is not entirely trivial, since you
     * can only write to it when it is listening, and it decides when to listen.
     * If the controller refuses to listen, the FDC chip is given a hard reset.
     */

    int retries;

    if( need_reset ) return;	/* if controller is not listening, return */

    /* It may take several tries to get the FDC to accept a command. */
    retries = MAX_FDC_RETRY;
    while( ( inportb( FDC_STATUS ) & ( MASTER | DIRECTION ) ) != ( MASTER | 0 ) )
	if( --retries == 0 ) {
	    /* Controller is not listening.  Hit it over the head with a hammer. */
	    need_reset = TRUE;
	    return;
	}
    outportb( FDC_DATA, val );
}



PRIVATE int recalibrate(fp)
     struct floppy *fp;	/* pointer tot he drive struct */
{
    /* The floppy disk controller has no way of determining its absolute arm
     * position (cylinder).  Instead, it steps the arm a cylinder at a time and
     * keeps track of where it thinks it is (in software).	However, after a
     * SEEK, the hardware reads information from the diskette telling where the
     * arm actually is.  If the arm is in the wrong place, a recalibration is done,
     * which forces the arm to cylinder 0.	This way the controller can get back
     * into sync with reality.
     */

    int r;

    /* Issue the RECALIBRATE command and wait for the interrupt. */
    start_motor( fp );		/* can't recalibrate with motor off */
    fdc_out( FDC_RECALIBRATE ); /* tell drive to recalibrate itself */
    fdc_out( fp->fl_drive );	/* specify drive */
    if( need_reset ) return( ERR_SEEK );	/* don't wait if controller is sick */
    await_event();

    /* Determine if the recalibration succeeded. */
    fdc_out( FDC_SENSE );		/* issue SENSE command to request results */
    r = fdc_results( fp );		/* get results of the FDC_RECALIBRATE command */
    fp->fl_curcyl = -1; 	/* force a SEEK next time */
    if( r != OK ||		/* controller would not respond */
	( fp->fl_results[ ST0 ] & ST0_BITS ) != SEEK_ST0 ||
	fp->fl_results[ ST_PCN ] !=0 ) {
	/* Recalibration failed.  FDC must be reset. */
	need_reset = TRUE;
	fp->fl_calibration = UNCALIBRATED;
	return( ERR_RECALIBRATE );
    } else {
	/* Recalibration succeeded. */
	fp->fl_calibration = CALIBRATED;
	if( ps || ( ( d == 6 ) || ( d == 3 ) ) )
	    /* give head time to settle on 3.5 */
	    wait_some_ticks( 2 );
	return( OK );
    }
}



PRIVATE void f_reset()
{
    /* Issue a reset to the controller.  This is done after any catastrophe,
     * like the controller refusing to respond.
     */

    int i;
    volatile int i_flag;

    /* Disable interrupts and strobe reset bit low. */
    need_reset = FALSE;

    /* It is not clear why the next lock is needed.  Writing 0 to DOR causes
     * interrupt, while the PC documentation says turning bit 8 off disables
     * interrupts.	Without the lock:
     *	 1) the interrupt handler sets the floppy mask bit in the 8259.
     *	 2) writing ENABLE_INT to DOR causes the FDC to assert the interrupt
     *		line again, but the mask stops the cpu being interrupted.
     *	 3) the sense interrupt clears the interrupt (not clear which one).
     * and for some reason the reset does not work.
     */
    DISABLE_INTR( i_flag );
    motor_status = 0;
    motor_goal = 0;
    outportb( DOR, 0 ); 	/* strobe reset bit low */
    outportb( DOR, ENABLE_INT );	/* strobe it high again */
    ENABLE_INTR( i_flag );
    await_event();
    fdc_out( FDC_SENSE );		/* probe FDC to make it return status */
    fdc_results( &floppy[ 0 ] );	/* flush controller using scratch structure */
    for( i = 0; i < NR_DRIVES; i++ )	/* Clear each drive. */
	floppy[ i ].fl_calibration = UNCALIBRATED;

    /* Tell FDC drive parameters. */
    fdc_out( FDC_SPECIFY ); 	/* specify some timing parameters */
    fdc_out( current_spec1 = spec1[ d ] ); /* step-rate and head-unload-time */
    fdc_out( SPEC2 );		/* head-load-time and non-dma */
}



PRIVATE void clock_mess( int count, TIMER_FUNC func )
{
    TIMER_MSG *msg;

    msg = HERMES_DATA_PTR( floppy_copy_buffer );
    msg->wakeup_mode = WAKEUP_BY_CALL;
    msg->func_to_call = func;
    msg->count = count;
    k_message( timer_port, floppy_copy_buffer, sizeof( TIMER_MSG ) );
}
