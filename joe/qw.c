/*
 *	Query windows
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "types.h"

/* Event handler: display query window */

static void dispqw(W *w, int flg)
{
	QW *qw = (QW *)w->object;
	ptrdiff_t y;

	/* Generate prompt */
	for (y = 0; y != w->h; ++y) {
		const char *s = qw->prompt;
		ptrdiff_t l = qw->promptlen;
		break_height(locale_map, &s, &l, qw->org_w, y);
		w->t->t->updtab[w->y + y] = 1;
		genfield(w->t->t,
		         w->t->t->scrn + (w->y + y) * w->t->t->co + w->x,
		         w->t->t->attr + (w->y + y) * w->t->t->co + w->x,
		         w->x,
		         w->y + y,
		         0,
		         s,
		         l,
		         BG_COLOR(bg_prompt),
		         w->w - w->x,
		         1,NULL);
		w->cury = y;
		w->curx = w->x + txtwidth(locale_map, s, l);
	}
}

/* Display query window: leave cursor in target window */

static void dispqwn(W *w, int flg)
{
	/* Set cursor position */
	if (w->win->watom->follow && w->win->object)
		w->win->watom->follow(w->win);
	if (w->win->watom->disp && w->win->object)
		w->win->watom->disp(w->win, 1);
	dispqw(w, flg);
	w->curx = w->win->curx;
	w->cury = w->win->cury + w->win->y - w->y;
}

/* When user hits a key in a query window */

static int utypeqw(W *w, int c)
{
	QW *qw = (QW *)w->object;
	struct query_result *r;
	int flg = qw->flg;
	r = qw->result;
	obj_free(qw->prompt);
	joe_free(qw);
	w->object = NULL;
	wabort(w);
	r->answer = c;
	if (flg & QW_NOMACRO) {
		co_sched(&r->t, 0);
		return 0;
	} else
		return co_resume(&r->t, 0);
}

static int abortqw(W *w)
{
	QW *qw = (QW *)w->object;
	struct query_result *r = qw->result;
	int flg = qw->flg;

	obj_free(qw->prompt);
	joe_free(qw);
	r->answer = -1;
	if (flg & QW_NOMACRO) {
		co_sched(&r->t, -1);
		return -1;
	} else
		return co_resume(&r->t, -1);
}

static WATOM watomqw = {
	"query",
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
	"querya",
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
	"querysr",
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

int query(W *w,				/* Prompt goes below this window */
          const char *prompt,		/* Prompt text */
          int len,			/* Length of prompt text */
          int flg)			/* Options: 0 = normal, 1 = cursor left in original,
						    2 = same as 1, but QW type code is different. */
{
	struct query_result t;
	QW *qw;
	W *n;
	WATOM *a = &watomqw;
	ptrdiff_t l = len;
	const char *s = prompt;
	ptrdiff_t h = break_height(locale_map, &s, &l, w->w, -1);
	
	a = &watomqw;
	if (flg & QW_STAY)
		a = &watqwn;
	if (flg & QW_SR)
		a = &watqwsr;
	n = wcreate(w->t, a, w, w, w->main, h, NULL);
	if (!n) {
		return -1;
	}
	wfit(n->t);
	n->object = (void *) (qw = (QW *) joe_malloc(SIZEOF(QW)));
	qw->parent = n;
	qw->prompt = vsncpy(NULL, 0, prompt, len);
	obj_perm(qw->prompt);
	qw->promptlen = len;
	qw->org_w = w->w;
	qw->org_h = h;
	qw->result = &t;
	qw->flg = flg;
	w->t->curwin = n;
	/* We get woken up when user hits a key */
	t.answer = -1;
	if (flg & QW_NOMACRO)
		co_suspend(&t.t, 0);
	else
		co_yield(&t.t, 0);
	return t.answer;
}
