/* This file may have been modified by DJ Delorie (Jan 1991).  If so,
** these modifications are Coyright (C) 1991 DJ Delorie, 24 Kirsten Ave,
** Rochester NH, 03867-2954, USA.
*/

#ifndef Incremental_h
#pragma once
#define Incremental_h
#define DECLARE_INIT_FUNCTION(USER_INIT_FUNCTION) \
static void USER_INIT_FUNCTION (); extern void (*_initfn)(); \
static struct xyzzy { xyzzy () {_initfn = USER_INIT_FUNCTION;}; \
~xyzzy () {};} __2xyzzy;
#else
#error Incremental.h was not the first file included in this module
#endif
