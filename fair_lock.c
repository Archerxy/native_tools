#include "fair_lock.h"

typedef struct FairLockNode_st {
    struct FairLockNode_st* next;
} FairLockNode;

typedef struct FairLock_st {
    FairLockNode*    queue_head; 
    FairLockNode*    queue_tail; 
    pthread_mutex_t  mutex; 
    pthread_cond_t   cond;
} FairLock;

FairLock * fair_lock_new() {
    FairLock* lock = (FairLock *)malloc(sizeof(FairLock));
    lock->queue_head = NULL;
    lock->queue_tail = NULL;
    pthread_mutex_init(&lock->mutex, NULL);
    pthread_cond_init(&lock->cond, NULL);
    return lock;
}

void fair_lock_acquire(FairLock *lock) { 
    FairLockNode *new_node = (FairLockNode *)malloc(sizeof(FairLockNode)); 
    new_node->next = NULL; 
    pthread_mutex_lock(&lock->mutex); 
    if (lock->queue_tail == NULL) { 
        lock->queue_head = new_node; 
    } else { 
        lock->queue_tail->next = new_node; 
    } 
    lock->queue_tail = new_node; 
    while (lock->queue_head != new_node) { 
        pthread_cond_wait(&lock->cond, &lock->mutex); 
    } 
    pthread_mutex_unlock(&lock->mutex); 
}


void fair_lock_release(FairLock *lock) { 
    int signal = 0;
    pthread_mutex_lock(&lock->mutex); 
    if(lock->queue_head != NULL) {
        FairLockNode *next = lock->queue_head->next; 
        FairLockNode *head = lock->queue_head; 
        if(lock->queue_head == lock->queue_tail) {
            lock->queue_head = next;
            lock->queue_tail = next;
        } else {
            lock->queue_head = next;
        }
        free(head); 
        signal = 1;
    }
    pthread_mutex_unlock(&lock->mutex); 
    if(signal) {
        pthread_cond_signal(&lock->cond); 
    }
}

void fair_lock_destroy(FairLock* lock) {
    pthread_mutex_destroy(&lock->mutex);
    pthread_cond_destroy(&lock->cond);
    FairLockNode *node = lock->queue_head;
    FairLockNode *next = NULL;
    while(node != NULL) {
        next = node->next;
        free(node);
        node = next;
    }
    lock->queue_head = NULL;
    lock->queue_tail = NULL;
}