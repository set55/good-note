/* hexesc.c：C 字串裡的 \x 會把後面「所有」十六進位數字都當成同一個跳脫 */
#include <stdio.h>

int main(void)
{
    const char s[] = "\x421";          /* 想要 0x42 '1'，C 卻讀成 \x421 一整個 */
    const char t[] = "\x42" "1";       /* 拆成兩個字串字面值，編譯器再接起來 */

    printf("sizeof s = %zu, s[0] = 0x%02x\n", sizeof s, (unsigned char)s[0]);
    printf("sizeof t = %zu, t = \"%s\"\n", sizeof t, t);
    return 0;
}
