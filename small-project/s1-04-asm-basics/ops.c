long widen_s(int x)       { return x; }
long widen_u(unsigned x)  { return x; }
int  get(int *a, long i)  { return a[i]; }
int *addr(int *a, long i) { return &a[i]; }
long times3(long x)       { return x * 3; }
int  lt_s(int a, int b)           { return a < b; }
int  lt_u(unsigned a, unsigned b) { return a < b; }