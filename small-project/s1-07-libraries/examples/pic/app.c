#include <stdio.h>

int verbose = 3;
int get_verbose(void);

int main(void)
{
    printf("get_verbose() = %d\n", get_verbose());
    return 0;
}
