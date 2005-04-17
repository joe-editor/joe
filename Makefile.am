AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = COPYING LIST HACKING ChangeLog NEWS README INSTALL INSTALL.AMIGA INSTALL.cvs \
	TODO docs/help-system.html \
	joerc.in jmacsrc.in jstarrc.in rjoerc.in jpicorc.in \
	joe.1.in dir.c syntax/c.jsf.in syntax/perl.jsf.in syntax/verilog.jsf.in \
	syntax/conf.jsf.in syntax/python.jsf.in syntax/php.jsf.in syntax/sh.jsf.in \
	syntax/pascal.jsf.in syntax/html.jsf.in syntax/vhdl.jsf.in syntax/fortran.jsf.in \
	syntax/java.jsf.in syntax/xml.jsf.in syntax/tcl.jsf.in charmaps/klingon.in HINTS \
	syntax/lisp.jsf.in syntax/csh.jsf.in syntax/mail.jsf.in syntax/mason.jsf.in \
	syntax/diff.jsf.in syntax/asm.jsf.in syntax/tex.jsf.in syntax/css.jsf.in \
	syntax/ocaml.jsf.in syntax/4gl.jsf.in syntax/sml.jsf.in syntax/sql.jsf.in \
	syntax/awk.jsf.in syntax/cobol.jsf.in syntax/sed.jsf.in syntax/ps.jsf.in \
	cygbuild setup.hint

sysconf_joedir = $(sysconfdir)/joe
sysconf_joe_DATA = joerc jmacsrc jstarrc rjoerc jpicorc

joerc : joerc.in
	sed 's,@JOERC\@,$(DESTDIR)$(sysconf_joedir),; s,@SPELL\@,@SPELL@,' < $(srcdir)/joerc.in > $@

jmacsrc : jmacsrc.in
	sed 's,@JOERC\@,$(DESTDIR)$(sysconf_joedir),; s,@SPELL\@,@SPELL@,' < $(srcdir)/jmacsrc.in > $@

jstarrc : jstarrc.in
	sed 's,@JOERC\@,$(DESTDIR)$(sysconf_joedir),; s,@SPELL\@,@SPELL@,' < $(srcdir)/jstarrc.in > $@

jpicorc : jpicorc.in
	sed 's,@JOERC\@,$(DESTDIR)$(sysconf_joedir),; s,@SPELL\@,@SPELL@,' < $(srcdir)/jpicorc.in > $@

rjoerc : rjoerc.in
	sed 's,@JOERC\@,$(DESTDIR)$(sysconf_joedir),; s,@SPELL\@,@SPELL@,' < $(srcdir)/rjoerc.in > $@

joe.1 : joe.1.in
	sed 's,@JOERC\@,$(DESTDIR)$(sysconf_joedir),; s,@SPELL\@,@SPELL@,; s,@BINDIR\@,$(DESTDIR)$(bindir),' < $(srcdir)/joe.1.in > $@

MOSTLYCLEANFILES = joerc jmacsrc jstarrc rjoerc jpicorc joe.1

sysconf_syntaxdir = $(sysconf_joedir)/syntax
sysconf_syntax_DATA = syntax/c.jsf syntax/perl.jsf syntax/verilog.jsf \
	syntax/conf.jsf syntax/python.jsf syntax/php.jsf syntax/sh.jsf \
	syntax/mail.jsf syntax/pascal.jsf syntax/html.jsf syntax/vhdl.jsf \
	syntax/fortran.jsf syntax/java.jsf syntax/xml.jsf syntax/tcl.jsf \
	syntax/lisp.jsf syntax/csh.jsf syntax/mason.jsf syntax/diff.jsf \
	syntax/asm.jsf syntax/tex.jsf syntax/css.jsf syntax/ocaml.jsf \
	syntax/4gl.jsf syntax/sml.jsf syntax/sql.jsf syntax/awk.jsf \
	syntax/cobol.jsf syntax/sed.jsf syntax/ps.jsf

sysconf_charmapsdir = $(sysconf_joedir)/charmaps
sysconf_charmaps_DATA = charmaps/klingon

sysconf_docdir = $(sysconf_joedir)/doc
sysconf_doc_DATA = LIST README HINTS ChangeLog HACKING NEWS

man_MANS = joe.1
noinst_HEADERS = b.h blocks.h bw.h cmd.h config.h hash.h help.h kbd.h \
	macro.h main.h menu.h path.h poshist.h pw.h queue.h qw.h rc.h regex.h \
	scrn.h tab.h termcap.h tty.h tw.h types.h ublock.h uedit.h uerror.h \
	ufile.h uformat.h uisrch.h umath.h undo.h usearch.h ushell.h utag.h \
	utils.h va.h vfile.h vs.h w.h utf8.h syntax.h i18n.h charmap.h mouse.h


bin_PROGRAMS = joe termidx
INCLUDES = -DJOERC="\"$(DESTDIR)$(sysconf_joedir)/\""

joe_LDADD = -lm

joe_SOURCES = b.c blocks.c bw.c cmd.c hash.c help.c kbd.c macro.c main.c menu.c \
	path.c poshist.c pw.c queue.c qw.c rc.c regex.c scrn.c tab.c termcap.c \
	tty.c tw.c ublock.c uedit.c uerror.c ufile.c uformat.c uisrch.c umath.c \
	undo.c usearch.c ushell.c utag.c va.c vfile.c vs.c w.c utils.c syntax.c \
	utf8.c selinux.c i18n.c charmap.c mouse.c

termidx_SOURCES = termidx.c

install-exec-hook:
	rm -f $(DESTDIR)$(bindir)/jmacs $(DESTDIR)$(bindir)/jstar $(DESTDIR)$(bindir)/rjoe $(DESTDIR)$(bindir)/jpico
	for i in jmacs jstar rjoe jpico ; do $(LN_S) joe $(DESTDIR)$(bindir)/$$i ; done

uninstall-local:
	rm -f $(DESTDIR)$(bindir)/jmacs $(DESTDIR)$(bindir)/jstar $(DESTDIR)$(bindir)/rjoe $(DESTDIR)$(bindir)/jpico

distclean-local:
	rm -f *~

.PHONY: termcap
