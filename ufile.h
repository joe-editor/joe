/*
	User file operations
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
*/

#ifndef _Iufile
#define _Iufile 1

extern int exask;

#include "config.h"
#include "bw.h"

void genexmsg PARAMS((BW *bw, int saved, char *name));

int ublksave PARAMS((BW *bw));
int ushell PARAMS((BW *bw));
int usave PARAMS((BW *bw));
int uedit PARAMS((BW *bw));
int uinsf PARAMS((BW *bw));
int uexsve PARAMS((BW *bw));
int unbuf PARAMS((BW *bw));
int upbuf PARAMS((BW *bw));
int uask PARAMS((BW *bw));
int ubufed PARAMS((BW *bw));
int ulose PARAMS((BW *bw));
int okrepl PARAMS((BW *bw));
int doedit PARAMS((BW *bw, char *s, void *obj, int *notify));

#endif
