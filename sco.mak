# Makefile for Joe's Own Editor

###############
# Configuration
###############

# Uncomment the second line below if you use
# an idle job killer which requires that you
# use the real tty, not /dev/tty

IDLE =
#IDLE = -DIDLEOUT

# Uncomment one of the lines below to select
# which TTY structure your system uses:

#TTY = -DBSDTTY
TTY = -DSVTTY
#TTY = -DPOSIXTTY

# Uncomment the second line below if your
# system has the 'setitimer' system call:

TIM =
#TIM = -DBSDTIMER

# Uncomment which ever addition include
# files you need in 'tty.c' to get the
# timer or the tty structure to work .
#
# SCO is for sys/stream.h and sys/ptem.h
# SYSPARAM is for sys/param.h
# SYSTIM is for sys/time.h
# TIM is for time.h

IA = -DSCO
#IA =
IB = -DSYSPARAM
#IB =
#IC = -DSYSTIM
IC =
ID = -DTIM
#ID =

# Uncomment the second line below if your
# system has the Xenix-style 'nap' system
# call:

CHK =
#CHK = -DXENIX

# Uncomment the second line below if your
# system is real BSD, and hence uses
# 'getwd' instead of 'getcwd':

REAL =
#REAL = -DREALBSD

# Uncomment the line below if your
# system uses 'struct dirent' instead
# of 'struct direct' for opendir().

DIRSTRUCT = -DIRECT
#DIRSTRUCT = -DIRENT

# Uncomment the second line below if your
# POSIX system has 'sigaction', but not
# 'signal':

HARDER =
#HARDER = -DREALPOSIX

# Uncomment the method to make new session
# leaders (or none if you don't have joe control
# or ptys).  GRPCALL is for 'setpgrp(getpid(),0)'
# and SIDCALL is for 'setsid()'.  GRPCALL
# is supposed to be more portable, but
# 'SIDCALL' always works, if it's provided.

#PGRP = -DSIDCALL
PGRP = -DGRPCALL

# Set directory where ptys and corresponding ttys can be found

WHEREPTY = /dev/
WHERETTY = /dev/

# Set where you want joe to go and where you
# want joe's initialization file (joerc) to go:

WHERERC = /usr/local/lib
WHEREJOE = /usr/local/bin

# You may also have to add some additional
# defines to get the include files to work
# right on some systems.
#
# for newer HPUX systems, you need to add:  -D_HPUX_SOURCE

CFLAGS = -O \
 $(TTY) $(TIM) $(CHK) $(REAL) $(DIRSTRUCT) $(HARDER) $(PGRP) \
 $(IA) $(IB) $(IC) $(ID) $(IDLE) \
 -DJOERC=\"$(WHERERC)/joerc\" \
 -DPTYPREFIX=\"$(WHEREPTY)\" -DTTYPREFIX=\"$(WHERETTY)\"

# You may have to include some extra libraries
# for some systems
#
# for Xenix, add: -lx
#
# for some systems you might have to add: -lbsd
# to get enough BSD extensions to use -DBSDTIMER
# above (I think ESIX needs this)
#
# If you wish to use terminfo, you have to
# add '-ltinfo', '-lcurses' or '-ltermlib',
# depending on the system.

EXTRALIBS = -lcurses

# Object files
#
# for systems with no 'opendir' (older SYS V)
# or confused 'opendir' (Xenix), add: olddir.o
#
# If you wish to use terminfo instead of
# termcap, replace 'termcap.o' below with 'terminfo.o'

OBJS = main.o terminfo.o vfile.o pathfunc.o queue.o blocks.o vs.o va.o scrn.o \
       b.o bw.o tw.o pw.o help.o heap.o toomany.o zstr.o edfuncs.o \
       kbd.o w.o reg.o tab.o pattern.o random.o regex.o undo.o menu.o macro.o \
       poshist.o tty.o msgs.o qw.o

CC = cc

# That's it!

joe: $(OBJS)
	$(CC) $(CFLAGS) -o joe $(OBJS) $(EXTRALIBS)

termidx: termidx.o
	$(CC) $(CFLAGS) -o termidx termidx.o

install: joe termidx
	strip joe
	strip termidx
	if [ ! -d $(WHEREJOE) ]; then mkdir $(WHEREJOE); chmod a+rx $(WHEREJOE); fi
	mv joe $(WHEREJOE)
	if [ ! -d $(WHERERC) ]; then mkdir $(WHERERC); chmod a+rx $(WHERERC); fi
	cp joerc $(WHERERC)
	mv termidx $(WHEREJOE)
	chmod a+x $(WHEREJOE)/joe
	chmod a+r $(WHERERC)/joerc
	chmod a+x $(WHEREJOE)/termidx

clean:
	rm -f $(OBJS) termidx.o
