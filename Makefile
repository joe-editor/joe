# Makefile for Joe's Editor

# Directory to install j into
WHERE = /usr/bin

# Use these two for 'cc'
CC = cc
CFLAGS = -DKEYDEF=\"$(WHERE)/keymap.j\" -O

# Use these two for 'gcc'
#CC = gcc
#CFLAGS = -DKEYDEF=\"$(WHERE)/keymap.j\" -traditional -O

foo:
	@echo Type make followed by one of the following
	@echo
	@echo bsd hpux xenix cruddy install clean

bsd: j.o asyncbsd.o blocks.o
	$(CC) $(CFLAGS) j.o asyncbsd.o blocks.o -ltermcap -o j
	cp keymapbsd keymap.j

xenix: j.o asyncxenix.o blocks.o
	$(CC) $(CFLAGS) j.o asyncxenix.o blocks.o -lx -ltermcap -o j
	cp keymapxenix keymap.j

hpux: j.o asynchpux.o blocks.o
	$(CC) $(CFLAGS) j.o asynchpux.o blocks.o -ltermcap -o j
	cp keymapbsd keymap.j

cruddy: j.o cruddy.o blocks.o
	$(CC) $(CFLAGS) j.o cruddy.o blocks.o -o j
	cp keymapbsd keymap.j

install:
	strip j
	mv j $(WHERE)
	mv keymap.j $(WHERE)
	chmod a+x $(WHERE)/j
	chmod a+r $(WHERE)/keymap.j

clean:
	rm asyncbsd.o asyncxenix.o asynchpux.o cruddy.o blocks.o j.o keymap.j

asyncbsd.o cruddy.o asyncxenix.o asynxhpux.o : async.h

blocks.o : blocks.h

j.o : blocks.h j.h async.h
