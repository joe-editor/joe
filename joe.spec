%define name joe
%define version 2.9.7
%define release pre1

Summary: An easy to use, modeless text editor.
Name: %{name}
Version: %{version}
Release: %{release}
Copyright: GPL
Group: Editors
Source: ftp://ftp.std.com/src/editors/joe-%{version}-%{release}-0.tgz
Source1: %{name}_icons.tar.bz2
BuildRequires: gpm-devel, ncurses-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot
Prefix: %{_prefix}

%description
Joe is an easy to use, modeless text editor which would be very
appropriate for novices.  Joe uses the same WordStar keybindings used in
Borland's development environment.

You should install joe if you've used it before and you liked it, or if
you're still deciding what text editor you'd like to use, or if you have a
fondness for WordStar.  If you're just starting out, you should probably
install joe because it is very easy to use.

%prep
%setup -q -n joe-%{version}-%{release}
tar xIvf %{SOURCE1}
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{prefix} --sysconfdir=%{_sysconfdir}

%build
%make

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{prefix}
%makeinstall
# make prefix=%{buildroot}%{prefix} install
strip %{buildroot}%{prefix}/bin/* || :

install -d $RPM_BUILD_ROOT%{_libdir}/menu 
mkdir -p $RPM_BUILD_ROOT%{_libdir}/menu/
cat << EOF > $RPM_BUILD_ROOT%{_libdir}/menu/%{name}
?package(%{name}):\
	needs="text"\
	section="Applications/Editors"\
	title="Joe"\
	longtitle="Joe - a text ANSI editor"\
	command="%{_bindir}/joe"\
	icon=joe.xpm
EOF

mkdir -p $RPM_BUILD_ROOT%{_datadir}/icons/{mini,large}
cd $RPM_BUILD_DIR/%{name}-%{version}-%{release}
install -m 644 %{name}_16.xpm $RPM_BUILD_ROOT%{_datadir}/icons/mini/%{name}.xpm
install -m 644 %{name}_32.xpm $RPM_BUILD_ROOT%{_datadir}/icons/%{name}.xpm
install -m 644 %{name}_48.xpm $RPM_BUILD_ROOT%{_datadir}/icons/large/%{name}.xpm

%post
%{update_menus}

%postun
%{clean_menus}

%files
%defattr (-,root,root)
%{_bindir}/*
%config(noreplace) %{_sysconfdir}/*
%{_mandir}/man1/joe.1.bz2
%{_libdir}/menu/joe
%{_datadir}/icons/%{name}.xpm
%{_datadir}/icons/large/%{name}.xpm
%{_datadir}/icons/mini/%{name}.xpm

%clean
rm -rf $RPM_BUILD_ROOT
