int main(void)
{
    FILE *f = fopen("/etc/hostname", "r");
    if (f == NULL)
        return 1;
    fclose(f);
    printf("EOF = %d\n", EOF);
    return 0;
}
