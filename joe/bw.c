/*
 *	Edit buffer window generation
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "types.h"

/* Attributes for line numbers, and current line */
int bg_linum = 0;
int bg_curlinum = 0;
int bg_curlin = 0;
int curlinmask = -1;

/* Display modes */
int dspasis = 0;
int marking = 0;

/* Selected text format */
int selectatr = INVERSE;
int selectmask = ~INVERSE;

static P *getto(P *p, P *cur, P *top, off_t line)
{

	if (p == NULL) {
		P *best = cur;
		off_t dist = MAXOFF;
		off_t d;

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
		p = pdup(best, "getto");
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

/* Recenter cursor on vertical scroll if true */
int opt_mid = 0;

/* Amount to horizontally scroll when cursor goes past edge */
/* -1 means 1/4 width of screen */
int opt_left = 8;
int opt_right = 8;

/* For hex */

void bwfllwh(W *thew)
{
	BW *w = (BW *)thew->object;
	/* Top must be a multiple of 16 bytes */
	if (w->top->byte%16) {
		pbkwd(w->top,w->top->byte%16);
	}

	/* Move backward */
	if (w->cursor->byte < w->top->byte) {
		off_t new_top = w->cursor->byte/16;
		if (opt_mid) {
			if (new_top >= w->h / 2)
				new_top -= w->h / 2;
			else
				new_top = 0;
		}
		if (w->top->byte/16 - new_top < w->h)
			nscrldn(w->t->t, w->y, w->y + w->h, (int) (w->top->byte/16 - new_top));
		else
			msetI(w->t->t->updtab + w->y, 1, w->h);
		pgoto(w->top,new_top*16);
	}

	/* Move forward */
	if (w->cursor->byte >= w->top->byte+(w->h*16)) {
		off_t new_top;
		if (opt_mid) {
			new_top = w->cursor->byte/16 - w->h / 2;
		} else {
			new_top = w->cursor->byte/16 - (w->h - 1);
		}
		if (new_top - w->top->byte/16 < w->h)
			nscrlup(w->t->t, w->y, w->y + w->h, (int) (new_top - w->top->byte/16));
		else {
			msetI(w->t->t->updtab + w->y, 1, w->h);
		}
		pgoto(w->top, new_top*16);
	}

	/* Adjust scroll offset */
	if (w->cursor->byte%16+60 < w->offset) {
		w->offset = w->cursor->byte%16+60;
		msetI(w->t->t->updtab + w->y, 1, w->h);
	} else if (w->cursor->byte%16+60 >= w->offset + w->w) {
		w->offset = w->cursor->byte%16+60 - (w->w - 1);
		msetI(w->t->t->updtab + w->y, 1, w->h);
	}
}

/* For text */

void bwfllwt(W *thew)
{
	BW *w = (BW *)thew->object;
	P *newtop;

	if (!pisbol(w->top)) {
		p_goto_bol(w->top);
	}

	if (w->cursor->line < w->top->line) {
		newtop = pdup(w->cursor, "bwfllwt");
		p_goto_bol(newtop);
		if (opt_mid) {
			if (newtop->line >= w->h / 2)
				pline(newtop, newtop->line - w->h / 2);
			else
				pset(newtop, newtop->b->bof);
		}
		if (w->top->line - newtop->line < w->h)
			nscrldn(w->t->t, w->y, w->y + w->h, (int) (w->top->line - newtop->line));
		else {
			msetI(w->t->t->updtab + w->y, 1, w->h);
		}
		pset(w->top, newtop);
		prm(newtop);
	} else if (w->cursor->line >= w->top->line + w->h) {
		/* newtop = pdup(w->top); */
		/* getto() creates newtop */
		if (opt_mid)
			newtop = getto(NULL, w->cursor, w->top, w->cursor->line - w->h / 2);
		else
			newtop = getto(NULL, w->cursor, w->top, w->cursor->line - (w->h - 1));
		if (newtop->line - w->top->line < w->h)
			nscrlup(w->t->t, w->y, w->y + w->h, (int) (newtop->line - w->top->line));
		else {
			msetI(w->t->t->updtab + w->y, 1, w->h);
		}
		pset(w->top, newtop);
		prm(newtop);
	}

/* Adjust column */
	if (w->cursor->xcol < w->offset) {
		/* Need to scroll left */
		off_t target = w->cursor->xcol;
		ptrdiff_t amnt;

		if (opt_left < 0) {
			amnt = w->w / (-opt_left);
		} else {
			amnt = opt_left - 1;
		}

		if (amnt >= w->w)
			amnt = w->w - 1;

		if (amnt < 0)
			amnt = 0;

		if (target < amnt) {
			target = 0;
		} else {
			target -= amnt;
		}
		w->offset = target;
		msetI(w->t->t->updtab + w->y, 1, w->h);
	}
	if (w->cursor->xcol >= w->offset + w->w) {
		/* Need to scroll right */
		ptrdiff_t amnt;
		if (opt_right < 0) {
			amnt = w->w - w->w/(-opt_right);
		} else {
			amnt = w->w - opt_right;
		}
		if (amnt >= w->w)
			amnt = w->w - 1;
		if (amnt < 0)
			amnt = 0;

		w->offset = w->cursor->xcol - amnt;

		msetI(w->t->t->updtab + w->y, 1, w->h);
	}

	if (w->o.hiline) {
		if (w->curlin != w->cursor->line) {
			/* Update old and new cursor lines */
			if (w->curlin >= w->top->line && w->curlin < (w->top->line + w->h))
				w->t->t->updtab[w->y + w->curlin - w->top->line] = 1;
			w->curlin = w->cursor->line;
			w->t->t->updtab[w->y + w->curlin - w->top->line] = 1;
		}
	} else {
		w->curlin = w->cursor->line;
	}
}

/* For either */

void bwfllw(W *w)
{
	BW *bw = (BW *)w->object;
	if (bw->o.hex)
		bwfllwh(w);
	else
		bwfllwt(w);
}

/* Determine highlighting state of a particular line on the window.
   If the state is not known, it is computed and the state for all
   of the remaining lines of the window are also recalculated. */

static HIGHLIGHT_STATE get_highlight_state(BW *w, P *p, off_t line)
{
	HIGHLIGHT_STATE state;

	if(!w->o.highlight || !w->o.syntax) {
		invalidate_state(&state);
		return state;
	}

	return lattr_get(w->db, w->o.syntax, p, line); /* FIXME: lattr database should be a in vfile */
}

/* Scroll a buffer window after an insert occurred.  'flg' is set to 1 if
 * the first line was split
 */

void bwins(BW *w, off_t l, off_t n, int flg)
{
	/* If highlighting is enabled... */
	if (w->o.highlight && w->o.syntax) {
		/* Invalidate cache */
		/* lattr_cut(w->db, l + 1); */
		/* Force updates */
		if (l < w->top->line) {
			msetI(w->t->t->updtab + w->y, 1, w->h);
		} else if ((l + 1) < w->top->line + w->h) {
			ptrdiff_t start = TO_DIFF_OK(l + 1 - w->top->line);
			ptrdiff_t size = w->h - start;
			msetI(w->t->t->updtab + w->y + start, 1, size);
		}
	}

	/* Scroll */
	if (l + flg + n < w->top->line + w->h && l + flg >= w->top->line && l + flg <= w->b->eof->line) {
		if (flg)
			w->t->t->sary[w->y + l - w->top->line] = w->t->t->li;
		nscrldn(w->t->t, (int) (w->y + l + flg - w->top->line), w->y + w->h, (int) n);
	}

	/* Force update of lines in opened hole */
	if (l < w->top->line + w->h && l >= w->top->line) {
		if (n >= w->h - (l - w->top->line)) {
			msetI(w->t->t->updtab + w->y + l - w->top->line, 1, w->h - (int) (l - w->top->line));
		} else {
			msetI(w->t->t->updtab + w->y + l - w->top->line, 1, (int) n + 1);
		}
	}
}

/* Scroll current windows after a delete */

void bwdel(BW *w, off_t l, off_t n, int flg)
{
	/* If highlighting is enabled... */
	if (w->o.highlight && w->o.syntax) {
		/* lattr_cut(w->db, l + 1); */
		if (l < w->top->line) {
			msetI(w->t->t->updtab + w->y, 1, w->h);
		} else if ((l + 1) < w->top->line + w->h) {
			ptrdiff_t start = TO_DIFF_OK(l + 1 - w->top->line);
			ptrdiff_t size = w->h - start;
			msetI(w->t->t->updtab + w->y + start, 1, size);
		}
	}

	/* Update the line where the delete began */
	if (l < w->top->line + w->h && l >= w->top->line)
		w->t->t->updtab[w->y + l - w->top->line] = 1;

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

struct ansi_sm
{
	int state;
};

static int ansi_decode(struct ansi_sm *sm, int bc)
{
	if (sm->state) {
		if ((bc >= 'a' && bc <= 'z') || (bc >= 'A' && bc <= 'Z'))
			sm->state = 0;
		return -1;
	} else if (bc == '\033') {
		sm->state = 1;
		return -1;
	} else
		return bc;
}

static void ansi_init(struct ansi_sm *sm)
{
	sm->state = 0;
}

#define SELECT_IF(c)	{ if (c) { ca = selectatr; cm = selectmask; } else { ca = 0; cm = -1; } }

/* Update a single line */

static int lgen(SCRN *t, ptrdiff_t y, int (*screen)[COMPOSE], int *attr, ptrdiff_t x, ptrdiff_t w, P *p, off_t scr, off_t from, off_t to,HIGHLIGHT_STATE st,BW *bw)


            			/* Screen line address */
      				/* Window */
     				/* Buffer pointer */
         			/* Starting column to display */
              			/* Range for marked block */
{
	int ansi = bw->o.ansi;
	ptrdiff_t ox = x;
	int tach;
	int done = 1;
	off_t col = 0;
	off_t byte = p->byte;
	char *bp;	/* Buffer pointer, 0 if not set */
	ptrdiff_t amnt;		/* Amount left in this segment of the buffer */
	int c;
	off_t ta;
	char bc;
	int ungetit = NO_MORE_DATA;

	struct utf8_sm utf8_sm;
	struct ansi_sm ansi_sm;

        int *syn = 0;
        P *tmp;
        int idx=0;
        int defatr = (bw->o.hiline && bw->cursor->line == y - bw->y + bw->top->line) ? (bg_text & curlinmask) | bg_curlin : bg_text;
        int atr = BG_COLOR(defatr);
        int ca = 0;		/* Additional attributes for current character */
        int cm = -1;		/* Attribute mask for current character */

	utf8_init(&utf8_sm);
	ansi_init(&ansi_sm);

	if(st.state!=-1) {
		tmp=pdup(p, "lgen");
		p_goto_bol(tmp);
		parse(bw->o.syntax,tmp,st,p->b->o.charmap);
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
			if (ungetit == NO_MORE_DATA)
				bc = *bp++;
			else {
				bc = TO_CHAR_OK(ungetit);
				ungetit = NO_MORE_DATA;
			}
			if(st.state!=-1) {
				atr = syn[idx++] & ~CONTEXT_MASK;
				if (!(atr & BG_MASK))
					atr |= defatr & BG_MASK;
				if (!(atr & FG_MASK))
					atr |= defatr & FG_MASK;
			}
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
					off_t tcol = col + p->b->o.tab - col % p->b->o.tab;

					SELECT_IF(tcol > from && tcol <= to);
				} else {
					SELECT_IF(col >= from && col < to);
				}
			else
				SELECT_IF(byte >= from && byte < to);
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
				int wid = 1;
				if (p->b->o.charmap->type) {
					c = utf8_decode(&utf8_sm,bc);

					if (c>=0) /* Normal decoded character */
						if (ansi) {
							c = ansi_decode(&ansi_sm, c);
							if (c >= 0) /* Not ansi */
								wid = joe_wcwidth(1, c);
							else { /* Skip ANSI character */
								wid = 0;
								++idx;
							}
						} else
							wid = joe_wcwidth(1,c);
					else if(c == UTF8_ACCEPTED) /* Character taken */
						wid = -1;
					else if(c == UTF8_INCOMPLETE) { /* Incomplete sequence (FIXME: do something better here) */
						wid = 1;
						ungetit = c;
						++amnt;
						--byte;
					}
					else if(c == UTF8_BAD) /* Control character 128-191, 254, 255 */
						wid = 1;
				} else {
					if (ansi) {
						c = ansi_decode(&ansi_sm, bc);
						if (c>=0) /* Not ANSI */
							wid = 1;
						else {
							wid = 0;
							++idx;
						}
					} else {
						wid = 1;
					}
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
				} else
					--idx;	/* Get highlighting character again.. */
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
			if (ungetit == NO_MORE_DATA)
				bc = *bp++;
			else {
				bc = TO_CHAR_OK(ungetit);
				ungetit = NO_MORE_DATA;
			}
			if(st.state!=-1) {
				atr = syn[idx++] & ~CONTEXT_MASK;
				if (!(atr & BG_MASK))
					atr |= defatr & BG_MASK;
				if (!(atr & FG_MASK))
					atr |= defatr & FG_MASK;
			}
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
			if (square) {
				if (bc == '\t') {
					off_t tcol = scr + x - ox + p->b->o.tab - (scr + x - ox) % p->b->o.tab;
					SELECT_IF(tcol > from && tcol <= to);
				} else {
					SELECT_IF(scr + x - ox >= from && scr + x - ox < to);
				}
			} else {
				SELECT_IF(byte >= from && byte < to);
			}
			++byte;
			if (bc == '\t') {
				ta = p->b->o.tab - (x - ox + scr) % p->b->o.tab;
				tach = ' ';
			      dota:
			      	while (x < w && ta--) {
					outatr(bw->b->o.charmap, t, screen + x, attr + x, x, y, tach, (atr & cm) | ca);
					++x;
				}
				if (ifhave)
					goto bye;
				if (x > w)
					goto eosl;
			} else if (bc == '\n')
				goto eobl;
			else {
				int wid = -1;
				int utf8_char;
				if (p->b->o.charmap->type) { /* UTF-8 */

					utf8_char = utf8_decode(&utf8_sm,bc);

					if (utf8_char >= 0) { /* Normal decoded character */
						if (ansi) {
							utf8_char = ansi_decode(&ansi_sm, utf8_char);
							if (utf8_char >= 0) {
								wid = joe_wcwidth(1, utf8_char);
							} else {
								wid = -1;
								++idx;
							}
						} else
							wid = joe_wcwidth(1,utf8_char);
					} else if(utf8_char == UTF8_ACCEPTED) { /* Character taken */
						wid = -1;
					} else if(utf8_char == UTF8_INCOMPLETE) { /* Incomplete sequence (FIXME: do something better here) */
						ungetit = bc;
						++amnt;
						--byte;
						utf8_char = 'X';
						wid = 1;
					} else if(utf8_char == UTF8_BAD) { /* Invalid UTF-8 start character 128-191, 254, 255 */
						/* Show as control character */
						wid = 1;
						utf8_char = 'X';
					}
				} else { /* Regular */
					if (ansi) {
						utf8_char = ansi_decode(&ansi_sm, bc);
						if (utf8_char >= 0) /* Not ANSI */
							wid = 1;
						else {
							wid = -1;
							++idx;
						}
					} else {
						utf8_char = (unsigned char)bc;
						wid = 1;
					}
				}

				if(wid >= 0) {
					if (x + wid > w) {
						/* If character hits right most column, don't display it */
						while (x < w) {
							outatr(bw->b->o.charmap, t, screen + x, attr + x, x, y, '>', (atr & cm) | ca);
							x++;
						}
						goto eosl;
					} else {
						outatr(bw->b->o.charmap, t, screen + x, attr + x, x, y, utf8_char, (atr & cm) | ca);
						x += wid;
					}
				} else
					--idx;

				if (ifhave)
					goto bye;
				if (x > w)
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
      	outatr_complete(t);
	if (x < w)
		done = eraeol(t, x, y, BG_COLOR(defatr));
	else
		done = 0;

/* Set p to bp/amnt */
      bye:
      	outatr_complete(t);
	if (bp - p->ptr <= p->hdr->hole)
		p->ofst = (short)(bp - p->ptr);
	else
		p->ofst = (short)(bp - p->ptr - (p->hdr->ehole - p->hdr->hole));
	p->byte = byte;
	return done;

      eosl:
      	outatr_complete(t);
	if (bp - p->ptr <= p->hdr->hole)
		p->ofst = (short)(bp - p->ptr);
	else
		p->ofst = (short)(bp - p->ptr - (p->hdr->ehole - p->hdr->hole));
	p->byte = byte;
	pnextl(p);
	return 0;
}

static void gennum(BW *w, int (*screen)[COMPOSE], int *attr, SCRN *t, ptrdiff_t y, int *comp)
{
	char buf[24];
	ptrdiff_t z, x;
	off_t lin = w->top->line + y - w->y;

	if (lin <= w->b->eof->line)
#ifdef HAVE_LONG_LONG
		joe_snprintf_1(buf, SIZEOF(buf), " %21lld ", (long long)(w->top->line + y - w->y + 1));
#else
		joe_snprintf_1(buf, SIZEOF(buf), " %21ld ", (long)(w->top->line + y - w->y + 1));
#endif
	else {
		for (x = 0; x != SIZEOF(buf) - 1; ++x)
			buf[x] = ' ';
		buf[x] = 0;
	}
	int atr = (w->o.hiline && lin == w->cursor->line) ? bg_curlinum : bg_linum;
	for (z = SIZEOF(buf) - w->lincols - 1, x = 0; buf[z]; ++z, ++x) {
		outatr(w->b->o.charmap, t, screen + x, attr + x, x, y, buf[z], BG_COLOR(atr));
		comp[x] = buf[z];
	}
	outatr_complete(t);
}

void bwgenh(BW *w)
{
	int (*screen)[COMPOSE];
	int *attr;
	P *q = pdup(w->top, "bwgenh");
	ptrdiff_t bot = w->h + w->y;
	ptrdiff_t y;
	SCRN *t = w->t->t;
	int flg = 0;
	off_t from;
	off_t to;
	int dosquare = 0;

	from = to = 0;

	if (markv(0) && markk->b == w->b)
		if (square) {
			from = markb->xcol;
			to = markk->xcol;
			dosquare = 1;
		} else {
			from = markb->byte;
			to = markk->byte;
		}
	else if (marking && w == (BW *)maint->curwin->object && markb && markb->b == w->b && w->cursor->byte != markb->byte && !from) {
		if (square) {
			from = off_min(w->cursor->xcol, markb->xcol);
			to = off_max(w->cursor->xcol, markb->xcol);
			dosquare = 1;
		} else {
			from = off_min(w->cursor->byte, markb->byte);
			to = off_max(w->cursor->byte, markb->byte);
		}
	}

	if (marking && w == (BW *)maint->curwin->object)
		msetI(t->updtab + w->y, 1, w->h);

	if (dosquare) {
		from = 0;
		to = 0;
	}

	y=w->y;
	attr = t->attr + y*w->t->w;
	for (screen = t->scrn + y * w->t->w; y != bot; ++y, (screen += w->t->w), (attr += w->t->w)) {
		char txt[80];
		int fmt[80];
		char bf[16];
		int x;
		memset(txt,' ',76);
		msetI(fmt,BG_COLOR(bg_text),76);
		txt[76]=0;
		if (w->o.hiline && (q->byte & ~15) == (w->cursor->byte & ~15)) {
			msetI(fmt,BG_COLOR(bg_curlinum),9);
		} else {
			msetI(fmt,BG_COLOR(bg_linum),9);
		}
		if (!flg) {
#if HAVE_LONG_LONG
			sprintf(bf,"%8llx ",(unsigned long long)q->byte);
#else
			sprintf(bf,"%8lx ",(unsigned long)q->byte);
#endif
			memcpy(txt,bf,9);
			for (x=0; x!=8; ++x) {
				int c;
				if (q->byte==w->cursor->byte && !flg) {
					fmt[10+x*3] = BG_COLOR(bg_cursor);
					fmt[10+x*3+1] = BG_COLOR(bg_cursor);
				}
				if (q->byte>=from && q->byte<to && !flg) {
					fmt[10+x*3] |= UNDERLINE;
					fmt[10+x*3+1] |= UNDERLINE;
					fmt[60+x] |= INVERSE;
				}
				c = pgetb(q);
				if (c != NO_MORE_DATA) {
					sprintf(bf,"%2.2x",c);
					txt[10+x*3] = bf[0];
					txt[10+x*3+1] = bf[1];
					if (c >= 0x20 && c <= 0x7E)
						txt[60+x] = TO_CHAR_OK(c);
					else
						txt[60+x] = '.';
				} else
					flg = 1;
			}
			for (x=8; x!=16; ++x) {
				int c;
				if (q->byte==w->cursor->byte && !flg) {
					fmt[11+x*3] = BG_COLOR(bg_cursor);
					fmt[11+x*3+1] = BG_COLOR(bg_cursor);
				}
				if (q->byte>=from && q->byte<to && !flg) {
					fmt[11+x*3] |= UNDERLINE;
					fmt[11+x*3+1] |= UNDERLINE;
					fmt[60+x] |= INVERSE;
				}
				c = pgetb(q);
				if (c != NO_MORE_DATA) {
					sprintf(bf,"%2.2x",c);
					txt[11+x*3] = bf[0];
					txt[11+x*3+1] = bf[1];
					if (c >= 0x20 && c <= 0x7E)
						txt[60+x] = TO_CHAR_OK(c);
					else
						txt[60+x] = '.';
				} else
					flg = 1;
			}
		}
		genfield(t, screen, attr, 0, y, TO_DIFF_OK(w->offset), txt, 76, BG_COLOR(bg_text), w->w, 1, fmt);
	}
	prm(q);
}

void bwgen(BW *w, int linums, int linchg)
{
	int (*screen)[COMPOSE];
	int *attr;
	P *p = NULL;
	P *q;
	ptrdiff_t bot = w->h + w->y;
	ptrdiff_t y;
	int dosquare = 0;
	off_t from, to;
	off_t fromline, toline;
	SCRN *t = w->t->t;

	/* Set w.db to correct value */
	if (w->o.highlight && w->o.syntax && (!w->db || w->db->syn != w->o.syntax))
		w->db = find_lattr_db(w->b, w->o.syntax);

	fromline = toline = from = to = 0;

	if (w->b == errbuf && w->b->err) {
		P *tmp = pdup(w->b->err, "bwgen");
		p_goto_bol(tmp);
		from = tmp->byte;
		pnextl(tmp);
		to = tmp->byte;
		prm(tmp);
	} else if (markv(0) && markk->b == w->b)
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
	else if (marking && w == (BW *)maint->curwin->object && markb && markb->b == w->b && w->cursor->byte != markb->byte && !from) {
		if (square) {
			from = off_min(w->cursor->xcol, markb->xcol);
			to = off_max(w->cursor->xcol, markb->xcol);
			fromline = off_min(w->cursor->line, markb->line);
			toline = off_max(w->cursor->line, markb->line);
			dosquare = 1;
		} else {
			from = off_min(w->cursor->byte, markb->byte);
			to = off_max(w->cursor->byte, markb->byte);
		}
	}

	if (marking && w == (BW *)maint->curwin->object)
		msetI(t->updtab + w->y, 1, w->h);

	q = pdup(w->cursor, "bwgen");

	y = TO_DIFF_OK(w->cursor->line - w->top->line) + w->y;
	attr = t->attr + y*w->t->w;
	for (screen = t->scrn + y * w->t->w; y != bot; ++y, (screen += w->t->w), (attr += w->t->w)) {
		if (ifhave)
			break;
		if (linums)
			gennum(w, screen, attr, t, y, t->compose);
		if (linchg || t->updtab[y]) {
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
					t->updtab[y] = lgen(t, y, screen, attr, w->x, w->x + w->w, p, w->offset, from, to, get_highlight_state(w,p,w->top->line+y-w->y),w);
				else
					t->updtab[y] = lgen(t, y, screen, attr, w->x, w->x + w->w, p, w->offset, 0L, 0L, get_highlight_state(w,p,w->top->line+y-w->y),w);
			else
				t->updtab[y] = lgen(t, y, screen, attr, w->x, w->x + w->w, p, w->offset, from, to, get_highlight_state(w,p,w->top->line+y-w->y),w);
		}
	}

	y = w->y;
	attr = t->attr + w->y * w->t->w;
	for (screen = t->scrn + w->y * w->t->w; y != w->y + w->cursor->line - w->top->line; ++y, (screen += w->t->w), (attr += w->t->w)) {
		if (ifhave)
			break;
		if (linums)
			gennum(w, screen, attr, t, y, t->compose);
		if (linchg || t->updtab[y]) {
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
					t->updtab[y] = lgen(t, y, screen, attr, w->x, w->x + w->w, p, w->offset, from, to, get_highlight_state(w,p,w->top->line+y-w->y),w);
				else
					t->updtab[y] = lgen(t, y, screen, attr, w->x, w->x + w->w, p, w->offset, 0L, 0L, get_highlight_state(w,p,w->top->line+y-w->y),w);
			else
				t->updtab[y] = lgen(t, y, screen, attr, w->x, w->x + w->w, p, w->offset, from, to, get_highlight_state(w,p,w->top->line+y-w->y),w);
		}
	}
	prm(q);
	if (p)
		prm(p);
}

void bwmove(BW *w, ptrdiff_t x, ptrdiff_t y)
{
	w->x = x;
	w->y = y;
}

void bwresz(BW *w, ptrdiff_t wi, ptrdiff_t he)
{
	if (he > w->h && w->y != -1) {
		msetI(w->t->t->updtab + w->y + w->h, 1, he - w->h);
	}
	w->w = wi;
	w->h = he;
	if (w->b->vt && w->b->pid && w == vtmaster(w->parent->t, w->b)) {
		vt_resize(w->b->vt, w->top, he, wi);
		ttstsz(w->b->out, wi, he);
	}
}

BW *bwmk(W *window, B *b, int prompt)
{
	BW *w = (BW *) joe_malloc(SIZEOF(BW));

	w->parent = window;
	w->b = b;
	if (prompt || (!window->y && staen) || window->h < 2) {
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
		w->top = pdup(b->bof, "bwmk");
		w->cursor = pdup(b->bof, "bwmk");
	}
	w->t = window->t;
	w->object = NULL;
	w->offset = 0;
	w->o = w->b->o;
	w->lincols = 0;
	w->curlin = 0;
	w->x = window->x;
	w->w = window->w;
	if (window == window->main) {
		rmkbd(window->kbd);
		window->kbd = mkkbd(kmap_getcontext(w->o.context));
	}
	w->top->xcol = 0;
	w->cursor->xcol = 0;
	w->top_changed = 1;
	w->db = 0;
	w->shell_flag = 0;
	return w;
}

/* Database of last file positions */

#define MAX_FILE_POS 20 /* Maximum number of file positions we track */

static struct file_pos {
	LINK(struct file_pos) link;
	char *name;
	off_t line;
} file_pos = { { &file_pos, &file_pos } };

static int file_pos_count;

static struct file_pos *find_file_pos(const char *name)
{
	struct file_pos *p;
	for (p = file_pos.link.next; p != &file_pos; p = p->link.next)
		if (!zcmp(p->name, name)) {
			promote(struct file_pos,link,&file_pos,p);
			return p;
		}
	p = (struct file_pos *)malloc(SIZEOF(struct file_pos));
	p->name = zdup(name);
	p->line = 0;
	enquef(struct file_pos,link,&file_pos,p);
	if (++file_pos_count == MAX_FILE_POS) {
		free(deque_f(struct file_pos,link,file_pos.link.prev));
		--file_pos_count;
	}
	return p;
}

int restore_file_pos;

off_t get_file_pos(const char *name)
{
	if (name && restore_file_pos) {
		struct file_pos *p = find_file_pos(name);
		return p->line;
	} else {
		return 0;
	}
}

void set_file_pos(const char *name, off_t pos)
{
	if (name) {
		struct file_pos *p = find_file_pos(name);
		p->line = pos;
	}
}

void save_file_pos(FILE *f)
{
	struct file_pos *p;
	for (p = file_pos.link.prev; p != &file_pos; p = p->link.prev) {
#ifdef HAVE_LONG_LONG
		fprintf(f,"	%lld ",(long long)p->line);
#else
		fprintf(f,"	%ld ",(long)p->line);
#endif
		emit_string(f,p->name,zlen(p->name));
		fprintf(f,"\n");
	}
	fprintf(f,"done\n");
}

void load_file_pos(FILE *f)
{
	char buf[1024];
	while (fgets(buf,SIZEOF(buf)-1,f) && zcmp(buf,"done\n")) {
		const char *p = buf;
		off_t pos;
		char name[1024];
		parse_ws(&p,'#');
		if (!parse_off_t(&p, &pos)) {
			parse_ws(&p, '#');
			if (parse_string(&p, name, SIZEOF(name)) > 0) {
				set_file_pos(name, pos);
			}
		}
	}
}

/* Save file position for all windows */

void set_file_pos_all(Screen *t)
{
	/* Step through all windows */
	W *w = t->topwin;
	do {
		if (w->watom == &watomtw) {
			BW *bw = (BW *)w->object;
			set_file_pos(bw->b->name, bw->cursor->line);
		}
		w = w->link.next;
	} while(w != t->topwin);
	/* Set through orphaned buffers */
	set_file_pos_orphaned();
}

/* Return master BW for a B.  It's the last window on the screen with the B.  If the B has a VT, then
 * it's the last window on the screen with the B and where the cursor matches the VT cursor. */

BW *vtmaster(Screen *t, B *b)
{
	W *w = t->topwin;
	BW *m = 0;
	do {
		if (w->watom == &watomtw) {
			BW *bw = (BW *)w->object;
			if (bw && w->y != -1 && bw->b == b && (!b->vt || b->vt->vtcur->byte == bw->cursor->byte))
				m = bw;
		}
		w = w->link.next;
	} while (w != t->topwin);
	return m;
}

void bwrm(BW *w)
{
	if (w->b == errbuf && w->b->count == 1) {
		/* Do not lose message buffer */
		orphit(w);
	}
	set_file_pos(w->b->name,w->cursor->line);
	prm(w->top);
	prm(w->cursor);
	brm(w->b);
	joe_free(w);
}

char *ustat_line;

int ustat(W *w, int k)
{
	BW *bw;
	int c;
	const char *msg;
	WIND_BW(bw, w);
	c = brch(bw->cursor);

	if (c == NO_MORE_DATA) {
		if (bw->o.zmsg) msg = bw->o.zmsg;
		else msg = "** Line %r Col %c Offset %o(0x%O) **";
	} else {
		if (bw->o.smsg) msg = bw->o.smsg;
		else msg = "** Line %r Col %c Offset %o(0x%O) %e %a(0x%A) Width %w **";
	}

	ustat_line = stagen(ustat_line, bw, msg, (char)(zlen(msg) ? msg[zlen(msg) - 1] : ' '));
	msgnw(bw->parent, ustat_line);

	return 0;
}

int ucrawlr(W *w, int k)
{
	BW *bw;
	ptrdiff_t amnt;
	WIND_BW(bw, w);

	if (opt_right < 0)
		amnt = bw->w / (-opt_right);
	else
		amnt = opt_right;

	if (amnt > bw->w)
		amnt = bw->w;
	if (amnt <= 0)
		amnt = 1;

	/* amnt = bw->w / 2; */

	pcol(bw->cursor, bw->cursor->xcol + amnt);
	bw->cursor->xcol += amnt;
	bw->offset += amnt;
	updall();
	return 0;
}

int ucrawll(W *w, int k)
{
	BW *bw;
	off_t amnt;
	WIND_BW(bw, w);
	int rtn = -1;

	if (opt_left < 0)
		amnt = bw->w / (-opt_left);
	else
		amnt = opt_left;

	if (amnt > bw->w)
		amnt = bw->w;

	if (amnt < 1)
		amnt = 1;

	if (amnt > bw->cursor->xcol) {
		if (bw->cursor->xcol)
			rtn = 0;
		bw->cursor->xcol = 0;
	} else {
		bw->cursor->xcol -= amnt;
		rtn = 0;
	}

	if (amnt > bw->offset) {
		if (bw->offset)
			rtn = 0;
		bw->offset = 0;
	} else {
		bw->offset -= amnt;
		rtn = 0;
	}

	if (rtn)
		return rtn;
	pcol(bw->cursor, bw->cursor->xcol);
	updall();
	return rtn;
}

/* If we are about to call bwrm, and b->count is 1, and orphan mode
 * is set, call this. */

void orphit(BW *bw)
{
	++bw->b->count; /* Assumes bwrm() is about to be called */
	bw->b->orphan = 1;
	pdupown(bw->cursor, &bw->b->oldcur, "orphit");
	pdupown(bw->top, &bw->b->oldtop, "orphit");
}

/* Calculate the width of the line number gutter for the Window */

int calclincols(BW *bw)
{
	int width = 0;
	off_t lines = bw->b->eof->line + 1;

	if (!bw->o.linums) {
		return 0;
	}

	if (lines < 10) {
		width = 1;
	} else if (lines < 100) {
		width = 2;
	} else if (lines < 1000) {
		width = 3;
	} else if (lines < 10000) {
		width = 4;
	} else {
		off_t l;
		for (l = 10000, width = 4; lines >= l; l *= 10, width++) {}
	}

	return width + 2;
}
