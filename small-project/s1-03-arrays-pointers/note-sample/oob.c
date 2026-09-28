#include <stdio.h>

int main(void)
{
    int a[4] = {1, 2, 3, 4};

    /* 讀：a[4] 已經超出陣列 */
    printf("a[4] = %d\n", a[4]);            /* 第 8 行 */

    /* 寫：i <= 4 多寫了一格 */
    for (int i = 0; i <= 4; i++)
        a[i] = 99;                          /* 第 12 行 */

    printf("寫完了，a[0] = %d\n", a[0]);
    return 0;
}