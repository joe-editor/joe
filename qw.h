/* Single-key query windows
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

#ifndef _Iquery
#define _Iquery 1

#include "config.h"
#include "w.h"

typedef struct query QW;
struct query
 {
 W *parent;			/* Window we're in */
 int (*func)();			/* Func. which gets called when key is hit */
 int (*abrt)();
 void *object;
 char *prompt;			/* Prompt string */
 int promptlen;			/* Width of prompt string */
 int promptofst;		/* Prompt scroll offset */
 };

#define TYPEQW 0x1000

/* QW *mkqw(BW *bw,char *prompt,int (*func)(),int (*abrt)(),void *object);
 * Create a query window for the given window
 */
QW *mkqw();
QW *mkqwna();
QW *mkqwnsr();

#endif
