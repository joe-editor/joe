/*
 *	Compiler error handler
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <stdio.h>

#include "b.h"
#include "bw.h"
#include "main.h"
#include "queue.h"
#include "tw.h"
#include "ufile.h"
#include "utils.h"
#include "vs.h"
#include "charmap.h"
#include "w.h"

/* Error database */

typedef struct error ERROR;

struct error {
	LINK(ERROR) link;	/* Linked list of errors */
	long line;		/* Target line number */
	long org;		/* Original target line number */
	unsigned char *file;		/* Target file name */
	long src;		/* Error-file line number */
	unsigned char *msg;		/* The message */
} errors = { { &errors, &errors} };
ERROR *errptr = &errors;	/* Current error row */

B *errbuf = NULL;		/* Buffer with error messages */

/* Insert and delete notices */

void inserr(unsigned char *name, long int where, long int n, int bol)
{
	ERROR *e;

	if (name) {
		for (e = errors.link.next; e != &errors; e = e->link.next) {
			if (!strcmp(e->file, name)) {
				if (e->line > where)
					e->line += n;
				else if (e->line == where && bol)
					e->line += n;
			}
		}
	}
}

void delerr(unsigned char *name, long int where, long int n)
{
	ERROR *e;

	if (name) {
		for (e = errors.link.next; e != &errors; e = e->link.next) {
			if (!strcmp(e->file, name)) {
				if (e->line > where + n)
					e->line -= n;
				else if (e->line > where)
					e->line = where;
			}
		}
	}
}

/* Abort notice */

void abrerr(unsigned char *name)
{
	ERROR *e;

	if (name)
		for (e = errors.link.next; e != &errors; e = e->link.next)
			if (!strcmp(e->file, name))
				e->line = e->org;
}

/* Save notice */

void saverr(unsigned char *name)
{
	ERROR *e;

	if (name)
		for (e = errors.link.next; e != &errors; e = e->link.next)
			if (!strcmp(e->file, name))
				e->org = e->line;
}

/* Pool of free error nodes */
ERROR errnodes = { {&errnodes, &errnodes} };

/* Free an error node */

static void freeerr(ERROR *n)
{
	vsrm(n->file);
	vsrm(n->msg);
	enquef(ERROR, link, &errnodes, n);
}

/* Free all errors */

static void freeall(void)
{
	while (!qempty(ERROR, link, &errors))
		freeerr(deque_f(ERROR, link, errors.link.next));
	errptr = &errors;
}

/* Parse error messages into database */

/* From joe's joe 2.9 */

/* First word (allowing ., /, _ and -) with a . is the file name.  Next number
   is line number.  Then there should be a ':' */

static void parseone(struct charmap *map,unsigned char *s,unsigned char **rtn_name,long *rtn_line)
{
	int x, y, flg;
	unsigned char *name = NULL;
	long line = -1;

	y=0;
	flg=0;

	do {
		/* Skip to first word */
		for (x = y; s[x] && !(joe_isalnum_(map,s[x]) || s[x] == '.' || s[x] == '/'); ++x) ;

		/* Skip to end of first word */
		for (y = x; joe_isalnum_(map,s[y]) || s[y] == '.' || s[y] == '/' || s[y]=='-'; ++y)
			if (s[y] == '.')
				flg = 1;
	} while (!flg && x!=y);

	/* Save file name */
	if (x != y)
		name = vsncpy(NULL, 0, s + x, y - x);

	/* Skip to first number */
	for (x = y; s[x] && (s[x] < '0' || s[x] > '9'); ++x) ;

	/* Skip to end of first number */
	for (y = x; s[y] >= '0' && s[y] <= '9'; ++y) ;

	/* Save line number */
	if (x != y)
		sscanf((char *)(s + x), "%ld", &line);
	if (line != -1)
		--line;

	/* Look for ':' */
	flg = 0;
	while (s[y]) {
		if (s[y]==':') {
			flg = 1;
			break;
		}
		++y;
	}

	if (!flg)
		line = -1;

	*rtn_name = name;
	*rtn_line = line;
}

static int parseit(struct charmap *map,unsigned char *s, long int row)
{
	unsigned char *name = NULL;
	long line = -1;
	ERROR *err;

	parseone(map,s,&name,&line);

	if (name) {
		if (line != -1) {
			/* We have an error */
			err = (ERROR *) alitem(&errnodes, sizeof(ERROR));
			err->file = name;
			err->org = err->line = line;
			err->src = row;
			err->msg = vsncpy(NULL, 0, sc("\\i"));
			err->msg = vsncpy(sv(err->msg), sv(s));
			enqueb(ERROR, link, &errors, err);
			return 1;
		} else
			vsrm(name);
	}
	return 0;
}

/* Parse the error output contained in a buffer */

static long parserr(B *b)
{
	P *p = pdup(b->bof);
	P *q = pdup(p);
	long nerrs = 0;

	freeall();
	do {
		unsigned char *s;

		pset(q, p);
		p_goto_eol(p);
		s = brvs(q, (int) (p->byte - q->byte));
		if (s) {
			nerrs += parseit(b->o.charmap, s, q->line);
			vsrm(s);
		}
	} while (pgetc(p) != NO_MORE_DATA);
	prm(p);
	prm(q);
	return nerrs;
}

BW *find_a_good_bw(B *b)
{
	W *w;
	BW *bw = 0;
	/* Find lowest window with buffer */
	if ((w = maint->topwin) != NULL) {
		do {
			if ((w->watom->what&TYPETW) && ((BW *)w->object)->b==b && w->y>=0)
				bw = (BW *)w->object;
			w = w->link.next;
		} while (w != maint->topwin);
	}
	if (bw)
		return bw;
	/* Otherwise just find lowest window */
	if ((w = maint->topwin) != NULL) {
		do {
			if ((w->watom->what&TYPETW) && w->y>=0)
				bw = (BW *)w->object;
			w = w->link.next;
		} while (w != maint->topwin);
	}
	return bw;
}

int parserrb(B *b)
{
	BW *bw;
	int n;
	errbuf = b;
	freeall();
	n = parserr(b);
	bw = find_a_good_bw(b);
	if (n)
		joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "%ld messages found", n);
	else
		joe_snprintf_0((char *)msgbuf, JOE_MSGBUFSIZE, "No messages found");
	msgnw(bw->parent, msgbuf);
	return 0;
}

int uparserr(BW *bw)
{
	int n;
	errbuf = bw->b;
	freeall();
	n = parserr(bw->b);
	if (n)
		joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "%ld messages found", n);
	else
		joe_snprintf_0((char *)msgbuf, JOE_MSGBUFSIZE, "No messages found");
	msgnw(bw->parent, msgbuf);
	return 0;
}

int jump_to_file_line(BW *bw,unsigned char *file,int line,unsigned char *msg)
{
	int omid;
	if (!bw->b->name || strcmp(file, bw->b->name)) {
		if (doswitch(bw, vsdup(file), NULL, NULL))
			return -1;
		bw = (BW *) maint->curwin->object;
	}
	omid = mid;
	mid = 1;
	pline(bw->cursor, line);
	dofollows();
	mid = omid;
	bw->cursor->xcol = piscol(bw->cursor);
	msgnw(bw->parent, msg);
	return 0;
}

/* Find line in error database: return pointer to message */

unsigned char *srcherr(BW *bw,unsigned char *file,long line)
{
	ERROR *p;
	for (p = errors.link.next; p != &errors; p=p->link.next)
		if (!strcmp(p->file,file) && p->org==line) {
			errptr = p;
			setline(errbuf, errptr->src);
			return errptr->msg;
			}
	return 0;
}

int ujump(BW *bw)
{
	int rtn = -1;
	P *p = pdup(bw->cursor);
	P *q = pdup(p);
	unsigned char *s;
	p_goto_bol(p);
	p_goto_eol(q);
	s = brvs(p, (int) (q->byte - p->byte));
	prm(p);
	prm(q);
	if (s) {
		unsigned char *name = NULL;
		long line = -1;
		parseone(bw->b->o.charmap,s,&name,&line);
		if (name && line != -1) {
			unsigned char *msg = srcherr(bw, name, line);
			unextw((BASE *)bw);
			/* Check that we made it to a tw */
			rtn = jump_to_file_line(maint->curwin->object,name,line,msg);
			vsrm(name);
		}
		vsrm(s);
	}
	return rtn;
}

int unxterr(BW *bw)
{
	if (errptr->link.next == &errors) {
		msgnw(bw->parent, US "No more errors");
		return -1;
	}
	errptr = errptr->link.next;
	setline(errbuf, errptr->src);
	return jump_to_file_line(bw,errptr->file,errptr->line,errptr->msg);
}

int uprverr(BW *bw)
{
	if (errptr->link.prev == &errors) {
		msgnw(bw->parent, US "No more errors");
		return -1;
	}
	errptr = errptr->link.prev;
	setline(errbuf, errptr->src);
	return jump_to_file_line(bw,errptr->file,errptr->line,errptr->msg);
}
