/* main.c */
#include <stdio.h>

int g = 5;


char *trim(char *s);

int main(void)
{
    char buf[] = "   hello";
    printf("[%s]\n", trim(buf));
    return 0;
}
