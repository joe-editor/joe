/*
 *	User text formatting functions
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "types.h"

/* Center line cursor is on and move cursor to beginning of next line */

int ucenter(W *w, int k)
{
	BW *bw;
	P *p, *q;
	off_t endcol, begcol, x;
	int c;

	WIND_BW(bw, w);
	p = bw->cursor;

	p_goto_eol(p);
	while (joe_isblank(bw->b->o.charmap, (c = prgetc(p))))
		/* do nothing */;
	if (c == '\n') {
		pgetc(p);
		goto done;
	}
	if (c == NO_MORE_DATA)
		goto done;
	pgetc(p);
	endcol = piscol(p);

	p_goto_bol(p);
	while (joe_isblank(bw->b->o.charmap, (c = pgetc(p))))
		/* do nothing */;
	if (c == '\n') {
		prgetc(p);
		goto done;
	}
	if (c == NO_MORE_DATA)
		goto done;
	prgetc(p);
	begcol = piscol(p);

	if (endcol - begcol > bw->o.rmargin + bw->o.lmargin)
		goto done;

	q = pdup(p, "ucenter");
	p_goto_bol(q);
	bdel(q, p);
	prm(q);

	for (x = 0; x != (bw->o.lmargin + bw->o.rmargin) / 2 - (endcol - begcol) / 2; ++x)
		binsc(p, ' ');

      done:
	if (!pnextl(p)) {
		binsc(p, '\n');
		pgetc(p);
		return -1;
	} else
		return 0;
}

/* Return true if c is a character which can indent a paragraph */

/*   > is for mail/news
 *   * is for C comments
 *   / is for C++ comments
 *   # is for shell script comments
 *   % is for TeX comments
 */

static int cpara(BW *bw, int c)
{
	if (c == ' ' || c == '\t')
		return 1;
	if (bw->o.cpara) {
		const char *s = bw->o.cpara;
		while (*s) {
			if (utf8_decode_fwrd(&s, NULL) == c)
				return 1;
		}
	}
	return 0;
#ifdef junk
	if (c == ' ' || c == '\t' || c == '\\' ||
	    c == '>' || c == '|' || c == ':' || c == '*' || c == '/' ||
	    c == ',' || c == '.' || c == '?' || c == ';' || c == ']' ||
	    c == '}' || c == '=' || c == '+' || c == '-' || c == '_' ||
	    c == ')' || c == '&' || c == '^' || c == '%' || c == '$' ||
	    c == '#' || c == '@' || c == '!' || c == '~')
		return 1;
	else
		return 0;
#endif
}

/* Return true if this first non-whitespace character means
   we are not a paragraph for sure (for example, '.' in nroff) */

static int cnotpara(BW *bw, int c)
{
	const char *s;
	if (bw->o.cnotpara) {
		s = bw->o.cnotpara;
		while (*s) {
			if (c == utf8_decode_fwrd(&s, NULL))
				return 1;
		}
	}
	return 0;
}

/* Return true if line is definitely not a paragraph line.
 * Lines which aren't paragraph lines:
 *  1) Blank lines
 *  2) Lines which begin with '.'
 */

static int pisnpara(BW *bw, P *p)
{
	P *q;
	int c;

	q = pdup(p, "pisnpara");
	p_goto_bol(q);
	while (cpara(bw, c = pgetc(q)))
		/* do nothing */;
	prm(q);
	if (cnotpara(bw, c) || c == '\r' || c == '\n')
		return 1;
	else
		return 0;
}

/* Determine amount of indentation on current line.  Set first
   to include '-' and '*' bullets. */

static off_t nindent(BW *bw, P *p, int first)
{
	P *q = pdup(p, "nindent");
	off_t col;
	int c;

	p_goto_bol(q);
	do {
		col = q->col;
	} while (cpara(bw, (c = pgetc(q))));
	if (first && (c == '-' || c == '*')) {
		c = pgetc(q);
		if (c == ' ') {
			col = q->col;
		}
	}
	prm(q);
	return col;
}

/* Get indentation prefix column */

static off_t prefix(BW *bw, P *p,int up)
{
	off_t len;
	P *q = pdup(p, "prefix");

	p_goto_bol(q);
	while (cpara(bw, brch(q)))
		pgetc(q);
	while (!pisbol(q)) {
		/* int c; */
		if (!joe_isblank(p->b->o.charmap, ( /* c = */ prgetc(q)))) {
		/*
			if (up && (c == '*' || c == '-')) {
				if (!pisbol(q)) {
					c = prgetc(q);
					pgetc(q);
					if (c == ' ' || c == '\t')
						goto skip;
				} else
					goto skip;
			}
			pgetc(q);
		*/
			break;
		/* 	skip:; */
		}
	}
	len = piscol(q);
	prm(q);
	return len;
}

/* Move pointer to beginning of paragraph
 *
 * This function simply moves backwards until it sees:
 *  0) The beginning of the file
 *  1) A blank line
 *  2) A line with a different indentation prefix
 *  3) A line with indentation greater than that of the line we started with
 *  4) A line with indentation less than that of the starting line, but with
 *     a blank line (or beginning of file) preceding it.
 */

int within = 0;

P *pbop(BW *bw, P *p)
{
	off_t indent;
	off_t prelen;
	P *last;

	p_goto_bol(p);
	indent = nindent(bw, p, 0);
	prelen = prefix(bw, p, 0);
	last = pdup(p, "pbop");
	while (!pisbof(p) && (!within || !markb || p->byte > markb->byte)) {
		off_t ind;
		off_t len;

		pprevl(p);
		p_goto_bol(p);
		ind = nindent(bw, p, 0);
		len = prefix(bw, p, 0);
		if (pisnpara(bw, p) || len != prelen) {
			pset(p, last);
			break;
		}
		if (ind > indent) {
/*
			int ok = 1;
			P *q = pdup(p, "pbop");
			if (pprevl(q)) {
				p_goto_bol(q);
				if (nindent(bw, q, 0) == ind)
					ok = 0;
			}
			prm(q);
			if (!ok)
				pnextl(p);
*/
			break;
		}
		if (ind < indent) {
			pset(p, last);
			break;
			/* if (pisbof(p)) {
				break;
			}
			pprevl(p);
			p_goto_bol(p);
			if (pisnpara(bw, p)) {
				pnextl(p);
				break;
			} else {
				pnextl(p);
				pnextl(p);
				break;
			} */
		}
		pset(last, p);
	}
	prm(last);
	return p;
}

/* Move pointer to end of paragraph.  Pointer must already be on first
 * line of paragraph for this to work correctly.
 *
 * This function moves forwards until it sees:
 *  0) The end of the file.
 *  1) A blank line
 *  2) A line with indentation different from the second line of the paragraph
 *  3) A line with prefix column different from first line
 */

P *peop(BW *bw, P *p)
{
	off_t indent;
	off_t prelen;

	if (!pnextl(p) || pisnpara(bw, p) || (within && markk && p->byte >= markk->byte))
		return p;
	indent = nindent(bw, p, 0);
	prelen = prefix(bw, p, 0);
	while (pnextl(p) && (!within || !markk || p->byte < markk->byte)) {
		off_t ind = nindent(bw, p, 0);
		off_t len = prefix(bw, p, 0);

		if (ind != indent || len != prelen || pisnpara(bw, p))
			break;
	}
	return p;
}

/* Motion commands */

int ubop(W *w, int k)
{
	BW *bw;
	P *q;
	WIND_BW(bw, w);
	q = pdup(bw->cursor, "ubop");

      up:
	while (pisnpara(bw, q) && !pisbof(q) && (!within || !markb || q->byte > markb->byte))
		pprevl(q);
	pbop(bw, q);
	if (q->byte != bw->cursor->byte) {
		pset(bw->cursor, q);
		prm(q);
		return 0;
	} else if (!pisbof(q)) {
		prgetc(q);
		goto up;
	} else {
		prm(q);
		return -1;
	}
}

int ueop(W *w, int k)
{
	P *q;
	BW *bw;
	WIND_BW(bw, w);
	q = pdup(bw->cursor, "ueop");

      up:
	while (pisnpara(bw, q) && !piseof(q))
		pnextl(q);
	pbop(bw, q);
	peop(bw, q);
	if (q->byte != bw->cursor->byte) {
		pset(bw->cursor, q);
		prm(q);
		return 0;
	} else if (!piseof(q)) {
		pnextl(q);
		goto up;
	} else {
		prm(q);
		return -1;
	}
}

/* Wrap word.  If 'french' is set, only one space will be placed
 * after . ? or !
 */

void wrapword(BW *bw, P *p, off_t indent, int french, int no_over, char *indents)
{
	P *q;
	P *r;
	P *s;
	int rmf = 0;
	int c;
	off_t to = p->byte;
	int my_indents = 0;

	/* autoindent when called by utype */
	if (!indents) {
		/* Get indentation prefix from beginning of line */
		s = pdup(p, "wrapword");
		p_goto_bol(s);
		pbop(bw, s);
		/* Record indentation of second line of paragraph, of first
		 * line if there is only one line */
		q = pdup(s, "wrapword");
		pnextl(q);
		if (q->line < p->line) {
			/* Second line */
			P *tr = pdup(q, "wrapword");

			indent = nindent(bw, q, 0);
			pcol(tr, indent);
			indents = brs(q, tr->byte - q->byte); /* Risky */
			prm(tr);
		} else {
			/* First line */
			P *tr = pdup(s, "uformat");
			ptrdiff_t x, y;

			indent = nindent(bw, s, 1);
			pcol(tr, indent);
			indents = brs(s, tr->byte - s->byte); /* Risky */
			prm(tr);
			if (!bw->o.autoindent) {
				/* Don't indent second line of single-line paragraphs if autoindent is off */
				ptrdiff_t tx = zlen(indents);
				ptrdiff_t orgx = tx;
				while (tx && (indents[tx - 1] == ' ' || indents[tx - 1] == '\t'))
					indents[--tx] = 0;
				if (tx && orgx != tx) {
					indents[tx++] = ' ';
					indents[tx] = 0;
				}
				indent = txtwidth1(bw->o.charmap, bw->o.tab, indents, tx);
			}
			for (x = 0; indents[x] && (indents[x] == ' ' || indents[x] == '\t'); ++x);
			y = zlen(indents);
			while (y && (indents[y - 1] == ' ' || indents[y - 1] == '\t'))
				--y;
			/* Don't duplicate bullet */
			if (y && ((indents[y - 1] == '*' && !cpara(bw, '*')) || (indents[y - 1] == '-' && !cpara(bw, '-'))) &&
			    (y == 1 || indents[y - 2] == ' ' || indents[y - 2] == '\t'))
			    	indents[y - 1] = ' ';
			/* Fix C comment */
			if (indents[x] == '/' && indents[x + 1] == '*')
				indents[x] = ' ';
		}
		if (bw->o.lmargin > indent) {
			int x;
			for (x = 0; indents[x] == ' ' || indents[x] == '\t'; ++x);
			if (!indents[x]) {
				joe_free(indents);
				indent = bw->o.lmargin;
				indents = (char *)joe_malloc(indent+1); /* Risky */
				for (x = 0; x != indent; ++x)
					indents[x] = ' ';
				indents[x] = 0;
			}
		}
		my_indents = 1;
		prm(q);
		prm(s);
	}


/*
	if(!indents) {
		int f = 0;
		P *r = pdup(p);

		p_goto_bol(r);
		q = pdup(r);
		while(cpara(c = brc(q))) {
			if(!joe_isblank(c))
				f = 1;
			pgetc(q);
		}
		if(f) {
			indents = brs(r, q->byte-r->byte);
			rmf = 1;
			if(indents[0] == '/' && indents[1] == '*')
				indents[0] = ' ';
		}
		prm(r);
		prm(q);
	}
*/

	/* Get to beginning of word */
	while (!pisbol(p) && piscol(p) > indent && !joe_isblank(p->b->o.charmap, prgetc(p)))
		/* do nothing */;

	/* If we found the beginning of a word... */
	if (!pisbol(p) && piscol(p) > indent) {
		/* Move q to two (or one if 'french' is set) spaces after end of previous
		   word */
		q = pdup(p, "wrapword");
		while (!pisbol(q))
			if (!joe_isblank(p->b->o.charmap, (c = prgetc(q)))) {
				pgetc(q);
				if ((c == '.' || c == '?' || c == '!')
				    && q->byte != p->byte && !french)
					pgetc(q);
				break;
			}
		pgetc(p);

		/* Delete space between start of word and end of previous word */
		to -= p->byte - q->byte;
		bdel(q, p);
		prm(q);

		if (bw->o.flowed) {
			binsc(p, ' ');
			pgetc(p);
			++to;
		}

		/* Move word to beginning of next line */
		binsc(p, '\n');

		/* When overtype is on, do not insert lines */
		if (!no_over && p->b->o.overtype){
			/* delete the next line break which is unnecessary */
			r = pdup(p, "wrapword");
			/* p_goto_eol(r); */
			pgetc(r);
			p_goto_eol(r);
			s = pdup(r, "wrapword");
			pgetc(r);
			bdel(s,r);
			binsc(r, ' ');

			/* Now we got to take care that all subsequent lines are not longer than the right margin */
			/* Move cursor to right margin */
			pfwrd(r, r->b->o.rmargin - r->col);

			/* Make a copy of the cursor and move the copied cursor to the end of the line */
			prm(s);
			s = pdup(r, "wrapword");
			p_goto_eol(s);

			/* If s is located behind r then the line goes beyond the right margin and we need to call wordwrap() for that line. */
/*
			if (r->byte < s->byte){
				wrapword(bw, r, indent, french, 1, indents);
			}
*/
			prm(r);
			prm(s);
		}

		++to;
		if (p->b->o.crlf)
			++to;
		pgetc(p);

		/* Indent to left margin */
		if (indents) {
			binss(p, indents);
			to += zlen(indents);
		} else
			while (indent--) {
				binsc(p, ' ');
				++to;
			}

		if (rmf)
			joe_free(indents);
	}

	/* Move cursor back to original position */
	pfwrd(p, to - p->byte);
	if (my_indents)
		joe_free(indents);
}

/* Reformat paragraph */

int uformat(W *w, int k)
{
	off_t indent;
	char *indents;
	B *buf;
	P *b;
	off_t curoff;
	int c;
	P *p, *q;
	BW *bw;
	int flag;
	WIND_BW(bw, w);

	p = pdup(bw->cursor, "uformat");
	p_goto_bol(p);

	/* Do nothing if we're not on a paragraph line */
	if (pisnpara(bw, p)) {
		prm(p);
		return 0;
	}

	/* Move p to beginning of paragraph, bw->cursor to end of paragraph and
	 * set curoff to original cursor offset within the paragraph */
	pbop(bw, p);
	curoff = bw->cursor->byte - p->byte;
	pset(bw->cursor, p);
	peop(bw, bw->cursor);

	/* Ensure that paragraph ends on a beginning of a line */
	if (!pisbol(bw->cursor))
		binsc(bw->cursor, '\n'), pgetc(bw->cursor);

	/* Record indentation of second line of paragraph, of first line if there
	 * is only one line */
	q = pdup(p, "uformat");
	pnextl(q);
	if (q->line != bw->cursor->line) {
		P *r = pdup(q, "uformat");

		indent = nindent(bw, q, 0);
		pcol(r, indent);
		indents = brs(q, r->byte - q->byte); /* Risky */
		prm(r);
	} else {
		P *r = pdup(p, "uformat");
		ptrdiff_t x, y;
		indent = nindent(bw, p, 1); /* allowing * and - here */
		pcol(r, indent);
		indents = brs(p, r->byte - p->byte); /* Risky */
		prm(r);
		if (!bw->o.autoindent) {
			/* Don't indent second line of single-line paragraphs if autoindent is off */
			ptrdiff_t tx = zlen(indents);
			while (tx && (indents[tx - 1] == ' ' || indents[tx - 1] == '\t'))
				indents[--tx] = 0;
			if (tx) {
				indents[tx++] = ' ';
				indents[tx] = 0;
			}
			indent = txtwidth1(bw->o.charmap, bw->o.tab, indents, tx);
		}
		for (x = 0; indents[x] && (indents[x] == ' ' || indents[x] == '\t'); ++x);
		y = zlen(indents);
		while (y && (indents[y - 1] == ' ' || indents[y - 1] == '\t'))
			--y;
		/* Don't duplicate if it looks like a bullet */
		if (y && ((indents[y - 1] == '*' && !cpara(bw, '*')) || (indents[y - 1] == '-' && !cpara(bw, '-'))) &&
		    (y == 1 || indents[y - 2] == ' ' || indents[y - 2] == '\t'))
			indents[y - 1] = ' ';
		/* Fix C comment */
		if (indents[x] == '/' && indents[x + 1] == '*')
			indents[x] = ' ';
	}
	prm(q);

	/* But if the left margin is greater, we use that instead */
	if (bw->o.lmargin > indent) {
		int x;
		for (x = 0; indents[x] == ' ' || indents[x] == '\t'; ++x);
		if (!indents[x]) {
			joe_free(indents);
			indent = bw->o.lmargin;
			indents = (char *)joe_malloc(indent+1); /* Risky, indent could be very large in theory */
			for (x = 0; x != indent; ++x)
				indents[x] = ' ';
			indents[x] = 0;
		}
	}

	/* Cut paragraph into new buffer */

	/* New buffer needs to inherit UTF-8 and CR-LF options */
	buf = bcpy(p, bw->cursor);
	buf->o.crlf = p->b->o.crlf;
	buf->o.charmap = p->b->o.charmap;
	bdel(p, bw->cursor);

	/* text is in buffer.  insert it at cursor */

	b = pdup(buf->bof, "uformat");

	/* First line: preserve whitespace within this line
	   (but still apply french spacing after periods) */
	flag = 0;
	while (!piseof(b)) {
		c = brch(b);
		if (joe_isblank(b->b->o.charmap,c) || c == '\n') {
			int f = 0;

			/* First space after end of word */
			if (flag && piscol(p) > bw->o.rmargin)
				wrapword(bw, p, indent, bw->o.french, 1, indents);

			flag = 0;

			/* Stop if we're at end of line */
			if (c == '\n' || piseolblank(b))
				break;

			/* Set f if previous character was '.', '?' or '!' */
			if (!pisbof(b)) {
				P *d = pdup(b, "uformat");
				int g = prgetc(d);
				if (g=='.' || g=='?' || g=='!') {
					f = 1;
				}
				prm(d);
			}

			if (f) {
				/* Skip past the whitespace. */
				while (joe_isblank(b->b->o.charmap, brc(b))) {
					if(b->byte == curoff)
						pset(bw->cursor, p);
					pgetc(b);
				}

				/* Insert proper amount of whitespace */
				if (!piseof(b)) {
					if (!bw->o.french)
						binsc(p, ' '), pgetc(p);
					binsc(p, ' ');
					pgetc(p);
				}
			} else {
				/* Insert whitespace character, advance pointer */
				binsc(p, pgetc(b));
				pgetc(p);
			}
		} else {
			/* Insert characters of word and wrap if necessary */
			if (b->byte == curoff)
				pset(bw->cursor, p);

			binsc(p, pgetc(b));
			pgetc(p);
			flag = 1;
		}
	}

	/* Remaining lines: collapse whitespace */

	flag = 0;
	while (!piseof(b)) {
		c = brc(b);
		if (joe_isblank(b->b->o.charmap,c) || c == '\n') {
			int f = 0;
			P *d;
			int g;

			/* First space at end of word, wrap it */
			if (flag && piscol(p) > bw->o.rmargin)
				wrapword(bw, p, indent, bw->o.french, 1, indents);

			flag = 0;

			/* Detect end of sentence */
			d=pdup(b, "uformat");
			g=prgetc(d);
			if (g=='.' || g=='?' || g=='!') {
				f = 1;
			}
			prm(d);

			/* Skip past the whitespace.  Skip over indentations */
		      loop:

			c = brc(b);
			if (c == '\n') {
				if (b->byte == curoff)
					pset(bw->cursor, p);

				pgetc(b);
				while (cpara(bw, (c=brch(b)))) {
					if (b->byte == curoff)
						pset(bw->cursor, p);
					pgetc(b);
				}
			}

			if (joe_isblank(b->b->o.charmap,c)) {
				if(b->byte == curoff)
					pset(bw->cursor, p);
				pgetc(b);
				goto loop;
			}

			/* Insert proper amount of whitespace */
			if (!piseof(b)) {
				if (f && !bw->o.french)
					binsc(p, ' '), pgetc(p);
				binsc(p, ' ');
				pgetc(p);
			}
		} else {
			/* Insert characters of word and wrap if necessary */
			if (b->byte == curoff)
				pset(bw->cursor, p);

			binsc(p, pgetc(b));
			pgetc(p);
			flag = 1;
		}
	}

	if (flag && piscol(p) > bw->o.rmargin)
		wrapword(bw, p, indent, bw->o.french, 1, indents);

	binsc(p, '\n');
	prm(p);
	brm(buf);
	joe_free(indents);
	return 0;
}

/* Format entire block */

int ufmtblk(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (markv(1) && bw->cursor->byte >= markb->byte && bw->cursor->byte <= markk->byte) {
		markk->end = 1;
		utomarkk(w, 0);
		within = 1;
		do {
			ubop(w, 0), uformat(w, 0);
		} while (bw->cursor->byte > markb->byte);
		within = 0;
		markk->end = 0;
		if (lightoff)
			unmark(w, 0);
		return 0;
	} else
		return uformat(w, 0);
}

/* Clean up indentation: Remove indentation from blank lines */

int utrimlines(W *w, int k)
{
	P *p, *q;
	BW *bw;
	struct charmap *map;
	int c;

	WIND_BW(bw, w);

	p = pdup(bw->b->bof, "utrimlines");
	map = bw->b->o.charmap;

	while (!piseof(p)) {
		p->valcol = 0; /* Avoid some extra work */
		p_goto_eol(p);
		q = pdup(p, "utrimlines");

		do {
			c = prgetc(q);
		} while (joe_isblank(map, c));

		/* We read a non-blank unless we bumped the bof */
		if (c != NO_MORE_DATA) {
			pgetc(q);
		}

		if (q->byte < p->byte) {
			bdel(q, p);
		}

		prm(q);
		pgetc(p);
	}

	prm(p);
	return 0;
}
