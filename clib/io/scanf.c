/* This is file SCANF.C */
/* This file may have been modified by DJ Delorie (Jan 1991).  If so,
** these modifications are Coyright (C) 1991 DJ Delorie, 24 Kirsten Ave,
** Rochester NH, 03867-2954, USA.
*/

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)scanf.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include	<stdio.h>

scanf(const char *fmt, ...)
{
	return(_doscan(stdin, fmt, (&fmt)+1));
}

fscanf(FILE *iop, const char *fmt, ...)
{
	return(_doscan(iop, fmt, (&fmt)+1));
}

sscanf(char *str, const char *fmt, ...)
{
	FILE _strbuf;

	_strbuf._flag = _IOREAD|_IOSTRG;
	_strbuf._ptr = _strbuf._base = str;
	_strbuf._cnt = 0;
	while (*str++)
		_strbuf._cnt++;
	_strbuf._bufsiz = _strbuf._cnt;
	return(_doscan(&_strbuf, fmt, (&fmt)+1));
}
