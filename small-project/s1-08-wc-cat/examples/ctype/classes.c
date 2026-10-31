#include <ctype.h>
#include <stdio.h>

/* 把 0～255 分成三類：空白、可印（不含空白）、其他；印出每一類的範圍 */
int main(void)
{
    const char *name[] = { "空白（isspace）", "可印、不是空白（isprint 且非 isspace）", "其他（兩者都不是）" };
    int prev = -1, start = 0;

    for (int c = 0; c <= 256; c++) {
        int kind = -1;
        if (c <= 255)
            kind = isspace(c) ? 0 : isprint(c) ? 1 : 2;
        if (kind != prev) {
            if (prev != -1)
                printf("0x%02x～0x%02x  %s\n", start, c - 1, name[prev]);
            prev = kind;
            start = c;
        }
    }
    return 0;
}
