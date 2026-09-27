#include <stdio.h>

int main() {
    unsigned int a = 3;
    unsigned int b = 5;
    if (a - b > 0)
        printf("a - b is greater than 0\n");
    else
        printf("a - b is not greater than 0\n");
    printf("a - b: %u\n", a - b);

    int i = -1;
    if (i < sizeof(int))
        printf("i is less than the size of int\n");
    else
        printf("i is not less than the size of int\n");
    return 0;
}