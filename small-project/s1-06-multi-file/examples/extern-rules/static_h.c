#include "hits.h"       /* 標頭檔說 extern long hits;（別的檔案看得到的 hits） */

static long hits = 0;   /* 又說 hits 只在本檔案有效 */

void hit(void)
{
    hits = hits + 1;
}
