#ifndef _Ierr
#define _Ierr 1

#include "config.h"

int unxterr PARAMS((BW *bw));
int uprverr PARAMS((BW *bw));
int uparserr PARAMS((BW *bw));
void inserr PARAMS((char *name, long int where, long int n, int bol));
void delerr PARAMS((char *name, long int where, long int n));
void abrerr PARAMS((char *name));
void saverr PARAMS((char *name));

#endif
