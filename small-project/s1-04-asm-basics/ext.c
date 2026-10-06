#include <stdio.h>

__attribute__((noinline)) int widen_char(signed char c)    { return c; }
__attribute__((noinline)) int widen_uchar(unsigned char c) { return c; }

int main(void)
{
    printf("%d %d\n", widen_char(-1), widen_uchar(255));
    return 0;
}
