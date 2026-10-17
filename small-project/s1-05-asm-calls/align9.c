/* align9.c：sum9 和 call9 放在同一個檔案（作業 2-5 的情境） */
int sum9(int a, int b, int c, int d, int e, int f, int g, int h, int i)
{
    return a + b + c + d + e + f + g + h + i;
}

int call9(void)
{
    return sum9(10, 20, 30, 40, 50, 60, 70, 80, 90);
}
