/* This is file discrete.h */
/* This file may have been modified by DJ Delorie (Jan 1991).  If so,
** these modifications are Coyright (C) 1991 DJ Delorie, 24 Kirsten Ave,
** Rochester NH, 03867-2954, USA.
*/

// This may look like C code, but it is really -*- C++ -*-
/* 
Copyright (C) 1988 Free Software Foundation
    written by Dirk Grunwald (grunwald@cs.uiuc.edu)

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
#ifndef _DiscreteUniform_h
#pragma once
#define _DiscreteUniform_h 1

#include <_Random.h>

//
//	The interval [lo..hi)
// 

class DiscreteUniform: public Random {
    long pLow;
    long pHigh;
    double delta;
public:
    DiscreteUniform(long low, long high, RNG *gen);

    long low();
    long low(long x);
    long high();
    long high(long x);

    virtual double operator()();
};

//#ifdef __OPTIMIZE__

inline DiscreteUniform::DiscreteUniform(long low, long high, RNG *gen) : (gen)
{
    pLow = (low < high) ? low : high;
    pHigh = (low < high) ? high : low;
    delta = (pHigh - pLow) + 1;
}

inline long DiscreteUniform::low() { return pLow; }

inline long DiscreteUniform::low(long x) {
  long tmp = pLow;
  pLow = x;
  delta = (pHigh - pLow) + 1;
  return tmp;
}

inline long DiscreteUniform::high() { return pHigh; }

inline long DiscreteUniform::high(long x) {
  long tmp = pHigh;
  pHigh = x;
  delta = (pHigh - pLow) + 1;
  return tmp;
}

//#endif
#endif
