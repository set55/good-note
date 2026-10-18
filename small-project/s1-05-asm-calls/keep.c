long g(long x);

long h(long a, long b)
{
    long x = g(a);
    long y = g(b);
    return x + y;
}