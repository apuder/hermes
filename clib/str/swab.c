/* This is file SWAB.C */
/* This file may have been modified by DJ Delorie (Jan 1991).  If so,
** these modifications are Coyright (C) 1991 DJ Delorie, 24 Kirsten Ave,
** Rochester NH, 03867-2954, USA.
*/

/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jeffrey Mogul.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)swab.c	5.8 (Berkeley) 6/27/90";
#endif /* LIBC_SCCS and not lint */

#include <string.h>

void
swab(from, to, n)
     void *from, *to;
     int n;
{
    unsigned long temp;

    n >>= 1; n++;
#define	STEP	temp = *(char *)from++,*(char *)to++ = *(char *)from++,*(char *)to++ = temp
    /* round to multiple of 8 */
    while ((--n) & 07)
	STEP;
    n >>= 3;
    while (--n >= 0) {
	STEP; STEP; STEP; STEP;
	STEP; STEP; STEP; STEP;
    }
}
