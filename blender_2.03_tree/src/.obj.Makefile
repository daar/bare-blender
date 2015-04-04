#
# $Id: .obj.Makefile,v 1.2 2000/06/05 10:12:29 hans Exp $
#

SDIR = $(HOME)/develop/source/blender2/src

all clean install:
	@echo "****> Object Makefile, chdir to $(SDIR) ..."
	@$(MAKE) -C $(SDIR) $@ || exit 1;
