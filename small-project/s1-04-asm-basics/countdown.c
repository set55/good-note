/* countdown.c：8 位元的計數器從 3 往回倒數 5 下，看它怎麼「翻過 0」 */
#include <stdio.h>

int main(void)
{
    unsigned char x = 3;            /* 8 位元，只能裝 0～255 */

    printf("開始：%3u (0x%02x)\n", x, x);
    for (int i = 1; i <= 5; i++) {
        x = x - 1;                  /* 每次減 1，總共減 5，就是 3 − 5 */
        printf("減第 %d 下：%3u (0x%02x)\n", i, x, x);
    }
    return 0;
}
