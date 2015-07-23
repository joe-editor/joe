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

struct uniblock {
	int first;
	int last;
	char *name;
};

extern struct uniblock uniblocks[];

int *lowerize(int *d, ptrdiff_t len, const int *s);

struct unicat *unicatlookup(const char *name);

int unicatcheck(struct unicat *cat, int ch);
