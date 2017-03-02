NASM = nasm
DD = dd
MAKEDEPEND = makedepend -f-

all: boot.bin second-stage

boot.bin: boot.s
	$(NASM) boot.s -o boot.bin

second-stage: second-stage.s
	$(NASM) second-stage.s -o boot.sec

clean :
	rm -f *.o *.bak .depend

ifeq (.depend, $(wildcard .depend))
include .depend
endif

.depend:
	echo '# Module dependencies' > .depend
	$(MAKEDEPEND) >> .depend
