/*
 *	Regular expression subroutines
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "types.h"

/* Parse one character.  It can be UTF-8 if utf8 is set.  b has pointer to string length or NULL for zero-terminated strings */

/* Returns the character or -256 for a category (in which case *cat is filled in if it's not NULL) */
/* Returns NUL if a/b is at end of string */

int escape(int utf8, const char **a, ptrdiff_t *b, struct Cclass **cat)
{
	int c = 0;
	const char *s = *a;
	ptrdiff_t l;

	if (b)
		l = *b;
	else
		l = -1;

	if ((b ? l >= 2 : s[1]) && *s == '\\') { /* Backslash.. */
		++s; --l;
		switch (*s) {
			case 'w': { /* Word */
				++s; --l;
				c = -256;
				
				if (cat)
					*cat = cclass_word;
				break;
			} case 'W': { /* Not a word */
				++s; --l;
				c = -256;
				
				if (cat)
					*cat = cclass_notword;
				break;
			} case 's': { /* Space */
				++s; --l;
				c = -256;
				
				if (cat)
					*cat = cclass_space;
				break;
			} case 'S': { /* Not a space */
				++s; --l;
				c = -256;
				
				if (cat)
					*cat = cclass_notspace;
				break;
			} case 'd': { /* Digit */
				++s; --l;
				c = -256;
				
				if (cat)
					*cat = cclass_digit;
				break;
			} case 'D': { /* Not a digit */
				++s; --l;
				c = -256;
				
				if (cat)
					*cat = cclass_notdigit;
				break;
			} case 'c': { /* Control */
				c = 0;
				++s; --l;
				if (l) {
					c = (*s++ & 0x1F);
					--l;
				}
				break;
			} case 'n': { /* Newline */
				c = 10;
				++s; --l;
				break;
			} case 't': case '9': { /* Tab */
				c = 9;
				++s; --l;
				break;
			} case 'a': { /* Alert */
				c = 7;
				++s; --l;
				break;
			} case 'b': case '8': { /* Backspace */
				c = 8;
				++s; --l;
				break;
			} case 'f': { /* Formfeed */
				c = 12;
				++s; --l;
				break;
			} case 'e': { /* Escape */
				c = 27;
				++s; --l;
				break;
			} case 'r': { /* Carriage return */
				c = 13;
				++s; --l;
				break;
			} case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': { /* Octal */
				c = *s - '0';
				++s; --l;
				if ((!b || l > 0) && *s >= '0' && *s <= '7') {
					c = c * 8 + s[1] - '0';
					++s; --l;
				}
				if ((!b || l > 0) && *s >= '0' && *s <= '7') {
					c = c * 8 + s[1] - '0';
					++s; --l;
				}
				break;
			} case 'p': { /* Character class made from unicode category */
				++s; --l;
				c = 'X';
				if ((!b || l > 0) && *s == '{') {
					++s; --l;
					char buf[80];
					ptrdiff_t idx = 0;
					while ((!b || l > 0) && *s && *s != '}') {
						if (idx != SIZEOF(buf) - 1)
							buf[idx++] = *s;
						++s; --l;
					}
					if ((!b || l > 0) && *s == '}') {
						++s; --l;
					}
					buf[idx] = 0;
					c = -256;
					if (cat)
						*cat = unicode(buf);
				}
				break;
			} case 'x': case 'X': { /* Hex or unicode */
				c = 0;
				++s; --l;
				if ((!b || l > 0) && *s == '{') {
					++s; --l;
					while (!b || l > 0) {
						if (*s >= '0' && *s <= '9') {
							c = c * 16 + *s - '0';
							++s; --l;
						} else if (*s >= 'A' && *s <= 'F') {
							c = c * 16 + *s - 'A' + 10;
							++s; --l;
						} else if (*s >= 'a' && *s <= 'f') {
							c = c * 16 + *s - 'a' + 10;
							++s; --l;
						} else {
							break;
						}
					}
					if ((!b || l > 0) && *s == '}') {
						++s; --l;
					}
				} else {
					if ((!b || l > 0) && *s >= '0' && *s <= '9') {
						c = c * 16 + *s - '0';
						++s; --l;
					} else if ((!b || l > 0) && *s >= 'A' && *s <= 'F') {
						c = c * 16 + *s - 'A' + 10;
						++s; --l;
					} else if ((!b || l > 0) && *s >= 'a' && *s <= 'f') {
						c = c * 16 + *s - 'a' + 10;
						++s; --l;
					}
					if ((!b || l > 0) && *s >= '0' && *s <= '9') {
						c = c * 16 + *s - '0';
						++s; --l;
					} else if ((!b || l > 0) && *s >= 'A' && *s <= 'F') {
						c = c * 16 + *s - 'A' + 10;
						++s; --l;
					} else if ((!b || l > 0) && *s >= 'a' && *s <= 'f') {
						c = c * 16 + *s - 'a' + 10;
						++s; --l;
					}
				}
				break;
			} default: { /* Character as-is after backslash */
				if (utf8)
					c = utf8_decode_fwrd(&s, (b ? &l : NULL));
				else {
					c = *(const unsigned char *)s++;
					--l;
				}
				break;
			}
		}
	} else if (!b || l > 0) {
		if (utf8) {
			c = utf8_decode_fwrd(&s, (b ? &l : NULL));
		} else {
			c = *(const unsigned char *)s++;
			--l;
		}
	}
	*a = s;
	if (b)
		*b = l;
	return c;
}

static int brack(int utf8,const char **a, ptrdiff_t *la, int c)
{
	int inverse = 0;
	int flag = 0;
	const char *s = *a;
	ptrdiff_t l = *la;

	if (!l)
		return 0;
	if (*s == '^' || *s == '*') {
		inverse = 1;
		++s;
		--l;
	}
	if (l && *s == ']') {
		++s;
		--l;
		if (c == ']')
			flag = 1;
	}
	while (l)
		if (*s == ']') {
			++s;
			--l;
			break;
		} else {
			int cl, cr;
			struct Cclass *cat = NULL;

			cl = escape(utf8, &s, &l, &cat);

			if (cl == -256) {
				flag = cclass_lookup(cat, c);
			} else {

				if (l >= 2 && s[0] == '-' && s[1] != ']') {
					--l;
					++s;
					cr = escape(utf8, &s, &l, NULL);
					if (c >= cl && c <= cr)
						flag = 1;
				} else if (c == cl)
					flag = 1;
			}
		}
	*a = s;
	*la = l;
	if (inverse)
		return !flag;
	else
		return flag;
}

/* Returns -1 (NO_MORE_DATA) for end of file.
 * Returns -2 if we skipped a special sequence and didn't take the character
 * after it (this happens for "strings").
 * Otherwise returns character after sequence (character will be >=0).
 */

static int skip_special(P *p)
{
	int to, s;

	switch (s = pgetc(p)) {
	case '"':
		do {
			if ((s = pgetc(p)) == '\\') {
				pgetc(p);
				s = pgetc(p);
			}
		} while (s != NO_MORE_DATA && s != '"');
		if (s == '"')
			return -2;
		break;
	case '\'':
		if ((s = pgetc(p)) == '\\') {
			s = pgetc(p);
			s = pgetc(p);
		}
		if (s == '\'')
			return -2;
		if ((s = pgetc(p)) == '\'')
			return -2;
		if ((s = pgetc(p)) == '\'')
			return -2;
		break;
	case '[':
		to = ']';
		goto skip;
	case '(':
		to = ')';
		goto skip;
	case '{':
		to = '}';
skip:
		do {
			s = skip_special(p);
		} while (s != to && s != NO_MORE_DATA);
		if (s == to)
			return -2;
		break;
	case '/':
		s = pgetc(p);
		if (s == '*')
			do {
				s = pgetc(p);
				while (s == '*')
					if ((s = pgetc(p)) == '/')
						return -2;
			} while (s != NO_MORE_DATA);
		else if (s != NO_MORE_DATA)
			s = prgetc(p);
		else
			s = '/';
		break;
	}
	return s;
}

int pmatch(Regmatch_t *matches, int nmatch, const char *regex, ptrdiff_t len, P *p, ptrdiff_t n, int icase)
{
	int c, d;
	P *q = pdup(p, "pmatch");
	P *o = NULL;
	int utf8 = p->b->o.charmap->type;
	struct charmap *map = p->b->o.charmap;
	struct utf8_sm sm;
	off_t byte;

	utf8_init(&sm);

	while (len) {
		if (utf8) {
			do {
				c = utf8_decode(&sm,*regex++);
				--len;
			} while (len && c<0);
			if (c<0)
				return 0;
		} else {
			c = *regex++;
			--len;
		}

		switch (c) {
		case '\\':
			if (!len--)
				goto fail;
			switch (c = *regex++) {
			case '?':
				byte = p->byte;
				d = pgetc(p);
				if (d == NO_MORE_DATA)
					goto fail;
				if (n < nmatch) {
					matches[n].rm_so = byte;
					matches[n].rm_eo = p->byte;
				}
				++n;
				break;
			case 'd':
			case 'D':
			case 'w':
			case 'W':
			case 's':
			case 'S':
			case 'n':
			case 'r':
			case 'a':
			case 'f':
			case 'b':
			case 't':
			case 'e':
			case 'x':
			case 'X':
			case 'p':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': {
				struct Cclass *cat = NULL;
				int e;
				d = pgetc(p);
				regex -= 2;
				len += 2;
				e = escape(utf8, &regex, &len, &cat);
				if (e != -256) {
					if (e != d)
						goto fail;
				} else {
					if (!cclass_lookup(cat, d))
						goto fail;
				}
				break;
			} case '*':
				/* Find shortest matching sequence */
				o = pdup(p, "pmatch");
				do {
					off_t pb = p->byte;

					if (!pmatch(matches, nmatch, regex, len, p, n + 1, icase)) {
						if (n < nmatch) {
							matches[n].rm_so = o->byte;
							matches[n].rm_eo = pb;
						}
						goto succeed;
					}
					c = pgetc(p);
				} while (c != NO_MORE_DATA && c != '\n');
				goto fail;
			case 'c':
				o = pdup(p, "pmatch");
				do {
					off_t pb = p->byte;

					if (!pmatch(matches, nmatch, regex, len, p, n + 1, icase)) {
						if (n < nmatch) {
							matches[n].rm_so = o->byte;
							matches[n].rm_eo = pb;
						}
						goto succeed;
					}
				} while (skip_special(p) != NO_MORE_DATA);
				goto fail;
			case '[':
				byte = p->byte;
				d = pgetc(p);
				if (d == NO_MORE_DATA)
					goto fail;
				if (!brack(utf8, &regex, &len, d))
					goto fail;
				if (n < nmatch) {
					matches[n].rm_so = byte;
					matches[n].rm_eo = p->byte;
				}
				++n;
				break;
			case '+':
				{
					const char *oregex = regex;	/* Point to character to skip */
					struct Cclass *cat = NULL;
					ptrdiff_t olen = len;

					const char *tregex;
					ptrdiff_t tlen;

					int match;

					P *r = NULL;

					d = 0;

					o = pdup(p, "pmatch");

					/* Advance over character to skip.  Save character in d unless
					   we're skipping over a \[..] */
					if (len >= 2 && regex[0] == '\\') {
						if (regex[1] == '[') {
							regex += 2;
							len -= 2;
							brack(utf8, &regex, &len, 0);
						} else {
							d = escape(utf8, &regex, &len, &cat);
							if (icase && d != -256)
								d = joe_tolower(map,d);
						}
					} else if (utf8) {
						if ((d = utf8_decode_fwrd(&regex, &len)) < 0)
							goto done;
						else if (icase)
							d = joe_tolower(map,d);
					} else {
						if (len >= 1) {
							--len;
							d = *regex++;
							if (icase)
								d = joe_tolower(map,d);
						} else
							goto done;
					}

					/* Now oregex/olen point to character to skip over and
					   regex/len point to sequence which follows */

					do {
						P *z = pdup(p, "pmatch");

						if (!pmatch(matches, nmatch, regex, len, p, n + 1, icase)) {
							if (n < nmatch) {
								matches[n].rm_so = o->byte;
								matches[n].rm_eo = z->byte;
							}
							if (r)
								prm(r);
							r = pdup(p, "pmatch");
						}
						pset(p, z);
						prm(z);
						c = pgetc(p);
						tregex = oregex;
						tlen = olen;
						if (*oregex == '\\') {
							if (oregex[1] == '[') {
								tregex += 2;
								tlen -= 2;
								match = brack(utf8, &tregex, &tlen, c);
							} else if (d == -256) {
								match = cclass_lookup(cat, c);
							} else
								match = (d == c);
						} else {
							if(icase)
								match = (joe_tolower(map,c) == d);
							else
								match = (c == d);
						}
					} while (c != NO_MORE_DATA && match);

				      done:
					if (r) {
						pset(p, r);
						prm(r);
					}
					if (r)
						goto succeed;
					else
						goto fail;
				}
			case '^':
				if (!pisbol(p))
					goto fail;
				break;
			case '$':
				if (!piseol(p))
					goto fail;
				break;
			case '<':
				if (!pisbow(p))
					goto fail;
				break;
			case '>':
				if (!piseow(p))
					goto fail;
				break;
			case '\\':
				d = pgetc(p);
				if (d != c)
					goto fail;
				break;
			default:
				goto fail;
			}
			break;
		default:
			d = pgetc(p);
			if (icase) {
				if (joe_tolower(map,d) != joe_tolower(map,c))
					goto fail;
			} else {
				if (d != c)
					goto fail;
			}
		}
	}
succeed:
	if (o)
		prm(o);
	prm(q);
	return 0;

fail:
	if (o)
		prm(o);
	pset(p, q);
	prm(q);
	return 1;
}

/* Compile regular expression */

/* Create tree node */

static int mk_node(struct regcomp *r, int ty, int nl, int nr)
{
	int no;
	if (r->len == r->size) {
		r->size *= 2;
		r->nodes = (struct node *)joe_realloc(r->nodes, SIZEOF(struct node) * r->size);
	}
	no = r->len++;

	r->nodes[no].type = ty;
	r->nodes[no].r = nr;
	r->nodes[no].l = nl;
	cclass_init(r->nodes[no].cclass);
	return no;
}

/* Print tree */

static void ind(int x)
{
	while (x--)
		logmessage_0("  ");
}

static void show(struct regcomp *r, int no, int x)
{
	if (no != -1) {
		if (r->nodes[no].type >= 0) {
			ind(x); logmessage_1("'%c'\n", r->nodes[no].type);
		} else {
			ind(x); logmessage_1("%c\n", -r->nodes[no].type);
		}
		if (r->nodes[no].type == -'[') {
			ind(x + 1); cclass_show(r->nodes[no].cclass);
		}
		show(r, r->nodes[no].l, x + 1);
		show(r, r->nodes[no].r, x + 1);
	}
}

/* Conventional syntax regex */

static int do_parse_conventional(struct regcomp *g, int prec)
{
	int no = -1;

	/* Get first item */
	again:
	if (!g->l || *g->ptr == ')' || *g->ptr == '|') {
		no = -1;
	} else if (*g->ptr == '(') {
		++g->ptr; --g->l;
		if (g->l >= 2 && g->ptr[0] == '?' && g->ptr[1] == '#') {
			/* Ignore comment */
			/* printf("Comment\n"); */
			g->ptr += 2; g->l -= 2;
			while (g->l && *g->ptr != ')') {
				++g->ptr; --g->l;
			}
			if (g->l && *g->ptr == ')') {
				++g->ptr; --g->l;
			}
			goto again;
		} else if (g->l >= 2 && g->ptr[0] == '?' && g->ptr[1] == ':') {
			/* Grouping, no sub-match addressing */
			g->ptr += 2;
			no = mk_node(g, -'(', -1, do_parse_conventional(g, 0));
		} else {
			/* Grouping with sub-match addressing */
			no = mk_node(g, -'{', -1, do_parse_conventional(g, 0));
		}
		if (g->l && *g->ptr == ')') {
			++g->ptr; --g->l;
		} else {
			printf("Unbalanced parenthesis\n");
			return 0;
		}
	} else if (g->l && (*g->ptr == '.' || *g->ptr == '^' || *g->ptr == '$')) {
		no = mk_node(g, -*g->ptr++, -1, -1);
		--g->l;
	} else if (g->l >= 2 && g->ptr[0] == '\\' &&
	           (g->ptr[1] == 'b' || g->ptr[1] == 'B' || g->ptr[1] == 'A' || g->ptr[1] == 'Z' || g->ptr[1] == 'z' ||
	            g->ptr[1] == '>' || g->ptr[1] == '<')) {
		++g->ptr;
		--g->l;
		no = mk_node(g, -*g->ptr++, -1, -1);
		--g->l;
	} else if (g->l && *g->ptr == '[') {
		no = mk_node(g, -'[', -1, -1);
		struct Cclass *m = g->nodes[no].cclass;
		int inv = 0;
		++g->ptr; --g->l;
		if (g->l && *g->ptr == '^') {
			inv = 1;
			++g->ptr; --g->l;
		}
		if (g->l && *g->ptr == ']') {
			cclass_add(m, ']', ']');
			++g->ptr; --g->l;
		}
		while (g->l && *g->ptr != ']') {
			struct Cclass *cat;
			int first, last;

			first = escape(g->cmap->type, &g->ptr, &g->l, &cat);

			if (first == -256) {
				if (cat)
					cclass_union(m, cat);
			} else {
				if (g->l >= 2 &&  g->ptr[0] == '-' && g->ptr[1] != ']') {
					++g->ptr;
					--g->l;
					last = escape(g->cmap->type, &g->ptr, &g->l, &cat);
				} else {
					last = first;
				}
				cclass_add(m, first, last);
			}
		}
		if (g->l && *g->ptr == ']') {
			++g->ptr; --g->l;
		}
		if (inv)
			cclass_inv(m);
	} else {
		struct Cclass *cat;
		int ch = escape(g->cmap->type, &g->ptr, &g->l, &cat);
		if (ch == -256) {
			no = mk_node(g, -'[', -1, -1);
			cclass_union(g->nodes[no].cclass, cat);
		} else {
			no = mk_node(g, ch, -1, -1);
		}
	}

	/* Extend it */
	for (;;) {
		if (g->l && *g->ptr == '*') {
			while (g->l && *g->ptr == '*') {
				++g->ptr;
				--g->l;
			}
			no = mk_node(g, -'*', -1, no);
		} else if (g->l && *g->ptr == '+') {
			while (g->l && *g->ptr == '+') {
				++g->ptr;
				--g->l;
			}
			no = mk_node(g, -'+', -1, no);
		} else if (g->l && *g->ptr == '?') {
			while (g->l && *g->ptr == '?') {
				++g->ptr;
				--g->l;
			}
			no = mk_node(g, -'?', -1, no);
		} else if (g->l && *g->ptr == '{') {
			int org = no;
			int min = 0, max = 0;
			++g->ptr;
			--g->l;
			while (g->l && *g->ptr >= '0' && *g->ptr <= '9') {
				min = min * 10 + *g->ptr++ - '0';
				--g->l;
			}
			if (g->l && *g->ptr == ',') {
				++g->ptr;
				--g->l;
				if (g->l && *g->ptr >= '0' && *g->ptr <= '9') {
					while (g->l && *g->ptr >= '0' && *g->ptr <= '9') {
						max = max * 10 + *g->ptr++ - '0';
						g->l--;
					}
				} else {
					max = -1;
				}
			}
			if (g->l && *g->ptr == '}') {
				++g->ptr;
				--g->l;
			}
			/* Turn max into optional */
			if (max > min)
				max -= min;
			/* Do min */
			no = -1;
			while (min--) {
				if (no == -1)
					no = org;
				else
					no = mk_node(g, -',', no, org);
			}
			if (max == 0) {
				/* Exact */
			} else if (max == -1) {
				/* Open */
				no = mk_node(g, -',', no, mk_node(g, -'*', -1, org));
			} else {
				/* Max */
				int q = -1;
				while (max--) {
					if (q == -1)
						q = mk_node(g, -'?', -1, org);
					else
						q = mk_node(g, -'?', -1, mk_node(g, -',', q, org));
				}
				no = mk_node(g, -',', no, q);
			}
		} else if (g->l && *g->ptr == '|') {
			if (prec < 1) {
				++g->ptr;
				--g->l;
				no = mk_node(g,-'|', no, do_parse_conventional(g, 1));
			} else {
				break;
			}
		} else if (!g->l || *g->ptr == ')') {
			break;
		} else if (prec < 2) {
			no = mk_node(g, -',', no, do_parse_conventional(g, 2));
		} else
			break;
	}
	return no;
}

/* JOE syntax regex */

static int do_parse(struct regcomp *g, int prec, int fold)
{
	int no = -1;

	/* Get first item */
	again:
	if (!g->l || (g->l >= 2 && g->ptr[0] == '\\' && g->ptr[1] == ')') || (g->l >= 2 && g->ptr[0] == '\\' && g->ptr[1] == '|')) {
		no = -1;
	} else if (g->l >= 2 && g->ptr[0] == '\\' && g->ptr[1] == '(') {
		g->ptr += 2; g->l -= 2;
		if (g->l >= 2 && g->ptr[0] == '?' && g->ptr[1] == '#') {
			/* Ignore comment */
			/* printf("Comment\n"); */
			g->ptr += 2; g->l -= 2;
			while (g->l >= 2 && !(g->ptr[0] == '\\' && g->ptr[1] == ')')) {
				++g->ptr; --g->l;
			}
			if (g->l >= 2 && g->ptr[0] == '\\' && g->ptr[1] == ')') {
				g->ptr += 2; g->l -= 2;
			} else if (!g->err) {
				g->err = "Missing \\\\)";
			}
			goto again;
		} else if (g->l >= 2 && g->ptr[0] == '?' && g->ptr[1] == ':') {
			/* Grouping, no sub-match addressing */
			g->ptr += 2;
			no = mk_node(g, -'(', -1, do_parse(g, 0, fold));
		} else {
			/* Grouping with sub-match addressing */
			no = mk_node(g, -'{', -1, do_parse(g, 0, fold));
		}
		if (g->l >= 2 && g->ptr[0] == '\\' && g->ptr[1] == ')') {
			g->ptr += 2; g->l -= 2;
		} else if (!g->err) {
			g->err = "Missing \\\\)";
		}
	} else if (g->l >= 2 && g->ptr[0] == '\\' && (g->ptr[1] == '^' || g->ptr[1] == '$' || g->ptr[1] == '>' || g->ptr[1] == '<' /* ||
	                                              g->ptr[1] == 'b' || g->ptr[1] == 'B' || g->ptr[1] == 'A' || g->ptr[1] == 'Z' ||
	                                              g->ptr[1] == 'z' */)) {
		no = mk_node(g, -g->ptr[1], -1, -1);
		g->ptr += 2; g->l -= 2;
	} else if (g->l >= 2 && g->ptr[0] == '\\' && (g->ptr[1] == '.')) {
		no = mk_node(g, -'.', -1, -1);
		g->ptr += 2; g->l -= 2;
	} else if (g->l >= 2 && g->ptr[0] == '\\' && g->ptr[1] == '[') {
		no = mk_node(g, -'[', -1, -1);
		struct Cclass *m = g->nodes[no].cclass;
		int inv = 0;
		g->ptr += 2; g->l -= 2;
		if (g->l && *g->ptr == '^') {
			inv = 1;
			++g->ptr; --g->l;
		}
		if (g->l && *g->ptr == ']') {
			cclass_add(m, ']', ']');
			++g->ptr; --g->l;
		}
		while (g->l && *g->ptr != ']') {
			struct Cclass *cat;
			int first, last;

			first = escape(g->cmap->type, &g->ptr, &g->l, &cat);

			if (first == -256) {
				if (cat)
					cclass_union(m, cat);
			} else {
				if (g->l >= 2 &&  g->ptr[0] == '-' && g->ptr[1] != ']') {
					++g->ptr;
					--g->l;
					last = escape(g->cmap->type, &g->ptr, &g->l, &cat);
				} else {
					last = first;
				}
				if (fold) {
					first = joe_tolower(g->cmap, first);
					last = joe_tolower(g->cmap, last);
				}
				cclass_add(m, first, last);
			}
		}
		if (g->l && *g->ptr == ']') {
			++g->ptr; --g->l;
		} else if (!g->err) {
			g->err = "Missing ]";
		}
		if (inv)
			cclass_inv(m);
	} else {
		struct Cclass *cat;
		int ch = escape(g->cmap->type, &g->ptr, &g->l, &cat);
		if (ch == -256) {
			no = mk_node(g, -'[', -1, -1);
			cclass_union(g->nodes[no].cclass, cat);
		} else {
			if (fold) {
				if (g->cmap->type) { /* Unicode folding... */
					int idx = rmap_lookup(rtree_fold, ch, 0);
					if (idx < FOLDMAGIC)
						no = mk_node(g, ch + idx, -1, -1);
					else {
						idx -= FOLDMAGIC;
						no = mk_node(g, fold_repl[idx][0], -1, -1);
						if (fold_repl[idx][1]) {
							no = mk_node(g, -',', no, mk_node(g, fold_repl[idx][1], -1, -1));
						}
						if (fold_repl[idx][2]) {
							no = mk_node(g, -',', no, mk_node(g, fold_repl[idx][2], -1, -1));
						}
					}
				} else {
					no = mk_node(g, joe_tolower(g->cmap, ch), -1, -1);
				}
			} else {
				no = mk_node(g, ch, -1, -1);
			}
		}
	}

	/* Extend it */
	for (;;) {
		if (g->l >=2 && g->ptr[0] == '\\' && g->ptr[1] == '*') {
			while (g->l >= 2 && g->ptr[0] == '\\' && g->ptr[1] == '*') {
				g->ptr += 2;
				g->l -= 2;
			}
			no = mk_node(g, -'*', -1, no);
		} else if (g->l >= 2 && g->ptr[0] == '\\' && g->ptr[1] == '+') {
			while (g->l >= 2 && g->ptr[0] == '\\' && g->ptr[1] == '+') {
				g->ptr += 2;
				g->l -= 2;
			}
			no = mk_node(g, -'+', -1, no);
		} else if (g->l >= 2 && g->ptr[0] == '\\' && g->ptr[1] == '?') {
			while (g->l >= 2 && g->ptr[0] == '\\' && g->ptr[1] == '?') {
				g->ptr += 2;
				g->l -= 2;
			}
			no = mk_node(g, -'?', -1, no);
		} else if (g->l >= 2 && g->ptr[0] == '\\' && g->ptr[1] == '{') {
			int org = no;
			int min = 0, max = 0;
			g->ptr += 2;
			g->l -= 2;
			while (g->l && *g->ptr >= '0' && *g->ptr <= '9') {
				min = min * 10 + *g->ptr++ - '0';
				--g->l;
			}
			if (g->l && *g->ptr == ',') {
				++g->ptr;
				--g->l;
				if (g->l && *g->ptr >= '0' && *g->ptr <= '9') {
					while (g->l && *g->ptr >= '0' && *g->ptr <= '9') {
						max = max * 10 + *g->ptr++ - '0';
						g->l--;
					}
				} else {
					max = -1;
				}
			}
			if (g->l && *g->ptr == '}') {
				++g->ptr;
				--g->l;
			} else if (!g->err) {
				g->err = "Missing }";
			}
			/* Turn max into optional */
			if (max > min)
				max -= min;
			/* Do min */
			no = -1;
			while (min--) {
				if (no == -1)
					no = org;
				else
					no = mk_node(g, -',', no, org);
			}
			if (max == 0) {
				/* Exact */
			} else if (max == -1) {
				/* Open */
				no = mk_node(g, -',', no, mk_node(g, -'*', -1, org));
			} else {
				/* Max */
				int q = -1;
				while (max--) {
					if (q == -1)
						q = mk_node(g, -'?', -1, org);
					else
						q = mk_node(g, -'?', -1, mk_node(g, -',', q, org));
				}
				no = mk_node(g, -',', no, q);
			}
		} else if (g->l >= 2 && g->ptr[0] == '\\' && g->ptr[1] == '|') {
			if (prec < 1) {
				g->ptr += 2; g->l -= 2;
				no = mk_node(g, -'|', no, do_parse(g, 1, fold));
			} else {
				break;
			}
		} else if (!g->l || (g->ptr[0] == '\\' && g->ptr[1] == ')')) {
			break;
		} else if (prec < 2) {
			no = mk_node(g, -',', no, do_parse(g, 2, fold));
		} else
			break;
	}
	return no;
}

/* Disassembler */

static const char *iname(int c)
{
	if (c >= 0) return "CHAR";
	else switch(c) {
		case iDOT: return "DOT";
		case iBOL: return "BOL";
		case iEOL: return "EOL";
		case iBOW: return "BOW";
		case iEOW: return "EOW";
		case iBRA: return "BRA";
		case iKET: return "KET";
		case iFORK: return "FORK";
		case iJUMP: return "JUMP";
		case iCLASS: return "CLASS";
		case iEND: return "END";
		default: return "HUH?";
	}
}

static void unasm(Frag *f)
{
	ptrdiff_t pc = 0;
	logmessage_0("PC	INSN\n");
	logmessage_0("--	----\n");
	for (;;) {
		ptrdiff_t i = pc;
		int c = fetchi(f, &pc);
		if (c >= 0)
			logmessage_2("%lld:	'%c'\n", (long long)i, c);
		else switch(c) {
			case iDOT:
				logmessage_1("%lld:	.\n", (long long)i);
				break;
			case iBOL:
				logmessage_1("%lld:	^\n", (long long)i);
				break;
			case iEOL:
				logmessage_1("%lld:	$\n", (long long)i);
				break;
			case iBOW:
				logmessage_1("%lld:	<\n", (long long)i);
				break;
			case iEOW:
				logmessage_1("%lld:	>\n", (long long)i);
				break;
			case iBRA:
				logmessage_2("%lld:	bra %d\n", (long long)i, fetchi(f, &pc));
				break;
			case iKET:
				logmessage_2("%lld:	ket %d\n", (long long)i, fetchi(f, &pc));
				break;
			case iFORK: {
				int arg = fetchi(f, &pc);
				logmessage_2("%lld:	fork %lld\n", (long long)i, (long long)(arg + (pc - SIZEOF(int))));
				break;
			} case iJUMP: {
				int arg = fetchi(f, &pc);
				logmessage_2("%lld:	jump %lld\n", (long long)i, (long long)(arg + (pc - SIZEOF(int))));
				break;
			} case iCLASS: {
				struct Cclass *r = (struct Cclass *)fetchp(f, &pc);
				logmessage_1("%lld:	class ", (long long)i);
				cclass_show(r);
				break;
			}
			case iEND:
				logmessage_1("%lld:	end\n", (long long)i);
				return;
		}
	}
}

/* Convert parse-tree into code with infinite loops */

static void codegen(struct regcomp *g, int no, int *end)
{
	while(no != -1) {
		switch (g->nodes[no].type) {
			case -'{': {
				int my_bra_no = g->bra_no++;
				emiti(g->frag, iBRA);
				emiti(g->frag, my_bra_no);
				codegen(g, g->nodes[no].r, 0);
				emiti(g->frag, iKET);
				emiti(g->frag, my_bra_no);
				break;
			}
			case -'(': {
				no = g->nodes[no].r;
				continue;
			}
			case -'*': {
				ptrdiff_t targ, start;
				emiti(g->frag, iFORK);
				targ = emiti(g->frag, 0);
				align_frag(g->frag, SIZEOF(int));
				start = g->frag->len;
				codegen(g, g->nodes[no].r, 0);
				emiti(g->frag, iFORK);
				emit_branch(g->frag, start);
				fixup_branch(g->frag,targ);
				break;
			}
			case -'+': {
				ptrdiff_t start;
				align_frag(g->frag, SIZEOF(int));
				start = g->frag->len;
				codegen(g, g->nodes[no].r, 0);
				emiti(g->frag, iFORK);
				emit_branch(g->frag, start);
				break;
			}
			case -'?': {
				ptrdiff_t targ;
				emiti(g->frag, iFORK);
				targ = emiti(g->frag, 0);
				codegen(g, g->nodes[no].r, 0);
				fixup_branch(g->frag, targ);
				break;
			}
			case -'|': {
				ptrdiff_t alt;
				int first;
				int my_end = 0;
				if (!end) {
					end = &my_end;
					first = 1;
				} else {
					first = 0;
				}
				emiti(g->frag, iFORK);
				alt = emiti(g->frag, 0);
				/* First */
				codegen(g, g->nodes[no].l, 0);
				emiti(g->frag, iJUMP);
				*end = (int)emiti(g->frag, *end);
				/* Rest */
				fixup_branch(g->frag, alt);
				codegen(g, g->nodes[no].r, end);
				/* All JUMPs go here */
				if (first) {
					frag_link(g->frag, *end);
				}
				break;
			}
			case -',': {
				codegen(g, g->nodes[no].l, 0);
				no = g->nodes[no].r;
				continue; /* This keeps recursion depth low */
			}
			case -'.': {
				emiti(g->frag, iDOT);
				break;
			}
			case -'^': {
				emiti(g->frag, iBOL);
				break;
			}
			case -'$': {
				emiti(g->frag, iEOL);
				break;
			}
			case -'<': {
				emiti(g->frag, iBOW);
				break;
			}
			case -'>': {
				emiti(g->frag, iEOW);
				break;
			}
			case -'[': {
				emiti(g->frag, iCLASS);
				emitp(g->frag, g->nodes[no].cclass);
				cclass_opt(g->nodes[no].cclass);
				break;
			}
			default: {
				emiti(g->frag, g->nodes[no].type);
				break;
			}
		}
		break;
	}
}

/* User level of parser: call it with a regex- it returns a program in a malloc block */

/* #define DEBUG 1 */

struct regcomp *joe_regcomp(struct charmap *cmap, const char *s, ptrdiff_t len, int cflags)
{
	int no;
	struct regcomp *g;

	g = (struct regcomp *)joe_malloc(SIZEOF(struct regcomp));
	g->len = 0;
	g->size = 10;
	g->nodes = (struct node *)joe_malloc(SIZEOF(struct node) * g->size);
	g->cmap = cmap;
	iz_frag(g->frag, SIZEOF(int));
	g->bra_no = 0;
	g->err = 0;

	/* Parse expression */
	g->ptr = s;
	g->l = len;
	no = do_parse(g, 0, cflags);

	if (g->l && !g->err) {
		g->err = "Extra junk at end of expression";
	}

	/* Print parse tree */
#ifdef DEBUG
	logmessage_0("Parse tree:\n");
	show(g, no, 0);
#endif

	/* Convert tree into NFA in the form of byte code */

	/* Surround regex with .*(  ).* */
#if 0
	/* .* */
	emiti(g->frag, iFORK);
	b = emit_branch(g->frag, 0);
#ifdef DEBUG
	logmessage_1("Total size = %d\n",g->frag->len);
#endif
	a = emiti(g->frag, iDOT);
	emiti(g->frag, iFORK);
	emit_branch(g->frag, a);
	fixup_branch(g->frag, b);

	/* ( */
	emiti(g->frag, iBRA);
	emiti(g->frag, 0);
#endif
	/* User's regex */
	codegen(g, no, 0);
#if 0
	/* ) */
	emiti(g->frag, iKET);
	emiti(g->frag, 0);

	/* .* */
	emiti(g->frag, iFORK);
	b = emit_branch(g->frag, 0);
	a = emiti(g->frag, iDOT);
	emiti(g->frag, iFORK);
	emit_branch(g->frag, a);
	fixup_branch(g->frag, b);
#endif
	emiti(g->frag, iEND);

	/* Give back space */
	fin_code(g->frag);

	/* Free parse tree */
	/* joe_free(g->nodes); g->nodes = 0; */

	/* Unassemble code */
#ifdef DEBUG
	logmessage_0("NFA-program:\n");
	unasm(g->frag);
	logmessage_1("Total size = %lld\n",(long long)g->frag->len);
#endif

	return g;
}

void joe_regfree(struct regcomp *g)
{
	if (g->nodes)
		joe_free(g->nodes);
	clr_frag(g->frag);
	joe_free(g);
}

#if 0
int rmatch(const unsigned char *pattern, const unsigned char *s, ...)
{
	int match;
	Regex_t r[1];
	Regmatch_t pmatch[MAX_MATCHES];
	unsigned char **results[MAX_MATCHES];
	int n = 0;
	va_list ap;
	va_start(ap, s);

	while (results[n] = va_arg(ap, unsigned char **))
		++n;

	printf("%d\n",n);

	va_end(ap);

	if (Regcomp(r, pattern, 0))
		return -1;

	if (n > r[0]->bra_no) {
		Regfree(r);
		return -2;
	}

	match = Regexec(r, s, n, pmatch, 0);

	Regfree(r);

	if (!match) {
		int x;
		for (x = 0; x != n; ++x) {
			int y;
			int start = pmatch[x].rm_so;
			int len = pmatch[x].rm_eo - pmatch[x].rm_so;
			unsigned char *q = (unsigned char *)malloc(len + 1);
			for (y = 0; y != len; ++y)
				q[y] = s[start + y];
			q[y] = 0;
			*results[x] = q;
		}
	}
	
	return match;
}
#endif

/* Regular expression matcher */

struct thread
{
	unsigned char *pc;
	Regmatch_t pos[MAX_MATCHES];
};

static int better(Regmatch_t *a, Regmatch_t *b, int bra_no)
{
	int y;
	for (y = 0; y != bra_no; ++y) {
		if (a[y].rm_so < b[y].rm_so)
			return 1;
		if (a[y].rm_so > b[y].rm_so)
			return 0;
		if (a[y].rm_eo > b[y].rm_eo)
			return 1;
		if (a[y].rm_eo < b[y].rm_eo)
			return 0;
	}
	return 0;
}

static int add_thread(struct thread *pool, unsigned char *start, int l, int le, unsigned char *pc, Regmatch_t *pos, int bra_no)
{
	int x;
	Regmatch_t *d;
	int t;

	for (t = l; t != le; ++t) {
		if (pool[t].pc == pc) {
			/* PCs are same, state is same... */
			int y;
			for (y = 0; y != bra_no; ++y) {
				if (pool[t].pos[y].rm_so == pos[y].rm_so &&
				    pool[t].pos[y].rm_eo == pos[y].rm_eo) {
				    	/* Identical... move on */
				} else if (pool[t].pos[y].rm_eo != -1 && pos[y].rm_eo != -1) {
					/* Both are complete */
					if (pool[t].pos[y].rm_so < pos[y].rm_so)
						return le; /* Current one starts earlier */
					else if (pool[t].pos[y].rm_so > pos[y].rm_so) {
						/* New one starts earlier */
						for (x = 0; x != bra_no; ++x) pool[t].pos[x] = pos[x];
						return le;
					} else if (pool[t].pos[y].rm_eo > pos[y].rm_eo) {
						/* Current is longer */
						return le;
					} else if (pool[t].pos[y].rm_eo < pos[y].rm_eo) {
						/* New one is longer */
						for (x = 0; x != bra_no; ++x) pool[t].pos[x] = pos[x];
						return le;
					}
					/* They are identical: look at next field */
				} else if (pool[t].pos[y].rm_so != -1 && pos[y].rm_so != -1) {
					if (pool[t].pos[y].rm_so < pos[y].rm_so)
						return le; /* Current one starts earlier */
					else if (pool[t].pos[y].rm_so > pos[y].rm_so) {
						/* New one starts earlier */
						for (x = 0; x != bra_no; ++x) pool[t].pos[x] = pos[x];
						return le;
					}
				} else {
					/* Add if there are no identical others */
					break;
				}
			}
			if (y == bra_no)
				return le; /* They are equal, leave the current one. */
		}
	}
	/* Add it */
	if (le - l == MAX_THREADS) {
		printf("ran out of threads\n");
		exit(-1);
		return le;
	}
	pool[le].pc = pc;
	d = pool[le].pos;
	for (x = 0;x != bra_no; ++x)
		d[x] = pos[x];
	return le + 1;
}

static int add_thread1(struct thread *pool, unsigned char *start, int l, int le, unsigned char *pc, Regmatch_t *pos, int bra_no)
{
	int x;
	Regmatch_t *d;
	int t;

	for (t = l; t != le; ++t) {
		if (pool[t].pc == pc) {
			/* PCs are same, state is same... */
			int y;
			for (y = 0; y != bra_no; ++y) {
				if (pool[t].pos[y].rm_so == pos[y].rm_so &&
				    pool[t].pos[y].rm_eo == pos[y].rm_eo) {
				    	/* Identical... move on */
				} else break;
			}
			if (y == bra_no)
				return le; /* They are equal, leave the current one. */
		}
	}

	/* Add it */
	if (le - l == MAX_THREADS) {
		printf("ran out of threads\n");
		exit(-1);
		return le;
	}
	pool[le].pc = pc;
	d = pool[le].pos;
	for (x = 0;x != bra_no; ++x)
		d[x] = pos[x];
	return le + 1;
}

int joe_regexec(struct regcomp *g, P *p, int nmatch, Regmatch_t *matches, int eflags)
{
	struct thread pool[MAX_THREADS * 2];

	int start_bol = pisbol(p);

	int repl1 = 0, repl2 = 0;

	int cl, cle, nl, nle;
	int t;

	int c, d;
	int match = -1;

	int bra_no = g->bra_no;

	off_t byte;

	if (nmatch < bra_no)
		bra_no = nmatch;

	/* First thread */
	cl = 0;
	cle = 1;
	nl = MAX_THREADS;
	nle = nl;

	c = '^'; /* Not a word character */

	pool[cl].pc = g->frag->start;
	for (c = 0; c != bra_no; ++c) {
		pool[cl].pos[c].rm_so = -1;
		pool[cl].pos[c].rm_eo = -1;
	}

	/* Scan string */
	do {
		d = c;
		byte = p->byte;
		if (eflags) { /* Case folding */
			if  (g->cmap->type) { /* Unicode folding */
				if (repl1) {
					c = repl1;
					repl1 = 0;
				} else if (repl2) {
					c = repl2;
					repl2 = 0;
				} else {
					int idx;
					c = pgetc(p);
					idx = rmap_lookup(rtree_fold, c, 0);
					if (idx < FOLDMAGIC) { /* Replace with single character */
						c += idx;
					} else { /* Replace with string */
						idx -= FOLDMAGIC;
						c = fold_repl[idx][0];
						repl1 = fold_repl[idx][1];
						repl2 = fold_repl[idx][2];
					}
				}
			} else {
				c = joe_tolower(g->cmap, pgetc(p));
			}
		} else { /* No case folding */
			c = pgetc(p);
		}
#ifdef DEBUG
		logmessage_2("'%c' (%lld)\n", c, (long long)byte);
#endif
		/* Give character to all threads */
		for (t = cl; t != cle; ++t) {
			unsigned char *pc = pool[t].pc;
			for (;;) {
				int i;
				i = *(int *)pc;
#ifdef DEBUG
				logmessage_3("  Thread %d PC=%lld insn=%s\n", t - cl, (long long)(pc - g->frag->start), iname(i));
#endif
				if (i >= 0) { /* A single character */
					if (c == i) {
						nle = add_thread(pool, g->frag->start, nl, nle, pc + SIZEOF(int), pool[t].pos, bra_no);
					}
				} else switch (i) {
					case iDOT: {
						if (c != NO_MORE_DATA)
							/* . doesn't match end of string */
							nle = add_thread(pool, g->frag->start, nl, nle, pc + SIZEOF(int), pool[t].pos, bra_no);
						break;
					}
					case iBOL: {
						if (start_bol || d == '\n') {
							pc += SIZEOF(int);
							continue;
						}
						break;
					}
					case iEOL: {
						if (c == NO_MORE_DATA || c == '\n') {
							pc += SIZEOF(int);
							continue;
						}
						break;
					}
					case iBOW: {
						if (joe_isalnum_(g->cmap, c) && !joe_isalnum_(g->cmap, d)) {
							pc += SIZEOF(int);
							continue;
						}
						break;
					}
					case iEOW: {
						if (!joe_isalnum_(g->cmap, c) && joe_isalnum_(g->cmap, d)) {
							pc += SIZEOF(int);
							continue;
						}
						break;
					}
					case iBRA: {
						int idx = *(int *)(pc + SIZEOF(int));
						if (idx < bra_no)
							pool[t].pos[idx].rm_so = byte;
						pc += 2 * SIZEOF(int);
						continue;
					}
					case iKET: {
						int idx = *(int *)(pc + SIZEOF(int));
						if (idx < bra_no)
							pool[t].pos[idx].rm_eo = byte;
						pc += 2 * SIZEOF(int);
						continue;
					}
					case iFORK: {
						cle = add_thread1(pool, g->frag->start, cl, cle, pc + *(int *)(pc + SIZEOF(int)) + SIZEOF(int), pool[t].pos, bra_no);
						pc += 2 * SIZEOF(int);
						continue;
					}
					case iJUMP: {
						pc += *(int *)(pc + SIZEOF(int)) + SIZEOF(int);
						continue;
					}
					case iCLASS: {
						struct Cclass *cclass;
						pc += SIZEOF(int);
						pc += align_o((pc - (unsigned char *)0), SIZEOF(struct Cclass *));
						cclass = *(struct Cclass **)pc;
						if (cclass_lookup(cclass, c)) {
							pc += SIZEOF(struct Cclass *);
							pc += align_o((pc - (unsigned char *)0), SIZEOF(int));
							nle = add_thread(pool, g->frag->start, nl, nle, pc, pool[t].pos, bra_no);
						}
						break;
					}
					case iEND: {
						if (c != NO_MORE_DATA)
							prgetc(p);
						if (match || better(pool[t].pos, matches, bra_no)) {
							int x;
							for (x = 0; x != bra_no; ++x) {
								matches[x] = pool[t].pos[x];
							}
							match = 0;
						}
						break;
					}
				}
				/* If we fall out of the switch, we break the for */
				break;
			}
		}


		/* New becomes current */
		cl = nl;
		cle = nle;
		/* Too many threads? */
		if (cle - cl == MAX_THREADS)
			return -2;
		/* Current becomes new */
		if (nl == MAX_THREADS)
			nl = 0;
		else
			nl = MAX_THREADS;
		 nle = nl;
		 start_bol = 0;
	} while (match && c != NO_MORE_DATA && cl != cle);

	for (c = bra_no; c < nmatch; ++c) {
		matches[c].rm_so = -1;
	}

	return match;
}
