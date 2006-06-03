#ifndef _Isyntax
#define _Isyntax 1

/*
 *	Syntax highlighting DFA interpreter
 *	Copyright
 *		(C) 2004 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

/* Color definition */

struct high_color {
	struct high_color *next;
	unsigned char *name;		/* Symbolic name of color */
	int color;			/* Color value */
};

/* State */

struct high_state {
	int no;				/* State number */
	unsigned char *name;		/* Highlight state name */
	int color;			/* Color for this state */
	struct high_cmd *cmd[256];	/* Character table */
	struct high_cmd *delim;		/* Matching delimiter */
};

/* Parameter list */

struct syparm {
	struct syparm *next;
	unsigned char *name;
};

/* Command (transition) */

struct high_cmd {
	unsigned noeat : 1;		/* Set to give this character to next state */
	unsigned start_buffering : 1;	/* Set if we should start buffering */
	unsigned stop_buffering : 1;	/* Set if we should stop buffering */
	unsigned save_c : 1;		/* Save character */
	unsigned save_s : 1;		/* Save string */
	unsigned ignore : 1;		/* Set to ignore case */
	unsigned start_mark : 1;	/* Set to begin marked area including this char */
	unsigned stop_mark : 1;		/* Set to end marked area excluding this char */
	unsigned recolor_mark : 1;	/* Set to recolor marked area with new state */
	int recolor;			/* No. chars to recolor if <0. */
	struct high_state *new_state;	/* The new state */
	HASH *keywords;			/* Hash table of keywords */
	struct high_cmd *delim;		/* Matching delimiter */
	unsigned char *call;		/* Set with name of file with subroutine */
	unsigned char *call_subr;	/* Set with name of subroutine (or NULL for whole file) */
	struct syparm *parms;		/* Parameters for call */
};

/* Loaded form of syntax file */

struct high_syntax {
	struct high_syntax *next;	/* Linked list of loaded syntaxes */
	unsigned char *name;			/* Name of this syntax */
	struct high_state **states;	/* The states of this syntax.  states[0] is idle state */
	HASH *ht_states;		/* Hash table of states */
	int nstates;			/* No. states */
	int szstates;			/* Malloc size of states array */
	struct high_color *color;	/* Linked list of color definitions */
	int sync_lines;			/* No. lines back to start parsing when we lose sync.  -1 means start at beginning */
	struct high_cmd default_cmd;	/* Default transition for new states */
	int istates;			/* Loaded no. states */
	int recur;			/* Recursion depth counter */
};

/* Find a syntax.  Load it if necessary. */

struct high_syntax *load_dfa PARAMS((unsigned char *name));

/* Parse a lines.  Returns new state. */

extern int *attr_buf;
HIGHLIGHT_STATE parse PARAMS((struct high_syntax *syntax,P *line,HIGHLIGHT_STATE state));

#define clear_state(s) ((s)->saved_s[0] = (s)->state = 0)
#define invalidate_state(s) ((s)->state = -1)
#define move_state(to,from) (*(to)= *(from))
#define eq_state(x,y) ((x)->state == (y)->state && !zcmp((x)->saved_s, (y)->saved_s))

extern struct high_color *global_colors;
void parse_color_def PARAMS((struct high_color **color_list,unsigned char *p,unsigned char *name,int line));

void dump_syntax PARAMS((BW *bw));

#endif
