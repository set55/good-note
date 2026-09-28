#include <stdio.h>

int main(void)
{
    char buf[8];
    int i;

    printf("buf 在 %p，i 在 %p\n", (void *)buf, (void *)&i);

    /* 往 8 byte 的陣列寫 32 個 byte */
    for (i = 0; i < 32; i++)
        buf[i] = 'A';                       /* 第 12 行 */

    printf("迴圈結束時 i = %d\n", i);
    return 0;                               /* 第 15 行 */
}                          