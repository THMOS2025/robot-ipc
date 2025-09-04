#ifndef ROBOT_LOGGER_H
#define ROBOT_LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>


// Log levels
typedef enum {
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} LogLevel;

void robot_log(int code, const char *fmt, ...);

#endif // ROBOT_LOGGER_H
