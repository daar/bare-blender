#
# $Id: Makefile,v 1.5 2000/09/14 08:08:01 frank Exp $
#
# Make module object directory and bounce make to subdirectories.

include nan.mk

DIR = $(HOM)/sound

DIRS = intern


all clean install: makedir
	@for i in $(DIRS); do \
	    echo "====> $(MAKE) $@ in $$i" ;\
	    $(MAKE) -C $$i $@ || exit 1; \
	done

dist:

makedir: FORCE
	@# create module obj directory
	@[ -d $(DIR) ] || mkdir $(DIR)

FORCE:
