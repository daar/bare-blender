#
# $Id: Makefile,v 1.8 2000/05/18 10:12:02 hans Exp $
#

include nan.mk

ifeq ($(OS),beos)
CFLAGS += -funsigned-char -fPIC
endif

ifeq ($(OS),freebsd)
CFLAGS += -funsigned-char
endif

ifeq ($(OS),linux)
CFLAGS += -funsigned-char
endif

ifeq ($(OS),solaris)
CFLAGS += -D__SUN
endif

ifeq ($(OS),windows)
CFLAGS += -funsigned-char
endif

DIR = $(HOM)/misc
VPATH = $(DIR)

OBJ=util.o storage.o

.c.o:
	$(CC) -c -o $(DIR)/$@ $< 

all: makedir libmisc.a

install: all
	@echo "****> install libmisc.a in $(BCGDIR)/lib/"
	@cp -f $(DIR)/*.a $(BCGDIR)/lib/

makedir: FORCE
	@[ -d $(DIR) ] || mkdir $(DIR)

libmisc.a: $(OBJ)
	(cd $(DIR) && \
	ar rv $@ $(OBJ) && \
	$(RANLIB) )

clean:
	cd $(DIR) && rm -f libmisc.a $(OBJ)

.SUFFIXES: .c .o

FORCE:
