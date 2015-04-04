#
# $Id: Makefile,v 1.9 2000/08/25 15:27:32 nzc Exp $
#

include nan.mk

ifeq ($(OS),beos)
CFLAGS += -DWORDS_LITTLEENDIAN -fPIC -funsigned-char
endif

ifeq ($(OS),freebsd)
CFLAGS += -DWORDS_LITTLEENDIAN
endif

ifeq ($(OS),irix)
CFLAGS += -DWORDS_BIGENDIAN -fullwarn
endif

ifeq ($(OS),linux)
CFLAGS += -DWORDS_LITTLEENDIAN
endif

ifeq ($(OS),solaris)
CFLAGS += -DWORDS_BIGEENDIAN
endif

ifeq ($(OS),windows)
CFLAGS += -DWORDS_LITTLEENDIAN
endif

DIR = $(HOM)/avi
VPATH = $(DIR)

#RANLIB = ranlib libavi.a

OBJS = avi.o avirgb.o codecs.o endian.o options.o jmf.o mjpeg.o rgb32.o

all: makedir libavi.a

install: all
	@echo "****> install libavi.a in $(BCGDIR)/lib/"
	@cp -f $(DIR)/*.a $(BCGDIR)/lib/

makedir: FORCE
	@[ -d $(DIR) ] || mkdir $(DIR)

libavi.a: $(OBJS)
	(cd $(DIR) && \
	ar rv libavi.a $(OBJS) && \
	$(RANLIB) )

clean:
	cd $(DIR) && rm -f *.o libavi.a libavi.so

$(OBJS): jmf.h avirgb.h mjpeg.h endian.h rgb32.h

.c.o:
	$(CC) -c -o $(DIR)/$@ $<

.SUFFIXES: .c .o

FORCE:
