#ifndef _FAIR_LOCK_H_
#define _FAIR_LOCK_H_

#include <pthread.h>
#include <stdlib.h>

struct FairLockNode_st;
struct FairLock_st;

typedef struct FairLockNode_st FairLockNode;
typedef struct FairLock_st FairLock;


FairLock * fair_lock_new();
void fair_lock_acquire(FairLock *lock);
void fair_lock_release(FairLock *lock);
void fair_lock_destroy(FairLock* lock);

#endif

