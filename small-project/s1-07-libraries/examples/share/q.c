#include "slot.h"        /* 要放第一行：裡面的 _GNU_SOURCE 必須在任何 #include 之前 */
#include <stdio.h>
#include <unistd.h>

int main(void)                                   /* q 沒有定義 say */
{
    printf("q: 格子在 %p，呼叫前裝 %p\n", (void *)say_slot(), *say_slot());
    greet();
    printf("q: 呼叫後裝 %p\n", *say_slot());
    fflush(stdout);
    sleep(5);
    return 0;
}
