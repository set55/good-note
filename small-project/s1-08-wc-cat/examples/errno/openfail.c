#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "用法：%s 檔名\n", argv[0]);
        return 2;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        int saved = errno;                       /* 先存起來：之後呼叫的函式可能改掉 errno */
        printf("fopen 失敗：errno = %d\n", saved);
        printf("strerror(errno) = \"%s\"\n", strerror(saved));
        errno = saved;
        perror(argv[1]);                         /* 印到 stderr：「<參數>: <strerror 的字串>」 */
        return 1;
    }

    int c = getc(fp);
    if (c == EOF && ferror(fp))                  /* EOF 有兩種原因：讀完了，或出錯了 */
        perror("getc");
    else
        printf("第一個 byte 讀到了：%d\n", c);
    fclose(fp);
    return 0;
}
