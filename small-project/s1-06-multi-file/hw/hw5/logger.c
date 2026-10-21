#include "logger.h"
#include <stdio.h>

int log_level = 2;

void log_msg(int level, const char *text)
{
    if (level <= log_level) {
        printf("[%d] %s\n", level, text);
    }
}