
/* Keyboard macros
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

#ifndef _Imacro
#define _Imacro 1

#include "config.h"

#include "cmd.h"

struct macro {
	int k;			/* Keycode */
	int arg;		/* Repeat argument */
	CMD *cmd;		/* Command address */
	int n;			/* Number of steps */
	int size;		/* Malloc size of steps */
	MACRO **steps;		/* Block */
};

struct recmac {
	struct recmac *next;
	int n;
	MACRO *m;
};

/* Set when macro is recording: for status line */
extern struct recmac *recmac;

/* Macro construction functions */
MACRO *mkmacro PARAMS((int k, int arg, int n, CMD *cmd));
void addmacro PARAMS((MACRO *macro, MACRO *m));
MACRO *dupmacro PARAMS((MACRO *mac));
void rmmacro PARAMS((MACRO *macro));
MACRO *macstk PARAMS((MACRO *m, int k));
MACRO *macsta PARAMS((MACRO *m, int a));

void chmac PARAMS((void));

/* Text to macro / Macro to text */
MACRO *mparse PARAMS((MACRO *m, char *buf, int *sta));
char *mtext PARAMS((char *s, MACRO *m));

/* Execute a macro */
extern MACRO *curmacro;
int exemac PARAMS((MACRO *m));
int exmacro PARAMS((MACRO *m, int u));

/* Keyboard macros user interface */
/* FIXME: cyclic dependency of header files (here on BW struct)
int uplay PARAMS((BW *bw, int c));
int urecord PARAMS((BW *bw, int c));
int uquery PARAMS((BW *bw));
int umacros PARAMS((BW *bw));
*/
int uplay();
int ustop PARAMS((void));
int urecord();
int uquery();
int umacros();

/* Repeat prefix user command */
/* FIXME: cyclic dependency of header files (here on BW struct)
int uarg PARAMS((BW *bw));
int uuarg PARAMS((BW *bw, int c));
*/
int uarg();
int uuarg();

#endif
