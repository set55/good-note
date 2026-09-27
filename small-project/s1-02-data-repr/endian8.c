#include <stdio.h>


int main() {
    long v = 0x1122334455667788;
    unsigned char *p = (unsigned char *)&v;
    printf("%02x\n", *p);
    return 0;
}