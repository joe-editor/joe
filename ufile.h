/*
 * 	User file operations
 *	Copyright
 *	(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_UFILE_H
#define _JOE_UFILE_H 1

#include "config.h"
#include "types.h"

extern int exask;

void genexmsg PARAMS((BW *bw, int saved, unsigned char *name));

int ublksave PARAMS((BW *bw));
int ushell PARAMS((BW *bw));
int usys PARAMS((BW *bw));
int usave PARAMS((BW *bw));
int usavenow PARAMS((BW *bw));
int uedit PARAMS((BW *bw));
int uswitch PARAMS((BW *bw));
int uscratch PARAMS((BW *bw));
int uinsf PARAMS((BW *bw));
int uexsve PARAMS((BW *bw));
int unbuf PARAMS((BW *bw));
int upbuf PARAMS((BW *bw));
int uask PARAMS((BW *bw));
int ubufed PARAMS((BW *bw));
int ulose PARAMS((BW *bw));
int okrepl PARAMS((BW *bw));
int doswitch PARAMS((BW *bw, unsigned char *s, void *obj, int *notify));
int uquerysave PARAMS((BW *bw));
int ukilljoe PARAMS((BW *bw));

extern B *filehist;

#endif
