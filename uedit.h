/* Basic user edit functions
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

#ifndef _Iuedit
#define _Iuedit 1

extern int pgamnt;

int ubol();
int ueol();
int ubof();
int ueof();
int ultarw();
int urtarw();
int uprvwrd();
int unxtwrd();
int utomatch();
int uuparw();
int udnarw();
int utos();
int ubos();
void scrup();
void scrdn();
int upgup();
int upgdn();
int uupslide();
int udnslide();
int uline();
int udelch();
int ubacks();
int udelw();
int ubackw();
int udelel();
int udelbl();
int udelln();
int uinsc();
int utypebw();
int uquote();
int uquote8();
int rtntw();
int uopen();
int usetmark();
int ugomark();
int ufwrdc();
int ubkwdc();
int umsg();
int uctrl();
int unedge();
int upedge();
int ubyte();
int ucol();
int utxt();

#endif
