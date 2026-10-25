/* ofrule.c：窮舉全部 256×256 組 8 位元加法，比較「有號溢位」和「進第 7 位 XOR 出第 7 位」 */
#include <stdio.h>

int main(void)
{
    int mismatch = 0;
    int seen[2][2][2] = {0};     /* [a 的第 7 位][b 的第 7 位][進到第 7 位的進位]，記錄 OF */

    for (int a = 0; a < 256; a++)
        for (int b = 0; b < 256; b++) {
            unsigned sum = (unsigned)a + (unsigned)b;
            unsigned cin  = ((a ^ b ^ sum) >> 7) & 1;   /* 進到第 7 位的進位 */
            unsigned cout = (sum >> 8) & 1;             /* 第 7 位往外的進位 */
            int real = (signed char)a + (signed char)b; /* 有號的正確答案，用 int 算不會溢位 */
            int of_math = real < -128 || real > 127;    /* 定義：當有號數，8 位元裝不下 */
            int of_carry = cin ^ cout;                  /* 規則：進來的跟出去的不一樣 */
            if (of_math != of_carry)
                mismatch++;
            seen[a >> 7][b >> 7][cin] = of_math + 1;    /* 存 1 或 2，0 代表沒出現過 */
        }

    printf("兩種算法不一致的組數：%d / 65536\n\n", mismatch);
    printf("a7 b7 進第7位 | 第7位留下 出第7位 | OF\n");
    for (int a7 = 0; a7 < 2; a7++)
        for (int b7 = 0; b7 < 2; b7++)
            for (int c = 0; c < 2; c++) {
                int t = a7 + b7 + c;
                printf(" %d  %d     %d     |     %d        %d     |  %d\n",
                       a7, b7, c, t & 1, t >> 1, seen[a7][b7][c] - 1);
            }
    return 0;
}
