
DIRS = nucleus clib appl.os appl tools


FATFORMAT  = tools/fat/fatformat
FATCOPY    = tools/fat/fatcopy
DISK_IMAGE = hermes_disk
BINARIES = nucleus/kernel.img appl/db/db appl/shell/shell appl/user/user \
           appl/user/tetris appl.os/boot/boot appl.os/window/window tools/loader/load.exe



alll:
	#for i in $(DIRS); do make -C $$i || exit 1; done
	rm -f $(DISK_IMAGE)
	#$(FATFORMAT) $(DISK_IMAGE) size=DISC
	cp freedos_image $(DISK_IMAGE)
	for i in $(BINARIES); do $(FATCOPY) $(DISK_IMAGE) $$i /`basename $$i`; done


all:
	for i in $(DIRS); do make -C $$i || exit 1; done
	rm -f $(DISK_IMAGE)
	$(FATFORMAT) $(DISK_IMAGE) size=DISC
	for i in $(BINARIES); do $(FATCOPY) $(DISK_IMAGE) $$i /`basename $$i`; done


clean:
	for i in $(DIRS); do make -C $$i clean || exit 1; done


depend:
	for i in $(DIRS); do make -C $$i .depend || exit 1; done

.depend:
	for i in $(DIRS); do make -C $$i .depend || exit 1; done
