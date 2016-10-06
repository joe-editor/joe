
#include "types.h"

const char *scheme_name = NULL;

static int bg_cursor;
static SCHEME allcolors = { {&allcolors, &allcolors} };

static struct color_builtin_specs color_builtins[] = {
	{ "text", &bg_text, NULL, 0, 0 },
	{ "selection", &selectatr, &selectmask, INVERSE, ~INVERSE },
	{ "help", &bg_help, NULL, 0, 0 },
	{ "status", &bg_stalin, NULL, 0, 0 },
	{ "menu", &bg_menu, NULL, 0, 0 },
	{ "menu_selection", &bg_menui, NULL, INVERSE, 0 },
	{ "prompt", &bg_prompt, NULL, 0, 0 },
	{ "message", &bg_msg, NULL, 0, 0 },
	{ "cursor", &bg_cursor, NULL, 0, 0 },
	{ NULL, NULL, NULL, 0, 0 }
};


static SCHEME *scheme_alloc(void)
{
	SCHEME *colors;

	colors = (SCHEME *) joe_calloc(1, SIZEOF(struct color_scheme));
	colors->sets = NULL;
	zcpy(colors->name, "");

	enquef(SCHEME, link, &allcolors, colors);
	return colors;
}

static COLORSET *colorset_alloc(void)
{
	COLORSET *colorset;
	int i;

	colorset = (COLORSET *) joe_calloc(1, SIZEOF(struct color_set));
	colorset->syntax = htmk(64);
	colorset->builtins = (struct color_spec *) joe_calloc(SIZEOF(color_builtins), SIZEOF(int));

	/* Clear termcolors */
	for (i = 0; i < 16; i++) {
		colorset->termcolors[i].type = COLORSPEC_TYPE_NONE;
	}

	/* Clear builtins */
	for (i = 0; color_builtins[i].name; i++) {
		colorset->builtins[i].type = COLORSPEC_TYPE_NONE;
	}

	return colorset;
}

static struct color_def *colordef_alloc(void)
{
	struct color_def *cdef;

	cdef = (struct color_def *) joe_calloc(1, SIZEOF(struct color_def));
	cdef->refs = NULL;
	cdef->spec.type = 0;

	return cdef;
}

static int parse_scoped_ident(const char **p, char *dest, ptrdiff_t sz)
{
	if (!parse_ident(p, dest, sz)) {
		if (!parse_char(p, '.')) {
			int n = zlen(dest);
			dest[n++] = '.';
			if (!parse_ident(p, dest, sz - n)) {
				return 0;
			}
		} else {
			return 0;
		}
	}
	
	return 1;
}

int parse_color_spec(const char **p, struct color_spec *dest)
{
	char buf[128];
	int fg = 1, bg = 0;
	int color;

	dest->type = COLORSPEC_TYPE_NONE;
	for (;;) {
		if (!parse_ws(p, '#')) {
			return 0;
		}
		
		if (!parse_char(p, '/')) {
			if (!fg) {
				/* Background already selected */
				return 1;
			}

			fg = 0;
			bg = 1;
		} else if (!parse_char(p, '$')) {
			int i;
			
			/* GUI color */
			if (dest->type != COLORSPEC_TYPE_NONE && dest->type != COLORSPEC_TYPE_GUI) {
				/* Can't mix GUI and term */
				return 1;
			}
			
			dest->type = COLORSPEC_TYPE_GUI;
			i = 0;
			while (**p && ((**p >= 'a' && **p <= 'f') || (**p >= '0' && **p <= '9') || (**p >= 'A' && **p <= 'F'))) {
				buf[i++] = *((*p)++);
			}
			
			buf[i] = 0;
			color = zhtoi(buf);
			
			if (fg) {
				dest->gui_fg = color;
			} else {
				dest->gui_bg = color;
			}
		} else if (!parse_int(p, &color)) {
			/* Color number */
			if (dest->type != COLORSPEC_TYPE_NONE && dest->type != COLORSPEC_TYPE_ATTR) {
				/* Can't mix GUI and term */
				return 1;
			}
			
			dest->type = COLORSPEC_TYPE_ATTR;
			if (fg) dest->atr |= (color << FG_SHIFT) | FG_NOT_DEFAULT;
			if (bg) dest->atr |= (color << BG_SHIFT) | BG_NOT_DEFAULT;
		} else if (!parse_ident(p, buf, SIZEOF(buf))) {
			/* Color or attribute */
			if (dest->type != COLORSPEC_TYPE_NONE && dest->type != COLORSPEC_TYPE_ATTR) {
				/* Can't mix GUI and term */
				return 1;
			}
			
			color = meta_color(buf);
			if (!color) {
				return 1;
			}
			
			dest->type = COLORSPEC_TYPE_ATTR;
			if (bg && (FG_MASK & color)) {
				dest->atr |= ((color & BG_MASK) >> BG_SHIFT) << FG_SHIFT;
				dest->atr |= BG_NOT_DEFAULT;
			} else if (fg && (FG_MASK & color)) {
				dest->atr |= color | FG_NOT_DEFAULT;
			} else {
				dest->atr |= color;
			}
		} else {
			return 1;
		}
	}
}

int parse_color_def1(const char **p, struct color_def *dest)
{
	char buf[COLORS_NAME_LENGTH];
	struct color_ref **last = &dest->refs;

	*last = NULL;

	for (;;) {
		if (!parse_ws(p, '#')) {
			/* EOL: Use default */
			return 0;
		}

		if (!parse_char(p, '+')) {
			if (!parse_scoped_ident(p, buf, SIZEOF(buf))) {
				struct color_ref *cref = (struct color_ref *) joe_calloc(1, SIZEOF(struct color_ref));

				cref->name = atom_add(buf);
				cref->next = NULL;
				*last = cref;
				last = &cref->next;
			} else {
				return 1;
			}
		} else {
			return parse_color_spec(p, &dest->spec);
		}
	}
}

SCHEME *load_scheme(const char *name)
{
	SCHEME *colors;
	COLORSET *curset;
	const char *p;
	char buf[1024];
	char bf[256];
	JFILE *f;
	int line;
	
	/* Find existing */
	for (colors = allcolors.link.next; colors != &allcolors; colors = colors->link.next) {
		if (!zcmp(name, colors->name)) {
			return colors;
		}
	}
	
	/* Find file */
	p = getenv("HOME");
	if (p) {
		joe_snprintf_2(buf, SIZEOF(buf), "%s/.joe/colors/%s.jcf", p, name);
		f = jfopen(buf, "r");
	}
	
	if (!f) {
		joe_snprintf_2(buf, SIZEOF(buf), "%scolors/%s.jcf", JOEDATA, name);
		f = jfopen(buf, "r");
	}
	if (!f) {
		joe_snprintf_1(buf, SIZEOF(buf), "*%s.jcf", name);
		f = jfopen(buf, "r");
	}
	
	if (!f) {
		return 0;
	}
	
	/* Create */
	colors = scheme_alloc();
	zncpy(colors->name, name, COLORS_NAME_LENGTH);
	curset = 0;
	line = 0;
	
	while (jfgets(buf, SIZEOF(buf), f)) {
		++line;
		p = buf;
		parse_ws(&p, '#');
		
		if (!parse_char(&p, '.')) {
			/* .colors */
			if (!parse_ident(&p, bf, SIZEOF(bf))) {
				if (!zcmp(bf, "colors")) {
					int count = -1;
					
					parse_ws(&p, '#');
					if (!parse_char(&p, '*')) {
						/* GUI */
						curset = colorset_alloc();
						curset->colors = COLORSET_GUI;
						curset->next = colors->sets;
						colors->sets = curset;
					} else if (!parse_int(&p, &count)) {
						curset = colorset_alloc();
						curset->colors = count;
						curset->next = colors->sets;
						colors->sets = curset;
					} else {
						logerror_2(joe_gettext(_("%s: %d: Invalid .colors specification\n")), name, line);
					}
				} else {
					logerror_3(joe_gettext(_("%s: %d: Unexpected directive %s")), name, line, bf);
				}
			} else {
				logerror_2(joe_gettext(_("%s: %d: Syntax error")), name, line);
			}
		} else if (!curset && parse_ws(&p, '#')) {
			/* Check a set is selected */
			logerror_2(joe_gettext(_("%s: %d: Unexpected declaration outside of color block")), name, line);
		} else if (!parse_char(&p, '-')) {
			if (!parse_ident(&p, bf, SIZEOF(bf))) {
				if (!zicmp(bf, "term")) {
					/* -term */
					int termcolor;
					
					parse_ws(&p, '#');
					if (!parse_int(&p, &termcolor) && termcolor >= 0 && termcolor <= 15) {
						if (parse_color_spec(&p, &curset->termcolors[termcolor])) {
							logerror_2(joe_gettext(_("%s: %d: Invalid color spec")), name, line);
						}
					} else {
						logerror_2(joe_gettext(_("%s: %d: Invalid terminal color")), name, line);
					}
				} else {
					/* -selection, -status, etc */
					int i;
					
					for (i = 0; color_builtins[i].name; i++) {
						if (!zicmp(bf, color_builtins[i].name)) {
							if (parse_color_spec(&p, &curset->builtins[i])) {
								logerror_2(joe_gettext(_("%s: %d: Invalid color spec")), name, line);
							}
							
							continue;
						}
					}
					
					logerror_3(joe_gettext(_("%s: %d: Unknown builtin color %s")), name, line, bf);
				}
			} else {
				logerror_2(joe_gettext(_("%s: %d: Expected identifier")), name, line);
			}
		} else if (!parse_char(&p, '=')) {
			/* =SyntaxColors */
			if (!parse_scoped_ident(&p, bf, SIZEOF(bf))) {
				struct color_def *cdef;
				
				cdef = (struct color_def *) joe_calloc(1, SIZEOF(struct color_def));
				cdef->name = atom_add(bf);
				
				if (!parse_color_def1(&p, cdef)) {
					htadd(curset->syntax, cdef->name, cdef);
				} else {
					logerror_2(joe_gettext(_("%s: %d: Invalid color spec")), name, line);
					joe_free(cdef);
				}
			} else {
				logerror_2(joe_gettext(_("%s: %d: Expected identifier")), name, line);
			}
		}
	}
	
	return colors;
}

char **get_colors(void)
{
	return find_configs(NULL, "jcf", "colors", "colors");
}

int apply_scheme(SCHEME *colors, int supported)
{
	struct color_set *best = NULL, *p;
	int i;
	
	/* Find matching set */
	for (p = colors->sets; p; p = p->next) {
		/* GUI set would need different rules but isn't implemented yet
		   and currently wouldn't match. */
		if (p->colors <= supported && (!best || best->colors < p->colors)) {
			best = p;
		}
	}
	
	if (!best) {
		/* No matching set found */
		return 1;
	}
	
	/* Apply builtins */
	for (i = 0; color_builtins[i].name; i++) {
		if (best->builtins[i].type == COLORSPEC_TYPE_NONE) {
			/* Use default */
			if (color_builtins[i].attribute)
				*color_builtins[i].attribute = color_builtins[i].default_attr;
			if (color_builtins[i].mask)
				*color_builtins[i].mask = color_builtins[i].default_mask;
		} else {
			/* Take from scheme */
		}
	}
	
	/* Resolve syntax colors */
	
	/* Apply colors to syntaxes and states */
	
	/* Redraw */
	
	return 0;
}
