#include <stdio.h>
#include <limits.h>

__attribute__((noinline)) int naive(int x, int y)
{
    int sum = x + y;
    return sum < x;                         /* 只在 y >= 0 時才是溢位檢查 */
}

__attribute__((noinline)) int builtin(int x, int y)
{
    int sum;
    return __builtin_add_overflow(x, y, &sum);   /* 真的溢位才回傳 1 */
}

int main(void)
{
    int xs[] = { 5, INT_MAX, INT_MIN };
    int ys[] = { -2, 1, -1 };
    for (int i = 0; i < 3; i++)
        printf("x=%d y=%d  naive=%d  builtin=%d\n",
               xs[i], ys[i], naive(xs[i], ys[i]), builtin(xs[i], ys[i]));
    return 0;
}
