#include <stdio.h>
#include <limits.h>

int safe_will_overflow(int x, int y)
{
    if (x > 0 && y > INT_MAX - x)
        return 1;  /* 溢位 */
    return 0;      /* 不溢位 */
}


int main() {
    printf("safe_will_overflow(INT_MAX, 1): %d\n", safe_will_overflow(INT_MAX, 1));
}