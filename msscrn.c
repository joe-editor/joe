/* MSDOS screen interface for JOE */

#include <stdio.h>
#include <dos.h>
#include "blocks.h"
#include "vs.h"
#include "tty.h"
#include "scrn.h"

int skiptop = 0;
int lines = 0;
int columns = 0;
int dopadding = 0;

extern int mid;

/* Table of MSDOS monochrome screen attribute combinations */

unsigned atab[] = {
	7 * 256, 112 * 256, 1 * 256, 112 * 256,
	15 * 256, 112 * 256, 9 * 256, 112 * 256
};

/* How to display characters */

unsigned xlata[256] = {
	UNDERLINE, UNDERLINE, UNDERLINE, UNDERLINE,
	UNDERLINE, UNDERLINE, UNDERLINE, UNDERLINE,
	UNDERLINE, UNDERLINE, UNDERLINE, UNDERLINE,
	UNDERLINE, UNDERLINE, UNDERLINE, UNDERLINE,
	UNDERLINE, UNDERLINE, UNDERLINE, UNDERLINE,
	UNDERLINE, UNDERLINE, UNDERLINE, UNDERLINE,
	UNDERLINE, UNDERLINE, UNDERLINE, UNDERLINE,
	UNDERLINE, UNDERLINE, UNDERLINE, UNDERLINE,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	UNDERLINE,

	INVERSE + UNDERLINE, INVERSE + UNDERLINE, INVERSE + UNDERLINE,
	INVERSE + UNDERLINE,
	INVERSE + UNDERLINE, INVERSE + UNDERLINE, INVERSE + UNDERLINE,
	INVERSE + UNDERLINE,
	INVERSE + UNDERLINE, INVERSE + UNDERLINE, INVERSE + UNDERLINE,
	INVERSE + UNDERLINE,
	INVERSE + UNDERLINE, INVERSE + UNDERLINE, INVERSE + UNDERLINE,
	INVERSE + UNDERLINE,
	INVERSE + UNDERLINE, INVERSE + UNDERLINE, INVERSE + UNDERLINE,
	INVERSE + UNDERLINE,
	INVERSE + UNDERLINE, INVERSE + UNDERLINE, INVERSE + UNDERLINE,
	INVERSE + UNDERLINE,
	INVERSE + UNDERLINE, INVERSE + UNDERLINE, INVERSE + UNDERLINE,
	INVERSE + UNDERLINE,
	INVERSE + UNDERLINE, INVERSE + UNDERLINE, INVERSE + UNDERLINE,
	INVERSE + UNDERLINE,

	INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE,
	INVERSE,
	INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE,
	INVERSE,
	INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE,
	INVERSE,
	INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE,
	INVERSE,
	INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE,
	INVERSE,
	INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE,
	INVERSE,
	INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE,
	INVERSE,
	INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE,
	INVERSE,
	INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE,
	INVERSE,
	INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE,
	INVERSE,
	INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE,
	INVERSE,
	INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE, INVERSE,

	INVERSE + UNDERLINE,
};

unsigned char xlatc[256] = {
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
	110,
	111,
	112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124,
	125,
	126, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
	110,
	111,
	112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124,
	125,
	126, 63
};

/* Set attributes */

int attr(t, c)
SCRN *t;
int c;
{
	return 0;
}

void setregn()
{
}
void setins()
{
}
int clrins()
{
	return 0;
}

int eraeol(t, x, y)
SCRN *t;
{
	while (x < t->co)
		t->scrn[y * t->co + x++] = 0x0720;
}

SCRN *nopen()
{
	SCRN *t = (SCRN *) malloc(sizeof(SCRN));
	short *screen;
	union REGS regs;

	ttopen();
	regs.h.ah = 0x0F;
	int86(0x10, &regs, &regs);
	if (regs.h.al == 7)
		t->scrn = (char *) 0xB0000000;
	else
		t->scrn = (char *) 0xB8000000;
	t->co = regs.h.ah;
	t->li = *(unsigned char *) 0x00400084 + 1;
	if (!t->li)
		t->li = 25;
	t->scroll = 0;
	t->insdel = 0;
	t->updtab = 0;
	t->sary = 0;
	t->compose = 0;
	nresize(t, t->co, t->li);
	return t;
}

void nresize(t, w, h)
SCRN *t;
{
	if (h < 4)
		h = 4;
	if (w < 8)
		w = 8;
	t->li = h;
	t->co = w;
	if (t->updtab)
		free(t->updtab);
	if (t->sary)
		free(t->sary);
	if (t->compose)
		free(t->compose);
	t->updtab = (int *) malloc(t->li * sizeof(int));
	t->sary = (int *) malloc(t->li * sizeof(int));
	t->compose = (int *) malloc(t->co * sizeof(int));

	nredraw(t);
}

int cpos(t, x, y)
SCRN *t;
{
	union REGS regs;

	regs.h.ah = 2;
	regs.h.dh = y;
	regs.h.dl = x;
	regs.h.bh = 0;
	int86(0x10, &regs, &regs);
	return 0;
}

void magic()
{
}
void nscroll()
{
}

void npartial(t)
SCRN *t;
{
}

void nescape(t)
SCRN *t;
{
	cpos(t, 0, t->li - 1);
}

void nreturn(t)
SCRN *t;
{
	nredraw(t);
}

void nclose(t)
SCRN *t;
{
	cpos(t, 0, t->li - 1);
	eraeol(t, 0, t->li - 1);
	ttclose();
}

void nscrldn(t, top, bot, amnt)
SCRN *t;
{
	nredraw(t);
}

void nscrlup(t, top, bot, amnt)
SCRN *t;
{
	nredraw(t);
}

void nredraw(t)
SCRN *t;
{
	msetI(t->updtab + skiptop, -1, t->li - skiptop);
}
