#include <stdio.h>
#include "hits.h"

long hits = 5;          /* 第二份「有初值」的定義：hits.c 已經有 long hits = 0; */

int main(void)
{
    hit();
    printf("hits = %ld\n", hits);
    return 0;
}
