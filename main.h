/*
	Editor startup and edit loop
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
 */

#ifndef _Imain
#define _Imain 1

#include "config.h"
#include "w.h"

extern char *exmsg;		/* Exit message */
extern int help;		/* Set to start with help on */
extern SCREEN *maint;		/* Primary screen */
void nungetc ();
void dofollows ();
int edloop ();

#endif
