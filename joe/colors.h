
#define COLORS_NAME_LENGTH	80

#define COLORSPEC_TYPE_NONE	0
#define COLORSPEC_TYPE_ATTR	1
#define COLORSPEC_TYPE_GUI	2
#define COLORSET_GUI		-1

struct color_builtin_specs {
	const char		*name;
	int			*attribute;
	int			*mask;
	int			default_attr;
	int			default_mask;
	int			*default_ptr;
};

struct color_spec {
	int			type;
	int			atr;
	int			gui_fg;
	int			gui_bg;
};

struct color_ref {
	const char		*name;
	struct color_ref	*next;
};

struct color_def {
	const char		*name;
	struct color_ref	*refs;
	struct color_def	*next;
	struct color_spec	spec;
	int			visited;
};

struct color_set {
	COLORSET		*next;
	int			colors;
	HASH			*syntax;
	struct color_def	*alldefs;
	struct color_spec	*builtins;
	struct color_spec	termcolors[16];
};

struct color_scheme {
	LINK(SCHEME)		link;
	char			name[COLORS_NAME_LENGTH];
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
