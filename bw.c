/*
 *	Edit buffer window generation
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <string.h>
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif


#include "b.h"
#include "blocks.h"
#include "kbd.h"
#include "rc.h"
#include "scrn.h"
#include "ublock.h"
#include "utils.h"
#include "syntax.h"
#include "utf8.h"
#include "w.h"

/* Display modes */
int dspasis = 0;
int marking = 0;
extern int square;
extern int staen;

static P *getto(P *p, P *cur, P *top, long int line)
{

	if (p == NULL) {
		P *best = cur;
		long dist = MAXLONG;
		long d;

		d = (line >= cur->line ? line - cur->line : cur->line - line);
		if (d < dist) {
			dist = d;
			best = cur;
		}
		d = (line >= top->line ? line - top->line : top->line - line);
		if (d < dist) {
			dist = d;
			best = top;
		}
		p = pdup(best);
		p_goto_bol(p);
	}
	while (line > p->line)
		if (!pnextl(p))
			break;
	if (line < p->line) {
		while (line < p->line)
			pprevl(p);
		p_goto_bol(p);
	}
	return p;
}

/* Scroll window to follow cursor */

int mid = 0;

void bwfllw(BW *w)
{
	P *newtop;

	if (w->cursor->line < w->top->line) {
		newtop = pdup(w->cursor);
		p_goto_bol(newtop);
		if (mid) {
			if (newtop->line >= w->h / 2)
				pline(newtop, newtop->line - w->h / 2);
			else
				pset(newtop, newtop->b->bof);
		}
		if (w->top->line - newtop->line < w->h)
			nscrldn(w->t->t, w->y, w->y + w->h, (int) (w->top->line - newtop->line));
		else {
			msetI(w->t->t->updtab + w->y, 1, w->h);
			msetI(w->t->t->syntab + w->y, -1, w->h);
		}
		pset(w->top, newtop);
		prm(newtop);
	} else if (w->cursor->line >= w->top->line + w->h) {
		newtop = pdup(w->top);
		if (mid)
			newtop = getto(NULL, w->cursor, w->top, w->cursor->line - w->h / 2);
		else
			newtop = getto(NULL, w->cursor, w->top, w->cursor->line - (w->h - 1));
		if (newtop->line - w->top->line < w->h)
			nscrlup(w->t->t, w->y, w->y + w->h, (int) (newtop->line - w->top->line));
		else {
			msetI(w->t->t->updtab + w->y, 1, w->h);
			msetI(w->t->t->syntab + w->y, -1, w->h);
		}
		pset(w->top, newtop);
		prm(newtop);
	}

/* Adjust column */
	if (w->cursor->xcol < w->offset) {
		w->offset = w->cursor->xcol;
		msetI(w->t->t->updtab + w->y, 1, w->h);
	} else if (w->cursor->xcol >= w->offset + w->w) {
		w->offset = w->cursor->xcol - (w->w - 1);
		msetI(w->t->t->updtab + w->y, 1, w->h);
	}
}

/* Determine highlighting state of a particular line on the window.
   If the state is not known, it is computed and the state for all
   of the remaining lines of the window are also recalculated. */

int get_highlight_state(BW *w,int line)
{
	P *tmp = 0;
	int state;

	/* Screen y position of requested line */
	int y = line-w->top->line+w->y;

	if(!w->o.highlight || !w->syntax)
		return -1;

	/* If we know the state, just return it */
	if (w->parent->t->t->syntab[y]>=0)
		return w->parent->t->t->syntab[y];

	/* Scan upwards until we have a line with known state or
	   we're on the first line */
	while (y > w->y && w->parent->t->t->syntab[y] < 0) --y;

	/* If we don't have state for this line, calculate by going 100 lines back */
	if (w->parent->t->t->syntab[y]<0) {
		/* We must be on the top line */
		state = 0;
		tmp = pdup(w->top);
		if(tmp->line>50)
			pline(tmp, tmp->line-50);
		else
			p_goto_bof(tmp);
		while(tmp->line!=y-w->y+w->top->line)
			state = parse(w->syntax,tmp,state);
		w->parent->t->t->syntab[y] = state;
		w->parent->t->t->updtab[y] = 1;
		prm(tmp);
	}

	/* Color to end of screen */
	tmp = pdup(w->top);
	pline(tmp, y-w->y+w->top->line);
	state = w->parent->t->t->syntab[y];
	while(tmp->line!=w->top->line+w->h-1 && !piseof(tmp)) {
		state = parse(w->syntax,tmp,state);
		w->parent->t->t->syntab[++y] = state;
		w->parent->t->t->updtab[y] = 1; /* This could be smarter: update only if we changed what was there before */
		}
	prm(tmp);
	while(y<w->y+w->h-1) {
		w->parent->t->t->syntab[++y] = state;
		}

	/* Line after window */
	/* state = parse_c(state,syn,tmp); */

	/* If we changed, fix other windows */
	/* w->state = state; */

	/* Return state of requested line */
	y = line - w->top->line + w->y;
	return w->parent->t->t->syntab[y];
}

/* Scroll a buffer window after an insert occured.  'flg' is set to 1 if
 * the first line was split
 */

void bwins(BW *w, long int l, long int n, int flg)
{
	if (l + flg + n < w->top->line + w->h && l + flg >= w->top->line && l + flg <= w->b->eof->line) {
		if (flg)
			w->t->t->sary[w->y + l - w->top->line] = w->t->t->li;
		nscrldn(w->t->t, (int) (w->y + l + flg - w->top->line), w->y + w->h, (int) n);
	}
	if (l < w->top->line + w->h && l >= w->top->line) {
		if (n >= w->h - (l - w->top->line)) {
			msetI(w->t->t->updtab + w->y + l - w->top->line, 1, w->h - (int) (l - w->top->line));
			msetI(w->t->t->syntab + w->y + l - w->top->line, -1, w->h - (int) (l - w->top->line));
		} else {
			msetI(w->t->t->updtab + w->y + l - w->top->line, 1, (int) n + 1);
			msetI(w->t->t->syntab + w->y + l - w->top->line, -1, (int) n + 1);
		}
	}
}

/* Scroll current windows after a delete */

void bwdel(BW *w, long int l, long int n, int flg)
{
/* Update the line where the delete began */
	if (l < w->top->line + w->h && l >= w->top->line)
		w->t->t->updtab[w->y + l - w->top->line] = 1;

/* Update highlight for line after first one which changed */
	if ((l+1) < w->top->line + w->h && (l+1) >= w->top->line) {
		w->t->t->syntab[w->y + (l+1) - w->top->line] = -1;
		w->t->t->updtab[w->y + (l+1) - w->top->line] = 1;
		}

/* Update the line where the delete ended */
	if (l + n < w->top->line + w->h && l + n >= w->top->line)
		w->t->t->updtab[w->y + l + n - w->top->line] = 1;

	if (l < w->top->line + w->h && (l + n >= w->top->line + w->h || (l + n == w->b->eof->line && w->b->eof->line >= w->top->line + w->h))) {
		if (l >= w->top->line)
			/* Update window from l to end */
			msetI(w->t->t->updtab + w->y + l - w->top->line, 1, w->h - (int) (l - w->top->line));
		else
			/* Update entire window */
			msetI(w->t->t->updtab + w->y, 1, w->h);
	} else if (l < w->top->line + w->h && l + n == w->b->eof->line && w->b->eof->line < w->top->line + w->h) {
		if (l >= w->top->line)
			/* Update window from l to end of file */
			msetI(w->t->t->updtab + w->y + l - w->top->line, 1, (int) n);
		else
			/* Update from beginning of window to end of file */
			msetI(w->t->t->updtab + w->y, 1, (int) (w->b->eof->line - w->top->line));
	} else if (l + n < w->top->line + w->h && l + n > w->top->line && l + n < w->b->eof->line) {
		if (l + flg >= w->top->line)
			nscrlup(w->t->t, (int) (w->y + l + flg - w->top->line), w->y + w->h, (int) n);
		else
			nscrlup(w->t->t, w->y, w->y + w->h, (int) (l + n - w->top->line));
	}
}

/* Update a single line */

static int lgen(SCRN *t, int y, int *screen, int *attr, int x, int w, P *p, long int scr, long int from, long int to,int st,BW *bw)
        
      
            			/* Screen line address */
      				/* Window */
     				/* Buffer pointer */
         			/* Starting column to display */
              			/* Range for marked block */
{
	int ox = x;
	int tach;
	int done = 1;
	long col = 0;
	long byte = p->byte;
	unsigned char *bp;	/* Buffer pointer, 0 if not set */
	int amnt;		/* Amount left in this segment of the buffer */
	int c, ta, c1;
	unsigned char bc;

	struct utf8_sm utf8_sm;

        int *syn;
        P *tmp;
        int idx=0;
        int atr = 0;

	utf8_init(&utf8_sm);

	if(st!=-1) {
		tmp=pdup(p);
		p_goto_bol(tmp);
		parse(bw->syntax,tmp,st);
		syn = attr_buf;
		prm(tmp);
	}

/* Initialize bp and amnt from p */
	if (p->ofst >= p->hdr->hole) {
		bp = p->ptr + p->hdr->ehole + p->ofst - p->hdr->hole;
		amnt = SEGSIZ - p->hdr->ehole - (p->ofst - p->hdr->hole);
	} else {
		bp = p->ptr + p->ofst;
		amnt = p->hdr->hole - p->ofst;
	}

	if (col == scr)
		goto loop;
      lp:			/* Display next character */
	if (amnt)
		do {
			bc = *bp++;
			if(st!=-1)
				atr = syn[idx++];
			if (p->b->o.crlf && bc == '\r') {
				++byte;
				if (!--amnt) {
				      pppl:
					if (bp == p->ptr + SEGSIZ) {
						if (pnext(p)) {
							bp = p->ptr;
							amnt = p->hdr->hole;
						} else
							goto nnnl;
					} else {
						bp = p->ptr + p->hdr->ehole;
						amnt = SEGSIZ - p->hdr->ehole;
						if (!amnt)
							goto pppl;
					}
				}
				if (*bp == '\n') {
					++bp;
					++byte;
					++amnt;
					goto eobl;
				}
			      nnnl:
				--byte;
				++amnt;
			}
			if (square)
				if (bc == '\t') {
					long tcol = col + p->b->o.tab - col % p->b->o.tab;

					if (tcol > from && tcol <= to)
						c1 = INVERSE;
					else
						c1 = 0;
				} else if (col >= from && col < to)
					c1 = INVERSE;
				else
					c1 = 0;
			else if (byte >= from && byte < to)
				c1 = INVERSE;
			else
				c1 = 0;
			++byte;
			if (bc == '\t') {
				ta = p->b->o.tab - col % p->b->o.tab;
				if (ta + col > scr) {
					ta -= scr - col;
					tach = ' ';
					goto dota;
				}
				if ((col += ta) == scr) {
					--amnt;
					goto loop;
				}
			} else if (bc == '\n')
				goto eobl;
			else {
				int wid;
				if (p->b->o.utf8) {
					c = utf8_decode(&utf8_sm,bc);

					if (c>=0) /* Normal decoded character */
						wid = mk_wcwidth(c);
					else if(c== -1) /* Character taken */
						wid = -1;
					else if(c== -2) /* Incomplete sequence (FIXME: do something better here) */
						wid = 1;
					else if(c== -3) /* Control character 128-191, 254, 255 */
						wid = 1;
				} else {
					wid = 1;
				}

				if(wid>0) {
					col += wid;
					if (col == scr) {
						--amnt;
						goto loop;
					} else if (col > scr) {
						ta = col - scr;
						tach = '<';
						goto dota;
					}
				}
			}
		} while (--amnt);
	if (bp == p->ptr + SEGSIZ) {
		if (pnext(p)) {
			bp = p->ptr;
			amnt = p->hdr->hole;
			goto lp;
		}
	} else {
		bp = p->ptr + p->hdr->ehole;
		amnt = SEGSIZ - p->hdr->ehole;
		goto lp;
	}
	goto eof;

      loop:			/* Display next character */
	if (amnt)
		do {
			bc = *bp++;
			if(st!=-1)
				atr=syn[idx++];
			if (p->b->o.crlf && bc == '\r') {
				++byte;
				if (!--amnt) {
				      ppl:
					if (bp == p->ptr + SEGSIZ) {
						if (pnext(p)) {
							bp = p->ptr;
							amnt = p->hdr->hole;
						} else
							goto nnl;
					} else {
						bp = p->ptr + p->hdr->ehole;
						amnt = SEGSIZ - p->hdr->ehole;
						if (!amnt)
							goto ppl;
					}
				}
				if (*bp == '\n') {
					++bp;
					++byte;
					++amnt;
					goto eobl;
				}
			      nnl:
				--byte;
				++amnt;
			}
			if (square)
				if (bc == '\t') {
					long tcol = scr + x - ox + p->b->o.tab - (scr + x - ox) % p->b->o.tab;

					if (tcol > from && tcol <= to)
						c1 = INVERSE;
					else
						c1 = 0;
				} else if (scr + x - ox >= from && scr + x - ox < to)
					c1 = INVERSE;
				else
					c1 = 0;
			else if (byte >= from && byte < to)
				c1 = INVERSE;
			else
				c1 = 0;
			++byte;
			if (bc == '\t') {
				ta = p->b->o.tab - ((x - ox + scr) % p->b->o.tab);
				tach = ' ';
			      dota:
				do {
					outatr(t, screen + x, attr + x, x, y, tach, c1|atr);
					if (ifhave)
						goto bye;
					if (++x == w)
						goto eosl;
				} while (--ta);
			} else if (bc == '\n')
				goto eobl;
			else {
				int wid = -1;
				int utf8_char;
				if (p->b->o.utf8) { /* UTF-8 */

					utf8_char = utf8_decode(&utf8_sm,bc);

					if (utf8_char >= 0) { /* Normal decoded character */
						if (utf8_char<32 || utf8_char>126 && utf8_char<160) { /* Control character */
							bc = utf8_char;
							xlat(&c, &bc);
							utf8_char = bc;
							c1 ^= c;
							wid = 1;
						} else {
							wid = mk_wcwidth(utf8_char);
						}
					} else if(c== -1) { /* Character taken */
						wid = -1;
					} else if(c== -2) { /* Incomplete sequence (FIXME: do something better here) */
						utf8_char = 'X';
						wid = 1;
					} else if(c== -3) { /* Invalid UTF-8 start character 128-191, 254, 255 */
						/* Show as control character */
						xlat(&c, &bc);
						utf8_char = bc;
						c1 ^= c;
						wid = 1;
					}
				} else { /* Regular */
					xlat(&c, &bc);
					utf8_char = bc;
					c1 ^= c;
					wid = 1;
				}

				if(wid>=0) {
					if (x+wid > w) {
						/* If character hits right most column, don't display it */
						while (x < w) {
							outatr(t, screen + x, attr + x, x, y, '>', c1|atr);
							x++;
						}
					} else {
						outatr(t, screen + x, attr + x, x, y, utf8_char, c1|atr);
						x += wid;
					}
				}

				if (ifhave)
					goto bye;
				if (x >= w)
					goto eosl;
			}
		} while (--amnt);
	if (bp == p->ptr + SEGSIZ) {
		if (pnext(p)) {
			bp = p->ptr;
			amnt = p->hdr->hole;
			goto loop;
		}
	} else {
		bp = p->ptr + p->hdr->ehole;
		amnt = SEGSIZ - p->hdr->ehole;
		goto loop;
	}
	goto eof;

      eobl:			/* End of buffer line found.  Erase to end of screen line */
	++p->line;
      eof:
	if (x != w)
		done = eraeol(t, x, y);
	else
		done = 0;

/* Set p to bp/amnt */
      bye:
	if (bp - p->ptr <= p->hdr->hole)
		p->ofst = bp - p->ptr;
	else
		p->ofst = bp - p->ptr - (p->hdr->ehole - p->hdr->hole);
	p->byte = byte;
	return done;

      eosl:
	if (bp - p->ptr <= p->hdr->hole)
		p->ofst = bp - p->ptr;
	else
		p->ofst = bp - p->ptr - (p->hdr->ehole - p->hdr->hole);
	p->byte = byte;
	pnextl(p);
	return 0;
}

/* Generate line into an array */

static int lgena(SCRN *t, int y, int *screen, int x, int w, P *p, long int scr, long int from, long int to)
        
      
            			/* Screen line address */
      				/* Window */
     				/* Buffer pointer */
         			/* Starting column to display */
              			/* Range for marked block */
{
	int ox = x;
	int done = 1;
	long col = 0;
	long byte = p->byte;
	unsigned char *bp;	/* Buffer pointer, 0 if not set */
	int amnt;		/* Amount left in this segment of the buffer */
	int c, ta, c1;
	unsigned char bc;

/* Initialize bp and amnt from p */
	if (p->ofst >= p->hdr->hole) {
		bp = p->ptr + p->hdr->ehole + p->ofst - p->hdr->hole;
		amnt = SEGSIZ - p->hdr->ehole - (p->ofst - p->hdr->hole);
	} else {
		bp = p->ptr + p->ofst;
		amnt = p->hdr->hole - p->ofst;
	}

	if (col == scr)
		goto loop;
      lp:			/* Display next character */
	if (amnt)
		do {
			bc = *bp++;
			if (square)
				if (bc == '\t') {
					long tcol = col + p->b->o.tab - col % p->b->o.tab;

					if (tcol > from && tcol <= to)
						c1 = INVERSE;
					else
						c1 = 0;
				} else if (col >= from && col < to)
					c1 = INVERSE;
				else
					c1 = 0;
			else if (byte >= from && byte < to)
				c1 = INVERSE;
			else
				c1 = 0;
			++byte;
			if (bc == '\t') {
				ta = p->b->o.tab - col % p->b->o.tab;
				if (ta + col > scr) {
					ta -= scr - col;
					goto dota;
				}
				if ((col += ta) == scr) {
					--amnt;
					goto loop;
				}
			} else if (bc == '\n')
				goto eobl;
			else if (++col == scr) {
				--amnt;
				goto loop;
			}
		} while (--amnt);
	if (bp == p->ptr + SEGSIZ) {
		if (pnext(p)) {
			bp = p->ptr;
			amnt = p->hdr->hole;
			goto lp;
		}
	} else {
		bp = p->ptr + p->hdr->ehole;
		amnt = SEGSIZ - p->hdr->ehole;
		goto lp;
	}
	goto eobl;

      loop:			/* Display next character */
	if (amnt)
		do {
			bc = *bp++;
			if (square)
				if (bc == '\t') {
					long tcol = scr + x - ox + p->b->o.tab - (scr + x - ox) % p->b->o.tab;

					if (tcol > from && tcol <= to)
						c1 = INVERSE;
					else
						c1 = 0;
				} else if (scr + x - ox >= from && scr + x - ox < to)
					c1 = INVERSE;
				else
					c1 = 0;
			else if (byte >= from && byte < to)
				c1 = INVERSE;
			else
				c1 = 0;
			++byte;
			if (bc == '\t') {
				ta = p->b->o.tab - ((x - ox + scr) % p->b->o.tab);
			      dota:
				do {
					screen[x] = ' ' + c1;
					if (++x == w)
						goto eosl;
				} while (--ta);
			} else if (bc == '\n')
				goto eobl;
			else {
				xlat(&c, &bc);
				c ^= c1;
				screen[x] = c + bc;
				if (++x == w)
					goto eosl;
			}
		} while (--amnt);
	if (bp == p->ptr + SEGSIZ) {
		if (pnext(p)) {
			bp = p->ptr;
			amnt = p->hdr->hole;
			goto loop;
		}
	} else {
		bp = p->ptr + p->hdr->ehole;
		amnt = SEGSIZ - p->hdr->ehole;
		goto loop;
	}
	goto eof;
      eobl:			/* End of buffer line found.  Erase to end of screen line */
	++p->line;
      eof:
	while (x != w)
		screen[x++] = ' ';
	done = 0;

/* Set p to bp/amnt */
	if (bp - p->ptr <= p->hdr->hole)
		p->ofst = bp - p->ptr;
	else
		p->ofst = bp - p->ptr - (p->hdr->ehole - p->hdr->hole);
	p->byte = byte;
	return done;

      eosl:
	if (bp - p->ptr <= p->hdr->hole)
		p->ofst = bp - p->ptr;
	else
		p->ofst = bp - p->ptr - (p->hdr->ehole - p->hdr->hole);
	p->byte = byte;
	pnextl(p);
	return 0;
}

static void gennum(BW *w, int *screen, int *attr, SCRN *t, int y, int *comp)
{
	unsigned char buf[12];
	int z;
	int lin = w->top->line + y - w->y;

	if (lin <= w->b->eof->line)
		snprintf((char *)buf, sizeof(buf), "%5ld ", w->top->line + y - w->y + 1);
	else
		strcpy(buf, "      ");
	for (z = 0; buf[z]; ++z) {
		outatr(t, screen + z, attr + z, z, y, buf[z], 0);
		if (ifhave)
			return;
		comp[z] = buf[z];
	}
}

void bwgen(BW *w, int linums)
{
	int *screen;
	int *attr;
	P *p = NULL;
	P *q = pdup(w->cursor);
	int bot = w->h + w->y;
	int y;
	int dosquare = 0;
	long from, to;
	long fromline, toline;
	SCRN *t = w->t->t;

	fromline = toline = from = to = 0;

	if (markv(0) && markk->b == w->b)
		if (square) {
			from = markb->xcol;
			to = markk->xcol;
			dosquare = 1;
			fromline = markb->line;
			toline = markk->line;
		} else {
			from = markb->byte;
			to = markk->byte;
		}
	else if (marking && markb && markb->b == w->b && w->cursor->byte != markb->byte && !from) {
		if (square) {
			from = long_min(w->cursor->xcol, markb->xcol);
			to = long_max(w->cursor->xcol, markb->xcol);
			fromline = long_min(w->cursor->line, markb->line);
			toline = long_max(w->cursor->line, markb->line);
			dosquare = 1;
		} else {
			from = long_min(w->cursor->byte, markb->byte);
			to = long_max(w->cursor->byte, markb->byte);
		}
	}

	if (marking)
		msetI(t->updtab + w->y, 1, w->h);

	y = w->cursor->line - w->top->line + w->y;
	attr = t->attr + y*w->t->w;
	for (screen = t->scrn + y * w->t->w; y != bot; ++y, (screen += w->t->w), (attr += w->t->w)) {
		if (ifhave && !linums)
			break;
		if (linums)
			gennum(w, screen, attr, t, y, t->compose);
		if (t->updtab[y]) {
			p = getto(p, w->cursor, w->top, w->top->line + y - w->y);
/*			if (t->insdel && !w->x) {
				pset(q, p);
				if (dosquare)
					if (w->top->line + y - w->y >= fromline && w->top->line + y - w->y <= toline)
						lgena(t, y, t->compose, w->x, w->x + w->w, q, w->offset, from, to);
					else
						lgena(t, y, t->compose, w->x, w->x + w->w, q, w->offset, 0L, 0L);
				else
					lgena(t, y, t->compose, w->x, w->x + w->w, q, w->offset, from, to);
				magic(t, y, screen, attr, t->compose, (int) (w->cursor->xcol - w->offset + w->x));
			} */
			if (dosquare)
				if (w->top->line + y - w->y >= fromline && w->top->line + y - w->y <= toline)
					t->updtab[y] = lgen(t, y, screen, attr, w->x, w->x + w->w, p, w->offset, from, to, get_highlight_state(w,w->top->line+y-w->y),w);
				else
					t->updtab[y] = lgen(t, y, screen, attr, w->x, w->x + w->w, p, w->offset, 0L, 0L, get_highlight_state(w,w->top->line+y-w->y),w);
			else
				t->updtab[y] = lgen(t, y, screen, attr, w->x, w->x + w->w, p, w->offset, from, to, get_highlight_state(w,w->top->line+y-w->y),w);
		}
	}

	y = w->y;
	attr = t->attr + w->y * w->t->w;
	for (screen = t->scrn + w->y * w->t->w; y != w->y + w->cursor->line - w->top->line; ++y, (screen += w->t->w), (attr += w->t->w)) {
		if (ifhave && !linums)
			break;
		if (linums)
			gennum(w, screen, attr, t, y, t->compose);
		if (t->updtab[y]) {
			p = getto(p, w->cursor, w->top, w->top->line + y - w->y);
/*			if (t->insdel && !w->x) {
				pset(q, p);
				if (dosquare)
					if (w->top->line + y - w->y >= fromline && w->top->line + y - w->y <= toline)
						lgena(t, y, t->compose, w->x, w->x + w->w, q, w->offset, from, to);
					else
						lgena(t, y, t->compose, w->x, w->x + w->w, q, w->offset, 0L, 0L);
				else
					lgena(t, y, t->compose, w->x, w->x + w->w, q, w->offset, from, to);
				magic(t, y, screen, attr, t->compose, (int) (w->cursor->xcol - w->offset + w->x));
			} */
			if (dosquare)
				if (w->top->line + y - w->y >= fromline && w->top->line + y - w->y <= toline)
					t->updtab[y] = lgen(t, y, screen, attr, w->x, w->x + w->w, p, w->offset, from, to, get_highlight_state(w,w->top->line+y-w->y),w);
				else
					t->updtab[y] = lgen(t, y, screen, attr, w->x, w->x + w->w, p, w->offset, 0L, 0L, get_highlight_state(w,w->top->line+y-w->y),w);
			else
				t->updtab[y] = lgen(t, y, screen, attr, w->x, w->x + w->w, p, w->offset, from, to, get_highlight_state(w,w->top->line+y-w->y),w);
		}
	}
	prm(q);
	if (p)
		prm(p);
}

void bwmove(BW *w, int x, int y)
{
	w->x = x;
	w->y = y;
}

void bwresz(BW *w, int wi, int he)
{
	if (he > w->h && w->y != -1) {
		msetI(w->t->t->updtab + w->y + w->h, 1, he - w->h);
		msetI(w->t->t->syntab + w->y + w->h, -1, he - w->h);
		}
	w->w = wi;
	w->h = he;
}

BW *bwmk(W *window, B *b, int prompt)
{
	BW *w = (BW *) joe_malloc(sizeof(BW));

	w->parent = window;
	w->pid = 0;
	w->out = -1;
	w->b = b;
	if (prompt || (!window->y && staen)) {
		w->y = window->y;
		w->h = window->h;
	} else {
		w->y = window->y + 1;
		w->h = window->h - 1;
	}
	if (b->oldcur) {
		w->top = b->oldtop;
		b->oldtop = NULL;
		w->top->owner = NULL;
		w->cursor = b->oldcur;
		b->oldcur = NULL;
		w->cursor->owner = NULL;
	} else {
		w->top = pdup(b->bof);
		w->cursor = pdup(b->bof);
	}
	w->t = window->t;
	w->object = NULL;
	w->offset = 0;
	w->o = w->b->o;
	if (w->o.linums) {
		w->x = window->x + LINCOLS;
		w->w = window->w - LINCOLS;
	} else {
		w->x = window->x;
		w->w = window->w;
	}
	if (window == window->main) {
		rmkbd(window->kbd);
		window->kbd = mkkbd(kmap_getcontext(w->o.context));
	}
	w->top->xcol = 0;
	w->cursor->xcol = 0;
	w->top_changed = 1;
	if (w->o.syntax)
		w->syntax = load_dfa(w->o.syntax);
	else
		w->syntax = 0;
	return w;
}

void bwrm(BW *w)
{
	prm(w->top);
	prm(w->cursor);
	brm(w->b);
	joe_free(w);
}

int ustat(BW *bw)
{
	static unsigned char buf[80];
	int c = brch(bw->cursor);

	if (c == NO_MORE_DATA)
		snprintf((char *)buf, sizeof(buf), "** Line %ld  Col %ld  Offset %ld(0x%lx) **", bw->cursor->line + 1, piscol(bw->cursor) + 1, bw->cursor->byte, bw->cursor->byte);
	else
		snprintf((char *)buf, sizeof(buf), "** Line %ld  Col %ld  Offset %ld(0x%lx)  Ascii %d(0%o/0x%X) Width %d **", bw->cursor->line + 1, piscol(bw->cursor) + 1, bw->cursor->byte, bw->cursor->byte, c, c, c, mk_wcwidth(c));
	msgnw(bw->parent, buf);
	return 0;
}

int ucrawlr(BW *bw)
{
	int amnt = bw->w / 2;

	pcol(bw->cursor, bw->cursor->xcol + amnt);
	bw->cursor->xcol += amnt;
	bw->offset += amnt;
	updall();
	return 0;
}

int ucrawll(BW *bw)
{
	int amnt = bw->w / 2;
	int curamnt = bw->w / 2;

	if (amnt > bw->offset) {
		amnt = bw->offset;
		curamnt = bw->offset;
	}
	if (!bw->offset)
		curamnt = bw->cursor->xcol;
	if (!curamnt)
		return -1;
	pcol(bw->cursor, bw->cursor->xcol - curamnt);
	bw->cursor->xcol -= curamnt;
	bw->offset -= amnt;
	updall();
	return 0;
}

void orphit(BW *bw)
{
	++bw->b->count;
	bw->b->orphan = 1;
	pdupown(bw->cursor, &bw->b->oldcur);
	pdupown(bw->top, &bw->b->oldtop);
}
