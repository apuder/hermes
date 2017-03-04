/*
 *  Common code for printf et al.
 *
 *  The calling routine typically takes a variable number of arguments,
 *  and passes the address of the first one.  This implementation
 *  assumes a straightforward, stack implementation, aligned to the
 *  machine's wordsize.  Increasing addresses are assumed to point to
 *  successive arguments (left-to-right), as is the case for a machine
 *  with a downward-growing stack with arguments pushed right-to-left.
 *
 *  To write, for example, fprintf() using this routine, the code
 *
 *	fprintf(fd, format, args)
 *	FILE *fd;
 *	char *format;
 *	{
 *	_doprnt(format, &args, fd);
 *	}
 *
 *  would suffice.  (This example does not handle the fprintf's "return
 *  value" correctly, but who looks at the return value of fprintf
 *  anyway?)
 *
 *  This version implements the following printf features:
 *
 *	%d	decimal conversion
 *	%u	unsigned conversion
 *	%x	hexadecimal conversion
 *	%X	hexadecimal conversion with capital letters
 *	%o	octal conversion
 *	%c	character
 *	%s	string
 *	%m.n	field width, precision
 *	%-m.n	left adjustment
 *	%0m.n	zero-padding
 *	%*.*	width and precision taken from arguments
 *
 *  This version does not implement %f, %e, or %g.  It accepts, but
 *  ignores, an `l' as in %ld, %lo, %lx, and %lu, and therefore will not
 *  work correctly on machines for which sizeof(long) != sizeof(int).
 *  It does not even parse %D, %O, or %U; you should be using %ld, %o and
 *  %lu if you mean long conversion.
 *
 *  This version implements the following nonstandard features:
 *
 *	%b	binary conversion
 *	%r	roman numeral conversion
 *	%R	roman numeral conversion with capital letters
 *
 *  As mentioned, this version does not return any reasonable value.
 *
 *  Permission is granted to use, modify, or propagate this code as
 *  long as this notice is incorporated.
 *
 *  Steve Summit 3/25/87
 */


#include "io.h"

#define isdigit(d) ((d) >= '0' && (d) <= '9')
#define Ctod(c) ((c) - '0')

#define TRUE   1
#define FALSE  0

#define MAXBUF (sizeof(long int) * 8)		 /* enough for binary */

char *printnum();



void printf( const char *fmt, ... )
{
	va_list		argp;
	char		buf[ 160 ];

	va_start( argp, fmt );
	vsprintf( buf, fmt, argp );
	output_string( buf );
	va_end( argp );
}



void vsprintf( char *buf, const char *fmt, va_list argp )
{
	char		*p;
	char		*p2;
	int			length;
	int			prec;
	int			ladjust;
	char		padc;
	int			n;
	unsigned int u;
	int			negflag;
	char		c;

	while (*fmt != '\0') {
	    if (*fmt != '%') {
			*buf++ = *fmt++;
			continue;
	    }
	    fmt++;
	    if (*fmt == 'l')
			fmt++;	     /* need to use it if sizeof(int) < sizeof(long) */

	    length = 0;
	    prec = -1;
	    ladjust = FALSE;
	    padc = ' ';

	    if (*fmt == '-') {
			ladjust = TRUE;
			fmt++;
	    }

	    if (*fmt == '0') {
			padc = '0';
			fmt++;
	    }

	    if (isdigit(*fmt)) {
			while(isdigit(*fmt))
			    length = 10 * length + Ctod(*fmt++);
	    }
	    else if (*fmt == '*') {
			length = va_arg(argp, int);
			fmt++;
			if (length < 0) {
			    ladjust = !ladjust;
			    length = -length;
			}
	    }

	    if (*fmt == '.') {
			fmt++;
			if (isdigit(*fmt)) {
		    	prec = 0;
			    while(isdigit(*fmt))
					prec = 10 * prec + Ctod(*fmt++);
			} else if (*fmt == '*') {
			    prec = va_arg(argp, int);
			    fmt++;
			}
	    }

	    negflag = FALSE;

	    switch(*fmt) {
			case 'b':
			case 'B':
				u = va_arg(argp, unsigned int);
				buf = printnum( buf, u, 2, FALSE, length, ladjust, padc, 0 );
				break;

			case 'c':
				c = va_arg(argp, int);
				*buf++ = c;
				break;

			case 'd':
			case 'D':
				n = va_arg(argp, int);
				if (n >= 0)
					u = n;
				else {
					u = -n;
					negflag = TRUE;
				}
				buf = printnum( buf, u, 10, negflag, length, ladjust, padc, 0 );
				break;

			case 'o':
			case 'O':
				u = va_arg(argp, unsigned int);
				buf = printnum( buf, u, 8, FALSE, length, ladjust, padc, 0 );
				break;

			case 's':
				p = va_arg(argp, char *);
				if (p == (char *)0)
					p = "(NULL)";
				if (length > 0 && !ladjust) {
					n = 0;
					p2 = p;
					for (; *p != '\0' && (prec == -1 || n < prec); p++)
						n++;
					p = p2;
					while (n < length) {
						*buf++ = ' ';
						n++;
					}
				}
				n = 0;
				while (*p != '\0') {
					if (++n > prec && prec != -1)
					break;
					*buf++ = *p++;
				}
				if (n < length && ladjust) {
					while (n < length) {
						*buf++ = ' ';
						n++;
					}
				}
				break;

			case 'u':
			case 'U':
				u = va_arg(argp, unsigned int);
				buf = printnum( buf, u, 10, FALSE, length, ladjust, padc, 0 );
				break;

			case 'x':
				u = va_arg(argp, unsigned int);
				buf = printnum( buf, u, 16, FALSE, length, ladjust, padc, 0 );
				break;

			case 'X':
				u = va_arg(argp, unsigned int);
				buf = printnum( buf, u, 16, FALSE, length, ladjust, padc, 1 );
				break;

			case '\0':
				fmt--;
				break;

			default:
				*buf++ = *fmt;
		}
		fmt++;
	}
	*buf = '\0';
}



char *printnum( b, u, base, negflag, length, ladjust, padc, upcase )
char			*b;
unsigned int	u;	/* number to print */
int				base;
int				negflag;
int				length;
int				ladjust;
char			padc;
int				upcase;
{
	char	buf[MAXBUF];	/* build number here */
	char	*p = &buf[MAXBUF-1];
	int		size;
	char	*digs;
	static char up_digs[] = "0123456789ABCDEF";
	static char low_digs[] = "0123456789abcdef";

	digs = upcase ? up_digs : low_digs;
	do {
	    *p-- = digs[ u % base ];
	    u /= base;
	} while( u != 0 );

	if( negflag )
	    *b++ = '-';

	size = &buf[ MAXBUF - 1 ] - p;

	if( size < length && !ladjust ) {
	    while( length > size ) {
			*b++ = padc;
			length--;
	    }
	}

	while( ++p != &buf[ MAXBUF ] )
	    *b++ = *p;

	if( size < length ) {
	    /* must be ladjust */
	    while( length > size ) {
			*b++ = padc;
			length--;
	    }
	}
	return b;
}
