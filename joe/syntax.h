/*
 *	Syntax highlighting DFA interpreter
 *	Copyright
 *		(C) 2004 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

/* State */

extern const char **state_names;

struct high_state {
	ptrdiff_t no;			/* State number */
	int name;			/* Highlight state name (index into state_names) */
	int color;			/* Color for this state */
	struct color_def *colorp;	/* Mapped color definition */

	struct Rtree rtree;		/* Character map (character ->struct high_cmd *) */
	struct high_cmd *dflt;		/* Default for no match */
	struct high_cmd *same_delim;	/* Same delimiter */
	struct high_cmd *delim;		/* Matching delimiter */
};

/* Parameter list */

struct high_param {
	struct high_param *next;
	char *name;
};

/* Command (transition) */

struct high_cmd {
	bool noeat;			/* Set to give this character to next state */
	bool start_buffering;		/* Set if we should start buffering */
	bool stop_buffering;		/* Set if we should stop buffering */
	bool save_c;			/* Save character */
	bool save_s;			/* Save string */
	bool push_c;			/* Push character */
	bool push_s;			/* Push string */
	bool pop_c;			/* Push character */
	bool pop_s;			/* Push string */
	bool ignore;			/* Set to ignore case */
	bool start_mark;		/* Set to begin marked area including this char */
	bool stop_mark;			/* Set to end marked area excluding this char */
	bool recolor_mark;		/* Set to recolor marked area with new state */
	bool rtn;			/* Set to return */
	bool reset;			/* Set to reset the call stack */
	ptrdiff_t recolor;		/* No. chars to recolor if <0. */
	struct high_state *new_state;	/* The new state */
	ZHASH *keywords;		/* Hash table of keywords */
	struct high_cmd *delim;		/* Matching delimiter */
	struct high_syntax *call;	/* Syntax subroutine to call */
};

/* Call stack frame */

struct high_frame {
	struct high_frame *parent;		/* Caller's frame */
	struct high_frame *child;		/* First callee's frame */
	struct high_frame *sibling;		/* Caller's next callee's frame */
	struct high_syntax *syntax;		/* Current syntax subroutine */
	struct high_state *return_state;	/* Return state in the caller's subroutine */
};

/* delimiter stack frame */

struct high_delim_frame {
	struct high_delim_frame *parent;	/* Previous pushed delimiter buffer */
	struct high_delim_frame *child;		/* Next pushed delimiter buffer */
	struct high_delim_frame *sibling;	/* Another pushed delimiter buffer at the same level */
	const int *saved_s;			/* Pushed delimiter buffer */
};

/* Loaded form of syntax file or subroutine */

struct high_syntax {
	struct high_syntax *next;			/* Linked list of loaded syntaxes */
	char *name;					/* Name of this syntax */
	char *subr;					/* Name of the subroutine (or NULL for whole file) */
	struct high_param *params;			/* Parameters defined */
	struct high_state **states;			/* The states of this syntax.  states[0] is idle state */
	HASH *ht_states;				/* Hash table of states */
	ptrdiff_t nstates;				/* No. states */
	ptrdiff_t szstates;				/* Malloc size of states array */
	struct color_def *color;			/* Linked list of color definitions */
	struct high_cmd default_cmd;			/* Default transition for new states */
	struct high_frame *stack_base;			/* Root of run-time call tree */
	struct high_delim_frame *delim_stack_base;	/* Root of run-time delimiter buffer tree */
};

/* Find a syntax.  Load it if necessary. */

struct high_syntax *load_syntax(const char *name);

/* Parse a lines.  Returns new state. */

HIGHLIGHT_STATE parse(struct high_syntax *syntax,P *line,HIGHLIGHT_STATE state,struct charmap *charmap);

typedef int attr_data;
struct state_debug_data {
	int name, recolor;
};
extern attr_data *attr_buf;
extern struct state_debug_data *syndebug_buf;

#define clear_state(s) (((s)->saved_s = 0), ((s)->state = 0), ((s)->stack = 0), ((s)->delim_stack = 0))
#define invalidate_state(s) (((s)->state = -1), ((s)->saved_s = 0), ((s)->stack = 0), ((s)->delim_stack = 0))
#define move_state(to,from) (*(to)= *(from))
#define eq_state(x,y) ((x)->state == (y)->state && (x)->stack == (y)->stack && (x)->delim_stack == (y)->delim_stack && (x)->saved_s == (y)->saved_s)

extern struct high_syntax *syntax_list;

void dump_syntax(BW *bw);
