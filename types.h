#ifndef _JOE_TYPES_H
#define _JOE_TYPES_H

#include "config.h"

/* Prefix to make string constants unsigned */
#define US (unsigned char *)

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>				/* we need pid_t */
#endif

#define LINK(type) struct { type *next; type *prev; }

#define KEYS		256
#define stdsiz		8192
#define FITHEIGHT	4		/* Minimum text window height */
#define LINCOLS		6
#define NPROC		8		/* Number of processes we keep track of */
#define UNDOKEEP	100
#define INC		16		/* Pages to allocate each time */

#define TYPETW		0x0100
#define TYPEPW		0x0200
#define TYPEMENU	0x0800
#define TYPEQW		0x1000

#ifdef junk					/* These are now defined in config.h */

/* Minimum page size for MS-DOS is 128 (for 32K vheaders table) or 256 (for
 * 64K vheaders table) */
#define PGSIZE 512		/* Page size in bytes (Must be power of 2) */
#define LPGSIZE 9		/* LOG base 2 of PGSIZE */
#define ILIMIT (PGSIZE*128L)	/* Max amount to buffer */
#define HTSIZE 128		/* Entries in hash table.  Must be pwr of 2 */
#endif


typedef struct header H;
typedef struct buffer B;
typedef struct point P;
typedef struct options OPTIONS;
typedef struct macro MACRO;
typedef struct cmd CMD;
typedef struct entry HENTRY;
typedef struct hash HASH;
typedef struct kmap KMAP;
typedef struct kbd KBD;
typedef struct key KEY;
typedef struct watom WATOM;
typedef struct screen SCREEN;
typedef struct window W;
typedef struct base BASE;
typedef struct bw BW;
typedef struct menu MENU;
typedef struct scrn SCRN;
typedef struct cap CAP;
typedef struct pw PW;
typedef struct stditem STDITEM;
typedef struct query QW;
typedef struct tw TW;
typedef struct irec IREC;
typedef struct undo UNDO;
typedef struct undorec UNDOREC;
typedef struct search SRCH;
typedef struct srchrec SRCHREC;
typedef struct vpage VPAGE;
typedef struct vfile VFILE;


struct header {
	LINK(H)	link;		/* LINK ??? */
	long	seg;		/* ??? */
	int	hole;		/* ??? */
	int	ehole;		/* ??? */
	int	nlines;		/* ??? */
};

struct point {
	LINK(P)	link;		/* ?LINK ??? */

	B	*b;		/* ?B ??? */
	int	ofst;		/* ??? */
	unsigned char	*ptr;	/* ??? */
	H	*hdr;		/* ?H ??? */

	long	byte;		/* ??? */
	long	line;		/* ??? */
	long	col;		/* current column */
	long	xcol;		/* ??? */
	int	valcol;		/* bool: is col valid? */
	int	end;		/* ??? */

	P	**owner;	/* ??? */
};

struct options {
	OPTIONS	*next;
	unsigned char	*name_regex;
	unsigned char	*contents_regex;
	int	overtype;
	int	lmargin;
	int	rmargin;
	int	autoindent;
	int	wordwrap;
	int	tab;
	int	indentc;
	int	istep;
	unsigned char	*context;
	unsigned char	*lmsg;
	unsigned char	*rmsg;
	int	linums;
	int	readonly;
	int	french;
	int	spaces;
	int	crlf;
	int	highlight;	/* Set to enable highlighting */
	struct high_syntax *syntax;	/* Syntax for highlighting */
	int	utf8;		/* Set for UTF-8 mode */
	MACRO	*mnew;		/* Macro to execute for new files */
	MACRO	*mold;		/* Macro to execute for existing files */
	MACRO	*msnew;		/* Macro to execute before saving new files */
	MACRO	*msold;		/* Macro to execute before saving existing files */
};

struct macro {
	int	k;		/* Keycode */
	int	arg;		/* Repeat argument */
	CMD	*cmd;		/* Command address */
	int	n;		/* Number of steps */
	int	size;		/* Malloc size of steps */
	MACRO	**steps;	/* Block */
};

struct recmac {
	struct recmac *next;
	int	n;
	MACRO	*m;
};


/* Command entry */

struct cmd {
	unsigned char	*name;		/* Command name */
	int	flag;		/* Execution flags */
	int	(*func) ();	/* Function bound to name */
	MACRO	*m;		/* Macro bound to name */
	int	arg;		/* 0= arg is meaningless, 1= ok */
	unsigned char	*negarg;	/* Command to use if arg was negative */
};



struct buffer {
	LINK(B)	link;
	P	*bof;
	P	*eof;
	unsigned char	*name;
	int	orphan;
	int	count;
	int	changed;
	int	backup;
	void	*undo;
	P	*marks[10];	/* Bookmarks */
	OPTIONS	o;		/* Options */
	P	*oldcur;	/* Last cursor position before orphaning */
	P	*oldtop;	/* Last top screen position before orphaning */
	int	rdonly;		/* Set for read-only */
	int	internal;	/* Set for internal buffers */
	int	er;		/* Error code when file was loaded */
};


struct entry {
	unsigned char	*name;
	HENTRY	*next;
	void	*val;
};

struct hash {
	int	len;
	HENTRY	**tab;
};


struct help {
	unsigned char	*text;		/* help text with attributes */
	unsigned int	lines;		/* number of lines */
	struct help	*prev;		/* previous help screen */
	struct help	*next;		/* nex help screen */
	unsigned char	*name;		/* context name for context sensitive help */
};

/* A key binding */
struct key {
	int	k;			/* Flag: 0=binding, 1=submap */
	union {
		void	*bind;		/* What key is bound to */
		KMAP	*submap;	/* Sub KMAP address (for prefix keys) */
	} value;
};

/* A map of keycode to command/sub-map bindings */
struct kmap {
	KEY	keys[KEYS];	/* KEYs */
};

/** A keyboard handler **/
struct kbd {
	KMAP	*curmap;	/* Current keymap */
	KMAP	*topmap;	/* Top-level keymap */
	int	seq[16];	/* Current sequence of keys */
	int	x;		/* What we're up to */
};


struct watom {
	unsigned char	*context;	/* Context name */
	void	(*disp) ();	/* Display window */
	void	(*follow) ();	/* Called to have window follow cursor */
	int	(*abort) ();	/* Common user functions */
	int	(*rtn) ();
	int	(*type) ();
	void	(*resize) ();	/* Called when window changed size */
	void	(*move) ();	/* Called when window moved */
	void	(*ins) ();	/* Called on line insertions */
	void	(*del) ();	/* Called on line deletions */
	int	what;		/* Type of this thing */
};

struct screen {
	SCRN	*t;		/* Screen data on this screen is output to */

	int	wind;		/* Number of help lines on this screen */

	W	*topwin;	/* Top-most window showing on screen */
	W	*curwin;	/* Window cursor is in */

	int	w, h;		/* Width and height of this screen */
};

struct window {
	LINK(W)	link;		/* Linked list of windows in order they
				   appear on the screen */

	SCREEN	*t;		/* Screen this thing is on */

	int	x, y, w, h;	/* Position and size of window */
				/* Currently, x = 0, w = width of screen. */
				/* y == -1 if window is not on screen */

	int	ny, nh;		/* Temporary values for wfit */

	int	reqh;		/* Requested new height or 0 for same */
				/* This is an argument for wfit */

	int	fixed;		/* If this is zero, use 'hh'.  If not, this
				   is a fixed size window and this variable
				   gives its height */

	int	hh;		/* Height window would be on a screen with
				   1000 lines.  When the screen size changes
				   this is used to calculate the window's
				   real height */

	W	*win;		/* Window this one operates on */
	W	*main;		/* Main window of this family */
	W	*orgwin;	/* Window where space from this window came */
	int	curx, cury;	/* Cursor position within window */
	KBD	*kbd;		/* Keyboard handler for this window */
	WATOM	*watom;		/* The type of this window */
	void	*object;	/* Object which inherits this */
#if 0
	union {			/* FIXME: instead of void *object we should */
		BW	*bw;	/* use this union to get strict type checking */
		PW	*pw;	/* from C compiler (need to check and change */
		QW	*qw;	/* all of the occurrencies of ->object) */
		TW	*tw;
		MENU	*menu;
		BASE	*base;
	} object;
#endif

	unsigned char	*msgt;		/* Message at top of window */
	unsigned char	*msgb;		/* Message at bottom of window */
	unsigned char	*huh;		/* Name of window for context sensitive hlp */
	int	*notify;	/* Address of kill notification flag */
};

/* Anything which goes in window.object must start like this: */
struct base {
	W	*parent;
};

struct bw {
	W	*parent;
	B	*b;
	P	*top;
	P	*cursor;
	long	offset;
	SCREEN	*t;
	int	h, w, x, y;

	OPTIONS	o;
	void	*object;

	pid_t	pid;		/* Process id */
	int	out;		/* fd to write to process */
	int	linums;
	int	top_changed;	/* Top changed */
};

struct menu {
	W	*parent;	/* Window we're in */
	unsigned char	**list;		/* List of items */
	int	top;		/* First item on screen */
	int	cursor;		/* Item cursor is on */
	int	width;		/* Width of widest item, up to 'w' max */
	int	perline;	/* Number of items on each line */
	int	nitems;		/* No. items in list */
	SCREEN	*t;		/* Screen we're on */
	int	h, w, x, y;
	int	(*abrt) ();	/* Abort callback function */
	int	(*func) ();	/* Return callback function */
	int	(*backs) ();	/* Backspace callback function */
	void	*object;
};

struct hentry {
	int	next;
	int	loc;
};

/* Each terminal has one of these */

#ifdef __MSDOS__

struct scrn {
	int	li;		/* Height of screen */
	int	co;		/* Width of screen */
	short	*scrn;		/* Buffer */
	int	scroll;
	int	insdel;
	int	*updtab;	/* Lines which need to be updated */
	/* HIGHLIGHT_STATE *syntab; */ /* Syntax highlight state at start of each line */
	int	*syntab;
	int	*compose;
	int	*sary;
};

#else
struct scrn {
	CAP	*cap;		/* Termcap/Terminfo data */

	int	li;		/* Screen height */
	int	co;		/* Screen width */

	unsigned char	*ti;		/* Initialization string */
	unsigned char	*cl;		/* Home and clear screen... really an
				   init. string */
	unsigned char	*cd;		/* Clear to end of screen */
	unsigned char	*te;		/* Restoration string */

	int	haz;		/* Terminal can't print ~s */
	int	os;		/* Terminal overstrikes */
	int	eo;		/* Can use blank to erase even if os */
	int	ul;		/* _ overstrikes */
	int	am;		/* Terminal has autowrap, but not magicwrap */
	int	xn;		/* Terminal has magicwrap */

	unsigned char	*so;		/* Enter standout (inverse) mode */
	unsigned char	*se;		/* Exit standout mode */

	unsigned char	*us;		/* Enter underline mode */
	unsigned char	*ue;		/* Exit underline mode */
	unsigned char	*uc;		/* Single time underline character */

	int	ms;		/* Ok to move when in standout/underline mode */

	unsigned char	*mb;		/* Enter blinking mode */
	unsigned char	*md;		/* Enter bold mode */
	unsigned char	*mh;		/* Enter dim mode */
	unsigned char	*mr;		/* Enter inverse mode */
	unsigned char	*me;		/* Exit above modes */

	unsigned char	*Sb;		/* Set background color */
	unsigned char	*Sf;		/* Set foregrond color */
	int	ut;		/* Screen erases with background color */

	int	da, db;		/* Extra lines exist above, below */
	unsigned char	*al, *dl, *AL, *DL;	/* Insert/delete lines */
	unsigned char	*cs;		/* Set scrolling region */
	int	rr;		/* Set for scrolling region relative addressing */
	unsigned char	*sf, *SF, *sr, *SR;	/* Scroll */

	unsigned char	*dm, *dc, *DC, *ed;	/* Delete characters */
	unsigned char	*im, *ic, *IC, *ip, *ei;	/* Insert characters */
	int	mi;		/* Set if ok to move while in insert mode */

	unsigned char	*bs;		/* Move cursor left 1 */
	int	cbs;
	unsigned char	*lf;		/* Move cursor down 1 */
	int	clf;
	unsigned char	*up;		/* Move cursor up 1 */
	int	cup;
	unsigned char	*nd;		/* Move cursor right 1 */

	unsigned char	*ta;		/* Move cursor to next tab stop */
	int	cta;
	unsigned char	*bt;		/* Move cursor to previous tab stop */
	int	cbt;
	int	tw;		/* Tab width */

	unsigned char	*ho;		/* Home cursor to upper left */
	int	cho;
	unsigned char	*ll;		/* Home cursor to lower left */
	int	cll;
	unsigned char	*cr;		/* Move cursor to left edge */
	int	ccr;
	unsigned char	*RI;		/* Move cursor right n */
	int	cRI;
	unsigned char	*LE;		/* Move cursor left n */
	int	cLE;
	unsigned char	*UP;		/* Move cursor up n */
	int	cUP;
	unsigned char	*DO;		/* Move cursor down n */
	int	cDO;
	unsigned char	*ch;		/* Set cursor column */
	int	cch;
	unsigned char	*cv;		/* Set cursor row */
	int	ccv;
	unsigned char	*cV;		/* Goto beginning of specified line */
	int	ccV;
	unsigned char	*cm;		/* Set cursor row and column */
	int	ccm;

	unsigned char	*ce;		/* Clear to end of line */
	int	cce;

	/* Basic abilities */
	int	scroll;		/* Set to use scrolling */
	int	insdel;		/* Set to use insert/delete within line */

	/* Current state of terminal */
	int	*scrn;		/* Characters on screen */
	int	*attr;		/* Attributes on screen */
	int	x, y;		/* Current cursor position (-1 for unknown) */
	int	top, bot;	/* Current scrolling region */
	int	attrib;		/* Current character attributes */
	int	ins;		/* Set if we're in insert mode */

	int	*updtab;	/* Dirty lines table */
	int	*syntab;
	int	avattr;		/* Bits set for available attributes */
	int	*sary;		/* Scroll buffer array */

	int	*compose;	/* Line compose buffer */
	int	*ofst;		/* stuff for magic */
	struct hentry	*htab;
	struct hentry	*ary;
};
#endif


struct sortentry {
	unsigned char	*name;
	unsigned char	*value;
};

struct cap {
	unsigned char	*tbuf;		/* Termcap entry loaded here */

	struct sortentry *sort;	/* Pointers to each capability stored in here */
	int	sortlen;	/* Number of capabilities */

	unsigned char	*abuf;		/* For terminfo compatible version */
	unsigned char	*abufp;

	int	div;		/* tenths of MS per char */
	int	baud;		/* Baud rate */
	unsigned char	*pad;		/* Padding string or NULL to use NUL */
	void	(*out) (unsigned char *, unsigned char);		/* Character output routine */
	void	*outptr;	/* First arg passed to output routine.  Second
				   arg is character to write */
	int	dopadding;	/* Set if pad characters should be used */
};


struct pw {
	int	(*pfunc) ();	/* Func which gets called when RTN is hit */
	int	(*abrt) ();	/* Func which gets called when window is aborted */
	int	(*tab) ();	/* Func which gets called when TAB is hit */
	unsigned char	*prompt;	/* Prompt string */
	int	promptlen;	/* Width of prompt string */
	int	promptofst;	/* Prompt scroll offset */
	B	*hist;		/* History buffer */
	void	*object;	/* Object */
};

struct stditem {
	LINK(STDITEM)	link;
};

struct query {
	W	*parent;	/* Window we're in */
	int	(*func) ();	/* Func. which gets called when key is hit */
	int	(*abrt) ();
	void	*object;
	unsigned char	*prompt;	/* Prompt string */
	int	promptlen;	/* Width of prompt string */
	int	promptofst;	/* Prompt scroll offset */
};


typedef struct mpx MPX;
struct mpx {
	int	ackfd;		/* Packetizer response descriptor */
	int	kpid;		/* Packetizer process id */
	int	pid;		/* Client process id */
	void	(*func) ();	/* Function to call when read occures */
	void	*object;	/* First arg to pass to function */
	void	(*die) ();	/* Function: call when client dies or closes */
	void	*dieobj;
};


struct tw {
	unsigned char	*stalin;	/* Status line info */
	unsigned char	*staright;
	int	staon;		/* Set if status line was on */
	long	prevline;	/* Previous cursor line number */
	int	changed;	/* Previous changed value */
};

struct irec {
	LINK(IREC)	link;
	int	what;		/* 0 repeat, >0 append n chars */
	long	start;		/* Cursor search position */
	long	disp;		/* Original cursor position */
};

struct isrch {
	IREC	irecs;		/* Linked list of positions */
	unsigned char	*pattern;	/* Search pattern string/prompt */
	int	ofst;		/* Offset in pattern past prompt */
	int	dir;		/* 0=fwrd, 1=bkwd */
	int	quote;		/* Set to quote next char */
};


struct undorec {
	LINK(UNDOREC)	link;
	UNDOREC	*unit;
	int	min;
	int	changed;	/* Status of modified flag before this record */
	long	where;		/* Buffer address of this record */
	long	len;		/* Length of insert or delete */
	int	del;		/* Set if this is a delete */
	B	*big;		/* Set to buffer containing a large amount of deleted data */
	unsigned char	*small;		/* Set to malloc block containg a small amount of deleted data */
};

struct undo {
	LINK(UNDO)	link;
	B	*b;
	int	nrecs;
	UNDOREC	recs;
	UNDOREC	*ptr;
	UNDOREC	*first;
	UNDOREC	*last;
};

struct srchrec {
	LINK(SRCHREC)	link;	/* Linked list of search & replace locations */
	int	yn;		/* Did we replace? */
	long	addr;		/* Where we were */
};

struct search {
	unsigned char	*pattern;	/* Search pattern */
	unsigned char	*replacement;	/* Replacement string */
	int	backwards;	/* Set if search should go backwards */
	int	ignore;		/* Set if we should ignore case */
	int	repeat;		/* Set with repeat count (or -1 for no repeat count) */
	int	replace;	/* Set if this is search & replace */
	int	rest;		/* Set to do remainder of search & replace w/o query */
	unsigned char	*entire;	/* Entire matched string */
	unsigned char	*pieces[26];	/* Peices of the matched string */
	int	flg;		/* Set after prompted for first replace */
	SRCHREC	recs;		/* Search & replace position history */
	P	*markb, *markk;	/* Original marks */
	int	valid;		/* Set if original marks are a valid block */
	long	addr;		/* Addr of last replacement or -1 for none */
	int	block_restrict;	/* Search restricted to marked block */
};



/* Page header */

struct vpage {
	VPAGE	*next;		/* Next page with same hash value */
	VFILE	*vfile;		/* Owner vfile */
	long	addr;		/* Address of this page */
	int	count;		/* Reference count */
	int	dirty;		/* Set if page changed */
	unsigned char	*data;		/* The data in the page */
};

/* File structure */

struct vfile {
	LINK(VFILE)	link;	/* Doubly linked list of vfiles */
	long	size;		/* Number of bytes in physical file */
	long	alloc;		/* Number of bytes allocated to file */
	int	fd;		/* Physical file */
	int	writeable;	/* Set if we can write */
	unsigned char	*name;		/* File name.  0 if unnamed */
	int	flags;		/* Set if this is only a temporary file */

	/* For array I/O */
	unsigned char	*vpage1;	/* Page address */
	long	addr;		/* File address of above page */

	/* For stream I/O */
	unsigned char	*bufp;		/* Buffer pointer */
	unsigned char	*vpage;		/* Buffer pointer points in here */
	int	left;		/* Space left in bufp */
	int	lv;		/* Amount of append space at end of buffer */
};

#endif
