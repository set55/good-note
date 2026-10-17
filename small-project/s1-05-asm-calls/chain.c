#include <stdio.h>

int inner(int y)
{
    return y * 3;
}

int outer(int x)
{
    return inner(x + 1) * 2;
}

int main(void)
{
    printf("%d\n", outer(4));
    return 0;
}