/*
 * 	User file operations
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <stdio.h>
#include <unistd.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef UTIME
#include <utime.h>
#define HAVEUTIME 1
#else
#ifdef SYSUTIME
#include <sys/utime.h>
#define HAVEUTIME 1
#endif
#endif

#include "b.h"
#include "bw.h"
#include "macro.h"
#include "main.h"
#include "menu.h"
#include "path.h"
#include "pw.h"
#include "qw.h"
#include "scrn.h"
#include "tab.h"
#include "tty.h"
#include "tw.h"
#include "ublock.h"
#include "uerror.h"
#include "ufile.h"
#include "ushell.h"
#include "utils.h"
#include "va.h"
#include "vs.h"
#include "w.h"

extern int orphan;
char *backpath = 0;		/* Place to store backup files */
static B *filehist = 0;		/* History of file names */
int nobackups = 0;
int exask = 0;

/* Ending message generator */
/**** message which is shown after closing joe (CTRL+x; CTRL+k) *****/
void genexmsg(BW *bw, int saved, char *name)
{
	char *s;

	if (bw->b->name && bw->b->name[0]) {
		s = bw->b->name;
	} else {
		s = "(Unnamed)";
	}

	if (name) {
		if (saved) {
			snprintf(msgbuf, JOE_MSGBUFSIZE, "File %s saved", name);
		} else {
			snprintf(msgbuf, JOE_MSGBUFSIZE, "File %s not saved", name);
		}
	} else if (bw->b->changed && bw->b->count == 1) {
		snprintf(msgbuf, JOE_MSGBUFSIZE, "File %s not saved", s);
	} else if (saved) {
		snprintf(msgbuf, JOE_MSGBUFSIZE, "File %s saved", s);
	} else {
		snprintf(msgbuf, JOE_MSGBUFSIZE, "File %s not changed so no update needed", s);
	}
	msgnw(bw->parent, msgbuf);

	if (!exmsg) {
		if (bw->b->changed && bw->b->count == 1) {
			exmsg = vsncpy(NULL, 0, sc("File "));
			exmsg = vsncpy(sv(exmsg), sz(s));
			exmsg = vsncpy(sv(exmsg), sc(" not saved."));
		} else if (saved) {
			exmsg = vsncpy(NULL, 0, sc("File "));
			exmsg = vsncpy(sv(exmsg), sz(s));
			exmsg = vsncpy(sv(exmsg), sc(" saved."));
		} else {
			exmsg = vsncpy(NULL, 0, sc("File "));
			exmsg = vsncpy(sv(exmsg), sz(s));
			exmsg = vsncpy(sv(exmsg), sc(" not changed so no update needed."));
		}
	}
}

/* Write highlighted block to a file */

int ublksave(BW *bw)
{
	if (markb && markk && markb->b == markk->b && (markk->byte - markb->byte) > 0 && (!square || piscol(markk) > piscol(markb))) {
		if (wmkpw(bw->parent, "Name of file to write (^C to abort): ", &filehist, dowrite, "Names", NULL, cmplt, NULL, NULL)) {
			return 0;
		} else {
			return -1;
		}
	} else {
		return usave(bw);
	}
}

/* Shell escape */

int ushell(BW *bw)
{
	nescape(bw->parent->t->t);
	ttsusp();
	nreturn(bw->parent->t->t);
	return 0;
}

/* Copy a file */

static int cp(char *from, char *to)
{
	int f, g, amnt;
	struct stat sbuf;

#ifdef HAVEUTIME
#ifdef NeXT
	time_t utbuf[2];
#else
	struct utimbuf utbuf;
#endif
#endif

	f = open(from, O_RDONLY);
	if (f < 0) {
		return -1;
	}
	if (fstat(f, &sbuf) < 0) {
		return -1;
	}
	g = creat(to, sbuf.st_mode);
	if (g < 0) {
		close(f);
		return -1;
	}
	while ((amnt = read(f, stdbuf, stdsiz)) > 0) {
		if (amnt != joe_write(g, stdbuf, amnt)) {
			break;
		}
	}
	close(f);
	close(g);
	if (amnt) {
		return -1;
	}

#ifdef HAVEUTIME
#ifdef NeXT
	utbuf[0] = (time_t) sbuf.st_atime;
	utbuf[1] = (time_t) sbuf.st_mtime;
#else
	utbuf.actime = sbuf.st_atime;
	utbuf.modtime = sbuf.st_mtime;
#endif
	utime(to, &utbuf);
#endif

	return 0;
}

/* Make backup file if it needs to be made
 * Returns 0 if backup file was made or didn't need to be made
 * Returns 1 for error
 */

static int backup(BW *bw)
{
	if (!bw->b->backup && !nobackups && bw->b->name && bw->b->name[0]) {
		char tmp[1024];
		char name[1024];

#ifdef __MSDOS__
		int x;

		if (backpath) {
			snprintf(name, sizeof(name), "%s/%s", backpath, namepart(tmp, bw->b->name));
		} else {
			snprintf(name, sizeof(name), "%s", bw->b->name);
		}

		for (x = strlen(name); name[--x] != '.';) {
			if (name[x] == '\\' || (name[x] == ':' && x == 1) || x == 0) {
				x = strlen(name);
				break;
			}
		}

		strcpy(name + x, ".bak");

#else

		/* Create backup file name */
		char *simple_backup_suffix = getenv("SIMPLE_BACKUP_SUFFIX");
		
		if (simple_backup_suffix == NULL) {
			simple_backup_suffix = "~";
		}
		if (backpath) {
			snprintf(name, sizeof(name), "%s/%s%s", backpath, namepart(tmp, bw->b->name), simple_backup_suffix);
		} else {
			snprintf(name, sizeof(name), "%s%s", bw->b->name, simple_backup_suffix);
		}
		
		/* Attempt to delete backup file first */
		unlink(name);

#endif

		/* Copy original file to backup file */
		if (cp(bw->b->name, name)) {
			return 1;
		} else {
			bw->b->backup = 1;
			return 0;
		}
	} else {
		return 0;
	}
}

/* Write file */

struct savereq {
	int (*callback) ();
	char *name;
};

static int saver(BW *bw, int c, struct savereq *req, int *notify)
{
	int (*callback) ();
	int fl;

	callback = req->callback;
	if (c == 'n' || c == 'N') {
		vsrm(req->name);
		joe_free(req);
		if (notify) {
			*notify = 1;
		}
		msgnw(bw->parent, "Couldn't make backup file... file not saved");
		if (callback) {
			return callback(bw, -1);
		} else {
			return -1;
		}
	}
	if (c != 'y' && c != 'Y') {
		if (mkqw(bw->parent, sc("Could not make backup file.  Save anyway (y,n,^C)? "), saver, NULL, req, notify)) {
			return 0;
		} else {
			if (notify)
				*notify = 1;
			return -1;
		}
	}
	if (notify) {
		*notify = 1;
	}
	if (bw->b->er == -1 && bw->o.msnew) {
		exemac(bw->o.msnew);
		bw->b->er = -3;
	}
	if (bw->b->er == 0 && bw->o.msold) {
		exemac(bw->o.msold);
	}
	if ((fl = bsave(bw->b->bof, req->name, bw->b->eof->byte)) != 0) {
		msgnw(bw->parent, msgs[fl + 5]);
		vsrm(req->name);
		joe_free(req);
		if (callback) {
			return callback(bw, -1);
		} else {
			return -1;
		}
	} else {
		if (!bw->b->name)
			bw->b->name = joesep(strdup(req->name));
		if (!strcmp(bw->b->name, req->name)) {
			bw->b->changed = 0;
			saverr(bw->b->name);
		}
		{
			/* Last UNDOREC which wasn't modified will be changed
			 * to modified. And because this block is
			 * executed after each 'save', there can't be more
			 * than one record which is not modified
			 *		24 Apr 2001, Marx
			 */
			UNDO *u = bw->b->undo;
			UNDOREC *rec, *rec_start;

			rec = rec_start = &u->recs;

			do {
				rec = rec->link.prev;
			} while (rec != rec_start && rec->changed);
			if(rec->changed == 0)
				rec->changed = 1;

		}
		genexmsg(bw, 1, req->name);
		vsrm(req->name);
		joe_free(req);
		if (callback) {
			return callback(bw, 0);
		} else {
			return 0;
		}
	}
}

static int dosave(BW *bw, char *s, int (*callback) (), int *notify)
{
	struct savereq *req = (struct savereq *) joe_malloc(sizeof(struct savereq));

	req->name = s;
	req->callback = callback;
	if (backup(bw)) {
		return saver(bw, 0, req, notify);
	} else {
		return saver(bw, 'y', req, notify);
	}
}

static int dosave2(BW *bw, int c, char *s, int *notify)
{
	if (c == 'y' || c == 'Y') {
		return dosave(bw, s, NULL, notify);
	} else if (c == 'n' || c == 'N') {
		if (notify) {
			*notify = 1;
		}
		genexmsg(bw, 0, s);
		vsrm(s);
		return -1;
	} else if (mkqw(bw->parent, sc("File exists.  Overwrite (y,n,^C)? "), dosave2, NULL, s, notify)) {
		return 0;
	} else {
		return -1;
	}
}

static int dosave1(BW *bw, char *s, void *object, int *notify)
{
	int f;

	if (s[0] != '!' && !(s[0] == '>' && s[1] == '>') && (!bw->b->name || strcmp(s, bw->b->name))) {
		f = open(s, O_RDONLY);
		if (f != -1) {
			close(f);
			return dosave2(bw, 0, s, notify);
		}
	}
	return dosave(bw, s, object, notify);
}

int usave(BW *bw)
{
	BW *pbw;
	
	pbw = wmkpw(bw->parent, "Name of file to save (^C to abort): ", &filehist, dosave1, "Names", NULL, cmplt, NULL, NULL);

	if (pbw && bw->b->name) {
		binss(pbw->cursor, bw->b->name);
		pset(pbw->cursor, pbw->b->eof);
		pbw->cursor->xcol = piscol(pbw->cursor);
	}
	if (pbw) {
		return 0;
	} else {
		return -1;
	}
}

/* Load file to edit */

int doedit(BW *bw, char *s, void *obj, int *notify)
{
	int ret = 0;
	int er;
	void *object;
	W *w;
	B *b;

	if (notify) {
		*notify = 1;
	}
	if (bw->pid) {
		msgnw(bw->parent, "Process running in this window");
		return -1;
	}
	b = bfind(s);
	er = error;
	if (bw->b->count == 1 && (bw->b->changed || bw->b->name)) {
		if (orphan) {
			orphit(bw);
		} else {
			if (uduptw(bw)) {
				brm(b);
				return -1;
			}
			bw = (BW *) maint->curwin->object;
		}
	}
	if (er) {
		msgnwt(bw->parent, msgs[er + 5]);
		if (er != -1) {
			ret = -1;
		}
	}
	object = bw->object;
	w = bw->parent;
	bwrm(bw);
	w->object = (void *) (bw = bwmk(w, b, 0));
	wredraw(bw->parent);
	bw->object = object;
	vsrm(s);
	if (er == -1 && bw->o.mnew) {
		exemac(bw->o.mnew);
	}
	if (er == 0 && bw->o.mold) {
		exemac(bw->o.mold);
	}
	return ret;
}

int okrepl(BW *bw)
{
	if (bw->b->count == 1 && bw->b->changed) {
		msgnw(bw->parent, "Can't replace modified file");
		return -1;
	} else {
		return 0;
	}
}

int uedit(BW *bw)
{
	if (wmkpw(bw->parent, "Name of file to edit (^C to abort): ", &filehist, doedit, "Names", NULL, cmplt, NULL, NULL)) {
		return 0;
	} else {
		return -1;
	}
}

/* Load file into buffer: can result in an orphaned buffer */

static int dorepl(BW *bw, char *s, void *obj, int *notify)
{
	void *object = bw->object;
	int ret = 0;
	int er;
	W *w = bw->parent;
	B *b;

	if (notify) {
		*notify = 1;
	}
	if (bw->pid) {
		msgnw(bw->parent, "Process running in this window");
		return -1;
	}
	b = bfind(s);
	er = error;
	if (error) {
		msgnwt(bw->parent, msgs[error + 5]);
		if (error != -1) {
			ret = -1;
		}
	}
	if (bw->b->count == 1 && (bw->b->changed || bw->b->name)) {
		orphit(bw);
	}
	bwrm(bw);
	w->object = (void *) (bw = bwmk(w, b, 0));
	wredraw(bw->parent);
	bw->object = object;
	vsrm(s);
	if (er == -1 && bw->o.mnew) {
		exemac(bw->o.mnew);
	}
	if (er == 0 && bw->o.mold) {
		exemac(bw->o.mold);
	}
	return ret;
}

/* Switch to next buffer in window */

int unbuf(BW *bw)
{
	void *object = bw->object;
	W *w = bw->parent;
	B *b;

	if (bw->pid) {
		msgnw(bw->parent, "Process running in this window");
		return -1;
	}
	b = bnext();
	if (b == bw->b) {
		b = bnext();
	}
	if (b == bw->b) {
		return -1;
	}
	if (!b->orphan) {
		++b->count;
	} else {
		b->orphan = 0;
	}
	if (bw->b->count == 1) {
		orphit(bw);
	}
	bwrm(bw);
	w->object = (void *) (bw = bwmk(w, b, 0));
	wredraw(bw->parent);
	bw->object = object;
	return 0;
}

int upbuf(BW *bw)
{
	void *object = bw->object;
	W *w = bw->parent;
	B *b;

	if (bw->pid) {
		msgnw(bw->parent, "Process running in this window");
		return -1;
	}
	b = bprev();
	if (b == bw->b) {
		b = bprev();
	}
	if (b == bw->b) {
		return -1;
	}
	if (!b->orphan) {
		++b->count;
	} else {
		b->orphan = 0;
	}
	if (bw->b->count == 1) {
		orphit(bw);
	}
	bwrm(bw);
	w->object = (void *) (bw = bwmk(w, b, 0));
	wredraw(bw->parent);
	bw->object = object;
	return 0;
}

int uinsf(BW *bw)
{
	if (wmkpw(bw->parent, "Name of file to insert (^C to abort): ", &filehist, doinsf, "Names", NULL, cmplt, NULL, NULL)) {
		return 0;
	} else {
		return -1;
	}
}

/* Save and exit */

static int exdone(BW *bw, int flg)
{
	if (flg) {
		if (bw->b->name)
			joe_free(bw->b->name);
		bw->b->name = 0;
		return -1;
	} else {
		bw->b->changed = 0;
		saverr(bw->b->name);
		return uabort(bw, MAXINT);
	}
}

static int exdone1(BW *bw, int flg)
{
	if (flg) {
		return -1;
	} else {
		bw->b->changed = 0;
		saverr(bw->b->name);
		return uabort(bw, MAXINT);
	}
}

static int doex(BW *bw, char *s, void *object,int *notify)
{
	bw->b->name = joesep(strdup(s));
	return dosave(bw, s, exdone, notify);
}

int uexsve(BW *bw)
{
	if (!bw->b->changed) {
		uabort(bw, MAXINT);
		return 0;
	} else if (bw->b->name && !exask) {
		return dosave(bw, vsncpy(NULL, 0, sz(bw->b->name)), exdone1, NULL);
	} else {
		BW *pbw = wmkpw(bw->parent, "Name of file to save (^C to abort): ", &filehist, doex, "Names", NULL, cmplt, NULL, NULL);

		if (pbw && bw->b->name) {
			binss(pbw->cursor, bw->b->name);
			pset(pbw->cursor, pbw->b->eof);
			pbw->cursor->xcol = piscol(pbw->cursor);
		}
		if (pbw) {
			return 0;
		} else {
			return -1;
		}
	}
}

/* If buffer is modified, prompt for saving */

static int nask(BW *bw, int c, void *object, int *notify)
{
	if (c == 'y' || c == 'Y') {
		if (bw->b->name) {
			return dosave1(bw, vsncpy(NULL, 0, sz(bw->b->name)), object, notify);
		} else {
			BW *pbw = wmkpw(bw->parent, "Name of file to save (^C to abort): ", &filehist, dosave1, "Names", NULL, cmplt, object, notify);

			if (pbw) {
				return 0;
			} else {
				return -1;
			}
		}
	} else if (c == 'n' || c == 'N') {
		genexmsg(bw, 0, NULL);
		if (notify) {
			*notify = 1;
		}
		return 0;
	} else if (bw->b->count == 1 && bw->b->changed) {
		if (mkqw(bw->parent, sc("Save changes to this file (y,n,^C)? "), nask, NULL, object, notify)) {
			return 0;
		} else {
			return -1;
		}
	} else {
		if (notify) {
			*notify = 1;
		}
		return 0;
	}
}

int uask(BW *bw)
{
	return nask(bw, 0, NULL, NULL);
}

/* FIXME: unused ???? */
#if 0
/* Ask to save file if it is modified.  If user answers yes, run save */

static int nask2(BW *bw, int c, void *object, int *notify)
{
	if (c == 'y' || c == 'Y') {
		BW *pbw = wmkpw(bw->parent, "Name of file to save (^C to abort): ", &filehist, dosave1, "Names", NULL, cmplt, object, notify);

		if (pbw) {
			return 0;
		} else {
			return -1;
		}
	} else if (c == 'n' || c == 'N') {
		genexmsg(bw, 0, NULL);
		if (notify) {
			*notify = 1;
		}
		return 0;
	} else if (bw->b->count == 1 && bw->b->changed) {
		if (mkqw(bw->parent, sc("Save changes to this file (y,n,^C)? "), nask, NULL, object, notify)) {
			return 0;
		} else {
			return -1;
		}
	} else {
		if (notify) {
			*notify = 1;
		}
		return 0;
	}
}

static int uask2(BW *bw)
{
	return nask2(bw, 0, NULL, NULL);
}
#endif

/* If buffer is modified, ask if it's ok to lose changes */

static int dolose(BW *bw, int c, void *object, int *notify)
{
	W *w;

	if (notify) {
		*notify = 1;
	}
	if (c != 'y' && c != 'Y') {
		return -1;
	}
	genexmsg(bw, 0, NULL);
	if (bw->b->count == 1) {
		bw->b->changed = 0;
	}
	object = bw->object;
	w = bw->parent;
	bwrm(bw);
	w->object = (void *) (bw = bwmk(w, bfind(""), 0));
	wredraw(bw->parent);
	bw->object = object;
	if (bw->o.mnew) {
		exemac(bw->o.mnew);
	}
	return 0;
}

int ulose(BW *bw)
{
	msgnw(bw->parent, NULL);
	if (bw->pid) {
		return ukillpid(bw);
	}
	if (bw->b->count == 1 && bw->b->changed) {
		if (mkqw(bw->parent, sc("Lose changes to this file (y,n,^C)? "), dolose, NULL, NULL, NULL)) {
			return 0;
		} else {
			return -1;
		}
	} else {
		return dolose(bw, 'y', NULL, NULL);
	}
}

/* Buffer list */

static int dobuf(MENU *m, int x, char **s)
{
	char *name;
	BW *bw = m->parent->win->object;
	int *notify = m->parent->notify;

	m->parent->notify = 0;
	name = vsdup(s[x]);
	uabort(m, MAXINT);
	return dorepl(bw, name, NULL, notify);
}

static int abrtb(MENU *m, int x, char **s)
{
	varm(s);
	return -1;
}

int ubufed(BW *bw)
{
	char **s = getbufs();

	vasort(av(s));
	if (mkmenu(bw->parent, s, dobuf, abrtb, NULL, 0, s, NULL))
		return 0;
	else {
		varm(s);
		return -1;
	}
}
