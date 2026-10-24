#include <stdio.h>

/* 把 1 byte 的每一位拆開，用兩種權重各算一次 */
static void show(unsigned char b)
{
    int as_unsigned = 0, as_signed = 0;

    printf("0x%02x = ", b);
    for (int i = 7; i >= 0; i--)
        putchar((b >> i) & 1 ? '1' : '0');

    for (int i = 0; i <= 7; i++) {
        if ((b >> i) & 1) {
            as_unsigned += 1 << i;                       /* 第 i 位的權重：+2^i */
            as_signed   += (i == 7) ? -(1 << 7) : (1 << i); /* 二補數：只有第 7 位是 -2^7 */
        }
    }
    printf("  無號權重和 = %4d  二補數權重和 = %4d  (signed char) = %4d\n",
           as_unsigned, as_signed, (signed char)b);
}

int main(void)
{
    unsigned char samples[] = { 0x01, 0x05, 0x7f, 0x80, 0x81, 0xfb, 0xff };

    printf("bit:   76543210   ← 位元編號，從 0 數起\n");
    for (unsigned i = 0; i < sizeof samples; i++)
        show(samples[i]);
    return 0;
}
