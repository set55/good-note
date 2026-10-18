/* float9.c：9 個 double 傳給 printf，第 9 個放不進 %xmm0～%xmm7 */
#include <stdio.h>

int main(void)
{
    printf("%.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f\n",
           1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    return 0;
}
