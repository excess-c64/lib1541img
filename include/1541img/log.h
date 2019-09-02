#ifndef I1541_LOG_H
#define I1541_LOG_H

#include <stdio.h>

typedef enum LogLevel
{
    L_FATAL,
    L_ERROR,
    L_WARNING,
    L_INFO,
    L_DEBUG
} LogLevel;

typedef void (*logwriter)(LogLevel level, const char *message, void *data);

void setFileLogger(FILE *file);
void setCustomLogger(logwriter writer, void *data);
void setMaxLogLevel(LogLevel level);

#endif

