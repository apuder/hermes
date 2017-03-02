
RANLIB = ranlib
CC = gcc
AR = ar
MAKEDEPEND = makedepend -f-

INCLUDES = -I../include -I../../nucleus/include

CC_OPT = -fno-builtin $(INCLUDES) -D__HERMES__ -O -Wall
AR_OPT = ruvcs


LIB_OBJS = open.o lseek.o close.o getdtabl.o write.o read.o fstat.o \
           dup.o dup2.o isatty.o setmode.o _exit.o

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
