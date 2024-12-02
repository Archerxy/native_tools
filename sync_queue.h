#ifndef _SYNC_QUEUE_H_
#define _SYNC_QUEUE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>

struct SyncQueue_st;
typedef struct SyncQueue_st SyncQueue;

SyncQueue * sync_queue_new();
void sync_queue_in(SyncQueue *queue, void *data);
void* sync_queue_out(SyncQueue *queue);
void sync_queue_free(SyncQueue *queue);

#endif
