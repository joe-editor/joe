/* e editor
 *
 * Written by: Joseph H. Allen
 *         on: September 20, 1988
 *
 * Target machine: unix
 *
 * This is an advanced screen editor program
 *
 * Joe Allen, April 2005: clean-up code enough
 * so that program will run.
 */

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

extern int errno;

char sstring[100];		/* search string */
char cstring[100];		/* replacement */

/** Set up these
*/

int vt52 = 0;			/* non-zero for vt52, 0 for ANSI */
int noscroll = 1;		/* Set for no scroll regions (I.E. stupid
				   ANSI */
int width = 80;			/* Screen width */
int height = 24;		/* Screen height */

/* TTY output buffer */

#define conbufsiz 2000
char conbuf[conbufsiz];
int conbufptr;

/* Attribute state */

int inverse;

/* Character read in */

char conch;

int edit_file_changed = 0;	/* set after file being edited has changed
				   (I.E. it should be saved when we exit) */
int pos;			/* current position within edit file */
int edit_file_siz;		/* current size of file */

char *edit_buf;			/* big buffer holding file being edited */
int edit_buf_siz;		/* size of edit buffer */
int edit_buf_end;		/* size of data in edit buffer */

int small_buf_pos;		/* current position of small buffer within
				   edit buffer */
int small_buf_end_pos;		/* buffer end position in edit buffer */
int small_buf_end;		/* offset to first free character in ram buf */
char *small_buf;		/* pointer to ram buffer */
int small_buf_changed;		/* set if small_buffer has edit_file_changed
				   (I.E. and should be put back into the
				   edit_buf when pos goes out of small_buf) */

#define small_buf_siz 4096	/* size of ram buffer */

#define helplines 11
#define helpwidth 79

char help[] = "\
+-- HELP WINDOW - turn off with ^KH ---------- And remember... ^ means ctrl --+\
|CURSOR            SEARCH FOR     DELETE          BLOCK     OTHER             |\
| ^B left ^F right ^KF stuff       ^D character   ^KB mark  ^KZ shell escape  |\
| ^P up   ^N down   ^L next match  ^W word        ^KM cut   ^KJ refrmt paragph|\
| ^A beg. of line   ^X next word   ^J end of line ^KC copy                    |\
| ^E end of line    ^Z prev. word  ^Y entire line ^KW save                    |\
| ^U up screen     ^KL line No.    ^H DEL backspc ^T insert                   |\
| ^V down screen   FILE       EXIT         SPELL             TYPING           |\
|^KU beg. of file  ^KS save   ^KX and save ^K  word ^KT mode      ^I tab      |\
|^KV end of file   ^KR insert ^KQ no save  ^KN rest   ` Ctrl char ^G abort ^K |\
+-------+-------+-------+-------+-------+-------+-------+-------+-------+-----+\
";

short *scrn;			/* Copy of actual screen */

int tops;			/* Scroll region */
int bots;

int wind;			/* Top of screen when help is on */
int xpos;			/* New calculated cursor position */
int ypos;
int saddr;			/* Screen address in file */
int xoffset;			/* Screen X offset */
int extend;

int oxpos;			/* Current cursor position */
int oypos;

int init;			/* Initial title line is on */

int backwards;			/* search direction */
int ignore;
int change;
int without;

int mark;			/* mark position */
int markf;
int handle;			/* handle for current file being edited */

/* editing mode flags */
int pic;
int autoind;
int lmar = 0;
int rmar = 78;
int overwrite;
int wrap;

int leave;			/* set if editor should now exit */
int added;

exe(p, a1, a2, a3, a4, a5, a6, a7)
{
	if (fork())
		wait(0);
	else {
		execl(p, p, a1, a2, a3, a4, a5, a6, a7);
		exit(1);
	}
}

int contrueifgotc(void)
{
	if (conch)
		return 1;
	else
		return 0;
}

void conwaitforc(void)
{
	if (conch)
		return;
	read(0, &conch, 1);
}

char congetc(void)
{
	char ch;
	conwaitforc();
	ch = conch;
	conch = 0;
	return ch;
}

void conflush()
{
	if (conbufptr) {
		write(0, conbuf, conbufptr);
		conbufptr = 0;
	}
}

void conputc(char c)
{
	conbuf[conbufptr++] = c;
	if (conbufptr == conbufsiz)
		conflush();
}

void conputs(char *s)
{
	int sz = strlen(s);
	if (sz + conbufptr > conbufsiz)
		conflush();
	if (sz > conbufsiz) {
		write(0, s, sz);
	} else {
		strcpy(conbuf + conbufptr, s);
		conbufptr += sz;
	}
}

/* Display attributes */
#define INVERSE 7
#define UNDERLINE 4

void conputcc(char c)
{
	c &= 127;
	if (vt52) {
		if (c < 32)
			c += '@';
		if (c == 127)
			c == '^';
		conputc(c);
		return;
	}
	if (c >= 32 && c <= 126) {
		if (inverse) {
			inverse = 0;
			conputs("\033[0m");
		}
		conputc(c);
	} else {
		if (inverse != 1) {
			if (inverse == 2)
				conputs("\033[0m");
			inverse = 1;
			conputs("\033[4m");
		}
		if (c == 127)
			c = ' ';
		else
			c += '@';
		conputc(c);
	}
}

void conputsc(char *s)
{
	while (*s)
		conputcc(*(s++));
}

void conopen(void)
{
	conch = 0;
	conbufptr = 0;
	inverse = 0;
	exe("/bin/stty", "raw", "-echo", 0);
}

void conclose(void)
{
	conflush();
	if (inverse && !vt52)
		conputs("\033[0m");
	exe("/bin/stty", "cooked", "echo", 0);
}

/* kch values
* 0 - 127   character typed in
* -1 - -31  control character
* -95 - -32 ^K character
* -8        DEL and ^H
*/

int kflag = 0;
int kch;

#define KESC 1
#define KK 2
#define KCTRL 3
#define KGOTC 4

int ktrueifgotc(void)
{
	char ch;
	if (kflag != KGOTC)
		if (contrueifgotc()) {
			ch = congetc() & 127;
			switch (kflag) {
			case 0:
				switch (ch) {
				case 'K' - '@':
					kflag = KK;
					break;
				case '[' - '@':
					kflag = KESC;
					break;
				case '`':
					kflag = KCTRL;
					break;
				case 'M' - '@':
					kflag = KGOTC;
					kch = '\n';
					break;
				case 'I' - '@':
					kch = ch;
					kflag = KGOTC;
					break;
				case 127:
					kch = -8;
					break;
				default:
					if (ch < ' ')
						kch = -ch;
					else
						kch = ch;
					kflag = KGOTC;
				}
				break;
			case KESC:
				if (ch == '[' || ch == 0x1b)
					break;
				switch (ch) {
				case 'A':
					kch = -('P' - '@');
					kflag = KGOTC;
					break;
				case 'B':
					kch = -('N' - '@');
					kflag = KGOTC;
					break;
				case 'C':
					kch = -('F' - '@');
					kflag = KGOTC;
					break;
				case 'D':
					kch = -('B' - '@');
					kflag = KGOTC;
					break;
				case 'H':
					kch = -('A' - '@');
					kflag = KGOTC;
					break;
				case 'F':
					kch = -('E' - '@');
					kflag = KGOTC;
					break;
				case 'G':
					kch = -('V' - '@');
					kflag = KGOTC;
					break;
				case 'I':
					kch = -('U' - '@');
					kflag = KGOTC;
					break;
				case 'K':
					kch = -('D' - '@');
					kflag = KGOTC;
					break;
				default:
					kflag = 0;
				}
				break;
			case KK:
				if (ch == 'K' - '@')
					break;
				if (ch == 'G' - '@') {
					kflag = 0;
					break;
				}
				if (ch >= 0x60)
					ch &= 0x5f;
				if (ch < ' ')
					ch += '@';
				kch = -ch;
				kflag = KGOTC;
				break;
			case KCTRL:
				kflag = KGOTC;
				if (ch == '`')
					kch = '`';
				else if (ch == 127)
					kch = 127;
				else
					kch = ch & 0x1f;
				break;
			}
		}
	return (kflag == KGOTC) ? 1 : 0;
}

void kwait(void)
{
	while (!ktrueifgotc())
		conwaitforc();
}

int kgetc(void)
{
	kwait();
	kflag = 0;
	return kch;
}

/* Move a block of memory.  No data is ever lost if blocks overlap.
*/

void cormove(char *dst, char *src, int s)
{
	if (src == dst || s == 0)
		return;
	if (src >= dst)
		do
			*(dst++) = *(src++);
		while (--s);
	else {
		dst += s;
		src += s;
		do
			*(--dst) = *(--src);
		while (--s);
	}
}

/* Return true if equal sized strings match
*/

int cormatch(char *dst, char *src, int s)
{
	if (s && dst != src)
		do
			if (*(dst++) != *(src++))
				return 0;
		while (--s);
	return 1;
}

/* Get the file manager ready */

int fmopen(int fd)
{
	struct stat buf;
	if ((small_buf = (char *) malloc(small_buf_siz)) != 0) {
		small_buf_changed = 0;
		pos = 0;
		edit_file_siz = 0;
		edit_buf_end = 0;
		edit_buf_siz = 0;
		small_buf_pos = 0;
		small_buf_end_pos = 0;
		small_buf_end = 0;
		if (fd) {
			fstat(fd, &buf);
			edit_file_siz = buf.st_size;
			edit_buf_end = edit_file_siz;
			edit_buf_siz = edit_file_siz;
			if ((edit_buf =
			     (char *) malloc(edit_file_siz)) != 0)
				if (edit_file_siz ==
				    read(fd, edit_buf, edit_file_siz))
					return 1;
		} else {
			edit_buf = (char *) malloc(1024);
			return 1;
		}
	}
	return 0;
}

void fmextract(int rsiz)
{
	small_buf_end = 0;
	small_buf_end_pos = small_buf_pos;
	if (rsiz > edit_file_siz - small_buf_pos)
		rsiz = edit_file_siz - small_buf_pos;
	if (rsiz == 0)
		return;
	cormove(small_buf, small_buf_pos + edit_buf, rsiz);
	small_buf_end = rsiz;
	small_buf_end_pos = small_buf_pos + rsiz;
}

/* if the ram buffer has changed, put it back in the file */

void fminsert(void)
{
	if (small_buf_changed) {
		small_buf_changed = 0;
		if (small_buf_end_pos - small_buf_pos == small_buf_end)
			cormove(edit_buf + small_buf_pos, small_buf,
				small_buf_end);
		else {
			if (edit_file_siz > edit_buf_siz) {
				edit_buf =
				    (char *) realloc(edit_buf,
						     edit_file_siz);
				edit_buf_siz = edit_file_siz;
			}
			cormove(small_buf_pos + small_buf_end + edit_buf,
				edit_buf + small_buf_end_pos,
				edit_buf_end - small_buf_end_pos);
			cormove(edit_buf + small_buf_pos, small_buf,
				small_buf_end);
		}
	}
	edit_buf_end = edit_file_siz;
	small_buf_end_pos = small_buf_pos;
	small_buf_end = 0;
}

/* get more data into buffer */

void fmadvance(void)
{
	fminsert();
	if (pos >= small_buf_siz / 4) {
		small_buf_pos = pos - small_buf_siz / 4;
		fmextract(small_buf_siz / 2);
	} else {
		small_buf_pos = 0;
		fmextract(small_buf_siz / 2);
	}
}

void fmfixup(void)
{
	if (small_buf_end > small_buf_siz - 160)
		fmadvance();
}

/* return with character at current position */

char fmrc(void)
{
	if (pos == small_buf_pos + small_buf_end)
		fmadvance();
	if (pos == edit_file_siz)
		return 'X';
	return small_buf[pos - small_buf_pos];
}

/* write character to current position */
/* Append character if at end of file */

void fmwc(char ch)
{
	edit_file_changed = 1;
	if (pos - small_buf_pos == small_buf_end)
		fmadvance();
	if (small_buf[pos - small_buf_pos] != ch) {
		small_buf_changed = 1;
		small_buf[pos - small_buf_pos] = ch;
	}
	if (pos == edit_file_siz) {
		small_buf_changed = 1;
		edit_file_siz++;
		small_buf_end++;
		fmfixup();
	}
}

int fmcmp(char *s, int sz)
{
	if (pos + sz > edit_file_siz)
		return 0;
	if (pos + sz > small_buf_end + small_buf_pos)
		fmadvance();
	return cormatch(s, small_buf + pos - small_buf_pos, sz);
}

int fmsrchf(char *s, int sz)
{
	char *se;
	char *sa;
	if (sz > edit_file_siz - pos)
		return 0;
	sa = pos + edit_buf;
	se = edit_buf + edit_file_siz - sz + 1;
	fminsert();
	do {
		if (cormatch(s, sa, sz)) {
			pos = sa - edit_buf;
			fmadvance();
			return 1;
		}
		sa++;
	}
	while (sa != se);
	fmadvance();
	return 0;
}

int fmsrchb(char *s, int sz)
{
	int save = pos;
	char *sa = pos + edit_buf + 1;
	char *se = edit_buf;
	fminsert();
	do {
		sa--;
		if (cormatch(s, sa, sz)) {
			pos = sa - edit_buf;
			fmadvance();
			return 1;
		}
	}
	while (sa != se);
	pos = save;
	fmadvance();
	return 0;
}

/* get character at current position and advance pointer */

char fmgetc(void)
{
	char ch;
	if (pos == edit_file_siz)
		return 'X';
	if (pos == small_buf_pos + small_buf_end)
		fmadvance();
	ch = small_buf[(pos++) - small_buf_pos];
	fmfixup();
	return ch;
}

/* insert character into current position */

void fminsc(char ch)
{
	edit_file_changed = 1;
	small_buf_changed = 1;
	cormove(small_buf + pos - small_buf_pos + 1,
		small_buf + pos - small_buf_pos,
		small_buf_end - pos + small_buf_pos);
	small_buf[pos - small_buf_pos] = ch;
	small_buf_end++;
	edit_file_siz++;
	fmfixup();
}

/* write a single character and then advance position */

void fmputc(char ch)
{
	edit_file_changed = 1;
	if (small_buf[pos - small_buf_pos] != ch) {
		small_buf[pos - small_buf_pos] = ch;
		small_buf_changed = 1;
	}
	if (pos - small_buf_pos == small_buf_end) {
		small_buf_changed = 1;
		small_buf_end++;
		if (pos == edit_file_siz)
			edit_file_siz++;
		fmfixup();
	}
	pos++;
}

void fminss(char *s, int len)
{
	while (len--)
		fmputc(*s++);
}

/* reposition edit file pointer */

void fmpoint(int newp)
{
	pos = newp;
	if (newp > small_buf_pos + small_buf_end || newp < small_buf_pos)
		fmadvance();
}

/* position pointer to first \n found */

int fmfnl(void)
{
	while (!(pos == edit_file_siz))
		if (fmgetc() == '\n') {
			fmpoint(pos - 1);
			return 1;
		}
	return 0;
}

/* position pointer to next \n found */

int fmnnl(void)
{
	if (!(pos == edit_file_siz)) {
		fmpoint(pos + 1);
		return fmfnl();
	}
	return 0;
}

/* position pointer to first \n found (search in reverse) */

int fmfrnl(void)
{
	do {
		if (fmrc() == '\n')
			return 1;
		if (pos == 0)
			break;
		fmpoint(pos - 1);
	} while (1);
	return 0;
}

/* position pointer to next \n found (search in reverse) */

int fmnrnl(void)
{
	if (pos) {
		fmpoint(pos - 1);
		return fmfrnl();
	}
	return 0;
}

/* Save stuff from the file into another file */

int fmsave(int savfil, int savsiz)
{
	int x;			/* amount to write from buffer */
	int y;			/* amount to write from file */
	int z;
	if (small_buf_end != pos - small_buf_pos) {
		if (savsiz <= small_buf_end - pos + small_buf_pos) {
			y = 0;
			x = savsiz;
		} else {
			x = small_buf_end - pos + small_buf_pos;
			y = savsiz - x;
		}
	} else
		x = 0, y = savsiz;
	if (x)
		if ((z =
		     write(savfil, small_buf + pos - small_buf_pos,
			   x)) != x) {
			return 0;
		}
	if (y)
		if ((z =
		     write(savfil, edit_buf + small_buf_end_pos,
			   y)) != y) {
			return 0;
		}
	return 1;
}

void fmdel(int delsiz)
{
	edit_file_changed = 1;
	if (delsiz == 0)
		return;
	if (pos + delsiz > edit_file_siz)
		delsiz = edit_file_siz - pos;
	small_buf_changed = 1;
	if (delsiz <= small_buf_end - pos + small_buf_pos)	/* if deletion can be done completely in buf */
		if (delsiz == small_buf_end - pos + small_buf_pos)
			small_buf_end -= delsiz;
		else {
			cormove(small_buf + pos - small_buf_pos,
				small_buf + pos - small_buf_pos + delsiz,
				small_buf_end - pos + small_buf_pos -
				delsiz);
			small_buf_end -= delsiz;
	} else {
		cormove(edit_buf + small_buf_end_pos,
			edit_buf + small_buf_end_pos + delsiz -
			small_buf_end + pos - small_buf_pos,
			edit_buf_end - small_buf_end_pos - delsiz +
			small_buf_end - pos + small_buf_pos);
		edit_buf_end -=
		    delsiz - small_buf_end + pos - small_buf_pos;
		small_buf_end = pos - small_buf_pos;
	}
	edit_file_siz -= delsiz;
}

/* Insert a disk file into the edit file
*/

int fminsfil(int fd)
{
	int asiz;
	struct stat buf;	/* Status buffer: to get file size */
	if (fstat(fd, &buf))
		return 0;	/* Get file size */
	if (buf.st_size == 0)
		return 1;	/* return without setting edit_file_changed if
				   nothing to insert */
	fminsert();		/* put small buffer into big buffer */
	asiz = edit_file_siz + buf.st_size;	/* calc needed size */
	if (asiz > edit_buf_siz) {	/* resize buffer */
		edit_buf = (char *) realloc(edit_buf, asiz);
		edit_buf_siz = asiz;
	}
	cormove(pos + edit_buf + buf.st_size, pos + edit_buf,
		edit_buf_end - pos);
	if (buf.st_size != (asiz = read(fd, edit_buf + pos, buf.st_size))) {
		cormove(pos + edit_buf + asiz,
			pos + edit_buf + buf.st_size, edit_buf_end - pos);
		edit_file_siz += asiz;
		edit_buf_end += asiz;
		fmadvance();
		return 0;
	}
	edit_file_siz += asiz;
	edit_buf_end += asiz;
	fmadvance();
	edit_file_changed = 1;
	return 1;
}

void cposs(int y, int x)
{
	char s[9];
	if (y == oypos) {
		if (x == oxpos)
			return;
		if (x + 1 == oxpos) {
			conputc(8);
			return;
		}
		if (x == 0) {
			conputc(13);
			return;
		}
		if (x + 1 == oxpos) {
			conputcc(scrn[oypos * width + oxpos]);
			return;
		}
		if (vt52) {
			conputc(033);
			conputc('Y');
			conputc(040 + oypos);
			conputc(040 + x);
			return;
		}
		if (x > oxpos)
			sprintf(s, "\033[%dC", x - oxpos);
		else
			sprintf(s, "\033[%dD", oxpos - x);
		conputs(s);
		return;
	}
	if (x == oxpos) {
		if (y == oypos + 1) {
			conputc(10);
			return;
		}
		if (y == 0 && x == 0) {
			if (vt52)
				conputs("\033H");
			else
				conputs("\033[H");
			return;
		}
		if (!vt52) {
			if (y > oypos)
				sprintf(s, "\033[%dB", y - oypos);
			else
				sprintf(s, "\033[%dA", oypos - y);
			conputs(s);
			return;
		}
	}
	if (x == 0 && y == oypos + 1) {
		conputs("\015\012");
		return;
	}
	if (x == 0 && y == 0) {
		if (vt52)
			conputs("\033H");
		else
			conputs("\033[H");
		return;
	}
	if (vt52) {
		conputc(033);
		conputc('Y');
		conputc(040 + y);
		conputc(040 + x);
		return;
	}
	sprintf(s, "\033[%d;%dH", y + 1, x + 1);
	conputs(s);
	return;
}

void cpos(int y, int x)
{
	cposs(y, x);
	oxpos = x;
	oypos = y;
}

void setregn(int top, int bot)
{
	char sst[16];
	if (noscroll || vt52)
		tops = 0, bots = height - 1;
	else if (top != tops || bots != bot) {
		sprintf(sst, "\033[%d;%dr", top + 1, bot + 1);
		oxpos = 0;
		oypos = 0;
		tops = top;
		bots = bot;
		conputs(sst);
	}
}

void clreoschk(int i, int j)
{
	int t;
	int k;
	if (j < xoffset)
		j = 0;
	else if (j >= xoffset + width - 1)
		j = 0, i++;
	else
		j -= xoffset;
	k = i * width + j;
	for (t = width * (height - init) - 1; t >= k; t--)
		if (scrn[t] != ' ')
			goto ohoh;
	return;
      ohoh:
	if (t == k) {
		cpos(i, j);
		conputc(' ');
		scrn[t] = ' ';
		oxpos++;
		return;
	}
	for (; t >= k; t--)
		scrn[t] = ' ';
	cpos(i, j);
	if (vt52)
		conputs("\033J");
	else
		conputs("\033[J");
}

void clreolchk(int i, int j)
{
	int k = i * width;
	int t;
	if (j < xoffset)
		j = 0;
	else if (j >= xoffset + width - 1)
		return;
	else
		j -= xoffset;
	for (t = width - 1; t >= j; t--)
		if (scrn[k + t] != ' ')
			goto ohoh;
	return;
      ohoh:
	if (t == j) {
		cpos(i, j);
		conputc(32);
		scrn[k + j] = ' ';
		oxpos++;
		return;
	}
	for (; t >= j; t--)
		scrn[k + t] = ' ';
	cpos(i, j);
	if (vt52)
		conputs("\033K");
	else
		conputs("\033[K");
}

int udline(int i)
{
	int j;
	int k = 1;
	int t;
	int u;
	char ch;
	for (j = 0; 1; j++) {
		if ((pos == edit_file_siz)) {
			clreoschk(i, j);
			return 0;
		}
		ch = fmgetc();
		if (ch == '\n') {
			clreolchk(i, j);
			return k;
		}
		if (ch == 9) {
			t = i * width + j - xoffset;
			do {
				if (j >= xoffset
				    && j < xoffset + width - 1) {
					u = scrn[t];
					if (' ' != u || u == -1) {
						if (ktrueifgotc())
							return 0;
						cpos(i, j - xoffset);
						scrn[t] = ' ';
						conputcc(' ');
						oxpos++;
						k = 2;
					}
				}
				t++;
				j++;
			} while (j & 7);
			j--;
		} else {
			t = i * width + j - xoffset;
			if (j >= xoffset && j < xoffset + width - 1) {
				u = scrn[t];
				if (ch != u || u == -1) {
					if (ktrueifgotc())
						return 0;
					cpos(i, j - xoffset);
					scrn[t] = ch;
					conputcc(ch);
					oxpos++;
					k = 2;
				}
			}
		}
	}
}

void udscrn(int cy)
{
	int i;
	int k;
	int ctop = pos;
	fmpoint(cy);
	for (i = ypos; i < height - init; i++) {
		if (!(k = udline(i)))
			break;
		else
			conflush();
	}
	fmpoint(ctop);
	for (i = wind; i < ypos; i++) {
		if (!(k = udline(i)))
			break;
		else
			conflush();
	}
}

void dupdate1(void)
{
	int y;
	int sve = pos;
	char ch;
	int sve1;
	int x = extend;
/* Calculate column position */

	if (!x) {
		if (fmnrnl())
			fmpoint(pos + 1);
		x = 0;
		while (pos != sve) {
			ch = fmgetc();
			if (ch == '\t')
				while ((++x) & 7);
			else
				x++;
		}
	}

	if (fmnrnl())
		fmpoint(pos + 1);
	sve1 = pos;
/* calculate what screen cursor position should be */

	if (x > xoffset + width - 2)
		xpos = width - 2, xoffset = x - (width - 2);
	else if (x < xoffset)
		xpos = 0, xoffset = x;
	else
		xpos = x - xoffset;

/* calculate new y cursor position and point to beginning of screen */

	if (pos <= saddr) {
		ypos = wind;
		saddr = pos;
	} else {
		/* is cursor within 24 lines of old beginning of screen */

		for (y = 1; y < height - wind; y++) {
			fmpoint(pos - 1);
			if (fmnrnl())
				fmpoint(pos + 1);
			if (pos == saddr)
				goto over;
		}
		y--;
	      over:
		saddr = pos;
		ypos = y + wind;
	}

/* Now update screen */
	if (ktrueifgotc()) {
		cpos(ypos, xpos);
	} else {
		udscrn(sve1);
		cpos(ypos, xpos);
	}
	fmpoint(sve);
}

void dupdatehelp(void)
{
	int i;
	int j;
	int t;
	int f = 0;
	wind = helplines;
	for (i = 0; i < helplines; i++) {
		for (j = 0; j < helpwidth; j++) {
			t = width * i + j;
			if (scrn[t] != help[f]) {
				if (ktrueifgotc())
					return;
				cpos(i, j);
				if (help[f] >= 'A' && help[f] <= 'Z'
				    && help[f - 1] != '^'
				    && help[f - 2] != '^'
				    && (help[f - 1] >= 'A'
					&& help[f - 1] <= 'Z'
					|| help[f + 1] >= 'A'
					&& help[f + 1] <= 'Z'))
					conputcc(0x1f &
						 (scrn[t] = help[f]));
				else
					conputcc((scrn[t] = help[f]));
				oxpos++;
			}
			f++;
		}
		conflush();
	}
}

void dupdate(void)
{
	if (wind)
		dupdatehelp();
	dupdate1();
	if (init)
		init = 0;
	conflush();
}

void rewrite(void)
{
	int t;
	oxpos = 0;
	oypos = 0;
	tops = -1;
	bots = -1;
	if (vt52)
		conputs("\033H\033J");
	else
		conputs("\033[H\033[J");
	for (t = 0; t < width * height; t++)
		scrn[t] = -1;
}

void thelp(void)
{
	if (wind)
		wind = 0;
	else
		wind = helplines;
}

void invalidate(void)
{
	int x;
	for (x = 0; x < width; x++)
		scrn[width * (height - 1) + x] = -1;
}

/* function to enter a line of input */
/* Return false if user types ^G */

int getl(char *prompt, char *dat)
{
	int x;
	int ch;
	dat[x = 0] = 0;

	cpos(height - 1, 0);
	conputs(prompt);
	conputs(" (Ctrl-C to abort): ");
	conputs(dat);
	if (vt52)
		conputs("\033K");
	else
		conputs("\033[K");
	conflush();
	while (1) {
		ch = kgetc();
		if (ch == '\n') {
			ch = 1;
			break;
		}
		if (ch >= 0) {
			if (x >= 40)
				continue;
			dat[x + 1] = 0;
			dat[x++] = ch;
			conputcc(ch);
			conflush();
			continue;
		}
		if (ch == -('H' - '@') && x) {
			x--;
			dat[x] = 0;
			conputs("\010 \010");
			conflush();
			continue;
		}
		if (ch == -('C' - '@')) {
			ch = 0;
			break;
		}
	}
	conputc(13);
	invalidate();
	return ch;
}

int msg(char *ms)
{
	int c;
	cpos(height - 1, 0);
	inverse = 0;
	conputs("\033[0m\033[7m");
	conputs(ms);
	conputs("\033[0m");
	inverse = 0;
	if (vt52)
		conputs("\033K");
	else
		conputs("\033[K");
	conflush();
	invalidate();
	c = kgetc();
	conputc(13);
	return c;
}

char query(char *ms)
{
	char ch;
	cpos(height - 1, 0);
	conputs(ms);
	if (vt52)
		conputs("\033K");
	else
		conputs("\033[K");
	conflush();
	invalidate();
	ch = kgetc();
	conputc(13);
	if (ch >= 'a' && ch <= 'z')
		ch &= 0x5f;
	return ch;
}

void exmsg(char *ms)
{
	cpos(height - 1, 0);
	conputs(ms);
	if (vt52)
		conputs("\033K\012\015");
	else
		conputs("\033[K\012\015");
}

void dopen(void)
{
	int x;

/* set terminal characteristics */

/* Allocate memory for screen buffer */

	scrn = (short *) malloc(height * width * sizeof(short));
	if (!scrn) {
		conputs("\rNOT ENOUGH RAM FOR SCREEN\r\n");
		conflush();
		conclose();
		exit(1);
	}

/* Clear screen buffer */

	for (x = 0; x < height * width; x++)
		scrn[x] = ' ';

/* init screen variables */

	saddr = 0;
	wind = 0;
	init = 1;
	tops = -1;
	bots = -1;
	oxpos = 0;
	oypos = 0;
	xoffset = 0;
	extend = 0;

/* Clear the screen */

	setregn(0, height - 1);
	if (!vt52)
		conputs("\033[H\033[J");
	else
		conputs("\033H\033J");
	cpos(height - 1, 0);
	if (vt52)
		conputs
		    ("** E editor written at WPI by Joe Allen, 1988 ** Hit Ctrl-KH for help        **");
	else
		conputs
		    ("\033[7m** E editor written at WPI by Joe Allen, 1988 ** Hit Ctrl-KH for help        **\033[0m");
	invalidate();
	oxpos = 79;
	cpos(0, 0);
	init = 1;
}

void dclose(void)
{
	setregn(0, height - 1);	/* Scroll regions to normal */
	cpos(height - 1, 0);
	if (vt52)
		conputs("\033K");
	else
		conputs("\033[K");
}

/* Fill from end of line to extend position */

void fillup(void)
{
	int x;
/* calculate column position of end of line */

	if (fmnrnl())
		fmpoint(pos + 1);
	x = 0;

	while (1) {
		if ((pos == edit_file_siz))
			goto dn;
		switch (fmgetc()) {
		case '\t':
			x += 8 - x % 8;
			break;
		case '\n':
			fmpoint(pos - 1);
			goto dn;
		default:
			x++;
		}
	}
      dn:
	for (x = extend - x; x; x--) {
		fminsc(' ');
		fmpoint(pos + 1);
	}
	extend = 0;
}

/* Move cursor to beginning of file */

void bof(void)
{
	extend = 0;
	fmpoint(0);
}

/* Move cursor to beginning of line */

void bol(void)
{
	if (fmnrnl())
		fmpoint(pos + 1);
	extend = 0;
}

/* Move cursor to end of line */

void eol(void)
{
	extend = 0;
	fmfnl();
}

/* Move cursor to end of file */

void eof(void)
{
	extend = 0;
	fmpoint(edit_file_siz);
}

/* Move cursor right */
void udnarw(void);
void urtarw(void)
{
	if (extend && pic) {
		fillup();
	}
	extend = 0;
	if (pic) {
		if ((pos == edit_file_siz)) {
			fminsc(' ');
		} else if (fmrc() == '\n')
			fminsc(' ');
	}
	if (fmrc() == '\n') {
		bol();
		udnarw();
	} else if (!(pos == edit_file_siz))
		fmpoint(pos + 1);
}

void rtarw(void)
{
	if (extend && pic) {
		fillup();
	}
	extend = 0;
	if (pic) {
		if ((pos == edit_file_siz)) {
			fminsc(' ');
		} else if (fmrc() == '\n')
			fminsc(' ');
	}
	if (!(pos == edit_file_siz))
		fmpoint(pos + 1);
}

/* Move cursor left */
void uuparw(void);

void ultarw(void)
{
	if (pic && extend) {
		fillup();
	}
	if (extend)
		extend = 0;
	else if (pos) {
		fmpoint(pos - 1);
		if (fmrc() == '\n') {
			fmpoint(pos + 1);
			uuparw();
			eol();
		}
	}
}

void ltarw(void)
{
	if (pic && extend) {
		fillup();
	}
	if (extend)
		extend = 0;
	else if (pos)
		fmpoint(pos - 1);
}

/* Move cursor up */

void uparw(void)
{
	int x;			/* desired column position */
	int y;			/* column counter */
	char ch;

/* determine desired column position */

	x = pos;

	if (extend) {
		x = extend;
		bol();
		extend = 0;
	} else {
		bol();
		y = 0;
		while (pos != x) {
			ch = fmgetc();
			if (ch == '\t')
				while ((++y) & 7);
			else
				y++;
		}
		bol();
		x = y;
	}

/* move to beginning of previous line */

	if (pos) {
		fmpoint(pos - 1);
		if (fmnrnl())
			fmpoint(pos + 1);
	}

/* now move to desired column position */

	for (y = 0; y != x; y++) {
		ch = fmgetc();
		if (ch == '\n') {
			fmpoint(pos - 1);
			extend = x;
			return;
		}
		if (ch == '\t') {
			while ((++y) & 7) {
				if (y == x) {
					fmpoint(pos - 1);
					extend = x;
					return;
				}
			}
			y -= 1;
		}
	}
}

/* user's cursor up routine (uses scrolling regions) */

void uuparw(void)
{
	int sve = pos;
	int y = wind * width;
	int x;
	if (!noscroll) {
		if (fmnrnl() && (!vt52 || !wind)) {
			if (pos + 1 == saddr) {
				if (fmnrnl())
					fmpoint(pos + 1);
				saddr = pos;
				if (!vt52)
					setregn(wind, (height - 1));
				if (vt52)
					conputs("\033I");
				else
					conputs("\033M");
				for (x = width * height - 1;
				     x >= y + width; x--)
					scrn[x] = scrn[x - width];
				for (x = y; x < y + width; x++)
					scrn[x] = ' ';
			}
		}
		fmpoint(sve);
	}
	uparw();
}

/* Move cursor down */

void dnarw(void)
{
	int x;			/* desired column */
	int y;			/* column counter */
	char ch;
	x = pos;
	if (extend) {
		x = extend;
		bol();
		extend = 0;
	} else {
		bol();
		y = 0;
		while (pos != x) {
			ch = fmgetc();
			if (ch == '\t')
				while ((++y) & 7);
			else
				y++;
		}
		bol();
		x = y;
	}

	if (!fmfnl())
		bol();
	else
		fmpoint(pos + 1);

	for (y = 0; y != x; y++) {
		if ((pos == edit_file_siz)) {
			extend = x;
			return;
		}
		ch = fmgetc();
		if (ch == '\n') {
			fmpoint(pos - 1);
			extend = x;
			return;
		}
		if (ch == '\t') {
			while ((++y) & 7) {
				if (y == x) {
					fmpoint(pos - 1);
					extend = x;
					return;
				}
			}
			y -= 1;
		}
	}
}

/* user's down arrow function */

void udnarw(void)
{
	int sve = pos;
	int x;
	if (vt52 && wind)
		goto cant1;
	if (!fmfnl()) {
		if (pic) {
			fminsc('\n');
			fmpoint(sve);
			udnarw();
			return;
		} else {
			goto cant;
		}
	}
	for (x = (height - 1) - wind; x; x--)
		if (!fmnrnl())
			goto cant;
	if (fmnrnl())
		fmpoint(pos + 1);
	if (pos == saddr) {
		fmfnl();
		fmpoint(pos + 1);
		saddr = pos;
		if (!vt52)
			setregn(wind, (height - 1));
		if (wind)
			goto cant;
		cpos((height - 1), oxpos);
		conputc(10);
		for (x = wind * width; x < (height - 1) * width; x++)
			scrn[x] = scrn[x + width];
		for (x = (height - 1) * width; x < width * height; x++)
			scrn[x] = ' ';
	}
      cant:
	fmpoint(sve);
      cant1:
	dnarw();
}

/* Delete character under cursor */

void delch(void)
{
	char ch;
	int x;
	if (extend && pic)
		return;
	if (extend) {
		extend = 0;
		return;
	}
	if (!(pos == edit_file_siz)) {
		if (fmrc() == '\n') {
			if (ypos < height - 2 && !vt52 && !noscroll) {
				for (x = (ypos + 1) * width;
				     x < width * height; x++)
					scrn[x] = scrn[x + width];
				for (x = width * (height - 1);
				     x < width * height; x++)
					scrn[x] = ' ';
				setregn(ypos + 1, height - 1);
				cpos(height - 1, oxpos);
				conputc(10);
				setregn(wind, height - 1);
			}
		}
		fmdel(1);
	}
}

/* Insert character typed into text */

void type(char ch)
{
	int x;
	int y;
	int temp;
	int temp1;
	if (extend && pic)
		fillup();
	if (extend) {
		extend = 0;
	}
	if (ch == '\n') {
		ch = '\n';
		fminsc(ch);
		fmpoint(pos + 1);
		if (ypos != (height - 1)) {
			if (ypos < height - 2 && !vt52 && !noscroll) {
				setregn(ypos + 1, (height - 1));
				cpos(ypos + 1, oxpos);
				conputs("\033M");
				setregn(wind, (height - 1));
				cpos(ypos, xpos);
				for (x = width * height - 1;
				     x >= (ypos + 2) * width; x--)
					scrn[x] = scrn[x - width];
				for (x = (ypos + 1) * width;
				     x < (ypos + 2) * width; x++)
					scrn[x] = ' ';
			}
		} else if ((!(vt52 || noscroll) || !wind)) {
			if (!vt52)
				setregn(wind, (height - 1));
			cpos((height - 1), oxpos);
			conputc(10);
			for (x = wind * width; x < (height - 1) * width;
			     x++)
				scrn[x] = scrn[x + width];
			for (x = (height - 1) * width; x < height * width;
			     x++)
				scrn[x] = ' ';
		}
		if (autoind) {
			temp = pos;
			uparw();
			for (x = 0; 1; x++) {
				ch = fmgetc();
				if (!(ch == ' ' || ch == '\t'))
					break;
				temp1 = pos;
				fmpoint(temp);
				fminsc(ch);
				added++;
				fmpoint(temp1);
				temp++;
			}
			fmpoint(pos - (x + 1));
			dnarw();
			y = overwrite, overwrite = 0;
			for (; x; x--)
				rtarw();
			overwrite = y;
		}
	} else {
		if (overwrite)
			if (fmrc() != '\n')
				fmdel(1);
		if (pos == 0)
			goto ind;
		fmpoint(pos - 1);
		if (fmrc() == '\n') {
			fmpoint(pos + 1);
		      ind:
			for (x = 0; x < lmar; x++)
				fminsc(' '), rtarw();
			goto skip1;
		} else
			fmpoint(1 + pos);
		if (wrap && (lmar != rmar)) {
			int sve = pos;
			int x = 0;
			char ch1;
			if (!x) {
				if (fmnrnl())
					fmpoint(pos + 1);
				x = 0;
				while (pos != sve) {
					ch1 = fmgetc();
					if (ch1 == '\t')
						while ((++x) & 7);
					else
						x++;
				}
			}
			if (x != (rmar))
				goto skip;
			if (ch == ' ')
				type('\n');
			else {
				temp = pos;
				while (1) {
					if (pos) {
						fmpoint(pos - 1);
						x = fmrc();
						if (x == '\n')
							break;
						if (x == ' ' || x == '\t') {
							fmdel(1);
							added = 0;
							type('\n');
							temp += added;
							break;
						}
					} else
						break;
				}
				for (x = 0; x < lmar; x++)
					fminsc(' '), rtarw();
				fmpoint(temp + lmar);
				fminsc(ch);
				rtarw();
			}
		} else {
		      skip:
			if (ch != '\t' && ch != '\n') {
				conputcc(ch);
				scrn[oypos * width + oxpos] = ch;
				oxpos++;
				conflush();
			}
		      skip1:
			fminsc(ch);
			rtarw();
		}
	}
}

void itype(char ch)
{
	int x;
	int y;
	int temp;
	int temp1;
	if (extend && pic)
		fillup();
	if (extend) {
		extend = 0;
	}
	if (ch == '\n') {
		fminsc(ch);
		fmpoint(pos + 1);
		if (autoind) {
			temp = pos;
			uparw();
			for (x = 0; 1; x++) {
				ch = fmgetc();
				if (!(ch == ' ' || ch == '\t'))
					break;
				temp1 = pos;
				fmpoint(temp);
				fminsc(ch);
				added++;
				fmpoint(temp1);
				temp++;
			}
			fmpoint(pos - (x + 1));
			dnarw();
			y = overwrite, overwrite = 0;
			for (; x; x--)
				rtarw();
			overwrite = y;
		}
	} else {
		if (overwrite)
			if (fmrc() != '\n')
				fmdel(1);
		if (wrap) {
			int sve = pos;
			int x = 0;
			char ch1;
			if (!x) {
				if (fmnrnl())
					fmpoint(pos + 1);
				x = 0;
				while (pos != sve) {
					ch1 = fmgetc();
					if (ch1 == '\t')
						while ((++x) & 7);
					else
						x++;
				}
			}

			if (x != (width - 2))
				goto skip;
			if (ch == ' ')
				itype('\n');
			else {
				temp = pos;
				while (1) {
					if (pos) {
						fmpoint(pos - 1);
						x = fmrc();
						if (x == '\n')
							break;
						if (x == ' ' || x == '\t') {
							fmdel(1);
							added = 0;
							itype('\n');
							temp += added;
							break;
						}
					} else
						break;
				}
				fmpoint(temp);
				fminsc(ch);
				rtarw();
			}
		} else {
		      skip:
			fminsc(ch);
			rtarw();
		}
	}
}

/* Deleting backspace */

void backs(void)
{
	if (extend && pic)
		fillup();
	if (extend) {
		extend = 0;
		return;
	}
	if (pos) {
		ultarw();
		if (overwrite) {
			itype(' ');
			ltarw();
		} else
			delch();
		if (oxpos)
			conputc(8), conputcc(32), conputc(8), oxpos--,
			    scrn[oypos * width + oxpos] = 32, conflush();
	}
}

/* quit: exit without saving */

void eexit(void)
{
	if (edit_file_changed) {
		char ch;
		ch=query("Do you really want to throw away this file?");
		if (!(ch == 'y' || ch == 'Y'))
			return;
		exmsg("File not saved.");
	} else {
		exmsg("File not changed so no update needed");
	}
	leave = 1;
}

void pgup(void)
{
	int nlins = height - wind;
	int hlins = nlins / 2;
	int x, y;
	int curpos = pos;
	fmpoint(saddr);
	for (x = 0; x < hlins; x++) {
		if (fmnrnl()) {
			if (fmnrnl())
				fmpoint(pos + 1);
		} else
			break;
	}
	if (!x) {
		fmpoint(curpos);
		return;
	}
	saddr = pos;
	fmpoint(curpos);
	if (!vt52 && !noscroll)
		setregn(wind, (height - 1));
	if ((!vt52 || !wind) && !noscroll) {
		cpos(wind, oxpos);
		for (y = 0; y < x; y++)
			conputs(vt52 ? "\033I" : "\033M");
		cpos(ypos, oxpos);
	}
	for (y = 0; y < x; y++)
		uparw();
	x *= width;
	if ((!vt52 || !wind) && !noscroll)
		for (y = wind * width; y < x + wind * width; y++) {
			scrn[y + x] = scrn[y];
			scrn[y] = ' ';
		}
}

void pgdn(void)
{
	int nlins = height - wind;
	int hlins = nlins / 2;
	int curpos = pos;
	int x, y;
	x = nlins;
	fmpoint(saddr);
	while (x--) {
		if (fmfnl())
			fmpoint(pos + 1);
		else {
			fmpoint(curpos);
			return;
		}
	}
	for (x = 1; x < hlins; x++) {
		if (fmfnl())
			fmpoint(pos + 1);
		else
			break;
	}
	if (!vt52)
		setregn(wind, (height - 1));
	cpos((height - 1), oxpos);
	fmpoint(saddr);
	for (y = 0; y < x; y++) {
		fmfnl();
		fmpoint(pos + 1);
	}
	saddr = pos;
	fmpoint(curpos);
	for (y = 0; y < x; y++) {
		dnarw();
		if (!(noscroll || vt52) || !wind)
			conputc(10);
	}
	cpos(ypos, oxpos);
	if (!(noscroll || vt52) || !wind) {
		y = width * x;
		for (curpos = wind * width + y; curpos < height * width;
		     curpos++)
			scrn[curpos - y] = scrn[curpos];
		for (curpos = height * width - width * x;
		     curpos < height * width; curpos++)
			scrn[curpos] = ' ';
	}
}

void deleol(void)
{
	int temp = pos;
	int temp1;
	if (extend && pic)
		return;
	extend = 0;
	fmfnl();
	temp1 = pos - temp;
	fmpoint(temp);
	if (temp1)
		fmdel(temp1);
}

void dellin(void)
{
	bol();
	deleol();
	delch();
}

int saveit1(char *tos)
{
	char sting[160];
	int temp = pos;
	fmpoint(0);
	handle = open(tos, O_WRONLY + O_TRUNC + O_CREAT, 0666);
	if (handle != -1) {
		if (!fmsave(handle, edit_file_siz)) {
			sprintf(sting, "Error writing to file %s.", tos);
			msg(sting);
			fmpoint(temp);
			return (0);
		}
		fmpoint(temp);
		if (close(handle)) {
			sprintf(sting, "Error closing file %s.", tos);
			msg(sting);
			fmpoint(temp);
			return (0);
		}
		return (1);
	} else {
		sprintf(sting, "Error opening file %s.", tos);
		msg(sting);
		fmpoint(temp);
		return (0);
	}
}

char gfnam[100];

void exsave(void)
{
	char sting[160];
	if (!edit_file_changed) {
		exmsg("File not changed so no update needed");
		leave = 1;
		return;
	}
	if (gfnam[0] == 0) {
		if (!getl("Save file", gfnam))
			return;
	}
	if (saveit1(gfnam)) {
		sprintf(sting, "File %s saved.", gfnam);
		exmsg(sting);
		leave = 1;
	}
}

void saveit(void)
{
	char gfnam1[160];
	char sting[160];
	strcpy(gfnam1, gfnam);
	if (!getl("Save file", gfnam1))
		return;
	if (saveit1(gfnam1)) {
	}
}

void findline(void)
{
	char sting[80];
	int x;
	sting[0] = 0;
	if (!getl("Goto line", sting))
		return;
	x = atol(sting);
	if (!x) {
		msg("Bad line number.");
		return;
	}
	x--;
	bof();
	for (; x; x--) {
		if (!fmfnl())
			break;
		fmpoint(pos + 1);
	}
	x = pos;
	if (fmnrnl())
		fmpoint(pos + 1);
	saddr = pos;
	fmpoint(x);
	return;
}

int dosb(void)
{
	return 0;
}

int dosf(void)
{
	int slen = strlen(sstring);
	int y = pos;
	extend = 0;
	if (ignore ? fmsrchf(sstring, slen) : fmsrchf(sstring, slen)) {
		if (change) {
			int c;
			if (!without) {
				dupdate();
				msg("Replace this (y,n,q)?");
				c = kgetc();
				if (c == 'y' || c == 'Y') {
					fmdel(slen);
					fminss(cstring, strlen(cstring));

					return 1;
				}
				if (c != 'n' && c != 'N')
					return 0;
			} else {
			}
		}
		fmpoint(pos + slen);
		y = pos;
		if (fmnrnl())
			fmpoint(pos + 1);
		saddr = pos;
		fmpoint(y);
		return 1;
	}
	fmpoint(y);
	return 0;
}

void search(void)
{
	char ss[80];
	char *s = ss;
	int rest = 0;

	backwards = 0;
	ignore = 0;
	change = 0;
	without = 0;
	ss[0] = 0;

	if (!getl("Search for", ss))
		return;
	if (ss[0])
		strcpy(sstring, ss);
	else if (!sstring[0])
		return;

	ss[0] = 0;
	if (!getl
	    ("Backwards, Ignore case, Change(Without asking), Rest", ss))
		return;
	while (*s) {
		if (*s == 'b' || *s == 'B')
			backwards = 1;
		else if (*s == 'c' || *s == 'C')
			change = 1;
		else if (*s == 'i' || *s == 'I')
			ignore = 1;
		else if (*s == 'w' || *s == 'W')
			without = 1;
		else if (*s == 'r' || *s == 'R')
			rest = 1;
		s++;
	}
	if (change) {
		ss[0] = 0;
		if (!getl("Replace with", ss))
			return;
		if (ss[0])
			strcpy(cstring, ss);
		if (!cstring[0])
			return;
	} else {
		without = 0;
	}
	if (backwards) {
		if (dosb() == -1) {
			msg("Not found");
			return;
		}
	} else {
		if (dosf() == -1) {
			msg("Not found");
			return;
		}
	}
	if (rest) {
		if (backwards)
			while (!dosb());
		else
			while (!dosf());
	}
}

void nextocc(void)
{
	if (!sstring[0]) {
		msg("No search string defined.  Use ^KF to define search string");
		return;
	}
	if (backwards) {
		if (dosb() == -1)
			msg("Not found");
	} else {
		if (dosf() == -1)
			msg("Not found");
	}
}

void setm(void)
{
	mark = pos;
	markf = 1;
}

void writeblk(void)
{
	char gfnam1[160];
	char sting[160];
	int x;
	int y = pos;
	if (!markf) {
		msg("The mark is not set.  Set it with ^KB");
		return;
	}
	gfnam1[0] = 0;
	if (!getl("File to write block to", gfnam1))
		return;
	handle = open(gfnam1, O_WRONLY + O_CREAT + O_TRUNC, 0666);
	if (handle != -1) {
		if (mark > y)
			x = mark - y;
		else {
			fmpoint(mark);
			x = y - mark;
		}
		if (!fmsave(handle, x)) {
			sprintf(sting, "Error writting to file %s;",
				gfnam1);
			msg(sting);
		}
		close(handle);
	} else {
		sprintf(sting, "Error opening file %s;", gfnam1);
		msg(sting);
	}
	fmpoint(y);
}

void cutblk(void)
{
	int x;
	int y = pos;
	if (!markf) {
		msg("The mark is not set.  Set it with ^KB");
		return;
	}
	handle = open("cut.e", O_CREAT + O_TRUNC + O_WRONLY, 0666);
	if (handle != -1) {
		if (mark > y)
			x = mark - y;
		else {
			fmpoint(mark);
			x = y - mark;
			y -= x;
		}
		if (!fmsave(handle, x)) {
			msg("Error writting to file cut.e;");
		}
		fmdel(x);
		fmpoint(y);
		close(handle);
	} else {
		msg("Couldn't open file cut.e;");
	}
}

void cpyblk(void)
{
	int x;
	int y = pos;
	if (!markf) {
		msg("The mark is not set.  Set it with ^KB");
		return;
	}
	handle = open("cut.e", O_WRONLY + O_CREAT + O_TRUNC, 0666);
	if (handle != -1) {
		if (mark > y)
			x = mark - y;
		else {
			fmpoint(mark);
			x = y - mark;
		}
		if (!fmsave(handle, x)) {
			msg("Error writting to file cut.e;");
		}
		fmpoint(y);
		close(handle);
	} else {
		msg("Couldn't open file cut.e;");
	}
}

void insblk(void)
{
	handle = open("cut.e", O_RDONLY);
	if (handle != -1) {
		if (!fminsfil(handle))
			msg("Error reading file cut.e;");
		close(handle);
	} else {
		msg("Error opening file cut.e;");
	}
}

void insfil(void)
{
	char gfnam1[160];
	char sting[160];
	gfnam1[0] = 0;
	if (!getl("File to insert", gfnam1))
		return;
	handle = open(gfnam1, O_RDONLY);
	if (handle != -1) {
		if (!fminsfil(handle)) {
			sprintf(sting, "Error inserting file %s;", gfnam1);
			msg(sting);
		}
		close(handle);
	} else {
		sprintf(sting, "Error opening file %s.", gfnam1);
		msg(sting);
		return;
	}
}

void push(void)
{
	if (vt52)
		conputs("\033Y7 \033K");
	else
		conputs("\033[1;24r\033[24;1H\033[K");
	conputs
	    ("You are now at the command shell.  The editor is still running,\
 however.\015\012");
	conputs("To continue editing, type the command: exit\015\012");
	conflush();

	conclose();
	exe("/bin/csh", 0);

	conopen();
	rewrite();
}

int calcs(void)
{
	int idt = 0;
	char ch;
	bol();
	while (!(pos == edit_file_siz)) {
		ch = fmgetc();
		if (ch == ' ')
			idt++;
		else if (ch == '\t')
			idt += 8 - (idt & 7);
		else {
			fmpoint(pos - 1);
			break;
		}
	}
	return idt;
}

/* Reformat a paragraph */

void reformat(void)
{
	int tmp;
	int idt;
	int first = 0;
	int idt1;
	char ch;
	int g;

/* First, determine indentation on current line */

      up:
	idt = calcs();
	if (fmrc() == '\n') {
		bol();
		dnarw();
		goto up;
	}
	bol();

/* Now find beginning of paragraph */
/* It will be indicated by a change of indentation or a blank line */

	while (pos) {
		uparw();
		idt1 = calcs();
		if (fmrc() == '\n') {
			bol();
			dnarw();
			first = 1;
			break;
		}
		bol();
		if (idt1 > idt)
			break;
		if (idt1 < idt) {
			dnarw();
			first = 1;
			break;
		}
	}

/* Point is now at beginning of paragraph (hopefully) */
/* Set the mark */

	setm();

/* Now move to after end of paragraph */

	while (1) {
		tmp = pos;
		dnarw();
		if (pos == tmp) {
			eol();
			break;
		}
		idt1 = calcs();
		if (fmrc() == '\n') {
			bol();
			break;
		}
		bol();
		if (first) {
			first = 0;
			idt = idt1;
			continue;
		}
		if (idt1 > idt)
			break;
	}

/* Point now after end of paragraph, cut paragraph */
	cutblk();

/* Now reinsert paragraph in a nice way */

	handle = open("cut.e", O_RDONLY);
	if (handle != -1) {
		int oldwrap = wrap;
		int oldoverwrite = overwrite;
		int oldauto = autoind;
		int flag = 0;
		int ccc = 0;
		overwrite = 0;
		wrap = 1;

		while (g = read(handle, &ch, 1)) {
			if (g == -1)
				break;
			if (ch == '\n')
				ch = ' ';
			if (ch == ' ' || ch == '\t') {
				if (flag == 0)
					type(ch);
				else {
					if (flag == 1) {
						type(' ');
						if (!
						    (ccc == '.'
						     || ccc == ':'
						     || ccc == '?'
						     || ccc == '!'
						     || ccc == '\"'
						     || ccc == ';'))
							flag = 2;
					}
				}
			} else {
				flag = 1;
				type(ch);
			}
			ccc = ch;
		}
		autoind = 0;
		if (flag)
			type('\n');
		close(handle);
		wrap = oldwrap;
		overwrite = oldoverwrite;
		autoind = oldauto;
	} else {
		msg("Error opening file cut.e;");
	}
}

void killword(void)
{
	char ch;
	ch = fmrc();
	if (((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
	    && !(pos == edit_file_siz))
		do {
			delch();
			ch = fmrc();
		} while (((ch >= 'a' && ch <= 'z')
			  || (ch >= 'A' && ch <= 'Z'))
			 && !(pos == edit_file_siz));
	else if ((ch != ' ' && ch != '\t' && ch != '\n')
		 && !(pos == edit_file_siz)) {
		do {
			delch();
			ch = fmrc();
		} while (!((ch >= 'a' && ch <= 'z')
			   || (ch >= 'A' && ch <= 'Z'))
			 && !(pos == edit_file_siz)
			 && ch != ' ' && ch != '\n' && ch != '\t');
	} else {
		if (pos) {
			fmpoint(pos - 1);
			ch = fmrc();
			fmpoint(pos + 1);
			if ((ch >= 'a' && ch <= 'z')
			    || (ch >= 'A' && ch <= 'Z')) {
			      up:
				backs();
				if (pos) {
					fmpoint(pos - 1);
					ch = fmrc();
					fmpoint(pos + 1);
					if ((ch >= 'a' && ch <= 'z')
					    || (ch >= 'A' && ch <= 'Z'))
						goto up;
				}
			} else {
				if (ch == ' ' || ch == '\t') {
				      up1:
					backs();
					if (pos) {
						fmpoint(pos - 1);
						ch = fmrc();
						fmpoint(pos + 1);
						if (ch == ' '
						    || ch == '\t')
							goto up1;
					}
				} else {
					if (ch == ')' || ch == '('
					    || ch == '[' || ch == ']'
					    || ch == '{' || ch == '}') {
						backs();
						return;
					}
				      up2:
					backs();
					if (pos) {
						fmpoint(pos - 1);
						ch = fmrc();
						fmpoint(pos + 1);
						if (!
						    (ch == ' '
						     || ch == '\t'
						     || (ch >= 'a'
							 && ch <= 'z')
						     || (ch >= 'A'
							 && ch <= 'Z')
						     || ch == '['
						     || ch == '{'
						     || ch == ']'
						     || ch == '}'
						     || ch == '('
						     || ch == ')'))
							goto up2;
					}
				}
			}
		} else
			delch();
	}
}

void wrdl(void)
{
	if (extend)
		ultarw();
	else {
		if (fmrc() == ' ' || fmrc() == '\t' || fmrc() == '\n') {
			while (pos) {
				ultarw();
				if (!
				    (fmrc() == ' ' || fmrc() == '\t'
				     || fmrc() == '\n'))
					break;
			}
		}
		ultarw();
		if (fmrc() == '\n')
			return;
		goto in;
		while (pos) {
			ultarw();
		      in:
			if (fmrc() == ' ' || fmrc() == '\t') {
				break;
			}
			if (fmrc() == '\n') {
				urtarw();
				break;
			}
		}
	}
}

void wrdr(void)
{
	if (extend)
		urtarw();
	else {
		if (fmrc() == '\n') {
			urtarw();
			return;
		}
		if (fmrc() == ' ' || fmrc() == '\t' || fmrc() == '\n') {
			while (!(pos == edit_file_siz)) {
				urtarw();
				if (!
				    (fmrc() == ' ' || fmrc() == '\t'
				     || fmrc() == '\n'))
					break;
			}
		}
		while (!(pos == edit_file_siz)) {
			urtarw();
			if (fmrc() == ' ' || fmrc() == '\t'
			    || fmrc() == '\n') {
				break;
			}
		}
	}
}

void emode(void)
{
	char buf[80];
	char ch;
      up:
	sprintf(buf, "1 %s 2 %s 3 %s 4 %s: ",
		overwrite ? "Overwrite" : "Insert",
		autoind ? "Auto indent" : "No autoind",
		pic ? "Picture" : "Shrink-wrap",
		wrap ? "Word wrap" : "No wrap");
	ch = query(buf);
	switch (ch) {
	case '1':
		overwrite = !overwrite;
		goto up;
	case '2':
		autoind = !autoind;
		goto up;
	case '3':
		pic = !pic;
		goto up;
	case '4':
		wrap = !wrap;
		goto up;
	}
}

void clmar(void)
{
	char buf[80];
	sprintf(buf, "%d", lmar);
	if (getl("Left margin", buf)) {
		sscanf(buf, "%d", &lmar);
		if (lmar > rmar)
			rmar = lmar;
	}
}

void crmar(void)
{
	char buf[80];
	sprintf(buf, "%d", rmar);
	if (getl("Right margin", buf)) {
		sscanf(buf, "%d", &rmar);
		if (rmar < lmar)
			lmar = rmar;
	}
}

void mar(void)
{
	char ch = query("Change which margin (Left, Right)?");
	if (ch == 'L')
		clmar();
	else if (ch == 'R')
		crmar();
}

/* Command table */

void (*functs[]) () = {

/* Control keys:  notice that there is no Ctrl-@ */

	bol, ultarw, eexit, delch, eol, urtarw, 0,		/* 0x01 - 0x07 */
	backs, 0, deleol, 0, nextocc, 0, udnarw, 0,		/* 0x08 - 0x0f */
	uuparw, 0, rewrite, 0, insblk, pgup, pgdn, killword,	/* 0x10 - 0x17 */
	wrdr, dellin, wrdl, 0, 0, 0, 0, 0,			/* 0x18 - 0x1f */

/* Ctrl-K keys:  Lowercase and control characters are converted to
                 uppercase by ktrueifgotc()
*/
	0, 0, 0, 0, 0, 0, 0, 0,					/* 0x20 - 0x27 */
	0, 0, 0, 0, 0, 0, 0, 0,					/* 0x28 - 0x2f */
	0, 0, 0, 0, 0, 0, 0, 0,					/* 0x30 - 0x37 */
	0, 0, 0, 0, 0, 0, 0, 0,					/* 0x38 - 0x3f */
	0, mar, setm, cpyblk, 0, 0, search, 0,			/* 0x40 - 0x47 */
	thelp, 0, reformat, 0, findline, cutblk, 0, 0,		/* 0x48 - 0x4f */
	0, eexit, insfil, saveit, emode, bof, eof, writeblk,	/* 0x50 - 0x57 */
	exsave, 0, push, 0, 0, 0, 0, 0				/* 0x58 - 0x5f */
};

void edit(void)
{
	int ch;
	void (*f) ();
	dupdate();
	do {
		ch = kgetc();
		if (ch >= 0)
			type(ch), dupdate();
		else {
			f = functs[-1 - ch];
			if (f)
				f(), dupdate();
		}
	} while (!leave);
}

int main(int argc, char *argv[])
{
	char sting[160];
	conopen();
	if (argc > 2) {
		conputs("*** Illegal number of command line arguments\r");
		conputs("\nEditor Command Format:  e [filename]\r\n");
		conclose();
		return 1;
	}
	dopen();
	markf = 0;
	leave = 0;
	pic = 0;
	autoind = 0;
	overwrite = 0;
	wrap = 1;
	extend = 0;
	gfnam[0] = 0;
	if (argc == 2) {
		char *kkkk;
		strcpy(gfnam, argv[1]);
		kkkk = (char *) strchr(gfnam, '.');
		if (kkkk) {
			if (*(kkkk + 1) == 'c' || *(kkkk + 1) == 'p'
			    || *(kkkk + 1) == 'h') {
				autoind = 1;
				wrap = 0;
			}
		}
		handle = open(argv[1], O_RDONLY);
		if (handle != -1) {
			if (!fmopen(handle)) {
				sprintf(sting, "Error reading file %s.",
					gfnam);
				msg(sting);
			}
			close(handle);
		} else {
			if (errno != ENOENT) {
				sprintf(sting, "Error opening file %s;",
					gfnam);
				msg(sting);
			}
			fmopen(0);
		}
	} else
		fmopen(0);
	edit();
	dclose();
	conclose();
	return 0;
}
