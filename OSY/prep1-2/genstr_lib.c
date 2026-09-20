#include "genstr_lib.h"

#include <stdint.h>
#include <stdlib.h>

#define MAX_WORD_LENGTH 10

char *generate_line(int max_words)
{
    static const char alphabet[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    if (max_words <= 0) {
        return NULL;
    }

    size_t words = (size_t)(rand() % max_words) + 1;
    /* Na slovo nejvyse 10 pismen a mezera; posledni mezeru nahradi '\0'. */
    if (words > SIZE_MAX / (MAX_WORD_LENGTH + 1)) {
        return NULL;
    }
    char *line = malloc(words * (MAX_WORD_LENGTH + 1));
    if (line == NULL) {
        return NULL;
    }

    size_t pos = 0;
    for (size_t word = 0; word < words; ++word) {
        int length = rand() % MAX_WORD_LENGTH + 1;
        for (int letter = 0; letter < length; ++letter) {
            line[pos++] = alphabet[rand() % (sizeof alphabet - 1)];
        }
        if (word + 1 < words) {
            line[pos++] = ' ';
        }
    }
    line[pos] = '\0';
    return line;
}
