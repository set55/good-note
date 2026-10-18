#include <stdio.h>
// #include <limits.h>

long sum8(long a, long b, long c, long d, long e, long f, long g, long h)
{
    return a + b + c + d + e + f + g + h;
}

long call8(void)
{
    return sum8(1, 2, 3, 4, 5, 6, 7, 8);
}

int main() {
    printf("%ld\n", call8());
    return 0;
}