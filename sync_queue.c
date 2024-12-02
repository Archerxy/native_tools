#include "sync_queue.h"

#ifndef intptr_t
#define intptr_t long long
#endif

typedef struct QueueNode_st {
    void *data;
    struct QueueNode_st *next;
} QueueNode;

typedef struct SyncQueue_st {
    atomic_intptr_t head;
    atomic_intptr_t tail;
} SyncQueue;

static QueueNode* queue_node_new(void *data) {
    QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));
    if (!node) {
        return NULL;
    }
    node->data = data;
    node->next = NULL;
    return node;
}

SyncQueue * sync_queue_new() {
    SyncQueue *queue = (SyncQueue *)malloc(sizeof(SyncQueue));
    QueueNode *dummy = queue_node_new(NULL);
    atomic_store(&queue->head, (intptr_t)dummy);
    atomic_store(&queue->tail, (intptr_t)dummy);
}

void sync_queue_in(SyncQueue *queue, void *data) {
    QueueNode *new_node = queue_node_new(data);
    if (!new_node) return;

    QueueNode *tail;
    QueueNode *next;

    while (1) {
        tail = (QueueNode *)atomic_load(&queue->tail);
        next = tail->next;
        if (next == NULL) {
            if (atomic_compare_exchange_weak(&tail->next, &next, new_node)) {
                atomic_compare_exchange_weak(&queue->tail, (intptr_t *)&tail, (intptr_t)new_node);
                return;
            }
        } else {
            atomic_compare_exchange_weak(&queue->tail, (intptr_t *)&tail, (intptr_t)next);
        }
    }
}

void* sync_queue_out(SyncQueue *queue) {
    QueueNode *head;
    QueueNode *tail;
    QueueNode *next;
    void *data;

    while (1) {
        head = (QueueNode *)atomic_load(&queue->head);
        tail = (QueueNode *)atomic_load(&queue->tail);
        next = head->next;
        if (head == tail) {
            if (next == NULL) {
                return NULL; 
            }
            atomic_compare_exchange_weak(&queue->tail, (intptr_t *)&tail, (intptr_t)next);
        } else {
            data = next->data;
            if (atomic_compare_exchange_weak(&queue->head, (intptr_t *)&head, (intptr_t)next)) {
                free(head); 
                return data;
            }
        }
    }
}

void sync_queue_free(SyncQueue *queue) {
    QueueNode *node = (QueueNode *)atomic_load(&queue->head);
    while (node) {
        QueueNode *next = node->next;
        free(node);
        node = next;
    }
    queue->head = 0;
    queue->tail = 0;
    free(queue);
}