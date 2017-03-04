/* This is file builtin.h */
/* This file may have been modified by DJ Delorie (Jan 1991).  If so,
** these modifications are Coyright (C) 1991 DJ Delorie, 24 Kirsten Ave,
** Rochester NH, 03867-2954, USA.
*/

// This may look like C code, but it is really -*- C++ -*-

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

/*
  arithmetic, etc. functions on built in types
*/


#ifndef _builtin_h
#pragma once
#define _builtin_h 1


typedef void (*one_arg_error_handler_t)(const char*);
typedef void (*two_arg_error_handler_t)(const char*, const char*);



#include <stddef.h>
#include <std.h>
#include <math.h>

long         gcd(long, long);
long         lg(unsigned long); 
double       pow(double, long);
long         pow(long, long);

double       start_timer();
#ifdef USE_BUILTIN_DOUBLES
double       return_elapsed_time(double last_time = 0.0);
#endif

char*        itoa(long x, int base = 10, int width = 0);
char*        itoa(unsigned long x, int base = 10, int width = 0);
#ifdef __GNUG__
char*        itoa(long long x, int base = 10, int width = 0);
char*        itoa(unsigned long long x, int base = 10, int width = 0);
#endif
char*        dtoa(double x, char cvt = 'g', int width = 0, int prec = 6);

char*        hex(long x, int width = 0);
char*        hex(unsigned long x, int width = 0);
char*        oct(long x, int width = 0);
char*        oct(unsigned long x, int width = 0);
char*        dec(long x, int width = 0);
char*        dec(unsigned long x, int width = 0);

char*        form(const char* fmt, ...);
char*        chr(char ch, int width = 0);
char*        str(const char* s, int width = 0);

unsigned int hashpjw(const char*);
unsigned int multiplicativehash(int);
unsigned int foldhash(double);

extern void default_one_arg_error_handler(const char*);
extern void default_two_arg_error_handler(const char*, const char*);

extern two_arg_error_handler_t lib_error_handler;

extern two_arg_error_handler_t 
       set_lib_error_handler(two_arg_error_handler_t f);

//#ifdef __OPTIMIZE__

#ifdef USE_BUILTIN_DOUBLES
static inline double abs(double arg) 
{
  return (arg < 0.0)? -arg : arg;
}

static inline float abs(float arg) 
{
  return (arg < 0.0)? -arg : arg;
}
#endif

static inline short abs(short arg) 
{
  return (arg < 0)? -arg : arg;
}

static inline long abs(long arg) 
{
  return (arg < 0)? -arg : arg;
}

static inline int sign(long arg)
{
  return (arg == 0) ? 0 : ( (arg > 0) ? 1 : -1 );
}

#ifdef USE_BUILTIN_DOUBLES
static inline int sign(double arg)
{
  return (arg == 0.0) ? 0 : ( (arg > 0.0) ? 1 : -1 );
}
#endif

static inline long sqr(long arg)
{
  return arg * arg;
}

static inline double sqr(double arg)
{
  return arg * arg;
}

static inline int even(long arg)
{
  return !(arg & 1);
}

static inline int odd(long arg)
{
  return (arg & 1);
}

static inline long lcm(long x, long y)
{
  return x / gcd(x, y) * y;
}

static inline void setbit(long& x, long b)
{
  x |= (1 << b);
}

static inline void clearbit(long& x, long b)
{
  x &= ~(1 << b);
}

static inline int testbit(long x, long b)
{
  return ((x & (1 << b)) != 0);
}

//#endif

#endif
