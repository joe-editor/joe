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

/* First word on line with a '.' in it.  This is the file name.  The next number after that is the line number. */

static int parseit(int wide,unsigned char *s, long int row)
{
	int x, y, flg;
	unsigned char *name = NULL;
	long line = -1;
	ERROR *err;

	y=0;
	flg=0;

	do {
		/* Skip to first word */
		for (x = y; s[x] && !(isalnum_(wide,s[x]) || s[x] == '.' || s[x] == '/'); ++x) ;

		/* Skip to end of first word */
		for (y = x; isalnum_(wide,s[y]) || s[y] == '.' || s[y] == '/'; ++y)
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
			nerrs += parseit(b->o.utf8, s, q->line);
			vsrm(s);
		}
	} while (pgetc(p) != NO_MORE_DATA);
	prm(p);
	prm(q);
	return nerrs;
}

int uparserr(BW *bw)
{
	errbuf = bw->b;
	freeall();
	snprintf((char *)msgbuf, JOE_MSGBUFSIZE, "Parsed %ld lines", parserr(bw->b));
	msgnw(bw->parent, msgbuf);
	return 0;
}

int unxterr(BW *bw)
{
	int omid;

	if (errptr->link.next == &errors) {
		msgnw(bw->parent, US "No more errors");
		return -1;
	}
	errptr = errptr->link.next;
	if (!bw->b->name || strcmp(errptr->file, bw->b->name)) {
		if (doedit(bw, vsdup(errptr->file), NULL, NULL))
			return -1;
		bw = (BW *) maint->curwin->object;
	}
	omid = mid;
	mid = 1;
	pline(bw->cursor, errptr->line);
	setline(errbuf, errptr->src);
	dofollows();
	mid = omid;
	bw->cursor->xcol = piscol(bw->cursor);
	msgnw(bw->parent, errptr->msg);
	return 0;
}

int uprverr(BW *bw)
{
	int omid;

	if (errptr->link.prev == &errors) {
		msgnw(bw->parent, US "No more errors");
		return -1;
	}
	errptr = errptr->link.prev;
	if (!bw->b->name || strcmp(errptr->file, bw->b->name)) {
		if (doedit(bw, vsdup(errptr->file), NULL, NULL))
			return -1;
		bw = (BW *) maint->curwin->object;
	}
	omid = mid;
	mid = 1;
	pline(bw->cursor, errptr->line);
	setline(errbuf, errptr->src);
	dofollows();
	mid = omid;
	bw->cursor->xcol = piscol(bw->cursor);
	msgnw(bw->parent, errptr->msg);
	return 0;
}
