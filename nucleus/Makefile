
LD = ld
LD_OPT = -nostdlib -Ttext 0

DIRS = kernel vulture mouse timer event keyb fs

LIB_OBJS = kernel.o vulture.o mouse.o timer.o event.o keyb.o fs.o



all:
	for i in $(DIRS); do make -C $$i || exit 1; done
	$(LD) $(LD_OPT) -o kernel.img $(LIB_OBJS)


clean:
	for i in $(DIRS); do make -C $$i clean || exit 1; done
	rm -f kernel.img *.o


.depend:
	for i in $(DIRS); do make -C $$i .depend || exit 1; done
