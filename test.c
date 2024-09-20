#ifndef __TEST__
#define __TEST__

#include "log.h"
#include "map.h"
#include <string.h>
#include <stdio.h>

#define my_put(m,a,b) \
        { \
        void *p = (void *)b; \
        map_put(m,a,p); \
        } \


void log_test() {

    LOG_init(NULL, LOG_TRACE);

    console_out("nihao %s", "console out");

    console_err("nihao %s", "console err");

    LOG_debug("nihao %s", "debug");

    LOG_info("nihao %s", "info");

    
    LOG_warn("nihao %s", "warn");

    sleep(2);
    
    console_warn("nihao %s", "console warn return");
}

void print_map_cb(char *key, void *arg) {
    int *v = arg;
    printf("key = %s, val = %d  ", key, *v);
}

void print_map(Map *map) {
    printf("********\n");
    map_foreach(map, print_map_cb);
    printf("********\n");
}

void map_test() {
    Map *map = map_new();
    void *arg = NULL;
    int a = 1, b =2, c = 3, d = 4, e = 5;
    arg = map_put(map, "a", &a);
    // my_put(map, "a", 1);
    if(arg != NULL) {
        printf("put a not null val = %d\n", *((int *)arg));
    }
    arg = map_put(map, "b", &b);
    if(arg != NULL) {
        printf("put b not null val = %d\n", *((int *)arg));
    }
    arg = map_put(map, "c", &c);
    if(arg != NULL) {
        printf("put c not null val = %d\n", *((int *)arg));
    }
    arg = map_put(map, "d", &d);
    if(arg != NULL) {
        printf("put d not null val = %d\n", *((int *)arg));
    }
    arg = map_put(map, "e", &e);
    if(arg != NULL) {
        printf("put e not null val = %d\n", *((int *)arg));
    }

    print_map(map);

    arg = map_remove(map, "h");
    if(arg != NULL) {
        printf("remove h not null val = %d\n", *((int *)arg));
    }

    arg = map_remove(map, "b");
    if(arg != NULL) {
        printf("remove b not null val = %d\n", *((int *)arg));
    }

    arg = map_remove(map, "d");
    if(arg != NULL) {
        printf("remove d not null val = %d\n", *((int *)arg));
    }
    
    print_map(map);
    
    arg = map_get(map, "d");
    if(arg != NULL) {
        printf("get d not null val = %d\n", *((int *)arg));
    }
    arg = map_get(map, "c");
    if(arg != NULL) {
        printf("get c not null val = %d\n", *((int *)arg));
    }
    printf("size = %d\n", map_size(map));

    map_free(map, NULL);
}


int main() {
    log_test();
    map_test();
    return 0;
}



#endif