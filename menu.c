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

#include "config.h"
#include <string.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include "scrn.h"
#include "w.h"
#include "vs.h"
#include "menu.h"

extern int dostaupd;

void menufllw(m)
MENU *m;
{
	m->top = m->cursor - m->cursor % m->perline;
}

void menudisp(m)
MENU *m;
{
	int col;
	int x;
	int *s = m->t->t->scrn + m->x + m->y * m->t->t->co;

	col = 0;
	for (x = 0; x != m->perline && m->list[x + m->top]; ++x) {
		int atr, z;

		if (x + m->top == m->cursor)
			atr = INVERSE;
		else
			atr = 0;
		if (col == m->w)
			break;
		for (z = 0; m->list[x + m->top][z]; ++z) {
			if (col == m->w)
				break;
			outatr(m->t->t, s + col, m->x + col, m->y, m->list[x + m->top][z], atr);
			++col;
		}
		while (z < m->width) {
			if (col == m->w)
				break;
			outatr(m->t->t, s + col, m->x + col, m->y, ' ', 0);
			++col;
			++z;
		}
		if (col != m->w) {
			outatr(m->t->t, s + col, m->x + col, m->y, ' ', 0);
			++col;
		}
	}
	if (col != m->w)
		eraeol(m->t->t, m->x + col, m->y);
	m->parent->cury = 0;
	m->parent->curx = (m->cursor - m->top) * (m->width + 1);
}

void menumove(m, x, y)
MENU *m;
{
	m->x = x;
	m->y = y;
}

void menuresz(m, wi, he)
MENU *m;
{
	m->w = wi;
	m->h = he;
}

void mconfig(m)
MENU *m;
{
	/* Configure menu display parameters */
	if (m->list) {
		int x;

		m->top = 0;
		for (x = 0, m->width = 0; m->list[x]; ++x)
			if (strlen(m->list[x]) > m->width)
				m->width = strlen(m->list[x]);
		m->nitems = x;
		if (m->width > m->w)
			m->width = m->w - 1;
		m->perline = m->w / (m->width + 1);
	}
}

int umbol(m)
MENU *m;
{
	m->cursor = m->top;
	return 0;
}

int umbof(m)
MENU *m;
{
	m->cursor = 0;
	return 0;
}

int umeof(m)
MENU *m;
{
	if (m->nitems)
		m->cursor = m->nitems - 1;
	return 0;
}

int umeol(m)
MENU *m;
{
	if (m->top + m->perline < m->nitems)
		m->cursor = m->top + m->perline - 1;
	else
		umeof(m);
	return 0;
}

int umrtarw(m)
MENU *m;
{
	if (m->cursor + 1 < m->nitems) {
		++m->cursor;
		return 0;
	} else
		return -1;
}

int umltarw(m)
MENU *m;
{
	if (m->cursor) {
		--m->cursor;
		return 0;
	} else
		return -1;
}

int umuparw(m)
MENU *m;
{
	if (m->cursor >= m->perline) {
		m->cursor -= m->perline;
		return 0;
	} else
		return -1;
}

int umdnarw(m)
MENU *m;
{
	if (m->cursor + m->perline < m->nitems) {
		m->cursor += m->perline;
		return 0;
	} else if (m->top + m->perline < m->nitems)
		return umeof(m);
	else
		return -1;
}

int umrtn(m)
MENU *m;
{
	dostaupd = 1;
	if (m->func)
		return m->func(m, m->cursor, m->object, 0);
	else
		return -1;
}

int umbacks(m)
MENU *m;
{
	if (m->backs)
		return m->backs(m, m->cursor, m->object);
	else
		return -1;
}

int umkey(m, c)
MENU *m;
{
	int x;
	int n = 0;

	if (c == '0') {
		if (m->func)
			return m->func(m, m->cursor, m->object, -1);
		else
			return -1;
	}
	if (c == '1') {
		if (m->func)
			return m->func(m, m->cursor, m->object, 1);
		else
			return -1;
	}
	c &= 0x1F;
	for (x = 0; x != m->nitems; ++x)
		if ((m->list[x][0] & 0x1F) == c)
			++n;
	if (!n)
		return -1;
	if (n == 1)
		for (x = 0; x != m->nitems; ++x)
			if ((m->list[x][0] & 0x1F) == c) {
				m->cursor = x;
				return umrtn(m);
			}
	do {
		++m->cursor;
		if (m->cursor == m->nitems)
			m->cursor = 0;
	} while ((m->list[m->cursor][0] & 0x1F) != c);

	return -1;
}

static int menuabort(m)
MENU *m;
{
	W *w = m->parent;
	int (*func) () = m->abrt;
	void *object = m->object;
	int x = m->cursor;
	W *win = w->win;

	free(m);
	if (func)
		return func(win->object, x, object);
	else
		return -1;
}

WATOM watommenu = {
	"menu",
	menudisp,
	menufllw,
	menuabort,
	umrtn,
	umkey,
	menuresz,
	menumove,
	0,
	0,
	TYPEMENU
};

void ldmenu(m, s, cursor)
MENU *m;
char **s;
{
	m->list = s;
	m->cursor = cursor;
	mconfig(m);
}

MENU *mkmenu(obw, s, func, abrt, backs, cursor, object, notify)
BASE *obw;
char **s;
int (*func) ();
int (*abrt) ();
int (*backs) ();
void *object;
int *notify;
{
	W *w = obw->parent;
	W *new = wcreate(w->t, &watommenu, w, w, w->main, 1, NULL, notify);
	MENU *m;

	if (!new) {
		if (notify)
			*notify = 1;
		return 0;
	}
	wfit(new->t);
	new->object = (void *) (m = (MENU *) malloc(sizeof(MENU)));
	m->parent = new;
	m->func = func;
	m->abrt = abrt;
	m->backs = backs;
	m->object = object;
	m->t = w->t;
	m->h = new->h;
	m->w = new->w;
	m->x = new->x;
	m->y = new->y;
	ldmenu(m, s, cursor);
	w->t->curwin = new;
	return m;
}

char *cull(a, b)
char *a, *b;
{
	int x;

	for (x = 0; a[x] && b[x] && a[x] == b[x]; ++x) ;
	return vstrunc(a, x);
}

char *mcomplete(m)
MENU *m;
{
	char *com;
	int x;

	if (!m->nitems)
		return vstrunc(NULL, 0);
	com = vsncpy(NULL, 0, sz(m->list[0]));
	for (x = 1; x != m->nitems; ++x)
		com = cull(com, m->list[x]);
	return com;
}
