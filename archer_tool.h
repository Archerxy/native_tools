#ifndef _ARCHER_TOOL_H_
#define _ARCHER_TOOL_H_

#ifdef _WIN32
#include <Windows.h>
#else

#include <ucontext.h>
#endif

#include <stddef.h>
#include <stdint.h>
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
#include <stdatomic.h>

struct ReentrantLock_st;
typedef struct ReentrantLock_st ReentrantLock;

ReentrantLock * reentrantlock_new(int fair);
void reentrantlock_acquire(ReentrantLock *lock);
void reentrantlock_release(ReentrantLock *lock);
void reentrantlock_destroy(ReentrantLock* lock);




struct List_st;
typedef struct List_st List;

typedef void (*list_foreach_cb)(const void *obj);
typedef void (*list_obj_free_cb)(void *obj);

List * list_new();
void list_free(List *list, list_obj_free_cb free_cb);
void list_push(List *list, void *obj);
void* list_pop(List *list);
void list_shift(List *list, void *obj);
void* list_unshift(List *list);
void list_foreach(List *list, list_foreach_cb fn);
void* list_get(List *list, int index);
size_t list_size(List *list);




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





struct Map_st;
typedef struct Map_st Map;

typedef void (*map_foreach_cb)(char * key, void *val);
typedef void (*map_key_val_free_cb)(char * key, void *val);

Map * map_new();
void map_free(Map *map, map_key_val_free_cb free_cb);
void map_foreach(Map *map, map_foreach_cb fn);
void* map_put(Map *map, char *key, void *val);
void * map_get(Map *map, char *key);
void * map_remove(Map *map, char *key);
size_t map_size(Map *map);



struct JsonVal_st;
typedef struct JsonVal_st JsonVal;


JsonVal *json_new(void);
int json_get_type(JsonVal *c);
void json_delete(JsonVal *c);
void json_minify(char *json);

void json_parse(JsonVal *c, const char *value);
char *json_stringify(const JsonVal *item);
const char *json_get_errstr(JsonVal *c);

JsonVal *json_get_array_item(const JsonVal *array, int index);
JsonVal *json_get_object_item(const JsonVal *object, const char *key);

void json_add_item_to_object(JsonVal *object, const char *key, JsonVal *item);
void json_add_item_to_array(JsonVal *array, JsonVal *item);

void json_delete_item_from_array(JsonVal *array, int index);
void json_delete_item_from_object(JsonVal *object, const char *key);

void json_add_null_to_object(JsonVal *object, const char *key);
void json_add_true_to_object(JsonVal *object, const char *key);
void json_add_false_to_object(JsonVal *object, const char *key);
void json_add_bool_to_object(JsonVal *object, const char *key, int bool);
void json_add_int_to_object(JsonVal *object, const char *key, int num);
void json_add_double_to_object(JsonVal *object, const char *key, double num);
void json_add_string_to_object(JsonVal *object, const char *key, const char *str);

int json_get_int(JsonVal *c);
double json_get_double(JsonVal *c);
const char * json_get_string(JsonVal *c);




struct SyncQueue_st;
typedef struct SyncQueue_st SyncQueue;

SyncQueue * sync_queue_new();
void sync_queue_in(SyncQueue *queue, void *data);
void* sync_queue_out(SyncQueue *queue);
void sync_queue_free(SyncQueue *queue);



struct AcoScheduler_st;
typedef struct AcoScheduler_st AcoScheduler;

struct ACoroutine_st;
typedef struct ACoroutine_st ACoroutine;

typedef void (* acoroutine_func)(AcoScheduler *aschd, void * arg);

typedef enum AcoStatus_st {       
    ACO_Dead       = 0,       
    ACO_Ready      = 1,      
    ACO_Running    = 2,     
    ACO_Suspend    = 3,     
} AcoStatus;


extern AcoScheduler *aco_scheduler_start(void);
extern void aco_scheduler_close(AcoScheduler *aschd);
extern ACoroutine * acoroutine_create(AcoScheduler *aschd, const char *name, acoroutine_func func, void * arg);
extern void acoroutine_destroy(ACoroutine *co);
extern void aco_scheduler_resume(AcoScheduler *aschd, ACoroutine *co);
extern void aco_scheduler_yield(AcoScheduler *aschd);
extern AcoStatus acoroutine_status(ACoroutine *co);
extern const char * acoroutine_name(ACoroutine *co);
extern ACoroutine * aco_scheduler_running(AcoScheduler *manager);

#endif

