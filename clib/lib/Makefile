
RANLIB = ranlib
CC = gcc
AR = ar
MAKEDEPEND = makedepend -f-

INCLUDES = -I../include -I../../nucleus/include

CC_OPT = -fno-builtin $(INCLUDES) -D__HERMES__ -O -Wall
AR_OPT = ruvcs


LIB_OBJS = ostrap.o startup.o basicio.o hprintf.o malloc.o sbrk.o \
           inout.o memcpy.o srr.o exec.o atoi.o strtol.o ctype.o getenv.o \
           setenv.o putenv.o modf.o atof.o strtod.o exit.o atexit.o onexit.o \
           abort.o syserr.o abs.o atol.o bsearch.o div.o labs.o ldexp.o \
           ldiv.o qsort.o rand.o strtoul.o tolower.o errno.o c0.o

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
