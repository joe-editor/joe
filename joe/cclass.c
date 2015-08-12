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

/* Return true if character ch is in radix tree r */

int rset_lookup(struct Rset *r, int ch)
{
	int a = (TOPMASK & (ch >> TOPSHIFT));
	int b = (SECONDMASK & (ch >> SECONDSHIFT));
	int c = (THIRDMASK & (ch >> THIRDSHIFT));
	int d = (LEAFMASK & (ch >> LEAFSHIFT));
	if (a >= TOPSIZE)
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
	int a = (TOPMASK & (ch >> TOPSHIFT));
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
	return NULL;
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
	return l->alloc++;
}

void rset_add(struct Rset *r, int ch)
{
	int a = (TOPMASK & (ch >> TOPSHIFT));
	int b = (THIRDMASK & (ch >> SECONDSHIFT));
	int c = (SECONDMASK & (ch >> THIRDSHIFT));
	int d = (LEAFMASK & (ch >> LEAFSHIFT));

	int ib;
	int ic;

	if (a >= TOPSIZE)
		return;

	ib = r->top.entry[a];
	if (ib == -1) {
		r->top.entry[a] = ib = rset_alloc(&r->second, 1);
	}

	ic = r->second.table.b[ib].entry[b];
	if (ic == -1) {
		r->second.table.b[ib].entry[b] = ic = rset_alloc(&r->third, 2);
	}

	r->third.table.c[ic].entry[c] |= (1 << d);
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
		int x;
		for (x = array[y].first; x <= array[y].last; ++x)
			rset_add(r, x);
	}
}

void rset_show(struct Rset *r)
{
	int first = -2;
	int last = -2;
	int a;
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
