sysconf_joedir = $(sysconfdir)/joe

sysconf_joe_built = joerc jicerc.ru jmacsrc jstarrc rjoerc jpicorc

sysconf_joe_DATA = $(sysconf_joe_built) ftyperc

EXTRA_DIST = joerc.in jicerc.ru.in jmacsrc.in jstarrc.in jpicorc.in rjoerc.in ftyperc

joerc : joerc.in
	sed 's,@JOERC\@,$(sysconf_joedir),; s,@SPELL\@,@SPELL@,' $< > $@

jicerc.ru : jicerc.ru.in
	sed 's,@JOERC\@,$(sysconf_joedir),; s,@SPELL\@,@SPELL@,' $< > $@

jmacsrc : jmacsrc.in
	sed 's,@JOERC\@,$(sysconf_joedir),; s,@SPELL\@,@SPELL@,' $< > $@

jstarrc : jstarrc.in
	sed 's,@JOERC\@,$(sysconf_joedir),; s,@SPELL\@,@SPELL@,' $< > $@

jpicorc : jpicorc.in
	sed 's,@JOERC\@,$(sysconf_joedir),; s,@SPELL\@,@SPELL@,' $< > $@

rjoerc : rjoerc.in
	sed 's,@JOERC\@,$(sysconf_joedir),; s,@SPELL\@,@SPELL@,' $< > $@

MOSTLYCLEANFILES = $(sysconf_joe_built)
