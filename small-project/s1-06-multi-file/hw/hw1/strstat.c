#include "strstat.h"

static int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

long count_digits(const char *s)
{
    long n = 0;
    for (; *s != '\0'; s++)
        if (is_digit(*s))
            n++;
    return n;
}

long length(const char *s)
{
    long n = 0;
    while (s[n] != '\0')
        n++;
    return n;
}