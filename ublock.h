/* Highlighted block functions
   Copyright (C) 1992 Joseph H. Allen

This file is part of JOE (Joe's Own Editor)

JOE is free software; you can redistribute it and/or modify it under the 
terms of the GNU General Public License as published by the Free Software 
Foundation; either version 1, or (at your option) any later version.  

JOE is distributed in the hope that it will be useful, but WITHOUT ANY 
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more 
details.  

You should have received a copy of the GNU General Public License along with 
JOE; see the file COPYING.  If not, write to the Free Software Foundation, 
675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef _Iublock
#define _Iublock 1

#include "config.h"
#include "b.h"
#include "bw.h"

extern int square;
extern int lightoff;
extern P *markb, *markk;

void pinsrect PARAMS((P *cur, B *tmp, long int width, int usetabs));
int ptabrect PARAMS((P *org, long int height, long int right));
void pclrrect PARAMS((P *org, long int height, long int right, int usetabs));
void pdelrect PARAMS((P *org, long int height, long int right));
B *pextrect PARAMS((P *org, long int height, long int right));
int markv PARAMS((int r));
int umarkb PARAMS((BW *bw));
int umarkk PARAMS((BW *bw));
int uswap PARAMS((BW *bw));
int umarkl PARAMS((BW *bw));
int utomarkb PARAMS((BW *bw));
int utomarkk PARAMS((BW *bw));
int utomarkbk PARAMS((BW *bw));
int ublkdel PARAMS((BW *bw));
int upicokill PARAMS((BW *bw));
int ublkmove PARAMS((BW *bw));
int ublkcpy PARAMS((BW *bw));
int dowrite PARAMS((BW *bw, char *s, void *object, int *notify));
int doinsf PARAMS((BW *bw, char *s, void *object, int *notify));
void setindent PARAMS((BW *bw));
int urindent PARAMS((BW *bw));
int ulindent PARAMS((BW *bw));
int ufilt PARAMS((BW *bw));
int unmark PARAMS((BW *bw));
int udrop PARAMS((BW *bw));
int upsh PARAMS((BW *bw));
int upop PARAMS((BW *bw));
extern int nstack;

#endif
