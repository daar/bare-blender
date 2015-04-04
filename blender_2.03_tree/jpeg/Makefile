#
# $Id: Makefile,v 1.6 2000/05/18 10:12:00 hans Exp $
#

include nan.mk

DIR = $(HOM)/jpeg
VPATH = $(DIR)

OBJS = \
	jcapimin.o jcapistd.o jctrans.o jcparam.o jdatadst.o jcinit.o \
	jcmaster.o jcmarker.o jcmainct.o jcprepct.o jccoefct.o jccolor.o \
	jcsample.o jchuff.o jcphuff.o jcdctmgr.o jfdctfst.o jfdctflt.o \
	jfdctint.o jdapimin.o jdapistd.o jdtrans.o jdatasrc.o jdmaster.o \
	jdinput.o jdmarker.o jdhuff.o jdphuff.o jdmainct.o jdcoefct.o \
	jdpostct.o jddctmgr.o jidctfst.o jidctflt.o jidctint.o jidctred.o \
	jdsample.o jdcolor.o jquant1.o jquant2.o jdmerge.o jcomapi.o \
	jutils.o jerror.o jmemmgr.o jmemnobs.o

all: makedir libjpeg.a

install: all
	@echo "****> install libjpeg.a in $(BCGDIR)/lib/"
	@cp -f $(DIR)/*.a $(BCGDIR)/lib/

makedir: FORCE
	@[ -d $(DIR) ] || mkdir $(DIR)

libjpeg.a: $(OBJS)
	(cd $(DIR) && \
	ar rv libjpeg.a $(OBJS) && \
	$(RANLIB) )

clean:
	cd $(DIR) && rm -f *.o libjpeg.a

.c.o:
	$(CC) -c -o $(DIR)/$@ $<
	
.SUFFIXES: .c .o

FORCE:
