#include "strcheck_lib.h"

size_t check_line(const char *line)
{
    size_t count = 0;
    for (const char *p = line; *p != '\0'; ++p) {
        if (*p >= 'A' && *p <= 'Z') {
            ++count;
        }
    }
    return count;
}
