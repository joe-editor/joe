/*
 *	tags file symbol lookup
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 * 	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <stdio.h>

#include "b.h"
#include "bw.h"
#include "main.h"
#include "pw.h"
#include "tab.h"
#include "ufile.h"
#include "usearch.h"
#include "utils.h"
#include "vs.h"
#include "w.h"

static int dotag(BW *bw, unsigned char *s, void *obj, int *notify)
{
	unsigned char buf[512];
	FILE *f;
	unsigned char *t = NULL;

	if (notify) {
		*notify = 1;
	}
	if (bw->b->name) {
		t = vsncpy(t, 0, sz(bw->b->name));
		t = vsncpy(sv(t), sc(":"));
		t = vsncpy(sv(t), sv(s));
	}
	f = fopen("tags", "r");
	if (!f) {
		msgnw(bw->parent, US "Couldn't open tags file");
		vsrm(s);
		vsrm(t);
		return -1;
	}
	while (fgets((char *)buf, 512, f)) {
		int x, y, c;

		for (x = 0; buf[x] && buf[x] != ' ' && buf[x] != '\t'; ++x) ;
		c = buf[x];
		buf[x] = 0;
		if (!strcmp(s, buf) || (t && !strcmp(t, buf))) {
			buf[x] = c;
			while (buf[x] == ' ' || buf[x] == '\t') {
				++x;
			}
			for (y = x; buf[y] && buf[y] != ' ' && buf[y] != '\t' && buf[y] != '\n'; ++y) ;
			if (x != y) {
				c = buf[y];
				buf[y] = 0;
				if (doedit(bw, vsncpy(NULL, 0, sz(buf + x)), NULL, NULL)) {
					vsrm(s);
					vsrm(t);
					fclose(f);
					return -1;
				}
				bw = (BW *) maint->curwin->object;
				buf[y] = c;
				while (buf[y] == ' ' || buf[y] == '\t') {
					++y;
				}
				for (x = y; buf[x] && buf[x] != '\n'; ++x) ;
				buf[x] = 0;
				if (x != y) {
					long line = 0;

					if (buf[y] >= '0' && buf[y] <= '9') {
						sscanf((char *)(buf + y), "%ld", &line);
						if (line >= 1) {
							int omid = mid;

							mid = 1;
							pline(bw->cursor, line - 1), bw->cursor->xcol = piscol(bw->cursor);
							dofollows();
							mid = omid;
						} else {
							msgnw(bw->parent, US "Invalid line number");
						}
					} else {
						if (buf[y] == '/' || buf[y] == '?') {
							++y;
							if (buf[y] == '^')
								buf[--y] = '\\';
							for (x = y+1; buf[x] && buf[x] != '\n' && buf[x-1] != '/'; ++x);
						}
						if (buf[x - 1] == '/' || buf[x - 1] == '?') {
							--x;
							buf[x] = 0;
							if (buf[x - 1] == '$') {
								buf[x - 1] = '\\';
								buf[x] = '$';
								++x;
								buf[x] = 0;
							}
						}
						if (x != y) {
							vsrm(s);
							vsrm(t);
							fclose(f);
							return dopfnext(bw, mksrch(vsncpy(NULL, 0, sz(buf + y)), NULL, 0, 0, -1, 0, 0), NULL);
						}
					}
				}
				vsrm(s);
				vsrm(t);
				fclose(f);
				return 0;
			}
		}
	}
	msgnw(bw->parent, US "Not found");
	vsrm(s);
	vsrm(t);
	fclose(f);
	return -1;
}

static B *taghist = NULL;

int utag(BW *bw)
{
	BW *pbw;

	pbw = wmkpw(bw->parent, US "Tag search: ", &taghist, dotag, NULL, NULL, cmplt, NULL, NULL, -1);
	if (pbw && isalnum_(bw->b->o.utf8,bw->b->o.charmap,brch(bw->cursor))) {
		P *p = pdup(bw->cursor);
		P *q = pdup(p);
		int c;

		while (isalnum_(bw->b->o.utf8,bw->b->o.charmap,(c = prgetc(p))))
			/* do nothing */;
		if (c != NO_MORE_DATA) {
			pgetc(p);
		}
		pset(q, p);
		while (isalnum_(bw->b->o.utf8,bw->b->o.charmap,(c = pgetc(q))))
			/* do nothing */;
		if (c != NO_MORE_DATA) {
			prgetc(q);
		}
		binsb(pbw->cursor, bcpy(p, q));
		pset(pbw->cursor, pbw->b->eof);
		pbw->cursor->xcol = piscol(pbw->cursor);
		prm(p);
		prm(q);
	}
	if (pbw) {
		return 0;
	} else {
		return -1;
	}
}
