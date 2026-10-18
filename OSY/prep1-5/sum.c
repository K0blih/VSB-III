#include <stdio.h>

int main(void)
{
    double number;
    double total = 0;

    while (scanf("%lf", &number) == 1) {
        total = total + number;
    }
    printf("%.15g\n", total);
    fflush(stdout);
    return 0;
}
