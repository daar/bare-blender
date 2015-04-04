#
# $Id: Makefile,v 1.5 2000/05/18 10:11:59 hans Exp $
#

include nan.mk

CFLAGS += -fPIC -funsigned-char

DIR = $(HOM)/glut
VPATH = $(DIR)

OBJS = \
    beos_x11.o glutBlocker.o glutCallback.o glutColor.o glutCursor.o \
    glutDraw.o glutDstr.o glutEvent.o glutGet.o glutInit.o glutMenu.o \
    glutOverlay.o glutWindow.o glut_8x13.o glut_9x15.o glut_bitmap.o \
    glut_bwidth.o glut_hel10.o glut_hel12.o glut_hel18.o glut_helb10.o \
    glut_helb12.o glut_helb14.o glut_helb8.o glut_mroman.o glut_roman.o \
    glut_shapes.o glut_stroke.o glut_swidth.o glut_teapot.o glut_tr10.o \
    glut_tr24.o glut_util.o glut_scr12.o glut_scr14.o glut_scr15.o \
    glut_sgib15.o

.c.o:
	$(CC) -c -o $(DIR)/$@ $<

.cpp.o:
	$(CC) -c -o $(DIR)/$@ $<

all: makedir libglut.a

install: all
	@echo "****> install libglut.a in $(BCGDIR)/lib/"
	@cp -f $(DIR)/*.a $(BCGDIR)/lib/

makedir: FORCE
	@[ -d $(DIR) ] || mkdir $(DIR)
	# NOTE: UGLY glut.h copy HACK :
	cp -f ../include/glut.h /boot/develop/headers/be/opengl/GL/

libglut.a : $(OBJS)
	(cd $(DIR) && \
	ar rv $@ $(OBJS) && \
	$(RANLIB) )

clean:
	cd $(DIR) && rm -f $(OBJS) libglut.a

.SUFFIXES: .c .o

FORCE:
