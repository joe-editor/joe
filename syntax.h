#ifndef _Isyntax
#define _Isyntax 1

#include "hash.h"

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

/* Command (transition) */

struct high_cmd {
	int noeat;			/* Set to give this character to next state */
	int recolor;			/* No. chars to recolor if <0. */
	int start_buffering;		/* Set if we should start buffering */
	int stop_buffering;		/* Set if we should stop buffering */
	int save_c;			/* Save character */
	int save_s;			/* Save string */
	struct high_state *new_state;	/* The new state */
	HASH *keywords;			/* Hash table of keywords */
	struct high_cmd *delim;		/* Matching delimiter */
	int ignore;			/* Set to ignore case */
};

/* Loaded form of syntax file */

struct high_syntax {
	struct high_syntax *next;	/* Linked list of loaded syntaxes */
	unsigned char *name;			/* Name of this syntax */
	struct high_state **states;	/* The states of this syntax.  states[0] is idle state */
	int nstates;			/* No. states */
	int szstates;			/* Malloc size of states array */
	struct high_color *color;	/* Linked list of color definitions */
	int sync_lines;			/* No. lines back to start parsing when we lose sync.  -1 means start at beginning */
	struct high_cmd default_cmd;	/* Default transition for new states */
};

/* Find a syntax.  Load it if necessary. */

struct high_syntax *load_dfa(unsigned char *name);

/* Parse a lines.  Returns new state. */

extern int *attr_buf;
HIGHLIGHT_STATE parse(struct high_syntax *syntax,P *line,HIGHLIGHT_STATE state);

#define clear_state(s) ((s)->saved_s[0] = (s)->state = 0)
#define invalidate_state(s) ((s)->state = -1)
#define move_state(to,from) (*(to)= *(from))
#define eq_state(x,y) ((x)->state == (y)->state && !zcmp((x)->saved_s, (y)->saved_s))

extern struct high_color *global_colors;
void parse_color_def(struct high_color **color_list,unsigned char *p,unsigned char *name,int line);

#endif
