#include "reentrant_lock.h"

typedef struct ReentrantLockNode_st {
    struct ReentrantLockNode_st* next;
} ReentrantLockNode;

typedef struct ReentrantLock_st {
    ReentrantLockNode*    queue_head; 
    ReentrantLockNode*    queue_tail; 
    pthread_mutex_t       mutex; 
    pthread_cond_t        cond;
    int                   fair_lock;
} ReentrantLock;

ReentrantLock * reentrantlock_new(int fair) {
    ReentrantLock* lock = (ReentrantLock *)malloc(sizeof(ReentrantLock));
    lock->queue_head = NULL;
    lock->queue_tail = NULL;
    lock->fair_lock = fair;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&lock->mutex, &attr);
    pthread_cond_init(&lock->cond, NULL);
    return lock;
}

void reentrantlock_acquire(ReentrantLock *lock) { 
    if(!lock->fair_lock) {
        pthread_mutex_lock(&lock->mutex);
        return ; 
    }
    ReentrantLockNode *new_node = (ReentrantLockNode *)malloc(sizeof(ReentrantLockNode)); 
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


void reentrantlock_release(ReentrantLock *lock) { 
    if(!lock->fair_lock) {
        pthread_mutex_unlock(&lock->mutex);
        return ; 
    }
    int signal = 0;
    pthread_mutex_lock(&lock->mutex); 
    if(lock->queue_head != NULL) {
        ReentrantLockNode *next = lock->queue_head->next; 
        ReentrantLockNode *head = lock->queue_head; 
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

void reentrantlock_destroy(ReentrantLock* lock) {
    pthread_mutex_destroy(&lock->mutex);
    pthread_cond_destroy(&lock->cond);
    if(!lock->fair_lock) {
        return ; 
    }
    ReentrantLockNode *node = lock->queue_head;
    ReentrantLockNode *next = NULL;
    while(node != NULL) {
        next = node->next;
        free(node);
        node = next;
    }
    lock->queue_head = NULL;
    lock->queue_tail = NULL;
}