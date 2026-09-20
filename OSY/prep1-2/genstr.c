#include "genstr_lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s N M\n", argv[0]);
        return 1;
    }

    int lines = atoi(argv[1]);
    int max_words = atoi(argv[2]);
    
    if (lines < 0 || max_words < 1) {
        fprintf(stderr, "N must be >= 0 and M must be >= 1.\n");
        return 1;
    }

    srand((unsigned int)time(NULL));
    for (int i = 0; i < lines; ++i) {
        char *line = generate_line(max_words);
        if (line == NULL) {
            fprintf(stderr, "Cannot allocate generated line.\n");
            return 1;
        }
        fprintf(stdout, "%s\n", line);
        free(line);
    }
    return 0;
}
