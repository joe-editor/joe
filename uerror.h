#ifndef _Ierr
#define _Ierr 1

#include "config.h"

int unxterr (BW *bw);
int uprverr (BW *bw);
int uparserr (BW *bw);
void inserr (char *name, long where, long n, int bol);
void delerr (char *name, long where, long n);
void abrerr (char *name);
void saverr (char *name);

#endif
