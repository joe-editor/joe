/*
 *	Simple hash table
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

struct Zentry {
	ZHENTRY *next;
	const int *name;
	ptrdiff_t hash_val;
	void *val;
};

struct Zhash {
	ptrdiff_t len;
	ZHENTRY **tab;
	ptrdiff_t nentries;
};

/* Compute hash code for a string */
ptrdiff_t zhash(const int *s);

/* Create a hash table of specified size, which must be a power of 2 */
ZHASH *Zhtmk(ptrdiff_t len);

/* Delete a hash table.  HENTRIES get freed, but name/vals don't. */
void Zhtrm(ZHASH *ht);

/* Add an entry to a hash table.
  Note: 'name' is _not_ strdup()ed */
void *Zhtadd(ZHASH *ht, const int *name, void *val);

/* Look up an entry in a hash table, returns NULL if not found */
void *Zhtfind(ZHASH *ht, const int *name);
