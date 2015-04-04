#
# $Id: Makefile,v 1.18 2000/09/08 16:12:59 hans Exp $
#

include nan.mk

DIRS=
ifeq ($(NAN_TARGET), BLENDER2)
    ifeq ($(OS),beos)
	DIRS = glut-beos
    endif

    ifeq ($(OS),$(findstring $(OS), "freebsd irix linux solaris"))
	DIRS = glut-x11
    endif

    ifeq ($(OS),windows)
	DIRS = glut-win
    endif

    DIRS += avi imbuf jpeg misc libs render sound makesdna src
endif

DIR = $(HOM)

all clean install:
	@# Create object directory
	@[ -d $(DIR) ] || mkdir $(DIR)
	@cp -f .obj.Makefile $(DIR)/Makefile
	@# Create binary directory tree
	@[ -d $(BCGDIR)/exe ] || mkdir $(BCGDIR)/exe
	@[ -d $(BCGDIR)/lib ] || mkdir $(BCGDIR)/lib
	@for i in $(DIRS); do \
	    echo "====> $$i" ;\
	    $(MAKE) -C $$i $@ || exit 1; \
	done
