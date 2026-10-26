#include <stdio.h>
#include <unistd.h>

int bump(void);

int main(void)
{
    int r = 0;
    for (int i = 0; i < 3; i++)               /* b 呼叫 3 次 */
        r = bump();
    printf("b: pid=%d  &bump=%p  bump() 回傳 %d\n", getpid(), (void *)bump, r);
    fflush(stdout);
    sleep(5);
    return 0;
}
