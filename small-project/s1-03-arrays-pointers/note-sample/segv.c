#include <stdio.h>

long sum(int *p, int n)
{
    long s = 0;
    for (int i = 0; i < n; i++)
        s += p[i];              /* 第 7 行 */
    return s;
}

int main(void)
{
    int a[10];
    for (int i = 0; i < 10; i++)
        a[i] = i;

    printf("sum(a, 10) = %ld\n", sum(a, 10));
    printf("sum(a, 1000000) = %ld\n", sum(a, 1000000));   /* 第 18 行：長度傳錯 */
    return 0;
}