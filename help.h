/* Help system
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

#ifndef _Ihelp
#define _Ihelp 1

#include "config.h"

extern char *hlptxt;
extern int hlpsiz, hlpbsz, hlplns;
extern char **help_names;
extern struct help **help_structs;
extern struct help *first_help;

void dsphlp();

struct help
 {
 char *hlptxt;
 int hlpsiz;
 int hlpbsz;
 int hlplns;
 char *name;
 struct help *next;
 };

int helpon();
int uhelp();
int uhnext();
int uhprev();

#endif
