/*
 *	Menu selection window
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_MENU_H
#define _JOE_MENU_H 1

#include "config.h"
#include "types.h"

#include "w.h"

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
