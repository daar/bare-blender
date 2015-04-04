#
# $Id: Makefile,v 1.6 2000/09/13 14:57:55 hans Exp $
#

include nan.mk

ifeq ($(OS),beos)
	CFLAGS += -fPIC -funsigned-char
endif

ifeq ($(OS),freebsd)
	CFLAGS += -funsigned-char
endif

ifeq ($(OS),irix)
	ifeq ($(OS_VERSION),6.2)
		CFLAGS += -float
	endif
endif

ifeq ($(OS),linux)
	CFLAGS += -funsigned-char -O3
	ifeq ($(CPU),powerpc)
		INCLUDE += -I/usr/src/MesaCVS/include/
	endif
endif

ifeq ($(OS),solaris)
	CFLAGS += -funsigned-char
endif

ifeq ($(OS),windows)
# windows needs to find glut, and has difficulties doing so
	CFLAGS += -funsigned-char -I../../../glut-win
endif

DIR = $(HOM)/render
VPATH = $(DIR)

# first /include is my own includes, second are the external includes
# third is the external interface. there should be a nicer way to say this 
CFLAGS += -I../include -I../../../include -I../../extern/include

SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)

all: makedir librender.a

install: all
	@echo "****> install render.a in $(BCGDIR)/lib/"
	@cp -f $(DIR)/*.a $(BCGDIR)/lib/

makedir: FORCE
	@[ -d $(DIR) ] || mkdir $(DIR)

librender.a: $(OBJS)
	(cd $(DIR) && \
	ar rv librender.a $(OBJS) && \
	$(RANLIB) )

clean:
	cd $(DIR) && rm -f *.o librender.a librender.so

$(OBJS): ../../extern/include/render.h       \
		../../extern/include/render_types.h  \
		../include/zbuf.h                    \
		../include/zbuf_int.h                \
		../include/zbuf_types.h              \
		../include/vanillaRenderPipe.h       \
		../include/vanillaRenderPipe_int.h   \
		../include/vanillaRenderPipe_types.h \
		../include/texture.h                 \
		../include/shadbuf.h                 \
		../include/renderfg.h                \
		../include/rendercore_int.h          \
		../include/rendercore.h              \
		../include/previewrender.h           \
		../include/pixelblending_types.h     \
		../include/pixelblending.h           \
		../include/initrender.h              \
		../include/envmap.h

.c.o:
	$(CC) -c -o $(DIR)/$@ $<

.SUFFIXES: .c .o

FORCE:

# eof
