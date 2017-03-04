/* This file may have been modified by DJ Delorie (Jan 1991).  If so,
** these modifications are Coyright (C) 1991 DJ Delorie, 24 Kirsten Ave,
** Rochester NH, 03867-2954, USA.
*/


#ifndef grp_h
#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
#define getgrent c_proto_getgrent
#define getgrgid c_proto_getgrgid
#define getgrnam c_proto_getgrnam
#define setgrent c_proto_setgrent
#define endgrent c_proto_endgrent
#define fgetgrent c_proto_fgetgrent

#define KERNEL

#include "//usr/include/grp.h"

#ifndef grp_h
#define grp_h 1
#endif

#undef getgrent
#undef getgrgid
#undef getgrnam
#undef KERNEL

extern struct group* getgrent();
extern struct group* fgetgrent(FILE*);
extern struct group* getgrgid(int);
extern struct group* getgrnam(const char*);
extern void          setgrent();
extern void          endgrent();

#ifdef __cplusplus
}
#endif

#endif
