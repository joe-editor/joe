#ifndef _JOE_CONFIG_H
#define _JOE_CONFIG_H

#include "autoconf.h"

#ifdef HAVE_SNPRINTF

#define joe_snprintf_0(buf,len,fmt) snprintf((buf),(len),(fmt))
#define joe_snprintf_1(buf,len,fmt,a) snprintf((buf),(len),(fmt),(a))
#define joe_snprintf_2(buf,len,fmt,a,b) snprintf((buf),(len),(fmt),(a),(b))
#define joe_snprintf_3(buf,len,fmt,a,b,c) snprintf((buf),(len),(fmt),(a),(b),(c))
#define joe_snprintf_4(buf,len,fmt,a,b,c,d) snprintf((buf),(len),(fmt),(a),(b),(c),(d))
#define joe_snprintf_5(buf,len,fmt,a,b,c,d,e) snprintf((buf),(len),(fmt),(a),(b),(c),(d),(e))
#define joe_snprintf_6(buf,len,fmt,a,b,c,d,e,f) snprintf((buf),(len),(fmt),(a),(b),(c),(d),(e),(f))
#define joe_snprintf_7(buf,len,fmt,a,b,c,d,e,f,g) snprintf((buf),(len),(fmt),(a),(b),(c),(d),(e),(f),(g))
#define joe_snprintf_8(buf,len,fmt,a,b,c,d,e,f,g,h) snprintf((buf),(len),(fmt),(a),(b),(c),(d),(e),(f),(g),(h))
#define joe_snprintf_9(buf,len,fmt,a,b,c,d,e,f,g,h,i) snprintf((buf),(len),(fmt),(a),(b),(c),(d),(e),(f),(g),(h),(i))
#define joe_snprintf_10(buf,len,fmt,a,b,c,d,e,f,g,h,i,j) snprintf((buf),(len),(fmt),(a),(b),(c),(d),(e),(f),(g),(h),(i),(j))

#else

#define joe_snprintf_0(buf,len,fmt) sprintf((buf),(fmt))
#define joe_snprintf_1(buf,len,fmt,a) sprintf((buf),(fmt),(a))
#define joe_snprintf_2(buf,len,fmt,a,b) sprintf((buf),(fmt),(a),(b))
#define joe_snprintf_3(buf,len,fmt,a,b,c) sprintf((buf),(fmt),(a),(b),(c))
#define joe_snprintf_4(buf,len,fmt,a,b,c,d) sprintf((buf),(fmt),(a),(b),(c),(d))
#define joe_snprintf_5(buf,len,fmt,a,b,c,d,e) sprintf((buf),(fmt),(a),(b),(c),(d),(e))
#define joe_snprintf_6(buf,len,fmt,a,b,c,d,e,f) sprintf((buf),(fmt),(a),(b),(c),(d),(e),(f))
#define joe_snprintf_7(buf,len,fmt,a,b,c,d,e,f,g) sprintf((buf),(fmt),(a),(b),(c),(d),(e),(f),(g))
#define joe_snprintf_8(buf,len,fmt,a,b,c,d,e,f,g,h) sprintf((buf),(fmt),(a),(b),(c),(d),(e),(f),(g),(h))
#define joe_snprintf_9(buf,len,fmt,a,b,c,d,e,f,g,h,i) sprintf((buf),(fmt),(a),(b),(c),(d),(e),(f),(g),(h),(i))
#define joe_snprintf_10(buf,len,fmt,a,b,c,d,e,f,g,h,i,j) sprintf((buf),(fmt),(a),(b),(c),(d),(e),(f),(g),(h),(i),(j))

#endif

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
