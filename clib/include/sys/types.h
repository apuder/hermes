/* This is file TYPES.H */
/*
** Copyright (C) 1991 DJ Delorie, 24 Kirsten Ave, Rochester NH 03867-2954
**
** This file is distributed under the terms listed in the document
** "copying.dj", available from DJ Delorie at the address above.
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _TYPES_H_
#define _TYPES_H_

typedef unsigned long time_t;

#ifndef __CI_TYPES__
typedef unsigned char u_char;
typedef unsigned int u_int;
#endif

typedef unsigned long u_long;
typedef long off_t;
typedef char *caddr_t;

#endif
