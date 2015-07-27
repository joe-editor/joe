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
