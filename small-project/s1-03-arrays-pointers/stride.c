#include <stdio.h>

int main()
{
    short s[6] = {1, 2, 3, 4, 5, 6};
    long l[3] = {10, 20, 30};

    printf("s address = %p\n", (void *)s);
    printf("s+3 address = %p\n", (void *)(s + 3));
    printf("l address = %p\n", (void *)l);
    printf("l+2 address = %p\n", (void *)(l + 2));
    printf("&s + 1 address = %p\n", (void *)(&s + 1));

    printf("&s[5] - &s[1] = %td\n", &s[5] - &s[1]);
    printf("(char *)(l + 2) - (char *)l = %td\n", (char *)(l + 2) - (char *)l);
    return 0;
}