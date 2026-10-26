#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    static char buf[4096];
    buf[0] = 'x';                                    /* 寫一下，確保這頁真的配了實體頁 */
    uintptr_t va = (uintptr_t)buf;
    uint64_t e;
    int fd = open("/proc/self/pagemap", O_RDONLY);   /* 每個虛擬頁一筆 8 byte 的紀錄 */
    pread(fd, &e, sizeof e, (va / 4096) * 8);
    printf("虛擬位址 %#lx：present=%lu  PFN=%lu\n",
           (unsigned long)va, (unsigned long)(e >> 63), (unsigned long)(e & ((1ULL << 55) - 1)));
    close(fd);
    return 0;
}
