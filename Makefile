# Makefile for Joe's Own Editor

# Directory to install joe and .joerc into
WHERE = /usr/bin

# Use these two for 'cc'
CC = cc
CFLAGS = -DKEYDEF=\"$(WHERE)/.joerc\" -O

# Use these two for 'gcc'
#CC = gcc
#CFLAGS = -DKEYDEF=\"$(WHERE)/.joerc\" -O

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

cruddy: joe.o cruddy.o blocks.o
	$(CC) $(CFLAGS) joe.o cruddy.o blocks.o -o joe

install:
	strip joe
	mv joe $(WHERE)
	cp .joerc $(WHERE)
	chmod a+x $(WHERE)/joe
	chmod a+r $(WHERE)/.joerc

clean:
	rm -f asyncbsd.o asyncxenix.o asynchpux.o asyncesix.o asyncposix.o \
cruddy.o blocks.o joe.o

asyncbsd.o cruddy.o asyncxenix.o asynxhpux.o asyncesix.o : async.h

blocks.o : blocks.h

joe.o : blocks.h joe.h async.h
