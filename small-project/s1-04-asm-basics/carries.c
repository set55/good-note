/* carries.c：8 位元加法逐位列出「每一位收到的進位」，對照 CF 與 OF */
#include <stdio.h>

static void bits8(unsigned v)
{
    for (int i = 7; i >= 0; i--)
        putchar((v >> i) & 1 ? '1' : '0');
}

static void add8(unsigned char a, unsigned char b)
{
    unsigned sum = (unsigned)a + b;        /* 9 位元的完整和，第 8 位元就是最高位往外的進位 */
    unsigned cin = (a ^ b ^ sum) & 0xff;   /* 第 i 位元 = 第 i 位收到的進位（從第 i-1 位進來的） */
    unsigned c_into7 = (cin >> 7) & 1;     /* 進到第 7 位（最高位）的進位 */
    unsigned c_out7  = (sum >> 8) & 1;     /* 從第 7 位往外的進位 = CF */

    printf("0x%02x + 0x%02x\n", a, b);
    printf("  收到的進位 "); bits8(cin);        printf("\n");
    printf("  a          "); bits8(a);          printf("\n");
    printf("  b          "); bits8(b);          printf("\n");
    printf("  留下 8 位  "); bits8(sum & 0xff); printf("   往外的進位=%u\n", c_out7);
    printf("  進到第 7 位=%u  第 7 位往外=%u  -> CF=%u  OF=%u\n\n",
           c_into7, c_out7, c_out7, c_into7 ^ c_out7);
}

int main(void)
{
    add8(0x01, 0x7f);   /* 使用者問的這組 */
    add8(0xff, 0x01);   /* 筆記⑧：255 + 1 */
    add8(0x80, 0x80);   /* 筆記⑩：-128 + -128 */
    add8(0x05, 0x03);   /* 沒有任何裝不下 */
    return 0;
}
