#ifndef __LOGGER__
#define __LOGGER__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <pthread.h>

#include "list.h"


#define MAX_LEVEL_LEN                 7
#define DATE_TIME_LEN                 11
#define DATE_SEC_TIME_LEN             20
#define FILE_NAME_AND_LINE_LEN        512
#define MAX_PATH_LEN                  1024
#define LOG_FILE_PATH_LEN             (MAX_PATH_LEN + DATE_TIME_LEN)

enum { LOG_LEVEL_NONE, LOG_LEVEL_TRACE, LOG_LEVEL_DEBUG, LOG_LEVEL_INFO, LOG_LEVEL_WARN, LOG_LEVEL_ERROR, LOG_LEVEL_FATAL };

struct Logger_st;

typedef struct Logger_st Logger;

Logger * logger_new();
Logger * logger_new_with_path_level(const char *log_path, const int lv);
void logger_set_path_level(Logger *logger, const char *log_path, const int lv);
void logger_free(Logger *logger);
void logger_log(Logger *logger, int lv, const char *srcfile, const int line, const char *fmt, ...);

#define LOGGER_trace(logger, ...) logger_log(logger, LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define LOGGER_debug(logger, ...) logger_log(logger, LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOGGER_info(logger, ...)  logger_log(logger, LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOGGER_warn(logger, ...)  logger_log(logger, LOG_LEVEL_WARN, __FILE__, __LINE__,  __VA_ARGS__)
#define LOGGER_error(logger, ...) logger_log(logger, LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOGGER_fatal(logger, ...) logger_log(logger, LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#endif

