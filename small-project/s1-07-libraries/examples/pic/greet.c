#include <stdio.h>

void say(void)
{
    printf("say() in libgreet.so\n");
}

void greet(void)
{
    say();
}
