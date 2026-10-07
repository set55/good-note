/* borrow.c：用直式減法一個位元一個位元地算 8 位元的 D − S，跟 CPU 的 cmpb 對照 */
#include <stdio.h>

unsigned long flags_cmp(unsigned char d, unsigned char s);   /* 在 flags.s */

static void print_bits(unsigned char x)
{
    for (int i = 7; i >= 0; i--)
        putchar(((x >> i) & 1) ? '1' : '0');
}

static void sub_by_hand(unsigned char d, unsigned char s)
{
    unsigned char r = 0;
    int borrow = 0;                       /* 從右邊那一位借走的 1 */

    printf("D = %3u = ", d); print_bits(d); putchar('\n');
    printf("S = %3u = ", s); print_bits(s); putchar('\n');
    printf("位元  D  S  借入  這一位的算式        借出\n");
    for (int i = 0; i < 8; i++) {         /* 從最右邊（第 0 位元）往左做 */
        int db = (d >> i) & 1, sb = (s >> i) & 1;
        int v = db - sb - borrow;         /* 這一位要算的：D 位 − S 位 − 被借走的 */
        int out = 0;
        if (v < 0) { v += 2; out = 1; }   /* 不夠減：向左邊那一位借 1（＝這一位的 2） */
        printf("  %d   %d  %d   %d    %d - %d - %d %s= %d    %d\n",
               i, db, sb, borrow, db, sb, borrow,
               out ? "+ 2 " : "    ", v, out);
        r |= (unsigned char)(v << i);
        borrow = out;
    }
    printf("手算結果 = "); print_bits(r);
    printf(" = 0x%02x，最高位元還要往外借：%d\n", r, borrow);

    unsigned sum = d + (unsigned char)~s + 1;   /* 加法器的做法：D + (S 反轉) + 1 */
    printf("加法器：%u + %u + 1 = %u，%s 255 → 往外進位 %d\n",
           d, (unsigned char)~s, sum, sum > 255 ? "超過" : "沒超過", sum > 255);

    unsigned long f = flags_cmp(d, s);
    printf("CPU cmpb：結果 0x%02x，CF = %lu\n\n", (unsigned char)(d - s), f & 1);
}

int main(void)
{
    sub_by_hand(3, 5);
    sub_by_hand(5, 3);
    return 0;
}
