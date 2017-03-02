/* This is file STRRCHR.C */
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

#include "string.h"

char *strrchr(const char *s, int c)
{
  const char *sp=(char *)0;
  while (*s)
  {
    if (*s == c)
      sp = s;
    s++;
  }
  if (c == 0)
    return (char *)s;
  return (char *)sp;
}

