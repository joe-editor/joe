#ifndef _Ii18n
#define _Ii18n 1

#include "config.h"
#include "types.h"

int joe_iswupper PARAMS((int c));
int joe_iswlower PARAMS((int c));
int joe_iswalpha PARAMS((int c));
int joe_iswdigit PARAMS((int c));
int joe_iswspace PARAMS((int c));
int joe_iswctrl PARAMS((int c));
int joe_iswpunct PARAMS((int c));
int joe_iswgraph PARAMS((int c));
int joe_iswprint PARAMS((int c));
int joe_iswxdigit PARAMS((int c));
int joe_iswblank PARAMS((int c));

int joe_wcwidth PARAMS((int wide,int c));

int joe_towupper PARAMS((int c));
int joe_towlower PARAMS((int c));

#endif
