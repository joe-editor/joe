/* Memory block functions header file
   Copyright (C) 1991 Joseph H. Allen

This file is part of J (Joe's Editor)

J is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software
Foundation; either version 1, or (at your option) any later version. 

J is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. 

You should have received a copy of the GNU General Public License
along with GNU Emacs; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

unsigned char *bzero();
unsigned char *bset();
unsigned char *bmove();
unsigned char *bfwrd();
unsigned char *bbkwd();
unsigned umin();
unsigned umax();
int min();
int max();
int beq();
int bieq();
unsigned char *bchr();
unsigned char *brchr();
