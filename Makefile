# Makefile for Joe's Own Editor

###############
# Configuration
###############

# Set where you want joe to go, where you
# want joe's initialization file (joerc)
# to go and where you want the man page
# to go:

WHEREJOE = /usr/bin
WHERERC = /usr/lib
WHEREMAN = /usr/man/man1

# If you want to use TERMINFO, you have to set
# the following variable to 1.  Also you have to
# include some additional libraries- see below.

TERMINFO = 1

# You may also have to add some additional
# defines to get the include files to work
# right on some systems.
#
# for some HPUX systems, you need to add:  -D_HPUX_SOURCE

# C compiler options: make's built-in rules use this variable

CFLAGS = -O2 -fsigned-char -fomit-frame-pointer -pipe

# C compiler to use: make's built-in rules use this variable

CC = gcc

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

EXTRALIBS = -lncurses

# Object files

OBJS = b.o blocks.o bw.o cmd.o hash.o help.o kbd.o macro.o main.o menu.o \
 path.o poshist.o pw.o queue.o qw.o rc.o regex.o scrn.o tab.o \
 termcap.o tty.o tw.o ublock.o uedit.o uerror.o ufile.o uformat.o uisrch.o \
 umath.o undo.o usearch.o ushell.o utag.o va.o vfile.o vs.o w.o \
 utils.o

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
	if [ ! -d $(WHEREJOE) ]; then mkdir $(WHEREJOE); chmod a+rx $(WHEREJOE); fi
	rm -f $(WHEREJOE)/joe $(WHEREJOE)/jmacs $(WHEREJOE)/jstar $(WHEREJOE)/jpico $(WHEREJOE)/rjoe $(WHEREJOE)/termidx
	install -s joe $(WHEREJOE)
	ln -s $(WHEREJOE)/joe $(WHEREJOE)/jmacs
	ln -s $(WHEREJOE)/joe $(WHEREJOE)/jstar
	ln -s $(WHEREJOE)/joe $(WHEREJOE)/rjoe
	ln -s $(WHEREJOE)/joe $(WHEREJOE)/jpico
	install -s termidx $(WHEREJOE)
	if [ ! -d $(WHERERC) ]; then mkdir $(WHERERC); chmod a+rx $(WHERERC); fi
	rm -f $(WHERERC)/joerc $(WHERERC)/jmacsrc $(WHERERC)/jstarrc $(WHERERC)/jpicorc $(WHERERC)/rjoerc $(WHEREMAN)/joe.1
	install -m 644 joerc $(WHERERC)
	install -m 644 jmacsrc $(WHERERC)
	install -m 644 jstarrc $(WHERERC)
	install -m 644 rjoerc $(WHERERC)
	install -m 644 jpicorc $(WHERERC)
	install -m 644 joe.1 $(WHEREMAN)
	#rm -f $(WHERERC)/termcap
	#cp termcap $(WHERERC)/termcap
	#chmod a+r $(WHERERC)/termcap
	#rm -f $(WHERERC)/terminfo
	#cp terminfo $(WHERERC)/terminfo
	#chmod a+r $(WHERERC)/terminfo

# Cleanup proceedure

clean:
	rm -f $(OBJS) termidx.o conf conf.o config.h joe jmacs jpico jstar rjoe termidx

