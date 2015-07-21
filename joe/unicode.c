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
