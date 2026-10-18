#ifndef COMMON_H
#define COMMON_H

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline int parse_count(const char *text, int *count)
{
    char *end;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno || end == text || *end || value < 0 || value > INT_MAX) {
        return 0;
    }
    *count = (int)value;
    return 1;
}

/* Cisla oddeluji mezery nebo nove radky. 1 = cislo, 0 = konec, -1 = neplatny vstup. */
static inline int read_number(long double *value)
{
    int result = scanf("%Lf", value);
    if (result == 1) {
        return 1;
    }
    if (result == EOF) {
        return 0;
    }
    return -1;
}

#endif
