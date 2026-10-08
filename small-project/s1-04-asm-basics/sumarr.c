long sum_arr(long *a, long n)
{
    long t = 0;
    for (long i = 0; i < n; i++)
        t += a[i];
    return t;
}