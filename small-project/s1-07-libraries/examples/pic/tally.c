int seen = 0;
long total = 0;

void add_sample(long x)
{
    seen = seen + 1;
    total = total + x;
}
