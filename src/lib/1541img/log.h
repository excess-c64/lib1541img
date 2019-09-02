#ifndef LOG_H
#define LOG_H

#include <1541img/log.h>

void logmsg(LogLevel level, const char *message);
void logfmt(LogLevel level, const char *format, ...);
void logsetsilent(int silent);

#endif

