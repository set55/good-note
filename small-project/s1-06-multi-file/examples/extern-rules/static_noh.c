void hit(void);         /* 不引入 hits.h，自己寫 hit 的宣告 */

static long hits = 0;   /* 只在本檔案有效 → LOCAL */

void hit(void)
{
    hits = hits + 1;
}
