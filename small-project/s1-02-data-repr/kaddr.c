#include <stdio.h>

int main(void)
{
    int local = 42;
    printf("local 的位址：%p\n", (void *)&local);
    printf("main  的位址：%p\n", (void *)main);

    int *k = (int *)0xffff888100000000;   /* 核心那一半的位址 */
    printf("準備讀 %p ...\n", (void *)k);
    fflush(stdout);
    printf("讀到 %d\n", *k);               /* 第 13 行 */
    return 0;
}
