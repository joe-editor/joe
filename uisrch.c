/*
 *	Incremental search
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include "b.h"
#include "bw.h"
#include "main.h"
#include "queue.h"
#include "qw.h"
#include "tty.h"
#include "usearch.h"
#include "utils.h"
#include "vs.h"

extern int smode;
struct isrch *lastisrch = 0;	/* Previous search */

unsigned char *lastpat = 0;	/* Previous pattern */

IREC fri = { {&fri, &fri} };	/* Free-list of irecs */

static IREC *alirec(void)
{				/* Allocate an IREC */
	return alitem(&fri, sizeof(IREC));
}

static void frirec(IREC *i)
{				/* Free an IREC */
	enquef(IREC, link, &fri, i);
}

static void rmisrch(struct isrch *isrch)
{				/* Eliminate a struct isrch */
	if (isrch) {
		vsrm(isrch->pattern);
		frchn(&fri, &isrch->irecs);
		joe_free(isrch);
	}
}

static int iabrt(BW *bw, struct isrch *isrch)
{				/* User hit ^C */
	rmisrch(isrch);
	return -1;
}

static void iappend(BW *bw, struct isrch *isrch, unsigned char *s, int len)
{				/* Append text and search */
	/* Append char and search */
	IREC *i = alirec();

	i->what = len;
	i->disp = bw->cursor->byte;
	isrch->pattern = vsncpy(sv(isrch->pattern), s, len);
	if (!qempty(IREC, link, &isrch->irecs)) {
		pgoto(bw->cursor, isrch->irecs.link.prev->start);
	}
	i->start = bw->cursor->byte;
	if (dopfnext(bw, mksrch(vsncpy(NULL, 0, isrch->pattern + isrch->ofst, sLen(isrch->pattern) - isrch->ofst), NULL, 0, isrch->dir, -1, 0, 0), NULL)) {
		ttputc(7);
	}
	enqueb(IREC, link, &isrch->irecs, i);
}

/* Main user interface */
static int itype(BW *bw, int c, struct isrch *isrch, int *notify)
{
	IREC *i;
	int omid;

	if (isrch->quote) {
		goto in;
	}
	if (c == 8 || c == 127) {	/* Backup */
		if ((i = isrch->irecs.link.prev) != &isrch->irecs) {
			pgoto(bw->cursor, i->disp);
			omid = mid;
			mid = 1;
			dofollows();
			mid = omid;
			isrch->pattern = vstrunc(isrch->pattern, sLEN(isrch->pattern) - i->what);
			frirec(deque_f(IREC, link, i));
		} else {
			ttputc(7);
		}
	} else if (c == 'Q' - '@' || c == '`') {
		isrch->quote = 1;
	} else if (c == 'S' - '@' || c == '\\' - '@' || c == 'L' - '@' || c == 'R' - '@') {
		/* Repeat */
		if (c == 'R' - '@') {
			isrch->dir = 1;
		} else {
			isrch->dir = 0;
		}
		if (qempty(IREC, link, &isrch->irecs)) {
			if (lastpat && lastpat[0]) {
				iappend(bw, isrch, sv(lastpat));
			}
		} else {
			i = alirec();
			i->disp = i->start = bw->cursor->byte;
			i->what = 0;
			if (dopfnext(bw, mksrch(vsncpy(NULL, 0, isrch->pattern + isrch->ofst, sLen(isrch->pattern) - isrch->ofst), NULL, 0, isrch->dir, -1, 0, 0), NULL)) {
				ttputc(7);
				frirec(i);
			} else {
				enqueb(IREC, link, &isrch->irecs, i);
			}
		}
	} else if ((c < 32 || c >= 256) && c != MAXINT) {	/* FIXME: overloaded MAXINT */
		/* Done */
		nungetc(c);
		if (notify) {
			*notify = 1;
		}
		smode = 2;
		if (lastisrch) {
			lastpat = vstrunc(lastpat, 0);
			lastpat = vsncpy(lastpat, 0, lastisrch->pattern + lastisrch->ofst, sLen(lastisrch->pattern) - lastisrch->ofst);
			rmisrch(lastisrch);
		}
		lastisrch = isrch;
		return 0;
	} else if (c != MAXINT) {	/* FIXME: overloaded MAXINT */
		/* Search */
		unsigned char k;

	      in:
		k = c;
		isrch->quote = 0;
		iappend(bw, isrch, &k, 1);
	}
	omid = mid;
	mid = 1;
	bw->cursor->xcol = piscol(bw->cursor);
	dofollows();
	mid = omid;
	if (mkqwnsr(bw->parent, sv(isrch->pattern), itype, iabrt, isrch, notify)) {
		return 0;
	} else {
		rmisrch(isrch);
		return -1;
	}
}

static int doisrch(BW *bw, int dir)
{				/* Create a struct isrch */
	struct isrch *isrch = (struct isrch *) joe_malloc(sizeof(struct isrch));

	izque(IREC, link, &isrch->irecs);
	isrch->pattern = vsncpy(NULL, 0, sc("I-find: "));
	isrch->ofst = sLen(isrch->pattern);
	isrch->dir = dir;
	isrch->quote = 0;
	return itype(bw, MAXINT, isrch, NULL);
}

int uisrch(BW *bw)
{
	if (smode && lastisrch) {
		struct isrch *isrch = lastisrch;

		lastisrch = 0;
		return itype(bw, 'S' - '@', isrch, NULL);
	} else {
		if (lastisrch) {
			lastpat = vstrunc(lastpat, 0);
			lastpat = vsncpy(lastpat, 0, lastisrch->pattern + lastisrch->ofst, sLen(lastisrch->pattern) - lastisrch->ofst);
			rmisrch(lastisrch);
			lastisrch = 0;
		}
		return doisrch(bw, 0);
	}
}

int ursrch(BW *bw)
{
	if (smode && lastisrch) {
		struct isrch *isrch = lastisrch;

		lastisrch = 0;
		return itype(bw, 'R' - '@', isrch, NULL);
	} else {
		if (lastisrch) {
			lastpat = vstrunc(lastpat, 0);
			lastpat = vsncpy(lastpat, 0, lastisrch->pattern + lastisrch->ofst, sLen(lastisrch->pattern) - lastisrch->ofst);
			rmisrch(lastisrch);
			lastisrch = 0;
		}
		return doisrch(bw, 1);
	}
}
