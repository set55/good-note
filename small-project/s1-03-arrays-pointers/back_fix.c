#include <stdio.h>

int main()
{
    int a[5] = {1, 2, 3, 4, 5};
    for (size_t i = sizeof a / sizeof a[0] - 1; i >= 0 && i < sizeof a / sizeof a[0]; i--)
        printf("%d\n", a[i]);
    return 0;
}