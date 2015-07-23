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
