#
# $Id: Makefile,v 1.10 2000/08/24 15:31:59 frank Exp $
#

include nan.mk

ifeq ($(OS),linux)
#    ifeq ($(CPU),alpha)
#	INCLUDE += -I/usr/src/Mesa-3.1/include/
#    endif
    ifeq ($(CPU),i386)
	INCLUDE += -I/usr/src/Mesa-3.1/include/
    endif
    ifeq ($(CPU),powerpc)
	INCLUDE += -I/usr/src/MesaCVS/include/
    endif
endif

ifeq ($(OS),solaris)
# the include below is needed on sun
    INCLUDE += -I/usr/src/Mesa-3.0/include/
# and this one is needed by ultra
    INCLUDE += -I/usr/local/include/
# they both are included here for compatiblity
endif

DIR = $(HOM)/glut
VPATH = $(DIR)

OBJS = \
	glut_bitmap.o glut_bwidth.o glut_cindex.o \
	glut_cmap.o glut_cursor.o glut_dials.o glut_dstr.o glut_draw.o \
	glut_event.o \
	glut_ext.o glut_fullscrn.o glut_get.o  \
	glut_helb8.o glut_helb10.o glut_helb12.o  \
	glut_hel10.o glut_hel12.o glut_hel18.o  \
	glut_init.o glut_input.o glut_menu.o glut_mesa.o glut_modifier.o \
	glut_overlay.o glut_scr12.o glut_scr15.o \
	glut_space.o glut_swap.o glut_swidth.o \
	glut_util.o \
	glut_vidresize.o glut_warp.o glut_win.o glut_winmisc.o layerutil.o

.c.o:
	$(CC) -c -o $(DIR)/$@ $<

all: makedir libglut.a

install: all
	@echo "****> install libglut.a in $(BCGDIR)/lib/"
	@cp -f $(DIR)/*.a $(BCGDIR)/lib/

makedir: FORCE
	@[ -d $(DIR) ] || mkdir $(DIR)

libglut.a : $(OBJS)
	(cd $(DIR) && \
	ar rv $@ $(OBJS) && \
	$(RANLIB) )

clean:
	cd $(DIR) && rm -f $(OBJS) libglut.a

.SUFFIXES: .c .o

FORCE:
