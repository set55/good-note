#include <stdio.h>
#include "temp.h"

int main(void)
{
    printf("%d %d\n", f_to_c(100), c_to_f(100));
    printf("temp_calls = %d\n", temp_calls);
    return 0;
}
