#include <stdio.h>

int g = 1;

int *where(void)
{
    return &g;          /* 拿全域變數的位址 */
}

int main(void)
{
    printf("&g   = %p\n", (void *)where());
    printf("main = %p\n", (void *)main);
    return 0;
}
