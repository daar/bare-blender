#
# $Id: Makefile,v 1.2 2000/08/25 15:27:33 nzc Exp $
#
# Makes module object directory and bounces make to subdirectories.

include nan.mk

DIR = $(HOM)/render

DIRS = intern

all clean install: makedir
	@for i in $(DIRS); do \
	    echo "====> $$i" ;\
	    $(MAKE) -C $$i $@ || exit 1; \
	done

dist:

makedir: FORCE
	@# create module obj directory
	@[ -d $(DIR) ] || mkdir $(DIR)

FORCE:
