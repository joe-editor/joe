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

static unsigned gather(unsigned val,unsigned sel)
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
		sel = (sel ^ mv) | (mv >> (1 << i));
		t = val & mv;
		val = (val ^ t) | (t >> (1 << i));
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

#if 0
#define RTREE_LEVELS 4

int rtree_alloc_unopt(struct Rtree_level_unopt *l)
{
	int a = l->alloc++;
	int x;
	if (a == l->len) {
		l->len *= 2;
		l->table = (struct Rtree_entry_unopt *)joe_realloc(l->table, SIZEOF(struct Rtree_entry_unopt) * l->len);
	}
	for (x = 0; x != SIZEOF(l->table[0].data) / SIZEOF(l->table[0].data[0]); ++x)
		l->table[a].data[x] = -1;
	l->table[a].equiv = -1;
	l->table[a].repl = -1;
	return a;
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
	int x, y, z, a;
	for (x = 0; x != RTREE_LEVELS; ++x)
		printf("level %d has %d entries\r\n", x, r->unopt[x].alloc);
/*
	for (x = 0; x != 32; ++x) {
		int i = r->unopt[0].table[0].data[x];
		if (i != -1) {
			for (y = 0; y != 32; ++y) {
				int j = r->unopt[1].table[i].data[y];
				if (j != -1) {
					for (z = 0; z != 32; ++z) {
						int k = r->unopt[2].table[j].data[z];
						if (k != -1) {
							for (a = 0; a != 32; ++a) {
								int l = r->unopt[3].table[k].data[a];
								printf("%x = %d\r\n",(unsigned)((x<<15)+(y<<10)+(z<<5)+a),l);
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
//	rtree_free_unopt(r);
	if (r->table)
		joe_free(r->table);
	joe_free(r);
}

int rtree_find_unopt(struct Rtree *r, int key)
{
	int a = (0x3f & (key >> 14));
	int b = (0x1f & (key >> 9));
	int c = (0x1f & (key >> 4));
	int d = (0xf & (key >> 0));

	int idx = r->unopt[0].table[0].data[a];
	if (idx != -1) {
		idx = r->unopt[1].table[idx].data[b];
		if (idx != -1) {
			idx = r->unopt[2].table[idx].data[c];
			if (idx != -1)
				return r->unopt[3].table[idx].data[d];
		}
	}
	return -1;
}

int rtree_find(struct Rtree *r, int key)
{
	int idx = r->top.data[0x1f & (key >> 15)];
	idx = r->table[idx].data[0x1f & (key >> 10)];
	idx = r->table[idx].data[0x1f & (key >> 5)];
	return r->table[idx].data[0x1f & (key >> 0)];
}

#if 0
int rtree_find(struct Rtree *r, int key)
{
	int a = (0x3f & (key >> 14));
	int b = (0x1f & (key >> 9));
	int c = (0x1f & (key >> 4));
	int d = (0xf & (key >> 0));
	if (a || b) { /* For characters >= 512 */
		int idx = r->top.data[a];
		idx = r->unopt[1].table[idx].data[b];
		idx = r->unopt[2].table[idx].data[c];
		return r->table[idx].data[d];
	} else { /* For characters < 512 */
		int idx = r->mid.data[c];
		return r->table[idx].data[d];
	}
}

int rtree_find(struct Rtree *r, int key)
{
	int l = (0x1ff & (key >> 11));
	int m = (0x7f & (key >> 4));
	int r = (0xf & (key >> 0));
	if (l) { // For characters >= 2048
		int idx = r->top.data[l];
		idx = r->table[idx].data[m];
		return r->table[idx].data[r];
	} else { // For characters < 2048
		int idx = r->med.data[m];
		return r->table[idx].data[r];
	}
}
#endif
void rtree_add(struct Rtree *r, int key, short data)
{
	int idx[RTREE_LEVELS - 1];

	idx[0] = r->unopt[0].table[0].data[0x3f & (key >> 14)];
	if (idx[0] == -1) {
		idx[0] = r->unopt[0].table[0].data[0x3f & (key >> 14)] = rtree_alloc_unopt(&r->unopt[1]);
	}

	idx[1] = r->unopt[1].table[idx[0]].data[0x1f & (key >> 9)];
	if (idx[1] == -1) {
		idx[1] = r->unopt[1].table[idx[0]].data[0x1f & (key >> 9)] = rtree_alloc_unopt(&r->unopt[2]);
	}

	idx[2] = r->unopt[2].table[idx[1]].data[0x1f & (key >> 4)];
	if (idx[2] == -1) {
		idx[2] = r->unopt[2].table[idx[1]].data[0x1f & (key >> 4)] = rtree_alloc_unopt(&r->unopt[3]);
	}
	r->unopt[3].table[idx[2]].data[0xf & (key >> 0)] = data;

#if 0

	r->unopt[3].table[idx[2]].data[0x7 & (key >> 0)] = data;
	idx[0] = r->unopt[0].table[0].data[0x1ff & (key >> 11)];
	if (idx[0] == -1) {
		idx[0] = r->unopt[0].table[0].data[0x1ff & (key >> 11)] = rtree_alloc_unopt(&r->unopt[1]);
	}

	idx[1] = r->unopt[1].table[idx[0]].data[0x7f & (key >> 4)];
	if (idx[1] == -1) {
		idx[1] = r->unopt[1].table[idx[0]].data[0x7f & (key >> 4)] = rtree_alloc_unopt(&r->unopt[2]);
	}
	r->unopt[2].table[idx[1]].data[0xf & (key >> 0)] = data;
#endif
}

void rtree_opt(struct Rtree *r)
{
	int x;
	for (x = RTREE_LEVELS - 1; x >= 0; --x) {
		int y, z;
		int cut = 0;
		if (x != RTREE_LEVELS - 1) {
			for (y = 0; y < r->unopt[x].alloc; ++y)
				for (z = 0; z != 512; ++z) {
					int idx = r->unopt[x].table[y].data[x];
					int equiv = -1;
					if (idx != -1)
						equiv = r->unopt[x + 1].table[idx].equiv;
					if (equiv != -1)
						r->unopt[x].table[y].data[x] = equiv;
				}
		}
		for (y = 0; y < r->unopt[x].alloc - 1; ++y) {
			int z;
			for (z = y + 1; z != r->unopt[x].alloc; ++z) {
				if (r->unopt[x].table[y].equiv == -1)
					if (!memcmp(r->unopt[x].table[y].data, r->unopt[x].table[z].data, 512 * SIZEOF(int))) {
						r->unopt[x].table[y].equiv = y;
						++cut;
					}
			}
		}
		printf("Removed %d from level %d\n", cut, x);
	}
}
#endif
