int printf(const char *fmt, ...);     /* 自己寫宣告，不 #include <stdio.h> */

int main(void)
{
    printf("hi %d\n", 42);
    return 0;
}
