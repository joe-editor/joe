/*
 *	Menu selection window
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <string.h>

#include "scrn.h"
#include "utils.h"
#include "va.h"
#include "vs.h"
#include "utf8.h"
#include "w.h"

extern int dostaupd;

static void menufllw(MENU *m)
{
	if (m->cursor < m->top)
		m->top = m->cursor - m->cursor % m->perline;
	else if (m->cursor >= m->top+m->perline*m->h)
		m->top = m->cursor - m->cursor % m->perline - m->perline*(m->h-1);
}

static void menudisp(MENU *m)
{
	int col;
	int x;
	int y;
	int *s = m->t->t->scrn + m->x + m->y * m->t->t->co;
	int *a = m->t->t->attr + m->x + m->y * m->t->t->co;
	struct utf8_sm sm;

	utf8_init(&sm);

	for (y = 0; y != m->h; ++y) {
		col = 0;
		for (x = 0; x != m->perline && y*m->perline+x+m->top<m->nitems; ++x) {
			int atr, z, lcol;
	
			if (x + y*m->perline + m->top == m->cursor && m->t->curwin==m->parent)
				atr = INVERSE;
			else
				atr = 0;

			if (col == m->w)
				break;

			/* Generate field */
			genfield(m->t->t,
			         s + col,
			         a + col,
			         m->x + col,
			         m->y + y,
			         0,
			         m->list[x + y*m->perline + m->top],
			         strlen((char *)m->list[x + y*m->perline + m->top]),
			         atr,
			         m->width,
			         0);

			col += m->width;

			/* Space between columns */
			if (col != m->w) {
				outatr(locale_map, m->t->t, s + col, a + col, m->x + col, m->y+y, ' ', 0);
				++col;
			}
		}
		/* Clear to end of line */
		if (col != m->w)
			eraeol(m->t->t, m->x + col, m->y + y);
		s += m->t->t->co;
		a += m->t->t->co;
	}
	m->parent->cury = (m->cursor - m->top) / m->perline;
	col = txtwidth(m->list[m->cursor],strlen((char *)m->list[m->cursor]));
	if (col < m->width)
		m->parent->curx = ((m->cursor - m->top) % m->perline) * (m->width + 1) + col;
	else
		m->parent->curx = ((m->cursor - m->top) % m->perline) * (m->width + 1) + m->width;
}

static void menumove(MENU *m, int x, int y)
{
	m->x = x;
	m->y = y;
}

static void menuresz(MENU *m, int wi, int he)
{
	m->w = wi;
	m->h = he;
}

static int mlines(unsigned char **s, int w)
{
	int x;
	int lines;
	int width;
	int nitems;
	int perline;

	for (x = 0, width = 0; s[x]; ++x) {
		int d = txtwidth(s[x],strlen((char *)(s[x])));
		if (d > width)
			width = d;
	}
	nitems = x;
	if (width > w)
		width = w - 1;
	perline = w / (width + 1);

	lines = (nitems + perline - 1) / perline;

	return lines;
}

static void mconfig(MENU *m)
{
	/* Configure menu display parameters */
	if (m->list) {
		int x;
		/* int lines; */

		m->top = 0;
		for (x = 0, m->width = 0; m->list[x]; ++x) {
			int d = txtwidth(m->list[x],strlen((char *)(m->list[x])));
			if (d > m->width)
				m->width = d;
		}
		m->nitems = x;
		if (m->width > m->w)
			m->width = m->w - 1;
		m->perline = m->w / (m->width + 1);

		/* lines = (m->nitems + m->perline - 1) / m->perline; */
	}
}

int umbol(MENU *m)
{
	m->cursor -= m->cursor % m->perline;
	return 0;
}

int umbof(MENU *m)
{
	m->cursor = 0;
	return 0;
}

int umeof(MENU *m)
{
	if (m->nitems)
		m->cursor = m->nitems - 1;
	return 0;
}

int umeol(MENU *m)
{
	m->cursor -= m->cursor % m->perline;

	if (m->cursor+m->perline-1 >= m->nitems)
		m->cursor = m->nitems - 1;
	else
		m->cursor += m->perline - 1;

	return 0;
}

int umrtarw(MENU *m)
{
	if (m->cursor + 1 < m->nitems) {
		++m->cursor;
		return 0;
	} else
		return -1;
}

int umtab(MENU *m)
{
	if (m->cursor + 1 >= m->nitems)
		m->cursor = 0;
	else
		++ m->cursor;
	return 0;
}

int umltarw(MENU *m)
{
	if (m->cursor) {
		--m->cursor;
		return 0;
	} else
		return -1;
}

int umuparw(MENU *m)
{
	if (m->cursor >= m->perline) {
		m->cursor -= m->perline;
		return 0;
	} else
		return -1;
}

int umdnarw(MENU *m)
{
	int col = m->cursor % m->perline;

        m->cursor -= col;

	if (m->cursor + m->perline < m->nitems) {
		m->cursor += m->perline;
		if (m->cursor + col >= m->nitems)
			if (m->nitems)
				m->cursor = m->nitems - 1;
			else
				m->cursor = 0;
		else
			m->cursor += col;
		return 0;
	} else {
		m->cursor += col;
		return -1;
	}
}

int umpgup(MENU *m)
{
	int amnt = (m->h+1)/2;
	if (m->top >= amnt*m->perline) {
		m->top -= amnt*m->perline;
		m->cursor -= amnt*m->perline;
		return 0;
	} else if (m->top) {
		m->cursor -= m->top;
		m->top = 0;
		return 0;
	} else if (m->cursor >= m->perline) {
		m->cursor = m->cursor % m->perline;
		return 0;
	} else
		return -1;
}

int umpgdn(MENU *m)
{
	int amnt = (m->h+1)/2;
	int col = m->cursor % m->perline;
	int y = m->cursor / m->perline;
	int h = (m->nitems + m->perline - 1) / m->perline;
	int t = m->top / m->perline;
	m->cursor -= col;

	if (t + m->h + amnt <= h) {
		m->top += amnt*m->perline;
		m->cursor += amnt*m->perline;
		if (m->cursor + col >= m->nitems)
			if (m->nitems)
				m->cursor = m->nitems - 1;
			else
				m->cursor = 0;
		else
			m->cursor += col;
		return 0;
	} else if (t + m->h < h) {
		amnt = h - (t + m->h);
		m->top += amnt*m->perline;
		m->cursor += amnt*m->perline;
		if (m->cursor + col >= m->nitems)
			if (m->nitems)
				m->cursor = m->nitems - 1;
			else
				m->cursor = 0;
		else
			m->cursor += col;
		return 0;
	} else if (y+1!=h) {
		m->cursor = (h-1)*m->perline;
		if (m->cursor + col >= m->nitems)
			if (m->nitems)
				m->cursor = m->nitems - 1;
			else
				m->cursor = 0;
		else
			m->cursor += col;
		return 0;
	} else {
		m->cursor += col;
		return -1;
	}

}

static int umrtn(MENU *m)
{
	dostaupd = 1;
	if (m->func)
		return m->func(m, m->cursor, m->object, 0);
	else
		return -1;
}

int umbacks(MENU *m)
{
	if (m->backs)
		return m->backs(m, m->cursor, m->object);
	else
		return -1;
}

static int umkey(MENU *m, int c)
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

static int menuabort(MENU *m)
{
	W *w = m->parent;
	int (*func) () = m->abrt;
	void *object = m->object;
	int x = m->cursor;
	W *win = w->win;

	joe_free(m);
	if (func)
		return func(win->object, x, object);
	else
		return -1;
}

WATOM watommenu = {
	US "menu",
	menudisp,
	menufllw,
	menuabort,
	umrtn,
	umkey,
	menuresz,
	menumove,
	NULL,
	NULL,
	TYPEMENU
};

void ldmenu(MENU *m, unsigned char **s, int cursor)
{
	m->list = s;
	m->cursor = cursor;
	mconfig(m);
}

MENU *mkmenu(W *w, unsigned char **s, int (*func) (/* ??? */), int (*abrt) (/* ??? */), int (*backs) (/* ??? */), int cursor, void *object, int *notify)
{
	W *new;
	MENU *m;
	int lines;
	int h = (w->main->h*40) / 100; /* 40% of window size */
	if (!h)
		h = 1;
	
	if (s) {
		lines = mlines(s,w->t->w-1);
		if (lines < h)
			h = lines;
	}

	new = wcreate(w->t, &watommenu, w, w, w->main, h, NULL, notify);

	if (!new) {
		if (notify)
			*notify = 1;
		return NULL;
	}
	wfit(new->t);
	new->object = (void *) (m = (MENU *) joe_malloc(sizeof(MENU)));
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
	m->top = 0;
	ldmenu(m, s, cursor);
	w->t->curwin = new;
	return m;
}

static unsigned char *cull(unsigned char *a, unsigned char *b)
{
	int x;

	for (x = 0; a[x] && b[x] && a[x] == b[x]; ++x) ;
	return vstrunc(a, x);
}

unsigned char *find_longest(unsigned char **lst)
{
	unsigned char *com;
	int x;

	if (!lst || !aLEN(lst))
		return vstrunc(NULL, 0);
	com = vsncpy(NULL, 0, sv(lst[0]));
	for (x = 1; x != aLEN(lst); ++x)
		com = cull(com, lst[x]);
	return com;
}

unsigned char *mcomplete(MENU *m)
{
	unsigned char *com;
	int x;

	if (!m->nitems)
		return vstrunc(NULL, 0);
	com = vsncpy(NULL, 0, sz(m->list[0]));
	for (x = 1; x != m->nitems; ++x)
		com = cull(com, m->list[x]);
	return com;
}
