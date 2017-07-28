/*
 *	Keyboard macros
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "types.h"

MACRO *freemacros = NULL;

/* Create a macro */

MACRO *mkmacro(int k, int flg, ptrdiff_t n, CMD *cmd)
{
	MACRO *macro;

	if (!freemacros) {
		ptrdiff_t x;

		macro = (MACRO *) joe_malloc(SIZEOF(MACRO) * 64);
		for (x = 0; x != 64; ++x) {
			macro[x].steps = (MACRO **) freemacros;
			freemacros = macro + x;
		}
	}
	macro = freemacros;
	freemacros = (MACRO *) macro->steps;
	macro->steps = NULL;
	macro->size = 0;
	macro->flg = flg;
	macro->n = n;
	macro->cmd = cmd;
	macro->k = k;
	macro->what = 0;
	return macro;
}

/* Eliminate a macro */

void rmmacro(MACRO *macro)
{
	if (macro) {
		if (macro->steps) {
			ptrdiff_t x;

			for (x = 0; x != macro->n; ++x)
				rmmacro(macro->steps[x]);
			joe_free(macro->steps);
		}
		macro->steps = (MACRO **) freemacros;
		freemacros = macro;
	}
}

/* Add a step to block macro */

void addmacro(MACRO *macro, MACRO *m)
{
	if (macro->n == macro->size) {
		if (macro->steps)
			macro->steps = (MACRO **) joe_realloc(macro->steps, (macro->size += 8) * SIZEOF(MACRO *));
		else
			macro->steps = (MACRO **) joe_malloc((macro->size = 8) * SIZEOF(MACRO *));
	}
	macro->steps[macro->n++] = m;
}

/* Duplicate a macro */

MACRO *dupmacro(MACRO *mac)
{
	MACRO *m = mkmacro(mac->k, mac->flg, mac->n, mac->cmd);

	if (mac->steps) {
		ptrdiff_t x;

		m->steps = (MACRO **) joe_malloc((m->size = mac->n) * SIZEOF(MACRO *));
		for (x = 0; x != m->n; ++x)
			m->steps[x] = dupmacro(mac->steps[x]);
	}
	return m;
}

/* Set key part of macro */

MACRO *macstk(MACRO *m, int k)
{
	if (k != -1)
		m->k = k;
	return m;
}

/* Set flg part of macro */

MACRO *macsta(MACRO *m, int a)
{
	m->flg = a;
	return m;
}

/* Parse text into a macro
 * sta is set to:  ending position in buffer for no error.
 *                 -1 for syntax error
 *                 -2 for need more input
 *
 * Set secure to allow only commands which being with "shell_".
 * Also, secure_type will be used instead of type.
 */

MACRO *mparse(MACRO *m, const char *buf, ptrdiff_t *sta, int secure)
{
	const char *org = buf;
	int bf[1024];
	char bf1[1024];
	int x;

      macroloop:

	/* Skip whitespace */
	parse_ws(&buf, 0);

	/* If the buffer is only whitespace then treat as unknown command */
	if (!*buf) {
		*sta = -1;
		return NULL;
	}

	/* Do we have a string? */
	if (parse_Zstring(&buf, bf, SIZEOF(bf)/SIZEOF(bf[0])) >= 0) {
		for (x = 0; bf[x]; ++x) {
			if (m) {
				if (!m->steps) {
					MACRO *macro = m;
					
					m = mkmacro(NO_MORE_DATA, 0, 0, NULL);
					addmacro(m, macro);
				}
			} else
				m = mkmacro(NO_MORE_DATA, 0, 0, NULL);
			addmacro(m, mkmacro(bf[x], 0, 0, secure ? findcmd("secure_type") : findcmd("type")));
		}
	} else { /* Do we have a command? */
		x = 0;
		while (*buf && *buf != '#' && *buf != '!' && *buf != '~' && *buf !='-' && *buf != ',' &&
		       *buf != ' ' && *buf != '\t' && *buf != '\n' && *buf != '\r') {
		       	if (x != SIZEOF(bf1) - 1)
			       	bf1[x++] = *buf;
		       	++buf;
		}
		bf1[x] = 0;
		if (x) {
			CMD *cmd;
			int flg = 0;

			if (!secure || !zncmp(bf1, "shell_", 6))
				cmd = findcmd(bf1);
			else
				cmd = 0;

			/* Parse flags */
			while (*buf == '-' || *buf == '!' || *buf == '#' || *buf == '~') {
				if (*buf == '-') flg |= 1;
				if (*buf == '!') flg |= 2;
				if (*buf == '#') flg |= 4;
				if (*buf == '~') flg |= 8;
				++buf;
			}

			if (!cmd) {
				*sta = -1;
				return NULL;
			} else if (m) {
				if (!m->steps) {
					MACRO *macro = m;

					m = mkmacro(NO_MORE_DATA, 0, 0, NULL);
					addmacro(m, macro);
				}
				addmacro(m, mkmacro(NO_MORE_DATA, flg, 0, cmd));
			} else
				m = mkmacro(NO_MORE_DATA, flg, 0, cmd);
		} else { /* not a valid command */
			*sta = -1;
			return NULL;
		}
	}

	/* Skip whitespace */
	parse_ws(&buf, 0);

	/* Do we have a comma? */
	if (*buf == ',') {
		++buf;
		parse_ws(&buf, 0);
		if (*buf && *buf != '\r' && *buf != '\n')
			goto macroloop;
		*sta = -2;
		return m;
	}

	/* Done */
	*sta = buf - org;
	return m;
}

/* Convert macro to text */

char *unescape(char *ptr, int c)
{
	if (c == '"') {
		ptr = vsadd(ptr, '\\');
		ptr = vsadd(ptr, '"');
	} else if (c == '\\') {
		ptr = vsadd(ptr, '\\');
		ptr = vsadd(ptr, '\\');
	} else if (c == '\'') {
		ptr = vsadd(ptr, '\\');
		ptr = vsadd(ptr, '\'');
	} else if (c < 32 || c == 127) {
		ptr = vsadd(ptr, '\\');
		ptr = vsadd(ptr, 'x');
		ptr = vsadd(ptr, "0123456789ABCDEF"[15 & (c >> 4)]);
		ptr = vsadd(ptr, "0123456789ABCDEF"[15 & c]);
	} else {
		char buf[8];
		utf8_encode(buf, c);
		ptr = vscat(ptr, sz(buf));
	}
	return ptr;
}

static char *domtext(MACRO *m, char *ptr, int *first, int *instr)
{
	ptrdiff_t x;

	if (!m)
		return ptr;
	if (m->steps) {
		for (x = 0; x != m->n; ++x)
			ptr = domtext(m->steps[x], ptr, first, instr);
	} else {
		if (*instr && zcmp(m->cmd->name, "type")) {
			ptr = vsadd(ptr, '"');
			*instr = 0;
		}
		if (*first)
			*first = 0;
		else if (!*instr)
			ptr = vsadd(ptr, ',');
		if (!zcmp(m->cmd->name, "type")) {
			if (!*instr) {
				ptr = vsadd(ptr, '"');
				*instr = 1;
			}
			ptr = unescape(ptr, m->k);
		} else {
			ptr = vscatz(ptr, m->cmd->name);
			if (!zcmp(m->cmd->name, "play") || !zcmp(m->cmd->name, "gomark") || !zcmp(m->cmd->name, "setmark") || !zcmp(m->cmd->name, "record") || !zcmp(m->cmd->name, "uarg")) {
				ptr = vsadd(ptr, ',');
				ptr = vsadd(ptr, '"');
				ptr = unescape(ptr, m->k);
				ptr = vsadd(ptr, '"');
			}
		}
	}
	return ptr;
}

char *mtext(char *s, MACRO *m)
{
	int first = 1;
	int instr = 0;
	s = domtext(m, s, &first, &instr);
	if (instr)
		s = vsadd(s, '"');
	return s;
}

/* Keyboard macro recorder */

static MACRO *kbdmacro[10];
static int playmode[10];

struct recmac *recmac = NULL;

static void unmac(void)
{
	if (recmac)
		rmmacro(recmac->m->steps[--recmac->m->n]);
}

void chmac(void)
{
	if (recmac && recmac->m->n)
		recmac->m->steps[recmac->m->n - 1]->k = 3;
}

static void record(MACRO *m, int k)
{
	if (recmac)
		addmacro(recmac->m, macstk(dupmacro(m), k));
}

static int ifdepth=0;		/* JM: Nesting level of if cmds */
static int ifflag=1;		/* JM: Truth flag for if */
static int iffail=0;		/* JM: Depth where ifflag became 0 */

/* Suspend macro record/play to allow for user input.
 */

int uquery(W *w, int k)
{
	int ret = 0;
	BW *bw;
	WIND_BW(bw, w);
	
	/* Suspend current macro until current prompt is complete. */
	if (bw->parent->coro) {
		/* Save macro execution state */
		int oid = ifdepth;
		int oifl = ifflag;
		int oifa = iffail;
		struct recmac *tmp = recmac;
		recmac = 0;
		/* Suspend macro: co_suspend suspends the macro player which called us and chains it
		   to bw->parent->coro so that it continues when bw->parent->coro is done */
		ret = co_query_suspend(bw->parent->coro, 0);
		/* Continue macro */
		recmac = tmp;
		ifdepth = oid;
		ifflag = oifl;
		iffail = oifa;
	}
	return ret;
}

/* Macro execution */

MACRO *curmacro = NULL;		/* Set if we're in a macro */
static int macroptr;
static int arg = 0;		/* Repeat argument */
static int argset = 0;		/* Set if 'arg' is set */

static int exsimple(MACRO *m, int myarg, int u, int k)
{
	CMD *cmd = m->cmd;
	int flg = 0; /* set if we should not try to merge minor changes into single undo record */
	int ret = 0;

	/* Find command to execute if repeat argument is negative */
	if (myarg < 0) {
		myarg = -myarg;
		if (cmd->negarg)
			cmd = findcmd(cmd->negarg);
		else
			myarg = 0; /* Do not execute */
	}

	/* Check if command doesn't like an arg... */
	if (myarg != 1 && !cmd->arg)
		myarg = 0; /* Do not execute */

	if (myarg != 1 || !(cmd->flag & EMINOR)
	    || maint->curwin->watom->what == TYPEQW)	/* Undo work right for s & r */
		flg = 1;

	if (ifflag || (cmd->flag&EMETA)) {
		if (flg && u)
			umclear();
		/* Repeat... */
		while (myarg-- && !leave && !ret)
			ret = execmd(cmd, m->k != NO_MORE_DATA ? m->k : k);
		if (leave)
			return ret;
		if (flg && u)
			umclear();
		if (u)
			undomark();
	}

	return ret;
}

int current_arg = 1;
int current_arg_set = 0;

int exmacro(MACRO *m, int u, int k)
{
	int larg;
	int negarg = 0;
	int oid=0, oifl=0, oifa=0;
	int ret = 0;
	int main_ret = 0;
	int o_arg_set = argset;
	int o_arg = arg;

	/* Take argument */

	if (argset) {
		larg = arg;
		arg = 0;
		argset = 0;
	} else {
		larg = 1;
	}

	/* Just a simple command? */

	if (!m->steps) {
		return exsimple(m, larg, u, k);
	}

	/* Must be a real macro then... */

	if (larg < 0) {
		larg = -larg;
		negarg = 1;
	}

	if (ifflag) {
		if (u)
			umclear();
		/* Repeat... */
		while (larg && !leave && !ret) {
			MACRO *tmpmac = curmacro;
			int tmpptr = macroptr;
			int x = 0;
			int stk = nstack;

			/* Steps of macro... */
			while (m && x != m->n && !leave && !ret) {
				MACRO *d;
				int tmp_arg;
				int tmp_set;

				d = m->steps[x++];
				curmacro = m;
				macroptr = x;
				tmp_arg = current_arg;
				tmp_set = current_arg_set;
				current_arg = o_arg;
				current_arg_set = o_arg_set;

				if(d->steps) oid=ifdepth, oifl=ifflag, oifa=iffail, ifdepth=iffail=0;

				/* If this step wants to know about negative args... */
				if ((d->flg&4)) {
					argset = o_arg_set;
					arg = o_arg;
					larg = 1;
				} else if ((d->flg&1) && negarg) {
					if (argset) {
						arg = -arg;
					} else {
						argset = 1;
						arg = -1;
					}
				}

				if (d->flg&8) {
					larg = 1;
				}

				/* This is the key step of the macro... */
				if (d->flg&2)
					main_ret = exmacro(d, 0, k);
				else
					ret = exmacro(d, 0, k);

				if(d->steps) ifdepth=oid, ifflag=oifl, iffail=oifa;
				current_arg = tmp_arg;
				current_arg_set = tmp_set;
				m = curmacro;
				x = macroptr;
			}
			curmacro = tmpmac;
			macroptr = tmpptr;

			/* Pop ^KB ^KK stack */
			while (nstack > stk)
				upop(NULL, 0);
		--larg;
		}
		ret |= main_ret;

		if (leave)
			return ret;
		if (u)
			umclear();
		if (u)
			undomark();
	}

	return ret;
}

/* Execute a macro - for user typing */
/* Records macro in macro recorder, resets if */

int exemac(va_list args)
{
	MACRO *m = va_arg(args, MACRO *);
	int k = va_arg(args, int);
	
	record(m, k);
	ifflag=1; ifdepth=iffail=0;
	return exmacro(m, 1, k);
}

/* Keyboard macro user routines */

int urecord(W *w, int c)
{
	int n;
	struct recmac *r;
	if (c < '0' || c > '9')
		c = query(w, sz(joe_gettext(_("Macro to record (0-9 or %{abort} to abort): "))), 0);
	if (c > '9' || c < '0') {
		nungetc(c);
		return -1;
	}
	for (n = 0; n != 10; ++n)
		if (playmode[n])
			return -1;
	r = (struct recmac *) joe_malloc(SIZEOF(struct recmac));
	r->m = mkmacro(NO_MORE_DATA, 0, 0, NULL);
	r->next = recmac;
	r->n = c - '0';
	recmac = r;
	return 0;
}

extern time_t timer_macro_delay;
extern MACRO *timer_macro;

int utimer(W *w, int k)
{
	char *s;
	int c;
	BW *bw;
	WIND_BW(bw, w);
	
	timer_macro_delay = 0;
	if (timer_macro) {
		rmmacro(timer_macro);
		timer_macro = 0;
	}
	
	c = query(w, sz(joe_gettext(_("Macro to play (0-9 or %{abort} to abort): "))), 0);
	if (c < '0' || c > '9') {
		return -1;
	}
	
	c -= '0';
	if (!kbdmacro[c]) {
		return -1;
	}
	
	if (timer_macro) {
		rmmacro(timer_macro);
	}
	
	timer_macro = dupmacro(kbdmacro[c]);
	timer_macro_delay = 0;
	
	s = ask(w, joe_gettext(_("Delay in seconds between macro invocation (%{abort} to abort): ")), NULL, NULL, math_cmplt, utf8_map, 0, 0, NULL);
	if (s) {
		long num = calc(bw, s, 0);
		if (merr) {
			msgnw(bw->parent, merr);
			return -1;
		}
		
		timer_macro_delay = num;
		return 0;
	} else {
		return -1;
	}
}

int ustop(W *w, int k)
{
	unmac();
	if (recmac) {
		struct recmac *r = recmac;
		MACRO *m;

		dostaupd = 1;
		recmac = r->next;
		if (kbdmacro[r->n])
			rmmacro(kbdmacro[r->n]);
		kbdmacro[r->n] = r->m;
		if (recmac)
			record(m = mkmacro(r->n + '0', 0, 0, findcmd("play")), NO_MORE_DATA), rmmacro(m);
		joe_free(r);
	}
	return 0;
}


int umacros(W *w, int k)
{
	int x;
	char *buf = vsmk(128);
	BW *bw;
	WIND_BW(bw, w);

	p_goto_eol(bw->cursor);
	for (x = 0; x != 10; ++x)
		if (kbdmacro[x]) {
			buf = mtext(buf, kbdmacro[x]);
			binss(bw->cursor, buf);
			p_goto_eol(bw->cursor);
			binss(bw->cursor, vsfmt(buf, 0, "\t^K %c\tMacro %d", x + '0', x));
			p_goto_eol(bw->cursor);
			binsc(bw->cursor, '\n');
			pgetc(bw->cursor);
		}
	return 0;
}

void save_macros(FILE *f)
{
	int x;
	char *buf = 0;
	for(x = 0; x!= 10; ++x)
		if(kbdmacro[x]) {
			buf = mtext(buf, kbdmacro[x]);
			fprintf(f,"	%d ",x);
			emit_string(f,buf,zlen(buf));
			fprintf(f,"\n");
		}
	fprintf(f,"done\n");
}

void load_macros(FILE *f)
{
	char *buf = 0;
	char *bf = 0;
	while(vsgets(&buf,f) && zcmp(buf, "done")) {
		const char *p = buf;
		int n;
		ptrdiff_t len;
		ptrdiff_t sta;
		parse_ws(&p, '#');
		if(!parse_int(&p,&n)) {
			parse_ws(&p, '#');
			len = parse_string(&p,&bf);
			if (len>0)
				kbdmacro[n] = mparse(NULL,bf,&sta,0);
		}
	}
}

int uplay(W *w, int c)
{
	if (c < '0' || c > '9')
		c = query(w, sz(joe_gettext(_("Play-"))), QW_STAY);
	if (c >= '0' && c <= '9') {
		int ret;
		c -= '0';
		if (playmode[c] || !kbdmacro[c])
			return -1;
		playmode[c] = 1;
		ret = exmacro(kbdmacro[c], 0, c);
		playmode[c] = 0;
		return ret;
	} else {
		nungetc(c);
		return -1;
	}
}

/* Repeat-count setting */

int uarg(W *w, int k)
{
	char *s;
	BW *bw;
	WIND_BW(bw, w);
	
	s = ask(w, joe_gettext(_("No. times to repeat next command (%{abort} to abort): ")),
	        NULL, NULL, math_cmplt, locale_map, 0, 0, NULL);
	
	if (s) {
		int num;
		num = (int)calc(bw, s, 1);
		if (merr) {
			msgnw(w, merr);
			return -1;
		}
		arg = num;
		argset = 1;
		return 0;
	} else {
		return -1;
	}
}

int iftest(W *w, const char *prompt)
{
	char *s;
	BW *bw;
	WIND_BW(bw, w);
	
	s = ask(w, prompt, NULL, NULL, math_cmplt, utf8_map, 0, 0, NULL);
	
	if (s) {
		int num;
		
		num = (int)calc(bw, s, 0);
		if (merr) {
			msgnw(bw->parent, merr);
			return -1;
		}
		
		ifflag = (num ? 1 : 0);
		iffail = ifdepth;
		return 0;
	} else {
		ifdepth--;
		return 0; /* return -1 for prompt creation error? */
	}
}

int uif(W *w, int k)
{
	ifdepth++;
	if (!ifflag) return 0;
	return iftest(w, joe_gettext(_("If (%{abort} to abort): ")));
}

int uelsif(W *w, int k)
{
	if (!ifdepth) {
		msgnw(w,joe_gettext(_("Elsif without if")));
		return -1;
	} else if(ifflag) {
		ifflag=iffail=0; /* don't let the next else/elsif get run */
	} else if(ifdepth == iffail) {
		ifflag=1;	/* so the script can type the condition :) */
		return iftest(w,joe_gettext(_("Else if: ")));
	}
	return 0;
}

int uelse(W *w, int k)
{
	if (!ifdepth) {
		msgnw(w,joe_gettext(_("Else without if")));
		return -1;
	} else if(ifdepth == iffail) {
		ifflag = !ifflag;
	}
	return 0;
}

int uendif(W *w, int k)
{
	if(!ifdepth) {
		msgnw(w,joe_gettext(_("Endif without if")));
		return -1;
	}
	if(iffail==ifdepth) iffail--, ifflag=1;
	ifdepth--;
	if(ifdepth==0) ifflag=1;
	return 0;
}


int unaarg;
int negarg;

int uuarg(W *w, int c)
{
	char *m;
	BW *bw;
	WIND_BW(bw, w);
	
	unaarg = 0;
	negarg = 0;

	if (c != '-' && (c < '0' || c > '9'))
		c = query(bw->parent, sz(joe_gettext(_("Repeat"))), QW_STAY);

	for (;;) {
		if (c == '-')
			negarg = !negarg;
		else if (c >= '0' && c <= '9')
			unaarg = unaarg * 10 + c - '0';
		else if (c == 'U' - '@')
			if (unaarg)
				unaarg *= 4;
			else
				unaarg = 16;
		else if (c == 7 || c == 3 || c == 32 || c == -1) {
			return -1;
		} else {
			nungetc(c);
			if (unaarg)
				arg = unaarg;
			else if (negarg)
				arg = 1;
			else
				arg = 4;
			if (negarg)
				arg = -arg;
			argset = 1;
			return 0;
		}
		m = vsfmt(NULL, 0, joe_gettext(_("Repeat %s%d")), negarg ? "-" : "", unaarg);
		c = query(w, sv(m), QW_STAY);
	}
}
