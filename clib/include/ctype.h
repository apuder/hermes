/* This file may have been modified by DJ Delorie (Jan 1991).  If so,
** these modifications are Coyright (C) 1991 DJ Delorie, 24 Kirsten Ave,
** Rochester NH, 03867-2954, USA.
*/

/*
 Here's a ctype.h for SunOS-3 and vax 4.3BSD.  
 It will probably work on most BSD derived systems. 
 Just compare it to the C version to verify.
 No big deal, but it will save you some typing.
*/
   
#ifndef _ctype_h
#pragma once
#define _ctype_h
 
#include <stdio.h>  /* sorry, but needed for USG stuff */
   
#define _U 01
#define _L 02
#define _N 04
#define _S 010
#define _P 020
#define _C 040


#if defined(USG) || defined(DGUX)
#define _B 0100	/* different from BSD */
#define _X 0200	/* different from BSD */
#else
#define _X 0100
#define _B 0200
#endif


#ifdef DGUX
#define CTYPE_TYPE	short
#else
#define CTYPE_TYPE	char
#endif

#if defined(DGUX) || defined(USG) || defined(hpux)
#define _ctype_ _ctype
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern	CTYPE_TYPE	_ctype_[];
#ifdef __cplusplus
}
#endif
   
#define isalpha(c)  ((_ctype_+1)[c]&(_U|_L))
#define isupper(c)  ((_ctype_+1)[c]&_U)
#define islower(c)  ((_ctype_+1)[c]&_L)
#define isdigit(c)  ((_ctype_+1)[c]&_N)
#define isxdigit(c) ((_ctype_+1)[c]&(_X|_N))
#define isspace(c)  ((_ctype_+1)[c]&_S)
#define ispunct(c)  ((_ctype_+1)[c]&_P)
#define isalnum(c)  ((_ctype_+1)[c]&(_U|_L|_N))
#define isprint(c)  ((_ctype_+1)[c]&(_P|_U|_L|_N|_B))
#define isgraph(c)  ((_ctype_+1)[c]&(_P|_U|_L|_N))
#define iscntrl(c)  ((_ctype_+1)[c]&_C)
#define isascii(c)  ((unsigned)(c)<=0177)
#define toupper(c)  (islower(c)? (c-'a'+'A') : c)
#define tolower(c)  (isupper(c)? (c-'A'+'a') : c)
#define toascii(c)  ((c)&0177)


#ifdef _ctype_
#undef _ctype_
#endif

#ifdef CTYPE_TYPE
#undef CTYPE_TYPE
#endif

#endif _ctype_h
