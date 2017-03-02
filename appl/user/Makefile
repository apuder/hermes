
CC = gcc
LD = ld
MAKEDEPEND = makedepend -f-

INCLUDES = -I../../clib/include
LIBS = -L../../clib

LD_OPT = $(LIBS) -N -Ttext 0 -nostdlib
CC_OPT = -fno-builtin $(INCLUDES) -D__HERMES__ -O -Wall


%.o: %.c .depend
	$(CC) $(CC_OPT) -o $@ -c $<

.s.o:
	$(CC) $(CC_OPT) -o $@ -c $<

all: tetris user

tetris: tetris.o button.o
	$(LD) $(LD_OPT) ../../clib/lib/c0.o tetris.o button.o -o tetris -lhermc

user: user.o
	$(LD) $(LD_OPT) ../../clib/lib/c0.o user.o -o user -lhermc

clean :
	rm -f *.o *.bak .depend

ifeq (.depend, $(wildcard .depend))
include .depend
endif

.depend:
	echo '# Module dependencies' > .depend
	$(MAKEDEPEND) -- $(CC_OPT) $(INCLUDES) -- tetris.c user.c button.c >> .depend
