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

extern int square;
extern int lightoff;
extern P *markb, *markk;

void pinsrect();
int ptabrect();
void pclrrect();
void pdelrect();
B *pextrect();
int markv();
int umarkb();
int umarkk();
int uswap();
int umarkl();
int utomarkb();
int utomarkk();
int utomarkbk();
int ublkdel();
int ublkmove();
int ublkcpy();
int dowrite();
int doinsf();
void setindent();
int urindent();
int ulindent();
int ufilt();
int unmark();
int udrop();
int upsh();
int upop();
extern int nstack;

#endif
