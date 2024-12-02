#ifndef __LIST__
#define __LIST__

#include <stdlib.h>

struct ListNode_st;
struct List_st;

typedef struct ListNode_st ListNode;
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

#endif

