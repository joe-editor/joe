/*
 *	Functions which make use of the Unicode facts in unifacts.c
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

/* Get a character class containing all characters matching a particular Unicode category or block */

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
struct Cclass cclass_notalpha_[1];
int joe_iswalpha_(struct charmap *foo, int ch) { return cclass_lookup(cclass_alpha_, ch); }

struct Cclass cclass_alnum[1];
int joe_iswalnum(struct charmap *foo, int ch) { return cclass_lookup(cclass_alnum, ch); }

struct Cclass cclass_alnum_[1];
struct Cclass cclass_notalnum_[1];
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

	/* Alphabetical + underscores (name start character) */
	/* Java has: isJavaIdentifierStart: \p{Nl} \p{L} \p{Pc} \p{Sc} */
	/* Unicode has ID_Start: L + Nl + other_id_start - pattern_syntax - pattern_whitespace
	     from DerivedCoreProperties.txt:
	     see: http://unicode.org/reports/tr31/
	     and: http://unicode.org/reports/tr31/tr31-1.html#Pattern_Syntax */

	/* Used for XML and \i */
	cclass_init(cclass_alpha_);
	cclass_union(cclass_alpha_, unicode("L"));
	cclass_union(cclass_alpha_, unicode("Pc"));
/*	cclass_union(cclass_alpha_, unicode("Sc")); */
	cclass_union(cclass_alpha_, unicode("Nl"));
	cclass_opt(cclass_alpha_);

	/* \I */
	cclass_init(cclass_notalpha_);
	cclass_union(cclass_notalpha_, cclass_alpha_);
	cclass_inv(cclass_notalpha_);
	cclass_opt(cclass_notalpha_);

	/* Alphanumeric */
	cclass_init(cclass_alnum);
	cclass_union(cclass_alnum, unicode("L"));
	cclass_union(cclass_alnum, unicode("M"));
	cclass_union(cclass_alnum, unicode("N"));
	cclass_opt(cclass_alnum);

	/* Alphanumeric + underscores (name continuation character) */
	/* Java has: isJavaIdentifierPart: isJavaIdentifierStart \p{Mn} \p{Mc} \p{Nd} ignorable */
	/* Ignorable: 0x00-0x08, 0x0e-0x1b, 0x7f-0x9F */
	/* Unicode has ID_Continue: ID_Start + Mn + Mc + Nd + Pc + Other_ID_Continue - Pattern_syntax - Pattern_whitespace */
	/* Used for XML */
	/* \c */
	cclass_init(cclass_alnum_);
	cclass_union(cclass_alnum_, unicode("L"));
	cclass_union(cclass_alnum_, unicode("Pc"));
/*	cclass_union(cclass_alpha_, unicode("Sc")); */
	cclass_union(cclass_alpha_, unicode("Nl"));
	cclass_union(cclass_alnum_, unicode("Mn"));
	cclass_union(cclass_alnum_, unicode("Mc"));
	cclass_union(cclass_alnum_, unicode("Nd"));
	cclass_add(cclass_alnum_, 0x200c, 0x200d);
	cclass_opt(cclass_alnum_);

	/* \C */
	cclass_init(cclass_notalnum_);
	cclass_union(cclass_notalnum_, cclass_alnum_);
	cclass_inv(cclass_notalnum_);
	cclass_opt(cclass_notalnum_);

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
	cclass_union(cclass_print, unicode("Co")); /* Assume private use characters are printable */
	cclass_opt(cclass_print);

	/* Graphical characters (no spaces) */
	cclass_init(cclass_graph);
	cclass_union(cclass_graph, unicode("L"));
	cclass_union(cclass_graph, unicode("M"));
	cclass_union(cclass_graph, unicode("S"));
	cclass_union(cclass_graph, unicode("N"));
	cclass_union(cclass_graph, unicode("P"));
	cclass_union(cclass_print, unicode("Co")); /* Assume private use characters are graphical */
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

#ifdef JOEWIN

/* Provide standard implementation for PuTTY */

int wcwidth(int ucs)
{
	if (!ucs)
		return 0;
	if (ucs < 32 || (ucs >= 0x7f && ucs < 0xa0))
		return -1;

	if (cclass_lookup(cclass_combining, ucs))
		return 0;

	if (cclass_lookup(cclass_double, ucs))
		return 2;

	return 1;
}

/* TODO: Somehow incorporate this into unicat.c generation */
int wcwidth_cjk(int ucs)
{
	static const struct
	{
		int first;
		int last;
	} ambiguous[] = {
		{ 0x00A1, 0x00A1 }, { 0x00A4, 0x00A4 }, { 0x00A7, 0x00A8 },
		{ 0x00AA, 0x00AA }, { 0x00AE, 0x00AE }, { 0x00B0, 0x00B4 },
		{ 0x00B6, 0x00BA }, { 0x00BC, 0x00BF }, { 0x00C6, 0x00C6 },
		{ 0x00D0, 0x00D0 }, { 0x00D7, 0x00D8 }, { 0x00DE, 0x00E1 },
		{ 0x00E6, 0x00E6 }, { 0x00E8, 0x00EA }, { 0x00EC, 0x00ED },
		{ 0x00F0, 0x00F0 }, { 0x00F2, 0x00F3 }, { 0x00F7, 0x00FA },
		{ 0x00FC, 0x00FC }, { 0x00FE, 0x00FE }, { 0x0101, 0x0101 },
		{ 0x0111, 0x0111 }, { 0x0113, 0x0113 }, { 0x011B, 0x011B },
		{ 0x0126, 0x0127 }, { 0x012B, 0x012B }, { 0x0131, 0x0133 },
		{ 0x0138, 0x0138 }, { 0x013F, 0x0142 }, { 0x0144, 0x0144 },
		{ 0x0148, 0x014B }, { 0x014D, 0x014D }, { 0x0152, 0x0153 },
		{ 0x0166, 0x0167 }, { 0x016B, 0x016B }, { 0x01CE, 0x01CE },
		{ 0x01D0, 0x01D0 }, { 0x01D2, 0x01D2 }, { 0x01D4, 0x01D4 },
		{ 0x01D6, 0x01D6 }, { 0x01D8, 0x01D8 }, { 0x01DA, 0x01DA },
		{ 0x01DC, 0x01DC }, { 0x0251, 0x0251 }, { 0x0261, 0x0261 },
		{ 0x02C4, 0x02C4 }, { 0x02C7, 0x02C7 }, { 0x02C9, 0x02CB },
		{ 0x02CD, 0x02CD }, { 0x02D0, 0x02D0 }, { 0x02D8, 0x02DB },
		{ 0x02DD, 0x02DD }, { 0x02DF, 0x02DF }, { 0x0391, 0x03A1 },
		{ 0x03A3, 0x03A9 }, { 0x03B1, 0x03C1 }, { 0x03C3, 0x03C9 },
		{ 0x0401, 0x0401 }, { 0x0410, 0x044F }, { 0x0451, 0x0451 },
		{ 0x2010, 0x2010 }, { 0x2013, 0x2016 }, { 0x2018, 0x2019 },
		{ 0x201C, 0x201D }, { 0x2020, 0x2022 }, { 0x2024, 0x2027 },
		{ 0x2030, 0x2030 }, { 0x2032, 0x2033 }, { 0x2035, 0x2035 },
		{ 0x203B, 0x203B }, { 0x203E, 0x203E }, { 0x2074, 0x2074 },
		{ 0x207F, 0x207F }, { 0x2081, 0x2084 }, { 0x20AC, 0x20AC },
		{ 0x2103, 0x2103 }, { 0x2105, 0x2105 }, { 0x2109, 0x2109 },
		{ 0x2113, 0x2113 }, { 0x2116, 0x2116 }, { 0x2121, 0x2122 },
		{ 0x2126, 0x2126 }, { 0x212B, 0x212B }, { 0x2153, 0x2154 },
		{ 0x215B, 0x215E }, { 0x2160, 0x216B }, { 0x2170, 0x2179 },
		{ 0x2190, 0x2199 }, { 0x21B8, 0x21B9 }, { 0x21D2, 0x21D2 },
		{ 0x21D4, 0x21D4 }, { 0x21E7, 0x21E7 }, { 0x2200, 0x2200 },
		{ 0x2202, 0x2203 }, { 0x2207, 0x2208 }, { 0x220B, 0x220B },
		{ 0x220F, 0x220F }, { 0x2211, 0x2211 }, { 0x2215, 0x2215 },
		{ 0x221A, 0x221A }, { 0x221D, 0x2220 }, { 0x2223, 0x2223 },
		{ 0x2225, 0x2225 }, { 0x2227, 0x222C }, { 0x222E, 0x222E },
		{ 0x2234, 0x2237 }, { 0x223C, 0x223D }, { 0x2248, 0x2248 },
		{ 0x224C, 0x224C }, { 0x2252, 0x2252 }, { 0x2260, 0x2261 },
		{ 0x2264, 0x2267 }, { 0x226A, 0x226B }, { 0x226E, 0x226F },
		{ 0x2282, 0x2283 }, { 0x2286, 0x2287 }, { 0x2295, 0x2295 },
		{ 0x2299, 0x2299 }, { 0x22A5, 0x22A5 }, { 0x22BF, 0x22BF },
		{ 0x2312, 0x2312 }, { 0x2460, 0x24E9 }, { 0x24EB, 0x254B },
		{ 0x2550, 0x2573 }, { 0x2580, 0x258F }, { 0x2592, 0x2595 },
		{ 0x25A0, 0x25A1 }, { 0x25A3, 0x25A9 }, { 0x25B2, 0x25B3 },
		{ 0x25B6, 0x25B7 }, { 0x25BC, 0x25BD }, { 0x25C0, 0x25C1 },
		{ 0x25C6, 0x25C8 }, { 0x25CB, 0x25CB }, { 0x25CE, 0x25D1 },
		{ 0x25E2, 0x25E5 }, { 0x25EF, 0x25EF }, { 0x2605, 0x2606 },
		{ 0x2609, 0x2609 }, { 0x260E, 0x260F }, { 0x2614, 0x2615 },
		{ 0x261C, 0x261C }, { 0x261E, 0x261E }, { 0x2640, 0x2640 },
		{ 0x2642, 0x2642 }, { 0x2660, 0x2661 }, { 0x2663, 0x2665 },
		{ 0x2667, 0x266A }, { 0x266C, 0x266D }, { 0x266F, 0x266F },
		{ 0x273D, 0x273D }, { 0x2776, 0x277F }, { 0xE000, 0xF8FF },
		{ 0xFFFD, 0xFFFD }, { 0xF0000, 0xFFFFD }, { 0x100000, 0x10FFFD }
	};

	int left = 0, right = (SIZEOF(ambiguous) / SIZEOF(ambiguous[0])) - 1;

	while (left <= right) {
		int mid = (left + right) / 2;
		if (ucs < ambiguous[mid].first) {
			right = mid - 1;
		} else if (ucs > ambiguous[mid].last) {
			left = mid + 1;
		} else {
			return 2;
		}
	}

	return wcwidth(ucs);
}

#endif

/* Width of a string: was in qw.c.  Do we need both this and txtwidth?
   This one does not account for tabs. */

ptrdiff_t joe_wcswidth(struct charmap *map,const char *s, ptrdiff_t len)
{
	if (!map->type) {
		return len;
	} else {
		int width = 0;
		while (len) {
			int c = utf8_decode_fwrd(&s, &len);
			if (c >= 0) {
				width += joe_wcwidth(1, c);
			} else
				++width;
		}
		return width;
	}
}

/* Return true if c is a control character which should not be sent directly
 * to the terminal, but should instead be displayed like <2028>.  joe_wcwidth gives
 * the displayed width of these control characters.
 */

int unictrl(int ucs)
{
	return !cclass_lookup(cclass_print, ucs);
}

/* Copy character from one string to another */

void copy_c(char **d, const char **s)
{
	if (locale_map->type) {
		*d += utf8_encode(*d, utf8_decode_fwrd(s, NULL));
	} else if (**s) {
		**d = **s;
		(*s)++;
		(*d)++;
	}
}

/* Get next character from string and advance it, locale dependent */

int fwrd_c(struct charmap *map, const char **s, ptrdiff_t *len)
{
	if (map->type)
		return utf8_decode_fwrd(s, len);
	else {
		int c = *(const unsigned char *)*s;
		*s = *s + 1;
		if (len)
			*len = *len - 1;
		return c;
	}
}
