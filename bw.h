/*
    Edit buffer window generation
    Copyright (C) 1992 Joseph H. Allen

    This file is part of JOE (Joe's Own Editor)
*/

#ifndef _Ibw
#define _Ibw 1

#include "config.h"
#include "b.h"
#include "rc.h"
#include "w.h"

#define LINCOLS 6

extern int dspasis;

typedef struct bw BW;

struct bw {
	W *parent;
	B *b;
	P *top;
	P *cursor;
	long offset;
	SCREEN *t;
	int h, w, x, y;

	OPTIONS o;
	void *object;

	int pid;		/* Process id */
	int out;		/* fd to write to process */
	int linums;
};

extern int mid;
void bwfllw();
void bwins();
void bwdel();
void bwgen();
BW *bwmk();
void bwmove();
void bwresz();
void bwrm();
int ustat();
int ucrawll();
int ucrawlr();
void orphit();

#endif
