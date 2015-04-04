#
# $Id: Makefile,v 1.8 2000/05/18 10:11:59 hans Exp $
#

include nan.mk

ifeq ($(OS),beos)
CFLAGS += -fPIC -funsigned-char
endif

ifeq ($(OS),freebsd)
CFLAGS += -funsigned-char
endif

ifeq ($(OS),linux)
CFLAGS += -funsigned-char
endif

ifeq ($(OS),solaris)
CFLAGS += -funsigned-char -D__SUN
endif

ifeq ($(OS),windows)
CFLAGS += -funsigned-char
endif


DIR = $(HOM)/imbuf
VPATH = $(DIR)

#RANLIB = ranlib libimbuf.a

OBJS = \
allocimbuf.o amiga.o anim.o antialias.o bitplanes.o cdi.o cmap.o \
cspace.o data.o delta.o dither.o divers.o ham.o hamx.o filter.o iff.o \
iris.o izoom.o jpeg.o makecmap.o mdec_decode.o readimage.o rectop.o \
rotate.o scaling.o targa.o tiff.o tim.o troep.o util.o writeimage.o \

all: makedir libimbuf.a

install: all
	@echo "****> install libimbuf.a in $(BCGDIR)/lib/"
	@cp -f $(DIR)/*.a $(BCGDIR)/lib/

makedir: FORCE
	@[ -d $(DIR) ] || mkdir $(DIR)

libimbuf.a: $(OBJS) $(JPGLIB)
	(cd $(DIR) && \
	ar rv libimbuf.a $(JPGOBS) $(OBJS) && \
	$(RANLIB) )

clean:
	cd $(DIR) && rm -f *.o libimbuf.a

.c.o:
	$(CC) -c -o $(DIR)/$@ $<
	
.SUFFIXES: .c .o

FORCE:
