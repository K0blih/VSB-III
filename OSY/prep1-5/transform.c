#include <math.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Pouziti: %s square|sqrt|neg\n", argv[0]);
        return 1;
    }
    char *mode = argv[1];
    if (strcmp(mode, "square") != 0 && strcmp(mode, "sqrt") != 0 && strcmp(mode, "neg") != 0) {
        fprintf(stderr, "Neznama transformace\n");
        return 1;
    }

    double number;
    while (scanf("%lf", &number) == 1) {
        if (strcmp(mode, "square") == 0) {
            number = number * number;
        } else if (strcmp(mode, "sqrt") == 0) {
            number = sqrt(number);
        } else {
            number = -number;
        }
        printf("%.15g\n", number);
    }
    fflush(stdout);
    return 0;
}
