dnl Check for properly working isblank()
AC_DEFUN(joe_ISBLANK,
	[AC_CACHE_CHECK([whether isblank() works correctly with side effect expressions],
		[joe_cv_isblank],
		[AC_TRY_RUN([
#include <ctype.h>
int main() {
  int a = 0;
  isblank(a++);
  exit(a != 1);
}
			],
			[joe_cv_isblank=yes],
			[joe_cv_isblank=no],
			[joe_cv_isblank=no])
		])
	if test "$joe_cv_isblank" = yes; then
		AC_DEFINE([HAVE_WORKING_ISBLANK], 1, [Define if isblank() works with expressions with side effects])
	fi
])

dnl Check if setpgrp must have two arguments
dnl autoconf-own macro is damaged for *BSD systems
AC_DEFUN(joe_SETPGRP,
	[AC_CACHE_CHECK([whether setpgrp() takes no arguments],
		[joe_cv_setpgrp_void],
		[AC_TRY_RUN([
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
int main() {
	/* exit succesfully if setpgrp() takes two args (*BSD systems) */
	exit(setpgrp(0, 0) != 0);
}],
			[joe_cv_setpgrp_void=no],
			[joe_cv_setpgrp_void=yes],
			[joe_cv_setpgrp_void=yes])
		])
	if test "$joe_cv_setpgrp_void" = yes; then
		AC_DEFINE([SETPGRP_VOID], 1, [Define if setpgrp() takes no arguments])
	fi
])
