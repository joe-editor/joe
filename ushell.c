/*
	Shell-window functions
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
*/

#include "config.h"
#include <unistd.h>
#include <signal.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include "b.h"
#include "bw.h"
#include "w.h"
#include "pw.h"
#include "qw.h"
#include "vs.h"
#include "va.h"
#include "ufile.h"
#include "main.h"
#include "ushell.h"

extern int orphan;

/* Executed when shell process terminates */

static void cdone(BW * bw)
{
	bw->pid = 0;
	close(bw->out);
	bw->out = -1;
	if (piseof(bw->cursor)) {
		binss(bw->cursor, "** Program finished **\n");
		p_goto_eof(bw->cursor);
		bw->cursor->xcol = piscol(bw->cursor);
	} else {
		P *q = pdup(bw->b->eof);

		binss(q, "** Program finished **\n");
		prm(q);
	}
}

/* Executed for each chunk of data we get from the shell */

static void cdata(BW * bw, char *dat, int siz)
{
	P *q = pdup(bw->cursor);
	P *r = pdup(bw->b->eof);
	char bf[1024];
	int x, y;

	for (x = y = 0; x != siz; ++x) {
		if (dat[x] == 13 || dat[x] == 0) {
			;
		} else if (dat[x] == 8 || dat[x] == 127) {
			if (y) {
				--y;
			} else if (piseof(bw->cursor)) {
				pset(q, bw->cursor), prgetc(q), bdel(q, bw->cursor);
				bw->cursor->xcol = piscol(bw->cursor);
			} else {
				pset(q, r), prgetc(q), bdel(q, r);
			}
		} else {
			bf[y++] = dat[x];
		}
	}
	if (y) {
		if (piseof(bw->cursor)) {
			binsm(bw->cursor, bf, y);
			p_goto_eof(bw->cursor);
			bw->cursor->xcol = piscol(bw->cursor);
		} else {
			binsm(r, bf, y);
		}
	}
	prm(r);
	prm(q);
}

static int cstart(BW * bw, char *name, char **s, void *obj, int *notify)
{
#ifdef __MSDOS__
	if (notify) {
		*notify = 1;
	}
	varm(s);
	msgnw(bw->parent, "Sorry, no sub-processes in DOS (yet)");
	return -1;
#else
	MPX *m;

	if (notify) {
		*notify = 1;
	}
	if (bw->pid && orphan) {
		msgnw(bw->parent, "Program already running in this window");
		varm(s);
		return -1;
	}
	if (doedit(bw, vsncpy(NULL, 0, sc("")), NULL, NULL)) {
		varm(s);
		return -1;
	}
	bw = (BW *) maint->curwin->object;
	if (!(m = mpxmk(&bw->out, name, s, cdata, bw, cdone, bw))) {
		varm(s);
		msgnw(bw->parent, "No ptys available");
		return -1;
	} else {
		bw->pid = m->pid;
	}
	return 0;
#endif
}

int ubknd(BW * bw)
{
	char **a;
	char *s;

	a = vamk(3);
	s = vsncpy(NULL, 0, sz(getenv("SHELL")));
	a = vaadd(a, s);
	s = vsncpy(NULL, 0, sc("-i"));
	a = vaadd(a, s);
	return cstart(bw, getenv("SHELL"), a, NULL, NULL);
}

/* Run a program in a window */

static int dorun(BW * bw, char *s, void *object, int *notify)
{
	char **a = vamk(10);
	char *cmd = vsncpy(NULL, 0, sc("/bin/sh"));

	a = vaadd(a, cmd);
	cmd = vsncpy(NULL, 0, sc("-c"));
	a = vaadd(a, cmd);
	a = vaadd(a, s);
	return cstart(bw, "/bin/sh", a, NULL, notify);
}

B *runhist = 0;

int urun(BW * bw)
{
	if (wmkpw(bw->parent, "Program to run: ", &runhist, dorun, "Run", NULL, NULL, NULL, NULL)) {
		return 0;
	} else {
		return -1;
	}
}

/* Kill program */

static int pidabort(BW * bw, int c, void *object, int *notify)
{
	if (notify) {
		*notify = 1;
	}
	if (c != 'y' && c != 'Y') {
		return -1;
	}
	if (bw->pid) {
		kill(bw->pid, 1);
		return -1;
	} else {
		return -1;
	}
}

int ukillpid(BW * bw)
{
	if (bw->pid) {
		if (mkqw(bw->parent, sc("Kill program (y,n,^C)?"), pidabort, NULL, NULL, NULL)) {
			return 0;
		} else {
			return -1;
		}
	} else {
		return 0;
	}
}
