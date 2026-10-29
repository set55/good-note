#include <stdio.h>
#include <unistd.h>
int main(void)
{
    printf("hello\n");
    /* 沒有 fflush */
    sleep(3);
    return 0;
}
