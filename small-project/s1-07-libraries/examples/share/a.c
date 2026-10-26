#include <stdio.h>
#include <unistd.h>

int bump(void);

int main(void)
{
    int r = bump();                           /* a 只呼叫 1 次 */
    printf("a: pid=%d  &bump=%p  bump() 回傳 %d\n", getpid(), (void *)bump, r);
    fflush(stdout);
    sleep(5);                                 /* 停 5 秒，讓我們看 /proc/<pid>/ */
    return 0;
}
