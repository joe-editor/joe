/*
 *	Command execution
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <string.h>

#include "b.h"
#include "bw.h"
#include "cmd.h"
#include "hash.h"
#include "help.h"
#include "macro.h"
#include "main.h"
#include "menu.h"
#include "path.h"
#include "poshist.h"
#include "pw.h"
#include "rc.h"
#include "tty.h"
#include "tw.h"
#include "ublock.h"
#include "uedit.h"
#include "uerror.h"
#include "ufile.h"
#include "uformat.h"
#include "uisrch.h"
#include "umath.h"
#include "undo.h"
#include "usearch.h"
#include "ushell.h"
#include "utag.h"
#include "utils.h"
#include "va.h"
#include "vs.h"
#include "w.h"

extern int smode;
int beep = 0;
int uexecmd(BW *bw);

/* Command table */

CMD cmds[] = {
	{"abort", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uabort, NULL, 0, NULL},
	{"abortbuf", TYPETW, uabortbuf, NULL, 0, NULL},
	{"arg", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uarg, NULL, 0, NULL},
	{"ask", TYPETW + TYPEPW, uask, NULL, 0, NULL},
	{"uarg", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uuarg, NULL, 0, NULL},
	{"backs", TYPETW + TYPEPW + ECHKXCOL + EFIXXCOL + EMINOR + EKILL + EMOD, ubacks, NULL, 1, "delch"},
	{"backsmenu", TYPEMENU, umbacks, NULL, 1, NULL},
	{"backw", TYPETW + TYPEPW + ECHKXCOL + EFIXXCOL + EKILL + EMOD, ubackw, NULL, 1, "delw"},
	{"bknd", TYPETW + TYPEPW, ubknd, NULL, 0, NULL},
	{"bkwdc", TYPETW + TYPEPW, ubkwdc, NULL, 1, "fwrdc"},
	{"blkcpy", TYPETW + TYPEPW + EFIXXCOL + EMOD, ublkcpy, NULL, 1, NULL},
	{"blkdel", TYPETW + TYPEPW + EFIXXCOL + EKILL + EMOD, ublkdel, NULL, 0, NULL},
	{"blkmove", TYPETW + TYPEPW + EFIXXCOL + EMOD, ublkmove, NULL, 0, NULL},
	{"blksave", TYPETW + TYPEPW, ublksave, NULL, 0, NULL},
	{"bof", TYPETW + TYPEPW + EMOVE + EFIXXCOL, u_goto_bof, NULL, 0, NULL},
	{"bofmenu", TYPEMENU, umbof, NULL, 0, NULL},
	{"bol", TYPETW + TYPEPW + EFIXXCOL, u_goto_bol, NULL, 0, NULL},
	{"bolmenu", TYPEMENU, umbol, NULL, 0, NULL},
	{"bop", TYPETW + TYPEPW + EFIXXCOL, ubop, NULL, 1, "eop"},
	{"bos", TYPETW + TYPEPW + EMOVE, ubos, NULL, 0, NULL},
	{"bufed", TYPETW, ubufed, NULL, 0, NULL},
	{"byte", TYPETW + TYPEPW, ubyte, NULL, 0, NULL},
	{"center", TYPETW + TYPEPW + EFIXXCOL + EMOD, ucenter, NULL, 1, NULL},
	{"ctrl", TYPETW + TYPEPW + EMOD, uctrl, NULL, 0, NULL},
	{"col", TYPETW + TYPEPW, ucol, NULL, 0, NULL},
	{"complete", TYPETW + TYPEPW + EMINOR + EMOD, ucmplt, NULL, 0, NULL},
	{"copy", TYPETW + TYPEPW, ucopy, NULL, 0, NULL},
	{"crawll", TYPETW + TYPEPW, ucrawll, NULL, 1, "crawlr"},
	{"crawlr", TYPETW + TYPEPW, ucrawlr, NULL, 1, "crawll"},
	{"delbol", TYPETW + TYPEPW + EFIXXCOL + EKILL + EMOD, udelbl, NULL, 1, "deleol"},
	{"delch", TYPETW + TYPEPW + ECHKXCOL + EFIXXCOL + EMINOR + EKILL + EMOD, udelch, NULL, 1, "backs"},
	{"deleol", TYPETW + TYPEPW + EKILL + EMOD, udelel, NULL, 1, "delbol"}, 
	{"dellin", TYPETW + TYPEPW + EFIXXCOL + EKILL + EMOD, udelln, NULL, 1, NULL}, 
	{"delw", TYPETW + TYPEPW + EFIXXCOL + ECHKXCOL + EKILL + EMOD, u_word_delete, NULL, 1, "backw"},
	{"dnarw", TYPETW + TYPEPW + EMOVE, udnarw, NULL, 1, "uparw"},
	{"dnarwmenu", TYPEMENU, umdnarw, NULL, 1, "uparwmenu"}, 
	{"dnslide", TYPETW + TYPEPW + TYPEMENU + TYPEQW + EMOVE, udnslide, NULL, 1, "upslide"},
	{"drop", TYPETW + TYPEPW, udrop, NULL, 0, NULL},
	{"dupw", TYPETW, uduptw, NULL, 0, NULL},
	{"edit", TYPETW + TYPEPW, uedit, NULL, 0, NULL},
	{"eof", TYPETW + TYPEPW + EFIXXCOL + EMOVE, u_goto_eof, NULL, 0, NULL},
	{"eofmenu", TYPEMENU, umeof, NULL, 0, NULL},
	{"eol", TYPETW + TYPEPW + EFIXXCOL, u_goto_eol, NULL, 0, NULL},
	{"eolmenu", TYPEMENU, umeol, NULL, 0, NULL},
	{"eop", TYPETW + TYPEPW + EFIXXCOL, ueop, NULL, 1, "bop"},
	{"execmd", TYPETW + TYPEPW, uexecmd, NULL, 0, NULL},
	{"explode", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uexpld, NULL, 0, NULL},
	{"exsave", TYPETW + TYPEPW, uexsve, NULL, 0, NULL},
	{"ffirst", TYPETW + TYPEPW, pffirst, NULL, 0, NULL},
	{"filt", TYPETW + TYPEPW + EMOD, ufilt, NULL, 0, NULL},
	{"fnext", TYPETW + TYPEPW, pfnext, NULL, 1, NULL},
	{"format", TYPETW + TYPEPW + EFIXXCOL + EMOD, uformat, NULL, 1, NULL},
	{"fmtblk", TYPETW + EMOD + EFIXXCOL, ufmtblk, NULL, 1, NULL},
	{"fwrdc", TYPETW + TYPEPW, ufwrdc, NULL, 1, "bkwdc"},
	{"gomark", TYPETW + TYPEPW + EMOVE, ugomark, NULL, 0, NULL},
	{"groww", TYPETW, ugroww, NULL, 1, "shrinkw"},
	{"isrch", TYPETW + TYPEPW, uisrch, NULL, 0, NULL},
	{"killproc", TYPETW + TYPEPW, ukillpid, NULL, 0, NULL},
	{"help", TYPETW + TYPEPW + TYPEQW, u_help, NULL, 0, NULL},
	{"hnext", TYPETW + TYPEPW + TYPEQW, u_help_next, NULL, 0, NULL},
	{"hprev", TYPETW + TYPEPW + TYPEQW, u_help_prev, NULL, 0, NULL},
	{"insc", TYPETW + TYPEPW + EFIXXCOL + EMOD, uinsc, NULL, 1, "delch"},
	{"insf", TYPETW + TYPEPW + EMOD, uinsf, NULL, 0, NULL}, 
	{"lindent", TYPETW + TYPEPW + EFIXXCOL + EMOD, ulindent, NULL, 1, "rindent"},
	{"line", TYPETW + TYPEPW, uline, NULL, 0, NULL},
	{"lose", TYPETW + TYPEPW, ulose, NULL, 0, NULL}, 
	{"ltarw", TYPETW + TYPEPW + EFIXXCOL + ECHKXCOL, u_goto_left, NULL, 1, "rtarw"},
	{"ltarwmenu", TYPEMENU, umltarw, NULL, 1, "rtarwmenu"},
	{"macros", TYPETW + EFIXXCOL, umacros, NULL, 0, NULL},
	{"markb", TYPETW + TYPEPW, umarkb, NULL, 0, NULL},
	{"markk", TYPETW + TYPEPW, umarkk, NULL, 0, NULL},
	{"markl", TYPETW + TYPEPW, umarkl, NULL, 0, NULL},
	{"math", TYPETW + TYPEPW, umath, NULL, 0, NULL},
	{"mode", TYPETW + TYPEPW + TYPEQW, umode, NULL, 0, NULL},
	{"msg", TYPETW + TYPEPW + TYPEQW + TYPEMENU, umsg, NULL, 0, NULL},
	{"nbuf", TYPETW, unbuf, NULL, 1, "pbuf"},
	{"nedge", TYPETW + TYPEPW + EFIXXCOL, unedge, NULL, 1, "pedge"}, 
	{"nextpos", TYPETW + TYPEPW + EFIXXCOL + EMID + EPOS, unextpos, NULL, 1, "prevpos"}, 
	{"nextw", TYPETW + TYPEPW + TYPEMENU + TYPEQW, unextw, NULL, 1, "prevw"},
	{"nextword", TYPETW + TYPEPW + EFIXXCOL, u_goto_next, NULL, 1, "prevword"},
	{"nmark", TYPETW + TYPEPW, unmark, NULL, 0, NULL},
	{"notmod", TYPETW, unotmod, NULL, 0, NULL},
	{"nxterr", TYPETW, unxterr, NULL, 1, "prverr"},
	{"open", TYPETW + TYPEPW + EFIXXCOL + EMOD, uopen, NULL, 1, "deleol"},
	{"parserr", TYPETW, uparserr, NULL, 0, NULL},
	{"pbuf", TYPETW, upbuf, NULL, 1, "nbuf"},
	{"pedge", TYPETW + TYPEPW + EFIXXCOL, upedge, NULL, 1, "nedge"}, 
	{"pgdn", TYPETW + TYPEPW + TYPEMENU + TYPEQW + EMOVE, upgdn, NULL, 1, "pgup"},
	{"pgup", TYPETW + TYPEPW + TYPEMENU + TYPEQW + EMOVE, upgup, NULL, 1, "pgdn"},
	{"picokill", TYPETW + TYPEPW + EFIXXCOL + EKILL + EMOD, upicokill, NULL, 1, NULL},
	{"play", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uplay, NULL, 1, NULL},	/* EFIXX? */ 
	{"prevpos", TYPETW + TYPEPW + EPOS + EMID + EFIXXCOL, uprevpos, NULL, 1, "nextpos"}, 
	{"prevw", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uprevw, NULL, 1, "nextw"}, 
	{"prevword", TYPETW + TYPEPW + EFIXXCOL + ECHKXCOL, u_goto_prev, NULL, 1, "nextword"},
	{"prverr", TYPETW, uprverr, NULL, 1, "nxterr"},
	{"psh", TYPETW + TYPEPW + TYPEMENU + TYPEQW, upsh, NULL, 0, NULL},
	{"pop", TYPETW + TYPEPW + TYPEMENU + TYPEQW, upop, NULL, 0, NULL},
	{"qrepl", TYPETW + TYPEPW + EMOD, pqrepl, NULL, 0, NULL},
	{"query", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uquery, NULL, 0, NULL},
	{"quote", TYPETW + TYPEPW + EMOD, uquote, NULL, 0, NULL},
	{"quote8", TYPETW + TYPEPW + EMOD, uquote8, NULL, 0, NULL},
	{"record", TYPETW + TYPEPW + TYPEMENU + TYPEQW, urecord, NULL, 0, NULL},
	{"redo", TYPETW + TYPEPW + EFIXXCOL, uredo, NULL, 1, "undo"},
	{"retype", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uretyp, NULL, 0, NULL},
	{"rfirst", TYPETW + TYPEPW, prfirst, NULL, 0, NULL}, 
	{"rindent", TYPETW + TYPEPW + EFIXXCOL + EMOD, urindent, NULL, 1, "lindent"},
	{"run", TYPETW + TYPEPW, urun, NULL, 0, NULL},
	{"rsrch", TYPETW + TYPEPW, ursrch, NULL, 0, NULL},
	{"rtarw", TYPETW + TYPEPW + EFIXXCOL, u_goto_right, NULL, 1, "ltarw"},
	{"rtarwmenu", TYPEMENU, umrtarw, NULL, 1, "ltarwmenu"},
	{"rtn", TYPETW + TYPEPW + TYPEMENU + TYPEQW + EMOD, urtn, NULL, 1, NULL},
	{"save", TYPETW + TYPEPW, usave, NULL, 0, NULL},
	{"setmark", TYPETW + TYPEPW, usetmark, NULL, 0, NULL},
	{"shell", TYPETW + TYPEPW + TYPEMENU + TYPEQW, ushell, NULL, 0, NULL},
	{"shrinkw", TYPETW, ushrnk, NULL, 1, "groww"},
	{"splitw", TYPETW, usplitw, NULL, 0, NULL},
	{"stat", TYPETW + TYPEPW, ustat, NULL, 0, NULL},
	{"stop", TYPETW + TYPEPW + TYPEMENU + TYPEQW, ustop, NULL, 0, NULL},
	{"swap", TYPETW + TYPEPW + EFIXXCOL, uswap, NULL, 0, NULL},
	{"tag", TYPETW + TYPEPW, utag, NULL, 0, NULL},
	{"tomarkb", TYPETW + TYPEPW + EFIXXCOL, utomarkb, NULL, 0, NULL},
	{"tomarkbk", TYPETW + TYPEPW + EFIXXCOL, utomarkbk, NULL, 0, NULL},
	{"tomarkk", TYPETW + TYPEPW + EFIXXCOL, utomarkk, NULL, 0, NULL},
	{"tomatch", TYPETW + TYPEPW + EFIXXCOL, utomatch, NULL, 0, NULL},
	{"tos", TYPETW + TYPEPW + EMOVE, utos, NULL, 0, NULL},
	{"tw0", TYPETW + TYPEPW + TYPEQW + TYPEMENU, utw0, NULL, 0, NULL},
	{"tw1", TYPETW + TYPEPW + TYPEQW + TYPEMENU, utw1, NULL, 0, NULL},
	{"txt", TYPETW + TYPEPW, utxt, NULL, 0, NULL}, 
	{"type", TYPETW + TYPEPW + TYPEQW + TYPEMENU + EMINOR + EMOD, utype, NULL, 1, "backs"},
	{"undo", TYPETW + TYPEPW + EFIXXCOL, uundo, NULL, 1, "redo"},
	{"uparw", TYPETW + TYPEPW + EMOVE, uuparw, NULL, 1, "dnarw"},
	{"uparwmenu", TYPEMENU, umuparw, NULL, 1, "dnarwmenu"}, 
	{"upslide", TYPETW + TYPEPW + TYPEMENU + TYPEQW + EMOVE, uupslide, NULL, 1, "dnslide"},
	{"yank", TYPETW + TYPEPW + EFIXXCOL + EMOD, uyank, NULL, 1, NULL},
	{"yapp", TYPETW + TYPEPW + EKILL, uyapp, NULL, 0, NULL},
	{"yankpop", TYPETW + TYPEPW + EFIXXCOL + EMOD, uyankpop, NULL, 1, NULL}
};

/* Execute a command n with key k */

int execmd(CMD *cmd, int k)
{
	BW *bw = (BW *) maint->curwin->object;
	int ret = -1;

	if (cmd->m)
		return exmacro(cmd->m, 0);

	/* We don't execute if we have to fix the column position first
	 * (i.e., left arrow when cursor is in middle of nowhere) */
	if ((cmd->flag & ECHKXCOL)
	    && bw->cursor->xcol != piscol(bw->cursor))
		goto skip;

	/* Don't execute command if we're in wrong type of window */
	if (!(cmd->flag & maint->curwin->watom->what))
		goto skip;

	if ((maint->curwin->watom->what & TYPETW) && bw->b->rdonly && (cmd->flag & EMOD)) {
		msgnw(bw->parent, "Read only");
		if (beep)
			ttputc(7);
		goto skip;
	}

	/* Execute command */
	ret = cmd->func(maint->curwin->object, k);

	if (smode)
		--smode;

	/* Don't update anything if we're going to leave */
	if (leave)
		return 0;

	bw = (BW *) maint->curwin->object;

	/* Maintain position history */
	/* If command was not a positioning command */
	if (!(cmd->flag & EPOS)
	    && (maint->curwin->watom->what & (TYPETW | TYPEPW)))
		afterpos();

	/* If command was not a movement */
	if (!(cmd->flag & (EMOVE | EPOS)) && (maint->curwin->watom->what & (TYPETW | TYPEPW)))
		aftermove(maint->curwin, bw->cursor);

	if (cmd->flag & EKILL)
		justkilled = 1;
	else
		justkilled = 0;

      skip:

	/* Make dislayed cursor column equal the actual cursor column
	 * for commands which arn't simple vertical movements */
	if (cmd->flag & EFIXXCOL)
		bw->cursor->xcol = piscol(bw->cursor);

	/* Recenter cursor to middle of screen */
	if (cmd->flag & EMID) {
		int omid = mid;

		mid = 1;
		dofollows();
		mid = omid;
	}

	if (beep && ret)
		ttputc(7);
	return ret;
}

/* Return command table index for given command name */

HASH *cmdhash = NULL;

static void izcmds(void)
{
	int x;

	cmdhash = htmk(256);
	for (x = 0; x != sizeof(cmds) / sizeof(CMD); ++x)
		htadd(cmdhash, cmds[x].name, cmds + x);
}

CMD *findcmd(char *s)
{
	if (!cmdhash)
		izcmds();
	return (CMD *) htfind(cmdhash, s);
}

void addcmd(unsigned char *s, MACRO *m)
{
	CMD *cmd = (CMD *) joe_malloc(sizeof(CMD));

	if (!cmdhash)
		izcmds();
	cmd->name = strdup(s);
	cmd->flag = 0;
	cmd->func = NULL;
	cmd->m = m;
	cmd->arg = 1;
	cmd->negarg = NULL;
	htadd(cmdhash, cmd->name, cmd);
}

static char **getcmds(void)
{
	char **s = vaensure(NULL, sizeof(cmds) / sizeof(CMD));
	int x;
	HENTRY *e;

	for (x = 0; x != cmdhash->len; ++x)
		for (e = cmdhash->tab[x]; e; e = e->next)
			s = vaadd(s, vsncpy(NULL, 0, sz(e->name)));
	vasort(s, aLen(s));
	return s;
}

/* Command line */

char **scmds = NULL;

static char **regsub(char **z, int len, char *s)
{
	char **lst = NULL;
	int x;

	for (x = 0; x != len; ++x)
		if (rmatch(s, z[x]))
			lst = vaadd(lst, vsncpy(NULL, 0, sz(z[x])));
	return lst;
}

static void inscmd(BW *bw, char *line)
{
	P *p = pdup(bw->cursor);

	p_goto_bol(p);
	p_goto_eol(bw->cursor);
	bdel(p, bw->cursor);
	binsm(bw->cursor, sv(line));
	p_goto_eol(bw->cursor);
	prm(p);
	bw->cursor->xcol = piscol(bw->cursor);
}

static int cmdabrt(BW *bw, int x, char *line)
{
	if (line) {
		inscmd(bw, line);
		vsrm(line);
	}
	return -1;
}

static int cmdrtn(MENU *m, int x, char *line)
{
	inscmd(m->parent->win->object, m->list[x]);
	vsrm(line);
	m->object = NULL;
	wabort(m->parent);
	return 0;
}

static int cmdcmplt(BW *bw)
{
	MENU *m;
	P *p, *q;
	char *line;
	char *line1;
	char **lst;

	if (!scmds)
		scmds = getcmds();
	p = pdup(bw->cursor);
	p_goto_bol(p);
	q = pdup(bw->cursor);
	p_goto_eol(q);
	line = brvs(p, (int) (q->byte - p->byte));	/* Assumes short lines :-) */
	prm(p);
	prm(q);
	m = mkmenu(bw->parent, NULL, cmdrtn, cmdabrt, NULL, 0, line, NULL);
	if (!m)
		return -1;
	line1 = vsncpy(NULL, 0, sv(line));
	line1 = vsadd(line1, '*');
	lst = regsub(scmds, aLEN(scmds), line1);
	vsrm(line1);
	ldmenu(m, lst, 0);
	if (!lst) {
		wabort(m->parent);
		ttputc(7);
		return -1;
	} else {
		if (aLEN(lst) == 1)
			return cmdrtn(m, 0, line);
		else if (smode || isreg(line))
			return 0;
		else {
			char *com = mcomplete(m);

			vsrm(m->object);
			m->object = com;
			wabort(m->parent);
			smode = 2;
			ttputc(7);
			return 0;
		}
	}
}

static int docmd(BW *bw, char *s, void *object, int *notify)
{
	MACRO *mac;
	int ret = -1;
	CMD *cmd = findcmd(s);

	if (!cmd)
		msgnw(bw->parent, "No such command");
	else {
		mac = mkmacro(MAXINT, 0, 0, cmd);
		ret = exmacro(mac, 1);
		rmmacro(mac);
	}
	if (notify)
		*notify = 1;
	return ret;
}

B *cmdhist = NULL;

int uexecmd(BW *bw)
{
	if (wmkpw(bw->parent, "cmd: ", &cmdhist, docmd, "cmd", NULL, cmdcmplt, NULL, NULL)) {
		return 0;
	} else {
		return -1;
	}
}
