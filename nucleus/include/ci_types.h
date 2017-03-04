/* Type-definitions for file-server */

#ifndef __CI_TYPES__
#define __CI_TYPES__

#ifdef __TURBOC__


typedef char          s_char;
typedef unsigned char u_char;
typedef int           s_short;
typedef unsigned int  u_short;
typedef long          s_int;
typedef unsigned long u_int;

#endif


#ifdef __GNUC__

typedef char               s_char;
typedef unsigned char      u_char;
typedef short int          s_short;
typedef unsigned short int u_short;
typedef int                s_int;
typedef unsigned int       u_int;

#endif

#endif
