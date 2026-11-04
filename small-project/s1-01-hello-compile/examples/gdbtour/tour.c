/* tour.c：gdb 筆記〈依任務查〉用的範例程式 */
#include <stdio.h>

struct point {
    int x;
    int y;
};

int counter = 0;                          /* 全域變數：watch 的例子用 */

int square(int n)
{
    int r = n * n;
    return r;
}

int sum_squares(int *a, int len)
{
    int total = 0;
    for (int i = 0; i < len; i++) {
        total += square(a[i]);
        counter++;
    }
    return total;
}

int main(void)
{
    int nums[4] = {1, 2, 3, 4};
    struct point p = {3, 4};
    int s = sum_squares(nums, 4);
    printf("sum=%d p=(%d,%d) counter=%d\n", s, p.x, p.y, counter);
    return 0;
}
