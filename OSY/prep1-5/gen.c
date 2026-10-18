#include <stdio.h>

int main(int argc, char **argv)
{
    int count;
    if (argc != 2) {
        fprintf(stderr, "Pouziti: %s N\n", argv[0]);
        return 1;
    }
    if (sscanf(argv[1], "%d", &count) != 1 || count < 0) {
        fprintf(stderr, "N musi byt nezaporne cele cislo\n");
        return 1;
    }

    for (int i = 0; i < count; i++) {
        printf("%d\n", i + 1);
    }
    fflush(stdout);
    return 0;
}
