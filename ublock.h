/*
 * 	Highlighted block functions
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_UBLOCK_H
#define _JOE_UBLOCK_H 1

#include "config.h"
#include "types.h"

extern int square;
extern int lightoff;
extern P *markb, *markk;

void pinsrect PARAMS((P *cur, B *tmp, long int width, int usetabs));
int ptabrect PARAMS((P *org, long int height, long int right));
void pclrrect PARAMS((P *org, long int height, long int right, int usetabs));
void pdelrect PARAMS((P *org, long int height, long int right));
B *pextrect PARAMS((P *org, long int height, long int right));
int markv PARAMS((int r));
int umarkb PARAMS((BW *bw));
int umarkk PARAMS((BW *bw));
int uswap PARAMS((BW *bw));
int umarkl PARAMS((BW *bw));
int utomarkb PARAMS((BW *bw));
int utomarkk PARAMS((BW *bw));
int utomarkbk PARAMS((BW *bw));
int ublkdel PARAMS((BW *bw));
int upicokill PARAMS((BW *bw));
int ublkmove PARAMS((BW *bw));
int ublkcpy PARAMS((BW *bw));
int dowrite PARAMS((BW *bw, unsigned char *s, void *object, int *notify));
int doinsf PARAMS((BW *bw, unsigned char *s, void *object, int *notify));
void setindent PARAMS((BW *bw));
int urindent PARAMS((BW *bw));
int ulindent PARAMS((BW *bw));
int ufilt PARAMS((BW *bw));
int unmark PARAMS((BW *bw));
int udrop PARAMS((BW *bw));
int utoggle_marking PARAMS((BW *bw));
int ubegin_marking PARAMS((BW *bw));
int uselect PARAMS((BW *bw));
int upsh PARAMS((BW *bw));
int upop PARAMS((BW *bw));
int ulower PARAMS((BW *bw));
int uupper PARAMS((BW *bw));
extern int nstack;
int blksum PARAMS((double *,double *));

#endif
