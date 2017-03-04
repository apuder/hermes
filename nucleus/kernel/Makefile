
CC = gcc
LD = ld
MAKEDEPEND = makedepend -f-

INCLUDES = -I../include -I../../clib/include

LD_OPT = -r

# PS/2 Mouse
CC_OPT = -Wall $(INCLUDES) -O -DUSE_PS2_MOUSE -fomit-frame-pointer -fno-defer-pop

# Serial Mouse
#CC_OPT = -Wall $(INCLUDES) -O -DUSE_SERIAL_MOUSE -fomit-frame-pointer -fno-defer-pop


# delete builtin rules
%.o: %.c
%.o: %.s

OBJS = startup.o main.o gdt.o idt.o intr.o 386.o params.o paging.o panic.o \
	   dispatch.o process.o port.o sema.o send.o receive.o message.o \
	   reply.o copy.o event.o printf.o basicio.o pfault.o boot.o \
	   kmemcpy.o inout.o sbrk.o testrcv.o suspend.o ktrap.o userproc.o \
	   tss.o debug.o version.o termin.o

%.o: %.s
	$(CC) $(CC_OPT) -o $@ -c $<

%.o: %.c .depend
	$(CC) $(CC_OPT) -o $@ -c $<

all : $(OBJS)
	$(LD) $(LD_OPT) -o ../kernel.o $(OBJS)

#                                      @$(SRC_PATH)/obj_list


# All the following file have function with the following pattern:
#
#   f()
#   {
#       ...
#       asm ("iret");
#   }
#
# In this case we by-pass the epilogue code of function f()
# The problem is that gcc aligns %esp to multiple of 16,
# which can cause problems since we do not know inside our
# inline assembly what the current alignment is. We therefore
# force gcc to forget about stack alignment for those files.

dispatch.o: dispatch.c .depend
	$(CC) $(CC_OPT) -mpreferred-stack-boundary=2 -o dispatch.o -c \
		dispatch.c

intr.o: intr.c .depend
	$(CC) $(CC_OPT) -mpreferred-stack-boundary=2 -o intr.o -c \
		intr.c

idt.o:  idt.c .depend
	$(CC) $(CC_OPT) -mpreferred-stack-boundary=2 -o idt.o -c \
		idt.c

userproc.o:  userproc.c .depend
	$(CC) $(CC_OPT) -mpreferred-stack-boundary=2 -o userproc.o -c \
		userproc.c

clean :
	rm -f *.o *.bak .depend

ifeq (.depend, $(wildcard .depend))
include .depend
endif

.depend:
	echo '# Module dependencies' > .depend
	$(MAKEDEPEND) -- $(CC_OPT) $(INCLUDES) -- $(OBJS:.o=.c) >> .depend
