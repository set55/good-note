#include <stdio.h>
long running_total(long x)
{
    static long total = 0;
    total += x;
    return total;
}

int calls(void)
{
    static int count = 100;
    count++;
    return count - 100;
}

int main(void)
{
    printf("running_total(5): %ld\n", running_total(5));
    printf("running_total(10): %ld\n", running_total(10));
    printf("running_total(-3): %ld\n", running_total(-3));

    printf("calls(): %d\n", calls());
    printf("calls(): %d\n", calls());
    printf("calls(): %d\n", calls());
    return 0;
}