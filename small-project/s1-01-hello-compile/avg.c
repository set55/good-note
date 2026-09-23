#include <stdio.h>

int average(int *a, int n)
{
    int sum = 0;                          /* 第 5 行 */
    for (int i = 0; i < n; i++)
        sum += a[i];
    int avg = sum / n;
    return avg;
}

int main(void)
{
    int a[] = {3, 6, 9};
    printf("%d\n", average(a, 3));        /* 第 15 行 */
    return 0;
}