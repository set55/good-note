/* rawio.c：不用 stdio，直接呼叫 glibc 的系統呼叫包裝函式 */
#include <fcntl.h>       /* open、O_RDONLY */
#include <unistd.h>      /* read、write、close、syscall */
#include <sys/syscall.h> /* SYS_write */

int main(void)
{
    char buf[16];
    int fd = open("in.txt", O_RDONLY);          /* 包裝函式叫 open */
    ssize_t n = read(fd, buf, sizeof buf);
    close(fd);
    write(1, buf, n);                           /* 包裝函式 write */
    syscall(SYS_write, 1, "raw\n", 4);          /* 不靠專用包裝，直接給號碼 */
    return 0;
}
