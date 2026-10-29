#include <stdio.h>
#include <unistd.h>
int main(void)
{
    printf("hello\n");
    fflush(stdout);
    sleep(3);
    return 0;
}
