/*
 *	Command execution
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"


#include "b.h"
#include "bw.h"
#include "cmd.h"
#include "mouse.h"
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
#include "utf8.h"
#include "kbd.h"
#include "w.h"

extern int nowmarking;
extern int smode;
int joe_beep = 0;
int uexecmd(BW *bw);

/* Command table */

int ubeep(BW *bw, int k)
{
	ttputc(7);
	return 0;
}

CMD cmds[] = {
	{US "abort", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uabort, NULL, 0, NULL},
	{US "abortbuf", TYPETW, uabortbuf, NULL, 0, NULL},
	{US "arg", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uarg, NULL, 0, NULL},
	{US "ask", TYPETW + TYPEPW, uask, NULL, 0, NULL},
	{US "uarg", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uuarg, NULL, 0, NULL},
	{US "backs", TYPETW + TYPEPW + ECHKXCOL + EFIXXCOL + EMINOR + EKILL + EMOD, ubacks, NULL, 1, US "delch"},
	{US "backsmenu", TYPEMENU, umbacks, NULL, 1, NULL},
	{US "backw", TYPETW + TYPEPW + ECHKXCOL + EFIXXCOL + EKILL + EMOD, ubackw, NULL, 1, US "delw"},
	{US "beep", TYPETW + TYPEPW + TYPEMENU + TYPEQW, ubeep, NULL, 0, NULL},
	{US "begin_marking", TYPETW + TYPEPW, ubegin_marking, NULL, 0, NULL},
	{US "bknd", TYPETW, ubknd, NULL, 0, NULL},
	{US "bkwdc", TYPETW + TYPEPW, ubkwdc, NULL, 1, US "fwrdc"},
	{US "blkcpy", TYPETW + TYPEPW + EFIXXCOL + EMOD + EBLOCK, ublkcpy, NULL, 1, NULL},
	{US "blkdel", TYPETW + TYPEPW + EFIXXCOL + EKILL + EMOD + EBLOCK, ublkdel, NULL, 0, NULL},
	{US "blkmove", TYPETW + TYPEPW + EFIXXCOL + EMOD + EBLOCK, ublkmove, NULL, 0, NULL},
	{US "blksave", TYPETW + TYPEPW + EBLOCK, ublksave, NULL, 0, NULL},
	{US "bof", TYPETW + TYPEPW + EMOVE + EFIXXCOL, u_goto_bof, NULL, 0, NULL},
	{US "bofmenu", TYPEMENU, umbof, NULL, 0, NULL},
	{US "bol", TYPETW + TYPEPW + EFIXXCOL, u_goto_bol, NULL, 0, NULL},
	{US "bolmenu", TYPEMENU, umbol, NULL, 0, NULL},
	{US "bop", TYPETW + TYPEPW + EFIXXCOL, ubop, NULL, 1, US "eop"},
	{US "bos", TYPETW + TYPEPW + EMOVE, ubos, NULL, 0, NULL},
	{US "bufed", TYPETW, ubufed, NULL, 0, NULL},
	{US "build", TYPETW + TYPEPW, ubuild, NULL, 0, NULL},
	{US "byte", TYPETW + TYPEPW, ubyte, NULL, 0, NULL},
	{US "cancel", TYPETW + TYPEPW + TYPEMENU + TYPEQW, ucancel, NULL, 0, NULL},
	{US "center", TYPETW + TYPEPW + EFIXXCOL + EMOD, ucenter, NULL, 1, NULL},
	{US "ctrl", TYPETW + TYPEPW + EMOD, uctrl, NULL, 0, NULL},
	{US "col", TYPETW + TYPEPW, ucol, NULL, 0, NULL},
	{US "complete", TYPETW + TYPEPW + EMINOR + EMOD, ucmplt, NULL, 0, NULL},
	{US "copy", TYPETW + TYPEPW, ucopy, NULL, 0, NULL},
	{US "crawll", TYPETW + TYPEPW, ucrawll, NULL, 1, US "crawlr"},
	{US "crawlr", TYPETW + TYPEPW, ucrawlr, NULL, 1, US "crawll"},
	{US "defmdown", TYPETW+TYPEPW+TYPEQW+TYPEMENU, udefmdown, 0, 0, 0 },
	{US "defmup", TYPETW+TYPEPW, udefmup, 0, 0, 0 },
	{US "defmdrag", TYPETW+TYPEPW, udefmdrag, 0, 0, 0 },
	{US "defm2down", TYPETW+TYPEPW+TYPEMENU, udefm2down, 0, 0, 0 },
	{US "defm2up", TYPETW+TYPEPW, udefm2up, 0, 0, 0 },
	{US "defm2drag", TYPETW+TYPEPW, udefm2drag, 0, 0, 0 },
	{US "defm3down", TYPETW+TYPEPW, udefm3down, 0, 0, 0 },
	{US "defm3up", TYPETW+TYPEPW, udefm3up, 0, 0, 0 },
	{US "defm3drag", TYPETW+TYPEPW, udefm3drag, 0, 0, 0 },
	{US "delbol", TYPETW + TYPEPW + EFIXXCOL + EKILL + EMOD, udelbl, NULL, 1, US "deleol"},
	{US "delch", TYPETW + TYPEPW + ECHKXCOL + EFIXXCOL + EMINOR + EKILL + EMOD, udelch, NULL, 1, US "backs"},
	{US "deleol", TYPETW + TYPEPW + EKILL + EMOD, udelel, NULL, 1, US "delbol"}, 
	{US "dellin", TYPETW + TYPEPW + EFIXXCOL + EKILL + EMOD, udelln, NULL, 1, NULL}, 
	{US "delw", TYPETW + TYPEPW + EFIXXCOL + ECHKXCOL + EKILL + EMOD, u_word_delete, NULL, 1, US "backw"},
	{US "dnarw", TYPETW + TYPEPW + EMOVE, udnarw, NULL, 1, US "uparw"},
	{US "dnarwmenu", TYPEMENU, umdnarw, NULL, 1, US "uparwmenu"}, 
	{US "dnslide", TYPETW + TYPEPW + TYPEMENU + TYPEQW + EMOVE, udnslide, NULL, 1, US "upslide"},
	{US "dnslidemenu", TYPEMENU, umscrdn, NULL, 1, US "upslidemenu"},
	{US "drop", TYPETW + TYPEPW, udrop, NULL, 0, NULL},
	{US "dupw", TYPETW, uduptw, NULL, 0, NULL},
	{US "edit", TYPETW, uedit, NULL, 0, NULL},
	{US "else", TYPETW+TYPEPW+TYPEMENU+TYPEQW+EMETA, uelse, 0, 0, 0 },
	{US "elsif", TYPETW+TYPEPW+TYPEMENU+TYPEQW+EMETA, uelsif, 0, 0, 0 },
	{US "endif", TYPETW+TYPEPW+TYPEMENU+TYPEQW+EMETA, uendif, 0, 0, 0 },
	{US "eof", TYPETW + TYPEPW + EFIXXCOL + EMOVE, u_goto_eof, NULL, 0, NULL},
	{US "eofmenu", TYPEMENU, umeof, NULL, 0, NULL},
	{US "eol", TYPETW + TYPEPW + EFIXXCOL, u_goto_eol, NULL, 0, NULL},
	{US "eolmenu", TYPEMENU, umeol, NULL, 0, NULL},
	{US "eop", TYPETW + TYPEPW + EFIXXCOL, ueop, NULL, 1, US "bop"},
	{US "execmd", TYPETW + TYPEPW, uexecmd, NULL, 0, NULL},
	{US "explode", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uexpld, NULL, 0, NULL},
	{US "exsave", TYPETW + TYPEPW, uexsve, NULL, 0, NULL},
	{US "ffirst", TYPETW + TYPEPW, pffirst, NULL, 0, NULL},
	{US "filt", TYPETW + TYPEPW + EMOD + EBLOCK, ufilt, NULL, 0, NULL},
	{US "finish", TYPETW + TYPEPW + EMOD, ufinish, NULL, 1, NULL},
	{US "fnext", TYPETW + TYPEPW, pfnext, NULL, 1, NULL},
	{US "format", TYPETW + TYPEPW + EFIXXCOL + EMOD, uformat, NULL, 1, NULL},
	{US "fmtblk", TYPETW + EMOD + EFIXXCOL + EBLOCK, ufmtblk, NULL, 1, NULL},
	{US "fwrdc", TYPETW + TYPEPW, ufwrdc, NULL, 1, US "bkwdc"},
	{US "gomark", TYPETW + TYPEPW + EMOVE, ugomark, NULL, 0, NULL},
	{US "grep", TYPETW, ugrep, NULL, 0, NULL},
	{US "groww", TYPETW, ugroww, NULL, 1, US "shrinkw"},
	{US "if", TYPETW+TYPEPW+TYPEMENU+TYPEQW+EMETA, uif, 0, 0, 0 },
	{US "isrch", TYPETW + TYPEPW, uisrch, NULL, 0, NULL},
	{US "jump", TYPETW, ujump, NULL, 0, NULL },
	{US "killjoe", TYPETW + TYPEPW + TYPEMENU + TYPEQW, ukilljoe, NULL, 0, NULL},
	{US "killproc", TYPETW + TYPEPW, ukillpid, NULL, 0, NULL},
	{US "help", TYPETW + TYPEPW + TYPEQW, u_help, NULL, 0, NULL},
	{US "home", TYPETW + TYPEPW + EFIXXCOL, uhome, NULL, 0, NULL},
	{US "hnext", TYPETW + TYPEPW + TYPEQW, u_help_next, NULL, 0, NULL},
	{US "hprev", TYPETW + TYPEPW + TYPEQW, u_help_prev, NULL, 0, NULL},
	{US "insc", TYPETW + TYPEPW + EFIXXCOL + EMOD, uinsc, NULL, 1, US "delch"},
	{US "keymap", TYPETW, ukeymap, 0, 0, 0 },    /* JM */
	{US "insf", TYPETW + TYPEPW + EMOD, uinsf, NULL, 0, NULL}, 
	{US "lindent", TYPETW + TYPEPW + EFIXXCOL + EMOD + EBLOCK, ulindent, NULL, 1, US "rindent"},
	{US "line", TYPETW + TYPEPW, uline, NULL, 0, NULL},
	{US "lose", TYPETW + TYPEPW, ulose, NULL, 0, NULL}, 
	{US "lower", TYPETW + TYPEPW + EMOD + EBLOCK, ulower, NULL, 0, NULL},
	{US "ltarw", TYPETW + TYPEPW /* + EFIXXCOL + ECHKXCOL */, u_goto_left, NULL, 1, US "rtarw"},
	{US "ltarwmenu", TYPEMENU, umltarw, NULL, 1, US "rtarwmenu"},
	{US "macros", TYPETW + EFIXXCOL, umacros, NULL, 0, NULL},
	{US "markb", TYPETW + TYPEPW, umarkb, NULL, 0, NULL},
	{US "markk", TYPETW + TYPEPW, umarkk, NULL, 0, NULL},
	{US "markl", TYPETW + TYPEPW, umarkl, NULL, 0, NULL},
	{US "math", TYPETW + TYPEPW, umath, NULL, 0, NULL},
	{US "mode", TYPETW + TYPEPW + TYPEQW, umode, NULL, 0, NULL},
	{US "msg", TYPETW + TYPEPW + TYPEQW + TYPEMENU, umsg, NULL, 0, NULL},
	{US "name", TYPETW + TYPEPW, uname_joe, NULL, 0, NULL}, 
	{US "nbuf", TYPETW, unbuf, NULL, 1, US "pbuf"},
	{US "nedge", TYPETW + TYPEPW + EFIXXCOL, unedge, NULL, 1, US "pedge"}, 
	{US "nextpos", TYPETW + TYPEPW + EFIXXCOL + EMID + EPOS, unextpos, NULL, 1, US "prevpos"}, 
	{US "nextw", TYPETW + TYPEPW + TYPEMENU + TYPEQW, unextw, NULL, 1, US "prevw"},
	{US "nextword", TYPETW + TYPEPW + EFIXXCOL, u_goto_next, NULL, 1, US "prevword"},
	{US "nmark", TYPETW + TYPEPW, unmark, NULL, 0, NULL},
	{US "notmod", TYPETW, unotmod, NULL, 0, NULL},
	{US "nxterr", TYPETW, unxterr, NULL, 1, US "prverr"},
	{US "open", TYPETW + TYPEPW + EFIXXCOL + EMOD, uopen, NULL, 1, US "deleol"},
	{US "parserr", TYPETW, uparserr, NULL, 0, NULL},
	{US "paste", TYPETW + TYPEPW + EMOD, upaste, NULL, 0, NULL },
	{US "pbuf", TYPETW, upbuf, NULL, 1, US "nbuf"},
	{US "pedge", TYPETW + TYPEPW + EFIXXCOL, upedge, NULL, 1, US "nedge"}, 
	{US "pgdn", TYPETW + TYPEPW + TYPEMENU + TYPEQW + EMOVE, upgdn, NULL, 1, US "pgup"},
	{US "pgdnmenu", TYPEMENU, umpgdn, NULL, 1, US "pgupmenu"}, 
	{US "pgup", TYPETW + TYPEPW + TYPEMENU + TYPEQW + EMOVE, upgup, NULL, 1, US "pgdn"},
	{US "pgupmenu", TYPEMENU, umpgup, NULL, 1, US "pgdnmenu"}, 
	{US "picokill", TYPETW + TYPEPW + EFIXXCOL + EKILL + EMOD, upicokill, NULL, 1, NULL},
	{US "play", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uplay, NULL, 1, NULL},	/* EFIXX? */ 
	{US "prevpos", TYPETW + TYPEPW + EPOS + EMID + EFIXXCOL, uprevpos, NULL, 1, US "nextpos"}, 
	{US "prevw", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uprevw, NULL, 1, US "nextw"}, 
	{US "prevword", TYPETW + TYPEPW + EFIXXCOL + ECHKXCOL, u_goto_prev, NULL, 1, US "nextword"},
	{US "prverr", TYPETW, uprverr, NULL, 1, US "nxterr"},
	{US "psh", TYPETW + TYPEPW + TYPEMENU + TYPEQW, upsh, NULL, 0, NULL},
	{US "pop", TYPETW + TYPEPW + TYPEMENU + TYPEQW, upop, NULL, 0, NULL},
	{US "qrepl", TYPETW + TYPEPW + EMOD, pqrepl, NULL, 0, NULL},
	{US "query", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uquery, NULL, 0, NULL},
	{US "querysave", TYPETW, uquerysave, NULL, 0, NULL},
	{US "quote", TYPETW + TYPEPW + EMOD, uquote, NULL, 0, NULL},
	{US "quote8", TYPETW + TYPEPW + EMOD, uquote8, NULL, 0, NULL},
	{US "record", TYPETW + TYPEPW + TYPEMENU + TYPEQW, urecord, NULL, 0, NULL},
	{US "redo", TYPETW + TYPEPW + EFIXXCOL, uredo, NULL, 1, US "undo"},
	{US "retype", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uretyp, NULL, 0, NULL},
	{US "rfirst", TYPETW + TYPEPW, prfirst, NULL, 0, NULL}, 
	{US "rindent", TYPETW + TYPEPW + EFIXXCOL + EMOD + EBLOCK, urindent, NULL, 1, US "lindent"},
	{US "run", TYPETW + TYPEPW, urun, NULL, 0, NULL},
	{US "rsrch", TYPETW + TYPEPW, ursrch, NULL, 0, NULL},
	{US "rtarw", TYPETW + TYPEPW /* + EFIXXCOL */, u_goto_right, NULL, 1, US "ltarw"}, /* EFIX removed for picture mode */
	{US "rtarwmenu", TYPEMENU, umrtarw, NULL, 1, US "ltarwmenu"},
	{US "rtn", TYPETW + TYPEPW + TYPEMENU + TYPEQW + EMOD, urtn, NULL, 1, NULL},
	{US "save", TYPETW + TYPEPW, usave, NULL, 0, NULL},
	{US "savenow", TYPETW + TYPEPW, usavenow, NULL, 0, NULL},
	{US "scratch", TYPETW + TYPEPW, uscratch, NULL, 0, NULL},
	{US "select", TYPETW + TYPEPW, uselect, NULL, 0, NULL},
	{US "setmark", TYPETW + TYPEPW, usetmark, NULL, 0, NULL},
	{US "shell", TYPETW + TYPEPW + TYPEMENU + TYPEQW, ushell, NULL, 0, NULL},
	{US "shrinkw", TYPETW, ushrnk, NULL, 1, US "groww"},
	{US "splitw", TYPETW, usplitw, NULL, 0, NULL},
	{US "stat", TYPETW + TYPEPW, ustat, NULL, 0, NULL},
	{US "stop", TYPETW + TYPEPW + TYPEMENU + TYPEQW, ustop, NULL, 0, NULL},
	{US "swap", TYPETW + TYPEPW + EFIXXCOL, uswap, NULL, 0, NULL},
	{US "switch", TYPETW + TYPEPW, uswitch, NULL, 0, NULL},
	{US "sys", TYPETW + TYPEPW, usys, NULL, 0, NULL },
	{US "tabmenu", TYPEMENU, umtab, NULL, 1, US "ltarwmenu"},
	{US "tag", TYPETW + TYPEPW, utag, NULL, 0, NULL},
	{US "toggle_marking", TYPETW + TYPEPW, utoggle_marking, NULL, 0, NULL},
	{US "then", TYPEPW+EMOD, urtn, 0, 0, 0 },
	{US "tomarkb", TYPETW + TYPEPW + EFIXXCOL + EBLOCK, utomarkb, NULL, 0, NULL},
	{US "tomarkbk", TYPETW + TYPEPW + EFIXXCOL + EBLOCK, utomarkbk, NULL, 0, NULL},
	{US "tomarkk", TYPETW + TYPEPW + EFIXXCOL + EBLOCK, utomarkk, NULL, 0, NULL},
	{US "tomatch", TYPETW + TYPEPW + EFIXXCOL, utomatch, NULL, 0, NULL},
	{US "tomouse", TYPETW+TYPEPW+TYPEQW+TYPEMENU, utomouse, 0, 0, 0 },
	{US "tos", TYPETW + TYPEPW + EMOVE, utos, NULL, 0, NULL},
	{US "tw0", TYPETW + TYPEPW + TYPEQW + TYPEMENU, utw0, NULL, 0, NULL},
	{US "tw1", TYPETW + TYPEPW + TYPEQW + TYPEMENU, utw1, NULL, 0, NULL},
	{US "txt", TYPETW + TYPEPW, utxt, NULL, 0, NULL}, 
	{US "type", TYPETW + TYPEPW + TYPEQW + TYPEMENU + EMINOR + EMOD, utype, NULL, 1, US "backs"},
	{US "undo", TYPETW + TYPEPW + EFIXXCOL, uundo, NULL, 1, US "redo"},
	{US "uparw", TYPETW + TYPEPW + EMOVE, uuparw, NULL, 1, US "dnarw"},
	{US "uparwmenu", TYPEMENU, umuparw, NULL, 1, US "dnarwmenu"}, 
	{US "upper", TYPETW + TYPEPW + EMOD + EBLOCK, uupper, NULL, 0, NULL},
	{US "upslide", TYPETW + TYPEPW + TYPEMENU + TYPEQW + EMOVE, uupslide, NULL, 1, US "dnslide"},
	{US "upslidemenu", TYPEMENU, umscrup, NULL, 1, US "dnslidemenu"},
	{US "xtmouse", TYPETW+TYPEPW+TYPEMENU+TYPEQW, uxtmouse, 0, 0, 0 },
	{US "yank", TYPETW + TYPEPW + EFIXXCOL + EMOD, uyank, NULL, 1, NULL},
	{US "yapp", TYPETW + TYPEPW + EKILL, uyapp, NULL, 0, NULL},
	{US "yankpop", TYPETW + TYPEPW + EFIXXCOL + EMOD, uyankpop, NULL, 1, NULL}
};

/* Steal Lock dialog */

int nolocks;

int steal_lock(BW *bw,int c,B *b,int *notify)
{
	if (c=='s' || c=='S') {
		unsigned char bf1[256];
		unsigned char bf[300];
		unlock_it(b->name);
		if (lock_it(b->name,bf1)) {
			int x;
			for(x=0;bf1[x] && bf1[x]!=':';++x);
			bf1[x]=0;
			if(bf1[0])
				joe_snprintf_1((char *)bf,sizeof(bf),"Locked by %s  (S)teal, (I)gnore, (Q)uit? ",bf1);
			else
				joe_snprintf_0((char *)bf,sizeof(bf),"Could not create lock.  (S)teal, (I)gnore, (Q)uit? ");
			if (mkqw(bw->parent, sz(bf), steal_lock, NULL, b, notify)) {
				return 0;
			} else {
				if (notify)
					*notify = -1;
				return -1;
			}
		} else {
			b->locked=1;
			if (notify)
				*notify = 1;
			return 0;
		}
	} else if (c=='i' || c=='I') {
		b->locked=1;
		b->ignored_lock=1;
		if (notify)
			*notify = 1;
		return 0;
	} else if (c=='q' || c=='Q') {
		if (notify)
			*notify = 1;
		return 0;
	} else {
		if (mkqw(bw->parent, sc("Could not lock.  (S)teal, (I)gnore, (Q)uit? "), steal_lock, NULL, b, notify)) {
			return 0;
		} else
			return -1;
	}
}

int file_changed(BW *bw,int c,B *b,int *notify)
{
	if (mkqw(bw->parent, sc("Notice: File on disk changed! (hit ^C to continue)  "), file_changed, NULL, b, notify)) {
		return 0;
	} else
		return -1;
}

/* Try to lock: start dialog if we can't.  Returns 0 if we couldn't lock */

int try_lock(BW *bw,B *b)
{
	/* First time we modify the file */
	/* If we're a plain file, acquire lock */
	if (!nolocks && plain_file(b)) {
		unsigned char bf1[256];
		unsigned char bf[300];
		int x;
		/* It's a plain file- try to lock it */
		if (lock_it(b->name,bf1)) {
			for(x=0;bf1[x] && bf1[x]!=':';++x);
			bf1[x]=0;
			if(bf1[0])
				joe_snprintf_1((char *)bf,sizeof(bf),"Locked by %s  (S)teal, (I)gnore, (Q)uit? ",bf1);
			else
				joe_snprintf_0((char *)bf,sizeof(bf),"Could not create lock.  (S)teal, (I)gnore, (Q)uit? ");
			if (mkqw(bw->parent, sz(bf), steal_lock, NULL, b, NULL)) {
				uquery(bw);
				if (!b->locked)
					return 0;
			} else
				return 0;
		} else {
			/* Remember to unlock it */
			b->locked = 1;
		}
	}
	return 1;
}

/* Called when we are about to modify a buffer */
/* Returns 0 if we're not allowed to modify buffer */

extern long last_time;
#define CHECK_INTERVAL 15
int nomodcheck;

int modify_logic(BW *bw,B *b)
{
	if (last_time > b->check_time + CHECK_INTERVAL) {
		b->check_time = last_time;
		if (!nomodcheck && check_mod(b)) {
			file_changed(bw,0,b,NULL);
			return 0;
		}
	}
	if (!b->didfirst) {
		/* This happens when we try to block move from a window
		   which is not on the screen */
		if (b!=bw->b) {
			msgnw(bw->parent,US "Modify other window first");
			return 0;
		}
		b->didfirst = 1;
		if (bw->o.mfirst)
			exmacro(bw->o.mfirst,1);
	}
	if (b->rdonly) {
		msgnw(bw->parent, US "Read only");
		if (joe_beep)
			ttputc(7);
		return 0;
	} else if (!b->changed && !b->locked) {
		if (!try_lock(bw,b))
			return 0;
	}
	return 1;
}

/* Execute a command n with key k */

int execmd(CMD *cmd, int k)
{
	BW *bw = (BW *) maint->curwin->object;
	int ret = -1;

	/* Warning: bw is a BW * only if maint->curwin->watom->what &
	    (TYPETW|TYPEPW) */

	/* Send data to shell window: this is broken ^K ^H (help) sends its ^H to shell */
	if ((maint->curwin->watom->what & TYPETW) && bw->b->pid && piseof(bw->cursor) &&
	(k==3 || k==9 || k==13 || k==8 || k==127 || k==4 || cmd->func==utype && k>=32 && k<256)) {
		unsigned char c = k;
		joe_write(bw->b->out, &c, 1);
		return 0;
	}

	if (cmd->m)
		return exmacro(cmd->m, 0);

	/* We don't execute if we have to fix the column position first
	 * (i.e., left arrow when cursor is in middle of nowhere) */
	if (cmd->flag & ECHKXCOL)
		if (bw->o.hex)
			bw->cursor->xcol = piscol(bw->cursor);
		else if (bw->cursor->xcol != piscol(bw->cursor))
			goto skip;

	/* Don't execute command if we're in wrong type of window */
	if (!(cmd->flag & maint->curwin->watom->what))
		goto skip;

	/* Complete selection for block commands */
	if ((cmd->flag & EBLOCK) && nowmarking)
		utoggle_marking(maint->curwin->object);

	/* We are about to modify the file */
	if ((maint->curwin->watom->what & TYPETW) && (cmd->flag & EMOD)) {
		if (!modify_logic(bw,bw->b))
			goto skip;
	}

	/* Execute command */
	ret = cmd->func(maint->curwin->object, k);

	if (smode)
		--smode;

	/* Don't update anything if we're going to leave */
	if (leave)
		return 0;

	/* cmd->func could have changed bw on us */
	/* This is bad: maint->curwin might not be the same window */
	/* Safer would be to attach a pointer to curwin- if curwin
	   gets clobbered, so does pointer. */
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

	if (joe_beep && ret)
		ttputc(7);
	return ret;
}

extern int auto_scroll;

void do_auto_scroll()
{
	static CMD *scrup = 0;
	static CMD *scrdn = 0;
	static CMD *drag = 0;
	if (!scrup) {
		scrup = findcmd(US "upslide");
		scrdn = findcmd(US "dnslide");
		drag = findcmd(US "defmdrag");
	}
	if (auto_scroll > 0)
		execmd(scrdn,0);
	else if (auto_scroll < 0)
		execmd(scrup,0);

	execmd(drag,0);
		
	reset_trig_time();
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

CMD *findcmd(unsigned char *s)
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
	cmd->name = zdup(s);
	cmd->flag = 0;
	cmd->func = NULL;
	cmd->m = m;
	cmd->arg = 1;
	cmd->negarg = NULL;
	htadd(cmdhash, cmd->name, cmd);
}

static unsigned char **getcmds(void)
{
	unsigned char **s = vaensure(NULL, sizeof(cmds) / sizeof(CMD));
	int x;
	HENTRY *e;

	for (x = 0; x != cmdhash->len; ++x)
		for (e = cmdhash->tab[x]; e; e = e->next)
			s = vaadd(s, vsncpy(NULL, 0, sz(e->name)));
	vasort(s, aLen(s));
	return s;
}

/* Command line */

unsigned char **scmds = NULL;	/* Array of command names */

static int cmdcmplt(BW *bw)
{
	if (!scmds)
		scmds = getcmds();
	return simple_cmplt(bw,scmds);
}

static int docmd(BW *bw, unsigned char *s, void *object, int *notify)
{
	MACRO *mac;
	int ret = -1;
	CMD *cmd = findcmd(s);

	vsrm(s);	/* allocated in pw.c::rtnpw() */
	if (!cmd)
		msgnw(bw->parent, US "No such command");
	else {
		mac = mkmacro(-1, 0, 0, cmd);
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
	if (wmkpw(bw->parent, US "cmd: ", &cmdhist, docmd, US "cmd", NULL, cmdcmplt, NULL, NULL, locale_map, 0)) {
		return 0;
	} else {
		return -1;
	}
}
