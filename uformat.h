/* User text formatting functions
   Copyright (C) 1992 Joseph H. Allen

This file is part of JOE (Joe's Own Editor)

JOE is free software; you can redistribute it and/or modify it under the 
terms of the GNU General Public License as published by the Free Software 
Foundation; either version 1, or (at your option) any later version.  

JOE is distributed in the hope that it will be useful, but WITHOUT ANY 
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more 
details.  

You should have received a copy of the GNU General Public License along with 
JOE; see the file COPYING.  If not, write to the Free Software Foundation, 
675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef _Iuformat
#define _Iuformat 1

#include "config.h"
#include "b.h"

int ucenter PARAMS((BW *bw));
P *pbop PARAMS((P *p));
P *peop PARAMS((P *p));
int ubop PARAMS((BW *bw));
int ueop PARAMS((BW *bw));
void wrapword PARAMS((P *p, long int indent, int french, char *indents));
int uformat PARAMS((BW *bw));
int ufmtblk PARAMS((BW *bw));

#endif
