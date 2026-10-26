#define _GNU_SOURCE
#include <dlfcn.h>

void greet(void);

/* 回傳 libgreet.so 裡 .got.plt 中 say 那一格的位址：載入基底 + 0x4008（readelf -r 查到的） */
static void **say_slot(void)
{
    Dl_info info;
    dladdr((void *)greet, &info);          /* dli_fbase = libgreet.so 的載入基底 */
    return (void **)((char *)info.dli_fbase + 0x4008);
}
