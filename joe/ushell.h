/*
 * 	Shell-window functions
 *	Copyright (C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

int ubknd(W *w, int k);
int uvtbknd(W *w, int k);
int ukillpid(W *w, int k);
int urun(W *w, int k);
int ubuild(W *w, int k);
int ugrep(W *w, int k);

#define SHELL_TYPE_DUMB 0 /* Dumb terminal */
#define SHELL_TYPE_VT 1 /* Smart terminal */
#define SHELL_TYPE_RAW 2 /* No terminal, just append data from program */
int cstart(BW *bw, const char *name, char **s, void *obj, int *notify, int build, int out_only, const char *first_command, int type);

extern B *runhist; /* Shell command history */
extern B *buildhist; /* Build command history */
extern B *grephist; /* Grep command history */

void vt_scrdn();
