typedef union {
	int 	num;		/* Found a numeric constant */
	char	*str;		/* Found an identifier */
} YYSTYPE;
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


extern YYSTYPE yylval;
