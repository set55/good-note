#include <stdio.h>
#include <limits.h>

int will_overflow(int x, int y)
{
    int sum = x + y;          /* 第 6 行 */
    return sum < x;           /* 加了正數反而變小，就代表溢位了？ */
}

int main(void)
{
    printf("will_overflow(INT_MAX, 1): %d\n", will_overflow(INT_MAX, 1));
    return 0;
}
