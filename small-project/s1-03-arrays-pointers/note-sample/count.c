#include <stdio.h>

int main(void)
{
    int a[5] = {1, 2, 3, 4, 5};
    int total = 0;

    for (int i = 0; i < sizeof a / sizeof a[0]; i++)   /* 第 8 行 */
        total += a[i];
    printf("total = %d\n", total);
    return 0;
}
