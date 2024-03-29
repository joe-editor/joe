/*
 *	Math
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "types.h"

const char *merr;

int mode_display; /* 0 = decimal, 1 = engineering, 2 = hex, 3 = octal, 4 = binary */
int mode_ins;

static BW *calc_bw;

double vzero = 0.0;

static RETSIGTYPE fperr(int unused)
{
	if (!merr) {
		merr = joe_gettext(_("Float point exception"));
	}
	REINSTALL_SIGHANDLER(SIGFPE, fperr);
}

struct var {
	char *name;
	double (*func)(double n);
	int set;
	double val;
	struct var *next;
} *vars = NULL;

static struct var *get(const char *str)
{
	struct var *v;

	for (v = vars; v; v = v->next) {
		if (!zcmp(v->name, str)) {
			return v;
		}
	}
	v = (struct var *) joe_malloc(SIZEOF(struct var));

	v->set = 0;
	v->func = 0;
	v->val = 0;
	v->next = vars;
	vars = v;
	v->name = zdup(str);
	return v;
}

const char *ptr;
struct var *dumb;
static double eval(const char *s, int secure);

int recur=0;

/* Convert number in string to double */

double joe_strtod(const char *bptr, const char **at_eptr)
{
	int inv = 0;
	char buf[128];
#ifdef HAVE_LONG_LONG
	unsigned long long n = 0;
#else
	unsigned long n = 0;
#endif
	double x = 0.0;
	if (bptr[0] == '-') {
		inv = 1;
		++bptr;
	}
	if (bptr[0] == '0' && (bptr[1] == 'b' || bptr[1] == 'B')) {
		bptr += 2;
		while ((*bptr >= '0' && *bptr <= '1') || *bptr == '_') {
			if (*bptr >= '0' && *bptr <= '1') {
				n <<= 1;
				n += (unsigned)(*bptr - '0');
			}
			++bptr;
		}
		x = (double)n;
	} else if (bptr[0] == '0' && (bptr[1] == 'o' || bptr[1] == 'O')) {
		bptr += 2;
		while ((*bptr >= '0' && *bptr <= '7') || *bptr == '_') {
			if (*bptr >= '0' && *bptr <= '7') {
				n <<= 3;
				n += (unsigned)(*bptr - '0');
			}
			++bptr;
		}
		x = (double)n;
	} else if (bptr[0] == '0' && (bptr[1] == 'x' || bptr[1] == 'X')) {
		bptr += 2;
		while ((*bptr >= '0' && *bptr <= '9') ||
			(*bptr >= 'a' && *bptr <= 'f') ||
			(*bptr >= 'A' && *bptr <= 'F') || *bptr == '_') {
			if (*bptr >= '0' && *bptr <= '9') {
				n <<= 4;
				n += (unsigned)(*bptr - '0');
			} else if (*bptr >= 'A' && *bptr <= 'F') {
				n <<= 4;
				n += (unsigned)(*bptr - 'A' + 10);
			} else if (*bptr >= 'a' && *bptr <= 'f') {
				n <<= 4;
				n += (unsigned)(*bptr - 'a' + 10);
			}
			++bptr;
		}
		x = (double)n;
	} else {
		int j = 0;
		while ((*bptr >= '0' && *bptr <= '9') || *bptr == '_') {
			if (*bptr >= '0' && *bptr <= '9')
				buf[j++] = *bptr;
			++bptr;
		}
		if (*bptr == '.') {
			buf[j++] = *bptr++;
			while ((*bptr >= '0' && *bptr <= '9') || *bptr == '_') {
				if (*bptr >= '0' && *bptr <= '9')
					buf[j++] = *bptr;
				++bptr;
			}
		}
		if (*bptr == 'e' || *bptr == 'E') {
			buf[j++] = *bptr++;
			if (*bptr == '-' || *bptr == '+') {
				buf[j++] = *bptr++;
			}
			while ((*bptr >= '0' && *bptr <= '9') || *bptr == '_') {
				if (*bptr >= '0' && *bptr <= '9')
					buf[j++] = *bptr;
				++bptr;
			}
		}
		buf[j] = 0;
		x = strtod(buf,NULL);
	}
	if (inv)
		x = -x;
	if (at_eptr)
		*at_eptr = bptr;
	return x;
}

/* en means enable evaluation */

static double expr(int prec, int en,struct var **rtv, int secure)
{
	double x = 0.0, y, z;
	struct var *v = NULL;
	char ident[256];
	char macr[256];

	parse_wsl(&ptr, '#');

	if (!parse_ident(&ptr, ident, SIZEOF(ident))) {
		if (!secure && !zcmp(ident ,"joe")) {
			v = 0;
			x = 0.0;
			parse_wsl(&ptr, '#');
			if (*ptr=='(') {
				ptrdiff_t idx;
				MACRO *m;
				ptrdiff_t sta;
				idx = 0;
				++ptr;
				while (*ptr && *ptr != ')') {
					if (idx != SIZEOF(macr) - 1)
						macr[idx++] = *ptr;
					++ptr;
				}
				macr[idx] = 0;
				if (*ptr != ')') {
					if (!merr)
						merr = joe_gettext(_("Missing )"));
				} else {
					ptr++;
				}
				if (en) {
					m = mparse(NULL,macr,&sta,0);
					if (m) {
						x = !exmacro(m, 1, NO_MORE_DATA);
						rmmacro(m);
					} else {
						if (!merr)
							merr = joe_gettext(_("Syntax error in macro"));
					}
				}
			} else {
				if (!merr)
					merr = joe_gettext(_("Missing ("));
			}
		} else if (!en) {
			v = 0;
			x = 0.0;
		} else if (!zcmp(ident, "hex")) {
			mode_display = 2;
			v = get("ans");
			x = v->val;
		} else if (!zcmp(ident, "oct")) {
			mode_display = 3;
			v = get("ans");
			x = v->val;
		} else if (!zcmp(ident, "bin")) {
			mode_display = 4;
			v = get("ans");
			x = v->val;
		} else if (!zcmp(ident, "dec")) {
			mode_display = 0;
			v = get("ans");
			x = v->val;
		} else if (!zcmp(ident, "eng")) {
			mode_display = 1;
			v = get("ans");
			x = v->val;
		} else if (!zcmp(ident, "ins")) {
			mode_ins = 1;
			v = get("ans");
			x = v->val;
		} else if (!zcmp(ident, "sum")) {
			double xsq;
			int cnt = blksum(calc_bw, &x, &xsq);
			if (!merr && cnt<=0)
				merr = joe_gettext(_("No numbers in block"));
			v = 0;
		} else if (!zcmp(ident, "cnt")) {
			double xsq;
			int cnt = blksum(calc_bw, &x, &xsq);
			if (!merr && cnt<=0)
				merr = joe_gettext(_("No numbers in block"));
			v = 0;
			x = cnt;
		} else if (!zcmp(ident, "avg")) {
			double xsq;
			int cnt = blksum(calc_bw, &x, &xsq);
			if (!merr && cnt<=0)
				merr = joe_gettext(_("No numbers in block"));
			v = 0;
			if (cnt)
				x /= (double)cnt;
		} else if (!zcmp(ident, "dev")) {
			double xsq;
			int cnt = blksum(calc_bw, &x, &xsq);
			if (!merr && cnt<=0)
				merr = joe_gettext(_("No numbers in block"));
			v = 0;
			if (cnt) {
				x = sqrt((xsq - x*x/(double)cnt)/(double)cnt);
			}
		} else if (!zcmp(ident, "samp")) {
			double xsq;
			int cnt = blksum(calc_bw, &x, &xsq);
			if (!merr && cnt<=0)
				merr = joe_gettext(_("No numbers in block"));
			v = 0;
			if (cnt) {
				x = sqrt((xsq - x*x/(double)cnt)/(double)(cnt - 1));
			}
		} else if (!zcmp(ident, "eval")) {
			const char *save = ptr;
			char *e = blkget(calc_bw);
			if (e) {
				v = 0;
				x = eval(e,secure);
				joe_free(e);
				ptr = save;
			} else if (!merr) {
				merr = joe_gettext(_("No block"));
			}
		} else {
			v = get(ident);
			x = v->val;
		}
	} else if ((*ptr >= '0' && *ptr <= '9') || *ptr == '.') {
		const char *eptr;
		x = joe_strtod(ptr, &eptr);
		ptr = eptr;
	} else if (*ptr == '(') {
		++ptr;
		x = expr(0, en, &v, secure);
		if (*ptr == ')')
			++ptr;
		else {
			if (!merr)
				merr = joe_gettext(_("Missing )"));
		}
	} else if (*ptr == '-') {
		++ptr;
		x = -expr(10, en, &dumb, secure);
	} else if (*ptr == '!') {
		++ptr;
		x = !expr(10, en, &dumb, secure);
	}
      loop:
	while (*ptr == ' ' || *ptr == '\t')
		++ptr;
	if (*ptr == '(' && 11 > prec) {
		++ptr;
		y = expr(0, en, &dumb, secure);
		if (*ptr == ')')
			++ptr;
		else {
			if (!merr)
				merr = joe_gettext(_("Missing )"));
		}
		if (v && v->func)
			x = v->func(y);
		else {
			if (!merr)
				merr = joe_gettext(_("Called object is not a function"));
		}
		goto loop;
	} else if (*ptr == '!' && ptr[1]!='=' && 10 >= prec) {
		++ptr;
		if (x == (int)x && x>=1.0 && x<70.0) {
			y = 1.0;
			while (x>1.0) {
				y *= x;
				x -= 1.0;
			}
			x = y;
		} else {
			if (!merr)
				merr = joe_gettext(_("Factorial can only take positive integers"));
		}
		v = 0;
		goto loop;
	} else if (*ptr == '*' && ptr[1] == '*' && 8 > prec) {
		ptr+=2;
		x = pow(x, expr(8, en, &dumb, secure));
		v = 0;
		goto loop;
	} else if (*ptr == '^' && 8 > prec) {
		++ptr;
		x = pow(x, expr(8, en, &dumb, secure));
		v = 0;
		goto loop;
	} else if (*ptr == '*' && 7 > prec) {
		++ptr;
		x *= expr(7, en, &dumb, secure);
		v = 0;
		goto loop;
	} else if (*ptr == '/' && 7 > prec) {
		++ptr;
		x /= expr(7, en, &dumb, secure);
		v = 0;
		goto loop;
	} else if(*ptr=='%' && 7>prec) {
		++ptr;
		y = expr(7, en, &dumb, secure);
		if ((int)y == 0) x = 1.0/vzero;
		else x = ((int) x) % (int)y;
		v = 0;
		goto loop;
	} else if (*ptr == '+' && 6 > prec) {
		++ptr;
		x += expr(6, en, &dumb, secure);
		v = 0;
		goto loop;
	} else if (*ptr == '-' && 6 > prec) {
		++ptr;
		x -= expr(6, en, &dumb, secure);
		v = 0;
		goto loop;
	} else if (*ptr == '<' && 5 > prec) {
		++ptr;
		if (*ptr == '=') ++ptr, x = (x <= expr(5, en, &dumb, secure));
		else x = (x < expr(5,en,&dumb, secure));
		v = 0;
		goto loop;
	} else if (*ptr == '>' && 5 > prec) {
		++ptr;
		if (*ptr == '=') ++ptr, x=(x >= expr(5,en,&dumb, secure));
		else x = (x > expr(5,en,&dumb, secure));
		v = 0;
		goto loop;
	} else if (*ptr == '=' && ptr[1] == '=' && 5 > prec) {
		++ptr, ++ptr;
		x = (x == expr(5,en,&dumb, secure));
		v = 0;
		goto loop;
	} else if (*ptr == '!' && ptr[1] == '=' && 5 > prec) {
		++ptr, ++ptr;
		x = (x != expr(5,en,&dumb,secure));
		v = 0;
		goto loop;
	} else if (*ptr == '&' && ptr[1] == '&' && 3 > prec) {
		++ptr, ++ptr;
		y = expr(3,x!=0.0 && en,&dumb,secure);
		x = (int)x && (int)y;
		v = 0;
		goto loop;
	} else if (*ptr=='|' && ptr[1]=='|' &&  3 > prec) {
		++ptr, ++ptr;
		y = expr(3,x==0.0 && en,&dumb,secure);
		x = (int)x || (int)y;
		v= 0;
		goto loop;
	} else if (*ptr=='?' && 2 >= prec) {
		++ptr;
		y = expr(2,x!=0.0 && en,&dumb,secure);
		if (*ptr==':') {
			++ptr;
			z = expr(2,x==0.0 && en,&dumb,secure);
			if (x != 0.0)
				x = y;
			else
				x = z;
			v = 0;
		} else if (!merr) {
			merr = ": missing after ?";
		}
		goto loop;
	} else if (*ptr == '=' && 1 >= prec) {
		++ptr;
		x = expr(1, en,&dumb,secure);
		if (v) {
			v->val = x;
			v->set = 1;
		} else {
			if (!merr)
				merr = joe_gettext(_("Left side of = is not an l-value"));
		}
		v = 0;
		goto loop;
	}
	*rtv = v;
	return x;
}

static double eval(const char *s, int secure)
{
	double result = 0.0;
	struct var *v;
	if(++recur==1000) {
		merr = joe_gettext(_("Recursion depth exceeded"));
		--recur;
		return 0.0;
	}
	ptr = s;
	while (!merr && parse_wsl(&ptr, '#')) {
		result = expr(0, 1, &dumb,secure);
		v = get("ans");
		v->val = result;
		v->set = 1;
		if (!merr) {
			parse_wsn(&ptr, '#');
			if (*ptr == ':' || *ptr == '\r' || *ptr == '\n') {
				if (*ptr == '\r') {
					++ptr;
					if (*ptr == '\n')
						++ptr;
				} else {
					++ptr;
				}
			} else if (*ptr) {
				merr = joe_gettext(_("Extra junk after end of expr"));
			}
		}
	}
	--recur;
	return result;
}

/* These don't all exist on some systems... */

#ifdef HAVE_SIN
static double m_sin(double n) { return sin(n); }
#else
#ifdef sin
static double m_sin(double n) { return sin(n); }
#endif
#endif

#ifdef HAVE_COS
static double m_cos(double n) { return cos(n); }
#else
#ifdef cos
static double m_cos(double n) { return cos(n); }
#endif
#endif

#ifdef HAVE_TAN
static double m_tan(double n) { return tan(n); }
#else
#ifdef tan
static double m_tan(double n) { return tan(n); }
#endif
#endif

#ifdef HAVE_EXP
static double m_exp(double n) { return exp(n); }
#else
#ifdef exp
static double m_exp(double n) { return exp(n); }
#endif
#endif

#ifdef HAVE_SQRT
static double m_sqrt(double n) { return sqrt(n); }
#else
#ifdef sqrt
static double m_sqrt(double n) { return sqrt(n); }
#endif
#endif

#ifdef HAVE_CBRT
static double m_cbrt(double n) { return cbrt(n); }
#else
#ifdef cbrt
static double m_cbrt(double n) { return cbrt(n); }
#endif
#endif

#ifdef HAVE_LOG
static double m_log(double n) { return log(n); }
#else
#ifdef log
static double m_log(double n) { return log(n); }
#endif
#endif

#ifdef HAVE_LOG10
static double m_log10(double n) { return log10(n); }
#else
#ifdef log10
static double m_log10(double n) { return log10(n); }
#endif
#endif

#ifdef HAVE_ASIN
static double m_asin(double n) { return asin(n); }
#else
#ifdef asin
static double m_asin(double n) { return asin(n); }
#endif
#endif

#ifdef HAVE_ACOS
static double m_acos(double n) { return acos(n); }
#else
#ifdef acos
static double m_acos(double n) { return acos(n); }
#endif
#endif

#ifdef HAVE_ATAN
static double m_atan(double n) { return atan(n); }
#else
#ifdef atan
static double m_atan(double n) { return atan(n); }
#endif
#endif

#ifdef HAVE_SINH
static double m_sinh(double n) { return sinh(n); }
#else
#ifdef sinh
static double m_sinh(double n) { return sinh(n); }
#endif
#endif

#ifdef HAVE_COSH
static double m_cosh(double n) { return cosh(n); }
#else
#ifdef cosh
static double m_cosh(double n) { return cosh(n); }
#endif
#endif

#ifdef HAVE_TANH
static double m_tanh(double n) { return tanh(n); }
#else
#ifdef tanh
static double m_tanh(double n) { return tanh(n); }
#endif
#endif

#ifdef HAVE_ASINH
static double m_asinh(double n) { return asinh(n); }
#else
#ifdef asinh
static double m_asinh(double n) { return asinh(n); }
#endif
#endif

#ifdef HAVE_ACOSH
static double m_acosh(double n) { return acosh(n); }
#else
#ifdef acosh
static double m_acosh(double n) { return acosh(n); }
#endif
#endif

#ifdef HAVE_ATANH
static double m_atanh(double n) { return atanh(n); }
#else
#ifdef atanh
static double m_atanh(double n) { return atanh(n); }
#endif
#endif

#ifdef HAVE_FLOOR
static double m_floor(double n) { return floor(n); }
#else
#ifdef floor
static double m_floor(double n) { return floor(n); }
#endif
#endif

#ifdef HAVE_CEIL
static double m_ceil(double n) { return ceil(n); }
#else
#ifdef ceil
static double m_ceil(double n) { return ceil(n); }
#endif
#endif

#ifdef HAVE_FABS
static double m_fabs(double n) { return fabs(n); }
#else
#ifdef fabs
static double m_fabs(double n) { return fabs(n); }
#else
static double m_fabs(double n) { return (n < 0.0) ? -n : n; }
#endif
#endif

#ifdef HAVE_ERF
static double m_erf(double n) { return erf(n); }
#else
#ifdef erf
static double m_erf(double n) { return erf(n); }
#endif
#endif

#ifdef HAVE_ERFC
static double m_erfc(double n) { return erfc(n); }
#else
#ifdef erfc
static double m_erfc(double n) { return erfc(n); }
#endif
#endif

#ifdef HAVE_J0
static double m_j0(double n) { return j0(n); }
#else
#ifdef j0
static double m_j0(double n) { return j0(n); }
#endif
#endif

#ifdef HAVE_J1
static double m_j1(double n) { return j1(n); }
#else
#ifdef j1
static double m_j1(double n) { return j1(n); }
#endif
#endif

#ifdef HAVE_Y0
static double m_y0(double n) { return y0(n); }
#else
#ifdef y0
static double m_y0(double n) { return y0(n); }
#endif
#endif

#ifdef HAVE_Y1
static double m_y1(double n) { return y1(n); }
#else
#ifdef y1
static double m_y1(double n) { return y1(n); }
#endif
#endif

#ifdef HAVE_HYPOT
static double m_hypot(double n, double m) { return hypot(n, m); }
#else
#ifdef hypot
static double m_hypot(double n, double m) { return hypot(n, m); }
#endif
#endif


static double m_int(double n) { return (int)(n); }

static double m_lr(double n)
{
	struct var *v;
	double xsq;
	double xsum;
	double ysq;
	double ysum;
	double xy;
	double A;
	double BB;
	double r;
	double cov;
	double xavg;
	double yavg;
	int cnt = blklr(calc_bw, &xsum, &xsq, &ysum, &ysq, &xy, 0, 0);
	if (!merr && cnt<=0) {
		merr = joe_gettext(_("No numbers in block"));
		return 0.0;
	}
	/* Linear regression coefficients: y = A + BB * x */
	BB = ((double)cnt * xy - xsum * ysum) / ((double)cnt * xsq - xsum * xsum);
	A = (ysum - BB * xsum) / (double)cnt;
	/* correlation coefficient */
	r = ((double)cnt * xy - xsum * ysum) / sqrt(m_fabs((double)cnt * xsq - xsum * xsum) * m_fabs((double)cnt * ysq - ysum * ysum));
	/* covariance */
	xavg = xsum / (double)cnt;
	yavg = ysum / (double)cnt;
	cov = (xy - (double)cnt * xavg * yavg) / (double)(cnt - 1);

	/* Side effects */
	v = get("b"); v->val = A; v->set = 1;
	v = get("m"); v->val = BB; v->set = 1;
	v = get("r"); v->val = r; v->set = 1;
	v = get("cov"); v->val = cov; v->set = 1;

	return A + BB * n;
}

static double m_Lr(double n)
{
	struct var *v;
	double xsq;
	double xsum;
	double ysq;
	double ysum;
	double xy;
	double A;
	double BB;
	double r;
	double cov;
	double xavg;
	double yavg;
	int cnt = blklr(calc_bw, &xsum, &xsq, &ysum, &ysq, &xy, 1, 0);
	if (!merr && cnt<=0) {
		merr = joe_gettext(_("No numbers in block"));
		return 0.0;
	}
	/* Linear regression coefficients: y = A + BB * x */
	BB = ((double)cnt * xy - xsum * ysum) / ((double)cnt * xsq - xsum * xsum);
	A = (ysum - BB * xsum) / (double)cnt;
	/* correlation coefficient */
	r = ((double)cnt * xy - xsum * ysum) / sqrt(m_fabs((double)cnt * xsq - xsum * xsum) * m_fabs((double)cnt * ysq - ysum * ysum));
	/* covariance */
	xavg = xsum / (double)cnt;
	yavg = ysum / (double)cnt;
	cov = (xy - (double)cnt * xavg * yavg) / (double)(cnt - 1);

	/* Side effects */
	v = get("b"); v->val = A; v->set = 1;
	v = get("m"); v->val = BB; v->set = 1;
	v = get("r"); v->val = r; v->set = 1;
	v = get("cov"); v->val = cov; v->set = 1;

	return A + BB * log(n);
}

static double m_lR(double n)
{
	struct var *v;
	double xsq;
	double xsum;
	double ysq;
	double ysum;
	double xy;
	double A;
	double BB;
	double r;
	double cov;
	double xavg;
	double yavg;
	int cnt = blklr(calc_bw, &xsum, &xsq, &ysum, &ysq, &xy, 0, 1);
	if (!merr && cnt<=0) {
		merr = joe_gettext(_("No numbers in block"));
		return 0.0;
	}
	/* Linear regression coefficients: y = A + BB * x */
	BB = ((double)cnt * xy - xsum * ysum) / ((double)cnt * xsq - xsum * xsum);
	A = (ysum - BB * xsum) / (double)cnt;
	/* correlation coefficient */
	r = ((double)cnt * xy - xsum * ysum) / sqrt(m_fabs((double)cnt * xsq - xsum * xsum) * m_fabs((double)cnt * ysq - ysum * ysum));
	/* covariance */
	xavg = xsum / (double)cnt;
	yavg = ysum / (double)cnt;
	cov = (xy - (double)cnt * xavg * yavg) / (double)(cnt - 1);

	/* Side effects */
	v = get("b"); v->val = exp(A); v->set = 1;
	v = get("m"); v->val = BB; v->set = 1;
	v = get("r"); v->val = r; v->set = 1;
	v = get("cov"); v->val = cov; v->set = 1;

	return exp(A + BB * n);
}

static double m_LR(double n)
{
	struct var *v;
	double xsq;
	double xsum;
	double ysq;
	double ysum;
	double xy;
	double A;
	double BB;
	double r;
	double cov;
	double xavg;
	double yavg;
	int cnt = blklr(calc_bw, &xsum, &xsq, &ysum, &ysq, &xy, 1, 1);
	if (!merr && cnt<=0) {
		merr = joe_gettext(_("No numbers in block"));
		return 0.0;
	}
	/* Linear regression coefficients: y = A + BB * x */
	BB = ((double)cnt * xy - xsum * ysum) / ((double)cnt * xsq - xsum * xsum);
	A = (ysum - BB * xsum) / (double)cnt;
	/* correlation coefficient */
	r = ((double)cnt * xy - xsum * ysum) / sqrt(m_fabs((double)cnt * xsq - xsum * xsum) * m_fabs((double)cnt * ysq - ysum * ysum));
	/* covariance */
	xavg = xsum / (double)cnt;
	yavg = ysum / (double)cnt;
	cov = (xy - (double)cnt * xavg * yavg) / (double)(cnt - 1);

	/* Side effects */
	v = get("b"); v->val = exp(A); v->set = 1;
	v = get("m"); v->val = BB; v->set = 1;
	v = get("r"); v->val = r; v->set = 1;
	v = get("cov"); v->val = cov; v->set = 1;

	return exp(A + BB * log(n));
}

static double m_rlr(double n)
{
	struct var *v;
	double xsq;
	double xsum;
	double ysq;
	double ysum;
	double xy;
	double A;
	double BB;
	double r;
	double cov;
	double xavg;
	double yavg;
	int cnt = blklr(calc_bw, &xsum, &xsq, &ysum, &ysq, &xy, 0, 0);
	if (!merr && cnt<=0) {
		merr = joe_gettext(_("No numbers in block"));
		return 0.0;
	}
	/* Linear regression coefficients: y = A + BB * x */
	BB = ((double)cnt * xy - xsum * ysum) / ((double)cnt * xsq - xsum * xsum);
	A = (ysum - BB * xsum) / (double)cnt;
	/* correlation coefficient */
	r = ((double)cnt * xy - xsum * ysum) / sqrt(m_fabs((double)cnt * xsq - xsum * xsum) * m_fabs((double)cnt * ysq - ysum * ysum));
	/* covariance */
	xavg = xsum / (double)cnt;
	yavg = ysum / (double)cnt;
	cov = (xy - (double)cnt * xavg * yavg) / (double)(cnt - 1);

	/* Side effects */
	v = get("b"); v->val = A; v->set = 1;
	v = get("m"); v->val = BB; v->set = 1;
	v = get("r"); v->val = r; v->set = 1;
	v = get("cov"); v->val = cov; v->set = 1;

	return (n - A) / BB;
}

static double m_rLr(double n)
{
	struct var *v;
	double xsq;
	double xsum;
	double ysq;
	double ysum;
	double xy;
	double A;
	double BB;
	double r;
	double cov;
	double xavg;
	double yavg;
	int cnt = blklr(calc_bw, &xsum, &xsq, &ysum, &ysq, &xy, 1, 0);
	if (!merr && cnt<=0) {
		merr = joe_gettext(_("No numbers in block"));
		return 0.0;
	}
	/* Linear regression coefficients: y = A + BB * x */
	BB = ((double)cnt * xy - xsum * ysum) / ((double)cnt * xsq - xsum * xsum);
	A = (ysum - BB * xsum) / (double)cnt;
	/* correlation coefficient */
	r = ((double)cnt * xy - xsum * ysum) / sqrt(m_fabs((double)cnt * xsq - xsum * xsum) * m_fabs((double)cnt * ysq - ysum * ysum));
	/* covariance */
	xavg = xsum / (double)cnt;
	yavg = ysum / (double)cnt;
	cov = (xy - (double)cnt * xavg * yavg) / (double)(cnt - 1);

	/* Side effects */
	v = get("b"); v->val = A; v->set = 1;
	v = get("m"); v->val = BB; v->set = 1;
	v = get("r"); v->val = r; v->set = 1;
	v = get("cov"); v->val = cov; v->set = 1;

	return exp((n - A) / BB);
}

static double m_rlR(double n)
{
	struct var *v;
	double xsq;
	double xsum;
	double ysq;
	double ysum;
	double xy;
	double A;
	double BB;
	double r;
	double cov;
	double xavg;
	double yavg;
	int cnt = blklr(calc_bw, &xsum, &xsq, &ysum, &ysq, &xy, 0, 1);
	if (!merr && cnt<=0) {
		merr = joe_gettext(_("No numbers in block"));
		return 0.0;
	}
	/* Linear regression coefficients: y = A + BB * x */
	BB = ((double)cnt * xy - xsum * ysum) / ((double)cnt * xsq - xsum * xsum);
	A = (ysum - BB * xsum) / (double)cnt;
	/* correlation coefficient */
	r = ((double)cnt * xy - xsum * ysum) / sqrt(m_fabs((double)cnt * xsq - xsum * xsum) * m_fabs((double)cnt * ysq - ysum * ysum));
	/* covariance */
	xavg = xsum / (double)cnt;
	yavg = ysum / (double)cnt;
	cov = (xy - (double)cnt * xavg * yavg) / (double)(cnt - 1);

	/* Side effects */
	v = get("b"); v->val = exp(A); v->set = 1;
	v = get("m"); v->val = BB; v->set = 1;
	v = get("r"); v->val = r; v->set = 1;
	v = get("cov"); v->val = cov; v->set = 1;

	return (log(n) - A) / BB;
}

static double m_rLR(double n)
{
	struct var *v;
	double xsq;
	double xsum;
	double ysq;
	double ysum;
	double xy;
	double A;
	double BB;
	double r;
	double cov;
	double xavg;
	double yavg;
	int cnt = blklr(calc_bw, &xsum, &xsq, &ysum, &ysq, &xy, 1, 1);
	if (!merr && cnt<=0) {
		merr = joe_gettext(_("No numbers in block"));
		return 0.0;
	}
	/* Linear regression coefficients: y = A + BB * x */
	BB = ((double)cnt * xy - xsum * ysum) / ((double)cnt * xsq - xsum * xsum);
	A = (ysum - BB * xsum) / (double)cnt;
	/* correlation coefficient */
	r = ((double)cnt * xy - xsum * ysum) / sqrt(m_fabs((double)cnt * xsq - xsum * xsum) * m_fabs((double)cnt * ysq - ysum * ysum));
	/* covariance */
	xavg = xsum / (double)cnt;
	yavg = ysum / (double)cnt;
	cov = (xy - (double)cnt * xavg * yavg) / (double)(cnt - 1);

	/* Side effects */
	v = get("b"); v->val = exp(A); v->set = 1;
	v = get("m"); v->val = BB; v->set = 1;
	v = get("r"); v->val = r; v->set = 1;
	v = get("cov"); v->val = cov; v->set = 1;

	return exp((log(n) - A) / BB);
}

static void setup_vars(BW *tbw)
{
	struct var *v;
	int c = brch(tbw->cursor);
	if (!vars) {
#ifdef HAVE_SIN
		v = get("sin"); v->func = m_sin;
#else
#ifdef sin
		v = get("sin"); v->func = m_sin;
#endif
#endif
#ifdef HAVE_COS
		v = get("cos"); v->func = m_cos;
#else
#ifdef cos
		v = get("cos"); v->func = m_cos;
#endif
#endif
#ifdef HAVE_TAN
		v = get("tan"); v->func = m_tan;
#else
#ifdef tan
		v = get("tan"); v->func = m_tan;
#endif
#endif
#ifdef HAVE_EXP
		v = get("exp"); v->func = m_exp;
#else
#ifdef exp
		v = get("exp"); v->func = m_exp;
#endif
#endif
#ifdef HAVE_SQRT
		v = get("sqrt"); v->func = m_sqrt;
#else
#ifdef sqrt
		v = get("sqrt"); v->func = m_sqrt;
#endif
#endif
#ifdef HAVE_CBRT
		v = get("cbrt"); v->func = m_cbrt;
#else
#ifdef cbrt
		v = get("cbrt"); v->func = m_cbrt;
#endif
#endif
#ifdef HAVE_LOG
		v = get("ln"); v->func = m_log;
#else
#ifdef log
		v = get("ln"); v->func = m_log;
#endif
#endif
#ifdef HAVE_LOG10
		v = get("log"); v->func = m_log10;
#else
#ifdef log10
		v = get("log"); v->func = m_log10;
#endif
#endif
#ifdef HAVE_ASIN
		v = get("asin"); v->func = m_asin;
#else
#ifdef asin
		v = get("asin"); v->func = m_asin;
#endif
#endif
#ifdef HAVE_ACOS
		v = get("acos"); v->func = m_acos;
#else
#ifdef acos
		v = get("acos"); v->func = m_acos;
#endif
#endif
#ifdef HAVE_ATAN
		v = get("atan"); v->func = m_atan;
#else
#ifdef atan
		v = get("atan"); v->func = m_atan;
#endif
#endif
#ifdef M_PI
		v = get("pi"); v->val = M_PI; v->set = 1;
#endif
#ifdef M_E
		v = get("e"); v->val = M_E; v->set = 1;
#endif
#ifdef HAVE_SINH
		v = get("sinh"); v->func = m_sinh;
#else
#ifdef sinh
		v = get("sinh"); v->func = m_sinh;
#endif
#endif
#ifdef HAVE_COSH
		v = get("cosh"); v->func = m_cosh;
#else
#ifdef cosh
		v = get("cosh"); v->func = m_cosh;
#endif
#endif
#ifdef HAVE_TANH
		v = get("tanh"); v->func = m_tanh;
#else
#ifdef tanh
		v = get("tanh"); v->func = m_tanh;
#endif
#endif
#ifdef HAVE_ASINH
		v = get("asinh"); v->func = m_asinh;
#else
#ifdef asinh
		v = get("asinh"); v->func = m_asinh;
#endif
#endif
#ifdef HAVE_ACOSH
		v = get("acosh"); v->func = m_acosh;
#else
#ifdef acosh
		v = get("acosh"); v->func = m_acosh;
#endif
#endif
#ifdef HAVE_ATANH
		v = get("atanh"); v->func = m_atanh;
#else
#ifdef atanh
		v = get("atanh"); v->func = m_atanh;
#endif
#endif
#ifdef HAVE_FLOOR
		v = get("floor"); v->func = m_floor;
#else
#ifdef floor
		v = get("floor"); v->func = m_floor;
#endif
#endif
#ifdef HAVE_CEIL
		v = get("ceil"); v->func = m_ceil;
#else
#ifdef ceil
		v = get("ceil"); v->func = m_ceil;
#endif
#endif
#ifdef HAVE_FABS
		v = get("abs"); v->func = m_fabs;
#else
#ifdef fabs
		v = get("abs"); v->func = m_fabs;
#endif
#endif
#ifdef HAVE_ERF
		v = get("erf"); v->func = m_erf;
#else
#ifdef erf
		v = get("erf"); v->func = m_erf;
#endif
#endif
#ifdef HAVE_ERFC
		v = get("erfc"); v->func = m_erfc;
#else
#ifdef erfc
		v = get("erfc"); v->func = m_erfc;
#endif
#endif
#ifdef HAVE_J0
		v = get("j0"); v->func = m_j0;
#else
#ifdef j0
		v = get("j0"); v->func = m_j0;
#endif
#endif
#ifdef HAVE_J1
		v = get("j1"); v->func = m_j1;
#else
#ifdef j1
		v = get("j1"); v->func = m_j1;
#endif
#endif
#ifdef HAVE_Y0
		v = get("y0"); v->func = m_y0;
#else
#ifdef y0
		v = get("y0"); v->func = m_y0;
#endif
#endif
#ifdef HAVE_Y1
		v = get("y1"); v->func = m_y1;
#else
#ifdef y1
		v = get("y1"); v->func = m_y1;
#endif
#endif
		v = get("int"); v->func = m_int;
		v = get("lr"); v->func = m_lr;
		v = get("rlr"); v->func = m_rlr;
		v = get("Lr"); v->func = m_Lr;
		v = get("rLr"); v->func = m_rLr;
		v = get("lR"); v->func = m_lR;
		v = get("rlR"); v->func = m_rlR;
		v = get("LR"); v->func = m_LR;
		v = get("rLR"); v->func = m_rLR;
	}

	v = get("top");
	v->val = (double)(tbw->top->line + 1);
	v->set = 1;
	v = get("lines");
	v->val = (double)(tbw->b->eof->line + 1);
	v->set = 1;
	v = get("line");
	v->val = (double)(tbw->cursor->line + 1);
	v->set = 1;
	v = get("col");
	v->val = (double)(tbw->cursor->col + 1);
	v->set = 1;
	v = get("byte");
	v->val = (double)(tbw->cursor->byte + 1);
	v->set = 1;
	v = get("size");
	v->val = (double)tbw->b->eof->byte;
	v->set = 1;
	v = get("height");
	v->val = (double)tbw->h;
	v->set = 1;
	v = get("width");
	v->val = (double)tbw->w;
	v->set = 1;
	v = get("char");
	v->val = (c == NO_MORE_DATA ? -1.0 : c);
	v->set = 1;
	v = get("markv");
	v->val = markv(1) ? 1.0 : 0.0;
	v->set = 1;
	v = get("rdonly");
	v->val = tbw->b->rdonly;
	v->set = 1;
	v = get("arg");
	v->val = current_arg;
	v->set = 1;
	v = get("argset");
	v->val = current_arg_set;
	v->set = 1;
	v = get("no_windows");
	v->val = countmain(tbw->parent->t);
	v->set = 1;
	merr = 0;
	v = get("is_shell");
	v->val = tbw->b->pid;
	v->set = 1;
}

double calc(BW *bw, char *s, int secure)
{
	calc_bw = bw;
	setup_vars(calc_bw);
	merr = 0;
	return eval(s, secure);
}

static void insert_commas(char *dst, char *src)
{
	int leading; /* Number of leading digits */
	int trailing;
	int n;
	char *s;
	/* First pass: calculate number of leading and trailing digits */
	s = src;
	leading = 0;
	trailing = 0;
	if (*s == '-') ++s;
	else if (*s == '+') ++s;
	while (*s >= '0' && *s <= '9') {
		++s;
		++leading;
	}
	if (*s == '.') {
		++s;
		while (*s >= '0' && *s <= '9') {
			++s;
			++trailing;
		}
	}
	/* Second pass: copy while inserting */
	s = src;
	n = 0;
	if (*s == '-') {
		*dst++ = *s++;
	} else if (*s == '+') {
		*dst++ = *s++;
	}
	while (*s >= '0' && *s <= '9') {
		*dst++ = *s++;
		--leading;
		if (leading != 0 && (leading % 3 == 0))
			*dst++ = '_';
	}
	if (*s == '.') {
		*dst++ = *s++;
		while (*s >= '0' && *s <= '9') {
			*dst++ = *s++;
			++n;
			if (n != trailing && (n % 3 == 0))
				*dst++ = '_';
		}
	}
	while (*s) {
		*dst++ = *s++;
	}
	*dst = 0;
}

/* Convert floating point ascii number into engineering format */

static char *eng(char *d, ptrdiff_t d_len, const char *s)
{
	const char *s_org = s;
	char *d_org = d;
	char a[128];
	int a_len;
	int sign;
	int dp;
	int myexp;
	int exp_sign;
	int x;
	int flg = 0;

	--d_len; /* For terminator */

	/* Get sign of number */
	if (*s == '-') {
		sign = 1;
		++s;
	} else if (*s == '+') {
		sign = 0;
		++s;
	} else {
		sign = 0;
	}

	/* Read digits before decimal point */
	/* Skip leading zeros */
	while (*s == '0') {
		flg = 1;
		++s;
	}
	a_len = 0;
	while (*s >= '0' && *s <= '9') {
		a[a_len++] = *s++;
		flg = 1;
	}
	/* Decimal point? */
	dp = 0;
	if (*s == '.') {
		flg = 1;
		++s;
		/* If we don't have any digits yet, trim leading zeros */
		if (!a_len) {
			while (*s == '0') {
				++s;
				++dp;
			}
		}
		if (*s >= '0' && *s <= '9') {
			while (*s >= '0' && *s <= '9') {
				a[a_len++] = *s++;
				++dp;
			}
		} else {
			/* No non-zero digits at all? */
			dp  = 0;
		}
	}
	/* Trim trailing zeros */
	while (a_len && a[a_len - 1] == '0') {
		--a_len;
		--dp;
	}

	/* Exponent? */
	if (*s == 'e' || *s == 'E') {
		++s;
		if (*s == '-') {
			++s;
			exp_sign = 1;
		} else if (*s == '+') {
			++s;
			exp_sign = 0;
		} else {
			exp_sign = 0;
		}
		myexp = 0;
		while  (*s >= '0' && *s <= '9') {
			myexp = myexp * 10 + *s++ - '0';
		}
	} else {
		myexp = 0;
		exp_sign = 0;
	}

	/* s should be at end of number now */
	if (!flg) { /* No digits found? */
		zlcpy(d, d_len, s_org);
		return d;
	}

	/* Sign of exponent */
	if (exp_sign)
		myexp = -myexp;
	/* Account of position of decimal point in exponent */
	myexp -= dp;

	a[a_len] = 0;

	/* For engineering format, make exponent a multiple of 3 such that
	   we have 1 - 3 leading digits */

	/* Don't assume modulus of negative number works consistently */
	if (myexp < 0) {
		switch((-myexp) % 3) {
			case 0: x = 0; break;
			case 1: x = 2; break;
			case 2: x = 1; break;
		}
	} else {
		x = (myexp % 3);
	}

	/* Make exponent a multiple of 3 */
	myexp -= x;

	/* Add zeros to mantissa to account for this */
	while (x--) {
		a[a_len++] = '0';
	}

	/* If number has no digits, add one now */
	if (!a_len)
		a[a_len++] = '0';

	/* Position decimal point near the left */
	dp = (a_len - 1) / 3;
	dp *= 3;

	/* Adjust exponent for this */
	myexp += dp;

	/* Now print */
	if (sign && d_len) {
		*d++ = '-';
		--d_len;
	}

	/* Digits to left of decimal point */
	for (x = 0; x != a_len - dp; ++x) {
		if (d_len) {
			*d++ = a[x];
			--d_len;
		}
	}

	/* Any more digits? */
	if (dp) {
		if (d_len) {
			*d++ = '.';
			--d_len;
		}
		for (x = a_len - dp; x != a_len; ++x) {
			if (d_len) {
				*d++ = a[x];
				--d_len;
			}
		}
		/* Trim trailing zeros */
		while (d != d_org && d[-1] == '0') {
			--d;
			++d_len;
		}
	}

	/* Exponent? */
	if (myexp) {
		joe_snprintf_1(d, (size_t)(d_len + 1), "e%d", myexp);
	} else {
		*d = 0;
	}

	return d_org;
}

static int doumath(W *w, char *s, void *object, int *notify);

/* Main user interface */
static int domath(W *w, char *s, void *object, int *notify, int secure)
{
	char buf[128];
	char buf1[128];
	double result;
	BW *bw;
	WIND_BW(bw, w);
	result = calc(bw, s, secure);

	if (notify) {
		*notify = 1;
	}
	if (merr) {
		msgnw(bw->parent, merr);
		return -1;
	}
	vsrm(s);
	switch (mode_display) {
		case 0: { /* Normal */
			joe_snprintf_1(buf, SIZEOF(buf), "%.16G", result);
			insert_commas(msgbuf, buf);
			break;
		} case 1: { /* Engineering */
			joe_snprintf_1(buf1, SIZEOF(buf1), "%.16G", result);
			eng(buf, sizeof(buf), buf1);
			insert_commas(msgbuf, buf);
			break;
		} case 2: { /* Hex */
#ifdef HAVE_LONG_LONG
			unsigned long long n = (unsigned long long)result;
#else
			unsigned long n = (unsigned long)result;
#endif
			int ofst = 0;
			int len = 0;
			int cnt = 0;
			msgbuf[ofst++] = '0';
			msgbuf[ofst++] = 'x';
			while (n) {
				buf[len++] = "0123456789ABCDEF"[n & 15];
				n >>= 4;
			}
			if (!len)
				buf[len++] = '0';
			cnt = len;
			do {
				if (len && cnt != len && (len & 3) == 0)
					msgbuf[ofst++] = '_';
				msgbuf[ofst++] = buf[len - 1];
			} while (--len);
			msgbuf[ofst] = 0;
/*
#ifdef HAVE_LONG_LONG
			joe_snprintf_1(msgbuf, JOE_MSGBUFSIZE, "0x%llX", (long long)result);
#else
			joe_snprintf_1(msgbuf, JOE_MSGBUFSIZE, "0x%lX", (long)result);
#endif
*/
			break;
		} case 3: { /* Octal */
#ifdef HAVE_LONG_LONG
			unsigned long long n = (unsigned long long)result;
#else
			unsigned long n = (unsigned long)result;
#endif
			int ofst = 0;
			int len = 0;
			int cnt = 0;
			msgbuf[ofst++] = '0';
			msgbuf[ofst++] = 'o';
			while (n) {
				buf[len++] = "01234567"[n & 7];
				n >>= 3;
			}
			if (!len)
				buf[len++] = '0';
			cnt = len;
			do {
				if (len && cnt != len && (len & 3) == 0)
					msgbuf[ofst++] = '_';
				msgbuf[ofst++] = buf[len - 1];
			} while (--len);
			msgbuf[ofst] = 0;
/*
#ifdef HAVE_LONG_LONG
			joe_snprintf_1(msgbuf, JOE_MSGBUFSIZE, "0o%llo", (long long)result);
#else
			joe_snprintf_1(msgbuf, JOE_MSGBUFSIZE, "0o%lo", (long)result);
#endif
*/
			break;
		} case 4: { /* Binary */
#ifdef HAVE_LONG_LONG
			unsigned long long n = (unsigned long long)result;
#else
			unsigned long n = (unsigned long)result;
#endif
			int ofst = 0;
			int len = 0;
			int cnt = 0;
			msgbuf[ofst++] = '0';
			msgbuf[ofst++] = 'b';
			while (n) {
				buf[len++] = (char)('0' + (char)(n & 1));
				n >>= 1;
			}
			if (!len)
				buf[len++] = '0';
			cnt = len;
			do {
				if (len && cnt != len && (len & 3) == 0)
					msgbuf[ofst++] = '_';
				msgbuf[ofst++] = buf[len - 1];
			} while (--len);
			msgbuf[ofst] = 0;
			break;
		}
	}

	if (bw->parent->watom->what != TYPETW || mode_ins) {
		binsm(bw->cursor, sz(msgbuf));
		pfwrd(bw->cursor, zlen(msgbuf));
		bw->cursor->xcol = piscol(bw->cursor);
	} else {
		msgnw(bw->parent, msgbuf);
	}
	if (mode_ins) { /* Exit if we are inserting */
		mode_ins = 0;
		return 0;
	} else {
		return 0;
#ifdef junk
		/* Stay at math prompt */
		if (wmkpw(w, "=", &mathhist, doumath, "Math", NULL, NULL, NULL, NULL, utf8_map, 0)) {
			return 0;
		} else {
			return -1;
		}
#endif
	}
}

static int doumath(W *w, char *s, void *object, int *notify)
{
	return domath(w, s, object, notify, 0);
}

static int dosmath(W *w, char *s, void *object, int *notify)
{
	return domath(w, s, object, notify, 1);
}


B *mathhist = NULL;

static char **math_word_list;

static void get_math_list(void)
{
	struct var *v;
	char *s;
	varm(math_word_list);
	math_word_list = 0;
	for (v = vars; v; v = v->next) {
		s = vsncpy(NULL, 0, sz(v->name));
		math_word_list = vaadd(math_word_list, s);
	}
}

int math_cmplt(BW *bw, int k)
{
	setup_vars(bw);
	get_math_list();

	if (!math_word_list) {
		ttputc(7);
		return 0;
	}

	return word_cmplt(bw, math_word_list);
}

int umath(W *w, int k)
{
	joe_set_signal(SIGFPE, fperr);
	if (wmkpw(w, "=", &mathhist, doumath, "Math", NULL, math_cmplt, NULL, NULL, utf8_map, 0)) {
		return 0;
	} else {
		return -1;
	}
}

/* Secure version: no macros allowed */

int usmath(W *w, int k)
{
	joe_set_signal(SIGFPE, fperr);
	if (wmkpw(w, "=", &mathhist, dosmath, "Math", NULL, math_cmplt, NULL, NULL, utf8_map, 0)) {
		return 0;
	} else {
		return -1;
	}
}
