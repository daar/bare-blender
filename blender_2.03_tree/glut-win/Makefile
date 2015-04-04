#
# $Id: Makefile,v 1.4 2000/05/18 10:11:59 hans Exp $
#

include nan.mk

DIR = $(HOM)/glut
VPATH = $(DIR)

OBJS = \
glut_8x13.o glut_9x15.o glut_bitmap.o glut_blender.o glut_bwidth.o \
glut_cindex.o glut_cmap.o glut_cursor.o glut_dials.o glut_dstr.o \
glut_event.o glut_ext.o glut_fullscrn.o glut_get.o glut_hel10.o \
glut_hel12.o glut_hel18.o glut_helb8.o glut_helb10.o glut_helb12.o \
glut_helb14.o glut_init.o glut_input.o win32_menu.o glut_mesa.o \
glut_modifier.o glut_mroman.o glut_overlay.o glut_roman.o \
glut_shapes.o glut_space.o glut_stroke.o glut_swap.o glut_swidth.o \
glut_tablet.o glut_teapot.o glut_tr10.o glut_tr24.o glut_util.o \
glut_vidresize.o glut_warp.o glut_win.o glut_winmisc.o win32_util.o \
win32_glx.o win32_x11.o glut_scr12.o glut_scr14.o glut_scr15.o glut_draw.o

all: makedir libglut.a

install: all
	@echo "****> install libglut.a in $(BCGDIR)/lib/"
	@cp -f $(DIR)/*.a $(BCGDIR)/lib/

makedir: FORCE
	@[ -d $(DIR) ] || mkdir $(DIR)

libglut.a: $(OBJS)
	(cd $(DIR) && \
	ar rv libglut.a $(OBJS) && \
	$(RANLIB) )

clean:
	cd $(DIR) && rm -f *.o libglut.a

.c.o:
	$(CC) -c -o $(DIR)/$@ $<

.SUFFIXES: .c .o

FORCE:
