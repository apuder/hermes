#include <kernel.h>
#include <keyb.h>


#define KEYBD           0x60
#define PORT_B          0x61
#define KBIT            0x80

#ifdef KEYBOARD_PS2

#else

#define SYSRQ           0x40        /* Code sent by SysRq-Key */
#define PRTSCR          0x41        /* Code sent by PrtScr-Key */
#define BREAK           0x42        /* Code sent by Break-Key */
#define AT_SIGN         0220
#define TOP_ROW         14


#endif

#define MULT            0x100       /* 1 for testing the computed codes
                                       of the cursor and function-keys
                                       else 0x100                       */

PORT_ID parent_port;

#ifdef LANG=US_EN

static unsigned char small[] =  { 27,'1','2','3','4','5','6','7','8','9','0','-','=','\b','\t','q','w','e','r','t','y','u','i','o','p','[',']',13,0,'a','s','d','f','g','h','j','k','l',';','\'','\`',0,'\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1','2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

static unsigned char capped[] = { 27,'1','2','3','4','5','6','7','8','9','0','-','=','\b','\t','Q','W','E','R','T','Y','U','I','O','P','[',']',13,0,'A','S','D','F','G','H','J','K','L',';',0,'\`','\'','\\','Z','X','C','V','B','N','M',',','.','/',0,'*',0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+',
'1','2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
,0 };

static unsigned char shfted[] = { 27,'!','@','#','$','%','^','&','*','(',')','_','+','\b','\t','Q','W','E','R','T','Y','U','I','O','P','{','}',13,0,'A','S','D','F','G','H','J','K','L',':','"','~',0,'|','Z','X','C','V','B','N','M','<','>','?',0,0,0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

#else

static unsigned char unshifted[] = {0,
	     27, '1', '2', '3', '4', '5', '6', '7', '8', '9',      /* 1 */
	     '0', 'ß',  39,'\b','\t', 'q', 'w', 'e', 'r', 't',      /* 2 */
	     'z', 'u', 'i', 'o', 'p', 'ü', '+',  13,  16, 'a',      /* 3 */
	     's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'ö', 'ä',      /* 4 */
	     94, 128, '#', 'y', 'x', 'c', 'v', 'b', 'n', 'm',      /* 5 */
	     ',', '.', '-',0201, '*',0203, ' ',0204,  59,  60,      /* 6 */
	     61,  62,  63,  64,  65,  66,  67,  68,   0,   0,      /* 7 */
	     71,  72,  73, '-',  75, '/',  77, '+',  79,  80,      /* 8 */
	     81,  82,  83,   0,   0, '<', 133, 134 };              /* 9 */


static unsigned char shifted[] = {0,
	   27, '!', '"',  21, '$', '%', '&', '/', '(', ')',      /* 1 */
	   '=', '?',  96,'\b',  14, 'Q', 'W', 'E', 'R', 'T',      /* 2 */
	   'Z', 'U', 'I', 'O', 'P', 'Ü', '*', 015, 020, 'A',      /* 3 */
	   'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'Ö', 'Ä',      /* 4 */
	   248,0200,  39, 'Y', 'X', 'C', 'V', 'B', 'N', 'M',      /* 5 */
	   ';', ':', '_',0201, '*',0203, ' ',0204,  84,  85,      /* 6 */
	   86,  87,  88,  89,  90,  91,  92,  93,   0,   0,      /* 7 */
	   '7', '8', '9', '-', '4', '5', '6', '+', '1', '2',      /* 8 */
	   '3', '0', ',',   0,   0, '>', 135, 136 };              /* 9 */


static unsigned char controled[] = {0,
	     27, 201, 205, 187, 186, 206, 186, 200, 205, 188,      /* 1 */
	     0,   0,   0,'\b','\t',  17,  23,   5,  18,  20,      /* 2 */
	     26,  21,   9,  15,  16,   0,   0,   0,  16,   1,      /* 3 */
	     19,   4,   6,   7,   8,  10,  11,  12,   0,   0,      /* 4 */
	     0,   0,   0,  25,  24,   3,  22,   2,  14,  13,      /* 5 */
	     0,   0,   0,   0,   0,   0,   0,   0,  94,  95,      /* 6 */
	     96,  97,  98,  99, 100, 101, 102, 103,   0,   0,      /* 7 */
	     119,   0, 131,   0, 115,   0, 116,   0, 117,   0,      /* 8 */
	     118,   0,   0,   0,   0,   0, 137, 138 };              /* 9 */


unsigned char ctrl_alt[] = {0,
	    27,   0, 253, 252, 156,   0,   0, '{', '[', ']',      /* 1 */
	    '}','\\',   0,'\b','\t', '@', 251, 238, 231, 233,      /* 2 */
	    157,   0, 244, 227,   0,   0, '~',   0,   0, 224,      /* 3 */
	    229, 235, 232, 226,   0, 245,   0,   0,   0,   0,      /* 4 */
	    0,   0,   0, 228,   0, 155,   0, 225, 239, 230,      /* 5 */
	    249, 250, 246,   0,   0,   0,   0,   0,   0,   0,      /* 6 */
	    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 7 */
	    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 8 */
	    0,   0,   0,   0,   0, '|',   0,   0 };              /* 9 */


static unsigned char alted[] = {0,
	 27, 218, 196, 191, 179, 197, 179, 192, 196, 217,      /* 1 */
	 174, 175,   0,'\b','\t',  16,  17,  18,  19,  20,      /* 2 */
	 44,  22,  23,  24,  25,   0,   0,   0,   0,  30,      /* 3 */
	 31,  32,  33,  34,  35,  36,  37,  38,   0,   0,      /* 4 */
	 0,   0,   0,  21,  45,  46,  47,  48,  49,  50,      /* 5 */
	 0,   0,   0,   0,   0,   0,   0,   0, 104, 105,      /* 6 */
	 106, 107, 108, 109, 110, 111, 112, 113,   0,   0,      /* 7 */
	 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 8 */
	 0,   0,   0,   0,   0,   0, 139, 140 };              /* 9 */


#endif

static unsigned char special = 0;
static unsigned char capslock = 0;
static unsigned char numlock = 0;
static unsigned char scrlock = 0;
static unsigned char alt = 0;


#ifdef KEYBOARD_PS2
	/* Variables indicating scancodes */
	static unsigned char brk = 0;
	static unsigned ignore = 0;
	/* Variables indicating keys */
	static unsigned char control = 0;
	static unsigned char shift = 0;
#else
	
	static unsigned char shift1 = 0;
	static unsigned char shift2 = 0;
	static unsigned char control1 = 0;
	static unsigned char control2 = 0;
	static unsigned char alt_gr = 0;
	static unsigned old_char = 0;
	static unsigned ascii = 0;
	static unsigned pause = 0;

#endif

#ifdef KEYBOARD_PS2
	static  unsigned char new_char;
#else
	static char 	new_char;
#endif
static char	 value;
static char	 k;
static unsigned  new_key;
static char     done;



void out_data( unsigned ch )                /* send data to the keyboard */
{
#ifdef KEYBOARD_PS2
    /* while( (inportb( 0x64 ) & 1) > 0 ); /* for some reason, this won't work */
#else
    while( (inportb( 0x64 ) & 2) > 0 );
#endif
    outportb(0x60,ch);
}


void set_led( void )                        /* set led-status an keyboard */
{
    out_data( 0xed );
    out_data( scrlock+2*numlock+4*capslock );
}


#ifdef KEYBOARD_PS2

#else

unsigned char get_state( void )             /* compute a state of shift, */
{                                           /* control,.... */
    unsigned char s = 0;

    s |= shift1;
    s |= shift2;
    s |= capslock;
    s |= 2 * control1;
    s |= 2 * control2;
    s |= 4 * alt;
    s |= 6 * alt_gr;
    return( s );
}


void make_ascii( unsigned char ch )         /* create an ascii-code with */
{                                           /* alt-np */
    if( ( ascii < 255 ) && ( ch >= 0 ) && ( ch <= 9 ) ) {
        ascii = 10 * ascii + ch;
        return;
    }
}


#endif


#ifdef KEYBOARD_PS2

#else

/* decode cursor- and functionkey code */

unsigned decode( unsigned char ch, unsigned char t )
     /* t=1 decode unshifted else */
{                                       /* decode control-codes */
    if( ( ( ch >= 0x3b ) && ( ch <= 0x44 ) ) ||     /* F1 - F10 */
        ( ( ch >= 0x57 ) && ( ch <= 0x58 ) ) )      /* F11, F12 */
        if( t )
            return( MULT * unshifted[ch] );
        else
            return( MULT * controled[ch] );
    else
        if( t )
            return( unshifted[ch] );
        else
            return( controled[ch] );
}


unsigned decode_shifted( unsigned char ch )
{
    if( ( ( ch >= 0x3b ) && ( ch <= 0x44 ) ) ||     /* F1 - F10 */
        ( ( ch >= 0x57 ) && ( ch <= 0x58 ) ) ||     /* F11, F12 */
        ( ch == 0x0f ) )
        return( MULT * shifted[ch] );
    else
        return( shifted[ch] );
}


unsigned decode_alt( unsigned char ch )
{
    if( ( ( ch >= 0x10 ) && ( ch <= 0x19 ) ) ||     /* Alt_ a-z */
        ( ( ch >= 0x1e ) && ( ch <= 0x26 ) ) ||
        ( ( ch >= 0x2c ) && ( ch <= 0x32 ) ) ||
        ( ( ch >= 0x3b ) && ( ch <= 0x44 ) ) ||     /* F1 - F10 */
        ( ( ch >= 0x57 ) && ( ch <= 0x58 ) ) )      /* F11, F12 */
        return( MULT * alted[ch] );
    else
        return( alted[ch] );
}


unsigned get_tbl( unsigned char ch )
{
    switch( get_state() ) {
    case 0:    return( decode( ch, 1 )); /* No special key pressed */
    case 1:    if( shift1 | shift2 )            /* Shift, Capslock */
	if( ( ch >= 2 ) && ( ch <= 0xb ) )
	    return( decode_shifted( ch ) );
	else
	    if( capslock )
		return( decode( ch, 1 ) );
	    else
		return( decode_shifted( ch ) );
    else
	if( ( ch >= 2 ) && ( ch <= 0xb ) )
	    return( decode( ch, 1 ) );
	else
	    return( decode_shifted( ch ) );
    case 2: return( decode( ch, 0 ) );      /* Control-key */
    case 4:    return( decode_alt( ch ) );        /* Alt-key */
    case 6: return( ctrl_alt[ch] );         /* Control-Alt or Alt_Gr */
    }
    return( 0 );
}

#endif

#ifdef KEYBOARD_PS2

unsigned get_keycode ( unsigned char ch ) {

	/* If it is a printable character, return the ascii value.
	If it is a non-printable character, return the scan code * MULT. */

	if( !brk ) {

        	/* F1 to F12 */
        	if( !special && !brk && !alt ) {
                	if( (ch > 0x3A) && (ch < 59) )
                        	return ch*MULT;
		}

		/* Keypad keys */

		/* UP, DN, LF, RT */    
		/* If numlock off || arrow keys */
		if( special ) {
			if ( (ch==0x48) || (ch==0x4B) || (ch==0x50) || (ch==0x4D) ) 
				return ch*MULT;
		}

		/* INS, DEL, HM, END, PG-UP, PD-DN */
		if( special ) {
                        if ( (ch==0x52) || (ch==0x47) || (ch==0x49) || (ch==0x53) || (ch==0x4F) || (ch==0x51) )
                                return ch*MULT;

		}

	}

        if( !special && !brk && !alt) {
                if( !shift && !capslock )
                        return small[ch-1];
                if( shift && !capslock )
                        return shfted[ch-1];
                if( !shift && capslock )
                        return capped[ch-1];
        }



	/* specials */
	if(special && !brk) {

	   if ( ch == 0x1C )	/* Keypad enter */
	   	return 13;
	   if ( ch == 0x35 )	/* Keypad / */
		return '/';

	}

	/* Ctrl - Alt - Del to restart */
	if( control && alt && ( ch==0x53 ) && !brk)
		shutdown("User break - CTRL-ALT-DEL pressed");

	return 0;

}

#else

unsigned convert_to_ascii( unsigned char ch )
{
    char c;

    if( ( ch >= 0x47 ) && ( ch <= 0x53 ) )  /* Cursor-keys and num-pad   */
        if( special ) {                        /* "only-cursor"-keys never  */
            special = 0;                    /* create digits or an ascii */
            if( control1 || control2 )      /* by pressing alt-key          */
                return( MULT * controled[ch] );
            else
                return( MULT * unshifted[ch] );
        }
        else {

	    /* CTRL-ALT-DEL restarts system */
	    /*
	      if( ch == 0x53 && ( control1 | control2 ) && ( alt ) &&
	      ( alt_gr == 0 ) )
	      shutdown( "User break !!!" );
	    */

            if ( alt )                        /* generate ascii with alt-np */
                make_ascii( shifted[ch] - 0x30 );
            else                            /* numpad creates digits */
                if( numlock || shift1 || shift2 )
                    return( shifted[ch] );
                else                        /* numpad creates cursor-codes */
                    if( control1 || control2 )
                        return( MULT * controled[ch] );
                    else
                        return( MULT * unshifted[ch] );
        }

    switch( ch ) {
    case 0x37:  if( special ) {                /* PrtSc/SysRq - Key */
	special = 0;
	if( shift1 | shift2 )
	    return( PRTSCR );
	else
	    return( SYSRQ );
    }
    else
	return( decode( ch, 1 ) );
    case 0x3a:  if( ch == old_char )    /* Capslock */
	return( 0 );
	capslock = 1 - capslock;
	set_led();
	old_char = ch;
	break;
    case 0x45:    if( ch == old_char )    /* Numlok */
	return( 0 );
	numlock = 1 - numlock;
	set_led();
	old_char = ch;
	break;
    case 0x46:  if( special == 0 ) {   /* Scrlock */
	if (ch == old_char )
	    return(0);
	scrlock = 1 - scrlock;
	set_led();
	old_char = ch;
    }
    else {
	special = 0;
	return( BREAK );
    }
	break;
    case 0x35:  if( ascii != 0 )        /* ascii-generation with alt-  */
	return( 0 );        /* np in process, don't return */
	if( special ) {            /* any other code              */
	    special = 0;        /* ÷-Key on Numpad */
	    return( 47 );
	}
	else                    /* -_ Key */
	    return( get_tbl( ch ) );
    case 0x1d:     if( special ) {            /* right-control pressed */
	control2 = 1;
	special = 0;
    }
    else                    /* left-control pressed */
	control1 = 1;
	break;
    case 0x9d:     if( special ) {         /* right-control depressed */
	control2 = 0;
	special = 0;
    }
    else                    /* left-control depressed */
	control1 = 0;
	break;
    case 0x38:    if( special ) {            /* alt_gr pressed */
	alt_gr = 1;
	special = 0;
    }
    else                    /* alt pressed */
	alt = 1;
	break;
    case 0xb8:    if( special ) {         /* alt_gr depressed */
	alt_gr = 0;
	special = 0;
    }
    else {                  /* alt depressed */
	alt = 0;
	if( ascii != 0 ) {  /* if ascii-code created with */
	    c = ascii;      /* alt-numpad exists, return  */
	    ascii = 0;        /* and clear this code          */
	    return( c );
	}
    }
	break;
    case 0x2a:  /* if( special ) {
		   special = 0;
		   return( PRTSCR );
		   }
		   else*/ {
	shift1 = 1;                /* left-shift pressed */
	break;
    }
    case 0x36:    shift2 = 1;             /* right-shift pressed */
	break;
    case 0xaa:    shift1 = 0;             /* left-shift depressed */
	break;
    case 0xb6:    shift2 = 0;             /* right-shift depressed */
	break;
    default:
	if( ascii == 0)                 /* if a code-creation with  */
	    return( get_tbl( ch ) );    /* alt-numpad is in process */
	else                            /* don't return any other   */
	    return( 0 );                /* code                        */
    }
    return( 0 );
}

#endif



void keyb_init( HANDLE handle )
{

#if KEYBOARD_PS2
    unsigned int stat;
    outportb( 0x64, 0x20 );
    stat=inportb( 0x60 );
    outportb( 0x64, 0x60 );
    while( (inportb( 0x64 ) & 1) > 0);
    outportb( 0x60, (stat | 64) );       /* Ensure Set 1 translation */
#endif

parent_port = (PORT_ID) handle;
set_led();
init_idt_entry( 0x61, IDT_ENTRY( keyb_isr ) );


}

#ifdef KEYBOARD_PS2

/* New implementation of Keyboard ISR with the intention to conform to 
 * Set 1 Scan codes from for US English language keyboard.
 * http://panda.cs.ndsu.nodak.edu/%7Eachapwes/PICmicro/keyboard/scancodes1.html
 */

void keyb_isr( void )
{

    /*
     *		PUSHAL			; Push context
     *		PUSH	%DS
     *		PUSH	%ES
     *		PUSH	%FS
     *		PUSH	%GS
     *		MOVW	$DATA_SELECTOR,%AX
     *		MOVW	%AX,%DS
     */
    asm( "pushal;push %ds;push %es;push %fs;push %gs" );
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );

#if 0
    printf("k ");
#endif

    new_char = inportb( KEYBD );
    value    = inportb( PORT_B );
    outportb( PORT_B, value | KBIT );
    outportb( PORT_B, value );
    done = FALSE;


    if( ( new_char == 0xE1 ) && !ignore ) {        /* For weird scancodes */
        ignore = 5;
        done = TRUE;
    }

    if( !done && ignore ) {        /* Ignore the remaining codes */
        ignore--;                        /* of the key with odd scancodes and the next */
        done = TRUE;                    /* pressed key will clear ignore */
    }   

    if ( !done && (new_char == 0xE0) ) {  /* Flag for codes which are */
        special = 2;                    /* used by two keys */
        done = TRUE;
    }

    if( !done && (new_char & 128) == 128) {   /* Flag for codes which are break codes */
        brk = 1;
	new_char = new_char & 127;		/* Reset MSB */
    }

    if( !done && !brk && (new_char == 0x2A) ) {
	if(special == 0)
		shift = 1;		 /* For Left Shift key */
	else 
		ignore = 3;		 /* For Print screen */			
	done = TRUE;
    }

    if( !done ) {				/* For those that req key press */
	if( new_char == 0x3A) {			/* go into effect only when key */
		capslock = 1 - capslock;	/* is pressed while it is pressed.  */
		done = TRUE;
		set_led();
	}
	if( new_char == 0x45) {		/* go into effect only when key */
		numlock = 1 - numlock;	/* is pressed. */
                done = TRUE;
		set_led();
        }
	if( new_char == 0x46) {
		scrlock = 1 - scrlock;
                done = TRUE;
		set_led();
        }
    }

    if ( !done ) {		
    if (brk) {			/* For those that req key release */
	if( new_char == 0x1D) {
                control = 0;		/* Ctrl, alt, shift lose their effect */
                done = TRUE;
        }
        if( new_char == 0x28) {		/* once key is lifted. */
                alt = 0;
                done = TRUE;
        }
        if( new_char == 0x36) {
                shift = 0;
                done = TRUE;
        }
	if( new_char == 0x2A) {
		shift = 0;
                done = TRUE;
        }
    }
    else {			/* For those that req key press */
        if( new_char == 0x1D) {         /* Left + Right Ctrl */
                control = 1;
                done = TRUE;
        }
        if( new_char == 0x38) {         /* Left + Right Alt */
                alt = 1;
                done = TRUE;
        }
        if( new_char == 0x36) {         /* Right Shift Key */
                shift = 1;
                done = TRUE;
        }
    }
    }

	
    if( !done && (( new_key = get_keycode( new_char ) ) != 0) ) {
        queue_event( &keyb_event, new_key );
        raise_event( parent_port );
	
    }

    if(special) special--;		/* these decrements will allow the */
    if(brk) brk--;			/* vars to become false (0) */
					/* after due number of scan codes */

    /*
     *		MOVB	$0x20,%AL	; Reset interrupt controller
     *		OUTB	%AL,$0x20
     *		POP 	%GS 		; Restore previously saved context
     *		POP 	%FS
     *		POP 	%ES
     *		POP 	%DS
     *		POPAL
     *		IRET			; Return to new process
     */
    asm( "movb $0x20,%al;outb %al,$0x20" );
    asm( "pop %gs;pop %fs;pop %es;pop %ds" );
    asm( "popal;iret" );

}

#else

void keyb_isr( void )
{

    /*
     *          PUSHAL                  ; Push context
     *          PUSH    %DS
     *          PUSH    %ES
     *          PUSH    %FS
     *          PUSH    %GS
     *          MOVW    $DATA_SELECTOR,%AX
     *          MOVW    %AX,%DS
     */
    asm( "pushal;push %ds;push %es;push %fs;push %gs" );
    asm( "movw %0,%%ax" : : "i" (DATA_SELECTOR) );
    asm( "movw %ax,%ds" );

#if 0
    printf("k ");
#endif

    new_char = inportb( KEYBD );
    value    = inportb( PORT_B );
    outportb( PORT_B, value | KBIT );
    outportb( PORT_B, value );

    done = FALSE;

    k = new_char - 0200;
    if( ( k == 0x61 ) && ( pause == 0 ) ) {        /* Pause-Key */
        pause = 6;
        done = TRUE;
    }
    if( !done && (pause > 0) ) {        /* Ignore the remaining codes */
        pause--;                        /* of the Pause-key and the next */
        done = TRUE;                    /* pressed key will clear pause- */
    }                                   /* flag */
    if( !done && (k == old_char) )      /* Break-code of the stored */
        old_char = 0;                   /* key */

    if( !done && (k == 0x60) )          /* Flag for codes which are */
        special = 1;                    /* used by two keys */

    if( !done && special && ( k > 0 ) &&/* Clear special-flag set by the */
        ( k != 0x60 ) && ( k != 0x38 )  /* break-code of a special key   */
        && ( k != 0x1d ) )              /* except of the right CTRL and  */
        special = 0;                    /* ALT_GR key                     */

    if( !done && (k > 0) ) {
        if( k != 0x1d && k != 0x38& k != 0x2a && k != 0x36 )
            done = TRUE;
    }
    if( !done && (( new_key = convert_to_ascii( new_char ) ) != 0) ) {
        queue_event( &keyb_event, new_key );
        raise_event( parent_port );
    }

    /*
     *          MOVB    $0x20,%AL       ; Reset interrupt controller
     *          OUTB    %AL,$0x20
     *          POP     %GS             ; Restore previously saved context
     *          POP     %FS
     *          POP     %ES
     *          POP     %DS
     *          POPAL
     *          IRET                    ; Return to new process
     */
    asm( "movb $0x20,%al;outb %al,$0x20" );
    asm( "pop %gs;pop %fs;pop %es;pop %ds" );
    asm( "popal;iret" );

}

#endif
