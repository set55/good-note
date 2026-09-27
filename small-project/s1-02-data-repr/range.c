#include <stdio.h>
#include <limits.h>

int main() {
    printf("%ld\n", sizeof(short));
    printf("%d\n", SHRT_MIN);
    printf("%d\n", SHRT_MAX);
    
    printf("%ld\n", sizeof(unsigned short));
    printf("%d\n", USHRT_MAX);
    
    printf("%ld\n", sizeof(signed char));
    printf("%d\n", SCHAR_MIN);
    printf("%d\n", SCHAR_MAX);
   
    printf("%ld\n", sizeof(unsigned char));
    printf("%d\n", UCHAR_MAX);
    return 0;
}