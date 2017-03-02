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

#ifndef fcntl_h
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define KERNEL


#define O_RDONLY	0x0001
#define	O_WRONLY	0x0002
#define	O_RDWR		0x0004
#define	O_CREAT		0x0100
#define	O_TRUNC		0x0200
#define O_EXCL		0x0400
#define O_APPEND	0x0800
#define O_TEXT		0x4000
#define O_BINARY	0x8000

extern int _fmode;

#ifndef fcntl_h
#define fcntl_h 1
#endif

#undef KERNEL

#ifdef __cplusplus
}
#endif

#include <std.h>

#endif
