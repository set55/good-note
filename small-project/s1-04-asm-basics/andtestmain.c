/* andtestmain.c：同一個 n，比較 andl $1 與 testl $1 做完後的 %edi 和旗標 */
#include <stdio.h>

unsigned and_val(unsigned n);              /* 在 andtest.s */
unsigned test_val(unsigned n);
unsigned long and_flags(unsigned n);
unsigned long test_flags(unsigned n);

/* CF 第 0 位元、ZF 第 6、SF 第 7、OF 第 11 */
static void show(const char *op, unsigned n, unsigned edi, unsigned long f)
{
    printf("%-6s n=%u：做完後 %%edi = %u   ZF=%lu SF=%lu CF=%lu OF=%lu\n", op, n, edi,
           (f >> 6) & 1, (f >> 7) & 1, (f >> 0) & 1, (f >> 11) & 1);
}

int main(void)
{
    unsigned ns[] = { 4, 5 };

    for (int k = 0; k < 2; k++) {
        unsigned n = ns[k];
        show("andl", n, and_val(n), and_flags(n));
        show("testl", n, test_val(n), test_flags(n));
    }
    return 0;
}
