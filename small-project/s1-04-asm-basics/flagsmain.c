/* flagsmain.c：對幾組 8 位元的數字做 cmp／add，印出結果與四個旗標 */
#include <stdio.h>

unsigned long flags_cmp(unsigned char d, unsigned char s);   /* 在 flags.s */
unsigned long flags_add(unsigned char d, unsigned char s);

/* rflags 裡四個旗標的位置：CF 第 0 位元、ZF 第 6、SF 第 7、OF 第 11 */
static void show(const char *op, unsigned char d, unsigned char s)
{
    unsigned long f;
    unsigned char r;

    if (op[0] == 'c') { f = flags_cmp(d, s); r = (unsigned char)(d - s); }
    else              { f = flags_add(d, s); r = (unsigned char)(d + s); }

    printf("%s D=0x%02x S=0x%02x -> 0x%02x | 無號 %3u %c %3u -> %3u | 有號 %4d %c %4d -> %4d | ZF=%lu SF=%lu CF=%lu OF=%lu\n",
           op, d, s, r,
           d, op[0] == 'c' ? '-' : '+', s, r,
           (signed char)d, op[0] == 'c' ? '-' : '+', (signed char)s, (signed char)r,
           (f >> 6) & 1, (f >> 7) & 1, (f >> 0) & 1, (f >> 11) & 1);
}

int main(void)
{
    show("cmp", 5, 5);        /* 相等 */
    show("cmp", 5, 3);        /* 一般的大減小 */
    show("cmp", 3, 5);        /* 小減大 */
    show("cmp", 0xff, 1);     /* 第五節實驗那組：-1 和 1 */
    show("cmp", 1, 0xff);     /* 反過來 */
    show("cmp", 0x80, 1);     /* -128 - 1 */
    show("add", 0x7f, 1);     /* 127 + 1 */
    show("add", 0xff, 1);     /* 255 + 1，也就是 -1 + 1 */
    show("add", 200, 100);    /* 200 + 100 */
    show("add", 0x80, 0x80);  /* -128 + -128 */
    return 0;
}
