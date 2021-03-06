/* This is file WILD.C */
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

int wild(char *pattern, char *string)
{
  int nlit;
  while (*pattern)
  {
    switch (*pattern)
    {
      case '*':
        pattern++;
        if (*pattern == 0)
          return 1;
        nlit=0;
        while ((pattern[nlit] != 0)
            && (pattern[nlit] != '*')
            && (pattern[nlit] != '?') )
          nlit++;
        while (1)
        {
          if (strncmp(string, pattern, nlit) == 0)
            break;
          string++;
          if (*string == 0)
            return 0;
        }
        break;
      case '?':
        if (*string == 0)
          return 0;
        pattern++;
        string++;
        break;
      default:
        if (*pattern != *string)
          return 0;
        pattern++;
        string++;
        break;
    }
  }
  if (*string)
    return 0;
  return 1;
}

#ifdef DEBUG
main(int argc, char **argv)
{
  int i;
  if (argc < 3)
    return 1;
  for (i=2; argv[i]; i++)
    printf("%s %s %d\n", argv[1], argv[i], wild(argv[1], argv[i]));
  return 0;
}
#endif
