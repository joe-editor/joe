/*
	Keyboard macros
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
 */

#ifndef _Imacro
#define _Imacro 1

#include "config.h"
#include "cmd.h"

struct macro
{
	int k;			/* Keycode */
	int arg;		/* Repeat argument */
	CMD *cmd;		/* Command address */
	int n;			/* Number of steps */
	int size;		/* Malloc size of steps */
	MACRO **steps;		/* Block */
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
MACRO *mkmacro ();
void addmacro ();
MACRO *dupmacro ();
void rmmacro ();
MACRO *macstk ();
MACRO *macsta ();

void chmac ();

/* Text to macro / Macro to text */
MACRO *mparse ();
char *mtext ();

/* Execute a macro */
extern MACRO *curmacro;
int exemac ();
int exmacro ();

/* Keyboard macros user interface */
int uplay ();
int ustop ();
int urecord ();
int uquery ();
int umacros ();

/* Repeat prefix user command */
int uarg ();
int uuarg ();

#endif
