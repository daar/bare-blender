#
# $Id: Makefile,v 1.6 2000/09/13 15:03:07 hans Exp $
#

include nan.mk

DIR = $(HOM)/sound
VPATH = $(DIR)

# first  ../include            our internal includes
# second ../../extern/include/ our external includes (API)
INCLUDE += -I../include/ -I../../extern/include/

INCLUDE += -I$(SRCHOME)/blender2/include/

ifeq ($(OS),freebsd)
    CFLAGS += -DUSE_OPENAL -Wall
    INCLUDE +=  -I$(SRCHOME)/extern/openal/distribution/include/
    INCLUDE += -I$(SRCHOME)/extern/openal/distribution/linux/include/
endif
ifeq ($(OS),linux)
    ifeq ($(CPU),i386)
	CFLAGS += -DUSE_OPENAL -Wall
	INCLUDE +=  -I$(SRCHOME)/extern/openal/distribution/include/
	INCLUDE += -I$(SRCHOME)/extern/openal/distribution/linux/include/
    endif
endif
ifeq ($(OS),windows)
    CFLAGS += -DUSE_OPENAL
    INCLUDE += -I$(SRCHOME)/extern/openal/distribution/win/
endif

SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)

all: makedir libsound.a

install: all
	@echo "****> install libsound.a in $(BCGDIR)/lib/"
	@cp -f $(DIR)/*.a $(BCGDIR)/lib/

makedir: FORCE
	@[ -d $(DIR) ] || mkdir $(DIR)

libsound.a: $(OBJS)
	cd $(DIR) && ar rv libsound.a $(OBJS)

clean:
	cd $(DIR) && rm -f *.o libsound.a

.c.o:
	$(CC) -c -o $(DIR)/$@ $<

.SUFFIXES: .c .o

FORCE:
