/* Process UnicodeData.txt into category tables */

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
        for (x = y; buf[x] == ';' || buf[x] == ' ' || buf[x] == '\t'; ++x); \
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

/* Generate case folding table */

int unifold(char *name)
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
    printf("struct casefold fold_table[] = {\n");

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
        if (!outval_ptr || outval_ptr > 3) {
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
    FILE  *f;
    char buf[1024];
    struct cat *cat;
    int low, high;
    int diglow, dighigh;
    int line = 0;
    int nd;

    /** Create table of categories from UnicodeData.txt **/

    f = fopen(name, "r");
    if (!f) {
        fprintf(stderr, "Couldn't open %s\n", name);
        return -1;
    }

    /* First pass: find all category codes */
    while (fgets(buf, sizeof(buf), f)) {
        int x, y, c;
        ++line;

        TOFIRST;

        /* Skip blank lines */
        if (buf[x] == '\r' || buf[x] == '\n' || buf[x] == '#' || !buf[x])
            continue;

        TOEND1; /* First field: code point */

        TONEXT;
        TOEND1; /* Second field: name */
        
        TONEXT;
        TOEND1; /* Third field: category */

        if (buf[0]) addcat(buf + x);
    }

    /* Generate a table for each category */
    for (cat = cats; cat; cat = cat->next) if (cat->idx == -1) {
        rewind(f); line = 0;
        int count = 0;
        low = high = -2;
        printf("\n");
        printf("struct interval %s_table[] = {\n", cat->name);
        nd = !strcmp(cat->name, "Nd"); /* Set for digit table */
        while (fgets(buf, sizeof(buf), f)) {
            int x, y, c;
            int val;

            ++line;

            TOFIRST;

            /* Skip blank lines */
            if (buf[x] == '\r' || buf[x] == '\n' || buf[x] == '#' || !buf[x])
                continue;

            TOEND1; /* First field: code point */
            sscanf(buf + x, "%x", &val);

            TONEXT;
            TOEND1; /* Second field: name */
            
            TONEXT;
            TOEND1; /* Third field: category */

            if (!strcmp(buf + x, cat->name)) {
                if (nd) { /* For digits: group them by 10 */
                    int digval;
                    TONEXT;
                    TOEND1; /* Fourth field */ 
                    TONEXT;
                    TOEND1; /* Fifth field */ 
                    TONEXT;
                    TOEND1; /* Sixth field */ 
                    TONEXT;
                    TOEND1; /* Seventh field */ 
                    TONEXT;
                    TOEND1; /* Eighth field */ 
                    if (buf[x] >= '0' && buf[x] <= '9') {
                        digval = buf[x] - '0';
                        if (val == high + 1 && digval == dighigh + 1) {
                            high = val;
                            dighigh = digval;
                        } else {
                            if (low != -2) {
                                ++count;
                                printf("	{ 0x%x, 0x%x },\n", low, high);
                            }
                            low = high = val;
                            diglow = dighigh = digval;
                        }
                    }
                } else {
                    if (val == high + 1) {
                        high = val;
                    } else {
                        if (low != -2) {
                            ++count;
                            printf("	{ 0x%x, 0x%x },\n", low, high);
                        }
                        low = high = val;
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

    fclose(f);
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

int main(int argc, char *argv[])
{
    int rtn;
    printf("/* Unicode facts */\n");
    printf("\n");
    printf("#include \"types.h\"\n");
    rtn = uniblocks("Blocks.txt");
    if (rtn)
        return rtn;
    rtn = unicat("UnicodeData.txt");
    if (rtn)
        return rtn;
    rtn = unifold("CaseFolding.txt");
    if (rtn)
        return rtn;
    return 0;
}