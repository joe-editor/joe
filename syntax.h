/*
 *	Syntax highlighting DFA interpreter
 *	Copyright
 *		(C) 2004 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

/* Loaded form of syntax file */

struct high_syntax {
	struct high_syntax *next;	/* Linked list of loaded syntaxes */
	char *name;			/* Name of this syntax */
	struct high_state **states;	/* The states of this syntax.  states[0] is idle state */
	int nstates;			/* No. states */
};

/* State */

struct high_state {
	int no;				/* State number */
	char *name;			/* Highlight state name */
	int color;			/* Color for this state */
	struct high_cmd *cmd[256];	/* Character table */
};

/* Keyword list */

struct high_keyword {
	struct high_keyword *next;
	char *name;
	struct high_state *new_state;
};

/* Command (transition) */

struct high_cmd {
	int noeat;			/* Set to give this character to next state */
	int recolor;			/* No. chars to recolor if <0. */
	int start_buffering;		/* Set if we should start buffering */
	struct high_state *new_state;	/* The new state */
	struct high_keyword *keyword_list;
  					/* If set, new state comes from matching keyword */
};

/* Find a syntax.  Load it if necessary. */

struct high_syntax *load_dfa(char *name);

/* Parse a lines.  Returns new state. */

extern int *attr_buf;
int parse(struct high_syntax *syntax,P *line,int state);
