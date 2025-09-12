#include "robot_ipc_logger.h"
#include "robot_ipc_constant.h"


// Map your code to log level
static LogLevel code_to_level(int code) {
    if (code >= 1 && code < 100) {
        return LOG_LEVEL_INFO;
    } else if (code >= 100 && code < 200) {
        return LOG_LEVEL_WARN;
    } else if (code >= 200 && code < 300) {
        return LOG_LEVEL_ERROR;
    } else if (code >= 300) {
        return LOG_LEVEL_FATAL;
    } else {
        return LOG_LEVEL_WARN;
    }
}

// Get log level string
static const char* log_level_str(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_INFO:  return "INFO";
        case LOG_LEVEL_WARN:  return "WARN";
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_FATAL: return "FATAL";
        default:              return "UNKNOWN";
    }
}

// Main log function
void robot_log(int code, const char *fmt, ...) {
    if (code == ROBOT_OK) {
        return; // Nothing to log
    }


    // Get current time
    time_t now = time(NULL);
    struct tm *timenow = localtime(&now);

    LogLevel level = code_to_level(code);

    // Print timestamp, log level, and code
    printf("[%04d-%02d-%02d %02d:%02d:%02d] [%s] (code=%d) ",
        timenow->tm_year + 1900, timenow->tm_mon + 1, timenow->tm_mday,
        timenow->tm_hour, timenow->tm_min, timenow->tm_sec,
        log_level_str(level), code);

    // Print user log message
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
}
