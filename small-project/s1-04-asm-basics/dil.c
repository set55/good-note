#include <stdio.h>

__attribute__((noinline)) int widen_char(signed char c) { return c; }

int main(void)
{
    printf("%d\n", widen_char(-1));
    return 0;
}