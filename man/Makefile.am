include $(top_srcdir)/Makevars

SUBDIRS = ru

man_MANS = joe.1

EXTRA_DIST = joe.1.in

joe.1 : joe.1.in
	$(AM_V_SED) LC_ALL=C $(SED) 's,@JOERC\@,$(sysconf_joedir),; s,@JOEDOC\@,$(data_docdir),; s,@JOEDATA\@,$(data_joedir),; s,@SPELL\@,@SPELL@,; s,@BINDIR\@,$(bindir),' $(srcdir)/joe.1.in > $@

MOSTLYCLEANFILES = $(man_MANS)

