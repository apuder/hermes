
DIRS = fat boot


all:
	for i in $(DIRS); do make -C $$i || exit 1; done


clean:
	for i in $(DIRS); do make -C $$i clean || exit 1; done


.depend:
	for i in $(DIRS); do make -C $$i .depend || exit 1; done
