/*
	Basic user edit functions
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
*/

#ifndef _Iuedit
#define _Iuedit 1

extern int pgamnt;

/*
 * Movable functions
 *	return 0 if action was done
 *	return -1 otherwise
 */
int u_goto_bol(BW * bw);	/* move cursor to beginning of line */
int u_goto_eol(BW * bw);	/* move cursor to end of line */
int u_goto_bof(BW * bw);	/* move cursor to beginning of file */
int u_goto_eof(BW * bw);	/* move cursor to end of file */
int u_goto_left(BW * bw);	/* move cursor to left (left arrow) */
int u_goto_right(BW * bw);	/* move cursor to right (right arrow) */
int u_goto_prev(BW * bw);	/* move cursor to prev. word, edge, 

				   or beginning of line */
int u_goto_next(BW * bw);	/* move cursor to next word, edge,

				   or end of line */

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
int u_word_delete();
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
