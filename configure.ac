# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.52)

# Init automake
AC_INIT
AC_CONFIG_SRCDIR([b.c])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

#### Here's the only place where to change version number ####
AM_INIT_AUTOMAKE(joe, 2.9.7)

AM_MAINTAINER_MODE
AM_CONFIG_HEADER(autoconf.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S

# Checks for libraries.
AC_SEARCH_LIBS(tgetflag, ncurses curses termcap, AC_DEFINE_UNQUOTED(TERMINFO, 1, [If we have newer terminfo/termcap capabilities]))
# AC_CHECK_LIB([ncurses], [tgetstr])


# Checks for header files.
AC_HEADER_DIRENT
AC_SYS_POSIX_TERMIOS
if test "$ac_cv_sys_posix_termios" = "yes"; then
	AC_DEFINE(HAVE_POSIX_TERMIOS, 1, [We have <termios.h> and <sys/termios.h>])
else
	AC_CHECK_HEADERS([termio.h sys/termio.h])
	if test "$ac_cv_header_termio_h" = "yes" -a "$ac_cvs_header_sys_termio_h" = "yes" ]; then
		AC_DEFINE(HAVE_SYSV_TERMIO, 1, [We have <termio.h> and <sys/termio.h>])
	fi
fi
AC_HEADER_TIOCGWINSZ
AC_CHECK_HEADERS([errno.h fcntl.h locale.h sgtty.h stdlib.h string.h])
AC_CHECK_HEADERS([sys/ioctl.h sys/param.h sys/time.h unistd.h utime.h])
AC_CHECK_HEADERS([ncurses.h curses.h sys/dirent.h time.h pwd.h paths.h])
AC_CHECK_HEADERS([term.h sys/types.h sys/stat.h sys/wait.h limits.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_PROTOTYPES
AC_C_CONST
if test "$cross_compiling" = "yes" ; then
	AC_MSG_WARN(cross compiling so assuming defaults for most 32 bit machines)
fi
AC_CHECK_SIZEOF(char, 1)
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(void *, 4)


# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_LSTAT
AC_FUNC_MALLOC
joe_SETPGRP
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_TYPE([ssize_t], [], [
	AC_DEFINE([ssize_t], [int], [Define to `int' if <sys/types.h> does not define.])
	], [])
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([getcwd getwd])
if test x"$ac_cv_func_getcwd" != xyes; then
	if test x"$ac_cv_func_getwd" != xyes; then
		AC_MSG_ERROR([don't know how to get current working directory])
	fi
fi
AC_CHECK_FUNCS([isblank])
if test x"$ac_cv_func_isblank" = xyes; then
	joe_ISBLANK
fi
AC_CHECK_FUNCS([alarm mkdir mkstemp setlocale strchr strdup utime setpgid])
AC_CHECK_FUNCS([setitimer siginterrupt sigprocmask])


dnl not yet :-)
dnl AM_GNU_GETTEXT

AH_VERBATIM([_PARAMS],[/* Check to see if we can use strict prototypes */
#ifndef PARAMS
#  if PROTOTYPES
#    define PARAMS(protos) protos
#  else
#    define PARAMS(protos) ()
#  endif
#endif
])

AC_CONFIG_FILES([Makefile joe.1 joerc jmacsrc jstarrc rjoerc jpicorc])
AC_OUTPUT
