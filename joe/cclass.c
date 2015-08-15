/*
 *	Characters classes
 *	Copyright
 *		(C) 2015 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#include "types.h"

/* Sort an array of struct intervals */

static int itest(const void *ia, const void *ib)
{
	struct interval *a = (struct interval *)ia;
	struct interval *b = (struct interval *)ib;
	if (a->first > b->first)
		return 1;
	else if (a->first < b->first)
		return -1;
	else
		return 0;
}

void interval_sort(struct interval *array, ptrdiff_t num)
{
	jsort(array, num, SIZEOF(struct interval), itest);
}

/* Test if character ch is in one of the struct intervals in array using binary search.
 * Returns index to interval, or -1 if none found. */

ptrdiff_t interval_test(struct interval *array, ptrdiff_t size, int ch)
{
	if (size) {
		ptrdiff_t min = 0;
		ptrdiff_t mid;
		ptrdiff_t max = size - 1;
		if (ch < array[min].first || ch > array[max].last)
			goto no_match;
		while (max >= min) {
			mid = (min + max) / 2;
			if (ch > array[mid].last)
				min = mid + 1;
			else if (ch < array[mid].first)
				max = mid - 1;
			else
				return mid;
		}
	}
	no_match:
	return -1;
}

struct interval_list *mkinterval(struct interval_list *next, int first, int last, void *map)
{
	struct interval_list *interval_list = (struct interval_list *)joe_malloc(SIZEOF(struct interval_list));
	interval_list->next = next;
	interval_list->interval.first = first;
	interval_list->interval.last = last;
	interval_list->map = map;
	return interval_list;
}

void rminterval(struct interval_list *interval_list)
{
	while (interval_list) {
		struct interval_list *n = interval_list->next;
		free(interval_list);
		interval_list = n;
	}
}

/* Add a single character range to an interval list */

struct interval_list *interval_add(struct interval_list *interval_list, int first, int last, void *map)
{
	struct interval_list *e, **p;
	for (p = &interval_list; *p;) {
		e = *p;
		if (first > e->interval.last + 1 || (first > e->interval.last && (map != e->map))) {
			/* e is below new range, skip it */
			p = &e->next;
		} else if (e->interval.first > last + 1 || (e->interval.first > last && (map != e->map))) {
			/* e is fully above new range, so insert new one here */
			break;
		} else if (e->map == map) {
			/* merge e into new */
			if (e->interval.first <= first) {
				if (e->interval.last >= last) { /* && e->interval.first <= first */
					/* Existing covers new: we're done */
					return interval_list;
				} else { /* e->interval.last < last && e->interval.first <= first */
					/* Enlarge new, delete existing */
					first = e->interval.first;
					*p = e->next;
					e->next = 0;
					rminterval(e);
				}
			} else { /* e->interval.first > first */
				if (e->interval.last <= last) { /* && e->interval.first > first */
					/* New fully covers existing, delete existing */
					*p = e->next;
					e->next = 0;
					rminterval(e);
				} else { /* e->interval.last > last && e->interval.first > first */
					/* Extend existing */
					e->interval.first = first;
					return interval_list;
				}
			}
		} else {
			/* replace e with new */
			if (e->interval.first < first) {
				if (e->interval.last <= last) { /* && e->interval.first < first */
					/* Top part of existing get cut-off by new */
					e->interval.last = first - 1;
				} else { /* e->interval.last > last && e->interval.first < first */
					int org = e->interval.last;
					void *orgmap = e->map;
					e->interval.last = first - 1;
					p = &e->next;
					*p = mkinterval(*p, first, last, map);
					p = &(*p)->next;
					*p = mkinterval(*p, last + 1, org, orgmap);
					return interval_list;
				}
			} else { /* e->interval.first >= first */
				if (e->interval.last <= last) { /* && e->interval.first >= first */
					/* Delete existing */
					*p = e->next;
					e->next = 0;
					rminterval(e);
				} else { /* e->interval.last > last && e->interval.first >= first */
					e->interval.first = last + 1;
					break;
				}
			}
		}
	}
	*p = mkinterval(*p, first, last, map);
	return interval_list;
}

/* Add a list of intervals (typically representing a character class) to an interval list */

struct interval_list *interval_set(struct interval_list *interval_list, struct interval *list, int size, void *map)
{
	int x;
	for (x = 0; x != size; ++x)
		interval_list = interval_add(interval_list, list[x].first, list[x].last, map);
	return interval_list;
}

void *interval_lookup(struct interval_list *list, void *dflt, int item)
{
	while (list) {
		if (item >= list->interval.first && item <= list->interval.last)
			return list->map;
		list = list->next;
	}
	return dflt;
}

void interval_show(struct interval_list *list)
{
	printf("Interval list at %p\n", list);
	while (list) {
		printf("%p show %x..%x -> %x\n", list, list->interval.first, list->interval.last, list->map);
		list = list -> next;
	}
}

/* Return true if character ch is in radix tree r */

int rset_lookup(struct Rset *r, int ch)
{
	int a, b, c, d;

	if (ch < 0)
		ch += 256;

	a = (ch >> TOPSHIFT);
	b = (SECONDMASK & (ch >> SECONDSHIFT));
	c = (THIRDMASK & (ch >> THIRDSHIFT));
	d = (LEAFMASK & (ch >> LEAFSHIFT));

	if (a < 0 || a >= TOPSIZE)
		return 0;

	if (a || b) { /* Full lookup for character >= 512 */
		int idx = r->top.entry[a];
		if (idx != -1) {
			idx = r->second.table.b[idx].entry[b];
			if (idx != -1) {
				idx = r->third.table.c[idx].entry[c];
				return ((1 << d) & idx) != 0;
			}
		}
	} else { /* Quick lookup for character < 512 */
		int idx = r->mid.entry[c];
		return ((1 << d) & idx) != 0;
	}
	return 0;
}

/* Same as above, but can be be called before rset_opt() */

int rset_lookup_unopt(struct Rset *r, int ch)
{
	int a = (ch >> TOPSHIFT);
	int b = (SECONDMASK & (ch >> SECONDSHIFT));
	int c = (THIRDMASK & (ch >> THIRDSHIFT));
	int d = (LEAFMASK & (ch >> LEAFSHIFT));
	int idx;
	if (a >= TOPSIZE)
		return 0;
	idx = r->top.entry[a];
	if (idx != -1) {
		idx = r->second.table.b[idx].entry[b];
		if (idx != -1) {
			idx = r->third.table.c[idx].entry[c];
			return ((1 << d) & idx) != 0;
		}
	}
	return 0;
}

void rset_init(struct Rset *r)
{
	int x;
	for (x = 0; x != TOPSIZE; ++x)
		r->top.entry[x] = -1;

	r->second.alloc = 0;
	r->second.size = 1;
	r->second.table.b = (struct Mid *)joe_malloc(r->second.size * SIZEOF(struct Mid));

	r->third.alloc = 0;
	r->third.size = 1;
	r->third.table.c = (struct Mid *)joe_malloc(r->third.size * SIZEOF(struct Mid));
}

void rset_clr(struct Rset *r)
{
	joe_free(r->second.table.b);
	joe_free(r->third.table.c);
}

static int rset_alloc(struct Level *l, int levelno)
{
	int x;
	if (l->alloc == l->size) {
		l->size *= 2;
		switch (levelno) {
			case 1: {
				l->table.b = (struct Mid *)joe_realloc(l->table.b, l->size * SIZEOF(struct Mid));
				break;
			} case 2: {
				l->table.c = (struct Mid *)joe_realloc(l->table.c, l->size * SIZEOF(struct Mid));
				break;
			}
		}
	}
	switch (levelno) {
		case 1: {
			for (x = 0; x != SECONDSIZE; ++x)
				l->table.b[l->alloc].entry[x] = -1;
			break;
		} case 2: {
			for (x = 0; x != THIRDSIZE; ++x)
				l->table.c[l->alloc].entry[x] = 0;
			break;
		}
	}
	if (l->alloc == 32768) {
		fprintf(stderr,"rset_alloc overflow\r\n");
		exit(-1);
	}
	return l->alloc++;
}

void rset_add(struct Rset *r, int ch, int che)
{
	int a = (TOPMASK & (ch >> TOPSHIFT));
	int b = (SECONDMASK & (ch >> SECONDSHIFT));
	int c = (THIRDMASK & (ch >> THIRDSHIFT));
	int d = (LEAFMASK & (ch >> LEAFSHIFT));

	int ib;
	int ic;

	while (ch <= che) {

		if (a >= TOPSIZE)
			return;

		ib = r->top.entry[a];
		if (ib == -1) {
			r->top.entry[a] = ib = rset_alloc(&r->second, 1);
		}

		while (ch <= che) {
			ic = r->second.table.b[ib].entry[b];
			if (ic == -1) {
				r->second.table.b[ib].entry[b] = ic = rset_alloc(&r->third, 2);
			}
			while (ch <= che) {
				r->third.table.c[ic].entry[c] |= (1 << d);
				++ch;
				if (++d == LEAFSIZE) {
					d = 0;
					if (++c == THIRDSIZE) {
						c = 0;
						break;
					}
				}
			}
			if (++b == SECONDSIZE) {
				b = 0;
				break;
			}
		}
		++a;
	}
}

/* Optimize radix tree: setup mid */

void rset_opt(struct Rset *r)
{
	int x;
	int idx;
	/* Set up mid table */
	for (x = 0; x != THIRDSIZE; ++x)
		r->mid.entry[x] = 0;
	idx = r->top.entry[0];
	if (idx != -1) {
		idx = r->second.table.b[idx].entry[0];
		if (idx != -1) {
			mcpy(r->mid.entry, r->third.table.c[idx].entry, SIZEOF(r->mid.entry));
		}
	}
}

void rset_set(struct Rset *r, struct interval *array, ptrdiff_t size)
{
	ptrdiff_t y;
	for (y = 0; y != size; ++y) {
		rset_add(r, array[y].first, array[y].last);
	}
}

void rset_show(struct Rset *r)
{
	int first = -2;
	int last = -2;
	int a;
	int len = 0;
	int total = 0;
	printf("Rset at %p\n", r);

	len = SIZEOF(struct Rset);
	printf("Top level size = %d\n", len);
	total += len;

	len = r->second.alloc * SIZEOF(struct Mid);
	printf("Second level size = %d (%d entries)\n", len, r->second.alloc);
	total += len;

	len = r->third.alloc * SIZEOF(struct Mid);
	printf("Third level size = %d (%d entries)\n", len, r->third.alloc);
	total += len;

	printf("Total size = %d bytes\n", total);

	for (a = 0; a != TOPSIZE; ++a) {
		int ib = r->top.entry[a];
		if (ib != -1) {
			int b;
			for (b = 0; b != SECONDSIZE; ++b) {
				int ic = r->second.table.b[ib].entry[b];
				if (ic != -1) {
					int c;
					for (c = 0; c != THIRDSIZE; ++c) {
						int id = r->third.table.c[ic].entry[c];
						int d;
						for (d = 0; d != LEAFSIZE; ++d) {
							if (id & (1 << d)) {
								int ch = (a << TOPSHIFT) + (b << SECONDSHIFT) + (c << THIRDSHIFT) + d;
								if (ch == last + 1) {
									last = ch;
								} else if (first != -2) {
									printf("	{ 0x%4.4X, 0x%4.4X },\n", first, last);
									first = last = ch;
								} else {
									first = last = ch;
								}
							}
						}
					}
				}
			}
		}
	}
	if (first != -2)
		printf("	{ 0x%4.4X, 0x%4.4X },\n", first, last);
}

/* Radix tree maps */

void *rtree_lookup(struct Rtree *r, int ch)
{
	int a = (ch >> TOPSHIFT);
	int b = (SECONDMASK & (ch >> SECONDSHIFT));
	int c = (THIRDMASK & (ch >> THIRDSHIFT));
	int d = (LEAFMASK & (ch >> LEAFSHIFT));
	if (a >= TOPSIZE)
		return NULL;
	if (a || b) { /* Full lookup for character >= 512 */
		int idx = r->top.entry[a];
		if (idx != -1) {
			idx = r->second.table.b[idx].entry[b];
			if (idx != -1) {
				idx = r->third.table.c[idx].entry[c];
				if (idx != -1)
					return r->leaf.table.d[idx].entry[d];
			}
		}
	} else { /* Quick lookup for character < 512 */
		int idx = r->mid.entry[c];
		if (idx != -1)
			return r->leaf.table.d[idx].entry[d];
	}
	return NULL;
}

void *rtree_lookup_unopt(struct Rtree *r, int ch)
{
	int a = (ch >> TOPSHIFT);
	int b = (SECONDMASK & (ch >> SECONDSHIFT));
	int c = (THIRDMASK & (ch >> THIRDSHIFT));
	int d = (LEAFMASK & (ch >> LEAFSHIFT));
	int idx;
	if (a >= TOPSIZE)
		return NULL;
	idx = r->top.entry[a];
	if (idx != -1) {
		idx = r->second.table.b[idx].entry[b];
		if (idx != -1) {
			idx = r->third.table.c[idx].entry[c];
			if (idx != -1)
				return r->leaf.table.d[idx].entry[d];
		}
	}
	return NULL;
}

void rtree_init(struct Rtree *r)
{
	int x;
	for (x = 0; x != TOPSIZE; ++x)
		r->top.entry[x] = -1;

	r->second.alloc = 0;
	r->second.size = 1;
	r->second.table.b = (struct Mid *)joe_malloc(r->second.size * SIZEOF(struct Mid));

	r->third.alloc = 0;
	r->third.size = 1;
	r->third.table.c = (struct Mid *)joe_malloc(r->third.size * SIZEOF(struct Mid));

	r->leaf.alloc = 0;
	r->leaf.size = 1;
	r->leaf.table.d = (struct Leaf *)joe_malloc(r->leaf.size * SIZEOF(struct Leaf));
}

void rtree_clr(struct Rtree *r)
{
	joe_free(r->second.table.b);
	joe_free(r->third.table.c);
	joe_free(r->leaf.table.d);
}

static int rtree_alloc(struct Level *l, int levelno)
{
	int x;
	if (l->alloc == l->size) {
		l->size *= 2;
		switch (levelno) {
			case 1: {
				l->table.b = (struct Mid *)joe_realloc(l->table.b, l->size * SIZEOF(struct Mid));
				break;
			} case 2: {
				l->table.c = (struct Mid *)joe_realloc(l->table.c, l->size * SIZEOF(struct Mid));
				break;
			} case 3: {
				l->table.d = (struct Leaf *)joe_realloc(l->table.d, l->size * SIZEOF(struct Leaf));
				break;
			}
		}
	}
	switch (levelno) {
		case 1: {
			for (x = 0; x != SECONDSIZE; ++x)
				l->table.b[l->alloc].entry[x] = -1;
			break;
		} case 2: {
			for (x = 0; x != THIRDSIZE; ++x)
				l->table.c[l->alloc].entry[x] = -1;
			break;
		} case 3: {
			for (x = 0; x != LEAFSIZE; ++x)
				l->table.d[l->alloc].entry[x] = NULL;
			l->table.d[l->alloc].refcount = 1;
			break;
		}
	}
	if (l->alloc == 32768) {
		fprintf(stderr,"rtree_alloc overflow\r\n");
		exit(-1);
	}
	return l->alloc++;
}

void rtree_add(struct Rtree *r, int ch, int che, void *map)
{
	int a = (TOPMASK & (ch >> TOPSHIFT));
	int b = (SECONDMASK & (ch >> SECONDSHIFT));
	int c = (THIRDMASK & (ch >> THIRDSHIFT));
	int d = (LEAFMASK & (ch >> LEAFSHIFT));

	int ib;
	int ic;
	int id;

	/* printf("%p rtree_add %x..%x [%d %d %d %d] -> %p\n",r, ch, che, a, b, c, d, map); */

	while (ch <= che) {

		if (a >= TOPSIZE)
			return;

		ib = r->top.entry[a];
		if (ib == -1) {
			r->top.entry[a] = ib = rtree_alloc(&r->second, 1);
		}

		while (ch <= che) {
			ic = r->second.table.b[ib].entry[b];
			if (ic == -1) {
				r->second.table.b[ib].entry[b] = ic = rtree_alloc(&r->third, 2);
			}

			while (ch <= che) {
				id = r->third.table.c[ic].entry[c];
				if (id == -1) {
					r->third.table.c[ic].entry[c] = id = rtree_alloc(&r->leaf, 3);
				}

				while (ch <= che) {
					struct Leaf *l = r->leaf.table.d + id;
					
					/* Copy on write */
					if (l->refcount != 1) {
						struct Leaf *org = l;
						r->third.table.c[ic].entry[c] = id = rtree_alloc(&r->leaf, 3);
						l = r->leaf.table.d + id;
						mcpy(l, org, SIZEOF(struct Leaf));
						--org->refcount;
						l->refcount = 1;
					}

					/* Write */
					l->entry[d] = map;

					/* Try to de-duplicate with previous entry */
					if (d == LEAFSIZE - 1 && id && id == r->leaf.alloc - 1 && !memcmp(l->entry, r->leaf.table.d[id - 1].entry, LEAFSIZE * SIZEOF(void *))) {
						--r->leaf.alloc;
						--id;
						l = r->leaf.table.d + id;
						++l->refcount;
						r->third.table.c[ic].entry[c] = id;
					}

					++ch;
					if (++d == LEAFSIZE) {
						d = 0;
						break;
					}
				}
				if (++c == THIRDSIZE) {
					c = 0;
					break;
				}
			}
			if (++b == SECONDSIZE) {
				b = 0;
				break;
			}
		}
		++a;
	}
}

/* Optimize radix tree: de-duplicate leaf nodes and setup mid */

struct rhentry {
	struct rhentry *next;
	struct Leaf *leaf;
	int idx;
};

static ptrdiff_t rhhash(struct Leaf *l)
{
	ptrdiff_t hval = 0;
	int x;
	for (x = 0; x != LEAFSIZE; ++x)
		hval = (hval << 4) + (hval >> 28) + (ptrdiff_t)l->entry[x];
	return hval;
		
}

void rtree_opt(struct Rtree *r)
{
	int idx;
	int x;
	int rhsize;
	struct rhentry **rhtable;
	int *equiv;
	int *repl;
	int dupcount;
	int newalloc;
	struct Leaf *l;

	/* De-duplicate leaf nodes (it's not worth bothering with interior nodes) */

	equiv = joe_malloc(SIZEOF(int) * r->leaf.alloc);
	repl = joe_malloc(SIZEOF(int) * r->leaf.alloc);

	/* Create hash table index of all the leaf nodes */
	dupcount = 0;
	rhsize = 1024;
	rhtable = joe_calloc(SIZEOF(struct rhentry *), rhsize);
	for (x = 0; x != r->leaf.alloc; ++x) {
		struct rhentry *rh;
		idx = ((rhsize - 1) & rhhash(r->leaf.table.d + x));
		/* Already exists? */
		for (rh = rhtable[idx]; rh; rh = rh->next)
			if (!memcmp(rh->leaf->entry, r->leaf.table.d[x].entry, LEAFSIZE * SIZEOF(void *)))
				break;
		if (rh) {
			equiv[x] = rh->idx;
			++dupcount;
		} else {
			equiv[x] = -1;
			rh = (struct rhentry *)joe_malloc(SIZEOF(struct rhentry));
			rh->next = rhtable[idx];
			rh->idx = x;
			rh->leaf = r->leaf.table.d + x;
			rhtable[idx] = rh;
		}
	}
	/* Free hash table */
	for (x = 0; x != 1024; ++x) {
		struct rhentry *rh, *nh;
		for (rh = rhtable[x]; rh; rh = nh) {
			nh = rh->next;
			joe_free(rh);
		}
	}
	joe_free(rhtable);

	/* Create new leaf table */
	newalloc = r->leaf.alloc - dupcount;
	l = joe_malloc(SIZEOF(struct Leaf) * newalloc);

	/* Copy entries */
	idx = 0;
	for (x = 0; x != r->leaf.alloc; ++x)
		if (equiv[x] == -1) {
			mcpy(l + idx, r->leaf.table.d + x, sizeof(struct Leaf));
			l[idx].refcount = 1;
			repl[x] = idx;
			++idx;
		}
	for (x = 0; x != r->leaf.alloc; ++x)
		if (equiv[x] != -1) {
			repl[x] = repl[equiv[x]];
			++l[repl[equiv[x]]].refcount;
		}

	/* Install new table */
	joe_free(r->leaf.table.d);
	r->leaf.table.d = l;
	r->leaf.alloc = newalloc;
	r->leaf.size = newalloc;

	/* Remap third level */
	for (x = 0; x != r->third.alloc; ++x)
		for (idx = 0; idx != THIRDSIZE; ++idx)
			if (r->third.table.c[x].entry[idx] != -1)
				r->third.table.c[x].entry[idx] = repl[r->third.table.c[x].entry[idx]];

	joe_free(equiv);
	joe_free(repl);

	/* Set up mid table */
	for (x = 0; x != THIRDSIZE; ++x)
		r->mid.entry[x] = -1;
	idx = r->top.entry[0];
	if (idx != -1) {
		idx = r->second.table.b[idx].entry[0];
		if (idx != -1) {
			mcpy(r->mid.entry, r->third.table.c[idx].entry, SIZEOF(r->mid.entry));
		}
	}
}

void rtree_set(struct Rtree *r, struct interval *array, ptrdiff_t len, void *map)
{
	ptrdiff_t y;
	for (y = 0; y != len; ++y) {
		rtree_add(r, array[y].first, array[y].last, map);
	}
}

void rtree_build(struct Rtree *r, struct interval_list *l)
{
	while (l) {
		// printf("%p build %x %x %p\n",r, l->interval.first, l->interval.last, l->map);
		rtree_add(r, l->interval.first, l->interval.last, l->map);
		l = l->next;
	}
}

void rtree_show(struct Rtree *r)
{
	int first = -2;
	int last = -2;
	void *val = 0;
	int a;

	int len = 0;
	int total = 0;
	printf("Rtree at %p\n", r);

	len = SIZEOF(struct Rtree);
	printf("Top level size = %d\n", len);
	total += len;

	len = r->second.alloc * SIZEOF(struct Mid);
	printf("Second level size = %d (%d entries)\n", len, r->second.alloc);
	total += len;

	len = r->third.alloc * SIZEOF(struct Mid);
	printf("Third level size = %d (%d entries)\n", len, r->third.alloc);
	total += len;

	len = r->leaf.alloc * SIZEOF(struct Leaf);
	printf("Fourth level size = %d (%d entries)\n", len, r->leaf.alloc);
	total += len;

	printf("Total size = %d bytes\n", total);
	
	for (a = 0; a != TOPSIZE; ++a) {
		int ib = r->top.entry[a];
		if (ib != -1) {
			int b;
			for (b = 0; b != SECONDSIZE; ++b) {
				int ic = r->second.table.b[ib].entry[b];
				if (ic != -1) {
					int c;
					for (c = 0; c != THIRDSIZE; ++c) {
						int id = r->third.table.c[ic].entry[c];
						if (id != -1) {
							int d;
							for (d = 0; d != LEAFSIZE; ++d) {
								void *ie = r->leaf.table.d[id].entry[d];
								int ch = (a << TOPSHIFT) + (b << SECONDSHIFT) + (c << THIRDSHIFT) + d;
								/* if (ie)
									printf("%d %d.%d %d.%d %d: %d=%p\n",a,ib,b,ic,c,id,d,ie); */
								if (ch == last + 1 && ie == val) {
									last = ch;
								} else if (first != -2) {
									printf("%p show %x %x %p\n", r, first, last, val);
									first = last = ch;
									val = ie;
								} else {
									first = last = ch;
									val = ie;
								}
							}
						}
					}
				}
			}
		}
	}
	if (first != -2)
		printf("%p show %x %x %p\n", r, first, last, val);
}

/* Radix tree maps */

int rmap_lookup(struct Rtree *r, int ch, int dflt)
{
	int a = (ch >> TOPSHIFT);
	int b = (SECONDMASK & (ch >> SECONDSHIFT));
	int c = (THIRDMASK & (ch >> THIRDSHIFT));
	int d = (LEAFMASK & (ch >> LEAFSHIFT));
	if (a >= TOPSIZE)
		return dflt;
	if (a || b) { /* Full lookup for character >= 512 */
		int idx = r->top.entry[a];
		if (idx != -1) {
			idx = r->second.table.b[idx].entry[b];
			if (idx != -1) {
				idx = r->third.table.c[idx].entry[c];
				if (idx != -1)
					return r->leaf.table.e[idx].entry[d];
			}
		}
	} else { /* Quick lookup for character < 512 */
		int idx = r->mid.entry[c];
		if (idx != -1)
			return r->leaf.table.e[idx].entry[d];
	}
	return dflt;
}

int rmap_lookup_unopt(struct Rtree *r, int ch, int dflt)
{
	int a = (ch >> TOPSHIFT);
	int b = (SECONDMASK & (ch >> SECONDSHIFT));
	int c = (THIRDMASK & (ch >> THIRDSHIFT));
	int d = (LEAFMASK & (ch >> LEAFSHIFT));
	int idx;
	if (a >= TOPSIZE)
		return dflt;
	idx = r->top.entry[a];
	if (idx != -1) {
		idx = r->second.table.b[idx].entry[b];
		if (idx != -1) {
			idx = r->third.table.c[idx].entry[c];
			if (idx != -1)
				return r->leaf.table.e[idx].entry[d];
		}
	}
	return dflt;
}

void rmap_init(struct Rtree *r)
{
	int x;
	for (x = 0; x != TOPSIZE; ++x)
		r->top.entry[x] = -1;

	r->second.alloc = 0;
	r->second.size = 1;
	r->second.table.b = (struct Mid *)joe_malloc(r->second.size * SIZEOF(struct Mid));

	r->third.alloc = 0;
	r->third.size = 1;
	r->third.table.c = (struct Mid *)joe_malloc(r->third.size * SIZEOF(struct Mid));

	r->leaf.alloc = 0;
	r->leaf.size = 1;
	r->leaf.table.e = (struct Ileaf *)joe_malloc(r->leaf.size * SIZEOF(struct Ileaf));
}

void rmap_clr(struct Rtree *r)
{
	joe_free(r->second.table.b);
	joe_free(r->third.table.c);
	joe_free(r->leaf.table.e);
}

static int rmap_alloc(struct Level *l, int levelno, int dflt)
{
	int x;
	if (l->alloc == l->size) {
		l->size *= 2;
		switch (levelno) {
			case 1: {
				l->table.b = (struct Mid *)joe_realloc(l->table.b, l->size * SIZEOF(struct Mid));
				break;
			} case 2: {
				l->table.c = (struct Mid *)joe_realloc(l->table.c, l->size * SIZEOF(struct Mid));
				break;
			} case 3: {
				l->table.e = (struct Ileaf *)joe_realloc(l->table.e, l->size * SIZEOF(struct Ileaf));
				break;
			}
		}
	}
	switch (levelno) {
		case 1: {
			for (x = 0; x != SECONDSIZE; ++x)
				l->table.b[l->alloc].entry[x] = -1;
			break;
		} case 2: {
			for (x = 0; x != THIRDSIZE; ++x)
				l->table.c[l->alloc].entry[x] = -1;
			break;
		} case 3: {
			for (x = 0; x != LEAFSIZE; ++x)
				l->table.e[l->alloc].entry[x] = dflt;
			l->table.e[l->alloc].refcount = 1;
			break;
		}
	}
	if (l->alloc == 32768) {
		fprintf(stderr,"rmap_alloc overflow\r\n");
		exit(-1);
	}
	return l->alloc++;
}

void rmap_add(struct Rtree *r, int ch, int che, int map, int dflt)
{
	int a = (TOPMASK & (ch >> TOPSHIFT));
	int b = (SECONDMASK & (ch >> SECONDSHIFT));
	int c = (THIRDMASK & (ch >> THIRDSHIFT));
	int d = (LEAFMASK & (ch >> LEAFSHIFT));

	int ib;
	int ic;
	int id;

	while (ch <= che) {

		if (a >= TOPSIZE)
			return;

		ib = r->top.entry[a];
		if (ib == -1) {
			r->top.entry[a] = ib = rmap_alloc(&r->second, 1, dflt);
		}

		while (ch <= che) {
			ic = r->second.table.b[ib].entry[b];
			if (ic == -1) {
				r->second.table.b[ib].entry[b] = ic = rmap_alloc(&r->third, 2, dflt);
			}

			while (ch <= che) {
				id = r->third.table.c[ic].entry[c];
				if (id == -1) {
					r->third.table.c[ic].entry[c] = id = rmap_alloc(&r->leaf, 3, dflt);
				}

				while (ch <= che) {
					struct Ileaf *l = r->leaf.table.e + id;

					/* Copy on write */
					if (l->refcount != 1) {
						struct Ileaf *org = l;
						r->third.table.c[ic].entry[c] = id = rmap_alloc(&r->leaf, 3, dflt);
						l = r->leaf.table.e + id;
						mcpy(l, org, SIZEOF(struct Ileaf));
						--org->refcount;
						l->refcount = 1;
					}

					/* Write */
					l->entry[d] = map;

					/* Try to de-duplicate with previous entry */
					if (d == LEAFSIZE - 1 && id && id == r->leaf.alloc - 1 && !memcmp(l->entry, r->leaf.table.e[id - 1].entry, LEAFSIZE * SIZEOF(int))) {
						--r->leaf.alloc;
						--id;
						l = r->leaf.table.e + id;
						++l->refcount;
						r->third.table.c[ic].entry[c] = id;
					}

					++ch;
					if (++d == LEAFSIZE) {
						d = 0;
						break;
					}
				}
				if (++c == THIRDSIZE) {
					c = 0;
					break;
				}
			}
			if (++b == SECONDSIZE) {
				b = 0;
				break;
			}
		}
		++a;
	}
}

/* Optimize radix tree: de-duplicate leaf nodes and setup mid */

struct rmaphentry {
	struct rmaphentry *next;
	struct Ileaf *leaf;
	int idx;
};

static ptrdiff_t rmaphash(struct Ileaf *l)
{
	ptrdiff_t hval = 0;
	int x;
	for (x = 0; x != LEAFSIZE; ++x)
		hval = (hval << 4) + (hval >> 28) + l->entry[x];
	return hval;
		
}

void rmap_opt(struct Rtree *r)
{
	int idx;
	int x;
	int rhsize;
	struct rmaphentry **rhtable;
	int *equiv;
	int *repl;
	int dupcount;
	int newalloc;
	struct Ileaf *l;

	/* De-duplicate leaf nodes (it's not worth bothering with interior nodes) */

	equiv = joe_malloc(SIZEOF(int) * r->leaf.alloc);
	repl = joe_malloc(SIZEOF(int) * r->leaf.alloc);

	/* Create hash table index of all the leaf nodes */
	dupcount = 0;
	rhsize = 1024;
	rhtable = joe_calloc(SIZEOF(struct rmapentry *), rhsize);
	for (x = 0; x != r->leaf.alloc; ++x) {
		struct rmaphentry *rh;
		idx = ((rhsize - 1) & rmaphash(r->leaf.table.e + x));
		/* Already exists? */
		for (rh = rhtable[idx]; rh; rh = rh->next)
			if (!memcmp(rh->leaf, r->leaf.table.e + x, SIZEOF(struct Ileaf)))
				break;
		if (rh) {
			equiv[x] = rh->idx;
			++dupcount;
		} else {
			equiv[x] = -1;
			rh = (struct rmaphentry *)joe_malloc(SIZEOF(struct rmaphentry));
			rh->next = rhtable[idx];
			rh->idx = x;
			rh->leaf = r->leaf.table.e + x;
			rhtable[idx] = rh;
		}
	}
	/* Free hash table */
	for (x = 0; x != 1024; ++x) {
		struct rmaphentry *rh, *nh;
		for (rh = rhtable[x]; rh; rh = nh) {
			nh = rh->next;
			joe_free(rh);
		}
	}
	joe_free(rhtable);

	/* Create new leaf table */
	newalloc = r->leaf.alloc - dupcount;
	l = joe_malloc(SIZEOF(struct Ileaf) * newalloc);

	/* Copy entries */
	idx = 0;
	for (x = 0; x != r->leaf.alloc; ++x)
		if (equiv[x] == -1) {
			mcpy(l + idx, r->leaf.table.e + x, sizeof(struct Ileaf));
			l[idx].refcount = 1;
			repl[x] = idx;
			++idx;
		}
	for (x = 0; x != r->leaf.alloc; ++x)
		if (equiv[x] != -1) {
			repl[x] = repl[equiv[x]];
			++l[repl[equiv[x]]].refcount;
		}

	/* Install new table */
	joe_free(r->leaf.table.e);
	r->leaf.table.e = l;
	r->leaf.alloc = newalloc;
	r->leaf.size = newalloc;

	/* Remap third level */
	for (x = 0; x != r->third.alloc; ++x)
		for (idx = 0; idx != THIRDSIZE; ++idx)
			if (r->third.table.c[x].entry[idx] != -1)
				r->third.table.c[x].entry[idx] = repl[r->third.table.c[x].entry[idx]];

	joe_free(equiv);
	joe_free(repl);

	/* Set up mid table */
	for (x = 0; x != THIRDSIZE; ++x)
		r->mid.entry[x] = -1;
	idx = r->top.entry[0];
	if (idx != -1) {
		idx = r->second.table.b[idx].entry[0];
		if (idx != -1) {
			mcpy(r->mid.entry, r->third.table.c[idx].entry, SIZEOF(r->mid.entry));
		}
	}
}

void rmap_set(struct Rtree *r, struct interval *array, ptrdiff_t len, int map, int dflt)
{
	ptrdiff_t y;
	for (y = 0; y != len; ++y) {
		rmap_add(r, array[y].first, array[y].last, map, dflt);
	}
}

void rmap_show(struct Rtree *r)
{
	int first = -2;
	int last = -2;
	int val = 0;
	int a;

	int len = 0;
	int total = 0;
	printf("Rmap at %p\n", r);

	len = SIZEOF(struct Rtree);
	printf("Top level size = %d\n", len);
	total += len;

	len = r->second.alloc * SIZEOF(struct Mid);
	printf("Second level size = %d (%d entries)\n", len, r->second.alloc);
	total += len;

	len = r->third.alloc * SIZEOF(struct Mid);
	printf("Third level size = %d (%d entries)\n", len, r->third.alloc);
	total += len;

	len = r->leaf.alloc * SIZEOF(struct Ileaf);
	printf("Fourth level size = %d (%d entries)\n", len, r->leaf.alloc);
	total += len;

	printf("Total size = %d bytes\n", total);
	
	for (a = 0; a != TOPSIZE; ++a) {
		int ib = r->top.entry[a];
		if (ib != -1) {
			int b;
			for (b = 0; b != SECONDSIZE; ++b) {
				int ic = r->second.table.b[ib].entry[b];
				if (ic != -1) {
					int c;
					for (c = 0; c != THIRDSIZE; ++c) {
						int id = r->third.table.c[ic].entry[c];
						if (id != -1) {
							int d;
							for (d = 0; d != LEAFSIZE; ++d) {
								int ie = r->leaf.table.e[id].entry[d];
								int ch = (a << TOPSHIFT) + (b << SECONDSHIFT) + (c << THIRDSHIFT) + d;
								if (ch == last + 1 && ie == val) {
									last = ch;
								} else if (first != -2) {
									printf("%p show %x %x -> %d\n", r, first, last, val);
									first = last = ch;
									val = ie;
								} else {
									first = last = ch;
									val = ie;
								}
							}
						}
					}
				}
			}
		}
	}
	if (first != -2)
		printf("%p show %x %x -> %d\n", r, first, last, val);
}

/* Character classes */

void cclass_init(struct Cclass *m)
{
	m->size = 0;
	m->len = 0;
	m->intervals = 0;
}

void cclass_clr(struct Cclass *m)
{
	if (m->intervals)
		joe_free(m->intervals);
	m->size = 0;
	m->len = 0;
	m->intervals = 0;
}

/* Delete range from character class at position x
 * x is in range 0 to (m->len - 1), inclusive.
 */
static void cclass_del(struct Cclass *m, int x)
{
	mmove(m->intervals + x, m->intervals + x + 1, SIZEOF(struct interval) * (m->len - (x + 1)));
	--m->len;
}

/* We are about to insert: expand array if necessary */
static void cclass_grow(struct Cclass *m)
{
	if (m->len == m->size) {
		if (!m->size) {
			m->size = 1;
			m->intervals = (struct interval *)joe_malloc(SIZEOF(struct interval) * (m->size));
		} else {
			m->size *= 2;
			m->intervals = (struct interval *)joe_realloc(m->intervals, SIZEOF(struct interval) * (m->size));
		}
	}
}

/* Insert a range into a character class at position x
 * x is in range 0 to m->len inclusive.
 */
static void cclass_ins(struct Cclass *m, int x, int first, int last)
{
	cclass_grow(m);
	mmove(m->intervals + x + 1, m->intervals + x, SIZEOF(struct interval) * (m->len - x));
	++m->len;
	m->intervals[x].first = first;
	m->intervals[x].last = last;
}

/* Add a single range [first,last] into class m.  The resulting m->intervals will
 * be sorted and consist of non-overlapping, non-adjacent ranges.
 */

void cclass_add(struct Cclass *m, int first, int last)
{
	int x;

	/* If it's invalid, ignore */
	if (last < first)
		return;

	for (x = 0; x != m->len; ++x) {
		if (first > m->intervals[x].last + 1) {
			/* intervals[x] is below new range, skip it. */
		} else if (m->intervals[x].first > last + 1) {
			/* intervals[x] is fully above new range, so insert new one here */
			break;
		} else if (m->intervals[x].first <= first) {
			if (m->intervals[x].last >= last) { /* && m->intervals[x].first <= first */
				/* Existing covers new: we're done. */
				return;
			} else { /* m->intervals[x].last < last && m->intervals[x].first <= first */
				/* Enlarge new, delete existing */
				first = m->intervals[x].first;
				cclass_del(m, x);
				--x;
			}
		} else { /* m->intervals[x].first > first */
			if (m->intervals[x].last <= last) { /* && m->intervals[x].first <= first */
				/* New fully covers existing, delete existing */
				cclass_del(m, x);
				--x;
			} else { /* m->intervals[x].last > last && m->intervals[x].first > first */
				/* Extend existing */
				m->intervals[x].first = first;
				return;
			}
		}
	}
	cclass_ins(m, x, first, last);
}

/* Merge class n into class m */

void cclass_union(struct Cclass *m, struct Cclass *n)
{
	int x;
	if (n)
		for (x = 0; x != n->len; ++x)
			cclass_add(m, n->intervals[x].first, n->intervals[x].last);
}

void cclass_merge(struct Cclass *m, struct interval *array, int len)
{
	int x;
	for (x = 0; x != len; ++x)
		cclass_add(m, array[x].first, array[x].last);
}

/* Subtract a single range [first,last] from class m.  The resulting m->array will
 * be sorted and consist of non-overlapping, non-adjacent ranges.
 */

void cclass_sub(struct Cclass *m, int first, int last)
{
	int x;

	/* If it's invalid, ignore */
	if (last < first)
		return;

	for (x = 0; x != m->len; ++x) {
		if (first > m->intervals[x].last) {
			/* intervals[x] is below range, skip it. */
		} else if (m->intervals[x].first > last) {
			/* intervals[x] is fully above new range, we're done */
			break;
		} else if (first <= m->intervals[x].first) {
			if (last >= m->intervals[x].last) { /* && first <= m->intervals[x].first */
				/* Range fully covers entry, delete it */
				cclass_del(m, x);
				--x;
			} else { /* last < m->intervals[x].last && first <= m->intervals[x].first */
				/* Range cuts off bottom of entry */
				m->intervals[x].first = last + 1;
			}
		} else { /* first > m->intervals[x].first */
			if (last >= m->intervals[x].last) { /* && first > m->intervals[x].first */
				/* Range cuts off top of entry */
				m->intervals[x].last = first - 1;
			} else { /* last < m->intervals[x].last && first > m->intervals[x].first */
				/* Range is in middle of entry, split it */
				cclass_ins(m, x, m->intervals[x].first, first - 1);
				m->intervals[x + 1].first = last + 1;
				++x;
			}
			
		}
	}
}

/* Remove any parts of m which also appear in n */

void cclass_diff(struct Cclass *m, struct Cclass *n)
{
	int x;
	if (n)
		for (x = 0; x != n->len; ++x)
			cclass_sub(m, n->intervals[x].first, n->intervals[x].last);
}

/* Compute inverse of class m */

#define UNICODE_LAST 0x10FFFF

void cclass_inv(struct Cclass *m)
{
//	printf("\r\nBefore:\n");
//	cclass_show(m);
	if (m->len && !m->intervals[0].first) {
		/* Starts at 0 */
		int x;
		int last = m->intervals[0].last;
		for (x = 0; x != m->len - 1; ++x) {
			m->intervals[x].first = last + 1;
			m->intervals[x].last = m->intervals[x + 1].first - 1;
			last = m->intervals[x + 1].last;
		}
		if (last == UNICODE_LAST) {
			--m->len; /* Delete last one */
		} else {
			m->intervals[x].first = last + 1;
			m->intervals[x].last = UNICODE_LAST;
		}
	} else {
		/* Does not start at 0 */
		int x;
		int first = 0;
		for (x = 0; x != m->len; ++x) {
			int new_first = m->intervals[x].last + 1;
			m->intervals[x].last = m->intervals[x].first - 1;
			m->intervals[x].first = first;
			first = new_first;
		}
		if (first != UNICODE_LAST) {
			cclass_grow(m);
			m->intervals[x].first = first;
			m->intervals[x].last = UNICODE_LAST;
			++m->len;
		}
	}
//	printf("\r\nAfter:\n");
//	cclass_show(m);
//	sleep(1);
}

int cclass_lookup_unopt(struct Cclass *m, int ch)
{
	return interval_test(m->intervals, m->len, ch) != -1;
}

void cclass_opt(struct Cclass *m)
{
	rset_init(m->rset);
	rset_set(m->rset, m->intervals, m->len);
	rset_opt(m->rset);
}

int cclass_lookup(struct Cclass *m, int ch)
{
	return rset_lookup(m->rset, ch);
}

void cclass_show(struct Cclass *m)
{
	int x;
	int first = 0;
	for (x = 0; x != m->len; ++x) {
		if (!first)
			first = 1;
		else
			printf(" ");
		printf("[%x..%x]", (unsigned)m->intervals[x].first, (unsigned)m->intervals[x].last);
	}
	printf("\n");
}
