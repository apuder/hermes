
RANLIB = ranlib
CC = gcc
AR = ar
MAKEDEPEND = makedepend -f-

INCLUDES = -I../include -I../../nucleus/include

CC_OPT = -fno-builtin $(INCLUDES) -D__HERMES__ -O -Wall
AR_OPT = ruvcs


LIB_OBJS = bcmp.o bcopy.o bzero.o ffs.o index.o memccpy.o memchr.o memcmp.o \
           memcpy.o memset.o rindex.o strcasec.o strcat.o strchr.o strcmp.o \
           strcoll.o strcpy.o strcspn.o strdup.o strerror.o strftime.o \
           strlen.o strlwr.o strncat.o strncmp.o strncpy.o strpbrk.o strrchr.o \
           strsep.o strspn.o strstr.o strtok.o strupr.o strxfrm.o swab.o \
           _bcopy.o _memcpy.o 

.s.o:
	$(CC) $(CC_OPT) -o $@ -c $<


%.o: %.c .depend
	$(CC) $(CC_OPT) -o $@ -c $<


all : $(LIB_OBJS)
	$(AR) $(AR_OPT) ../libhermc.a $(LIB_OBJS)
	$(RANLIB) ../libhermc.a


clean :
	rm -f *.o *.bak .depend

ifeq (.depend, $(wildcard .depend))
include .depend
endif

.depend:
	echo '# Module dependencies' > .depend
	$(MAKEDEPEND) -- $(CC_OPT) $(INCLUDES) -- $(LIB_OBJS:.o=.c) >> .depend
