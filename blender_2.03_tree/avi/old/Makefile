# libavi makefile
ENDIAN = -DWORDS_BIGENDIAN   # Big endian machines

OBJS = avi.o avirgb.o codecs.o endian.o options.o jmf.o mjpeg.o rgb32.o
LIBS = -lc

CFLAGS = -O2 -I../include/
CPPADD =

libavi: libavi.a libavi.so

libavi.a: $(OBJS)
	touch libavi.a
	rm libavi.a
	ar scru libavi.a $(OBJS) 

libavi.so: libavi.a
	ld -soname libavi.so -shared -all libavi.a -o temp

install: libavi.so avi.h
	$(INSTALL) -m 644 -f /usr/people/blend/OpenGL/include avi.h
	$(INSTALL) -m 644 -f /usr/lib/ libavi.so.0.3.0

clean:
	rm -rf *.o *~ libavi.a libavi.so

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPADD) $(ENDIAN) -c -o $@ $<

.SUFFIXES: .c .o


