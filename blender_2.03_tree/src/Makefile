#
# $Id: Makefile,v 1.77 2000/09/17 19:07:19 ton Exp $
#

include nan.mk

DIR = $(HOM)/src
VPATH = $(DIR)

# little hack for the render module
RENDERDIR = ../render/extern/include/
CFLAGS += -I../render/extern/include/

AUDIOLIB = $(HOM)/sound/libsound.a

ifeq ($(OS),beos)
    CFLAGS += -pipe -funsigned-char -fPIC
    LLIBS = -L/boot/develop/lib/x86/ -lGL -lbe -L/boot/home/config/lib/ -lpython1.5
    OB_ADD = beosstuff.o
    BLENDER = blender
    EXT =
endif

ifeq ($(OS),freebsd)
    CFLAGS += -pipe -funsigned-char -DUSE_OPENAL -pthread
    EXT =
    ifeq ($(NAN_GLLIB), glx)
	COMMENT = "GLx experimental"
	DADD = -lGL -lGLU
	SADD = /usr/X11R6/lib/libGL.a /usr/X11R6/lib/libGLU.a
	BLENDER = blender
    else
	COMMENT = "MESA 3.0"
	CFLAGS += -DMESA30
	DADD = -lMesaGL -lMesaGLU
	SADD = /usr/X11R6/lib/libMesaGL.a /usr/X11R6/lib/libMesaGLU.a
	BLENDER = blender blenderstatic
    endif
    DADD += -L/usr/X11R6/lib/ -lXmu -lXext -lX11 -lm
    #SADD += -static -L/usr/X11R6/lib/ -lXmu -lXext -lX11 -lm
    SADD += /usr/X11R6/lib/libXmu.a /usr/X11R6/lib/libXext.a \
	/usr/X11R6/lib/libX11.a /usr/lib/libm.a
    PYLIB = $(HOM)/libs/libpython.a
    AUDIOLIB += $(BCGDIR)/lib/libopenal.a $(HOM)/sound/libsound.a
endif

ifeq ($(OS),irix)
    ifeq ($(OS_VERSION),6.2)
	CFLAGS += -float
    endif
    LLIBS = -lmovieGL -lGLU -lGL -lXmu -lXext -lX11 -lc -lm -ldmedia -lcl -laudio
    PYLIB = $(HOM)/libs/libpython.a
    BLENDER = blender
    EXT =
endif

ifeq ($(OS),linux)
    EXT =
    ifeq ($(CPU),alpha)
	CFLAGS += -pipe -funsigned-char
	LLIBS = -lGL -lGLU -L/usr/X11R6/lib/ -lXmu -lXext -lX11 -lc -lm -ldl
	LOPTS = -export-dynamic
	PYLIB = $(HOM)/libs/libpython.a
	BLENDER = blender
    endif
    ifeq ($(CPU),i386)
	COMMENT = "MESA 3.1"
	INCLUDE += -I/usr/src/Mesa-3.1/include/
	CFLAGS += -pipe -funsigned-char -DUSE_OPENAL -pthread
	LLIBS = -L/usr/src/Mesa-3.1/lib/ -L/usr/X11R6/lib/ \
		-lXmu -lXext -lX11 -lc -lm -ldl -lpthread
	LOPTS = -export-dynamic
	DADD = -lGL -lGLU
	SADD = /usr/src/Mesa-3.1/lib/libGL.a /usr/src/Mesa-3.1/lib/libGLU.a
	PYLIB = $(HOM)/libs/libpython.a
	AUDIOLIB += $(BCGDIR)/lib/libopenal.a
	BLENDER = blender blenderstatic
    endif
    ifeq ($(CPU),powerpc)
	COMMENT = "MESA 3.1"
	INCLUDE += -I/usr/src/MesaCVS/include/
	CFLAGS += -pipe -funsigned-char
	# -DUSE_OPENAL -lpthread
	LLIBS = /usr/src/MesaCVS/lib/libGL.a /usr/src/MesaCVS/lib/libGLU.a -L/usr/X11R6/lib/ -lXmu -lXext -lX11 -lc -ldl -lm
	LOPTS = -export-dynamic
	PYLIB = $(HOM)/libs/libpython.a
	BLENDER = blenderstatic
    endif
endif

ifeq ($(OS),solaris)
    CFLAGS += -D__SUN -pipe -funsigned-char -DSUN_OGL_NO_VERTEX_MACROS
    LLIBS = -lGLU -lGL -lXmu -lXext -lX11 -lc -lm -ldl -lsocket -lnsl
    PYLIB = $(HOM)/libs/libpython.a
    BLENDER = blender
    EXT =
endif

ifeq ($(OS),windows)
    CFLAGS += -funsigned-char -DUSE_OPENAL
    DADD = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib opengl32.lib glu32.lib 
    OB_ADD = winstuff.o winblender.res
    OB_ADD_PATH = $(DIR)/winstuff.o
    PYLIB = $(HOM)/libs/python15.lib
    AUDIOLIB += $(HOM)/openal/*/*.o
    BLENDER = blender.exe
    EXT = .exe
endif

# this might not work for Windows; then it needs to be incorporated in
# the above OS dependencies...
ifdef NAN_GAME
    COMMENT += "NAN_GAME"
	SUMO   = -L$(OCGDIR)/sumo/$(NAN_SUMO_DEBUG)
	KETSJI = -L$(OCGDIR)/gameengine
	QHULL  = -L$(SRCHOME)/extern/qhull
	CFLAGS += -DNAN_GAME
    LLIBS += -lketsji -llogic -lrasterizer -lbloutines -lexpression -lsolid -lmoto -lqhull -lstdc++
    INCLUDE += -I../../sumo/include/
else
    COMMENT += "without NAN_GAME"
    SUMO  = -L$(OCGDIR)/sumo/$(NAN_SUMO_DEBUG)
#   QHULL = -L$(SRCHOME)/extern/qhull -lqhull
#   LLIBS += -lsolid -lmoto -lstdc++
    INCLUDE += -I../../sumo/include/
endif

# the line INCLUDE += -I../include/python/ should be done after other
# include paths have been added. This enables overriding include files
# in ../include/python....
# NO LONGER NEEDED #INCLUDE += -I../include/python/
I = ../include/

GLUT = $(HOM)/wtk/libwtk.a
GLUT = $(HOM)/glut/libglut.a
SLIBS = $(HOM)/misc/libmisc.a $(HOM)/imbuf/libimbuf.a \
	$(HOM)/avi/libavi.a $(HOM)/jpeg/libjpeg.a \
	$(HOM)/render/librender.a \
	$(AUDIOLIB)
DBLENDER = dblender dblenderstatic

LIBSTATIC = $(KETSJI) $(SUMO) $(QHULL) $(PYLIB) $(GLUT) $(SLIBS) $(LLIBS) $(SADD)
LIBS =     $(KETSJI) $(SUMO) $(QHULL) $(PYLIB) $(GLUT) $(SLIBS) $(LLIBS) $(DADD)

all: makedir blendtarget

makedir: FORCE
	@[ -d $(DIR) ] || mkdir $(DIR)
	@cp -f .obj.Makefile $(DIR)/Makefile
    ifdef COMMENT
	@echo "****> NOTE: $(COMMENT)"
    endif

install: all
	@echo "****> install blender$(EXT) in $(BCGDIR)/exe/"
	@[ ! -f $(DIR)/blender$(EXT) ] || \
	    cp -f $(DIR)/blender$(EXT) $(BCGDIR)/exe/
	@[ ! -f $(DIR)/blenderstatic ] || \
	    cp -f $(DIR)/blenderstatic $(BCGDIR)/exe/

# Note: FREE=-DFREE may be inherited from source/Makefile.

OB_EDIT = edit.o editcurve.o editfont.o editipo.o \
	editmesh.o editobject.o isect.o scanfill.o editface.o  \
	editsound.o sector.o life.o editgroup.o

OB_GRAPHICS = buttons.o drawimage.o \
	drawimasel.o drawipo.o displist.o \
	drawobject.o drawoops.o drawscene.o \
	drawseq.o drawtext.o drawview.o \
	editika.o editimasel.o editkey.o editlattice.o \
	editmball.o editoops.o drawmesh.o \
	editseq.o editsima.o editview.o filesel.o \
	headerbuttons.o mywindow.o os_specific.o playanim.o \
	screen.o space.o toets.o \
	toolbox.o usiblender.o view.o view2d.o drawsound.o \
	vpaint.o interface.o editsca.o packedFile.o

OB_FILE = genfile.o fileops.o readfile.o writefile.o

OB_STANDALONE = $(DIR)/../makesdna/DNA.o keyval.o imageprocess.o noise.o psfont.o rct.o 

OB_MIXMODEL = anim.o arithb.o blender.o curve.o effect.o \
	exotic.o font.o ika.o image.o imasel.o ipo.o \
	key.o library.o material.o mball.o mesh.o object.o oops.o \
	scene.o sequence.o text.o world.o property.o sca.o group.o 

OB_RENDER =  writeavi.o writemovie.o

OB_DATA = Bfont.o splash.jpg.o blenderbuttons.o cmap.tga.o cmovie.tga.o B.blend.o Bfs.o

OB_RADIO = radio.o radfactors.o radnode.o radpreprocess.o radpostprocess.o \
	raddisplay.o

OB_PYTHON = py_main.o py_vector.o py_matrix.o py_datablock.o\
	py_interface.o py_draw.o py_demo.o py_nmesh.o py_nurbs.o

OBJS = $(OB_EDIT) $(OB_GRAPHICS) $(OB_FILE) $(OB_STANDALONE) $(OB_MIXMODEL) \
	$(OB_RENDER) $(OB_DATA) $(OB_RADIO) $(OB_PYTHON) $(OB_ADD)

.c.o:
	$(CC) $(FREE) -c -o $(DIR)/$@ $<

.rc.res:
	rc /fo$(DIR)/$@ $<

dist: $(DBLENDER)

dblender: blender
	(cd $(DIR) && \
	strip blender && \
	cp blender .. )

dblenderstatic: blenderstatic
	(cd $(DIR) && \
	strip blenderstatic && \
	cp blenderstatic .. )

blendtarget: $(BLENDER)

blenderp: $(OBJS) $(SLIBS) $(GLUT)
	$(CC) -DWITH_PYTHON $(FREE) $(CFLAGS) $(INCLUDE) -c -o $(DIR)/text.o ../src/text.c
	$(CC) -o $(DIR)/blenderp $(OBJS) $(PYLIB) $(LIBS) $(LOPTS)

blender$(EXT): $(OBJS) $(SLIBS) $(GLUT)
	@echo "****> Dynamic version"
	cd $(DIR) && $(CC) -o blender$(EXT) $(OBJS) $(LIBS) $(LOPTS)

blenderstatic: $(OBJS) $(SLIBS) $(GLUT)
	@echo "****> Static version"
	cd $(DIR) && $(CC) -o blenderstatic $(OBJS) $(LIBSTATIC) $(LOPTS)

gltest: gltest.o
	$(CC)  gltest.o $(GLUT) -lGLU -lGL -lXmu -lXext -lX11 -lc -o $(DIR)/gltest

padding: padding.o
	$(CC) padding.o -lc -o $(DIR)/padding

clean:
	cd $(DIR) && rm -f *.o blender$(EXT) $(BLENDER)

$(OB_EDIT):  	  $(I)blender.h  $(I)screen.h  $(I)edit.h $(I)mydevice.h
$(OB_GRAPHICS):   $(I)blender.h  $(I)screen.h  $(I)edit.h $(I)graphics.h $(I)mydevice.h
$(OB_MIXMODEL):   $(I)blender.h  $(I)screen.h  $(I)edit.h
$(OB_FILE): 	  $(I)blender.h  $(I)screen.h  $(I)file.h $(RENDERDIR)render.h $(I)effect.h $(I)sequence.h $(I)ika.h $(I)oops.h
$(OB_RENDER): 	  $(I)blender.h  $(RENDERDIR)render.h
$(OB_RADIO): 	  $(I)blender.h  $(I)radio.h
$(OB_PYTHON):	  $(I)blender.h  
$(OB_STANDALONE): $(I)blender.h  

writeavi.o: $(I)avi.h

editsca.o sca.o readfile.o writefile.o sector.o life.o: $(I)game.h $(I)sector.h
interface.o editsca.o usiblender.o toolbox.o buttons.o headerbuttons.o space.o screen.o drawipo.o filesel.o: $(I)interface.h
drawimasel.o imasel.o editimasel.o readfile.o writefile.o: $(I)imasel.h
drawoops.o editoops.o oops.o: $(I)oops.h
drawipo.o editipo.o: $(I)ipo.h
world.o material.o image.o buttons.o screen.o filesel.o toets.o displist.o: $(RENDERDIR)render.h
sequence.o editseq.o drawseq.o editipo.o ipo.o drawipo.o: $(I)sequence.h 
effect.o buttons.o drawobject.o initrender.o: $(I)effect.h
ika.o editlattice.o ipo.o editika.o drawobject.o object.o library.o headerbuttons.o editobject.o buttons.o: $(I)ika.h
text.o drawtext.o: $(I)text.h
filesel.o library.o space.o headerbuttons.o editsound.o drawsound.o readfile.o writefile.o:$(I)sound.h
group.o editgroup.o headerbuttons.o readfile.o writefile.o:$(I)group.h

struct: genfile.o struct.c
	cc genfile.o struct.c ../misc/libmisc.a -lc -o $(DIR)/struct

.SUFFIXES: .c .o .rc .res

FORCE:
