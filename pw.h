/* Prompt windows
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

#ifndef _Ipw
#define _Ipw 1

#include "config.h"
#include "bw.h"

typedef struct pw PW;

struct pw
 {
 int (*pfunc)();	/* Func which gets called when RTN is hit */
 int (*abrt)();		/* Func which gets called when window is aborted */
 int (*tab)();		/* Func which gets called when TAB is hit */
 char *prompt;		/* Prompt string */
 int promptlen;		/* Width of prompt string */
 int promptofst;	/* Prompt scroll offset */
 B *hist;		/* History buffer */
 void *object;		/* Object */
 };

#define TYPEPW 0x200

/* BW *wmkpw(BW *bw,char *prompt,int (*func)(),char *huh,int (*abrt)(),
             int (*tab)(),void *object,int *notify);
 * Create a prompt window for the given window
 */
BW *wmkpw();

int ucmplt();

#endif
