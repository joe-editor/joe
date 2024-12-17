/*
 *	Editor startup and main edit loop
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 * 	This file is part of JOE (Joe's Own Editor)
 */
#include "types.h"

#ifdef MOUSE_GPM
#include <gpm.h>
#endif

char *exmsg = NULL;		/* Message to display when exiting the editor */
char *xmsg;			/* Message to display when starting the editor */
int usexmouse=0;
int xmouse=0;
int nonotice;
int noexmsg = 0;
int pastehack;
int helpon;

Screen *maint;			/* Main edit screen */

/* Make windows follow cursor */

void dofollows(void)
{
	W *w = maint->curwin;

	do {
		if (w->y != -1 && w->h && w->watom->follow && w->object)
			w->watom->follow(w);
		w = (W *)(w->link.next);
	} while (w != maint->curwin);
}

/* Update screen */

volatile int dostaupd = 1;

void edupd(int flg)
{
	W *w;
	ptrdiff_t wid, hei;

	if (dostaupd) {
		staupd = 1;
		dostaupd = 0;
	}
	ttgtsz(&wid, &hei);
	if (nresize(maint->t, wid, hei)) {
		sresize(maint);
#ifdef MOUSE_GPM
		gpm_mx = wid;
		gpm_my = hei;
#endif
	}
	dofollows();
	ttflsh();
	nscroll(maint->t, BG_COLOR(bg_text));
	help_display(maint);
	w = maint->curwin;
	do {
		if (w->y != -1) {
			if (w->object && w->watom->disp)
				w->watom->disp(w, flg);
			msgout(w);
		}
		w = (W *) (w->link.next);
	} while (w != maint->curwin);
	cpos(maint->t, maint->curwin->x + maint->curwin->curx, maint->curwin->y + maint->curwin->cury);
	staupd = 0;
}

static int ahead = 0;
static int ungot = 0;
static int ungotc = 0;

void nungetc(int c)
{
	if (c != 'C' - '@' && c != 'M' - '@') {
		chmac();
		ungot = 1;
		ungotc = c;
	}
}

MACRO *type_backtick;

/* Execute a macro every nn seconds */

time_t last_timer_time = 0;
time_t cur_time;
time_t timer_macro_delay;
MACRO *timer_macro;

MACRO *timer_play(void)
{
	cur_time = time(NULL);
	if (timer_macro && timer_macro_delay && cur_time >= last_timer_time + timer_macro_delay) {
		last_timer_time = cur_time;
		return timer_macro;
	}
	return 0;
}

KBD *shell_kbd;

int edloop(int flg)
{
	int term = 0;
	int ret = 0;

	if (flg) {
		if (maint->curwin->watom->what == TYPETW)
			return 0;
		else
			maint->curwin->notify = &term;
	}
	while (!leave && (!flg || !term)) {
		W *w;
		MACRO *m;
		BW *bw;
		int c;
		int auto_off = 0;
		int word_off = 0;
		int spaces_off = 0;

		if (exmsg && !flg) {
			vsrm(exmsg);
			exmsg = NULL;
		}
		edupd(1);
		if (!ahead && !have)
			ahead = 1;
		if (ungot) {
			c = ungotc;
			ungot = 0;
		} else
			c = ttgetch();

		/* Clear temporary messages */
		w = maint->curwin;
		do {
			if (w->y != -1) {
				msgclr(w);
			}
			w = (W *) (w->link.next);
		} while (w != maint->curwin);

		if (c == 10 && (!ahead || ((BW *)maint->curwin->object)->pasting))
			c = 13;

		more_no_auto:

		/* Use special kbd if we're handing data to a shell window */
		bw = (BW *)maint->curwin->object;
		if (shell_kbd && (maint->curwin->watom->what & TYPETW) && bw->b->pid && !bw->b->vt && !bw->b->raw && piseof(bw->cursor))
			m = dokey(shell_kbd, c);
		else if ((maint->curwin->watom->what & TYPETW) && bw->b->pid && bw->b->vt && bw->cursor->byte == bw->b->vt->vtcur->byte)
			m = dokey(bw->b->vt->kbd, c);
		else
			m = dokey(maint->curwin->kbd, c);

		/* leading part of backtick hack... */
		/* should only do this if backtick is uquote, but you're not likely to get quick typeahead with ESC ' as uquote */
		if (pastehack && m && m->cmd && m->cmd->func == uquote && ttcheck()) {
			m = type_backtick;
		}

		/* disable autoindent if it looks like a mouse paste... */
		if (pastehack && m && m->cmd && (m->cmd->func == utype || m->cmd->func == urtn) && (maint->curwin->watom->what & TYPETW) &&
		    (bw->o.autoindent || bw->o.wordwrap || bw->o.spaces) && ttcheck()) {
			auto_off = bw->o.autoindent;
			bw->o.autoindent = 0;
			word_off = bw->o.wordwrap;
			bw->o.wordwrap = 0;
			spaces_off = bw->o.spaces;
			bw->o.spaces = 0;
		}

		if (maint->curwin->main && maint->curwin->main != maint->curwin) {
			ptrdiff_t x = maint->curwin->kbd->x;

			maint->curwin->main->kbd->x = x;
			if (x)
				maint->curwin->main->kbd->seq[x - 1] = maint->curwin->kbd->seq[x - 1];
		}
		if (!m) {
			m = timer_play();
			c = NO_MORE_DATA;
		}
		if (m)
			ret = exemac(m, c);

		/* trailing part of backtick hack... */
		/* for case where ` is very last character of pasted block */
		while (pastehack && !leave && (!flg || !term) && m && (m == type_backtick || (m->cmd && (m->cmd->func == utype || m->cmd->func == urtn))) && ttcheck() && havec == '`') {
			ttgetch();
			ret = exemac(type_backtick, NO_MORE_DATA);
		}

		/* trailing part of disabled autoindent */
		if (pastehack && !leave && (!flg || !term) && m && (m == type_backtick || (m->cmd && (m->cmd->func == utype || m->cmd->func == urtn))) && ttcheck()) {
			if (ungot) {
				c = ungotc;
				ungot = 0;
			} else
				c = ttgetch();
			goto more_no_auto;
		}

		/* Restore modes */
		if (!leave && maint->curwin->watom->what & TYPETW) {
			bw = (BW *)maint->curwin->object;

			if (auto_off) {
				auto_off = 0;
				bw->o.autoindent = 1;
			}

			if (word_off) {
				word_off = 0;
				bw->o.wordwrap = 1;
			}

			if (spaces_off) {
				spaces_off = 0;
				bw->o.spaces = 1;
			}
		}

	}

	if (term == -1)
		return -1;
	else
		return ret;
}

#ifdef __MSDOS__
extern void setbreak();
extern int breakflg;
#endif

const char * const *mainenv;

B *startup_log = NULL;
static int logerrors = 0;

char i_msg[128];

void internal_msg(char *s)
{
	P *t = pdup(startup_log->eof, "internal_msg");
	binss(t, s);
	prm(t);
}

void setlogerrs(void)
{
	logerrors = 1;
}

/* Opens new bw with startup log */
int ushowlog(W *w, int k)
{
	if (startup_log) {
		B *copied;
		BW *newbw;
		void *object;

		if (uduptw(w, k)) {
			return -1;
		}

		copied = bcpy(startup_log->bof, startup_log->eof);
		copied->name = zdup("* Startup Log *");
		copied->internal = 1;

		newbw = (BW *) maint->curwin->object;
		object = newbw->object;
		w = newbw->parent;
		bwrm(newbw);
		w->object = (void *) (newbw = bwmk(w, copied, 0));
		wredraw(newbw->parent);
		newbw->object = object;

		return 0;
	}

	return 1;
}

int main(int argc, char **real_argv, const char * const *envv)
{
	CAP *cap;
	char **argv = (char **)real_argv;
	struct stat sbuf;
	char *s;
	char *t;
	time_t time_rc;
	char *run;
#ifdef __MSDOS__
	char *rundir;
#endif
	SCRN *n;
	int opened = 0;
	int omid;
	int backopt;
	int c;
	int filesonly;

	joe_iswinit();
	joe_locale();

	mainenv = envv;

	vmem = vtmp();
	startup_log = bfind_scratch("* Startup Log *");
	startup_log->internal = 1;
	startup_log->current_dir = vsncpy(NULL, 0, NULL, 0);

#ifdef __MSDOS__
	_fmode = O_BINARY;
	zlcpy(stdbuf, SIZEOF(stdbuf), argv[0]);
	joesep(stdbuf);
	run = namprt(stdbuf);
	rundir = dirprt(stdbuf);
	for (c = 0; run[c]; ++c)
		if (run[c] == '.') {
			run = vstrunc(run, c);
			break;
		}
#else
	run = namprt(argv[0]);
#endif

	if ((s = getenv("LINES")) != NULL)
		env_lines = ztoi(s);
	if ((s = getenv("COLUMNS")) != NULL)
		env_columns = ztoi(s);
	if ((s = getenv("BAUD")) != NULL)
		Baud = ztoi(s);
	if (getenv("DOPADDING"))
		dopadding = 1;
	if (getenv("NOXON"))
		noxon = 1;
	if ((s = getenv("JOETERM")) != NULL)
		joeterm = s;

#ifndef __MSDOS__
	if (!(cap = my_getcap(NULL, 9600, NULL, NULL))) {
		logerror_0(joe_gettext(_("Couldn't load termcap/terminfo entry\n")));
		goto exit_errors;
	}
#endif

#ifdef __MSDOS__

	s = vsncpy(NULL, 0, sv(run));
	s = vsncpy(sv(s), sc("rc"));
	c = procrc(cap, s);
	if (c == 0)
		goto donerc;
	if (c == 1) {
		logerror_1(joe_gettext(_("There were errors in '%s'.  Falling back on default.\n")), s);
	}

	vsrm(s);
	s = vsncpy(NULL, 0, sv(rundir));
	s = vsncpy(sv(s), sv(run));
	s = vsncpy(sv(s), sc("rc"));
	c = procrc(cap, s);
	if (c == 0)
		goto donerc;
	if (c == 1) {
		logerror_1(joe_gettext(_("There were errors in '%s'.  Falling back on default.\n")), s);
	}
#else

	/* Name of system joerc file.  Try to find one with matching language... */

	/* Try full language: like joerc.de_DE */
	t = vsncpy(NULL, 0, sc(JOERC));
	t = vsncpy(sv(t), sv(run));
	t = vsncpy(sv(t), sc("rc."));
	t = vsncpy(sv(t), sz(locale_msgs));
	if (!stat(t,&sbuf))
		time_rc = sbuf.st_mtime;
	else {
		/* Try generic language: like joerc.de */
		if (locale_msgs[0] && locale_msgs[1] && locale_msgs[2]=='_') {
			vsrm(t);
			t = vsncpy(NULL, 0, sc(JOERC));
			t = vsncpy(sv(t), sv(run));
			t = vsncpy(sv(t), sc("rc."));
			t = vsncpy(sv(t), locale_msgs, 2);
			if (!stat(t,&sbuf))
				time_rc = sbuf.st_mtime;
			else
				goto nope;
		} else {
			nope:
			vsrm(t);
			/* Try Joe's bad English */
			t = vsncpy(NULL, 0, sc(JOERC));
			t = vsncpy(sv(t), sv(run));
			t = vsncpy(sv(t), sc("rc"));
			if (!stat(t,&sbuf))
				time_rc = sbuf.st_mtime;
			else
				time_rc = 0;
		}
	}

	/* User's joerc file */
	s = getenv("HOME");
	if (s) {
		s = vsncpy(NULL, 0, sz(s));
		s = vsncpy(sv(s), sc("/."));
		s = vsncpy(sv(s), sv(run));
		s = vsncpy(sv(s), sc("rc"));

		if (!stat(s,&sbuf)) {
			if (sbuf.st_mtime < time_rc) {
				logmessage_2(joe_gettext(_("Warning: %s is newer than your %s.\n")),t,s);
			}
		}

		c = procrc(cap, s);
		if (c == 0) {
			vsrm(t);
			goto donerc;
		}
		if (c == 1) {
			logerror_1(joe_gettext(_("There were errors in '%s'.  Falling back on default.\n")), s);
		}
	}

	vsrm(s);
	s = t;
	c = procrc(cap, s);
	if (c == 0)
		goto donerc;
	if (c == 1) {
		logerror_1(joe_gettext(_("There were errors in '%s'.  Falling back on default.\n")), s);
	}

	/* Try built-in joerc */
	s = vsncpy(NULL, 0, sc("*"));
	s = vsncpy(sv(s), sv(run));
	s = vsncpy(sv(s), sc("rc"));
	c = procrc(cap, s);
	if (c != 0 && c != 1) {
		/* If *fancyjoerc not present, use *joerc which is always there */
		s = vstrunc(s, 0);
		s = vsncpy(sv(s),sc("*joerc"));
		c = procrc(cap, s);
	}
	if (c == 0)
		goto donerc;
	if (c == 1) {
		logerror_1(joe_gettext(_("There were errors in '%s'.  Falling back on default.\n")), s);
	}
#endif

	logerror_1(joe_gettext(_("Couldn't open '%s'\n")), s);
	goto exit_errors;
	return 1;

	donerc:

	if (validate_rc()) {
		logerror_0(joe_gettext(_("rc file has no :main key binding section or no bindings.  Bye.\n")));
		goto exit_errors;
	}

	{
		char buf[10];
		ptrdiff_t x;
		zlcpy(buf, SIZEOF(buf), "\"`\"	`  ");
		type_backtick = mparse(0, buf, &x, 0);
	}

	shell_kbd = mkkbd(kmap_getcontext("shell"));

	if (!isatty(fileno(stdin)))
		idleout = 0;

	for (c = 1; argv[c]; ++c) {
		if (!strcmp(argv[c], "-help") || !strcmp(argv[c], "--help")) {
			printf("Joe's Own Editor v%s\n\n", VERSION);
			printf("Usage: %s [global-options] [ [local-options] filename ]...\n\n", argv[0]);
			printf("Global options:\n");
			cmd_help(0);
			printf("\nLocal options:\n");
			printf("    %-23s Start cursor on specified line\n", "+nnn");
			cmd_help(1);
			return 0;
		} else if (argv[c][0] == '-') {
			if (argv[c][1] == '-' && !argv[c][2])
				break;
			if (argv[c][1])
				switch (glopt(argv[c] + 1, argv[c + 1], NULL, 1)) {
				case 0:
					logerror_1(joe_gettext(_("Unknown option '%s'\n")), argv[c]);
					break;
				case 1:
					break;
				case 2:
					++c;
					break;
			} else
				idleout = 0;
		}
	}

	/* initialize mouse support */
	if (xmouse && (s=getenv("TERM")) && zstr(s,"xterm"))
		usexmouse=1;

	if (!(n = nopen(cap)))
		goto exit_errors;
	maint = screate(n);

	load_state();

	/* It would be better if this ran uedit() to load files */

	/* The business with backopt is to load the file first, then apply file
	 * local options afterwords */

	/* orphan is not compatible with exemac()- macros need a window to exist */
	for (c = 1, backopt = 0, filesonly = 0; argv[c]; ++c)
		if (!filesonly && argv[c][0] == '+' && argv[c][1]>='0' && argv[c][1]<='9') {
			if (!backopt)
				backopt = c;
		} else if (!filesonly && argv[c][0] == '-' && argv[c][1]) {
			if (argv[c][1] == '-' && !argv[c][2])
				filesonly = 1;
			else {
				if (!backopt)
					backopt = c;
				if (glopt(argv[c] + 1, argv[c + 1], NULL, 0) == 2)
					++c;
			}
		} else {
			B *b = bfind(argv[c]);
			BW *bw = NULL;
			int er = berror;

			/* This is too annoying */
			/* set_current_dir(argv[c],1); */

			setup_history(&filehist);
			append_history(filehist,sz(argv[c]));

			/* wmktw inserts the window before maint->curwin */
			if (!orphan || !opened) {
				bw = wmktw(maint, b);
				if (er)
					msgnwt(bw->parent, joe_gettext(msgs[-er]));
			} else {
				off_t line;
				b->orphan = 1;
				b->oldcur = pdup(b->bof, "main");
				pline(b->oldcur, get_file_pos(b->name));
				p_goto_bol(b->oldcur);
				line = b->oldcur->line - (maint->h - 1) / 2;
				if (line < 0)
					line = 0;
				b->oldtop = pdup(b->oldcur, "main");
				pline(b->oldtop, line);
				p_goto_bol(b->oldtop);
			}
			if (bw) {
				off_t lnum = 0;

				bw->o.readonly = bw->b->rdonly;
				if (backopt) {
					while (backopt != c) {
						if (argv[backopt][0] == '+') {
							lnum = ztoo(argv[backopt] + 1);
							++backopt;
						} else {
							if (glopt(argv[backopt] + 1, argv[backopt + 1], &bw->o, 0) == 2)
								backopt += 2;
							else
								backopt += 1;
						}
					}
				}
				/* bw->b->o = bw->o; */
				lazy_opts(bw->b, &bw->o);
				bw->o = bw->b->o;
				bw->b->rdonly = bw->o.readonly;
				/* Put cursor in window, so macros work properly */
				maint->curwin = bw->parent;
				/* Hmm... window might not exist any more... depends on what macro does... */
				if (lnum > 0)
					pline(bw->cursor, lnum - 1);
				else
					pline(bw->cursor, get_file_pos(bw->b->name));
				p_goto_bol(bw->cursor);
				/* Execute macro */
				if (er == -1 && bw->o.mnew)
					exmacro(bw->o.mnew, 1, NO_MORE_DATA);
				if (er == 0 && bw->o.mold)
					exmacro(bw->o.mold, 1, NO_MORE_DATA);
				/* Go back to first window so windows are in same order as command line  */
				if (opened)
					wnext(maint);

			}
			opened = 1;
			backopt = 0;
		}



	if (opened) {
		wshowall(maint);
		omid = opt_mid;
		opt_mid = 1;
		dofollows();
		opt_mid = omid;
	} else {
		BW *bw = wmktw(maint, bfind(""));

		if (bw->o.mnew)
			exmacro(bw->o.mnew, 1, NO_MORE_DATA);
	}
	maint->curwin = maint->topwin;

	if (logerrors) {
		B *copied = bcpy(startup_log->bof, startup_log->eof);
		BW *bw = wmktw(maint, copied);
		copied->name = zdup(startup_log->name);
		copied->internal = 1;
		maint->curwin = bw->parent;
		wshowall(maint);
	}

	init_colors();

	if (helpon) {
		help_on(maint);
	}
	if (!nonotice) {
		if (xmsg) {
			xmsg = stagen(NULL, (BW *)(lastw(maint)->object), joe_gettext(xmsg), ' ');
			msgnw(((BASE *)lastw(maint)->object)->parent, xmsg);
		} else {
			joe_snprintf_3(msgbuf,JOE_MSGBUFSIZE,joe_gettext(_("\\i** Joe's Own Editor v%s ** (%s) ** Copyright %s 2015 **\\i")),VERSION,locale_map->name,(locale_map->type ? "©" : "(C)"));
			msgnw(((BASE *)lastw(maint)->object)->parent, msgbuf);
		}

	}

	if (!idleout) {
		if (!isatty(fileno(stdin)) && modify_logic((BW *)maint->curwin->object, ((BW *)maint->curwin->object)->b)) {
			/* Read stdin into window asynchronously */
			cstart ((BW *)maint->curwin->object, NULL, NULL, NULL, NULL, 0, 1, NULL, SHELL_TYPE_RAW);
			/* Close stdin, so that if user kills the copying the program feeding stdin sees a SIGPIPE */
			/* It won't if there are any extra file descriptors open */
			fclose(stdin);
		}
	}

	edloop(0);

	save_state();

	/* Delete all buffer so left over locks get eliminated */
	brmall();

	vclose(vmem);
	nclose(n);

	if  (noexmsg) {
		if (notite)
			fprintf(stderr, "\n");
	} else {
		if (exmsg)
			fprintf(stderr, "\n%s\n", exmsg);
		else if (notite)
			fprintf(stderr, "\n");
	}

	return 0;

exit_errors:

	/* Write out error log to console if we are exiting with errors. */
	if (startup_log && startup_log->eof->byte)
		bsavefd(startup_log->bof, 2, startup_log->eof->byte);

	return 1;
}
