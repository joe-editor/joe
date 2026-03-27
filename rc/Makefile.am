sysconf_joedir = $(sysconfdir)/joe
data_joedir = $(datadir)/joe
data_docdir = $(docdir)

sysconf_joe_built = joerc jicerc.ru jmacsrc jstarrc rjoerc jpicorc joerc.zh_TW

sysconf_joe_DATA = $(sysconf_joe_built) ftyperc shell.sh shell.csh

EXTRA_DIST = joerc.in joerc.zh_TW.in jicerc.ru.in jmacsrc.in jstarrc.in jpicorc.in rjoerc.in ftyperc shell.sh shell.csh

joerc : joerc.in
joerc.zh_TW : joerc.zh_TW.in
jicerc.ru : jicerc.ru.in
jmacsrc : jmacsrc.in
jstarrc : jstarrc.in
jpicorc : jpicorc.in
rjoerc : rjoerc.in

SUFFIXES = .in
.in:
	LC_ALL=C $(SED) 's,@JOERC\@,$(sysconf_joedir),; s,@JOEDOC\@,$(data_docdir),; s,@JOEDATA\@,$(data_joedir),; s,@SPELL\@,@SPELL@,' $(srcdir)/$< > $@

MOSTLYCLEANFILES = $(sysconf_joe_built)
