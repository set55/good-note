#include "hits.h"

/* 自我測驗 4-3：只把標頭檔改成 extern，卻沒有任何 .c 寫定義 */

void hit(void)
{
    hits = hits + 1;
}
