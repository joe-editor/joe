/*
 *	JOE state file
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#include "types.h"

/* Set to enable use of ~/.joe_state file */
int joe_state;

/* Save a history buffer */

void save_hist(FILE *f,B *b)
{
	char *buf = vsmk(128);
	if (b) {
		P *p = pdup(b->bof, "save_hist");
		if (b->eof->line>10)
			pline(p,b->eof->line-10);
		while (!piseof(p)) {
			buf = brzs(buf, p);
			buf = vsadd(buf, '\n');
			pnextl(p);
			fprintf(f,"\t");
			emit_string(f,sv(buf));
			fprintf(f,"\n");
		}
		prm(p);
	}
	fprintf(f,"done\n");
}

/* Load a history buffer */

void load_hist(FILE *f,B **bp)
{
	B *b;
	char *buf = 0;
	char *bf = 0;
	P *q;

	b = *bp;
	if (!b)
		*bp = b = bmk(NULL);

	q = pdup(b->eof, "load_hist");

	while(vsgets(&buf,f) && zcmp(buf, "done")) {
		const char *p = buf;
		ptrdiff_t len;
		parse_ws(&p,'#');
		len = parse_string(&p,&bf);
		if (len>0) {
			if (bf[len - 1] != '\n')
				bf[len - 1] = '\n';
			binsm(q,bf,len);
			pset(q,b->eof);
		}
	}

	prm(q);
}

/* Save state */

#define STATE_ID "# JOE state file v1.0\n"

void save_state()
{
	const char *home = getenv("HOME");
	char *path = NULL;
	mode_t old_mask;
	FILE *f;
	
	if (!joe_state)
		return;
	if (!home)
		return;
	
	path = vsfmt(NULL,0,"%s/.joe_state",path);
	old_mask = umask(0066);
	f = fopen(path, "w");
	umask(old_mask);
	if(!f)
		return;

	/* Write ID */
	fprintf(f, "%s\n", STATE_ID);

	/* Write state information */
	fprintf(f,"search\n"); save_srch(f);
	fprintf(f,"macros\n"); save_macros(f);
	fprintf(f,"files\n"); save_hist(f,filehist);
	fprintf(f,"find\n"); save_hist(f,findhist);
	fprintf(f,"replace\n"); save_hist(f,replhist);
	fprintf(f,"run\n"); save_hist(f,runhist);
	fprintf(f,"build\n"); save_hist(f,buildhist);
	fprintf(f,"grep\n"); save_hist(f,grephist);
	fprintf(f,"cmd\n"); save_hist(f,cmdhist);
	fprintf(f,"math\n"); save_hist(f,mathhist);
	fprintf(f,"yank\n"); save_yank(f);
	fprintf(f,"file_pos\n"); save_file_pos(f);
	fclose(f);
}

/* Load state */

void load_state()
{
	char *path = (char *)getenv("HOME");
	char *buf = vsmk(128);
	FILE *f;
	if (!joe_state)
		return;
	if (!path)
		return;
	path = vsfmt(NULL, 0, "%s/.joe_state", path);
	f = fopen(path, "r");
	if(!f)
		return;

	/* Only read state information if the version is correct */
	if (vsgets(&buf, f) && !zcmp(buf,STATE_ID)) {

		/* Read state information */
		while(vsgets(&buf,f)) {
			if(!zcmp(buf,"search"))
				load_srch(f);
			else if(!zcmp(buf, "macros"))
				load_macros(f);
			else if(!zcmp(buf, "files"))
				load_hist(f,&filehist);
			else if(!zcmp(buf, "find"))
				load_hist(f,&findhist);
			else if(!zcmp(buf, "replace"))
				load_hist(f,&replhist);
			else if(!zcmp(buf, "run"))
				load_hist(f,&runhist);
			else if(!zcmp(buf, "build"))
				load_hist(f,&buildhist);
			else if(!zcmp(buf, "grep"))
				load_hist(f,&grephist);
			else if(!zcmp(buf, "cmd"))
				load_hist(f,&cmdhist);
			else if(!zcmp(buf, "math"))
				load_hist(f,&mathhist);
			else if(!zcmp(buf, "yank"))
				load_yank(f);
			else if(!zcmp(buf, "file_pos"))
				load_file_pos(f);
			else { /* Unknown... skip until next done */
				while(vsgets(&buf,f) && zcmp(buf,"done"));
			}
		}
	}

	fclose(f);
}
