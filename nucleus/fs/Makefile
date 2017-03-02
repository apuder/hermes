
CC = gcc
LD = ld
MAKEDEPEND = makedepend -f-

INCLUDES = -I../include -I../../clib/include

LD_OPT = -N -r
CC_OPT = -fomit-frame-pointer -fno-defer-pop $(INCLUDES) -O -Wall -D__HERMES__

# delete builtin rules
%.o: %.c
%.o: %.s

OBJS = fat.o fd.o

%.o: %.c .depend
	$(CC) $(CC_OPT) -mpreferred-stack-boundary=2 -o $@ -c $<

%.o: %.s
	$(CC) $(CC_OPT) -o $@ -c $<

all : $(OBJS)
	$(LD) $(LD_OPT) -o ../fs.o $(OBJS)

clean :
	rm -f *.o *.bak .depend

ifeq (.depend, $(wildcard .depend))
include .depend
endif

.depend:
	echo '# Module dependencies' > .depend
	$(MAKEDEPEND) -- $(CC_OPT) $(INCLUDES) -- $(OBJS:.o=.c) >> .depend
