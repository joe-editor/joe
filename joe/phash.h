/* Perfect hash mapping tables */

#define PHASH_WAYS 6

struct Phash_entry {
	unsigned keys[PHASH_WAYS];
	int vals[PHASH_WAYS];
};

struct Phash {
	unsigned select;	/* Bits to select from input */
	unsigned len;	/* Size of hash table */
	unsigned count;	/* Number of items in table */
	struct Phash_entry *table;	/* Hash table */
};

struct Phash *mkphash(unsigned sel, unsigned len);

void rmphash(struct Phash *h);

int phash_find(struct Phash *h, unsigned key);

void phash_add(struct Phash *h, unsigned key, int val);

/* Perfect hash set tables */

struct Pset_entry {
	unsigned keys[PHASH_WAYS];
};

struct Pset {
	unsigned select;
	unsigned len;
	unsigned count;
	struct Pset_entry *table;
};

struct Pset *mkpset(unsigned sel, unsigned len);

void rmpset(struct Pset *h);

int pset_find(struct Pset *h, unsigned key);

void pset_add(struct Pset *h, unsigned key);

/* Radix tree */

struct Rtree_entry_unopt {
	int data[32];
	int equiv; /* Which one this is equivalent to */
	int repl; /* Which one we are replacing it with */
};

struct Rtree_level_unopt {
	int alloc;
	int len;
	struct Rtree_entry_unopt *table;
};

struct Rtree_entry {
	short data[32];
};

struct Rtree {
	/* Unoptimized */
	struct Rtree_level_unopt *unopt;
	/* Optimized */
	struct Rtree_entry top;
	struct Rtree_entry *table;
};

struct Rtree *mkrtree();

void rtree_show(struct Rtree *r);

void rmrtree(struct Rtree *r);

int rtree_find(struct Rtree *r, int key);

void rtree_add(struct Rtree *r, int key, short data);

void rtree_opt(struct Rtree *r);
