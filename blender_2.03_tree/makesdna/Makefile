#
# $Id: Makefile,v 1.7 2000/08/13 19:22:38 ton Exp $
#

include nan.mk

DIR = $(HOM)/makesdna
VPATH = $(DIR)
I = ../include/

ifneq ($(OS),irix)
    CFLAGS += -funsigned-char
endif

all: makedir $(DIR)/makesdna $(DIR)/DNA.o

makedir: FORCE
	@[ -d $(DIR) ] || mkdir $(DIR)

install: all

.c.o:
	$(CC) $(FREE) -c -o $(DIR)/$@ $<

$(DIR)/makesdna: makesdna.o
	$(CC) $(DIR)/makesdna.o $(HOM)/misc/util.o -o $(DIR)/makesdna

$(DIR)/DNA.c: $(I)sound.h $(I)util.h $(I)blender.h $(I)oops.h $(I)imasel.h $(I)screen.h $(I)file.h $(I)sequence.h $(I)effect.h $(I)game.h $(I)sector.h $(DIR)/makesdna
	$(DIR)/makesdna $(DIR)/DNA.c

$(DIR)/DNA.o: $(DIR)/DNA.c
	$(CC) $(FREE) -c -o $@ $(DIR)/DNA.c

clean:
	cd $(DIR) && rm -f *.o makesdna DNA $(BLENDER)

.SUFFIXES: .c .o

FORCE:
