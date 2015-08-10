struct unicat {
	char *name;
	int size;
	struct interval *table;
	struct unicat *next;
};

extern struct unicat unicat[];

struct casefold {
	int first;
	int last;
	int a;
	int b;
	int c;
};

extern struct casefold fold_table[];

int *lowerize(int *d, ptrdiff_t len, const int *s);

struct unicat *unicatlookup(const char *name);

int unicatcheck(struct unicat *cat, int ch);

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
