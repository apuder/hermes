
RANLIB = ranlib
CC = gcc
AR = ar
MAKEDEPEND = makedepend -f-

INCLUDES = -I../include -I../../nucleus/include

CC_OPT = -fno-builtin $(INCLUDES) -D__HERMES__ -O -Wall
AR_OPT = ruvcs


LIB_OBJS = ctime.o difftime.o time.o timezone.o

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
