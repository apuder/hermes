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

#ifndef _Filebuf_h
#ifdef __GNUG__
#pragma once
#pragma interface
#endif
#define _Filebuf_h 1

#include <_File.h>
#include <streambuf.h>

class Filebuf: public streambuf // libg++ File version
{
public:
  File*       Fp;

  void        init_streambuf_ptrs();

  int         overflow(int c = EOF);
  int         underflow();

              Filebuf();
              Filebuf(const char* filename, io_mode m, access_mode a);
              Filebuf(const char* filename, const char* m);   
              Filebuf(int filedesc, io_mode m);
              Filebuf(FILE* fileptr);
  
             ~Filebuf();

  const char* name();
  streambuf*  setbuf(char* buf, int buflen, int preloaded_count = 0);

  streambuf*  open(const char* name, open_mode m);
  streambuf*  open(const char* filename, io_mode m, access_mode a);
  streambuf*  open(const char* filename, const char* m);
  streambuf*  open(int  filedesc, io_mode m);
  streambuf*  open(FILE* fileptr);

  int         is_open();
  int         close();

  void        error();
};


#endif
