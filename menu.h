/* Menu selection window
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

#ifndef _Imenu
#define _Imenu 1

#include "config.h"

#include "w.h"

typedef struct menu MENU;

struct menu {
	W *parent;		/* Window we're in */
	char **list;		/* List of items */
	int top;		/* First item on screen */
	int cursor;		/* Item cursor is on */
	int width;		/* Width of widest item, up to 'w' max */
	int perline;		/* Number of items on each line */
	int nitems;		/* No. items in list */
	SCREEN *t;		/* Screen we're on */
	int h, w, x, y;
	int (*abrt) ();		/* Abort callback function */
	int (*func) ();		/* Return callback function */
	int (*backs) ();	/* Backspace callback function */
	void *object;
};

#define TYPEMENU 0x800

/* Create a menu */
/* FIXME: ??? ---> */
MENU *mkmenu PARAMS((W *w, char **s, int (*func) (/* ??? */), int (*abrt) (/* ??? */), int (*backs) (/* ??? */), int cursor, void *object, int *notify));

/* Menu user functions */

int umuparw PARAMS((MENU *m));
int umdnarw PARAMS((MENU *m));
int umltarw PARAMS((MENU *m));
int umrtarw PARAMS((MENU *m));
int umbof PARAMS((MENU *m));
int umeof PARAMS((MENU *m));
int umbol PARAMS((MENU *m));
int umeol PARAMS((MENU *m));
int umbacks PARAMS((MENU *m));

void ldmenu PARAMS((MENU *m, char **s, int cursor));

char *mcomplete PARAMS((MENU *m));

#endif
