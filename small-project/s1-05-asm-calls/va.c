#include <stdio.h>

void showd(double x)
{
    printf("%f\n", x);
}

int main(void)
{
    printf("%d %f %d %f %f\n", 1, 0.5, 2, 1.5, 2.5);
    showd(0.25);
    printf("%f %f %f %f %f %f %f %f %f\n",
           1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    return 0;
}