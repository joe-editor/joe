/*
 *	Prompt windows
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "types.h"

/* The current directory */

int bg_prompt;
int nocurdir;

char *get_cd(W *w)
{
	BW *bw;
	w = w->main;
	bw = (BW *)w->object;
	return bw->b->current_dir;
}

void set_current_dir(BW *bw, char *s,int simp)
{
	W *w = bw->parent->main;
	B *b;
	bw = (BW *)w->object;
	b = bw->b;
	
	if (s[0]=='!' || (s[0]=='>' && s[1]=='>'))
		return;
	obj_free(b->current_dir);
	if (s) {
		b->current_dir=dirprt(s);
		if (simp) {
			b->current_dir = simplify_prefix(b->current_dir);
		}
	}
	else
		b->current_dir = 0;
	obj_perm(b->current_dir);
}

static void disppw(W *w, int flg)
{
	BW *bw = (BW *)w->object;
	char *bf;
	int i;
	PW *pw = (PW *) bw->object;

	if (!flg) {
		return;
	}

	/* Try a nice graphics separator */
	bf = joe_malloc(w->w + 1);
	for (i = 0; i != w->w; ++i)
		bf[i] = '-';
	bf[i] = 0;
	genfmt(w->t->t, w->x, w->y, 0, bf, bg_stalin, 0);
	joe_free(bf);

	/* Scroll buffer and position prompt */
	if (pw->promptlen > w->w - 5) {
		pw->promptofst = pw->promptlen - w->w / 2;
		if (piscol(bw->cursor) < w->w - (pw->promptlen - pw->promptofst)) {
			bw->offset = 0;
		} else {
			bw->offset = piscol(bw->cursor) - (w->w - (pw->promptlen - pw->promptofst) - 1);
		}
	} else {
		if (piscol(bw->cursor) < w->w - pw->promptlen) {
			pw->promptofst = 0;
			bw->offset = 0;
		} else if (piscol(bw->cursor) >= w->w) {
			pw->promptofst = pw->promptlen;
			bw->offset = piscol(bw->cursor) - (w->w - 1);
		} else {
			pw->promptofst = pw->promptlen - TO_DIFF_OK((w->w - piscol(bw->cursor) - 1));
			bw->offset = piscol(bw->cursor) - (w->w - (pw->promptlen - pw->promptofst) - 1);
		}
	}

	/* Set cursor position */
	w->curx = TO_DIFF_OK(piscol(bw->cursor) - bw->offset + pw->promptlen - pw->promptofst);
	w->cury = bw->cursor->line - bw->top->line + 1;
	/* w->cury = w->h - 1; */

	/* Generate prompt */
	w->t->t->updtab[w->y + w->cury] = 1;
	if (w->cury != pw->oldcury) {
		int n;
		for (n = 0; n != w->h; ++n)
			w->t->t->updtab[w->y + n] = 1;
		pw->oldcury = w->cury;
	}
	/* w->t->t->updtab[w->y + w->h - 1] = 1;
	genfmt(w->t->t, w->x, w->y + w->h - 1, pw->promptofst, pw->prompt, BG_COLOR(bg_prompt), 0); */

	/* Position and size buffer */
	/* bwmove(bw, w->x + pw->promptlen - pw->promptofst, w->y);
	bwresz(bw, w->w - (pw->promptlen - pw->promptofst), w->h); */
	bwmove(bw, w->x, w->y + 1);
	bwresz(bw, w->w, w->h - 1);

	/* Generate buffer */
	bwgen(bw, 0);
}

/* History functions */

void setup_history(B **history)
{
	if (!*history) {
		*history = bmk(NULL);
	}
}

/* Add line to history buffer */

void append_history(B *hist,char *s,ptrdiff_t len)
{
	P *q = pdup(hist->eof, "append_history");
	binsm(q, s, len);
	p_goto_eof(q);
	binsc(q, '\n');
	prm(q);
}

/* Promote line to end of history buffer */

void promote_history(B *hist, off_t line)
{
	P *q = pdup(hist->bof, "promote_history");
	P *r;
	P *t;

	pline(q, line);
	r = pdup(q, "promote_history");
	pnextl(r);
	t = pdup(hist->eof, "promote_history");
	binsb(t, bcpy(q, r));
	bdel(q, r);
	prm(q);
	prm(r);
	prm(t);
}

/* When user hits return in a prompt window */

static int rtnpw(W *w)
{
	BW *bw = (BW *)w->object;
	PW *pw = (PW *)bw->object;
	char *s;
	W *win;
	int (*pfunc)(W *w, char *s, void *object);
	void *object;
	off_t byte;

	/* Extract entered text from buffer */
	p_goto_eol(bw->cursor);
	byte = bw->cursor->byte;
	p_goto_bol(bw->cursor);
	s = brvs(NULL, bw->cursor, byte - bw->cursor->byte);

	if (pw->file_prompt) {
		s = canonical(s);
	}

	/* Save text into history buffer */
	if (pw->hist) {
		if (bw->b->changed) {
			append_history(pw->hist, sv(s));
		} else {
			promote_history(pw->hist, bw->cursor->line);
		}
	}

	/* Do ~ expansion and set new current directory */
	if (pw->file_prompt&2) {
		set_current_dir(bw, s,1);
	}

	win = w->win;
	pfunc = pw->pfunc;
	object = pw->object;
	bwrm(bw);
	joe_free(pw->prompt);
	joe_free(pw);
	w->object = NULL;
	wabort(w);
	dostaupd = 1;

	/* Call callback function */
	if (pfunc) {
		return pfunc(win, s, object);
	} else {
		return -1;
	}
}

int ucmplt(W *w, int k)
{
	BW *bw;
	PW *pw;
	WIND_BW(bw, w);
	pw = (PW *) bw->object;

	if (pw->tab) {
		return pw->tab(bw, k);
	} else {
		return -1;
	}
}

static void inspw(W *w, B *b, off_t l, off_t n, int flg)
{
	BW *bw = (BW *)w->object;
	if (b == bw->b) {
		bwins(bw, l, n, flg);
	}
}

static void delpw(W *w, B *b, off_t l, off_t n, int flg)
{
	BW *bw = (BW *)w->object;
	if (b == bw->b) {
		bwdel(bw, l, n, flg);
	}
}

static int abortpw(W *w)
{
	BW *bw = (BW *)w->object;
	PW *pw = (PW *)bw->object;
	void *object = pw->object;
	int (*abrt)(W *w, void *object) = pw->abrt;
	W *win = bw->parent->win;

	bwrm(bw);
	joe_free(pw->prompt);
	joe_free(pw);
	if (abrt) {
		return abrt(win, object);
	} else {
		return -1;
	}
}

WATOM watompw = {
	"prompt",
	disppw,
	bwfllwt,
	abortpw,
	rtnpw,
	utypew,
	NULL,
	NULL,
	inspw,
	delpw,
	TYPEPW
};

/* Create a prompt window */

BW *wmkpw(W *w, const char *prompt, B **history, int (*func) (W *w, char *s, void *object), const char *huh,
          int (*abrt)(W *w, void *object),
          int (*tab)(BW *bw, int k), void *object, struct charmap *map, int file_prompt)
{
	W *neww;
	PW *pw;
	BW *bw;

	neww = wcreate(w->t, &watompw, w, w, w->main, 2, huh);
	if (!neww) {
		return NULL;
	}
	neww->fixed = 0;
	wfit(neww->t);
	neww->object = (void *) (bw = bwmk(neww, bmk(NULL), 0, prompt));
	bw->b->o.charmap = map;
	bw->object = (void *) (pw = (PW *) joe_malloc(SIZEOF(PW)));
	pw->abrt = abrt;
	pw->oldcury = -1;
	pw->tab = tab;
	pw->object = object;
	pw->prompt = zdup(prompt);
	pw->promptlen = fmtlen(prompt);
	pw->promptofst = 0;
	pw->pfunc = func;
	pw->file_prompt = file_prompt;
	if (pw->file_prompt) {
		bw->b->o.syntax = load_syntax("filename");
		bw->b->o.highlight = 1;
		bw->o.syntax = bw->b->o.syntax;
		bw->o.highlight = bw->b->o.highlight;
	}
	if (history) {
		setup_history(history);
		pw->hist = *history;
		binsb(bw->cursor, bcpy(pw->hist->bof, pw->hist->eof));
		bw->b->changed = 0;
		p_goto_eof(bw->cursor);
		/* p_goto_eof(bw->top);
		p_goto_bol(bw->top); */
	} else {
		pw->hist = NULL;
	}
	/* Install current directory */
	if ((file_prompt&4) && !nocurdir) {
		char *curd = get_cd(w);
		binsmq(bw->cursor, sv(curd));
		p_goto_eof(bw->cursor);
		bw->cursor->xcol = piscol(bw->cursor);
	}
	w->t->curwin = neww;
	return bw;
}

/* Tab completion functions */

char **regsub(char **z, ptrdiff_t len, char *s)
{
	char **lst = NULL;
	int x;

	for (x = 0; x != len; ++x)
		if (rmatch(s, z[x]))
			lst = vaadd(lst, vsncpy(NULL, 0, sz(z[x])));
	return lst;
}

void cmplt_ins(BW *bw, char *line)
{
	P *p = pdup(bw->cursor, "cmplt_ins");

	p_goto_bol(p);
	p_goto_eol(bw->cursor);
	bdel(p, bw->cursor);
	binsm(bw->cursor, sv(line));
	p_goto_eol(bw->cursor);
	prm(p);
	bw->cursor->xcol = piscol(bw->cursor);
}

static void p_goto_bow(P *ptr)
{
	struct charmap *map = ptr->b->o.charmap;
	int c;
	while (joe_isalnum_(map, (c = prgetc(ptr))))
		;
	if (c != NO_MORE_DATA)
		pgetc(ptr);
}

static void p_goto_eow(P *ptr)
{
	struct charmap *map = ptr->b->o.charmap;
	while (joe_isalnum_(map, brch(ptr)))
		pgetc(ptr);
}

static void word_ins(BW *bw, char *line)
{
	P *p = pdup(bw->cursor, "cmplt_ins");

	p_goto_bow(p);
	p_goto_eow(bw->cursor);
	bdel(p, bw->cursor);
	binsm(bw->cursor, sv(line));
	p_goto_eol(bw->cursor);
	prm(p);
	bw->cursor->xcol = piscol(bw->cursor);
}

int cmplt_abrt(W *w, ptrdiff_t x, void *object)
{
	char *line = (char *)object;
	if (line) {
		/* cmplt_ins(bw, line); */
		obj_free(line);
	}
	return -1;
}

int cmplt_rtn(MENU *m, ptrdiff_t x, void *object, int k)
{
	char *line = (char *)object;
	cmplt_ins((BW *)m->parent->win->object, m->list[x]);
	obj_free(line);
	m->object = NULL;
	wabort(m->parent);
	return 0;
}

static int word_rtn(MENU *m, ptrdiff_t x, void *object, int k)
{
	char *line = (char *)object;
	word_ins((BW *)m->parent->win->object, m->list[x]);
	obj_free(line);
	m->object = NULL;
	wabort(m->parent);
	return 0;
}

int simple_cmplt(BW *bw,char **list)
{
	MENU *m;
	P *p, *q;
	char *line;
	char *line1;
	char **lst;

	p = pdup(bw->cursor, "simple_cmplt");
	p_goto_bol(p);
	q = pdup(bw->cursor, "simple_cmplt");
	p_goto_eol(q);
	line = brvs(NULL, p, q->byte - p->byte);	/* FIXME: Assumes short lines :-) */
	prm(p);
	prm(q);

	line1 = vsncpy(NULL, 0, sv(line));
	line1 = vsadd(line1, '*');
	lst = regsub(av(list), line1);

	if (!lst) {
		ttputc(7);
		return -1;
	}

	if (menu_above) {
		if (bw->parent->link.prev->watom==&watommenu) {
			wabort(bw->parent->link.prev);
		}
	} else {
		if (bw->parent->link.next->watom==&watommenu) {
			wabort(bw->parent->link.next);
		}
	}

	obj_perm(line);
	vaperm(lst);
	m = mkmenu((menu_above ? bw->parent->link.prev : bw->parent), bw->parent, lst, cmplt_rtn, cmplt_abrt, NULL, 0, line);
	if (!m) {
		return -1;
	}
	if (valen(lst) == 1)
		return cmplt_rtn(m, 0, line, 0);
	else if (smode || isreg(line)) {
		if (!menu_jump)
			bw->parent->t->curwin=bw->parent;
		return 0;
	} else {
		char *com = mcomplete(m);

		obj_free(m->object);
		m->object = com;
		obj_perm(com);
		
		cmplt_ins(bw, com);
		wabort(m->parent);
		smode = 2;
		ttputc(7);
		return 0;
	}
}

int word_cmplt(BW *bw,char **list)
{
	MENU *m;
	P *p, *q;
	char *line;
	char *line1;
	char **lst;

	p = pdup(bw->cursor, "word_cmplt");
	p_goto_bow(p);
	q = pdup(bw->cursor, "word_cmplt");
	p_goto_eow(q);
	line = brvs(NULL, p, q->byte - p->byte);
	prm(p);
	prm(q);

	line1 = vsncpy(NULL, 0, sv(line));
	line1 = vsadd(line1, '*');
	lst = regsub(list, valen(list), line1);

	if (!lst) {
		ttputc(7);
		return -1;
	}

	if (menu_above) {
		if (bw->parent->link.prev->watom==&watommenu) {
			wabort(bw->parent->link.prev);
		}
	} else {
		if (bw->parent->link.next->watom==&watommenu) {
			wabort(bw->parent->link.next);
		}
	}

	m = mkmenu((menu_above ? bw->parent->link.prev : bw->parent), bw->parent, lst, word_rtn, cmplt_abrt, NULL, 0, line);
	if (!m) {
		return -1;
	}
	if (valen(lst) == 1)
		return word_rtn(m, 0, line, 0);
	else if (smode || isreg(line)) {
		if (!menu_jump)
			bw->parent->t->curwin=bw->parent;
		return 0;
	} else {
		char *com = mcomplete(m);

		obj_free(m->object);
		m->object = com;
		obj_perm(m->object);
		
		word_ins(bw, com);
		wabort(m->parent);
		smode = 2;
		ttputc(7);
		return 0;
	}
}

/* Simplified prompting... convert original event-driven style to
 * coroutine model */

struct prompt_result {
	Coroutine t;
	char *answer;
};

int prompt_cont(W *w, char *s, void *object)
{
	struct prompt_result *r = (struct prompt_result *)object;
	r->answer = s;

	/* move answer to original coroutine's obj_stack */
	obj_perm(r->answer);

	co_resume(&r->t, 0);

	return 0;
}

int prompt_abrt(W *w, void *object)
{
	struct prompt_result *r = (struct prompt_result *)object;
	r->answer = 0;
	co_resume(&r->t, -1);
	return -1;
}

char *ask(W *w,			/* Prompt goes below this window */
          const char *prompt,		/* Prompt text */
          B **history,			/* History buffer to use */
          char *huh,			/* Name of help screen for this prompt */
          int (*tab)(),		/* Called when tab key is pressed */
          struct charmap *map,		/* Character map for prompt */
          int file_prompt,		/* Set for file-name tilde expansion */
          int retrieve_last,		/* Set for cursor to go on last line of history */
          char *preload)		/* Text to preload into prompt */
{
	struct prompt_result t;
	BW *bw = wmkpw(w, prompt, history, prompt_cont, huh, prompt_abrt, tab, 
	               &t, map, file_prompt);
	if (!bw)
		return 0;

	bw->parent->coro = &t.t;
	if (preload) {
		/* Load hint, put cursor after it */
		binss(bw->cursor, preload);
		pset(bw->cursor, bw->b->eof);
		bw->cursor->xcol = piscol(bw->cursor);
	} else if (retrieve_last) {
		/* One step back through history */
		uuparw(w, NO_MORE_DATA);
		u_goto_eol(w, NO_MORE_DATA);
		bw->cursor->xcol = piscol(bw->cursor);
	}

	/* We get woken up when user hits return */
	if (!co_yield(&t.t, 0)) {
		/* Moving answer to original coroutine's stack */
		obj_temp(t.answer);
		return t.answer;
	} else {
		return 0;
	}
}
