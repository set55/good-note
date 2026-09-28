#include <stdio.h>
#include <string.h>

int main(void)
{
    char s1[] = "hi";       /* 陣列：在堆疊上有自己的 3 個 byte */
    char *s2 = "hi";        /* 指標：指向字串常數 */

    printf("sizeof s1 = %zu, strlen(s1) = %zu\n", sizeof s1, strlen(s1));
    printf("sizeof s2 = %zu, strlen(s2) = %zu\n", sizeof s2, strlen(s2));
    printf("s1 在 %p\n", (void *)s1);
    printf("s2 指向 %p\n", (void *)s2);

    s1[0] = 'H';
    printf("改完 s1 = %s\n", s1);

    s2[0] = 'H';            /* 第 17 行：寫進唯讀記憶體 */
    printf("改完 s2 = %s\n", s2);
    return 0;
}
