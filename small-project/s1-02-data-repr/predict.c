#include <stdio.h>

int main() {
    printf("(signed char)200: %d\n", (signed char)200);
    
    printf("(unsigned short)-1: %u\n", (unsigned short)-1);
    
    printf("(int)(short)0x8000: %u,%x\n", (int)(short)0x8000, (int)(short)0x8000);

    printf("(unsigned int)(signed char)-100: %u,%x\n", (unsigned int)(signed char)-100, (unsigned int)(signed char)-100);

    printf("(int)(unsigned char)-100: %d\n", (int)(unsigned char)-100);
    return 0;
}