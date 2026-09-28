#include <stdio.h>
#include <stddef.h>

int main(void)
{
    int a[4] = {10, 20, 30, 40};
    int *p = a;                 /* 等於 &a[0] */
    unsigned char *c = (unsigned char *)a;
    double d[2] = {1.5, 2.5};
    double *q = d;

    printf("p     = %p   p + 1 = %p\n", (void *)p, (void *)(p + 1));
    printf("c     = %p   c + 1 = %p\n", (void *)c, (void *)(c + 1));
    printf("q     = %p   q + 1 = %p\n", (void *)q, (void *)(q + 1));

    printf("*(p + 2) = %d   p[2] = %d   2[p] = %d\n", *(p + 2), p[2], 2[p]);

    int *first = &a[0];
    int *last = &a[3];
    ptrdiff_t n = last - first;
    ptrdiff_t back = first - last;
    printf("last - first = %td   first - last = %td\n", n, back);
    printf("以 byte 算：%td\n", (unsigned char *)last - (unsigned char *)first);
    printf("sizeof(ptrdiff_t) = %zu\n", sizeof(ptrdiff_t));
    return 0;
}