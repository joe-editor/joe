/*
 *	Functions which make use of the unicode facts in unifacts.c
 *
 *	Copyright
 *		(C) 2015 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#include "types.h"

/* Convert string to lowercase for case folding */

/* Perfect hash table of character to lowercase character or to fold_table index if it's a string
   Table index is indicated by a -10000 + index
*/

PHASH *fold_hash_table;

void init_fold_hash_table()
{
	int x;
	fold_hash_table = mkphash(0x1FF, 512);
	for (x = 0; fold_table[x].first; ++x) {
		unsigned y;
		for (y = fold_table[x].first; y <= fold_table[x].last; ++y)
			phash_add(fold_hash_table, y, fold_table[x].b ? (-10000 + x) : y - fold_table[x].first + fold_table[x].a);
	}
}

/* Convert unicode string to lowercase */

int *lowerize(int *d, ptrdiff_t len, const int *s)
{
	int *org = d;
	if (!fold_hash_table)
		init_fold_hash_table();
	if (!len) {
		fprintf(stderr, "lowerize called with len == 0\n");
		exit(1);
	}
	--len;
	while (len && *s) {
		int idx = phash_find(fold_hash_table, *s);
		if (idx == -1) { /* Not in table */
			*d++ = *s++;
			--len;
		} else if (idx >= 0) { /* Replace it with a single character */
			++s;
			*d++ = idx;
			--len;
		} else { /* Replace it with a string */
			idx += 10000;
			++s;
			*d++ = fold_table[idx].a;
			--len;
			if (len && fold_table[idx].b) {
				*d++ = fold_table[idx].b;
				--len;
				if (len && fold_table[idx].c) {
					*d++ = fold_table[idx].c;
					--len;
				}
			}
		}
	}
	*d = 0;
	return org;
}

/* Lookup unicode category */

static struct unicat *derived;

struct unicat *unicatlookup(const char *cat)
{
	struct interval_list *l = 0;
	struct cmap cmap;
	time_t ti;

	struct unicat *der;
	ptrdiff_t x, y, z, a;
	ptrdiff_t size;
	struct Rtree *r;
	/* Native category like Lu */
	for (x = 0; unicat[x].name; ++x)
		if (!zcmp(unicat[x].name, cat))
			return &unicat[x];
	/* Derived category like L: maybe we already have it? */
	for (der = derived; der; der = der->next)
		if (!zcmp(der->name, cat))
			return der;
	/* We don't have it: try to create it */
	size = 0;
	for (x = 0; unicat[x].name; ++x)
		if (unicat[x].name[1] && !unicat[x].name[2] && !zncmp(unicat[x].name, cat, zlen(cat)))
			size += unicat[x].size;
	/* No match */
	if (!size)
		return 0;
	der = (struct unicat *)joe_malloc(sizeof(struct unicat));
	der->next = derived;
	derived = der;
	der->name = zdup(cat);
	der->size = size;
	der->table = (struct interval *)joe_malloc(sizeof(struct interval) * der->size);
	y = 0;
	for (x = 0; unicat[x].name; ++x)
		if (unicat[x].name[1] && !unicat[x].name[2] && !zncmp(unicat[x].name, cat, zlen(cat))) {
			mcpy(der->table + y, unicat[x].table, sizeof(struct interval) * unicat[x].size);
			y += unicat[x].size;
		}
	interval_sort(der->table, der->size);

#if 0
	for (x = 0; x != der->size; ++x)
		printf("\r\n%d %d\r\n", der->table[x].first, der->table[x].last);

	sleep(10);
#endif

#if 0
	l = interval_set(l, der->table, der->size, (void *)1);
	cmap_build(&cmap, l, NULL);
	ti = time(0);
	y = 0;
	for (x = 0; x != 1000000000; ++x) {
		y += (ptrdiff_t)cmap_lookup(&cmap, 0xA0);
	}
	ti = time(0) - ti;
	printf("%d %d\n", ti, y);
#endif

#if 0
	r = mkrtree();
	for (x = 0; x != der->size; ++x)
		for (y = der->table[x].first; y <= der->table[x].last; ++y)
			rtree_add(r, y, 1);
	printf("\r\n\n%s: %d\r\n\n", cat, der->size);
	rtree_show(r);

//	rtree_opt(r);

	ti = time(0);
	y = 0;
	for (x = 0; x != 1000000000; ++x) {
		y += rtree_find_unopt(r, 0xA0);
	}
	ti = time(0) - ti;
	printf("%d %d\n", ti, y);
	

	sleep(15);
#endif

	return der;
}

int unicatcheck(struct unicat *cat, int ch)
{
	ptrdiff_t x;
	if (!cat)
		return 0;
	if (ch < 0)
		ch += 256;
	return interval_test(cat->table, cat->size, ch) != -1;
//	for (x = 0; x != cat->size; ++x)
//		if (ch >= cat->table[x].first && ch <= cat->table[x].last)
//			return 1;
	return 0;
}

/*
int main()
{
	int hi[]={'H','E','L','L','O',0};
	int out[49];
	int x;
	lowerize(out, SIZEOF(out)/SIZEOF(out[0]), hi);
	for (x = 0; x != 10; ++x)
	  printf("%d\n", out[x]);
}
*/

/*
int main(int argc, char *argv[])
{
	char buf[80];
	PHASH *h;
	init_fold_hash_table();
	h = fold_hash_table;
	printf("Select=%x Len=%u Count=%u Size=%u\n", h->select, h->len, h->count, (h->len * sizeof(struct Phash_entry)));
	while (gets(buf)) {
		if (buf[0] == 'f') {
			unsigned key;
			int val;
			sscanf(buf + 2,"%x", &key);
			val = phash_find(h, key);
			if (val != -1)
				printf("%d %x..%x\n", val, fold_table[val].first, fold_table[val].last);
		} else if (buf[0] == 's') {
			unsigned y, x;
			for (y = 0; y != h->len; ++y) {
				for (x = 0; x != PHASH_WAYS; ++x)
					if (h->table[y].vals[x])
						printf("%x.%x: %x\n", y, x, h->table[y].keys[x]);
			}
		}
	}
}
*/

static struct Rset rset_upper[1];

int joe_iswupper(struct charmap *foo, int ch) { return rset_lookup(rset_upper, ch); }

static struct Rset rset_lower[1];

int joe_iswlower(struct charmap *foo, int ch) { return rset_lookup(rset_lower, ch); }

static struct Rset rset_alpha[1];

int joe_iswalpha(struct charmap *foo, int ch) { return rset_lookup(rset_alpha, ch); }

static struct Rset rset_alpha_[1];

int joe_iswalpha_(struct charmap *foo, int ch) { return rset_lookup(rset_alpha_, ch); }

static struct Rset rset_alnum[1];

int joe_iswalnum(struct charmap *foo, int ch) { return rset_lookup(rset_alnum, ch); }

static struct Rset rset_alnum_[1];

int joe_iswalnum_(struct charmap *foo, int ch) { return rset_lookup(rset_alnum_, ch); }

static struct Rset rset_digit[1];

int joe_iswdigit(struct charmap *foo, int ch) { return rset_lookup(rset_digit, ch); }

static struct Rset rset_xdigit[1];

int joe_iswxdigit(struct charmap *foo, int ch) { return rset_lookup(rset_xdigit, ch); }

static struct Rset rset_punct[1];

int joe_iswpunct(struct charmap *foo, int ch) { return rset_lookup(rset_punct, ch); }

static struct Rset rset_space[1];

int joe_iswspace(struct charmap *foo, int ch) { return rset_lookup(rset_space, ch); }

static struct Rset rset_blank[1];

int joe_iswblank(struct charmap *foo, int ch) { return rset_lookup(rset_blank, ch); }

static struct Rset rset_ctrl[1];

int joe_iswctrl(struct charmap *foo, int ch) { return rset_lookup(rset_ctrl, ch); }

static struct Rset rset_graph[1];

int joe_iswgraph(struct charmap *foo, int ch) { return rset_lookup(rset_graph, ch); }

static struct Rset rset_print[1];

int joe_iswprint(struct charmap *foo, int ch) { return rset_lookup(rset_print, ch); }

void joe_iswinit()
{
	struct unicat *cat;
	struct interval x[1];

	rset_init(rset_upper);
	if ((cat = unicatlookup("Lu"))) rset_set(rset_upper, cat->table, cat->size);
	rset_opt(rset_upper);
	/* printf("\nUpper\n"); rset_show(rset_upper); */

	rset_init(rset_lower);
	if ((cat = unicatlookup("Ll"))) rset_set(rset_lower, cat->table, cat->size);
	rset_opt(rset_lower);
	/* printf("\nLower\n"); rset_show(rset_lower); */

	rset_init(rset_alpha);
	if ((cat = unicatlookup("L"))) rset_set(rset_alpha, cat->table, cat->size);
	if ((cat = unicatlookup("M"))) rset_set(rset_alpha, cat->table, cat->size);
	rset_opt(rset_alpha);
	/* printf("\nAlpha\n"); rset_show(rset_alpha); */

	rset_init(rset_alpha_);
	if ((cat = unicatlookup("L"))) rset_set(rset_alpha_, cat->table, cat->size);
	if ((cat = unicatlookup("M"))) rset_set(rset_alpha_, cat->table, cat->size);
	if ((cat = unicatlookup("Pc"))) rset_set(rset_alpha_, cat->table, cat->size);
	rset_opt(rset_alpha_);
	/* printf("\nAlpha_\n"); rset_show(rset_alpha_); */

	rset_init(rset_alnum);
	if ((cat = unicatlookup("L"))) rset_set(rset_alnum, cat->table, cat->size);
	if ((cat = unicatlookup("M"))) rset_set(rset_alnum, cat->table, cat->size);
	if ((cat = unicatlookup("N"))) rset_set(rset_alnum, cat->table, cat->size);
	rset_opt(rset_alnum);
	/* printf("\nAlnum\n"); rset_show(rset_alnum); */

	rset_init(rset_alnum_);
	if ((cat = unicatlookup("L"))) rset_set(rset_alnum_, cat->table, cat->size);
	if ((cat = unicatlookup("M"))) rset_set(rset_alnum_, cat->table, cat->size);
	if ((cat = unicatlookup("N"))) rset_set(rset_alnum_, cat->table, cat->size);
	if ((cat = unicatlookup("Pc"))) rset_set(rset_alnum_, cat->table, cat->size);
	rset_opt(rset_alnum_);
	/* printf("\nAlnum_\n"); rset_show(rset_alnum_); */

	rset_init(rset_digit);
	if ((cat = unicatlookup("Nd"))) rset_set(rset_digit, cat->table, cat->size);
	rset_opt(rset_digit);
	/* printf("\nDigit\n"); rset_show(rset_digit); */

	rset_init(rset_xdigit);
	if ((cat = unicatlookup("Nd"))) rset_set(rset_xdigit, cat->table, cat->size);
	x->first = 'A'; x->last = 'F'; rset_set(rset_xdigit, x, 1);
	x->first = 'a'; x->last = 'f'; rset_set(rset_xdigit, x, 1);
	rset_opt(rset_xdigit);
	/* printf("\nXdigit\n"); rset_show(rset_xdigit); */

	rset_init(rset_punct);
	if ((cat = unicatlookup("P"))) rset_set(rset_punct, cat->table, cat->size);
	rset_opt(rset_punct);
	/* printf("\nPunct\n"); rset_show(rset_punct); */

	rset_init(rset_space);
	rset_add(rset_space, ' ');
	rset_add(rset_space, '\t');
	rset_add(rset_space, '\r');
	rset_add(rset_space, '\n');
	rset_add(rset_space, '\f');
	if ((cat = unicatlookup("Z"))) rset_set(rset_space, cat->table, cat->size);
	rset_opt(rset_space);
	/* printf("\nSpace\n"); rset_show(rset_space); */

	rset_init(rset_blank);
	rset_add(rset_blank, ' ');
	rset_add(rset_blank, '\t');
	if ((cat = unicatlookup("Zs"))) rset_set(rset_blank, cat->table, cat->size);
	rset_opt(rset_blank);
	/* printf("\nBlank\n"); rset_show(rset_blank); */

	rset_init(rset_ctrl);
	if ((cat = unicatlookup("C"))) rset_set(rset_ctrl, cat->table, cat->size);
	if ((cat = unicatlookup("Zl"))) rset_set(rset_ctrl, cat->table, cat->size);
	if ((cat = unicatlookup("Zp"))) rset_set(rset_ctrl, cat->table, cat->size);
	rset_opt(rset_ctrl);
	/* printf("\nCtrl\n"); rset_show(rset_ctrl); */

	rset_init(rset_graph);
	if ((cat = unicatlookup("L"))) rset_set(rset_graph, cat->table, cat->size);
	if ((cat = unicatlookup("M"))) rset_set(rset_graph, cat->table, cat->size);
	if ((cat = unicatlookup("S"))) rset_set(rset_graph, cat->table, cat->size);
	if ((cat = unicatlookup("N"))) rset_set(rset_graph, cat->table, cat->size);
	if ((cat = unicatlookup("P"))) rset_set(rset_graph, cat->table, cat->size);
	rset_opt(rset_graph);
	/* printf("\nGraph\n"); rset_show(rset_graph); */

	rset_init(rset_print);
	if ((cat = unicatlookup("L"))) rset_set(rset_print, cat->table, cat->size);
	if ((cat = unicatlookup("M"))) rset_set(rset_print, cat->table, cat->size);
	if ((cat = unicatlookup("S"))) rset_set(rset_print, cat->table, cat->size);
	if ((cat = unicatlookup("N"))) rset_set(rset_print, cat->table, cat->size);
	if ((cat = unicatlookup("P"))) rset_set(rset_print, cat->table, cat->size);
	if ((cat = unicatlookup("Zs"))) rset_set(rset_print, cat->table, cat->size);
	rset_opt(rset_print);
	/* printf("\nPrint\n"); rset_show(rset_print); */
	/* exit(-1); */
}

/* Digit value of any \p{Nd} digit */

static struct unicat *digtable = 0;

int digval(int ch)
{
	if (!digtable)
		digtable = unicatlookup("Nd");
	if (digtable) {
		int idx = interval_test(digtable->table, digtable->size, ch);
		if (idx != -1) {
			return ch - digtable->table[idx].first;
		}
	}
	return -1;
}
