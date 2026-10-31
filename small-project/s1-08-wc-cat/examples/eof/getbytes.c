#include <stdio.h>

int main(void)
{
    int c;                 /* 一定要用 int：要裝得下 0～255 再加上 EOF */
    long n = 0;

    while ((c = getchar()) != EOF)
        n++;
    printf("讀到 %ld 個 byte，EOF = %d\n", n, EOF);
    return 0;
}
