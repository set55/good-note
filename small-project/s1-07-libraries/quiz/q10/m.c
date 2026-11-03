/* m.c */
#include <stdio.h>
#include <unistd.h>

long calc(long w, long h);

int main(void)
{
    printf("m: calc(3, 4) = %ld\n", calc(3, 4));
    fflush(stdout);
    sleep(5);
    return 0;
}
