#include <stdio.h>
#include "mathx.h"

int main(void)
{
    long r = square(100000);
    printf("%ld\n", r);
    return 0;
}
