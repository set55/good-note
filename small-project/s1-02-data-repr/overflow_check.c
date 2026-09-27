#include <stdio.h>
#include <limits.h>

int will_overflow(int x, int y)
{
    int sum = x + y;
    return sum < x;      /* 加了正數反而變小，就代表溢位了？ */
}


int main() {
    printf("will_overflow(INT_MAX, 1): %d\n", will_overflow(INT_MAX, 1));
}