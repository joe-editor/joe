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
