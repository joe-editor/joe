/*
 *	File selection menu
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "b.h"
#include "blocks.h"
#include "menu.h"
#include "path.h"
#include "tty.h"
#include "utils.h"
#include "va.h"
#include "w.h"

typedef struct tab TAB;

extern int smode;		/* ??? */
extern int beep;
int menu_explorer = 0;		/* Stay in menu system when directory selected */

struct tab {
	int first_len;			/* Original size of path */
	int ofst;			/* Starting offset to path */
	unsigned char *path;		/* current directory */
	unsigned char *pattern;		/* search pattern */
	int len;		/* no. entries in files */
	unsigned char **files;		/* array of file names */
	unsigned char **list;
	unsigned char *type;		/* file type array */
	int prv;
	unsigned char *orgpath;
	unsigned char *orgnam;
};

#define F_DIR		1	/* type codes for file type array */
#define F_NORMAL	2
#define F_EXEC		4

/* Read matching files from a directory
 *  Directory is given in tab.path
 *  Pattern is given in tab.pattern
 *
 * Returns with -1 if there was an error
 * Otherwise returns index to file with inode given in prv
 * len and files are set with the file names
 * type is set with the file types
 */

static int get_entries(TAB *tab, int prv)
{
	int a;
	int which = 0;
	unsigned char *oldpwd = pwd();
	unsigned char **files;

	if (chpwd(tab->path))
		return -1;
	files = rexpnd(tab->pattern);
	if (!files) {
		chpwd(oldpwd);
		return -1;
	}
	if (!aLEN(files)) {
		chpwd(oldpwd);
		return -1;
	}
	tab->len = aLEN(files);
	varm(tab->files);
	tab->files = files;
	vasort(files, tab->len);
	if (tab->type)
		joe_free(tab->type);
	tab->type = (unsigned char *) joe_malloc(tab->len);
	for (a = 0; a != tab->len; a++) {
		struct stat buf;
		mset(&buf, 0, sizeof(struct stat));

		stat((char *)(files[a]), &buf);
		if (buf.st_ino == prv)
			which = a;
		if ((buf.st_mode & S_IFMT) == S_IFDIR)
			tab->type[a] = F_DIR;
		else if (buf.st_mode & (0100 | 0010 | 0001))
			tab->type[a] = F_EXEC;
		else
			tab->type[a] = F_NORMAL;
	}
	chpwd(oldpwd);
	return which;
}

static void insnam(BW *bw, unsigned char *path, unsigned char *nam, int dir, int ofst)
{
	P *p = pdup(bw->cursor);

	pgoto(p, ofst);
	p_goto_eol(bw->cursor);
	bdel(p, bw->cursor);
	if (sLEN(path)) {
		binsm(bw->cursor, sv(path));
		p_goto_eol(bw->cursor);
		if (path[sLEN(path) - 1] != '/') {
			binsm(bw->cursor, sc("/"));
			p_goto_eol(bw->cursor);
		}
	}
	binsm(bw->cursor, sv(nam));
	p_goto_eol(bw->cursor);
	if (dir) {
		binsm(bw->cursor, sc("/"));
		p_goto_eol(bw->cursor);
	}
	prm(p);
	bw->cursor->xcol = piscol(bw->cursor);
}

/* Given a menu structure with a tab structure as its object,
 * a pattern and path set in the tab structure:
 *
 * Load the menu with a list of file names and set the file name in
 * the prompt window to the directory the menu was read in from.
 * If flg is set, treload attempts to position to the previous directory
 * which was visited.
 *
 * Returns with -1 if there was an error
 * Returns with 0 for success
 */

static unsigned char **treload(TAB *tab,MENU *m, BW *bw, int flg,int *defer)
{
	int x;
	int which;
	struct stat buf;

	if ((which = get_entries(tab, tab->prv)) < 0)
		return 0;
	if (tab->path && tab->path[0])
		stat((char *)tab->path, &buf);
	else
		stat(".", &buf);
	tab->prv = buf.st_ino;
	if (!flg)
		which = 0;

	tab->list = vatrunc(tab->list, aLEN(tab->files));

	for (x = 0; tab->files[x]; ++x) {
		unsigned char *s = vsncpy(NULL, 0, sv(tab->files[x]));

		tab->list = vaset(tab->list, x, s);
		if (tab->type[x] == F_DIR)
			tab->list[x] = vsadd(tab->list[x], '/');
		else if (tab->type[x] == F_EXEC)
			tab->list[x] = vsadd(tab->list[x], '*');
	}
	if (defer) {
		*defer = which;
		insnam(bw, tab->path, tab->pattern, 0, tab->ofst);
		return tab->list;
	} else {
		ldmenu(m, tab->list, which);
		insnam(bw, tab->path, tab->pattern, 0, tab->ofst);
		return tab->list;
	}
}

static void rmtab(TAB *tab)
{
	vsrm(tab->orgpath);
	vsrm(tab->orgnam);
	varm(tab->list);
	vsrm(tab->path);
	vsrm(tab->pattern);
	varm(tab->files);
	if (tab->type)
		joe_free(tab->type);
	joe_free(tab);
}
/*****************************************************************************/
/****************** The user hit return **************************************/
/*****************************************************************************/
static int tabrtn(MENU *m, int cursor, TAB *tab)
{
	if (menu_explorer && tab->type[cursor] == F_DIR) {	/* Switch directories */
		unsigned char *orgpath = tab->path;
		unsigned char *orgpattern = tab->pattern;
		unsigned char *e = endprt(tab->path);

		/* if (!strcmp(tab->files[cursor], "..") && sLEN(e)
		    && !(e[0] == '.' && e[1] == '.' && (!e[2] || e[2] == '/')))
			tab->path = begprt(tab->path);
		else */ {
			tab->path = vsncpy(NULL, 0, sv(tab->path));
			tab->path = vsncpy(sv(tab->path), sv(m->list[cursor]));
		}
		vsrm(e);
		tab->pattern = vsncpy(NULL, 0, sc("*"));
		if (!treload(m->object, m, m->parent->win->object, 0, NULL)) {
			msgnw(m->parent, US "Couldn't read directory ");
			vsrm(tab->pattern);
			tab->pattern = orgpattern;
			vsrm(tab->path);
			tab->path = orgpath;
			return -1;
		} else {
			vsrm(orgpattern);
			vsrm(orgpath);
			return 0;
		}
	} else {		/* Select name */
		BW *bw = m->parent->win->object;

		insnam(bw, tab->path, tab->files[cursor], (tab->type[cursor]==F_DIR), tab->ofst);
		rmtab(tab);
		m->object = NULL;
		m->abrt = NULL;
		wabort(m->parent);
		return 0;
	}
}

/* Like above, but treats directories as files (adds them to path instead of
 * traverse hierarchy) */

static int tabrtn1(MENU *m, int cursor, TAB *tab)
{
	/* New way: just add directory to path */
	BW *bw = m->parent->win->object;

	insnam(bw, tab->path, tab->files[cursor], (tab->type[cursor]==F_DIR ? 1 : 0), tab->ofst);
	rmtab(tab);
	m->object = NULL;
	m->abrt = NULL;
	wabort(m->parent);
	return 0;
}


/*****************************************************************************/
/****************** The user hit backspace ***********************************/
/*****************************************************************************/
static int tabbacks(MENU *m, int cursor, TAB *tab)
{
	unsigned char *orgpath = tab->path;
	unsigned char *orgpattern = tab->pattern;
	unsigned char *e = endprt(tab->path);

	if (sLEN(e) && sLEN(tab->path)!=tab->first_len)
		tab->path = begprt(tab->path);
	else {
		wabort(m->parent);
		return 0;
	}
	vsrm(e);
	tab->pattern = vsncpy(NULL, 0, sc("*"));

	if (!treload(m->object, m, m->parent->win->object, 1, NULL)) {
		msgnw(m->parent, US "Couldn't read directory ");
		vsrm(tab->pattern);
		tab->pattern = orgpattern;
		vsrm(tab->path);
		tab->path = orgpath;
		return -1;
	} else {
		vsrm(orgpattern);
		vsrm(orgpath);
		return 0;
	}
}
/*****************************************************************************/
static int tababrt(BW *bw, int cursor, TAB *tab)
{
	insnam(bw, tab->orgpath, tab->orgnam, 0, tab->ofst);
	rmtab(tab);
	return -1;
}

P *p_goto_start_of_path(P *p)
{
	int c;
	do
		c = prgetc(p);
	while (c!=NO_MORE_DATA && c!=' ' && c!='\n');

	if (c!=NO_MORE_DATA)
		pgetc(p);

	return p;
}

/*****************************************************************************/
/****************** Create a tab window **************************************/
/*****************************************************************************/
int cmplt(BW *bw)
{
	MENU *new;
	TAB *tab;
	P *p, *q;
	unsigned char *cline, *tmp;
	long a, b;
	int which;
	unsigned char **l;
	int ofst;

	tab = (TAB *) joe_malloc(sizeof(TAB));
	tab->files = NULL;
	tab->type = NULL;
	tab->list = NULL;
	tab->prv = 0;
	tab->len = 0;

	q = pdup(bw->cursor);
	p_goto_eol(q);
	p = pdup(q);
	p_goto_start_of_path(p);
	ofst = p->byte;

	tmp = brvs(p, (int) (q->byte - p->byte));
	cline = parsens(tmp, &a, &b);
	vsrm(tmp);
	prm(p);
	prm(q);

	tab->ofst = ofst;
	tab->pattern = namprt(cline);
	tab->path = dirprt(cline);
	tab->first_len = sLEN(tab->path);
	tab->orgnam = vsncpy(NULL, 0, sv(tab->pattern));
	tab->orgpath = vsncpy(NULL, 0, sv(tab->path));
	tab->pattern = vsadd(tab->pattern, '*');
	vsrm(cline);

	l = treload(tab, 0, bw, 0, &which);

	if (l && (new = mkmenu(bw->parent, l, tabrtn, tababrt, tabbacks, which, tab, NULL))) {
		if (sLEN(tab->files) == 1)
			return tabrtn1(new, 0, tab);
		else if (smode || isreg(tab->orgnam))
			return 0;
		else {
			unsigned char *com = mcomplete(new);

			vsrm(tab->orgnam);
			tab->orgnam = com;
			wabort(new->parent);
			smode = 2;
			/* if(beep) */
				ttputc(7);
			return 0;
		}
	} else {
		/* if(beep) */
			ttputc(7);
		rmtab(tab);
		return -1;
	}
}
