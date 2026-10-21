#include "strstat.h"
#include <stdio.h>

int main(void)
{
    const char *msg = "room 101, floor 3";
    printf("length = %ld, digits = %ld\n", length(msg), count_digits(msg));
    return 0;
}   