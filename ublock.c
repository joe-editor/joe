/*
 * 	Highlighted block functions
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <unistd.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include "b.h"
#include "pw.h"
#include "queue.h"
#include "scrn.h"
#include "tty.h"
#include "ublock.h"
#include "uedit.h"
#include "utils.h"
#include "vs.h"
#include "utf8.h"
#include "charmap.h"
#include "w.h"

/* Global options */

int square = 0;			/* Set for rectangle mode */
int lightoff = 0;		/* Set if highlighting should turn off

				   after block operations */
extern int marking;

/* Global variables */

P *markb = NULL;		/* Beginning and end of block */
P *markk = NULL;

/* Push markb & markk */

typedef struct marksav MARKSAV;
struct marksav {
	LINK(MARKSAV) link;
	P *markb, *markk;
} markstack = { { &markstack, &markstack} };
MARKSAV markfree = { {&markfree, &markfree} };
int nstack = 0;

int upsh(BW *bw)
{
	MARKSAV *m = alitem(&markfree, sizeof(MARKSAV));

	m->markb = 0;
	m->markk = 0;
	if (markk)
		pdupown(markk, &m->markk);
	if (markb)
		pdupown(markb, &m->markb);
	enqueb(MARKSAV, link, &markstack, m);
	++nstack;
	return 0;
}

int upop(BW *bw)
{
	MARKSAV *m = markstack.link.prev;

	if (m != &markstack) {
		--nstack;
		prm(markk);
		prm(markb);
		markk = m->markk;
		if (markk)
			markk->owner = &markk;
		markb = m->markb;
		if (markb)
			markb->owner = &markb;
		demote(MARKSAV, link, &markfree, m);
		if (lightoff)
			unmark(bw);
		updall();
		return 0;
	} else
		return -1;
}

/* Return true if markb/markk are valid */

int markv(int r)
{
	if (markb && markk && markb->b == markk->b && markk->byte > markb->byte && (!square || markk->xcol > markb->xcol))
		return 1;
	else
		return 0;
}

/* Rectangle-mode subroutines */

/* B *pextrect(P *org,long height,long left,long right);
 * Copy a rectangle into a new buffer
 *
 * org points to top-left corner of rectangle.
 * height is number of lines in rectangle.
 * right is rightmost column of rectangle + 1
 */

B *pextrect(P *org, long int height, long int right)
{
	P *p = pdup(org);	/* Left part of text to extract */
	P *q = pdup(p);		/* After right part of text to extract */
	B *tmp = bmk(NULL);	/* Buffer to extract to */
	P *z = pdup(tmp->eof);	/* Buffer pointer */

	while (height--) {
		pcol(p, org->xcol);
		pset(q, p);
		pcolwse(q, right);
		p_goto_eof(z);
		binsb(z, bcpy(p, q));
		p_goto_eof(z);
		binsc(z, '\n');
		pnextl(p);
	}
	prm(p);
	prm(q);
	prm(z);
	return tmp;
}

/* void pdelrect(P *org,long height,long right);
 * Delete a rectangle.
 */

void pdelrect(P *org, long int height, long int right)
{
	P *p = pdup(org);
	P *q = pdup(p);

	while (height--) {
		pcol(p, org->xcol);
		pset(q, p);
		pcol(q, right);
		bdel(p, q);
		pnextl(p);
	}
	prm(p);
	prm(q);
}

/* void pclrrect(P *org,long height,long right,int usetabs);
 * Blank-out a rectangle.
 */

void pclrrect(P *org, long int height, long int right, int usetabs)
{
	P *p = pdup(org);
	P *q = pdup(p);

	while (height--) {
		long pos;

		pcol(p, org->xcol);
		pset(q, p);
		pcoli(q, right);
		pos = q->col;
		bdel(p, q);
		pfill(p, pos, usetabs);
		pnextl(p);
	}
	prm(p);
	prm(q);
}

/* int ptabrect(P *org,long height,long right)
 * Check if there are any TABs in a rectangle
 */

int ptabrect(P *org, long int height, long int right)
{
	P *p = pdup(org);

	while (height--) {
		int c;

		pcol(p, org->xcol);
		while ((c = pgetc(p)) != NO_MORE_DATA && c != '\n') {
			if (c == '\t') {
				prm(p);
				return '\t';
			} else if (piscol(p) > right)
				break;
		}
		if (c != '\n')
			pnextl(p);
	}
	prm(p);
	return ' ';
}

/* Insert rectangle */

void pinsrect(P *cur, B *tmp, long int width, int usetabs)
{
	P *p = pdup(cur);	/* We insert at & move this pointer */
	P *q = pdup(tmp->bof);	/* These are for scanning through 'tmp' */
	P *r = pdup(q);

	if (width)
		while (pset(r, q), p_goto_eol(q), (q->line != tmp->eof->line || piscol(q))) {
			pcol(p, cur->xcol);
			if (piscol(p) < cur->xcol)
				pfill(p, cur->xcol, usetabs);
			binsb(p, bcpy(r, q));
			pfwrd(p, q->byte - r->byte);
			if (piscol(p) < cur->xcol + width)
				pfill(p, cur->xcol + width, usetabs);
			if (piseol(p))
				pbackws(p);
			if (!pnextl(p)) {
				binsc(p, '\n');
				pgetc(p);
			}
			if (pgetc(q) == NO_MORE_DATA)
				break;
		}
	prm(p);
	prm(q);
	prm(r);
}

/* Block functions */

/* Set beginning */

int umarkb(BW *bw)
{
	pdupown(bw->cursor, &markb);
	markb->xcol = bw->cursor->xcol;
	updall();
	return 0;
}

int udrop(BW *bw)
{
	prm(markk);
	if (marking && markb)
		prm(markb);
	else
		umarkb(bw);
	return 0;
}

int ubegin_marking(BW *bw)
{
	if (marking)
		/* We're marking now... don't stop */
		return 0;
	else if (markv(0) && bw->cursor->b==markb->b)
		/* Try to extend current block */
		if (bw->cursor->byte==markb->byte) {
			pset(markb,markk);
			prm(markk); markk=0;
			marking = 1;
			return 0;
		} else if(bw->cursor->byte==markk->byte) {
			prm(markk); markk=0;
			marking = 1;
			return 0;
		}
	/* Start marking - no message */
	prm(markb); markb=0;
	prm(markk); markk=0;
	updall();
	marking = 1;
	return umarkb(bw);
}

int utoggle_marking(BW *bw)
{
	if (markv(0) && bw->cursor->b==markb->b && bw->cursor->byte>=markb->byte && bw->cursor->byte<=markk->byte) {
		/* Just clear selection */
		prm(markb); markb=0;
		prm(markk); markk=0;
		updall();
		marking = 0;
		msgnw(bw->parent, US "Selection cleared.");
		return 0;
	} else if (markk) {
		/* Clear selection and start new one */
		prm(markb); markb=0;
		prm(markk); markk=0;
		updall();
		marking = 1;
		msgnw(bw->parent, US "Selection started.");
		return umarkb(bw);
	} else if (markb && markb->b==bw->cursor->b) {
		marking = 0;
		if (bw->cursor->byte<markb->byte) {
			pdupown(markb, &markk);
			prm(markb); markb=0;
			pdupown(bw->cursor, &markb);
			markb->xcol = bw->cursor->xcol;
		} else {
			pdupown(bw->cursor, &markk);
			markk->xcol = bw->cursor->xcol;
		}
		updall(); /* Because other windows could be changed */
		return 0;
	} else {
		marking = 1;
		msgnw(bw->parent, US "Selection started.");
		return umarkb(bw);
	}
}

int uselect(BW *bw)
{
	if (!markb)
		umarkb(bw);
	return 0;
}

/* Set end */

int umarkk(BW *bw)
{
	pdupown(bw->cursor, &markk);
	markk->xcol = bw->cursor->xcol;
	updall();
	return 0;
}

/* Unset marks */

int unmark(BW *bw)
{
	prm(markb);
	prm(markk);
	updall();
	return 0;
}

/* Mark line */

int umarkl(BW *bw)
{
	p_goto_bol(bw->cursor);
	umarkb(bw);
	pnextl(bw->cursor);
	umarkk(bw);
	utomarkb(bw);
	pcol(bw->cursor, bw->cursor->xcol);
	return 0;
}

int utomarkb(BW *bw)
{
	if (markb && markb->b == bw->b) {
		pset(bw->cursor, markb);
		return 0;
	} else
		return -1;
}

int utomarkk(BW *bw)
{
	if (markk && markk->b == bw->b) {
		pset(bw->cursor, markk);
		return 0;
	} else
		return -1;
}

int uswap(BW *bw)
{
	if (markb && markb->b == bw->b) {
		P *q = pdup(markb);

		umarkb(bw);
		pset(bw->cursor, q);
		prm(q);
		return 0;
	} else
		return -1;
}

int utomarkbk(BW *bw)
{
	if (markb && markb->b == bw->b && bw->cursor->byte != markb->byte) {
		pset(bw->cursor, markb);
		return 0;
	} else if (markk && markk->b == bw->b && bw->cursor->byte != markk->byte) {
		pset(bw->cursor, markk);
		return 0;
	} else
		return -1;
}

/* Delete block */

extern int udelln(BW *bw);

int ublkdel(BW *bw)
{
	if (markv(1)) {
		if (square)
			if (bw->o.overtype) {
				long ocol = markk->xcol;

				pclrrect(markb, markk->line - markb->line + 1, markk->xcol, ptabrect(markb, markk->line - markb->line + 1, markk->xcol));
				pcol(markk, ocol);
				markk->xcol = ocol;
			} else
				pdelrect(markb, markk->line - markb->line + 1, markk->xcol);
		else
			bdel(markb, markk);
		if (lightoff)
			unmark(bw);
	} else {
		msgnw(bw->parent, US "No block");
		return -1;
	}
	return 0;
}

/* Special delete block function for PICO */

int upicokill(BW *bw)
{
	upsh(bw);
	umarkk(bw);
	if (markv(1)) {
		if (square)
			if (bw->o.overtype) {
				long ocol = markk->xcol;

				pclrrect(markb, markk->line - markb->line + 1, markk->xcol, ptabrect(markb, markk->line - markb->line + 1, markk->xcol));
				pcol(markk, ocol);
				markk->xcol = ocol;
			} else
				pdelrect(markb, markk->line - markb->line + 1, markk->xcol);
		else
			bdel(markb, markk);
		if (lightoff)
			unmark(bw);
	} else
		udelln(bw);
	return 0;
}

/* Move highlighted block */

int ublkmove(BW *bw)
{
	if (markv(1)) {
		if (markb->b->rdonly) {
			msgnw(bw->parent, US "Read only");
			return -1;
		}
		if (square) {
			long height = markk->line - markb->line + 1;
			long width = markk->xcol - markb->xcol;
			int usetabs = ptabrect(markb, height, markk->xcol);
			long ocol = piscol(bw->cursor);
			B *tmp = pextrect(markb, height, markk->xcol);
			int update_xcol = (bw->cursor->xcol >= markk->xcol && bw->cursor->line >= markb->line && bw->cursor->line <= markk->line);

			ublkdel(bw);
			/* now we can't use markb and markk until we set them again */
			/* ublkdel() frees them */
			if (bw->o.overtype) {
				/* If cursor was in block, blkdel moves it to left edge of block, so fix it
				 * back to its original place here */
				pcol(bw->cursor, ocol);
				pfill(bw->cursor, ocol, ' ');
				pdelrect(bw->cursor, height, piscol(bw->cursor) + width);
			} else if (update_xcol)
				/* If cursor was to right of block, xcol was not properly updated */
				bw->cursor->xcol -= width;
			pinsrect(bw->cursor, tmp, width, usetabs);
			brm(tmp);
			if (lightoff)
				unmark(bw);
			else {
				umarkb(bw);
				umarkk(bw);
				pline(markk, markk->line + height - 1);
				pcol(markk, markb->xcol + width);
				markk->xcol = markb->xcol + width;
			}
			return 0;
		} else if (bw->cursor->b != markk->b || bw->cursor->byte > markk->byte || bw->cursor->byte < markb->byte) {
			long size = markk->byte - markb->byte;

			binsb(bw->cursor, bcpy(markb, markk));
			bdel(markb, markk);
			if (lightoff)
				unmark(bw);
			else {
				umarkb(bw);
				umarkk(bw);
				pfwrd(markk, size);
			}
			updall();
			return 0;
		}
	}
	msgnw(bw->parent, US "No block");
	return -1;
}

/* Duplicate highlighted block */

int ublkcpy(BW *bw)
{
	if (markv(1)) {
		if (square) {
			long height = markk->line - markb->line + 1;
			long width = markk->xcol - markb->xcol;
			int usetabs = ptabrect(markb, height, markk->xcol);
			B *tmp = pextrect(markb, height, markk->xcol);

			if (bw->o.overtype)
				pdelrect(bw->cursor, height, piscol(bw->cursor) + width);
			pinsrect(bw->cursor, tmp, width, usetabs);
			brm(tmp);
			if (lightoff)
				unmark(bw);
			else {
				umarkb(bw);
				umarkk(bw);
				pline(markk, markk->line + height - 1);
				pcol(markk, markb->xcol + width);
				markk->xcol = markb->xcol + width;
			}
			return 0;
		} else {
			long size = markk->byte - markb->byte;

			binsb(bw->cursor, bcpy(markb, markk));
			if (lightoff)
				unmark(bw);
			else {
				umarkb(bw);
				umarkk(bw);
				pfwrd(markk, size);
			}
			updall();
			return 0;
		}
	} else {
		msgnw(bw->parent, US "No block");
		return -1;
	}
}

/* Write highlighted block to a file */
/* This is called by ublksave in ufile.c */

int dowrite(BW *bw, unsigned char *s, void *object, int *notify)
{
	if (notify)
		*notify = 1;
	if (markv(1)) {
		if (square) {
			int fl;
			int ret = 0;
			B *tmp = pextrect(markb,
					  markk->line - markb->line + 1,
					  markk->xcol);

			if ((fl = bsave(tmp->bof, s, tmp->eof->byte, 0)) != 0) {
				msgnw(bw->parent, msgs[-fl]);
				ret = -1;
			}
			brm(tmp);
			if (lightoff)
				unmark(bw);
			vsrm(s);
			return ret;
		} else {
			int fl;
			int ret = 0;

			if ((fl = bsave(markb, s, markk->byte - markb->byte, 0)) != 0) {
				msgnw(bw->parent, msgs[-fl]);
				ret = -1;
			}
			if (lightoff)
				unmark(bw);
			vsrm(s);
			return ret;
		}
	} else {
		vsrm(s);
		msgnw(bw->parent, US "No block");
		return -1;
	}
}

/* Set highlighted block on a program block */

void setindent(BW *bw)
{
	P *p, *q;
	long indent;

	if (pisblank(bw->cursor))
		return;

	p = pdup(bw->cursor);
	q = pdup(p);
	indent = pisindent(p);

	do {
		if (!pprevl(p))
			goto done;
		else
			p_goto_bol(p);
	} while (pisindent(p) >= indent || pisblank(p));
	pnextl(p);
	/* Maybe skip blank lines at beginning */
      done:
	p_goto_bol(p);
	p->xcol = piscol(p);
	if (markb)
		prm(markb);
	markb = p;
	p->owner = &markb;

	do {
		if (!pnextl(q))
			break;
	} while (pisindent(q) >= indent || pisblank(q));
	/* Maybe skip blank lines at end */
	if (markk)
		prm(markk);
	q->xcol = piscol(q);
	markk = q;
	q->owner = &markk;

	updall();
}

/* Purity check */
/* Verifies that at least n indentation characters (for non-blank lines) match c */
/* If n is 0 (for urindent), this fails if c is space but indentation begins with tab */

int purity_check(int c, int n)
{
	P *p = pdup(markb);
	while (p->byte < markk->byte) {
		int x;
		p_goto_bol(p);
		if (!n && c==' ' && brc(p)=='\t') {
			prm(p);
			return 0;
		} else if (!piseol(p))
			for (x=0; x!=n; ++x)
				if (pgetc(p)!=c) {
					prm(p);
					return 0;
				}
		pnextl(p);
	}
	prm(p);
	return 1;
}

/* Left indent check */
/* Verify that there is enough whitespace to do the left indent */

int lindent_check(int c, int n)
{
	P *p = pdup(markb);
	int indwid;
	if (c=='\t')
		indwid = n * p->b->o.tab;
	else
		indwid = n;
	while (p->byte < markk->byte) {
		int x;
		p_goto_bol(p);
		if (!piseol(p) && pisindent(p)<indwid) {
			prm(p);
			return 0;
		}
		pnextl(p);
	}
	prm(p);
	return 1;
}

/* Indent more */

int urindent(BW *bw)
{
	if (square) {
		if (markb && markk && markb->b == markk->b && markb->byte <= markk->byte && markb->xcol <= markk->xcol) {
			P *p = pdup(markb);

			do {
				pcol(p, markb->xcol);
				pfill(p, markb->xcol + bw->o.istep, bw->o.indentc);
			} while (pnextl(p) && p->line <= markk->line);
			prm(p);
		}
	} else {
		if (!markb || !markk || markb->b != markk->b || bw->cursor->byte < markb->byte || bw->cursor->byte > markk->byte || markb->byte == markk->byte) {
			setindent(bw);
		} else if ( 1 /* bw->o.purify */) {
			P *p = pdup(markb);
			P *q = pdup(markb);
			int indwid;

			if (bw->o.indentc=='\t')
				indwid = bw->o.tab * bw->o.istep;
			else
				indwid = bw->o.istep;

			while (p->byte < markk->byte) {
				p_goto_bol(p);
				if (!piseol(p)) {
					int col;
					pset(q, p);
					p_goto_indent(q, bw->o.indentc);
					col = piscol(q);
					bdel(p,q);
					pfill(p,col+indwid,bw->o.indentc);
				}
				pnextl(p);
			}
			prm(p);
			prm(q);
		} else if (purity_check(bw->o.indentc,0)) {
			P *p = pdup(markb);

			while (p->byte < markk->byte) {
				p_goto_bol(p);
				if (!piseol(p))
					while (piscol(p) < bw->o.istep) {
						binsc(p, bw->o.indentc);
						pgetc(p);
					}
				pnextl(p);
			}
			prm(p);
		} else {
			/* Purity failure */
			msgnw(bw->parent,US "Selected lines not properly indented");
			return 1;
		}
	}
	return 0;
}

/* Indent less */

int ulindent(BW *bw)
{
	if (square) {
		if (markb && markk && markb->b == markk->b && markb->byte <= markk->byte && markb->xcol <= markk->xcol) {
			P *p = pdup(markb);
			P *q = pdup(p);

			do {
				pcol(p, markb->xcol);
				while (piscol(p) < markb->xcol + bw->o.istep) {
					int c = pgetc(p);

					if (c != ' ' && c != '\t' && c != bw->o.indentc) {
						prm(p);
						prm(q);
						return -1;
					}
				}
			} while (pnextl(p) && p->line <= markk->line);
			pset(p, markb);
			do {
				pcol(p, markb->xcol);
				pset(q, p);
				pcol(q, markb->xcol + bw->o.istep);
				bdel(p, q);
			} while (pnextl(p) && p->line <= markk->line);
			prm(p);
			prm(q);
		}
	} else {
		if (!markb || !markk || markb->b != markk->b || bw->cursor->byte < markb->byte || bw->cursor->byte > markk->byte || markb->byte == markk->byte) {
			setindent(bw);
		} else if (1 /* bw->o.purify */ && lindent_check(bw->o.indentc,bw->o.istep)) {
			P *p = pdup(markb);
			P *q = pdup(markb);
			int indwid;

			if (bw->o.indentc=='\t')
				indwid = bw->o.tab * bw->o.istep;
			else
				indwid = bw->o.istep;

			while (p->byte < markk->byte) {
				p_goto_bol(p);
				if (!piseol(p)) {
					int col;
					pset(q, p);
					p_goto_indent(q, bw->o.indentc);
					col = piscol(q);
					bdel(p,q);
					pfill(p,col-indwid,bw->o.indentc);
				}
				pnextl(p);
			}
			prm(p);
			prm(q);
		} else if (purity_check(bw->o.indentc,bw->o.istep)) {
			P *p = pdup(markb);
			P *q = pdup(p);

			p_goto_bol(p);
			while (p->byte < markk->byte) {
				if (!piseol(p)) {
					pset(q, p);
					while (piscol(q) < bw->o.istep)
						pgetc(q);
					bdel(p, q);
				}
				pnextl(p);
			}
			prm(p);
			prm(q);
		} else {
			/* Purity failure */
			msgnw(bw->parent,US "Selected lines not properly indented");
			return 1;
		}
	}
	return 0;
}

/* Insert a file */

int doinsf(BW *bw, unsigned char *s, void *object, int *notify)
{
	if (notify)
		*notify = 1;
	if (square)
		if (markv(1)) {
			B *tmp;
			long width = markk->xcol - markb->xcol;
			long height;
			int usetabs = ptabrect(markb,
					       markk->line - markb->line + 1,
					       markk->xcol);

			tmp = bload(s);
			if (error) {
				msgnw(bw->parent, msgs[-error]);
				brm(tmp);
				return -1;
			}
			if (piscol(tmp->eof))
				height = tmp->eof->line + 1;
			else
				height = tmp->eof->line;
			if (bw->o.overtype) {
				pclrrect(markb, long_max(markk->line - markb->line + 1, height), markk->xcol, usetabs);
				pdelrect(markb, height, width + markb->xcol);
			}
			pinsrect(markb, tmp, width, usetabs);
			pdupown(markb, &markk);
			markk->xcol = markb->xcol;
			if (height) {
				pline(markk, markk->line + height - 1);
				pcol(markk, markb->xcol + width);
				markk->xcol = markb->xcol + width;
			}
			brm(tmp);
			updall();
			return 0;
		} else {
			msgnw(bw->parent, US "No block");
			return -1;
	} else {
		int ret = 0;
		B *tmp = bload(s);

		if (error) {
			msgnw(bw->parent, msgs[-error]), brm(tmp);
			ret = -1;
		} else
			binsb(bw->cursor, tmp);
		vsrm(s);
		bw->cursor->xcol = piscol(bw->cursor);
		return ret;
	}
}


/* Filter highlighted block through a UNIX command */

static int filtflg = 0;

static int dofilt(BW *bw, unsigned char *s, void *object, int *notify)
{
	int fr[2];
	int fw[2];

	if (notify)
		*notify = 1;
	if (markb && markk && !square && markb->b == bw->b && markk->b == bw->b && markb->byte == markk->byte)
		goto ok;
	if (!markv(1)) {
		msgnw(bw->parent, US "No block");
		return -1;
	}
      ok:

	pipe(fr);
	pipe(fw);
	npartial(bw->parent->t->t);
	ttclsn();
	if (!fork()) {
#ifdef HAVE_PUTENV
		unsigned char		*fname, *name;
		unsigned	len;
#endif
		signrm();
		close(0);
		close(1);
		close(2);
		dup(fw[0]);
		dup(fr[1]);
		dup(fr[1]);
		close(fw[0]);
		close(fr[1]);
		close(fw[1]);
		close(fr[0]);
#ifdef HAVE_PUTENV
		fname = vsncpy(NULL, 0, sc("JOE_FILENAME="));
		name = bw->b->name ? bw->b->name : (unsigned char *)"Unnamed";
		if((len = slen(name)) >= 512)	/* limit filename length */
			len = 512;
		fname = vsncpy(sv(fname), name, len);
		putenv((char *)fname);
		vsrm(fname);
#endif
		execl("/bin/sh", "/bin/sh", "-c", s, NULL);
		_exit(0);
	}
	close(fr[1]);
	close(fw[0]);
	if (fork()) {
		long szz;

		close(fw[1]);
		if (square) {
			B *tmp;
			long width = markk->xcol - markb->xcol;
			long height;
			int usetabs = ptabrect(markb,
					       markk->line - markb->line + 1,
					       markk->xcol);

			tmp = bread(fr[0], MAXLONG);
			if (piscol(tmp->eof))
				height = tmp->eof->line + 1;
			else
				height = tmp->eof->line;
			if (bw->o.overtype) {
				pclrrect(markb, markk->line - markb->line + 1, markk->xcol, usetabs);
				pdelrect(markb, long_max(height, markk->line - markb->line + 1), width + markb->xcol);
			} else
				pdelrect(markb, markk->line - markb->line + 1, markk->xcol);
			pinsrect(markb, tmp, width, usetabs);
			pdupown(markb, &markk);
			markk->xcol = markb->xcol;
			if (height) {
				pline(markk, markk->line + height - 1);
				pcol(markk, markb->xcol + width);
				markk->xcol = markb->xcol + width;
			}
			if (lightoff)
				unmark(bw);
			brm(tmp);
			updall();
		} else {
			bdel(markb, markk);
			szz = markk->b->eof->byte;
			binsb(markk, bread(fr[0], MAXLONG));
			pfwrd(markk, markk->b->eof->byte - szz);
			if (lightoff)
				unmark(bw);
		}
		close(fr[0]);
		wait(NULL);
		wait(NULL);
	} else {
		if (square) {
			B *tmp = pextrect(markb,
					  markk->line - markb->line + 1,
					  markk->xcol);

			bsavefd(tmp->bof, fw[1], tmp->eof->byte);
		} else
			bsavefd(markb, fw[1], markk->byte - markb->byte);
		close(fw[1]);
		_exit(0);
	}
	vsrm(s);
	ttopnn();
	if (filtflg)
		unmark(bw);
	bw->cursor->xcol = piscol(bw->cursor);
	return 0;
}

static B *filthist = NULL;

static void markall(BW *bw)
{
	pdupown(bw->cursor->b->bof, &markb);
	markb->xcol = 0;
	pdupown(bw->cursor->b->eof, &markk);
	markk->xcol = piscol(markk);
	updall();
}

static int checkmark(BW *bw)
{
	if (!markv(1))
		if (square)
			return 2;
		else {
			markall(bw);
			filtflg = 1;
			return 1;
	} else {
		filtflg = 0;
		return 0;
	}
}

int ufilt(BW *bw)
{
#ifdef __MSDOS__
	msgnw(bw->parent, "Sorry, no sub-processes in DOS (yet)");
	return -1;
#else
	switch (checkmark(bw)) {
	case 0:
		if (wmkpw(bw->parent, US "Command to filter block through (^C to abort): ", &filthist, dofilt, NULL, NULL, utypebw, NULL, NULL, locale_map))
			return 0;
		else
			return -1;
	case 1:
		if (wmkpw(bw->parent, US "Command to filter file through (^C to abort): ", &filthist, dofilt, NULL, NULL, utypebw, NULL, NULL, locale_map))
			return 0;
		else
			return -1;
	case 2:
	default:
		msgnw(bw->parent, US "No block");
		return -1;
	}
#endif
}

/* Force region to lower case */

int ulower(BW *bw)
{
	if (markv(0)) {
	        P *p;
	        int c;
	        int flg;
		B *b = bcpy(markb,markk);
		if (bw->cursor->b==markk->b && bw->cursor->byte==markk->byte)
			flg = 1;
		else
			flg = 0;
		bdel(markb,markk);
		b->o.charmap = markb->b->o.charmap;
		p=pdup(b->bof);
		while ((c=pgetc(p))!=NO_MORE_DATA) {
			c = joe_tolower(b->o.charmap,c);
			binsc(markk,c);
			pgetc(markk);
			if (flg)
				pgetc(bw->cursor);
		}
		prm(p);
		brm(b);
		bw->cursor->xcol = piscol(bw->cursor);
		return 0;
	} else
		return -1;
}

/* Force region to upper case */

int uupper(BW *bw)
{
	if (markv(0)) {
	        P *p;
	        int c;
	        int flg;
		B *b = bcpy(markb,markk);
		if (bw->cursor->b==markk->b && bw->cursor->byte==markk->byte)
			flg = 1;
		else
			flg = 0;
		bdel(markb,markk);
		b->o.charmap = markb->b->o.charmap;
		p=pdup(b->bof);
		while ((c=pgetc(p))!=NO_MORE_DATA) {
			c = joe_toupper(b->o.charmap,c);
			binsc(markk,c);
			pgetc(markk);
			if (flg)
				pgetc(bw->cursor);
		}
		prm(p);
		brm(b);
		bw->cursor->xcol = piscol(bw->cursor);
		return 0;
	} else
		return -1;
}
