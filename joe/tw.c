/* 
 *	Text editing windows
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "types.h"

int staen = 0;
int staupd = 0;
int keepup = 0;
int bg_stalin;

/* Move text window */

static void movetw(W *w, ptrdiff_t x, ptrdiff_t y)
{
	BW *bw = (BW *)w->object;
	TW *tw = (TW *)bw->object;

	if ((y || !staen) && w->h > 1) {
		if (!tw->staon) {	/* Scroll down and shrink */
			nscrldn(bw->parent->t->t, y, bw->parent->nh + y, 1);
		}
		bwmove(bw, x + bw->lincols, y + 1);
		tw->staon = 1;
	} else {
		if (tw->staon) {	/* Scroll up and grow */
			nscrlup(bw->parent->t->t, y, bw->parent->nh + y, 1);
		}
		bwmove(bw, x + bw->lincols, y);
		tw->staon = 0;
	}
}

/* Resize text window */

static void resizetw(W *w, ptrdiff_t wi, ptrdiff_t he)
{
	BW *bw = (BW *)w->object;
	if ((bw->parent->ny || !staen) && he > 1)
		bwresz(bw, wi - bw->lincols, he - 1);
	else
		bwresz(bw, wi - bw->lincols, he);
}

/* Get current context */

/* Use context.jsf to determine context.  The advantage of this vs.  the old
 * method is that the line attribute cache will help find context line
 * quickly in large files.
 */

static struct high_syntax *context_syntax;

static const int *get_context(BW *bw)
{
	static int buf1[SAVED_SIZE*2]; /* Double size because we replace \ with \\ */
	const int *src;
	P *p;
	struct lattr_db *db;
	HIGHLIGHT_STATE st;
	clear_state(&st);
	p = pdup(bw->cursor, "get_context");
	p_goto_bol(p);
	if (!context_syntax)
		context_syntax = load_syntax("context");
	if (context_syntax) {
		db = find_lattr_db(bw->b, context_syntax);
		if (db) {
			st = lattr_get(db, context_syntax, p, p->line + 1);
			/* Handles last line better */
			/* st = parse(context_syntax, p, st, p->b->o.charmap); */
		}
	}
	prm(p);
	src = st.saved_s;
	buf1[0] = 0;
	if (src) {
		ptrdiff_t i, j, spc;
		/* replace tabs to spaces and remove adjoining spaces */
		for (i=0,j=0,spc=0; src[i] && i < SAVED_SIZE-1; i++) {
			if (src[i]=='\t' || src[i]==' ') {
				if (spc) continue;
				spc = 1;
			}
			else spc = 0;
			if (src[i]=='\t')
				buf1[j++] = ' ';
			else if (src[i]=='\\') {
				buf1[j++] = '\\';
				buf1[j++] = '\\';
			} else if (src[i] != '\r') {
				buf1[j++] = src[i];
			}
		}
		buf1[j]= '\0';
	}
	return buf1;
}

/* Find first line (going backwards) which has 0 indentation level
 * and is not a comment, blank, or block structuring line.  This is
 * likely to be the line with the function name.
 *
 * There are actually two possibilities:
 *
 * We want the first line- 
 *
 * int
 * foo(int x,int y) {
 *
 *   }
 *
 * We want the last line-
 *
 * program foo(input,output);
 * var a, b, c : real;
 * begin
 *
 */

#if 0
static char *get_context(BW *bw)
{
	P *p = pdup(bw->cursor, "get_context");
	P *q = pdup(p, "get_context");
	char buf[6];
	char *buf1;
	int i, spc, c;

	buf1 = vsmk(128);
	/* Find first line with 0 indentation which is not a comment line */
	do {
		p_goto_bol(p);
		if (!pisindent(p) && !pisblank(p)) {
			/* next: */
			/* Somewhat inefficient, but better than hitting a long line with brlinevs */
			pset(q, p);
			for (i = 0; i < (sizeof(buf) - 1) && NO_MORE_DATA != (c = pgetb(q)); i++) {
				buf[i] = c;
			}
			buf[i] = 0;
			/* Ignore comment and block structuring lines */
			if (!(buf[0]=='{' ||
			    (buf[0]=='/' && buf[1]=='*') ||
			    buf[0]=='\f' ||
			    (buf[0]=='/' && buf[1]=='/') ||
			    buf[0]=='#' ||
			    (buf[0]=='b' && buf[1]=='e' && buf[2]=='g' && buf[3]=='i' && buf[4]=='n') ||
			    (buf[0]=='B' && buf[1]=='E' && buf[2]=='G' && buf[3]=='I' && buf[4]=='N') ||
			    (buf[0]=='-' && buf[1]=='-') ||
			    buf[0]==';')) {
 				/* replace tabs to spaces and remove adjoining spaces */
				pset(q, p);
 				buf1 = vstrunc(buf1, 0);
 				for (i=0,spc=0; i < 1024 && !piseol(q); i++) {
					c = pgetb(q);

 					if (c=='\t' || c==' ') {
 						if (spc) continue;
 						spc = 1;
 					}
 					else spc = 0;
 					if (c=='\t')
 						buf1 = vsadd(buf1, ' ');
					else if (c=='\\') {
						buf1 = vsadd(buf1, '\\');
						buf1 = vsadd(buf1, '\\');
					} else
						buf1 = vsadd(buf1, c);
 				}
				/* Uncomment to get the last line instead of the first line (see above)
			    	if (pprevl(p)) {
			    		p_goto_bol(p);
			    		if (!pisindent(p) && !pisblank(p))
			    			goto next;
			    	}
			    	*/
			    	break;
			}
			
		}
	} while (!buf1[0] && pprevl(p));

	prm(p);
	prm(q);

	return buf1;
}
#endif

char *duplicate_backslashes(const char *s, ptrdiff_t len)
{
	char *m;
	ptrdiff_t x, count;
	for (x = count = 0; x != len; ++x)
		if (s[x] == '\\')
			++count;
	m = vsmk(len + count);
	for (x = 0; x != len; ++x) {
		m = vsadd(m, s[x]);
		if (s[x] == '\\')
			m = vsadd(m, '\\');
	}
	return m;
}

char *stagen(char *stalin, BW *bw, const char *s, char fill)
{
	char buf[80];
	int x;
	int field;
	W *w = bw->parent;
	time_t n=time(NULL);
	struct tm *cas;
	cas=localtime(&n);

	stalin = vstrunc(stalin, 0);
	obj_perm(stalin);
	while (*s) {
		if (*s == '%' && s[1]) {
			field = 0;
			++s;
			while (*s >= '0' && *s <= '9' && s[1]) {
				field = field * 10 + *s - '0';
				++s;
			}
			switch (*s) {
			case 'v': /* Version of JOE */
				{
					joe_snprintf_1(buf, SIZEOF(buf), "%s", VERSION);
					stalin = vsncpy(sv(stalin), sz(buf));
				}
				break;

			case 'x': /* Context (but only if autoindent is enabled) */
				{
					if (bw->o.title) {
						const int *ts = get_context(bw);
						if (ts) {
							/* We need to translate between file's character set to
							   locale */
							char *t = my_iconv1(NULL, locale_map, ts);
							stalin = vscat(stalin, sv(t));
						}
					}
				}
				break;

			case 'y':
				{
					if (bw->o.syntax) {
						joe_snprintf_1(buf, SIZEOF(buf), "(%s)", bw->o.syntax->name);
						stalin = vsncpy(sv(stalin), sz(buf));
					}
				}
				break;
			case 't':
				{
					time_t curtime = time(NULL);
					int l;
					char *d = ctime(&curtime);

					l = (d[11] - '0') * 10 + d[12] - '0';
					if (l > 12)
						l -= 12;
					joe_snprintf_1(buf, SIZEOF(buf), "%2.2d", l);
					if (buf[0] == '0')
						buf[0] = fill;
					stalin = vsncpy(sv(stalin), buf, 2);
					stalin = vsncpy(sv(stalin), d + 13, 3);
				}
				break;
			    case 'd':
				{
					if (s[1]) switch (*++s) {
						case 'd' : joe_snprintf_1(buf, SIZEOF(buf), "%02d",cas->tm_mday); break;
						case 'm' : joe_snprintf_1(buf, SIZEOF(buf), "%02d",cas->tm_mon + 1); break;
						case 'y' : joe_snprintf_1(buf, SIZEOF(buf), "%02d",cas->tm_year % 100); break;
						case 'Y' : joe_snprintf_1(buf, SIZEOF(buf), "%04d",cas->tm_year + 1900); break;
						case 'w' : joe_snprintf_1(buf, SIZEOF(buf), "%d",cas->tm_wday); break;
						case 'D' : joe_snprintf_1(buf, SIZEOF(buf), "%03d",cas->tm_yday); break;
						default : buf[0]='d'; buf[1]=*s; buf[2]=0;
					}
					else { buf[0]='d'; buf[1]=0;}
						stalin=vsncpy(sv(stalin),sz(buf));
				}
				break;

				case 'E':
				{
					int l;
					char *ch;
					buf[0]=0;
					for(l=0;s[l+1] && s[l+1] != '%'; l++) buf[l]=s[l+1];
					if (s[l+1]=='%' && buf[0]) {
						buf[l]=0;
						s+=l+1;
						ch=getenv(buf);
						if (ch) stalin=vsncpy(sv(stalin),sz(ch));
					} 
				}
				break;

			case 'Z':
				{
					const char *ch;
					int l;
					buf[0]=0;
					for(l=0;s[l+1] && s[l+1] != '%'; l++) buf[l]=s[l+1];
					if (s[l+1]=='%' && buf[0]) {
						buf[l]=0;
						s+=l+1;
						ch=get_status(bw, buf);
						if (ch) stalin=vsncpy(sv(stalin),sz(ch));
					} 
				}
				break;

			case 'u':
				{
					time_t curtime = time(NULL);
					char *d = ctime(&curtime);

					stalin = vsncpy(sv(stalin), d + 11, 5);
				}
				break;
			case 'T':
				if (bw->o.overtype)
					stalin = vsadd(stalin, 'O');
				else
					stalin = vsadd(stalin, 'I');
				break;
			case 'W':
				if (bw->o.wordwrap)
					stalin = vsadd(stalin, 'W');
				else
					stalin = vsadd(stalin, fill);
				break;
			case 'I':
				if (bw->o.autoindent)
					stalin = vsadd(stalin, 'A');
				else
					stalin = vsadd(stalin, fill);
				break;
			case 'X':
				if (square)
					stalin = vsadd(stalin, 'X');
				else
					stalin = vsadd(stalin, fill);
				break;
			case 'n':
				{
				if (bw->b->name) {
					char *tmp = simplify_prefix(bw->b->name);
					char *tmp1 = duplicate_backslashes(sv(tmp));
					stalin = vsncpy(sv(stalin), sv(tmp1));
				} else {
					stalin = vsncpy(sv(stalin), sz(joe_gettext(_("Unnamed"))));
				}
				}
				break;
			case 'm':
				if (bw->b->changed)
					stalin = vsncpy(sv(stalin), sz(joe_gettext(_("(Modified)"))));
				break;
			case 'R':
				if (bw->b->rdonly)
					stalin = vsncpy(sv(stalin), sz(joe_gettext(_("(Read only)"))));
				break;
			case '*':
				if (bw->b->changed)
					stalin = vsadd(stalin, '*');
				else
					stalin = vsadd(stalin, fill);
				break;
			case 'r':
				if (field)
#ifdef HAVE_LONG_LONG
					joe_snprintf_1(buf, SIZEOF(buf), "%-4lld", (long long)(bw->cursor->line + 1));
#else
					joe_snprintf_1(buf, SIZEOF(buf), "%-4ld", (long)(bw->cursor->line + 1));
#endif
				else
#ifdef HAVE_LONG_LONG
					joe_snprintf_1(buf, SIZEOF(buf), "%lld", (long long)(bw->cursor->line + 1));
#else
					joe_snprintf_1(buf, SIZEOF(buf), "%ld", (long)(bw->cursor->line + 1));
#endif
				for (x = 0; buf[x]; ++x)
					if (buf[x] == ' ')
						buf[x] = fill;
				stalin = vsncpy(sv(stalin), sz(buf));
				break;
			case 'o':
				if (field)
#ifdef HAVE_LONG_LONG
					joe_snprintf_1(buf, SIZEOF(buf), "%-4lld", (long long)bw->cursor->byte);
#else
					joe_snprintf_1(buf, SIZEOF(buf), "%-4ld", (long)bw->cursor->byte);
#endif
				else
#ifdef HAVE_LONG_LONG
					joe_snprintf_1(buf, SIZEOF(buf), "%lld", (long long)bw->cursor->byte);
#else
					joe_snprintf_1(buf, SIZEOF(buf), "%ld", (long)bw->cursor->byte);
#endif
				for (x = 0; buf[x]; ++x)
					if (buf[x] == ' ')
						buf[x] = fill;
				stalin = vsncpy(sv(stalin), sz(buf));
				break;
			case 'O':
				if (field)
#ifdef HAVE_LONG_LONG
					joe_snprintf_1(buf, SIZEOF(buf), "%-4llX", (unsigned long long)bw->cursor->byte);
#else
					joe_snprintf_1(buf, SIZEOF(buf), "%-4lX", (unsigned long)bw->cursor->byte);
#endif
				else
#ifdef HAVE_LONG_LONG
					joe_snprintf_1(buf, SIZEOF(buf), "%llX", (unsigned long long)bw->cursor->byte);
#else
					joe_snprintf_1(buf, SIZEOF(buf), "%lX", (unsigned long)bw->cursor->byte);
#endif
				for (x = 0; buf[x]; ++x)
					if (buf[x] == ' ')
						buf[x] = fill;
				stalin = vsncpy(sv(stalin), sz(buf));
				break;
			case 'a':
				if (!piseof(bw->cursor))
					joe_snprintf_1(buf, SIZEOF(buf), "%3d", brch(bw->cursor));
				else
					joe_snprintf_0(buf, SIZEOF(buf), "   ");
				for (x = 0; buf[x]; ++x)
					if (buf[x] == ' ')
						buf[x] = fill;
				stalin = vsncpy(sv(stalin), sz(buf));
				break;
			case 'A':
				if (field)
					if (!piseof(bw->cursor))
						joe_snprintf_1(buf, SIZEOF(buf), "%2.2X", brch(bw->cursor));
					else
						joe_snprintf_0(buf, SIZEOF(buf), "  ");
				else
					if (!piseof(bw->cursor))
						joe_snprintf_1(buf, SIZEOF(buf), "%x", brch(bw->cursor));
					else
						joe_snprintf_0(buf, SIZEOF(buf), "");
				for (x = 0; buf[x]; ++x)
					if (buf[x] == ' ')
						buf[x] = fill;
				stalin = vsncpy(sv(stalin), sz(buf));
				break;
			case 'c':
				if (field)
#ifdef HAVE_LONG_LONG
					joe_snprintf_1(buf, SIZEOF(buf), "%-3lld", (long long)(piscol(bw->cursor) + 1));
#else
					joe_snprintf_1(buf, SIZEOF(buf), "%-3ld", (long)(piscol(bw->cursor) + 1));
#endif
				else
#ifdef HAVE_LONG_LONG
					joe_snprintf_1(buf, SIZEOF(buf), "%lld", (long long)(piscol(bw->cursor) + 1));
#else
					joe_snprintf_1(buf, SIZEOF(buf), "%ld", (long)(piscol(bw->cursor) + 1));
#endif
				for (x = 0; buf[x]; ++x)
					if (buf[x] == ' ')
						buf[x] = fill;
				stalin = vsncpy(sv(stalin), sz(buf));
				break;
			case 'p':
				if (bw->b->eof->byte >= 1024*1024)
#ifdef HAVE_LONG_LONG
					joe_snprintf_1(buf, SIZEOF(buf), "%3lld", ((long long)bw->cursor->byte >> 10) * 100 / ((long long)bw->b->eof->byte >> 10));
#else
					joe_snprintf_1(buf, SIZEOF(buf), "%3ld", ((long)bw->cursor->byte >> 10) * 100 / ((long)bw->b->eof->byte >> 10));
#endif
				else if (bw->b->eof->byte)
#ifdef HAVE_LONG_LONG
					joe_snprintf_1(buf, SIZEOF(buf), "%3lld", (long long)bw->cursor->byte * 100 / (long long)bw->b->eof->byte);
#else
					joe_snprintf_1(buf, SIZEOF(buf), "%3ld", (long)bw->cursor->byte * 100 / (long)bw->b->eof->byte);
#endif
				else
					joe_snprintf_0(buf, SIZEOF(buf), "100");
				for (x = 0; buf[x]; ++x)
					if (buf[x] == ' ')
						buf[x] = fill;
				stalin = vsncpy(sv(stalin), sz(buf));
				break;
			case 'l':
				if (field)
#ifdef HAVE_LONG_LONG
					joe_snprintf_1(buf, SIZEOF(buf), "%-4lld", (long long)(bw->b->eof->line + 1));
#else
					joe_snprintf_1(buf, SIZEOF(buf), "%-4ld", (long)(bw->b->eof->line + 1));
#endif
				else
#ifdef HAVE_LONG_LONG
					joe_snprintf_1(buf, SIZEOF(buf), "%lld", (long long)(bw->b->eof->line + 1));
#else
					joe_snprintf_1(buf, SIZEOF(buf), "%ld", (long)(bw->b->eof->line + 1));
#endif
				for (x = 0; buf[x]; ++x)
					if (buf[x] == ' ')
						buf[x] = fill;
				stalin = vsncpy(sv(stalin), sz(buf));
				break;
			case 'k':
				{
					ptrdiff_t i;
					char *mycpos = buf;

					buf[0] = 0;
					if (w->kbd->x && w->kbd->seq[0])
						for (i = 0; i != w->kbd->x; ++i) {
							char c = w->kbd->seq[i] & 127;

							if (c < 32) {
								mycpos[0] = '^';
								mycpos[1] = (char)(c + '@');
								mycpos += 2;
							} else if (c == 127) {
								mycpos[0] = '^';
								mycpos[1] = '?';
								mycpos += 2;
							} else {
								mycpos[0] = c;
								mycpos += 1;
							}
						}
					*mycpos++ = fill;
					while (mycpos - buf < 4)
						*mycpos++ = fill;
					stalin = vsncpy(sv(stalin), buf, mycpos - buf);
				}
				break;
			case 'S':
				if (bw->b->pid)
					stalin = vsncpy(sv(stalin), sz(joe_gettext(_("*SHELL*"))));
				break;
			case 'M':
				if (recmac) {
					joe_snprintf_1(buf, SIZEOF(buf), joe_gettext(_("(Macro %d recording...)")), recmac->n);
					stalin = vsncpy(sv(stalin), sz(buf));
				}
				break;
			case 'e':
				stalin = vsncpy(sv(stalin), sz(bw->b->o.charmap->name));
				break;
			case 'b':
				stalin = vsncpy(sv(stalin), sz(locale_map->name));
				break;
			case 'w':
				if (!piseof(bw->cursor)) {
					joe_snprintf_1(buf, SIZEOF(buf), "%d", joe_wcwidth(bw->o.charmap->type, brch(bw->cursor)));
					stalin = vsncpy(sv(stalin), sz(buf));
				}
				break;
			default:
				stalin = vsadd(stalin, *s);
			}
		} else
			stalin = vsadd(stalin, *s);
		++s;
	}
	return stalin;
}

static void disptw(W *w, int flg)
{
	BW *bw = (BW *)w->object;
	TW *tw = (TW *)bw->object;
	int newcols = calclincols(bw);
	int linchg = 0;

	if (bw->lincols != newcols) {
		bw->lincols = newcols;
		resizetw(w, w->w, w->h);
		movetw(w, w->x, w->y);
		bwfllw(w);
		linchg = 1;
	}

	if (bw->o.hex) {
		w->cury = TO_DIFF_OK((bw->cursor->byte-bw->top->byte)/16 + bw->y - w->y);
		w->curx = TO_DIFF_OK((bw->cursor->byte-bw->top->byte)%16 + 60 - bw->offset);
	} else {
		w->cury = TO_DIFF_OK(bw->cursor->line - bw->top->line + bw->y - w->y);
		w->curx = TO_DIFF_OK(bw->cursor->xcol - bw->offset + newcols);
	}

	if ((staupd || keepup || bw->cursor->line != tw->prevline || bw->b->changed != tw->changed || bw->b != tw->prev_b) && (w->y || !staen) && w->h > 1) {
		char fill;

		tw->prevline = bw->cursor->line;
		tw->changed = bw->b->changed;
		tw->prev_b = bw->b;
		if (bw->o.rmsg[0])
			fill = bw->o.rmsg[0];
		else
			fill = ' ';
		tw->stalin = stagen(tw->stalin, bw, bw->o.lmsg, fill);
		tw->staright = stagen(tw->staright, bw, bw->o.rmsg, fill);
		if (fmtlen(tw->staright) < w->w) {
			ptrdiff_t x = fmtpos(tw->stalin, w->w - fmtlen(tw->staright));

			if (x > vslen(tw->stalin))
				tw->stalin = vsfill(sv(tw->stalin), fill, x - vslen(tw->stalin));
			tw->stalin = vsncpy(tw->stalin, fmtpos(tw->stalin, w->w - fmtlen(tw->staright)), sv(tw->staright));
		}
		tw->stalin = vstrunc(tw->stalin, fmtpos(tw->stalin, w->w));
		genfmt(w->t->t, w->x, w->y, 0, tw->stalin, bg_stalin, 0, 0);
		w->t->t->updtab[w->y] = 0;
	}

	if (flg) {
		if (bw->o.hex)
			bwgenh(bw);
		else
			bwgen(bw, bw->o.linums, linchg);
	}
}

/* Split current window */

static void iztw(TW *tw, ptrdiff_t y)
{
	tw->stalin = NULL;
	tw->staright = NULL;
	tw->changed = -1;
	tw->prevline = -1;
	tw->staon = (!staen || y);
	tw->prev_b = 0;
}

int usplitw(W *w, int k)
{
	BW *bw;
	ptrdiff_t newh = getgrouph(w);
	W *neww;
	TW *newtw;
	BW *newbw;
	WIND_BW(bw, w);

	dostaupd = 1;
	if (newh / 2 < FITHEIGHT)
		return -1;
	neww = wcreate(w->t, w->watom, findbotw(w), NULL, w, newh / 2 + (newh & 1), NULL);
	if (!neww)
		return -1;
//	wfit(neww->t);
	neww->object = (void *) (newbw = bwmk(neww, bw->b, 0, NULL));
	++bw->b->count;
	newbw->offset = bw->offset;
	newbw->object = (void *) (newtw = (TW *) joe_malloc(SIZEOF(TW)));
	iztw(newtw, neww->y);
	pset(newbw->top, bw->top);
	pset(newbw->cursor, bw->cursor);
	newbw->cursor->xcol = bw->cursor->xcol;
	neww->t->curwin = neww;
	wfit(neww->t);
	return 0;
}

int uduptw(W *w, int k)
{
	BW *bw;
	ptrdiff_t newh = getgrouph(w);
	W *neww;
	TW *newtw;
	BW *newbw;
	WIND_BW(bw, w);

	dostaupd = 1;
	neww = wcreate(w->t, w->watom, findbotw(w), NULL, NULL, newh, NULL);
	if (!neww)
		return -1;
	if (demotegroup(w))
		neww->t->topwin = neww;
	neww->object = (void *) (newbw = bwmk(neww, bw->b, 0, NULL));
	++bw->b->count;
	newbw->offset = bw->offset;
	newbw->object = (void *) (newtw = (TW *) joe_malloc(SIZEOF(TW)));
	iztw(newtw, neww->y);
	pset(newbw->top, bw->top);
	pset(newbw->cursor, bw->cursor);
	newbw->cursor->xcol = bw->cursor->xcol;
	neww->t->curwin = neww;
	wfit(w->t);
	return 0;
}

static void instw(W *w, B *b, off_t l, off_t n, int flg)
{
	BW *bw = (BW *)w->object;
	if (b == bw->b)
		bwins(bw, l, n, flg);
}

static void deltw(W *w, B *b, off_t l, off_t n, int flg)
{
	BW *bw = (BW *)w->object;
	if (b == bw->b)
		bwdel(bw, l, n, flg);
}

WATOM watomtw = {
	"main",
	disptw,
	bwfllw,
	NULL,
	rtntw,
	utypew,
	resizetw,
	movetw,
	instw,
	deltw,
	TYPETW
};

int abortit(W *w, int k)
{
	BW *bw;
	TW *tw;
	B *b;
	
	WIND_BW(bw, w);
	if (bw->parent->watom != &watomtw)
		return wabort(bw->parent);
	if (bw->b->pid && bw->b->count==1)
		return ukillpid(bw->parent, 0);
	w = bw->parent;
	tw = (TW *) bw->object;
	/* If only one main window on the screen... */
	if (countmain(w->t) == 1)
		/* Replace it with an orphaned buffer if there are any */
		if ((b = borphan()) != NULL) {
			void *object = bw->object;
			/* FIXME: Shouldn't we wabort() and wcreate here to kill
			   any prompt windows? */

			bwrm(bw);
			w->object = (void *) (bw = bwmk(w, b, 0, NULL));
			wredraw(bw->parent);
			bw->object = object;
			bw = (BW *)w->object;
			bw->cursor->xcol = piscol(bw->cursor);
			return 0;
		}
	bwrm(bw);
	obj_free(tw->stalin);
	joe_free(tw);
	w->object = NULL;
	wabort(w);	/* Eliminate this window and it's children */
#ifdef JOEWIN
	notify_selection();
#endif
	return 0;
}

static B *wpop(BW *bw)
{
	B *b;
	struct bstack *e = bw->parent->bstack;
	b = e->b;
	if (b->oldcur) prm(b->oldcur);
	if (b->oldtop) prm(b->oldtop);
	b->oldcur = e->cursor;
	b->oldtop = e->top;
	bw->parent->bstack = e->next;
	free(e);
	--b->count;
#ifdef JOEWIN
	notify_selection();
	notify_changed_buffer(b);
#endif
	return b;
}

/* Pop, or do nothing if no window on stack */
int upopabort(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (bw->parent->bstack) {
		int rtn;
		B *b = wpop(bw);
		w = bw->parent;
		rtn = get_buffer_in_window(bw, b);
		bw = (BW *)w->object;
		bw->cursor->xcol = piscol(bw->cursor);
		return rtn;
	} else {
		return 0;
	}
}

/* k is last character types which lead to uabort.  If k is -1, it means uabort
   was called internally, and not by the user: which means uabort will not send
   Ctrl-C to process */
int uabort(W *w, int k)
{
	BW *bw;
	
	if (w->watom != &watomtw)
		return wabort(w);
	WIND_BW(bw, w);
	if (markv(0) && markb->b == bw->b) {
		prm(markk);
		markk = 0;
		updall();
		return 0;
	}
	if (bw->parent->bstack) {
		int rtn;
		B *b = wpop(bw);
		w = bw->parent;
		rtn = get_buffer_in_window(bw, b);
		bw = (BW *)w->object;
		bw->cursor->xcol = piscol(bw->cursor);
		return rtn;
	}
	if (bw->b->pid && bw->b->count==1)
		return ukillpid(bw->parent, 0);
	if (bw->b->changed && bw->b->count == 1 && !bw->b->scratch) {
		int c = query(w, sz(joe_gettext(_("Lose changes to this file (y,n,%{abort})? "))), 0);
		if (!yncheck(yes_key, c))
			return -1;
	}
	if (bw->b->count == 1)
		genexmsg(bw, 0, NULL);
	return abortit(w, NO_MORE_DATA);
}

int ucancel(W *w, int k)
{
	if (w->watom != &watomtw) {
		wabort(w);
		return 0;
	} else
		return uabort(w, k);
}

/* Same as above, but only calls genexmsg if nobody else has */

int uabort1(W *w, int k)
{
	BW *bw;
	
	if (w->watom != &watomtw)
		return wabort(w);
	
	bw = (BW *)w->object;
	if (bw->b->pid && bw->b->count==1)
		return ukillpid(bw->parent, 0);
	if (bw->b->changed && bw->b->count == 1 && !bw->b->scratch) {
		int c = query(w, sz(joe_gettext(_("Lose changes to this file (y,n,%{abort})? "))), 0);
		if (!yncheck(yes_key, c))
			return -1;
	}
	if (!exmsg) genexmsg(bw, 0, NULL);
	return abortit(w, NO_MORE_DATA);
}

/* Abort buffer without prompting: just fail if this is last window on buffer */

int uabortbuf(W *w, int k)
{
	BW *bw;
	B *b;

	WIND_BW(bw, w);

	if (bw->b->pid && bw->b->count==1)
		return ukillpid(w, 0);

	if (okrepl(bw))
		return -1;

	if ((b = borphan()) != NULL) {
		void *object = bw->object;

		bwrm(bw);
		w->object = (void *) (bw = bwmk(w, b, 0, NULL));
		wredraw(bw->parent);
		bw->object = object;
		return 0;
	}

	if (bw->b->count == 1)
		genexmsg(bw, 0, NULL);
	return abortit(w, NO_MORE_DATA);
}

/* Kill current window (orphans buffer) */

int utw0(W *w, int k)
{
	BW *bw;
	w = w->main;
	bw = (BW *)w->object;

	if (w->bstack)
		return uabort(w, -1);
	if (countmain(w->t) == 1)
		return -1;
	if (bw->b->count == 1)
		orphit(bw);
	return uabort(w, -1);
}

/* Kill all other windows (orphans buffers) */

int utw1(W *w, int k)
{
	W *starting = w;
	W *mainw = starting->main;
	Screen *t = mainw->t;
	int myyn;

	do {
		myyn = 0;
	      loop:
		do {
			wnext(t);
		} while (t->curwin->main == mainw && t->curwin != starting);
		if (t->curwin->main != mainw) {
			utw0(t->curwin->main, 0);
			myyn = 1;
			goto loop;
		}
	} while (myyn);
	return 0;
}

void setline(B *b, off_t line)
{
	W *w = maint->curwin;

	do {
		if (w->watom->what == TYPETW) {
			BW *bw = (BW *)w->object;

			if (bw->b == b) {
				off_t oline = bw->top->line;

				/* pline(bw->top, line); */
				pline(bw->cursor, line);
				if (!bw->b->err)
					bw->b->err = pdup(bw->cursor, "setline");
				pline(bw->b->err, line);
				if (w->y >= 0 && bw->top->line > oline && bw->top->line - oline < bw->h)
					nscrlup(w->t->t, bw->y, bw->y + bw->h, (int) (bw->top->line - oline));
				else if (w->y >= 0 && bw->top->line < oline && oline - bw->top->line < bw->h)
					nscrldn(w->t->t, bw->y, bw->y + bw->h, (int) (oline - bw->top->line));
				msetI(bw->t->t->updtab + bw->y, 1, bw->h);
			}
		}
	} while ((w = w->link.next) != maint->curwin);
	/* In case error buffer was orphaned */
	if (errbuf == b && b->oldcur) {
		pline(b->oldcur, line);
		if (!b->err)
			b->err = pdup(b->oldcur, ("setline1"));
		pline(b->err, line);
	}
}

/* Create a text window.  It becomes the last window on the screen */

BW *wmktw(Screen *t, B *b)
{
	W *w;
	BW *bw;
	TW *tw;

	w = wcreate(t, &watomtw, NULL, NULL, NULL, t->h, NULL);
	wfit(w->t);
	w->object = (void *) (bw = bwmk(w, b, 0, NULL));
	bw->object = (void *) (tw = (TW *) joe_malloc(SIZEOF(TW)));
	iztw(tw, w->y);
#ifdef JOEWIN
	notify_new_buffer(b);
#endif
	return bw;
}
