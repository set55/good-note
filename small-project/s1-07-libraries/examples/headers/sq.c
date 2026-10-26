#include <math.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    (void)argv;
    printf("%f\n", sqrt((double)argc + 1.0));
    return 0;
}
