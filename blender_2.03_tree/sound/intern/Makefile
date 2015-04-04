#
# $Id: Makefile,v 1.1 2000/09/01 12:49:18 hans Exp $
#
# just bounce the make down

include nan.mk

DIRS = source

all clean install:
	@for i in $(DIRS); do \
	    echo "====> $(MAKE) $@ in $$i" ;\
	    $(MAKE) -C $$i $@ || exit 1; \
	done

dist:
