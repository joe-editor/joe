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
	0,		/* lmargin */
	76,		/* rmargin */
	false,		/* overtype */
	false,		/* autoindent */
	false,		/* wordwrap */
	false,		/* nobackup */
	' ',		/* indent char */
	8,		/* tab */
	1,		/* indent step */
	NULL,		/* *context */
	NULL,		/* *lmsg */
	NULL,		/* *rmsg */
	NULL,		/* *smsg */
	NULL,		/* *zmsg */
	NULL,		/* Syntax name */
	NULL,		/* Syntax */
	NULL,		/* Name of character set */
	NULL,		/* Character set */
	NULL,		/* Language */
	false,		/* line numbers */
	false,		/* highlight current line */
	false,		/* read only */
	false,		/* french spacing */
	false,		/* flowed text */
	false,		/* spaces */
#ifdef __MSDOS__
	true,		/* crlf */
#else
	false,		/* crlf */
#endif
	false,		/* Highlight */
	false,		/* Visible whitespace */
	0,		/* Syntax debugging */
	false,		/* Smart home key */
	false,		/* Goto indent first */
	false,		/* Smart backspace key */
	false,		/* Purify indentation */
	false,		/* Picture mode */
	false,		/* highlighter_context */
	false,		/* single_quoted */
	false,		/* no_double_quoted */
	false,		/* c_comment */
	false,		/* cpp_comment */
	false,		/* hash_comment */
	false,		/* vhdl_comment */
	false,		/* semi_comment */
	false,		/* tex_comment */
	false,		/* hex */
	false,		/* hide ansi */
	false,		/* status line context */
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
	0,		/* lmargin */
	76,		/* rmargin */
	false,		/* overtype */
	false,		/* autoindent */
	false,		/* wordwrap */
	false,		/* nobackup */
	' ',		/* indent char */
	8,		/* tab */
	1,		/* indent step */
	"main",		/* *context */
	"\\i%n %m %M",	/* *lmsg */
	" %S Ctrl-K H for help",	/* *rmsg */
	NULL,		/* *smsg */
	NULL,		/* *zmsg */
	NULL,		/* Syntax name */
	NULL,		/* Syntax */
	NULL,		/* Name of character set */
	NULL,		/* Character set */
	NULL,		/* Language */
	false,		/* line numbers */
	false,		/* higlight current line */
	false,		/* read only */
	false,		/* french spacing */
	false,		/* flowed text */
	false,		/* spaces */
#ifdef __MSDOS__
	true,		/* crlf */
#else
	false,		/* crlf */
#endif
	false,		/* Highlight */
	false,		/* Visible whitespace */
	0,		/* Syntax debugging */
	false,		/* Smart home key */
	false,		/* Goto indent first */
	false,		/* Smart backspace key */
	false,		/* Purity indentation */
	false,		/* Picture mode */
	false,		/* highlighter_context */
	false,		/* single_quoted */
	false,		/* no_double_quoted */
	false,		/* c_comment */
	false,		/* cpp_comment */
	false,		/* hash_comment */
	false,		/* vhdl_comment */
	false,		/* semi_comment */
	false,		/* tex_comment */
	false,		/* hex */
	false,		/* hide ansi */
	false,		/* status line context */
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
		b->o.hex_saved = 0;

		/* Hex not allowed with UTF-8 */
		if (b->o.charmap->type) {
			b->o.charmap = find_charmap("c");
			b->o.hex_saved |= HEX_RESTORE_UTF8;
		}

		/* Hex not allowed with CRLF */
		if (b->o.crlf) {
			b->o.crlf = 0;
			b->o.hex_saved |= HEX_RESTORE_CRLF;
		}

		if (!b->o.overtype) {
			b->o.overtype = 1;
			b->o.hex_saved |= HEX_RESTORE_INSERT;
		}

		if (b->o.wordwrap) {
			b->o.wordwrap = 0;
			b->o.hex_saved |= HEX_RESTORE_WORDWRAP;
		}

		if (b->o.autoindent) {
			b->o.autoindent = 0;
			b->o.hex_saved |= HEX_RESTORE_AUTOINDENT;
		}

		if (b->o.ansi) {
			b->o.ansi = 0;
			b->o.hex_saved |= HEX_RESTORE_ANSI;
		}

		if (b->o.picture) {
			b->o.picture = 0;
			b->o.hex_saved |= HEX_RESTORE_PICTURE;
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

enum opt_type {
	GLO_OPT_BOOL,      /* global option flag */
	GLO_OPT_INT,       /* global option int */
	GLO_OPT_STRING,    /* global option string (in locale encoding) */
	GLO_OPT_PATH,	   /* same as GLO_OPT_STRING, but string is a path */
	LOC_OPT_BOOL,      /* local option flag */
	LOC_OPT_INT,       /* local option int */
	LOC_OPT_OFFSET,    /* local option off_t */
	LOC_OPT_STRING,    /* local option string (in utf8) */
	LOC_OPT_RANGE,     /* local option off_t+1, with range checking */
	LOC_OPT_SYNTAX,    /* syntax (options->syntax_name) */
	LOC_OPT_ENCODING,  /* byte encoding (options->map_name) */
	LOC_OPT_FILE_TYPE, /* file type (options->ftype) */
	LOC_OPT_COLORS,    /* color scheme */
};

union opt_storage_p {
	void  *v;
	char  *c;
	bool  *b;
	int   *i;
	off_t *o;
	char **s;
};					/* Address of global option */

struct glopts {
	const char *name;		/* Option name */
	enum opt_type type;
	union opt_storage_p set;	/* Address of global option */
	const char *addr;		/* Local options structure member address */
	const char *yes;		/* Message if option was turned on, or prompt string */
	const char *no;		/* Message if option was turned off */
	const char *menu;		/* Menu string */
	ptrdiff_t ofst;		/* Local options structure member offset */
	int low;		/* Low limit for numeric options */
	int high;		/* High limit for numeric options */
	/* For *_OPT_BOOL, if .yes=NULL or .no=NULL, a message using the menu
	 * string (if non-NULL) & "ON" or "OFF" will be generated at need */
} glopts[] = {
	{"overwrite",           LOC_OPT_BOOL, { NULL }, (char *) &fdefault.overtype, _("Overtype mode"), _("Insert mode"), _("Overtype mode"), 0, 0, 0 },
	{"hex",                 LOC_OPT_BOOL, { NULL }, (char *) &fdefault.hex, _("Hex edit mode"), _("Text edit mode"), _("Hex edit display mode"), 0, 0, 0 },
	{"ansi",                LOC_OPT_BOOL, { NULL }, (char *) &fdefault.ansi, _("Hide ANSI sequences"), _("Reveal ANSI sequences"), _("Hide ANSI mode"), 0, 0, 0 },
	{"title",               LOC_OPT_BOOL, { NULL }, (char *) &fdefault.title, _("Status line context enabled"), _("Status line context disabled"), _("Status line context display mode"), 0, 0, 0 },
	{"autoindent",          LOC_OPT_BOOL, { NULL }, (char *) &fdefault.autoindent, _("Autoindent enabled"), _("Autoindent disabled"), _("Autoindent mode"), 0, 0, 0 },
	{"wordwrap",            LOC_OPT_BOOL, { NULL }, (char *) &fdefault.wordwrap, _("Wordwrap enabled"), _("Wordwrap disabled"), _("Word wrap mode"), 0, 0, 0 },
	{"tab",                 LOC_OPT_OFFSET, { NULL }, (char *) &fdefault.tab, _("Tab width (%lld): "), 0, _("Tab width"), 0, 1, 64 },
	{"lmargin",             LOC_OPT_RANGE, { NULL }, (char *) &fdefault.lmargin, _("Left margin (%d): "), 0, _("Left margin "), 0, 0, 63 },
	{"rmargin",             LOC_OPT_RANGE, { NULL }, (char *) &fdefault.rmargin, _("Right margin (%d): "), 0, _("Right margin "), 0, 7, 255 },
	{"restore",             GLO_OPT_BOOL, { &restore_file_pos }, NULL, _("Restore cursor position when files loaded"), _("Don't restore cursor when files loaded"), _("Restore cursor mode"), 0, 0, 0 },
	{"regex",               GLO_OPT_BOOL, { &std_regex }, NULL, _("Standard regular expression format"), _("JOE regular expression format"), _("Standard or JOE regular expression syntax"), 0, 0, 0 },
	{"square",              GLO_OPT_BOOL, { &square }, NULL, _("Rectangle mode"), _("Text-stream mode"), _("Rectangular region mode"), 0, 0, 0 },
	{"icase",               GLO_OPT_BOOL, { &opt_icase }, NULL, _("Search ignores case by default"), _("Case sensitive search by default"), _("Case insensitive search mode "), 0, 0, 0 },
	{"wrap",                GLO_OPT_BOOL, { &wrap }, NULL, _("Search wraps"), _("Search doesn't wrap"), _("Search wraps mode"), 0, 0, 0 },
	{"menu_explorer",       GLO_OPT_BOOL, { &menu_explorer }, NULL, _("Menu explorer mode"), _("Simple completion mode"), _("Menu explorer mode"), 0, 0, 0 },
	{"menu_above",          GLO_OPT_BOOL, { &menu_above }, NULL, _("Menu above prompt"), _("Menu below prompt"), _("Menu above/below mode"), 0, 0, 0 },
	{"notagsmenu",          GLO_OPT_BOOL, { &notagsmenu }, NULL, _("Tags menu disabled"), _("Tags menu enabled"), _("Tags menu mode"), 0, 0, 0 },
	{"search_prompting",    GLO_OPT_BOOL, { &pico }, NULL, _("Search prompting on"), _("Search prompting off"), _("Search prompting mode"), 0, 0, 0 },
	{"menu_jump",           GLO_OPT_BOOL, { &menu_jump }, NULL, _("Jump into menu is on"), _("Jump into menu is off"), _("Jump into menu mode"), 0, 0, 0 },
	{"autoswap",            GLO_OPT_BOOL, { &autoswap }, NULL, _("Autoswap ^KB and ^KK"), _("Autoswap off "), _("Autoswap mode "), 0, 0, 0 },
	{"indentc",             LOC_OPT_INT, { NULL }, (char *) &fdefault.indentc, _("Indent char %d (SPACE=32, TAB=9, %{abort} to abort): "), 0, _("Indent char "), 0, 0, 255 },
	{"istep",               LOC_OPT_OFFSET, { NULL }, (char *) &fdefault.istep, _("Indent step %lld (%{abort} to abort): "), 0, _("Indent step "), 0, 1, 64 },
	{"french",              LOC_OPT_BOOL, { NULL }, (char *) &fdefault.french, _("One space after periods for paragraph reformat"), _("Two spaces after periods for paragraph reformat"), _("French spacing mode"), 0, 0, 0 },
	{"flowed",              LOC_OPT_BOOL, { NULL }, (char *) &fdefault.flowed, _("One space after paragraph line"), _("No spaces after paragraph lines"), _("Flowed text mode"), 0, 0, 0 },
	{"highlight",           LOC_OPT_BOOL, { NULL }, (char *) &fdefault.highlight, _("Highlighting enabled"), _("Highlighting disabled"), _("Syntax highlighting mode"), 0, 0, 0 },
	{"spaces",              LOC_OPT_BOOL, { NULL }, (char *) &fdefault.spaces, _("Inserting spaces when tab key is hit"), _("Inserting tabs when tab key is hit"), _("No tabs mode"), 0, 0, 0 },
	{"mid",                 GLO_OPT_BOOL, { &opt_mid }, NULL, _("Cursor will be recentered on scrolls"), _("Cursor will not be recentered on scroll"), _("Center on scroll mode"), 0, 0, 0 },
	{"left",                GLO_OPT_INT, { &opt_left }, NULL, _("Columns to scroll left or -1 for 1/2 window (%d): "), 0, _("Left scroll amount"), 0, -128, 127 },
	{"right",               GLO_OPT_INT, { &opt_right }, NULL, _("Columns to scroll right or -1 for 1/2 window (%d): "), 0, _("Right scroll amount"), 0, -128, 127 },
	{"guess_crlf",          GLO_OPT_BOOL, { &guesscrlf }, NULL, _("Automatically detect MS-DOS files"), _("Do not automatically detect MS-DOS files"), _("Auto detect CR-LF mode"), 0, 0, 0 },
	{"guess_indent",        GLO_OPT_BOOL, { &guessindent }, NULL, _("Automatically detect indentation"), _("Do not automatically detect indentation"), _("Guess indent mode"), 0, 0, 0 },
	{"guess_non_utf8",      GLO_OPT_BOOL, { &guess_non_utf8 }, NULL, _("Automatically detect non-UTF-8 in UTF-8 locale"), _("Do not automatically detect non-UTF-8"), _("Guess non-UTF-8 mode"), 0, 0, 0 },
	{"guess_utf8",          GLO_OPT_BOOL, { &guess_utf8 }, NULL, _("Automatically detect UTF-8 in non-UTF-8 locale"), _("Do not automatically detect UTF-8"), _("Guess UTF-8 mode"), 0, 0, 0 },
	{"guess_utf16",         GLO_OPT_BOOL, { &guess_utf16 }, NULL, _("Automatically detect UTF-16"), _("Do not automatically detect UTF-16"), _("Guess UTF-16 mode"), 0, 0, 0 },
	{"transpose",           GLO_OPT_BOOL, { &transpose }, NULL, _("Menu is transposed"), _("Menus are not transposed"), _("Transpose menus mode"), 0, 0, 0 },
	{"crlf",                LOC_OPT_BOOL, { NULL }, (char *) &fdefault.crlf, _("CR-LF is line terminator"), _("LF is line terminator"), _("CR-LF (MS-DOS) mode"), 0, 0, 0 },
	{"linums",              LOC_OPT_BOOL, { NULL }, (char *) &fdefault.linums, _("Line numbers enabled"), _("Line numbers disabled"), _("Line numbers mode"), 0, 0, 0 },
	{"hiline",              LOC_OPT_BOOL, { NULL }, (char *) &fdefault.hiline, _("Highlighting cursor line"), _("Not highlighting cursor line"), _("Highlight cursor line"), 0, 0, 0 },
	{"marking",             GLO_OPT_BOOL, { &marking }, NULL, _("Anchored block marking on"), _("Anchored block marking off"), _("Region marking mode"), 0, 0, 0 },
	{"asis",                GLO_OPT_BOOL, { &dspasis }, NULL, _("Characters above 127 shown as-is"), _("Characters above 127 shown in inverse"), _("Display meta chars as-is mode"), 0, 0, 0 },
	{"force",               GLO_OPT_BOOL, { &force }, NULL, _("Last line forced to have NL when file saved"), _("Last line not forced to have NL"), _("Force last NL mode"), 0, 0, 0 },
	{"joe_state",           GLO_OPT_BOOL, { &joe_state }, NULL, _("~/.joe_state file will be updated"), _("~/.joe_state file will not be updated"), _("Joe_state file mode"), 0, 0, 0 },
	{"nobackup",            LOC_OPT_BOOL, { NULL }, (char *) &fdefault.nobackup, _("Nobackup enabled"), _("Nobackup disabled"), _("No backup mode"), 0, 0, 0 },
	{"nobackups",           GLO_OPT_BOOL, { &nobackups }, NULL, _("Backup files will not be made"), _("Backup files will be made"), _("Disable backups mode"), 0, 0, 0 },
	{"nodeadjoe",           GLO_OPT_BOOL, { &nodeadjoe }, NULL, _("DEADJOE files will not be made"), _("DEADJOE files will be made"), _("Disable DEADJOE mode"), 0, 0, 0 },
	{"nolocks",             GLO_OPT_BOOL, { &nolocks }, NULL, _("Files will not be locked"), _("Files will be locked"), _("Disable locks mode"), 0, 0, 0 },
	{"nomodcheck",          GLO_OPT_BOOL, { &nomodcheck }, NULL, _("No file modification time check"), _("File modification time checking enabled"), _("Disable mtime check mode"), 0, 0, 0 },
	{"nocurdir",            GLO_OPT_BOOL, { &nocurdir }, NULL, _("No current dir"), _("Current dir enabled"), _("Disable current dir "), 0, 0, 0 },
	{"break_hardlinks",     GLO_OPT_BOOL, { &break_links }, NULL, _("Hardlinks will be broken"), _("Hardlinks not broken"), _("Break hard links "), 0, 0, 0 },
	{"break_links",         GLO_OPT_BOOL, { &break_symlinks }, NULL, _("Links will be broken"), _("Links not broken"), _("Break links "), 0, 0, 0 },
	{"lightoff",            GLO_OPT_BOOL, { &lightoff }, NULL, _("Highlighting turned off after block operations"), _("Highlighting not turned off after block operations"), _("Auto unmark "), 0, 0, 0 },
	{"exask",               GLO_OPT_BOOL, { &exask }, NULL, _("Prompt for filename in save & exit command"), _("Don't prompt for filename in save & exit command"), _("Exit ask "), 0, 0, 0 },
	{"beep",                GLO_OPT_BOOL, { &joe_beep }, NULL, _("Warning bell enabled"), _("Warning bell disabled"), _("Beeps "), 0, 0, 0 },
	{"nosta",               GLO_OPT_BOOL, { &staen }, NULL, _("Top-most status line disabled"), _("Top-most status line enabled"), _("Disable status line "), 0, 0, 0 },
	{"keepup",              GLO_OPT_BOOL, { &keepup }, NULL, _("Status line updated constantly"), _("Status line updated once/sec"), _("Fast status line "), 0, 0, 0 },
	{"pg",                  GLO_OPT_INT, { &pgamnt }, NULL, _("Lines to keep for PgUp/PgDn or -1 for 1/2 window (%d): "), 0, _("No. PgUp/PgDn lines "), 0, -1, 64 },
	{"undo_keep",           GLO_OPT_INT, { &undo_keep }, NULL, _("No. undo records to keep, or (0 for infinite): "), 0, _("No. undo records "), 0, -1, 64 },
	{"csmode",              GLO_OPT_BOOL, { &csmode }, NULL, _("Start search after a search repeats previous search"), _("Start search always starts a new search"), _("Continued search "), 0, 0, 0 },
	{"rdonly",              LOC_OPT_BOOL, { NULL }, (char *) &fdefault.readonly, _("Read only"), _("Full editing"), _("Read only "), 0, 0, 0 },
	{"smarthome",           LOC_OPT_BOOL, { NULL }, (char *) &fdefault.smarthome, _("Smart home key enabled"), _("Smart home key disabled"), _("Smart home key "), 0, 0, 0 },
	{"indentfirst",         LOC_OPT_BOOL, { NULL }, (char *) &fdefault.indentfirst, _("Smart home goes to indentation first"), _("Smart home goes home first"), _("To indent first "), 0, 0, 0 },
	{"smartbacks",          LOC_OPT_BOOL, { NULL }, (char *) &fdefault.smartbacks, _("Smart backspace key enabled"), _("Smart backspace key disabled"), _("Smart backspace "), 0, 0, 0 },
	{"purify",              LOC_OPT_BOOL, { NULL }, (char *) &fdefault.purify, _("Indentation clean up enabled"), _("Indentation clean up disabled"), _("Clean up indents "), 0, 0, 0 },
	{"picture",             LOC_OPT_BOOL, { NULL }, (char *) &fdefault.picture, _("Picture drawing mode enabled"), _("Picture drawing mode disabled"), _("Picture mode "), 0, 0, 0 },
	{"backpath",            GLO_OPT_PATH, { &backpath }, NULL, _("Backup files stored in (%s): "), 0, _("Path to backup files "), 0, 0, 0 },
	{"backup_file_suffix",  GLO_OPT_STRING, { &backup_file_suffix }, NULL, _("Backup file suffix (%s): "), 0, _("Backup file suffix "), 0, 0, 0 },
	{"syntax_debug",	LOC_OPT_INT, { NULL }, (char *) &fdefault.syntax_debug, _("Syntax debug info %d (0=off, 1=state, 2=recolor, 3=both)"), NULL, _("Syntax debug mode"), 0, 0, 3 },
	{"syntax",              LOC_OPT_SYNTAX, { NULL }, NULL, _("Select syntax (%{abort} to abort): "), 0, _("Syntax"), 0, 0, 0 },
	{"colors",              LOC_OPT_COLORS, { NULL }, NULL, _("Select color scheme (%{abort} to abort): "), 0, _("Scheme "), 0, 0, 0 },
	{"encoding",            LOC_OPT_ENCODING, { NULL }, NULL, _("Select file character set (%{abort} to abort): "), 0, _("Encoding "), 0, 0, 0 },
	{"type",                LOC_OPT_FILE_TYPE, { NULL }, NULL, _("Select file type (%{abort} to abort): "), 0, _("File type "), 0, 0, 0 },
	{"highlighter_context", LOC_OPT_BOOL, { NULL }, (char *) &fdefault.highlighter_context, _("Highlighter context enabled"), _("Highlighter context disabled"), _("^G uses highlighter context "), 0, 0, 0 },
	{"single_quoted",       LOC_OPT_BOOL, { NULL }, (char *) &fdefault.single_quoted, _("Single quoting enabled"), _("Single quoting disabled"), _("^G ignores '... ' "), 0, 0, 0 },
	{"no_double_quoted",    LOC_OPT_BOOL, { NULL }, (char *) &fdefault.no_double_quoted, _("Double quoting disabled"), _("Double quoting enabled"), _("^G ignores \"... \" "), 0, 0, 0 },
	{"c_comment",           LOC_OPT_BOOL, { NULL }, (char *) &fdefault.c_comment, _("/* comments enabled"), _("/* comments disabled"), _("^G ignores /*...*/ "), 0, 0, 0 },
	{"cpp_comment",         LOC_OPT_BOOL, { NULL }, (char *) &fdefault.cpp_comment, _("// comments enabled"), _("// comments disabled"), _("^G ignores //... "), 0, 0, 0 },
	{"pound_comment",       LOC_OPT_BOOL, { NULL }, (char *) &fdefault.hash_comment, _("# comments enabled"), _("# comments disabled"), _("^G ignores #... "), 0, 0, 0 },
	{"hash_comment",        LOC_OPT_BOOL, { NULL }, (char *) &fdefault.hash_comment, _("# comments enabled"), _("# comments disabled"), _("^G ignores #... "), 0, 0, 0 },
	{"vhdl_comment",        LOC_OPT_BOOL, { NULL }, (char *) &fdefault.vhdl_comment, _("-- comments enabled"), _("-- comments disabled"), _("^G ignores --... "), 0, 0, 0 },
	{"semi_comment",        LOC_OPT_BOOL, { NULL }, (char *) &fdefault.semi_comment, _("; comments enabled"), _("; comments disabled"), _("^G ignores ;... "), 0, 0, 0 },
	{"tex_comment",         LOC_OPT_BOOL, { NULL }, (char *) &fdefault.tex_comment, _("% comments enabled"), _("% comments disabled"), _("^G ignores %... "), 0, 0, 0 },
	{"text_delimiters",     LOC_OPT_STRING, { NULL }, (char *) &fdefault.text_delimiters, _("Text delimiters (%s): "), 0, _("Text delimiters "), 0, 0, 0 },
	{"language",            LOC_OPT_STRING, { NULL }, (char *) &fdefault.language, _("Language (%s): "), 0, _("Language "), 0, 0, 0 },
	{"cpara",               LOC_OPT_STRING, { NULL }, (char *) &fdefault.cpara, _("Characters which can indent paragraphs (%s): "), 0, _("Paragraph indent chars "), 0, 0, 0 },
	{"cnotpara",            LOC_OPT_STRING, { NULL }, (char *) &fdefault.cnotpara, _("Characters which begin non-paragraph lines (%s): "), 0, _("Non-paragraph chars "), 0, 0, 0 },
	{"floatmouse",          GLO_OPT_BOOL, { &floatmouse }, 0, _("Clicking can move the cursor past end of line"), _("Clicking past end of line moves cursor to the end"), _("Click past end "), 0, 0, 0 },
	{"rtbutton",            GLO_OPT_BOOL, { &rtbutton }, 0, _("Mouse action is done with the right button"), _("Mouse action is done with the left button"), _("Right button "), 0, 0, 0 },
	{"nonotice",            GLO_OPT_BOOL, { &nonotice }, NULL, 0, 0, _("Suppress startup notice"), 0, 0, 0 },
	{"noexmsg",             GLO_OPT_BOOL, { &noexmsg }, NULL, 0, 0, _("Suppress exit message"), 0, 0, 0 },
	{"help_is_utf8",        GLO_OPT_BOOL, { &help_is_utf8 }, NULL, 0, 0, _("Help is UTF-8"), 0, 0, 0 },
	{"noxon",               GLO_OPT_BOOL, { &noxon }, NULL, 0, 0, _("Disable XON/XOFF"), 0, 0, 0 },
	{"orphan",              GLO_OPT_BOOL, { &orphan }, NULL, 0, 0, _("Orphan extra files"), 0, 0, 0 },
	{"helpon",              GLO_OPT_BOOL, { &helpon }, NULL, 0, 0, _("Start editor with help displayed"), 0, 0, 0 },
	{"dopadding",           GLO_OPT_BOOL, { &dopadding }, NULL, 0, 0, _("Emit padding NULs"), 0, 0, 0 },
	{"lines",               GLO_OPT_INT, { &env_lines }, NULL, 0, 0, _("No. screen lines (if no window size ioctl)"), 0, 2, 1024 },
	{"baud",                GLO_OPT_INT, { &Baud }, NULL, 0, 0, _("Baud rate"), 0, 50, 32767 },
	{"columns",             GLO_OPT_INT, { &env_columns }, NULL, 0, 0, _("No. screen columns (if no window size ioctl)"), 0, 2, 1024 },
	{"skiptop",             GLO_OPT_INT, { &skiptop }, NULL, 0, 0, _("No. screen lines to skip"), 0, 0, 64 },
	{"notite",              GLO_OPT_BOOL, { &notite }, NULL, 0, 0, _("Suppress tty init sequence"), 0, 0, 0 },
	{"brpaste",             GLO_OPT_BOOL, { &brpaste }, NULL, 0, 0, _("Bracketed paste mode"), 0, 0, 0 },
	{"pastehack",           GLO_OPT_BOOL, { &pastehack }, NULL, 0, 0, _("Paste quoting hack"), 0, 0, 0 },
	{"nolinefeeds",         GLO_OPT_BOOL, { &nolinefeeds }, NULL, 0, 0, _("Suppress history preserving linefeeds"), 0, 0, 0 },
	{"mouse",               GLO_OPT_BOOL, { &xmouse }, NULL, 0, 0, _("Enable mouse"), 0, 0, 0 },
	{"usetabs",             GLO_OPT_BOOL, { &opt_usetabs }, NULL, 0, 0, _("Screen update uses tabs"), 0, 0, 0 },
	{"assume_color",        GLO_OPT_BOOL, { &assume_color }, NULL, 0, 0, _("Assume terminal supports color"), 0, 0, 0 },
	{"assume_256color",     GLO_OPT_BOOL, { &assume_256color }, NULL, 0, 0, _("Assume terminal supports 256 colors"), 0, 0, 0 },
	{"joexterm",            GLO_OPT_BOOL, { &joexterm }, NULL, 0, 0, _("Assume xterm patched for JOE"), 0, 0, 0 },
	{"visiblews",           LOC_OPT_BOOL, { NULL }, (char *) &fdefault.visiblews, _("Visible whitespace enabled"), _("Visible whitespace disabled"), _("Visible whitespace"), 0, 0, 0 },
	{ NULL,                 GLO_OPT_BOOL, { NULL }, NULL, NULL, NULL, NULL, 0, 0, 0 }
};

/* Print command line help */

void cmd_help(int type)
{
	int x;

	for (x = 0; glopts[x].name; ++x) {
		char buf[80];
		buf[0] = 0;
		if (type == 0) {
			int y = 0;
			switch (glopts[x].type) {
				case LOC_OPT_BOOL:
				{
					joe_snprintf_1(buf, SIZEOF(buf), "-[-]%s", glopts[x].name);
					y = 1;
					break;
				}
				case LOC_OPT_INT:
				case LOC_OPT_OFFSET:
				case LOC_OPT_RANGE:
				{
					joe_snprintf_1(buf, SIZEOF(buf), "-%s nnn", glopts[x].name);
					y = 1;
					break;
				}
				case LOC_OPT_STRING:
				case LOC_OPT_SYNTAX:
				case LOC_OPT_ENCODING:
				case LOC_OPT_FILE_TYPE:
				case LOC_OPT_COLORS:
				{
					joe_snprintf_1(buf, SIZEOF(buf), "-%s sss", glopts[x].name);
					y = 1;
					break;
				}
				case GLO_OPT_BOOL:
				case GLO_OPT_INT:
				case GLO_OPT_STRING:
				case GLO_OPT_PATH:
				{
					break;
				}

			}
			if (y) {
				if (glopts[x].menu)
					printf("    %-23s %s\n", buf, glopts[x].menu);
				else
					printf("    %-23s\n", buf);
			}
		} else if (type == 1) {
			int y = 0;
			switch (glopts[x].type) {
				case GLO_OPT_BOOL:
				{
					joe_snprintf_1(buf, SIZEOF(buf), "-[-]%s", glopts[x].name);
					y = 1;
					break;
				}
				case GLO_OPT_INT:
				{
					joe_snprintf_1(buf, SIZEOF(buf), "-%s nnn", glopts[x].name);
					y = 1;
					break;
				}
				case GLO_OPT_STRING:
				case GLO_OPT_PATH:
				{
					joe_snprintf_1(buf, SIZEOF(buf), "-%s sss", glopts[x].name);
					y = 1;
					break;
				}
				case LOC_OPT_BOOL:
				case LOC_OPT_INT:
				case LOC_OPT_OFFSET:
				case LOC_OPT_RANGE:
				case LOC_OPT_STRING:
				case LOC_OPT_SYNTAX:
				case LOC_OPT_ENCODING:
				case LOC_OPT_FILE_TYPE:
				case LOC_OPT_COLORS:
				{
					break;
				}

			}
			if (y) {
				if (glopts[x].menu)
					printf("    %-23s %s\n", buf, glopts[x].menu);
				else
					printf("    %-23s\n", buf);
			}
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
		case LOC_OPT_BOOL:
		case LOC_OPT_INT:
		case LOC_OPT_STRING:
		case LOC_OPT_RANGE:
		case LOC_OPT_OFFSET:
			glopts[x].ofst = glopts[x].addr - (char *) &fdefault;
		/* these are unhandled */
		case GLO_OPT_BOOL:
		case GLO_OPT_INT:
		case GLO_OPT_STRING:
		case GLO_OPT_PATH:
		case LOC_OPT_SYNTAX:
		case LOC_OPT_ENCODING:
		case LOC_OPT_FILE_TYPE:
		case LOC_OPT_COLORS:
		default:;
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
B *encodinghist = NULL;
B *colorhist = NULL;
B *syntaxhist = NULL;

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

#define OPTPTR(opts, off, type) ((type*) ((char *)(opts) + (off)))

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
		case GLO_OPT_BOOL: /* Global variable flag option */
			if (set)
				*opt->set.b = st;
			ret = 1;
			break;
		case GLO_OPT_INT: /* Global variable integer option */
			if (set && arg) {
				val = ztoi(arg);
				if (val >= opt->low && val <= opt->high)
					*opt->set.i = val;
			}
			ret = arg ? 2 : 1;
			break;
		case GLO_OPT_STRING: /* Global variable string option */
		case GLO_OPT_PATH:
			if (set) {
				*opt->set.s = arg ? zdup(arg) : NULL;
			}
			ret = arg ? 2 : 1;
			break;
		case LOC_OPT_BOOL: /* Local option flag */
			if (options)
				*OPTPTR(options, opt->ofst, bool) = st;
			ret = 1;
			break;
		case LOC_OPT_INT: /* Local option integer */
			if (arg) {
				if (options) {
					val = ztoi(arg);
					if (val >= opt->low && val <= opt->high)
						*OPTPTR(options, opt->ofst, int) = val;
				}
			}
			ret = arg ? 2 : 1;
			break;
		case LOC_OPT_OFFSET: /* Local option off_t */
			if (arg) {
				if (options) {
					off_t zz = ztoo(arg);
					if (zz >= opt->low && zz <= opt->high)
						*OPTPTR(options, opt->ofst, off_t) = zz;
				}
			}
			ret = arg ? 2 : 1;
			break;
		case LOC_OPT_FILE_TYPE: /* Set file type */
			if (arg && options) {
				OPTIONS *o = find_ftype(arg);
				if (o) {
					*options = *o;
				}
			}
			ret = arg ? 2 : 1;
			break;
		case LOC_OPT_STRING: /* Local string option */
			if (options) {
				*OPTPTR(options, opt->ofst, char *) = arg ? zdup(arg) : NULL;
			}
			ret = arg ? 2 : 1;
			break;
		case LOC_OPT_RANGE: /* Local option numeric + 1, with range checking */
			if (arg) {
				off_t zz = ztoo(arg);
				if (zz >= opt->low && zz <= opt->high) {
					--zz;
					if (options)
						*OPTPTR(options, opt->ofst, off_t) = zz;
				}
			}
			ret = arg ? 2 : 1;
			break;

		case LOC_OPT_SYNTAX: /* Set syntax */
			if (arg && options)
				options->syntax_name = zdup(arg);
			/* this was causing all syntax files to be loaded...
			if (arg && options)
				options->syntax = load_syntax(arg); */
			ret = arg ? 2 : 1;
			break;

		case LOC_OPT_ENCODING: /* Set byte mode encoding */
			if (arg && options)
				options->map_name = zdup(arg);
			ret = arg ? 2 : 1;
			break;

		case LOC_OPT_COLORS: /* Set color scheme */
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
	case GLO_OPT_INT:
		v = (int)calc(bw, s, 0);
		if (merr) {
			msgnw(bw->parent, merr);
			ret = -1;
		} else if (v >= glopts[x].low && v <= glopts[x].high)
			*glopts[x].set.i = v;
		else {
			msgnw(bw->parent, joe_gettext(_("Value out of range")));
			ret = -1;
		}
		break;
	case GLO_OPT_STRING:
	case GLO_OPT_PATH:
		if (s[0])
			*glopts[x].set.s = zdup(s);
		break;
	case LOC_OPT_STRING:
		*OPTPTR(&bw->o, glopts[x].ofst, char *) = zdup(s);
		break;
	case LOC_OPT_INT:
		v = (int)calc(bw, s, 0);
		if (merr) {
			msgnw(bw->parent, merr);
			ret = -1;
		} else if (v >= glopts[x].low && v <= glopts[x].high)
			*OPTPTR(&bw->o, glopts[x].ofst, int) = v;
		else {
			msgnw(bw->parent, joe_gettext(_("Value out of range")));
			ret = -1;
		}
		break;
	case LOC_OPT_OFFSET:
		vv = (off_t)calc(bw, s, 0);
		if (merr) {
			msgnw(bw->parent, merr);
			ret = -1;
		} else if (vv >= glopts[x].low && vv <= glopts[x].high)
			*OPTPTR(&bw->o, glopts[x].ofst, off_t) = vv;
		else {
			msgnw(bw->parent, joe_gettext(_("Value out of range")));
			ret = -1;
		}
		break;
	case LOC_OPT_RANGE:
		vv = (off_t)(calc(bw, s, 0) - 1.0);
		if (merr) {
			msgnw(bw->parent, merr);
			ret = -1;
		} else if (vv >= glopts[x].low && vv <= glopts[x].high)
			*OPTPTR(&bw->o, glopts[x].ofst, off_t) = vv;
		else {
			msgnw(bw->parent, joe_gettext(_("Value out of range")));
			ret = -1;
		}
		break;
	/* not all handled here */
	case GLO_OPT_BOOL:
	case LOC_OPT_BOOL:
	case LOC_OPT_ENCODING:
	case LOC_OPT_FILE_TYPE:
	case LOC_OPT_COLORS:
	case LOC_OPT_SYNTAX:
	default:;
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

/* Add any strings from src that do not already appear in dst.
   If trim is set, delete filename extension from src strings.
   Do not add ".." to list */

static char **merge_options(char **dst, char **src, int trim)
{
	if (src) {
		int x;
		for (x = 0; x < aLEN(src); ++x) {
			int y;
			if (trim) {
				char *e = zrchr(src[x], '.');
				if (e)
					*e = 0;
			}
			if (zcmp(src[x], "..")) {
				for (y = 0; y < aLEN(dst); ++y)
					if (!zcmp(src[x], dst[y]))
						break;
				if (y == aLEN(dst))
					dst = vaadd(dst, vsncpy(NULL, 0, sv(src[x])));
			}
		}
	}
	return dst;
}

char **find_configs(char **ary, const char *prefix, const char *extension)
{
	char wildcard[32];
	char *oldpwd = pwd();
	const char *home = getenv("HOME");
	const char *xdg = xdg_config_dir();
	char *path;
	char **t;

	if (extension) {
		joe_snprintf_1(wildcard, SIZEOF(wildcard), "*%s", extension);
	} else {
		zcpy(wildcard, "*");
	}

	/* Look in /usr/share/joe/<prefix> */

	path = vsncpy(NULL, 0, sc(JOEDATA));
	path = vsncpy(sv(path), sz(prefix));

	if (!chpwd(path) && (t = rexpnd(wildcard))) {
		ary = merge_options(ary, t, !!extension);
		varm(t);
	}
	vsrm(path);

	/* Look in $HOME/.joe/<prefix> */

	if (home) {
		path = vsncpy(NULL, 0, sz(home));
		path = vsncpy(sv(path), sc("/.joe/"));
		path = vsncpy(sv(path), sz(prefix));

		if (!chpwd(path) && (t = rexpnd(wildcard))) {
			ary = merge_options(ary, t, !!extension);
			varm(t);
		}
		vsrm(path);
	}

	/* Look in $HOME/.config/joe/<prefix> */

	if (xdg) {
		path = vsncpy(NULL, 0, sv(xdg));
		path = vsncpy(sv(path), sz(prefix));

		if (!chpwd(path) && (t = rexpnd(wildcard))) {
			ary = merge_options(ary, t, !!extension);
			varm(t);
		}
		vsrm(path);
	}

	/* Look through built-in files */

	if (extension) {
		t = jgetbuiltins(extension);

		if (t) {
			ary = merge_options(ary, t, !!extension);
		}
		varm(t);
	}

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
		syntaxes = find_configs(NULL, "syntax", ".jsf");
	}

	return simple_file_cmplt(bw,syntaxes);
}

char **colorfiles = NULL; /* Array of available color schemes */

static int colorscmplt(BW *bw, int k)
{
	if (!colorfiles) {
		colorfiles = find_configs(NULL, "colors", ".jcf");
	}

	return simple_file_cmplt(bw, colorfiles);
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
	return simple_file_cmplt(bw,encodings);
}

static int find_option(char *s)
{
	int y;
	for (y = 0; glopts[y].name; ++y)
		if (!zcmp(glopts[y].name, s))
			return y;
	return -1;
}

static int applyopt(BW *bw, bool *optp, int y, int flg)
{
	bool oldval, newval;
	const char *msg;

	oldval = *optp;
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

	*optp = newval;

	msg = newval ? glopts[y].yes : glopts[y].no;
	if (msg)
		msgnw(bw->parent, joe_gettext(msg));
	else {
		msg = newval ? "ON" : "OFF";
		if (glopts[y].menu) {
			joe_snprintf_2(msgbuf, JOE_MSGBUFSIZE, "%s: %s", joe_gettext(glopts[y].menu), msg);
			msgnw(bw->parent, msgbuf);
		} else
			msgnw(bw->parent, msg);
	}
	return oldval;
}

static int olddoopt(BW *bw, int y, int flg, int *notify)
{
	int *xx, oldval;
	char buf[OPT_BUF_SIZE];

	if (y >= 0) {
		switch (glopts[y].type) {
		case GLO_OPT_BOOL:
			applyopt(bw, glopts[y].set.b, y, flg);
			break;
		case LOC_OPT_BOOL:
			oldval = applyopt(bw, OPTPTR(&bw->o, glopts[y].ofst, bool), y, flg);

			/* Propagate readonly bit to B */
			if (glopts[y].ofst == offsetof(OPTIONS, readonly))
				bw->b->rdonly = bw->o.readonly;

			/* Kill UTF-8 and CRLF modes if we switch to hex display */
			if (glopts[y].ofst == offsetof(OPTIONS, hex)) {
				oldval = bw->o.hex_saved;

				if (bw->o.hex && !oldval) {
					bw->o.hex = 1;
					bw->o.hex_saved = 0;
					if (bw->b->o.charmap->type) {
						/* Switch out of UTF-8 mode */
						doencoding(bw->parent, vsncpy(NULL, 0, sc("C")), NULL, NULL);
						bw->o.hex_saved |= HEX_RESTORE_UTF8;
					}

					if (bw->o.crlf) {
						/* Switch out of CRLF mode */
						bw->o.crlf = 0;
						bw->o.hex_saved |= HEX_RESTORE_CRLF;
					}

					if (!bw->o.overtype) {
						bw->o.overtype = 1;
						bw->o.hex_saved |= HEX_RESTORE_INSERT;
					}

					if (bw->o.wordwrap) {
						bw->o.wordwrap = 0;
						bw->o.hex_saved |= HEX_RESTORE_WORDWRAP;
					}

					if (bw->o.autoindent) {
						bw->o.autoindent = 0;
						bw->o.hex_saved |= HEX_RESTORE_AUTOINDENT;
					}

					if (bw->o.ansi) {
						bw->o.ansi = 0;
						bw->o.hex_saved |= HEX_RESTORE_ANSI;
					}

					if (bw->o.picture) {
						bw->o.picture = 0;
						bw->o.hex_saved |= HEX_RESTORE_PICTURE;
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
		case LOC_OPT_STRING:
			xx = (int *) joe_malloc(SIZEOF(int));
			*xx = y;
			if(*OPTPTR(&bw->o, glopts[y].ofst, char *))
				joe_snprintf_1(buf, OPT_BUF_SIZE, glopts[y].yes, *OPTPTR(&bw->o, glopts[y].ofst, char *));
			else
				joe_snprintf_1(buf, OPT_BUF_SIZE, glopts[y].yes,"");
			if(wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, utypebw, xx, notify, utf8_map, 0))
				return 0;
			else
				return -1;
			/* break; warns on some systems */
		case GLO_OPT_INT:
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), *glopts[y].set.i);
			xx = (int *) joe_malloc(SIZEOF(int));

			*xx = y;
			if (wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, math_cmplt, xx, notify, utf8_map, 0))
				return 0;
			else
				return -1;
		case GLO_OPT_STRING:
			if (*glopts[y].set.s)
				joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), *glopts[y].set.s);
			else
				joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), "");
			xx = (int *) joe_malloc(SIZEOF(int));

			*xx = y;
			if (wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, utypebw, xx, notify, locale_map, 0))
				return 0;
			else
				return -1;
		case GLO_OPT_PATH:
			if (*glopts[y].set.s)
				joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), *glopts[y].set.s);
			else
				joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), "");
			xx = (int *) joe_malloc(SIZEOF(int));

			*xx = y;
			if (wmkpw(bw->parent, buf, &filehist, doopt1, NULL, doabrt1, cmplt_file, xx, notify, locale_map, 0))
				return 0;
			else
				return -1;
		case LOC_OPT_INT:
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), *OPTPTR(&bw->o, glopts[y].ofst, int));
			goto in;
		case LOC_OPT_OFFSET:
#ifdef HAVE_LONG_LONG
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), (long long)*OPTPTR(&bw->o, glopts[y].ofst, off_t));
#else
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), (long)*OPTPTR(&bw->o, glopts[y].ofst, off_t));
#endif
			goto in;
		case LOC_OPT_RANGE:
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), *OPTPTR(&bw->o, glopts[y].ofst, int) + 1);
		      in:xx = (int *) joe_malloc(SIZEOF(int));

			*xx = y;
			if (wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, math_cmplt, xx, notify, utf8_map, 0))
				return 0;
			else
				return -1;

		case LOC_OPT_SYNTAX:
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), "");
			if (wmkpw(bw->parent, buf, &syntaxhist, dosyntax, NULL, NULL, syntaxcmplt, NULL, notify, utf8_map, 0))
				return 0;
			else
				return -1;

		case LOC_OPT_ENCODING:
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), "");
			if (wmkpw(bw->parent, buf, &encodinghist, doencoding, NULL, NULL, encodingcmplt, NULL, notify, utf8_map, 0))
				return 0;
			else
				return -1;

		case LOC_OPT_FILE_TYPE:
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), "");
			if (wmkpw(bw->parent, buf, &ftypehist, doftype, "ftype", NULL, ftypecmplt, NULL, notify, utf8_map, 0))
				return 0;
			else
				return -1;

		case LOC_OPT_COLORS:
			joe_snprintf_1(buf, OPT_BUF_SIZE, joe_gettext(glopts[y].yes), "");
			if (wmkpw(bw->parent, buf, &colorhist, docolors, NULL, NULL, colorscmplt, NULL, notify, utf8_map, 0))
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
			case GLO_OPT_BOOL:
				return *glopts[y].set.b ? "ON" : "OFF";
			case GLO_OPT_INT:
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%d", *glopts[y].set.i);
				return buf;
			case GLO_OPT_STRING:
			case GLO_OPT_PATH:
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%s", *glopts[y].set.s ? *glopts[y].set.s : "");
				return buf;
			case LOC_OPT_BOOL:
				return *OPTPTR(&bw->o, glopts[y].ofst, bool) ? "ON" : "OFF";
			case LOC_OPT_INT:
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%d", *OPTPTR(&bw->o, glopts[y].ofst, int));
				return buf;
			case LOC_OPT_STRING:
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%s", *OPTPTR(&bw->o, glopts[y].ofst, char *) ? *OPTPTR(&bw->o, glopts[y].ofst, char *) : "");
				return buf;
			case LOC_OPT_RANGE:
#ifdef HAVE_LONG_LONG
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%lld", (long long)*OPTPTR(&bw->o, glopts[y].ofst, off_t) + 1);
#else
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%ld", (long)*OPTPTR(&bw->o, glopts[y].ofst, off_t) + 1);
#endif
				return buf;
			case LOC_OPT_SYNTAX:
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%s", bw->o.syntax_name ? bw->o.syntax_name : "");
				return buf;
			case LOC_OPT_ENCODING:
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%s", bw->o.map_name ? bw->o.map_name : "");
				return buf;
			case LOC_OPT_OFFSET:
#ifdef HAVE_LONG_LONG
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%lld", (long long)*OPTPTR(&bw->o, glopts[y].ofst, off_t));
#else
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%ld", (long)*OPTPTR(&bw->o, glopts[y].ofst, off_t));
#endif
				return buf;
			case LOC_OPT_FILE_TYPE:
				return bw->o.ftype;
			case LOC_OPT_COLORS:
				joe_snprintf_1(buf, OPT_BUF_SIZE, "%s", scheme_name ? scheme_name : "");
				return buf;
			default:
				return "";
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
