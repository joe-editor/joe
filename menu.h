/*
	Menu selection window
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
*/

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

MENU *mkmenu ();

/* Menu user functions */

int umuparw (MENU *m);
int umdnarw (MENU *m);
int umltarw (MENU *m);
int umrtarw (MENU *m);
int umbof (MENU *m);
int umeof (MENU *m);
int umbol (MENU *m);
int umeol (MENU *m);
int umbacks (MENU *m);

void ldmenu (MENU *m, char **s, int cursor);

char *mcomplete (MENU *m);

#endif
