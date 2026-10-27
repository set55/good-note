#include <stdio.h>

/* 用 8 位元有號數（-128～127）把所有 x、y 組合都試一次 */
int main(void)
{
    int total = 0, overflow = 0, differ = 0, differ_no_ovf = 0;
    for (int x = -128; x <= 127; x++)
        for (int y = -128; y <= 127; y++) {
            int math = x + y;                    /* 數學上的和（int 夠大，不會溢位） */
            signed char wrapped = (signed char)math;  /* 8 位元 CPU 實際留下的結果（回繞） */
            int ovf = (math < -128 || math > 127);
            int a = wrapped < x;                 /* 原本的檢查 x + y < x */
            int b = y < 0;                       /* 化簡後的 y < 0 */
            total++;
            overflow += ovf;
            if (a != b) {
                differ++;
                if (!ovf) differ_no_ovf++;
            }
        }
    printf("全部組合: %d\n", total);
    printf("會溢位的組合: %d\n", overflow);
    printf("x+y<x 與 y<0 結果不同的組合: %d\n", differ);
    printf("其中「沒溢位」卻不同的: %d\n", differ_no_ovf);
    return 0;
}
