#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Pouziti: %s even|odd\n", argv[0]);
        return 1;
    }

    int want_even;
    if (strcmp(argv[1], "even") == 0) {
        want_even = 1;
    } else if (strcmp(argv[1], "odd") == 0) {
        want_even = 0;
    } else {
        fprintf(stderr, "Neznamy filtr\n");
        return 1;
    }

    int number;
    while (scanf("%d", &number) == 1) {
        int is_even = number % 2 == 0;
        if (is_even == want_even) {
            printf("%d\n", number);
        }
    }
    fflush(stdout);
    return 0;
}
