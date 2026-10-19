#include "hits.h"

/* 這一版沒有自己寫 long hits = 0;，只靠標頭檔裡的 long hits; */

void hit(void)
{
    hits = hits + 1;
}
