#include "hits.h"

long hits = 0;          /* 定義：真正配置 8 byte 的地方，整個程式只能有一個 */

void hit(void)
{
    hits = hits + 1;
}
