/*
 *	Color scheme handler
 *	Copyright
 *		(C) 2016
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#define COLORSPEC_TYPE_NONE	0
#define COLORSPEC_TYPE_ATTR	1
#define COLORSPEC_TYPE_GUI	2
#define COLORSET_GUI		0x1000000

/* Color specification (either an attribute, or RGB colors */
struct color_spec {
	int			type;
	int			atr;
	int			mask;
	int			gui_fg;
	int			gui_bg;
};

/* Linked list of references to other colors by name */
struct color_ref {
	const char		*name;
	struct color_ref	*next;
};

/* Syntax color definition */
struct color_def {
	const char		*name;
	struct color_ref	*refs;
	struct color_def	*next;
	struct color_spec	spec;
	struct color_spec	orig;
	int			visited;
};

/* Section of a color scheme (by supported terminal colors) */
struct color_set {
	COLORSET		*next;
	int			colors;
	HASH			*syntax;
	int			*palette;
	struct color_def	*alldefs;
	struct color_spec	*builtins;
	struct color_spec	termcolors[16];
};

/* Color scheme object (collection of sections) */
struct color_scheme {
	LINK(SCHEME)		link;
	const char		*name;
	struct color_set	*sets;
};

char **get_colors(void);
SCHEME *load_scheme(const char *);
int parse_color_spec(const char **, struct color_spec *);
int parse_color_def(const char **, struct color_def *);
int apply_scheme(SCHEME *);
void resolve_syntax_colors(COLORSET *, struct high_syntax *);
void dump_colors(BW *);
void load_colors_state(FILE *);
void save_colors_state(FILE *);
int init_colors(void);

extern const char *scheme_name;
extern struct color_scheme *curscheme;
extern struct color_set *curschemeset;
extern int bg_cursor;
