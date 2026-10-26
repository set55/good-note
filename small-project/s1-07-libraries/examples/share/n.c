#include <stdio.h>
#include <unistd.h>

long calc(long w, long h);

int main(void)
{
    printf("n: calc(5, 6) = %ld\n", calc(5, 6));
    fflush(stdout);
    sleep(5);
    return 0;
}
