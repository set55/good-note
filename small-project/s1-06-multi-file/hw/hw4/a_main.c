#include <stdio.h>
#include "clamp.h"

int set_volume(int v);

int main(void)
{
    printf("%d %d\n", set_volume(150), clamp(-5, 0, 10));
    return 0;
}
