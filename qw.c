/*
 *	Query windows
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include "utils.h"
#include "vs.h"
#include "w.h"

static void dispqw(QW *qw)
{
	W *w = qw->parent;

	/* Scroll buffer and position prompt */
	if (qw->promptlen > w->w / 2 + w->w / 4)
		qw->promptofst = qw->promptlen - w->w / 2;
	else
		qw->promptofst = 0;

	/* Set cursor position */
	w->curx = qw->promptlen - qw->promptofst;
	w->cury = 0;

	/* Generate prompt */
	w->t->t->updtab[w->y] = 1;
	genfield(w->t->t,
	         w->t->t->scrn + w->y * w->t->t->co + w->x,
	         w->t->t->attr + w->y * w->t->t->co + w->x,
	         w->x,
	         w->y,
	         qw->promptofst,
	         qw->prompt,
	         qw->promptlen,
	         0,
	         w->w - w->x,
	         1);
}

static void dispqwn(QW *qw)
{
	W *w = qw->parent;

	/* Scroll buffer and position prompt */
	if (qw->promptlen > w->w / 2 + w->w / 4)
		qw->promptofst = qw->promptlen - w->w / 2;
	else
		qw->promptofst = 0;

	/* Set cursor position */
	if (w->win->watom->follow && w->win->object)
		w->win->watom->follow(w->win->object);
	if (w->win->watom->disp && w->win->object)
		w->win->watom->disp(w->win->object);
	w->curx = w->win->curx;
	w->cury = w->win->cury + w->win->y - w->y;

	/* Generate prompt */
	w->t->t->updtab[w->y] = 1;
	genfield(w->t->t,
	         w->t->t->scrn + w->y * w->t->t->co + w->x,
	         w->t->t->attr + w->y * w->t->t->co + w->x,
	         w->x,
	         w->y,
	         qw->promptofst,
	         qw->prompt,
	         qw->promptlen,
	         0,
	         w->w - w->x,
	         1);
}

/* When user hits a key in a query window */

static int utypeqw(QW *qw, int c)
{
	W *win;
	W *w = qw->parent;
	int *notify = w->notify;
	int (*func) ();
	void *object = qw->object;

	win = qw->parent->win;
	func = qw->func;
	vsrm(qw->prompt);
	joe_free(qw);
	w->object = NULL;
	w->notify = NULL;
	wabort(w);
	if (func)
		return func(win->object, c, object, notify);
	return -1;
}

static int abortqw(QW *qw)
{
	W *win = qw->parent->win;
	void *object = qw->object;
	int (*abrt) () = qw->abrt;

	vsrm(qw->prompt);
	joe_free(qw);
	if (abrt)
		return abrt(win->object, object);
	else
		return -1;
}

static WATOM watomqw = {
	US "query",
	dispqw,
	NULL,
	abortqw,
	NULL,
	utypeqw,
	NULL,
	NULL,
	NULL,
	NULL,
	TYPEQW
};

static WATOM watqwn = {
	US "querya",
	dispqwn,
	NULL,
	abortqw,
	NULL,
	utypeqw,
	NULL,
	NULL,
	NULL,
	NULL,
	TYPEQW
};

static WATOM watqwsr = {
	US "querysr",
	dispqwn,
	NULL,
	abortqw,
	NULL,
	utypeqw,
	NULL,
	NULL,
	NULL,
	NULL,
	TYPEQW
};

/* Create a query window */

QW *mkqw(W *w, unsigned char *prompt, int len, int (*func) (/* ??? */), int (*abrt) (/* ??? */), void *object, int *notify)
{
	W *new;
	QW *qw;

	new = wcreate(w->t, &watomqw, w, w, w->main, 1, NULL, notify);
	if (!new) {
		if (notify)
			*notify = 1;
		return NULL;
	}
	wfit(new->t);
	new->object = (void *) (qw = (QW *) joe_malloc(sizeof(QW)));
	qw->parent = new;
	qw->prompt = vsncpy(NULL, 0, prompt, len);
	qw->promptlen = len;
	qw->promptofst = 0;
	qw->func = func;
	qw->abrt = abrt;
	qw->object = object;
	w->t->curwin = new;
	return qw;
}

/* Same as above, but cursor is left in original window */
/* For Ctrl-Meta thing */

QW *mkqwna(W *w, unsigned char *prompt, int len, int (*func) (/* ??? */), int (*abrt) (/* ??? */), void *object, int *notify)
{
	W *new;
	QW *qw;

	new = wcreate(w->t, &watqwn, w, w, w->main, 1, NULL, notify);
	if (!new) {
		if (notify)
			*notify = 1;
		return NULL;
	}
	wfit(new->t);
	new->object = (void *) (qw = (QW *) joe_malloc(sizeof(QW)));
	qw->parent = new;
	qw->prompt = vsncpy(NULL, 0, prompt, len);
	qw->promptlen = len;
	qw->promptofst = 0;
	qw->func = func;
	qw->abrt = abrt;
	qw->object = object;
	w->t->curwin = new;
	return qw;
}

/* Same as above, but cursor is left in original window */
/* For search and replace thing */

QW *mkqwnsr(W *w, unsigned char *prompt, int len, int (*func) (/* ??? */), int (*abrt) (/* ??? */), void *object, int *notify)
{
	W *new;
	QW *qw;

	new = wcreate(w->t, &watqwsr, w, w, w->main, 1, NULL, notify);
	if (!new) {
		if (notify)
			*notify = 1;
		return NULL;
	}
	wfit(new->t);
	new->object = (void *) (qw = (QW *) joe_malloc(sizeof(QW)));
	qw->parent = new;
	qw->prompt = vsncpy(NULL, 0, prompt, len);
	qw->promptlen = len;
	qw->promptofst = 0;
	qw->func = func;
	qw->abrt = abrt;
	qw->object = object;
	w->t->curwin = new;
	return qw;
}
