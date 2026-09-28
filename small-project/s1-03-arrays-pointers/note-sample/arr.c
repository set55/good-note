#include <stdio.h>

int main(void)
{
    int a[5] = {10, 20, 30, 40, 50};

    printf("sizeof a    = %zu\n", sizeof a);
    printf("sizeof a[0] = %zu\n", sizeof a[0]);
    printf("元素個數    = %zu\n", sizeof a / sizeof a[0]);
    for (int i = 0; i < 5; i++)
        printf("a[%d] = %d  位址 %p\n", i, a[i], (void *)&a[i]);
    return 0;
}