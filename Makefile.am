AUTOMAKE_OPTIONS = foreign

SUBDIRS = man rc po syntax

EXTRA_DIST = COPYING LIST HACKING ChangeLog NEWS README INSTALL INSTALL.AMIGA INSTALL.cvs \
	TODO docs/help-system.html dir.c charmaps/klingon.in HINTS \
	cygbuild setup.hint xterm-patch xterm-readme autojoe

sysconf_joedir = $(sysconfdir)/joe
data_joedir = $(datadir)/joe

data_charmapsdir = $(data_joedir)/charmaps
data_charmaps_DATA = charmaps/klingon

data_docdir = $(docdir)
data_doc_DATA = LIST README HINTS ChangeLog HACKING NEWS

noinst_HEADERS = b.h blocks.h bw.h cmd.h config.h hash.h help.h kbd.h \
	macro.h main.h menu.h path.h poshist.h pw.h queue.h qw.h rc.h regex.h \
	scrn.h tab.h termcapj.h tty.h tw.h types.h ublock.h uedit.h uerror.h \
	ufile.h uformat.h uisrch.h umath.h undo.h usearch.h ushell.h utag.h \
	utils.h va.h vfile.h vs.h w.h utf8.h syntax.h i18n.h charmap.h mouse.h \
	lattr.h gettext.h builtin.h

bin_PROGRAMS = joe termidx
INCLUDES = -DJOERC="\"$(sysconf_joedir)/\"" -DJOEDATA="\"$(data_joedir)/\""

joe_LDADD = -lm

joe_SOURCES = b.c blocks.c bw.c cmd.c hash.c help.c kbd.c macro.c main.c menu.c \
	path.c poshist.c pw.c queue.c qw.c rc.c regex.c scrn.c tab.c termcap.c \
	tty.c tw.c ublock.c uedit.c uerror.c ufile.c uformat.c uisrch.c umath.c \
	undo.c usearch.c ushell.c utag.c va.c vfile.c vs.c w.c utils.c syntax.c \
	utf8.c selinux.c i18n.c charmap.c mouse.c lattr.c gettext.c builtin.c \
	builtins.c

termidx_SOURCES = termidx.c

install-exec-hook:
	rm -f $(DESTDIR)$(bindir)/jmacs $(DESTDIR)$(bindir)/jstar $(DESTDIR)$(bindir)/rjoe $(DESTDIR)$(bindir)/jpico
	rm -f $(DESTDIR)$(bindir)/jmacs.exe $(DESTDIR)$(bindir)/jstar.exe $(DESTDIR)$(bindir)/rjoe.exe $(DESTDIR)$(bindir)/jpico.exe
	for i in jmacs jstar rjoe jpico ; do $(LN_S) joe $(DESTDIR)$(bindir)/$$i ; done

uninstall-local:
	rm -f $(DESTDIR)$(bindir)/jmacs $(DESTDIR)$(bindir)/jstar $(DESTDIR)$(bindir)/rjoe $(DESTDIR)$(bindir)/jpico
	rm -f $(DESTDIR)$(bindir)/jmacs.exe $(DESTDIR)$(bindir)/jstar.exe $(DESTDIR)$(bindir)/rjoe.exe $(DESTDIR)$(bindir)/jpico.exe

distclean-local:
	rm -f *~

.PHONY: termcap
