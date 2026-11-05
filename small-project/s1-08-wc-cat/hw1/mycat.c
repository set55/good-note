#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
    // printf("總共有 %d 個參數：\n", argc);

    // for (int i = 0; i < argc; i++) {
    //     printf("argv[%d] = %s\n", i, argv[i]);
    // }
    int c;

    int return_value = 0;

    for (int i = 1; i < argc; i++)
    {
        FILE *fp = fopen(argv[i], "r");
        if (strcmp(argv[i], "-") == 0)
        {
            fp = stdin;
        }
        if (fp == NULL)
        {
            int saved = errno; /* 先存起來：之後呼叫的函式可能改掉 errno */
            errno = saved;
            fprintf(stderr, "mycat: <%s>: <%s>\n", argv[i], strerror(saved));
            return_value = 1;
        }
        else
        {
            while ((c = fgetc(fp)) != EOF)
            {
                putchar(c);
            }
            if (ferror(fp))
            {
                int saved = errno; /* 先存起來：之後呼叫的函式可能改掉 errno */
                errno = saved;
                fprintf(stderr, "mycat: <%s>: <%s>\n", argv[i], strerror(saved));
                return_value = 1;
            }
            fclose(fp);
        }
    }

    if (argc == 1)
    {
        while ((c = fgetc(stdin)) != EOF)
        {
            putchar(c);
        }
    }
    return return_value;
}