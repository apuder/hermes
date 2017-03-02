
RANLIB = ranlib
CC = gcc
AR = ar
MAKEDEPEND = makedepend -f-

INCLUDES = -I../include -I../../nucleus/include

CC_OPT = -fno-builtin $(INCLUDES) -D__HERMES__ -O -Wall
AR_OPT = ruvcs


LIB_OBJS = fopen.o findiop.o flsbuf.o fgets.o filbuf.o freopen.o fprintf.o \
           printf.o doprnt.o fputs.o fgetc.o fputc.o fread.o fwrite.o \
           sprintf.o vprintf.o vsprintf.o vfprintf.o fdopen.o scanf.o \
           doscan.o ungetc.o clrerr.o ftell.o fseek.o getchar.o gets.o \
           getw.o putchar.o puts.o putw.o rew.o setbuf.o setbuffe.o setvbuf.o

.s.o:
	$(CC) $(CC_OPT) -o $@ -c $<


%.o: %.c .depend
	$(CC) $(CC_OPT) -o $@ -c $<


all : $(LIB_OBJS)
	$(AR) $(AR_OPT) ../libhermc.a $(LIB_OBJS)
	$(RANLIB) ../libhermc.a


clean :
	rm -f *.o *.bak .depend

ifeq (.depend, $(wildcard .depend))
include .depend
endif

.depend:
	echo '# Module dependencies' > .depend
	$(MAKEDEPEND) -- $(CC_OPT) $(INCLUDES) -- $(LIB_OBJS:.o=.c) >> .depend
