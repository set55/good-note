int g = 1;              /* 自己這個檔定義的全域變數 */
extern int ext;         /* 別的檔定義的全域變數 */

int helper(void) { return 2; }

int use(void)
{
    return g + ext + helper();
}
