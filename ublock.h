/*
	Highlighted block functions
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
 */

#ifndef _Iublock
#define _Iublock 1

#include "config.h"
#include "b.h"

extern int square;
extern int lightoff;
extern P *markb, *markk;

void pinsrect ();
int ptabrect ();
void pclrrect ();
void pdelrect ();
B *pextrect ();
int markv ();
int umarkb ();
int umarkk ();
int uswap ();
int umarkl ();
int utomarkb ();
int utomarkk ();
int utomarkbk ();
int ublkdel ();
int upicokill ();
int ublkmove ();
int ublkcpy ();
int dowrite ();
int doinsf ();
void setindent ();
int urindent ();
int ulindent ();
int ufilt ();
int unmark ();
int udrop ();
int upsh ();
int upop ();
extern int nstack;

#endif
