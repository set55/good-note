#include <stdio.h>

/* 用法：./countch 檔名 字元 —— 數這個檔案裡有幾個該字元 */
int main(int argc, char *argv[])
{
    if (argc != 3 || argv[2][0] == '\0' || argv[2][1] != '\0') {
        fprintf(stderr, "用法：%s 檔名 單一字元\n", argv[0]);
        return 2;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror(argv[1]);
        return 1;
    }

    int target = (unsigned char)argv[2][0];
    long hits = 0;
    int c;
    while ((c = getc(fp)) != EOF)
        if (c == target)
            hits++;

    if (ferror(fp)) {                 /* 迴圈停下來，是讀完了還是出錯？ */
        perror(argv[1]);
        fclose(fp);
        return 1;
    }
    fclose(fp);

    printf("%ld %s\n", hits, argv[1]);
    return 0;
}
