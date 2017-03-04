
/*  A Bison parser, made from dbyacc.y  */

#define	TOKEN_UNDEF	258
#define	COMMAND_HELP	259
#define	COMMAND_CLS	260
#define	COMMAND_DISP_REGS	261
#define	COMMAND_DUMP	262
#define	COMMAND_TRANSLATE	263
#define	COMMAND_GDT	264
#define	COMMAND_PS	265
#define	COMMAND_WAKEUP	266
#define	COMMAND_VAR	267
#define	COMMAND_EXIT	268
#define	COMMAND_LOAD_EXE	269
#define	COMMAND_DISP_SYMS	270
#define	COMMAND_SHOW	271
#define	COMMAND_STEP	272
#define	NUMBER	273
#define	STRING	274
#define	LAST_VALUE	275
#define	VALUE1	276
#define	VALUE2	277
#define	VALUE3	278
#define	VALUE4	279
#define	VALUE5	280
#define	VALUE6	281
#define	VALUE7	282
#define	VALUE8	283
#define	VALUE9	284
#define	COLON	285
#define	DOLLAR	286
#define	COMMA	287
#define	ASSIGN	288
#define	OP_PLUS	289
#define	OP_MINUS	290
#define	OP_TIMES	291
#define	OP_DIV	292
#define	OP_AND	293
#define	OP_OR	294
#define	OP_RIGHT	295
#define	OP_LEFT	296
#define	OP_MOD	297
#define	OP_NOT	298
#define	BRACKET_OPEN	299
#define	BRACKET_CLOSE	300
#define	SQR_BRACKET_OPEN	301
#define	SQR_BRACKET_CLOSE	302

#line 1 "dbyacc.y"


#include <ci_types.h>
#include <cinclude.h>
#include <winlib.h>
#include <sysmsg.h>
#include "db.h"


#define YYDEBUG 0

unsigned last_value = 0;


yyerror( char *s )
{
	wl_printf( debug_wid, "%s\n\r", s );
}



#line 24 "dbyacc.y"
typedef union {
	int 	num;		/* Found a numeric constant */
	char	*str;		/* Found an identifier */
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#ifndef YYDEBUG
#define YYDEBUG 1
#endif

#include <stdio.h>

#ifndef __STDC__
#define const
#endif



#define	YYFINAL		73
#define	YYFLAG		-32768
#define	YYNTBASE	48

#define YYTRANSLATE(x) ((unsigned)(x) <= 302 ? yytranslate[x] : 57)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47
};

static const short yyprhs[] = {     0,
     0,     1,     3,     5,     7,     9,    11,    13,    16,    18,
    23,    25,    27,    31,    33,    36,    38,    40,    43,    45,
    47,    50,    55,    57,    59,    62,    66,    70,    74,    78,
    82,    86,    88,    92,    96,   100,   102,   105,   107,   109,
   113,   115,   117,   119,   121,   123,   125,   127,   129
};

static const short yyrhs[] = {    -1,
    49,     0,    50,     0,    51,     0,    52,     0,     9,     0,
    10,     0,    11,    52,     0,     5,     0,     8,    52,    32,
    52,     0,     4,     0,    12,     0,    56,    33,    52,     0,
    13,     0,    14,    19,     0,    15,     0,    17,     0,    16,
    52,     0,     6,     0,     7,     0,     7,    52,     0,     7,
    52,    32,    52,     0,    53,     0,    54,     0,    43,    54,
     0,    53,    34,    54,     0,    53,    35,    54,     0,    53,
    38,    54,     0,    53,    39,    54,     0,    53,    40,    54,
     0,    53,    41,    54,     0,    55,     0,    54,    36,    55,
     0,    54,    37,    55,     0,    54,    42,    55,     0,    18,
     0,    35,    18,     0,    20,     0,    56,     0,    44,    53,
    45,     0,    21,     0,    22,     0,    23,     0,    24,     0,
    25,     0,    26,     0,    27,     0,    28,     0,    29,     0
};

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    87,    88,    92,    93,    94,    99,   103,   107,   111,   115,
   119,   123,   127,   131,   135,   139,   143,   147,   154,   161,
   165,   169,   177,   181,   182,   183,   184,   185,   186,   187,
   188,   192,   193,   194,   195,   199,   200,   201,   202,   203,
   208,   209,   210,   211,   212,   213,   214,   215,   216
};

static const char * const yytname[] = {   "$",
"error","$illegal.","TOKEN_UNDEF","COMMAND_HELP","COMMAND_CLS","COMMAND_DISP_REGS","COMMAND_DUMP","COMMAND_TRANSLATE","COMMAND_GDT","COMMAND_PS",
"COMMAND_WAKEUP","COMMAND_VAR","COMMAND_EXIT","COMMAND_LOAD_EXE","COMMAND_DISP_SYMS","COMMAND_SHOW","COMMAND_STEP","NUMBER","STRING","LAST_VALUE",
"VALUE1","VALUE2","VALUE3","VALUE4","VALUE5","VALUE6","VALUE7","VALUE8","VALUE9","COLON",
"DOLLAR","COMMA","ASSIGN","OP_PLUS","OP_MINUS","OP_TIMES","OP_DIV","OP_AND","OP_OR","OP_RIGHT",
"OP_LEFT","OP_MOD","OP_NOT","BRACKET_OPEN","BRACKET_CLOSE","SQR_BRACKET_OPEN","SQR_BRACKET_CLOSE","prg","debug_command","disp_regs",
"dump_memory","expr","expr1","term","factor","db_value",""
};
#endif

static const short yyr1[] = {     0,
    48,    48,    49,    49,    49,    49,    49,    49,    49,    49,
    49,    49,    49,    49,    49,    49,    49,    49,    50,    51,
    51,    51,    52,    53,    53,    53,    53,    53,    53,    53,
    53,    54,    54,    54,    54,    55,    55,    55,    55,    55,
    56,    56,    56,    56,    56,    56,    56,    56,    56
};

static const short yyr2[] = {     0,
     0,     1,     1,     1,     1,     1,     1,     2,     1,     4,
     1,     1,     3,     1,     2,     1,     1,     2,     1,     1,
     2,     4,     1,     1,     2,     3,     3,     3,     3,     3,
     3,     1,     3,     3,     3,     1,     2,     1,     1,     3,
     1,     1,     1,     1,     1,     1,     1,     1,     1
};

static const short yydefact[] = {     1,
    11,     9,    19,    20,     0,     6,     7,     0,    12,    14,
     0,    16,     0,    17,    36,    38,    41,    42,    43,    44,
    45,    46,    47,    48,    49,     0,     0,     0,     2,     3,
     4,     5,    23,    24,    32,    39,    21,    39,     0,     8,
    15,    18,    37,    25,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    40,    26,    27,
    28,    29,    30,    31,    33,    34,    35,    13,    22,    10,
     0,     0,     0
};

static const short yydefgoto[] = {    71,
    29,    30,    31,    32,    33,    34,    35,    38
};

static const short yypact[] = {     6,
-32768,-32768,-32768,    36,    36,-32768,-32768,    36,-32768,-32768,
    17,-32768,    36,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,     7,    63,    36,-32768,-32768,
-32768,-32768,     4,   -34,-32768,    13,     5,-32768,     8,-32768,
-32768,-32768,-32768,   -34,    74,    63,    63,    63,    63,    63,
    63,    63,    63,    63,    36,    36,    36,-32768,   -34,   -34,
   -34,   -34,   -34,   -34,-32768,-32768,-32768,-32768,-32768,-32768,
    47,    48,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,    -4,    27,    55,   -47,    66
};


#define	YYLAST		119


static const short yytable[] = {    37,
    39,    52,    53,    40,    65,    66,    67,    54,    42,     1,
     2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
    12,    13,    14,    15,    43,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    41,    56,    46,    47,    57,
    26,    48,    49,    50,    51,    55,    72,    73,    27,    28,
    68,    69,    70,    15,    45,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    36,     0,     0,     0,     0,
    26,     0,     0,     0,     0,     0,     0,     0,    27,    28,
    15,    44,    16,    17,    18,    19,    20,    21,    22,    23,
    24,    25,     0,     0,     0,     0,     0,    26,     0,     0,
    59,    60,    61,    62,    63,    64,    28,    46,    47,     0,
     0,    48,    49,    50,    51,     0,     0,     0,    58
};

static const short yycheck[] = {     4,
     5,    36,    37,     8,    52,    53,    54,    42,    13,     4,
     5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
    15,    16,    17,    18,    18,    20,    21,    22,    23,    24,
    25,    26,    27,    28,    29,    19,    32,    34,    35,    32,
    35,    38,    39,    40,    41,    33,     0,     0,    43,    44,
    55,    56,    57,    18,    28,    20,    21,    22,    23,    24,
    25,    26,    27,    28,    29,     0,    -1,    -1,    -1,    -1,
    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
    18,    27,    20,    21,    22,    23,    24,    25,    26,    27,
    28,    29,    -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,
    46,    47,    48,    49,    50,    51,    44,    34,    35,    -1,
    -1,    38,    39,    40,    41,    -1,    -1,    -1,    45
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* Not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__)
#include <alloca.h>
#else /* Not sparc */
#ifdef MSDOS
#include <malloc.h>
#endif /* MSDOS */
#endif /* Not sparc.  */
#endif /* Not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#define YYLEX		yylex(&yylval, &yylloc)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_bcopy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_bcopy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif

#line 160 "bison.simple"
int
yyparse()
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yysp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yysp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
#ifdef YYLSP_NEEDED
		 &yyls1, size * sizeof (*yylsp),
#endif
		 &yystacksize);

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Next token is %d (%s)\n", yychar, yytname[yychar1]);
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symboles being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 5:
#line 95 "dbyacc.y"
{
			wl_printf( debug_wid, "Hex = %32x\n\rDec = %32u\n\r",
									yyvsp[0].num, yyvsp[0].num, yyvsp[0].num );
		;
    break;}
case 6:
#line 100 "dbyacc.y"
{
			db_show_gdt();
		;
    break;}
case 7:
#line 104 "dbyacc.y"
{
			db_show_pcb();
		;
    break;}
case 8:
#line 108 "dbyacc.y"
{
			db_wakeup( (PID) yyvsp[0].num );
		;
    break;}
case 9:
#line 112 "dbyacc.y"
{
			wl_clrscr( debug_wid );
		;
    break;}
case 10:
#line 116 "dbyacc.y"
{
			db_translate( yyvsp[-2].num, yyvsp[0].num );
		;
    break;}
case 11:
#line 120 "dbyacc.y"
{
			db_help_debug();
		;
    break;}
case 12:
#line 124 "dbyacc.y"
{
			db_show_var();
		;
    break;}
case 13:
#line 128 "dbyacc.y"
{
			db_value[ yyvsp[-2].num ] = yyvsp[0].num;
		;
    break;}
case 14:
#line 132 "dbyacc.y"
{
			exit( 0 );
		;
    break;}
case 15:
#line 136 "dbyacc.y"
{
			syms_init( yyvsp[0].str );
		;
    break;}
case 16:
#line 140 "dbyacc.y"
{
			syms_list( 0 );
		;
    break;}
case 17:
#line 144 "dbyacc.y"
{
			single_step();
		;
    break;}
case 18:
#line 148 "dbyacc.y"
{
			display_source( yyvsp[0].num );
		;
    break;}
case 19:
#line 155 "dbyacc.y"
{
			db_display_regs();
		;
    break;}
case 20:
#line 162 "dbyacc.y"
{
			db_hex_dump( 0xffffffff, 0x100 );
		;
    break;}
case 21:
#line 166 "dbyacc.y"
{
			db_hex_dump( yyvsp[0].num, 0x100 );
		;
    break;}
case 22:
#line 170 "dbyacc.y"
{
			db_hex_dump( yyvsp[-2].num, yyvsp[0].num );
		;
    break;}
case 23:
#line 177 "dbyacc.y"
{ yyval.num = yyvsp[0].num; last_value = yyvsp[0].num; ;
    break;}
case 24:
#line 181 "dbyacc.y"
{ yyval.num = yyvsp[0].num; ;
    break;}
case 25:
#line 182 "dbyacc.y"
{ yyval.num = ~yyvsp[0].num; ;
    break;}
case 26:
#line 183 "dbyacc.y"
{ yyval.num = yyvsp[-2].num + yyvsp[0].num; ;
    break;}
case 27:
#line 184 "dbyacc.y"
{ yyval.num = yyvsp[-2].num - yyvsp[0].num; ;
    break;}
case 28:
#line 185 "dbyacc.y"
{ yyval.num = yyvsp[-2].num & yyvsp[0].num; ;
    break;}
case 29:
#line 186 "dbyacc.y"
{ yyval.num = yyvsp[-2].num | yyvsp[0].num; ;
    break;}
case 30:
#line 187 "dbyacc.y"
{ yyval.num = yyvsp[-2].num >> yyvsp[0].num; ;
    break;}
case 31:
#line 188 "dbyacc.y"
{ yyval.num = yyvsp[-2].num << yyvsp[0].num; ;
    break;}
case 32:
#line 192 "dbyacc.y"
{ yyval.num = yyvsp[0].num; ;
    break;}
case 33:
#line 193 "dbyacc.y"
{ yyval.num = yyvsp[-2].num * yyvsp[0].num; ;
    break;}
case 34:
#line 194 "dbyacc.y"
{ yyval.num = yyvsp[-2].num / yyvsp[0].num; ;
    break;}
case 35:
#line 195 "dbyacc.y"
{ yyval.num = yyvsp[-2].num % yyvsp[0].num; ;
    break;}
case 36:
#line 199 "dbyacc.y"
{ yyval.num = yyvsp[0].num; ;
    break;}
case 37:
#line 200 "dbyacc.y"
{ yyval.num = -yyvsp[0].num; ;
    break;}
case 38:
#line 201 "dbyacc.y"
{ yyval.num = last_value; ;
    break;}
case 39:
#line 202 "dbyacc.y"
{ yyval.num = db_value[ yyvsp[0].num ]; ;
    break;}
case 40:
#line 204 "dbyacc.y"
{ yyval.num = yyvsp[-1].num; ;
    break;}
case 41:
#line 208 "dbyacc.y"
{ yyval.num = 0; ;
    break;}
case 42:
#line 209 "dbyacc.y"
{ yyval.num = 1; ;
    break;}
case 43:
#line 210 "dbyacc.y"
{ yyval.num = 2; ;
    break;}
case 44:
#line 211 "dbyacc.y"
{ yyval.num = 3; ;
    break;}
case 45:
#line 212 "dbyacc.y"
{ yyval.num = 4; ;
    break;}
case 46:
#line 213 "dbyacc.y"
{ yyval.num = 5; ;
    break;}
case 47:
#line 214 "dbyacc.y"
{ yyval.num = 6; ;
    break;}
case 48:
#line 215 "dbyacc.y"
{ yyval.num = 7; ;
    break;}
case 49:
#line 216 "dbyacc.y"
{ yyval.num = 8; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 423 "bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  for (x = 0; x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) xmalloc(size + 15);
	  strcpy(msg, "parse error");

	  if (count < 5)
	    {
	      count = 0;
	      for (x = 0; x < (sizeof(yytname) / sizeof(char *)); x++)
		if (yycheck[x + yyn] == x)
		  {
		    strcat(msg, count == 0 ? ", expecting `" : " or `");
		    strcat(msg, yytname[x]);
		    strcat(msg, "'");
		    count++;
		  }
	    }
	  yyerror(msg);
	  free(msg);
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 218 "dbyacc.y"
