
#ifndef __FD__
#define __FD__

#include <ci_types.h>


#define BLOCK_SIZE	1024

/* I/O Ports used by floppy disk task. */
#define DOR 		0x3F2	/* motor drive control bits */
#define FDC_STATUS	0x3F4	/* floppy disk controller status register */
#define FDC_DATA	0x3F5	/* floppy disk controller data register */
#define FDC_RATE	0x3F7	/* transfer rate register */
#define DMA_ADDR	0x004	/* port for low 16 bits of DMA address */
#define DMA_TOP 	0x081	/* port for top 4 bits of 20-bit DMA addr */
#define DMA_COUNT	0x005	/* port for DMA count (count =	bytes - 1) */
#define DMA_M2		0x00C	/* DMA status port */
#define DMA_M1		0x00B	/* DMA status port */
#define DMA_INIT	0x00A	/* DMA init port */
#define DMA_RESET_VAL	0x06

/* Status registers returned as result of operation. */
#define ST0 		0x00	/* status register 0 */
#define ST1 		0x01	/* status register 1 */
#define ST2 		0x02	/* status register 2 */
#define ST3 		0x00	/* status register 3 (return by DRIVE_SENSE) */
#define ST_CYL		0x03	/* slot where controller reports cylinder */
#define ST_HEAD 	0x04	/* slot where controller reports head */
#define ST_SEC		0x05	/* slot where controller reports sector */
#define ST_PCN		0x01	/* slot where controller reports present cyl */

/* Fields within the I/O ports. */
/* Main status register. */
#define CTL_BUSY	0x10	/* bit is set when read or write in progress */
#define DIRECTION	0x40	/* bit is set when reading data reg is valid */
#define MASTER		0x80	/* bit is set when data reg can be accessed */

/* Digital output port (DOR). */
#define MOTOR_MASK	0xF0	/* these bits control the motors in DOR */
#define ENABLE_INT	0x0C	/* used for setting DOR port */

/* ST0. */
#define ST0_BITS	0xF8	/* check top 5 bits of seek status */
#define TRANS_ST0	0x00	/* top 5 bits of ST0 for READ/WRITE */
#define SEEK_ST0	0x20	/* top 5 bits of ST0 for SEEK */

/* ST1. */
#define BAD_SECTOR	0x05	/* if these bits are set in ST1, recalibrate */
#define WRITE_PROTECT	0x02	/* bit is set if diskette is write protected */

/* ST2. */
#define BAD_CYL 	0x1F	/* if any of these bits are set, recalibrate */

/* ST3 (not used). */
#define ST3_FAULT	0x80	/* if this bit is set, drive is sick */
#define ST3_WR_PROTECT	0x40	/* set when diskette is write protected */
#define ST3_READY	0x20	/* set when drive is ready */

/* Floppy disk controller command bytes. */
#define FDC_SEEK	0x0F	/* command the drive to seek */
#define FDC_READ	0xE6	/* command the drive to read */
#define FDC_WRITE	0xC5	/* command the drive to write */
#define FDC_SENSE	0x08	/* command the controller to tell its status */
#define FDC_RECALIBRATE 0x07	/* command the drive to go to cyl 0 */
#define FDC_SPECIFY 	0x03	/* command the drive to accept params */
#define FDC_READ_ID 	0x4A	/* command the drive to read sector identity */
#define FDC_FORMAT	0x4D	/* command the drive to format a track */

/* DMA channel commands. */
#define DMA_READ	0x46	/* DMA read opcode */
#define DMA_WRITE	0x4A	/* DMA write opcode */

/* Parameters for the disk drive. */
#define SECTOR_SIZE 	512	/* physical sector size in bytes */
#define HC_SIZE 	2880	/* # sectors on largest legal disk (1.44MB) */
#define NR_HEADS	0x02	/* two heads (i.e., two tracks/cylinder) */
#define DTL 		0xFF	/* determines data length (sector size) */
#define SPEC2		0x02	/* second parameter to SPECIFY */

#define MOTOR_OFF	3*HZ	/* how long to wait before stopping motor */

/* Error codes */
#define ERR_SEEK	-1	/* bad seek */
#define ERR_TRANSFER	-2	/* bad transfer */
#define ERR_STATUS	-3	/* something wrong when getting status */
#define ERR_RECALIBRATE -4	/* recalibrate didn't work properly */
#define ERR_WR_PROTECT	-5	/* diskette is write protected */
#define ERR_DRIVE	-6	/* something wrong with a drive */
#define ERR_READ_ID 	-7	/* bad read id */
#define ERR_PARAM	-8	/* Bad user parameters */

/* Encoding of drive type in minor device number. */
#define DEV_TYPE_BITS	0x7C	/* drive type + 1, if nonzero */
#define DEV_TYPE_SHIFT	2	/* right shift to normalize type bits */
#define FORMAT_DEV_BIT	0x80	/* bit in minor to turn write into format */

/* Miscellaneous. */
#define MOTOR_RUNNING	0xFF	/* message type for clock interrupt */
#define MAX_ERRORS	3	/* how often to try rd/wt before quitting */
#define MAX_RESULTS 	7	/* max number of bytes controller returns */
#define NR_DRIVES	2	/* maximum number of drives */
#define DIVISOR 	128	/* used for sector size encoding */
#define MAX_FDC_RETRY	100	/* max # times to try to output to FDC */
#define NT		7	/* number of diskette/drive combinations */
#define AUTOMATIC	0x3F	/* bit map allowing both 3.5 and 5.25 disks */
				/* except for drive type 6, because that is */
				/* hard to distinguish from drive type 1 */
#define THREE_INCH	0x48	/* bit map allowing only 3.5 inch diskettes */
#define FIVE_INCH	0x37	/* bit map allowing only 5.25 inch diskettes */

#define DISK_READ	0
#define DISK_WRITE	1

typedef struct {
    int 	req;
    int 	device;
    unsigned	position;
    unsigned	address;
    int 	count;
} FLOPPY_REQ;



/* -- Messages from and to drivers -- */

/* -- Messages -- */
#define DRIVER_ACK	1
#define DEVICE_READ	2
#define DEVICE_WRITE    3

#define BYTES_PER_BLOCK 512

typedef struct {
    u_int Size;
    u_int Type;
    u_int MajorDevice;
    u_int MinorDevice;
    u_int SectorAddr;
    u_int MemoryAddr;
    u_int Seq;
} ToDriverMsg;

typedef struct {
    u_int Size;
    u_int Type;
    u_int Seq;
    u_int Result;
} DriverMsg;

void floppy_task( PORT_ID first_port, PORT_ID parent_port );


#endif // __FD__
