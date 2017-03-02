/* This is file stddef.h */
/* This file may have been modified by DJ Delorie (Jan 1991).  If so,
** these modifications are Coyright (C) 1991 DJ Delorie, 24 Kirsten Ave,
** Rochester NH, 03867-2954, USA.
*/

/* This may look like C code, but it is really -*- C++ -*- */
/* 
Copyright (C) 1988 Free Software Foundation
    written by Doug Lea (dl@rocky.oswego.edu)

This file is part of GNU CC.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU CC General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
GNU CC, but only under the conditions described in the
GNU CC General Public License.   A copy of this license is
supposed to have been given to you along with GNU CC so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  
*/

#ifndef _stddef_h
//AP #pragma once

#define _stddef_h

/* TRUE, FALSE, bool, are now in <bool.h>, for ANSI compatibility */

#define size_t unsigned 
/* typedef unsigned long   size_t; */
typedef long int	    ptrdiff_t;

#ifdef DGUX
typedef	int	wchar_t;
#else
typedef	unsigned short	wchar_t;
#endif

#ifndef NULL
#define NULL 0
#endif


#endif