/*
 *	Search & Replace system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "types.h"

int wrap = 0;			/* Allow wrap */
int smode = 0;			/* Decremented to zero by execmd */
int csmode = 0;			/* Set for continued search mode */
int opt_icase = 0;			/* Set to force case insensitive search */
int pico = 0;			/* Pico search prompting */

B *findhist = NULL;		/* Search string history */
B *replhist = NULL;		/* Replacement string history */

SRCH *globalsrch = NULL;	/* Most recent completed search data */

SRCHREC fsr = { {&fsr, &fsr} };

/* Clear compiled version of pattern */

static void clrcomp(SRCH *srch)
{
	if (srch->comp) {
		joe_regfree(srch->comp);
		srch->comp = 0;
	}
}

/* Set pattern, clear compiled version of it */

void setpat(SRCH *srch, char *s)
{
	vsrm(srch->pattern);
	srch->pattern = s;
	clrcomp(srch);
}

/* Completion stuff: should go somewhere else */

char **word_list;

#define MAX_WORD_SIZE 64
static char **get_word_list(B *b,off_t ignore)
{
	char buf[MAX_WORD_SIZE];
	char *s;
	char **list = 0;
	HASH *h;
	HENTRY *t;
	P *p;
	int c;
	ptrdiff_t idx;
	off_t start = 0;

	h = htmk(1024);

	p = pdup(b->bof, "get_word_list");
	idx = 0;
	do {
		c = pgetc(p);
		if (idx) {
			if (joe_isalnum_(b->o.charmap, c)) {
				if (b->o.charmap->type) {
					if (idx + 8 < MAX_WORD_SIZE) {
						idx += utf8_encode(buf+idx, c);
					}
				} else {
					if (idx!=MAX_WORD_SIZE)
						buf[idx++] = TO_CHAR_OK(c);
				}
			} else {
				if (idx!=MAX_WORD_SIZE && start!=ignore) {
					buf[idx] = 0;
					if (!htfind(h,buf)) {
						s = vsncpy(NULL,0,buf,idx);
						htadd(h, s, s);
					}
				}
				idx = 0;
			}
		} else {
			start=p->byte-1;
			if (joe_isalpha_(b->o.charmap, c)) {
				if (b->o.charmap->type) {
					idx += utf8_encode(buf+idx, c);
				} else {
					buf[idx++] = TO_CHAR_OK(c);
				}
			}
		}
	} while (c != NO_MORE_DATA);
	prm(p);

	for (idx = 0;idx != h->len;++idx)
		for (t = h->tab[idx];t;t=t->next)
			list = vaadd(list, (char *)t->val);
	if (list)
		vasort(list,sLEN(list));	

	htrm(h);

	return list;
}

static void fcmplt_ins(BW *bw, char *line)
{
	P *p;
	int c;

	if (!piseol(bw->cursor)) {
		c = brch(bw->cursor);
		if (joe_isalnum_(bw->b->o.charmap, c))
			return;
	}

	/* Move p to beginning of word */

	p = pdup(bw->cursor, "fcmplt_ins");
	do
		c = prgetc(p);
		while (joe_isalnum_(bw->b->o.charmap,c));
	if (c!=NO_MORE_DATA)
		pgetc(p);

	if (bw->cursor->byte!=p->byte && bw->cursor->byte-p->byte<64) {
		/* Insert single match */
		bdel(p,bw->cursor);
		binsm(bw->cursor,sv(line));
		pfwrd(bw->cursor,sLEN(line));
		bw->cursor->xcol = piscol(bw->cursor);
		prm(p);
	} else {
		prm(p);
	}
}

static int fcmplt_abrt(W *w, ptrdiff_t x, void *obj)
{
	BW *bw;
	char *line = (char *)obj;
	WIND_BW(bw, w);
	if (line) {
		fcmplt_ins(bw, line);
		vsrm(line);
	}
	return -1;
}

static int fcmplt_rtn(MENU *m, ptrdiff_t x, void *obj, int k)
{
	char *line = (char *)obj;
	fcmplt_ins((BW *)m->parent->win->object, m->list[x]);
	vsrm(line);
	m->object = NULL;
	wabort(m->parent);
	return 0;
}

int ufinish(W *w, int k)
{
	char *line;
	char *line1;
	char **lst;
	P *p;
	int c;
	MENU *m;
	BW *bw;
	WIND_BW(bw, w);

	/* Make sure we're not in a word */

	if (!piseol(bw->cursor)) {
		c = brch(bw->cursor);
		if (joe_isalnum_(bw->b->o.charmap, c))
			return -1;
	}

	/* Move p to beginning of word */

	p = pdup(bw->cursor, "ufinish");
	do
		c = prgetc(p);
		while (joe_isalnum_(bw->b->o.charmap,c));
	if (c!=NO_MORE_DATA)
		pgetc(p);

	if (bw->cursor->byte!=p->byte && bw->cursor->byte-p->byte<64) {
		line = brvs(p, bw->cursor->byte - p->byte); /* CVT: Risky- we assume the match fits in memory */

		/* We have a word */

		/* Get word list */
		if (word_list)
			varm(word_list);

		word_list = get_word_list(bw->b, p->byte);

		if (!word_list) {
			vsrm(line);
			prm(p);
			return -1;
		}

		line1 = vsncpy(NULL,0,sv(line));
		line1 = vsadd(line1,'*');
		lst = regsub(word_list, aLEN(word_list), line1);
		vsrm(line1);

		if (!lst) {
			ttputc(7);
			vsrm(line);
			return -1;
		}

		m = mkmenu(bw->parent, bw->parent, lst, fcmplt_rtn, fcmplt_abrt, NULL, 0, line, NULL);
		if (!m) {
			varm(lst);
			vsrm(line);
			return -1;
		}

		/* Possible match list is now in lst */

		if (aLEN(lst) == 1)
			return fcmplt_rtn(m, 0, line, 0);
		else if (smode)
			return 0;
		else {
			char *com = mcomplete(m);
			vsrm((char *)m->object);
			m->object = com;
			wabort(m->parent);
			smode = 2;
			ttputc(7);
			return 0;
		}
	} else {
		prm(p);
		return -1;
	}
}

static int srch_cmplt(BW *bw, int k)
{
	if (word_list)
		varm(word_list);

	word_list = get_word_list(((BW *)bw->parent->win->object)->b, -1);

	if (!word_list) {
		ttputc(7);
		return 0;
	}

	return simple_cmplt(bw,word_list);
}

/* Search forward.
   bw, pattern and ignore must be set

   The first possible string we can find is the one beginning under p

   Returns p if we found a string:
     The found string is placed in entire/pieces
     p is placed right after the found string

   Return 0 if we did not find the string:
     p is left in its original spot
*/

static P *searchf(BW *bw,SRCH *srch, P *p)
{
	P *start;
	P *end;
	int flag = 0;

	start = pdup(p, "searchf");
	end = pdup(p, "searchf");

	try_again:

	wrapped:
	while (srch->ignore ? pifind(start, srch->comp->prefix, srch->comp->prefix_len) : pfind(start, srch->comp->prefix, srch->comp->prefix_len)) {
		pset(end, start);
		/* pfwrd(end, x); */ /* Comment out for regexec */
		if (srch->wrap_flag && start->byte>=srch->wrap_p->byte)
			break;
		if (!joe_regexec(srch->comp, end, NMATCHES, srch->pieces, srch->ignore)) {
			if (end->byte == srch->last_repl && !flag) {
				/* Stuck on zero-width regex? */
				pset(start, p);
				if (pgetc(start) == NO_MORE_DATA)
					break;
				pset(end, start);
				++flag; /* Try repeating, but only one time */
				goto try_again;
			} else {
				srch->entire.rm_so = start->byte;
				srch->entire.rm_eo = end->byte;
				pset(p, end);
				prm(start);
				prm(end);
				srch->last_repl = p->byte; /* Prevent getting stuck with zero-length regex */
				return p;
			}
		}
		if (pgetc(start) == NO_MORE_DATA)
			break;
	}
	if (srch->allow_wrap && !srch->wrap_flag && srch->wrap_p) {
		msgnw(bw->parent, joe_gettext(_("Wrapped")));
		srch->wrap_flag = 1;
		p_goto_bof(start);
		goto wrapped;
	}
	srch->last_repl = -1;
	prm(start);
	prm(end);
	return NULL;
}

/* Search backwards.
   bw, pattern and ignore must be set

   The first possible string we can find is the one beginning one position
   to the left of p.

   Returns 1 if we found a string:
     The found string is placed in entire
     p is placed at the beginning of the string

   Return 0 if we did not find the string:
     p is left in its original spot
*/

static P *searchb(BW *bw,SRCH *srch, P *p)
{
	P *start;
	P *end;
	int flag = 0;

	start = pdup(p, "searchb");
	end = pdup(p, "searchb");

	try_again:

	wrapped:
	while (pbkwd(start, 1L)
	       && (srch->ignore ? prifind(start, srch->comp->prefix, srch->comp->prefix_len) : prfind(start, srch->comp->prefix, srch->comp->prefix_len))) {
		pset(end, start);
		/* pfwrd(end, x); */ /* Comment out for joe_regexec */
		if (srch->wrap_flag && start->byte<srch->wrap_p->byte)
			break;
		if (!joe_regexec(srch->comp, end, NMATCHES, srch->pieces, srch->ignore)) {
			if (start->byte == srch->last_repl && !flag) {
				/* Stuck? */
				pset(start, p);
				if (prgetc(start) == NO_MORE_DATA)
					break;
				pset(end, start);
				++flag;
				goto try_again;
			} else {
				srch->entire.rm_so = start->byte;
				srch->entire.rm_eo = end->byte;
				pset(p, start);
				prm(start);
				prm(end);
				return p;
			}
		}
	}

	if (srch->allow_wrap && !srch->wrap_flag && srch->wrap_p) {
		msgnw(bw->parent, joe_gettext(_("Wrapped")));
		srch->wrap_flag = 1;
		p_goto_eof(start);
		goto wrapped;
	}
	srch->last_repl = -1;
	prm(start);
	prm(end);
	return NULL;
}

/* Make a search stucture */

static SRCH *setmark(SRCH *srch)
{
	if (markv(0))
		srch->valid = 1;

	srch->markb = markb;
	if (srch->markb)
		srch->markb->owner = &srch->markb;
	markb = NULL;

	srch->markk = markk;
	if (srch->markk)
		srch->markk->owner = &srch->markk;
	markk = NULL;

	return srch;
}

SRCH *mksrch(char *pattern, char *replacement, int ignore, int backwards, int repeat, int replace, int rest, int all, int regex)
{
	SRCH *srch = (SRCH *) joe_malloc(SIZEOF(SRCH));
	int x;

	srch->first = NULL;
	srch->current = NULL;
	srch->all = all;
	srch->pattern = pattern;
	srch->comp = 0;
	srch->replacement = replacement;
	srch->ignore = ignore;
	srch->regex = regex;
	srch->debug = 0;
	srch->backwards = backwards;
	srch->repeat = repeat;
	srch->replace = replace;
	srch->rest = rest;
	srch->flg = 0;
	srch->addr = -1;
	srch->last_repl = -1;
	srch->markb = NULL;
	srch->markk = NULL;
	srch->wrap_p = NULL;
	srch->allow_wrap = wrap;
	srch->wrap_flag = 0;
	srch->valid = 0;
	srch->block_restrict = 0;
	izque(SRCHREC, link, &srch->recs);
	for (x = 0; x != NMATCHES; ++x) {
		srch->pieces[x].rm_so = -1;
		srch->pieces[x].rm_eo = -1;
	}
	srch->entire.rm_so = -1;
	srch->entire.rm_eo = -1;
	return srch;
}

/* Eliminate a search structure */

void rmsrch(SRCH *srch)
{
	if (srch->comp)
		joe_regfree(srch->comp);
	prm(srch->wrap_p);
	if (srch->markb || srch->markk) {
		/* We don't want isearch to clear an existing block, which it will do if
		   both srch->markb and srch->markk are null.  On the other hand, finishing
		   a find/replace should absolutely restore the old state, even if only one
		   of the two was set.  If we prm inside the if's below, then we get a half-
		   way state with a weird, unexpected block. */
		prm(markb);
		prm(markk);
	}
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
	frchn(&fsr, &srch->recs);
	vsrm(srch->pattern);
	vsrm(srch->replacement);
	joe_free(srch);
	updall();
}

/* Insert a replacement string
 * p is advanced past the inserted text
 */

static P *insert(SRCH *srch, P *p, const char *s, ptrdiff_t len, B **entire, B **pieces)
{
	ptrdiff_t x;
	off_t starting = p->byte;
	int nth;
	int case_flag = 0;
	B *b;

	while (len) {
		for (x = 0; x != len && s[x] != '\\'; ++x) ;
		if (x) {
			const char *t = s;
			ptrdiff_t y = x;
			switch (case_flag) {
				case 1: {
					case_flag = 0;
				} case 2: {
					while (y) {
						int ch = fwrd_c(p->b->o.charmap, &t, &y);
						ch = joe_tolower(p->b->o.charmap, ch);
						binsc(p, ch);
						pgetc(p);
					}
					break;
				} case -1: {
					case_flag = 0;
				} case -2: {
					while (y) {
						int ch = fwrd_c(p->b->o.charmap, &t, &y);
						ch = joe_toupper(p->b->o.charmap, ch);
						binsc(p, ch);
						pgetc(p);
					}
					break;
				}
			}
			if (y) {
				binsm(p, t, y);
				pfwrd(p, y);
			}
			len -= x;
			s += x;
		} else if (len >= 2) {
			if (s[1] == 'l') {
				case_flag = 1;
				s += 2;
				len -= 2;
			} else if (s[1] == 'L') {
				case_flag = 2;
				s += 2;
				len -= 2;
			} else if (s[1] == 'u') {
				case_flag = -1;
				s += 2;
				len -= 2;
			} else if (s[1] == 'U') {
				case_flag = -2;
				s += 2;
				len -= 2;
			} else if (s[1] == 'E') {
				case_flag = 0;
				s += 2;
				len -= 2;
			} else if (s[1] >= '1' && s[1] <= '9') {
				nth = s[1] - '1';
				s += 2;
				len -= 2;
				if (pieces[nth]) {
					b = bcpy(pieces[nth]->bof, pieces[nth]->eof);
					goto insertit;
				}
			} else if (s[1] == '&') {
				s += 2;
				len -= 2;
				if (*entire) {
					b = bcpy(entire[0]->bof, entire[0]->eof);
					insertit:
					if (case_flag) {
						P *q = pdup(b->bof, "insert");
						while (!piseof(q)) {
							int ch = pgetc(q);
							switch (case_flag) {
								case 1: {
									case_flag = 0;
								} case 2: {
									ch = joe_tolower(p->b->o.charmap, ch);
									break;
								} case -1: {
									case_flag = 0;
								} case -2: {
									ch = joe_toupper(p->b->o.charmap, ch);
									break;
								}
							}
							binsc(p, ch);
							pgetc(p);
						}
						prm(q);
						brm(b);
					} else {
						off_t l = b->eof->byte;
						binsb(p, b);
						pfwrd(p, l);
					}
				}
			} else {
				const char *a = s + x;
				ptrdiff_t l = len - x;
				int ch = escape(p->b->o.charmap->type, &a, &l, NULL);
				if (ch != -256) {
					switch (case_flag) {
						case 1: {
							case_flag = 0;
						} case 2: {
							ch = joe_tolower(p->b->o.charmap, ch);
							break;
						} case -1: {
							case_flag = 0;
						} case -2: {
							ch = joe_toupper(p->b->o.charmap, ch);
							break;
						}
					}
					binsc(p, ch);
					pgetc(p);
				}
				len -= a - s;
				s = a;
			}
		} else
			len = 0;
	}

	if (srch->backwards)
		pbkwd (p, p->byte - starting);

	return p;
}

/* Search system user interface */

/* Query for search string, search options, possible replacement string,
 * and execute first search */

char srchstr[] = "Search";	/* Context sensitive help identifier */
char replstr[] = "Replace";	/* Context sensitive help identifier */
char srchopt[] = "SearchOptions";

static int pfabort(W *w, void *obj)
{
	SRCH *srch = (SRCH *)obj;
	if (srch)
		rmsrch(srch);
	return -1;
}

static int pfsave(W *w, void *obj)
{
	SRCH *srch = (SRCH *)obj;
	if (srch) {
		if (globalsrch)
			rmsrch(globalsrch);
		globalsrch = srch;
		srch->rest = 0;
		srch->repeat = -1;
		srch->flg = 0;

		if (srch->markb || srch->markk) {
			/* We don't want isearch to clear an existing block, which it will do if
			   both srch->markb and srch->markk are null.  On the other hand, finishing
			   a find/replace should absolutely restore the old state, even if only one
			   of the two was set.  If we prm inside the if's below, then we get a half-
			   way state with a weird, unexpected block. */
			prm(markb);
			prm(markk);
		}
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
		srch->markb = NULL;
		srch->markk = NULL;

		updall();
	}
	return -1;
}

static int set_replace(W *w, char *s, void *obj, int *notify)
{
	SRCH *srch = (SRCH *)obj;
	BW *bw;
	WIND_BW(bw, w);
	
	if (sLEN(s) || !globalsrch || !pico)
		srch->replacement = s;
	else {
		/* Use previous string: this prevents replace with empty string */
		/* vsrm(s);
		srch->replacement = vsdup(globalsrch->replacement); */
		srch->replacement = s;
	}
	return dopfnext(bw, setmark(srch), notify);
}

/* Option characters */

const char *all_key = _("|all files|aA");
const char *list_key = _("|error list files|eE");
const char *replace_key = _("|search and replace|rR");
const char *backwards_key = _("|backwards|bB");
const char *ignore_key = _("|ignore case|iI");
const char *block_key = _("|restrict to highlighted block|kK");
const char *noignore_key = _("|don't ignore case|sS");
const char *wrap_key = _("|wrap|wW");
const char *nowrap_key = _("|don't wrap|nN");
const char *regex_key = _("|regex|xX");
const char *noregex_key = _("|no regex|yY");
const char *regex_debug_key = _("|regex_debug|v");

static int set_options(W *w, char *s, void *obj, int *notify)
{
	SRCH *srch = (SRCH *)obj;
	BW *bw;
	char buf[80];
	const char *t;
	WIND_BW(bw, w);
	srch->ignore = opt_icase;

	t = s;
	while (*t) {
		int c = fwrd_c(locale_map, &t, NULL);
		if (yncheck(all_key, c))
			srch->all = 1;
		else if (yncheck(list_key, c))
			srch->all = 2;
		else if (yncheck(replace_key, c))
			srch->replace = 1;
		else if (yncheck(backwards_key, c))
			srch->backwards = 1;
		else if (yncheck(ignore_key, c))
			srch->ignore = 1;
		else if (yncheck(noignore_key, c))
			srch->ignore = 0;
		else if (yncheck(wrap_key, c))
			srch->allow_wrap = 1;
		else if (yncheck(nowrap_key, c))
			srch->allow_wrap = 0;
		else if (yncheck(block_key, c))
			srch->block_restrict = 1;
		else if (yncheck(regex_key, c))
			srch->regex = 1;
		else if (yncheck(regex_debug_key, c))
			srch->debug = 1;
		else if (yncheck(noregex_key, c))
			srch->regex = 0;
		else if (c >= '0' && c <= '9') {
			if (srch->repeat == -1)
				srch->repeat = 0;
			srch->repeat = srch->repeat * 10 + c - '0';
		}
	}	
	vsrm(s);
	if (srch->replace) {
		/* if (pico && globalsrch && globalsrch->replacement) {
			joe_snprintf_1(bf1,30,"%s",globalsrch->replacement);
			if (zlen(globalsrch->replacement)>29)
				zlcat(bf1, SIZEOF(bf1), "$");
			joe_snprintf_1(buf,SIZEOF(buf),joe_gettext(_("Replace with (%{help} for help) [%s]: ")),bf1);
		} else */
			joe_snprintf_0(buf, SIZEOF(buf), joe_gettext(_("Replace with (%{help} for help): ")));
		if (wmkpw(bw->parent, buf, &replhist, set_replace, replstr, pfabort, srch_cmplt, srch, notify, bw->b->o.charmap, 0))
			return 0;
		else
			return -1;
	} else
		return dopfnext(bw, setmark(srch), notify);
}

static int set_pattern(W *w, char *s, void *obj, int *notify)
{
	SRCH *srch = (SRCH *)obj;
	BW *bw;
	BW *pbw;
	const char *p;
	WIND_BW(bw, w);
	if (opt_icase)
		p = joe_gettext(_("case (S)ensitive (R)eplace (B)ackwards Bloc(K) (%{help} for help): "));
	else
		p = joe_gettext(_("(I)gnore (R)eplace (B)ackwards Bloc(K) (%{help} for help): "));

	if (sLEN(s) || !globalsrch || !pico) {
		setpat(srch, s);
	} else {
		vsrm(s);
		setpat(srch, vsdup(globalsrch->pattern));
	}
	if ((pbw = wmkpw(bw->parent, p, NULL, set_options, srchopt, pfabort, utypebw, srch, notify, locale_map, 0)) != NULL) {
		char buf[10];

		if (srch->ignore) {
			const char *t = joe_gettext(ignore_key);
			binsc(pbw->cursor, fwrd_c(locale_map, &t, NULL));
		}
		if (srch->replace) {
			const char *t = joe_gettext(replace_key);
			binsc(pbw->cursor, fwrd_c(locale_map, &t, NULL));
		}
		if (srch->backwards) {
			const char *t = joe_gettext(backwards_key);
			binsc(pbw->cursor, fwrd_c(locale_map, &t, NULL));
		}
		if (srch->repeat >= 0)
			joe_snprintf_1(buf, SIZEOF(buf), "%d", srch->repeat), binss(pbw->cursor, buf);
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

/* Unescape for text going to genfmt */

static void unesc_genfmt(char *d, char *s, ptrdiff_t len,ptrdiff_t max)
{
	while (max > 0 && len) {
		if (!*s) {
			*d++ = '\\';
			*d++ = '@';
			++s;
		} else {
			if (*s == '\\') {
				*d++ = '\\';
				--max;
			}
			*d++ = *s++;
		}
		--len;
		--max;
	}
	if (len)
		*d++ = '$';
	*d = 0;
}

int std_regex = 0; /* Standard regex format by default */

int dofirst(BW *bw, int back, int repl, char *hint)
{
	SRCH *srch;
	BW *pbw;
	char bf1[80];
	char buf[80];

	if (smode && globalsrch) {
		globalsrch->backwards = back;
		globalsrch->replace = repl;
		return pfnext(bw->parent, 0);
	}
	if (bw->parent->huh == srchstr) {
		off_t byte;

		p_goto_eol(bw->cursor);
		byte = bw->cursor->byte;
		p_goto_bol(bw->cursor);
		if (byte == bw->cursor->byte)
			prgetc(bw->cursor);
		return urtn(bw->parent, -1);
	}
	srch = mksrch(NULL, NULL, 0, back, -1, repl, 0, 0, std_regex);
	srch->addr = bw->cursor->byte;
	srch->wrap_p = pdup(bw->cursor, "dofirst");
	srch->wrap_p->owner = &srch->wrap_p;
	if (pico && globalsrch && globalsrch->pattern) {
		unesc_genfmt(bf1, sv(globalsrch->pattern), 30);
		joe_snprintf_1(buf,SIZEOF(buf),joe_gettext(_("Find (%{help} for help) [%s]: ")),bf1);
	} else
		joe_snprintf_0(buf, SIZEOF(buf), joe_gettext(_("Find (%{help} for help): ")));
	if ((pbw=wmkpw(bw->parent, buf, &findhist, set_pattern, srchstr, pfabort, srch_cmplt, srch, NULL, bw->b->o.charmap, 0))) {
		if (hint) {
			binss(pbw->cursor, hint);
			pset(pbw->cursor, pbw->b->eof);
			pbw->cursor->xcol = piscol(pbw->cursor);
		}
		return 0;
	} else {
		rmsrch(srch);
		return -1;
	}
}

int pffirst(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	return dofirst(bw, 0, 0, NULL);
}

int prfirst(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	return dofirst(bw, 1, 0, NULL);
}

int pqrepl(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	return dofirst(bw, 0, 1, NULL);
}

/* Execute next search */

static int doreplace(BW *bw, SRCH *srch)
{
	P *from, *to;
	P *q;

	int x;
	B *pieces[NMATCHES];
	B *entire;

	if (!modify_logic(bw,bw->b))
		return -1;
	if (markk)
		markk->end = 1;
	if (srch->markk)
		srch->markk->end = 1;

	/* Copy matched strings before we delete */
	from = pdup(bw->cursor, "doreplace:from");
	to = pdup(bw->cursor, "doreplace:to");
	for (x = 0; x != NMATCHES; ++x) {
		Regmatch_t *m = &srch->pieces[x];
		if (m->rm_eo > m->rm_so) {
			pgoto(from, m->rm_so);
			pgoto(to, m->rm_eo);
			pieces[x] = bcpy(from, to);
		} else {
			pieces[x] = 0;
		}
	}
	if (srch->entire.rm_eo > srch->entire.rm_so) {
		pgoto(from, srch->entire.rm_so);
		pgoto(to, srch->entire.rm_eo);
		entire = bcpy(from, to);
	} else {
		entire = 0;
	}
	prm(from);
	prm(to);

	q = pdup(bw->cursor, "doreplace");
	if (srch->backwards) {
		q = pfwrd(q, srch->entire.rm_eo - srch->entire.rm_so);
		bdel(bw->cursor, q);
		prm(q);
	} else {
		q = pbkwd(q, (srch->entire.rm_eo - srch->entire.rm_so));
		bdel(q, bw->cursor);
		prm(q);
	}
	insert(srch, bw->cursor, sv(srch->replacement), &entire, pieces);

	/* Delete copies */
	for (x = 0; x != NMATCHES; ++x)
		if (pieces[x])
			brm(pieces[x]);
	if (entire)
		brm(entire);

	srch->addr = bw->cursor->byte;
	srch->last_repl = bw->cursor->byte;
	if (markk)
		markk->end = 0;
	if (srch->markk)
		srch->markk->end = 0;
	return 0;
}

static void visit(SRCH *srch, BW *bw, int myyn)
{
	SRCHREC *r = (SRCHREC *) alitem(&fsr, SIZEOF(SRCHREC));
	r->addr = bw->cursor->byte;
	r->yn = myyn;
	r->wrap_flag = srch->wrap_flag;
	r->last_repl = srch->last_repl;
	r->b = bw->b;
	enqueb(SRCHREC, link, &srch->recs, r);
}

static void goback(SRCH *srch, BW *bw)
{
	SRCHREC *r = srch->recs.link.prev;

	if (r != &srch->recs) {
		srch->current = r->b;
		if (r->yn) {
			uundo(bw->parent, 0);
		}
		if (r->b != bw->b) {
			W *w = bw->parent;
			get_buffer_in_window(bw, r->b);
			bw = (BW *)w->object;
		}
		if (bw->cursor->byte != r->addr)
			pgoto(bw->cursor, r->addr);
		srch->wrap_flag = r->wrap_flag;
		srch->last_repl = r->last_repl;
		demote(SRCHREC, link, &fsr, r);
	}
}

const char *rest_key = _("|rest of file|rR");
const char *backup_key = _("|backup|bB");

static int dopfrepl(W *w, int c, void *obj, int *notify)
{
	BW *bw;
	SRCH *srch = (SRCH *)obj;
	WIND_BW(bw, w);
	srch->addr = bw->cursor->byte;
	/* Backspace means no for jmacs */
	if (c == NO_CODE || c == 8 || c == 127 || yncheck(no_key, c))
		return dopfnext(bw, srch, notify);
	else if (c == YES_CODE || yncheck(yes_key, c) || c == ' ') {
		srch->recs.link.prev->yn = 1;
		if (doreplace(bw, srch)) {
			pfsave(bw->parent, srch);
			return -1;
		} else
			return dopfnext(bw, srch, notify);
	} else if (yncheck(rest_key, c) || c == '!') {
		if (doreplace(bw, srch))
			return -1;
		srch->rest = 1;
		return dopfnext(bw, srch, notify);
	} else if (/* c == 8 || c == 127 || */ yncheck(backup_key, c)) {
 		W *tw = bw->parent;
		goback(srch, bw);
		goback(srch, (BW *)tw->object);
		return dopfnext((BW *)tw->object, srch, notify);
	} else if (c != -1) {
		if (notify)
			*notify = 1;
		pfsave(bw->parent, srch);
		nungetc(c);
		return 0;
	}
	if (mkqwnsr(bw->parent, sz(joe_gettext(_("Replace (Y)es (N)o (R)est (B)ackup (%{abort} to abort)?"))), dopfrepl, pfsave, srch, notify))
		return 0;
	else
		return pfsave(bw->parent, srch);
}

/* Test if found text is within region
 * return 0 if it is,
 * -1 if we should keep searching
 * 1 if we're done
 */

static int restrict_to_block(BW *bw, SRCH *srch)
{
	if (!srch->valid || !srch->block_restrict)
		return 0;
	bw->cursor->xcol = piscol(bw->cursor);
	if (srch->backwards)
		if (!square) {
			if (bw->cursor->byte < srch->markb->byte)
				return 1;
			else if (bw->cursor->byte + (srch->entire.rm_eo - srch->entire.rm_so) > srch->markk->byte)
				return -1;
		} else {
			if (bw->cursor->line < srch->markb->line)
				return 1;
			else if (bw->cursor->line > srch->markk->line)
				return -1;
			else if (piscol(bw->cursor) + (srch->entire.rm_eo - srch->entire.rm_so) > srch->markk->xcol || piscol(bw->cursor) < srch->markb->xcol)
				return -1;
	} else if (!square) {
		if (bw->cursor->byte > srch->markk->byte)
			return 1;
		else if (bw->cursor->byte - (srch->entire.rm_eo - srch->entire.rm_so) < srch->markb->byte)
			return -1;
	} else {
		if (bw->cursor->line > srch->markk->line)
			return 1;
		if (bw->cursor->line < srch->markb->line)
			return -1;
		if (piscol(bw->cursor) > srch->markk->xcol || piscol(bw->cursor) - (srch->entire.rm_eo - srch->entire.rm_so) < srch->markb->xcol)
			return -1;
	}
	return 0;
}

/* Possible results:
 *   0) Search or search & replace is finished.
 *   1) Search string was not found.
 *   2) Search string was found.
 *   3) Abort due to infinite loop
 *   4) Abort due to compile error
 */

static int fnext(BW *bw, SRCH *srch)
{
	P *sta;

	if (!srch->first) {
		srch->first = bw->b;
		srch->current = bw->b;
	}

	next:
	if (srch->repeat != -1) {
		if (!srch->repeat)
			return 0;
		else
			--srch->repeat;
	}
	again:
	/* Clear compiled version of pattern if character map changed (perhaps because we switched buffer) */
	if (srch->comp && srch->comp->cmap != bw->b->o.charmap) {
		clrcomp(srch);
		/* Fail if character map of search prompt doesn't match map of buffer */
		msgnw(bw->parent, joe_gettext(_("Character set of buffer does not match character set of search string")));
		return 4;
	}
	/* Compile pattern if we don't already have it */
	if (!srch->comp) {
		srch->comp = joe_regcomp(bw->b->o.charmap, srch->pattern, sLEN(srch->pattern), srch->ignore, srch->regex, srch->debug);
		if (srch->comp->err) {
			msgnw(bw->parent, joe_gettext(srch->comp->err));
			return 4;
		}
	}
	if (srch->backwards)
		sta = searchb(bw, srch, bw->cursor);
	else
		sta = searchf(bw, srch, bw->cursor);
	if (!sta && srch->all) {
		B *b;
		if (srch->all == 2)
			b = beafter(srch->current);
		else {
			berror = 0;
			b = bafter(srch->current);
		}
		if (b && b != srch->first && !berror) {
			W *w = bw->parent;
			srch->current = b;
			/* this bumps reference count of b */
			get_buffer_in_window(bw, b);
			bw = (BW *)w->object;
			p_goto_bof(bw->cursor);
			goto again;
		} else if (berror) {
			msgnw(bw->parent, joe_gettext(msgs[-berror]));
		}
	}
	if (!sta) {
		srch->repeat = -1;
		return 1;
	}
	if (srch->rest || (srch->repeat != -1 && srch->replace)) {
		if (srch->valid)
			switch (restrict_to_block(bw, srch)) {
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
			switch (restrict_to_block(bw, srch)) {
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
	W *w;
	int fnr;
	int orgmid = opt_mid;	/* Original mid status */
	int ret = 0;

	opt_mid = 1;		/* Screen recenters mode during search */
	if (csmode)
		smode = 2;	/* We have started a search mode */
	if (srch->replace)
		visit(srch, bw, 0);
again:	w = bw->parent;
	fnr = fnext(bw, srch);
	bw  = (BW *)w->object;
	switch (fnr) {
	case 0:
		break;
	case 1:
bye:		if (!srch->flg && !srch->rest) {
			if (srch->valid && srch->block_restrict)
				msgnw(bw->parent, joe_gettext(_("Not found (search restricted to marked block)")));
			else
				msgnw(bw->parent, joe_gettext(_("Not found")));
			ret = -1;
		}
		break;
	case 3:
		msgnw(bw->parent, joe_gettext(_("Infinite loop aborted: your search repeatedly matched same place")));
		ret = -1;
		break;
	case 4:
		ret = -1;
		break;
	case 2:
		if (srch->valid)
			switch (restrict_to_block(bw, srch)) {
			case -1:
				goto again;
			case 1:
				if (srch->addr >= 0)
					pgoto(bw->cursor, srch->addr);
				goto bye;
			}
		srch->addr = bw->cursor->byte;

		/* Make sure found text is fully on screen */
		if(srch->backwards) {
			bw->offset=0;
			pfwrd(bw->cursor,(srch->entire.rm_eo - srch->entire.rm_so));
			bw->cursor->xcol = piscol(bw->cursor);
			dofollows();
			pbkwd(bw->cursor,(srch->entire.rm_eo - srch->entire.rm_so));
		} else {
			bw->offset=0;
			pbkwd(bw->cursor,(srch->entire.rm_eo - srch->entire.rm_so));
			bw->cursor->xcol = piscol(bw->cursor);
			dofollows();
			pfwrd(bw->cursor,(srch->entire.rm_eo - srch->entire.rm_so));
		}

		if (srch->replace) {
			if (square)
				bw->cursor->xcol = piscol(bw->cursor);
			if (srch->backwards) {
				pdupown(bw->cursor, &markb, "dopfnext");
				markb->xcol = piscol(markb);
				pdupown(markb, &markk, "dopfnext");
				pfwrd(markk, (srch->entire.rm_eo - srch->entire.rm_so));
				markk->xcol = piscol(markk);
			} else {
				pdupown(bw->cursor, &markk, "dopfnext");
				markk->xcol = piscol(markk);
				pdupown(bw->cursor, &markb, "dopfnext");
				pbkwd(markb, (srch->entire.rm_eo - srch->entire.rm_so));
				markb->xcol = piscol(markb);
			}
			srch->flg = 1;
			if (dopfrepl(bw->parent, -1, srch, notify))
				ret = -1;
			notify = 0;
			srch = 0;
		}
		break;
	}
	bw->cursor->xcol = piscol(bw->cursor);
	dofollows();
	opt_mid = orgmid;
	if (notify)
		*notify = 1;
	if (srch)
		pfsave(bw->parent, srch);
	else
		updall();
	return ret;
}

int pfnext(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (!globalsrch) {	/* Query for search string if there isn't any */
		return pffirst(bw->parent, 0);
	} else {
		SRCH *srch = globalsrch;

		globalsrch = 0;
		srch->addr = bw->cursor->byte;
		if (!srch->wrap_p || srch->wrap_p->b!=bw->b) {
			prm(srch->wrap_p);
			srch->wrap_p = pdup(bw->cursor, "pfnext");
			srch->wrap_p->owner = &srch->wrap_p;
			srch->wrap_flag = 0;
		}
		return dopfnext(bw, setmark(srch), NULL);
	}
}

void save_srch(FILE *f)
{
	if(globalsrch) {
		if(globalsrch->pattern) {
			fprintf(f,"	pattern ");
			emit_string(f,globalsrch->pattern,sLEN(globalsrch->pattern));
			fprintf(f,"\n");
		}
		if(globalsrch->replacement) {
			fprintf(f,"	replacement ");
			emit_string(f,globalsrch->replacement,sLEN(globalsrch->replacement));
			fprintf(f,"\n");
		}
		fprintf(f,"	backwards %d\n",globalsrch->backwards);
		fprintf(f,"	ignore %d\n",globalsrch->ignore);
		fprintf(f,"	replace %d\n",globalsrch->replace);
		fprintf(f,"	block_restrict %d\n",globalsrch->block_restrict);
		fprintf(f,"	regex %d\n",globalsrch->regex);
	}
	fprintf(f,"done\n");
}

void load_srch(FILE *f)
{
	char buf[1024];
	char bf[1024];
	char *pattern = 0;
	char *replacement = 0;
	int backwards = 0;
	int ignore = 0;
	int regex = 0;
	int replace = 0;
	int block_restrict = 0;
	while(fgets(buf,sizeof(buf),f) && zcmp(buf,"done\n")) {
		const char *p=buf;
		parse_ws(&p,'#');
		if(!parse_kw(&p,"pattern")) {
			ptrdiff_t len;
			parse_ws(&p,'#');
			bf[0] = 0;
			len = parse_string(&p,bf,SIZEOF(bf));
			if (len>0)
				pattern = vsncpy(NULL,0,bf,len);
		} else if(!parse_kw(&p,"replacement")) {
			ptrdiff_t len;
			parse_ws(&p,'#');
			bf[0] = 0;
			len = parse_string(&p,bf,SIZEOF(bf));
			if (len>0)
				replacement = vsncpy(NULL,0,bf,len);
		} else if(!parse_kw(&p,"backwards")) {
			parse_ws(&p,'#');
			parse_int(&p,&backwards);
		} else if(!parse_kw(&p,"ignore")) {
			parse_ws(&p,'#');
			parse_int(&p,&ignore);
		} else if(!parse_kw(&p,"regex")) {
			parse_ws(&p,'#');
			parse_int(&p,&ignore);
		} else if(!parse_kw(&p,"replace")) {
			parse_ws(&p,'#');
			parse_int(&p,&replace);
		} else if(!parse_kw(&p,"block_restrict")) {
			parse_ws(&p,'#');
			parse_int(&p,&block_restrict);
		}
	}
	globalsrch = mksrch(pattern,replacement,ignore,backwards,-1,replace,0,0,regex);
	globalsrch->block_restrict = block_restrict;
}
