#ifndef __TEST__
#define __TEST__

#include "archer_tool.h"
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


void *threadFunc0(void *arg) {
    ReentrantLock *fair_lock = arg;
    reentrantlock_acquire(fair_lock);
    printf("func0\n");
    sleep(1);
    reentrantlock_release(fair_lock);
}
void *threadFunc1(void *arg) {
    ReentrantLock *fair_lock = arg;
    reentrantlock_acquire(fair_lock);
    printf("func1\n");
    sleep(1);
    reentrantlock_release(fair_lock);
}
void *threadFunc2(void *arg) {
    ReentrantLock *fair_lock = arg;
    reentrantlock_acquire(fair_lock);
    printf("func2\n");
    sleep(1);
    reentrantlock_release(fair_lock);
}
void *threadFunc3(void *arg) {
    ReentrantLock *fair_lock = arg;
    reentrantlock_acquire(fair_lock);
    printf("func3\n");
    sleep(1);
    reentrantlock_release(fair_lock);
}

void reentrantLockTest() {
    ReentrantLock *fair_lock = reentrantlock_new();
    pthread_t t0, t1, t2, t3;
    pthread_create(&t0, NULL, threadFunc0, fair_lock);
    pthread_create(&t1, NULL, threadFunc1, fair_lock);
    pthread_create(&t2, NULL, threadFunc2, fair_lock);
    pthread_create(&t3, NULL, threadFunc3, fair_lock);

    printf("t0 id = %ld\n", t0);
    printf("t1 id = %ld\n", t1);
    printf("t2 id = %ld\n", t2);
    printf("t3 id = %ld\n", t3);

    sleep(5);
}


void jsonTest() {
    const char *s = "{\"the\":{\"ret\":true,\"b\":10,\"code\":100,\"money\":123.4,\"msg\":\"mm\\\"} \\\"mss\",\"t\":\"2024-10-31 13:07:51\",\"list\":[[{\"ret\":true,\"b\":10,\"code\":100,\"money\":123.4,\"msg\":\"mm\\\"} \\\"mss\",\"t\":\"2024-10-31 13:07:51\",\"list\":null,\"strs\":null},{\"ret\":true,\"b\":10,\"code\":100,\"money\":123.4,\"msg\":\"mm\\\"} \\\"mss\",\"t\":\"2024-10-31 13:07:51\",\"list\":null,\"strs\":null}]],\"strs\":null}}";
	JsonVal *root = json_new();
    json_parse(root, s);
    JsonVal *the = json_get_object_item(root, "the");


    JsonVal *moneyval = json_get_object_item(the, "money");
    printf("the.money = %lf\n", json_get_double(moneyval));

    JsonVal *msgval = json_get_object_item(the, "msg");
    printf("the.money = %s\n", json_get_string(msgval));

    JsonVal *listArr = json_get_object_item(the, "list");
    if(!listArr) {
        printf("the.list is NULL\n");
        return ;
    }
    JsonVal *list0 = json_get_array_item(listArr, 0);
    if(!list0) {
        printf("the.list[0] is NULL\n");
        return ;
    }    
    JsonVal *list00 = json_get_array_item(list0, 0);
    if(!list00) {
        printf("the.list[0][0] is NULL\n");
        return ;
    }
    JsonVal *list00Msg = json_get_object_item(list00, "msg");
    if(!list00Msg) {
        printf("the.list[0].msg is NULL\n");
        return ;
    }
    printf("the.list[0][0].msg = %s\n", json_get_string(list00Msg));

    json_delete_item_from_object(the, "list");
    char *jsonstr = json_stringify(the);
    printf("%s\n", jsonstr);


    JsonVal *newVal = json_new();
    json_parse(newVal, jsonstr);

    JsonVal *newMsg = json_get_object_item(newVal, "msg");
    printf("newMsg.msg = %s\n", json_get_string(newMsg));
}

// gcc *.c -lpthread -o test.exe
int main() {
    // log_test();

    // map_test();
    // reentrantLockTest();
    jsonTest();
    return 0;
}



#endif
