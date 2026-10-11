#include <stdio.h>

int main(int argc, char *argv[])
{
    char empty[] = "";            /* 1 byte：只有 '\0' */
    char c11[11] = "0123456789";  /* 11 byte：10 個字元 + '\0' */
    int i = argc;                 /* 不帶參數執行時 argc = 1 */

    (void)argv;
    printf("%d\n", c11[0]);
    printf("%d\n", empty[i]);     /* 第 11 行：empty[1]，越界 1 格 */
    return 0;
}
