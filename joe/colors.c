
#include "types.h"

#define COLORDEF_VISITED	1
#define COLORDEF_VISITING	2

const char *scheme_name = NULL;
struct color_scheme *curscheme = NULL;
struct color_set *curschemeset = NULL;

static int bg_cursor;
static SCHEME allcolors = { {&allcolors, &allcolors} };

static void visit_colordef(COLORSET *, struct high_syntax *, struct color_def *);

static struct color_builtin_specs color_builtins[] = {
	{ "text", &bg_text, NULL, 0, 0, 0 },	/* Must come first (because default_text) */
	{ "linum", &bg_linum, NULL, 0, 0, &bg_text },
	{ "curlin", &bg_curlin, &curlinmask, 0, -1, &bg_text },
	{ "curlinum", &bg_curlinum, NULL, 0, 0, &bg_linum },
	{ "selection", &selectatr, &selectmask, INVERSE, ~INVERSE, NULL },
	{ "help", &bg_help, NULL, 0, 0, &bg_text },
	{ "status", &bg_stalin, NULL, 0, 0, &bg_text },
	{ "menu", &bg_menu, NULL, 0, 0, &bg_text },
	{ "menusel", &bg_menusel, &bg_menumask, INVERSE, ~INVERSE, NULL },
	{ "prompt", &bg_prompt, NULL, 0, 0, &bg_text },
	{ "message", &bg_msg, NULL, 0, 0, &bg_text },
	{ "cursor", &bg_cursor, NULL, 0, 0, NULL },
	{ NULL, NULL, NULL, 0, 0, NULL }
};

struct color_states {
	struct color_states	*next;
	char			*term;
	char			*scheme;
};

static struct color_states *saved_scheme_configs = NULL;

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
	colorset->alldefs = NULL;

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
	cdef->next = NULL;
	cdef->spec.type = 0;

	return cdef;
}

static int parse_scoped_ident(const char **p, char *dest, ptrdiff_t sz)
{
	if (!parse_ident(p, dest, sz)) {
		if (!parse_char(p, '.')) {
			int n = zlen(dest);
			dest[n++] = '.';
			if (!parse_ident(p, &dest[n], sz - n)) {
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
	dest->atr = 0;
	dest->gui_fg = 0;
	dest->gui_bg = 0;

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
				dest->atr |= ((color & FG_MASK) >> FG_SHIFT) << BG_SHIFT;
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

int parse_color_def(const char **p, struct color_def *dest)
{
	char buf[COLORS_NAME_LENGTH];
	struct color_ref **last = &dest->refs;

	dest->spec.type = COLORSPEC_TYPE_NONE;
	dest->visited = 0;
	dest->refs = NULL;
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
	struct color_def **lastdef;
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
						lastdef = &curset->alldefs;
					} else if (!parse_int(&p, &count)) {
						curset = colorset_alloc();
						curset->colors = count;
						curset->next = colors->sets;
						colors->sets = curset;
						lastdef = &curset->alldefs;
					} else {
						logerror_2(joe_gettext(_("%s: %d: Invalid .colors specification\n")), name, line);
					}
				} else {
					logerror_3(joe_gettext(_("%s: %d: Unexpected directive %s\n")), name, line, bf);
				}
			} else {
				logerror_2(joe_gettext(_("%s: %d: Syntax error\n")), name, line);
			}
		} else if (!curset && parse_ws(&p, '#')) {
			/* Check a set is selected */
			logerror_2(joe_gettext(_("%s: %d: Unexpected declaration outside of color block\n")), name, line);
		} else if (!parse_char(&p, '-')) {
			if (!parse_ident(&p, bf, SIZEOF(bf))) {
				if (!zicmp(bf, "term")) {
					/* -term */
					int termcolor;
					
					parse_ws(&p, '#');
					if (!parse_int(&p, &termcolor) && termcolor >= 0 && termcolor <= 15) {
						if (parse_color_spec(&p, &curset->termcolors[termcolor])) {
							logerror_2(joe_gettext(_("%s: %d: Invalid color spec\n")), name, line);
						}
					} else {
						logerror_2(joe_gettext(_("%s: %d: Invalid terminal color\n")), name, line);
					}
				} else {
					/* -selection, -status, etc */
					int i;
					
					for (i = 0; color_builtins[i].name; i++) {
						if (!zicmp(bf, color_builtins[i].name)) {
							if (parse_color_spec(&p, &curset->builtins[i])) {
								logerror_2(joe_gettext(_("%s: %d: Invalid color spec\n")), name, line);
							}
							
							break;
						}
					}
					
					if (!color_builtins[i].name) {
						logerror_3(joe_gettext(_("%s: %d: Unknown builtin color %s\n")), name, line, bf);
					}
				}
			} else {
				logerror_2(joe_gettext(_("%s: %d: Expected identifier\n")), name, line);
			}
		} else if (!parse_char(&p, '=')) {
			/* =SyntaxColors */
			if (!parse_scoped_ident(&p, bf, SIZEOF(bf))) {
				struct color_def *cdef;
				
				cdef = (struct color_def *) joe_calloc(1, SIZEOF(struct color_def));
				cdef->name = atom_add(bf);
				
				if (!parse_color_def(&p, cdef)) {
					htadd(curset->syntax, cdef->name, cdef);
					*lastdef = cdef;
					lastdef = &cdef->next;
				} else {
					logerror_2(joe_gettext(_("%s: %d: Invalid color spec\n")), name, line);
					joe_free(cdef);
				}
			} else {
				logerror_2(joe_gettext(_("%s: %d: Expected identifier\n")), name, line);
			}
		}
	}
	
	/* Resolve refs in each set */
	for (curset = colors->sets; curset; curset = curset->next) {
		struct color_def *cdef;
		
		/* Reset */
		for (cdef = curset->alldefs; cdef; cdef = cdef->next) {
			cdef->visited = 0;
		}
		
		/* Visit */
		for (cdef = curset->alldefs; cdef; cdef = cdef->next) {
			struct color_ref *cref = cdef->refs;
			
			visit_colordef(curset, NULL, cdef);
			
			/* Remove references */
			while (cref) {
				struct color_ref *prev = cref;
				cref = cref->next;
				joe_free(prev);
			}
			
			cdef->refs = NULL;
			
			/* Lock in color */
			if (cdef->spec.type == COLORSPEC_TYPE_NONE) {
				cdef->spec.type = COLORSPEC_TYPE_ATTR;
				cdef->spec.atr = 0;
			}
		}
	}
	
	return colors;
}

char **get_colors(void)
{
	return find_configs(NULL, "jcf", "colors", "colors");
}

int apply_scheme(SCHEME *colors)
{
	struct high_syntax *stx;
	struct color_set *best = NULL, *p;
	int i;
	int supported = maint->t->assume_256 ? 256 : maint->t->Co;
	
	if (!colors)
		return 1;
	
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
			if (color_builtins[i].attribute) {
				if (color_builtins[i].default_ptr)
					*color_builtins[i].attribute = *color_builtins[i].default_ptr;
				else
					*color_builtins[i].attribute = color_builtins[i].default_attr;
			}
			if (color_builtins[i].mask)
				*color_builtins[i].mask = color_builtins[i].default_mask;
		} else {
			/* Take from scheme */
			if (color_builtins[i].attribute)
				*color_builtins[i].attribute = best->builtins[i].atr;
			if (color_builtins[i].mask) {
				int atr = best->builtins[i].atr;
				int mask = -1;
				
				if (atr & FG_NOT_DEFAULT)
					mask &= ~FG_MASK;
				if (atr & BG_NOT_DEFAULT)
					mask &= ~BG_MASK;
				if (atr & AT_MASK)
					mask &= ~AT_MASK;
				
				*color_builtins[i].mask = mask;
			}
		}
	}
	
	/* Apply colors to syntaxes and states */
	for (stx = syntax_list; stx; stx = stx->next) {
		resolve_syntax_colors(best, stx);
	}
	
	curscheme = colors;
	curschemeset = best;
	scheme_name = curscheme->name;
	
	return 0;
}

static void visit_colordef(COLORSET *cset, struct high_syntax *syntax, struct color_def *cdef)
{
	struct color_ref *cref;

	if (cdef->visited == COLORDEF_VISITED)
		return;
	
	if (cdef->visited == COLORDEF_VISITING) {
		logerror_2(joe_gettext(_("%s: Recursive color definition found involving %s\n")), syntax->name, cdef->name);
		return;
	}
	
	if (cdef->spec.type != COLORSPEC_TYPE_NONE) {
		cdef->visited = COLORDEF_VISITED;
		return;
	}
	
	cdef->visited = COLORDEF_VISITING;
	
	for (cref = cdef->refs; cref; cref = cref->next) {
		struct color_def *rcdef = NULL;
		if (syntax) {
			for (rcdef = syntax->color; rcdef; rcdef = rcdef->next) {
				if (!zcmp(rcdef->name, cref->name)) {
					break;
				}
			}
		}
		
		if (!rcdef) {
			rcdef = htfind(cset->syntax, cref->name);
		}
		
		if (rcdef) {
			/* Visit first and make sure it has a value */
			visit_colordef(cset, syntax, rcdef);

			if (rcdef->spec.type != COLORSPEC_TYPE_NONE) {
				/* Found something */
				memcpy(&cdef->spec, &rcdef->spec, SIZEOF(struct color_spec));
				break;
			}
		}
	}
	
	cdef->visited = COLORDEF_VISITED;
}

void resolve_syntax_colors(COLORSET *cset, struct high_syntax *syntax)
{
	int i;
	struct color_def *scdef;
	
	/* Reset */
	for (scdef = syntax->color; scdef; scdef = scdef->next) {
		memset(&scdef->spec, 0, SIZEOF(struct color_spec));
		scdef->spec.type = COLORSPEC_TYPE_NONE;
		scdef->visited = 0;
	}
	
	if (cset) {
		/* Find colors from the scheme matching the syntax's class name. */
		for (scdef = syntax->color; scdef; scdef = scdef->next) {
			/* Is there a color by this one's name? */
			struct color_def *cdef;
			char buf[128];
			
			joe_snprintf_2(buf, SIZEOF(buf), "%s.%s", syntax->name, scdef->name);
			cdef = htfind(cset->syntax, buf);
			if (!cdef) {
				cdef = htfind(cset->syntax, scdef->name);
			}
			
			if (cdef) {
				memcpy(&scdef->spec, &cdef->spec, SIZEOF(struct color_spec));
				scdef->visited = COLORDEF_VISITED;
			}
		}
		
		/* Follow refs */
		for (scdef = syntax->color; scdef; scdef = scdef->next) {
			visit_colordef(cset, syntax, scdef);
		}
	}
	
	/* Propagate colors into the states */
	for (i = 0; i < syntax->nstates; i++) {
		struct high_state *st = syntax->states[i];
		
		if (st->colorp) {
			st->color = st->colorp->spec.atr | (st->color & CONTEXT_MASK);
		} else {
			st->color &= CONTEXT_MASK;
		}
	}
}

void dump_colors(BW *bw)
{
	char buf[256];
	SCHEME *colors;
	
	for (colors = allcolors.link.next; colors != &allcolors; colors = colors->link.next) {
		COLORSET *cset;
		
		joe_snprintf_1(buf, SIZEOF(buf), "Color scheme [%s]\n", colors->name);
		binss(bw->cursor, buf);
		pnextl(bw->cursor);
		
		for (cset = colors->sets; cset; cset = cset->next) {
			struct color_def *cdef;
			
			joe_snprintf_1(buf, SIZEOF(buf), "* Color set [%d]\n", cset->colors);
			binss(bw->cursor, buf);
			pnextl(bw->cursor);
			
			for (cdef = cset->alldefs; cdef; cdef = cdef->next) {
				struct color_ref *cref;
				
				joe_snprintf_1(buf, SIZEOF(buf), "  * Color Definition [%s]\n", cdef->name);
				binss(bw->cursor, buf);
				pnextl(bw->cursor);
				
				for (cref = cdef->refs; cref; cref = cref->next) {
					joe_snprintf_1(buf, SIZEOF(buf), "    * Color Reference [%s]\n", cref->name);
					binss(bw->cursor, buf);
					pnextl(bw->cursor);
				}

				joe_snprintf_3(buf, SIZEOF(buf), "    * Spec type=%d [%d/%d]\n", 
					cdef->spec.type, 
					(cdef->spec.atr & FG_MASK & ~FG_NOT_DEFAULT) >> FG_SHIFT, 
					(cdef->spec.atr & BG_MASK & ~FG_NOT_DEFAULT) >> BG_SHIFT);
				binss(bw->cursor, buf);
				pnextl(bw->cursor);
			}
		}
	}
}

void load_colors_state(FILE *fp)
{
	char buf[256];
	char bf[256];
	
	while (fgets(buf, SIZEOF(buf)-1, fp) && zcmp(buf, "done\n")) {
		const char *p = buf;
		ptrdiff_t len;
		char *term;
		
		parse_ws(&p, '#');
		
		/* Key */
		term = NULL;
		len = parse_string(&p, bf, SIZEOF(bf));
		if (len <= 0) continue;
		term = zdup(bf);
		
		parse_ws(&p, '#');
		
		/* Value */
		len = parse_string(&p, bf, SIZEOF(bf));
		if (len > 0) {
			struct color_states *st = joe_malloc(SIZEOF(struct color_states));
			st->term = term;
			st->scheme = zdup(bf);
			st->next = saved_scheme_configs;
			saved_scheme_configs = st;
		} else if (term) {
			joe_free(term);
		}
	}
}

void save_colors_state(FILE *fp)
{
	struct color_states *st;
	const char *myterm = getenv("TERM");
	
	for (st = saved_scheme_configs; st; st = st->next) {
		if (zcmp(myterm, st->term)) {
			fprintf(fp, "\t");
			emit_string(fp, st->term, zlen(st->term));
			fprintf(fp, " ");
			emit_string(fp, st->scheme, zlen(st->scheme));
			fprintf(fp, "\n");
		}
	}
	
	/* Save my scheme */
	if (scheme_name) {
		fprintf(fp, "\t");
		emit_string(fp, myterm, zlen(myterm));
		fprintf(fp, " ");
		emit_string(fp, scheme_name, zlen(scheme_name));
		fprintf(fp, "\n");
	}
	
	fprintf(fp, "done\n");
}

int init_colors(void)
{
	const char *myterm = getenv("TERM");
	struct color_states *st;
	int supported = maint->t->assume_256 ? 256 : maint->t->Co;
	
	/* Is one specified by global option? */
	if (scheme_name) {
		if (!apply_scheme(load_scheme(scheme_name)))
			return 0;
	}
	
	/* Search by terminal type */
	for (st = saved_scheme_configs; st; st = st->next) {
		if (st->term && !zcmp(st->term, myterm)) {
			if (!apply_scheme(load_scheme(st->scheme)))
				return 0;
		}
	}
	
	/* Try default */
	return apply_scheme(load_scheme("default"));
}
