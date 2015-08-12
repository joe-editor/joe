/*
 *	Functions which make use of the unicode facts in unifacts.c
 *
 *	Copyright
 *		(C) 2015 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#include "types.h"

/* Convert string to lowercase for case folding */

/* Perfect hash table of character to lowercase character or to fold_table index if it's a string
   Table index is indicated by a -10000 + index
*/

PHASH *fold_hash_table;

static void init_fold_hash_table()
{
	int x;
	fold_hash_table = mkphash(0x1FF, 512);
	for (x = 0; fold_table[x].first; ++x) {
		unsigned y;
		for (y = fold_table[x].first; y <= fold_table[x].last; ++y)
			phash_add(fold_hash_table, y, fold_table[x].b ? (-10000 + x) : y - fold_table[x].first + fold_table[x].a);
	}
}

/* Convert unicode string to lowercase */

int *lowerize(int *d, ptrdiff_t len, const int *s)
{
	int *org = d;
	if (!fold_hash_table)
		init_fold_hash_table();
	if (!len) {
		fprintf(stderr, "lowerize called with len == 0\n");
		exit(1);
	}
	--len;
	while (len && *s) {
		int idx = phash_find(fold_hash_table, *s);
		if (idx == -1) { /* Not in table */
			*d++ = *s++;
			--len;
		} else if (idx >= 0) { /* Replace it with a single character */
			++s;
			*d++ = idx;
			--len;
		} else { /* Replace it with a string */
			idx += 10000;
			++s;
			*d++ = fold_table[idx].a;
			--len;
			if (len && fold_table[idx].b) {
				*d++ = fold_table[idx].b;
				--len;
				if (len && fold_table[idx].c) {
					*d++ = fold_table[idx].c;
					--len;
				}
			}
		}
	}
	*d = 0;
	return org;
}

/* Get a character class containing all characters matching a particular unicode category or block */

HASH *unicat_hash;

struct Cclass *unicode(const char *cat)
{
	struct Cclass *m;
	if (!unicat_hash)
		unicat_hash = htmk(256);
	m = htfind(unicat_hash, cat);
	if (!m) {
		int x;
		m = (struct Cclass *)joe_malloc(SIZEOF(struct Cclass));
		cclass_init(m);
		for (x = 0; unicat[x].name; ++x)
			/* Match exact category name or set of categories like 'L' matches 'Ll', 'Lu', etc. */
			if (!zcmp(unicat[x].name, cat) || (cat[0] == unicat[x].name[0] && !cat[1] && unicat[x].name[1] && !unicat[x].name[2])) {
				cclass_merge(m, unicat[x].intervals, unicat[x].len);
			}
		if (!m->len) {
			joe_free(m);
			m = 0;
		} else {
			cclass_opt(m);
			htadd(unicat_hash, zdup(cat), m);
		}
	}
	return m;
}

/* iswxxx functions */

struct Cclass cclass_upper[1];
int joe_iswupper(struct charmap *foo, int ch) { return cclass_lookup(cclass_upper, ch); }

struct Cclass cclass_lower[1];
int joe_iswlower(struct charmap *foo, int ch) { return cclass_lookup(cclass_lower, ch); }

struct Cclass cclass_alpha[1];
int joe_iswalpha(struct charmap *foo, int ch) { return cclass_lookup(cclass_alpha, ch); }

struct Cclass cclass_alpha_[1];
int joe_iswalpha_(struct charmap *foo, int ch) { return cclass_lookup(cclass_alpha_, ch); }

struct Cclass cclass_alnum[1];
int joe_iswalnum(struct charmap *foo, int ch) { return cclass_lookup(cclass_alnum, ch); }

struct Cclass cclass_alnum_[1];
int joe_iswalnum_(struct charmap *foo, int ch) { return cclass_lookup(cclass_alnum_, ch); }

struct Cclass cclass_digit[1];
int joe_iswdigit(struct charmap *foo, int ch) { return cclass_lookup(cclass_digit, ch); }

struct Cclass cclass_notdigit[1];

struct Cclass cclass_xdigit[1];
int joe_iswxdigit(struct charmap *foo, int ch) { return cclass_lookup(cclass_xdigit, ch); }

struct Cclass cclass_punct[1];
int joe_iswpunct(struct charmap *foo, int ch) { return cclass_lookup(cclass_punct, ch); }

struct Cclass cclass_space[1];
int joe_iswspace(struct charmap *foo, int ch) { return cclass_lookup(cclass_space, ch); }

struct Cclass cclass_notspace[1];

struct Cclass cclass_blank[1];
int joe_iswblank(struct charmap *foo, int ch) { return cclass_lookup(cclass_blank, ch); }

struct Cclass cclass_ctrl[1];
int joe_iswctrl(struct charmap *foo, int ch) { return cclass_lookup(cclass_ctrl, ch); }

struct Cclass cclass_graph[1];
int joe_iswgraph(struct charmap *foo, int ch) { return cclass_lookup(cclass_graph, ch); }

struct Cclass cclass_print[1];
int joe_iswprint(struct charmap *foo, int ch) { return cclass_lookup(cclass_print, ch); }

struct Cclass cclass_word[1];
struct Cclass cclass_notword[1];

/* This is how ASCII is classified in UNICODE:

       Cc: 00 - 1F, 7F
       Zs: 20
       Po: ! " # % & ' * , . / : ; ? @ \
       Sc: $
       Pd: -
       Ps: ( [ {
       Pe: ) ] }
       Sm: + < = > | ~
       Sk: ^ `
       Pc: _
       Nd: 0 - 9
       Lu: A - Z
       Ll: a - z

   Notes: For "blank", you probably want Zs and tab
          For "whitespace", you probably want Zs, tab, newline, carriage return and form-feed
          For "identifier start", you probably want letters, Pc and maybe Sc
          For "identifier rest", you probably want letters, digits, Pc and maybe Sc
   
Convenient character classes:

   see http://www.w3.org/TR/xml11/#NT-NameStartChar
   see http://www.w3.org/TR/xmlschma11-2/#regexs

	cclass_digit:		\d	Digit: same as \p{Nd}
	cclass_notdigit:	\D	opposite

	cclass_space:		\s	space, tab, newline, return [JOE also includes formfeed!]
	cclass_notspace:	\S	opposite

				\i	NameStartChar
					: A-Z _ a-z C0-D6 D8-F6 F8-2FF 370-37D 37F-1FFF 200C-200D
					2070-218F 2C00-2FEF 3001-D7FF F900-FDCF FDF0-FFFD 10000-EFFFF
				\I	opposite

				\c	NameChar
					\i - . 0-9 B7 0300-036F 203F-2040
				\C	opposite

	cclass_word:		\w	word character: [\x{0}-\x{10ffff}]-[\p{P}\p{Z}\p{C}]
	cclass_notword:		\W	opposite
*/

void joe_iswinit()
{
	/* Upper */
	cclass_init(cclass_upper);
	cclass_union(cclass_upper, unicode("Lu"));
	cclass_opt(cclass_upper);

	/* Lower */
	cclass_init(cclass_lower);
	cclass_union(cclass_lower, unicode("Ll"));
	cclass_opt(cclass_lower);

	/* Alphabetical */
	cclass_init(cclass_alpha);
	cclass_union(cclass_alpha, unicode("L"));
	cclass_union(cclass_alpha, unicode("M"));
	cclass_opt(cclass_alpha);

	/* Alphabetical + underscores */
	cclass_init(cclass_alpha_);
	cclass_union(cclass_alpha_, unicode("L"));
	cclass_union(cclass_alpha_, unicode("M"));
	cclass_union(cclass_alpha_, unicode("Pc"));
	cclass_opt(cclass_alpha_);

	/* Alphanumeric */
	cclass_init(cclass_alnum);
	cclass_union(cclass_alnum, unicode("L"));
	cclass_union(cclass_alnum, unicode("M"));
	cclass_union(cclass_alnum, unicode("N"));
	cclass_opt(cclass_alnum);

	/* Alphanumeric + underscores */
	cclass_init(cclass_alnum_);
	cclass_union(cclass_alnum_, unicode("L"));
	cclass_union(cclass_alnum_, unicode("M"));
	cclass_union(cclass_alnum_, unicode("N"));
	cclass_union(cclass_alnum_, unicode("Pc"));
	cclass_opt(cclass_alnum_);

	/* Digit */
	cclass_init(cclass_digit);
	cclass_union(cclass_digit, unicode("Nd"));
	cclass_opt(cclass_digit);

	/* Not a digit */
	cclass_init(cclass_notdigit);
	cclass_union(cclass_notdigit, cclass_digit);
	cclass_inv(cclass_notdigit);
	cclass_opt(cclass_notdigit);

	/* Hex digit */
	cclass_init(cclass_xdigit);
	cclass_union(cclass_xdigit, unicode("Nd"));
	cclass_add(cclass_xdigit, 'a', 'f');
	cclass_add(cclass_xdigit, 'A', 'F');
	cclass_opt(cclass_xdigit);

	/* Punctuation */
	cclass_init(cclass_punct);
	cclass_union(cclass_punct, unicode("P"));
	cclass_opt(cclass_punct);

	/* Whitespace */
	cclass_init(cclass_space);
	cclass_add(cclass_space, '\t', '\t');
	cclass_add(cclass_space, '\r', '\r');
	cclass_add(cclass_space, '\n', '\n');
	cclass_add(cclass_space, '\f', '\f');
	cclass_union(cclass_space, unicode("Z"));
	cclass_opt(cclass_space);

	/* Not whitespace */
	cclass_init(cclass_notspace);
	cclass_union(cclass_notspace, cclass_space);
	cclass_inv(cclass_notspace);
	cclass_opt(cclass_notspace);

	/* Blanks: tab included */
	cclass_init(cclass_blank);
	cclass_add(cclass_blank, '\t', '\t');
	cclass_union(cclass_blank, unicode("Zs"));
	cclass_opt(cclass_blank);

	/* Control characters */
	cclass_init(cclass_ctrl);
	cclass_union(cclass_ctrl, unicode("C"));
	cclass_union(cclass_ctrl, unicode("Zl"));
	cclass_union(cclass_ctrl, unicode("Zp"));
	cclass_opt(cclass_ctrl);

	/* Printable characters (kind of inverse of control characters) */
	cclass_init(cclass_print);
	cclass_union(cclass_print, unicode("L"));
	cclass_union(cclass_print, unicode("M"));
	cclass_union(cclass_print, unicode("S"));
	cclass_union(cclass_print, unicode("N"));
	cclass_union(cclass_print, unicode("P"));
	cclass_union(cclass_print, unicode("Zs"));
	cclass_opt(cclass_print);

	/* Graphical characters (no spaces) */
	cclass_init(cclass_graph);
	cclass_union(cclass_graph, unicode("L"));
	cclass_union(cclass_graph, unicode("M"));
	cclass_union(cclass_graph, unicode("S"));
	cclass_union(cclass_graph, unicode("N"));
	cclass_union(cclass_graph, unicode("P"));
	cclass_opt(cclass_graph);

	/* Not word characters */
	cclass_init(cclass_notword);
	cclass_union(cclass_notword, unicode("C"));
	cclass_union(cclass_notword, unicode("P"));
	cclass_union(cclass_notword, unicode("Z"));
	cclass_opt(cclass_notword);

	/* Word characters */
	cclass_init(cclass_word);
	cclass_union(cclass_word, cclass_notword);
	cclass_inv(cclass_word);
	cclass_opt(cclass_word);
}


/* Digit value of any \p{Nd} digit */
/* Note that intervals in Nd table are not merged! */

static struct unicat *digtable = 0;

int digval(int ch)
{
	if (!digtable) {
		int x;
		for (x = 0; unicat[x].name; ++x)
			if (!zcmp(unicat[x].name, "Nd")) {
				digtable = &unicat[x];
				break;
			}
	}
	if (digtable) {
		int idx = interval_test(digtable->intervals, digtable->len, ch);
		if (idx != -1) {
			return ch - digtable->intervals[idx].first;
		}
	}
	return -1;
}
