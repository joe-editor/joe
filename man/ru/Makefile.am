man_MANS = joe.1

mandir = @mandir@/ru

EXTRA_DIST = joe.1.in

joe.1 : joe.1.in
	sed 's,@JOERC\@,$(DESTDIR)$(sysconf_joedir),; s,@SPELL\@,@SPELL@,; s,@BINDIR\@,$(DESTDIR)$(bindir),' $< > $@

MOSTLYCLEANFILES = $(man_MANS)
