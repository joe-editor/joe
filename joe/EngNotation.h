//
// EngNotation.h
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

#ifndef EngineeringNotationFormatter_EngNotation_h
#define EngineeringNotationFormatter_EngNotation_h

enum { eng_prefixed=0, eng_exponential };

// converts a double to the specified number of digits, in either SI or exponential notation. The return string must be free()'d
char *to_engineering_string(char *buf, ptrdiff_t buf_size, double value, unsigned int digits, /* bool */int numeric);

// as above, but append the final string
char *to_engineering_string_unit(char *buf, ptrdiff_t buf_size, double value, unsigned int digits, /* bool */int numeric, const char *unit);

// converts the output of to_engineering_string() into a double
double from_engineering_string(const char *val);

// step a value by the smallest possible increment
char *step_engineering_string(char *buf, ptrdiff_t buf_size, const char *val, unsigned int digits, /* bool */ int numeric, /* bool */ int positive);

#endif
