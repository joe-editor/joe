# Makefile for Joe's Own Editor

###############
# Configuration
###############

# Set where you want joe to go, where you
# want joe's initialization file (joerc)
# to go and where you want the man page
# to go:

WHEREJOE = /usr/local/bin
WHERERC = /usr/local/lib
WHEREMAN = /usr/man/man1

# If you want to use TERMINFO, you have to set
# the following variable to 1.  Also you have to
# include some additional libraries- see below.

TERMINFO = 0

# You may also have to add some additional
# defines to get the include files to work
# right on some systems.
#
# for some HPUX systems, you need to add:  -D_HPUX_SOURCE

# C compiler options: make's built-in rules use this variable

CFLAGS = -O

# C compiler to use: make's built-in rules use this variable

CC = cc

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

OBJS = b.o blocks.o bw.o cmd.o hash.o help.o kbd.o macro.o main.o menu.o \
 path.o poshist.o pw.o queue.o qw.o random.o rc.o regex.o scrn.o tab.o \
 termcap.o tty.o tw.o ublock.o uedit.o uerror.o ufile.o uformat.o uisrch.o \
 umath.o undo.o usearch.o ushell.o utag.o va.o vfile.o vs.o w.o zstr.o

# That's it!

# How to make joe from object files.  Object files are made from source
# files using make's built-in rules.

joe: $(OBJS)
	rm -f jmacs jstar rjoe jpico
	$(CC) $(CFLAGS) -o joe $(OBJS) $(EXTRALIBS)
	ln joe jmacs
	ln joe jstar
	ln joe rjoe
	ln joe jpico

# All object files depend on config.h

$(OBJS): config.h

# How to make config.h

config.h:
	$(CC) conf.c -o conf
	./conf $(WHERERC) $(TERMINFO)

# How to make termidx

termidx: termidx.o
	$(CC) $(CFLAGS) -o termidx termidx.o

# Install proceedure

install: joe termidx
	strip joe
	strip termidx
	if [ ! -d $(WHEREJOE) ]; then mkdir $(WHEREJOE); chmod a+rx $(WHEREJOE); fi
	rm -f $(WHEREJOE)/joe $(WHEREJOE)/jmacs $(WHEREJOE)/jstar $(WHEREJOE)/jpico $(WHEREJOE)/rjoe $(WHEREJOE)/termidx
	mv joe $(WHEREJOE)
	ln $(WHEREJOE)/joe $(WHEREJOE)/jmacs
	ln $(WHEREJOE)/joe $(WHEREJOE)/jstar
	ln $(WHEREJOE)/joe $(WHEREJOE)/rjoe
	ln $(WHEREJOE)/joe $(WHEREJOE)/jpico
	mv termidx $(WHEREJOE)
	if [ ! -d $(WHERERC) ]; then mkdir $(WHERERC); chmod a+rx $(WHERERC); fi
	rm -f $(WHERERC)/joerc $(WHERERC)/jmacsrc $(WHERERC)/jstarrc $(WHERERC)/jpicorc $(WHERERC)/rjoerc $(WHEREMAN)/joe.1
	cp joerc $(WHERERC)
	cp jmacsrc $(WHERERC)
	cp jstarrc $(WHERERC)
	cp rjoerc $(WHERERC)
	cp jpicorc $(WHERERC)
	cp joe.1 $(WHEREMAN)
	chmod a+x $(WHEREJOE)/joe
	chmod a+x $(WHEREJOE)/jmacs
	chmod a+x $(WHEREJOE)/jstar
	chmod a+x $(WHEREJOE)/rjoe
	chmod a+x $(WHEREJOE)/jpico
	chmod a+r $(WHERERC)/joerc
	chmod a+r $(WHERERC)/jmacsrc
	chmod a+r $(WHERERC)/jstarrc
	chmod a+r $(WHERERC)/rjoerc
	chmod a+r $(WHERERC)/jpicorc
	chmod a+r $(WHEREMAN)/joe.1
	chmod a+x $(WHEREJOE)/termidx
	rm -f $(WHERERC)/termcap
	cp termcap $(WHERERC)/termcap
	chmod a+r $(WHERERC)/termcap
	rm -f $(WHERERC)/terminfo
	cp terminfo $(WHERERC)/terminfo
	chmod a+r $(WHERERC)/terminfo

# Cleanup proceedure

clean:
	rm -f $(OBJS) termidx.o conf conf.o config.h
