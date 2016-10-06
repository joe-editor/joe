
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
};

struct color_gui_spec {
	int			fg;
	int			bg;
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
	struct color_spec	spec;
};

struct color_set {
	COLORSET		*next;
	int			colors;
	HASH			*syntax;
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
int parse_color_def1(const char **, struct color_def *);
int apply_scheme(SCHEME *colors, int supported);

extern const char *scheme_name;
