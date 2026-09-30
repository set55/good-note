#include <stdio.h>

int data[8] = {3, 1, 4, 1, 5, 9, 2, 6};

int *find(int *p, int target, int len)
{
    int *start = p;             /* 只是為了在 gdb 裡算「走了多遠」 */
    while (len-- > 0 && *p != target)        /* 第 8 行 */
        p++;
    (void)start;
    if (*p != target)          /* 告訴編譯器「我知道 start 沒被用到」，避免 -Wextra 的警告 */
        p = NULL;
    return p;
}

int main(void)
{
    int *p;
    p = find(data, 5, sizeof data / sizeof data[0]);
    if (p != NULL)
        printf("找 5：第 %td 個\n", p - data);
    else
        printf("找 5：未找到\n");

    p = find(data, 7, sizeof data / sizeof data[0]);
    if (p != NULL)
        printf("找 7：第 %td 個\n", p - data);
    else
        printf("找 7：未找到\n");
    return 0;
}