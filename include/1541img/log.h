#ifndef I1541_LOG_H
#define I1541_LOG_H
#ifdef __cplusplus
extern "C" {
#endif

/** Declarations for the Logging module
 * @file
 */

/** @defgroup Logging Logging
 * Library output facilities
 *
 * `#include <1541img/log.h>`
 * @{
 *
 * This module contains functions to control how lib1541img will output
 * informative, diagnostic and error messages.
 *
 * If you don't call any of the functions from this module, lib1541img will
 * stay completely silent.
 *
 */

#include <stdio.h>

/** The log level of a message
 */
typedef enum LogLevel
{
    L_FATAL,    /**< program execution can't continue */
    L_ERROR,    /**< an error message, can't successfully complete */
    L_WARNING,  /**< a warning message, something seems wrong */
    L_INFO,     /**< an information message */
    L_DEBUG     /**< a debugging message, very verbose */
} LogLevel;

/** Delegate for writing log messages from the library
 * @param level the log level of the message
 * @param message the actual message
 * @param data some additional data you might need internally
 */
typedef void (*logwriter)(LogLevel level, const char *message, void *data);

/** Setup logging to a file.
 * This will add a simple log writer appending to an opened file. You may
 * supply one of the standard streams stdout or stderr here. For example,
 * simple console output could be configured like this:
 * 
 *     setFileLogger(stderr);
 *
 * @param file the file to write messages to
 */
void setFileLogger(FILE *file);

/** Setup logging using your own function
 * @param writer your log writing function
 * @param data some additional data to pass to your writing function
 */
void setCustomLogger(logwriter writer, void *data);

/** Configure the maximum log level
 * lib1541img will suppress any messages less severe than the level given
 * here. By default, DEBUG messages will be suppressed.
 * @param level the maximum level to generate messages for
 */
void setMaxLogLevel(LogLevel level);

/**@}*/

#ifdef __cplusplus
}
#endif
#endif

