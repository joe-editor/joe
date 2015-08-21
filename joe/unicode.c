/*
 *	Functions which make use of the unicode facts in unifacts.c
 *
 *	Copyright
 *		(C) 2015 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#include "types.h"

/* Convert UTF-32 string to lowercase for case folding */

struct Rtree rtree_fold[1];

int *lowerize(int *d, ptrdiff_t len, const int *s)
{
	int *org = d;
	if (!len) {
		fprintf(stderr, "lowerize called with len == 0\n");
		exit(1);
	}
	--len;
	while (len && *s) {
		int idx = rmap_lookup(rtree_fold, *s, 0);
		if (idx < FOLDMAGIC) { /* Replace it with a single character */
			*d++ = *s++ + idx;
			--len;
		} else { /* Replace it with a string */
			idx -= FOLDMAGIC;
			++s;
			*d++ = fold_repl[idx][0];
			--len;
			if (len && fold_repl[idx][1]) {
				*d++ = fold_repl[idx][1];
				--len;
				if (len && fold_repl[idx][2]) {
					*d++ = fold_repl[idx][2];
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
	m = (struct Cclass *)htfind(unicat_hash, cat);
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

struct Rtree rtree_tolower[1];
int joe_towlower(struct charmap *foo, int ch)
{
	return ch + rmap_lookup(rtree_tolower, ch, 0);
}

struct Rtree rtree_toupper[1];
int joe_towupper(struct charmap *foo, int ch)
{
	return ch + rmap_lookup(rtree_toupper, ch, 0);
}

/* Combining characters */
struct Cclass cclass_combining[1];

/* Double-width characters */
struct Cclass cclass_double[1];

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
	int x;

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

	/* Convert to uppercase */
	rmap_init(rtree_toupper);
	for (x = 0; toupper_table[x].first; ++x) {
		rmap_add(rtree_toupper, toupper_table[x].first, toupper_table[x].last, toupper_cvt[x] - toupper_table[x].first, 0);
	}
	rmap_opt(rtree_toupper);

	/* Convert to lowercase */
	rmap_init(rtree_tolower);
	for (x = 0; tolower_table[x].first; ++x) {
		rmap_add(rtree_tolower, tolower_table[x].first, tolower_table[x].last, tolower_cvt[x] - tolower_table[x].first, 0);
	}
	rmap_opt(rtree_tolower);

	/* Set up fold table */
	rmap_init(rtree_fold);
	for (x = 0; fold_table[x].first; ++x) {
		if (fold_repl[x][1])
			rmap_add(rtree_fold, fold_table[x].first, fold_table[x].last, FOLDMAGIC + x, 0);
		else
			rmap_add(rtree_fold, fold_table[x].first, fold_table[x].last, fold_repl[x][0] - fold_table[x].first, 0);
	}
	rmap_opt(rtree_fold);

/* Combining characters: for JOE this means
      - we don't account for their width, they are merged with a start character
      - a start plus combining contribute to the appearance of the character, so we update
        the character (resend the whole sequence to the terminal) if any of them change */
	cclass_init(cclass_combining);
	cclass_union(cclass_combining, unicode("Me"));
	cclass_union(cclass_combining, unicode("Mn"));
	cclass_add(cclass_combining, 0x1160, 0x11FF); /* These act like combining characters */
	cclass_opt(cclass_combining);

	/* Double width characters */
	cclass_init(cclass_double);
	for (x = 0; width_table[x].first; ++x)
		cclass_add(cclass_double, width_table[x].first, width_table[x].last);
	cclass_opt(cclass_double);
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
		ptrdiff_t idx = interval_test(digtable->intervals, digtable->len, ch);
		if (idx != -1) {
			return ch - digtable->intervals[idx].first;
		}
	}
	return -1;
}

/*
 * This is an implementation of wcwidth() and wcswidth() (defined in
 * IEEE Std 1002.1-2001) for Unicode.
 *
 * http://www.opengroup.org/onlinepubs/007904975/functions/wcwidth.html
 * http://www.opengroup.org/onlinepubs/007904975/functions/wcswidth.html
 *
 * In fixed-width output devices, Latin characters all occupy a single
 * "cell" position of equal width, whereas ideographic CJK characters
 * occupy two such cells. Interoperability between terminal-line
 * applications and (teletype-style) character terminals using the
 * UTF-8 encoding requires agreement on which character should advance
 * the cursor by how many cell positions. No established formal
 * standards exist at present on which Unicode character shall occupy
 * how many cell positions on character terminals. These routines are
 * a first attempt of defining such behavior based on simple rules
 * applied to data provided by the Unicode Consortium.
 *
 * For some graphical characters, the Unicode standard explicitly
 * defines a character-cell width via the definition of the East Asian
 * FullWidth (F), Wide (W), Half-width (H), and Narrow (Na) classes.
 * In all these cases, there is no ambiguity about which width a
 * terminal shall use. For characters in the East Asian Ambiguous (A)
 * class, the width choice depends purely on a preference of backward
 * compatibility with either historic CJK or Western practice.
 * Choosing single-width for these characters is easy to justify as
 * the appropriate long-term solution, as the CJK practice of
 * displaying these characters as double-width comes from historic
 * implementation simplicity (8-bit encoded characters were displayed
 * single-width and 16-bit ones double-width, even for Greek,
 * Cyrillic, etc.) and not any typographic considerations.
 *
 * Much less clear is the choice of width for the Not East Asian
 * (Neutral) class. Existing practice does not dictate a width for any
 * of these characters. It would nevertheless make sense
 * typographically to allocate two character cells to characters such
 * as for instance EM SPACE or VOLUME INTEGRAL, which cannot be
 * represented adequately with a single-width glyph. The following
 * routines at present merely assign a single-cell width to all
 * neutral characters, in the interest of simplicity. This is not
 * entirely satisfactory and should be reconsidered before
 * establishing a formal standard in this area. At the moment, the
 * decision which Not East Asian (Neutral) characters should be
 * represented by double-width glyphs cannot yet be answered by
 * applying a simple rule from the Unicode database content. Setting
 * up a proper standard for the behavior of UTF-8 character terminals
 * will require a careful analysis not only of each Unicode character,
 * but also of each presentation form, something the author of these
 * routines has avoided to do so far.
 *
 * http://www.unicode.org/unicode/reports/tr11/
 *
 * Markus Kuhn -- 2007-05-26 (Unicode 5.0)
 *
 * Permission to use, copy, modify, and distribute this software
 * for any purpose and without fee is hereby granted. The author
 * disclaims all warranties with regard to this software.
 *
 * Latest version: http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 */

/* The following two functions define the column width of an ISO 10646
 * character as follows:
 *
 *    - The null character (U+0000) has a column width of 0.
 *
 *    - Other C0/C1 control characters and DEL will lead to a return
 *      value of -1.
 *
 *    - Non-spacing and enclosing combining characters (general
 *      category code Mn or Me in the Unicode database) have a
 *      column width of 0.
 *
 *    - SOFT HYPHEN (U+00AD) has a column width of 1.
 *
 *    - Other format characters (general category code Cf in the Unicode
 *      database) and ZERO WIDTH SPACE (U+200B) have a column width of 0.
 *
 *    - Hangul Jamo medial vowels and final consonants (U+1160-U+11FF)
 *      have a column width of 0.
 *
 *    - Spacing characters in the East Asian Wide (W) or East Asian
 *      Full-width (F) category as defined in Unicode Technical
 *      Report #11 have a column width of 2.
 *
 *    - All remaining characters (including all printable
 *      ISO 8859-1 and WGL4 characters, Unicode control characters,
 *      etc.) have a column width of 1.
 *
 * This implementation assumes that wchar_t characters are encoded
 * in ISO 10646.
 */

/* Modified for JOE: returns printed width of control and other non-printable
   characters */

int joe_wcwidth(int wide,int ucs)
{
	/* If ANSI color sequences exist (ansi mode), they are 0 width */
	if (ucs & ANSI_BIT)
		return 0;

	/* If terminal is not UTF-8 or file is not UTF-8: width is 1 */
	/* FIXME */
	if (!locale_map->type || !wide)
		return 1;

	/* Negative characters are characters in range 128 - 255 converted from signed char to int */
	if (ucs < 0)
		ucs += 256;

	/* Printed width of non-printable characters */
	if (!cclass_lookup(cclass_print, ucs)) {
		if (ucs < 0x80) /* Ctrl-A is printed as underlined A in JOE */
			return 1;
		else if (ucs < 0x100) /* <FF> */
			return 4;
		else if (ucs < 0x1000) /* <FFF> */
			return 5;
		else if (ucs < 0x10000) /* <FFFF> */
			return 6;
		else if (ucs < 0x100000) /* <FFFFF> */
			return 7;
		else if (ucs < 0x1000000) /* <FFFFFF> */
			return 8;
		else if (ucs < 0x10000000) /* <FFFFFFF> */
			return 9;
		else /* <FFFFFFFF> */
			return 10;
	}

	/* Combining characters are merged with their start character so they have no width */
	if (cclass_lookup(cclass_combining, ucs))
		return 0;

	/* Some characters are double-width */
	if (cclass_lookup(cclass_double, ucs))
		return 2;
	
	return 1;
}

/* Return true if c is a control character which should not be sent directly
 * to the terminal, but should instead be displayed like <2028>.  joe_wcwidth gives
 * the displayed width of these control characters.
 */

int unictrl(int ucs)
{
	return !cclass_lookup(cclass_print, ucs);
}
