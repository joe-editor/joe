/*
	Regular expression subroutines
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
*/

#include <ctype.h>
#include "config.h"
#include "vs.h"
#include "b.h"
#include "regex.h"

int
escape (a, b)
     unsigned char **a;
     int *b;
{
	int c;
	unsigned char *s = *a;
	int l = *b;
	if (*s == '\\' && l >= 2)
	  {
		  ++s;
		  --l;
		  switch (*s)
		    {
		    case 'n':
			    c = 10;
			    break;
		    case 't':
			    c = 9;
			    break;
		    case 'a':
			    c = 7;
			    break;
		    case 'b':
			    c = 8;
			    break;
		    case 'f':
			    c = 12;
			    break;
		    case 'e':
			    c = 27;
			    break;
		    case 'r':
			    c = 13;
			    break;
		    case '8':
			    c = 8;
			    break;
		    case '9':
			    c = 9;
			    break;

		    case '0':
		    case '1':
		    case '2':
		    case '3':
		    case '4':
		    case '5':
		    case '6':
		    case '7':
			    c = *s - '0';
			    if (l > 1 && s[1] >= '0' && s[1] <= '7')
				    c = c * 8 + s[1] - '0', ++s, --l;
			    if (l > 1 && s[1] >= '0' && s[1] <= '7')
				    c = c * 8 + s[1] - '0', ++s, --l;
			    break;

		    case 'x':
		    case 'X':
			    c = 0;
			    if (l > 1 && s[1] >= '0' && s[1] <= '9')
				    c = c * 16 + s[1] - '0', ++s, --l;
			    else if (l > 1 && s[1] >= 'A' && s[1] <= 'F')
				    c = c * 16 + s[1] - 'A' + 10, ++s, --l;
			    else if (l > 1 && s[1] >= 'a' && s[1] <= 'f')
				    c = c * 16 + s[1] - 'a' + 10, ++s, --l;
			    if (l > 1 && s[1] >= '0' && s[1] <= '9')
				    c = c * 16 + s[1] - '0', ++s, --l;
			    else if (l > 1 && s[1] >= 'A' && s[1] <= 'F')
				    c = c * 16 + s[1] - 'A' + 10, ++s, --l;
			    else if (l > 1 && s[1] >= 'a' && s[1] <= 'f')
				    c = c * 16 + s[1] - 'a' + 10, ++s, --l;
			    break;

		    default:
			    c = *s;
			    break;
		    }
		  ++s;
		  --l;
	  }
	else
		(c = *s++), --l;
	*a = s;
	*b = l;
	return c;
}

static int
brack (a, la, c)
     unsigned char **a;
     int *la;
     unsigned char c;
{
	int inverse = 0;
	int flag = 0;
	unsigned char *s = *a;
	int l = *la;
	if (!l)
		return 0;
	if (*s == '^' || *s == '*')
		inverse = 1, ++s, --l;
	if (l && *s == ']')
	  {
		  ++s;
		  --l;
		  if (c == ']')
			  flag = 1;
	  }
	while (l)
		if (*s == ']')
		  {
			  ++s;
			  --l;
			  break;
		  }
		else
		  {
			  int cl, cr;
			  cl = escape (&s, &l);
			  if (l >= 2 && s[0] == '-' && s[1] != ']')
			    {
				    --l;
				    ++s;
				    cr = escape (&s, &l);
				    if (c >= cl && c <= cr)
					    flag = 1;
			    }
			  else if (c == cl)
				  flag = 1;
		  }
	*a = s;
	*la = l;
	if (inverse)
		return !flag;
	else
		return flag;
}

static void
savec (pieces, n, c)
     char *pieces[];
     char c;
{
	char *s = 0;
	if (pieces[n])
		vsrm (pieces[n]);
	s = vsncpy (s, 0, &c, 1);
	pieces[n] = s;
}

static void
saves (pieces, n, p, szz)
     char *pieces[];
     P *p;
     long szz;
{
	if (szz >= MAXINT - 31)
		pieces[n] = vstrunc (pieces[n], 0);
	else
	  {
		  pieces[n] = vstrunc (pieces[n], (int) szz);
		  brmem (p, pieces[n], (int) szz);
	  }
}

static int
skip_special (p)
     P *p;
{
	int to, s;
	switch (s = pgetc (p))
	  {
	  case '"':
		  do
			  if ((s = pgetc (p)) == '\\')
				  pgetc (p), s = pgetc (p);
		  while (s != MAXINT && s != '\"');
		  if (s == '\"')
			  return MAXINT - 1;
		  break;

	  case '\'':
		  if ((s = pgetc (p)) == '\\')
			  s = pgetc (p), s = pgetc (p);
		  if (s == '\'')
			  return MAXINT - 1;
		  if ((s = pgetc (p)) == '\'')
			  return MAXINT - 1;
		  if ((s = pgetc (p)) == '\'')
			  return MAXINT - 1;
		  break;

	  case '[':
		  to = ']';
		  goto skip;
	  case '(':
		  to = ')';
		  goto skip;
	  case '{':
		  to = '}';
		skip:do
			  s = skip_special (p);
		  while (s != to && s != MAXINT);
		  if (s == to)
			  return MAXINT - 1;
		  break;

	  case '/':
		  s = pgetc (p);
		  if (s == '*')
			  do
			    {
				    s = pgetc (p);
				    while (s == '*')
					    if ((s = pgetc (p)) == '/')
						    return MAXINT - 1;
			    }
			  while (s != MAXINT);
		  else if (s != MAXINT)
			  s = prgetc (p);
		  else
			  s = '/';
		  break;

	  }
	return s;
}

int
pmatch (pieces, regex, len, p, n, icase)
     char *pieces[];
     char *regex;
     P *p;
{
	int c, d;
	P *q = pdup (p);
	P *o = 0;
	while (len--)
		switch (c = *regex++)
		  {
		  case '\\':
			  if (!len--)
				  goto fail;
			  switch (c = *regex++)
			    {
			    case '?':
				    d = pgetc (p);
				    if (d == MAXINT)
					    goto fail;
				    savec (pieces, n++, (char) d);
				    break;

			    case 'n':
			    case 'r':
			    case 'a':
			    case 'f':
			    case 'b':
			    case 't':
			    case 'e':
			    case 'x':
			    case 'X':
			    case '0':
			    case '1':
			    case '2':
			    case '3':
			    case '4':
			    case '5':
			    case '6':
			    case '7':
			    case '8':
			    case '9':
				    regex -= 2;
				    len += 2;
				    if (pgetc (p) != escape (&regex, &len))
					    goto fail;
				    break;

			    case '*':
				    /* Find shortest matching sequence */
				    o = pdup (p);
				    do
				      {
					      long pb = p->byte;
					      if (pmatch
						  (pieces, regex, len, p,
						   n + 1, icase))
						{
							saves (pieces, n, o,
							       pb - o->byte);
							goto succeed;
						}
					      c = pgetc (p);
				      }
				    while (c != MAXINT && c != '\n');
				    goto fail;

			    case 'c':
				    o = pdup (p);
				    do
				      {
					      long pb = p->byte;
					      if (pmatch
						  (pieces, regex, len, p,
						   n + 1, icase))
						{
							saves (pieces, n, o,
							       pb - o->byte);
							goto succeed;
						}
				      }
				    while (skip_special (p) != MAXINT);
				    goto fail;

			    case '[':
				    d = pgetc (p);
				    if (d == MAXINT)
					    goto fail;
				    if (!brack (&regex, &len, d))
					    goto fail;
				    savec (pieces, n++, (char) d);
				    break;

			    case '+':
				    {
					    char *oregex = regex;	/* Point to character to skip */
					    int olen = len;

					    char *tregex;
					    int tlen;

					    P *r = 0;
					    o = pdup (p);

					    /* Advance over character to skip */
					    if (len >= 2 && regex[0] == '\\'
						&& regex[1] == '[')
					      {
						      regex += 2;
						      len -= 2;
						      brack (&regex, &len, 0);
					      }
					    else if (len >= 1)
						    --len, ++regex;
					    else
						    goto done;

					    /* Now oregex/olen point to character to skip over and
					       regex/len point to sequence which follows */

					    do
					      {
						      P *z = pdup (p);
						      if (pmatch
							  (pieces, regex, len,
							   p, n + 1, icase))
							{
								saves (pieces,
								       n, o,
								       z->
								       byte -
								       o->
								       byte);
								if (r)
									prm
										(r);
								r = pdup (p);
							}
						      pset (p, z);
						      prm (z);
						      c = pgetc (p);
					      }
					    while (c != MAXINT &&
						   (*oregex == '\\' ?
						    (tregex =
						     oregex + 2, tlen =
						     olen - 2, brack (&tregex,
								      &tlen,
								      c))
						    : (icase ? toupper (c) ==
						       toupper (*oregex) : c
						       == *oregex)));

					  done:
					    if (r)
						    pset (p, r), prm (r);
					    if (r)
						    goto succeed;
					    else
						    goto fail;
				    }

			    case '^':
				    if (!pisbol (p))
					    goto fail;
				    break;

			    case '$':
				    if (!piseol (p))
					    goto fail;
				    break;

			    case '<':
				    if (!pisbow (p))
					    goto fail;
				    break;

			    case '>':
				    if (!piseow (p))
					    goto fail;
				    break;

			    default:
				    d = pgetc (p);
				    if (icase)
				      {
					      if (toupper (d) != toupper (c))
						      goto fail;
				      }
				    else
				      {
					      if (d != c)
						      goto fail;
				      }
			    }
			  break;

		  default:
			  d = pgetc (p);
			  if (icase)
			    {
				    if (toupper (d) != toupper (c))
					    goto fail;
			    }
			  else
			    {
				    if (d != c)
					    goto fail;
			    }
		  }
      succeed:
	if (o)
		prm (o);
	prm (q);
	return 1;

      fail:
	if (o)
		prm (o);
	pset (p, q);
	prm (q);
	return 0;
}
