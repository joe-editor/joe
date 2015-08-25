/*
 *	Regular expression subroutines
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

/* Parse a character or escape sequence from ptr/len.
 * Returns with character or -256 and cat (if not NULL) filled in with a character class
 */
int escape(int utf8, const char **ptr, ptrdiff_t *len, struct Cclass **cat);

/* Submatch addresses */

typedef struct regmatch Regmatch_t;

struct regmatch {
	off_t rm_so, rm_eo; /* Starting and ending byte offset of a match, or -1 if no match */
};

/* Here is the original recursive matcher:
     Returns 0 if regex/len matches buffer at p
       set icase for a case insensitive match
       call with n = 0.
       matches is filled in with submatch addresses
       nmatch is size of pmatch array
 */

int pmatch(Regmatch_t *matches, int nmatch, const char *regex, ptrdiff_t len, P *p, ptrdiff_t n, int icase);

#define MAX_MATCHES 10
#define MAX_THREADS 50

/* Instructions */

enum {
	/* Code >= 0: Match a single specific character */
	iDOT = -512,	/* Match any single character */
	iEXPR,	/* Match any single character, but skip entire expressions */
	iBOL,	/* Match beginning of line */
	iEOL,	/* Match end of line */
	iBOW,	/* Match beginning of word */
	iEOW,	/* Match end of word */
	iBRA,	/* Opening parenthesis.  Parenthesis number follows. */
	iKET,	/* Close parenthesis.  Parenthesis number follows. */
	iFORK,	/* Alternate paths. */
	iJUMP,	/* Jump, don't eat char. */
	iCLASS,	/* Character class (address of struct range_map follows) */
	iEND,	/* End of expression. */
};

/* A comiled regular expression */

struct node {
	int type;			/* Character or type when negative */
	int r;				/* Next node */
	int l;				/* Alternate next node */
	struct Cclass cclass[1];	/* Character class if type is -'[' */
};

struct regcomp {
	const char *ptr;		/* Regular expression input string */
	ptrdiff_t l;			/* Input string remaining length */
	struct charmap *cmap;
	struct node *nodes;		/* NFA nodes */
	int len;			/* Number of allocated nodes */
	int size;			/* Malloc size of nodes */
	
	char *prefix;			/* Leading prefix of search string */
	ptrdiff_t prefix_len;
	ptrdiff_t prefix_size;

	/* Bracket number */
	int bra_no;

	/* NFA in form of generated code */
	Frag frag[1];

	const char *err;		/* Compiler error message */
};

/* Compile an expression s/len into a struct regcomp
 * Check err for compile errors.
 */
struct regcomp *joe_regcomp(struct charmap *charmap, const char *s, ptrdiff_t len, int icase, int stdfmt);

void joe_regfree(struct regcomp *r);

/* Returns 0 if buffer at p matches compiled expression r
   matches is filled in with submatch addresses
   nmatch has size of matches array
*/
int joe_regexec(struct regcomp *r, P *p, int nmatch, struct regmatch *matches, int eflags);
