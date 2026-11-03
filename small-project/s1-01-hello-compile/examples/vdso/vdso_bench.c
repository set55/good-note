#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>

#define N 1000000

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(void)
{
    struct timespec ts;
    double t0 = now();
    for (int i = 0; i < N; i++)
        clock_gettime(CLOCK_MONOTONIC, &ts);
    double t1 = now();
    for (int i = 0; i < N; i++)
        syscall(SYS_clock_gettime, CLOCK_MONOTONIC, &ts);
    double t2 = now();
    printf("vDSO   : %.1f ns/次\n", (t1 - t0) / N * 1e9);
    printf("syscall: %.1f ns/次\n", (t2 - t1) / N * 1e9);
    return 0;
}
