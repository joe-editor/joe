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

struct macro
 {
 int k;		/* Keycode */
 int arg;	/* Repeat argument */
 CMD *cmd;	/* Command address */
 int n;		/* Number of steps */
 int size;	/* Malloc size of steps */
 MACRO **steps;	/* Block */
 };

struct recmac
 {
 struct recmac *next;
 int n;
 MACRO *m;
 };

/* Set when macro is recording: for status line */
extern struct recmac *recmac;

/* Macro construction functions */
MACRO *mkmacro();
void addmacro();
MACRO *dupmacro();
void rmmacro();
MACRO *macstk();
MACRO *macsta();

void chmac();

/* Text to macro / Macro to text */
MACRO *mparse();
char *mtext();

/* Execute a macro */
extern MACRO *curmacro;
int exemac();
int exmacro();

/* Keyboard macros user interface */
int uplay();
int ustop();
int urecord();
int uquery();
int umacros();

/* Repeat prefix user command */
int uarg();
int uuarg();

#endif
