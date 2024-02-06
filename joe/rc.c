/*
 *	*rc file parser
 *	Copyright
 *		(C) 1992 Joseph H. Allen; 
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "types.h"

/* Validate joerc file */

int validate_rc()
{
	KMAP *k;
	if (!(k = ngetcontext("main")) || kmap_empty(k)) {
		logerror_0(joe_gettext(_("Missing or empty :main keymap\n")));
		return -1;
	}

	if (!(k = ngetcontext("prompt")) || kmap_empty(k)) {
		logerror_0(joe_gettext(_("Missing or empty :prompt keymap\n")));
		return -1;
	}

	if (!(k = ngetcontext("query")) || kmap_empty(k)) {
		logerror_0(joe_gettext(_("Missing or empty :query keymap\n")));
		return -1;
	}

	if (!(k = ngetcontext("querya")) || kmap_empty(k)) {
		logerror_0(joe_gettext(_("Missing or empty :querya keymap\n")));
		return -1;
	}

	if (!(k = ngetcontext("querysr")) || kmap_empty(k)) {
		logerror_0(joe_gettext(_("Missing or empty :querysr keymap\n")));
		return -1;
	}

	if (!(k = ngetcontext("shell")) || kmap_empty(k)) {
		logerror_0(joe_gettext(_("Missing or empty :shell keymap\n")));
	}

	if (!(k = ngetcontext("vtshell")) || kmap_empty(k)) {
		logerror_0(joe_gettext(_("Missing or empty :vtshell keymap\n")));
	}

	return 0;
}

/* Parse a macro- allow it to cross lines */

static MACRO *multiparse(JFILE *fd, int *refline, char *buf, ptrdiff_t *ofst, int *referr, char *name)
{
	MACRO *m;
	ptrdiff_t x = *ofst;
	int err = *referr;
	int line = *refline;
	m = 0;
	for (;;) {
		m = mparse(m, buf + x, &x, 0);
		if (x == -1) { /* Error */
			err = -1;
			logerror_2(joe_gettext(_("%s %d: Unknown command in macro\n")), name, line);
			break;
		} else if (x == -2) { /* Get more input */
			jfgets(buf, 1024, fd);
			++line;
			x = 0;
		} else /* We're done */
			break;
	}
	*referr = err;
	*refline = line;
	*ofst = x;
	return m; 
}

/* Process rc file
 * Returns 0 if the rc file was successfully processed
 *        -1 if the rc file couldn't be opened
 *         1 if there was a syntax error in the file
 */

int procrc(CAP *cap, char *name)
{
	OPTIONS *o = &fdefault;	/* Current options */
	KMAP *context = NULL;	/* Current context */
	struct rc_menu *current_menu = NULL;
	char buf[1024];	/* Input buffer */
	char buf1[1024];	/* Input buffer */
	JFILE *fd;		/* rc file */
	int line = 0;		/* Line number */
	int err = 0;		/* Set to 1 if there was a syntax error */

	zlcpy(buf, SIZEOF(buf), name);
#ifdef __MSDOS__
	fd = jfopen(buf, "rt");
#else
	fd = jfopen(buf, "r");
#endif

	if (!fd)
		return -1;	/* Return if we couldn't open the rc file */

	logmessage_1(joe_gettext(_("Processing '%s'...\n")), name);

	while (jfgets(buf, SIZEOF(buf), fd)) {
		line++;
		switch (buf[0]) {
		case ' ':
		case '\t':
		case '\n':
		case '\f':
		case 0:
			break;	/* Skip comment lines */

		case '[':	/* Select file types for file-type dependent options */
			{
				int x;

				o = (OPTIONS *)joe_malloc(SIZEOF(OPTIONS));
				*o = fdefault;
				o->match = 0;
				for (x = 0; buf[x] && buf[x] != ']' && buf[x] != '\r' && buf[x] != '\n' && buf[x] != ' ' && buf[x] != '\t'; ++x) ;
				buf[x] = 0;
				o->next = options_list;
				options_list = o;
				o->ftype = zdup(buf + 1);
			}
			break;
		case '*':	/* Select file types for file-type dependent options */
			{ /* Space and tab introduce comments- which means we can't have them in the regex */
				if (o) {
					struct options_match *m;
					int x;
					for (x = 0; buf[x] && buf[x] != '\n' && buf[x] != ' ' && buf[x] != '\t'; ++x) ;
					buf[x] = 0;
					m = (struct options_match *)joe_malloc(SIZEOF(struct options_match));
					m->next = 0;
					m->name_regex = zdup(buf);
					m->contents_regex = 0;
					m->r_contents_regex = 0;
					m->next = o->match;
					o->match = m;
				}
			}
			break;
		case '+':	/* Set file contents match regex */
			{ /* No comments allowed- entire line used. */
				int x;

				for (x = 0; buf[x] && buf[x] != '\n' && buf[x] != '\r'; ++x) ;
				buf[x] = 0;
				if (o && o->match) {
					if (o->match->contents_regex) {
						struct options_match *m = (struct options_match *)joe_malloc(SIZEOF(struct options_match));
						*m = *o->match;
						m->next = o->match;
						o->match = m;
					}
					o->match->contents_regex = zdup(buf+1);
				}
			}
			break;
		case '-':	/* Set an option */
			{ /* parse option and arg.  arg goes to end of line.  This is bad. */
				char *opt = buf + 1;
				int x;
				char *arg = NULL;

				for (x = 0; buf[x] && buf[x] != '\n' && buf[x] != ' ' && buf[x] != '\t'; ++x) ;
				if (buf[x] && buf[x] != '\n') {
					buf[x] = 0;
					for (arg = buf + ++x; buf[x] && buf[x] != '\n'; ++x) ;
				}
				buf[x] = 0;
				if (!glopt(opt, arg, o, 2)) {
					err = 1;
					logerror_3(joe_gettext(_("%s %d: Unknown option %s\n")), name, line, opt);
				}
			}
			break;
		case '{':	/* Process help text.  No comment allowed after {name */
			{	/* everything after } is ignored. */
				line = help_init(fd,buf,line);
			}
			break;
		case ':':	/* Select context */
			{
				ptrdiff_t x, c;
				char ch;

				for (x = 1; !joe_isspace_eos(locale_map,buf[x]); ++x) ;
				ch = buf[x];
				buf[x] = 0;
				if (x != 1)
					if (!zcmp(buf + 1, "def")) {
						ptrdiff_t y;

						for (buf[x] = ch; joe_isblank(locale_map,buf[x]); ++x) ;
						for (y = x; !joe_isspace_eos(locale_map,buf[y]); ++y) ;
						ch = buf[y];
						buf[y] = 0;
						zlcpy(buf1, SIZEOF(buf1), buf + x);
						if (y != x) {
							ptrdiff_t sta = y + 1;
							MACRO *m;

							if (joe_isblank(locale_map,ch)
							    && (m = multiparse(fd, &line, buf, &sta, &err, name)))
								addcmd(buf1, m);
							else {
								err = 1;
								logerror_2(joe_gettext(_("%s %d: macro missing from :def\n")), name, line);
							}
						} else {
							err = 1;
							logerror_2(joe_gettext(_("%s %d: command name missing from :def\n")), name, line);
						}
					} else if (!zcmp(buf + 1, "inherit")) {
						if (context) {
							for (buf[x] = ch; joe_isblank(locale_map,buf[x]); ++x) ;
							for (c = x; !joe_isspace_eos(locale_map,buf[c]); ++c) ;
							buf[c] = 0;
							if (c != x)
								kcpy(context, kmap_getcontext(buf + x));
							else {
								err = 1;
								logerror_2(joe_gettext(_("%s %d: context name missing from :inherit\n")), name, line);
							}
						} else {
							err = 1;
							logerror_2(joe_gettext(_("%s %d: No context selected for :inherit\n")), name, line);
						}
					} else if (!zcmp(buf + 1, "include")) {
						for (buf[x] = ch; joe_isblank(locale_map,buf[x]); ++x) ;
						for (c = x; !joe_isspace_eos(locale_map,buf[c]); ++c) ;
						buf[c] = 0;
						if (c != x) {
							char bf[1024];
							char *p = getenv("HOME");
							int rtn = -1;
							bf[0] = 0;
							if (p && buf[x] != '/') {
							joe_snprintf_2(bf,SIZEOF(bf),"%s/.config/joe/%s",p,buf + x);
							rtn = procrc(cap, bf);
							}
							if (rtn == -1 && buf[x] != '/') {
								joe_snprintf_2(bf,SIZEOF(bf),"%s/.joe/%s",p,buf + x);
								rtn = procrc(cap, bf);
							}
							if (rtn == -1 && buf[x] != '/') {
								joe_snprintf_2(bf,SIZEOF(bf),"%s%s",JOERC,buf + x);
								rtn = procrc(cap, bf);
							}
							if (rtn == -1 && buf[x] != '/') {
								joe_snprintf_1(bf,SIZEOF(bf),"*%s",buf + x);
								rtn = procrc(cap, bf);
							}
							if (rtn == -1 && buf[x] == '/') {
								joe_snprintf_1(bf,SIZEOF(bf),"%s",buf + x);
								rtn = procrc(cap, bf);
							}
							switch (rtn) {
							case 1:
								err = 1;
								break;
							case -1:
								logerror_3(joe_gettext(_("%s %d: Couldn't open %s\n")), name, line, bf);
								err = 1;
								break;
							}
							context = 0;
							o = &fdefault;
						} else {
							err = 1;
							logerror_2(joe_gettext(_("%s %d: :include missing file name\n")), name, line);
						}
					} else if (!zcmp(buf + 1, "delete")) {
						if (context) {
							ptrdiff_t y;

							for (buf[x] = ch; joe_isblank(locale_map,buf[x]); ++x) ;
							for (y = x; buf[y] != 0 && buf[y] != '\t' && buf[y] != '\n' && (buf[y] != ' ' || buf[y + 1]
															!= ' '); ++y) ;
							buf[y] = 0;
							kdel(context, buf + x);
						} else {
							err = 1;
							logerror_2(joe_gettext(_("%s %d: No context selected for :delete\n")), name, line);
						}
					} else if (!zcmp(buf + 1, "defmap")) {
						for (buf[x] = ch; joe_isblank(locale_map,buf[x]); ++x) ;
						for (c = x; !joe_isspace_eos(locale_map,buf[c]); ++c) ;
						buf[c] = 0;
						if (c != x) {
							context = kmap_getcontext(buf + x);
							current_menu = 0;
						} else {
							err = 1;
							logerror_2(joe_gettext(_("%s %d: :defmap missing name\n")), name, line);
						}
					} else if (!zcmp(buf + 1, "defmenu")) {
						MACRO *m = 0;
						char d;
						ptrdiff_t y;
						for (buf[x] = ch; joe_isblank(locale_map,buf[x]); ++x) ;
						for (c = x; !joe_isspace_eos(locale_map,buf[c]); ++c) ;
						d = buf[c];
						buf[c] = 0;
						zlcpy(buf1, SIZEOF(buf1), buf + x);
						buf[c] = d;
						for (y = c; joe_isblank(locale_map, buf[y]); ++y);
						if (!joe_isspace_eos(locale_map, buf[y]))
							m = multiparse(fd, &line, buf, &y, &err, name);
						current_menu = create_menu(buf1, m);
						context = 0;
					} else {
						context = kmap_getcontext(buf + 1);
						current_menu = 0;
						/* err = 1;
						logerror_2(joe_gettext(_("%s %d: unknown :command\n")), name, line);*/
					}
				else {
					err = 1;
					logerror_2(joe_gettext(_("%s %d: Invalid context name\n")), name, line);
				}
			}
			break;
		default:	/* Get key-sequence to macro binding */
			{
				ptrdiff_t x, y;
				MACRO *m;

				if (!context && !current_menu) {
					err = 1;
					logerror_2(joe_gettext(_("%s %d: No context selected for macro to key-sequence binding\n")), name, line);
					break;
				}

				x = 0;
				m = multiparse(fd, &line, buf, &x, &err, name);
				if (x == -1)
					break;
				if (!m)
					break;

				/* Skip to end of key sequence */
				for (y = x; buf[y] != 0 && buf[y] != '\t' && buf[y] != '\n' && (buf[y] != ' ' || buf[y + 1] != ' '); ++y) ;
				buf[y] = 0;

				if (current_menu) {
					/* Add menu entry */
					add_menu_entry(current_menu, buf + x, m);
				} else {
					/* Add binding to context */
					if (kadd(cap, context, buf + x, m) == -1) {
						logerror_3(joe_gettext(_("%s %d: Bad key sequence '%s'\n")), name, line, buf + x);
						err = 1;
					}
				}
			}
			break;
		}
	}
	jfclose(fd);		/* Close rc file */

	/* Print proper ending string */
	logmessage_1(joe_gettext(_("Finished processing %s\n")), name);

	return err;		/* 0 for success, 1 for syntax error */
}

/* Check to see if a joe resource file exists. If it does, return its path.
   If not, return NULL. */
static char *try_get_joerc_file_path(const char *env_var, const char *rest_path, const char *file) {
    char *var_path = env_var ? getenv(env_var) : "";
    char *result;
    const char *real_file = file ? file : "";
    size_t result_len;
    struct stat sb;
    
    if (!var_path || !rest_path)
        return NULL;

    result_len = zlen(var_path) + zlen(rest_path) + zlen(real_file) + 1;
    result = (char*)joe_malloc(result_len);
    joe_snprintf_3(result, result_len, "%s%s%s", var_path, rest_path, real_file);
    
    if (stat(result, &sb)) {
        joe_free(result);
        return NULL;
    }

    return result;
}

/* Get a file path in the JOERC directory. If file is NULL get the directory 
   itself. First checks $XDG_CONFIG_HOME/joe, then ~/.config/joe, and 
   finally ~/.joe. If no user JOERC dir exists, it check JOEDATA.
   Returns NULL on failure. 
   Returns a dynamically allocated string (path to the file) on success.
*/
char *get_joerc_file_path(const char *file) {
    char *result = try_get_joerc_file_path("XDG_CONF_HOME", "/joe/", file);
    if (!result)
        result = try_get_joerc_file_path("HOME", "/.config/joe/", file);
    if (!result)
        result = try_get_joerc_file_path("HOME", "/.joe/", file);
    if (!result)
        result = try_get_joerc_file_path(NULL, JOEDATA, file);

    return result;
}

/* Get joe's main resource file. 
   Run is argv[0].
   Returns a dynmaically allocated path to the joerc. 
   Returns NULL on error.
*/
char *get_joerc_path(const char *run) {
    if (!run)
        return NULL;
    
    size_t runrc_len = zlen(run) + sizeof("rc") + 2; /* 2 for NUL and possible prefix dot. */
    char *runrc = (char*)joe_malloc(runrc_len);

    joe_snprintf_2(runrc, runrc_len, "%s%s", run, "rc");
    
    char *result = try_get_joerc_file_path("XDG_CONF_HOME", "/joe/", runrc);
    if (!result)
        result = try_get_joerc_file_path("HOME", "/.config/joe/", runrc);
    if (!result) {
        joe_snprintf_2(runrc, runrc_len, ".%s%s", run, "rc");
        result = try_get_joerc_file_path("HOME", "/", runrc);
    }

    joe_free(runrc);
    return result;
}

/* Load a cache file. */
static FILE *load_cache_file(const char *env_prefix, const char *res_dir_path, const char *file_name, const char *mode) {
    char *cachedir = getenv(env_prefix);
    FILE *result;
    mode_t old_mask;
    size_t i;
    size_t mode_len;
    char *full_path = NULL;
    size_t full_path_len;
    
    if (!cachedir || !mode)
        return NULL;

    full_path_len = zlen(cachedir) + zlen(res_dir_path) + zlen(file_name) + 1;
    full_path = (char*)joe_malloc(full_path_len);
    joe_snprintf_3(full_path, full_path_len, "%s%s%s", cachedir, res_dir_path, file_name);

    /* Determine fopen options, if we write we umask. */
    mode_len = zlen(mode);
    for(i = 0; i < mode_len; i++) {
        if(mode[i] == 'w' || mode[i] == '+' || mode[i] == 'a') {
            old_mask = umask(0066);
            result = fopen(full_path, "w");
            umask(old_mask);
            joe_free(full_path);
            return result;
        }
    }
    
    result = fopen(full_path, "r");
    joe_free(full_path);
    return result;
}

/* Get cache file of name NAME. Starts by checking XDG_CACHE_HOME, if no 
   dedicated cache dir is found, it will place the file into the joerc
   directory. If there is no JOERC directory, it will place the file
   as a dotfile in the user's home. 
   Returns a valid FILE* on success, returns NULL on error.
*/
FILE *get_cache_file(const char *name, const char *mode) {
    if (!name)
        return NULL;

    char *dotfile_name;
    size_t dotfile_len;

    FILE *result = load_cache_file("XDG_CACHE_HOME", "/joe/", name, mode);
    if (!result)
        result = load_cache_file("HOME", "/.cache/joe/", name, mode);
    if (!result)
        result = load_cache_file("XDG_CONF_HOME", "/joe/", name, mode);
    if (!result)
        result = load_cache_file("HOME", "/.config/joe/", name, mode);
    if (!result) {
        dotfile_len = zlen(name) + 2;
        dotfile_name = (char*)joe_malloc(dotfile_len);
        joe_snprintf_1(dotfile_name, dotfile_len, ".%s", name);
        result = load_cache_file("HOME", "/", dotfile_name, mode);
        joe_free(dotfile_name);
    }

    return result;
}

