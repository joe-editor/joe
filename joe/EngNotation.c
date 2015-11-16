//
// EngNotation.c
// EngineeringNotationFormatter
//
// Copyright (C) 2005-2009 by Jukka Korpela
// Copyright (C) 2009-2013 by David Hoerl
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "types.h"
#include <ctype.h>
#include <assert.h>

#include "EngNotation.h"

// Set to 1 for debugging info
#if 0
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif


/* 
 * Smallest power of ten for which there is a prefix defined.
 * If the set of prefixes will be extended, change this constant
 * and update the table "prefix".
 */

#ifndef ENG_FORMAT_MICRO_GLYPH
# define ENG_FORMAT_MICRO_GLYPH "µ"
#endif

#define NUM_PREFIX 17
static const char *prefix[NUM_PREFIX] = {
  "y", "z", "a", "f",
  "p", "n", ENG_FORMAT_MICRO_GLYPH, "m",
  "",  "k", "M", "G",
  "T", "P", "E", "Z",
  "Y"
};
static const char *reversePrefix[NUM_PREFIX] = {
 "e-24", "e-21", "e-18", "e-15",
 "e-12", "e-9", "e-6", "e-3",
 "e0", "e3", "e6", "e9",
 "e12", "e15", "e18", "e21",
 "e24"
};

#define PREFIX_START -24
const int prefix_count = NUM_PREFIX;
const int prefix_start = PREFIX_START;
const int prefix_end   = PREFIX_START + 3 * ( NUM_PREFIX - 1 );

static char *engineering_to_exponential(const char *val);


char *to_engineering_string(char *buf, ptrdiff_t buf_size, double value, unsigned int digits, /* bool */ int exponential)
{
	/* bool */ int is_signed = signbit(value);
	const char *sign = is_signed ? "-" : "";
	
	int classify = fpclassify(value);
	if(classify != FP_NORMAL) {
		switch(classify) {
		case FP_SUBNORMAL:
		case FP_ZERO:
		default:
		{
			if(exponential) {
				joe_snprintf_3(buf, (size_t)buf_size, "%s%.*fe0", sign, digits-1, 0.0);
			} else {
				joe_snprintf_3(buf, (size_t)buf_size, "%s%.*f", sign, digits-1, 0.0);
			}
			return buf;
		}
		case FP_INFINITE:
			joe_snprintf_0(buf, buf_size, "INFINITE");
			return buf;
			break;
		case FP_NAN:
			joe_snprintf_0(buf, buf_size, "NaN");
			return buf;
			break;
		}
	}
	if(is_signed) {
		value = fabs(value);
	}

	if(digits < 3) {
		digits = 3;
	} else
	if(digits > 9) {
		digits = 9;
	}
	LOG("\n\n");

	// correctly round to desired precision
	long expof10 = lrint( floor( log10(value) ) );
	if(expof10 > 0) {
		expof10 = (expof10/3)*3;
		LOG("GT: expof10=%ld\n", expof10);
	} else {
		expof10 = ((-expof10+3)/3)*(-3);
		LOG("LTE: expof10=%ld\n", expof10);
	}

	value *= pow(10.0, (double)(-expof10));
	LOG("value=%lf\n", value);

	long lintgr, lfract;
	{
		double intgr, fract;
		
		fract = modf(value, &intgr);
		lintgr = lrint(intgr);
		LOG("compute: %ld . %lf\n", lintgr, fract);
	
		if(lintgr >= 1000) {
			digits -= 3;				// fractional digits
		} else
		if(lintgr >= 100) {
			LOG("lvalue > 100 (%ld)\n", lintgr);
			digits -= 2;				// fractional digits
		} else
		if(lintgr >= 10) {
			LOG("is > 10 (%ld)\n", lintgr);
			digits -= 1;				// fractional digits
		} else
		if(lintgr >= 1) {
			LOG("is > 1 (%ld)\n", lintgr);
		} else {
			LOG("YIKES: < 1\n");
			//assert(!"Impossible to get < 1 unless the fractional part is 1!");
			digits += 1;				// fractional digits
		}
		
		// how much to multiple the fraction to get it to round the one-off value
		double fractMult = pow(10.0, (int)digits - 1);
		long lfractMult = lrint(fractMult);
		LOG("digits=%d fractMult=%lf lfractMult=%ld\n", digits, fractMult, lfractMult);

		// round the fraction to the correct number of places
		fract *= fractMult;
		lfract = lrint(fract);
		LOG("lintgr=%ld lfract=%ld fract=%lf\n", lintgr, lfract, fract);

		// did the rounding the fractional component cause an increase in the integral value?
		if(lfract >= lfractMult) {
			LOG("ROUND\n");
			lfract -= lfractMult;			// remove overflow value

			long nlintgr = lintgr + 1;
			if( (lintgr < 1000 && nlintgr >= 1000) || (lintgr < 100 && nlintgr >= 100) || (lintgr < 10 && nlintgr >= 10) || (lintgr < 1 && nlintgr >= 1) ) {
				LOG("WRAP\n");
				lfract /= 10;
				fractMult /= 10;
				digits -= 1;
			}
			lintgr = nlintgr;				// rounded up, so increase integral part
			LOG("lintgr=%ld lfract=%ld\n", lintgr, lfract);
		}

		if(lintgr >= 1000) {
			LOG(">=1000\n");
			expof10 += 3;
			digits += 3;

			long fullVal = lrint((double)lintgr*fractMult) + lfract;
			long fullMult = lrint(1000.0 * fractMult);
			LOG("fullVal=%ld fullMult=%ld\n", fullVal, fullMult);

			lintgr = fullVal / fullMult;
			lfract = fullVal - (lintgr * fullMult);
			LOG("lintgr=%ld lfract=%ld\n", lintgr, lfract);
		}
	}

	--digits;
	const char *decimal_str = digits ? "." : "";
	
	if(exponential || (expof10 < prefix_start) || (expof10 > prefix_end)) {
		LOG("RESULT 1: digits=%d\n", digits);
		joe_snprintf_6(buf, (size_t)buf_size, "%s%ld%s%0.*lde%ld", sign, lintgr, decimal_str, digits, lfract, expof10);
	} else {
		LOG("RESULT 2: digits=%d\n", digits);
		const char *s = prefix[(expof10-prefix_start)/3];
		joe_snprintf_7(buf, (size_t)buf_size, "%s%ld%s%0.*ld%s%s", sign, lintgr, decimal_str, digits, lfract, *s ? " " : "", s);
	}
	return buf;
}

char *to_engineering_string_unit(char *buf, ptrdiff_t buf_size, double value, unsigned int digits, /* bool */ int numeric, const char *unit)
{
	char *ret = to_engineering_string(buf, buf_size, value, digits, numeric);
	
	if(isdigit(ret[0])) {
		size_t len = strlen(ret);
		if(numeric) ++len;
		ret = (char *)realloc(ret, len + strlen(unit));	// string already had the trailing null, so no need for another
		if(numeric) strcat(ret, " ");
		strcat(ret, unit);
	}
	return ret;
}

// converts the output of to_engineering_string() into a double
double from_engineering_string(const char *val)
{
	char *tstVal = engineering_to_exponential(val);
	double ret = strtod(tstVal, NULL);
	free(tstVal);
	return ret;
}

char *step_engineering_string(char *buf, ptrdiff_t buf_size, const char *val, unsigned int digits, /* bool */ int exponential, /* bool */ int positive)
{
	char *tstVal = engineering_to_exponential(val);
	double value = strtod(tstVal, NULL);
	free(tstVal);

	if(digits < 3) {
		digits = 3;
	}
	// correctly round to desired precision
	long expof10 = lrint( floor( log10(value) ) );
	long power =  expof10 + 1 - (int)digits;
	
	double inc = pow(10, (double)power) * (positive ? +1 : -1);
	double ret = value + inc;
	
	char *str = to_engineering_string(buf, buf_size, ret, digits, exponential);
	return str;
}

// Set to 1 for debugging info
#if 0
#undef LOG
#if 1
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif
#endif

static char *engineering_to_exponential(const char *val)
{
	size_t len = strlen(val);
	if(!len || !isdigit(val[0])) {
		return strdup("NaN");
	}
	char *ret = NULL;

	if(len >= 3) {
		// space and last value which might be a UTF-8 char. Formats are "1 y", "900.", "900. y", "1.23 M"
		int c = val[len-1];
		if(!isdigit(c) && c != '.') {
			int i = NUM_PREFIX;
			size_t j;
			/* bool */ int look = 0;
			const char *strt = val + len;
			
			for(j=0; j<len; ++j) {
				--strt;
				if(*strt == ' ') {
					look = j ? 1 : 0;	// anything after the space?
					++strt;
					break;
				}
			}
			if(look) {
				size_t strtlen = strlen(strt);
				for(i=0; i<NUM_PREFIX; ++i) {
					const char *p = prefix[i];
					size_t plen = strlen(p);
					if(!plen) continue;				// no prefix
					if(plen > strtlen) continue;	// insufficient chars
					LOG("COMPARE %.*s to %s\n", (int)plen, strt, p);
					if(!strncmp(strt, p, plen)) {
						// need 4+null for new string
						size_t vallen = (size_t)((strt - val) - 1);	// skip space
						ret = (char *)joe_malloc((ptrdiff_t)(vallen + 4 + 1));	// 4 the e-xx
						strncpy(ret, val, vallen);	// -1 removes the space
						ret[vallen] = '\0';
						strcat(ret, reversePrefix[i]);
						LOG("FINAL %s\n", ret);
						break;
					}
				}
			}
			if(i == NUM_PREFIX) {
				ret = strdup("NaN");
			}
		}
	}
	if(!ret) {
		// could be "1" (cheating, as the eng will not return a string < 3 chars)
		ret = strdup(val);
	}
	LOG("engineering_to_exponential RETURN %s\n", ret);
	return ret;
}
