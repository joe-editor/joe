/*
 *	JOE options
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#include "types.h"

#define HEX_RESTORE_UTF8	2
#define HEX_RESTORE_CRLF	4
#define HEX_RESTORE_INSERT	8
#define HEX_RESTORE_WORDWRAP	16
#define HEX_RESTORE_AUTOINDENT	32
#define HEX_RESTORE_ANSI	64
#define HEX_RESTORE_PICTURE	128
#define OPT_BUF_SIZE 300

const char *aborthint = "^C";
const char *helphint = "^K H";

OPTIONS *options_list = NULL; /* File name dependent list of options */

/* Default options for prompt windows */

OPTIONS pdefault = {
	NULL,		/* *next */
	"prompt",	/* ftype */
	NULL,		/* *match */
	0,		/* overtype */
	0,		/* lmargin */
	76,		/* rmargin */
	0,		/* autoindent */
	0,		/* wordwrap */
	0,		/* nobackup */
	8,		/* tab */
	' ',		/* indent char */
	1,		/* indent step */
	NULL,		/* *context */
	NULL,		/* *lmsg */
	NULL,		/* *rmsg */
	NULL,		/* *smsg */
	NULL,		/* *zmsg */
	0,		/* line numbers */
	0,		/* highlight current line */
	0,		/* read only */
	0,		/* french spacing */
	0,		/* flowed text */
	0,		/* spaces */
#ifdef __MSDOS__
	1,		/* crlf */
#else
	0,		/* crlf */
#endif
	0,		/* Highlight */
	0,		/* Visible whitespace */
	NULL,		/* Syntax name */
	NULL,		/* Syntax */
	NULL,		/* Name of character set */
	NULL,		/* Character set */
	NULL,		/* Language */
	0,		/* Smart home key */
	0,		/* Goto indent first */
	0,		/* Smart backspace key */
	0,		/* Purify indentation */
	0,		/* Picture mode */
	0,		/* highlighter_context */
	0,		/* single_quoted */
	0,		/* no_double_quoted */
	0,		/* c_comment */
	0,		/* cpp_comment */
	0,		/* pound_comment */
	0,		/* vhdl_comment */
	0,		/* semi_comment */
	0,		/* tex_comment */
	0,		/* hex */
	0,		/* hide ansi */
	0,		/* status line context */
	NULL,		/* text_delimiters */
	NULL,		/* Characters which can indent paragraphs */
	NULL,		/* Characters which begin non-paragraph lines */
	NULL,		/* macro to execute for new files */
	NULL,		/* macro to execute for existing files */
	NULL,		/* macro to execute before saving new files */
	NULL,		/* macro to execute before saving existing files */
	NULL		/* macro to execute on first change */
};

/* Default options for file windows */

OPTIONS fdefault = {
	NULL,		/* *next */
	"default",	/* ftype */
	NULL,		/* *match */
	0,		/* overtype */
	0,		/* lmargin */
	76,		/* rmargin */
	0,		/* autoindent */
	0,		/* wordwrap */
	0,		/* nobackup */
	8,		/* tab */
	' ',		/* indent char */
	1,		/* indent step */
	"main",		/* *context */
	"\\i%n %m %M",	/* *lmsg */
	" %S Ctrl-K H for help",	/* *rmsg */
	NULL,		/* *smsg */
	NULL,		/* *zmsg */
	0,		/* line numbers */
	0,		/* higlight current line */
	0,		/* read only */
	0,		/* french spacing */
	0,		/* flowed text */
	0,		/* spaces */
#ifdef __MSDOS__
	1,		/* crlf */
#else
	0,		/* crlf */
#endif
	0,		/* Highlight */
	0,		/* Visible whitespace */
	NULL,		/* Syntax name */
	NULL,		/* Syntax */
	NULL,		/* Name of character set */
	NULL,		/* Character set */
	NULL,		/* Language */
	0,		/* Smart home key */
	0,		/* Goto indent first */
	0,		/* Smart backspace key */
	0,		/* Purity indentation */
	0,		/* Picture mode */
	0,		/* highlighter_context */
	0,		/* single_quoted */
	0,		/* no_double_quoted */
	0,		/* c_comment */
	0,		/* cpp_comment */
	0,		/* pound_comment */
	0,		/* vhdl_comment */
	0,		/* semi_comment */
	0,		/* tex_comment */
	0,		/* hex */
	0,		/* hide ansi */
	0,		/* status line context */
	NULL,		/* text_delimiters */
	">;!#%/",	/* Characters which can indent paragraphs */
	".",	/* Characters which begin non-paragraph lines */
	NULL, NULL, NULL, NULL, NULL	/* macros (see above) */
};

/* Commands which just type in variable values */

int ucharset(W *w, int k)
{
	const char *s;
	BW *bw;
	WIND_BW(bw, w);
	w = w->main;
	s = ((BW *)w->object)->o.charmap->name;
	if (!s || !*s)
		return -1;
	while (*s)
		if (utypebw(bw,*(const unsigned char *)s++))
			return -1;
	return 0;
}

int ulanguage(W *w, int k)
{
	BW *bw;
	const char *s;
	WIND_BW(bw, w);
	w = bw->parent->main;
	s = ((BW *)w->object)->o.language;
	if (!s || !*s)
		return -1;
	while (*s)
		if (utypebw(bw,*(const unsigned char *)s++))
			return -1;
	return 0;
}

/* Update options
 * This determines option values based on option names, for example it looks up the character map based on name */

void lazy_opts(B *b, OPTIONS *o)
{
	struct charmap *orgmap = b->o.charmap; /* Remember guessed character map */
	o->syntax = load_syntax(o->syntax_name);
	b->o = *o;
	if (!b->o.map_name || !(b->o.charmap = find_charmap(b->o.map_name))) {
		/* Use the guessed one if it wasn't explicitly given */
		b->o.charmap = orgmap;
		if (orgmap)
			b->o.map_name = orgmap->name;
	}
	if (!b->o.charmap)
		b->o.charmap = locale_map;
	if (!b->o.language)
		b->o.language = locale_msgs;
	if (b->o.hex) {
		/* Hex not allowed with UTF-8 */
		if (b->o.charmap->type) {
			b->o.charmap = find_charmap("c");
			b->o.hex |= HEX_RESTORE_UTF8;
		}

		/* Hex not allowed with CRLF */
		if (b->o.crlf) {
			b->o.crlf = 0;
			b->o.hex |= HEX_RESTORE_CRLF;
		}

		if (!b->o.overtype) {
			b->o.overtype = 1;
			b->o.hex |= HEX_RESTORE_INSERT;
		}

		if (b->o.wordwrap) {
			b->o.wordwrap = 0;
			b->o.hex |= HEX_RESTORE_WORDWRAP;
		}

		if (b->o.autoindent) {
			b->o.autoindent = 0;
			b->o.hex |= HEX_RESTORE_AUTOINDENT;
		}

		if (b->o.ansi) {
			b->o.ansi = 0;
			b->o.hex |= HEX_RESTORE_ANSI;
		}

		if (b->o.picture) {
			b->o.picture = 0;
			b->o.hex |= HEX_RESTORE_PICTURE;
		}
	}


}

/* Set local options depending on file name and contents */

void setopt(B *b, const char *parsed_name)
{
	OPTIONS *o;

	for (o = options_list; o; o = o->next) {
		struct options_match *match;
		for (match = o->match; match; match = match->next) {
			if (rmatch(match->name_regex, parsed_name)) {
				if(match->contents_regex) {
					P *p = pdup(b->bof, "setopt");
					if (!match->r_contents_regex)
						match->r_contents_regex = joe_regcomp(ascii_map, match->contents_regex, zlen(match->contents_regex), 0, 1, 0);
					if (match->r_contents_regex && !joe_regexec(match->r_contents_regex, p, 0, 0, 0)) {
						prm(p);
						lazy_opts(b, o);
						goto done;
					} else {
						prm(p);
					}
				} else {
					lazy_opts(b, o);
					goto done;
				}
			}
		}
	}

	lazy_opts(b, &fdefault);

	done:;
}

/* Table of options and how to set them */

/* local means it's in an OPTION structure, global means it's in a global
 * variable */

struct glopts {
	const char *name;		/* Option name */
	int type;		/*      0 for global option flag
				   1 for global option int
				   2 for global option string (in locale encoding)
				   4 for local option flag
				   5 for local option int
				   14 for local option off_t
				   6 for local option string (in utf8)
				   7 for local option off_t+1, with range checking
				   9 for syntax (options->syntax_name)
				   13 for byte encoding (options->map_name)
				   15 for file type (options->ftype)
				   17 for color scheme
				 */
	void *set;		/* Address of global option */
	const char *addr;		/* Local options structure member address */
	const char *yes;		/* Message if option was turned on, or prompt string */
	const char *no;		/* Message if option was turned off */
	const char *menu;		/* Menu string */
	ptrdiff_t ofst;		/* Local options structure member offset */
	int low;		/* Low limit for numeric options */
	int high;		/* High limit for numeric options */
} glopts[] = {
	{"overwrite",4, NULL, (char *) &fdefault.overtype, _("Overtype mode"), _("Insert mode"), _("Overtype mode"), 0, 0, 0 },
	{"hex",4, NULL, (char *) &fdefault.hex, _("Hex edit mode"), _("Text edit mode"), _("Hex edit display mode"), 0, 0, 0 },
	{"ansi",4, NULL, (char *) &fdefault.ansi, _("Hide ANSI sequences"), _("Reveal ANSI sequences"), _("Hide ANSI mode"), 0, 0, 0 },
	{"title",4, NULL, (char *) &fdefault.title, _("Status line context enabled"), _("Status line context disabled"), _("Status line context display mode"), 0, 0, 0 },
	{"autoindent",	4, NULL, (char *) &fdefault.autoindent, _("Autoindent enabled"), _("Autoindent disabled"), _("Autoindent mode"), 0, 0, 0 },
	{"wordwrap",	4, NULL, (char *) &fdefault.wordwrap, _("Wordwrap enabled"), _("Wordwrap disabled"), _("Word wrap mode"), 0, 0, 0 },
	{"tab",	14, NULL, (char *) &fdefault.tab, _("Tab width (%lld): "), 0, _("Tab width"), 0, 1, 64 },
	{"lmargin",	7, NULL, (char *) &fdefault.lmargin, _("Left margin (%d): "), 0, _("Left margin "), 0, 0, 63 },
	{"rmargin",	7, NULL, (char *) &fdefault.rmargin, _("Right margin (%d): "), 0, _("Right margin "), 0, 7, 255 },
	{"restore",	0, &restore_file_pos, NULL, _("Restore cursor position when files loaded"), _("Don't restore cursor when files loaded"), _("Restore cursor mode"), 0, 0, 0 },
	{"regex",	0, &std_regex, NULL, _("Standard regular expression format"), _("JOE regular expression format"), _("Standard or JOE regular expression syntax"), 0, 0, 0 },
	{"square",	0, &square, NULL, _("Rectangle mode"), _("Text-stream mode"), _("Rectangular region mode"), 0, 0, 0 },
	{"icase",	0, &opt_icase, NULL, _("Search ignores case by default"), _("Case sensitive search by default"), _("Case insensitive search mode "), 0, 0, 0 },
	{"wrap",	0, &wrap, NULL, _("Search wraps"), _("Search doesn't wrap"), _("Search wraps mode"), 0, 0, 0 },
	{"menu_explorer",	0, &menu_explorer, NULL, _("Menu explorer mode"), _("Simple completion mode"), _("Menu explorer mode"), 0, 0, 0 },
	{"menu_above",	0, &menu_above, NULL, _("Menu above prompt"), _("Menu below prompt"), _("Menu above/below mode"), 0, 0, 0 },
	{"notagsmenu",	0, &notagsmenu, NULL, _("Tags menu disabled"), _("Tags menu enabled"), _("Tags menu mode"), 0, 0, 0 },
	{"search_prompting",	0, &pico, NULL, _("Search prompting on"), _("Search prompting off"), _("Search prompting mode"), 0, 0, 0 },
	{"menu_jump",	0, &menu_jump, NULL, _("Jump into menu is on"), _("Jump into menu is off"), _("Jump into menu mode"), 0, 0, 0 },
	{"autoswap",	0, &autoswap, NULL, _("Autoswap ^KB and ^KK"), _("Autoswap off "), _("Autoswap mode "), 0, 0, 0 },
	{"indentc",	5, NULL, (char *) &fdefault.indentc, _("Indent char %d (SPACE=32, TAB=9, %{abort} to abort): "), 0, _("Indent char "), 0, 0, 255 },
	{"istep",	14, NULL, (char *) &fdefault.istep, _("Indent step %lld (%{abort} to abort): "), 0, _("Indent step "), 0, 1, 64 },
	{"french",	4, NULL, (char *) &fdefault.french, _("One space after periods for paragraph reformat"), _("Two spaces after periods for paragraph reformat"), _("French spacing mode"), 0, 0, 0 },
	{"flowed",	4, NULL, (char *) &fdefault.flowed, _("One space after paragraph line"), _("No spaces after paragraph lines"), _("Flowed text mode"), 0, 0, 0 },
	{"highlight",	4, NULL, (char *) &fdefault.highlight, _("Highlighting enabled"), _("Highlighting disabled"), _("Syntax highlighting mode"), 0, 0, 0 },
	{"spaces",	4, NULL, (char *) &fdefault.spaces, _("Inserting spaces when tab key is hit"), _("Inserting tabs when tab key is hit"), _("No tabs mode"), 0, 0, 0 },
	{"mid",	0, &opt_mid, NULL, _("Cursor will be recentered on scrolls"), _("Cursor will not be recentered on scroll"), _("Center on scroll mode"), 0, 0, 0 },
	{"left", 1, &opt_left, NULL, _("Columns to scroll left or -1 for 1/2 window (%d): "), 0, _("Left scroll amount"), 0, -128, 127 },
	{"right", 1, &opt_right, NULL, _("Columns to scroll right or -1 for 1/2 window (%d): "), 0, _("Right scroll amount"), 0, -128, 127 },
	{"guess_crlf",0, &guesscrlf, NULL, _("Automatically detect MS-DOS files"), _("Do not automatically detect MS-DOS files"), _("Auto detect CR-LF mode"), 0, 0, 0 },
	{"guess_indent",0, &guessindent, NULL, _("Automatically detect indentation"), _("Do not automatically detect indentation"), _("Guess indent mode"), 0, 0, 0 },
	{"guess_non_utf8",0, &guess_non_utf8, NULL, _("Automatically detect non-UTF-8 in UTF-8 locale"), _("Do not automatically detect non-UTF-8"), _("Guess non-UTF-8 mode"), 0, 0, 0 },
	{"guess_utf8",0, &guess_utf8, NULL, _("Automatically detect UTF-8 in non-UTF-8 locale"), _("Do not automatically detect UTF-8"), _("Guess UTF-8 mode"), 0, 0, 0 },
	{"guess_utf16",0, &guess_utf16, NULL, _("Automatically detect UTF-16"), _("Do not automatically detect UTF-16"), _("Guess UTF-16 mode"), 0, 0, 0 },
	{"transpose",0, &transpose, NULL, _("Menu is transposed"), _("Menus are not transposed"), _("Transpose menus mode"), 0, 0, 0 },
	{"crlf",	4, NULL, (char *) &fdefault.crlf, _("CR-LF is line terminator"), _("LF is line terminator"), _("CR-LF (MS-DOS) mode"), 0, 0, 0 },
	{"linums",	4, NULL, (char *) &fdefault.linums, _("Line numbers enabled"), _("Line numbers disabled"), _("Line numbers mode"), 0, 0, 0 },
	{"hiline",	4, NULL, (char *) &fdefault.hiline, _("Highlighting cursor line"), _("Not highlighting cursor line"), _("Highlight cursor line"), 0, 0, 0 },
	{"marking",	0, &marking, NULL, _("Anchored block marking on"), _("Anchored block marking off"), _("Region marking mode"), 0, 0, 0 },
	{"asis",	0, &dspasis, NULL, _("Characters above 127 shown as-is"), _("Characters above 127 shown in inverse"), _("Display meta chars as-is mode"), 0, 0, 0 },
	{"force",	0, &force, NULL, _("Last line forced to have NL when file saved"), _("Last line not forced to have NL"), _("Force last NL mode"), 0, 0, 0 },
	{"joe_state",0, &joe_state, NULL, _("~/.joe_state file will be updated"), _("~/.joe_state file will not be updated"), _("Joe_state file mode"), 0, 0, 0 },
	{"nobackup",	4, NULL, (char *) &fdefault.nobackup, _("Nobackup enabled"), _("Nobackup disabled"), _("No backup mode"), 0, 0, 0 },
	{"nobackups",	0, &nobackups, NULL, _("Backup files will not be made"), _("Backup files will be made"), _("Disable backups mode"), 0, 0, 0 },
	{"nodeadjoe",	0, &nodeadjoe, NULL, _("DEADJOE files will not be made"), _("DEADJOE files will be made"), _("Disable DEADJOE mode"), 0, 0, 0 },
	{"nolocks",	0, &nolocks, NULL, _("Files will not be locked"), _("Files will be locked"), _("Disable locks mode"), 0, 0, 0 },
	{"nomodcheck",	0, &nomodcheck, NULL, _("No file modification time check"), _("File modification time checking enabled"), _("Disable mtime check mode"), 0, 0, 0 },
	{"nocurdir",	0, &nocurdir, NULL, _("No current dir"), _("Current dir enabled"), _("Disable current dir "), 0, 0, 0 },
	{"break_hardlinks",	0, &break_links, NULL, _("Hardlinks will be broken"), _("Hardlinks not broken"), _("Break hard links "), 0, 0, 0 },
	{"break_links",	0, &break_symlinks, NULL, _("Links will be broken"), _("Links not broken"), _("Break links "), 0, 0, 0 },
	{"lightoff",	0, &lightoff, NULL, _("Highlighting turned off after block operations"), _("Highlighting not turned off after block operations"), _("Auto unmark "), 0, 0, 0 },
	{"exask",	0, &exask, NULL, _("Prompt for filename in save & exit command"), _("Don't prompt for filename in save & exit command"), _("Exit ask "), 0, 0, 0 },
	{"beep",	0, &joe_beep, NULL, _("Warning bell enabled"), _("Warning bell disabled"), _("Beeps "), 0, 0, 0 },
	{"nosta",	0, &staen, NULL, _("Top-most status line disabled"), _("Top-most status line enabled"), _("Disable status line "), 0, 0, 0 },
	{"keepup",	0, &keepup, NULL, _("Status line updated constantly"), _("Status line updated once/sec"), _("Fast status line "), 0, 0, 0 },
	{"pg",		1, &pgamnt, NULL, _("Lines to keep for PgUp/PgDn or -1 for 1/2 window (%d): "), 0, _("No. PgUp/PgDn lines "), 0, -1, 64 },
	{"undo_keep",		1, &undo_keep, NULL, _("No. undo records to keep, or (0 for infinite): "), 0, _("No. undo records "), 0, -1, 64 },
	{"csmode",	0, &csmode, NULL, _("Start search after a search repeats previous search"), _("Start search always starts a new search"), _("Continued search "), 0, 0, 0 },
	{"rdonly",	4, NULL, (char *) &fdefault.readonly, _("Read only"), _("Full editing"), _("Read only "), 0, 0, 0 },
	{"smarthome",	4, NULL, (char *) &fdefault.smarthome, _("Smart home key enabled"), _("Smart home key disabled"), _("Smart home key "), 0, 0, 0 },
	{"indentfirst",	4, NULL, (char *) &fdefault.indentfirst, _("Smart home goes to indentation first"), _("Smart home goes home first"), _("To indent first "), 0, 0, 0 },
	{"smartbacks",	4, NULL, (char *) &fdefault.smartbacks, _("Smart backspace key enabled"), _("Smart backspace key disabled"), _("Smart backspace "), 0, 0, 0 },
	{"purify",	4, NULL, (char *) &fdefault.purify, _("Indentation clean up enabled"), _("Indentation clean up disabled"), _("Clean up indents "), 0, 0, 0 },
	{"picture",	4, NULL, (char *) &fdefault.picture, _("Picture drawing mode enabled"), _("Picture drawing mode disabled"), _("Picture mode "), 0, 0, 0 },
	{"backpath",	2, &backpath, NULL, _("Backup files stored in (%s): "), 0, _("Path to backup files "), 0, 0, 0 },
	{"syntax",	9, NULL, NULL, _("Select syntax (%{abort} to abort): "), 0, _("Syntax"), 0, 0, 0 },
	{"colors",	17, NULL, NULL, _("Select color scheme (%{abort} to abort): "), 0, _("Scheme "), 0, 0, 0 },
	{"encoding",13, NULL, NULL, _("Select file character set (%{abort} to abort): "), 0, _("Encoding "), 0, 0, 0 },
	{"type",	15, NULL, NULL, _("Select file type (%{abort} to abort): "), 0, _("File type "), 0, 0, 0 },
	{"highlighter_context",	4, NULL, (char *) &fdefault.highlighter_context, _("Highlighter context enabled"), _("Highlighter context disabled"), _("^G uses highlighter context "), 0, 0, 0 },
	{"single_quoted",	4, NULL, (char *) &fdefault.single_quoted, _("Single quoting enabled"), _("Single quoting disabled"), _("^G ignores '... ' "), 0, 0, 0 },
	{"no_double_quoted",4, NULL, (char *) &fdefault.no_double_quoted, _("Double quoting disabled"), _("Double quoting enabled"), _("^G ignores \"... \" "), 0, 0, 0 },
	{"c_comment",	4, NULL, (char *) &fdefault.c_comment, _("/* comments enabled"), _("/* comments disabled"), _("^G ignores /*...*/ "), 0, 0, 0 },
	{"cpp_comment",	4, NULL, (char *) &fdefault.cpp_comment, _("// comments enabled"), _("// comments disabled"), _("^G ignores //... "), 0, 0, 0 },
	{"pound_comment",	4, NULL, (char *) &fdefault.pound_comment, _("# comments enabled"), _("# comments disabled"), _("^G ignores #... "), 0, 0, 0 },
	{"vhdl_comment",	4, NULL, (char *) &fdefault.vhdl_comment, _("-- comments enabled"), _("-- comments disabled"), _("^G ignores --... "), 0, 0, 0 },
	{"semi_comment",	4, NULL, (char *) &fdefault.semi_comment, _("; comments enabled"), _("; comments disabled"), _("^G ignores ;... "), 0, 0, 0 },
	{"tex_comment",	4, NULL, (char *) &fdefault.tex_comment, _("% comments enabled"), _("% comments disabled"), _("^G ignores %... "), 0, 0, 0 },
	{"text_delimiters",	6, NULL, (char *) &fdefault.text_delimiters, _("Text delimiters (%s): "), 0, _("Text delimiters "), 0, 0, 0 },
	{"language",	6, NULL, (char *) &fdefault.language, _("Language (%s): "), 0, _("Language "), 0, 0, 0 },
	{"cpara",		6, NULL, (char *) &fdefault.cpara, _("Characters which can indent paragraphs (%s): "), 0, _("Paragraph indent chars "), 0, 0, 0 },
	{"cnotpara",	6, NULL, (char *) &fdefault.cnotpara, _("Characters which begin non-paragraph lines (%s): "), 0, _("Non-paragraph chars "), 0, 0, 0 },
	{"floatmouse",	0, &floatmouse, 0, _("Clicking can move the cursor past end of line"), _("Clicking past end of line moves cursor to the end"), _("Click past end "), 0, 0, 0 },
	{"rtbutton",	0, &rtbutton, 0, _("Mouse action is done with the right button"), _("Mouse action is done with the left button"), _("Right button "), 0, 0, 0 },
	{"nonotice",	0, &nonotice, NULL, 0, 0, _("Suppress startup notice"), 0, 0, 0 },
	{"noexmsg",	0, &noexmsg, NULL, 0, 0, _("Suppress exit message"), 0, 0, 0 },
	{"help_is_utf8",	0, &help_is_utf8, NULL, 0, 0, _("Help is UTF-8"), 0, 0, 0 },
	{"noxon",	0, &noxon, NULL, 0, 0, _("Disable XON/XOFF"), 0, 0, 0 },
	{"orphan",	0, &orphan, NULL, 0, 0, _("Orphan extra files"), 0, 0, 0 },
	{"helpon",	0, &helpon, NULL, 0, 0, _("Start editor with help displayed"), 0, 0, 0 },
	{"dopadding",	0, &dopadding, NULL, 0, 0, _("Emit padding NULs"), 0, 0, 0 },
	{"lines",	1, &env_lines, NULL, 0, 0, _("No. screen lines (if no window size ioctl)"), 0, 2, 1024 },
	{"baud",	1, &Baud, NULL, 0, 0, _("Baud rate"), 0, 50, 32767 },
	{"columns",	1, &env_columns, NULL, 0, 0, _("No. screen columns (if no window size ioctl)"), 0, 2, 1024 },
	{"skiptop",	1, &skiptop, NULL, 0, 0, _("No. screen lines to skip"), 0, 0, 64 },
	{"notite",	0, &notite, NULL, 0, 0, _("Suppress tty init sequence"), 0, 0, 0 },
	{"brpaste",	0, &brpaste, NULL, 0, 0, _("Bracketed paste mode"), 0, 0, 0 },
	{"pastehack",	0, &pastehack, NULL, 0, 0, _("Paste quoting hack"), 0, 0, 0 },
	{"nolinefeeds",	0, &nolinefeeds, NULL, 0, 0, _("Suppress history preserving linefeeds"), 0, 0, 0 },
	{"mouse",	0, &xmouse, NULL, 0, 0, _("Enable mouse"), 0, 0, 0 },
	{"usetabs",	0, &opt_usetabs, NULL, 0, 0, _("Screen update uses tabs"), 0, 0, 0 },
	{"assume_color", 0, &assume_color, NULL, 0, 0, _("Assume terminal supports color"), 0, 0, 0 },
	{"assume_256color", 0, &assume_256color, NULL, 0, 0, _("Assume terminal supports 256 colors"), 0, 0, 0 },
	{"joexterm",	0, &joexterm, NULL, 0, 0, _("Assume xterm patched for JOE"), 0, 0, 0 },
	{"visiblews",	4, NULL, (char *) &fdefault.visiblews, _("Visible whitespace enabled"), _("Visible whitespace disabled"), _("Visible whitespace"), 0, 0, 0 },
	{ NULL,		0, NULL, NULL, NULL, NULL, NULL, 0, 0, 0 }
};

/* Print command line help */

void cmd_help(int type)
{
	int x;

	for (x = 0; glopts[x].name; ++x) {
		char buf[80];
		buf[0] = 0;
		if ((type == 0 && glopts[x].type < 3) || (type == 1 && glopts[x].type >= 3)) {
			if (glopts[x].type == 0 || glopts[x].type == 4)
				joe_snprintf_1(buf, SIZEOF(buf), "-[-]%s", glopts[x].name);
			else if (glopts[x].type == 1 || glopts[x].type == 5 || glopts[x].type == 14 || glopts[x].type == 7)
				joe_snprintf_1(buf, SIZEOF(buf), "-%s nnn", glopts[x].name);
			else if (glopts[x].type == 2 || glopts[x].type == 6 || glopts[x].type == 9 || glopts[x].type == 13 || glopts[x].type == 15 || glopts[x].type == 17)
				joe_snprintf_1(buf, SIZEOF(buf), "-%s sss", glopts[x].name);
			if (glopts[x].menu)
				printf("    %-23s %s\n", buf, glopts[x].menu);
			else
				printf("    %-23s\n", buf);
		}
	}

	if (type == 0) {
		printf("    %-23s %s\n", "-xmsg sss", "Exit message");
		printf("    %-23s %s\n", "-aborthint sss", "Abort hint");
		printf("    %-23s %s\n", "-helphint sss", "Help hint");
		printf("    %-23s %s\n", "-lmsg sss", "Left side status line format");
		printf("    %-23s %s\n", "-rmsg sss", "Right side status line format");
		printf("    %-23s %s\n", "-smsg sss", "Status command format");
		printf("    %-23s %s\n", "-zmsg sss", "Status command format EOF");
		printf("    %-23s %s\n", "-keymap sss", "Keymap to use");
		printf("    %-23s %s\n", "-mnew sss", "Macro to execute for new files");
		printf("    %-23s %s\n", "-mfirst sss", "Macro to execute on first change");
		printf("    %-23s %s\n", "-mold sss", "Macro to execute on existing files");
		printf("    %-23s %s\n", "-msnew sss", "Macro to execute when new files are saved");
		printf("    %-23s %s\n", "-msold sss", "Macro to execute when existing files are saved");
		printf("    %-23s %s\n", "-text_color sss", "Text color");
		printf("    %-23s %s\n", "-help_color sss", "Help color");
		printf("    %-23s %s\n", "-status_color sss", "Status bar color");
		printf("    %-23s %s\n", "-menu_color sss", "Menu color");
		printf("    %-23s %s\n", "-prompt_color sss", "Prompt color");
		printf("    %-23s %s\n", "-msg_color sss", "Message color");
	}
}

/* Initialize .ofsts above.  Is this really necessary? */

int isiz = 0;
HASH *opt_tab;

static void izopts(void)
{
	int x;

	opt_tab = htmk(128);

	for (x = 0; glopts[x].name; ++x) {
		htadd(opt_tab, glopts[x].name, glopts + x);
		switch (glopts[x].type) {
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 14:
			glopts[x].ofst = glopts[x].addr - (char *) &fdefault;
		}
	}
	isiz = 1;
}

/* Set file type option */

static char **getftypes(void)
{
	OPTIONS *o;
	char **s = vaensure(NULL, 20);

	for (o = options_list; o; o = o->next)
		s = vaadd(s, vsncpy(NULL, 0, sz(o->ftype)));
	vasort(s, aLen(s));
	return s;
}

char **ftypes = NULL;	/* Array of file types */

static int ftypecmplt(BW *bw, int k)
{
	if (!ftypes)
		ftypes = getftypes();
	return simple_cmplt(bw, ftypes);
}

static OPTIONS *find_ftype(const char *s)
{
	OPTIONS *o;
	for (o = options_list; o; o = o->next)
		if (!zcmp(o->ftype, s))
			break;
	return o;
}

static int doftype(W *w, char *s, void *object, int *notify)
{
	BW *bw;
	OPTIONS *o = find_ftype(s);
	WIND_BW(bw, w);
	vsrm(s);
	if (!o) {
		msgnw(bw->parent, joe_gettext(_("No such file type")));
		if (notify)
			*notify = 1;
		return -1;
	} else {
		lazy_opts(bw->b, o);
		bw->o = bw->b->o;
		return 0;
	}
}

B *ftypehist = NULL;

/* Set a global or local option:
 * 's' is option name
 * 'arg' is a possible argument string (taken only if option has an arg)
 * 'options' points to options structure to modify (can be NULL).
 * 'set'==0: set only in 'options' if it's given.
 * 'set'!=0: set global variable option.
 * return value: no. of fields taken (1 or 2), or 0 if option not found.
 *
 * So this function is used both to set options, and to parse over options
 * without setting them.
 *
 * These combinations are used:
 *
 * glopt(name,arg,NULL,1): set global variable option
 * glopt(name,arg,NULL,0): parse over option
 * glopt(name,arg,options,0): set file local option
 * glopt(name,arg,&fdefault,1): set default file options
 * glopt(name,arg,options,1): set file local option
 */

int glopt(char *s, char *arg, OPTIONS *options, int set)
{
	int val;
	int ret = 0;
	int st = 1;	/* 1 to set option, 0 to clear it */
	struct glopts *opt;

	/* Initialize offsets */
	if (!isiz)
		izopts();

	/* Clear instead of set? */
	if (s[0] == '-') {
		st = 0;
		++s;
	}

	opt = (struct glopts *)htfind(opt_tab, s);

	if (opt) {
		switch (opt->type) {
		case 0: /* Global variable flag option */
			if (set)
				*(int *)opt->set = st;
			ret = 1;
			break;
		case 1: /* Global variable integer option */
			if (set && arg) {
				val = ztoi(arg);
				if (val >= opt->low && val <= opt->high)
					*(int *)opt->set = val;
			}
			if (arg)
				ret = 2;
			else
				ret = 1;
			break;
		case 2: /* Global variable string option */
			if (set) {
				if (arg)
					*(char **) opt->set = zdup(arg);
				else
					*(char **) opt->set = 0;
			}
			if (arg)
				ret = 2;
			else
				ret = 1;
			break;
		case 4: /* Local option flag */
			if (options)
				*(int *) ((char *) options + opt->ofst) = st;
			ret = 1;
			break;
		case 5: /* Local option integer */
			if (arg) {
				if (options) {
					val = ztoi(arg);
					if (val >= opt->low && val <= opt->high)
						*(int *) ((char *)
							  options + opt->ofst) = val;
				}
			}
			if (arg)
				ret = 2;
			else
				ret = 1;
			break;
		case 14: /* Local option off_t */
			if (arg) {
				if (options) {
					off_t zz = ztoo(arg);
					if (zz >= opt->low && zz <= opt->high)
						*(off_t *) ((char *)
							  options + opt->ofst) = zz;
				}
			}
			if (arg)
				ret = 2;
			else
				ret = 1;
			break;
		case 15: /* Set file type */
			if (arg && options) {
				OPTIONS *o = find_ftype(arg);
				if (o) {
					*options = *o;
				}
			}
			if (arg)
				ret = 2;
			else
				ret = 1;
			break;
		case 6: /* Local string option */
			if (options) {
				if (arg) {
					*(char **) ((char *)
							  options + opt->ofst) = zdup(arg);
				} else {
					*(char **) ((char *)
							  options + opt->ofst) = 0;
				}
			}
			if (arg)
				ret = 2;
			else
				ret = 1;
			break;
		case 7: /* Local option numeric + 1, with range checking */
			if (arg) {
				off_t zz = ztoo(arg);
				if (zz >= opt->low && zz <= opt->high) {
					--zz;
					if (options)
						*(off_t *) ((char *)
							  options + opt->ofst) = zz;
				}
			}
			if (arg)
				ret = 2;
			else
				ret = 1;
			break;

		case 9: /* Set syntax */
			if (arg && options)
				options->syntax_name = zdup(arg);
			/* this was causing all syntax files to be loaded...
			if (arg && options)
				options->syntax = load_syntax(arg); */
			if (arg)
				ret = 2;
			else
				ret = 1;
			break;

		case 13: /* Set byte mode encoding */
			if (arg && options)
				options->map_name = zdup(arg);
			if (arg)
				ret = 2;
			else
				ret = 1;
			break;

		case 17: /* Set color scheme */
			if (arg) {
				scheme_name = zdup(arg);
				ret = 2;
			} else
				ret = 1;
			break;
		}
	} else {
		/* Why no case 6, string option? */
		/* Keymap, mold, mnew, etc. are not strings */
		/* These options do not show up in ^T */
		if (!zcmp(s, "xmsg")) {
			if (arg) {
				xmsg = zdup(arg);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "aborthint")) {
			if (arg) {
				aborthint = zdup(arg);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "helphint")) {
			if (arg) {
				helphint = zdup(arg);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "lmsg")) {
			if (arg) {
				if (options)
					options->lmsg = zdup(arg);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "rmsg")) {
			if (arg) {
				if (options)
					options->rmsg = zdup(arg);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "smsg")) {
			if (arg) {
				if (options)
					options->smsg = zdup(arg);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "zmsg")) {
			if (arg) {
				if (options)
					options->zmsg = zdup(arg);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "keymap")) {
			if (arg) {
				int y;

				for (y = 0; !joe_isspace(locale_map,arg[y]); ++y) ;
				if (!arg[y])
					arg[y] = 0;
				if (options && y)
					options->context = zdup(arg);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "mnew")) {
			if (arg) {
				ptrdiff_t sta;

				if (options)
					options->mnew = mparse(NULL, arg, &sta, 0);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "mfirst")) {
			if (arg) {
				ptrdiff_t sta;

				if (options)
					options->mfirst = mparse(NULL, arg, &sta, 0);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "mold")) {
			if (arg) {
				ptrdiff_t sta;

				if (options)
					options->mold = mparse(NULL, arg, &sta, 0);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "msnew")) {
			if (arg) {
				ptrdiff_t sta;

				if (options)
					options->msnew = mparse(NULL, arg, &sta, 0);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "msold")) {
			if (arg) {
				ptrdiff_t sta;

				if (options)
					options->msold = mparse(NULL, arg, &sta, 0);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "text_color")) {
			if (arg) {
				bg_text = meta_color(arg);
				bg_help = bg_text;
				bg_prompt = bg_text;
				bg_menu = bg_text;
				bg_msg = bg_text;
				bg_stalin = bg_text;
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "help_color")) {
			if (arg) {
				bg_help = meta_color(arg);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "status_color")) {
			if (arg) {
				bg_stalin = meta_color(arg);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "menu_color")) {
			if (arg) {
				bg_menu = meta_color(arg);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "prompt_color")) {
			if (arg) {
				bg_prompt = meta_color(arg);
				ret = 2;
			} else
				ret = 1;
		} else if (!zcmp(s, "msg_color")) {
			if (arg) {
				bg_msg = meta_color(arg);
				ret = 2;
			} else
				ret = 1;
		}
	}

	return ret;
}

/* Option setting user interface (^T command) */

static int doabrt1(W *w, void *obj)
{
	int *xx = (int *)obj;
	joe_free(xx);
	return -1;
}

static int doopt1(W *w, char *s, void *obj, int *notify)
{
	BW *bw;
	int ret = 0;
	int *xx = (int *)obj;
	int x = *xx;
	int v;
	off_t vv;
	WIND_BW(bw, w);

	joe_free(xx);
	switch (glopts[x].type) {
	case 1:
		v = (int)calc(bw, s, 0);
		if (merr) {
			msgnw(bw->parent, merr);
			ret = -1;
		} else if (v >= glopts[x].low && v <= glopts[x].high)
			*(int *)glopts[x].set = v;
		else {
			msgnw(bw->parent, joe_gettext(_("Value out of range")));
			ret = -1;
		}
		break;
	case 2:
		if (s[0])
			*(char **) glopts[x].set = zdup(s);
		break;
	case 6:
		*(char **)((char *)&bw->o+glopts[x].ofst) = zdup(s);
		break;
	case 5:
		v = (int)calc(bw, s, 0);
		if (merr) {
			msgnw(bw->parent, merr);
			ret = -1;
		} else if (v >= glopts[x].low && v <= glopts[x].high)
			*(int *) ((char *) &bw->o + glopts[x].ofst) = v;
		else {
			msgnw(bw->parent, joe_gettext(_("Value out of range")));
			ret = -1;
		}
		break;
	case 14:
		vv = (off_t)calc(bw, s, 0);
		if (merr) {
			msgnw(bw->parent, merr);
			ret = -1;
		} else if (vv >= glopts[x].low && vv <= glopts[x].high)
			*(off_t *) ((char *) &bw->o + glopts[x].ofst) = vv;
		else {
			msgnw(bw->parent, joe_gettext(_("Value out of range")));
			ret = -1;
		}
		break;
	case 7:
		vv = (off_t)(calc(bw, s, 0) - 1.0);
		if (merr) {
			msgnw(bw->parent, merr);
			ret = -1;
		} else if (vv >= glopts[x].low && vv <= glopts[x].high)
			*(off_t *) ((char *) &bw->o + glopts[x].ofst) = vv;
		else {
			msgnw(bw->parent, joe_gettext(_("Value out of range")));
			ret = -1;
		}
		break;
	}
	vsrm(s);
	bw->b->o = bw->o;
	wfit(bw->parent->t);
	updall();
	if (notify)
		*notify = 1;
	return ret;
}

static int dosyntax(W *w, char *s, void *obj, int *notify)
{
	BW *bw;
	int ret = 0;
	struct high_syntax *syn;
	WIND_BW(bw, w);

	syn = load_syntax(s);

	if (syn)
		bw->o.syntax = syn;
	else
		msgnw(bw->parent, joe_gettext(_("Syntax definition file not found")));

	vsrm(s);
	bw->b->o = bw->o;
	updall();
	if (notify)
		*notify = 1;
	return ret;
}

static int docolors(W *w, char *s, void *obj, int *notify)
{
	BW *bw;
	SCHEME *scheme;

	WIND_BW(bw, w);

	scheme = load_scheme(s);
	if (scheme) {
		if (apply_scheme(scheme))
			msgnw(bw->parent, joe_gettext(_("Color scheme failed to apply")));
	} else {
		msgnw(bw->parent, joe_gettext(_("Color scheme file not found")));
	}

	vsrm(s);
	nredraw(maint->t);
	if (notify)
		*notify = 1;

	return 0;
}

char **find_configs(char **ary, const char *extension, const char *datadir, const char *homedir)
{
	char wildcard[32];
	char buf[2048];
	char *oldpwd = pwd();
	char **t = NULL;
	char *p;
	int x, y;

	if (extension) {
		joe_snprintf_1(wildcard, SIZEOF(wildcard), "*.%s", extension);
	} else {
		zcpy(wildcard, "*");
	}

	if (datadir) {
		joe_snprintf_2(buf, SIZEOF(buf), "%s%s", JOEDATA, datadir);

		/* Load first from global (NOTE: Order here does not matter.) */
		if (!chpwd(buf) && (t = rexpnd(wildcard))) {
			for (x = 0; x < aLEN(t); ++x) {
				if (extension) *zrchr(t[x], '.') = 0;
				for (y = 0; y < aLEN(ary); ++y)
					if (!zcmp(t[x], ary[y]))
						break;
				if (y == aLEN(ary))
					ary = vaadd(ary, vsncpy(NULL, 0, sv(t[x])));
			}

			varm(t);
			t = NULL;
		}
	}

	if (homedir) {
		/* Load from home directory. */
		p = getenv("HOME");
		if (p) {
			joe_snprintf_2(buf, SIZEOF(buf), "%s/.joe/%s", p, homedir);

			if (!chpwd(buf) && (t = rexpnd(wildcard))) {
				for (x = 0; x < aLEN(t); ++x) {
					if (extension) *zrchr(t[x],'.') = 0;
					for (y = 0; y < aLEN(ary); ++y)
						if (!zcmp(t[x],ary[y]))
							break;
					if (y == aLEN(ary))
						ary = vaadd(ary, vsncpy(NULL, 0, sv(t[x])));
				}

				varm(t);
				t = NULL;
			}
		}
	}

	/* Load from builtins. */
	if (extension) {
		joe_snprintf_1(wildcard, SIZEOF(wildcard), ".%s", extension);
		t = jgetbuiltins(wildcard);

		for (x = 0; x < aLEN(t); ++x) {
			if (extension) *zrchr(t[x], '.') = 0;
			for (y = 0; y < aLEN(ary); ++y)
				if (!zcmp(t[x], ary[y]))
					break;
			if (y == aLEN(ary)) {
				ary = vaadd(ary, vsncpy(NULL, 0, sv(t[x])));
			}
		}

		varm(t);
		t = NULL;
	}

	varm(t);
	chpwd(oldpwd);

	if (aLEN(ary)) {
		vasort(av(ary));
	}

	return ary;
}

char **syntaxes = NULL; /* Array of available syntaxes */

static int syntaxcmplt(BW *bw, int k)
{
	if (!syntaxes) {
		syntaxes = find_configs(NULL, "jsf", "syntax", "syntax");
	}

	return simple_cmplt(bw,syntaxes);
}

char **colorfiles = NULL; /* Array of available color schemes */

static int colorscmplt(BW *bw, int k)
{
	if (!colorfiles) {
		colorfiles = find_configs(NULL, "jcf", "colors", "colors");
	}

	return simple_cmplt(bw, colorfiles);
}

static int check_for_hex(BW *bw)
{
	W *w;
	if (bw->o.hex)
		return 1;
	for (w = bw->parent->link.next; w != bw->parent; w = w->link.next)
		if ((w->watom == &watomtw || w->watom == &watompw) && ((BW *)w->object)->b == bw->b &&
		    ((BW *)w->object)->o.hex)
		    	return 1;
	return 0;
}

static int doencoding(W *w, char *s, void *obj, int *notify)
{
	BW *bw;
	int ret = 0;
	struct charmap *map;
	WIND_BW(bw, w);

	map = find_charmap(s);

	if (map && map->type && check_for_hex(bw)) {
		msgnw(bw->parent, joe_gettext(_("UTF-8 encoding not allowed with hexadecimal windows")));
		if (notify)
			*notify = 1;
		return -1;
	}

	if (map) {
		bw->o.charmap = map;
		joe_snprintf_1(msgbuf, JOE_MSGBUFSIZE, joe_gettext(_("%s encoding assumed for this file")), map->name);
		msgnw(bw->parent, msgbuf);
	} else
		msgnw(bw->parent, joe_gettext(_("Character set not found")));

	vsrm(s);
	bw->b->o = bw->o;
	bw->cursor->valcol = 0;
	bw->cursor->xcol = piscol(bw->cursor);
	updall();
	if (notify)
		*notify = 1;
	return ret;
}

char **encodings = NULL; /* Array of available encodings */

static int encodingcmplt(BW *bw, int k)
{
	if (!encodings) {
		encodings = get_encodings();
		vasort(av(encodings));
	}
	return simple_cmplt(bw,encodings);
}

static int find_option(char *s)
{
	int y;
	for (y = 0; glopts[y].name; ++y)
		if (!zcmp(glopts[y].name, s))
			return y;
	return -1;
}

static int applyopt(BW *bw, void *optp, int y, int flg)
{
	int oldval, newval;

	oldval = *(int *)optp;
	if (flg == 0) {
		/* Return pressed: toggle */
		newval = !oldval;
	} else if (flg == 1) {
		/* '1' pressed */
		newval = oldval ? oldval : 1; /* Keep oldval if already 'on' */
	} else {
		/* '0' or backspace or something else */
		newval = 0;
	}

	*(int *)optp = newval;
	msgnw(bw->parent, newval ? joe_gettext(glopts[y].yes) : joe_gettext(glopts[y].no));

	return oldval;
}

static int olddoopt(BW *bw, int y, int flg, int *notify)
{
	int *xx, oldval;
	char buf[OPT_BUF_SIZE];

	if (y >= 0) {
		switch (glopts[y].type) {
		case 0:
			applyopt(bw, glopts[y].set, y, flg);
			break;
		case 4:
			oldval = applyopt(bw, (char *) &bw->o + glopts[y].ofst, y, flg);

			/* Propagate readonly bit to B */
			if (glopts[y].ofst == (char *) &fdefault.readonly - (char *) &fdefault)
				bw->b->rdonly = bw->o.readonly;

			/* Kill UTF-8 and CRLF modes if we switch to hex display */
			if (glopts[y].ofst == (char *) &fdefault.hex - (char *) &fdefault) {
				if (bw->o.hex && !oldval) {
					bw->o.hex = 1;
					if (bw->b->o.charmap->type) {
						/* Switch out of UTF-8 mode */
						doencoding(bw->parent, vsncpy(NULL, 0, sc("C")), NULL, NULL);
						bw->o.hex |= HEX_RESTORE_UTF8;
					}

					if (bw->o.crlf) {
						/* Switch out of CRLF mode */
						bw->o.crlf = 0;
						bw->o.hex |= HEX_RESTORE_CRLF;
					}

					if (!bw->o.overtype) {
						bw->o.overtype = 1;
						bw->o.hex |= HEX_RESTORE_INSERT;
					}

					if (bw->o.wordwrap) {
						bw->o.wordwrap = 0;
						bw->o.hex |= HEX_RESTORE_WORDWRAP;
					}

					if (bw->o.autoindent) {
						bw->o.autoindent = 0;
						bw->o.hex |= HEX_RESTORE_AUTOINDENT;
					}

					if (bw->o.ansi) {
						bw->o.ansi = 0;
						bw->o.hex |= HEX_RESTORE_ANSI;
					}

					if (bw->o.picture) {
						bw->o.picture = 0;
						bw->o.hex |= HEX_RESTORE_PICTURE;
					}
					/* Try to put entire hex dump on screen in case where we were
					   scrolled far to the right */
					bw->offset = 0;
				} else if (!bw->o.hex && oldval) {
					if ((oldval & HEX_RESTORE_UTF8) && !zcmp(bw->b->o.charmap->name, "ascii")) {
						/* Switch back into UTF-8 */
						doencoding(bw->parent, vsncpy(NULL, 0, sc("UTF-8")), NULL, NULL);
					}

					if (oldval & HEX_RESTORE_CRLF) {
						/* Turn CRLF back on */
						bw->o.crlf = 1;
					}

					if (oldval & HEX_RESTORE_INSERT) {
						bw->o.overtype = 0;
					}
					if (oldval & HEX_RESTORE_WORDWRAP) {
						bw->o.wordwrap = 1;
					}
					if (oldval & HEX_RESTORE_AUTOINDENT) {
						bw->o.autoindent = 1;
					}
					if (oldval & HEX_RESTORE_ANSI) {
						bw->o.ansi = 1;
					}
					if (oldval & HEX_RESTORE_PICTURE) {
						bw->o.picture = 1;
					}
					/* Update column in case we moved while in hex mode */
					bw->cursor->xcol = piscol(bw->cursor);
				}
			}
			break;
		case 6:
			xx = (int *) joe_malloc(SIZEOF(int));
			*xx = y;
			if(*(char **)((char *)&bw->o+glopts[y].ofst))
				joe_snprintf_1(buf, OPT_BUF_SIZE, glopts[y].yes,*(char **)((char *)&bw->o+glopts[y].ofst));
			else
				joe_snprintf_1(buf, OPT_BUF_SIZE, glopts[y].yes,"");
			if(wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, utypebw, xx, notify, utf8_map, 0))
				return 0;
			else
				return -1;
			/* break; warns on some systems */
		case 1:
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), *(int *)glopts[y].set);
			xx = (int *) joe_malloc(SIZEOF(int));

			*xx = y;
			if (wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, math_cmplt, xx, notify, utf8_map, 0))
				return 0;
			else
				return -1;
		case 2:
			if (*(char **) glopts[y].set)
				joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), *(char **) glopts[y].set);
			else
				joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), "");
			xx = (int *) joe_malloc(SIZEOF(int));

			*xx = y;
			if (wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, utypebw, xx, notify, locale_map, 0))
				return 0;
			else
				return -1;
		case 5:
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), *(int *) ((char *) &bw->o + glopts[y].ofst));
			goto in;
		case 14:
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), (long long)*(off_t *) ((char *) &bw->o + glopts[y].ofst));
			goto in;
		case 7:
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), *(int *) ((char *) &bw->o + glopts[y].ofst) + 1);
		      in:xx = (int *) joe_malloc(SIZEOF(int));

			*xx = y;
			if (wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, math_cmplt, xx, notify, utf8_map, 0))
				return 0;
			else
				return -1;

		case 9:
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), "");
			if (wmkpw(bw->parent, buf, NULL, dosyntax, NULL, NULL, syntaxcmplt, NULL, notify, utf8_map, 0))
				return 0;
			else
				return -1;

		case 13:
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), "");
			if (wmkpw(bw->parent, buf, NULL, doencoding, NULL, NULL, encodingcmplt, NULL, notify, utf8_map, 0))
				return 0;
			else
				return -1;

		case 15:
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), "");
			if (wmkpw(bw->parent, buf, &ftypehist, doftype, "ftype", NULL, ftypecmplt, NULL, notify, utf8_map, 0))
				return 0;
			else
				return -1;

		case 17:
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), "");
			if (wmkpw(bw->parent, buf, NULL, docolors, NULL, NULL, colorscmplt, NULL, notify, utf8_map, 0))
				return 0;
			else
				return -1;
		}
	}
	if (notify)
		*notify = 1;
	bw->b->o = bw->o;
	wfit(bw->parent->t);
	updall();
	return 0;
}

/* Get option in printable format for %Zoption-name% */

const char *get_status(BW *bw, char *s)
{
	static char buf[OPT_BUF_SIZE];
	int y = find_option(s);
	if (y == -1)
		return "???";
	else {
		switch (glopts[y].type) {
			case 0: {
				return *(int *)glopts[y].set ? "ON" : "OFF";
			} case 1: {
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%d", *(int *)glopts[y].set);
				return buf;
			} case 2: {
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%s", *(char **)glopts[y].set ? *(char **)glopts[y].set : "");
				return buf;
			} case 4: {
				return *(int *) ((char *) &bw->o + glopts[y].ofst) ? "ON" : "OFF";
			} case 5: {
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%d", *(int *) ((char *) &bw->o + glopts[y].ofst));
				return buf;
			} case 6: {
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%s", *(char **)((char *) &bw->o + glopts[y].ofst) ? *(char **)((char *) &bw->o + glopts[y].ofst) : "");
				return buf;
			} case 7: {
#ifdef HAVE_LONG_LONG
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%lld", (long long)*(off_t *) ((char *) &bw->o + glopts[y].ofst) + 1);
#else
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%ld", (long)*(off_t *) ((char *) &bw->o + glopts[y].ofst) + 1);
#endif
				return buf;
			} case 9: {
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%s", bw->o.syntax_name ? bw->o.syntax_name : "");
				return buf;
			} case 13: {
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%s", bw->o.map_name ? bw->o.map_name : "");
				return buf;
			} case 14: {
#ifdef HAVE_LONG_LONG
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%lld", (long long)*(off_t *) ((char *) &bw->o + glopts[y].ofst));
#else
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%ld", (long)*(off_t *) ((char *) &bw->o + glopts[y].ofst));
#endif
				return buf;
			} case 15: {
				return bw->o.ftype;
			} case 17: {
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%s", scheme_name ? scheme_name : "");
				return buf;
			} default: {
				return "";
			}
		}
	}
}

/* Simplified mode command */

static char **getoptions(void)
{
	char **s = vaensure(NULL, 20);
	int x;

	for (x = 0; glopts[x].name; ++x)
		s = vaadd(s, vsncpy(NULL, 0, sz(glopts[x].name)));
	vasort(s, aLen(s));
	return s;
}

/* Command line */

char **sopts = NULL;	/* Array of command names */

static int optcmplt(BW *bw, int k)
{
	if (!sopts)
		sopts = getoptions();
	return simple_cmplt(bw,sopts);
}

static int doopt(W *w, char *s, void *object, int *notify)
{
	BW *bw;
	int y = find_option(s);
	WIND_BW(bw, w);
	vsrm(s);
	if (y == -1) {
		msgnw(bw->parent, joe_gettext(_("No such option")));
		if (notify)
			*notify = 1;
		return -1;
	} else {
		int flg = menu_flg;
		menu_flg = 0;
		return olddoopt(bw, y, flg, notify);
	}
}

B *opthist = NULL;

int umode(W *w, int k)
{
	if (wmkpw(w, joe_gettext(_("Option: ")), &opthist, doopt, "opt", NULL, optcmplt, NULL, NULL, locale_map, 0)) {
		return 0;
	} else {
		return -1;
	}
}
