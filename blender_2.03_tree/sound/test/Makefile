#
# $Id: Makefile,v 1.3 2000/09/13 15:03:07 hans Exp $
#

include nan.mk

DIR = $(HOM)/sound/test
VPATH = $(DIR)

INCLUDE += -I. -I../extern/include
INCLUDE += -I$(SRCHOME)/extern/openal/distribution/include

ifeq ($(OS),freebsd)
    CFLAGS += -g -Wall
    INCLUDE += -I$(SRCHOME)/extern/openal/distribution/linux/include
    LIBS = $(BCGDIR)/lib/libopenal.a -lm -pthread
endif
ifeq ($(OS),linux)
    CFLAGS += -g -Wall
    INCLUDE += -I$(SRCHOME)/extern/openal/distribution/linux/include
    LIBS = $(BCGDIR)/lib/libopenal.a -lm -lpthread -ldl
    # NOTE: -pthread works on i386, -lpthread is required for ppc
endif
ifeq ($(OS),windows)
    INCLUDE += -I$(SRCHOME)/extern/openal/distribution/win/
    LIBS = $(HOM)/openal/*/*.o
endif

PROGS = testopenal

TOSRCS = testopenal.c
TOOBJS = $(TOSRCS:%.c=%.o)

.c.o:
	$(CC) $(CFLAGS) -c -o $(DIR)/$@ $<

all test: makedir $(PROGS)

testopenal: $(TOOBJS)
	cd $(DIR) && $(CC) $(CFLAGS) $(INCLUDE) -o $@ $(TOOBJS) $(LIBS)
	@echo "====> Created $(DIR)/$@"
	
install:

makedir: FORCE
	@[ -d $(DIR) ] || mkdir $(DIR)

clean:
	cd $(DIR) && rm -f *.o $(PROGS)

.SUFFIXES: .c .o

FORCE:
