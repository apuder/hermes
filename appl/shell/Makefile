
CC = gcc
LD = ld
MAKEDEPEND = makedepend -f-

INCLUDES = -I../../clib/include
LIBS = -L../../clib

LD_OPT = $(LIBS) -N -Ttext 0 -nostdlib
CC_OPT = $(INCLUDES) -D__HERMES__ -O -Wall

OBJS = shell.o

%.o: %.c .depend
	$(CC) $(CC_OPT) -o $@ -c $<

.s.o:
	$(CC) $(CC_OPT) -o $@ -c $<

shell: $(OBJS)
	$(LD) $(LD_OPT) ../../clib/lib/c0.o $(OBJS) -o shell -lhermc

clean :
	rm -f *.o *.bak .depend

ifeq (.depend, $(wildcard .depend))
include .depend
endif

.depend:
	echo '# Module dependencies' > .depend
	$(MAKEDEPEND) -- $(CC_OPT) $(INCLUDES) -- $(OBJS:.o=.c) >> .depend
