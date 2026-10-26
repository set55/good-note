#include <stdio.h>

extern int level;
int get_level(void);

int main(void)
{
    printf("一開始 level = %d\n", level);
    level = 7;
    printf("get_level() = %d\n", get_level());
    return 0;
}
