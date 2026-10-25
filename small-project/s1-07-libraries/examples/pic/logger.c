#include <stdio.h>

void log_msg(const char *s)
{
    printf("[log] %s\n", s);
}

void run(void)
{
    log_msg("start");
    log_msg("stop");
}
