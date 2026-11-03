#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>

int main(void)
{
    struct timespec ts;
    for (int i = 0; i < 3; i++)
        clock_gettime(CLOCK_MONOTONIC, &ts);       /* 經 vDSO */
    for (int i = 0; i < 3; i++)
        syscall(SYS_clock_gettime, CLOCK_MONOTONIC, &ts); /* 強制真的系統呼叫 */
    printf("%ld.%09ld\n", (long)ts.tv_sec, ts.tv_nsec);
    return 0;
}
