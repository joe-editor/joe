#ifndef _JOE_CONFIG_H
#define _JOE_CONFIG_H

#include "autoconf.h"

#define MAXINT  ((((unsigned int)-1)/2)-1)
#define MAXLONG ((((unsigned long)-1L)/2)-1)

#include <stdio.h>
#ifndef EOF
#define EOF -1
#endif
#define NO_MORE_DATA EOF

#if defined __MSDOS__ && SIZEOF_INT == 2 /* real mode ms-dos compilers */
#if SIZEOF_VOID_P == 4 /* real mode ms-dos compilers with 'far' memory model or something like that */
#define physical(a)  (((unsigned long)(a)&0xFFFF)+(((unsigned long)(a)&0xFFFF0000)>>12))
#define normalize(a) ((void *)(((unsigned long)(a)&0xFFFF000F)+(((unsigned long)(a)&0x0000FFF0)<<12)))
#else
#define physical(a) ((unsigned long)(a))
#define normalize(a) (a)
#endif /* sizeof(void *) == 4 */

#define SEGSIZ 1024
#define PGSIZE 1024
#define LPGSIZE 10
#define ILIMIT (PGSIZE*96L)
#define HTSIZE 128

#else /* not real mode ms-dos */

#define physical(a) ((unsigned long)(a))
#define normalize(a) (a)
#ifdef PAGE_SIZE
#define PGSIZE PAGE_SIZE
#else
#define PGSIZE 4096
#endif
#define SEGSIZ PGSIZE
#define LPGSIZE 12
#define ILIMIT (PGSIZE*1024)
#define HTSIZE 2048

#endif /* real mode ms-dos */

#endif /* ifndef _JOE_CONFIG_H */
