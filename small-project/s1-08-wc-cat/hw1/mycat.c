#include <stdio.h>
#include <errno.h>
#include <string.h>

int call_one(char *filename)
{
    FILE *fp;
    int err = 0;
    char c = 0;

    fp = (strcmp(filename, "-") == 0) ? stdin : fopen(filename, "r");

    if (fp == NULL)
        err = errno;
    else
    {
        while ((c = fgetc(fp)) != EOF)
        {
            putchar(c);
        }
        if (ferror(fp))
        {
            err = errno;
        }
        if (fp != stdin)
        {
            fclose(fp);
        }
    }

    if (err != 0)
    {
        fprintf(stderr, "mycat: %s: %s\n", filename, strerror(err));
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int return_value = 0;

    if (argc == 1)
        return call_one("-");

    for (int i = 1; i < argc; i++)
        return_value |= call_one(argv[i]);

    

    return return_value;
}
