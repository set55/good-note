#include <stdio.h>

int data[8] = {3, 1, 4, 1, 5, 9, 2, 6};

int *find(int *p, int target)
{
    int *start = p;             /* 只是為了在 gdb 裡算「走了多遠」 */
    while (*p != target)        /* 第 8 行 */
        p++;
    (void)start;                /* 告訴編譯器「我知道 start 沒被用到」，避免 -Wextra 的警告 */
    return p;
}

int main(void)
{
    printf("找 5：第 %td 個\n", find(data, 5) - data);
    printf("找 7：第 %td 個\n", find(data, 7) - data);   /* 第 17 行 */
    return 0;
}