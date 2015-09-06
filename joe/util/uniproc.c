/* Process UnicodeData.txt into category tables */

/* UnicodeData.txt does not include every character!

   Instead it gives a range like this:

    3400;<CJK Ideograph Extension A, First>;Lo;0;L;;;;;N;;;;;
    4DB5;<CJK Ideograph Extension A, Last>;Lo;0;L;;;;;N;;;;;
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Field parsing macros */

#define TOFIRST \
    do { \
        for (x = 0; buf[x] == ' ' || buf[x] == '\t'; ++x); \
    } while (0)

#define TONEXT \
    do { \
        buf[y] = c; \
        x = y; \
        if (buf[x] == ';') \
            ++x; \
        while (buf[x] == ' ' || buf[x] == '\t') \
            ++x; \
    } while (0)

#define TOEND \
    do { \
        for (y = x; buf[y] && buf[y] != '#' && buf[y] != ';' && buf[y] != ' ' && buf[y] != '\t' && buf[y] != '\n' && buf[y] != '\r'; ++y); \
        c = buf[y]; buf[y] = 0; \
    } while (0)

#define TOEND1 \
    do { \
        for (y = x; buf[y] && buf[y] != '#' && buf[y] != ';' && buf[y] != '\n' && buf[y] != '\r'; ++y); \
        c = buf[y]; buf[y] = 0; \
    } while (0)

#define COMMA \
    do { \
        if (first) { \
            printf(",\n"); \
        } else { \
            first = 1; \
        } \
    } while (0)

/* Generate full case folding tables:
       First find character in fold_table.
       Next, use same index into repl_table.
       Replace the character with 1 - 3 characters found in repl_table.

       For Turkic languages (tr, az), there are two modifications:
           0x49 -> 0x131 (instead of 0x69)
           0x130 -> 0x60 (instead of 0x69, 0x307)
*/

#define REPLLEN 3 /* Maximum number of replacement characters for case folding */

int unifold_full(char *name)
{
    int repl[1024][REPLLEN];
    FILE *f;
    char buf[1024];
    unsigned in_low, in_high;
    unsigned out_low, out_high;
    int line = 0;
    int first = 0;
    int len = 0;
    int x;
    f = fopen(name, "r");
    if (!f) {
        fprintf(stderr, "couldn't open %s\n", name);
        return -1;
    }
    in_low = 0;
    out_low = 0;

    printf("\n");
    printf("struct interval fold_table[] = {\n");

    while (fgets(buf, sizeof(buf), f)) {
        int x, y, c;
        unsigned inval;
        char flag;
        unsigned outval[8];
        int outval_ptr;
        char *endp; 

        ++line;

        for (outval_ptr = 0; outval_ptr != 8; ++outval_ptr)
            outval[outval_ptr] = 0;

        TOFIRST;

        /* Skip blank lines */
        if (buf[x] == '\r' || buf[x] == '\n' || buf[x] == '#' || !buf[x])
            continue;

        /* Skip to end of field */
        TOEND;

        /* Input character */
        inval = (unsigned int)strtol(buf + x, &endp, 16);
        if (endp != buf + y || endp == buf + x) {
            fprintf(stderr, "%s %d: invalid input character code\n", name, line);
            return -1;
        }

        /* flag field */
        TONEXT;
        TOEND;
        if (!buf[x] && buf[x + 1]) {
            fprintf(stderr, "%s %d: bad flag character\n", name, line);
            return -1;
        }

        flag = buf[x];
        /* Ignore turkish special case and simple folding */
        if (flag == 'T' || flag == 'S')
            continue;

        if (flag != 'F' && flag != 'C') {
            fprintf(stderr,"%s %d: unknown flag", name, line);
            return -1;
        }

        for (outval_ptr = 0; ;++outval_ptr) {
            TONEXT;
            TOEND;
            if (!buf[x])
                break;
            outval[outval_ptr] = (unsigned int)strtol(buf + x, &endp, 16);
            if (endp != buf + y || endp == buf + x) {
                fprintf(stderr, "%s %d: invalid output character code\n", name, line);
                return -1;
            }
        }
        if (!outval_ptr || outval_ptr > REPLLEN) {
            fprintf(stderr, "%s %d: invalid output string\n", name, line);
            return -1;
        }

        if (outval_ptr == 1) {
            /* Simple mapping */
            if (in_low == 0) {
                /* New */
                in_low = in_high = inval;
                out_low = out_high = outval[0];
            } else if (in_high + 1 == inval && out_high + 1 == outval[0]) {
                /* Extend */
                in_high = inval;
                out_high = outval[0];
            } else {
                /* Jump */
                COMMA;
                printf("	{ 0x%x, 0x%x }", in_low, in_high);
                repl[len][0] = out_low;
                repl[len][1] = 0;
                repl[len][2] = 0;
                ++len;
                in_low = in_high = inval;
                out_low = out_high = outval[0];
            }
        } else {
            /* Character to string mapping */
            if (in_low != 0) {
                COMMA;
                printf("	{ 0x%x, 0x%x }", in_low, in_high);
                repl[len][0] = out_low;
                repl[len][1] = 0;
                repl[len][2] = 0;
                ++len;
                in_low = 0;
            }
            COMMA;
            printf("	{ 0x%x, 0x%x }", inval, inval );
            repl[len][0] = outval[0];
            repl[len][1] = outval[1];
            repl[len][2] = outval[2];
            ++len;
        }
    }
    if (in_low != 0) {
        COMMA;
        printf("	{ 0x%x, 0x%x }", in_low, in_high );
        repl[len][0] = out_low;
        repl[len][1] = 0;
        repl[len][2] = 0;
        ++len;
    }
    COMMA;
    printf("	{ 0x0, 0x0 }");
    printf("\n};\n");
    first = 0;
    printf("\nint fold_repl[][REPLLEN] = {\n");
    for (x = 0; x != len; ++x) {
        COMMA;
        printf("	{ 0x%x, 0x%x, 0x%x }", repl[x][0], repl[x][1], repl[x][2]);
    }
    printf("\n};\n");
    fclose(f);
    return 0;
}

/* Generate simple case folding table */

int unifold_simple(char *name)
{
    FILE *f;
    char buf[1024];
    unsigned in_low, in_high;
    unsigned out_low, out_high;
    int line = 0;
    int first = 0;
    f = fopen(name, "r");
    if (!f) {
        fprintf(stderr, "couldn't open %s\n", name);
        return -1;
    }
    in_low = 0;
    out_low = 0;

    printf("\n");
    printf("struct casefold tolower_table[] = {\n");

    while (fgets(buf, sizeof(buf), f)) {
        int x, y, c;
        unsigned inval;
        char flag;
        unsigned outval[8];
        int outval_ptr;
        char *endp; 

        ++line;

        for (outval_ptr = 0; outval_ptr != 8; ++outval_ptr)
            outval[outval_ptr] = 0;

        TOFIRST;

        /* Skip blank lines */
        if (buf[x] == '\r' || buf[x] == '\n' || buf[x] == '#' || !buf[x])
            continue;

        /* Skip to end of field */
        TOEND;

        /* Input character */
        inval = (unsigned int)strtol(buf + x, &endp, 16);
        if (endp != buf + y || endp == buf + x) {
            fprintf(stderr, "%s %d: invalid input character code\n", name, line);
            return -1;
        }

        /* flag field */
        TONEXT;
        TOEND;
        if (!buf[x] && buf[x + 1]) {
            fprintf(stderr, "%s %d: bad flag character\n", name, line);
            return -1;
        }

        flag = buf[x];
        /* Ignore turkish special case and full folding */
        if (flag == 'T' || flag == 'F')
            continue;

        /* Complain if we're not left with Simple or Common */
        if (flag != 'S' && flag != 'C') {
            fprintf(stderr,"%s %d: unknown flag", name, line);
            return -1;
        }

        for (outval_ptr = 0; ;++outval_ptr) {
            TONEXT;
            TOEND;
            if (!buf[x])
                break;
            outval[outval_ptr] = (unsigned int)strtol(buf + x, &endp, 16);
            if (endp != buf + y || endp == buf + x) {
                fprintf(stderr, "%s %d: invalid output character code\n", name, line);
                return -1;
            }
        }
        if (!outval_ptr || outval_ptr > 1) {
            fprintf(stderr, "%s %d: invalid output string\n", name, line);
            return -1;
        }

        if (outval_ptr == 1) {
            /* Simple mapping */
            if (in_low == 0) {
                /* New */
                in_low = in_high = inval;
                out_low = out_high = outval[0];
            } else if (in_high + 1 == inval && out_high + 1 == outval[0]) {
                /* Extend */
                in_high = inval;
                out_high = outval[0];
            } else {
                /* Jump */
                COMMA;
                printf("	{ 0x%x, 0x%x, 0x%x, 0x0, 0x0 }", in_low, in_high, out_low);
                in_low = in_high = inval;
                out_low = out_high = outval[0];
            }
        } else {
            /* Character to string mapping */
            if (in_low != 0) {
                COMMA;
                printf("	{ 0x%x, 0x%x, 0x%x, 0x0, 0x0 }", in_low, in_high, out_low);
                in_low = 0;
            }
            COMMA;
            printf("	{ 0x%x, 0x%x, 0x%x, 0x%x, 0x%x }", inval, inval, outval[0], outval[1], outval[2] );
        }
    }
    if (in_low != 0) {
        COMMA;
        printf("	{ 0x%x, 0x%x, 0x%x, 0x0, 0x0 }", in_low, in_high, out_low);
    }
    COMMA;
    printf("	{ 0x0, 0x0, 0x0, 0x0, 0x0 }");
    printf("\n};\n");
    fclose(f);
    return 0;
}

/* Loaded version of UnicodeData.txt file */

struct unidata {
    struct unidata *next;
    int line;
    char *code_string;
    int code;
    char *char_name;
    char *cat;
    char *combining;
    char *bidi;
    char *decomp;
    char *ddigval;
    char *digval;
    char *numval;
    char *mirrored;
    char *oldname;
    char *comment;
    char *upper_string;
    int upper;
    char *lower_string;
    int lower;
    char *title_string;
    int title;
};

struct unidata *uniload(char *name)
{
    struct unidata *first, *last;
    char buf[1024];
    int line = 0;
    FILE  *f;
    first = 0;
    f = fopen(name, "r");
    if (!f) {
        fprintf(stderr, "Couldn't open %s\n", name);
        return NULL;
    }
    while (fgets(buf, sizeof(buf), f)) {
        struct unidata *u;
        int x, y, c;
        u = (struct unidata *)malloc(sizeof(struct unidata));
        u->next = 0;
        u->line = ++line;
        TOFIRST;
        /* Skip blank lines */
        if (buf[x] == '\r' || buf[x] == '\n' || buf[x] == '#' || !buf[x])
            continue;
        TOEND1;
        u->code_string = strdup(buf + x);
        if (1 != sscanf(u->code_string, "%x", &u->code)) {
            fprintf(stderr, "%s %d: Couldn't parse code\n", name, line);
            exit(-1);
        }
        TONEXT;
        TOEND1;
        u->char_name = strdup(buf + x);
        TONEXT;
        TOEND1;
        u->cat = strdup(buf + x);
        TONEXT;
        TOEND1;
        u->combining = strdup(buf + x);
        TONEXT;
        TOEND1;
        u->bidi = strdup(buf + x);
        TONEXT;
        TOEND1;
        u->decomp = strdup(buf + x);
        TONEXT;
        TOEND1;
        u->ddigval = strdup(buf + x);
        TONEXT;
        TOEND1;
        u->digval = strdup(buf + x);
        TONEXT;
        TOEND1;
        u->numval = strdup(buf + x);
        TONEXT;
        TOEND1;
        u->mirrored = strdup(buf + x);
        TONEXT;
        TOEND1;
        u->oldname = strdup(buf + x);
        TONEXT;
        TOEND1;
        u->comment = strdup(buf + x);
        TONEXT;
        TOEND1;
        u->upper_string = strdup(buf + x);
        if (1 != sscanf(u->upper_string, "%x", &u->upper))
            u->upper = -1;
        TONEXT;
        TOEND1;
        u->lower_string = strdup(buf + x);
        if (1 != sscanf(u->lower_string, "%x", &u->lower))
            u->lower = -1;
        TONEXT;
        TOEND1;
        u->title_string = strdup(buf + x);
        if (1 != sscanf(u->title_string, "%x", &u->title))
            u->title = -1;
        if (strstr(u->char_name, " Last>")) {
            int n;
            for (n = last->code + 1; n < u->code; ++n) {
                struct unidata *v = (struct unidata *)malloc(sizeof(struct unidata));
                *v = *u;
                v->code = n;
                last->next = v;
                last = v;
            }
        }
        if (!first) {
            first = last = u;
        } else {
            last->next = u;
            last = u;
        }
    }
    fclose(f);
    return first;
}

/* Generate category tables */

struct cat {
    struct cat *next;
    char *name;
    int size;
    int idx;
} *cats;

struct cat *addcat(char *s)
{
    struct cat *c;
    for (c = cats; c; c = c->next)
        if (!strcmp(c->name, s))
            return c;
    if (!c) {
        c = (struct cat *)malloc(sizeof(struct cat));
        c->next = cats;
        cats = c;
        c->name = strdup(s);
        c->size = 0;
        c->idx = -1;
        return c;
        /* printf("New categry %s\n", s); */
    }
}

int unicat(char *name)
{
    struct unidata *u, *v;
    struct cat *cat;
    int low, high;
    int diglow, dighigh;
    int nd;
    int first;
    int reclow[2048];
    int rechigh[2048];
    int cvt[2048];
    int len;
    int n;

    /** Create table of categories from UnicodeData.txt **/
    u = uniload(name);
    if (!u)
        return -1;

    /* First pass: find all category codes */
    for (v = u; v; v = v->next)
        addcat(v->cat);

    /* Generate a table for each category */
    for (cat = cats; cat; cat = cat->next) if (cat->idx == -1) {
        int count = 0;
        low = high = -2;
        printf("\n");
        printf("struct interval %s_table[] = {\n", cat->name);
        nd = !strcmp(cat->name, "Nd"); /* Set for digit table */
        for (v = u; v; v = v->next) {
            if (!strcmp(v->cat, cat->name)) {
                int digval;
                if (nd) { /* For digits: group them by 10 */
                    if (v->ddigval[0] >= '0' && v->ddigval[0] <= '9') {
                        digval = v->ddigval[0] - '0';
                        if (v->code == high + 1 && digval == dighigh + 1) {
                            high = v->code;
                            dighigh = digval;
                        } else {
                            if (low != -2) {
                                ++count;
                                printf("	{ 0x%x, 0x%x },\n", low, high);
                            }
                            low = high = v->code;
                            diglow = dighigh = digval;
                        }
                    }
                } else {
                    if (v->code == high + 1) {
                        high = v->code;
                    } else {
                        if (low != -2) {
                            ++count;
                            printf("	{ 0x%x, 0x%x },\n", low, high);
                        }
                        low = high = v->code;
                    }
                }
            }
        }
        if (low != -2) {
            printf("	{ 0x%x, 0x%x }\n", low, high);
            ++count;
        }
        printf("};\n");
        cat->size = count;
    }

    /* Generate convert to uppercase table */
    printf("\n");
    printf("struct interval toupper_table[] = {\n");
    low = high = -2;
    diglow = dighigh = -2;
    first = 0;
    len = 0;
    for (v = u; v; v = v->next) {
        if (v->upper != -1 && v->upper != v->code) {
            if (v->code == high + 1 && v->upper == dighigh + 1) {
                high = v->code;
                dighigh = v->upper;
            } else {
                if (low != -2) {
                    printf("	{ 0x%x, 0x%x }, /* 0x%x */\n", low, high, diglow);
                    reclow[len] = low;
                    rechigh[len] = high;
                    cvt[len++] = diglow;
                }
                low = high = v->code;
                diglow = dighigh = v->upper;
            }
        }
    }
    if (low != -2) {
        COMMA;
        printf("	{ 0x%x, 0x%x }, /* 0x%x */\n", low, high, diglow);
        reclow[len] = low;
        rechigh[len] = high;
        cvt[len++] = diglow;
    }
    printf("	{ 0x0, 0x0 }\n");
    printf("};\n");
    printf("int toupper_cvt[] = {\n");
    for (n = 0; n != len; ++n)
        printf("	0x%x, /* 0x%x..0x%x */\n", cvt[n],reclow[n],rechigh[n]);
    printf("	0x0\n");
    printf("};\n");

    /* Generate convert to lowercase table */
    printf("\n");
    printf("struct interval tolower_table[] = {\n");
    low = high = -2;
    diglow = dighigh = -2;
    first = 0;
    len = 0;
    for (v = u; v; v = v->next) {
        if (v->lower != -1 && v->lower != v->code) {
            if (v->code == high + 1 && v->lower == dighigh + 1) {
                high = v->code;
                dighigh = v->lower;
            } else {
                if (low != -2) {
                    printf("	{ 0x%x, 0x%x }, /* 0x%x */\n", low, high, diglow);
                    reclow[len] = low;
                    rechigh[len] = high;
                    cvt[len++] = diglow;
                }
                low = high = v->code;
                diglow = dighigh = v->lower;
            }
        }
    }
    if (low != -2) {
        COMMA;
        printf("	{ 0x%x, 0x%x }, /* 0x%x */\n", low, high, diglow);
        reclow[len] = low;
        rechigh[len] = high;
        cvt[len++] = diglow;
    }
    printf("	{ 0x0, 0x0 }\n");
    printf("};\n");
    printf("int tolower_cvt[] = {\n");
    for (n = 0; n != len; ++n)
        printf("	0x%x, /* 0x%x..0x%x */\n", cvt[n],reclow[n],rechigh[n]);
    printf("	0x0\n");
    printf("};\n");

    /* Generate convert to titlecase table */
    printf("\n");
    printf("struct interval totitle_table[] = {\n");
    low = high = -2;
    diglow = dighigh = -2;
    first = 0;
    len = 0;
    for (v = u; v; v = v->next) {
        if (v->title != -1 && v->title != v->code) {
            if (v->code == high + 1 && v->title == dighigh + 1) {
                high = v->code;
                dighigh = v->title;
            } else {
                if (low != -2) {
                    printf("	{ 0x%x, 0x%x }, /* 0x%x */\n", low, high, diglow);
                    reclow[len] = low;
                    rechigh[len] = high;
                    cvt[len++] = diglow;
                }
                low = high = v->code;
                diglow = dighigh = v->title;
            }
        }
    }
    if (low != -2) {
        COMMA;
        printf("	{ 0x%x, 0x%x }, /* 0x%x */\n", low, high, diglow);
        reclow[len] = low;
        rechigh[len] = high;
        cvt[len++] = diglow;
    }
    printf("	{ 0x0, 0x0 }\n");
    printf("};\n");
    printf("int totitle_cvt[] = {\n");
    for (n = 0; n != len; ++n)
        printf("	0x%x, /* 0x%x..0x%x */\n", cvt[n],reclow[n],rechigh[n]);
    printf("	0x0\n");
    printf("};\n");

    /* Generate lookup table */
    printf("\n");
    printf("struct unicat unicat[] = {\n");
    for (cat = cats; cat; cat = cat->next) {
        if (cat->idx == -1)
            printf("	{ \"%s\", %d, %s_table, 0 },\n", cat->name, cat->size, cat->name);
        else
            printf("	{ \"%s\", %d, uniblocks + %d, 0 },\n", cat->name, cat->size, cat->idx);
    }
    printf("	{ 0, 0, 0, 0 }\n");
    printf("};\n");
    return 0;
}

/* Generate block name table */

int uniblocks(char *name)
{
    FILE *f;
    char buf[1024];
    int first = 0;
    int idx = 0;
    /** Create table of block names **/
    f = fopen(name, "r");
    if (!f) {
        fprintf(stderr,"Couldn't open %s\n", name);
        return -1;
    }
    printf("\n/* Unicode blocks */\n");
    printf("\n");
    printf("struct interval uniblocks[] = {\n");
    while (fgets(buf, sizeof(buf), f)) {
        if (buf[0] >= '0' && buf[0] <= '9' ||
            buf[0] >= 'A' && buf[0] <= 'F' ||
            buf[0] >= 'a' && buf[0] <= 'f') {
                unsigned low;
                unsigned high;
                char buf1[1024];
                int x, y, c;
                struct cat *cat;
                TOFIRST;
                TOEND;
                sscanf(buf, "%x..%x", &low, &high);
                TONEXT;
                TOEND1;
                COMMA;
                printf("	{ 0x%x, 0x%x } /* %s */", low, high, buf + x);
                cat = addcat(buf + x);
                cat->size = 1;
                cat->idx = idx;
                ++idx;
        }
    }
    fclose(f);
    printf("\n};\n");
    return 0;
}

/* Generate width table */

int uniwidth(char *name)
{
    FILE *f;
    char buf[1024];
    int idx = 0;
    int low = -2, high = -2;
    /** Create table of block names **/
    f = fopen(name, "r");
    if (!f) {
        fprintf(stderr,"Couldn't open %s\n", name);
        return -1;
    }
    printf("\n/* Double-wide characters */\n");
    printf("\n");
    printf("struct interval width_table[] = {\n");
    while (fgets(buf, sizeof(buf), f)) {
        if (buf[0] >= '0' && buf[0] <= '9' ||
            buf[0] >= 'A' && buf[0] <= 'F' ||
            buf[0] >= 'a' && buf[0] <= 'f') {
                unsigned l;
                unsigned h;
                int x, y, c;
                struct cat *cat;
                TOFIRST;
                TOEND;
                if (1 == sscanf(buf, "%x..%x", &l, &h))
                    h = l;
                TONEXT;
                TOEND1;
                if (buf[x] == 'W' || buf[x] == 'F') {
                    if (l != high + 1) {
                        if (low != -2) {
                            printf("	{ 0x%x, 0x%x },\n", low, high);
                        }
                        low = l;
                        high = h;
                    } else {
                        high = h;
                    }
                }
        }
    }
    fclose(f);
    if (low != -2) {
        printf("	{ 0x%x, 0x%x },\n", low, high);
    }
    printf("	{ 0x0, 0x0 }\n");
    printf("};\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int rtn;
    if (argc != 5) {
        fprintf(stderr,"%s Blocks.txt UnicodeData.txt CaseFolding.txt EastAsianWidth.txt", argv[0]);
        return -1;
    }
    printf("/* Unicode facts */\n");
    printf("\n");
    printf("#include \"types.h\"\n");
    rtn = uniblocks(argv[1]);
    if (rtn)
        return rtn;
    rtn = unicat(argv[2]);
    if (rtn)
        return rtn;
    rtn = unifold_full(argv[3]);
    if (rtn)
        return rtn;
    rtn = uniwidth(argv[4]);
    if (rtn)
        return rtn;

/*
    rtn = unifold_simple("CaseFolding.txt");
    if (rtn)
        return rtn;
*/
    return 0;
}
