/* savemsg.c：把第二個參數寫進第一個參數指定的檔案（檔案不存在就建立、存在就清空） */
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "用法：%s 檔名 訊息\n", argv[0]);
        return 2;
    }
    FILE *fp = fopen(argv[1], "w");
    if (fp == NULL) {
        perror(argv[1]);
        return 1;
    }
    fprintf(fp, "%s\n", argv[2]);
    fclose(fp);
    return 0;
}
