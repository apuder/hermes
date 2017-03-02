
CC = gcc
LD = ld
MAKEDEPEND = makedepend -f-

 INCLUDES = -DKEYBOARD_PS2 -DLANG=US_EN -I../include -I../../clib/include
# INCLUDES = -I../include -I../../clib/include

LD_OPT = -r
CC_OPT = -Wall $(INCLUDES) -O -fomit-frame-pointer -fno-defer-pop

# delete builtin rules
%.o: %.c
%.o: %.s

OBJS = keyb.o


%.o: %.s
	$(CC) $(CC_OPT) -o $@ -c $<

%.o: %.c .depend
	$(CC) $(CC_OPT) -mpreferred-stack-boundary=2 -o $@ -c $<

all : $(OBJS)
	$(LD) $(LD_OPT) -o ../keyb.o $(OBJS)

clean :
	rm -f *.o *.bak .depend

ifeq (.depend, $(wildcard .depend))
include .depend
endif

.depend:
	echo '# Module dependencies' > .depend
	$(MAKEDEPEND) -- $(CC_OPT) $(INCLUDES) -- $(OBJS:.o=.c) >> .depend
