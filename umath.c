/*
 *	Math
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "b.h"
#include "pw.h"
#include "utils.h"
#include "vs.h"
#include "utf8.h"
#include "charmap.h"
#include "w.h"

unsigned char *merr;

int mode_hex;
int mode_eng;
int mode_ins;

static RETSIGTYPE fperr(int unused)
{
	if (!merr) {
		merr = US "Float point exception";
	}
	REINSTALL_SIGHANDLER(SIGFPE, fperr);
}

struct var {
	unsigned char *name;
	double (*func)(double n);
	int set;
	double val;
	struct var *next;
} *vars = NULL;

static struct var *get(unsigned char *str)
{
	struct var *v;

	for (v = vars; v; v = v->next) {
		if (!strcmp(v->name, str)) {
			return v;
		}
	}
	v = (struct var *) joe_malloc(sizeof(struct var));

	v->set = 0;
	v->func = 0;
	v->val = 0;
	v->next = vars;
	vars = v;
	v->name = joe_strdup(str);
	return v;
}

unsigned char *ptr;
struct var *dumb;

static double expr(int prec, struct var **rtv)
{
	double x = 0.0, y;
	struct var *v = NULL;

	while (*ptr == ' ' || *ptr == '\t') {
		++ptr;
	}
	if ((*ptr >= 'a' && *ptr <= 'z') || (*ptr >= 'A' && *ptr <= 'Z')
	    || *ptr == '_') {
		unsigned char *s = ptr, c;

		while ((*ptr >= 'a' && *ptr <= 'z')
		       || (*ptr >= 'A' && *ptr <= 'Z')
		       || *ptr == '_' || (*ptr >= '0' && *ptr <= '9')) {
			++ptr;
		}
		c = *ptr;
		*ptr = 0;
		if (!strcmp(s,"hex")) {
			mode_hex = 1;
			mode_eng = 0;
			v = get("ans");
			x = v->val;
		} else if (!strcmp(s,"dec")) {
			mode_hex = 0;
			mode_eng = 0;
			v = get("ans");
			x = v->val;
		} else if (!strcmp(s,"eng")) {
			mode_hex = 0;
			mode_eng = 1;
			v = get("ans");
			x = v->val;
		} else if (!strcmp(s,"ins")) {
			mode_ins = 1;
			v = get("ans");
			x = v->val;
		} else {
			v = get(s);
			x = v->val;
		}
		*ptr = c;
	} else if ((*ptr >= '0' && *ptr <= '9') || *ptr == '.') {
		x = strtod(ptr,(char **)&ptr);
	} else if (*ptr == '(') {
		++ptr;
		x = expr(0, &v);
		if (*ptr == ')')
			++ptr;
		else {
			if (!merr)
				merr = US "Missing )";
		}
	} else if (*ptr == '-') {
		++ptr;
		x = -expr(10, &dumb);
	}
      loop:
	while (*ptr == ' ' || *ptr == '\t')
		++ptr;
	if (*ptr == '(' && 11 > prec) {
		++ptr;
		y = expr(0, &dumb);
		if (*ptr == ')')
			++ptr;
		else {
			if (!merr)
				merr = US "Missing )";
		}
		if (v && v->func)
			x = v->func(y);
		else {
			if (!merr)
				merr = US "Called object is not a function";
		}
		goto loop;
	} else if (*ptr == '*' && ptr[1] == '*' && 8 > prec) {
		ptr+=2;
		x = pow(x, expr(8, &dumb));
		v = 0;
		goto loop;
	} else if (*ptr == '^' && 8 > prec) {
		++ptr;
		x = pow(x, expr(8, &dumb));
		v = 0;
		goto loop;
	} else if (*ptr == '*' && 7 > prec) {
		++ptr;
		x *= expr(7, &dumb);
		v = 0;
		goto loop;
	} else if (*ptr == '/' && 7 > prec) {
		++ptr;
		x /= expr(7, &dumb);
		v = 0;
		goto loop;
	} else if(*ptr=='%' && 7>prec) {
		++ptr;
		y = expr(7, &dumb);
		if ((int)y == 0) x = 1.0/0.0;
		else x = ((int) x) % (int)y;
		v = 0;
		goto loop;
	} else if (*ptr == '+' && 6 > prec) {
		++ptr;
		x += expr(6, &dumb);
		v = 0;
		goto loop;
	} else if (*ptr == '-' && 6 > prec) {
		++ptr;
		x -= expr(6, &dumb);
		v = 0;
		goto loop;
	} else if (*ptr == '<' && 5 > prec) {
		++ptr;
		if (*ptr == '=') ++ptr, x = (x <= expr(5,&dumb));
		else x = (x < expr(5,&dumb));
		v = 0;
		goto loop;
	} else if (*ptr == '>' && 5 > prec) {
		++ptr;
		if (*ptr == '=') ++ptr, x=(x >= expr(5,&dumb));
		else x = (x > expr(5,&dumb));
		v = 0;
		goto loop;
	} else if (*ptr == '=' && ptr[1] == '=' && 5 > prec) {
		++ptr, ++ptr;
		x = (x == expr(5,&dumb));
		v = 0;
		goto loop;
	} else if (*ptr == '!' && ptr[1] == '=' && 5 > prec) {
		++ptr, ++ptr;
		x = (x != expr(5,&dumb));
		v = 0;
		goto loop;
	} else if(*ptr == '&' && ptr[1] == '&' && 3 > prec) {
		++ptr, ++ptr;
		y = expr(3,&dumb);
		x = (int)x && (int)y;
		v = 0;
		goto loop;
	} else if(*ptr=='|' && ptr[1]=='|' &&  3 > prec) {
		++ptr, ++ptr;
		y = expr(3,&dumb);
		x = (int)x || (int)y;
		v= 0;
		goto loop;
	} else if (*ptr == '=' && 2 >= prec) {
		++ptr;
		x = expr(2, &dumb);
		if (v) {
			v->val = x;
			v->set = 1;
		} else {
			if (!merr)
				merr = US "Left side of = is not an l-value";
		}
		v = 0;
		goto loop;
	}
	*rtv = v;
	return x;
}

double m_sin(double n) { return sin(n); }
double m_cos(double n) { return cos(n); }
double m_tan(double n) { return tan(n); }
double m_exp(double n) { return exp(n); }
double m_sqrt(double n) { return sqrt(n); }
double m_cbrt(double n) { return cbrt(n); }
double m_log(double n) { return log(n); }
double m_log10(double n) { return log10(n); }
double m_asin(double n) { return asin(n); }
double m_acos(double n) { return acos(n); }
double m_atan(double n) { return atan(n); }
double m_sinh(double n) { return sinh(n); }
double m_cosh(double n) { return cosh(n); }
double m_tanh(double n) { return tanh(n); }
double m_asinh(double n) { return asinh(n); }
double m_acosh(double n) { return acosh(n); }
double m_atanh(double n) { return atanh(n); }
double m_int(double n) { return (int)(n); }
double m_floor(double n) { return floor(n); }
double m_ceil(double n) { return ceil(n); }
double m_fabs(double n) { return fabs(n); }
double m_erf(double n) { return erf(n); }
double m_erfc(double n) { return erfc(n); }
double m_j0(double n) { return j0(n); }
double m_j1(double n) { return j1(n); }
double m_y0(double n) { return y0(n); }
double m_y1(double n) { return y1(n); }

double calc(BW *bw, unsigned char *s)
{
	double result;
	struct var *v;
	BW *tbw = bw->parent->main->object;
	int c = brch(bw->cursor);

	if (!vars) {
		v = get(US "sin"); v->func = m_sin;
		v = get(US "cos"); v->func = m_cos;
		v = get(US "tan"); v->func = m_tan;
		v = get(US "exp"); v->func = m_exp;
		v = get(US "sqrt"); v->func = m_sqrt;
		v = get(US "cbrt"); v->func = m_cbrt;
		v = get(US "ln"); v->func = m_log;
		v = get(US "log"); v->func = m_log10;
		v = get(US "asin"); v->func = m_asin;
		v = get(US "acos"); v->func = m_acos;
		v = get(US "atan"); v->func = m_atan;
		v = get(US "pi"); v->val = M_PI; v->set = 1;
		v = get(US "e"); v->val = M_E; v->set = 1;
		v = get(US "sinh"); v->func = m_sinh;
		v = get(US "cosh"); v->func = m_cosh;
		v = get(US "tanh"); v->func = m_tanh;
		v = get(US "asinh"); v->func = m_asinh;
		v = get(US "acosh"); v->func = m_acosh;
		v = get(US "atanh"); v->func = m_atanh;
		v = get(US "int"); v->func = m_int;
		v = get(US "floor"); v->func = m_floor;
		v = get(US "ceil"); v->func = m_ceil;
		v = get(US "abs"); v->func = m_fabs;
		v = get(US "erf"); v->func = m_erf;
		v = get(US "erfc"); v->func = m_erfc;
		v = get(US "j0"); v->func = m_j0;
		v = get(US "j1"); v->func = m_j1;
		v = get(US "y0"); v->func = m_y0;
		v = get(US "y1"); v->func = m_y1;
	}

	v = get(US "top");
	v->val = tbw->top->line + 1;
	v->set = 1;
	v = get(US "lines");
	v->val = tbw->b->eof->line + 1;
	v->set = 1;
	v = get(US "line");
	v->val = tbw->cursor->line + 1;
	v->set = 1;
	v = get(US "col");
	v->val = tbw->cursor->col + 1;
	v->set = 1;
	v = get(US "byte");
	v->val = tbw->cursor->byte + 1;
	v->set = 1;
	v = get(US "height");
	v->val = tbw->h;
	v->set = 1;
	v = get(US "width");
	v->val = tbw->w;
	v->set = 1;
	v = get(US "char");
	v->val = (c == NO_MORE_DATA ? -1.0 : c);
	v->set = 1;
	ptr = s;
	merr = 0;
      up:
	result = expr(0, &dumb);
	v = get(US "ans");
	v->val = result;
	v->set = 1;
	if (!merr) {
		while (*ptr == ' ' || *ptr == '\t') {
			++ptr;
		}
		if (*ptr == ':') {
			++ptr;
			while (*ptr == ' ' || *ptr == '\t') {
				++ptr;
			}
			if (*ptr) {
				goto up;
			}
		} else if (*ptr) {
			merr = US "Extra junk after end of expr";
		}
	}
	return result;
}

/* Main user interface */
static int domath(BW *bw, unsigned char *s, void *object, int *notify)
{
	double result = calc(bw, s);

	if (notify) {
		*notify = 1;
	}
	if (merr) {
		msgnw(bw->parent, merr);
		return -1;
	}
	vsrm(s);
	if (mode_hex)
		joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "0x%lX", (long)result);
	else if (mode_eng)
		joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "%.16G", result);
	else
		joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "%.16G", result);
	if (bw->parent->watom->what != TYPETW || mode_ins) {
		binsm(bw->cursor, sz(msgbuf));
		pfwrd(bw->cursor, strlen((char *)msgbuf));
		bw->cursor->xcol = piscol(bw->cursor);
	} else {
		msgnw(bw->parent, msgbuf);
	}
	mode_ins = 0;
	return 0;
}

B *mathhist = NULL;

int umath(BW *bw)
{
	joe_set_signal(SIGFPE, fperr);
	if (wmkpw(bw->parent, US "=", &mathhist, domath, US "Math", NULL, NULL, NULL, NULL, locale_map)) {
		return 0;
	} else {
		return -1;
	}
}
