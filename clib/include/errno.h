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

#ifndef errno_h
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define ENOENT	 2		/* No such file or directory	*/
#define ENOTDIR  3              /* No path                      */
#define EMFILE	 4		/* Too many open files		*/
#define EACCES	 5		/* Permission denied		*/
#define EBADF	 6		/* Bad file number		*/
#define EARENA   7		/* Arena trashed		*/
#define ENOMEM	 8		/* Not enough core		*/
#define ESEGV    9		/* invalid memory address	*/
#define EBADENV 10		/* invalid environment		*/
#define ENODEV	15		/* No such device		*/
#define EINVAL	19		/* Invalid argument		*/
#define E2BIG	20		/* Arg list too long		*/
#define ENOEXEC 21		/* Exec format error		*/
#define EXDEV	22		/* Cross-device link		*/
#define EDOM	33		/* Math argument		*/
#define ERANGE	34		/* Result too large		*/
#define EEXIST	35		/* File already exists		*/

#ifdef __cplusplus
}
#endif

#ifndef errno_h
#define errno_h 1
#endif

#include <std.h>

#endif
