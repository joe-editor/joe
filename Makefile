# Makefile for Joe's Own Editor

# Directory to install joe and .joerc into
WHEREJOE = /usr/local/stusrc/joe
WHEREJOERC = /usr/local/stusrc/joe

# Use these two for 'cc'
CC = cc
CFLAGS = -DKEYDEF=\"$(WHEREJOERC)/.joerc\" -O

# Use these two for 'gcc'
#CC = gcc
#CFLAGS = -DKEYDEF=\"$(WHEREJOERC)/.joerc\"

foo:
	@echo Type make followed by one of the following
	@echo
	@echo bsd hpux xenix esix posix cruddy install clean

bsd: joe.o asyncbsd.o blocks.o
	$(CC) $(CFLAGS) joe.o asyncbsd.o blocks.o -ltermcap -o joe

xenix: joe.o asyncxenix.o blocks.o
	$(CC) $(CFLAGS) joe.o asyncxenix.o blocks.o -lx -ltermcap -o joe

hpux: joe.o asynchpux.o blocks.o
	$(CC) $(CFLAGS) joe.o asynchpux.o blocks.o -ltermcap -o joe

esix: joe.o asyncesix.o blocks.o
	$(CC) $(CFLAGS) joe.o asyncesix.o blocks.o -lcurses -lbsd -o joe

posix: joe.o asyncposix.o blocks.o
	$(CC) $(CFLAGS) joe.o asyncposix.o blocks.o -ltermcap -o joe

linux: joe.o asynclinux.o blocks.o
	$(CC) $(CFLAGS) joe.o asynclinux.o blocks.o -o joe

cruddy: joe.o cruddy.o blocks.o
	$(CC) $(CFLAGS) joe.o cruddy.o blocks.o -o joe

install:
	strip joe
	mv joe $(WHEREJOE)
	cp .joerc $(WHEREJOERC)
	chmod a+x $(WHEREJOE)/joe
	chmod a+r $(WHEREJOERC)/.joerc

clean:
	rm -f asyncbsd.o asyncxenix.o asynchpux.o asyncesix.o asyncposix.o \
cruddy.o blocks.o joe.o asynclinux.o

asyncbsd.o asynclinux.o cruddy.o asyncxenix.o asynxhpux.o asyncesix.o : async.h

blocks.o : blocks.h

joe.o : blocks.h joe.h async.h
