/*
 *	Basic user edit functions
 *	Copyright
 * 		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <stdio.h>
#include <ctype.h>

#include "b.h"
#include "bw.h"
#include "macro.h"
#include "main.h"
#include "pw.h"
#include "qw.h"
#include "scrn.h"
#include "ublock.h"
#include "uformat.h"
#include "umath.h"
#include "utils.h"
#include "vs.h"
#include "utf8.h"
#include "w.h"

/***************/
/* Global options */
int pgamnt = -1;		/* No. of PgUp/PgDn lines to keep */

/******** i don't like global var ******/

/* 
 * Move cursor to beginning of line
 */
int u_goto_bol(BW *bw)
{
	p_goto_bol(bw->cursor);
	return 0;
}

/*
 * Move cursor to first non-whitespace character, unless it is
 * already there, in which case move it to beginning of line
 */
int uhome(BW *bw)
{
	P *p = pdup(bw->cursor);

	if ((bw->o.smarthome) && (piscol(p) > pisindent(p))) { 
		p_goto_bol(p);
		while (joe_isblank(brc(p)))
			pgetc(p);
	} else
		p_goto_bol(p);
	pset(bw->cursor, p);
	prm(p);
	return 0;
}

/*
 * Move cursor to end of line
 */
int u_goto_eol(BW *bw)
{
	p_goto_eol(bw->cursor);
	return 0;
}

/*
 * Move cursor to beginning of file
 */
int u_goto_bof(BW *bw)
{
	p_goto_bof(bw->cursor);
	return 0;
}

/*
 * Move cursor to end of file
 */
int u_goto_eof(BW *bw)
{
	p_goto_eof(bw->cursor);
	return 0;
}

/*
 * Move cursor left
 */
int u_goto_left(BW *bw)
{
	if (prgetc(bw->cursor) != NO_MORE_DATA)
		return 0;
	else
		return -1;
}

/*
 * Move cursor right
 */
int u_goto_right(BW *bw)
{
	if (pgetc(bw->cursor) != NO_MORE_DATA)
		return 0;
	else
		return -1;
}

/*
 * Move cursor to beginning of previous word or if there isn't 
 * previous word then go to beginning of the file
 *
 * WORD is a sequence non-white-space characters
 */
int u_goto_prev(BW *bw)
{
	P *p = pdup(bw->cursor);
	int c = prgetc(p);

	if (isalnum_(bw->b->o.utf8,c)) {
		while (isalnum_(bw->b->o.utf8,(c=prgetc(p))))
			/* Do nothing */;
		if (c != NO_MORE_DATA)
			pgetc(p);
	} else if (isspace(c) || ispunct(c)) {
		while ((c=prgetc(p)), (isspace(c) || ispunct(c)))
			/* Do nothing */;
		while(isalnum_(bw->b->o.utf8,(c=prgetc(p))))
			/* Do nothing */;
		if (c != NO_MORE_DATA)
			pgetc(p);
	}
	if (p->byte == bw->cursor->byte) {
		prm(p);
		return -1;
	}
	pset(bw->cursor, p);
	prm(p);
	return 0;
}

/*
 * Move cursor to end of next word or if there isn't 
 * next word then go to end of the file
 *
 * WORD is a sequence non-white-space characters
 */
int u_goto_next(BW *bw)
{
	P *p = pdup(bw->cursor);
	int c = brch(p);

	if (isalnum_(bw->b->o.utf8,c))
		while (isalnum_(bw->b->o.utf8,(c = brch(p))))
			pgetc(p);
	else if (isspace(c) || ispunct(c)) {
		while (isspace(c = brch(p)) || ispunct(c))
			pgetc(p);
		while (isalnum_(bw->b->o.utf8,(c = brch(p))))
			pgetc(p);
	} else
		pgetc(p);
	if (p->byte == bw->cursor->byte) {
		prm(p);
		return -1;
	}
	pset(bw->cursor, p);
	prm(p);
	return 0;
}

static P *pboi(P *p)
{
	p_goto_bol(p);
	while (joe_isblank(brch(p)))
		pgetc(p);
	return p;
}

static int pisedge(P *p)
{
	P *q;
	int c;

	if (pisbol(p))
		return -1;
	if (piseol(p))
		return 1;
	q = pdup(p);
	pboi(q);
	if (q->byte == p->byte)
		goto left;
	if (joe_isblank(c = brch(p))) {
		pset(q, p);
		if (joe_isblank(prgetc(q)))
			goto no;
		if (c == '\t')
			goto right;
		pset(q, p);
		pgetc(q);
		if (pgetc(q) == ' ')
			goto right;
		goto no;
	} else {
		pset(q, p);
		c = prgetc(q);
		if (c == '\t')
			goto left;
		if (c != ' ')
			goto no;
		if (prgetc(q) == ' ')
			goto left;
		goto no;
	}

      right:prm(q);
	return 1;
      left:prm(q);
	return -1;
      no:prm(q);
	return 0;
}

int upedge(BW *bw)
{
	if (prgetc(bw->cursor) == NO_MORE_DATA)
		return -1;
	while (pisedge(bw->cursor) != -1)
		prgetc(bw->cursor);
	return 0;
}

int unedge(BW *bw)
{
	if (pgetc(bw->cursor) == NO_MORE_DATA)
		return -1;
	while (pisedge(bw->cursor) != 1)
		pgetc(bw->cursor);
	return 0;
}

/* Move cursor to matching delimiter */

int utomatch(BW *bw)
{
	int d;
	int c,			/* Character under cursor */
	 f,			/* Character to find */
	 dir;			/* 1 to search forward, -1 to search backward */

	switch (c = brch(bw->cursor)) {
	case '(':
		f = ')';
		dir = 1;
		break;
	case '[':
		f = ']';
		dir = 1;
		break;
	case '{':
		f = '}';
		dir = 1;
		break;
	case '`':
		f = '\'';
		dir = 1;
		break;
	case '<':
		f = '>';
		dir = 1;
		break;
	case ')':
		f = '(';
		dir = -1;
		break;
	case ']':
		f = '[';
		dir = -1;
		break;
	case '}':
		f = '{';
		dir = -1;
		break;
	case '\'':
		f = '`';
		dir = -1;
		break;
	case '>':
		f = '<';
		dir = -1;
		break;
	default:
		return -1;
	}

	if (dir == 1) {
		P *p = pdup(bw->cursor);
		int cnt = 0;	/* No. levels of delimiters we're in */

		while ((d = pgetc(p)) != NO_MORE_DATA) {
			if (d == c)
				++cnt;
			else if (d == f && !--cnt) {
				prgetc(p);
				pset(bw->cursor, p);
				break;
			}
		}
		prm(p);
	} else {
		P *p = pdup(bw->cursor);
		int cnt = 0;	/* No. levels of delimiters we're in */

		while ((d = prgetc(p)) != NO_MORE_DATA) {
			if (d == c)
				++cnt;
			else if (d == f)
				if (!cnt--) {
					pset(bw->cursor, p);
					break;
				}
		}
		prm(p);
	}
	if (d == NO_MORE_DATA)
		return -1;
	else
		return 0;
}

/* Move cursor up */

int uuparw(BW *bw)
{
	if (bw->cursor->line) {
		pprevl(bw->cursor);
		pcol(bw->cursor, bw->cursor->xcol);
		return 0;
	} else
		return -1;
}

/* Move cursor down */

int udnarw(BW *bw)
{
	if (bw->cursor->line != bw->b->eof->line) {
		pnextl(bw->cursor);
		pcol(bw->cursor, bw->cursor->xcol);
		return 0;
	} else
		return -1;
}

/* Move cursor to top of window */

int utos(BW *bw)
{
	long col = bw->cursor->xcol;

	pset(bw->cursor, bw->top);
	pcol(bw->cursor, col);
	bw->cursor->xcol = col;
	return 0;
}

/* Move cursor to bottom of window */

int ubos(BW *bw)
{
	long col = bw->cursor->xcol;

	pline(bw->cursor, bw->top->line + bw->h - 1);
	pcol(bw->cursor, col);
	bw->cursor->xcol = col;
	return 0;
}

/* Scroll buffer window up n lines
 * If beginning of file is close, scrolls as much as it can
 * If beginning of file is on-screen, cursor jumps to beginning of file
 *
 * If flg is set: cursor stays fixed relative to screen edge
 * If flg is clr: cursor stays fixed on the buffer line
 */

void scrup(BW *bw, int n, int flg)
{
	int scrollamnt = 0;
	int cursoramnt = 0;
	int x;

	/* Decide number of lines we're really going to scroll */

	if (bw->top->line >= n)
		scrollamnt = cursoramnt = n;
	else if (bw->top->line)
		scrollamnt = cursoramnt = bw->top->line;
	else if (flg)
		cursoramnt = bw->cursor->line;
	else if (bw->cursor->line >= n)
		cursoramnt = n;

	/* Move top-of-window pointer */
	for (x = 0; x != scrollamnt; ++x)
		pprevl(bw->top);
	p_goto_bol(bw->top);

	/* Move cursor */
	for (x = 0; x != cursoramnt; ++x)
		pprevl(bw->cursor);
	p_goto_bol(bw->cursor);
	pcol(bw->cursor, bw->cursor->xcol);

	/* If window is on the screen, give (buffered) scrolling command */
	if (bw->parent->y != -1)
		nscrldn(bw->parent->t->t, bw->y, bw->y + bw->h, scrollamnt);
}

/* Scroll buffer window down n lines
 * If end of file is close, scrolls as much as possible
 * If end of file is on-screen, cursor jumps to end of file
 *
 * If flg is set: cursor stays fixed relative to screen edge
 * If flg is clr: cursor stays fixed on the buffer line
 */

void scrdn(BW *bw, int n, int flg)
{
	int scrollamnt = 0;
	int cursoramnt = 0;
	int x;

	/* How much we're really going to scroll... */
	if (bw->top->b->eof->line < bw->top->line + bw->h) {
		cursoramnt = bw->top->b->eof->line - bw->cursor->line;
		if (!flg && cursoramnt > n)
			cursoramnt = n;
	} else if (bw->top->b->eof->line - (bw->top->line + bw->h) >= n)
		cursoramnt = scrollamnt = n;
	else
		cursoramnt = scrollamnt = bw->top->b->eof->line - (bw->top->line + bw->h) + 1;

	/* Move top-of-window pointer */
	for (x = 0; x != scrollamnt; ++x)
		pnextl(bw->top);

	/* Move cursor */
	for (x = 0; x != cursoramnt; ++x)
		pnextl(bw->cursor);
	pcol(bw->cursor, bw->cursor->xcol);

	/* If window is on screen, give (buffered) scrolling command to terminal */
	if (bw->parent->y != -1)
		nscrlup(bw->parent->t->t, bw->y, bw->y + bw->h, scrollamnt);
}

/* Page up */

int upgup(BW *bw)
{
	bw = (BW *) bw->parent->main->object;
	if (!bw->cursor->line)
		return -1;
	if (pgamnt < 0)
		scrup(bw, bw->h / 2 + bw->h % 2, 1);
	else if (pgamnt < bw->h)
		scrup(bw, bw->h - pgamnt, 1);
	else
		scrup(bw, 1, 1);
	return 0;
}

/* Page down */

int upgdn(BW *bw)
{
	bw = (BW *) bw->parent->main->object;
	if (bw->cursor->line == bw->b->eof->line)
		return -1;
	if (pgamnt < 0)
		scrdn(bw, bw->h / 2 + bw->h % 2, 1);
	else if (pgamnt < bw->h)
		scrdn(bw, bw->h - pgamnt, 1);
	else
		scrdn(bw, 1, 1);
	return 0;
}

/* Scroll by a single line.  The cursor moves with the scroll */

int uupslide(BW *bw)
{
	bw = (BW *) bw->parent->main->object;
	if (bw->top->line) {
		if (bw->top->line + bw->h - 1 != bw->cursor->line)
			udnarw(bw);
		scrup(bw, 1, 0);
		return 0;
	} else
		return -1;
}

int udnslide(BW *bw)
{
	bw = (BW *) bw->parent->main->object;
	if (bw->top->line + bw->h <= bw->top->b->eof->line) {
		if (bw->top->line != bw->cursor->line)
			uuparw(bw);
		scrdn(bw, 1, 0);
		return 0;
	} else
		return -1;
}

/* Move cursor to specified line number */

static B *linehist = NULL;	/* History of previously entered line numbers */

static int doline(BW *bw, unsigned char *s, void *object, int *notify)
{
	long num = calc(bw, s);

	if (notify)
		*notify = 1;
	vsrm(s);
	if (num >= 1 && !merr) {
		int tmp = mid;

		if (num > bw->b->eof->line)
			num = bw->b->eof->line + 1;
		pline(bw->cursor, num - 1), bw->cursor->xcol = piscol(bw->cursor);
		mid = 1;
		dofollows();
		mid = tmp;
		return 0;
	} else {
		if (merr)
			msgnw(bw->parent, merr);
		else
			msgnw(bw->parent, US "Invalid line number");
		return -1;
	}
}

int uline(BW *bw)
{
	if (wmkpw(bw->parent, US "Go to line (^C to abort): ", &linehist, doline, NULL, NULL, NULL, NULL, NULL, -1))
		return 0;
	else
		return -1;
}

/* Move cursor to specified column number */

static B *colhist = NULL;	/* History of previously entered column numbers */

static int docol(BW *bw, unsigned char *s, void *object, int *notify)
{
	long num = calc(bw, s);

	if (notify)
		*notify = 1;
	vsrm(s);
	if (num >= 1 && !merr) {
		int tmp = mid;

		pcol(bw->cursor, num - 1), bw->cursor->xcol = piscol(bw->cursor);
		mid = 1;
		dofollows();
		mid = tmp;
		return 0;
	} else {
		if (merr)
			msgnw(bw->parent, merr);
		else
			msgnw(bw->parent, US "Invalid column number");
		return -1;
	}
}

int ucol(BW *bw)
{
	if (wmkpw(bw->parent, US "Go to column (^C to abort): ", &colhist, docol, NULL, NULL, NULL, NULL, NULL, -1))
		return 0;
	else
		return -1;
}

/* Move cursor to specified byte number */

static B *bytehist = NULL;	/* History of previously entered byte numbers */

static int dobyte(BW *bw, unsigned char *s, void *object, int *notify)
{
	long num = calc(bw, s);

	if (notify)
		*notify = 1;
	vsrm(s);
	if (num >= 0 && !merr) {
		int tmp = mid;

		pgoto(bw->cursor, num), bw->cursor->xcol = piscol(bw->cursor);
		mid = 1;
		dofollows();
		mid = tmp;
		return 0;
	} else {
		if (merr)
			msgnw(bw->parent, merr);
		else
			msgnw(bw->parent, US "Invalid byte number");
		return -1;
	}
}

int ubyte(BW *bw)
{
	if (wmkpw(bw->parent, US "Go to byte (^C to abort): ", &bytehist, dobyte, NULL, NULL, NULL, NULL, NULL, -1))
		return 0;
	else
		return -1;
}

/* Delete character under cursor
 * or write ^D to process if we're at end of file in a shell window
 */

int udelch(BW *bw)
{
	if (bw->pid && bw->cursor->byte == bw->b->eof->byte) {
		unsigned char c = 4;
		joe_write(bw->out, &c, 1);	/* Send Ctrl-D to process */
	} else {
		P *p;

		if (piseof(bw->cursor))
			return -1;
		pgetc(p = pdup(bw->cursor));
		bdel(bw->cursor, p);
		prm(p);
	}
	return 0;
}

/* Backspace, or if cursor is at end of file in a shell window, send backspace
 * to shell */

int ubacks(BW *bw, int k)
{
	if (bw->pid && bw->cursor->byte == bw->b->eof->byte) {
		unsigned char c = k;

		joe_write(bw->out, &c, 1);
	} else if (bw->parent->watom->what == TYPETW || !pisbol(bw->cursor)) {
		P *p;
		int c;
		int indent;
		int col;
		int indwid;
		int wid;
		int pure = 1;

		if (pisbof(bw->cursor))
			return -1;

		/* Indentation point of this line */
		indent = pisindent(bw->cursor);

		/* Column position of cursor */
		col = piscol(bw->cursor);

		/* Indentation step in columns */
		if (bw->o.indentc=='\t')
			wid = bw->o.tab;
		else
			wid = 1;

		indwid = (bw->o.istep*wid);

		/* Smart backspace when: cursor is at indentation point, indentation point
		   is a multiple of indentation width, we're not at beginning of line,
		   'smarthome' option is enabled, and indentation is purely made out of
		   indent characters. */
		if (col == indent && (col%indwid)==0 && col!=0 && bw->o.smarthome && pispure(bw->cursor,bw->o.indentc)) {
			P *p;
			int x;

			/* Delete all indentation */
			p = pdup(bw->cursor);
			p_goto_bol(p);
			bdel(p,bw->cursor);
			prm(p);

			/* Indent to new position */
			col -= indwid;

			for (x=0; x!=col; x+=wid) {
				binsc(bw->cursor, bw->o.indentc);
				pgetc(bw->cursor);
			}
		} else {
			P *p = pdup(bw->cursor);
			if ((c = prgetc(bw->cursor)) != NO_MORE_DATA)
				if (!bw->o.overtype || c == '\t' || pisbol(p) || piseol(p))
					bdel(bw->cursor, p);
			prm(p);
		}
	} else
		return -1;
	return 0;
}

/* 
 * Delete sequence of characters (alphabetic, numeric) or (white-space)
 *	if cursor is on the white-space it will delete all white-spaces
 *		until alphanumeric character
 *      if cursor is on the alphanumeric it will delete all alphanumeric
 *		characters until character that is not alphanumeric
 */
int u_word_delete(BW *bw)
{
	P *p = pdup(bw->cursor);
	int c = brch(p);

	if (isalnum_(bw->b->o.utf8,c))
		while (isalnum_(bw->b->o.utf8,(c = brch(p))))
			pgetc(p);
	else if (isspace(c))
		while (isspace(c = brch(p)))
			pgetc(p);
	else
		pgetc(p);

	if (p->byte == bw->cursor->byte) {
		prm(p);
		return -1;
	}
	bdel(bw->cursor, p);
	prm(p);
	return 0;
}

/* Delete from cursor to beginning of word it's in or immediately after,
 * to start of whitespace, or a single character
 */

int ubackw(BW *bw)
{
	P *p = pdup(bw->cursor);
	int c = prgetc(bw->cursor);

	if (isalnum_(bw->b->o.utf8,c)) {
		while (isalnum_(bw->b->o.utf8,(c = prgetc(bw->cursor))))
			/* do nothing */;
		if (c != NO_MORE_DATA)
			pgetc(bw->cursor);
	} else if (isspace(c)) {
		while (isspace(c = prgetc(bw->cursor)))
			/* do nothing */;
		if (c != NO_MORE_DATA)
			pgetc(bw->cursor);
	}
	if (bw->cursor->byte == p->byte) {
		prm(p);
		return -1;
	}
	bdel(bw->cursor, p);
	prm(p);
	return 0;
}

/* Delete from cursor to end of line, or if there's nothing to delete,
 * delete the line-break
 */

int udelel(BW *bw)
{
	P *p = p_goto_eol(pdup(bw->cursor));

	if (bw->cursor->byte == p->byte) {
		prm(p);
		return udelch(bw);
	} else
		bdel(bw->cursor, p);
	prm(p);
	return 0;
}

/* Delete to beginning of line, or if there's nothing to delete,
 * delete the line-break
 */

int udelbl(BW *bw)
{
	P *p = p_goto_bol(pdup(bw->cursor));

	if (p->byte == bw->cursor->byte) {
		prm(p);
		return ubacks(bw, 8);	/* The 8 goes to the process if we're at EOF of shell window */
	} else
		bdel(p, bw->cursor);
	prm(p);
	return 0;
}

/* Delete entire line */

int udelln(BW *bw)
{
	P *p = pdup(bw->cursor);

	p_goto_bol(bw->cursor);
	pnextl(p);
	if (bw->cursor->byte == p->byte) {
		prm(p);
		return -1;
	}
	bdel(bw->cursor, p);
	prm(p);
	return 0;
}

/* Insert a space */

int uinsc(BW *bw)
{
	binsc(bw->cursor, ' ');
	return 0;
}

/* Type a character into the buffer (deal with left margin, overtype mode and
 * word-wrap), if cursor is at end of shell window buffer, just send character
 * to process.
 */

struct utf8_sm utype_utf8_sm;
extern int utf8;

int utypebw(BW *bw, int k)
{
	if (bw->pid && bw->cursor->byte == bw->b->eof->byte) {
		unsigned char c = k;

		utype_utf8_sm.state = 0;
		utype_utf8_sm.ptr = 0;

		joe_write(bw->out, &c, 1);
	} else if (k == '\t' && bw->o.spaces) {
		long n = piscol(bw->cursor);

		utype_utf8_sm.state = 0;
		utype_utf8_sm.ptr = 0;

		n = bw->o.tab - n % bw->o.tab;
		while (n--)
			utypebw(bw, ' ');
	} else {
		int upd;
		int simple;
		int x;

		/* UTF8 decoder */
		if(utf8) {
			int utf8_char = utf8_decode(&utype_utf8_sm,k);

			if(utf8_char >= 0)
				k = utf8_char;
			else
				return 0;
		}

		upd = bw->parent->t->t->updtab[bw->y + bw->cursor->line - bw->top->line];
		simple = 1;

		if (pisblank(bw->cursor))
			while (piscol(bw->cursor) < bw->o.lmargin) {
				binsc(bw->cursor, ' ');
				pgetc(bw->cursor);
			}

		if(utf8 && !bw->b->o.utf8) {
			unsigned char buf[10];
			utf8_encode(buf,k);
			k = from_utf8(buf);
		} else if(!utf8 && bw->b->o.utf8) {
			unsigned char buf[10];
			to_utf8(buf,k);
			k = utf8_decode_string(buf);
		}
		
		binsc(bw->cursor, k);

		/* We need x position before we move cursor */
		x = piscol(bw->cursor) - bw->offset;
		pgetc(bw->cursor);
		if (bw->o.wordwrap && piscol(bw->cursor) > bw->o.rmargin && !joe_isblank(k)) {
			wrapword(bw->cursor, (long) bw->o.lmargin, bw->o.french, NULL);
			simple = 0;
		} else if (bw->o.overtype && !piseol(bw->cursor) && k != '\t')
			udelch(bw);
		bw->cursor->xcol = piscol(bw->cursor);
#ifndef __MSDOS__
		if (x < 0 || x >= bw->w)
			simple = 0;
		if (bw->cursor->line < bw->top->line || bw->cursor->line >= bw->top->line + bw->h)
			simple = 0;
		if (simple && bw->parent->t->t->sary[bw->y + bw->cursor->line - bw->top->line])
			simple = 0;
		if (simple && k != '\t' && k != '\n' && !curmacro) {
			int a;
			int atr = 0;
			unsigned char c = k;
			SCRN *t = bw->parent->t->t;
			int y = bw->y + bw->cursor->line - bw->top->line;
			int *screen = t->scrn + y * t->co;
			int *attr = t->attr + y * t->co;
			x += bw->x;

			if (!upd && piseol(bw->cursor) && !bw->o.highlight)
				t->updtab[y] = 0;
			if (markb &&
			    markk &&
			    markb->b == bw->b &&
			    markk->b == bw->b &&
			   ((!square && bw->cursor->byte >= markb->byte && bw->cursor->byte < markk->byte) ||
			    ( square && bw->cursor->line >= markb->line && bw->cursor->line <= markk->line && piscol(bw->cursor) >= markb->xcol && piscol(bw->cursor) < markk->xcol)))
				atr = INVERSE;
			if (bw->b->o.utf8) {
				if (k<32 || k>126 && k<160) {
					xlat_utf_ctrl(&a, &c);
					k = c;
					atr ^= a;
					}
			} else {
				xlat(&a, &c);
				k = c;
				atr ^= a;
			}
			outatr(bw->b->o.utf8, t, screen + x, attr + x, x, y, k, atr);
		}
#endif
	}
	return 0;
}

/* Quoting */

static B *unicodehist = NULL;	/* History of previously entered unicode characters */

static int dounicode(BW *bw, unsigned char *s, void *object, int *notify)
{
	int num;
	unsigned char buf[8];
	int x;
	sscanf(s,"%x",&num);
	if (notify)
		*notify = 1;
	vsrm(s);
	utf8_encode(buf,num);
	for(x=0;buf[x];++x)
		utypebw(bw, buf[x]);
	bw->cursor->xcol = piscol(bw->cursor);
	return 0;
}

int quotestate;
int quoteval;

static int doquote(BW *bw, int c, void *object, int *notify)
{
	unsigned char buf[40];

	if (c < 0 || c >= 256) {
		nungetc(c);
		return -1;
	}
	switch (quotestate) {
	case 0:
		if (c >= '0' && c <= '9') {
			quoteval = c - '0';
			quotestate = 1;
			snprintf((char *)buf, sizeof(buf), "ASCII %c--", c);
			if (!mkqwna(bw->parent, sz(buf), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		} else if (c == 'x' || c == 'X') {
			if (bw->b->o.utf8) {
				if (!wmkpw(bw->parent, US "Unicode (ISO-10646) character in hex (^C to abort): ", &unicodehist, dounicode,
				           NULL, NULL, NULL, NULL, NULL, -1))
					return 0;
				else
					return -1;
			} else {
				quotestate = 3;
				if (!mkqwna(bw->parent, sc("ASCII 0x--"), doquote, NULL, NULL, notify))
					return -1;
				else
					return 0;
			}
		} else if (c == 'o' || c == 'O') {
			quotestate = 5;
			if (!mkqwna(bw->parent, sc("ASCII 0---"), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		} else {
			if ((c >= 0x40 && c <= 0x5F) || (c >= 'a' && c <= 'z'))
				c &= 0x1F;
			if (c == '?')
				c = 127;
			utypebw(bw, c);
			bw->cursor->xcol = piscol(bw->cursor);
		}
		break;
	case 1:
		if (c >= '0' && c <= '9') {
			snprintf((char *)buf, sizeof(buf), "ASCII %c%c-", quoteval + '0', c);
			quoteval = quoteval * 10 + c - '0';
			quotestate = 2;
			if (!mkqwna(bw->parent, sz(buf), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		}
		break;
	case 2:
		if (c >= '0' && c <= '9') {
			quoteval = quoteval * 10 + c - '0';
			utypebw(bw, quoteval);
			bw->cursor->xcol = piscol(bw->cursor);
		}
		break;
	case 3:
		if (c >= '0' && c <= '9') {
			snprintf((char *)buf, sizeof(buf), "ASCII 0x%c-", c);
			quoteval = c - '0';
			quotestate = 4;
			if (!mkqwna(bw->parent, sz(buf), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		} else if (c >= 'a' && c <= 'f') {
			snprintf((char *)buf, sizeof(buf), "ASCII 0x%c-", c + 'A' - 'a');
			quoteval = c - 'a' + 10;
			quotestate = 4;
			if (!mkqwna(bw->parent, sz(buf), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		} else if (c >= 'A' && c <= 'F') {
			snprintf((char *)buf, sizeof(buf), "ASCII 0x%c-", c);
			quoteval = c - 'A' + 10;
			quotestate = 4;
			if (!mkqwna(bw->parent, sz(buf), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		}
		break;
	case 4:
		if (c >= '0' && c <= '9') {
			quoteval = quoteval * 16 + c - '0';
			utypebw(bw, quoteval);
			bw->cursor->xcol = piscol(bw->cursor);
		} else if (c >= 'a' && c <= 'f') {
			quoteval = quoteval * 16 + c - 'a' + 10;
			utypebw(bw, quoteval);
			bw->cursor->xcol = piscol(bw->cursor);
		} else if (c >= 'A' && c <= 'F') {
			quoteval = quoteval * 16 + c - 'A' + 10;
			utypebw(bw, quoteval);
			bw->cursor->xcol = piscol(bw->cursor);
		}
		break;
	case 5:
		if (c >= '0' && c <= '7') {
			snprintf((char *)buf, sizeof(buf), "ASCII 0%c--", c);
			quoteval = c - '0';
			quotestate = 6;
			if (!mkqwna(bw->parent, sz(buf), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		}
		break;
	case 6:
		if (c >= '0' && c <= '7') {
			snprintf((char *)buf, sizeof(buf), "ASCII 0%c%c-", quoteval + '0', c);
			quoteval = quoteval * 8 + c - '0';
			quotestate = 7;
			if (!mkqwna(bw->parent, sz(buf), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		}
		break;
	case 7:
		if (c >= '0' && c <= '7') {
			quoteval = quoteval * 8 + c - '0';
			utypebw(bw, quoteval);
			bw->cursor->xcol = piscol(bw->cursor);
		}
		break;
	}
	if (notify)
		*notify = 1;
	return 0;
}

int uquote(BW *bw)
{
	quotestate = 0;
	if (mkqwna(bw->parent, sc("Ctrl- (or 0-9 for dec. ascii, x for hex, or o for octal)"), doquote, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

static int doquote9(BW *bw, int c, void *object, int *notify)
{
	if (notify)
		*notify = 1;
	if ((c >= 0x40 && c <= 0x5F) || (c >= 'a' && c <= 'z'))
		c &= 0x1F;
	if (c == '?')
		c = 127;
	c |= 128;
	utypebw(bw, c);
	bw->cursor->xcol = piscol(bw->cursor);
	return 0;
}

static int doquote8(BW *bw, int c, void *object, int *notify)
{
	if (c == '`') {
		if (mkqwna(bw->parent, sc("Meta-Ctrl-"), doquote9, NULL, NULL, notify))
			return 0;
		else
			return -1;
	}
	if (notify)
		*notify = 1;
	c |= 128;
	utypebw(bw, c);
	bw->cursor->xcol = piscol(bw->cursor);
	return 0;
}

int uquote8(BW *bw)
{
	if (mkqwna(bw->parent, sc("Meta-"), doquote8, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

extern unsigned char srchstr[];

static int doctrl(BW *bw, int c, void *object, int *notify)
{
	int org = bw->o.overtype;

	if (notify)
		*notify = 1;
	bw->o.overtype = 0;
	if (bw->parent->huh == srchstr && c == '\n') {
		utypebw(bw, '\\');
		utypebw(bw, 'n');
	} else
		utypebw(bw, c);
	bw->o.overtype = org;
	bw->cursor->xcol = piscol(bw->cursor);
	return 0;
}

int uctrl(BW *bw)
{
	if (mkqwna(bw->parent, sc("Quote"), doctrl, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

/* User hit Return.  Deal with autoindent.  If cursor is at end of shell
 * window buffer, send the return to the shell
 */

int rtntw(BW *bw)
{
	if (bw->pid && bw->cursor->byte == bw->b->eof->byte) {
		joe_write(bw->out, "\n", 1);
	} else {
		P *p = pdup(bw->cursor);
		unsigned char c;

		binsc(bw->cursor, '\n'), pgetc(bw->cursor);
		if (bw->o.autoindent) {
			p_goto_bol(p);
			while (isspace(c = pgetc(p)) && c != 10) {
				binsc(bw->cursor, c);
				pgetc(bw->cursor);
			}
		}
		prm(p);
		bw->cursor->xcol = piscol(bw->cursor);
	}
	return 0;
}

/* Open a line */

int uopen(BW *bw)
{
	P *q = pdup(bw->cursor);

	rtntw(bw);
	pset(bw->cursor, q);
	prm(q);
	return 0;
}

/* Set book-mark */

static int dosetmark(BW *bw, int c, void *object, int *notify)
{
	if (notify)
		*notify = 1;
	if (c >= '0' && c <= '9') {
		pdupown(bw->cursor, bw->b->marks + c - '0');
		poffline(bw->b->marks[c - '0']);
		snprintf((char *)msgbuf, JOE_MSGBUFSIZE, "Mark %d set", c - '0');
		msgnw(bw->parent, msgbuf);
		return 0;
	} else {
		nungetc(c);
		return -1;
	}
}

int usetmark(BW *bw, int c)
{
	if (c >= '0' && c <= '9')
		return dosetmark(bw, c, NULL, NULL);
	else if (mkqwna(bw->parent, sc("Set mark (0-9):"), dosetmark, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

/* Goto book-mark */

static int dogomark(BW *bw, int c, void *object, int *notify)
{
	if (notify)
		*notify = 1;
	if (c >= '0' && c <= '9')
		if (bw->b->marks[c - '0']) {
			pset(bw->cursor, bw->b->marks[c - '0']);
			bw->cursor->xcol = piscol(bw->cursor);
			return 0;
		} else {
			snprintf((char *)msgbuf, JOE_MSGBUFSIZE, "Mark %d not set", c - '0');
			msgnw(bw->parent, msgbuf);
			return -1;
	} else {
		nungetc(c);
		return -1;
	}
}

int ugomark(BW *bw, int c)
{
	if (c >= '0' && c <= '9')
		return dogomark(bw, c, NULL, NULL);
	else if (mkqwna(bw->parent, sc("Goto bookmark (0-9):"), dogomark, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

/* Goto next instance of character */

static int dobkwdc;

static int dofwrdc(BW *bw, int k, void *object, int *notify)
{
	int c;
	P *q;

	if (notify)
		*notify = 1;
	if (k < 0 || k >= 256) {
		nungetc(k);
		return -1;
	}
	q = pdup(bw->cursor);
	if (dobkwdc) {
		while ((c = prgetc(q)) != NO_MORE_DATA)
			if (c == k)
				break;
	} else {
		while ((c = pgetc(q)) != NO_MORE_DATA)
			if (c == k)
				break;
	}
	if (c == NO_MORE_DATA) {
		msgnw(bw->parent, US "Not found");
		prm(q);
		return -1;
	} else {
		pset(bw->cursor, q);
		bw->cursor->xcol = piscol(bw->cursor);
		prm(q);
		return 0;
	}
}

int ufwrdc(BW *bw, int k)
{
	dobkwdc = 0;
	if (k >= 0 && k < 256)
		return dofwrdc(bw, k, NULL, NULL);
	else if (mkqw(bw->parent, sc("Fwrd to char: "), dofwrdc, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

int ubkwdc(BW *bw, int k)
{
	dobkwdc = 1;
	if (k >= 0 && k < 256)
		return dofwrdc(bw, k, NULL, NULL);
	else if (mkqw(bw->parent, sc("Bkwd to char: "), dofwrdc, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

/* Display a message */

static int domsg(BASE *b, unsigned char *s, void *object, int *notify)
{
	if (notify)
		*notify = 1;
	strcpy(msgbuf, s);
	vsrm(s);
	msgnw(b->parent, msgbuf);
	return 0;
}

int umsg(BASE *b)
{
	if (wmkpw(b->parent, US "Msg (^C to abort): ", NULL, domsg, NULL, NULL, NULL, NULL, NULL, -1))
		return 0;
	else
		return -1;
}

/* Insert text */

static int dotxt(BW *bw, unsigned char *s, void *object, int *notify)
{
	int x;

	if (notify)
		*notify = 1;
	for (x = 0; x != sLEN(s); ++x)
		utypebw(bw, s[x]);
	vsrm(s);
	return 0;
}

int utxt(BW *bw)
{
	if (wmkpw(bw->parent, US "Insert (^C to abort): ", NULL, dotxt, NULL, NULL, utypebw, NULL, NULL, bw->b->o.utf8))
		return 0;
	else
		return -1;
}
