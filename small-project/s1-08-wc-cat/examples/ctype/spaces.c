#include <ctype.h>
#include <stdio.h>

int main(void)
{
    for (int c = 0; c <= 255; c++)          /* isspace 只能吃 0～255 或 EOF */
        if (isspace(c))
            printf("%3d  0x%02x\n", c, c);
    return 0;
}
