/*
	Text editing windows
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
 */

#ifndef _Itw
#define _Itw 1

#include "config.h"
#include "main.h"
#include "bw.h"

typedef struct tw TW;

struct tw
{
	/* Status line info */
	char *stalin;
	char *staright;
	int staon;		/* Set if status line was on */
	long prevline;		/* Previous cursor line number */
	int changed;		/* Previous changed value */
};

#define TYPETW 0x100

/* BW *wmktw(SCREEN *t,B *b)
 */
BW *wmktw ();

int usplitw ();
int uduptw ();
int utw0 ();
int utw1 ();
int uabortbuf ();
int uabort ();
void setline ();

extern int staen;

#endif
