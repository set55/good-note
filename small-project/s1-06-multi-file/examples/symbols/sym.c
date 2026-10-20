#include <stdio.h>

int counter = 3;          /* 全域變數，有初值 */
int buf[100];             /* 全域變數，沒有初值 */
extern int shared;        /* 只有宣告：定義在 other.c */

int twice(int x)          /* 函式定義 */
{
    return 2 * x;
}

int main(void)
{
    int local = 5;        /* 區域變數（放在堆疊上） */
    printf("%d\n", twice(local) + counter + buf[0] + shared);
    return 0;
}
