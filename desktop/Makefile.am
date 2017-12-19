desktopdir = $(datarootdir)/applications
dist_desktop_DATA = joe.desktop jmacs.desktop jstar.desktop jpico.desktop

update-databases:
	-update-desktop-database $(desktopdir)

install-data-hook: update-databases
uninstall-hook: update-databases
