all: plugins

plugins:
	@/bin/sh -c 'for i in *.c; do ../bmake $$i; done;'

clean:
	rm -rf *.o *.so
