#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    FILE *bad = fopen("nofile.txt", "r");      /* 失敗：設 errno */
    printf("失敗之後：bad = %p, errno = %d\n", (void *)bad, errno);

    FILE *good = fopen("ok.txt", "r");         /* 成功：不會把 errno 清成 0 */
    printf("成功之後：good = %p, errno = %d (%s)\n", (void *)good, errno, strerror(errno));

    errno = 0;                                 /* 要拿 errno 判斷，得自己先清 */
    FILE *good2 = fopen("ok.txt", "r");
    printf("先清 0 再成功：errno = %d\n", errno);

    fclose(good);
    fclose(good2);
    return 0;
}
