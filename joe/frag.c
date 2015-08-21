/* Byte code fragment construction functions */

#include "types.h"

/* Initialize a fragment */

void iz_frag(Frag *f, ptrdiff_t alignment)
{
	f->start = joe_malloc(f->size = 1024);
	f->len = 0;
	f->align = alignment;
}

void fin_code(Frag *f)
{
	f->start = joe_realloc(f->start, f->len);
	f->size = f->len;
}

void clr_frag(Frag *f)
{
	joe_free(f->start);
}

/* Expand a fragment */

/* Expand code block by at least 'size' words */

static void expand_frag(Frag *frag, ptrdiff_t size)
{
	if ((frag->size >> 1) > size)
		// Grow by 50%
		frag->size = frag->size + (frag->size >> 1);
	else
		// Grow by 50% plus requested size
		frag->size += frag->size + (frag->size >> 1);

	frag->start = (unsigned char *)joe_realloc(frag->start, frag->size);
}

/* Emit a byte: this does no alignment */

ptrdiff_t emitb_noalign(Frag *f, char c)
{
	ptrdiff_t start;
	if (f->len + SIZEOF(unsigned char) > f->size)
		expand_frag(f, SIZEOF(unsigned char));
	start = f->len;
	f->start[f->len++] = (unsigned char)c;
	return start;
}

/* Align to some power of 2 */

void align_frag(Frag *f,ptrdiff_t alignment)
{
	ptrdiff_t x;
	ptrdiff_t add = align_o(f->len, alignment);
	for (x = 0; x != add; ++x)
		emitb_noalign(f, '-');
}

/* Emit a byte and align */

ptrdiff_t emitb(Frag *f, char c)
{
	ptrdiff_t ofst = emitb_noalign(f, c);
	if (f->len & (f->align - 1))
		align_frag(f, f->align);
	return ofst;
}

/* Emit a short */

ptrdiff_t emith(Frag *f, short c)
{
	ptrdiff_t start;
	if (f->len & (SIZEOF(short) - 1))
		align_frag(f, SIZEOF(short));
	if (f->len + SIZEOF(short) > f->size)
		expand_frag(f, SIZEOF(short));
	start = f->len;
	*(short *)(f->start + f->len) = c;
	f->len += SIZEOF(short);
	if (f->len & (f->align - 1))
		align_frag(f, f->align);
	return start;
}

/* Emit an integer */

ptrdiff_t emiti(Frag *f, int c)
{
	ptrdiff_t start;
	if (f->len & (SIZEOF(int) - 1))
		align_frag(f, SIZEOF(int));
	if (f->len + SIZEOF(int) > f->size)
		expand_frag(f, SIZEOF(int));
	start = f->len;
	*(int *)(f->start + f->len) = c;
	f->len += SIZEOF(int);
	if (f->len & (f->align - 1))
		align_frag(f, f->align);
	return start;
}

/* Emit a double */

ptrdiff_t emitd(Frag *f, double d)
{
	ptrdiff_t start;
	if (f->len & (SIZEOF(double) - 1))
		align_frag(f, SIZEOF(double));
	if (f->len + SIZEOF(double) > f->size)
		expand_frag(f, SIZEOF(double));
	start = f->len;
	*(double *)(f->start + f->len) = d;
	f->len += SIZEOF(double);
	if (f->len & (f->align - 1))
		align_frag(f, f->align);
	return start;
}

/* Emit a pointer */

ptrdiff_t emitp(Frag *f, void *p)
{
	ptrdiff_t start;
	if (f->len & (SIZEOF(void *) - 1))
		align_frag(f, SIZEOF(void *));
	if (f->len + SIZEOF(void *) > f->size)
		expand_frag(f, SIZEOF(void *));
	start = f->len;
	*(void **)(f->start + f->len) = p;
	f->len += SIZEOF(void *);
	if (f->len & (f->align - 1))
		align_frag(f, f->align);
	return start;
}

/* Append a string to the code block */

ptrdiff_t emits(Frag *f, unsigned char *s, int len)
{
	ptrdiff_t start;

	start = emiti(f, len);

	if (f->len + len + 1 > f->size)
		expand_frag(f, len + 1);

	if (len)
		mcpy(f->start + f->len, s, len);

	f->start[f->len + len] = 0;

	f->len += len + 1;

	if (f->len & (f->align - 1))
		align_frag(f, f->align);

	return start;
}

ptrdiff_t emit_branch(Frag *f, ptrdiff_t target)
{
	ptrdiff_t ofst = emiti(f, 0);
	fragi(f, ofst) = (int)(target - ofst);
	return ofst;
}

void fixup_branch(Frag *f, ptrdiff_t pos)
{
	align_frag(f, SIZEOF(int));
	fragi(f, pos) = (int)(f->len - pos);
}

/* Modify each item in a linked-list to point to here */

void frag_link(Frag *f, ptrdiff_t chain)
{
	ptrdiff_t ket;
	align_frag(f, SIZEOF(int));
	ket = f->len;
	while (chain) {
		int next = fragi(f, chain);
		fragi(f, chain) = (int)(ket - chain);
		chain = next;
	}
}

short fetchh(Frag *f, ptrdiff_t *pcp)
{
	ptrdiff_t pc = *pcp;
	short i;
	pc += align_o(pc, SIZEOF(short));
	i = fragh(f, pc);
	pc += SIZEOF(short);
	if (pc & (f->align - 1))
		pc += align_o(pc, f->align);
	*pcp = pc;
	return i;
}

int fetchi(Frag *f, ptrdiff_t *pcp)
{
	ptrdiff_t pc = *pcp;
	int i;
	pc += align_o(pc, SIZEOF(int));
	i = fragi(f, pc);
	pc += SIZEOF(int);
	if (pc & (f->align - 1))
		pc += align_o(pc, f->align);
	*pcp = pc;
	return i;
}

void *fetchp(Frag *f, ptrdiff_t *pcp)
{
	ptrdiff_t pc = *pcp;
	void *p;
	pc += align_o(pc, SIZEOF(void *));
	p = fragp(f, pc);
	pc += SIZEOF(void *);
	if (pc & (f->align - 1))
		pc += align_o(pc, f->align);
	*pcp = pc;
	return p;
}
