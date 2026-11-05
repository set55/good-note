#include <stdio.h>
#include <string.h>

int main(void)
{
    char dst[16];
    volatile size_t n = 6;          /* volatile：不讓 gcc 把 memcpy 直接展開成 mov */
    memcpy(dst, "hello", n);
    puts(dst);
    return 0;
}
