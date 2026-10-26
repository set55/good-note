#include <stdio.h>

extern int counter;        /* 只宣告：counter 定義在 libcount.so 裡 */
int bump(void);

int main(void)
{
    printf("bump() = %d\n", bump());
    printf("counter = %d\n", counter);
    printf("&counter = %p\n", (void *)&counter);
    return 0;
}
