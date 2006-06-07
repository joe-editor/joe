sysconf_joedir = $(sysconfdir)/joe
man_MANS = joe.1

mandir = @mandir@/ru

EXTRA_DIST = joe.1.in

joe.1 : joe.1.in
	sed 's,@JOERC\@,$(sysconf_joedir),; s,@SPELL\@,@SPELL@,; s,@BINDIR\@,$(bindir),' $(srcdir)/joe.1.in > $@

MOSTLYCLEANFILES = $(man_MANS)
