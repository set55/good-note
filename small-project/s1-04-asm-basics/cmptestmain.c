/* cmptestmain.c：模仿 sum_to 的兩個判斷——cmpl 比 i 和 n、testl 看 n 的正負 */
#include <stdio.h>

unsigned long flags_cmpl(int i, int n);   /* 在 cmptest.s */
unsigned long flags_testl(int n);

/* CF 第 0 位元、ZF 第 6、SF 第 7、OF 第 11 */
static void show(const char *what, unsigned long f)
{
    printf("%-22s ZF=%lu SF=%lu CF=%lu OF=%lu\n", what,
           (f >> 6) & 1, (f >> 7) & 1, (f >> 0) & 1, (f >> 11) & 1);
}

int main(void)
{
    char buf[64];
    int pairs[][2] = { {3, 5}, {5, 5}, {6, 5} };   /* {i, n} */
    int ns[] = { 5, 0, -3 };

    for (int k = 0; k < 3; k++) {
        int i = pairs[k][0], n = pairs[k][1];
        snprintf(buf, sizeof buf, "cmpl: i=%d, n=%d", i, n);
        show(buf, flags_cmpl(i, n));
    }
    for (int k = 0; k < 3; k++) {
        snprintf(buf, sizeof buf, "testl: n=%d", ns[k]);
        show(buf, flags_testl(ns[k]));
    }
    return 0;
}
