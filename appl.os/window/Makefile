
CC = gcc
LD = ld
MAKEDEPEND = makedepend -f-

INCLUDES = -I../../clib/include
LIBS = -L../../clib

LD_OPT = $(LIBS) -N -Ttext 0 -nostdlib
#CC_OPT = -fno-builtin $(INCLUDES) -D__HERMES__ -DALLOW_GR_MOUSE -O -Wall
CC_OPT = -fno-builtin $(INCLUDES) -D__HERMES__ -O -Wall

OBJS = win_serv.o ws_intrn.o ws_mouse.o specials.o mevent.o manager.o

.c.o: .depend
	$(CC) $(CC_OPT) -o $@ -c $<

.s.o:
	$(CC) $(CC_OPT) -o $@ -c $<

window: $(OBJS)
	$(LD) $(LD_OPT) ../../clib/lib/c0.o $(OBJS) -o window -lhermc

clean :
	rm -f *.o *.bak .depend

ifeq (.depend, $(wildcard .depend))
include .depend
endif

.depend:
	echo '# Module dependencies' > .depend
	$(MAKEDEPEND) -- $(CC_OPT) $(INCLUDES) -- $(OBJS:.o=.c) >> .depend
