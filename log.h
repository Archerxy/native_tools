#ifndef __LOG__
#define __LOG__

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


#define MAX_LEVEL_LEN                 7
#define DATE_TIME_LEN                 11
#define DATE_SEC_TIME_LEN             20
#define FILE_NAME_AND_LINE_LEN        512
#define MAX_PATH_LEN                  1024
#define LOG_FILE_PATH_LEN             (MAX_PATH_LEN + DATE_TIME_LEN)

enum { LOG_NONE, LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

void LOG_init(const char *path, const int lv);
void LOG_log(int lv, const char *srcfile, const int line, const char *fmt, ...);
void LOG_console(int lv, const char *fmt, ...);

#define console_out(...) LOG_console(LOG_INFO, __VA_ARGS__)
#define console_warn(...) LOG_console(LOG_WARN, __VA_ARGS__)
#define console_err(...) LOG_console(LOG_ERROR, __VA_ARGS__)

#define LOG_trace(...) LOG_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_debug(...) LOG_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_info(...)  LOG_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_warn(...)  LOG_log(LOG_WARN, __FILE__, __LINE__,  __VA_ARGS__)
#define LOG_error(...) LOG_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_fatal(...) LOG_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)


#endif

