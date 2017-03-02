
CC = gcc
LD = ld
MAKEDEPEND = makedepend -f-

INCLUDES = -I../../clib/include
LIBS = -L../../clib

LD_OPT = $(LIBS) -N -Ttext 0 -nostdlib
CC_OPT = -fno-builtin $(INCLUDES) -I../../clib/include -O -Wall -D__HERMES__


OBJS     = c0boot.o boot.o

.c.o: .depend
	$(CC) $(CC_OPT) -o $@ -c $<

.s.o:
	$(CC) $(CC_OPT) -o $@ -c $<


boot: $(OBJS)
	$(LD) $(LD_OPT) $(OBJS) -o boot -lhermc

clean :
	rm -f *.o *.bak .depend

ifeq (.depend, $(wildcard .depend))
include .depend
endif

.depend:
	echo '# Module dependencies' > .depend
	$(MAKEDEPEND) -- $(CC_OPT) $(INCLUDES) -- $(OBJS:.o=.c) >> .depend
