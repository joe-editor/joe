SUBDIRS = ru

man_MANS = joe.1

EXTRA_DIST = $(man_MANS)

joe.1 : joe.1.in
	sed 's,@JOERC\@,$(DESTDIR)$(sysconf_joedir),; s,@SPELL\@,@SPELL@,; s,@BINDIR\@,$(DESTDIR)$(bindir),' $< > $@

MOSTLYCLEANFILES = $(man_MANS)

