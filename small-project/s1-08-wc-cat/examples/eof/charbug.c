#include <stdio.h>

int main(void)
{
    char c;                /* 錯誤示範：用 char 裝 getchar() 的回傳值 */
    long n = 0;

    while ((c = getchar()) != EOF)
        n++;
    printf("讀到 %ld 個 byte\n", n);
    return 0;
}
