struct unicat {
	const char *name;
	int len;
	struct interval *intervals;
	struct unicat *next;
};

/* Tables in unicat.c */

extern struct unicat unicat[];	/* Catgories */
extern struct interval toupper_table[];	/* Convert to uppercase */
extern int toupper_cvt[];
extern struct interval tolower_table[];	/* Convert to lowercase */
extern int tolower_cvt[];
extern struct interval totitle_table[];	/* Convert to titlecase */
extern int totitle_cvt[];
extern struct interval fold_table[]; /* Case folding table */
#define REPLLEN 3
#define FOLDMAGIC 0x4000000
extern struct Rtree rtree_fold[1];
extern int fold_repl[][REPLLEN];
extern struct interval width_table[]; /* Double-wide characters */

int *lowerize(int *d, ptrdiff_t len, const int *s);

struct Cclass *unicode(const char *name);

extern struct Cclass cclass_upper[1];
extern struct Cclass cclass_lower[1];
extern struct Cclass cclass_alpha[1];
extern struct Cclass cclass_alpha_[1];
extern struct Cclass cclass_alnum[1];
extern struct Cclass cclass_alnum_[1];
extern struct Cclass cclass_digit[1];
extern struct Cclass cclass_notdigit[1];
extern struct Cclass cclass_xdigit[1];
extern struct Cclass cclass_punct[1];
extern struct Cclass cclass_space[1];
extern struct Cclass cclass_notspace[1];
extern struct Cclass cclass_blank[1];
extern struct Cclass cclass_ctrl[1];
extern struct Cclass cclass_graph[1];
extern struct Cclass cclass_print[1];
extern struct Cclass cclass_word[1];
extern struct Cclass cclass_notword[1];
extern struct Cclass cclass_combining[1];

void joe_iswinit();

int joe_iswupper(struct charmap *,int c);
int joe_iswlower(struct charmap *,int c);

int joe_iswalpha(struct charmap *,int c);	/* or _ */
int joe_iswalpha_(struct charmap *,int c);

int joe_iswalnum(struct charmap *,int c);
int joe_iswalnum_(struct charmap *,int c);

int joe_iswdigit(struct charmap *,int c);
int joe_iswspace(struct charmap *,int c);
int joe_iswctrl(struct charmap *,int c);
int joe_iswpunct(struct charmap *,int c);
int joe_iswgraph(struct charmap *,int c);
int joe_iswprint(struct charmap *,int c);
int joe_iswxdigit(struct charmap *,int c);
int joe_iswblank(struct charmap *,int c);

/* Value of any \p{Nd} digit */
int digval(int ch);

int joe_wcwidth(int wide,int c);
/* Looking for wswidth? Take a look at scrn.c/txtwidth() */

int joe_towupper(struct charmap *,int c);
int joe_towlower(struct charmap *,int c);

int unictrl(int c);
