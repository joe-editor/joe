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

#ifdef WITH_SELINUX
int copy_security_context(const char *from_file, const char *to_file);
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
#include "utf8.h"
#include "charmap.h"
#include "w.h"

extern int orphan;
unsigned char *backpath = NULL;		/* Place to store backup files */
B *filehist = NULL;	/* History of file names */
int nobackups = 0;
int exask = 0;

/* Ending message generator */
/**** message which is shown after closing joe (CTRL+x; CTRL+k) *****/
void genexmsg(BW *bw, int saved, unsigned char *name)
{
	unsigned char *s;

	if (bw->b->name && bw->b->name[0]) {
		s = bw->b->name;
	} else {
		s = US "(Unnamed)";
	}

	if (name) {
		if (saved) {
			joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "File %s saved", name);
		} else {
			joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "File %s not saved", name);
		}
	} else if (bw->b->changed && bw->b->count == 1) {
		joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "File %s not saved", s);
	} else if (saved) {
		joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "File %s saved", s);
	} else {
		joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "File %s not changed so no update needed", s);
	}
	msgnw(bw->parent, msgbuf);

	if (exmsg)
		vsrm(exmsg);

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

/* For ^X ^C */
void genexmsgmulti(BW *bw, int saved, int skipped)
{
	if (saved)
		if (skipped)
			joe_snprintf_0((char *)msgbuf, JOE_MSGBUFSIZE, "Some files have not been saved.");
		else
			joe_snprintf_0((char *)msgbuf, JOE_MSGBUFSIZE, "All modified files have been saved.");
	else
		joe_snprintf_0((char *)msgbuf, JOE_MSGBUFSIZE, "No modified files, so no updates needed.");

	msgnw(bw->parent, msgbuf);

	exmsg = vsncpy(NULL,0,sz(msgbuf));
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

static int cp(unsigned char *from, unsigned char *to)
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

	f = open((char *)from, O_RDONLY);
	if (f < 0) {
		return -1;
	}
	if (fstat(f, &sbuf) < 0) {
		return -1;
	}
	g = creat((char *)to, sbuf.st_mode & ~(S_ISUID | S_ISGID));
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

#ifdef WITH_SELINUX
	copy_security_context(from,to);
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
		unsigned char tmp[1024];
		unsigned char name[1024];

#ifdef __MSDOS__
		int x;

		if (backpath) {
			joe_snprintf_2(name, sizeof(name), "%s/%s", backpath, namepart(tmp, bw->b->name));
		} else {
			joe_snprintf_1(name, sizeof(name), "%s", bw->b->name);
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
		unsigned char *simple_backup_suffix = (unsigned char *)getenv("SIMPLE_BACKUP_SUFFIX");
		
		if (simple_backup_suffix == NULL) {
			simple_backup_suffix = US "~";
		}
		if (backpath) {
			joe_snprintf_3((char *)name, sizeof(name), "%s/%s%s", backpath, namepart(tmp, bw->b->name), simple_backup_suffix);
		} else {
			joe_snprintf_2((char *)name, sizeof(name), "%s%s", bw->b->name, simple_backup_suffix);
		}
		
		/* Attempt to delete backup file first */
		unlink((char *)name);

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

/* Continuation structure */

struct savereq {
	int (*callback) ();
	unsigned char *name;
	B *first;
	int not_saved;	/* Set if a modified file was not saved */
	int rename;	/* Set if we're renaming the file during save */
	int block_save; // Flag, if we want to save a block#
	char *message; // String for messages to be shown to the user
};

struct savereq *mksavereq(int (*callback)(), unsigned char *name, B *first,int rename, int block_save)
{
	struct savereq *req = (struct savereq *) joe_malloc(sizeof(struct savereq));
	req->callback = callback;
	req->name = name;
	req->first = first;
	req->not_saved = 0;
	req->rename = rename;
	req->block_save = block_save;
	return req;
}

static void rmsavereq(struct savereq *req)
{
	vsrm(req->name);
	joe_free(req);
}

static int saver(BW *bw, int c, struct savereq *req, int *notify)
{
	int fl;
	if (c == 'n' || c == 'N') {
		msgnw(bw->parent, US "Couldn't make backup file... file not saved");
		if (req->callback) {
			return req->callback(bw, req, -1, notify);
		} else {
			if (notify) {
				*notify = 1;
			}
			rmsavereq(req);
			return -1;
		}
	}
	if (c != 'y' && c != 'Y') {
		if (mkqw(bw->parent, sc("Could not make backup file.  Save anyway (y,n,^C)? "), saver, NULL, req, notify)) {
			return 0;
		} else {
			rmsavereq(req);
			if (notify)
				*notify = 1;
			return -1;
		}
	}
	if (bw->b->er == -1 && bw->o.msnew) {
		exemac(bw->o.msnew);
		bw->b->er = -3;
	}
	if (bw->b->er == 0 && bw->o.msold) {
		exemac(bw->o.msold);
	}
	if ((fl = bsave(bw->b->bof, req->name, bw->b->eof->byte, 1)) != 0) {
		msgnw(bw->parent, msgs[-fl]);
		if (req->callback) {
			return req->callback(bw, req, -1, notify);
		} else {
			rmsavereq(req);
			if (notify) {
				*notify = 1;
			}
			return -1;
		}
	} else {
		if (req->rename) {
			joe_free(bw->b->name);
			bw->b->name = 0;
		}
		if (!bw->b->name)
			bw->b->name = joesep(joe_strdup(req->name));
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
		if (req->callback) {
			return req->callback(bw, req, 0, notify);
		} else {
			rmsavereq(req);
			return 0;
		}
	}
}

static int dosave(BW *bw, struct savereq *req, int *notify)
{
	if (req->block_save)
	{
		if (notify)
			*notify = 1;
		if (markv(1)) {
			if (square) {
				int fl;
				int ret = 0;
				B *tmp = pextrect(markb,
						  markk->line - markb->line + 1,
						  markk->xcol);
						  
				if ((fl = bsave(tmp->bof, req->name, tmp->eof->byte, 0)) != 0) {
					msgnw(bw->parent, msgs[-fl]);
					ret = -1;
				}
				brm(tmp);
				if (!ret) {
					joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "Block written to file %s", req->name);
					msgnw(bw->parent, msgbuf);
				}
				if (lightoff)
					unmark(bw);
				vsrm(req->name);
				return ret;
			} else {
				int fl;
				int ret = 0;

				if ((fl = bsave(markb, req->name, markk->byte - markb->byte, 0)) != 0) {
					msgnw(bw->parent, msgs[-fl]);
					ret = -1;
				}
				if (!ret) {
					joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "Block written to file %s", req->name);
					msgnw(bw->parent, msgbuf);
				}
				if (lightoff)
					unmark(bw);
					vsrm(req->name);
				return ret;
			}
		} else {
			vsrm(req->name);
			msgnw(bw->parent, US "No block");
			return -1;
		}
	}
	else
	{	
		if (backup(bw)) {
			return saver(bw, 0, req, notify);
		} else {
			return saver(bw, 'y', req, notify);
		}
	}
}

static int dosave2(BW *bw, int c, struct savereq *req, int *notify)
{
	if (c == 'y' || c == 'Y') {
		return dosave(bw, req, notify);
	} else if (c == 'n' || c == 'N') {
		if (notify) {
			*notify = 1;
		}
		genexmsg(bw, 0, req->name);
		rmsavereq(req);
		return -1;
	} else if (mkqw(bw->parent, sz(req->message), dosave2, NULL, req, notify)) {
		return 0;
	} else {
		/* Should be in abort function */
		rmsavereq(req);
		return -1;
	}
}

/* Checks if file exists. */

static int dosave1(BW *bw, unsigned char *s, struct savereq *req, int *notify)
{
	int f;

	if (req->name)
		vsrm(req->name);
	req->name = s;

	if (s[0] != '!' && !(s[0] == '>' && s[1] == '>')) {
		/* It's a normal file: not a pipe or append */
		if (!bw->b->name || strcmp(s, bw->b->name)) {
			/* Newly named file or name is different than buffer */
			f = open((char *)s, O_RDONLY);
			if (f != -1) {
				close(f);
				//char *msg = "File exists. Overwrite (y,n,^C)? ";
				//req->message = msg;
				req->message = "File exists. Overwrite (y,n,^C)? ";
				return dosave2(bw, 0, req, notify);
			}
		}
		else {
			/* We're saving a newer version of the same file */
			struct stat sbuf;
			if (!stat((char *)s,&sbuf)) {
				if (sbuf.st_mtime>bw->b->mod_time) {
					//char *msg = "File on disk is newer. Overwrite (y,n,^C)? ";
					//req->message = msg;
					req->message = "File on disk is newer. Overwrite (y,n,^C)? ";
					return dosave2(bw, 0, req, notify);
				}
			}
		}
	}

	return dosave(bw, req, notify);
}

/* User command: ^K D */

int usave(BW *bw)
{
	BW *pbw;
	
	pbw = wmkpw(bw->parent, US "Name of file to save (^C to abort): ", &filehist, dosave1, US "Names", NULL, cmplt, mksavereq(NULL,NULL,NULL,0, 0), NULL, locale_map);

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

/* Write highlighted block to a file */

int ublksave(BW *bw)
{
	if (markb && markk && markb->b == markk->b && (markk->byte - markb->byte) > 0 && (!square || piscol(markk) > piscol(markb))) {
		if (wmkpw(bw->parent, US "Name of file to write (^C to abort): ", &filehist, dosave1, US "Names", NULL, cmplt, mksavereq(NULL, NULL, NULL, 0, 1), NULL, locale_map)) {
			return 0;
		} else {
			return -1;
		}
	} else {
		return usave(bw);
	}
}


/* Load file to edit */

int doedit1(BW *bw,int c,unsigned char *s,int *notify)
{
	int ret = 0;
	int er;
	void *object;
	W *w;
	B *b;
	if (c=='y' || c=='Y') {
		/* Reload from file */

		if (notify) {
			*notify = 1;
		}

		b = bfind_reload(s);
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
			msgnwt(bw->parent, msgs[-er]);
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
	} else if(c=='n' || c=='N') {
		/* Edit already loaded buffer */

		if (notify) {
			*notify = 1;
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
			msgnwt(bw->parent, msgs[-er]);
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
	} else {
		/* FIXME: need abort handler to prevent leak */
		if (mkqw(bw->parent, sc("Load original file from disk (y,n,^C)? "), doedit1, NULL, s, notify))
			return 0;
		else {
			vsrm(s);
			return -1;
		}
	}
}

int doedit(BW *bw, unsigned char *s, void *obj, int *notify)
{
	int ret = 0;
	int er;
	void *object;
	W *w;
	B *b;

	b = bcheck_loaded(s);

	if (b) {
		if (b->changed)
			/* Modified buffer exists, so ask */
			return doedit1(bw, 0, s, notify);
		else
			/* Buffer not modified- just use it as is */
			return doedit1(bw, 'n', s, notify);
	} else
		/* File not in buffer: don't ask */
		return doedit1(bw, 'y', s, notify);
}

int okrepl(BW *bw)
{
	if (bw->b->count == 1 && bw->b->changed) {
		msgnw(bw->parent, US "Can't replace modified file");
		return -1;
	} else {
		return 0;
	}
}

int uedit(BW *bw)
{
	if (wmkpw(bw->parent, US "Name of file to edit (^C to abort): ", &filehist, doedit, US "Names", NULL, cmplt, NULL, NULL, locale_map)) {
		return 0;
	} else {
		return -1;
	}
}

int doswitch(BW *bw, unsigned char *s, void *obj, int *notify)
{
	int ret = 0;
	int er;
	void *object;
	W *w;
	B *b;

	/* Try buffer, then file */
	return doedit1(bw, 'n', s, notify);
}

int uswitch(BW *bw)
{
	if (wmkpw(bw->parent, US "Name of buffer to edit (^C to abort): ", &filehist, doswitch, US "Names", NULL, cmplt, NULL, NULL, locale_map)) {
		return 0;
	} else {
		return -1;
	}
}

int doscratch(BW *bw, unsigned char *s, void *obj, int *notify)
{
	int ret = 0;
	int er;
	void *object;
	W *w;
	B *b;

	if (notify) {
		*notify = 1;
	}

	b = bfind_scratch(s);
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
		msgnwt(bw->parent, msgs[-er]);
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

int uscratch(BW *bw)
{
	if (wmkpw(bw->parent, US "Name of scratch buffer to edit (^C to abort): ", &filehist, doscratch, US "Names", NULL, cmplt, NULL, NULL, locale_map)) {
		return 0;
	} else {
		return -1;
	}
}

/* Load file into buffer: can result in an orphaned buffer */

static int dorepl(BW *bw, unsigned char *s, void *obj, int *notify)
{
	void *object = bw->object;
	int ret = 0;
	int er;
	W *w = bw->parent;
	B *b;

	if (notify) {
		*notify = 1;
	}
	b = bfind(s);
	er = error;
	if (error) {
		msgnwt(bw->parent, msgs[-error]);
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
	b = bnext();
	if (b == bw->b) {
		b = bnext();
	}
	if (b == bw->b) {
		return 0;
		/* return -1; this helps with querysave (no error when only one buffer) */
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
	b = bprev();
	if (b == bw->b) {
		b = bprev();
	}
	if (b == bw->b) {
		return 0;
		/* return -1; */
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
	if (wmkpw(bw->parent, US "Name of file to insert (^C to abort): ", &filehist, doinsf, US "Names", NULL, cmplt, NULL, NULL, locale_map)) {
		return 0;
	} else {
		return -1;
	}
}

/* Save and exit */

static int exdone(BW *bw, struct savereq *req,int flg,int *notify)
{
	if (notify)
		*notify = 1;
	rmsavereq(req);
	if (flg) {
		return -1;
	} else {
		bw->b->changed = 0;
		saverr(bw->b->name);
		return uabort1(bw, -1);
	}
}

int uexsve(BW *bw)
{
	if (!bw->b->changed || bw->b->scratch) {
		/* It didn't change or it's just a scratch buffer: don't save */
		uabort(bw, -1);
		return 0;
	} else if (bw->b->name && !exask) {
		/* It changed, it's not a scratch buffer and it's named */
		return dosave1(bw, vsncpy(NULL, 0, sz(bw->b->name)), mksavereq(exdone,NULL,NULL,0,0), NULL);
	} else {
		BW *pbw = wmkpw(bw->parent, US "Name of file to save (^C to abort): ", &filehist, dosave1, US "Names", NULL, cmplt, mksavereq(exdone,NULL,NULL,1,0), NULL, locale_map);

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

/* If buffer is modified, prompt for saving: if user types 'n', uabort(), otherwise just return. */
/* If buffer is not modified, just return. */

static int nask(BW *bw, int c, void *object, int *notify)
{
	if (c == 'y' || c == 'Y') {
		/* uexsve macro should be here... */
		if(notify)
			*notify = 1;
		return 0;
	} else if (c == 'n' || c == 'N') {
		if(notify)
			*notify = -1;
		genexmsg(bw, 0, NULL);
		abortit(bw);
		return -1;
	} else if (bw->b->count == 1 && bw->b->changed && !bw->b->scratch) {
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

/* Kill a buffer: any windows which have it get their buffer replaced with a
 * a scratch buffer */

static int dolose(BW *bw, int c, void *object, int *notify)
{
	W *w;
	B *b, *new_b;
	int cnt;

	if (notify) {
		*notify = 1;
	}
	if (c != 'y' && c != 'Y') {
		return -1;
	}

	b=bw->b;
	cnt = b->count;
	b->count = 1;
	genexmsg(bw, 0, NULL);
	b->count = cnt;

	if ((w = maint->topwin) != NULL) {
		do {
			if ((w->watom->what&TYPETW) && ((BW *)w->object)->b==b) {
				if ((new_b = borphan()) != NULL) {
					BW *bw = (BW *)w->object;
					void *object = bw->object;
					/* FIXME: Shouldn't we wabort() and wcreate here to kill
					   any prompt windows? */

					bwrm(bw);
					w->object = (void *) (bw = bwmk(w, new_b, 0));
					wredraw(w);
					bw->object = object;
				} else {
					BW *bw = (BW *)w->object;
					object = bw->object;
					bwrm(bw);
					w->object = (void *) (bw = bwmk(w, bfind(US ""), 0));
					wredraw(w);
					bw->object = object;
					if (bw->o.mnew)
						exemac(bw->o.mnew);
				}
			}
		w = w->link.next;
		} while (w != maint->topwin);
	}
	return 0;
}

int ulose(BW *bw)
{
	msgnw(bw->parent, NULL);
	if (bw->b->count==1 && bw->b->pid) {
		return ukillpid(bw);
	}
	if (bw->b->changed && !bw->b->scratch) {
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

static int dobuf(MENU *m, int x, unsigned char **s)
{
	unsigned char *name;
	BW *bw = m->parent->win->object;
	int *notify = m->parent->notify;

	m->parent->notify = 0;
	name = vsdup(s[x]);
	wabort(m->parent);
	return dorepl(bw, name, NULL, notify);
}

static int abrtb(MENU *m, int x, unsigned char **s)
{
	varm(s);
	return -1;
}

int ubufed(BW *bw)
{
	unsigned char **s = getbufs();

	vasort(av(s));
	if (mkmenu(bw->parent, s, dobuf, abrtb, NULL, 0, s, NULL))
		return 0;
	else {
		varm(s);
		return -1;
	}
}

/* Query save loop */

static int doquerysave(BW *bw,int c,struct savereq *req,int *notify)
{
	W *w = bw->parent;
	if (c == 'y' || c == 'Y') {
		if (bw->b->name && bw->b->name[0])
			return dosave1(bw, vsncpy(NULL,0,sz(bw->b->name)), req, notify);
		else {
			BW *pbw;
			pbw = wmkpw(bw->parent, US "Name of file to save (^C to abort): ", &filehist, dosave1, US "Names", NULL, cmplt, req, notify, locale_map);

			if (pbw) {
				return 0;
			} else {
				joe_free(req);
				return -1;
			}
		}
	} else if (c == 'n' || c == 'N') {
		/* Find next buffer to save */
		if (bw->b->changed)
			req->not_saved = 1;
		next:
		if (unbuf(bw)) {
			if (notify)
				*notify = 1;
			rmsavereq(req);
			return -1;
		}
		bw = w->object;
		if (bw->b==req->first) {
			if (notify)
				*notify = 1;
			rmsavereq(req);
			genexmsgmulti(bw,1,req->not_saved);
			return 0;
		}
		if (!bw->b->changed || bw->b->scratch)
			goto next;

		return doquerysave(bw,0,req,notify);
	} else {
		unsigned char buf[1024];
		joe_snprintf_1((char *)buf,1024,"File %s has been modified.  Save it (y,n,^C)? ",bw->b->name ? bw->b->name : US "(Unnamed)" );
		if (mkqw(bw->parent, sz(buf), doquerysave, NULL, req, notify)) {
			return 0;
			} else {
			/* Should be in abort function */
			rmsavereq(req);
			return -1;
		}
	}
}

static int query_next(BW *bw, struct savereq *req,int flg,int *notify)
{
	if (flg) {
		if (notify)
			*notify = 1;
		rmsavereq(req);
		return -1;
	} else
		return doquerysave(bw,'N',req,notify);
}

int uquerysave(BW *bw)
{
	W *w = bw->parent;
	B *first = bw->b;

	/* Find a modified buffer */
	do {
		if (bw->b->changed && !bw->b->scratch)
			return doquerysave(bw,0,mksavereq(query_next,NULL,first,0,0),NULL);
		else if (unbuf(bw))
			return -1;
		bw = w->object;
	} while(bw->b!=first);

	genexmsgmulti(bw,0,0);

	return 0;
}

int ukilljoe(BW *bw)
{
	leave = 1;
	return 0;
}
