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
