# Makefile for Joe's Own Editor

###############
# Configuration
###############

# Set where you want joe to go, where you
# want joe's initialization file (joerc)
# to go and where you want the man page
# to go:

PREFIX	= /usr
WHEREJOE = $(PREFIX)/bin
WHERERC = $(PREFIX)/lib
WHEREMAN = $(PREFIX)/man/man1

# If you want to be able to edit '-', which causes joe to read in or write out
# to the stdin/stdout, change the '1' below to '0'.  Be warned however: this
# makes joe use /dev/tty to open the tty, which means that the modification
# times on the real tty don't get updated.  Idle session killers and screen
# blankers will think that no one is using the terminal and log you out or
# blank the screen.

IDLEOUT = 1

# You may also have to add some additional
# defines to get the include files to work
# right on some systems.
#
# for some HPUX systems, you need to add:  -D_HPUX_SOURCE

CFLAGS = -O2 -m486

# You may have to include some extra libraries
# for some systems
#
# for Xenix, add (in this order!!): -ldir -lx
#
# For some systems you might have to add: -lbsd
# to get access to the timer system calls.
#
# If you wish to use terminfo, you have to
# add '-ltinfo', '-lcurses' or '-ltermlib',
# depending on the system.

EXTRALIBS =

# Object files
#
# If you wish to use terminfo instead of
# termcap, replace 'termcap.o' below with 'terminfo.o'

OBJS = b.o blocks.o bw.o cmd.o hash.o help.o kbd.o macro.o main.o menu.o \
 path.o poshist.o pw.o queue.o qw.o random.o rc.o regex.o scrn.o tab.o \
 termcap.o tty.o tw.o ublock.o uedit.o uerror.o ufile.o uformat.o uisrch.o \
 umath.o undo.o usearch.o ushell.o utag.o va.o vfile.o vs.o w.o zstr.o

CC = gcc

# That's it!

all: joe

joe: $(OBJS)
	$(CC) $(CFLAGS) -o joe $(OBJS) $(EXTRALIBS)
	rm -f jmacs
	rm -f jstar
	rm -f rjoe
	ln -s joe jmacs
	ln -s joe jstar
	ln -s joe rjoe

$(OBJS): config.h

config.h:
	$(CC) conf.c -o conf
	./conf $(WHERERC) $(IDLEOUT)

termidx: termidx.o
	$(CC) $(CFLAGS) -o termidx termidx.o

install: joe termidx
	if [ ! -d $(WHEREJOE) ]; then \
		mkdir $(WHEREJOE) ;\
		chmod a+rx $(WHEREJOE) ;\
	fi
	rm -f $(WHEREJOE)/joe $(WHEREJOE)/jmacs $(WHEREJOE)/jstar $(WHEREJOE)/rjoe $(WHEREJOE)/termidx
	install -s -m755 joe $(WHEREJOE)
	install -s -m755 termidx $(WHEREJOE)
	ln -s joe $(WHEREJOE)/jmacs
	ln -s joe $(WHEREJOE)/jstar
	ln -s joe $(WHEREJOE)/rjoe
	if [ ! -d $(WHERERC) ]; then \
		mkdir $(WHERERC) ;\
		chmod a+rx $(WHERERC) ;\
	fi
	rm -f $(WHERERC)/joerc $(WHERERC)/jmacsrc $(WHERERC)/jstarrc $(WHERERC)/rjoerc $(WHEREMAN)/joe.1
	install -m644 joerc $(WHERERC)
	install -m644 jmacsrc $(WHERERC)
	install -m644 jstarrc $(WHERERC)
	install -m644 rjoerc $(WHERERC)
	install -m644 joe.1 $(WHEREMAN)

clean:
	rm -f joe jmacs jstar rjoe termidx $(OBJS) termidx.o conf conf.o config.h
