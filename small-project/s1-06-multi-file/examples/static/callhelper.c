int helper(void);       /* 想呼叫 a.c 裡的 static helper */

int main(void)
{
    return helper();
}
