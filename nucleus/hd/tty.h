/* This file contains declarations common to the upper and lower levels of the
 * tty drivers.
 *
 * It also contains information that in theory need only be known by tty.c.
 */

#define NR_CONSOLES	   4	/* how many virtual consoles (1 min.) */
#define NR_RS_LINES	   2	/* how many rs232 lines (2 max.) */

/*
 * Values below here will not normally need to be altered.
 */

/* Miscellaneous parameters. */
#define SYSTEM_CONSOLE	   0	/* minor number for system console */
#define CON_MINOR	   1	/* minor number for first virtual console */
#define RS_MINOR	  64	/* minor number for first rs232 line */
#define NR_MINORS     (RS_MINOR + NR_RS_LINES)	/* how many minor numbers */
#define KB_IN_BYTES      500	/* keyboard input queue size */
#define RS_IBUFSIZE      256	/* RS232 input buffer size */
#define RS_IN_BYTES   (1024 + 2 * RS_IBUFSIZE)	/* RS232 input queue size */
#define RS_OUT_BYTES     640	/* RS232 output queue size */
#define TAB_SIZE           8	/* distance between tabs */
#define TAB_MASK          07	/* mask for tty_column when tabbing */
#define EVENT_THRESHOLD   64	/* events to accumulate before waking TTY */

/* Keyboard types. */
#define BOOT_KBD	   0	/* Keyboard that was chosen from boot menu */
#define IBM_PC		   1	/* Standard IBM keyboard */
#define OLIVETTI	   2	/* Olivetti keyboard */
#define DUTCH_EXT	   3	/* Dutch extended IBM keyboard */
#define US_EXT		   4	/* U.S. extended keyboard */

/* Default control characters. */
#define ERASE_CHAR      '\b'	/* default erase character (BS) */
#define KILL_CHAR        030 	/* default kill character (CTRL-X) */
#define INTR_CHAR (char)0177	/* default interrupt character (DEL) */
#define QUIT_CHAR (char) 034	/* default quit character (CTRL-\) */
#define XOFF_CHAR (char) 023	/* default x-off character (CTRL-S) */
#define XON_CHAR  (char) 021	/* default x-on character (CTRL-Q) */
#define EOT_CHAR  (char) 004	/* default end of text character (CTRL-D) */

/* This MARKER is used as an unambiguous flag for an unescaped end of file
 * character.  It is meaningful only in cooked mode.  0200 should never be
 * used in cooked mode, since that is supposed to be used only for 7-bit ASCII.
 * Be careful that code only checks for MARKER in cooked mode.  This kludge is
 * needed because chars are stored in char arrays, so there's no way to have a
 * completely out of band value.
 */
#define MARKER   (char) 0200	/* non-escaped CTRL-D stored as MARKER */
#define ESC       (char) 033	/* escape */
#define BRACKET          '['	/* Part of the ESC [ letter escape seq */

/* Tty_struct declares the types common to all terminals.  It also currently
 * (unfortunately) contains some information only used by the rs232 drivers.
 */
EXTERN struct tty_struct {
  char tty_type;		/* console or rs232 terminal */

  /* Input queue.  Typed characters are stored here until read by a program. */
  char *tty_inbuf;		/* pointer to input buffer */
  char *tty_inbufend;		/* pointer to place after last in buffer */
  char *tty_inhead;		/* pointer to place where next char goes */
  int tty_ihighwater;		/* threshold for queue too full */
  int tty_ilow_water;		/* threshold for queue not too full */
  int tty_insize;		/* size of buffer */
  char *tty_intail;		/* pointer to next char to be given to prog */
  phys_bytes tty_inphys;	/* physical address of input buffer */
  int tty_incount;		/* # chars in tty_inqueue */
  int tty_lfct;			/* # line feeds in tty_inqueue */
  int (*tty_devread) ();	/* routine to read from low level buffers */

  /* Output section. */
  char *tty_outbuf;		/* address of output buffer */
  phys_bytes tty_outphys;	/* physical address of output buffer */
  int tty_rwords;		/* number of WORDS (not bytes) in outqueue */
  void (*tty_devstart) ();	/* routine to start actual device output */

  /* Echo buffer. Echoing is also delayed by output in progress. */
  char *tty_ebufend;		/* end of echo buffer */
  char *tty_etail;		/* tail of echo buffer (head is fixed) */

  /* Terminal parameters and status. */
  int tty_mode;			/* terminal mode set by IOCTL */
  int tty_speed;		/* low byte is ispeed; high byte is ospeed */
  int tty_column;		/* current column number (0-origin) */
  char tty_escaped;		/* 1 when '\' just seen, else 0 */
  char tty_inhibited;		/* 1 when CTRL-S just seen (stops output) */
  char tty_makebreak;		/* 1 for terminals that interrupt twice/key */
  char tty_waiting;		/* 1 when output process waiting for reply */
  int tty_pgrp;			/* slot number of controlling process */

  /* User settable characters: erase, kill, interrupt, quit, x-on, x-off. */
  char tty_erase;		/* char used to erase 1 char (init ^H) */
  char tty_kill;		/* char used to erase a line (init @) */
  char tty_intr;		/* char used to send SIGINT  (init DEL) */
  char tty_quit;		/* char used for core dump   (init CTRL-\) */
  char tty_xon;			/* char used to start output (init CTRL-Q) */
  char tty_xoff;		/* char used to stop output  (init CTRL-S) */
  char tty_eof;			/* char used to stop output  (init CTRL-D) */

  /* Information about incomplete I/O requests is stored here. */
  char tty_incaller;		/* process that made the call (usually FS) */
  char tty_inproc;		/* process that wants to read from tty */
  char *tty_in_vir;		/* virtual address where data is to go */
  int tty_inleft;		/* how many chars are still needed */
  char tty_otcaller;		/* process that made the call (usually FS) */
  char tty_outproc;		/* process that wants to write to tty */
  phys_bytes tty_phys;		/* physical address where data comes from */
  int tty_outleft;		/* # chars yet to be output */
  int tty_cum;			/* # chars output so far */

  /* Cross reference to avoid slow pointer subtraction. */
  int tty_line;			/* base of 0 for each device class */

  /* Large arrays moved to end for shorter addresses. */
  char tty_ebuf[32];		/* echo buffer */
} tty_struct[NR_CONSOLES + NR_RS_LINES];

/* Values for the fields. */
#define CONSOLE		   0	/* tty is a console */
#define RS232		   1	/* tty is a RS232 terminal */
#define NOT_ESCAPED        0	/* previous character on this line not '\' */
#define ESCAPED            1	/* previous character on this line was '\' */
#define RUNNING            0	/* no CRTL-S has been typed to stop the tty */
#define STOPPED            1	/* CTRL-S has been typed to stop the tty */
#define INACTIVE           0	/* the tty is not printing */
#define BUSY               1	/* the tty is printing */
#define ONE_INT            0	/* regular terminals interrupt once per char */
#define TWO_INTS           1	/* IBM console interrupts two times per char */
#define NOT_WAITING        0	/* no output process is hanging */
#define WAITING            1	/* an output process is waiting for a reply */
#define SUSPENDED          2	/* like WAITING but different reply type */

/* Array and macro to convert minor number to tty_struct pointer. */
EXTERN struct tty_struct *p_tty_addr[NR_MINORS];
#define tty_addr(minor)    (p_tty_addr[minor])

/* Shared variables. */
EXTERN int fg_console;		/* currently active console */
EXTERN unsigned tty_events;	/* weighted input chars + output completions */

/* Appropiately sized buffers for keyboard and RS232 lines. */
EXTERN char kb_inbuf[NR_CONSOLES][KB_IN_BYTES];
EXTERN char rs_inbuf[NR_RS_LINES][RS_IN_BYTES];
EXTERN char rs_outbuf[NR_RS_LINES][RS_OUT_BYTES];
