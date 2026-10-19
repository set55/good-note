#include <stdio.h>

int next_id(void)
{
    static int n = 0;   /* 只初始化一次，值留到下一次呼叫 */
    n = n + 1;
    return n;
}

int plain_id(void)
{
    int n = 0;          /* 每次呼叫都重新放在堆疊上、重新設成 0 */
    n = n + 1;
    return n;
}

int main(void)
{
    for (int i = 0; i < 3; i++)
        printf("next_id() = %d, plain_id() = %d\n", next_id(), plain_id());
    return 0;
}
