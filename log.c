#include "log.h"
#include "list.h"

static char log_root_path[MAX_PATH_LEN];
static int log_root_path_len = 0;
static int log_default_level = LOG_INFO;
static pthread_mutex_t log_mutex;
static pthread_cond_t log_cond;
static pthread_t log_thread;
static List  *log_list = NULL;

static const char *level_strings[] = {
  "[NONE]", "[TRACE]", "[DEBUG]", "[INFO]", "[WARN]", "[ERROR]", "[FATAL]"
};

static void LOG_line_to_str(int num, char *str, int *str_len) {
    if(num == 0) {
        str[0] = '0';
        *str_len = 1;
        return ;
    }
    char map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    if(num / 10000 > 0) {
        *str_len = 5;
    } else if(num / 1000 > 0) {
        *str_len = 4;
    } else if(num / 100 > 0) {
        *str_len = 3;
    } else if(num / 10 > 0) {
        *str_len = 2;
    } else {
        *str_len = 1;
    }
    int off = (*str_len) - 1;
    while(num > 0) {
        str[off--] = map[num % 10];
        num /= 10;
    }
}

static void do_mkdir(const char *path) {
    if(access(path, 0)) {
#ifdef _WIN32
        mkdir(path);
#else
        mkdir(path, S_IRWXU);
#endif
    }
}

static void do_mkdirs(const char *path) {
    size_t len = strlen(path);
    char *dir = (char *) malloc(len + 1);
    memcpy(dir, path, len);
    dir[len] = '\0';
    if(dir[len - 1] == 92 || dir[len - 1] == 47) {
        dir[len - 1] = '\0';
        --len;
    }
    for(int i = 1; i < len; i++) {
        if(dir[i] == 47 || dir[i] == 92) {
            dir[i] = 0;
            do_mkdir(dir);
            dir[i] = '/';
        }
    }
    do_mkdir(dir);
}

static int is_absolute_path(const char *path) {
#ifdef __WIN32
    return (isalpha(path[0]) && path[1] == ':') || (path[0] == 92 && path[1] == 92);
#else
    return (path[0] == '/');
#endif
}

static void LOG_get_now_date_time(char *date) {
    time_t t = time(NULL);
    strftime(date, DATE_SEC_TIME_LEN, "%Y-%m-%d %H:%M:%S", localtime(&t));
}

static void LOG_init_root_path(const char *path) {
    if(path) {
        size_t path_len = strlen(path);
        if(is_absolute_path(path)) {
            if(path[path_len - 1] == 47 || path[path_len - 1] == 92) {
                --path_len;
            }
            memcpy(log_root_path, path, path_len);
            log_root_path_len = path_len;
        } else {
            if(getcwd(log_root_path, MAX_PATH_LEN - 1)) {}
            log_root_path_len = strlen(log_root_path);
            if(path[0] != 47 && path[0] != 92) {
                log_root_path[log_root_path_len++] = '/';
            }
            if(path[path_len - 1] == 47 || path[path_len - 1] == 92) {
                --path_len;
            }
            memcpy(log_root_path + log_root_path_len, path, path_len);
            log_root_path_len += path_len;
        }
    } else {
        if(getcwd(log_root_path, MAX_PATH_LEN - 1)) {}
        log_root_path_len = strlen(log_root_path);
        log_root_path[log_root_path_len++] = '/';
        log_root_path[log_root_path_len++] = 'l';
        log_root_path[log_root_path_len++] = 'o';
        log_root_path[log_root_path_len++] = 'g';
        log_root_path[log_root_path_len++] = 's';
    }
    log_root_path[log_root_path_len] = '\0';
    do_mkdirs(log_root_path);
}

static int LOG_get_file(char *file_path, char *date) {
    memcpy(file_path, log_root_path, log_root_path_len);
    int off = log_root_path_len;
    file_path[off++] = '/';
    memcpy(&file_path[off], date, DATE_TIME_LEN - 1);
    off += DATE_TIME_LEN - 1;
    file_path[off++] = '.';
    file_path[off++] = 'l';
    file_path[off++] = 'o';
    file_path[off++] = 'g';
    file_path[off] = '\0';
    return off;
}

static char * simplify_filename(const char *filename) {
    size_t len = strlen(filename);
    long off = len - 1;
    while(off >= 0 && filename[off] != 47 && filename[off] != 92) {
        --off;
    }
    char *new_name =  (char *)malloc(len - off);
    memcpy(new_name, filename + off + 1, len - off - 1);
    new_name[len - off - 1] = '\0';
    return new_name;
}


static void * logger_thread_fn(void *arg) {
    char *log_msg = NULL;
    char date[DATE_SEC_TIME_LEN];
    char file_path[LOG_FILE_PATH_LEN];
    int len = 0;
    while(1) {
        pthread_mutex_lock(&log_mutex);
        log_msg = list_unshift(log_list);
        if(log_msg == NULL) {
            pthread_cond_wait(&log_cond, &log_mutex); 
        }
        pthread_mutex_unlock(&log_mutex);

        if(log_msg == NULL) {
            continue ;
        }
        LOG_get_now_date_time(date);
        len = LOG_get_file(file_path, date);
        
        FILE *f = fopen(file_path, "ab");
        if(!f) {
            continue ;
        }
        fprintf(f, "%s\n", log_msg);
        fflush(f);
        fclose(f);
        free(log_msg);
    }
    return NULL;
}


void LOG_console(int lv, const char *fmt, ...) {
    char log_msg[MAX_LEVEL_LEN + FILE_NAME_AND_LINE_LEN + DATE_SEC_TIME_LEN], date[DATE_SEC_TIME_LEN];
    const char *level = level_strings[lv];
    size_t off = 0, lv_len = strlen(level);
    LOG_get_now_date_time(date);
    date[DATE_SEC_TIME_LEN - 1] = '\0';
    if(lv >= LOG_WARN) {
        log_msg[off++] = '\033';
        log_msg[off++] = '[';
        log_msg[off++] = '3';
        log_msg[off++] = '1';
        log_msg[off++] = 'm';
    }

    memcpy(log_msg + off, level, lv_len);
    off += lv_len;
    log_msg[off++] = '[';
    memcpy(log_msg + off, date, DATE_SEC_TIME_LEN - 1);
    off += DATE_SEC_TIME_LEN - 1;
    log_msg[off++] = ']';
    log_msg[off++] = '\0';
    
    fprintf(stdout, "%s", log_msg);
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
    if(lv >= LOG_WARN) {
        fprintf(stdout, "%s", "\033[0m");
    }
    fflush(stdout);
}


void LOG_init(const char *path, const int lv) {
    log_default_level = (lv >= LOG_TRACE && lv <= LOG_FATAL) ? lv : LOG_INFO;
    LOG_init_root_path(path);
    log_list = list_new();
    pthread_mutex_init(&log_mutex, NULL);
    pthread_cond_init(&log_cond, NULL);

    pthread_create(&log_thread, NULL, logger_thread_fn, NULL);
}

void LOG_log(int lv, const char *filename, const int line, const char *fmt, ...) {
    if(log_list == NULL) {
        return ;
    }
    if(lv < LOG_TRACE || lv > LOG_FATAL) {
        return ;
    }
    if(lv < log_default_level) {
        return ;
    }

    char date[DATE_SEC_TIME_LEN];
    LOG_get_now_date_time(date);
    char file_path[LOG_FILE_PATH_LEN];
    int len = LOG_get_file(file_path, date);

    char log_msg[MAX_LEVEL_LEN + FILE_NAME_AND_LINE_LEN + DATE_SEC_TIME_LEN];
    const char *level = level_strings[lv];
    char *file_name = simplify_filename(filename);
    size_t off = strlen(level), file_len = strlen(file_name);
    int line_str_len = 0;

    memcpy(log_msg, level, off);
    log_msg[off++] = '[';
    memcpy(log_msg + off, date, DATE_SEC_TIME_LEN - 1);
    off += DATE_SEC_TIME_LEN - 1;
    log_msg[off++] = ']';
    log_msg[off++] = '[';
    memcpy(log_msg + off, file_name, file_len);
    free(file_name);
    off += file_len;
    log_msg[off++] = ':';

    LOG_line_to_str(line, log_msg + off, &line_str_len);
    off += line_str_len;

    log_msg[off++] = ']';
    log_msg[off++] = ' ';
    log_msg[off++] = '\0';

    va_list args;
    va_start(args, fmt);
    int size = vsnprintf(NULL, 0, fmt, args) + off;
    va_end(args);
    char* log_line = (char*)malloc(size);
    memcpy(log_line, log_msg, off - 1);
    va_start(args, fmt);
    vsnprintf(log_line + off - 1, size, fmt, args);
    va_end(args);

    pthread_mutex_lock(&log_mutex);
    list_push(log_list, log_line);
    pthread_mutex_unlock(&log_mutex);
    
    pthread_cond_signal(&log_cond);
}