
RANLIB = ranlib
DIRS = lib str winlib sys io gnu gen


all:
	for i in $(DIRS); do make -C $$i || exit 1; done
	$(RANLIB) libhermc.a


clean:
	for i in $(DIRS); do make -C $$i clean || exit 1; done
	rm -f libhermc.a


.depend:
	for i in $(DIRS); do make -C $$i .depend || exit 1; done
