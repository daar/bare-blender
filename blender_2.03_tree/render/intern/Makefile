#
# $Id: Makefile,v 1.1 2000/08/25 15:27:33 nzc Exp $
#
# just bounce the make down

include nan.mk

DIRS = source

all clean install:
	@for i in $(DIRS); do \
	    echo "====> $$i" ;\
	    $(MAKE) -C $$i $@ || exit 1; \
	done

dist:
