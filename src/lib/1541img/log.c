#include <stdio.h>
#include <stdarg.h>

#include "log.h"

static void nowrite(LogLevel level, const char *message, void *data)
{
    (void)level; // unused
    (void)message; // unused
    (void)data; // unused
}

static logwriter currentwriter = nowrite;
static void *writerdata;
static LogLevel maxlevel = L_INFO;
static int logsilent = 0;

static const char *levels[] =
{
    "[FATAL]  ",
    "[ERROR]  ",
    "[WARN ]  ",
    "[INFO ]  ",
    "[DEBUG]  "
};

static void writeFile(LogLevel level, const char *message, void *data)
{
    FILE *target = data;
    fputs(levels[level], target);
    fputs(message, target);
    fputc('\n', target);
    fflush(target);
}

void logmsg(LogLevel level, const char *message)
{
    if (logsilent && level > L_ERROR) return;
    if (level > maxlevel) return;
    currentwriter(level, message, writerdata);
}

void logfmt(LogLevel level, const char *format, ...)
{
    if (logsilent && level > L_ERROR) return;
    if (level > maxlevel) return;
    char buf[8192];
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, 8192, format, ap);
    va_end(ap);
    logmsg(level, buf);
}

void logsetsilent(int silent)
{
    logsilent = silent;
}

void setFileLogger(FILE *file)
{
    currentwriter = writeFile;
    writerdata = file;
}

void setCustomLogger(logwriter writer, void *data)
{
    currentwriter = writer;
    writerdata = data;
}

void setMaxLogLevel(LogLevel level)
{
    maxlevel = level;
}

