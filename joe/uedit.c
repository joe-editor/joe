/*
 *	Basic user edit functions
 *	Copyright
 * 		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "types.h"

/***************/
/* Global options */
int pgamnt = -1;		/* No. of PgUp/PgDn lines to keep */

/*
 * Move cursor to beginning of line
 */
int u_goto_bol(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (bw->o.hex) {
		pbkwd(bw->cursor,bw->cursor->byte%16);
	} else {
		p_goto_bol(bw->cursor);
	}
	return 0;
}

/*
 * Move cursor to first non-whitespace character, unless it is
 * already there, in which case move it to beginning of line
 */
int uhome(W *w, int k)
{
	BW *bw;
	P *p;
	WIND_BW(bw, w);

	if (bw->o.hex) {
		return u_goto_bol(w, 0);
	}

	p = pdup(bw->cursor, "uhome");

	if (bw->o.indentfirst) {
		if ((bw->o.smarthome) && (piscol(p) > pisindent(p))) {
			p_goto_bol(p);
			while (joe_isblank(p->b->o.charmap,brc(p)))
				pgetc(p);
		} else
			p_goto_bol(p);
	} else {
		if (bw->o.smarthome && piscol(p)==0 && pisindent(p)) {
			while (joe_isblank(p->b->o.charmap,brc(p)))
				pgetc(p);
		} else
			p_goto_bol(p);
	}

	pset(bw->cursor, p);
	prm(p);
	return 0;
}

/*
 * Move cursor to end of line
 */
int u_goto_eol(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (bw->o.hex) {
		if (bw->cursor->byte + 15 - bw->cursor->byte%16 > bw->b->eof->byte)
			pset(bw->cursor,bw->b->eof);
		else
			pfwrd(bw->cursor, 15 - bw->cursor->byte%16);
	} else
		p_goto_eol(bw->cursor);
	return 0;
}

/*
 * Move cursor to beginning of file
 */
int u_goto_bof(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	p_goto_bof(bw->cursor);
	return 0;
}

/*
 * Move cursor to end of file
 */
int u_goto_eof(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (bw->b->vt && bw->b->pid) {
		pset(bw->cursor, bw->b->vt->vtcur);
	} else {
		p_goto_eof(bw->cursor);
	}
	return 0;
}

/*
 * Move cursor left
 */
int u_goto_left(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (bw->o.hex) {
		if (prgetb(bw->cursor) != NO_MORE_DATA) {
			return 0;
		} else {
			return -1;
		}
	}
	if (bw->o.picture) {
		if (bw->cursor->xcol) {
			--bw->cursor->xcol;
			pcol(bw->cursor,bw->cursor->xcol);
			return 0;
		} else
			return -1;
	} else {
		/* Have to do ECHKXCOL here because of picture mode */
		if (bw->cursor->xcol != piscol(bw->cursor)) {
			bw->cursor->xcol = piscol(bw->cursor);
			return 0;
		} else if (prgetc(bw->cursor) != NO_MORE_DATA) {
			bw->cursor->xcol = piscol(bw->cursor);
			return 0;
		} else {
			return -1;
		}
	}
}

/*
 * Move cursor right
 */
int u_goto_right(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (bw->o.hex) {
		if (pgetb(bw->cursor) != NO_MORE_DATA) {
			return 0;
		} else {
			return -1;
		}
	}
	if (bw->o.picture) {
		++bw->cursor->xcol;
		pcol(bw->cursor,bw->cursor->xcol);
		return 0;
	} else {
		int rtn;
		if (pgetc(bw->cursor) != NO_MORE_DATA) {
			bw->cursor->xcol = piscol(bw->cursor);
			rtn = 0;
		} else {
			rtn = -1;
		}
		/* Have to do EFIXXCOL here because of picture mode */
		if (bw->cursor->xcol != piscol(bw->cursor))
			bw->cursor->xcol = piscol(bw->cursor);
		return rtn;
	}
}

/*
 * Move cursor to beginning of previous word or if there isn't
 * previous word then go to beginning of the file
 *
 * WORD is a sequence non-white-space characters
 */
static int p_goto_prev(P *ptr)
{
	P *p = pdup(ptr, "p_goto_prev");
	struct charmap *map=ptr->b->o.charmap;
	int c = prgetc(p);

	if (joe_isalnum_(map,c)) {
		while (joe_isalnum_(map,(c=prgetc(p))))
			/* Do nothing */;
		if (c != NO_MORE_DATA)
			pgetc(p);
	} else if (joe_isspace(map,c) || joe_ispunct(map,c)) {
		while ((c=prgetc(p)), (joe_isspace(map,c) || joe_ispunct(map,c)))
			/* Do nothing */;
		while(joe_isalnum_(map,(c=prgetc(p))))
			/* Do nothing */;
		if (c != NO_MORE_DATA)
			pgetc(p);
	}
	pset(ptr, p);
	prm(p);
	return 0;
}

int u_goto_prev(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	return p_goto_prev(bw->cursor);
}

/*
 * Move cursor to end of next word or if there isn't
 * next word then go to end of the file
 *
 * WORD is a sequence non-white-space characters
 */
static int p_goto_next(P *ptr)
{
	P *p = pdup(ptr, "p_goto_next");
	struct charmap *map=ptr->b->o.charmap;
	int c = brch(p);
	int rtn = -1;

	if (joe_isalnum_(map,c)) {
		rtn = 0;
		while (joe_isalnum_(map,(c = brch(p))))
			pgetc(p);
	} else if (joe_isspace(map,c) || joe_ispunct(map,c)) {
		while (joe_isspace(map, (c = brch(p))) || joe_ispunct(map,c))
			pgetc(p);
		while (joe_isalnum_(map,(c = brch(p)))) {
			rtn = 0;
			pgetc(p);
		}
	} else
		pgetc(p);
	pset(ptr, p);
	prm(p);
	return rtn;
}

int u_goto_next(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	return p_goto_next(bw->cursor);
}

static P *pboi(P *p)
{
	p_goto_bol(p);
	while (joe_isblank(p->b->o.charmap,brc(p)))
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
	q = pdup(p, "pisedge");
	pboi(q);
	if (q->byte == p->byte)
		goto left;
	if (joe_isblank(p->b->o.charmap,(c = brc(p)))) {
		pset(q, p);
		if (joe_isblank(p->b->o.charmap,prgetc(q)))
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

int upedge(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (prgetc(bw->cursor) == NO_MORE_DATA)
		return -1;
	while (pisedge(bw->cursor) != -1)
		prgetc(bw->cursor);
	return 0;
}

int unedge(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (pgetc(bw->cursor) == NO_MORE_DATA)
		return -1;
	while (pisedge(bw->cursor) != 1)
		pgetc(bw->cursor);
	return 0;
}

/* Move cursor to matching delimiter */
/*
 * begin end
 *
 * module endmodule
 *
 * function endfunction
 *
 * <word </word
 *
 * if elif else fi
 *
 * do done
 *
 * case esac, endcase
 *
 * #if #ifdef #ifndef #elseif #else #endif
 *
 * `ifdef  `ifndef  `else `endif
 *
 */

/* A delimiter set list is a : separated list of delimiter sets.
   A delimiter set is two or more groups of matching delimiters.
   A group is a list of equivalent delimiters separated with |.

   For example, here is a delimiter set list, with three sets:
   	"case|casex|casez=endcase:begin=end:if=elif=else=fi:

   In the first delimiter set: "case," "casex" and "casez" all match with
   "endcase." In the third set: "if" matches with "elif," which matches with
   "else," which finally matches with "fi".

   The search goes forward if the delimiter matches any words of any group
   but the last of the set.  If the delimiter matches a word in the last
   group, the search goes backward to the first delimiter.

   Delimiter sets lists are now UTF-8.
*/

/* Return pointer to first matching set in delimiter set list.  Returns NULL
   if no matches were found. */

static const char *next_set(const char *set)
{
	while (*set && *set!=':')
		++set;
	if (*set==':')
		++set;
	return set;
}

static const char *next_group(const char *group)
{
	while (*group && *group!='=' && *group!=':')
		++group;
	if (*group=='=')
		++group;
	return group;
}

static const char *next_word(const char *word)
{
	while (*word && *word!='|' && *word!='=' && *word!=':')
		++word;
	if (*word=='|')
		++word;
	return word;
}

static int match_word(const char *word,const int *s)
{
	while (*s && *word && utf8_decode_fwrd(&word, NULL) == *s++);

	if (!*s && (!*word || *word=='|' || *word=='=' || *word==':'))
		return 1;
	else
		return 0;
}

static int is_in_group(const char *group,const int *s)
{
	while (*group && *group!='=' && *group!=':') {
		if (match_word(group, s))
			return 1;
		else
			group = next_word(group);
	}
	return 0;
}

static int is_in_any_group(const char *group,const int *s)
{
	while (*group && *group!=':') {
		if (match_word(group, s))
			return 1;
		else {
			group = next_word(group);
			if (*group == '=')
				++group;
		}
	}
	return 0;
}

static const char *find_last_group(const char *group)
{
	const char *s;
	for (s = group; *s && *s!=':'; s=next_group(s))
		group = s;
	return group;
}

#define MAX_WORD_SIZE 255

/* Search for matching delimiter: ignore things in comments or strings */

static int tomatch_char_or_word(BW *bw,int word_delimiter,int c,int f,const char *set,const char *group,int backward)
{
	P *p = pdup(bw->cursor, "tomatch_char_or_word");
	P *q = pdup(p, "tomatch_char_or_word");
	const char *last_of_set = "";
	int buf[MAX_WORD_SIZE+1];
	int len;
	int query_highlighter = bw->o.highlighter_context && bw->o.syntax && bw->db;
	int initial_context = 0;
	int col = 0;
	int cnt = 0;	/* No. levels of delimiters we're in */
	int d;
	off_t sod = 0; /* Start of delimiter */

	if (word_delimiter) {
		if (backward) {
			last_of_set = find_last_group(set);
			p_goto_next(p);
			p_goto_prev(p);
		} else {
			last_of_set = find_last_group(group);
			p_goto_next(p);
		}
		pset(q, p);
	}

	if (query_highlighter) {
		col = -1;
		do {
			d = prgetc(q);
			++col;
		} while (d != NO_MORE_DATA && d != '\n');
		if (d != NO_MORE_DATA)
			pgetc(q);
		parse(bw->o.syntax, q, lattr_get(bw->db, bw->o.syntax, q, q->line),bw->o.charmap);
		initial_context = attr_buf[col] & CONTEXT_MASK;
	}

	if (backward) {
		/* Backward search */
		while ((d = prgetc(p)) != NO_MORE_DATA) {
			int peek;
			int peek1;

			if (query_highlighter && d == '\n'){
				pset(q, p);
				col = -1;
				do {
					d = prgetc(q);
					++col;
				} while (d != NO_MORE_DATA && d != '\n');
				if (d != NO_MORE_DATA)
					pgetc(q);
				parse(bw->o.syntax, q, lattr_get(bw->db, bw->o.syntax, q, q->line), bw->o.charmap);
				continue;
			}

			peek = prgetc(p);
			peek1 = 0;
			if(peek != NO_MORE_DATA) {
				peek1 = prgetc(p);
				if (peek1 != NO_MORE_DATA)
					pgetc(p);
				pgetc(p);
			}
			--col;

			if (query_highlighter
			    && (attr_buf[col] & (CONTEXT_COMMENT | CONTEXT_STRING))
			    && (attr_buf[col] & CONTEXT_MASK) != initial_context) {
				/* Ignore */
			} else if (!query_highlighter
			           && (bw->o.cpp_comment || bw->o.pound_comment ||
			               bw->o.semi_comment || bw->o.tex_comment || bw->o.vhdl_comment) && d == '\n') {
				int cc;
				pset(q, p);
				p_goto_bol(q);
				while((cc = pgetc(q)) != '\n') {
					if (cc == '\\') {
						if (pgetc(q) == '\n')
							break;
					} else if (bw->o.pound_comment && cc == '$' && brch(q)=='#') {
						pgetc(q);
					} else if(!bw->o.no_double_quoted && cc=='"') {
						while ((cc = pgetc(q)) != '\n')
							if (cc == '"') break;
							else if (cc == '\\') if ((cc = pgetc(q)) == '\n') break;
						if (cc == '\n')
							break;
					} else if (bw->o.single_quoted && cc == '\'') {
						while((cc = pgetc(q)) != '\n')
							if (cc == '\'') break;
							else if (cc == '\\') if ((cc = pgetc(q)) == '\n') break;
						if (cc == '\n')
							break;
					} else if (bw->o.cpp_comment && cc == '/') {
						if (brch(q)=='/') {
							prgetc(q);
							pset(p,q);
							break;
						}
					} else if (bw->o.vhdl_comment && cc == '-') {
						if (brch(q)=='-') {
							prgetc(q);
							pset(p,q);
							break;
						}
					} else if (bw->o.pound_comment && cc == '#') {
						pset(p,q);
						break;
					} else if (bw->o.semi_comment && cc == ';') {
						pset(p,q);
						break;
					} else if (bw->o.tex_comment && cc == '%') {
						pset(p,q);
						break;
					}
				}
			} else if (peek == '\\' && peek1!='\\') {
				/* Ignore */
			} else if (!query_highlighter && !bw->o.no_double_quoted && d == '"') {
				while((d = prgetc(p)) != NO_MORE_DATA) {
					if (d == '"') {
						d = prgetc(p);
						if (d != '\\') {
							if (d != NO_MORE_DATA)
								pgetc(p);
							break;
						}
					}
				}
			} else if (!query_highlighter && bw->o.single_quoted && d == '\'' && c != '\'' && c != '`') {
				while((d = prgetc(p)) != NO_MORE_DATA)
					if (d == '\'') {
						d = prgetc(p);
						if (d != '\\') {
							if (d != NO_MORE_DATA)
								pgetc(p);
							break;
						}
					}
			} else if (!query_highlighter && bw->o.c_comment && d == '/') {
				d = prgetc(p);
				if (d == '*') {
					d = prgetc(p);
					do {
						do {
							if (d == '*') break;
						} while ((d = prgetc(p)) != NO_MORE_DATA);
						d = prgetc(p);
					} while (d != NO_MORE_DATA && d != '/');
				} else if (d != NO_MORE_DATA)
					pgetc(p);
			} else if (word_delimiter) {
				if (joe_isalnum_(p->b->o.charmap, d)) {
					int x;
					int flg=0;
					P *r;
					len=0;
					while (joe_isalnum_(p->b->o.charmap, d)) {
						if(len!=MAX_WORD_SIZE)
							buf[len++] = d;
						d=prgetc(p);
						--col;
					}
					/* ifdef hack */
					r = pdup(p, "tomatch_char_or_word");
					while (d ==' ' || d=='\t')
						d=prgetc(r);
					/* VHDL hack */
					if ((d=='d' || d=='D') && bw->o.vhdl_comment) {
						d=prgetc(r);
						if(d=='n' || d=='N') {
							d=prgetc(r);
							if(d=='e' || d=='E') {
								d=prgetc(r);
								if(d==' ' || d=='\t' || d=='\n' || d==NO_MORE_DATA)
									flg=1;
							}
						}
					}
					prm(r);
					if (d == utf8_decode_string(set))
						buf[len++] = d;
					if(d!=NO_MORE_DATA)
						pgetc(p);
					++col;
					buf[len]=0;
					for(x=0;x!=len/2;++x) {
						int e = buf[x];
						buf[x] = buf[len-x-1];
						buf[len-x-1] = e;
					}
					if (is_in_group(last_of_set,buf)) {
						++cnt;
					} else if(is_in_group(set,buf) && !flg && !cnt--) {
						pset(bw->cursor,p);
						prm(q);
						prm(p);
						return 0;
					}
				}
			} else if (d == c) {
				++cnt;
			} else if (d == f && !cnt--) {
				pset(bw->cursor, p);
				prm(q);
				prm(p);
				return 0;
			}
		}
	} else {
		/* Forward search */
		while ((sod = p->byte), ((d = pgetc(p)) != NO_MORE_DATA)) {
			if (query_highlighter && d == '\n') {
				parse(bw->o.syntax, q, lattr_get(bw->db, bw->o.syntax, q, q->line), bw->o.charmap);
				col = 0;
				continue;
			}

			if (query_highlighter
			    && (attr_buf[col] & (CONTEXT_COMMENT | CONTEXT_STRING))
			    && (attr_buf[col] & CONTEXT_MASK) != initial_context) {
				/* Ignore */
			} else if (d == '\\') {
				if (!(query_highlighter && brch(p) == '\n')) {
					pgetc(p);
					++col;
				}
			} else if (!query_highlighter && !bw->o.no_double_quoted && d == '"') {
				while ((d = pgetc(p)) != NO_MORE_DATA)
					if (d == '"') break;
					else if (d == '\\') pgetc(p);
			} else if (!query_highlighter && bw->o.single_quoted && d == '\'' && c != '\'' && c != '`') {
				while((d = pgetc(p)) != NO_MORE_DATA)
					if (d == '\'') break;
					else if (d == '\\') pgetc(p);
			} else if (!query_highlighter && d == '$' && brch(p)=='#' && bw->o.pound_comment) {
				pgetc(p);
			} else if (!query_highlighter
			           && ((bw->o.pound_comment && d == '#') ||
				       (bw->o.semi_comment && d == ';') ||
				       (bw->o.tex_comment && d == '%') ||
				       (bw->o.vhdl_comment && d == '-' && brch(p) == '-') ||
				       (bw->o.cpp_comment && d == '/' && brch(p) == '/'))) {
				while ((d = pgetc(p)) != NO_MORE_DATA)
					if (d == '\n')
						break;
			} else if (!query_highlighter && bw->o.c_comment && d == '/' && brch(p) == '*') {
				pgetc(p);
				d = pgetc(p);
				do {
					do {
						if (d == '*') break;
					} while ((d = pgetc(p)) != NO_MORE_DATA);
					d = pgetc(p);
				} while (d != NO_MORE_DATA && d != '/');
			} else if (word_delimiter) {
				int set0 = utf8_decode_string(set);
				if (d == set0) {
					/* ifdef hack */
					len = 0;
					if (!joe_isalnum_(p->b->o.charmap, d)) { /* If it's a # in #ifdef, allow spaces after it */
						sod = p->byte;
						while ((d = pgetc(p))!=NO_MORE_DATA) {
							++col;
							if (d!=' ' && d!='\t')
								break;
							sod = p->byte;
						}
						buf[0] = set0;
						len=1;
					}
					if (joe_isalnum_(p->b->o.charmap, d))
						goto doit;
					if (d!=NO_MORE_DATA) {
						prgetc(p);
						--col;
					}
				} else if (joe_isalpha_(p->b->o.charmap, d)) {
					len=0;
					doit:
					while (joe_isalnum_(p->b->o.charmap, d)) {
						if(len!=MAX_WORD_SIZE)
							buf[len++] = d;
						d=pgetc(p);
						++col;
					}
					if (d!=NO_MORE_DATA) {
						prgetc(p);
						--col;
					}
					buf[len]=0;
					if (is_in_group(set,buf)) {
						++cnt;
					} else if (cnt==0) {
						if (is_in_any_group(group,buf)) {
							pgoto(p, sod);
							pset(bw->cursor,p);
							prm(q);
							prm(p);
							return 0;
						}
					} else if(is_in_group(last_of_set,buf)) {
						/* VHDL hack */
						if (bw->o.vhdl_comment && (match_word("end", buf) || !match_word("END", buf)))
							while((d=pgetc(p))!=NO_MORE_DATA) {
								++col;
								if (d==';' || d=='\n') {
									prgetc(p);
									--col;
									break;
								}
							}
						--cnt;
					}
				}
			} else if (d == c) {
				++cnt;
			} else if (d == f && !--cnt) {
				prgetc(p);
				pset(bw->cursor, p);
				prm(q);
				prm(p);
				return 0;
			}
			++col;
		}
	}
	prm(q);
	prm(p);
	return -1;
}

static int tomatch_char(BW *bw,int c,int f,int dir)
{
	return tomatch_char_or_word(bw, 0, c, f, 0, 0, dir == -1);
}

static int tomatch_word(BW *bw,const char *set,const char *group)
{
	return tomatch_char_or_word(bw, 1, 0, 0, set, group, !*group || *group==':');
}

/* Return true if <foo /> */

static int xml_startend(P *p)
{
	int c, d=0;
	p=pdup(p, "xml_startend");
	while((c=pgetc(p)) != NO_MORE_DATA) {
		if(d=='/' && c=='>') {
			prm(p);
			return 1;
		} else if(c=='>')
			break;
		d=c;
	}
	prm(p);
	return 0;
}

static int tomatch_xml(BW *bw,int *word,int dir)
{
	if (dir== -1) {
		/* Backward search */
		P *p=pdup(bw->cursor, "tomatch_xml");
		int c;
		int buf[MAX_WORD_SIZE+1];
		int len;
		int cnt = 1;
		p_goto_next(p);
		p_goto_prev(p);
		while ((c=prgetc(p)) != NO_MORE_DATA) {
			if (joe_isalnum_(p->b->o.charmap, c) || c == '.' || c == ':' || c == '-') {
				int x;
				len=0;
				while (joe_isalnum_(p->b->o.charmap, c) || c=='.' || c==':' || c == '-') {
					if(len!=MAX_WORD_SIZE)
						buf[len++] = c;
					c=prgetc(p);
				}
				if(c!=NO_MORE_DATA)
					c = pgetc(p);
				buf[len]=0;
				for(x=0;x!=len/2;++x) {
					int d = buf[x];
					buf[x] = buf[len-x-1];
					buf[len-x-1] = d;
				}
				if (!Zcmp(word,buf) && !xml_startend(p)) {
					if (c=='<') {
						if (!--cnt) {
							pset(bw->cursor,p);
							prm(p);
							return 0;
						}
					}
					else if (c=='/') {
						++cnt;
					}
				}
			}
		}
		prm(p);
		return -1;
	} else {
		/* Forward search */
		P *p=pdup(bw->cursor, "tomatch_xml");
		int c;
		int buf[MAX_WORD_SIZE+1];
		int len;
		int cnt = 1;
		off_t sod = 0;
		while ((c=pgetc(p)) != NO_MORE_DATA) {
			if (c == '<') {
				int e = 1;
				sod = p->byte;
				c = pgetc(p);
				if (c=='/') {
					sod = p->byte;
					e = 0;
					c = pgetc(p);
				}
				if (joe_isalpha_(p->b->o.charmap, c) || c==':' || c=='-' || c=='.') {
					len=0;
					while (joe_isalnum_(p->b->o.charmap, c) || c==':' || c=='-' || c=='.') {
						if(len!=MAX_WORD_SIZE)
							buf[len++]=c;
						c=pgetc(p);
					}
					if (c!=NO_MORE_DATA)
						prgetc(p);
					buf[len]=0;
					if (!Zcmp(word,buf) && !xml_startend(p)) {
						if (e) {
							++cnt;
						}
						else if (!--cnt) {
							pgoto(p, sod);
							pset(bw->cursor,p);
							prm(p);
							return 0;
						}
					}
				} else if (c!=NO_MORE_DATA) {
					prgetc(p);
				}
			}
		}
		prm(p);
		return -1;
	}
}

static void get_xml_name(P *p,int *buf)
{
	int c;
	int len=0;
	p=pdup(p, "get_xml_name");
	c=pgetc(p);
	while (joe_isalnum_(p->b->o.charmap, c) || c==':' || c=='-' || c=='.') {
		if(len!=MAX_WORD_SIZE)
			buf[len++]=c;
		c=pgetc(p);
	}
	buf[len]=0;
	prm(p);
}

static void get_delim_name(P *q,int *buf)
{
	int c;
	int len=0;
	P *p=pdup(q, "get_delim_name");
	while ((c=prgetc(p))!=NO_MORE_DATA)
		if (c!=' ' && c!='\t')
			break;
	prm(p);
	/* preprocessor directive hack */
	if (c=='#' || c=='`')
		buf[len++]=c;

	p=pdup(q, "get_delim_name");
	c=pgetc(p);
	while (joe_isalnum_(p->b->o.charmap, c)) {
		if(len!=MAX_WORD_SIZE)
			buf[len++]=c;
		c=pgetc(p);
	}
	buf[len]=0;
	prm(p);
}

int utomatch(W *w, int k)
{
	int d;
	int c,			/* Character under cursor */
	 f,			/* Character to find */
	 dir;			/* 1 to search forward, -1 to search backward */
	BW *bw;
	WIND_BW(bw, w);

	c = brch(bw->cursor);

	/* Check for word delimiters */
	if (joe_isalnum_(bw->cursor->b->o.charmap, c)) {
		P *p;
		int buf[MAX_WORD_SIZE+1];
		char utf8_buf[MAX_WORD_SIZE * 6 + 1]; /* Possibly UTF-8 version of buf */
		int buf1[MAX_WORD_SIZE+1];
		const char *list = bw->b->o.text_delimiters;
		const char *set;
		const char *group;
		const char *word;
		int flg=0;
		p=pdup(bw->cursor, "utomatch");
		p_goto_next(p);
		p_goto_prev(p);
		get_delim_name(p,buf);
		get_xml_name(p,buf1);
		c=prgetc(p);
		if (c=='<')
			flg = 1;
		else if (c=='/') {
			c=prgetc(p);
			if (c=='<')
				flg = -1;
		}
		prm(p);

		if (flg) {
			return tomatch_xml(bw, buf1, flg);
		}

		for (set = list; set && *set; set = next_set(set)) {
			for (group = set; *group && *group!='=' && *group!=':'; group=next_group(group)) {
				for (word = group; *word && *word!='|' && *word!='=' && *word!=':'; word=next_word(word)) {
					if (match_word(word, buf)) {
						return tomatch_word(bw, set, next_group(word));
					}
				}
			}

		}

		/* We don't know the word, so start a search */
		if (bw->b->o.charmap->type) {
			Ztoutf8(utf8_buf, SIZEOF(utf8_buf), buf);
		} else {
			Ztoz(utf8_buf, SIZEOF(utf8_buf), buf);
		}
		return dofirst(bw, 0, 0, utf8_buf);
	}

	switch (c) {
	case '/':
		dir = 1;
		pgetc(bw->cursor);
		f = brch(bw->cursor);
		prgetc(bw->cursor);
		if(f=='*') f = '/';
		else {
			dir = -1;
			f = prgetc(bw->cursor);
			if (f!=NO_MORE_DATA)
				pgetc(bw->cursor);
			if(f=='*') f = '/';
			else
				return -1;
		}
		break;
	case '*':
		dir = -1;
		pgetc(bw->cursor);
		f = brch(bw->cursor);
		prgetc(bw->cursor);
		if(f=='/') f = '*';
		else {
			dir = 1;
			f = prgetc(bw->cursor);
			if (f!=NO_MORE_DATA)
				pgetc(bw->cursor);
			if(f=='/') f = '*';
			else
				return -1;
		}
		break;
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

	/* Search for matching C comment */
	if (f == '/' || f == '*') {
		P *p = pdup(bw->cursor, "utomatch");
		if (dir == 1) {
			d = pgetc(p);
			do {
				do {
					if (d == '*') break;
				} while ((d = pgetc(p)) != NO_MORE_DATA);
				d = pgetc(p);
			} while (d != NO_MORE_DATA && d != '/');
			if (d == '/') {
				if (f == '*') {
					prgetc(p);
				}
				pset(bw->cursor,p);
				prgetc(bw->cursor);
			}
		} else {
			d = prgetc(p);
			do {
				do {
					if (d == '*') break;
				} while ((d = prgetc(p)) != NO_MORE_DATA);
				d = prgetc(p);
			} while (d != NO_MORE_DATA && d != '/');
			if (d == '/') {
				if (f == '*') {
					pgetc(p);
				}
				pset(bw->cursor,p);
			}
		}
		prm(p);
		if (d == NO_MORE_DATA)
			return -1;
		else
			return 0;
	}

	return tomatch_char(bw, c, f, dir);
}

/* Move cursor up */

int uuparw(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (bw->o.hex) {
		if (bw->cursor->byte<16)
			return -1;
		else {
			pbkwd(bw->cursor, 16);
			return 0;
		}
	}
	if (bw->cursor->line) {
		pprevl(bw->cursor);
		pcol(bw->cursor, bw->cursor->xcol);
		return 0;
	} else
		return -1;
}

/* Move cursor down */

int udnarw(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (bw->o.hex) {
		if (bw->cursor->byte+16 <= bw->b->eof->byte) {
			pfwrd(bw->cursor, 16);
			return 0;
		} else if (bw->cursor->byte != bw->b->eof->byte) {
			pset(bw->cursor, bw->b->eof);
			return 0;
		} else {
			return -1;
		}
	}
	if (bw->cursor->line != bw->b->eof->line) {
		pnextl(bw->cursor);
		pcol(bw->cursor, bw->cursor->xcol);
		return 0;
	} else if(bw->o.picture) {
		p_goto_eol(bw->cursor);
		binsc(bw->cursor,'\n');
		pgetc(bw->cursor);
		pcol(bw->cursor, bw->cursor->xcol);
		return 0;
	} else
		return -1;
}

/* Move cursor to top of window */

int utos(W *w, int k)
{
	off_t col;
	BW *bw;
	WIND_BW(bw, w);
	col = bw->cursor->xcol;

	pset(bw->cursor, bw->top);
	pcol(bw->cursor, col);
	bw->cursor->xcol = col;
	return 0;
}

/* Move cursor to bottom of window */

int ubos(W *w, int k)
{
	BW *bw;
	off_t col;
	WIND_BW(bw, w);
	col = bw->cursor->xcol;

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

void scrup(BW *bw, ptrdiff_t n, int flg)
{
	ptrdiff_t scrollamnt = 0;
	ptrdiff_t cursoramnt = 0;
	ptrdiff_t x;

	/* Decide number of lines we're really going to scroll */

	if (bw->o.hex) {
		if (bw->top->byte/16 >= n)
			scrollamnt = cursoramnt = n;
		else if (bw->top->byte/16)
			scrollamnt = cursoramnt = (ptrdiff_t)(bw->top->byte/16);
		else if (flg)
			cursoramnt = (ptrdiff_t)(bw->cursor->byte/16);
		else if (bw->cursor->byte/16 >= n)
			cursoramnt = n;
	} else {
		if (bw->top->line >= n)
			scrollamnt = cursoramnt = n;
		else if (bw->top->line)
			scrollamnt = cursoramnt = (ptrdiff_t)bw->top->line;
		else if (flg)
			cursoramnt = (ptrdiff_t)bw->cursor->line;
		else if (bw->cursor->line >= n)
			cursoramnt = n;
	}

	if (bw->o.hex) {
		/* Move top-of-window pointer */
		pbkwd(bw->top,scrollamnt*16);
		/* Move cursor */
		pbkwd(bw->cursor,cursoramnt*16);
		/* If window is on the screen, give (buffered) scrolling command */
		if (bw->parent->y != -1)
			nscrldn(bw->parent->t->t, bw->y, bw->y + bw->h, scrollamnt);
	} else {
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
}

/* Scroll buffer window down n lines
 * If end of file is close, scrolls as much as possible
 * If end of file is on-screen, cursor jumps to end of file
 *
 * If flg is set: cursor stays fixed relative to screen edge
 * If flg is clr: cursor stays fixed on the buffer line
 */

void scrdn(BW *bw, ptrdiff_t n, int flg)
{
	ptrdiff_t scrollamnt = 0;
	ptrdiff_t cursoramnt = 0;
	ptrdiff_t x;

	/* How much we're really going to scroll... */
	if (bw->o.hex) {
		if (bw->top->b->eof->byte/16 < bw->top->byte/16 + bw->h) {
			cursoramnt = (ptrdiff_t)(bw->top->b->eof->byte/16 - bw->cursor->byte/16);
			if (!flg && cursoramnt > n)
				cursoramnt = n;
		} else if (bw->top->b->eof->byte/16 - (bw->top->byte/16 + bw->h) >= n)
			cursoramnt = scrollamnt = n;
		else
			cursoramnt = scrollamnt = (ptrdiff_t)(bw->top->b->eof->byte/16 - (bw->top->byte/16 + bw->h) + 1);
	} else {
		if (bw->top->b->eof->line < bw->top->line + bw->h) {
			cursoramnt = (ptrdiff_t)(bw->top->b->eof->line - bw->cursor->line);
			if (!flg && cursoramnt > n)
				cursoramnt = n;
		} else if (bw->top->b->eof->line - (bw->top->line + bw->h) >= n)
			cursoramnt = scrollamnt = n;
		else
			cursoramnt = scrollamnt = (ptrdiff_t)(bw->top->b->eof->line - (bw->top->line + bw->h) + 1);
	}

	if (bw->o.hex) {
		/* Move top-of-window pointer */
		pfwrd(bw->top,16*scrollamnt);
		/* Move cursor */
		pfwrd(bw->cursor,16*cursoramnt);
		/* If window is on screen, give (buffered) scrolling command to terminal */
		if (bw->parent->y != -1)
			nscrlup(bw->parent->t->t, bw->y, bw->y + bw->h, scrollamnt);
	} else {
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
}

/* Page up */

int upgup(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (menu_above) {
		if (w->link.prev->watom == &watommenu && w->link.prev->win == w) {
			return umpgup(w->link.prev, 0);
		}
	} else {
		if (w->link.next->watom == &watommenu && w->link.next->win == w) {
			return umpgup(w->link.next, 0);
		}
	}
	bw = (BW *) bw->parent->main->object;

	if (bw->o.hex ? bw->cursor->byte < 16 : !bw->cursor->line)
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

int upgdn(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (menu_above) {
		if (w->link.prev->watom == &watommenu && w->link.prev->win == w) {
			return umpgdn(w->link.prev, 0);
		}
	} else {
		if (w->link.next->watom == &watommenu && w->link.next->win == w) {
			return umpgdn(w->link.next, 0);
		}
	}
	bw = (BW *)bw->parent->main->object;
	if (bw->o.hex ? bw->cursor->byte/16 == bw->b->eof->byte/16 : bw->cursor->line == bw->b->eof->line)
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

int uupslide(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (bw->o.hex ? bw->top->byte/16 : bw->top->line) {
		if (bw->o.hex ? bw->top->byte/16 + bw->h -1 != bw->cursor->byte/16 : bw->top->line + bw->h - 1 != bw->cursor->line)
			udnarw(w, 0);
		scrup(bw, 1, 0);
		return 0;
	} else
		/* was return -1; */
		return uuparw(w, 0);
}

int udnslide(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (bw->o.hex ? bw->top->line/16 + bw->h <= bw->top->b->eof->byte/16 : bw->top->line + bw->h <= bw->top->b->eof->line) {
		if (bw->o.hex ? bw->top->byte/16 != bw->cursor->byte/16 : bw->top->line != bw->cursor->line)
			uuparw(w, 0);
		scrdn(bw, 1, 0);
		return 0;
	} else
		/* was return -1; */
		return udnarw(w, 0);
}

/* Move cursor to specified line number */

static B *linehist = NULL;	/* History of previously entered line numbers */

static int doline(W *w, char *s, void *object, int *notify)
{
	BW *bw;
	off_t num;
	WIND_BW(bw, w);
	num = (off_t)calc(bw, s, 1);

	if (notify)
		*notify = 1;
	vsrm(s);
	if (num >= 1 && !merr) {
		int tmp = opt_mid;

		if (num > bw->b->eof->line)
			num = bw->b->eof->line + 1;
		pline(bw->cursor, num - 1), bw->cursor->xcol = piscol(bw->cursor);
		opt_mid = 1;
		dofollows();
		opt_mid = tmp;
		return 0;
	} else {
		if (merr)
			msgnw(bw->parent, merr);
		else
			msgnw(bw->parent, joe_gettext(_("Invalid line number")));
		return -1;
	}
}

int uline(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (wmkpw(bw->parent, joe_gettext(_("Go to line (%{abort} to abort): ")), &linehist, doline, NULL, NULL, math_cmplt, NULL, NULL, utf8_map, 0))
		return 0;
	else
		return -1;
}

/* Move cursor to specified column number */

static B *colhist = NULL;	/* History of previously entered column numbers */

static int docol(W *w, char *s, void *object, int *notify)
{
	BW *bw;
	off_t num;
	WIND_BW(bw, w);
	num = (off_t)calc(bw, s, 1);

	if (notify)
		*notify = 1;
	vsrm(s);
	if (num >= 1 && !merr) {
		int tmp = opt_mid;

		pcol(bw->cursor, num - 1), bw->cursor->xcol = piscol(bw->cursor);
		opt_mid = 1;
		dofollows();
		opt_mid = tmp;
		return 0;
	} else {
		if (merr)
			msgnw(bw->parent, merr);
		else
			msgnw(bw->parent, joe_gettext(_("Invalid column number")));
		return -1;
	}
}

int ucol(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (wmkpw(bw->parent, joe_gettext(_("Go to column (%{abort} to abort): ")), &colhist, docol, NULL, NULL, math_cmplt, NULL, NULL, utf8_map, 0))
		return 0;
	else
		return -1;
}

/* Move cursor to specified byte number */

static B *bytehist = NULL;	/* History of previously entered byte numbers */

static int dobyte(W *w, char *s, void *object, int *notify)
{
	BW *bw;
	off_t num;
	WIND_BW(bw, w);
	num = (off_t)calc(bw, s, 1);

	if (notify)
		*notify = 1;
	vsrm(s);
	if (num >= 0 && !merr) {
		int tmp = opt_mid;

		pgoto(bw->cursor, num), bw->cursor->xcol = piscol(bw->cursor);
		opt_mid = 1;
		dofollows();
		opt_mid = tmp;
		return 0;
	} else {
		if (merr)
			msgnw(bw->parent, merr);
		else
			msgnw(bw->parent, joe_gettext(_("Invalid byte number")));
		return -1;
	}
}

int ubyte(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (wmkpw(bw->parent, joe_gettext(_("Go to byte (%{abort} to abort): ")), &bytehist, dobyte, NULL, NULL, math_cmplt, NULL, NULL, utf8_map, 0))
		return 0;
	else
		return -1;
}

/* Delete character under cursor
 * or write ^D to process if we're at end of file in a shell window
 */

int udelch(W *w, int k)
{
	BW *bw;
	P *p;
	WIND_BW(bw, w);

	if (piseof(bw->cursor))
		return -1;
	pgetc(p = pdup(bw->cursor, "udelch"));
	bdel(bw->cursor, p);
	prm(p);
	return 0;
}

/* Backspace */

int ubacks(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	/* Don't backspace when at beginning of line in prompt windows */
	if (bw->parent->watom->what == TYPETW || !pisbol(bw->cursor)) {
		int c;
		off_t indent;
		off_t col;
		off_t indwid;
		off_t wid;

		/* Degenerate into ltarw for overtype mode */
		if (bw->o.overtype) {
			return u_goto_left(bw->parent, 0);
		}

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
		   indent characters (or purify indents is enabled). */

		/* Ignore purify for backspace */
		if (col == indent && (col%indwid)==0 && col!=0 && bw->o.smartbacks && bw->o.autoindent) {
			P *p;

			/* Delete all indentation */
			p = pdup(bw->cursor, "ubacks");
			p_goto_bol(p);
			bdel(p,bw->cursor);
			prm(p);

			/* Indent to new position */
			pfill(bw->cursor,col-indwid,bw->o.indentc);
		} else if (col<indent && bw->o.smartbacks && !pisbol(bw->cursor)) {
			/* We're before indent point: delete indwid worth of space but do not
			   cross line boundary.  We could probably replace the above with this. */
			off_t cw=0;
			P *p = pdup(bw->cursor, "ubacks");
			do {
				c = prgetc(bw->cursor);
				if(c=='\t') cw += bw->o.tab;
				else cw += 1;
				bdel(bw->cursor, p);
			} while(!pisbol(bw->cursor) && cw<indwid);
			prm(p);
		} else {
			/* Regular backspace */
			P *p = pdup(bw->cursor, "ubacks");
			if ((c = prgetc(bw->cursor)) != NO_MORE_DATA)
				if (!bw->o.overtype || c == '\t' || pisbol(p) || piseol(p))
					bdel(bw->cursor, p);
			prm(p);
		}
		return 0;
	} else
		return -1;
}

/*
 * Delete sequence of characters (alphabetic, numeric) or (white-space)
 *	if cursor is on the white-space it will delete all white-spaces
 *		until alphanumeric character
 *      if cursor is on the alphanumeric it will delete all alphanumeric
 *		characters until character that is not alphanumeric
 */
int u_word_delete(W *w, int k)
{
	BW *bw;
	P *p;
	struct charmap *map;
	int c;
	WIND_BW(bw, w);

	p = pdup(bw->cursor, "u_word_delete");
	map=bw->b->o.charmap;
	c = brch(p);

	if (joe_isalnum_(map,c))
		while (joe_isalnum_(map,(c = brch(p))))
			pgetc(p);
	else if (joe_isspace(map,c))
		while (joe_isspace(map,(c = brch(p))))
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

int ubackw(W *w, int k)
{
	BW *bw;
	P *p;
	int c;
	struct charmap *map;
	WIND_BW(bw, w);

	p = pdup(bw->cursor, "ubackw");
	c = prgetc(bw->cursor);
	map=bw->b->o.charmap;

	if (joe_isalnum_(map,c)) {
		while (joe_isalnum_(map,(c = prgetc(bw->cursor))))
			/* do nothing */;
		if (c != NO_MORE_DATA)
			pgetc(bw->cursor);
	} else if (joe_isspace(map,c)) {
		while (joe_isspace(map,(c = prgetc(bw->cursor))))
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

int udelel(W *w,int k)
{
	BW *bw;
	P *p;
	WIND_BW(bw, w);
	p = p_goto_eol(pdup(bw->cursor, "udelel"));

	if (bw->cursor->byte == p->byte) {
		prm(p);
		return udelch(w, 0);
	} else
		bdel(bw->cursor, p);
	prm(p);
	return 0;
}

/* Delete to beginning of line, or if there's nothing to delete,
 * delete the line-break
 */

int udelbl(W *w, int k)
{
	BW *bw;
	P *p;
	WIND_BW(bw, w);
	p = p_goto_bol(pdup(bw->cursor, "udelbl"));

	if (p->byte == bw->cursor->byte) {
		prm(p);
		return ubacks(w, 8);	/* The 8 goes to the process if we're at EOF of shell window */
	} else
		bdel(p, bw->cursor);
	prm(p);
	return 0;
}

/* Delete entire line */

int udelln(W *w, int k)
{
	BW *bw;
	P *p;
	WIND_BW(bw, w);
	p = pdup(bw->cursor, "udelln");

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

int uinsc(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	binsc(bw->cursor, ' ');
	return 0;
}

/* Move p backwards to first non-blank line and return its indentation */

static off_t find_indent(P *p)
{
	int x;
	for (x=0; x != 10; ++x) {
		if (!pprevl(p)) return -1;
		p_goto_bol(p);
		if (!pisblank(p)) break;
	}
	if (x==10)
		return -1;
	else
		return pisindent(p);
}

/* Type a character into the buffer (deal with left margin, overtype mode and
 * word-wrap), if cursor is at end of shell window buffer, just send character
 * to process.
 */

static int utypebw_raw(BW *bw, int k, int no_decode)
{
	/* Character map of buffer */
	struct charmap *map = bw->b->o.charmap;


	/* Send data to shell window */
	if ((bw->b->pid && !bw->b->vt && piseof(bw->cursor)) ||
	   ( bw->b->pid && bw->b->vt && bw->cursor->byte == bw->b->vt->vtcur->byte)) {
	   	if (locale_map->type) {
	   		char buf[8];
	   		ptrdiff_t len = utf8_encode(buf, k);
	   		joe_write(bw->b->out, buf, len);
	   	} else {
	   		if (!no_decode) {
		   		k = from_uni(locale_map, k);
			}
	   		if (k != -1) {
	   			char c = TO_CHAR_OK(k);
	   			joe_write(bw->b->out, &c, 1);
	   		}
		}
		return 0;
	}

	/* Hex mode overtype needs to preserve file size */
	if (bw->o.hex && bw->o.overtype) {
		char buf[8];
		ptrdiff_t x;
		ptrdiff_t len;
		if (map->type) {
			len = utf8_encode(buf, k);
		} else {
			if (!no_decode)
				k = from_uni(map, k);
			if (k == -1)
				return 1;
			buf[0] = TO_CHAR_OK(k);
			len = 1;
		}
		binsm(bw->cursor, buf, len);
		for (x = 0; x != len; ++x)
			pgetb(bw->cursor);
		while (len--) {
			P *p;
			if (piseof(bw->cursor))
				return 0;
			p = pdup(bw->cursor, "utypebw_raw");
			pgetb(p);
			bdel(bw->cursor, p);
			prm(p);
		}
		return 0;
	}

	if (k == '\t' && bw->o.overtype && !piseol(bw->cursor) && !no_decode) { /* TAB in overtype mode is supposed to be just cursor motion */
		off_t col = bw->cursor->xcol;		/* Current cursor column */
		col = col + bw->o.tab - (col%bw->o.tab);/* Move to next tab stop */
		pcol(bw->cursor,col);			/* Try to position cursor there */
		if (!bw->o.picture && piseol(bw->cursor) && piscol(bw->cursor)<col) {	/* We moved past end of line, insert a tab (unless in picture mode) */
			if (bw->o.spaces)
				pfill(bw->cursor,col,' ');
			else
				pfill(bw->cursor,col,'\t');
		}
		bw->cursor->xcol = col;			/* Put cursor there even if we can't really go there */
	} else if (k == '\t' && bw->o.smartbacks && bw->o.autoindent && pisindent(bw->cursor)>=piscol(bw->cursor) && !no_decode) {
		P *p = pdup(bw->cursor, "utypebw_raw");
		off_t n = find_indent(p);
		if (n != -1 && pisindent(bw->cursor)==piscol(bw->cursor) && n > pisindent(bw->cursor)) {
			if (!pisbol(bw->cursor))
				udelbl(bw->parent, 0);
			while (joe_isspace(map,(k = pgetc(p))) && k != '\n') {
				binsc(bw->cursor, k);
				pgetc(bw->cursor);
			}
		} else {
			int x;
			for (x=0;x<bw->o.istep;++x) {
				binsc(bw->cursor,bw->o.indentc);
				pgetc(bw->cursor);
			}
		}
		bw->cursor->xcol = piscol(bw->cursor);
		prm (p);
	} else if (k == '\t' && bw->o.spaces && !no_decode) {
		off_t n;

		if (bw->o.picture)
			n = bw->cursor->xcol;
		else
			n = piscol(bw->cursor);

		n = bw->o.tab - n % bw->o.tab;
		while (n--)
			utypebw(bw, ' ');
	} else {
		int upd;
		int simple;
		ptrdiff_t x;

		/* Picture mode */
		if (bw->o.picture && bw->cursor->xcol!=piscol(bw->cursor))
			pfill(bw->cursor,bw->cursor->xcol,' '); /* Why no tabs? */

		upd = bw->parent->t->t->updtab[bw->y + bw->cursor->line - bw->top->line];
		simple = 1;

		if (pisblank(bw->cursor))
			while (piscol(bw->cursor) < bw->o.lmargin) {
				binsc(bw->cursor, ' ');
				pgetc(bw->cursor);
			}

		if (!no_decode) {
			if(!map->type) {
				/* Convert to byte code */
				k = from_uni(map, k);
			}
		}

		binsc(bw->cursor, k);

		/* We need x position before we move cursor */
		x = piscol(bw->cursor) - bw->offset;
		pgetc(bw->cursor);

		/* Tabs are weird here... */
		if (bw->o.overtype && !piseol(bw->cursor) && k != '\t')
			udelch(bw->parent, 0);

		/* Not sure if we're in right position for wordwrap when we're in overtype mode */
		if (bw->o.wordwrap && piscol(bw->cursor) > bw->o.rmargin && !joe_isblank(map,k)) {
			wrapword(bw, bw->cursor, bw->o.lmargin, bw->o.french, 0, NULL);
			simple = 0;
		}

		bw->cursor->xcol = piscol(bw->cursor);
#ifndef __MSDOS__
		if (x < 0 || x >= bw->w)
			simple = 0;
		if (bw->cursor->line < bw->top->line || bw->cursor->line >= bw->top->line + bw->h)
			simple = 0;
		if (simple && bw->parent->t->t->sary[bw->y + bw->cursor->line - bw->top->line])
			simple = 0;
		if (cclass_lookup(cclass_combining, k))
			simple = 0;
		if (simple && k != '\t' && k != '\n' && !curmacro) {
			int atr;
			SCRN *t = bw->parent->t->t;
			ptrdiff_t y = bw->y + TO_DIFF_OK(bw->cursor->line - bw->top->line);
			int (*screen)[COMPOSE] = t->scrn + y * t->co;
			int *attr = t->attr + y * t->co;
			x += bw->x;

			atr = BG_COLOR(bg_text);

			if (!upd && piseol(bw->cursor) && !bw->o.highlight)
				t->updtab[y] = 0;
			if (markb &&
			    markk &&
			    markb->b == bw->b &&
			    markk->b == bw->b &&
			   ((!square && bw->cursor->byte >= markb->byte && bw->cursor->byte < markk->byte) ||
			    ( square && bw->cursor->line >= markb->line && bw->cursor->line <= markk->line && piscol(bw->cursor) >= markb->xcol && piscol(bw->cursor) < markk->xcol)))
				atr |= INVERSE;
			outatr(bw->b->o.charmap, t, screen + x, attr + x, x, y, k, atr);
		}
#endif
	}
	return 0;
}

int utypebw(BW *bw, int k)
{
	return utypebw_raw(bw, k, 0);
}

int utypew(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	return utypebw(bw, k);
}

/* Quoting */

static B *unicodehist = NULL;	/* History of previously entered unicode characters */

static int dounicode(W *w, char *s, void *object, int *notify)
{
	BW *bw;
	int num;
	WIND_BW(bw, w);
	num = zhtoi(s);
	if (notify)
		*notify = 1;
	vsrm(s);
	utypebw_raw(bw, num, 1);
	bw->cursor->xcol = piscol(bw->cursor);
	return 0;
}

int quotestate;
int quoteval;

static int doquote(W *w, int c, void *object, int *notify)
{
	BW *bw;
	char buf[40];
	WIND_BW(bw, w);

/*
	if (c < 0 || c >= 256) {
		nungetc(c);
		return -1;
	}
*/
	switch (quotestate) {
	case 0:
		if (c >= '0' && c <= '9') {
			quoteval = c - '0';
			quotestate = 1;
			joe_snprintf_1(buf, SIZEOF(buf), "ASCII %c--", c);
			if (!mkqwna(bw->parent, sz(buf), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		} else if (c == 'x' || c == 'X') {
			if (bw->b->o.charmap->type) {
				if (!wmkpw(bw->parent, joe_gettext(_("Unicode (ISO-10646) character in hex (%{abort} to abort): ")), &unicodehist, dounicode,
				           NULL, NULL, NULL, NULL, NULL, locale_map, 0))
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
			utypebw_raw(bw, c, 1);
			bw->cursor->xcol = piscol(bw->cursor);
		}
		break;
	case 1:
		if (c >= '0' && c <= '9') {
			joe_snprintf_2(buf, SIZEOF(buf), "ASCII %c%c-", quoteval + '0', c);
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
			utypebw_raw(bw, quoteval, 1);
			bw->cursor->xcol = piscol(bw->cursor);
		}
		break;
	case 3:
		if (c >= '0' && c <= '9') {
			joe_snprintf_1(buf, SIZEOF(buf), "ASCII 0x%c-", c);
			quoteval = c - '0';
			quotestate = 4;
			if (!mkqwna(bw->parent, sz(buf), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		} else if (c >= 'a' && c <= 'f') {
			joe_snprintf_1(buf, SIZEOF(buf), "ASCII 0x%c-", c + 'A' - 'a');
			quoteval = c - 'a' + 10;
			quotestate = 4;
			if (!mkqwna(bw->parent, sz(buf), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		} else if (c >= 'A' && c <= 'F') {
			joe_snprintf_1(buf, SIZEOF(buf), "ASCII 0x%c-", c);
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
			utypebw_raw(bw, quoteval, 1);
			bw->cursor->xcol = piscol(bw->cursor);
		} else if (c >= 'a' && c <= 'f') {
			quoteval = quoteval * 16 + c - 'a' + 10;
			utypebw_raw(bw, quoteval, 1);
			bw->cursor->xcol = piscol(bw->cursor);
		} else if (c >= 'A' && c <= 'F') {
			quoteval = quoteval * 16 + c - 'A' + 10;
			utypebw_raw(bw, quoteval, 1);
			bw->cursor->xcol = piscol(bw->cursor);
		}
		break;
	case 5:
		if (c >= '0' && c <= '7') {
			joe_snprintf_1(buf, SIZEOF(buf), "ASCII 0%c--", c);
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
			joe_snprintf_2(buf, SIZEOF(buf), "ASCII 0%c%c-", quoteval + '0', c);
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
			utypebw_raw(bw, quoteval, 1);
			bw->cursor->xcol = piscol(bw->cursor);
		}
		break;
	}
	if (notify)
		*notify = 1;
	return 0;
}

int uquote(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	quotestate = 0;
	if (mkqwna(bw->parent, sz(joe_gettext(_("Ctrl- (or 0-9 for dec. ascii, x for hex, or o for octal)"))), doquote, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

static int doquote9(W *w, int c, void *object, int *notify)
{
	BW *bw;
	WIND_BW(bw, w);
	if (notify)
		*notify = 1;
	if ((c >= 0x40 && c <= 0x5F) || (c >= 'a' && c <= 'z'))
		c &= 0x1F;
	if (c == '?')
		c = 127;
	if (c >= 0 && c <= 127)
		c |= 128;
	utypebw_raw(bw, c, 1);
	bw->cursor->xcol = piscol(bw->cursor);
	return 0;
}

static int doquote8(W *w, int c, void *object, int *notify)
{
	BW *bw;
	WIND_BW(bw, w);
	if (c == '`') {
		if (mkqwna(bw->parent, sc("Meta-Ctrl-"), doquote9, NULL, NULL, notify))
			return 0;
		else
			return -1;
	}
	if (notify)
		*notify = 1;
	if (c >= 0 && c <= 127)
		c |= 128;
	utypebw_raw(bw, c, 1);
	bw->cursor->xcol = piscol(bw->cursor);
	return 0;
}

int uquote8(W *w, int k)
{
	if (mkqwna(w, sc("Meta-"), doquote8, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

static int doctrl(W *w, int c, void *object, int *notify)
{
	BW *bw;
	int org;
	WIND_BW(bw, w);
	org = bw->o.overtype;

	if (notify)
		*notify = 1;
	bw->o.overtype = 0;
	if ((bw->parent->huh == srchstr || bw->parent->huh == replstr) && c == '\n') {
		utypebw(bw, '\\');
		utypebw(bw, 'n');
	} else
		utypebw_raw(bw, c, 1);
	bw->o.overtype = org;
	bw->cursor->xcol = piscol(bw->cursor);
	return 0;
}

int uctrl(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (mkqwna(bw->parent, sz(joe_gettext(_("Quote"))), doctrl, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

/* User hit Return.  Deal with autoindent.
 */

int rtntw(W *w)
{
	BW *bw;
	WIND_BW(bw, w);
	if (bw->o.overtype) {
		p_goto_eol(bw->cursor);
		if (piseof(bw->cursor))
			binsc(bw->cursor, '\n');
		pgetc(bw->cursor);
		bw->cursor->xcol = piscol(bw->cursor);
	} else {
		P *p = pdup(bw->cursor, "rtntw");
		int c;

		binsc(bw->cursor, '\n'), pgetc(bw->cursor);
		/* Suppress autoindent if we're on a space or tab... */
		if (bw->o.autoindent /* && (brch(bw->cursor)!=' ' && brch(bw->cursor)!='\t')*/) {
			p_goto_bol(p);
			while (joe_isspace(bw->b->o.charmap,(c = pgetc(p))) && c != '\n') {
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

int uopen(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	binsc(bw->cursor,'\n');
	if (bw->o.autoindent && (brch(bw->cursor)!=' ' && brch(bw->cursor)!='\t')) {
		P *p = pdup(bw->cursor, "uopen");
		P *q = pdup(p, "uopen");
		int c;
		pgetc(q);
		p_goto_bol(p);
		while (joe_isspace(bw->b->o.charmap,(c = pgetc(p))) && c != '\n') {
			binsc(q, c);
			pgetc(q);
		}
		prm(p); prm(q);
	}

	return 0;
}

/* Set book-mark */

static int dosetmark(W *w, int c, void *object, int *notify)
{
	BW *bw;
	WIND_BW(bw, w);
	if (notify)
		*notify = 1;
	if (c >= '0' && c <= ':') {
		pdupown(bw->cursor, bw->b->marks + c - '0', "dosetmark");
		poffline(bw->b->marks[c - '0']);
		if (c!=':') {
			joe_snprintf_1(msgbuf, JOE_MSGBUFSIZE, joe_gettext(_("Mark %d set")), c - '0');
			msgnw(bw->parent, msgbuf);
		}
		return 0;
	} else {
		nungetc(c);
		return -1;
	}
}

int usetmark(W *w, int c)
{
	if (c >= '0' && c <= ':')
		return dosetmark(w, c, NULL, NULL);
	else if (mkqwna(w, sz(joe_gettext(_("Set mark (0-9):"))), dosetmark, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

/* Goto book-mark */

static int dogomark(W *w, int c, void *object, int *notify)
{
	BW *bw;
	WIND_BW(bw, w);
	if (notify)
		*notify = 1;
	if (c >= '0' && c <= ':')
		if (bw->b->marks[c - '0']) {
			pset(bw->cursor, bw->b->marks[c - '0']);
			bw->cursor->xcol = piscol(bw->cursor);
			return 0;
		} else {
			joe_snprintf_1(msgbuf, JOE_MSGBUFSIZE, joe_gettext(_("Mark %d not set")), c - '0');
			msgnw(bw->parent, msgbuf);
			return -1;
	} else {
		nungetc(c);
		return -1;
	}
}

int ugomark(W *w, int c)
{
	if (c >= '0' && c <= '9')
		return dogomark(w, c, NULL, NULL);
	else if (mkqwna(w, sz(joe_gettext(_("Goto bookmark (0-9):"))), dogomark, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

/* Goto next instance of character */

static int dobkwdc;

static int dofwrdc(W *w, int k, void *object, int *notify)
{
	BW *bw;
	int c;
	P *q;
	WIND_BW(bw, w);

	if (notify)
		*notify = 1;
	if (k < 0 || k >= 256) {
		nungetc(k);
		return -1;
	}
	q = pdup(bw->cursor, "dofwrdc");
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
		msgnw(bw->parent, joe_gettext(_("Not found")));
		prm(q);
		return -1;
	} else {
		pset(bw->cursor, q);
		bw->cursor->xcol = piscol(bw->cursor);
		prm(q);
		return 0;
	}
}

int ufwrdc(W *w, int k)
{
	dobkwdc = 0;
	if (k >= 0 && k < 256)
		return dofwrdc(w, k, NULL, NULL);
	else if (mkqw(w, sz(joe_gettext(_("Forward to char: "))), dofwrdc, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

int ubkwdc(W *w, int k)
{
	dobkwdc = 1;
	if (k >= 0 && k < 256)
		return dofwrdc(w, k, NULL, NULL);
	else if (mkqw(w, sz(joe_gettext(_("Backward to char: "))), dofwrdc, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

/* Display a message */

static int domsg(W *w, char *s, void *object, int *notify)
{
	if (notify)
		*notify = 1;
	zlcpy(msgbuf, SIZEOF(msgbuf), s);
	vsrm(s);
	msgnw(w, msgbuf);
	return 0;
}

int umsg(W *w, int k)
{
	if (wmkpw(w, joe_gettext(_("Message (%{abort} to abort): ")), NULL, domsg, NULL, NULL, NULL, NULL, NULL, locale_map, 0))
		return 0;
	else
		return -1;
}

/* Insert text */

static int dotxt(W *w, char *s, void *object, int *notify)
{
	char fill;
	char *str;
	BW *bw;
	WIND_BW(bw, w);

	if (notify)
		*notify = 1;
	if (s[0] == '`') {
		str = vsmk(1024);
		fill = ' ';
		str = stagen(str, bw, &s[1], fill);
		vsrm(s);
		s = str;
	}
	if (s) {
	 	const char *t = s;
		ptrdiff_t len = sLEN(s);
		while (len) {
			int c;
			if (bw->b->o.charmap->type)
				c = utf8_decode_fwrd(&t, &len);
			else {
				c = *(const unsigned char *)t++;
				--len;
			}
			if (c >= 0)
				utypebw_raw(bw, c, 1);
		}
		vsrm(s);
	}
	return 0;
}

int utxt(W *w, int k)
{
	BW *bw;
	WIND_BW(bw, w);
	if (wmkpw(w, joe_gettext(_("Insert (%{abort} to abort): ")), NULL, dotxt, NULL, NULL, utypebw, NULL, NULL, bw->b->o.charmap, 0))
		return 0;
	else
		return -1;
}

/* Insert current file name */

int uname_joe(W *w, int k)
{
	const char *s;
	s=((BW *)w->main->object)->b->name;
	if (!s || !*s)
		return -1;
	while (*s)
		if (utypew(w,*(const unsigned char *)s++))
			return -1;
	return 0;
}

/* Insert until non-base64 character received */

int upaste(W *w, int k)
{
	int c;
	int accu = 0;
	int count;
	int tmp_ww;
	int tmp_ai;
	BW *bw;
	WIND_BW(bw, w);
	tmp_ww = bw->o.wordwrap;
	tmp_ai = bw->o.autoindent;

	bw->o.wordwrap = 0;
	bw->o.autoindent = 0;
	count = 0;

	/* We have to wait for the second ';' */
	while ((c = ttgetch()) != -1)
		if (c == ';')
			break;
	if (c == -1)
		goto bye;

	while ((c = ttgetch()) != -1) {
		if (c >= 'A' && c <= 'Z')
			c = c - 'A';
		else if (c >= 'a' && c <= 'z')
			c = c - 'a' + 26;
		else if (c >= '0' && c <= '9')
			c = c - '0' + 52;
		else if (c == '+')
			c = 62;
		else if (c == '/')
			c = 63;
		else if (c == '=')
			continue;
		else
			break;

		switch (count) {
			case 0:
				accu = c;
				count = 6;
				break;
			case 2:
				accu = (accu << 6) + c;
				if (accu == 13)
					rtntw(bw->parent);
				else
					utypebw(bw, accu);
				count = 0;
				break;
			case 4:
				accu = (accu << 4) + (c >> 2);
				if (accu == 13)
					rtntw(bw->parent);
				else
					utypebw(bw, accu);
				accu = (c & 0x3);
				count = 2;
				break;
			case 6:
				accu = (accu << 2) + (c >> 4);
				if (accu == 13)
					rtntw(bw->parent);
				else
					utypebw(bw, accu);
				accu = (c & 0xF);
				count = 4;
				break;
		}
	}
	/* Terminator is ESC \ */
	if (c == 033) {
		ttgetch();
	}

	bye:

	bw->o.wordwrap = tmp_ww;
	bw->o.autoindent = tmp_ai;

	return 0;
}

/* Bracketed paste */

int saved_ww;
int saved_ai;
int saved_sp;

int ubrpaste(W *w, int k)
{
	BW *bw;

	WIND_BW(bw, w);

	saved_ww = bw->o.wordwrap;
	saved_ai = bw->o.autoindent;
	saved_sp = bw->o.spaces;

	bw->o.wordwrap = bw->o.autoindent = bw->o.spaces = 0;
	bw->pasting = 1;

	return 0;
}

int ubrpaste_done(W *w, int k)
{
	BW *bw;

	WIND_BW(bw, w);

	bw->o.wordwrap = saved_ww;
	bw->o.autoindent = saved_ai;
	bw->o.spaces = saved_sp;
	bw->pasting = 0;

	return 0;
}
