/*
 *	Search & Replace system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <ctype.h>
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "b.h"
#include "bw.h"
#include "main.h"
#include "pw.h"
#include "queue.h"
#include "qw.h"
#include "regex.h"
#include "ublock.h"
#include "uedit.h"
#include "undo.h"
#include "usearch.h"
#include "vs.h"
#include "w.h"

int smode = 0;			/* Decremented to zero by execmd */
int csmode = 0;			/* Set for continued search mode */

B *findhist = 0;		/* Search string history */
B *replhist = 0;		/* Replacement string history */

SRCH *globalsrch = 0;		/* Most recent completed search data */

SRCHREC fsr = { {&fsr, &fsr} };

/* Search forward.
   bw, pattern and ignore must be set

   The first possible string we can find is the one beginning under p

   Returns p if we found a string:
     The found string is placed in entire/pieces
     p is placed right after the found string

   Return 0 if we did not find the string:
     p is left in its orignal spot
*/

static P *searchf(SRCH *srch, P *p)
{
	char *pattern = srch->pattern;
	P *start = pdup(p);
	P *end = pdup(p);
	int x;

	for (x = 0; x != sLEN(pattern) && pattern[x] != '\\'; ++x)
		if (srch->ignore)
			pattern[x] = toupper(pattern[x]);
	while (srch->ignore ? pifind(start, pattern, x) : pfind(start, pattern, x)) {
		pset(end, start);
		pfwrd(end, (long) x);
		if (pmatch(srch->pieces, pattern + x, sLEN(pattern) - x, end, 0, srch->ignore)) {
			srch->entire = vstrunc(srch->entire, (int) (end->byte - start->byte));
			brmem(start, srch->entire, (int) (end->byte - start->byte));
			pset(p, end);
			prm(start);
			prm(end);
			return p;
		}
		if (pgetc(start) == MAXINT)
			break;
	}
	prm(start);
	prm(end);
	return 0;
}

/* Search backwards.
   bw, pattern and ignore must be set

   The first possible string we can find is the one beginning one position
   to the left of p.

   Returns 1 if we found a string:
     The found string is placed in entire
     p is placed at the beginning of the string

   Return 0 if we did not find the string:
     p is left in its orignal spot
*/

static P *searchb(SRCH *srch, P *p)
{
	char *pattern = srch->pattern;
	P *start = pdup(p);
	P *end = pdup(p);
	int x;

	for (x = 0; x != sLEN(pattern) && pattern[x] != '\\'; ++x)
		if (srch->ignore)
			pattern[x] = toupper(pattern[x]);
	while (pbkwd(start, 1L)
	       && (srch->ignore ? prifind(start, pattern, x) : prfind(start, pattern, x))) {
		pset(end, start);
		pfwrd(end, (long) x);
		if (pmatch(srch->pieces, pattern + x, sLEN(pattern) - x, end, 0, srch->ignore)) {
			srch->entire = vstrunc(srch->entire, (int) (end->byte - start->byte));
			brmem(start, srch->entire, (int) (end->byte - start->byte));
			pset(p, start);
			prm(start);
			prm(end);
			return p;
		}
	}
	prm(start);
	prm(end);
	return 0;
}

/* Make a search stucture */

static SRCH *setmark(SRCH *srch)
{
	if (markv(1))
		srch->valid = 1;

	srch->markb = markb;
	if (srch->markb)
		srch->markb->owner = &srch->markb;
	markb = 0;

	srch->markk = markk;
	if (srch->markk)
		srch->markk->owner = &srch->markk;
	markk = 0;

	return srch;
}

SRCH *mksrch(char *pattern, char *replacement, int ignore, int backwards, int repeat, int replace, int rest)
{
	SRCH *srch = (SRCH *) malloc(sizeof(SRCH));
	int x;

	srch->pattern = pattern;
	srch->replacement = replacement;
	srch->ignore = ignore;
	srch->backwards = backwards;
	srch->repeat = repeat;
	srch->replace = replace;
	srch->rest = rest;
	srch->entire = 0;
	srch->flg = 0;
	srch->addr = -1;
	srch->markb = 0;
	srch->markk = 0;
	srch->valid = 0;
	srch->restrict = 0;
	izque(SRCHREC, link, &srch->recs);
	for (x = 0; x != 26; ++x)
		srch->pieces[x] = 0;
	return srch;
}

/* Eliminate a search structure */

void rmsrch(SRCH *srch)
{
	int x;

	prm(markb);
	prm(markk);
	if (srch->markb) {
		markb = srch->markb;
		markb->owner = &markb;
		markb->xcol = piscol(markb);
	}
	if (srch->markk) {
		markk = srch->markk;
		markk->owner = &markk;
		markk->xcol = piscol(markk);
	}
	for (x = 0; x != 26; ++x)
		vsrm(srch->pieces[x]);
	frchn(&fsr, &srch->recs);
	vsrm(srch->pattern);
	vsrm(srch->replacement);
	vsrm(srch->entire);
	free(srch);
	updall();
}

/* Insert a replacement string
 * p is advanced past the inserted text
 */

static P *insert(SRCH *srch, P *p, char *s, int len)
{
	int x;

	while (len) {
		for (x = 0; x != len && s[x] != '\\'; ++x) ;
		if (x) {
			binsm(p, s, x);
			pfwrd(p, (long) x);
			len -= x;
			s += x;
		} else if (len >= 2) {
			if (s[1] == '\\')
				binsc(p, '\\'), pgetc(p);
			else if (s[1] == 'n')
				binsc(p, '\n'), pgetc(p);
			else if (((s[1] >= 'a' && s[1] <= 'z') || (s[1] >= 'A' && s[1] <= 'Z'))
				 && srch->pieces[(s[1] & 0x1f) - 1]) {
				binsm(p, sv(srch->pieces[(s[1] & 0x1f) - 1]));
				pfwrd(p, (long) sLEN(srch->pieces[(s[1] & 0x1f) - 1]));
			} else if (s[1] >= '0' && s[1] <= '9' && srch->pieces[s[1] - '0']) {
				binsm(p, sv(srch->pieces[s[1] - '0']));
				pfwrd(p, (long) sLEN(srch->pieces[s[1] - '0']));
			} else if (s[1] == '&' && srch->entire) {
				binsm(p, sv(srch->entire));
				pfwrd(p, (long) sLEN(srch->entire));
			}
			s += 2;
			len -= 2;
		} else
			len = 0;
	}
	return p;
}

/* Search system user interface */

/* Query for search string, search options, possible replacement string,
 * and execute first search */

char srchstr[] = "Search";	/* Context sensitive help identifier */

static int pfabort(BW *bw, SRCH *srch)
{
	if (srch)
		rmsrch(srch);
	return -1;
}

static int pfsave(BW *bw, SRCH *srch)
{
	if (srch) {
		if (globalsrch)
			rmsrch(globalsrch);
		globalsrch = srch;
		srch->rest = 0;
		srch->repeat = -1;
		srch->flg = 0;

		prm(markb);
		prm(markk);
		if (srch->markb) {
			markb = srch->markb;
			markb->owner = &markb;
			markb->xcol = piscol(markb);
		}
		if (srch->markk) {
			markk = srch->markk;
			markk->owner = &markk;
			markk->xcol = piscol(markk);
		}
		srch->markb = 0;
		srch->markk = 0;

		updall();
	}
	return -1;
}

static int set_replace(BW *bw, char *s, SRCH *srch, int *notify)
{
	srch->replacement = s;
	return dopfnext(bw, srch, notify);
}

static int set_options(BW *bw, char *s, SRCH *srch, int *notify)
{
	int x;

	for (x = 0; s[x]; ++x) {
		switch (s[x]) {
		case 'r':
		case 'R':
			srch->replace = 1;
			break;
		case 'b':
		case 'B':
			srch->backwards = 1;
			break;
		case 'i':
		case 'I':
			srch->ignore = 1;
			break;
		case 'k':
		case 'K':
			srch->restrict = 1;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (srch->repeat == -1)
				srch->repeat = 0;
			srch->repeat = srch->repeat * 10 + s[x] - '0';
			break;
		}
	}
	vsrm(s);
	if (srch->replace) {
		if (wmkpw(bw->parent, "Replace with (^C to abort): ", &replhist, set_replace, srchstr, pfabort, utypebw, srch, notify))
			return 0;
		else
			return -1;
	} else
		return dopfnext(bw, srch, notify);
}

static int set_pattern(BW *bw, char *s, SRCH *srch, int *notify)
{
	BW *pbw;

	vsrm(srch->pattern);
	srch->pattern = s;
	if ((pbw = wmkpw(bw->parent, "(I)gnore (R)eplace (B)ackwards Bloc(K) NNN (^C to abort): ", NULL, set_options, srchstr, pfabort, utypebw, srch, notify)) != NULL) {
		char buf[10];

		if (srch->ignore)
			binsc(pbw->cursor, 'i');
		if (srch->replace)
			binsc(pbw->cursor, 'r');
		if (srch->backwards)
			binsc(pbw->cursor, 'b');
		if (srch->repeat >= 0)
			snprintf(buf, JOE_MSGBUFSIZE, "%d", srch->repeat), binss(pbw->cursor, buf);
		pset(pbw->cursor, pbw->b->eof);
		pbw->cursor->xcol = piscol(pbw->cursor);
		srch->ignore = 0;
		srch->replace = 0;
		srch->backwards = 0;
		srch->repeat = -1;
		return 0;
	} else {
		rmsrch(srch);
		return -1;
	}
}

static int dofirst(BW *bw, int back, int repl)
{
	SRCH *srch;

	if (smode && globalsrch) {
		globalsrch->backwards = back;
		globalsrch->replace = repl;
		return pfnext(bw);
	}
	if (bw->parent->huh == srchstr) {
		long byte;

		p_goto_eol(bw->cursor);
		byte = bw->cursor->byte;
		p_goto_bol(bw->cursor);
		if (byte == bw->cursor->byte)
			prgetc(bw->cursor);
		return urtn(bw, MAXINT);
	}
	srch = setmark(mksrch(NULL, NULL, 0, back, -1, repl, 0));
	srch->addr = bw->cursor->byte;
	if (wmkpw(bw->parent, "Find (^C to abort): ", &findhist, set_pattern, srchstr, pfabort, utypebw, srch, NULL))
		return 0;
	else {
		rmsrch(srch);
		return -1;
	}
}

int pffirst(BW *bw)
{
	return dofirst(bw, 0, 0);
}

int prfirst(BW *bw)
{
	return dofirst(bw, 1, 0);
}

int pqrepl(BW *bw)
{
	return dofirst(bw, 0, 1);
}

/* Execute next search */

static int doreplace(BW *bw, SRCH *srch)
{
	P *q;

	if (bw->b->rdonly) {
		msgnw(bw->parent, "Read only");
		return -1;
	}
	if (markk)
		markk->end = 1;
	if (srch->markk)
		srch->markk->end = 1;
	q = pdup(bw->cursor);
	if (srch->backwards) {
		q = pfwrd(q, (long) sLEN(srch->entire));
		bdel(bw->cursor, q);
		prm(q);
	} else {
		q = pbkwd(q, (long) sLEN(srch->entire));
		bdel(q, bw->cursor);
		prm(q);
	}
	insert(srch, bw->cursor, sv(srch->replacement));
	srch->addr = bw->cursor->byte;
	if (markk)
		markk->end = 0;
	if (srch->markk)
		srch->markk->end = 0;
	return 0;
}

static void visit(SRCH *srch, BW *bw, int yn)
{
	SRCHREC *r = (SRCHREC *) alitem(&fsr, sizeof(SRCHREC));

	r->addr = bw->cursor->byte;
	r->yn = yn;
	enqueb(SRCHREC, link, &srch->recs, r);
}

static void goback(SRCH *srch, BW *bw)
{
	SRCHREC *r = srch->recs.link.prev;

	if (r != &srch->recs) {
		if (r->yn)
			uundo(bw);
		if (bw->cursor->byte != r->addr)
			pgoto(bw->cursor, r->addr);
		demote(SRCHREC, link, &fsr, r);
	}
}

static int dopfrepl(BW *bw, int c, SRCH *srch, int *notify)
{
	srch->addr = bw->cursor->byte;
	if (c == 'N' || c == 'n')
		return dopfnext(bw, srch, notify);
	else if (c == 'Y' || c == 'y' || c == ' ') {
		srch->recs.link.prev->yn = 1;
		if (doreplace(bw, srch)) {
			pfsave(bw, srch);
			return -1;
		} else
			return dopfnext(bw, srch, notify);
	} else if (c == 'R' || c == 'r') {
		if (doreplace(bw, srch))
			return -1;
		srch->rest = 1;
		return dopfnext(bw, srch, notify);
	} else if (c == 8 || c == 127 || c == 'b' || c == 'B') {
		goback(srch, bw);
		goback(srch, bw);
		return dopfnext(bw, srch, notify);
	} else if (c != MAXINT) {
		if (notify)
			*notify = 1;
		pfsave(bw, srch);
		nungetc(c);
		return 0;
	}
	if (mkqwnsr(bw->parent, sc("Replace (Y)es (N)o (R)est (B)ackup (^C to abort)?"), dopfrepl, pfsave, srch, notify))
		return 0;
	else
		return pfsave(bw, srch);
}

/* Test if found text is within region
 * return 0 if it is,
 * -1 if we should keep searching
 * 1 if we're done
 */

static int restrict(BW *bw, SRCH *srch)
{
	if (!srch->valid || !srch->restrict)
		return 0;
	bw->cursor->xcol = piscol(bw->cursor);
	if (srch->backwards)
		if (!square) {
			if (bw->cursor->byte < srch->markb->byte)
				return 1;
			else if (bw->cursor->byte + sLEN(srch->entire) > srch->markk->byte)
				return -1;
		} else {
			if (bw->cursor->line < srch->markb->line)
				return 1;
			else if (bw->cursor->line > srch->markk->line)
				return -1;
			else if (piscol(bw->cursor) + sLEN(srch->entire) > srch->markk->xcol || piscol(bw->cursor) < srch->markb->xcol)
				return -1;
	} else if (!square) {
		if (bw->cursor->byte > srch->markk->byte)
			return 1;
		else if (bw->cursor->byte - sLEN(srch->entire) < srch->markb->byte)
			return -1;
	} else {
		if (bw->cursor->line > srch->markk->line)
			return 1;
		if (bw->cursor->line < srch->markb->line)
			return -1;
		if (piscol(bw->cursor) > srch->markk->xcol || piscol(bw->cursor) - sLEN(srch->entire) < srch->markb->xcol)
			return -1;
	}
	return 0;
}

/* Possible results:
 *   0) Search or search & replace is finished.
 *   1) Search string was not found.
 *   2) Search string was found.
 */

static int fnext(BW *bw, SRCH *srch)
{
	P *sta;

      next:
	if (srch->repeat != -1) {
		if (!srch->repeat)
			return 0;
		else
			--srch->repeat;
	}
      again:if (srch->backwards)
		sta = searchb(srch, bw->cursor);
	else
		sta = searchf(srch, bw->cursor);
	if (!sta) {
		srch->repeat = -1;
		return 1;
	} else if (srch->rest || (srch->repeat != -1 && srch->replace)) {
		if (srch->valid)
			switch (restrict(bw, srch)) {
			case -1:
				goto again;
			case 1:
				if (srch->addr >= 0)
					pgoto(bw->cursor, srch->addr);
				return !srch->rest;
			}
		if (doreplace(bw, srch))
			return 0;
		goto next;
	} else if (srch->repeat != -1) {
		if (srch->valid)
			switch (restrict(bw, srch)) {
			case -1:
				goto again;
			case 1:
				if (srch->addr >= 0)
					pgoto(bw->cursor, srch->addr);
				return 1;
			}
		srch->addr = bw->cursor->byte;
		goto next;
	} else
		return 2;
}

int dopfnext(BW *bw, SRCH *srch, int *notify)
{
	int orgmid = mid;	/* Original mid status */
	int ret = 0;

	mid = 1;		/* Screen recenters mode during search */
	if (csmode)
		smode = 2;	/* We have started a search mode */
	if (srch->replace)
		visit(srch, bw, 0);
again:	switch (fnext(bw, srch)) {
	case 0:
		break;
	case 1:
bye:		if (!srch->flg && !srch->rest) {
			if (srch->valid && srch->restrict)
				msgnw(bw->parent, "Not found (search restricted to marked block)");
			else
				msgnw(bw->parent, "Not found");
			ret = -1;
		}
		break;
	case 2:
		if (srch->valid)
			switch (restrict(bw, srch)) {
			case -1:
				goto again;
			case 1:
				if (srch->addr >= 0)
					pgoto(bw->cursor, srch->addr);
				goto bye;
			}
		srch->addr = bw->cursor->byte;
		if (srch->replace) {
			if (square)
				bw->cursor->xcol = piscol(bw->cursor);
			if (srch->backwards) {
				pdupown(bw->cursor, &markb);
				markb->xcol = piscol(markb);
				pdupown(markb, &markk);
				pfwrd(markk, (long) sLEN(srch->entire));
				markk->xcol = piscol(markk);
			} else {
				pdupown(bw->cursor, &markk);
				markk->xcol = piscol(markk);
				pdupown(bw->cursor, &markb);
				pbkwd(markb, (long) sLEN(srch->entire));
				markb->xcol = piscol(markb);
			}
			srch->flg = 1;
			if (dopfrepl(bw, MAXINT, srch, notify))
				ret = -1;
			notify = 0;
			srch = 0;
		}
		break;
	}
	bw->cursor->xcol = piscol(bw->cursor);
	dofollows();
	mid = orgmid;
	if (notify)
		*notify = 1;
	if (srch)
		pfsave(bw, srch);
	else
		updall();
	return ret;
}

int pfnext(BW *bw)
{
	if (!globalsrch)	/* Query for search string if there isn't any */
		return pffirst(bw);
	else {
		SRCH *srch = globalsrch;

		globalsrch = 0;
		srch->addr = bw->cursor->byte;
		return dopfnext(bw, setmark(srch), NULL);
	}
}
