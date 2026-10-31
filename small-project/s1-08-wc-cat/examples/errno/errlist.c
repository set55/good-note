#define _GNU_SOURCE            /* strerrorname_np 是 GNU 擴充 */
#include <stdio.h>
#include <string.h>

int main(void)
{
    for (int e = 1; e < 200; e++) {
        const char *name = strerrorname_np(e);
        if (name != NULL)
            printf("%3d  %-16s %s\n", e, name, strerror(e));
    }
    return 0;
}
