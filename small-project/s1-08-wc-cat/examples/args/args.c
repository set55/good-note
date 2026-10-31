#include <stdio.h>

int main(int argc, char *argv[])
{
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++)
        printf("argv[%d] = \"%s\"\n", i, argv[i]);
    printf("argv[%d] = %p\n", argc, (void *)argv[argc]);   /* 最後一格固定是 NULL */
    return 0;
}
