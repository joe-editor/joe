/*
 *	Search & Replace system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

struct srchrec {
	LINK(SRCHREC)	link;	/* Linked list of search & replace locations */
	int	yn;		/* Did we replace? */
	int	wrap_flag;	/* Did we wrap? */
	off_t	addr;		/* Where we were */
	B *b;			/* Buffer address is in */
	off_t	last_repl;
};

#define NMATCHES 26

struct search {
	char	*pattern;	/* Search pattern */
	struct regcomp *comp;	/* Compiled pattern */
	char	*replacement;	/* Replacement string */
	int	backwards;	/* Set if search should go backwards */
	int	ignore;		/* Set if we should ignore case */
	int	regex;		/* Set for standard regex syntax, clear for JOE syntax */
	int	repeat;		/* Set with repeat count (or -1 for no repeat count) */
	int	replace;	/* Set if this is search & replace */
	int	debug;		/* Set to have regex debug information sent to log buffer */
	int	rest;		/* Set to do remainder of search & replace w/o query */
	Regmatch_t pieces[NMATCHES];	/* Sub-matches we found */
	Regmatch_t entire;	/* Entire matching string */
	int	flg;		/* Set after prompted for first replace */
	SRCHREC	recs;		/* Search & replace position history */
	P	*markb, *markk;	/* Original marks */
	P	*wrap_p;	/* Wrap point */
	int	wrap_flag;	/* Set if we've wrapped */
	int	allow_wrap;	/* Set to allow wrapping */
	int	valid;		/* Set if original marks are a valid block */
	off_t	addr;		/* Where to place cursor after failed restruct_to_block() test */
	off_t	last_repl;	/* Address of last replacement (prevents infinite loops) */
	int	block_restrict;	/* Search restricted to marked block */
	int	all;		/* Set to continue in other windows */
	B	*first;		/* Starting buffer */
	B	*current;	/* Current buffer */
};

extern int std_regex; /* Standard regex format by default */

SRCH *mksrch(char *pattern, char *replacement, int ignore, int backwards, int repeat, int replace, int rest, int all, int regex);
void rmsrch(SRCH *srch);

void setpat(SRCH *srch, char *pattern);

int dopfnext(BW *bw, SRCH *srch, int *notify);

int pffirst(W *w, int k);
int pfnext(W *w, int k);

int pqrepl(W *w, int k);
int prfirst(W *w, int k);

int ufinish(W *w, int k);
int dofirst(BW *bw, int back, int repl, char *hint);

extern B *findhist; /* Search history buffer */
extern B *replhist; /* Replace history buffer */

void save_srch(FILE *f);
void load_srch(FILE *f);

extern int smode;
extern int csmode;
extern int opt_icase;
extern int wrap;
extern int pico;
extern char srchstr[];
extern SRCH *globalsrch;

extern const char *rest_key;
extern const char *backup_key;
