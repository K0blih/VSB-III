#define _GNU_SOURCE

#include "strcheck_lib.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    char *line = NULL;
    size_t capacity = 0;
    size_t total = 0;

    while (getline(&line, &capacity, stdin) != -1) {
        total += check_line(line);
    }
    free(line);
    fprintf(stdout, "Total count: %zu\n", total);
    return 0;
}
