#include <stdio.h>
#include <unistd.h>

int hit(void);

int main(void)
{
    int r = 0;
    for (int i = 0; i < 2; i++)               /* x 呼叫 2 次 */
        r = hit();
    printf("x: &hit=%p  hit() 回傳 %d\n", (void *)hit, r);
    fflush(stdout);
    sleep(5);
    return 0;
}
