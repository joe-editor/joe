/*
 *	Characters maps
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#include "types.h"

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

/* Build a cmap from an interval list */

void cmap_build(struct cmap *cmap, struct interval_list *list, void *dflt_map)
{
	struct interval_list *l;
	int x;
	/* Record default */
	cmap->dflt_map = dflt_map;
	for (x = 0; x != 128; ++x)
		cmap->direct_map[x] = dflt_map;
	/* Fill in direct map */
	for (l = list; l; l = l->next)
		if (l->interval.first >= 128)
			break;
		else {
			int ch;
			for (ch = l->interval.first; ch < 128 && ch <= l->interval.last; ++ch)
				cmap->direct_map[ch] = l->map;
		}
	/* Skip over list items which are wholly in the direct map */
	while (list && list->interval.last < 128)
		 list = list -> next;
	/* Calculate size of array */
	cmap->size = 0;
	for (l = list; l; l = l->next)
		++cmap->size;
	if (cmap->size) {
		/* Allocate and populate array */
		cmap->range_map = (struct interval_map *)joe_malloc(SIZEOF(struct interval_map) * cmap->size);
		x = 0;
		for (l = list; l; l = l->next) {
			cmap->range_map[x].interval.first = l->interval.first;
			cmap->range_map[x].interval.last = l->interval.last;
			cmap->range_map[x].map = l->map;
			if (cmap->range_map[x].interval.first < 128)
				cmap->range_map[x].interval.first = 128;
			++x;
		}
	} else
		cmap->range_map = 0;
}

/* Clear cmap, free space */

void clr_cmap(struct cmap *cmap)
{
	if (cmap->range_map)
		joe_free(cmap->range_map);
	cmap->range_map = 0;
	cmap->size = 0;
}

/* Lookup a character in a cmap, return its assigned mapping */

void *cmap_lookup(struct cmap *cmap, int ch)
{
	if (ch < 128)
		return cmap->direct_map[ch];
	if (cmap->size) {
		ptrdiff_t min = 0;
		ptrdiff_t mid;
		ptrdiff_t max = cmap->size - 1;
		if (ch < cmap->range_map[min].interval.first || ch > cmap->range_map[max].interval.last)
			goto no_match;
		while (max >= min) {
			mid = (min + max) / 2;
			if (ch > cmap->range_map[mid].interval.last)
				min = mid + 1;
			else if (ch < cmap->range_map[mid].interval.first)
				max = mid - 1;
			else
				return cmap->range_map[mid].map;
		}
	}
	no_match:
	return cmap->dflt_map;
}

#ifdef junk
struct slist {
	struct slist *next;
	char *s;
} *slist;

char *find(char *s)
{
	struct slist *l;
	for (l = slist; l; l = l->next)
		if (!zcmp(l->s, s))
			return l->s;
	l = (struct slist *)joe_malloc(SIZEOF(struct slist));
	l->next = slist;
	slist = l;
	l->s = zdup(s);
	return l->s;
}

int main(int argc, char *argv)
{
	char buf[100];
	struct interval_list *list = 0;
	while (gets(buf)) {
		if (buf[0] == 'a') {
			int first, last;
			int map;
			sscanf(buf + 1," %d %d %d",&first,&last,&map);
			printf("a %d %d %d\n", first, last, map);
			list = interval_add(list, first, last, map);
		} else if (buf[0] == 's') {
			struct interval_list *l;
			for (l = list; l; l = l->next)
				printf("%d %d %d\n",l->interval.first,l->interval.last,l->map);
		}
	}
}
#endif

/* Radix tree maps */

void *rtree_lookup(struct Rtree *r, int ch)
{
	int a = (TOPMASK & (ch >> TOPSHIFT));
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
	int a = (TOPMASK & (ch >> TOPSHIFT));
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
			break;
		}
	}
	return l->alloc++;
}

void rtree_add(struct Rtree *r, int ch, int che, void *map)
{
	int a = (TOPMASK & (ch >> TOPSHIFT));
	int b = (THIRDMASK & (ch >> SECONDSHIFT));
	int c = (SECONDMASK & (ch >> THIRDSHIFT));
	int d = (LEAFMASK & (ch >> LEAFSHIFT));

	int ib;
	int ic;
	int id;

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
					r->leaf.table.d[id].entry[d] = map;
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
			if (!memcmp(rh->leaf, r->leaf.table.d + x, SIZEOF(struct Leaf)))
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
			repl[x] = idx;
			++idx;
		}
	for (x = 0; x != r->leaf.alloc; ++x)
		if (equiv[x] != -1) {
			repl[x] = repl[equiv[x]];
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
/*
		int x;
		for (x = array[y].first; x <= array[y].last; ++x)
			rtree_add(r, x, map);
*/
	}
}
