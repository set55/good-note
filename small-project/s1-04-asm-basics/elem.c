int   get_short(short *a, long i)         { return a[i]; }
int   get_uchar(unsigned char *a, long i) { return a[i]; }
long  get_long2(long *a, long i)          { return a[i + 2]; }
long *next_long(long *a, long i)          { return &a[i + 1]; }