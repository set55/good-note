#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include <locale.h>

int main() {
    // setlocale(LC_ALL, "");
    printf("sizeof('A'): %zu\n", sizeof('A'));
    printf("sizeof(\"A\"): %zu\n", sizeof("A"));
    printf("sizeof(\"字\"): %zu\n", sizeof("字"));

    printf("strlen(\"字\"): %zu\n", strlen("字"));
    printf("mbstowcs(NULL, \"字\", 0): %zu\n", mbstowcs(NULL, "字", 0));


    return 0;
}