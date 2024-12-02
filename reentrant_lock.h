#ifndef _REENTRANT_LOCK_H_
#define _REENTRANT_LOCK_H_

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include <pthread.h>
#include <stdlib.h>

struct ReentrantLockNode_st;
struct ReentrantLock_st;

typedef struct ReentrantLockNode_st ReentrantLockNode;
typedef struct ReentrantLock_st ReentrantLock;


ReentrantLock * reentrantlock_new(int fair);
void reentrantlock_acquire(ReentrantLock *lock);
void reentrantlock_release(ReentrantLock *lock);
void reentrantlock_destroy(ReentrantLock* lock);

#endif

