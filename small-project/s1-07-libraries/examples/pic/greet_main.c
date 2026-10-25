#include <stdio.h>

void greet(void);

void say(void)
{
    printf("say() in main\n");
}

int main(void)
{
    greet();
    return 0;
}
