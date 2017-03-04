/* This file may have been modified by DJ Delorie (Jan 1991).  If so,
** these modifications are Coyright (C) 1991 DJ Delorie, 24 Kirsten Ave,
** Rochester NH, 03867-2954, USA.
*/

#ifndef pwd_h
#pragma once

/* the Interviews-based standard kludge again */

#ifdef __cplusplus
extern "C" {
#endif

#define getpwent c_proto_getpwent
#define getpwuid c_proto_getpwuid
#define getpwnam c_proto_getpwnam
#define setpwent c_proto_setpwent
#define endpwent c_proto_endpwent
#define KERNEL

#include "//usr/include/pwd.h"

#ifndef pwd_h
#define pwd_h 1
#endif

#undef getpwent
#undef getpwuid
#undef getpwnam
#undef setpwent
#undef endpwent
#undef KERNEL

extern struct passwd* getpwent();
extern struct passwd* getpwuid(int);
extern struct passwd* getpwnam(char*);
extern int            setpwent();
extern int            endpwent();

#ifdef __cplusplus
}
#endif

#endif
