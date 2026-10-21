#ifndef LOGGER_H
#define LOGGER_H

extern int log_level;

void log_msg(int level, const char *text);

#endif // LOGGER_H