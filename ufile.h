/*
	User file operations
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
*/

#ifndef _Iufile
#define _Iufile 1

extern int exask;

#include "bw.h"

void genexmsg ();

int ublksave ();
int ushell ();
int usave ();
int uedit ();
int uinsf ();
int uexsve ();
int unbuf ();
int upbuf ();
int uask ();
int ubufed ();
int ulose ();
int okrepl ();
int doedit ();

#endif
