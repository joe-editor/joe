AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = COPYING INFO LIST NEWS README SOURCES TODO \
	joerc.in jmacsrc.in jstarrc.in rjoerc.in jpicorc.in \
	joe.1.in conf.c chain.asm dir.c kbdhdl.c msscrn.c \
	mstty.c termcap terminfo

sysconf_DATA = joerc jmacsrc jstarrc rjoerc jpicorc
man_MANS = joe.1
noinst_HEADERS = b.h blocks.h bw.h cmd.h config.h hash.h help.h kbd.h kbdhdl.h \
	macro.h main.h menu.h path.h poshist.h pw.h queue.h qw.h rc.h regex.h \
	scrn.h tab.h termcap.h tty.h tw.h ublock.h uedit.h uerror.h ufile.h \
	uformat.h uisrch.h umath.h undo.h usearch.h ushell.h utag.h utils.h \
	va.h vfile.h vs.h w.h


bin_PROGRAMS = joe termidx
INCLUDES = -DJOERC="\"@sysconfdir@/\""

joe_SOURCES = b.c blocks.c bw.c cmd.c hash.c help.c kbd.c macro.c main.c menu.c \
	path.c poshist.c pw.c queue.c qw.c rc.c regex.c scrn.c tab.c termcap.c \
	tty.c tw.c ublock.c uedit.c uerror.c ufile.c uformat.c uisrch.c umath.c \
	undo.c usearch.c ushell.c utag.c va.c vfile.c vs.c w.c utils.c

termidx_SOURCES = termidx.c

install-exec-hook:
	rm -f $(bindir)/jmacs $(bindir)/jstar $(bindir)/rjoe $(bindir)/jpico
	for i in jmacs jstar rjoe jpico ; do $(LN_S) $(bindir)/joe $(bindir)/$$i ; done

uninstall-local:
	rm -f $(bindir)/jmacs $(bindir)/jstar $(bindir)/rjoe $(bindir)/jpico

distclean-local:
	rm -f *~

.PHONY: termcap
