#include <stdio.h>

int counter = 100;
int bump(void);

int main(void)
{
    printf("bump() = %d\n", bump());
    printf("counter = %d\n", counter);
    return 0;
}
