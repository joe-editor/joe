/*
 *	Perfect hash tables which map an unsigned int key to an int.
 *
 *	Copyright
 *		(C) 2015 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#include "types.h"

/* Select bits in val which are set in sel and compress them all to the right */

unsigned gather(unsigned val,unsigned sel)
{
	unsigned long mk, mp, mv, t;
	int i;
	val &= sel;
	mk = ~sel << 1;
	for (i = 0; i != 5; ++i) {
		mp = mk ^ (mk << 1);
		mp = mp ^ (mp << 2);
		mp = mp ^ (mp << 4);
		mp = mp ^ (mp << 8);
		mp = mp ^ (mp << 16);
		mv = mp & sel;
		sel = sel ^ mv | (mv >> (1 << i));
		t = val & mv;
		val = val ^ t | (t >> (1 << i));
		mk = mk & ~mp;
	}
	return val;
}

/* sel is initial selector: it's OK to use 0
   len is initial length: it's OK to use 1
 */
struct Phash *mkphash(unsigned sel, unsigned len)
{
	struct Phash *h = (struct Phash *)calloc(1, sizeof(struct Phash));
	h->select = sel;
	h->len = len;
	h->table = (struct Phash_entry *)calloc(h->len, sizeof(struct Phash_entry));
	return h;
}

void rmphash(struct Phash *h)
{
	joe_free(h->table);
	joe_free(h);
}

int phash_find(struct Phash *h, unsigned key)
{
	unsigned idx = gather(key, h->select);
	struct Phash_entry *e = &h->table[idx];
	unsigned x;
	for (x = 0; x != PHASH_WAYS && e->keys[x]; ++x)
		if (e->keys[x] == key)
			return e->vals[x];
	return -1;
}

void phash_add(struct Phash *h, unsigned key, int val)
{
	unsigned idx = gather(key, h->select);
	unsigned newbit;
	struct Phash_entry *table = h->table;
	struct Phash_entry *e = &table[idx];
	unsigned len = h->len;
	unsigned x, y, z;

	/* Maybe we already have it? */
	for (x = 0; x != PHASH_WAYS && e->keys[x]; ++x)
		if (e->keys[x] == key) {
			e->vals[x] = val;
			return;
		}

	/* We don't have it, but it fits in current table */
	if (x != PHASH_WAYS) {
		e->keys[x] = key;
		e->vals[x] = val;
		++h->count;
		return;
	}

	/* No space in entry: we need to expand */

	/* Find a new select bit */
	newbit = (e->keys[0] ^ key) & ~h->select; /* All bits which are different, but not already selected */
	newbit = newbit & -newbit; /* Pick least significant one */

	/* Add it to the set */
	h->select |= newbit;

	/* Double the table size */
	h->len *= 2;
	h->table = (struct Phash_entry *)calloc(h->len, sizeof(struct Phash_entry));

	/* Copy entries into new table */
	h->count = 0;
	for (z = 0; z != len; ++z)
		for (y = 0; y != PHASH_WAYS && table[z].keys[y]; ++y)
			phash_add(h, table[z].keys[y], table[z].vals[y]);

	/* Add new entry */
	phash_add(h, key, val);

	/* Delete old table */
	joe_free(table);
}

/* sel is initial selector: it's OK to use 0
   len is initial length: it's OK to use 1
 */
struct Pset *mkpset(unsigned sel, unsigned len)
{
	struct Pset *h = (struct Pset *)calloc(1, sizeof(struct Pset));
	h->select = sel;
	h->len = len;
	h->table = (struct Pset_entry *)calloc(h->len, sizeof(struct Pset_entry));
	return h;
}

void rmpset(struct Pset *h)
{
	joe_free(h->table);
	joe_free(h);
}

int pset_find(struct Pset *h, unsigned key)
{
	unsigned idx = gather(key, h->select);
	struct Pset_entry *e = &h->table[idx];
	unsigned x;
	for (x = 0; x != PHASH_WAYS && e->keys[x]; ++x)
		if (e->keys[x] == key)
			return 1;
	return 0;
}

void pset_add(struct Pset *h, unsigned key)
{
	unsigned idx = gather(key, h->select);
	unsigned newbit;
	struct Pset_entry *table = h->table;
	struct Pset_entry *e = &table[idx];
	unsigned len = h->len;
	unsigned x, y, z;

	/* Maybe we already have it? */
	for (x = 0; x != PHASH_WAYS && e->keys[x]; ++x)
		if (e->keys[x] == key) {
			return;
		}

	/* We don't have it, but it fits in current table */
	if (x != PHASH_WAYS) {
		e->keys[x] = key;
		++h->count;
		return;
	}

	/* No space in entry: we need to expand */

	/* Find a new select bit */
	newbit = (e->keys[0] ^ key) & ~h->select; /* All bits which are different, but not already selected */
	newbit = newbit & -newbit; /* Pick least significant one */

	/* Add it to the set */
	h->select |= newbit;

	/* Double the table size */
	h->len *= 2;
	h->table = (struct Pset_entry *)calloc(h->len, sizeof(struct Pset_entry));

	/* Copy entries into new table */
	h->count = 0;
	for (z = 0; z != len; ++z)
		for (y = 0; y != PHASH_WAYS && table[z].keys[y]; ++y)
			pset_add(h, table[z].keys[y]);

	/* Add new entry */
	pset_add(h, key);

	/* Delete old table */
	joe_free(table);
}

#define RTREE_LEVELS 4

int rtree_alloc_unopt(struct Rtree_level_unopt *l)
{
	int a = l->alloc++;
	int x;
	if (a == l->len) {
		l->len *= 2;
		l->table = (struct Rtree_entry_unopt *)joe_realloc(l->table, SIZEOF(struct Rtree_entry_unopt) * l->len);
	}
	for (x = 0; x != 32; ++x)
		l->table[a].data[x] = -1;
	l->table[a].equiv = -1;
	l->table[a].repl = -1;
	return a;
}

void rtree_free_unopt(struct Rtree *r)
{
	ptrdiff_t x;
	if (r->unopt) {
		for (x = 0; x != RTREE_LEVELS; ++x) {
			if (r->unopt[x].table)
				joe_free(r->unopt[x].table);
		}
		joe_free(r->unopt);
		r->unopt = 0;
	}
}

struct Rtree *mkrtree()
{
	ptrdiff_t x;
	struct Rtree *r = (struct Rtree *)joe_malloc(SIZEOF(struct Rtree));
	r->unopt = (struct Rtree_level_unopt *)joe_malloc(SIZEOF(struct Rtree_level_unopt) * RTREE_LEVELS);
	for (x = 0; x != RTREE_LEVELS; ++x) {
		r->unopt[x].alloc = 0;
		r->unopt[x].len = 1;
		r->unopt[x].table = (struct Rtree_entry_unopt *)joe_malloc(SIZEOF(struct Rtree_entry_unopt) * r->unopt[x].len);
	}
	rtree_alloc_unopt(&r->unopt[0]);
	r->table = 0;
	return r;
}

void rtree_show(struct Rtree *r)
{
	int x;
	for (x = 0; x != RTREE_LEVELS; ++x)
		printf("level %d has %d entries\r\n", x, r->unopt[x].alloc);
/*
	for (x = 0; x != 32; ++x) {
		int i = r->top.data[x];
		if (i != -1) {
			for (y = 0; y != 32; ++y) {
				int j = r->table[i].data[y];
				if (j != -1) {
					for (z = 0; z != 32; ++z) {
						int k = r->table[j].data[z];
						if (k != -1) {
							for (a = 0; a != 32; ++a) {
								int l = r->table[k].data[a];
								printf("%d.%d.%d.%d = %d\r\n",(int)x,(int)y,(int)z,(int)a,l);
							}
						}
					}
				}
			}
		}
	}
*/
}

void rmrtree(struct Rtree *r)
{
	rtree_free_unopt(r);
	if (r->table)
		joe_free(r->table);
	joe_free(r);
}

int rtree_find_unopt(struct Rtree *r, int key)
{
	int idx = r->unopt[0].table[0].data[0x1f & (key >> 15)];
	idx = r->unopt[1].table[idx].data[0x1f & (key >> 10)];
	idx = r->unopt[2].table[idx].data[0x1f & (key >> 5)];
	return r->unopt[3].table[idx].data[0x1f & (key >> 0)];
}

int rtree_find(struct Rtree *r, int key)
{
	int idx = r->top.data[0x1f & (key >> 15)];
	idx = r->table[idx].data[0x1f & (key >> 10)];
	idx = r->table[idx].data[0x1f & (key >> 5)];
	return r->table[idx].data[0x1f & (key >> 0)];
}

void rtree_add(struct Rtree *r, int key, short data)
{
	int idx[RTREE_LEVELS - 1];

	idx[0] = r->unopt[0].table[0].data[0x1f & (key >> 15)];
	if (idx[0] == -1) {
		idx[0] = r->unopt[0].table[0].data[0x1f & (key >> 15)] = rtree_alloc_unopt(&r->unopt[1]);
	}

	idx[1] = r->unopt[1].table[idx[0]].data[0x1f & (key >> 10)];
	if (idx[1] == -1) {
		idx[1] = r->unopt[1].table[idx[0]].data[0x1f & (key >> 10)] = rtree_alloc_unopt(&r->unopt[2]);
	}

	idx[2] = r->unopt[2].table[idx[1]].data[0x1f & (key >> 5)];
	if (idx[2] == -1) {
		idx[2] = r->unopt[2].table[idx[1]].data[0x1f & (key >> 5)] = rtree_alloc_unopt(&r->unopt[3]);
	}

	r->unopt[3].table[idx[2]].data[0x1f & (key >> 0)] = data;
}

void rtree_opt(struct Rtree *r)
{
	int x;
	for (x = RTREE_LEVELS - 1; x >= 0; --x) {
		int y;
		for (y = 0; y < r->unopt[x].alloc - 1; ++y) {
			int z;
			for (z = y + 1; z != r->unopt[x].alloc; ++z) {
				if (r->unopt[x].table[y].equiv == -1)
					if (!memcmp(r->unopt[x].table[y].data, r->unopt[x].table[z].data, 32 * SIZEOF(int)))
						r->unopt[x].table[y].equiv = y;
			}
		}
	}
}
